/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/PSP/PSPModule.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <Python.h>

#include <locale.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <sstream>
#include <utility>

#include <El/Exception.hpp>
#include <El/Lang.hpp>
#include <El/NameValueMap.hpp>
#include <El/Logging/StreamLogger.hpp>
#include <El/Localization/Loc.hpp>
#include <El/Net/HTTP/Utility.hpp>
#include <El/Net/HTTP/Headers.hpp>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/Logger.hpp>
#include <El/Net/HTTP/Python/Utility.hpp>
#include <El/PSP/Config.hpp>
/*
// SHIT
#include <apr_errno.h>
#include <apr_strings.h>
#include <httpd/httpd.h>
#include <httpd/http_config.h>
#include <httpd/http_protocol.h>
#include <httpd/http_log.h>

// SHIT
#ifdef APLOG_USE_MODULE
APLOG_USE_MODULE(psp);
#endif
*/
#include "PSPModule.hpp"

EL_APACHE_MODULE_INSTANCE(El::PSP::Module, psp_module);

namespace Aspect
{
  const char MODULE[] = "PSP_Module";
  const char CACHE[] = "PSP_Cache";
}

namespace El
{
  namespace PSP
  {
    //
    // El::PSP::Module class
    //
    Module::Module(El::Apache::ModuleRef& mod_ref) throw(El::Exception)
        : El::Apache::Module<ModuleConfig>(mod_ref,
                                           false,
                                           APR_HOOK_FIRST),
          cache_trace_enabled_(false)
    {
      register_directive("PSP_RootDir",
                         "string:nws",
                         "PSP_RootDir <root directory>.",
                         1,
                         1);

      register_directive("PSP_Config",
                         "string:nws;string;...",
                         "PSP_Config <option> <string value>+.",
                         0,
                         LONG_MAX);

      register_directive("PSP_Object",
                         "string:nws;string:nws;string;string",
                         "PSP_Object <var name> <factory function> <object cleanup function> <config key>.",
                         0,
                         LONG_MAX);

      register_directive("PSP_SearchPath",
                         "string:nws",
                         "PSP_SearchPath <search path>.",
                         0,
                         LONG_MAX);

      register_directive("PSP_ErrorInResponse",
                         "numeric:0,1",
                         "PSP_ErrorInResponse <if 1, then page generating "
                         "error will be printed into the HTTP response; "
                         "default 0>",
                         0,
                         1);
      
      register_directive("PSP_LangCalc",
                         "string:nws;...",
                         "PSP_LangCalc (param=<param name>|url|"
                         "preference=<cookie name:pref name>|"
                         "cookie=<cookie name>|header)+|none.",
                         0,
                         1);

      register_directive(
        "PSP_LangValid",
        "string:nws;...",
        "PSP_LangValid (<l3 language code>|'*'):<l3 language code>+.",
        0,
        1);
      
      register_directive("PSP_SubPathHandling",
                         "string:nws;...",
                         "PSP_SubPathHandling (<subpath>)+|none.",
                         0,
                         LONG_MAX);

      register_directive("PSP_ETag",
                         "string:nws",
                         "PSP_ETag static|none.",
                         0,
                         1);

      register_directive("PSP_Localization",
                         "string:nws",
                         "PSP_Localization <localization directory path>.",
                         0,
                         1);

      register_directive("PSP_LogLevel",
                         "numeric:0",
                         "PSP_LogLevel <unsigned long>",
                         0,
                         1);      

      register_directive("PSP_LogAspects",
                         "string",
                         "PSP_LogAspects <comma-separated aspects or *>.",
                         0,
                         1);

      register_directive("PSP_CacheEnabled",
                         "numeric:0,1",
                         "PSP_CacheEnabled <0|1>",
                         0,
                         1);      

      register_directive("PSP_CacheTraceEnabled",
                         "numeric:0,1",
                         "PSP_CacheTraceEnabled <0|1>",
                         0,
                         1);      

      register_directive("PSP_CacheEntryTimeout",
                         "numeric:0,max",
                         "PSP_CacheEntryTimeout <seconds>",
                         0,
                         1);      

      register_directive("PSP_CacheEntryTimeoutDelay",
                         "numeric:0,max",
                         "PSP_CacheEntryTimeoutDelay <seconds>",
                         0,
                         1);

      register_directive("PSP_CacheEntryUnusedTimeout",
                         "numeric:0,max",
                         "PSP_CacheEntryUnusedTimeout <seconds>",
                         0,
                         1);
      
      register_directive("PSP_CacheEntryUnusedCheckPeriod",
                         "numeric:0,max",
                         "PSP_CacheEntryUnusedCheckPeriod <seconds>",
                         0,
                         1);

      register_directive("PSP_CacheKey",
                         "string:nws",
                         "PSP_CacheKey uri|uri_crawler|none.",
                         0,
                         1);
      
      register_directive("PSP_CacheCondition",
                         "string:nws;...",
                         "PSP_CacheCondition reset|p:<param name>(=|!=)<param value>.",
                         0,
                         LONG_MAX);
      
      register_directive("PSP_DeflateLevel",
                         "numeric:-1,9",
                         "PSP_DeflateLevel <level>; -1 default, "
                           "0 no compression, "
                           "1-9 bigger value - more compression",
                         0,
                         LONG_MAX);
      
      register_directive("PSP_CanonicalEndpoint",
                         "string",
                         "PSP_CanonicalEndpoint <host[:port]>.",
                         0,
                         1);

      register_handler("psp-script");
      register_handler("psp-template");
    }
 
