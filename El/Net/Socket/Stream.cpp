/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/Socket/Stream.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string.h>

#include <sstream>
#include <iostream>

#include <ace/OS.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>

#include "Stream.hpp"

namespace El
{
  namespace Net
  {
    namespace Socket
    {
      //
      // StreamBuf class
      //
      StreamBuf::StreamBuf(ACE_SOCK_Stream& socket,
                           const ACE_Time_Value* send_timeout,
                           const ACE_Time_Value* recv_timeout,
                           size_t send_buffer_size,
                           size_t recv_buffer_size,
                           size_t putback_buffer_size,
                           Interceptor* interceptor)
        throw(InvalidArg, El::Exception)
          : socket_(socket),
            send_timeout_(send_timeout ?
                          new ACE_Time_Value(*send_timeout) : 0),
            recv_timeout_(recv_timeout ?
                          new ACE_Time_Value(*recv_timeout) : 0),
            send_buffer_size_(send_buffer_size),
            recv_buffer_size_(recv_buffer_size),
            putback_buffer_size_(putback_buffer_size),
            recv_buffer_total_size_(recv_buffer_size + putback_buffer_size),
            send_buffer_(new char[send_buffer_size]),
            recv_buffer_(new char[recv_buffer_total_size_]),
            sent_bytes_(0),
            received_bytes_(0),
            read_limit_(0),
            has_read_limit_(false),
            last_error_(0),
            debug_istream_(0),
            debug_ostream_(0),
            interceptor_(interceptor)
      {
        if(!send_buffer_size_ && !recv_buffer_size_)
        {
          throw InvalidArg("El::Net::Socket::StreamBuf::StreamBuf: both "
                           "send_buffer_size and recv_buffer_size are 0");
        }

        if(socket.get_handle () == ACE_INVALID_HANDLE)
        {
          throw InvalidArg("El::Net::Socket::StreamBuf::StreamBuf: "
                           "sock_stream is not a valid stream");
        }
      }
      
      std::streamsize
      StreamBuf::showmanyc()
      {
        if(!last_error_desc_.empty())
        {
          return 0;
        }

        if(!recv_buffer_size_)
        {
          return 0;
        }

        return egptr() - gptr();
      }
        
      StreamBuf::int_type
      StreamBuf::underflow()
      {
        if(!last_error_desc_.empty())
        {
          return traits_type::eof();
        }
        
        if(!recv_buffer_size_)
        {
          last_error_desc_ = "El::Net::Socket::StreamBuf::underflow: "
            "this is a write-only object";
          
          return traits_type::eof();
        }

        if(has_read_limit_ && read_limit_ == 0)
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
          char* buffer_end = recv_buffer_.get() + recv_buffer_total_size_;
          
          if(egptr() < buffer_end)
          {
            return read_to_buf(eback(), gptr(), buffer_end - egptr());
          }
        }

        memmove(recv_buffer_.get(),
                recv_buffer_.get() + recv_buffer_size_,
                putback_buffer_size_);
        
        return read_to_buf(recv_buffer_.get(),
                           recv_buffer_.get() + putback_buffer_size_,
                           recv_buffer_size_);
      }

