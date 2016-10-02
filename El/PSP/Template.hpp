/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Template.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_TEMPLATE_HPP_
#define _ELEMENTS_EL_PSP_TEMPLATE_HPP_

#include <string>

#include <ext/hash_map>

#include <El/Exception.hpp>
#include <El/Lang.hpp>
#include <El/String/Manip.hpp>
#include <El/Hash/Hash.hpp>

#include <El/Cache/LocalizedTemplateFileCache.hpp>
#include <El/Cache/EncodingAwareLocalizedTemplate.hpp>

#include <El/Apache/Request.hpp>
#include <El/PSP/CommonTypes.hpp>

namespace El
{
  namespace PSP
  {
    class TemplateCache
    {
    public:
      TemplateCache(ACE_Time_Value review_filetime_period =
                    ACE_Time_Value::zero) throw(El::Exception);
      
      virtual ~TemplateCache() throw() {}

      int run(const char* template_path,
              El::Apache::Request& request,
              const ArgArrayMap& options,
              const El::Lang& lang,
              ETagCalc etag_calc)
        throw(El::Cache::Exception, El::Exception);

    protected:

      struct TemplateParseInterceptor :
        public El::Cache::EncodingAwareTemplateParseInterceptor
      {
        virtual ~TemplateParseInterceptor() throw() {}

        virtual void update(const std::string& tag,
                            const std::string& value,
                            El::String::Template::Chunk& chunk) const
          throw(El::String::Template::ParsingFailed,
                El::String::Template::Exception,
                El::Exception);
      };

      TemplateParseInterceptor parse_interceptor_;
      El::Cache::LocalizedTemplateFileCache template_cache_;
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
    // TemplateCache class
    //
    inline
    TemplateCache::TemplateCache(ACE_Time_Value review_filetime_period)
      throw(El::Exception)
        : template_cache_("<?",
                          "?>",
                          review_filetime_period,
                          &parse_interceptor_,
                          "")
    {
    }

  }
}

#endif // _ELEMENTS_EL_PSP_TEMPLATE_HPP_