    void
    Module::directive(const El::Directive& directive, Config& config)
      throw(El::Exception)
    {
      const El::Directive::Arg& arg = directive.arguments[0];
      const El::Directive::ArgArray& args = directive.arguments;
    
      const std::string& dname = directive.name;

      if(dname == "PSP_Config")
      {
        ArgArrayMap::iterator it = config.options.insert(
          std::make_pair(arg.string(), Directive::ArgArray())).first;

        it->second.assign(args.begin() + 1, args.end());        
      }
      else if(dname == "PSP_Object")
      {
        Config::ObjectInfoMap::iterator it = config.objects.insert(
          std::make_pair(arg.string(), Config::ObjectInfo())).first;

        std::string factory = args[1].string();

        if(strcasecmp(factory.c_str(), "reset"))
        {
          it->second.factory_function = factory;
        }
        
        if(args.size() > 2)
        {
          it->second.cleanup_function = args[2].string();
        }

        if(args.size() > 3)
        {
          it->second.config_key = args[3].string();
        }
      }
      else if(dname == "PSP_SearchPath")
      {
        config.search_paths.insert(arg.string());
      }
      else if(dname == "PSP_RootDir")
      {
        std::string& root = config.root;
        
        root = arg.string();

        size_t len = root.length();
        
        if(len && root[len - 1] == '/')
        {
          root.resize(len - 1);
        }
      }
      else if(dname == "PSP_ErrorInResponse")
      {
        config.error_in_response = arg.numeric();
      }
      else if(dname == "PSP_LangCalc")
      {
        config.parse_calc_lang(args);
      }
      else if(dname == "PSP_LangValid")
      {
        config.parse_valid_lang(args);
      }
      else if(dname == "PSP_SubPathHandling")
      {
        config.parse_subpath_handling(args);
      }
      else if(dname == "PSP_ETag")
      {
        if(strcasecmp(arg.string(), "none") == 0)
        {
          config.etag_calc = ETC_NONE;
        }
        else if(strcasecmp(arg.string(), "static") == 0)
        {
          config.etag_calc = ETC_STATIC;
        }
        else
        {
          std::ostringstream ostr;
          ostr << "Invalid value '" << arg.string()
               << "' for PSP_ETag option";

          throw Exception(ostr.str());
        }
      }
      else if(dname == "PSP_Localization")
      {
        config.localization = arg.string();
      }
      else if(dname == "PSP_LogLevel")
      {
        config.log_level = arg.numeric();
      }
      else if(dname == "PSP_LogAspects")
      {
        config.log_aspects = arg.string();
      }
      else if(dname == "PSP_CacheKey")
      {
        if(strcasecmp(arg.string(), "uri") == 0)
        {
          config.rcache_key_type = RequestCache::CKT_URI;
        }
        else if(strcasecmp(arg.string(), "uri_crawler") == 0)
        {
          config.rcache_key_type = RequestCache::CKT_URI_CRAWLER;
        }
        else if(strcasecmp(arg.string(), "none") == 0)
        {
          config.rcache_key_type = RequestCache::CKT_NONE;  
        }
        else
        {
          std::ostringstream ostr;
          ostr << "Invalid value '" << arg.string()
               << "' for PSP_CacheKey option";

          throw Exception(ostr.str());
        }
      }
      else if(dname == "PSP_CacheCondition")
      {
        RequestCache::CondKey key;
        RequestCache::CondValue value;
        
        const char* ptr = arg.string();

        if(strcasecmp(ptr, "reset") == 0)
        {
          key.type = RequestCache::CT_RESET;
        }
        else if(strncmp(ptr, "p:", 2) == 0)
        {
          ptr += 2;
          const char* eq = strchr(ptr, '=');

          if(eq != 0 && eq > ptr)
          {
            const char* pval = eq + 1;
            
            if(*(eq - 1) == '!')
            {
              value.op = RequestCache::CO_NE;
              --eq;
            }

            El::String::Manip::trim(ptr, key.value, eq - ptr);

            if(!key.value.empty())
            {
              key.type = RequestCache::CT_PARAM;
              El::String::Manip::trim(pval, value.value);
            }
          }
        }

        if(key.type == RequestCache::CT_UNDEFINED)
        {
          std::ostringstream ostr;
          ostr << "Invalid value '" << arg.string()
               << "' for PSP_CacheCondition option";

          throw Exception(ostr.str());
        }

        config.rcache_conditions[key] = value;
      }
      else if(dname == "PSP_CacheEntryTimeout")
      {
        config.entry_timeout = arg.numeric();
      }
      else if(dname == "PSP_CacheEntryTimeoutDelay")
      {
        config.entry_timeout_delay = arg.numeric();
      }
      else if(dname == "PSP_CacheEntryUnusedTimeout")
      {
        config.entry_unused_timeout = arg.numeric();
      }
      else if(dname == "PSP_CacheEntryUnusedCheckPeriod")
      {
        config.entry_unused_check_period = arg.numeric();
      }
      else if(dname == "PSP_CacheEnabled")
      {
        config.cache_enabled = arg.numeric() != 0;
      }
      else if(dname == "PSP_CacheTraceEnabled")
      {
        config.cache_trace_enabled = arg.numeric() != 0;
      }
      else if(dname == "PSP_DeflateLevel")
      {
        config.deflate_level = arg.numeric();
      }
      else if(dname == "PSP_CanonicalEndpoint")
      {
        config.canonical_endpoint = arg.string();
      }
    }

    void
    Module::child_init() throw(El::Exception)
    {
      {
        El::Python::EnsureCurrentThread guard;

        PyObject* module = PyImport_AddModule("sys");

        if(module == 0)
        {
          El::Python::handle_error(
            "Module::child_init: PyImport_AddModule failed");
        }

        El::Python::Object_var paths = PyObject_GetAttrString(module, "path");
        
        if(paths.in() == 0)
        {
          El::Python::handle_error(
            "Module::child_init: PyObject_GetAttrString failed");
        }

        if(!PyList_Check(paths.in()))
        {
          throw Exception("Module::child_init: sys.path not a list");
        }

        for(Config::StringSet::const_iterator it = search_paths_.begin();
            it != search_paths_.end(); it++)
        {
          El::Python::Object_var path = PyString_FromString(it->c_str());
          
          PyList_Append(paths.in(), path.in());
        }
      }
      
      logger()->info(
        "El::PSP::Module::child_init: module initialized ...",
        Aspect::MODULE);
    }
    