      StreamBuf::int_type
      StreamBuf::read_to_buf(char* eback_ptr,
                             char* read_ptr,
                             size_t bytes_to_read)
        throw(El::Exception)
      {
        if(has_read_limit_ && read_limit_ < bytes_to_read)
        {
          bytes_to_read = read_limit_;
        }
        
        int bytes_read = 0;

        while(true)
        {
          bytes_read = socket_.recv(read_ptr,
                                    bytes_to_read,
                                    recv_timeout_.get());
          
          if(bytes_read > 0)
          {
            break;
          }
          
          if(bytes_read < 0)
          {
            int last_error = ACE_OS::last_error();

            if(last_error == EINTR)
            {
              continue;
            }
            
            last_error_ = last_error;
              
            if(last_error_)
            {
              std::ostringstream ostr;            
              ostr << "El::Net::Socket::StreamBuf::read_to_buf: "
                "recv failed attemping to read " << bytes_to_read
                   << " bytes. Reason: " << ACE_OS::strerror(last_error_);
                
              last_error_desc_ = ostr.str();
            }
          }
          else
          {
            std::ostringstream ostr;
            ostr << "El::Net::Socket::StreamBuf::read_to_buf: "
              "recv failed attemping to read " << bytes_to_read
                 << " bytes. Reason: socket closed by remote peer";
              
            last_error_desc_ = ostr.str();
          }
            
          return traits_type::eof();
        }

        if(debug_istream_)
        {
          debug_istream_->write(read_ptr, bytes_read);
        }
        
        if(interceptor_)
        {
          interceptor_->socket_stream_read(
              (const unsigned char*)read_ptr, bytes_read);
        }
        
        received_bytes_ += bytes_read;          

        if(has_read_limit_)
        {
          read_limit_ -= bytes_read;
        }

        setg(eback_ptr, read_ptr, read_ptr + bytes_read);

        unsigned char uc = *read_ptr;
        return uc;
      }
      
      StreamBuf::int_type
      StreamBuf::overflow(int_type c)
      {
        if(!last_error_desc_.empty())
        {
          return traits_type::eof();
        }
        
        if(pptr() == 0)
        {
          if(send_buffer_size_ == 0)
          {
            last_error_desc_ = "El::Net::Socket::StreamBuf::underflow: "
              "this is a read-only object";
            
            return traits_type::eof();
          }
          else
          {
            send_buffer_.get()[0] = c;
            
            setp(send_buffer_.get() + 1,
                 send_buffer_.get() + send_buffer_size_);
            
            return c;
          }
        }
        
        int written_bytes = 0;

        while(true)
        {
          written_bytes = socket_.send(send_buffer_.get(),
                                       send_buffer_size_,
                                       send_timeout_.get());
          
          if(written_bytes > 0)
          {
            break;
          }
          
          int last_error = ACE_OS::last_error();
            
          if(written_bytes < 0 && last_error == EINTR)
          {
            continue;
          }
            
          last_error_ = last_error;
            
          std::ostringstream ostr;            
          ostr << "El::Net::Socket::StreamBuf::overflow: send failed. "
            "Reason: " << last_error_ << ", "
               << ACE_OS::strerror(last_error_);
            
          last_error_desc_ = ostr.str();
          return traits_type::eof();
        }

        if(debug_ostream_)
        {
          debug_ostream_->write(send_buffer_.get(), written_bytes);
        }

        if(interceptor_)
        {
          interceptor_->socket_stream_write(
            (const unsigned char*)send_buffer_.get(), written_bytes);
        }        
        
        sent_bytes_ += written_bytes;
  
        size_t left_bytes = send_buffer_size_ - written_bytes;
        
        memmove(send_buffer_.get(),
                send_buffer_.get() + written_bytes,
                left_bytes);
        
        send_buffer_.get()[left_bytes] = c;
        
        setp(send_buffer_.get() + left_bytes + 1,
             send_buffer_.get() + send_buffer_size_);

        return c;
      }
      
      int
      StreamBuf::sync()
      {
        if(!last_error_desc_.empty() || pptr() == 0)
        {
          return -1;
        }

        size_t bytes_to_write = pptr() - send_buffer_.get();

        for(char* ptr = send_buffer_.get(); bytes_to_write > 0; )
        {
          int written_bytes = socket_.send(ptr,
                                           bytes_to_write,
                                           send_timeout_.get());

          if(written_bytes <= 0)
          {
            last_error_ = ACE_OS::last_error();
 
            std::ostringstream ostr;            
            ostr << "El::Net::Socket::StreamBuf::sync: send failed. Reason: "
                 << ACE_OS::strerror(last_error_);

            last_error_desc_ = ostr.str();

            return -1;
          }

          if(debug_ostream_)
          {
            debug_ostream_->write(ptr, written_bytes);
          }

          if(interceptor_)
          {
            interceptor_->socket_stream_write(
              (const unsigned char*)ptr, written_bytes);
          }  
          
          ptr += written_bytes;
          sent_bytes_ += written_bytes;
          bytes_to_write -= written_bytes;
        }

        setp(send_buffer_.get(), send_buffer_.get() + send_buffer_size_);
        return 0;
      }

