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
 * @file   Elements/El/PSP/PSPModule.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_PSP_PSPMODULE_HPP_
#define _ELEMENTS_EL_PSP_PSPMODULE_HPP_

#include <string>
#include <memory>

#include <Python.h>

#include <El/Exception.hpp>
#include <El/Apache/Module.hpp>

#include <El/Directive.hpp>

#include <El/Logging/LoggerBase.hpp>

#include <El/Python/Utility.hpp>
#include <El/Python/RefCount.hpp>

#include <El/PSP/Code.hpp>
#include <El/PSP/Template.hpp>
#include <El/PSP/CommonTypes.hpp>
#include <El/PSP/RequestCache.hpp>

namespace El
{
  namespace PSP
  {
    struct ModuleConfig
    {
      static El::Apache::ModuleRef mod_ref;    

      EL_EXCEPTION(Exception, El::ExceptionBase);

      ModuleConfig() throw(El::Exception);

      void parse_calc_lang(const El::Directive::ArgArray& args)
        throw(El::Exception);

      void parse_valid_lang(const El::Directive::ArgArray& args)
        throw(Exception, El::Exception);

      void parse_subpath_handling(const El::Directive::ArgArray& args)
        throw(El::Exception);
      
    // Config merging constructor
      ModuleConfig(const ModuleConfig& cf_base, const ModuleConfig& cf_new)
        throw(El::Exception);

      bool request_cacheable(El::Apache::Request& request,
                             El::Logging::Logger* logger) const throw();

      enum LangCalcArgType
      {
        LCAT_PARAM,
        LCAT_URL,
        LCAT_PREF,
        LCAT_COOKIE,
        LCAT_HEADER,
        LCAT_CRAWLER,
        LCAT_RESET
      };

      struct LangCalcArg
      {
        LangCalcArgType type;
        std::string value;
        std::string sub_value;
      };

      struct ObjectInfo
      {
        std::string factory_function;
        std::string cleanup_function;
        std::string config_key;
        El::Python::Object_var object;

        ObjectInfo() throw(El::Exception) {}
        ObjectInfo(const ObjectInfo& src) throw(El::Exception);
        ~ObjectInfo() throw();
        
        ObjectInfo& operator=(const ObjectInfo& src) throw(El::Exception);
      };

      typedef std::list<LangCalcArg> LangCalcArgList;
//      typedef std::map<std::string, El::Lang> LangMap;
      typedef std::map<std::string, ObjectInfo> ObjectInfoMap;
      typedef std::set<std::string> StringSet;

      struct StrWithLen
      {
        std::string string;
        size_t length;
        
        StrWithLen(const char* str) : string(str), length(strlen(str)) {}
      };
        
      typedef std::vector<StrWithLen> StrWithLenArray;

      RequestCache::ConditionMap rcache_conditions;
      RequestCache::CacheKeyType rcache_key_type;
      
      ArgArrayMap options;
      ObjectInfoMap objects;
      std::string root;
      int error_in_response;
      LangCalcArgList lang_calc_args;
      El::PSP::Request::LangMap valid_languages;
      StrWithLenArray subpath_handlings;
      StringSet search_paths;
      int etag_calc;
      std::string localization;
      unsigned long log_level;
      std::string log_aspects;
      bool cache_enabled;
      bool cache_trace_enabled;
      time_t entry_timeout;
      time_t entry_timeout_delay;
      time_t entry_unused_timeout;
      time_t entry_unused_check_period;
      int deflate_level;
      std::string canonical_endpoint;
    };
    
    class Module : public El::Apache::Module<ModuleConfig>
    {
    public:
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
    public:
      Module(El::Apache::ModuleRef& mod_ref) throw(El::Exception);
  
      El::Logging::LoggerBase* logger() const throw();

    private:
      virtual void directive(const El::Directive& directive,
                             Config& config)
        throw(El::Exception);

      virtual int handler(Context& context) throw(El::Exception);

      virtual void child_init(Config& conf) throw(El::Exception);      
      virtual void child_init() throw(El::Exception);      
      virtual void child_cleanup(Config& conf) throw(El::Exception);      
      virtual void child_cleanup() throw(El::Exception);
    
      static void remove_deflate_output_filter(El::Apache::Request& request)
        throw(El::Exception);
      
      int handle_error(El::Apache::Request& request,
                       const char* error,
                       bool error_in_response,
                       unsigned long code)
        throw(El::Exception);
      
      void error_custom_response(
        El::Apache::Request& request,
        const char* text,
        unsigned long code) throw(El::Exception);

      El::Lang calc_language(El::Apache::Request& request,
                             Config& conf,
                             std::string& request_uri)
        throw(Exception, El::Exception);
      
      El::PSP::Config* create_conf(const ArgArrayMap& options)
        throw(Exception, El::Exception);

      Request::Forward* script_handler(
        Context& context,
        const char* code_path,
        const El::Lang& lang,
        const El::PSP::Request::LangMap& valid_languages,
        ETagCalc etag_calc,
        Request::Forward* forward_ags,
        int& result) throw(El::Exception);
      
      int template_handler(Context& context,
                           const char* code_path,
                           const El::Lang& lang,
                           ETagCalc etag_calc) throw(El::Exception);

    void add_options(El::PSP::Config* conf,
                     const char* key,
                     const Directive::ArgArray& vals)
      throw(Exception, El::Exception);
      
    public:
      virtual ~Module() throw();

    private:
      std::auto_ptr<El::Logging::LoggerBase> logger_;
      
