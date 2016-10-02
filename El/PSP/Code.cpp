/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Code.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <sstream>
#include <memory>
#include <utility>
#include <string>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/Net/HTTP/Headers.hpp>
#include <El/Python/RefCount.hpp>
#include <El/PSP/Context.hpp>

#include "Code.hpp"

#include <httpd/http_core.h>

namespace El
{
  namespace PSP
  {
    const unsigned long PSP_CODE_REQUEST_CALLBACK_ID = 1;

    //
    // ThreadSwitcher class
    //
    class ThreadSwitcher :
      virtual public El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>,
      virtual public El::Apache::Request::Callback
    {
    public:
        
      ThreadSwitcher() throw() {}
      virtual ~ThreadSwitcher() throw() {}
        
      virtual void content_type(const char* value) throw(El::Exception) {}

      virtual void send_header(const char* name, const char* value)
        throw(El::Exception) {}
        
      virtual void start_writing(bool deflate) throw(El::Exception) {}
        
      virtual void on_write(const char* buffer, size_t size)
        throw(El::Exception) {}

      virtual void pre_ap_rwrite() throw();
      virtual void post_ap_rwrite() throw();
      
      virtual void finalize(int result) throw(El::Exception) {}

    private:
      std::auto_ptr<El::Python::AllowOtherThreads> guard_;

    private:
      ThreadSwitcher(const ThreadSwitcher&);
      void operator=(const ThreadSwitcher&);
    };

    typedef El::RefCount::SmartPtr<ThreadSwitcher> ThreadSwitcher_var;

    void
    ThreadSwitcher::pre_ap_rwrite() throw()
    {
//      std::cerr << "AAA1\n";
      guard_.reset(new El::Python::AllowOtherThreads());
//      std::cerr << "AAA2\n";
    }
    
    void
    ThreadSwitcher::post_ap_rwrite() throw()
    {
//      std::cerr << "BBB1\n";
      guard_.reset(0);
//      std::cerr << "BBB2\n";
    }
    
    //
    // El::PSP::Code class
    //
    PyObject*
    Code::run(El::Apache::Request& request,
              CodeGuard& code_guard,
              const El::Lang& lang,
              const El::PSP::Request::LangMap& valid_languages,
              El::PSP::Config* config,
              El::PSP::Localization* localization,
              PyObject* forward_ags,
              const ObjectMap* objects,
              El::Cache::VariablesMapCache& localization_cache)
      throw(El::Cache::Exception, El::Python::Exception, El::Exception)
    {        
      El::Python::Object_var dictionary = PyDict_New();

      if(dictionary.in() == 0)
      {
        El::Python::handle_error(
          "El::PSP::Code::run: PyDict_New failed");
      }

      try
      {
        ThreadSwitcher_var switcher = new ThreadSwitcher();
        request.callback(switcher.in(), PSP_CODE_REQUEST_CALLBACK_ID);
        
        El::PSP::Request_var req =
          new El::PSP::Request(request,
                               lang,
                               valid_languages,
                               localization);
        
        Context_var context = new Context(this,
                                          req.in(),
                                          config,
                                          localization,
                                          &localization_cache,
                                          valid_languages,
                                          forward_ags,
                                          cache_.in(),
                                          dictionary.in());
        
        if(objects)
        {
          for(ObjectMap::const_iterator it = objects->begin();
              it != objects->end(); it++)
          {
            if(PyDict_SetItemString(dictionary.in(),
                                    it->first.c_str(),
                                    it->second.in()))
            {
              El::Python::handle_error(
                "El::PSP::Code::run: PyDict_SetItemString[1] failed");
            }
          }
        }
        
        for(ObjectMap::iterator it = static_vars_.begin();
            it != static_vars_.end(); it++)
        {
          if(PyDict_SetItemString(dictionary.in(),
                                  it->first.c_str(),
                                  it->second.in()))
          {
            El::Python::handle_error(
              "El::PSP::Code::run: PyDict_SetItemString[2] failed");
          }
        }

        // TODO: fix leak in this call !
        if(PyDict_SetItemString(dictionary.in(), "context", context.in()))
        {
          El::Python::handle_error(
            "El::PSP::Code::run: PyDict_SetItemString[3] failed");
        }

        PyObject* module = PyImport_AddModule("__main__");

        if(module == 0)
        {
          El::Python::handle_error(
            "El::PSP::Code::run: PyImport_AddModule failed");
        }

        PyObject* main_global_dict = PyModule_GetDict(module);

        //
        // Copying to dictionary builtins, name, doc string, imported modules
        //
        if(PyDict_Merge(dictionary.in(), main_global_dict, 0) < 0)
        {
          El::Python::handle_error(
            "El::PSP::Code::run: PyDict_Merge failed");
        }
      
        context->run_number(run_number_++);

        if(run_number_ > 1)
        {
          code_guard.release();
        }

        El::Python::Object_var result =
          code_.run(dictionary.in(), dictionary.in());

        request.callback(0, PSP_CODE_REQUEST_CALLBACK_ID);        
        PyDict_Clear(dictionary.in());

        return result.retn();
      }
      catch(...)
      {
        request.callback(0, PSP_CODE_REQUEST_CALLBACK_ID);
        PyDict_Clear(dictionary.in());
        throw;
      }
    }

