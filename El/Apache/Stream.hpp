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
 * @file Elements/El/Apache/Stream.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_APACHE_STREAM_HPP_
#define _ELEMENTS_EL_APACHE_STREAM_HPP_

#include <streambuf>
#include <iostream>
#include <memory>

#include <httpd/httpd.h>

#include <El/Exception.hpp>
#include <El/Apache/Exception.hpp>

namespace El
{
  namespace Apache
  {
    class InStreamBuf
      : public std::basic_streambuf<char, std::char_traits<char> >
    {        
    public:
      InStreamBuf(request_rec* r,
                  unsigned long recv_buffer_size = 4096,
                  unsigned long putback_buffer_size = 1024)
        throw(InvalidArg, Exception, El::Exception);
          
      virtual ~InStreamBuf() throw();

      virtual std::streamsize showmanyc();
      virtual int_type underflow();

      int last_error() const throw();
      const std::string& last_error_desc() const throw(El::Exception);

      void discard() throw();
      
    protected:
      int_type read_to_buf(char* eback_ptr,
                           char* read_ptr,
                           unsigned long bytes_to_read)
        throw(El::Exception);
        
    protected:
      request_rec* request_;
      
      unsigned long recv_buffer_size_;
      unsigned long putback_buffer_size_;
      unsigned long recv_buffer_total_size_;
        
      char* recv_buffer_;
      unsigned long long recv_bytes_;

      int last_error_;
      std::string last_error_desc_;

      bool discarded_;
    };

    class InStream :
      public std::basic_istream<char, std::char_traits<char> >
    {
    public:
      InStream(request_rec* r,
               unsigned long recv_buffer_size = 4096,
               unsigned long putback_buffer_size = 1024)
        throw(InvalidArg, Exception, El::Exception);

      virtual ~InStream() throw();

      int last_error() const throw();
      const std::string& last_error_desc() const throw(El::Exception);

      void discard() throw();

    protected:

      typedef std::auto_ptr<InStreamBuf> InStreamBufPtr;

      InStreamBufPtr streambuf_;
    };

    class OutStreamBuf
      : public std::basic_streambuf<char, std::char_traits<char> >
    {
    public:

      struct Callback
      {
        virtual void start_writing() throw() = 0;
        
        virtual void write(const char* buffer, size_t size)
          throw(El::Exception) = 0;

        virtual void pre_ap_rwrite() throw() = 0;
        virtual void post_ap_rwrite() throw() = 0;
          
        virtual ~Callback() throw() {}
      };
      
    public:
      OutStreamBuf(request_rec* r,
                   Callback* callback = 0,
                   unsigned long send_buffer_size = 4096)
        throw(InvalidArg, Exception, El::Exception);
          
      virtual ~OutStreamBuf() throw();

      virtual int_type overflow(int_type c);
      virtual int sync();

      int last_error() const throw();
      const std::string& last_error_desc() const throw(El::Exception);

    protected:
      request_rec* request_;
      Callback* callback_;

      unsigned long send_buffer_size_;        
      char* send_buffer_;

      unsigned long long send_bytes_;

      int last_error_;
      std::string last_error_desc_;
    };

    class OutStream :
      public std::basic_ostream<char, std::char_traits<char> >
    {
    public:
      OutStream(request_rec* r,
                OutStreamBuf::Callback* callback = 0,
                unsigned long send_buffer_size = 4096)
        throw(InvalidArg, Exception, El::Exception);

      virtual ~OutStream() throw();

      int last_error() const throw();
      const std::string& last_error_desc() const throw(El::Exception);

    protected:
      typedef std::auto_ptr<OutStreamBuf> OutStreamBufPtr;

      OutStreamBufPtr streambuf_;
    };
    
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Apache
  {
    //
    // InStreamBuf class
    //
    
    inline
    int
    InStreamBuf::last_error() const throw()
    {
      return last_error_;
    }
      
    inline
    const std::string&
    InStreamBuf::last_error_desc() const throw(El::Exception)
    {
      return last_error_desc_;
    }

    //
    // InStream class
    //
    
    inline
    InStream::~InStream() throw()
    {
    }
      
    inline
    int
    InStream::last_error() const throw()
    {
      return streambuf_->last_error();
    }
      
    inline
    const std::string&
    InStream::last_error_desc() const throw(El::Exception)
    {
      return streambuf_->last_error_desc();
    }
      
    inline
    void
    InStream::discard() throw()
    {
      streambuf_->discard();
    }
    
    //
    // OutStreamBuf class
    //
    
    inline
    int
    OutStreamBuf::last_error() const throw()
    {
      return last_error_;
    }
      
    inline
    const std::string&
    OutStreamBuf::last_error_desc() const throw(El::Exception)
    {
      return last_error_desc_;
    }
      
    //
    // OutStream class
    //
    
    inline
    OutStream::~OutStream() throw()
    {
    }
    
    inline
    int
    OutStream::last_error() const throw()
    {
      return streambuf_->last_error();
    }
      
    inline
    const std::string&
    OutStream::last_error_desc() const throw(El::Exception)
    {
      return streambuf_->last_error_desc();
    }
      
  }
}

#endif // _ELEMENTS_EL_APACHE_STREAM_HPP_