    void
    Module::child_init(Config& conf) throw(El::Exception)
    {
//      std::cerr << "child_init root: " << conf.root << std::endl;
      
      if(logger_.get() == 0)
      {
        logger_.reset(
          new El::Logging::StreamLogger(
            std::cerr,
            conf.log_level,
            conf.log_aspects.empty() ? "*" : conf.log_aspects.c_str()));

        srand(time(0));
        
        if (!::setlocale(LC_CTYPE, "en_US.utf8"))
        {
          throw Exception(
            "El::PSP::Module::child_init: cannot set en_US.utf8 locale");
        }

        python_use_.reset(new El::Python::Use());
        allow_threads_.reset(new El::Python::AllowOtherThreads());
      }

      if(request_cache_.get() == 0 && conf.cache_enabled)
      {
        request_cache_.reset(
          new RequestCache::Cache(conf.entry_timeout_delay,
                                  conf.entry_unused_check_period,
                                  conf.cache_trace_enabled ?
                                  logger_.get() : 0));

        cache_trace_enabled_ = conf.cache_trace_enabled;
      }

      search_paths_.insert(conf.search_paths.begin(),
                           conf.search_paths.end());

      if(!conf.localization.empty())
      {
        El::Loc::Localizer::instance().init(conf.localization.c_str());
      }
      
      El::Python::EnsureCurrentThread guard;
      
      for(Config::ObjectInfoMap::iterator it = conf.objects.begin();
          it != conf.objects.end(); it++)
      {
        Config::ObjectInfo& oi = it->second;

        if(oi.factory_function.empty())
        {
          continue;
        }

        PyCFunction factory_function =
          El::Python::Module::find_function(oi.factory_function.c_str());

        if(factory_function == 0)
        {
          std::ostringstream ostr;
          ostr << "El::PSP::Module::child_init: can't find factory-function "
               << oi.factory_function;

          throw Exception(ostr.str());
        }

        El::PSP::Config_var config = new El::PSP::Config();

        if(!oi.config_key.empty())
        {
          size_t len = oi.config_key.size();
          const char* key = oi.config_key.c_str();
            
          for(ArgArrayMap::const_iterator it = conf.options.begin();
              it != conf.options.end(); it++)
          {
            if(strncmp(key, it->first.c_str(), len) == 0)
            {
              add_options(config.in(), it->first.c_str() + len, it->second);
            }
          }
        }

        El::Python::Object_var lg =
          new El::Logging::Python::Logger(logger(), false);
          
        El::Python::Object_var args =
          Py_BuildValue("(OO)",
                        static_cast<PyObject*>(config.in()),
                        static_cast<PyObject*>(lg.in()));

        oi.object = (*factory_function)(0, args.in());

        if(oi.object.in() == 0)
        {
          El::Python::handle_error("Module::child_init");
        }
      }
    }

    void
    Module::child_cleanup(Config& conf) throw(El::Exception)
    {
      if(allow_threads_.get() == 0)
      {
        return;
      }

//      std::cerr << "-----\n";

      El::Python::EnsureCurrentThread guard;

      for(Config::ObjectInfoMap::iterator it = conf.objects.begin();
          it != conf.objects.end(); it++)
      {
        Config::ObjectInfo& oi = it->second;

        if(oi.cleanup_function.empty() || oi.object.in() == 0)
        {
          continue;
        }

        PyCFunction func =
          El::Python::Module::find_function(oi.cleanup_function.c_str());

        if(func == 0)
        {
          continue;
        }

        El::Python::Object_var args =
          Py_BuildValue("(O)", static_cast<PyObject*>(oi.object.in()));

        El::Python::Object_var res = (*func)(0, args.in());
      }

/*
      for(Config::ObjectInfoMap::iterator it = conf.objects.begin();
          it != conf.objects.end(); it++)
      {
        Config::ObjectInfo& oi = it->second;

        std::cerr << it->first << " : ";

        if(oi.object.in() != 0)
        {
          std::cerr << oi.object->ob_refcnt;
        }
        else
        {
          std::cerr << "null";
        }

        std::cerr << std::endl;
      }
*/    
      conf.objects.clear();
    }
      
    void
    Module::child_cleanup() throw(El::Exception)
    {
      {
        El::Python::EnsureCurrentThread guard;
        code_cache_.clear();
      }
      
      logger()->info("El::PSP::Module::child_cleanup: module terminated",
                     Aspect::MODULE);
    }

    void
    Module::remove_deflate_output_filter(El::Apache::Request& request)
      throw(El::Exception)
    {
      for(ap_filter_t *f = request.ap_request->output_filters; f;
          f = f->next)
      {
        if(strcmp(f->frec->name, "deflate") == 0)
        {
          ap_remove_output_filter(f);
          return;
        }
      }
    }

