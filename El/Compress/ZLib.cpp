/*
 * Elements - useful abstractions library.
 * Copyright (C) 2005-2015 Karen Arutyunov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/**
 * @file Elements/El/Compress/ZLib.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <zlib.h>

#include <iostream>
#include <sstream>

#include <ace/OS.h>
#include <ace/High_Res_Timer.h>

#include "ZLib.hpp"

namespace El
{
  namespace Compress
  {
    namespace ZLib
    {
      //
      // OutStreamBuf class
      //
      OutStreamBuf::OutStreamBuf(OutStreamCallback* callback,
                                 int level,
                                 size_t in_buffer_size,
                                 size_t out_buffer_size,
                                 bool raw_deflate,
                                 bool calc_crc)
        throw(InvalidArg, Exception, El::Exception)
          : callback_(callback),
            in_buffer_size_(in_buffer_size),
            out_buffer_size_(out_buffer_size),
            write_pending_bytes_(0),
            in_buffer_(new char[in_buffer_size]),
            out_buffer_(new char[out_buffer_size]),
            in_bytes_(0),
            out_bytes_(0),
            in_crc_(0),
            calc_crc_(calc_crc),
            last_error_(0),
            deflate_initialized_(false),
            finalized_(false)
      {
        if(in_buffer_size < 15 || out_buffer_size < 15)
        {
          //
          // ZLIb API can get into a dead loop if less then 11
          //
          throw InvalidArg("El::Compress::ZLib::OutStreamBuf::OutStreamBuf: "
                           "buffer sizes should not less than 15");
        }

        if(!callback)
        {
          throw InvalidArg("El::Compress::ZLib::OutStreamBuf::OutStreamBuf: "
                           "callback is 0");
        }
        
        memset(&z_stream_, 0, sizeof(z_stream_));

        z_stream_.zalloc = Z_NULL;
        z_stream_.zfree = Z_NULL;
        z_stream_.opaque = Z_NULL;
        
        int ret = raw_deflate ?
          deflateInit2(&z_stream_,
                       level,
                       Z_DEFLATED,
                       -15,
                       9,
                       Z_DEFAULT_STRATEGY) :
          deflateInit(&z_stream_, level);

        if(ret != Z_OK)
        {
          std::ostringstream ostr;
          ostr << "El::Compress::ZLib::OutStreamBuf::OutStreamBuf: "
            "deflateInit failed with code " << ret;

          if(ret == Z_STREAM_ERROR)
          {
            throw InvalidArg(ostr.str());
          }
          else
          {
            throw Exception(ostr.str());
          }
        }

        deflate_initialized_ = true;

        if(calc_crc_)
        {
          in_crc_ = crc32(0L, Z_NULL, 0);
        }
      }

      int
      OutStreamBuf::deflate(int flush) throw()
      {
        if(calc_crc_)
        {
          unsigned char* buff = z_stream_.next_in;
          size_t buff_len = z_stream_.avail_in;
          
          int ret = ::deflate(&z_stream_, flush);

          if(ret == Z_OK || ret == Z_STREAM_END)
          {
            in_crc_ = crc32(in_crc_,
                             (const Bytef*)buff,
                             buff_len - z_stream_.avail_in);
          }

          return ret;
        }
        else
        {
          return ::deflate(&z_stream_, flush);
        }
      }
      
      OutStreamBuf::int_type
      OutStreamBuf::overflow(int_type c)
      {
        if(!last_error_desc_.empty())
        {
          return traits_type::eof();
        }

        if(finalized_)
        {
          std::ostringstream ostr;
          ostr << "El::Compress::ZLib::OutStreamBuf::overflow: "
            "streambuf is already finalized";
            
          last_error_desc_ = ostr.str();
          
          return traits_type::eof();          
        }
        
        if(pptr() == 0)
        {
          in_buffer_[0] = c;
          setp(in_buffer_ + 1, in_buffer_ + in_buffer_size_);
          return c;
        }

        push_write_pending();

        if(write_pending_bytes_ == out_buffer_size_)
        {
          std::ostringstream ostr;
          ostr << "El::Compress::ZLib::OutStreamBuf::overflow: "
            "can't push out deflated data; bytes in " << in_bytes_
               << ", out " << out_bytes_;
            
          last_error_desc_ = ostr.str();
          
          return traits_type::eof();          
        }

        z_stream_.next_in = (unsigned char*)in_buffer_;
        z_stream_.avail_in = in_buffer_size_;
        
        while(z_stream_.avail_in > 0)
        {
          char* out_ptr = out_buffer_ + write_pending_bytes_;
          size_t out_avail = out_buffer_size_ - write_pending_bytes_;
            
          z_stream_.next_out = (unsigned char*)out_ptr;
          z_stream_.avail_out = out_avail;
          
          size_t avail_in = z_stream_.avail_in;
          
          ACE_High_Res_Timer timer;
          timer.start();
          
          int ret = deflate(Z_NO_FLUSH);

          timer.stop();
          
          if(ret != Z_OK)
          {
            std::ostringstream ostr;
            ostr << "El::Compress::ZLib::OutStreamBuf::overflow: "
              "deflate failed with code " << ret;
            
            last_error_desc_ = ostr.str();
            last_error_ = ret;
            
            return traits_type::eof();
          }

          ACE_Time_Value tm;
          timer.elapsed_time(tm);

          compression_time_ += tm;          
          in_bytes_ += avail_in - z_stream_.avail_in;

          write_pending_bytes_ += out_avail - z_stream_.avail_out;
          push_write_pending();          
        }

        if(z_stream_.avail_in == in_buffer_size_)
        {
          std::ostringstream ostr;
          ostr << "El::Compress::ZLib::OutStreamBuf::overflow: "
            "can't push out deflated data; bytes in " << in_bytes_
               << ", out " << out_bytes_;
             
          last_error_desc_ = ostr.str();
          
          return traits_type::eof();          
        }

        if(z_stream_.avail_in)
        {
          memmove(in_buffer_, z_stream_.next_in, z_stream_.avail_in);
        }

        in_buffer_[z_stream_.avail_in] = c;
        
        setp(in_buffer_ + z_stream_.avail_in + 1,
             in_buffer_ + in_buffer_size_);

        return c;
      }
      
      int
      OutStreamBuf::sync()
      {
        if(!last_error_desc_.empty())
        {
          return -1;
        }

        if(finalized_)
        {
          std::ostringstream ostr;
          ostr << "El::Compress::ZLib::OutStreamBuf::sync: "
            "streambuf is already finalized";
            
          last_error_desc_ = ostr.str();
          
          return -1;
        }
        
        push_write_pending();

        z_stream_.next_in = (unsigned char*)in_buffer_;
        z_stream_.avail_in = pptr() ? pptr() - in_buffer_ : 0;

        while(z_stream_.avail_in > 0)
        {
          char* out_ptr = out_buffer_ + write_pending_bytes_;
          size_t out_avail = out_buffer_size_ - write_pending_bytes_;
            
          z_stream_.next_out = (unsigned char*)out_ptr;
          z_stream_.avail_out = out_avail;
          
          size_t avail_in = z_stream_.avail_in;
          
          ACE_High_Res_Timer timer;
          timer.start();
          
          int ret = deflate(Z_NO_FLUSH);

          timer.stop();

          if(ret != Z_OK)
          {
            std::ostringstream ostr;
            ostr << "El::Compress::ZLib::OutStreamBuf::sync: "
              "deflate failed with code " << ret;
            
            last_error_desc_ = ostr.str();
            last_error_ = ret;
            
            return -1;
          }

          ACE_Time_Value tm;
          timer.elapsed_time(tm);
          
          compression_time_ += tm;          
          in_bytes_ += avail_in - z_stream_.avail_in;

          write_pending_bytes_ += out_avail - z_stream_.avail_out;          
          push_write_pending();
        }

        setp(in_buffer_, in_buffer_ + in_buffer_size_);
        
        return flush_zlib(false);
      }

      int
      OutStreamBuf::flush_zlib(bool finish) throw(El::Exception)
      {
        z_stream_.next_in = (unsigned char*)in_buffer_;
        z_stream_.avail_in = 0;

        if(!push_write_pending())
        {
          std::ostringstream ostr;
          ostr << "El::Compress::ZLib::OutStreamBuf::flush_zlib: "
            "can't push out deflated data; bytes in " << in_bytes_
               << ", out " << out_bytes_;
            
          last_error_desc_ = ostr.str();
            
          return -1;
        }
        
        while(true)
        {
          z_stream_.next_out = (unsigned char*)out_buffer_;
          z_stream_.avail_out = out_buffer_size_;
          
          ACE_High_Res_Timer timer;
          timer.start();
          
          int ret = deflate(finish ? Z_FINISH : Z_SYNC_FLUSH);

          timer.stop();

          if(ret != Z_OK)
          {
            if(finish)
            {
              if(ret != Z_STREAM_END)
              {
                std::ostringstream ostr;
                ostr << "El::Compress::ZLib::OutStreamBuf::flush_zlib: "
                  "deflate failed while finishing with code " << ret;
              
                last_error_desc_ = ostr.str();
                last_error_ = ret;
            
                return traits_type::eof();
              }
            }
            else
            {
              if(ret == Z_BUF_ERROR)
              {
                break;
              }
            
              std::ostringstream ostr;
              ostr << "El::Compress::ZLib::OutStreamBuf::flush_zlib: "
                "deflate failed while flushing with code " << ret;
              
              last_error_desc_ = ostr.str();
              last_error_ = ret;
            
              return traits_type::eof();
            }
          }

          ACE_Time_Value tm;
          timer.elapsed_time(tm);

          compression_time_ += tm;          
          
          if(out_buffer_size_ == z_stream_.avail_out)
          {
            break;
          }

          write_pending_bytes_ = out_buffer_size_ - z_stream_.avail_out;   

          if(!push_write_pending())
          {
            std::ostringstream ostr;
            ostr << "El::Compress::ZLib::OutStreamBuf::flush_zlib: "
              "can't push out deflated data; bytes in " << in_bytes_
                 << ", out " << out_bytes_;
            
            last_error_desc_ = ostr.str();
            
            return -1;
          }
        }

        return 0;
      }
      
      //
      // InStreamBuf class
      //
      InStreamBuf::InStreamBuf(InStreamCallback* callback,
                               size_t in_buffer_size,
                               size_t out_buffer_size,
                               size_t putback_buffer_size,
                               bool raw_deflate,
                               bool calc_crc)
        throw(InvalidArg, Exception, El::Exception)
          : callback_(callback),
            in_buffer_size_(in_buffer_size),
            out_buffer_size_(out_buffer_size),
            putback_buffer_size_(putback_buffer_size),
            out_buffer_total_size_(out_buffer_size + putback_buffer_size),
            in_buffer_(new char[in_buffer_size]),
            out_buffer_(new char[out_buffer_total_size_]),
            in_bytes_(0),
            out_bytes_(0),
            out_crc_(0),
            calc_crc_(calc_crc),
            last_error_(0),
            inflate_initialized_(false),
            end_of_stream_(false)
      {
        if(in_buffer_size < 15 || out_buffer_size < 15)
        {
          //
          // ZLIb API can get into a dead loop if less then 11
          //
          throw InvalidArg("El::Compress::ZLib::InStreamBuf::InStreamBuf: "
                           "buffer sizes should not less than 15");
        }

        if(!callback)
        {
          throw InvalidArg("El::Compress::ZLib::InStreamBuf::InStreamBuf: "
                           "callback is 0");
        }
        
        memset(&z_stream_, 0, sizeof(z_stream_));

        z_stream_.zalloc = Z_NULL;
        z_stream_.zfree = Z_NULL;
        z_stream_.opaque = Z_NULL;
        
        int ret = raw_deflate ?
          inflateInit2(&z_stream_, -15) : inflateInit(&z_stream_);

        if(ret != Z_OK)
        {
          std::ostringstream ostr;
          ostr << "El::Compress::ZLib::InStreamBuf::InStreamBuf: "
            "inflateInit failed with code " << ret;

          throw Exception(ostr.str());
        }

        inflate_initialized_ = true;

        if(calc_crc_)
        {
          out_crc_ = crc32(0L, Z_NULL, 0);
        }
      }
      
      std::streamsize
      InStreamBuf::showmanyc()
      {
        if(!last_error_desc_.empty())
        {
          return 0;
        }

        return egptr() - gptr();
      }
      
      InStreamBuf::int_type
      InStreamBuf::underflow()
      {
        if(!last_error_desc_.empty())
        {
          return traits_type::eof();
        }
        
        if (gptr() < egptr())
        {
          unsigned char uc = *gptr();
          return uc;
        }

        if(egptr())
        {
          char* buffer_end = out_buffer_ + out_buffer_total_size_;
          
          if(egptr() < buffer_end)
          {
            return read_to_buf(eback(), gptr(), buffer_end - egptr());
          }
        }

        memmove(out_buffer_,
                out_buffer_ + out_buffer_size_,
                putback_buffer_size_);
        
        return read_to_buf(out_buffer_,
                           out_buffer_ + putback_buffer_size_,
                           out_buffer_size_);
      }
      
      InStreamBuf::int_type
      InStreamBuf::read_to_buf(char* eback_ptr,
                               char* read_ptr,
                               size_t bytes_to_read)
        throw(El::Exception)
      {
        z_stream_.next_out = (unsigned char*)read_ptr;
        z_stream_.avail_out = bytes_to_read;

        size_t bytes_read = 0;

        while(bytes_read == 0)
        {
          size_t avail_in = z_stream_.avail_in;
          int ret = 0;
          
          if(end_of_stream_)
          {
            ret = Z_STREAM_END;
          }
          else
          {
            if(avail_in == 0)
            {
              avail_in = callback_->read(in_buffer_, in_buffer_size_);

              z_stream_.next_in = (unsigned char*)in_buffer_;
              z_stream_.avail_in = avail_in;
              in_bytes_ += avail_in;
            }

            ACE_High_Res_Timer timer;
            timer.start();
          
            ret = inflate(&z_stream_, Z_SYNC_FLUSH);

            timer.stop();

            ACE_Time_Value tm;
            timer.elapsed_time(tm);
            decompression_time_ += tm;
          }
          
          if(ret != Z_OK && ret != Z_STREAM_END)
          {
            std::ostringstream ostr;
            ostr << "El::Compress::ZLib::InStreamBuf::read_to_buf: "
              "inflate failed with code " << ret;
            
            last_error_desc_ = ostr.str();
            last_error_ = ret;
            
            return traits_type::eof();
          }
          
          bytes_read = bytes_to_read - z_stream_.avail_out;
          end_of_stream_ = ret == Z_STREAM_END;
          
          if(avail_in == 0 && bytes_read == 0)
          {
            if(!end_of_stream_)
            {
              std::ostringstream ostr;
              ostr << "El::Compress::ZLib::InStreamBuf::read_to_buf: "
                "failed to read compressed data; bytes in " << in_bytes_
                   << ", out " << out_bytes_;
              
              last_error_desc_ = ostr.str();
            }
            
            return traits_type::eof();
          }

          if(end_of_stream_ && bytes_read == 0 && z_stream_.avail_in != 0)
          {
            callback_->putback((char*)z_stream_.next_in, z_stream_.avail_in);
            return traits_type::eof();            
          }
          
          out_bytes_ += bytes_read;
        }        

        if(calc_crc_)
        {
          out_crc_ = crc32(out_crc_, (const Bytef*)read_ptr, bytes_read);
        }
        
        setg(eback_ptr, read_ptr, read_ptr + bytes_read);

        unsigned char uc = *read_ptr;
        return uc;
      }

      //
      // OutStream class
      //
      OutStream::OutStream(OutStreamCallback* callback,
                           int level,
                           size_t in_buffer_size,
                           size_t out_buffer_size,
                           bool raw_deflate,
                           bool calc_crc)
        throw(InvalidArg, Exception, El::Exception)
          : std::basic_ostream<char, std::char_traits<char> >(0)
      {
        streambuf_.reset(new OutStreamBuf(callback,
                                          level,
                                          in_buffer_size,
                                          out_buffer_size,
                                          raw_deflate,
                                          calc_crc));
        init(streambuf_.get());
      }

      //
      // InStream class
      //
      InStream::InStream(InStreamCallback* callback,
                         size_t in_buffer_size,
                         size_t out_buffer_size,
                         size_t putback_buffer_size,
                         bool raw_deflate,
                         bool calc_crc)
        throw(InvalidArg, Exception, El::Exception)
          : std::basic_istream<char, std::char_traits<char> >(0)
      {
        streambuf_.reset(new InStreamBuf(callback,
                                         in_buffer_size,
                                         out_buffer_size,
                                         putback_buffer_size,
                                         raw_deflate,
                                         calc_crc));
        init(streambuf_.get());
      }
    }
  }
}
