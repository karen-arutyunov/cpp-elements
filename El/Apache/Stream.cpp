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
 * @file Elements/El/Apache/Stream.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>
#include <streambuf>
#include <iostream>

#include <httpd/httpd.h>
#include <httpd/http_protocol.h>

#include <El/Exception.hpp>

#include "Stream.hpp"

namespace El
{
  namespace Apache
  {
    //
    // InStreamBuf class
    //

    InStreamBuf::InStreamBuf(request_rec* r,
                             unsigned long recv_buffer_size,
                             unsigned long putback_buffer_size)
      throw(InvalidArg, Exception, El::Exception)
        : request_(r),
          recv_buffer_size_(recv_buffer_size),
          putback_buffer_size_(putback_buffer_size),
          recv_buffer_total_size_(recv_buffer_size + putback_buffer_size),
          recv_buffer_(new char[recv_buffer_total_size_]),
          recv_bytes_(0),
          last_error_(0),
          discarded_(false)
    {
      if(r == 0)
      {
        throw InvalidArg(
          "El::Apache::InStreamBuf::InStreamBuf: request_rec "
          "ptr argument is 0");
      }

      if(recv_buffer_size == 0)
      {
        throw InvalidArg(
          "El::Apache::InStreamBuf::InStreamBuf: recv_buffer_size "
          "argument is 0");
      }
    }

    InStreamBuf::~InStreamBuf() throw()
    {
      delete [] recv_buffer_;
    }

    std::streamsize
    InStreamBuf::showmanyc()
    {
      if(!last_error_desc_.empty() || discarded_)
      {
        return 0;
      }

      if(!recv_buffer_size_)
      {
        return 0;
      }

      return egptr() - gptr();
    }
        
    InStreamBuf::int_type
    InStreamBuf::underflow()
    {
      if(!last_error_desc_.empty() || discarded_)
      {
        return traits_type::eof();
      }
        
      if (gptr() < egptr())
      {
        return *gptr();
      }

      if(egptr())
      {
        char* buffer_end = recv_buffer_ + recv_buffer_total_size_;
          
        if(egptr() < buffer_end)
        {
          return read_to_buf(eback(), gptr(), buffer_end - egptr());
        }
      }

      memmove(recv_buffer_,
              recv_buffer_ + recv_buffer_size_,
              putback_buffer_size_);
        
      return read_to_buf(recv_buffer_,
                         recv_buffer_ + putback_buffer_size_,
                         recv_buffer_size_);
    }
    
    InStreamBuf::int_type
    InStreamBuf::read_to_buf(char* eback_ptr,
                             char* read_ptr,
                             unsigned long bytes_to_read)
      throw(El::Exception)
    {
      int bytes_read = ap_get_client_block(request_, read_ptr, bytes_to_read);

      if (bytes_read == 0)
      {
        return traits_type::eof();
      }

      if (bytes_read < 0)
      {
        last_error_ = -1;
        
        std::ostringstream ostr;            
        ostr << "El::Apache::InStreamBuf::read_to_buf: "
          "ap_get_client_block failed after " << recv_bytes_ << " bytes read";
          
        last_error_desc_ = ostr.str();

        return traits_type::eof();
      }

      recv_bytes_ += bytes_read;
      setg(eback_ptr, read_ptr, read_ptr + bytes_read);

      unsigned char uc = *read_ptr;
      return uc;
    }
      
    void
    InStreamBuf::discard() throw()
    {
      discarded_ = true;
    }

    //
    // InStream class
    //
    
    InStream::InStream(request_rec* r,
                       unsigned long recv_buffer_size,
                       unsigned long putback_buffer_size)
      throw(InvalidArg, Exception, El::Exception)
        : std::basic_istream<char, std::char_traits<char> >(0)
    {
      streambuf_.reset(
        new InStreamBuf(r, recv_buffer_size, putback_buffer_size));
      
      init(streambuf_.get());
    }

    //
    // InStreamBuf class
    //