    int
    Module::handler(Context& context) throw(El::Exception)
    {
      El::Apache::Request& request = context.request;
      Config& conf = *context.config();

//      std::cerr << "handler root: " << conf.root << std::endl;
      
      if(!conf.canonical_endpoint.empty() &&
         request.method_number() == M_GET && *request.endpoint() != '\0' &&
         conf.canonical_endpoint != request.endpoint())
      {
        const char* ignore_canonical =
          request.in().parameters(true).find("psp-ice");

        if(ignore_canonical == 0)
        {
          ignore_canonical = request.in().cookies().most_specific("psp-ice");
        }
        
        bool ice = false;
        std::auto_ptr< ::El::Net::HTTP::CookieSetter > cookie_setter;
        
        if(ignore_canonical)
        {
          ice = strcmp(ignore_canonical, "1") == 0;
          
          if(ice)
          {
            cookie_setter.reset(
              new ::El::Net::HTTP::CookieSetter("psp-ice", "1", 0, "/"));
          }
          else
          {
            cookie_setter.reset(
              new ::El::Net::HTTP::CookieSetter("psp-ice", "0", 0, "/"));
          }
        }

        if(cookie_setter.get())
        {
          request.out().send_cookie(*cookie_setter);
        }

        if(!ice)
        {
          std::string redirect_url(request.secure() ? "https://" : "http://");
          redirect_url += conf.canonical_endpoint + request.unparsed_uri();
          
          request.out().send_location(redirect_url.c_str());
          return HTTP_MOVED_PERMANENTLY;
        }
      }
      
      if(request_cache_.get())
      {
        bool log = cache_trace_enabled_ && logger_.get() != 0 &&
          logger_->will_log(El::Logging::DEBUG);
        
        if(conf.request_cacheable(request, log ? logger_.get() : 0))
        {
          RequestCache::Entry_var entry =
            request_cache_->entry(request,
                                  conf.rcache_key_type,
                                  conf.entry_timeout,
                                  conf.entry_unused_timeout);
        
          if(entry.in())
          {
            remove_deflate_output_filter(request);
            return entry->respond(request, log ? logger_.get() : 0);
          }
        }
      }

      if(conf.deflate_level > -2)
      {
        bool remove_deflate_filter = true;
        
        if(conf.deflate_level)
        {
          remove_deflate_filter =
            request.deflate(conf.deflate_level > 0 ? conf.deflate_level :
                            Z_DEFAULT_COMPRESSION);
        }

        if(remove_deflate_filter)
        {
          remove_deflate_output_filter(request);
        }
      }
        
      std::string handler;
      El::String::Manip::to_lower(request.ap_request->handler, handler);

      int error_in_response = std::max(conf.error_in_response, 0);
      
      try
      {
        std::string request_uri;
        El::Lang lang = calc_language(request, conf, request_uri);

        if(lang == El::Lang::nonexistent)
        {
          request.out().send_location(request_uri.c_str());
          return HTTP_MOVED_TEMPORARILY;
        }

        ETagCalc etag_calc = conf.etag_calc < 0 ? ETC_NONE :
          static_cast<ETagCalc>(conf.etag_calc);

        Request::Forward_var forward;

        while(true)
        {
          if(handler == "psp-script")
          {
            std::string path;

            if(forward)
            {
              const std::string& dest_path = forward->dest_path;
              
              path = strncmp(dest_path.c_str(), "//", 2) ?
                (conf.root + dest_path) : dest_path;
            }
            else
            {
              const ModuleConfig::StrWithLenArray& subpath_handlings =
                conf.subpath_handlings;
              
              ModuleConfig::StrWithLenArray::const_iterator it =
                subpath_handlings.begin();
              
              for(; it != subpath_handlings.end() &&
                    strncmp(it->string.c_str(),
                            request_uri.c_str(),
                            it->length); it++);

              if(it == subpath_handlings.end())
              {
                path = conf.root + request_uri + ".psp";
              }
              else
              {
                std::string handler_path(it->string.c_str(), it->length - 1);
                path = conf.root + handler_path + ".psp";
              }
            }

//            std::cerr << request_uri << "||" << path << std::endl;
            
            int result = 0;
            forward = script_handler(context,
                                     path.c_str(),
                                     lang,
                                     conf.valid_languages,
                                     etag_calc,
                                     forward.in(),
                                     result);

            if(forward.in() == 0)
            {
              return result;
            }

            handler = forward->is_script ? "psp-script" : "psp-template";
            etag_calc = ETC_NONE;
          }
          else if(handler == "psp-template")
          { 
            std::string path = conf.root +
              (forward ? forward->dest_path : (request_uri + ".tsp"));
                                               
            return template_handler(context, path.c_str(), lang, etag_calc);
          }
          else
          {
            std::ostringstream ostr;
            ostr << "El::PSP::Module::handler: unexpected handler '" << handler
                 << "'";
      
            throw Exception(ostr.str());
          }
        }
      }
      catch(const El::Lang::Exception& e)
      {
        return handle_error(request,
                            e.what(),
                            error_in_response,
                            HTTP_BAD_REQUEST);
      }
      catch(const El::Cache::NotFound& e)
      {
        return handle_error(request,
                            e.what(),
                            error_in_response,
                            HTTP_NOT_FOUND);
      }
      catch(const El::Exception& e)
      {
        int res = handle_error(request,
                               e.what(),
                               error_in_response,
                               HTTP_INTERNAL_SERVER_ERROR);

        if(res == HTTP_INTERNAL_SERVER_ERROR)
        {
          throw;
        }

        return res;
      }
    }

    int
    Module::handle_error(El::Apache::Request& request,
                         const char* error,
                         bool error_in_response,
                         unsigned long code)
      throw(El::Exception)
    {
      if(request.state() >= El::Apache::Request::RPS_HEADERS_WRITING &&
         request.out().stream_created())
      {
        std::ostream& ostr = request.out().stream();
        
        ostr << "<p>";

        if(error_in_response)
        {
          ostr << "<pre>";
          
          El::String::Manip::xml_encode(error,
                                        ostr,
                                        El::String::Manip::XE_TEXT_ENCODING |
                                        El::String::Manip::XE_PRESERVE_UTF8 |
                                        El::String::Manip::XE_LAX_ENCODING);
          ostr << "</pre>";
        }
        else
        {
          ostr << "<h1>Internal Server Error</h1>";
        }
        
        return OK;
      }
      
      if(error_in_response)
      {
        error_custom_response(request, error, code);
      }

      return code;
    }
      
