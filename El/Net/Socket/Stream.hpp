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
 * @file Elements/El/Net/Socket/Stream.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_SOCKET_STREAM_HPP_
#define _ELEMENTS_EL_NET_SOCKET_STREAM_HPP_

#include <streambuf>
#include <iostream>
#include <memory>

#include <ace/OS.h>
#include <ace/SOCK_Stream.h>

#include <El/Exception.hpp>
#include <El/ArrayPtr.hpp>
#include <El/Net/Exception.hpp>

namespace El
{
  namespace Net
  {
    namespace Socket
    {
      EL_EXCEPTION(Exception, ::El::Net::Exception);
      EL_EXCEPTION(InvalidArg, ::El::Net::Socket::Exception);
      EL_EXCEPTION(InvalidOperation, ::El::Net::Socket::Exception);

      class Stream;
      
      class Interceptor
      {
      public:
        virtual void socket_stream_connected(El::Net::Socket::Stream& stream)
          throw(El::Exception) = 0;
        
        virtual void socket_stream_read(const unsigned char* buff, size_t size)
          throw(El::Exception) = 0;

        virtual void socket_stream_write(const unsigned char* buff,
                                         size_t size)
          throw(El::Exception) = 0;

        virtual ~Interceptor() throw() {}
      };
      
      class StreamBuf
        : public std::basic_streambuf<char, std::char_traits<char> >
      {
      public:
        StreamBuf(ACE_SOCK_Stream& socket,
                  const ACE_Time_Value* send_timeout = 0,
                  const ACE_Time_Value* recv_timeout = 0,
                  size_t send_buffer_size = 1024,
                  size_t recv_buffer_size = 1024,
                  size_t putback_buffer_size = 1024,
                  Interceptor* interceptor = 0)
          throw(InvalidArg, El::Exception);
        
        virtual ~StreamBuf() throw();

        unsigned long long sent_bytes(bool reset_counter = false) throw();
        unsigned long long received_bytes(bool reset_counter = false) throw();

        void set_sent_bytes(unsigned long long value) throw();
        void set_received_bytes(unsigned long long value) throw();
        
        ACE_SOCK_Stream& socket() const throw();

        int last_error() const throw();
        const std::string& last_error_desc() const throw(El::Exception);

        void read_limit(unsigned long long val) throw();
        
        unsigned long long read_limit() throw();
        bool has_read_limit() throw();

        virtual std::streamsize showmanyc();
        virtual int_type underflow();

        virtual int_type overflow(int_type c);
        virtual int sync();

        char_type* eback() const;
        char_type* gptr() const;
        char_type* egptr() const;
        void setg(char_type* gbeg, char_type* gnext, char_type* gend);

        void debug_istream(std::ostream* istr) throw();
        void debug_ostream(std::ostream* ostr) throw();

        std::ostream* debug_istream() throw();
        std::ostream* debug_ostream() throw();
        
        const ACE_Time_Value* send_timeout() const throw();
        const ACE_Time_Value* recv_timeout() const throw();

        size_t send_buffer_size() const throw();
        size_t recv_buffer_size() const throw();
        size_t putback_buffer_size() const throw();

      protected:
        StreamBuf::int_type read_to_buf(char* eback_ptr,
                                        char* read_ptr,
                                        size_t bytes_to_read)
          throw(El::Exception);
        
      protected:
        ACE_SOCK_Stream& socket_;
        
        std::auto_ptr<ACE_Time_Value> send_timeout_;
        std::auto_ptr<ACE_Time_Value> recv_timeout_;
        size_t send_buffer_size_;
        size_t recv_buffer_size_;
        size_t putback_buffer_size_;
        size_t recv_buffer_total_size_;
        
        El::ArrayPtr<char> send_buffer_;
        El::ArrayPtr<char> recv_buffer_;

        unsigned long long sent_bytes_;
        unsigned long long received_bytes_;

        unsigned long long read_limit_;
        bool has_read_limit_;

        int last_error_;
        std::string last_error_desc_;

        std::ostream* debug_istream_;
        std::ostream* debug_ostream_;

        Interceptor* interceptor_;
      };

      class Stream : public std::basic_iostream<char, std::char_traits<char> >
      {
      public:
        EL_EXCEPTION(Timeout, ::El::Net::Socket::Exception);
        
      public:
        Stream(ACE_SOCK_Stream& sock_stream,
               const ACE_Time_Value* send_timeout = 0,
               const ACE_Time_Value* recv_timeout = 0,
               size_t send_buffer_size = 1024,
               size_t recv_buffer_size = 1024,
               size_t putback_buffer_size = 1024,
               Interceptor* interceptor = 0)
          throw(InvalidArg, ::El::Net::Socket::Exception, El::Exception);

        Stream(const char* host,
               unsigned short port,
               const ACE_Time_Value* connect_timeout = 0,
               const ACE_Time_Value* send_timeout = 0,
               const ACE_Time_Value* recv_timeout = 0,
               size_t send_buffer_size = 1024,
               size_t recv_buffer_size = 1024,
               size_t putback_buffer_size = 1024,
               Interceptor* interceptor = 0)
          throw(Timeout,
                InvalidArg,
                ::El::Net::Socket::Exception,
                El::Exception);

        Stream(const ACE_Time_Value* send_timeout = 0,
               const ACE_Time_Value* recv_timeout = 0,
               size_t send_buffer_size = 1024,
               size_t recv_buffer_size = 1024,
               size_t putback_buffer_size = 1024,
               Interceptor* interceptor = 0)
          throw(InvalidArg, ::El::Net::Socket::Exception, El::Exception);

        virtual ~Stream() throw();

        void connect(const char* host,
                     unsigned short port,
                     const ACE_Time_Value* connect_timeout)
          throw(Timeout,
                InvalidArg,
                InvalidOperation,
                ::El::Net::Socket::Exception,
                El::Exception);

