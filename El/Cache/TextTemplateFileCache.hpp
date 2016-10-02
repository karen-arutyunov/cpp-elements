/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Cache/TextTemplateFileCache.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CACHE_TEXTTEMPLATEFILECACHE_HPP_
#define _ELEMENTS_EL_CACHE_TEXTTEMPLATEFILECACHE_HPP_

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/String/Template.hpp>

#include <El/Cache/ObjectCache.hpp>
#include <El/Cache/TextFileCache.hpp>

namespace El
{
  namespace Cache
  {
    class TextTemplateFileCache;
    
    class TextTemplateFile :
      public virtual Object,
      public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
    {
    public:
      TextTemplateFile(Container* container,
                       unsigned long long sequence_number,
                       const char* filename)
        throw(Exception, El::Exception);
      
      ~TextTemplateFile() throw();

      std::string instantiate(const El::String::Template::Variables& variables,
                              bool lax = false)
        const throw(El::String::Template::VariableNotFound, El::Exception);
      
      void instantiate(const El::String::Template::Variables& variables,
                       std::ostream& output,
                       bool lax = false) const
        throw(El::String::Template::VariableNotFound, El::Exception);

      void parse(const char* template_text)
        throw(Exception,
              El::String::Template::InvalidArgument,
              El::String::Template::ParsingFailed,
              El::Exception);

      const El::String::Template::Parser& parser() const throw();
      
    protected:
      virtual void read_chunk(const unsigned char* buff, size_t size)
        throw(Exception, El::Exception);

      virtual void reserve(size_t size) throw(Exception, El::Exception);

    protected:
      TextTemplateFileCache* container_;
      TextFile_var text_file_;
      El::String::Template::Parser parser_;
    };

    typedef El::RefCount::SmartPtr<TextTemplateFile> TextTemplateFile_var;

    class TextTemplateFileCache : public virtual FileCache<TextTemplateFile>
    {
    public:
      TextTemplateFileCache(
        const char* var_left_marker,
        const char* var_right_marker,
        ACE_Time_Value review_filetime_period =
        ACE_Time_Value::zero,
        const El::String::Template::ParseInterceptor* interceptor = 0)
        throw(El::Exception);
      
      virtual ~TextTemplateFileCache() throw();

      const char* var_left_marker() const throw(El::Exception);
      const char* var_right_marker() const throw(El::Exception);

      const El::String::Template::ParseInterceptor* parse_interceptor() const
        throw();
      
    protected:
      std::string var_left_marker_;
      std::string var_right_marker_;
      const El::String::Template::ParseInterceptor* interceptor_;
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
    // TextTemplateFile class
    //
    inline
    TextTemplateFile::TextTemplateFile(Container* container,
                                       unsigned long long sequence_number,
                                       const char* filename)
      throw(Exception, El::Exception)
        : Object(sequence_number),
          container_(dynamic_cast<TextTemplateFileCache*>(container))
    {
      if(container_ == 0)
      {
        throw Exception("El::Cache::TextTemplateFileCache::"
                        "TextTemplateFileCache: invalid container");
      }
      
      text_file_ = new TextFile(container, sequence_number, filename);
    }

    inline
    const El::String::Template::Parser&
    TextTemplateFile::parser() const throw()
    {
      return parser_;
    }
    
    inline
    TextTemplateFile::~TextTemplateFile() throw()
    {
    }
    
    inline
    void
    TextTemplateFile::read_chunk(const unsigned char* buff,
                                 size_t size)
      throw(Exception, El::Exception)
    {
      text_file_->read_chunk(buff, size);
      
      if(size == 0)
      {
        parse(text_file_->text());
        text_file_ = 0;
      }
    }
    
    inline
    void
    TextTemplateFile::parse(const char* template_text)
      throw(Exception,
            El::String::Template::InvalidArgument,
            El::String::Template::ParsingFailed,
            El::Exception)
    {
      parser_.parse(template_text,
                    container_->var_left_marker(),
                    container_->var_right_marker(),
                    container_->parse_interceptor());
    }
    
    inline
    void
    TextTemplateFile::reserve(size_t size) throw(Exception, El::Exception)
    {
      text_file_->reserve(size);
    }
    
    inline
    std::string
    TextTemplateFile::instantiate(
      const El::String::Template::Variables& variables,
      bool lax)
      const throw(El::String::Template::VariableNotFound, El::Exception)
    {
      return parser_.instantiate(variables, lax);
    }
      
    inline
    void
    TextTemplateFile::instantiate(
      const El::String::Template::Variables& variables,
      std::ostream& output,
      bool lax) const
      throw(El::String::Template::VariableNotFound, El::Exception)
    {
      parser_.instantiate(variables, output, lax);
    }

    //
    // TextTemplateFileCache class
    //
    inline
    TextTemplateFileCache::TextTemplateFileCache(
      const char* var_left_marker,
      const char* var_right_marker,
      ACE_Time_Value review_filetime_period,
      const El::String::Template::ParseInterceptor* interceptor)
      throw(El::Exception)
        : FileCache<TextTemplateFile>(review_filetime_period),
          var_left_marker_(var_left_marker),
          var_right_marker_(var_right_marker),
          interceptor_(interceptor)
    {
    }

    inline
    TextTemplateFileCache::~TextTemplateFileCache() throw()
    {
    }
    
    inline
    const char*
    TextTemplateFileCache::var_left_marker() const throw(El::Exception)
    {
      return var_left_marker_.c_str();
    }

    inline
    const char*
    TextTemplateFileCache::var_right_marker() const throw(El::Exception)
    {
      return var_right_marker_.c_str();
    }
    
    inline
    const El::String::Template::ParseInterceptor*
    TextTemplateFileCache::parse_interceptor() const throw()
    {
      return interceptor_;
    }
    
  }
}

#endif // _ELEMENTS_EL_CACHE_TEXTTEMPLATEFILECACHE_HPP_