    void
    Module::error_custom_response(El::Apache::Request& request,
                                  const char* text,
                                  unsigned long code) throw(El::Exception)
    {
      std::ostringstream ostr;
      ostr << "<html><body><pre>";
      
      El::String::Manip::xml_encode(text,
                                    ostr,
                                    El::String::Manip::XE_TEXT_ENCODING |
                                    El::String::Manip::XE_PRESERVE_UTF8 |
                                    El::String::Manip::XE_LAX_ENCODING);
          
      ostr << "</pre></body></html>";
      
      request.out().error_custom_response(code, ostr.str().c_str());
    }

    Request::Forward*
    Module::script_handler(Context& context,
                           const char* code_path,
                           const El::Lang& lang,
                           const El::PSP::Request::LangMap& valid_languages,
                           ETagCalc etag_calc,
                           Request::Forward* forward_ags,
                           int& result) throw(El::Exception)
    {
      El::Apache::Request& request = context.request;
      Config& conf = *context.config();

      El::PSP::Code::CodeGuard code_guard(code_cache_.code_lock(code_path));
      El::Python::EnsureCurrentThread guard;

      try
      {
        El::PSP::Config_var psp_conf = create_conf(conf.options);
        El::PSP::Code::ObjectMap objects;

        for(Config::ObjectInfoMap::iterator it = conf.objects.begin();
            it != conf.objects.end(); it++)
        {
          Config::ObjectInfo& oi = it->second;

          if(oi.object.in() != 0)
          {
            objects[it->first] = oi.object;
          }
        }

        El::Python::Object_var res =
          code_cache_.run(code_path,
                          code_guard,
                          request,
                          lang,
                          valid_languages,
                          psp_conf.in(),
                          forward_ags ? forward_ags->parameter.in() : 0,
                          &objects,
                          etag_calc);

        if(res.in() == 0 || res.in() == Py_None)
        {
          result = OK;
          return 0;
        }

        if(Request::Forward::Type::check_type(res.in()))
        {
          result = OK;
          return Request::Forward::Type::down_cast(res.in(), true);
        }

        if(PyNumber_Check(res.in()))
        {
          res = PyNumber_Long(res.in());
          result = PyLong_AsLong(res.in());
          return 0;
        }

        El::Python::Object_var str = PyObject_Str(res.in());
        const char* str_res = str.in() ? PyString_AsString(str.in()) : 0;
        
        std::ostringstream ostr;
        ostr << "El::PSP::Module::script_handler: unexpected script "
          "execution result '" << (str_res ? str_res : "<unknown>") << "'";

        throw Exception(ostr.str());        
      }
      catch(const El::Python::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "El::PSP::Module::script_handler: El::Python::Exception "
          "caught.\nUri: " << request.unparsed_uri();

        ostr << "\nParams:";
        const El::Net::HTTP::ParamList& params = request.in().parameters();
        
        for(El::Net::HTTP::ParamList::const_iterator it = params.begin();
            it != params.end(); it++)
        {
          ostr << std::endl << it->name << ":" << it->value;
        }
        
        ostr << "\nHeaders:";
        const El::Net::HTTP::HeaderList& headers = request.in().headers();

        for(El::Net::HTTP::HeaderList::const_iterator it = headers.begin();
            it != headers.end(); it++)
        {
          ostr << std::endl << it->name << ":" << it->value;
        }
        
        ostr << "Descripion:\n" << e;

        throw Exception(ostr.str());
      }
    }

    int
    Module::template_handler(Context& context,
                             const char* template_path,
                             const El::Lang& lang,
                             ETagCalc etag_calc)
      throw(El::Exception)
    {
      El::Apache::Request& request = context.request;
      Config& conf = *context.config();

      return template_cache_.run(template_path,
                                 request,
                                 conf.options,
                                 lang,
                                 etag_calc);
    }

    El::PSP::Config*
    Module::create_conf(const ArgArrayMap& options)
      throw(Exception, El::Exception)
    {
      El::PSP::Config_var conf = new El::PSP::Config();
      
      for(ArgArrayMap::const_iterator it = options.begin();
          it != options.end(); it++)
      {
        add_options(conf.in(), it->first.c_str(), it->second);
      }

      return conf.retn();
    }

