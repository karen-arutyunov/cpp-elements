/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Cache/EncodingAwareLocalizedTemplate.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CACHE_ENCODINGAWARELOCALIZEDTEMPLATE_HPP_
#define _ELEMENTS_EL_CACHE_ENCODINGAWARELOCALIZEDTEMPLATE_HPP_

#include <iostream>
#include <string>

#include <El/String/Template.hpp>
#include <El/Cache/LocalizedTemplateFileCache.hpp>

namespace El
{
  namespace Cache
  {
    class EncodableVariables : public El::String::Template::Variables
    {
    public:

      enum ValueEncoding
      {
        VE_DEFAULT,
        VE_NONE,
        VE_MIME_URL,
        VE_XML,
        VE_XML_STRICT
      };
      
      EncodableVariables() throw();

      static bool encode(const char* text,
                         std::ostream& output,
                         const El::String::Template::Chunk& chunk)
        throw(El::Exception);

      static void encode(const char* text,
                         std::ostream& output,
                         ValueEncoding encoding)
        throw(El::Exception);
    };

    class LocalizationInstantiationInterceptor :
      public El::Cache::LocalizedTemplateFileCache::InstantiationInterceptor
    {
    public:
      LocalizationInstantiationInterceptor(const char* var_left_marker,
                                           const char* var_right_marker)
        throw();
         
      virtual ~LocalizationInstantiationInterceptor() {}
      
      virtual El::String::Template::Variables* variables(
        const El::Cache::VariablesMap& localization)
        throw(El::Exception);

    protected:
      std::string var_left_marker_;
      std::string var_right_marker_;      
    };

    //
    // LocalizationVariables class
    //
    class LocalizationVariables : public Cache::EncodableVariables
    {
    public:
      LocalizationVariables(
        const char* var_left_marker,
        const char* var_right_marker,
        const El::Cache::VariablesMap& localization) throw();
      
      virtual ~LocalizationVariables() throw() {}
      
      virtual bool write(const El::String::Template::Chunk& chunk,
                         std::ostream& output) const
        throw(El::Exception);

    private:
      std::string var_left_marker_;
      std::string var_right_marker_;
      const El::Cache::VariablesMap& localization_;
      unsigned long default_encoding_;
    };

    class EncodingAwareTemplateParseInterceptor :
      public El::String::Template::ParseInterceptor
    {
    public:
      
      virtual ~EncodingAwareTemplateParseInterceptor() throw() {}
      
      virtual void update(El::String::Template::Chunk& chunk) const
        throw(El::String::Template::ParsingFailed,
              El::String::Template::Exception,
              El::Exception);

      virtual void update(const std::string& tag,
                          const std::string& value,
                          El::String::Template::Chunk& chunk) const
        throw(El::String::Template::ParsingFailed,
              El::String::Template::Exception,
              El::Exception);
      
    protected:

      static char separators_[];      
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
    // EncodableVariables class
    //
    inline
    EncodableVariables::EncodableVariables() throw()
    {
    }
      
    inline
    bool
    EncodableVariables::encode(const char* text,
                               std::ostream& output,
                               const El::String::Template::Chunk& chunk)
      throw(El::Exception)
    {
      unsigned long enc = reinterpret_cast<unsigned long>(chunk.tag); 
      encode(text, output, (ValueEncoding)enc);
      return true;
    }
    
    //
    // LocalizationInstantiationInterceptor class
    //
    inline
    LocalizationInstantiationInterceptor::LocalizationInstantiationInterceptor(
      const char* var_left_marker,
      const char* var_right_marker) throw()
        : var_left_marker_(var_left_marker),
          var_right_marker_(var_right_marker)
    {
    }

    //
    // LocalizationVariables class
    //
    inline
    LocalizationVariables::LocalizationVariables(
      const char* var_left_marker,
      const char* var_right_marker,
      const El::Cache::VariablesMap& localization) throw()
        : var_left_marker_(var_left_marker),
          var_right_marker_(var_right_marker),
          localization_(localization),
          default_encoding_(VE_NONE)
    {
    }

    inline
    void
    EncodingAwareTemplateParseInterceptor::update(
      const std::string& tag,
      const std::string& value,
      El::String::Template::Chunk& chunk) const
      throw(El::String::Template::ParsingFailed,
            El::String::Template::Exception,
            El::Exception)
    {
    }
  }
}

#endif // _ELEMENTS_EL_CACHE_ENCODINGAWARELOCALIZEDTEMPLATE_HPP_
