/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Compress/GZip.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <zlib.h>

#include <iostream>
#include <sstream>

#include "GZip.hpp"

namespace El
{
  namespace Compress
  {
    namespace ZLib
    {
      //
      // GZip class
      //
      GZip::GZip(OutStreamCallback* callback,
                 int level,
                 size_t in_buffer_size,
                 size_t out_buffer_size)
        throw(InvalidArg, Exception, El::Exception)
          : callback_(callback),
            stream_(callback,
                    level,
                    in_buffer_size,
                    out_buffer_size,
                    true,
                    true),
            finalized_(false)
      {
//        char S = 0;
//        callback->write(&S, 1);
          
        char header[10];
        memset(header, 0, sizeof(header));

        header[0] = 0x1F;
        header[1] = 0x8B;
        header[2] = Z_DEFLATED;

/*        
        header[3] = 0x8; // Name present

        header[4] = 0x1C;
        header[5] = 0x49;
        header[6] = 0x3D;
        header[7] = 0x4D;
*/      
        
        header[8] = 0;
        header[9] = 3;

        write(header, sizeof(header));
//        write("AAA", 4);
      }

      void
      GZip::write(char* buff, size_t len) throw(Exception, El::Exception)
      {
        while(len)
        {
          size_t written_bytes = callback_->write(buff, len);
          
          if(!written_bytes)
          {
            std::ostringstream ostr;
            ostr << "El::Compress::ZLib::GZip::write: failed to write "
                 << len << " bytes";

            throw Exception(ostr.str());
          }

          len -= written_bytes;
          buff += written_bytes;
        }
      }
      
      void
      GZip::write_uint32(uint32_t val) throw(Exception, El::Exception)
      {
        unsigned char buff[4];
        
        buff[0] = val & 0xFF;
        buff[1] = (val >> 8) & 0xFF;
        buff[2] = (val >> 16) & 0xFF;
        buff[3] = (val >> 24) & 0xFF;
        
        write((char*)buff, sizeof(buff));
      }
        
      void
      GZip::finalize() throw(Exception, El::Exception)
      {
        if(!finalized_)
        {
          finalized_ = true;
          stream_.finalize();
        
          if(!stream_.last_error_desc().empty())
          {
            std::ostringstream ostr;
            ostr << "El::Compress::ZLib::GZip::finalize: operation failed. "
              "Error:\n" << stream_.last_error_desc();
            
            throw Exception(ostr.str());
          }

          write_uint32(stream_.in_crc());
          write_uint32(stream_.in_bytes());
        }
      }

      //
      // GUnzip class
      //
      GUnzip::GUnzip(InStreamCallback* callback,
                     size_t in_buffer_size,
                     size_t out_buffer_size,
                     size_t putback_buffer_size)
        throw(InvalidArg, Exception, El::Exception)
          : callback_(callback),
            stream_(callback,
                    in_buffer_size,
                    out_buffer_size,
                    putback_buffer_size,
                    true,
                    true)
      {
/*        
        unsigned char id1 = response_body_stream_->get();
        unsigned char id2 = response_body_stream_->get();
        unsigned char compression_method = response_body_stream_->get();
        unsigned char flags = response_body_stream_->get();
        
        char mtime[4];
        response_body_stream_->read(mtime, sizeof(mtime));

        response_body_stream_->get(); // reading "extra" flags
        response_body_stream_->get(); // reading OS flag
*/
        
        unsigned char header[10];
        memset(header, 0, sizeof(header));
        read((char*)header, sizeof(header));

        if(header[0] != 0x1F && header[1] != 0x8B)
        {
          std::ostringstream ostr;            
          ostr << "El::Compress::ZLib::GUnzip::GUnzip: "
            "reading GZIP header failed. Reason: unexpected ids 0x"
               << std::hex << (unsigned long)header[0] << ", 0x" << std::hex
               << (unsigned long)header[1] << " instead of 0x1F, 0x8B";
            
          throw Exception(ostr.str());                
        }

        if(header[2] != Z_DEFLATED)
        {
          std::ostringstream ostr;            
          ostr << "El::Compress::ZLib::GUnzip::GUnzip: "
            "reading GZIP header failed. Reason: unexpected "
            "compression method " << (unsigned long)header[2]
               << " instead of " << Z_DEFLATED;
                
          throw Exception(ostr.str());       
        }

        unsigned char flags = header[3];
          
        if(flags & 0x4) // extra field
        {
          skip(read_uint16());
        }

        if(flags & 0x8) // Orig. name
        {
          skip_till('\0');
        }

        if(flags & 0x10) // Comment
        {
          skip_till('\0');
        }

        if(flags & 0x2) // CRC16
        {
          read_uint16();
        }              
      }

      void
      GUnzip::read(char* buff, size_t len) throw(Exception, El::Exception)
      {
        while(len)
        {
          size_t read_bytes = callback_->read(buff, len);
          
          if(!read_bytes)
          {
            std::ostringstream ostr;
            ostr << "El::Compress::ZLib::GUnzip::read: failed to read "
                 << len << " bytes";

            throw Exception(ostr.str());
          }

          len -= read_bytes;
          buff += read_bytes;
        }
      }
      
      void
      GUnzip::skip_till(char chr) throw(Exception, El::Exception)
      {
        char buff = '\0';
        
        while(true)
        {
          size_t read_bytes = callback_->read(&buff, 1);
          
          if(!read_bytes)
          {
            throw Exception(
              "El::Compress::ZLib::GUnzip::read_till: failed to read  1 byte");
          }
        }
      }
      
      void
      GUnzip::skip(size_t len) throw(Exception, El::Exception)
      {
        char buff[2];
        
        while(len)
        {
          size_t to_read = std::min(len, sizeof(buff));
          size_t read_bytes = callback_->read(buff, to_read);
          
          if(!read_bytes)
          {
            std::ostringstream ostr;
            ostr << "El::Compress::ZLib::GUnzip::skip: failed to read "
                 << to_read << " bytes";

            throw Exception(ostr.str());
          }

          len -= read_bytes;
        }
      }
      
      uint32_t
      GUnzip::read_uint32() throw(Exception, El::Exception)
      {
        unsigned char buff[4];
        read((char*)buff, sizeof(buff));

        return ((uint32_t)buff[0]) | (((uint32_t)buff[1]) << 8) |
          (((uint32_t)buff[2]) << 16) | (((uint32_t)buff[3]) << 24);
      }

      uint16_t
      GUnzip::read_uint16() throw(Exception, El::Exception)
      {
        unsigned char buff[2];
        read((char*)buff, sizeof(buff));
        
        return ((uint16_t)buff[0]) | (((uint16_t)buff[1]) << 8);
      }
      
    }
  }
}
