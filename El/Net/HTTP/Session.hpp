/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/Session.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_SESSION_HPP_
#define _ELEMENTS_EL_NET_HTTP_SESSION_HPP_

#include <stdint.h>

#include <streambuf>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include <El/Exception.hpp>

#include <ace/OS.h>

#include <El/Compress/ZLib.hpp>
#include <El/String/Manip.hpp>

#include <El/Net/Socket/Stream.hpp>
#include <El/Net/HTTP/Exception.hpp>
#include <El/Net/HTTP/URL.hpp>
#include <El/Net/HTTP/Params.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      EL_EXCEPTION(Timeout, El::Net::HTTP::Exception);
      EL_EXCEPTION(UnsupportedSchema, El::Net::HTTP::Exception);

      enum Method
      {
        GET,
        POST
      };

      enum Version
      {
        HTTP_1_0,
        HTTP_1_1
      };

      enum TransferEncoding
      {
        TE_NONE,
        TE_CHUNKED
      };

      enum ContentEncoding
      {
        CE_IDENTITY,
        CE_DEFLATE,
        CE_GZIP,
        CE_COMPRESS
      };

      class Session
      {
      public:

        class Interceptor : public El::Net::Socket::Interceptor
        {
        public:
          virtual void chunk_begins(unsigned long long size)
            throw(El::Exception) = 0;

          virtual void socket_stream_created(El::Net::Socket::Stream& stream)
            throw(El::Exception) = 0;

          virtual ~Interceptor() throw() {}
        };

      public:
        Session(const char* url,
                Version version = HTTP_1_1,
                Interceptor* interceptor = 0,
                bool preserve_content_encoding = false)
          throw(InvalidArg, Exception, El::Exception);
        
        Session(const URL* url,
                Version version = HTTP_1_1,
                Interceptor* interceptor = 0,
                bool preserve_content_encoding = false)
          throw(InvalidArg, Exception, El::Exception);

        virtual ~Session() throw();

        void open(const ACE_Time_Value* connect_timeout = 0,
                  const ACE_Time_Value* send_timeout = 0,
                  const ACE_Time_Value* recv_timeout = 0,
                  size_t send_buffer_size = 1024,
                  size_t recv_buffer_size = 1024,
                  size_t putback_buffer_size = 1024)
          throw(Timeout, Exception, El::Exception);

        std::string send_request(Method method,
                                 const ParamList& params = ParamList(),
                                 const HeaderList& headers = HeaderList(),
                                 const char* body = 0,
                                 size_t body_len = 0,
                                 size_t follow_redirects = 0)
          throw(UnsupportedSchema, Timeout, Exception, El::Exception);

        //
        // true if read successfully and
        // status_code >= 200 && status_code < 400
        //
        bool recv_response_status()
          throw(Timeout, Exception, El::Exception);

        bool recv_response_header(Header& header)
          throw(Timeout, Exception, El::Exception);

        void close() throw();

        bool save_body(const char* file,
                       uint64_t max_size = UINT64_MAX,
                       uint64_t* crc = 0,
                       const char* enforce_encoding = 0)
          throw(Exception, El::Exception);
        
        std::istream& response_body() const throw(Exception, El::Exception);
        
        void test_completion() throw(Timeout, Exception, El::Exception);

        uint32_t status_code() const throw();
        const char* status_text() const throw(El::Exception);

        long long content_length() throw();
        const char* charset() throw() { return charset_.c_str(); }

        const HeaderList& trailer() const throw();
        
        bool valid() const throw();
        bool opened() const throw();

        unsigned long long sent_bytes(bool reset_counter = false) throw();
        
        unsigned long long received_bytes(bool reset_counter = false) throw();

        void debug_istream(std::ostream* ostr) throw();
        void debug_ostream(std::ostream* ostr) throw();

        ACE_SOCK_Stream& socket() const throw();

        enum StreamLayer
        {
          SL_NONE,
          SL_SOCKET,
          SL_TRANSFER_DECODER,
          SL_CONTENT_DECODER
        };
        
        int stream_last_error(StreamLayer& layer) const throw();
        const std::string& stream_last_error_desc() const throw(El::Exception);
        
        const URL* url() const throw() { return url_.in(); }

        const El::String::Array& all_urls() const throw() { return all_urls_; }
        
      protected:
        class ChunksDecodingStream;
        class ChunksDecodingStreamBuf;
        
        std::iostream& stream() const throw(Exception, El::Exception);
        void validate_gzip_header() throw(Timeout, Exception, El::Exception);
        
        static uint32_t ulong(const char* buff) throw();
        static uint16_t ushort(const char* buff) throw();

        void send_one_request(Method method,
                              const ParamList& params,
                              const HeaderList& headers,
                              const char* body,
                              size_t body_len)
          throw(Timeout, Exception, El::Exception);        
        
      protected:
        URL_var url_;
        Version version_;
        Interceptor* interceptor_;
        bool preserve_content_encoding_;
        bool opened_;
        bool valid_;
        bool status_code_read_;
        bool headers_read_;
        std::auto_ptr<ACE_Time_Value> connect_timeout_;
        size_t recv_buffer_size_;
        size_t putback_buffer_size_;
        uint32_t status_code_;
        std::string status_text_;
        long long content_length_;
        TransferEncoding transfer_encoding_;
        ContentEncoding content_encoding_;
        std::string charset_;
        HeaderList trailer_;
        El::String::Array all_urls_;
        
        typedef std::auto_ptr<Socket::Stream> SocketStreamPtr;
        SocketStreamPtr socket_stream_;

        typedef std::auto_ptr<ChunksDecodingStream> ChunksDecodingStreamPtr;
        ChunksDecodingStreamPtr chunks_decoding_stream_;

        std::auto_ptr<El::Compress::ZLib::InputStreamReader>
        deflate_decoding_stream_reader_;

        std::auto_ptr<El::Compress::ZLib::InStream>
        deflate_decoding_stream_;
        
        std::istream* response_body_stream_;
        const std::string empty_str_;
      };

      class Session::ChunksDecodingStreamBuf
        : public std::basic_streambuf<char, std::char_traits<char> >
      {
      public:
        ChunksDecodingStreamBuf(Socket::Stream& socket_stream,
                                HeaderList& trailer,
                                size_t recv_buffer_size,
                                size_t putback_buffer_size,
                                Interceptor* interceptor)
          throw(Exception, El::Exception);
        
        virtual ~ChunksDecodingStreamBuf() throw();

        int last_error() const throw();
        const std::string& last_error_desc() const throw(El::Exception);
        
      protected:
        virtual std::streamsize showmanyc();
        virtual int_type underflow();

        int_type read_to_buf(char* eback_ptr,
                             char* read_ptr,
                             size_t bytes_to_read)
          throw(El::Exception);
        
        bool read_trailer() throw(El::Exception);
        
      protected:
        Socket::Stream& socket_stream_;
        HeaderList& trailer_;

        size_t recv_buffer_size_;
        size_t putback_buffer_size_;
        size_t recv_buffer_total_size_;
        unsigned long long chunk_size_;
        Interceptor* interceptor_;
        
        char* recv_buffer_;        

        int last_error_;
        std::string last_error_desc_;
      };

      class Session::ChunksDecodingStream :
        public std::basic_istream<char, std::char_traits<char> >
      {
      public:
        ChunksDecodingStream(Socket::Stream& socket_stream,
                             HeaderList& trailer,
                             size_t recv_buffer_size,
                             size_t putback_buffer_size,
                             Interceptor* interceptor)
          throw(Exception, El::Exception);

        virtual ~ChunksDecodingStream() throw();

        int last_error() const throw();
        const std::string& last_error_desc() const throw(El::Exception);
        
      private:
        typedef std::auto_ptr<ChunksDecodingStreamBuf>
        ChunksDecodingStreamBufPtr;

        ChunksDecodingStreamBufPtr streambuf_;
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
    namespace HTTP
    {
      //
      // Session class
      //

      inline
      ACE_SOCK_Stream&
      Session::socket() const throw()
      {
        if(opened_)
        {
          return socket_stream_->socket();
        }

        throw Exception("El::Net::HTTP::Session::socket: "
                        "session is not opened");
      }
      
      inline
      std::iostream&
      Session::stream() const throw(Exception, El::Exception)
      {
        if(opened_)
        {
          return *socket_stream_;
        }

        throw Exception("El::Net::HTTP::Session::stream: "
                        "session is not opened");
      }
      
      inline
      std::istream&
      Session::response_body() const throw(Exception, El::Exception)
      {
        if(response_body_stream_ == 0)
        {
          throw Exception("El::Net::HTTP::Session::response_body: "
                          "didn't read up to the body yet");
        }

        return *response_body_stream_;
      }
      
      inline
      uint32_t
      Session::status_code() const throw()
      {
        return status_code_;
      }
      
      inline
      const char*
      Session::status_text() const throw(El::Exception)
      {
        return status_text_.c_str();
      }

      inline
      const HeaderList&
      Session::trailer() const throw()
      {
        return trailer_;
      }
      
      inline
      bool
      Session::opened() const throw()
      {
        return opened_;
      }

      inline
      bool
      Session::valid() const throw()
      {
        return valid_;
      }

      inline
      long long
      Session::content_length() throw()
      {
        return content_length_;
      }

      inline
      unsigned long long
      Session::sent_bytes(bool reset_counter) throw()
      {
        return socket_stream_.get() ?
          socket_stream_->sent_bytes(reset_counter) : 0;
      }
      
      inline
      unsigned long long
      Session::received_bytes(bool reset_counter) throw()
      {
        return socket_stream_.get() ?
          socket_stream_->received_bytes(reset_counter) : 0;
      }
      
      inline
      void
      Session::debug_ostream(std::ostream* ostr) throw()
      {
        socket_stream_->socket_streambuf().debug_ostream(ostr);
      }
      
      inline
      void
      Session::debug_istream(std::ostream* ostr) throw()
      {
        socket_stream_->socket_streambuf().debug_istream(ostr);
      }

      inline
      uint32_t
      Session::ulong(const char* buff) throw()
      {
        unsigned char* ubuff = (unsigned char*)buff;
        
        return ((uint32_t)ubuff[0]) |
          (((uint32_t)ubuff[1]) << 8) |
          (((uint32_t)ubuff[2]) << 16) |
          (((uint32_t)ubuff[3]) << 24);
      }
      
      inline
      uint16_t
      Session::ushort(const char* buff) throw()
      {
        unsigned char* ubuff = (unsigned char*)buff; 
        return ((uint16_t)ubuff[0]) | (((uint16_t)ubuff[1]) << 8);
      }
      
      //
      // Session::ChunksDecodingStreamBuf
      //
      inline
      Session::ChunksDecodingStreamBuf::~ChunksDecodingStreamBuf() throw()
      {
        delete [] recv_buffer_;
      }

      inline
      int
      Session::ChunksDecodingStreamBuf::last_error() const throw()
      {
        return last_error_;
      }
      
      inline
      const std::string&
      Session::ChunksDecodingStreamBuf::last_error_desc() const
        throw(El::Exception)
      {
        return last_error_desc_;
      }
      
      //
      // Session::ChunksDecodingStream
      //
      inline
      Session::ChunksDecodingStream::~ChunksDecodingStream() throw()
      {
      }

      inline
      int
      Session::ChunksDecodingStream::last_error() const throw()
      {
        return streambuf_->last_error();
      }
      
      inline
      const std::string&
      Session::ChunksDecodingStream::last_error_desc() const
        throw(El::Exception)
      {
        return streambuf_->last_error_desc();
      }

    }
  }
}

#endif // _ELEMENTS_EL_NET_HTTP_SESSION_HPP_
