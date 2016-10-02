/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Apache/Request.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string>
#include <sstream>

#include <El/String/ListParser.hpp>
#include <El/Net/HTTP/URL.hpp>

#include "Request.hpp"

#include <httpd/http_core.h>

namespace El
{
  namespace Apache
  {
    //
    // Request::In class
    //
    const El::Net::HTTP::ParamList&
    Request::In::parameters(bool lax) throw(El::Exception)
    {
      if((flags_ & RIS_PARAMS_READ) == 0)
      {
        El::String::ListParser parser(request_->ap_request->args, "&");

        const char* item = 0;
        while((item = parser.next_item()) != 0)
        {
          try
          {
            parameters_.add(item);
          }
          catch(const El::Net::HTTP::InvalidArg&)
          {
            if(!lax)
            {
              throw;
            }
          }
        }

        if(request_->request_body_present_)
        {
          const apr_array_header_t* ha =
            apr_table_elts(request_->ap_request->headers_in);
          
          unsigned long count = ha->nelts;

          for (apr_table_entry_t* he = (apr_table_entry_t *)ha->elts;
               count--; he++)
          {
            if(strcasecmp(he->key, "content-type") == 0 &&
               strncasecmp(he->val, "application/x-www-form-urlencoded", 33) ==
               0)
            {
              if(request_->state() >= RPS_HEADERS_WRITING)
              {
                throw Exception("El::Apache::Request::In::parameters: "
                                "body contained parameters discarded with "
                                "headers writing");
              }
                
              El::String::ListParser parser(stream(), "&");

              const char* item = 0;
              while((item = parser.next_item()) != 0)
              {
                try
                {
                  parameters_.add(item);
                }
                catch(const El::Net::HTTP::InvalidArg&)
                {
                  if(!lax)
                  {
                    throw;
                  }
                }
              }
              
              break;
            }
          }
        }      

        flags_ |= RIS_PARAMS_READ;
      }
      
      return parameters_;
    }
    
    const El::Net::HTTP::HeaderList&
    Request::In::headers() throw(El::Exception)
    {
      if((flags_ & RIS_HEADERS_READ) == 0)
      {
        const apr_array_header_t* ha =
          apr_table_elts(request_->ap_request->headers_in);
        
        unsigned long count = ha->nelts;

        for (apr_table_entry_t* he = (apr_table_entry_t *)ha->elts;
             count--; he++)
        {
          headers_.add(he->key, he->val);
        }
        
        flags_ |= RIS_HEADERS_READ;
      }
      
      return headers_;
    }

    const El::Net::HTTP::CookieList&
    Request::In::cookies() throw(El::Exception)
    {
      if((flags_ & RIS_COOKIES_READ) == 0)
      {
        const apr_array_header_t* ha =
          apr_table_elts(request_->ap_request->headers_in);
        
        unsigned long count = ha->nelts;

        for (apr_table_entry_t* he = (apr_table_entry_t *)ha->elts;
             count--; he++)
        {
          if(strcasecmp(he->key, "cookie") == 0)
          {
            cookies_.add(he->val);
          }
        }
        
        flags_ |= RIS_COOKIES_READ;
      }
      
      return cookies_;
    }

    const El::Net::HTTP::AcceptEncodingList&
    Request::In::accept_encodings() throw(El::Exception)
    {
      if((flags_ & RIS_ACCEPT_ENCODING) == 0)
      {
        const apr_array_header_t* ha =
          apr_table_elts(request_->ap_request->headers_in);
        
        unsigned long count = ha->nelts;

        for (apr_table_entry_t* he = (apr_table_entry_t *)ha->elts;
             count--; he++)
        {
          if(strcasecmp(he->key, El::Net::HTTP::HD_ACCEPT_ENCODING) == 0)
          {
            El::String::ListParser parser(he->val, ",");
            const char* item;

            while((item = parser.next_item()) != 0)
            {
              const char* q = strchr(item, ';');

              El::Net::HTTP::AcceptEncoding accept_encoding;

              if(q)
              {
                std::string coding;
                El::String::Manip::trim(item, coding, q - item);
                
                El::String::Manip::to_lower(coding.c_str(),
                                            accept_encoding.coding);
                
                const char* qval = strchr(q, '=');

                if(qval)
                {
                  accept_encoding.qvalue = atof(qval + 1);
                }
              }
              else
              {
                std::string coding;
                El::String::Manip::trim(item, coding);
                
                El::String::Manip::to_lower(coding.c_str(),
                                            accept_encoding.coding);
              }

              El::Net::HTTP::AcceptEncodingList::iterator it =
                accept_encodings_.begin();
              
              for(; it != accept_encodings_.end() &&
                    it->qvalue >= accept_encoding.qvalue; it++);

              accept_encodings_.insert(it, accept_encoding);              
            }
          }
        }
        
        flags_ |= RIS_ACCEPT_ENCODING;
      }
      
      return accept_encodings_;
    }