        unsigned long long sent_bytes(bool reset_counter = false) throw();
        unsigned long long received_bytes(bool reset_counter = false) throw();

        void set_sent_bytes(unsigned long long value) throw();
        void set_received_bytes(unsigned long long value) throw();

        ACE_SOCK_Stream& socket() const throw();

        StreamBuf& socket_streambuf() const throw();
    
        int last_error() const throw();
        const std::string& last_error_desc() const throw(El::Exception);

      protected:
        typedef std::auto_ptr<StreamBuf> StreamBufPtr;
        typedef std::auto_ptr<ACE_SOCK_Stream> ACE_SOCK_StreamPtr;

        bool connected_;
        ACE_SOCK_StreamPtr socket_;
        StreamBufPtr       streambuf_;
        Interceptor*       interceptor_;

      private:
        Stream(const Stream& );
        void operator=(const Stream& );
      };
      
    } 
  }  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Net
  {
    namespace Socket
    {
      //
      // StreamBuf class
      //
      inline
      StreamBuf::~StreamBuf() throw()
      {    
      }
  
      inline
      unsigned long long
      StreamBuf::sent_bytes(bool reset_counter) throw()
      {
        unsigned long long ret = sent_bytes_;

        if(reset_counter)
        {
          sent_bytes_ = 0;
        }
        
        return ret;
      }
  
      inline
      unsigned long long 
      StreamBuf::received_bytes(bool reset_counter) throw()
      {
        unsigned long long ret = received_bytes_;

        if(reset_counter)
        {
          received_bytes_ = 0;
        }
        
        return ret;
      }

      inline
      void
      StreamBuf::set_sent_bytes(unsigned long long value) throw()
      {
        sent_bytes_ = value;
      }
      
      inline
      void
      StreamBuf::set_received_bytes(unsigned long long value) throw()
      {
        received_bytes_ = value;
      }

      inline
      ACE_SOCK_Stream&
      StreamBuf::socket() const throw()
      {
        return socket_;
      }

      inline
      int
      StreamBuf::last_error() const throw()
      {
        return last_error_;
      }
      
      inline
      const std::string&
      StreamBuf::last_error_desc() const throw(El::Exception)
      {
        return last_error_desc_;
      }
      
      inline
      StreamBuf::char_type*
      StreamBuf::eback() const
      {
        return std::basic_streambuf<char, std::char_traits<char> >::eback();
      }

      inline
      StreamBuf::char_type*
      StreamBuf::gptr() const
      {
        return std::basic_streambuf<char, std::char_traits<char> >::gptr();
      }

      inline
      StreamBuf::char_type*
      StreamBuf::egptr() const
      {
        return std::basic_streambuf<char, std::char_traits<char> >::egptr();
      }
      
      inline
      void
      StreamBuf::setg(char_type* gbeg, char_type* gnext, char_type* gend)
      {
        std::basic_streambuf<char, std::char_traits<char> >::setg(gbeg,
                                                                  gnext,
                                                                  gend);
      }
      
      inline
      unsigned long long
      StreamBuf::read_limit() throw()
      {
        return read_limit_;
      }
      
      inline
      bool
      StreamBuf::has_read_limit() throw()
      {
        return has_read_limit_;
      }
      
      inline
      void
      StreamBuf::debug_istream(std::ostream* ostr) throw()
      {
        debug_istream_ = ostr;
      }
      
      inline
      void
      StreamBuf::debug_ostream(std::ostream* ostr) throw()
      {
        debug_ostream_ = ostr;
      }

      inline
      std::ostream*
      StreamBuf::debug_istream() throw()
      {
        return debug_istream_;
      }
      
      inline
      std::ostream*
      StreamBuf::debug_ostream() throw()
      {
        return debug_ostream_;
      }

      inline
      const ACE_Time_Value*
      StreamBuf::send_timeout() const throw()
      {
        return send_timeout_.get();
      }
        
      inline
      const ACE_Time_Value*
      StreamBuf::recv_timeout() const throw()
      {
        return recv_timeout_.get();
      }  
      
      inline
      size_t
      StreamBuf::send_buffer_size() const throw()
      {
        return send_buffer_size_;
      }

      inline
      size_t
      StreamBuf::recv_buffer_size() const throw()
      {
        return recv_buffer_size_;
      }

      inline
      size_t
      StreamBuf::putback_buffer_size() const throw()
      {
        return putback_buffer_size_;
      }

      //
      // Stream class
      //
      inline
      unsigned long long
      Stream::sent_bytes(bool reset_counter) throw()
      {
        return streambuf_->sent_bytes(reset_counter);
      }
      
      inline
      unsigned long long
      Stream::received_bytes(bool reset_counter) throw()
      {
        return streambuf_->received_bytes(reset_counter);
      }

      inline
      void
      Stream::set_sent_bytes(unsigned long long value) throw()
      {
        streambuf_->set_sent_bytes(value);
      }
      
      inline
      void
      Stream::set_received_bytes(unsigned long long value) throw()
      {
        streambuf_->set_received_bytes(value);
      }      

      inline
      ACE_SOCK_Stream&
      Stream::socket() const throw()
      {
        return streambuf_->socket();
      }
      
      inline
      int
      Stream::last_error() const throw()
      {
        return streambuf_->last_error();
      }
      
      inline
      const std::string&
      Stream::last_error_desc() const throw(El::Exception)
      {
        return streambuf_->last_error_desc();
      }

      inline
      StreamBuf&
      Stream::socket_streambuf() const throw()
      {
        return *streambuf_;
      }
      
    }
  }
}

#endif // _ELEMENTS_EL_NET_SOCKET_STREAM_HPP_
