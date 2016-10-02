/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Service/ProcessPool.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PROCESSPOOL_HPP_
#define _ELEMENTS_EL_PROCESSPOOL_HPP_

#include <limits.h>

#include <stdexcept>
#include <string>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/SyncPolicy.hpp>
#include <El/BinaryStream.hpp>
#include <El/Queue.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ServiceBase.hpp>

namespace El
{
  namespace Service
  {
    class ProcessPool :
      public virtual ServiceBase<El::Sync::ThreadRWPolicy>,
      public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
    {
    public:
      EL_EXCEPTION(Exception, El::Service::Exception);
      EL_EXCEPTION(InvalidArg, Exception);

    public:
      
      class Task : public virtual El::RefCount::Interface
      {
      public:
        virtual ~Task() throw();
        
        virtual const char* type_id() const throw(El::Exception) = 0;
        virtual void execute() throw(El::Exception) = 0;

        // If true then task should be executed by all means,
        // otherwise it can be dropped if not yet executed at the moment
        // of stopping ProcessPool
        virtual bool execution_required() throw(El::Exception) = 0;

        virtual void error(const char* str) throw(El::Exception) = 0;
        virtual std::string error() const throw(El::Exception) = 0;

        virtual void timeout_occured(bool val) throw() = 0;
        virtual bool timeout_occured() const throw() = 0;

        virtual void write_arg(El::BinaryOutStream& bstr) const
          throw(El::Exception) = 0;
        
        virtual void read_arg(El::BinaryInStream& bstr)
          throw(El::Exception) = 0;

        virtual void write_res(El::BinaryOutStream& bstr)
          throw(El::Exception) = 0;
        
        virtual void read_res(El::BinaryInStream& bstr)
          throw(El::Exception) = 0;

        virtual void wait() throw(El::Exception) = 0;
        virtual void signal_completion() throw(El::Exception) = 0;
      };

      typedef El::RefCount::SmartPtr<Task> Task_var;
      typedef El::Queue<Task_var> TaskQueue;

      struct TaskFactoryInterface
      {
        typedef std::exception Exception;
      
        virtual ~TaskFactoryInterface() throw() {}

        // If return 0 or "" object considered to be fully functional.
        virtual const char* creation_error() throw() = 0;

        // Should destroy an object
        virtual void release() throw(Exception) = 0;

        virtual Task* create_task(const char* id) throw(El::Exception) = 0;
      };      

      class TaskBase :
        public virtual Task,
        public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
      {
      public:
        TaskBase(bool execution_required) throw(El::Exception);
        virtual ~TaskBase() throw() {}

        virtual bool execution_required() throw(El::Exception);

        virtual void error(const char* str) throw(El::Exception);        
        virtual std::string error() const throw(El::Exception);

        virtual void timeout_occured(bool val) throw();
        virtual bool timeout_occured() const throw();
        
        virtual void wait() throw(El::Exception);
        virtual void signal_completion() throw(El::Exception);
        
      protected:
        typedef ACE_Condition<ACE_Thread_Mutex> Condition;

        Condition completed_signal_;
        bool completed_;
        bool execution_required_;
        bool timeout_occured_;
        std::string error_;
      };

    public:
      
      ProcessPool(
        Callback* callback,
        const char* factory_lib,
        const char* factory_func,
        const char* factory_args = 0,
        const char* extra_libs = 0,
        const char* name = 0,
        unsigned long processes = 1,
        unsigned long timeout = 0, // msec
        size_t max_result_len = SIZE_MAX,
        size_t queue_size = SIZE_MAX,
        TaskQueue::EnqueueStrategy enqueue_strategy = TaskQueue::ES_BACK)
        throw(InvalidArg, El::Exception);

      virtual ~ProcessPool() throw();

      bool execute(
        Task* task,
        const ACE_Time_Value* wait_time = 0,
        TaskQueue::EnqueueStrategy enqueue_strategy = TaskQueue::ES_DEFAULT)
        throw(InvalidArg, El::Exception);

      virtual bool stop() throw(Exception, El::Exception);
      virtual void wait() throw(Exception, El::Exception);

      size_t queue_size() const throw() { return tasks_.size(); }
      
    private:
      
      virtual void run() throw(Exception, El::Exception);

      pid_t create_process(bool initial,
                           int& input,
                           int& output,
                           std::string& error)
        throw(Exception, El::Exception);

      bool write(int fd,
                 const void* buff,
                 size_t count,
                 std::string& error,
                 bool& timeout_occured) const throw(El::Exception);
    
      bool read(int fd,
                void* buff,
                size_t count,
                std::string& error,
                bool& timeout_occured) const
        throw(El::Exception);
      
      bool send_command(const std::string& data,
                        int output,
                        int input,
                        std::string& result,
                        bool& timeout_occured) const
        throw(El::Exception);

      static bool check_error_result(El::BinaryInStream& bstr,
                                     std::string& error)
        throw(Exception, El::Exception);
      
      bool execute(Task* task,
                   int input,
                   int output,
                   bool& timeout_occured) const
        throw(El::Exception);

      bool close_process(bool regular,
                         pid_t& pid,
                         int& input,
                         int& output,
                         bool timeout_occured,
                         std::string& error)
        throw(El::Exception);

    protected:
      std::string factory_lib_;
      std::string factory_func_;
      std::string factory_args_;
      std::string extra_libs_;
      
      TaskQueue tasks_;
      size_t queue_size_;
      TaskQueue::EnqueueStrategy enqueue_strategy_;
      int timeout_;
      size_t max_result_len_;
    };