    bool
    Request::In::accept_encoding(const char* coding) throw(El::Exception)
    {
      const El::Net::HTTP::AcceptEncodingList& encodings = accept_encodings();
      int others = -1;

      for(El::Net::HTTP::AcceptEncodingList::const_iterator
            i(encodings.begin()), e(encodings.end()); i != e; ++i)
      {
        if(strcasecmp(coding, i->coding.c_str()) == 0)
        {
          return i->qvalue != 0;
        }

        if(i->coding == "*")
        {
          others = i->qvalue > 0 ? 1 : 0;
        }
      }

      if(others == 0)
      {
        return false;
      }

      if(others > 0)
      {
        return true;
      }
      
      return strcasecmp(coding, "identity") == 0;
    }
    
    const El::Net::HTTP::AcceptLanguageList&
    Request::In::accept_languages() throw(El::Exception)
    {
      if((flags_ & RIS_ACCEPT_LANG) == 0)
      {
        const apr_array_header_t* ha =
          apr_table_elts(request_->ap_request->headers_in);
        
        unsigned long count = ha->nelts;

        for (apr_table_entry_t* he = (apr_table_entry_t *)ha->elts;
             count--; he++)
        {
          if(strcasecmp(he->key, El::Net::HTTP::HD_ACCEPT_LANGUAGE) == 0)
          {
            El::String::ListParser parser(he->val, ",");
            const char* item;

            while((item = parser.next_item()) != 0)
            {
              const char* q = strchr(item, ';');
              std::string lang_country;

              El::Net::HTTP::AcceptLanguage accept_lang;

              if(q)
              {
                lang_country.assign(item, q - item);
                
                const char* qval = strchr(q, '=');

                if(qval)
                {
                  accept_lang.qvalue = atof(qval + 1);
                }
              }
              else
              {
                lang_country = item;
              }

              std::string lang_s;

              std::string::size_type pos = lang_country.find('-');
              if(pos == std::string::npos)
              {
                El::String::Manip::trim(lang_country.c_str(), lang_s);
              }
              else
              {
                El::String::Manip::trim(lang_country.c_str(), lang_s, pos);
                
                El::String::Manip::trim(lang_country.c_str() + pos + 1,
                                        accept_lang.subtag);

                try
                {
                  accept_lang.country =
                    El::Country(accept_lang.subtag.c_str());
                }
                catch(const El::Country::InvalidArg& )
                {
                }                
              }

              if(lang_s != "*")
              {
                try
                {
                  accept_lang.language = El::Lang(lang_s.c_str()); 
                }
                catch(const El::Lang::InvalidArg& )
                {
                  continue;
                }
              }

              El::Net::HTTP::AcceptLanguageList::iterator it =
                accept_languages_.begin();
              
              for(; it != accept_languages_.end() &&
                    it->qvalue >= accept_lang.qvalue; it++);

              accept_languages_.insert(it, accept_lang);
            }
          }
        }
        
        flags_ |= RIS_ACCEPT_LANG;
      }
      
      return accept_languages_;
    }

    //
    // Request::Out class
    //
    void
    Request::Out::send_location(const char* value)
      throw(InvalidOperation, El::Net::URL::Exception, El::Exception)
    {
      El::Net::HTTP::URL_var url = new El::Net::HTTP::URL(value);      
      send_header(El::Net::HTTP::HD_LOCATION, url->idn_string());
    }
    
    void
    Request::Out::error_custom_response(unsigned long code,
                                        const char* text,
                                        bool redirect)
      throw(El::Exception)
    {
      if(redirect)
      {
        ap_custom_response(request_->ap_request, code, text);
      }
      else
      {
        std::string txt = std::string("\"") + text + "\"";
        ap_custom_response(request_->ap_request, code, txt.c_str());        
      }
    }
    
    void
    Request::Out::write(const char* buffer, size_t size) throw(El::Exception)
    {
      for(CallbackPtrMap::iterator i(callbacks_.begin()),
            e(callbacks_.end()); i != e; ++i)
      {
        i->second->on_write(buffer, size);
      }
    }
      
    void
    Request::Out::deflate(int level) throw(Exception, El::Exception)
    {
      if(pstream_)
      {
        std::ostringstream ostr;
        ostr << "El::Apache::Request::Out::deflate: late call; "
          "already writing body";
        
        throw Exception(ostr.str());
      }
      
      deflate_ = true;
      deflate_level_ = level;
    }
    
