/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Cache/LocalizedTemplateFileCache.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CACHE_LOCALIZEDTEMPLATEFILECACHE_HPP_
#define _ELEMENTS_EL_CACHE_LOCALIZEDTEMPLATEFILECACHE_HPP_

#include <string>

#include <ext/hash_map>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/String/Manip.hpp>
#include <El/String/Template.hpp>
#include <El/Lang.hpp>
#include <El/Hash/Hash.hpp>
#include <El/CRC.hpp>

#include <El/Cache/ObjectCache.hpp>
#include <El/Cache/TextFileCache.hpp>
#include <El/Cache/TextTemplateFileCache.hpp>
#include <El/Cache/VariablesMapCache.hpp>

namespace El
{
  namespace Cache
  {
    class LocalizedTemplateFileCache : public TextTemplateFileCache
    {
    public:
      LocalizedTemplateFileCache(
        const char* var_left_marker,
        const char* var_right_marker,
        ACE_Time_Value review_filetime_period =
        ACE_Time_Value::zero,
        const El::String::Template::ParseInterceptor* interceptor = 0,
        const char* file_ext = 0)
        throw(El::Exception);
      
      virtual ~LocalizedTemplateFileCache() throw();

      struct InstantiationInterceptor
      {
        virtual ~InstantiationInterceptor() {}

        virtual El::String::Template::Variables* variables(
          const El::Cache::VariablesMap& localization)
          throw(El::Exception) = 0;
      };

      TextTemplateFile* get(const char* file_name,
                            const El::Lang& lang = El::Lang::null,
                            InstantiationInterceptor* interceptor = 0)
        throw(InvalidArg, NotFound, Exception, El::Exception);

    protected:

      struct LocalizedTemplateHolder
      {
        TextTemplateFile_var object;
        unsigned long long template_seq_num;
        unsigned long long localization_seq_num;
      };
      
      typedef __gnu_cxx::hash_map<std::string,
                                  LocalizedTemplateHolder,
                                  El::Hash::String> LocalizedTemplateMap;

      VariablesMapCache localizations_;
      LocalizedTemplateMap localized_templates_;
      std::string file_ext_;
    };
    
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Cache
  {
    //
    // LocalizedTemplateFileCache class
    //
    inline
    LocalizedTemplateFileCache::LocalizedTemplateFileCache(
      const char* var_left_marker,
      const char* var_right_marker,
      ACE_Time_Value review_filetime_period,
      const El::String::Template::ParseInterceptor* interceptor,
      const char* file_ext)
      throw(El::Exception)
        : TextTemplateFileCache(var_left_marker,
                                var_right_marker,
                                review_filetime_period,
                                interceptor),
          file_ext_(file_ext ? file_ext : ".loc")
    {
    }

    inline
    LocalizedTemplateFileCache::~LocalizedTemplateFileCache() throw()
    {
    }    

    inline
    TextTemplateFile*
    LocalizedTemplateFileCache::get(const char* file_name,
                                    const El::Lang& lang,
                                    InstantiationInterceptor* interceptor)
      throw(InvalidArg, NotFound, Exception, El::Exception)
    {
      TextTemplateFile_var obj = TextTemplateFileCache::get(file_name);

      if(lang == El::Lang::null)
      {
        return El::RefCount::add_ref(obj.in());
      }

      std::string localization_file = std::string(file_name) + file_ext_ +
        "." + lang.l3_code();
      
      VariablesMap_var localization =
        localizations_.get(localization_file.c_str());
      
      {
        ReadGuard guard(lock_);
      
        LocalizedTemplateMap::const_iterator it =
          localized_templates_.find(lang.l3_code());

        if(it != localized_templates_.end() &&
           it->second.template_seq_num == obj->sequence_number() &&
           it->second.localization_seq_num == localization->sequence_number())
        {
          return El::RefCount::add_ref(it->second.object.in());
        }
      }
      
      WriteGuard guard(lock_);
      
      LocalizedTemplateMap::const_iterator it =
        localized_templates_.find(lang.l3_code());

      if(it != localized_templates_.end() &&
         it->second.template_seq_num == obj->sequence_number() &&
         it->second.localization_seq_num == localization->sequence_number())
      {
        return El::RefCount::add_ref(it->second.object.in());
      }

      std::string localized_text;

      if(interceptor)
      {
        std::auto_ptr<El::String::Template::Variables> vars(
          interceptor->variables(*localization));

        localized_text = obj->instantiate(*vars, true);
      }
      else
      {
        localized_text = obj->instantiate(*localization, true);
      }

      TextTemplateFile_var localized_template =
        new TextTemplateFile(this, sequence_number_++, file_name);

      localized_template->parse(localized_text.c_str());

      unsigned long long hash = 0;
      
      CRC(hash,
          (const unsigned char*)localized_text.c_str(),
          localized_text.length());

      localized_template->hash(hash);
      
      LocalizedTemplateHolder holder;
      
      holder.object = localized_template;
      holder.template_seq_num = obj->sequence_number();
      holder.localization_seq_num = localization->sequence_number();

      localized_templates_[lang.l3_code()] = holder;
        
      return localized_template.retn();
    }

  }
}

#endif // _ELEMENTS_EL_CACHE_LOCALIZEDTEMPLATEFILECACHE_HPP_
