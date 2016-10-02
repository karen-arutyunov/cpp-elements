/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Template.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>
#include <string>
#include <iostream>

#include <El/Exception.hpp>
#include <El/Lang.hpp>
#include <El/String/Template.hpp>
#include <El/Apache/Request.hpp>
#include <El/Net/HTTP/Utility.hpp>
#include <El/Net/HTTP/Headers.hpp>
#include <El/Cache/EncodingAwareLocalizedTemplate.hpp>

#include "Template.hpp"

#include <httpd/http_core.h>

namespace El
{
  namespace PSP
  {
    class RuntimeVariables : public Cache::EncodableVariables
    {
    public:
      RuntimeVariables(El::Apache::Request& request,
                       const ArgArrayMap& options,
                       const El::Lang& lang) throw();
      
      virtual ~RuntimeVariables() throw() {}
      
    private:
      
      virtual bool write(const El::String::Template::Chunk& chunk,
                         std::ostream& output) const
        throw(El::Exception);
      
    private:
      El::Apache::Request& request_;
      const ArgArrayMap& options_;
      const El::Lang lang_;
    };
    
    //
    // El::PSP::TemplateCache class
    //
    int
    TemplateCache::run(const char* template_path,
                       El::Apache::Request& request,
                       const ArgArrayMap& options,
                       const El::Lang& lang,
                       ETagCalc etag_calc)
      throw(El::Cache::Exception, El::Exception)
    {
      Cache::LocalizationInstantiationInterceptor
        localization_interceptor(template_cache_.var_left_marker(),
                                 template_cache_.var_right_marker());

      El::Cache::TextTemplateFile_var localized_template =
        template_cache_.get(template_path, lang, &localization_interceptor);

      if(etag_calc == ETC_STATIC)
      {
        unsigned long long hash = localized_template->hash();

        std::string hash_b64;
      
        El::String::Manip::base64_encode((const unsigned char*)&hash,
                                         sizeof(hash),
                                         hash_b64);
        
        const char* current_etag =
          request.in().headers().find(El::Net::HTTP::HD_IF_NONE_MATCH);
        
        request.out().send_header(El::Net::HTTP::HD_ETAG, hash_b64.c_str());

        if(current_etag && hash_b64 == current_etag)
        {
          return HTTP_NOT_MODIFIED;
        }
      }

      RuntimeVariables vars(request, options, lang);      
      localized_template->instantiate(vars, request.out().stream());

      return OK;
    }
    
    //
    // RuntimeVariables class
    //
    RuntimeVariables::RuntimeVariables(
      El::Apache::Request& request,
      const ArgArrayMap& options,
      const El::Lang& lang) throw()
        : request_(request),
          options_(options),
          lang_(lang)
    {
    }

    bool
    RuntimeVariables::write(const El::String::Template::Chunk& chunk,
                            std::ostream& output) const
      throw(El::Exception)
    {
      const char* var_name = chunk.text.c_str();
      
      if(strcmp(var_name, "lang") == 0)
      {
        return encode(lang_.l3_code(), output, chunk);
      }
      else if(strncmp(var_name, "param ", 6) == 0)
      {
        return encode(request_.in().parameters().find(var_name + 6),
                      output,
                      chunk);
      }
      else if(strncmp(var_name, "conf ", 5) == 0)
      {
        ArgArrayMap::const_iterator it = options_.find(var_name + 5);

        if(it != options_.end())
        {
          const Directive::ArgArray& vals = it->second;

          if(!vals.empty())
          {
            return encode(vals[0].string(), output, chunk);
          }
        }
      }
      else if(strncmp(var_name, "browser", 7) == 0)
      {
        const char* ua =
          request_.in().headers().find(El::Net::HTTP::HD_USER_AGENT);
        
        return encode(El::Net::HTTP::browser(ua), output, chunk);
      }
      else if(strncmp(var_name, "os", 2) == 0)
      {
        const char* ua =
          request_.in().headers().find(El::Net::HTTP::HD_USER_AGENT);
        
        return encode(El::Net::HTTP::os(ua), output, chunk);
      }
      else if(strncmp(var_name, "header ", 7) == 0)
      {
        return encode(request_.in().headers().find(var_name + 7),
                      output,
                      chunk);
      }
      else if(strncmp(var_name, "cookie ", 7) == 0)
      {
        return encode(request_.in().cookies().most_specific(var_name + 7),
                      output,
                      chunk);
      }
      else if(*var_name == '\0')
      {
        return encode("<?", output, chunk);        
      }
      
      return false;
    }
    
    //
    // TemplateCache::TemplateParseInterceptor class
    //
    void
    TemplateCache::TemplateParseInterceptor::update(
      const std::string& tag,
      const std::string& value,
      El::String::Template::Chunk& chunk) const
      throw(El::String::Template::ParsingFailed,
            El::String::Template::Exception,
            El::Exception)
    {
      if(tag == "lang")
      {
        chunk.text = tag;
      }
      else if(tag == "param")
      {
        chunk.text = std::string("param ") + value;
      }
      else if(tag == "conf")
      {
        chunk.text = std::string("conf ") + value;
      }
      else if(tag == "browser" || tag == "os")
      {
        chunk.text = tag;
      }
      else if(tag == "header")
      {
        chunk.text = std::string("header ") + value;
      }
      else if(tag == "cookie")
      {
        chunk.text = std::string("cookie ") + value;
      }
      else
      {
        std::ostringstream ostr;
        ostr << "El::PSP::TemplateCache::TemplateParseInterceptor::update: "
          "not valid PSP tag '" << chunk.text << "'";

        throw El::String::Template::ParsingFailed(ostr.str());
      }
    }

  }
}