    typedef El::RefCount::SmartPtr<ProcessPool> ProcessPool_var;    
  }
}

typedef ::El::Service::ProcessPool::TaskFactoryInterface*
(* CreateProcessPoolTaskFactory)(const char* args);

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Service
  {
    //
    // ProcessPool class
    //
    inline
    ProcessPool::ProcessPool(Callback* callback,
                             const char* factory_lib,
                             const char* factory_func,
                             const char* factory_args,
                             const char* extra_libs,
                             const char* name,
                             unsigned long processes,
                             unsigned long timeout,
                             size_t max_result_len,
                             size_t queue_size,
                             TaskQueue::EnqueueStrategy enqueue_strategy)
      throw(InvalidArg, El::Exception)
        : ServiceBase<El::Sync::ThreadRWPolicy>(callback,
                                                name,
                                                processes,
                                                1024 * 1024),
          factory_lib_(factory_lib),
          factory_func_(factory_func),
          factory_args_(factory_args ? factory_args : ""),
          extra_libs_(extra_libs ? extra_libs : ""),
          tasks_(queue_size),
          queue_size_(queue_size),
          enqueue_strategy_(enqueue_strategy),
          timeout_(timeout ? timeout : -1),
          max_result_len_(max_result_len)
    {
      if(queue_size == 0)
      {
        throw Exception(
          "El::Service::ProcessPool::ProcessPool: queue_size is 0");
      }      
    }
    
    inline
    ProcessPool::~ProcessPool() throw()
    {
    }

    inline
    bool
    ProcessPool::stop() throw(Exception, El::Exception)
    {
      bool ret = ServiceBase<El::Sync::ThreadRWPolicy>::stop();

      tasks_.max_size(0);
      tasks_.awake();
      return ret;
    }

    inline
    void
    ProcessPool::wait() throw(Exception, El::Exception)
    {
      ServiceBase<El::Sync::ThreadRWPolicy>::wait();
      
      //
      // Need to restore task queue max_size as one consequence of
      // thread pool stopping is setting max_size to zero.
      // Otherwise the object would be unusable after started again.
      //
      tasks_.max_size(queue_size_);
    }
    
    //
    // ProcessPool::Task class
    //
    inline
    ProcessPool::Task::~Task() throw()
    {
    }

    //
    // ProcessPool::TaskBase class
    //
    inline
    ProcessPool::TaskBase::TaskBase(bool execution_required)
      throw(El::Exception)
        : completed_signal_(lock_),
          completed_(false),
          execution_required_(execution_required),
          timeout_occured_(false)
    {    
    }
    
    inline
    bool
    ProcessPool::TaskBase::execution_required() throw(El::Exception)
    {
      return execution_required_;
    }

    inline
    void
    ProcessPool::TaskBase::error(const char* str)
      throw(El::Exception)
    {
      WriteGuard_ guard(lock_);
      error_ = str;
    }

    inline
    std::string
    ProcessPool::TaskBase::error() const throw(El::Exception)
    {
      ReadGuard_ guard(lock_);
      return error_;
    }

    inline
    void
    ProcessPool::TaskBase::timeout_occured(bool val)throw()
    {
      WriteGuard_ guard(lock_);
      timeout_occured_ = val;
    }

    inline
    bool
    ProcessPool::TaskBase::timeout_occured() const throw()
    {
      ReadGuard_ guard(lock_);
      return timeout_occured_;
    }

    inline
    void
    ProcessPool::TaskBase::wait() throw(El::Exception)
    {
      ReadGuard_ guard(lock_);

      while(!completed_)
      {
        if(completed_signal_.wait(0))
        {
          int error = ACE_OS::last_error();
          
          std::ostringstream ostr;
          ostr << "ProcessPool::TaskBase::wait: "
            "completed_signal_.wait() failed. "
            "Errno " << error << ". Description:" << std::endl
               << ACE_OS::strerror(error);
          
          throw Exception(ostr.str());
        }
      }
    }
    
    inline
    void
    ProcessPool::TaskBase::signal_completion() throw(El::Exception)
    {
      WriteGuard_ guard(lock_);
      completed_ = true;
      completed_signal_.signal();
    }
  }
}

#endif // _ELEMENTS_EL_PROCESSPOOL_HPP_