    void
    Module::add_options(El::PSP::Config* conf,
                        const char* key,
                        const Directive::ArgArray& vals)
      throw(Exception, El::Exception)
    {
      size_t vals_size = vals.size();
        
      El::Python::Object_var val;
        
      if(vals_size == 1)
      {
        val = PyString_FromString(vals[0].string());
      }
      else if(vals_size > 1)
      {
        val = PyList_New(vals_size);

        size_t i = 0;
        
        for(Directive::ArgArray::const_iterator dit = vals.begin();
            dit != vals.end(); dit++, i++)
        {
          PyList_SetItem(val.in(), i, PyString_FromString(vals[i].string()));
        }
      }
        
      conf->set(key, val.in());
    }
    
      
    El::Lang
    Module::calc_language(El::Apache::Request& request,
                          Config& conf,
                          std::string& request_uri)
      throw(Exception, El::Exception)
    {
      request_uri = request.uri();

      bool refered_by_search_engine = false;
      
      const char* referer =
        request.in().headers().find(El::Net::HTTP::HD_REFERER);

//      referer = "http://webalta.ru/poisk?q=%D0%BE%D0%B1%D1%80%D0%B0%D0%B7%D1%86%D1%8B+%D1%84%D0%BE%D1%80%D0%BC%D1%8B+%D0%BF%D0%BE%D0%BB%D0%B8%D1%86%D0%B8%D0%B8";
//      referer = "http://yandex.ru/yandsearch?text=%f7%f2%ee+%e8%e7%ee%e1%f0%e0%e6%e5%ed%ee+%ed%e0+%eb%ee%e3%ee%f2%e8%ef%e5+%e8%e7%e4%e0%f2%e5%eb%fc%f1%f2%e2%e0+%22%eb%e0%e1%e8%f0%e8%ed%f2+%ef%f0%e5%f1%f1%22?&lr=213";
      
      if(referer)
      {
        El::Net::HTTP::SearchInfo si = El::Net::HTTP::search_info(referer);
        refered_by_search_engine = !si.engine.empty();
      }

      El::Lang lang;

      El::Lang param_lang;
      std::string param_lang_name;
      
      El::Lang url_lang;
      size_t url_lang_offset = 0;
      
      Config::LangCalcArgList& lang_calc_args = conf.lang_calc_args;

      for(Config::LangCalcArgList::const_iterator i(lang_calc_args.begin()),
            e(lang_calc_args.end()); i != e; ++i)
      {
        const Config::LangCalcArg& lca = *i;
        
        switch(lca.type)
        {
        case Config::LCAT_CRAWLER:
          {
            if(lang != El::Lang::null)
            {
              continue;
            }
            
            const char* crawler =
              El::Net::HTTP::crawler(
                request.in().headers().find(El::Net::HTTP::HD_USER_AGENT));

            if(*crawler != '\0')
            {
              try
              {
                lang = El::Lang(lca.value.c_str());
              }
              catch(...)
              {
              }
            }
            
            break;
          }
        case Config::LCAT_PARAM:
          {
            if(param_lang != El::Lang::null)
            {
              continue;
            }
            
            const El::Net::HTTP::ParamList& parameters =
              request.in().parameters(true);
             
            for(El::Net::HTTP::ParamList::const_iterator
                  pi(parameters.begin()), pe(parameters.end()); pi != pe; ++pi)
            {
              if(pi->name == lca.value)
              {
                param_lang = El::Lang(pi->value.c_str());
                param_lang_name = lca.value;
                break;
              }
            }
            
            if(lang == El::Lang::null && !refered_by_search_engine)
            {
              lang = param_lang;
            }            

            break;
          }
        case Config::LCAT_URL:
          {
            const char* uri = request.uri();
            const char* fname = strrchr(uri, '/');

            if(fname == uri)
            {
              fname = uri + strlen(uri);
            }

            if(fname != 0)
            {
              request_uri.assign(uri, fname - uri);
              const char* l = strrchr(request_uri.c_str(), '/');

              if(l)
              {
                try
                {
                  url_lang = El::Lang(l + 1);

                  if(lang == El::Lang::null && !refered_by_search_engine)
                  {
                    lang = url_lang;
                  }
                  
                  url_lang_offset = l - request_uri.c_str();
                  request_uri.reserve(url_lang_offset + strlen(fname));
                  request_uri.resize(url_lang_offset);
                }
                catch(...)
                {
                }
              }

              request_uri += *fname == '\0' ? "/" : fname;
            } 

            break;
          }
        case Config::LCAT_PREF:
          {
            if(lang != El::Lang::null)
            {
              continue;
            }

            El::NameValueMap preferences(
              request.in().cookies().most_specific(lca.value.c_str()),
              ':',
              '-');

            try
            {
              lang = El::Lang(preferences.value(lca.sub_value.c_str()));
            }
            catch(...)
            {
            }
              
            break;
          }
        case Config::LCAT_COOKIE:
          {
            if(lang != El::Lang::null)
            {
              continue;
            }

            const char* l =
              request.in().cookies().most_specific(lca.value.c_str());

            try
            {
              lang = El::Lang(l);
            }
            catch(...)
            {
            }              
              
            break;
          }
        case Config::LCAT_HEADER:
          {
            if(lang != El::Lang::null)
            {
              continue;
            }

            const El::Net::HTTP::AcceptLanguageList& al =
              request.in().accept_languages();

            if(!al.empty())
            {
              try
              {
                lang = al.begin()->language;
              }
              catch(...)
              {
              }            
            }
              
            break;
          }
        case Config::LCAT_RESET: return El::Lang::null;
        }
      }

      const El::PSP::Request::LangMap& valid_languages = conf.valid_languages;
      
      El::PSP::Request::LangMap::const_iterator i =
        valid_languages.find(lang.l3_code());

      if(i == valid_languages.end())
      {
        i = valid_languages.find("*");
      }
      
      lang = i == valid_languages.end() ? El::Lang::null : i->second;

      if(refered_by_search_engine && lang != El::Lang::null)
      {
        bool switch_param_lang = param_lang != El::Lang::null &&
          param_lang != lang;

        bool switch_url_lang = url_lang != El::Lang::null && url_lang != lang;

        const char* host = 0;
        
        if((switch_param_lang || switch_url_lang) &&
           request.method_number() == M_GET &&
           (host = request.in().headers().find(El::Net::HTTP::HD_HOST)) != 0)
        {
          std::string redirect_url(request.secure() ? "https://" : "http://");
          redirect_url += host;
          
          if(switch_url_lang)
          {
            const char* u = request_uri.c_str();
            const char* l = u + url_lang_offset + 1;

            redirect_url += (std::string(u, l - u) + lang.l3_code() + "/" + l);
          }
          else
          {
            redirect_url += request_uri;
          }

          const El::Net::HTTP::ParamList& parameters =
            request.in().parameters(true);

          if(!parameters.empty())
          {
            std::ostringstream ostr;
        
            for(El::Net::HTTP::ParamList::const_iterator
                  pb(parameters.begin()), pi(pb), pe(parameters.end());
                pi != pe; ++pi)
            {
              ostr << (pi == pb ? "?" : "&");
            
              const std::string& name = pi->name;
              El::String::Manip::mime_url_encode(name.c_str(), ostr);
              ostr << "=";
            
              El::String::Manip::mime_url_encode(
                switch_param_lang && name == param_lang_name ? lang.l3_code() :
                pi->value.c_str(), ostr);          
            }

            redirect_url += ostr.str();
          }

//          std::cerr << "DDD:" << redirect_url << std::endl;

          request_uri.swap(redirect_url);
          return El::Lang::nonexistent;
        }
      }

      return lang;
    }
    
