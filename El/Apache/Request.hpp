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
 * @file Elements/El/Apache/Request.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_APACHE_REQUEST_HPP_
#define _ELEMENTS_EL_APACHE_REQUEST_HPP_

#include <string>
#include <memory>
#include <ext/hash_map>

#include <ace/OS.h>

#include <apr_strings.h>
#include <httpd/http_protocol.h>
#include <httpd/ap_release.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/Compress/GZip.hpp>

#include <El/Net/URL.hpp>
#include <El/Net/HTTP/Params.hpp>
#include <El/Net/HTTP/Cookies.hpp>
#include <El/Net/HTTP/Headers.hpp>

#include <El/Apache/Exception.hpp>
#include <El/Apache/Stream.hpp>

namespace El
{
  namespace Apache
  {
    struct Request
    {
      EL_EXCEPTION(Exception, El::Apache::Exception);
      EL_EXCEPTION(InvalidOperation, Exception);

      struct Callback : public virtual El::RefCount::Interface
      {
        virtual void content_type(const char* value) throw(El::Exception) = 0;

        virtual void send_header(const char* name, const char* value)
          throw(El::Exception) = 0;

        virtual void start_writing(bool deflate) throw(El::Exception) = 0;
        
        virtual void on_write(const char* buffer, size_t size)
          throw(El::Exception) = 0;

        virtual void pre_ap_rwrite() throw() = 0;
        virtual void post_ap_rwrite() throw() = 0;
        
        virtual void finalize(int result) throw(El::Exception) = 0;
      };

      typedef El::RefCount::SmartPtr<Callback> Callback_var;
      
      typedef __gnu_cxx::hash_map<uint32_t, Callback_var> CallbackMap;
      typedef __gnu_cxx::hash_map<uint32_t, Callback*> CallbackPtrMap;

      struct In
      {          
        const El::Net::HTTP::ParamList& parameters(bool lax = false)
          throw(El::Exception);
        
        const El::Net::HTTP::HeaderList& headers() throw(El::Exception);
        const El::Net::HTTP::CookieList& cookies() throw(El::Exception);

        const El::Net::HTTP::AcceptLanguageList& accept_languages()
          throw(El::Exception);

        const El::Net::HTTP::AcceptEncodingList& accept_encodings()
          throw(El::Exception);

        bool accept_encoding(const char* coding) throw(El::Exception);

        InStream& stream() throw();
          
      private:

        friend class Request;

        In(Request* req) throw();

      private:

        enum RequestInputState
        {
          RIS_PARAMS_READ = 0x1,
          RIS_HEADERS_READ = 0x2,
          RIS_COOKIES_READ = 0x4,
          RIS_ACCEPT_LANG = 0x8,
          RIS_ACCEPT_ENCODING = 0x10
        };
          
        unsigned long flags_;
        Request* request_;
          
        El::Net::HTTP::ParamList parameters_;
        El::Net::HTTP::HeaderList headers_;
        El::Net::HTTP::CookieList cookies_;
        El::Net::HTTP::AcceptLanguageList accept_languages_;
        El::Net::HTTP::AcceptEncodingList accept_encodings_;
          
        InStream stream_;
      };

      struct Out : public OutStreamBuf::Callback
      {
        virtual ~Out() throw();

        void callback(El::Apache::Request::Callback* value,
                      unsigned long id) throw();
          
        std::ostream& stream() throw();
        bool stream_created() const throw();

        enum StreamLayer
        {
          SL_NONE,
          SL_AP,
          SL_GZIP
        };

        int stream_last_error(StreamLayer& layer) const throw();
        const std::string& stream_last_error_desc() const throw(El::Exception);
        
        void content_type(const char* ct)
          throw(InvalidOperation, El::Exception);

        void send_header(const char* name, const char* value)
          throw(InvalidOperation, El::Exception);

        void send_cookie(const El::Net::HTTP::CookieSetter& cookie)
          throw(InvalidOperation, El::Exception);

        void send_location(const char* value)
          throw(InvalidOperation, El::Net::URL::Exception, El::Exception);

        void error_custom_response(unsigned long code,
                                   const char* text,
                                   bool redirect = false)
          throw(El::Exception);

        void deflate(int level) throw(Exception, El::Exception);
        void finalize() throw(El::Exception);
        void discard() throw(El::Exception);
        
      private:
        
        virtual void start_writing() throw();

        virtual void write(const char* buffer, size_t size)
          throw(El::Exception);

