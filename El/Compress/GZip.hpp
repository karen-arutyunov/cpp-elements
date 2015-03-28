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
 * @file Elements/El/Compress/GZip.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_COMPRESS_GZIP_HPP_
#define _ELEMENTS_EL_COMPRESS_GZIP_HPP_

#include <zlib.h>
#include <stdint.h>

#include <string>
#include <streambuf>
#include <iostream>
#include <memory>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/Compress/ZLib.hpp>

namespace El
{
  namespace Compress
  {
    namespace ZLib
    {
      class GZip
      {
      public:
        GZip(OutStreamCallback* callback,
             int level = Z_DEFAULT_COMPRESSION,
             size_t in_buffer_size = 4096,
             size_t out_buffer_size = 4096)
          throw(InvalidArg, Exception, El::Exception);

        OutStream& stream() throw() { return stream_; }

        void finalize() throw(Exception, El::Exception);

      private:
        void write(char* buff, size_t len) throw(Exception, El::Exception);
        void write_uint32(uint32_t val) throw(Exception, El::Exception);

      private:
        OutStreamCallback* callback_;
        OutStream stream_;
        bool finalized_;
      };

      class GUnzip
      {
      public:
        GUnzip(InStreamCallback* callback,
               size_t in_buffer_size = 4096,
               size_t out_buffer_size = 4096,
               size_t putback_buffer_size = 4096)
          throw(InvalidArg, Exception, El::Exception);

        InStream& stream() throw() { return stream_; }

      private:
        void read(char* buff, size_t len) throw(Exception, El::Exception);
        void skip_till(char chr) throw(Exception, El::Exception);
        void skip(size_t len) throw(Exception, El::Exception);
        
        uint32_t read_uint32() throw(Exception, El::Exception);
        uint16_t read_uint16() throw(Exception, El::Exception);

      private:
        InStreamCallback* callback_;
        InStream stream_;
      };
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Compress
  {
    namespace ZLib
    {
    }
  }
}

#endif // _ELEMENTS_EL_COMPRESS_GZIP_HPP_
