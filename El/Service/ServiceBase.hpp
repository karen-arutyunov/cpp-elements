/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Service/ServiceBase.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_SERVICE_SERVICEBASE_HPP_
#define _ELEMENTS_EL_SERVICE_SERVICEBASE_HPP_

#include <pthread.h>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/ArrayPtr.hpp>
#include <El/SyncPolicy.hpp>

#include <El/Service/Service.hpp>

namespace El
{
  namespace Service
  {
    template<typename SynchPolicy>    
    class ServiceBase : public virtual Service
    {
    public:
      ServiceBase(Callback* callback,
                  const char* name = 0,
                  unsigned long threads = 1,
                  size_t stack_size = 0)
        throw(InvalidArg, El::Exception);

      virtual ~ServiceBase() throw();
      
      virtual bool start() throw(Exception, El::Exception);
      virtual bool stop() throw(Exception, El::Exception);
      virtual void wait() throw(Exception, El::Exception);
      
      virtual bool started() throw(Exception, El::Exception);

      virtual const char* name() const throw(El::Exception);

    protected:
      virtual void run() throw(Exception, El::Exception) = 0;

      static void* thread_func(void* args) throw();
      void exec_thread_func() throw();

    protected:
      
      typedef typename SynchPolicy::Mutex Mutex;
      typedef typename SynchPolicy::ReadGuard ReadGuard;
      typedef typename SynchPolicy::WriteGuard WriteGuard;

      mutable Mutex srv_lock_;
      
      Callback* callback_;
      std::string name_;
      unsigned long threads_;
      size_t stack_size_;

      typedef El::ArrayPtr<pthread_t> ThreadHandleArray;
      ThreadHandleArray thread_handles_;

      unsigned long started_;
      bool stop_;
    };
  }
  
}