    void
    Request::Out::finalize() throw(El::Exception)
    {
      if(gzip_.get())
      {
        gzip_->finalize();
        gzip_.reset(0);
      }
      
      stream_.flush();
    }
    
    int
    Request::Out::stream_last_error(StreamLayer& layer) const throw()
    {
      if(!stream_.last_error_desc().empty())
      {
        layer = SL_AP;
        return stream_.last_error();
      }

      if(gzip_.get() && gzip_->stream().last_error_desc().empty())
      {
        layer = SL_GZIP;
        return gzip_->stream().last_error();        
      }

      return 0;
    }
    
    const std::string&
    Request::Out::stream_last_error_desc() const throw(El::Exception)
    {
      if(!stream_.last_error_desc().empty())
      {
        return stream_.last_error_desc();
      }

      if(gzip_.get() && gzip_->stream().last_error_desc().empty())
      {
        return gzip_->stream().last_error_desc();        
      }

      return empty_str_;
    }
    
    //
    // Request class
    //
    void
    Request::start_reading() throw(El::Exception)
    {
      if(state_ != RPS_INPUT_HEADERS_PARSED)
      {
        return;
      }
      
      int status = ap_setup_client_block(ap_request, REQUEST_CHUNKED_DECHUNK);
      
      if(status != OK)
      {
        char buff[1024];

        std::ostringstream ostr;
        ostr << "El::Apache::Request::start_reading: "
          "ap_setup_client_block failed. Code " << status << ". Reason: "
             << apr_strerror(status, buff, sizeof(buff));

        throw Exception(ostr.str());
      }

      request_body_present_ = ap_should_client_block(ap_request) != 0;
      state_ = RPS_INPUT_READING;
    }    
    
    void
    Request::start_writing_headers() throw(El::Exception)
    {
      if(state_ < RPS_INPUT_READING)
      {
        start_reading();
      }

      if(state_ != RPS_INPUT_READING)
      {
        return;
      }

      int status = ap_discard_request_body(ap_request);
      
      if(status != OK)
      {
        char buff[1024];

        std::ostringstream ostr;
        ostr << "El::Apache::Request::start_writing_headers: "
          "ap_discard_request_body failed. Code " << status << ". Reason: "
             << apr_strerror(status, buff, sizeof(buff));

        throw Exception(ostr.str());
      }

      const std::string& error = in().stream().last_error_desc();
      
      if(!error.empty())
      {
        std::ostringstream ostr;
        ostr << "El::Apache::Request::start_writing_headers: "
          "request data reading failed. Reason: " << error;

        throw Exception(ostr.str());
      }
      
      in().stream().discard();
      state_ = RPS_HEADERS_WRITING;
    }

    void
    Request::start_writing_body() throw(El::Exception)
    {
      if(state_ < RPS_HEADERS_WRITING)
      {
        start_writing_headers();
      }

      if(state_ != RPS_HEADERS_WRITING)
      {
        return;
      }

      state_ = RPS_BODY_WRITING;
    }

    void
    Request::finalize(int handler_result)
      throw(Exception, El::Exception)
    {  
      if(handler_result == DECLINED)
      {        
        if(state_ >= RPS_INPUT_READING)
        {
          throw Exception("El::Apache::Request::finalize: "
                          "request declined after started to read input");
        }
      }
      else
      {
        if(state_ == RPS_INPUT_HEADERS_PARSED)
        {
          start_reading();
        }

        if(state_ == RPS_INPUT_READING)
        {
          start_writing_headers();
        }

        out().finalize();

        const std::string& error = out().stream_last_error_desc();
      
        if(!error.empty())
        {
          std::ostringstream ostr;
          ostr << "El::Apache::Request::finalize: "
            "last response data writing failed. Reason: " << error;

          throw Exception(ostr.str());
        }
        
        for(CallbackMap::iterator i(callbacks_.begin()), e(callbacks_.end());
            i != e; ++i)
        {
          i->second->finalize(handler_result);
        }

        clear_callbacks();
      }
    }

    bool
    Request::deflate(int level) throw(Exception, El::Exception)
    {
      if(state_ > RPS_HEADERS_WRITING)
      {
        std::ostringstream ostr;
        ostr << "El::Apache::Request::deflate: late call; state " << state_;
        throw Exception(ostr.str());
      }

      if(in().accept_encoding("gzip"))
      {
        // Do not use out() here as it change request state
        // discarding the body
        out_.deflate(level);
        return true;
      }

      return false;
    }

  }
}