    //
    // ModuleConfig class
    //
    bool
    ModuleConfig::request_cacheable(El::Apache::Request& request,
                                    El::Logging::Logger* logger) const throw()
    {
      if(!RequestCache::Entry::time(entry_timeout) ||
         !rcache_conditions.request_cacheable(request))
      {
        return false;
      }

      return true;      
/*
      std::auto_ptr<std::ostringstream> ostr;

      if(logger)
      {
        ostr.reset(new std::ostringstream());
        *ostr << "ModuleConfig::request_cacheable:\nIP: "
              << request.remote_ip()
              << ", URI: '" << request.unparsed_uri() << "'";
      }

      bool can_deflate = deflate_level > 0 || deflate_level == -1;

      for(ap_filter_t *f = request.ap_request->output_filters;
          !can_deflate && f; f = f->next)
      {
        can_deflate = strcmp(f->frec->name, "deflate") == 0;
      }

      if(!can_deflate)
      {
        if(logger)
        {
          *ostr << "\nNo - can't deflate";
          logger->debug(ostr->str(), Aspect::CACHE);
        }
      
        return true;
      }

      const char* accept_enc =
        request.in().headers().find(El::Net::HTTP::HD_ACCEPT_ENCODING);
      
      if(accept_enc == 0)
      {
        if(logger)
        {
          *ostr << "\nNo - accept encoding not specified\nHeaders:";
          const El::Net::HTTP::HeaderList& headers = request.in().headers();
          
          for(El::Net::HTTP::HeaderList::const_iterator i(headers.begin()),
                e(headers.end()); i != e; ++i)
          {
            *ostr << "\n" << i->name << ":" << i->value;
          }

          logger->debug(ostr->str(), Aspect::CACHE);
        }
        
        return false;
      }
      
      const char* b = strstr(accept_enc, "gzip");
      const char* e = 0;

      bool res = b && (b == accept_enc || strchr("\t ,;", *(b - 1)) != 0) &&
        (*(e = b + 4) == '\0' || strchr("\t ,;", *e) != 0);

      if(logger)
      {
        if(res)
        {
          *ostr << "\nYes";
        }
        else
        {
          *ostr << "\nNo - GZIP unsupported\nHeaders:";
          const El::Net::HTTP::HeaderList& headers = request.in().headers();
          
          for(El::Net::HTTP::HeaderList::const_iterator i(headers.begin()),
                e(headers.end()); i != e; ++i)
          {
            *ostr << "\n" << i->name << ":" << i->value;
          }
        }

        logger->debug(ostr->str(), Aspect::CACHE);
      }

      return res;
*/
    }
    
    void
    ModuleConfig::parse_valid_lang(const El::Directive::ArgArray& args)
      throw(Exception, El::Exception)
    {
      for(El::Directive::ArgArray::const_iterator it = args.begin();
          it != args.end(); it++)
      {
        std::string lp = it->string();
        std::string::size_type pos = lp.find(':');

        if(pos == std::string::npos)
        {
          std::ostringstream ostr;
          ostr << "El::PSP::ModuleConfig::parse_valid_lang: '"
               << lp << "' entry do not contain ':'";

          throw Exception(ostr.str());
        }
        
        valid_languages[std::string(lp, 0, pos)] =
          El::Lang(lp.c_str() + pos + 1);          
      }
    }
    
    void
    ModuleConfig::parse_subpath_handling(const El::Directive::ArgArray& args)
      throw(El::Exception)
    {
      for(El::Directive::ArgArray::const_iterator it = args.begin();
          it != args.end(); it++)
      {
        std::string subpath = it->string();

        if(!subpath.empty())
        {
          if(subpath[subpath.length() - 1] != '/')
          {
            subpath += "/";
          }
          
          subpath_handlings.push_back(StrWithLen(subpath.c_str()));
        }
      }
    }
    
    void
    ModuleConfig::parse_calc_lang(const El::Directive::ArgArray& args)
      throw(El::Exception)
    {
      for(El::Directive::ArgArray::const_iterator it = args.begin();
          it != args.end(); it++)
      {
        LangCalcArg lca;
        const char* arg = it->string();
          
        if(strcmp(arg, "reset") == 0)
        {
          lang_calc_args.clear();
          lca.type = LCAT_RESET;
          lang_calc_args.push_back(lca);

          return;
        }
        else if(strcmp(arg, "url") == 0)
        {
          lca.type = LCAT_URL;
        }
        else if(strcmp(arg, "header") == 0)
        {
          lca.type = LCAT_HEADER;
        }
        else
        {
          const char* val = strchr(arg, '=');

          if(val == 0 || val[1] == '\0')
          {
            std::ostringstream ostr;
            ostr << "El::PSP::Module::parse_calc_lang: unexpected "
              "PSP_LangCalc argument " << arg;

            throw Exception(ostr.str());
          }

          std::string name(arg, val++ - arg);

          if(name == "param")
          {
            lca.type = LCAT_PARAM;
            lca.value = val;
          }
          else if(name == "cookie")
          {
            lca.type = LCAT_COOKIE;            
            lca.value = val;
          }
          else if(name == "preference")
          {
            lca.type = LCAT_PREF;
            
            const char* sub_val = strchr(val, ':');
            
            if(sub_val == 0 || sub_val == val || sub_val[1] == '\0')
            {
              std::ostringstream ostr;
              ostr << "El::PSP::Module::parse_calc_lang: unexpected "
                "PSP_LangCalc argument " << arg;

              throw Exception(ostr.str());
            }
            
            lca.value.assign(val, sub_val++ - val);
            lca.sub_value = sub_val;
          }
          else if(name == "crawler")
          {
            lca.type = LCAT_CRAWLER;
            lca.value = val;
          }
          else
          {
            std::ostringstream ostr;
            ostr << "El::PSP::Module::parse_calc_lang: unexpected "
              "PSP_LangCalc argument " << arg;

            throw Exception(ostr.str());
          }
        }

        lang_calc_args.push_back(lca);
      }
    }

