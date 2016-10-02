/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/Session.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include <sstream>
#include <fstream>

#include <ace/OS.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>
#include <ace/INET_Addr.h>

#include <El/CRC.hpp>

#include "Session.hpp"
#include "StatusCodes.hpp"
#include "Headers.hpp"

namespace
{
  const char WHITESPACES[] = " \t";
};

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      //
      // Session class
      //
      Session::Session(const char* url,
                       Version version,
                       Interceptor* interceptor,
                       bool preserve_content_encoding)
        throw(InvalidArg, Exception, El::Exception)
          : version_(version),
            interceptor_(interceptor),
            preserve_content_encoding_(preserve_content_encoding),
            opened_(false),
            valid_(false),
            status_code_read_(false),
            headers_read_(false),
            recv_buffer_size_(0),
            putback_buffer_size_(0),
            status_code_(0),
            content_length_(-1),
            transfer_encoding_(TE_NONE),
            content_encoding_(CE_IDENTITY),
            response_body_stream_(0)
      {
        try
        {
          url_ = new URL(url);
        }
        catch(const URL::InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::Session::Session: URL::InvalidArg exception "
            "caught. Description: " << e;

          throw InvalidArg(ostr.str());
        }

        all_urls_.push_back(url);
      }
      
      Session::Session(const URL* url,
                       Version version,
                       Interceptor* interceptor,
                       bool preserve_content_encoding)
        throw(InvalidArg, Exception, El::Exception)
          : version_(version),
            interceptor_(interceptor),
            preserve_content_encoding_(preserve_content_encoding),
            opened_(false),
            valid_(false),
            status_code_read_(false),
            headers_read_(false),
            recv_buffer_size_(0),
            putback_buffer_size_(0),
            status_code_(0),
            content_length_(-1),
            transfer_encoding_(TE_NONE),
            content_encoding_(CE_IDENTITY),
            response_body_stream_(0)
      {
        if(url == 0)
        {
          throw InvalidArg("El::Net::HTTP::Session::Session: url is 0");
        }

        try
        {
          url_ = new URL(*url);
        }
        catch(const URL::InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::Session::Session: URL::InvalidArg exception "
            "caught. Description: " << e;

          throw InvalidArg(ostr.str());
        }

        all_urls_.push_back(url->string());
      }
      
      Session::~Session() throw()
      {
        close();
      }
      
      void
      Session::open(const ACE_Time_Value* connect_timeout,
                    const ACE_Time_Value* send_timeout,
                    const ACE_Time_Value* recv_timeout,
                    size_t send_buffer_size,
                    size_t recv_buffer_size,
                    size_t putback_buffer_size)
        throw(Timeout, Exception, El::Exception)
      {
        if(send_buffer_size == 0 || recv_buffer_size == 0)
        {
          throw
            InvalidArg("El::Net::HTTP::Session::open: "
                       "not send_buffer_size nor recv_buffer_size can be 0");
        }
        
        close();

        recv_buffer_size_ = recv_buffer_size;
        putback_buffer_size_ = putback_buffer_size;
          
        try
        {
          socket_stream_ =
            SocketStreamPtr(new Socket::Stream(send_timeout,
                                               recv_timeout,
                                               send_buffer_size,
                                               recv_buffer_size,
                                               putback_buffer_size,
                                               interceptor_));

          if(interceptor_)
          {
            interceptor_->socket_stream_created(*socket_stream_);
          }

            
          socket_stream_->connect(url_->idn_host(),
                                  url_->port(),
                                  connect_timeout);

          connect_timeout_.reset(
            connect_timeout ? new ACE_Time_Value(*connect_timeout) : 0);
          
          opened_ = true;
          valid_ = true;
        }
        catch(const Socket::Stream::Timeout& e)
        {
          socket_stream_.reset();
          
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::Session::open: "
            "Socket::Stream::Timeout exception caught. Description:\n"
               << e;

          throw Timeout(ostr.str());
        }
        
      }

      void
      Session::close() throw()
      {
        if(opened_)
        {
          opened_ = false;

          deflate_decoding_stream_.reset();
          deflate_decoding_stream_reader_.reset();
          chunks_decoding_stream_.reset();
          socket_stream_.reset();
        }

        valid_ = false;
        status_code_read_ = false;
        headers_read_ = false;
        connect_timeout_.reset(0);
        status_code_ = 0;
        status_text_.clear();
        content_length_ = -1;
        transfer_encoding_ = TE_NONE;
        content_encoding_ = CE_IDENTITY;
        response_body_stream_ = 0;
        recv_buffer_size_ = 0;
        putback_buffer_size_ = 0;
        trailer_.clear();
      }

      std::string
      Session::send_request(Method method,
                            const ParamList& params,
                            const HeaderList& headers,
                            const char* body,
                            size_t body_len,
                            size_t follow_redirects)
        throw(UnsupportedSchema, Timeout, Exception, El::Exception)
      {
        std::string permanent_location;
        
        while(true)
        {
          if(url_->secure())
          {
            throw UnsupportedSchema("El::Net::HTTP::Session::send_request: "
                                    "SSL is not supported at the moment");
          }
          
          send_one_request(method, params, headers, body, body_len);

          if(follow_redirects == 0)
          {
            return permanent_location;
          }

          recv_response_status();
          uint32_t code = status_code();
          
          switch(code)
          {
          case El::Net::HTTP::SC_SEE_OTHER:
            method = GET;
            body = 0;
            body_len = 0;
            // break is ommited intentionally !
          case El::Net::HTTP::SC_FOUND:
          case El::Net::HTTP::SC_MOVED_PERMANENTLY:
          case El::Net::HTTP::SC_TEMPORARY_REDIRECT:
            {
              std::string new_url;
              El::Net::HTTP::Header header;
            
              while(recv_response_header(header) && new_url.empty())
              {
                if(!strcasecmp(header.name.c_str(),
                               El::Net::HTTP::HD_LOCATION))
                {
                  new_url = url_->abs_url(header.value.c_str());
                }
              }

              if(new_url.empty())
              {
                std::ostringstream ostr;
                ostr << "El::Net::HTTP::Session::send_request: no proper "
                     << El::Net::HTTP::HD_LOCATION << " header "
                  "discovered in response for " << url_->string();
              
                throw Exception(ostr.str());
              }

              std::auto_ptr<ACE_Time_Value> connect_timeout;
              
              if(connect_timeout_.get())
              {
                connect_timeout.reset(new ACE_Time_Value(*connect_timeout_));
              }

              El::Net::Socket::StreamBuf& streambuf =
                socket_stream_->socket_streambuf();
              
              std::auto_ptr<ACE_Time_Value> send_timeout;
              
              if(streambuf.send_timeout())
              {
                send_timeout.reset(
                  new ACE_Time_Value(*streambuf.send_timeout()));
              } 

              std::auto_ptr<ACE_Time_Value> recv_timeout;
              
              if(streambuf.recv_timeout())
              {
                recv_timeout.reset(
                  new ACE_Time_Value(*streambuf.recv_timeout()));
              }

              size_t send_buffer_size = streambuf.send_buffer_size();
              size_t recv_buffer_size = streambuf.recv_buffer_size();
              size_t putback_buffer_size = streambuf.putback_buffer_size();

              unsigned long long sent_bytes = streambuf.sent_bytes();
              unsigned long long received_bytes = streambuf.received_bytes();

              std::ostream* dbg_istream = streambuf.debug_istream();
              std::ostream* dbg_ostream = streambuf.debug_ostream();

              close();

              url_ = new URL(new_url.c_str());

              if(code == El::Net::HTTP::SC_MOVED_PERMANENTLY)
              {
                permanent_location = url_->string();
              }              

              open(connect_timeout.get(),
                   send_timeout.get(),
                   recv_timeout.get(),
                   send_buffer_size,
                   recv_buffer_size,
                   putback_buffer_size);

              all_urls_.push_back(new_url);

              socket_stream_->set_sent_bytes(sent_bytes);
              socket_stream_->set_received_bytes(received_bytes);
                
              debug_istream(dbg_istream);
              debug_ostream(dbg_ostream);
              
              follow_redirects--;
              continue;
            }
          default:
            {
              return permanent_location;
            }
          }
        }
      }
 
      void
      Session::send_one_request(Method method,
                                const ParamList& params,
                                const HeaderList& headers,
                                const char* body,
                                size_t body_len)
        throw(Timeout, Exception, El::Exception)
      {
        switch(method)
        {
        case GET:
          {
            stream() << "GET " << url_->path();

            bool has_params = *url_->params() != '\0';
            
            if(has_params)
            {
              stream() << "?" << url_->params();
            }

            for(ParamList::const_iterator it = params.begin();
                it != params.end(); it++)
            {
              stream() << (has_params ? "&" : "?");
              
              if(it != params.begin())
              {
                stream() << "&";
              }

              stream() << *it;
            }

            stream() << " "
                     << (version_ == HTTP_1_0 ? "HTTP/1.0" : "HTTP/1.1")
                     << "\r\n";
            
            break;
          }
        case POST:
          {
            stream() << "POST " << url_->path() << " "
                     << (version_ == HTTP_1_0 ? "HTTP/1.0" : "HTTP/1.1")
                     << "\r\n";
            
            break;
          }
        }

        for(HeaderList::const_iterator it = headers.begin();
            it != headers.end(); it++)
        {
          stream() << *it;
        }

        std::string params_post_body;
        
        if(method == POST)
        {
          std::ostringstream ostr;
          
          for(ParamList::const_iterator it = params.begin();
              it != params.end(); it++)
          {
            if(it != params.begin())
            {
              ostr << "&";
            }

            ostr << *it;
          }

          params_post_body = ostr.str();
          
          body = params_post_body.c_str();
          body_len = params_post_body.size();
        }

        if(method == POST || body_len)
        {
          stream() << HD_CONTENT_LENGTH << ": " << body_len << "\r\n";
        }

        stream() << HD_HOST << ": " << url_->idn_host();

        if(url_->port() != (url_->secure() ? 443 : 80))
        {
          stream() << ":" << url_->port();
        }
        
        stream() << "\r\n\r\n";

        if(body_len)
        {
          stream().write(body, body_len);
        }
        
        stream().flush();

        if(stream().fail() || stream().bad())
        {
          valid_ = false;
          
          int error = socket_stream_->last_error();

          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::send_request: send failed. "
            "Reason: " << error << ", " << socket_stream_->last_error_desc();
          
          if (error == ETIME)
          {
            throw Timeout(ostr.str());
          }
          else
          {
            throw Exception(ostr.str());
          }
        }
      }
      
      bool
      Session::recv_response_status() throw(Timeout, Exception, El::Exception)
      {
        if(!valid())
        {
          throw Exception("El::Net::HTTP::Session::recv_response_status: "
                          "invalid session");
        }

        if(!status_code_read_)
        {
          std::string version;
        
          // Read HTTP protocol version and status
          stream() >> version >> status_code_;
          std::getline(stream(), status_text_, '\r');

          char ch = stream().get();

          if(stream().fail() || stream().bad())
          {
            status_code_ = 0;
            status_text_.clear();
            valid_ = false;
          
            int error = socket_stream_->last_error();

            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::recv_response_status: "
              "read failed. Reason: " << error << ", "
                 << socket_stream_->last_error_desc();
          
            if (error == ETIME)
            {
              throw Timeout(ostr.str());
            }
            else
            {
              throw Exception(ostr.str());
            }
          }
        
          if(ch != '\n')
          {
            valid_ = false;
          
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::recv_response_status: "
              "unexpected character encountered: '" << ch << "'";
          
            throw Exception(ostr.str());
          }

          status_code_read_ = true;
        }
        
        return status_code_ >= SC_OK && status_code_ < SC_BAD_REQUEST;
      }

      bool
      Session::recv_response_header(Header& header)
        throw(Timeout, Exception, El::Exception)
      {
        if(!valid())
        {
          throw Exception("El::Net::HTTP::Session::recv_response_header: "
                          "invalid session");
        }

        if(headers_read_)
        {
          return false;
        }
        
        std::string line;
        std::getline(stream(), line, '\r');
          
        char ch = stream().get();
          
        if(stream().fail() || stream().bad())
        {
          valid_ = false;
          
          int error = socket_stream_->last_error();

          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::recv_response_header: "
            "read failed. Reason: " << error << ", "
               << socket_stream_->last_error_desc();
          
          if (error == ETIME)
          {
            throw Timeout(ostr.str());
          }
          else
          {
            throw Exception(ostr.str());
          }
        }
        
        if(ch != '\n')
        {
          valid_ = false;
          
          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::recv_response_header: "
            "unexpected character encountered: '" << ch << "'";
          
          throw Exception(ostr.str());
        }

        if(line.empty())
        {
          //
          // End of headers
          //

          headers_read_ = true;

          if(transfer_encoding_ == TE_CHUNKED)
          {
            chunks_decoding_stream_.reset(
              new ChunksDecodingStream(*socket_stream_,
                                       trailer_,
                                       recv_buffer_size_,
                                       putback_buffer_size_,
                                       interceptor_));
            
            response_body_stream_ = chunks_decoding_stream_.get();
          }
          else
          {
            response_body_stream_ = socket_stream_.get();

            if(content_length_ >= 0)
            {
              socket_stream_->socket_streambuf().read_limit(content_length_);
            }
          }

          if(!preserve_content_encoding_)
          {
            switch(content_encoding_)
            {
            case CE_GZIP:
              {
                validate_gzip_header();
              
                // no break here, continue as with deflate encoding
              }
            case CE_COMPRESS:
              {
                // no break here, continue as with deflate encoding
              }
            case CE_DEFLATE:
              {
                deflate_decoding_stream_reader_.reset(
                  new El::Compress::ZLib::InputStreamReader(
                    *response_body_stream_));

                bool raw_deflate;
                
                switch(content_encoding_)
                {
                case CE_GZIP:
                case CE_COMPRESS:
                  {
                    raw_deflate = true;
                    break;
                  }
                default:
                  {
                    unsigned char buff[2];
                    memset(buff, 0, sizeof(buff));
                    
                    size_t len =
                      deflate_decoding_stream_reader_->
                      read((char*)buff, sizeof(buff));
                    
                    raw_deflate = buff[0] != 0x78 || buff[1] != 0x9C;
                    deflate_decoding_stream_reader_->putback((char*)buff, len);
                    break;
                  }
                }
                
                deflate_decoding_stream_.reset(
                  new El::Compress::ZLib::InStream(
                    deflate_decoding_stream_reader_.get(),
                    recv_buffer_size_,
                    recv_buffer_size_,
                    putback_buffer_size_,
                    raw_deflate,
                    true));

                response_body_stream_ = deflate_decoding_stream_.get();

                break;
              }
            default: ;
            }
          }
          
          return false;
        }          

        std::string::size_type len = line.find(':');

        if(len == std::string::npos)
        {
          valid_ = false;
          
          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::recv_response_header: "
            "character ':' not found while parsing HTTP response header. "
            "Line:\n" << line;
          
          throw Exception(ostr.str());
        }

        const char* ptr = line.c_str();
        size_t pos = strspn(ptr, WHITESPACES);

        ptr += pos;
        len -= pos;

        if(!len)
        {
          valid_ = false;
          
          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::recv_response_header: "
            "invalid HTTP response header:\n" << line;
          
          throw Exception(ostr.str());
        }
        
        for(pos = len - 1; pos > 0 && strchr(WHITESPACES, ptr[pos]) != 0;
            pos--);

        header.name.assign(ptr, pos + 1);

        ptr += len + 1;
        ptr += strspn(ptr, WHITESPACES);

        for(pos = strlen(ptr);
            pos > 0 && strchr(WHITESPACES, ptr[pos - 1]) != 0; pos--);

        header.value.assign(ptr, pos);

        if(strcasecmp(header.name.c_str(), HD_CONTENT_LENGTH) == 0)
        {
          if(transfer_encoding_ != TE_CHUNKED)
          {
            char* invalid_char_ptr = 0;
            long long len = strtoll(header.value.c_str(),
                                    &invalid_char_ptr, 10);

            if((invalid_char_ptr != 0 && *invalid_char_ptr != '\0') || len < 0)
            {
              valid_ = false;
            
              std::ostringstream ostr;            
              ostr << "El::Net::HTTP::Session::recv_response_header: "
                "invalid Content-Length header:\n" << line;
          
              throw Exception(ostr.str());
            }

            content_length_ = len;
          }
          
        }
        else if(strcasecmp(header.name.c_str(), HD_TRANSFER_ENCODING) == 0)
        {
          if(version_ == HTTP_1_0)
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::recv_response_header: "
              "unexpected for HTTP/1.0 Transfer-Encoding header:\n" << line;
          
            throw Exception(ostr.str());            
          }
            
          if(strcasecmp(header.value.c_str(), "chunked") == 0)
          {
            transfer_encoding_ = TE_CHUNKED;
            content_length_ = -1;
          }
          else
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::recv_response_header: "
              "unsupported Transfer-Encoding header:\n" << line;
          
            throw Exception(ostr.str());
          }
        }
        else if(strcasecmp(header.name.c_str(), HD_CONTENT_ENCODING) == 0)
        {
          if(strcasecmp(header.value.c_str(), "deflate") == 0)
          {
            content_encoding_ = CE_DEFLATE;
          }
          else if(strcasecmp(header.value.c_str(), "gzip") == 0)
          {
            content_encoding_ = CE_GZIP;
          }
          else if(strcasecmp(header.value.c_str(), "none"))
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::recv_response_header: "
              "content encoding '" << header.value << "' is not supported";
            
            throw Exception(ostr.str());
          }
        }
        else if(strcasecmp(header.name.c_str(), HD_CONTENT_TYPE) == 0)
        {
//        if(charset_.empty())
// Always reset charset which could be inherited from redirect response
// and can not correspond to final content returned by server          
          {
            content_type(header.value.c_str(), charset_);
          }
        }
        
        return true;
      }

      void
      Session::validate_gzip_header() throw(Timeout, Exception, El::Exception)
      {
        unsigned char id1 = response_body_stream_->get();
        unsigned char id2 = response_body_stream_->get();
        unsigned char compression_method = response_body_stream_->get();
        unsigned char flags = response_body_stream_->get();
        char mtime[4];
        response_body_stream_->read(mtime, sizeof(mtime));

        response_body_stream_->get(); // reading "extra" flags
        response_body_stream_->get(); // reading OS flag

        if(!response_body_stream_->fail() &&
           !response_body_stream_->bad())
        {
          if(id1 != 0x1F && id2 != 0x8B)
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::validate_gzip_header: "
              "reading GZIP header failed. Reason: unexpected ids 0x"
                 << std::hex << (unsigned long)id1 << ", 0x" << std::hex
                 << (unsigned long)id2 << " instead of 0x1F, 0x8B";

            throw Exception(ostr.str());                
          }

          if(compression_method != Z_DEFLATED)
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::validate_gzip_header: "
              "reading GZIP header failed. Reason: unexpected "
              "compression method " << (unsigned long) compression_method
                 << " instead of " << Z_DEFLATED;
                
            throw Exception(ostr.str());                
          }

          if(flags & 0x4) // extra field
          {
            char buff[2];
            response_body_stream_->read(buff, sizeof(buff));

            size_t len = ushort(buff);
            while(len--)
            {
              response_body_stream_->get();
            }
          }

          if(flags & 0x8) // Orig. name
          {
            while(!response_body_stream_->fail() &&
                  response_body_stream_->get() != '\0');
          }

          if(flags & 0x10) // Comment
          {
            while(!response_body_stream_->fail() &&
                  response_body_stream_->get() != '\0');
          }

          if(flags & 0x2) // CRC16
          {
            char buff[2];
            response_body_stream_->read(buff, sizeof(buff));
          }
        }
              
        if(response_body_stream_->fail() || response_body_stream_->bad())
        {
          valid_ = false;

          StreamLayer stream_layer = SL_NONE;
          int error = stream_last_error(stream_layer);
                  
          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::validate_gzip_header: "
            "reading GZIP header failed. Reason: " << error << ", "
               << stream_last_error_desc();
          
          if (error == ETIME && stream_layer == SL_SOCKET)
          {
            throw Timeout(ostr.str());
          }

          throw Exception(ostr.str());
        }
      }
      
      void
      Session::test_completion() throw(Timeout, Exception, El::Exception)
      {
        if(!opened())
        {
          throw Exception("El::Net::HTTP::Session::test_completion: "
                          "session not opened");
        }
        
        if(!valid())
        {
          throw Exception("El::Net::HTTP::Session::test_completion: "
                          "session invalidated");
        }

        const std::string& error_desc = stream_last_error_desc();

        if(!error_desc.empty())
        {
          StreamLayer stream_layer = SL_NONE;
          int error = stream_last_error(stream_layer);

          if(error == 0 && stream_layer == SL_SOCKET)
          {
            // Nothing. Server just closed connection.
          }
          else
          {
            std::ostringstream ostr;
            ostr << "El::Net::HTTP::Session::test_completion: "
              "network failure occured. Reason:" << std::endl << "errno "
                 << error << ", " << error_desc;
      
            if (error == ETIME && stream_layer == SL_SOCKET)
            {
              throw Timeout(ostr.str());
            }

            throw Exception(ostr.str());
          }
          
        }

        bool deflate_checked = false;
        
        if(!preserve_content_encoding_ && deflate_decoding_stream_.get() != 0)
        {
          if(content_encoding_ == CE_GZIP)
          {
            char buff[4];
            memset(buff, 0, sizeof(buff));

            size_t len =
              deflate_decoding_stream_reader_->read(buff, sizeof(buff));

            if(len < sizeof(buff))
            {
              throw Exception("El::Net::HTTP::Session::test_completion: "
                              "can't read data CRC");              
            }
            
            if(ulong(buff) != deflate_decoding_stream_->out_crc())
            {
              std::ostringstream ostr;
              ostr << "El::Net::HTTP::Session::test_completion: "
                "data crc 0x" << std::hex
                   << deflate_decoding_stream_->out_crc()
                   << " do not match expected one 0x" << std::hex
                   << ulong(buff);
            
              throw Exception(ostr.str());              
            }

            len = deflate_decoding_stream_reader_->read(buff, sizeof(buff));

            if(len < sizeof(buff))
            {
              throw Exception("El::Net::HTTP::Session::test_completion: "
                              "can't read uncompressed data len");
            }
            
            if(ulong(buff) != deflate_decoding_stream_->out_bytes())
            {
              std::ostringstream ostr;
              ostr << "El::Net::HTTP::Session::test_completion: "
                "uncompressed data len "
                   << deflate_decoding_stream_->out_bytes()
                   << " do not match expected one " << ulong(buff);
            
              throw Exception(ostr.str());              
            }

            deflate_checked = true;
          }
        }

        if(response_body_stream_ != 0 && !deflate_checked)
        {
          if(content_length_ < 0)
          {
            if(!response_body_stream_->eof() && !response_body_stream_->fail())
            {
              throw Exception("Session::test_completion: "
                              "still has unread some data");
            }
          }
          else
          {
            unsigned long long read_limit =
              socket_stream_->socket_streambuf().read_limit();
          
            if(read_limit > 0)
            {
              std::ostringstream ostr;
              ostr << "Session::test_completion: still has unread "
                   << read_limit << " bytes";
          
              throw Exception(ostr.str());
            }
          }
        }
      }

      int
      Session::stream_last_error(StreamLayer& layer) const throw()
      {
        if(socket_stream_.get() != 0 &&
           !socket_stream_->last_error_desc().empty())
        {
          layer = SL_SOCKET;
          return socket_stream_->last_error();
        }

        if(chunks_decoding_stream_.get() != 0 &&
           !chunks_decoding_stream_->last_error_desc().empty())
        {
          layer = SL_TRANSFER_DECODER;
          return chunks_decoding_stream_->last_error();
        }
        
        if(deflate_decoding_stream_.get() != 0 &&
           !deflate_decoding_stream_->last_error_desc().empty())
        {
          layer = SL_CONTENT_DECODER;
          return deflate_decoding_stream_->last_error();
        }

        layer = SL_NONE;
        
        return 0;
      }
      
      const std::string&
      Session::stream_last_error_desc() const throw(El::Exception)
      {
        if(socket_stream_.get() != 0 &&
           !socket_stream_->last_error_desc().empty())
        {
          return socket_stream_->last_error_desc();
        }

        if(chunks_decoding_stream_.get() != 0 &&
           !chunks_decoding_stream_->last_error_desc().empty())
        {
          return chunks_decoding_stream_->last_error_desc();
        }
        
        if(deflate_decoding_stream_.get() != 0 &&
           !deflate_decoding_stream_->last_error_desc().empty())
        {
          return deflate_decoding_stream_->last_error_desc();
        }

        return empty_str_;
      }

      bool
      Session::save_body(const char* file_name,
                         uint64_t max_size,
                         uint64_t* crc,
                         const char* enforce_encoding)
        throw(Exception, El::Exception)
      {
        std::fstream file(file_name, ios::out);
        
        if(!file.is_open())
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::Session::save_body: failed to open '"
               << file_name << "' for write access";
          
          throw Exception(ostr.str());
        }

        std::istream& body_str = response_body();
        uint64_t size = 0;

        if(enforce_encoding && strcasecmp(enforce_encoding, "UTF-8") == 0)
        {
          while(!body_str.fail() && size < max_size)
          {
            wchar_t chr = 0;
            
            size_t read_bytes =
              El::String::Manip::utf8_to_wchar(body_str, chr, true);

            size += read_bytes;

            if(chr)
            {
              El::String::Manip::wchar_to_utf8(chr, file, crc);
              
              if(file.fail())
              {
                std::ostringstream ostr;
                ostr << "El::Net::HTTP::Session::save_body: "
                  "failed to write to '" << file_name << "'";
                
                throw Exception(ostr.str());
              }
            }
          }
        }
        else
        {
          while(!body_str.fail() && size < max_size)
          {
            char buff[1024];
            
            body_str.read(buff,
                          std::min((uint64_t)sizeof(buff), max_size - size));
          
            file.write(buff, body_str.gcount());
        
            if(file.fail())
            {
              std::ostringstream ostr;
              ostr << "El::Net::HTTP::Session::save_body: failed to write to '"
                   << file_name << "'";
            
              throw Exception(ostr.str());
            }

            size_t read_bytes = body_str.gcount();
            size += read_bytes;

            if(crc)
            {
              El::CRC(*crc, (const unsigned char*)buff, read_bytes);
            }
          }
        }
        
        if(body_str.fail())
        {
          test_completion();
        }
        else
        {
          return false;
        }

        if(file.bad() || file.fail())
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::Session::save_body: "
            "failed to write to file '" << file_name << "'";
        
          throw Exception(ostr.str());
        }

        file.flush();
        return true;
      }
      
      //
      // Session::ChunksDecodingStreamBuf
      //

      Session::ChunksDecodingStreamBuf::ChunksDecodingStreamBuf(
        Socket::Stream& socket_stream,
        HeaderList& trailer,
        size_t recv_buffer_size,
        size_t putback_buffer_size,
        Interceptor* interceptor)
        throw(Exception, El::Exception)
          : socket_stream_(socket_stream),
            trailer_(trailer),
            recv_buffer_size_(recv_buffer_size),
            putback_buffer_size_(putback_buffer_size),
            recv_buffer_total_size_(recv_buffer_size + putback_buffer_size),
            chunk_size_(0),
            interceptor_(interceptor),
            recv_buffer_(new char[recv_buffer_total_size_]),
            last_error_(0)
      {
        if(!recv_buffer_size)
        {
          throw Exception("El::Net::HTTP::Session::ChunksDecodingStreamBuf: "
                          "recv_buffer_size is 0");
        }
      }

      std::streamsize
      Session::ChunksDecodingStreamBuf::showmanyc()
      {
        if(!last_error_desc_.empty())
        {
          return 0;
        }

        return egptr() - gptr();
      }
      
      Session::ChunksDecodingStreamBuf::int_type
      Session::ChunksDecodingStreamBuf::underflow()
      {
        if(!last_error_desc_.empty())
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
      
      Session::ChunksDecodingStreamBuf::int_type
      Session::ChunksDecodingStreamBuf::read_to_buf(
        char* eback_ptr,
        char* read_ptr,
        size_t bytes_to_read)
        throw(El::Exception)
      {
        if(chunk_size_ == 0)
        {
          std::string line;
          std::getline(socket_stream_, line, '\r');

          char ch = socket_stream_.get();
          
          if(socket_stream_.fail() || socket_stream_.bad())
          {
            last_error_ = socket_stream_.last_error();
            last_error_desc_ = socket_stream_.last_error_desc();

            if(last_error_desc_.empty())
            {
              std::ostringstream ostr;            
              ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
                "read_to_buf: failed to read chunk size";

              last_error_desc_ = ostr.str();
            }

            return traits_type::eof();
          }

          if(ch != '\n')
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
              "read_to_buf: while reading chunk size "
              "unexpected character encountered: '" << ch << "'";

            last_error_desc_ = ostr.str();

            return traits_type::eof();
          }
          
          char* end = 0;
          unsigned long long chunk_size = strtoll(line.c_str(), &end, 16);

          if(end && strchr("\t ;", *end) == 0)
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
              "read_to_buf: failed to read chunk size from line '" << line
                 << "'";

            last_error_desc_ = ostr.str();

            return traits_type::eof();
          }

          chunk_size_ = chunk_size;

          if(interceptor_)
          {
            interceptor_->chunk_begins(chunk_size_);
          }

          if(chunk_size_ == 0)
          {
            while(read_trailer());
            
            return traits_type::eof();            
          }
        }

        if(bytes_to_read > chunk_size_)
        {
          bytes_to_read = chunk_size_;
        }
        
        socket_stream_.read(read_ptr, bytes_to_read);
        size_t bytes_read = socket_stream_.gcount();

        if(bytes_read == 0)
        {
          last_error_ = socket_stream_.last_error();
          last_error_desc_ = socket_stream_.last_error_desc();

          if(last_error_desc_.empty())
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
              "read_to_buf: failed to read chunk (" << bytes_to_read
                 << " bytes)";

            last_error_desc_ = ostr.str();
          }
          
          return traits_type::eof();
        }

        chunk_size_ -= bytes_read;

        if(!chunk_size_)
        {
          char ch1 = socket_stream_.get();
          char ch2 = socket_stream_.get();

          if(socket_stream_.fail() || socket_stream_.bad())
          {
            last_error_ = socket_stream_.last_error();
            last_error_desc_ = socket_stream_.last_error_desc();

            if(last_error_desc_.empty())
            {
              std::ostringstream ostr;            
              ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
                "read_to_buf: failed to read chunk data end marker";

              last_error_desc_ = ostr.str();
            }

            return traits_type::eof();
          }

          if(ch1 != '\r' || ch2 != '\n')
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
              "read_to_buf: while reading chunk data end marker "
              "unexpected character encountered: '"
                 << (ch1 != '\r' ? (char)ch1 : (char)ch2) << "'";

            last_error_desc_ = ostr.str();

            return traits_type::eof();
          }
        }

        setg(eback_ptr, read_ptr, read_ptr + bytes_read);

        int_type ret = *read_ptr;

        return ret & 0xFF;
      }

      bool
      Session::ChunksDecodingStreamBuf::read_trailer()
        throw(El::Exception)
      {
        std::string line;
        std::getline(socket_stream_, line, '\r');
          
        char ch = socket_stream_.get();
          
        if(socket_stream_.fail() || socket_stream_.bad())
        {
          last_error_ = socket_stream_.last_error();
          last_error_desc_ = socket_stream_.last_error_desc();

          if(last_error_desc_.empty())
          {
            std::ostringstream ostr;            
            ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
              "read_trailer: failed to read trailer";

            last_error_desc_ = ostr.str();
          }
          
          return false;
        }
        
        if(ch != '\n')
        {
          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
            "read_trailer: while reading trailer "
            "unexpected character encountered: '" << ch << "'";

          last_error_desc_ = ostr.str();

          return false;
        }

        if(line.empty())
        {
          //
          // End of trailer
          //

          return false;
        }          

        std::string::size_type len = line.find(':');

        if(len == std::string::npos)
        {
          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
            "read_trailer: character ':' not found while parsing trailer. "
            "Line:\n" << line;

          last_error_desc_ = ostr.str();

          return false;
        }

        const char* ptr = line.c_str();
        size_t pos = strspn(ptr, WHITESPACES);

        ptr += pos;
        len -= pos;

        if(!len)
        {
          std::ostringstream ostr;            
          ostr << "El::Net::HTTP::Session::ChunksDecodingStreamBuf::"
            "read_trailer: invalid trailer. Line:\n" << line;

          last_error_desc_ = ostr.str();

          return false;
        }
        
        for(pos = len - 1; pos > 0 && strchr(WHITESPACES, ptr[pos]) != 0;
            pos--);

        trailer_.push_back(Header());

        trailer_.rbegin()->name.assign(ptr, pos + 1);

        ptr += len + 1;
        ptr += strspn(ptr, WHITESPACES);
        pos = strcspn(ptr, WHITESPACES);

        trailer_.rbegin()->value.assign(ptr, pos);

        return true;
      }
      
      //
      // Session::ChunksDecodingStream
      //

      Session::ChunksDecodingStream::ChunksDecodingStream(
        Socket::Stream& socket_stream,
        HeaderList& trailer,
        size_t recv_buffer_size,
        size_t putback_buffer_size,
        Interceptor* interceptor)
        throw(Exception, El::Exception)
          : std::basic_istream<char, std::char_traits<char> >(0)
      {
        streambuf_ =
          ChunksDecodingStreamBufPtr(
            new ChunksDecodingStreamBuf(socket_stream,
                                        trailer,
                                        recv_buffer_size,
                                        putback_buffer_size,
                                        interceptor));
        
        init(streambuf_.get());
      }
      
    }
  }
}

