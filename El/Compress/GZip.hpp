/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