        virtual void pre_ap_rwrite() throw();
        virtual void post_ap_rwrite() throw();        

      private:

        friend class Request;

        Out(Request* req) throw();

      private:
        Request* request_;
        OutStream stream_;
        El::Apache::Request::CallbackPtrMap callbacks_;
        std::auto_ptr<El::Compress::ZLib::OutStreamWriter> stream_writer_;
        std::auto_ptr<El::Compress::ZLib::GZip> gzip_;
        bool deflate_;
        int deflate_level_;
        std::ostream* pstream_;
        std::string empty_str_;
      };

      request_rec* ap_request;
        
      Request(request_rec* ap_req) throw();
      ~Request() throw();

      bool deflate(int level) throw(Exception, El::Exception);

      void callback(Callback* value, unsigned long id) throw();
      Callback* callback(unsigned long id) throw();

      ACE_Time_Value time() const throw();

      const char* unparsed_uri() const throw();
      const char* uri() const throw();
      const char* args() const throw();

      const char* remote_ip() const throw();
        
      const char* host() throw(El::Exception);
      unsigned short port() throw(El::Exception);
      const char* endpoint() throw(El::Exception);

      const char* method() const throw();

      // M_GET, M_POST, etc. For the full list look into httpd/httpd.h.
      int method_number() const throw();

      bool secure() const throw();

      enum RequestProcessingState
      {
        RPS_INPUT_HEADERS_PARSED,
        RPS_INPUT_READING,
        RPS_HEADERS_WRITING,
        RPS_BODY_WRITING
      };
        
      RequestProcessingState state() const throw();

      //
      // Prepares to read requst input, sets RPS_INPUT_READING state.
      // After this method called request handler can not
      // return DECLINED status.
      //
      void start_reading() throw(El::Exception);

      //
      // Prepares to write headers, sets RPS_HEADERS_WRITING state.
      // Implicitly calls start_reading if in RPS_INPUT_HEADERS_PARSED state.
      // After this method called request handler can not
      // return DECLINED status.
      //
      void start_writing_headers() throw(El::Exception);

      //
      // Prepares to write body, sets RPS_BODY_WRITING state.
      // Implicitly calls writing_headers if in state "lower"
      // than RPS_HEADERS_WRITING.
      // Implicitly called when first byte of response body is written.
      // After this method called request handler can not
      // return DECLINED status.
      //
      void start_writing_body() throw(El::Exception);

      //
      // Implicitly calls start_reading if in RPS_INPUT_HEADERS_PARSED state.
      // After this method called request handler can not
      // return DECLINED status.
      //
      In& in() throw(El::Exception);

      //
      Out& out() throw(El::Exception);

    private:
      friend class In;
      friend class Out;
      template<typename CONFIG> friend class Module;

      void calc_endpoint() throw(El::Exception);
      void finalize(int handler_result) throw(Exception, El::Exception);
      void discard() throw(El::Exception);

      void clear_callbacks() throw();

      enum RequestFlags
      {
        RF_ENDPOINT_CALCULATED = 0x1
      };
          
      unsigned long flags_;
      
      In in_;
      Out out_;
      
      ACE_Time_Value time_;
      unsigned short port_;
      std::string host_;
      std::string endpoint_;
      RequestProcessingState state_;
      bool request_body_present_;
      CallbackMap callbacks_;

    private:
      Request(const Request&);
      void operator=(const Request&);
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
    // Request::Out class
    //

    inline
    Request::Out::Out(Request* req) throw()
        :  request_(req),
           stream_(req->ap_request, this),
           deflate_(false),
           deflate_level_(0),
           pstream_(0)
    {
    }

    inline
    Request::Out::~Out() throw()
    {
    }

    inline
    bool
    Request::Out::stream_created() const throw()
    {
      return pstream_ != 0;
    }
    
    inline
    std::ostream&
    Request::Out::stream() throw()
    {
      if(pstream_ == 0)
      {
        if(deflate_)
        {
          send_header(El::Net::HTTP::HD_CONTENT_ENCODING, "gzip");
          
          stream_writer_.reset(
            new El::Compress::ZLib::OutStreamWriter(stream_));
          
          gzip_.reset(new El::Compress::ZLib::GZip(stream_writer_.get(),
                                                   deflate_level_));
          pstream_ = &gzip_->stream();
        }
        else
        {
          pstream_ = &stream_;
        }
      }

      return *pstream_; 
    }

