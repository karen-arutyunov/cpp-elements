/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/InterceptorImpl.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_INTERCEPTORIMPL_HPP_
#define _ELEMENTS_EL_PYTHON_INTERCEPTORIMPL_HPP_

#include <pthread.h>
#include <Python.h>
#include <frameobject.h>

#include <vector>

#include <google/sparse_hash_map>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/Python/Exception.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Interceptor.hpp>

namespace El
{
  namespace Python
  {
    class InterceptorImpl : public Interceptor
    {
    public:
      
      virtual void thread_callback(Callback* callback, unsigned long flags)
        throw();

      virtual Callback* thread_callback() const throw();

      static void install(unsigned long flags)
        throw(Exception, El::Exception);
      
      static void uninstall() throw();
      
      static void* count_alloc(void* ptr, size_t size, void* old_ptr = 0)
        throw();
      
      static void count_free(void* ptr) throw();

      struct Installer
      {
        Installer(unsigned long flags) throw();
        ~Installer() throw();
      };

      static InterceptorImpl instance;
      
    private:
      InterceptorImpl() throw(Exception, El::Exception);
      virtual ~InterceptorImpl() throw() { mem_alloc_intercept_ = false; }

      void on_install(unsigned long flags) throw(Exception, El::Exception);
      void on_uninstall() throw();      

      static PyObject* builtin_import_hook(PyObject *self, PyObject *args)
        throw();

      static int trace_hook(PyObject *obj,
                            PyFrameObject *frame,
                            int what,
                            PyObject *arg) throw();
      
      void register_import_hook() throw(Exception, El::Exception);
      void register_trace_hook(bool on) throw(Exception, El::Exception);
      
      void count_thread_alloc(void* ptr, size_t size, void* old_ptr) throw();
      void count_thread_free(void* ptr) throw();

      PyObject* get_builtins_dict() throw(Exception, El::Exception);

      struct ThreadInfo
      {
        Callback* callback;
        unsigned long flags;
        size_t import_level;
        size_t allocated;

        ThreadInfo(Callback* cl = 0, unsigned long fl = 0) throw();        
      };
      
      bool thread_info(ThreadInfo& info) const throw();
      void update_thread_import_level(bool inc) throw();

    private:
      
      typedef ACE_Recursive_Thread_Mutex Mutex;
      typedef ACE_Guard<Mutex> Guard;
      
      mutable Mutex lock_;

      static bool mem_alloc_intercept_;
      static PyCFunction builtin_import_def_func_;
      
      unsigned long flags_;
      bool mem_counting_;
      PyMethodDef hook_def_builtin_import_;

      class ThreadInfoMap :
        public google::sparse_hash_map<pthread_t,
                                       ThreadInfo,
                                       El::Hash::Numeric<pthread_t> >
      {
      public:
        ThreadInfoMap() throw(El::Exception) { set_deleted_key(0); }
      };

      ThreadInfoMap thread_infos_;

      struct MemThread
      {
        size_t size;
        pthread_t tid;

        MemThread(size_t s = 0, pthread_t t = 0) : size(s), tid(t) {}
      };
      
      class MemThreadMap :
        public google::sparse_hash_map<void*,
                                       MemThread,
                                       El::Hash::Numeric<void*> >
      {
      public:
        MemThreadMap() throw(El::Exception) { set_deleted_key(0); }
      };