    //
    // El::PSP::PyModule class
    //
    class PyModule : public El::Python::ModuleImpl<PyModule>
    {
    public:
      static PyModule instance;

      PyModule() throw(El::Exception);

      PyObject* py_browser(PyObject* args) throw(El::Exception);
      PyObject* py_feed_reader(PyObject* args) throw(El::Exception);
      PyObject* py_crawler(PyObject* args) throw(El::Exception);
      PyObject* py_os(PyObject* args) throw(El::Exception);
      PyObject* py_computer(PyObject* args) throw(El::Exception);
      PyObject* py_tab(PyObject* args) throw(El::Exception);
      PyObject* py_phone(PyObject* args) throw(El::Exception);
      PyObject* py_search_info(PyObject* args) throw(El::Exception);
  
      PY_MODULE_METHOD_VARARGS(
        py_browser,
        "browser",
        "Returns browser name from User-Agent header value");  

      PY_MODULE_METHOD_VARARGS(
        py_feed_reader,
        "feed_reader",
        "Returns feed reader name from User-Agent header value");

      PY_MODULE_METHOD_VARARGS(
        py_crawler,
        "crawler",
        "Returns crawler name from User-Agent header value");

      PY_MODULE_METHOD_VARARGS(
        py_computer,
        "computer",
        "Returns computer name from User-Agent header value");  

      PY_MODULE_METHOD_VARARGS(
        py_tab,
        "tab",
        "Returns tab name from User-Agent header value");  

      PY_MODULE_METHOD_VARARGS(
        py_phone,
        "phone",
        "Returns phone name from User-Agent header value");  

      PY_MODULE_METHOD_VARARGS(
        py_os,
        "os",
        "Returns OS name from User-Agent header value");  

      PY_MODULE_METHOD_VARARGS(
        py_search_info,
        "search_info",
        "Returns search info from url for known engines");
    };

    PyModule PyModule::instance;
    
    PyModule::PyModule() throw(El::Exception)
        : El::Python::ModuleImpl<PyModule>(
          "el.psp",
          "Module containing PSP library objects, types and methods.",
          true)
    {
    }

    PyObject*
    PyModule::py_browser(PyObject* args) throw(El::Exception)
    {
      char* user_agent = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.psp.browser", &user_agent))
      {
        El::Python::handle_error("El::PyModule::py_browser");
      }

      return PyString_FromString(El::Net::HTTP::browser(user_agent));
    }

    PyObject*
    PyModule::py_feed_reader(PyObject* args) throw(El::Exception)
    {
      char* user_agent = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.psp.feed_reader", &user_agent))
      {
        El::Python::handle_error("El::PyModule::py_feed_reader");
      }

      return PyString_FromString(El::Net::HTTP::feed_reader(user_agent));
    }

    PyObject*
    PyModule::py_crawler(PyObject* args) throw(El::Exception)
    {
      char* user_agent = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.psp.crawler", &user_agent))
      {
        El::Python::handle_error("El::PyModule::py_crawler");
      }

      return PyString_FromString(El::Net::HTTP::crawler(user_agent));
    }

    PyObject*
    PyModule::py_computer(PyObject* args) throw(El::Exception)
    {
      char* user_agent = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.psp.computer", &user_agent))
      {
        El::Python::handle_error("El::PyModule::py_computer");
      }

      return PyString_FromString(El::Net::HTTP::computer(user_agent));
    }

    PyObject*
    PyModule::py_tab(PyObject* args) throw(El::Exception)
    {
      char* user_agent = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.psp.tab", &user_agent))
      {
        El::Python::handle_error("El::PyModule::py_tab");
      }

      return PyString_FromString(El::Net::HTTP::tab(user_agent));
    }

    PyObject*
    PyModule::py_phone(PyObject* args) throw(El::Exception)
    {
      char* user_agent = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.psp.phone", &user_agent))
      {
        El::Python::handle_error("El::PyModule::py_phone");
      }

      return PyString_FromString(El::Net::HTTP::phone(user_agent));
    }

    PyObject*
    PyModule::py_os(PyObject* args) throw(El::Exception)
    {
      char* user_agent = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.psp.os", &user_agent))
      {
        El::Python::handle_error("El::PyModule::py_os");
      }

      return PyString_FromString(El::Net::HTTP::os(user_agent));
    }

    PyObject*
    PyModule::py_search_info(PyObject* args) throw(El::Exception)
    {
      char* url = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.psp.search_info", &url))
      {
        El::Python::handle_error("El::PyModule::py_search_info");
      }

      El::Net::HTTP::SearchInfo si = El::Net::HTTP::search_info(url);

      if(si.engine.empty())
      {
        return El::Python::add_ref(Py_None);
      }

      return new El::Net::HTTP::Python::SearchInfo(si);
    }

  }
}