      void
      StreamBuf::read_limit(unsigned long long val) throw()
      {
        read_limit_ = val;
        has_read_limit_ = true;

        size_t buffered_bytes = egptr() - gptr();

        if(buffered_bytes <= read_limit_)
        {
          read_limit_ -= buffered_bytes;
        }
        else
        {
          setg(eback(), gptr(), gptr() + read_limit_);
          read_limit_ = 0;
        }
        
      }
      
      //
      // Stream class
      //
      
      Stream::Stream(const ACE_Time_Value* send_timeout,
                     const ACE_Time_Value* recv_timeout,
                     size_t send_buffer_size,
                     size_t recv_buffer_size,
                     size_t putback_buffer_size,
                     Interceptor* interceptor)
        throw(InvalidArg, ::El::Net::Socket::Exception, El::Exception)
          : std::basic_iostream<char, std::char_traits<char> >(0),
            connected_(false),
            interceptor_(interceptor)
      {
        socket_ = ACE_SOCK_StreamPtr(new ACE_SOCK_Stream());

        if(socket_->open(SOCK_STREAM, AF_INET, 0, 0) == -1)
        {
          socket_.reset();
          
          int error = ACE_OS::last_error();
      
          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::Stream: failed to open. Reason:"
               << std::endl << "errno " << error << ", "
               << ACE_OS::strerror(error);
          
          throw ::El::Net::Socket::Exception(ostr.str());
        }

        try
        {
          streambuf_.reset(new StreamBuf(*socket_,
                                         send_timeout,
                                         recv_timeout,
                                         send_buffer_size,
                                         recv_buffer_size,
                                         putback_buffer_size,
                                         interceptor_));
        }
        catch(const InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::Stream: "
            "El::Net::Socket::StreamBuf::InvalidArg exception caught. "
            "Description:\n" << e;

          throw InvalidArg(ostr.str());
        }

        init(streambuf_.get());
      }
      
      Stream::Stream(ACE_SOCK_Stream& sock_stream,
                     const ACE_Time_Value* send_timeout,
                     const ACE_Time_Value* recv_timeout,
                     size_t send_buffer_size,
                     size_t recv_buffer_size,
                     size_t putback_buffer_size,
                     Interceptor* interceptor)
        throw(InvalidArg, ::El::Net::Socket::Exception, El::Exception)
          : std::basic_iostream<char, std::char_traits<char> >(0),
            connected_(true),
            interceptor_(interceptor)
      {
        try
        {
          streambuf_.reset(new StreamBuf(sock_stream,
                                         send_timeout,
                                         recv_timeout,
                                         send_buffer_size,
                                         recv_buffer_size,
                                         putback_buffer_size,
                                         interceptor_));
        }
        catch(const InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::Stream: "
            "El::Net::Socket::StreamBuf::InvalidArg exception caught. "
            "Description:\n" << e;

          throw InvalidArg(ostr.str());
        }

        init(streambuf_.get());
      }