      std::auto_ptr<El::Python::Use> python_use_;
      El::PSP::CodeCache code_cache_;
      std::auto_ptr<El::Python::AllowOtherThreads> allow_threads_;
      El::PSP::TemplateCache template_cache_;
      Config::StringSet search_paths_;
      std::auto_ptr<RequestCache::Cache> request_cache_;
      bool cache_trace_enabled_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace PSP
  {
    //
    // ModuleConfig class
    //
    inline
    ModuleConfig::ModuleConfig() throw(El::Exception)
        : rcache_key_type(RequestCache::CKT_UNDEFINED),
          error_in_response(-1),
          etag_calc(-1),
          log_level(ULONG_MAX),
          cache_enabled(false),
          cache_trace_enabled(false),
          entry_timeout(RequestCache::TIME_UNSET),
          entry_timeout_delay(0),
          entry_unused_timeout(RequestCache::TIME_UNSET),
          entry_unused_check_period(0),
          deflate_level(-2)
    {
    }
    
    inline
    ModuleConfig::ModuleConfig(const ModuleConfig& cf_base,
                               const ModuleConfig& cf_new) throw(El::Exception)
    {
      options = cf_new.options;
      options.insert(cf_base.options.begin(), cf_base.options.end());

      objects = cf_new.objects;
      objects.insert(cf_base.objects.begin(), cf_base.objects.end());

      root = cf_new.root.empty() ? cf_base.root : cf_new.root;

      error_in_response = cf_new.error_in_response < 0 ?
        cf_base.error_in_response : cf_new.error_in_response;

      lang_calc_args = cf_new.lang_calc_args.empty() ?
        cf_base.lang_calc_args : cf_new.lang_calc_args;

      valid_languages = cf_new.valid_languages.empty() ?
        cf_base.valid_languages : cf_new.valid_languages;

      subpath_handlings = cf_new.subpath_handlings.empty() ?
        cf_base.subpath_handlings : cf_new.subpath_handlings;

      search_paths = cf_new.search_paths;
      search_paths.insert(cf_base.search_paths.begin(),
                          cf_base.search_paths.end());
      
      etag_calc = cf_new.etag_calc < 0 ? cf_base.etag_calc :
        cf_new.etag_calc;

      localization = cf_new.localization.empty() ?
        cf_base.localization : cf_new.localization;

      log_level = cf_new.log_level == ULONG_MAX ?
        cf_base.log_level : cf_new.log_level;

      log_aspects = cf_new.log_aspects.empty() ?
        cf_base.log_aspects : cf_new.log_aspects;

      rcache_key_type = cf_new.rcache_key_type == RequestCache::CKT_UNDEFINED ?
        cf_base.rcache_key_type : cf_new.rcache_key_type;

      rcache_conditions = cf_new.rcache_conditions;

      RequestCache::CondKey reset_key(RequestCache::CT_RESET);
      
      if(cf_new.rcache_conditions.find(reset_key) ==
         cf_new.rcache_conditions.end())
      {
        rcache_conditions.insert(cf_base.rcache_conditions.begin(),
                                 cf_base.rcache_conditions.end());
      }

      rcache_conditions.erase(reset_key);

      cache_enabled = std::max(cf_base.cache_enabled, cf_new.cache_enabled);
      
      cache_trace_enabled =
        std::max(cf_base.cache_trace_enabled, cf_new.cache_trace_enabled);
      
      entry_timeout = cf_new.entry_timeout == RequestCache::TIME_UNSET ?
        cf_base.entry_timeout : cf_new.entry_timeout;
      
      entry_unused_timeout =
        cf_new.entry_unused_timeout == RequestCache::TIME_UNSET ?
        cf_base.entry_unused_timeout : cf_new.entry_unused_timeout;

      entry_timeout_delay = std::max(cf_base.entry_timeout_delay,
                                     cf_new.entry_timeout_delay);

      entry_unused_check_period = std::max(cf_base.entry_unused_check_period,
                                           cf_new.entry_unused_check_period);

      deflate_level = cf_new.deflate_level > -2 ? cf_new.deflate_level :
        cf_base.deflate_level;

      canonical_endpoint = cf_new.canonical_endpoint.empty() ?
        cf_base.canonical_endpoint : cf_new.canonical_endpoint;
    }

    //
    // ModuleConfig::ObjectInfo struct
    //
    inline
    ModuleConfig::ObjectInfo::ObjectInfo(const ObjectInfo& src)
      throw(El::Exception)
    {
      factory_function = src.factory_function;
      cleanup_function = src.cleanup_function;
      config_key = src.config_key;

      std::auto_ptr<El::Python::EnsureCurrentThread> guard;
      if(object.in() != 0 || src.object.in() != 0)
      {
        guard.reset(new El::Python::EnsureCurrentThread());
      }
      
      object = src.object;
    }
    
    inline
    ModuleConfig::ObjectInfo::~ObjectInfo() throw()
    {      
      if(object.in() != 0)
      {
        El::Python::EnsureCurrentThread guard;      
        object = 0;
      }
    }
        
    inline
    ModuleConfig::ObjectInfo&
    ModuleConfig::ObjectInfo::operator=(const ObjectInfo& src)
      throw(El::Exception)
    {
      factory_function = src.factory_function;
      cleanup_function = src.cleanup_function;
      config_key = src.config_key;
      
      std::auto_ptr<El::Python::EnsureCurrentThread> guard;
      if(object.in() != 0 || src.object.in() != 0)
      {
        guard.reset(new El::Python::EnsureCurrentThread());
      }

      object = src.object;

      return *this;
    }
    
    //
    // Module class
    //
    inline
    Module::~Module() throw()
    {
    }

    inline
    El::Logging::LoggerBase*
    Module::logger() const throw()
    {
      return logger_.get();
    }
  
  }
}
  
#endif // _ELEMENTS_EL_PSP_PSPMODULE_HPP_