    OutStreamBuf::OutStreamBuf(request_rec* r,
                               Callback* callback,
                               unsigned long send_buffer_size)
        throw(InvalidArg, Exception, El::Exception)
          : request_(r),
            callback_(callback),
            send_buffer_size_(send_buffer_size),
            send_buffer_(new char[send_buffer_size]),
            send_bytes_(0),
            last_error_(0)
    {
      if(r == 0)
      {
        throw InvalidArg(
          "El::Apache::InStreamBuf::InStreamBuf: request_rec "
          "ptr argument is 0");
      }

      if(send_buffer_size == 0)
      {
        throw InvalidArg(
          "El::Apache::InStreamBuf::InStreamBuf: send_buffer_size "
          "argument is 0");
      }
    }

    OutStreamBuf::~OutStreamBuf() throw()
    {
      delete [] send_buffer_;
    }

    OutStreamBuf::int_type
    OutStreamBuf::overflow(int_type c)
    {
      if(!last_error_desc_.empty())
      {
        return traits_type::eof();
      }
/*
      std::cerr << "PPTR: " << std::hex << (unsigned long)pptr()
                << std::dec << ", CHR: " << (char)c << std::endl;
*/
      if(pptr() == 0)
      {
        if(callback_)
        {
          callback_->start_writing();
        }
        
        send_buffer_[0] = c;
        setp(send_buffer_ + 1, send_buffer_ + send_buffer_size_);

        return c;
      }
/*
      std::cerr << "Writing:\n";
      std::cerr.write(send_buffer_, send_buffer_size_);
      std::cerr << "\nEnd of writing:\n";
*/

      if(callback_)
      {
        callback_->pre_ap_rwrite();
      }
      
      int written_bytes =
        ap_rwrite(send_buffer_, send_buffer_size_, request_);

      if(callback_)
      {
        callback_->post_ap_rwrite();
      }

      if(written_bytes <= 0)
      {
        last_error_ = -1;
          
        std::ostringstream ostr;            
        ostr << "El::Apache::OutStreamBuf::overflow: "
          "ap_rwrite failed after " << send_bytes_ << " bytes written";
          
        last_error_desc_ = ostr.str();
          
        return traits_type::eof();
      }
      else if(callback_)
      {
        callback_->write(send_buffer_, written_bytes);
      }

      send_bytes_ += written_bytes;
  
      unsigned long left_bytes = send_buffer_size_ - written_bytes;

      memmove(send_buffer_,
              send_buffer_ + written_bytes,
              left_bytes);
        
      send_buffer_[left_bytes] = c;
      setp(send_buffer_ + left_bytes + 1, send_buffer_ + send_buffer_size_);

      return c;
    }
      
    int
    OutStreamBuf::sync()
    {
      if(!last_error_desc_.empty() || pptr() == 0)
      {
        return -1;
      }
/*
      std::cerr << "PPTR: " << std::hex << (unsigned long)pptr()
                << std::dec << std::endl;
*/
      unsigned long bytes_to_write = pptr() - send_buffer_;

      for(char* ptr = send_buffer_; bytes_to_write > 0; )
      {
        int written_bytes =
          ap_rwrite(ptr, bytes_to_write, request_);

        if(written_bytes <= 0)
        {
          last_error_ = -1;
          
          std::ostringstream ostr;            
          ostr << "El::Apache::OutStreamBuf::sync: "
            "ap_rwrite failed after " << send_bytes_ << " bytes written";
        
          last_error_desc_ = ostr.str();

          return -1;
        }
        else if(callback_)
        {
          callback_->write(ptr, written_bytes);
        }

        ptr += written_bytes;
        send_bytes_ += written_bytes;
        bytes_to_write -= written_bytes;
      }

      setp(send_buffer_, send_buffer_ + send_buffer_size_);
      return 0;
    }

    //
    // OutStream class
    //
    OutStream::OutStream(request_rec* r,
                         OutStreamBuf::Callback* callback,
                         unsigned long send_buffer_size)
      throw(InvalidArg, Exception, El::Exception)
        : std::basic_ostream<char, std::char_traits<char> >(0)
    {
      streambuf_.reset(new OutStreamBuf(r, callback, send_buffer_size));
      init(streambuf_.get());
    }    
  }
}