      Stream::Stream(const char* host,
                     unsigned short port,
                     const ACE_Time_Value* connect_timeout,
                     const ACE_Time_Value* send_timeout,
                     const ACE_Time_Value* recv_timeout,
                     size_t send_buffer_size,
                     size_t recv_buffer_size,
                     size_t putback_buffer_size,
                     Interceptor* interceptor)
        throw(Timeout, InvalidArg, ::El::Net::Socket::Exception, El::Exception)
          : std::basic_iostream<char, std::char_traits<char> >(0),
            connected_(true),
            interceptor_(interceptor)
      {
        if(host == 0 || *host == '\0')
        {
          throw InvalidArg("El::Net::Socket::Stream::Stream: host undefined");
        }
        
        if(port == 0)
        {
          throw InvalidArg("El::Net::Socket::Stream::Stream: port is 0");
        }
        
        socket_ = ACE_SOCK_StreamPtr(new ACE_SOCK_Stream());

        if(socket_->open(SOCK_STREAM, AF_INET, 0, 0) == -1)
        {
          socket_.reset();
          
          int error = ACE_OS::last_error();
      
          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::Stream: failed to open "
            "before connecting to " << host << ":" << port << ". Reason:"
               << std::endl << "errno " << error << ", "
               << ACE_OS::strerror(error);
          
          throw ::El::Net::Socket::Exception(ostr.str());
        }
        
/*        
        unsigned int bufsize = 2048;
          
        if(socket_->set_option (SOL_SOCKET,
                                SO_RCVBUF,
                                &bufsize,
                                sizeof(bufsize)) == -1)
        {
          int error = ACE_OS::last_error();

          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::Stream: unable to set socket "
            "option SO_RCVBUF with size " << bufsize << ". Error "
               << error << ", reason " << ACE_OS::strerror(error);
              
          throw Exception(ostr.str());
        }
*/

        ACE_SOCK_Connector connector;
        ACE_INET_Addr address(port, host);

        if(connector.connect(*socket_, address, connect_timeout) == -1)
        {
          int error = ACE_OS::last_error();
      
          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::Stream: failed to connect to "
               << host << ":" << port << ". Reason:" << std::endl << "errno "
               << error << ", " << ACE_OS::strerror(error);
      
          if (error == ETIME)
          {
            throw Timeout(ostr.str());
          }
          
          throw ::El::Net::Socket::Exception(ostr.str());
        }

        try
        {
          streambuf_.reset(new StreamBuf(*socket_,
                                         send_timeout,
                                         recv_timeout,
                                         send_buffer_size,
                                         recv_buffer_size,
                                         putback_buffer_size,
                                         interceptor_));
        }
        catch(const InvalidArg& e)
        {
          socket_->close();
          socket_.reset();

          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::Stream: "
            "El::Net::Socket::StreamBuf::InvalidArg exception caught. "
            "Description:\n" << e;

          throw InvalidArg(ostr.str());
        }
        catch(...)
        {
          socket_->close();
          socket_.reset();

          throw;
        }
        
        init(streambuf_.get());

        if(interceptor_)
        {
          interceptor_->socket_stream_connected(*this);
        }
      }
      
      void
      Stream::connect(const char* host,
                      unsigned short port,
                      const ACE_Time_Value* connect_timeout)
        throw(Timeout,
              InvalidArg,
              InvalidOperation,
              ::El::Net::Socket::Exception,
              El::Exception)
      {
        if(connected_)
        {
          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::connect: can't connect to "
               << host << ":" << port << " as already connected";

          throw InvalidOperation(ostr.str());          
        }

        ACE_SOCK_Connector connector;
        ACE_INET_Addr address(port, host);

        if(connector.connect(*socket_, address, connect_timeout) == -1)
        {
          int error = ACE_OS::last_error();
      
          std::ostringstream ostr;
          ostr << "El::Net::Socket::Stream::connect: failed to connect to "
               << host << ":" << port << ". Reason:" << std::endl << "errno "
               << error << ", " << ACE_OS::strerror(error);
      
          if (error == ETIME)
            throw Timeout(ostr.str());

          throw ::El::Net::Socket::Exception(ostr.str());
        }

        connected_ = true;

        if(interceptor_)
        {
          interceptor_->socket_stream_connected(*this);
        }        
      }
      
      Stream::~Stream() throw()
      {
        if(socket_.get())
        {
          socket_->close();
        }
      }
    }
  }
}