      MemThreadMap mem_threads_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {

    //
    // InterceptorImpl class
    //
    inline
    InterceptorImpl::ThreadInfo::ThreadInfo(Callback* cl, unsigned long fl)
      throw()
        : callback(cl),
          flags(fl),
          import_level(0),
          allocated(0)
    {
    }
    
    //
    // InterceptorImpl class
    //
    inline
    InterceptorImpl::Installer::Installer(unsigned long flags) throw()
    {
      El::Python::InterceptorImpl::install(flags);  
    }

    inline
    InterceptorImpl::Installer::~Installer() throw()
    {
      El::Python::InterceptorImpl::uninstall();
    }        

    inline
    Interceptor::Callback*
    InterceptorImpl::thread_callback() const throw()
    {
      pthread_t tid = pthread_self();
      
      Guard guard(lock_);

      ThreadInfoMap::const_iterator i = thread_infos_.find(tid);
      return i == thread_infos_.end() ? 0 : i->second.callback;
    }
    
    inline
    void
    InterceptorImpl::thread_callback(Callback* callback, unsigned long flags)
      throw()
    {
      register_trace_hook(callback != 0);

      pthread_t tid = pthread_self();
      
      Guard guard(lock_);

      mem_counting_ = false;
      
      if(callback)
      {
        thread_infos_[tid] = ThreadInfo(callback, flags);
      }
      else
      {
        thread_infos_.erase(tid);

        for(MemThreadMap::iterator i(mem_threads_.begin()),
              e(mem_threads_.end()); i != e; ++i)
        {
          if(i->second.tid == tid)
          {
            mem_threads_.erase(i);
          }
        }
      }

      mem_counting_ = true;
    }

    inline
    bool
    InterceptorImpl::thread_info(ThreadInfo& info) const throw()
    {
      pthread_t tid = pthread_self();

      Guard guard(lock_);
      
      ThreadInfoMap::const_iterator i = thread_infos_.find(tid);

      if(i == thread_infos_.end())
      {
        info = ThreadInfo();
        return false;
      }

      info = i->second;
      return true;
    }
    
    inline
    void
    InterceptorImpl::update_thread_import_level(bool inc) throw()
    {
      pthread_t tid = pthread_self();

      Guard guard(lock_);
      
      ThreadInfoMap::iterator i = thread_infos_.find(tid);

      if(i != thread_infos_.end())
      {
        if(inc)
        {
          ++(i->second.import_level);
        }
        else
        {
          --(i->second.import_level);
        }
      }
    }

    inline
    void
    InterceptorImpl::count_thread_alloc(void* ptr, size_t size, void* old_ptr)
      throw()
    {
      size = std::max(size, (size_t)32);
      pthread_t tid = pthread_self();

      Guard guard(lock_);

      if(old_ptr)
      {
        count_thread_free(old_ptr);
      }      

      if(mem_counting_)
      {
        mem_counting_ = false;
        
        ThreadInfoMap::iterator i = thread_infos_.find(tid);

        if(i != thread_infos_.end())
        {
          i->second.allocated += size;          
          mem_threads_[ptr] = MemThread(size, tid);
        }

        mem_counting_ = true;
      } 
    }
    
    inline
    void
    InterceptorImpl::count_thread_free(void* ptr) throw()
    {
      Guard guard(lock_);

      if(mem_counting_)
      {
        mem_counting_ = false;        
      
        MemThreadMap::iterator i = mem_threads_.find(ptr);
        
        if(i != mem_threads_.end())
        {
          const MemThread& mem = i->second;
          ThreadInfoMap::iterator j = thread_infos_.find(mem.tid);
          
          if(j != thread_infos_.end())
          {
            ThreadInfo& counter = j->second;
            counter.allocated -= std::min(counter.allocated, mem.size);
          }

          mem_threads_.erase(i);
        }

        mem_counting_ = true;        
      }
    }    
    
    inline
    void*
    InterceptorImpl::count_alloc(void* ptr, size_t size, void* old_ptr) throw()
    {
      if(mem_alloc_intercept_ && ptr && instance.flags_)
      {
        instance.count_thread_alloc(ptr, size, old_ptr);
      }

      return ptr;
    }
    
    inline
    void
    InterceptorImpl::count_free(void* ptr) throw()
    {
      if(mem_alloc_intercept_ && ptr && instance.flags_)
      {
        instance.count_thread_free(ptr);
      }
    }    
  }
}

#endif // _ELEMENTS_EL_PYTHON_INTERCEPTORIMPL_HPP_
