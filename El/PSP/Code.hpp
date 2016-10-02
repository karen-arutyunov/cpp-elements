/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Code.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_CODE_HPP_
#define _ELEMENTS_EL_PSP_CODE_HPP_

#include <string>
#include <map>

#include <ext/hash_map>

#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>

#include <El/Cache/ObjectCache.hpp>
#include <El/Cache/TextFileCache.hpp>

#include <El/Apache/Request.hpp>

#include <El/Python/Code.hpp>

#include <El/PSP/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/PSP/Request.hpp>
#include <El/PSP/Localization.hpp>
#include <El/PSP/CommonTypes.hpp>
#include <El/PSP/Config.hpp>

namespace El
{
  namespace PSP
  {
    class Context;

    class Code : public El::Cache::TextFile
    {
    public:
      
      Code(El::Cache::Container* container,
           unsigned long long sequence_number,
           const char* filename)
        throw(El::Cache::Exception, El::Exception);

      virtual ~Code() throw() {}

      const char* filename() const throw() { return filename_.c_str(); }

      typedef std::map<std::string, El::Python::Object_var> ObjectMap;

      typedef ACE_Thread_Mutex CodeMutex;
      typedef ACE_Guard<CodeMutex> CodeGuard;      
      
      PyObject* run(El::Apache::Request& request,
                    CodeGuard& code_guard,
                    const El::Lang& lang,
                    const El::PSP::Request::LangMap& valid_languages,
                    El::PSP::Config* config,
                    El::PSP::Localization* localization,
                    PyObject* forward_ags,
                    const ObjectMap* objects,
                    El::Cache::VariablesMapCache& localization_cache)
        throw(El::Cache::Exception, El::Python::Exception, El::Exception);

    protected:
      
      virtual void read_chunk(const unsigned char* buff, size_t size)
        throw(El::Cache::Exception, El::Exception);

    private:

      friend class El::PSP::Context;
      
      std::string filename_;      
      El::Python::Code code_;
      El::Python::Object_var cache_;
      ObjectMap static_vars_;
      unsigned long long run_number_;
    };

    typedef El::RefCount::SmartPtr<Code> Code_var;

    class CodeCache : public virtual El::Cache::FileCache<Code>
    {
    public:
      CodeCache(ACE_Time_Value review_filetime_period = ACE_Time_Value::zero)
        throw(El::Exception);
      
      virtual ~CodeCache() throw();

      PyObject* run(const char* code_path,
                    Code::CodeGuard& code_guard,
                    El::Apache::Request& request,
                    const El::Lang& lang,
                   const El::PSP::Request::LangMap& valid_languages,
                    El::PSP::Config* config,
                    PyObject* forward_ags,
                    const Code::ObjectMap* objects,
                    ETagCalc etag_calc)
        throw(El::Cache::Exception, El::Python::Exception, El::Exception);

      Code::CodeMutex& code_lock(const char* code_path) throw(El::Exception);
        
    private:
      
      typedef __gnu_cxx::hash_map<std::string,
                                  Code::CodeMutex*,
                                  El::Hash::String> CodeMutexMap;
      
      El::Cache::VariablesMapCache localizations_;
      CodeMutexMap code_locks_;
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
    // Code class
    //
    inline
    Code::Code(El::Cache::Container* container,
               unsigned long long sequence_number,
               const char* filename)
      throw(El::Cache::Exception, El::Exception)
        : El::Cache::Object(sequence_number),
          El::Cache::BinaryFile(container, sequence_number, filename),
          El::Cache::TextFile(container, sequence_number, filename),
          filename_(filename),
          run_number_(0)
    {
      cache_ = PyDict_New();

      if(cache_.in() == 0)
      {
        El::Python::handle_error(
          "El::PSP::Code::Code: PyDict_New failed");
      }        
    }
    
    inline
    void
    Code::read_chunk(const unsigned char* buff, size_t size)
      throw(El::Cache::Exception, El::Exception)
    {
      El::Cache::TextFile::read_chunk(buff, size);

      if(!size)
      {
        code_.compile(text(), filename_.c_str());
      }
    }
    
    //
    // CodeCache class
    //
    inline
    CodeCache::CodeCache(ACE_Time_Value review_filetime_period)
      throw(El::Exception)
        : El::Cache::FileCache<Code>(review_filetime_period)
    {
    }
  }
}

#endif // _ELEMENTS_EL_PSP_CODE_HPP_