    //
    // El::PSP::CodeCache class
    //

    CodeCache::~CodeCache() throw()
    {
      for(CodeMutexMap::iterator it = code_locks_.begin();
          it != code_locks_.end(); ++it)
      {
        delete it->second;
      }
    }

    Code::CodeMutex&
    CodeCache::code_lock(const char* code_path) throw(El::Exception)
    {
      {
        ReadGuard guard(lock_);

        CodeMutexMap::iterator it = code_locks_.find(code_path);
        
        if(it != code_locks_.end())
        {
          return *it->second;
        }        
      }

      WriteGuard guard(lock_);
      
      CodeMutexMap::iterator it = code_locks_.find(code_path);
        
      if(it == code_locks_.end())
      {
        it = code_locks_.insert(std::make_pair(code_path,
                                               new Code::CodeMutex)).first;
      }
      
      return *it->second;
    }
    
    PyObject*
    CodeCache::run(const char* code_path,
                   Code::CodeGuard& code_guard,
                   El::Apache::Request& request,
                   const El::Lang& lang,
                   const El::PSP::Request::LangMap& valid_languages,
                   El::PSP::Config* config,
                   PyObject* forward_ags,
                   const Code::ObjectMap* objects,
                   ETagCalc etag_calc)
      throw(El::Cache::Exception, El::Python::Exception, El::Exception)
    {      
      El::PSP::Code_var code = get(code_path);

      El::Cache::VariablesMap_var loc;
      El::PSP::Localization_var localization;

      if(lang != El::Lang::null)
      {
        loc = localizations_.get(code_path, lang);
      }
      
      localization = new El::PSP::Localization(loc.in());

      if(etag_calc == ETC_STATIC)
      {
        unsigned long long code_hash = code->hash();

        std::string code_hash_b64;
        code_hash_b64.reserve(100);
      
        El::String::Manip::base64_encode((const unsigned char*)&code_hash,
                                         sizeof(code_hash),
                                         code_hash_b64);
        if(loc)
        {
          unsigned long long loc_hash = loc->hash();

          std::string loc_hash_b64;

          El::String::Manip::base64_encode((const unsigned char*)&loc_hash,
                                           sizeof(loc_hash),
                                           loc_hash_b64);

          code_hash_b64 += ".";
          code_hash_b64 += loc_hash_b64;
        }

        const char* current_etag =
          request.in().headers().find(El::Net::HTTP::HD_IF_NONE_MATCH);
        
          
        if(current_etag && code_hash_b64 == current_etag)
        {
          return PyInt_FromLong(HTTP_NOT_MODIFIED);
        }                

        request.out().send_header(El::Net::HTTP::HD_ETAG,
                                  code_hash_b64.c_str());       
      }

      return code->run(request,
                       code_guard,
                       lang,
                       valid_languages,
                       config,
                       localization.in(),
                       forward_ags,
                       objects,
                       localizations_);
    }
    
  }
}