    inline
    void
    Request::Out::discard() throw(El::Exception)
    {  
      gzip_.reset(0);
      callbacks_.clear();
      stream_.setstate(std::ios_base::badbit | std::ios_base::failbit);
    }

    inline
    void
    Request::Out::start_writing() throw()
    {
      for(CallbackPtrMap::iterator i(callbacks_.begin()), e(callbacks_.end());
          i != e; ++i)
      {
        i->second->start_writing(deflate_);
      }
      
      if(request_->state() < RPS_BODY_WRITING)
      {
        request_->start_writing_body();
      }
    }

    inline
    void
    Request::Out::pre_ap_rwrite() throw()
    {
      for(CallbackPtrMap::iterator i(callbacks_.begin()), e(callbacks_.end());
          i != e; ++i)
      {
        i->second->pre_ap_rwrite();
      }
    }
    
    inline
    void
    Request::Out::post_ap_rwrite() throw()
    {
      for(CallbackPtrMap::iterator i(callbacks_.begin()), e(callbacks_.end());
          i != e; ++i)
      {
        i->second->post_ap_rwrite();
      }
    }
    
    inline
    void
    Request::Out::content_type(const char* ct)
      throw(InvalidOperation, El::Exception)
    {
      if(request_->state() >= RPS_BODY_WRITING)
      {
        throw InvalidOperation(
          "content type to be set prior to writing output");
      }
      
      ap_set_content_type(
        request_->ap_request,
        ct ? apr_pstrdup(request_->ap_request->pool, ct) : 0);

      for(CallbackPtrMap::iterator i(callbacks_.begin()), e(callbacks_.end());
          i != e; ++i)
      {
        i->second->content_type(ct);
      }
    }

    inline
    void
    Request::Out::send_header(const char* name, const char* value)
      throw(InvalidOperation, El::Exception)
    {
      if(request_->state() >= RPS_BODY_WRITING)
      {
        throw InvalidOperation("headers to be sent prior to writing body");
      }
      
      if (!ACE_OS::strcasecmp(name, El::Net::HTTP::HD_CONTENT_TYPE))
      {
        content_type(value);
      }
      else
      {
        apr_table_add(request_->ap_request->err_headers_out, name, value);
        
        for(CallbackPtrMap::iterator i(callbacks_.begin()),
              e(callbacks_.end()); i != e; ++i)
        {
          i->second->send_header(name, value);
        }
      }
    }

    inline
    void
    Request::Out::send_cookie(
      const El::Net::HTTP::CookieSetter& cookie)
      throw(InvalidOperation, El::Exception)
    {
      send_header("Set-Cookie", cookie.string().c_str());
    }
    
    inline
    void
    Request::Out::callback(El::Apache::Request::Callback* value,
                           unsigned long id) throw()
    {
      if(value)
      {
        callbacks_[id] = value;
      }
      else
      {
        callbacks_.erase(id);
      }
    }
    
    //
    // Request::In class
    //
    
    inline
    Request::In::In(Request* req) throw()
        :  flags_(0),
           request_(req),
           stream_(req->ap_request)
    {
    }
    
    inline
    InStream&
    Request::In::stream() throw()
    {
      return stream_;
    }
    
    //
    // Request class
    //
    
    inline
    Request::Request(request_rec* ap_req) throw()
        : ap_request(ap_req),
          flags_(0),
          in_(this),
          out_(this),
          time_(ap_req->request_time/1000000, ap_req->request_time%1000000),
          port_(0),
          state_(RPS_INPUT_HEADERS_PARSED),
          request_body_present_(false)
    {
    }

    inline
    Request::~Request() throw()
    {
      clear_callbacks();
    }

    inline
    void
    Request::clear_callbacks() throw()
    {
      for(CallbackMap::iterator i(callbacks_.begin()), e(callbacks_.end());
          i != e; ++i)
      {
        out_.callback(0, i->first);
      }

      callbacks_.clear();
    }
    
    inline
    void
    Request::callback(Callback* value, unsigned long id) throw()
    {
      Callback_var tmp;
      CallbackMap::iterator i = callbacks_.find(id);

      if(i != callbacks_.end())
      { 
        tmp = callbacks_[id].retn();
      }
      
      if(value)
      {
        callbacks_[id] = El::RefCount::add_ref(value);
      }
      else
      {
        callbacks_.erase(id);
      }
      
      out_.callback(value, id);
    }