///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Service
  {
    template<typename SynchPolicy>    
    const char*
    ServiceBase<SynchPolicy>::name() const throw(El::Exception)
    {
      return name_.c_str();
    }
    
    template<typename SynchPolicy>    
    bool
    ServiceBase<SynchPolicy>::started() throw(Exception, El::Exception)
    {
      ReadGuard guard(srv_lock_);
      return started_ && !stop_;
    }

    template<typename SynchPolicy>    
    void*
    ServiceBase<SynchPolicy>::thread_func(void* args) throw()
    {
      reinterpret_cast<ServiceBase*>(args)->exec_thread_func();
      return 0;
    }
    
    template<typename SynchPolicy>    
    ServiceBase<SynchPolicy>::ServiceBase(Callback* callback,
                                          const char* name,
                                          unsigned long threads,
                                          size_t stack_size)
      throw(InvalidArg, El::Exception)
        : callback_(callback),
          name_(name ? name : ""),
          threads_(threads),
          stack_size_(stack_size),
          started_(0),
          stop_(true)
    {
      if(!threads_)
      {
        throw InvalidArg("El::Service::ServiceBase::ServiceBase: "
                         "threads number should be positive");
      }
      
    }
    
    template<typename SynchPolicy>    
    ServiceBase<SynchPolicy>::~ServiceBase() throw()
    {
    }
    
    template<typename SynchPolicy>    
    bool
    ServiceBase<SynchPolicy>::start() throw(Exception, El::Exception)
    {
      WriteGuard guard(srv_lock_);

      if(started_)
      {
        return false;
      }

      pthread_attr_t attr;
      pthread_attr_t* pattr = 0;

      try
      {
        if(stack_size_)
        {
          if(pthread_attr_init(&attr))
          {
            int error = ACE_OS::last_error();
            
            std::ostringstream ostr;            
            ostr << "El::Service::ServiceBase::start: "
              "pthread_attr_init failed. Reason: "
                 << ACE_OS::strerror(error);
            
            throw Exception(ostr.str()); 
          }
          
          pattr = &attr;
          
          if(pthread_attr_setstacksize(&attr, stack_size_))
          {
            int error = ACE_OS::last_error();
            
            std::ostringstream ostr;            
            ostr << "El::Service::ServiceBase::start: "
              "pthread_attr_setstacksize failed. Reason: " <<
              ACE_OS::strerror(error);
            
            throw Exception(ostr.str()); 
          }
        }

        ThreadHandleArray thread_handles(new pthread_t[threads_]);        

        for(unsigned long i = 0; i < threads_; i++)
        {
          if(pthread_create(&thread_handles[i],
                            pattr,
                            thread_func,
                            reinterpret_cast<void*>(this)))
          {
            int error = ACE_OS::last_error();
            
            std::ostringstream ostr;
            ostr << "El::Service::ServiceBase::start: "
              "pthread_create failed. Reason: " <<
              ACE_OS::strerror(error);

            guard.release(); // To allow threads to see stop_ flag
            
            for(unsigned long j = 0; j < i; j++)
            {
              pthread_join(thread_handles[j], NULL);
            }
            
            throw Exception(ostr.str());
          }

        }
        
        thread_handles_ = thread_handles;

        if(pattr)
        {
          pthread_attr_destroy(pattr);
        }
      }
      catch(...)
      {
        if(pattr)
        {
          pthread_attr_destroy(pattr);
        }
        
        throw;
      }
      
      started_ = threads_;  
      stop_ = false;
      
      return true;
    }
    
    template<typename SynchPolicy>    
    bool
    ServiceBase<SynchPolicy>::stop() throw(Exception, El::Exception)
    {
      WriteGuard guard(srv_lock_);

      if(!started_ || stop_)
      {
        return false;
      }

      stop_ = true;
      return true;
    }
    
    template<typename SynchPolicy>    
    void
    ServiceBase<SynchPolicy>::wait() throw(Exception, El::Exception)
    {
      ThreadHandleArray thread_handles;
      unsigned long threads = 0;
      
      {  
        WriteGuard guard(srv_lock_);

        if(thread_handles_.get() == 0)
        {
          return;
        }
        
        thread_handles = thread_handles_;
        threads = threads_;
      }

      for(unsigned long i = 0; i < threads; i++)
      {
        if(pthread_join(thread_handles[i], 0))
        {
          int error = ACE_OS::last_error();
          
          std::ostringstream ostr;            
          ostr << "El::Service::ServiceBase::wait: "
            "ACE_Thread::join failed. Reason: "
               << ACE_OS::strerror(error);

          throw Exception(ostr.str());
        }
      }

      ReadGuard guard(srv_lock_);

      if(started_)
      {
        std::ostringstream ostr;            
        ostr << "El::Service::ServiceBase::wait: "
          "started_ is unexpectedly equal " << started_;

        throw Exception(ostr.str());
      }
    }
      
    template<typename SynchPolicy>    
    void
    ServiceBase<SynchPolicy>::exec_thread_func() throw()
    {
      bool finalize = true;
      
      try
      {
        try
        {
          {
            ReadGuard guard(srv_lock_);
            
            if(!started_)
            {
              // start method failed
              return;
            }
          }
          
          run();

          finalize = false;

          WriteGuard guard(srv_lock_);

          if(!started_)
          {
            Error error("El::Service::ServiceBase::exec_thread_func: "
                        "started_ is unexpectedly equal 0", this);
            
            callback_->notify(&error);
          }
          else
          {
            started_--;
          }
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Service::ServiceBase::exec_thread_func: "
            "El::Exception caught. Description:\n" << e;
          
          Error error(ostr.str(), this);
          callback_->notify(&error);

          if(finalize)
          {
            finalize = false;
            
            WriteGuard guard(srv_lock_);

            if(!started_)
            {
              Error error("El::Service::ServiceBase::exec_thread_func: "
                          "started_ is unexpectedly equal 0", this);
            
              callback_->notify(&error);
            }
            else
            {
              started_--;
            }
          }
          
        }
      }
      catch(...)
      {
        Error error("El::Service::ServiceBase::exec_thread_func: "
                    "unknown exception caught", this);

        try
        {
          callback_->notify(&error);
        }
        catch(...)
        {
        }
      }
      
    }    
    
  }
}

#endif // _ELEMENTS_EL_SERVICE_SERVICEBASE_HPP_