    inline
    Request::Callback*
    Request::callback(unsigned long id) throw()
    {
      CallbackMap::iterator i = callbacks_.find(id);
      return i == callbacks_.end() ? 0 : i->second.in();
    }
    
    inline
    ACE_Time_Value
    Request::time() const throw()
    {
      return time_;
    }

    inline
    Request::RequestProcessingState
    Request::state() const throw()
    {
      return state_;
    }
    
    inline
    Request::In&
    Request::in() throw(El::Exception)
    {
      if(state_ == RPS_INPUT_HEADERS_PARSED)
      {
        start_reading();
      }

      return in_;
    }

    inline
    Request::Out&
    Request::out() throw(El::Exception)
    {
      if(state_ < RPS_HEADERS_WRITING)
      {
        start_writing_headers();
      }

      return out_;
    }

    inline
    void
    Request::discard() throw(El::Exception)
    {  
      clear_callbacks();

      if(state_ == RPS_INPUT_HEADERS_PARSED)
      {
        return;
      }

      ap_discard_request_body(ap_request);

      if(state_ == RPS_INPUT_READING)
      {
        return;
      }

      out_.discard();
    }
    
    inline
    bool
    Request::secure() const throw()
    {
      bool secure = false;
      
      for(ap_filter_t *f = ap_request->connection->output_filters;
          f && !secure; f = f->next)
      {
        secure = f->frec && f->frec->name &&
          strncasecmp(f->frec->name, "ssl/tls", 7) == 0;
      }

      return secure;
    }
    
    inline
    const char*
    Request::unparsed_uri() const throw()
    {
      return ap_request->unparsed_uri;
    }

    inline
    const char*
    Request::uri() const throw()
    {
      return ap_request->uri;
    }

    inline
    const char*
    Request::args() const throw()
    {
      return ap_request->args ? ap_request->args : "";
    }

    inline
    const char*
    Request::remote_ip() const throw()
    {
#     if EL_APACHE_VERSION < 24
        return ap_request->connection->remote_ip;
#     else
        return ap_request->connection->client_ip; 
#     endif
    }
        
    inline
    const char*
    Request::method() const throw()
    {
      return ap_request->method;
    }

    inline
    int
    Request::method_number() const throw()
    {
      return ap_request->method_number;
    }

    inline
    void
    Request::calc_endpoint() throw(El::Exception)
    {
      if(flags_ & RF_ENDPOINT_CALCULATED)
      {
        return;
      }
      
      const apr_array_header_t* ha = apr_table_elts(ap_request->headers_in);
      unsigned long count = ha->nelts;
      
      for (apr_table_entry_t* he = (apr_table_entry_t *)ha->elts;
           count--; ++he)
      {
        if(strcasecmp(he->key, El::Net::HTTP::HD_HOST) == 0)
        {
          const char* port = strchr(he->val, ':');

          if(port == 0)
          {
            host_ = he->val;

            if(!host_.empty())
            {
              port_ = secure() ? DEFAULT_HTTPS_PORT : DEFAULT_HTTP_PORT;
            }
          }
          else
          {
            unsigned short port_val = 0;
            
            if(El::String::Manip::numeric(port + 1, port_val) && port_val)
            {
              host_.assign(he->val, port - he->val);

              if(!host_.empty())
              {
                port_ = port_val;
              }
            }
          }
          
          break;
        }
      }

      if(host_.empty() && ap_request->hostname)
      {
        host_ = ap_request->hostname;
      }

      if(host_.empty() && ap_request->server->server_hostname)
      {
        host_ = ap_request->server->server_hostname;
      }
        
      if(port_ == 0)
      {
        port_ = ap_request->server->port;
      }

      if(port_ && !host_.empty())
      {
        std::ostringstream ostr;
        ostr << host_;

        if(port_ != (secure() ? DEFAULT_HTTPS_PORT : DEFAULT_HTTP_PORT))
        {
          ostr << ":" << port_;
        }
        
        endpoint_ = ostr.str();
      }
      
      flags_ |= RF_ENDPOINT_CALCULATED;
    }

    inline
    unsigned short
    Request::port() throw(El::Exception)
    {
      calc_endpoint();
      return port_;
    }
    
    inline
    const char*
    Request::host() throw(El::Exception)
    {
      calc_endpoint();      
      return host_.c_str();
    }

    inline
    const char*
    Request::endpoint() throw(El::Exception)
    {
      calc_endpoint();      
      return endpoint_.c_str();
    }

  }
}

#endif // _ELEMENTS_EL_APACHE_REQUEST_HPP_
