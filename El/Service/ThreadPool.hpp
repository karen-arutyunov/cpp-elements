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
 * @file Elements/El/Service/ThreadPool.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_THREADPOOL_HPP_
#define _ELEMENTS_EL_THREADPOOL_HPP_

#include <limits.h>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/SyncPolicy.hpp>

#include <El/Queue.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ServiceBase.hpp>

namespace El
{
  namespace Service
  {
    class ThreadPool :
      public virtual ServiceBase<El::Sync::ThreadRWPolicy>,
      public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
    {
    public:
      EL_EXCEPTION(Exception, El::Service::Exception);
      EL_EXCEPTION(InvalidArg, Exception);
      
    public:
      
      class Task :
        public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
      {
      public:
        virtual ~Task() throw();
        virtual void execute() throw(El::Exception) = 0;

        // If true then task should be executed by all means,
        // otherwise it can be dropped if not yet executed at the moment
        // of stopping ThreadPool
        virtual bool execution_required() throw(El::Exception) = 0;
      };

      typedef El::RefCount::SmartPtr<Task> Task_var;
      typedef El::Queue<Task_var> TaskQueue;

      class TaskBase : public virtual Task
      {
      public:
        TaskBase(bool execution_required) throw(El::Exception);

        virtual bool execution_required() throw(El::Exception);
        
      protected:
        bool execution_required_;
      };

      class ServiceEvent : public virtual TaskBase,
                           public virtual El::Service::ServiceEvent
      {
      public:
        ServiceEvent(Callback* callback,
                     Service* source,
                     bool execution_required = false)
          throw(InvalidArg, Exception, El::Exception);
        
        virtual ~ServiceEvent() throw();
        
        virtual void execute() throw(El::Exception);
      };
      
    public:
      
      ThreadPool(
        Callback* callback,
        const char* name = 0,
        unsigned long threads = 1,
        size_t stack_size = 0,
        size_t queue_size = SIZE_MAX,
        TaskQueue::EnqueueStrategy enqueue_strategy = TaskQueue::ES_BACK)
        throw(InvalidArg, El::Exception);

      virtual ~ThreadPool() throw();

      bool execute(
        Task* task,
        const ACE_Time_Value* wait_time = 0,
        TaskQueue::EnqueueStrategy enqueue_strategy = TaskQueue::ES_DEFAULT)
        throw(InvalidArg, El::Exception);

      virtual bool stop() throw(Exception, El::Exception);
      virtual void wait() throw(Exception, El::Exception);

      size_t queue_size() const throw() { return tasks_.size(); }
      
    protected:
      virtual void run() throw(Exception, El::Exception);

    protected:
      TaskQueue tasks_;
      size_t queue_size_;
      TaskQueue::EnqueueStrategy enqueue_strategy_;
    };

    typedef El::RefCount::SmartPtr<ThreadPool> ThreadPool_var;
  }
  
}

#define El__Service__ThreadPool__ServiceEventBase(callback, source, execution_required) \
          El::Service::ThreadPool::TaskBase(execution_required), \
          El::Service::Event(source), \
          El::Service::ServiceEvent(callback, source), \
          El::Service::ThreadPool::ServiceEvent(callback, source, execution_required)

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Service
  {
    //
    // ThreadPool class
    //
    inline
    ThreadPool::ThreadPool(Callback* callback,
                           const char* name,
                           unsigned long threads,
                           size_t stack_size,
                           size_t queue_size,
                           TaskQueue::EnqueueStrategy enqueue_strategy)
      throw(InvalidArg, El::Exception)
        : ServiceBase<El::Sync::ThreadRWPolicy>(callback,
                                                name,
                                                threads,
                                                stack_size),
          tasks_(queue_size),
          queue_size_(queue_size),
          enqueue_strategy_(enqueue_strategy)
    {
      if(queue_size == 0)
      {
        throw Exception(
          "El::Service::ThreadPool::ThreadPool: queue_size is 0");
      }
      
    }
    
    inline
    ThreadPool::~ThreadPool() throw()
    {
    }

    inline
    bool
    ThreadPool::stop() throw(Exception, El::Exception)
    {
      bool ret = ServiceBase<El::Sync::ThreadRWPolicy>::stop();

      tasks_.max_size(0);
      tasks_.awake();
      return ret;
    }

    inline
    void
    ThreadPool::wait() throw(Exception, El::Exception)
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
    // ThreadPool::Task class
    //
    inline
    ThreadPool::Task::~Task() throw()
    {
    }

    //
    // ThreadPool::TaskBase class
    //
    inline
    ThreadPool::TaskBase::TaskBase(bool execution_required)
      throw(El::Exception)
        : execution_required_(execution_required)
    {    
    }
    
    inline
    bool
    ThreadPool::TaskBase::execution_required() throw(El::Exception)
    {
      return execution_required_;
    }
    
    //
    // ThreadPool::ServiceEvent class
    //
    inline
    ThreadPool::ServiceEvent::ServiceEvent(Callback* callback,
                                           Service* source,
                                           bool execution_required)
      throw(InvalidArg, Exception, El::Exception)
        : TaskBase(execution_required),
          Event(source),
          El::Service::ServiceEvent(callback, source)
    {
      if(callback == 0)
      {
        throw InvalidArg(
          "El::Service::ThreadPool::ServiceEvent::ServiceEvent: "
          "callback is null");
      }
    }
    
    inline
    ThreadPool::ServiceEvent::~ServiceEvent() throw()
    {
    }
    
    inline
    void 
    ThreadPool::ServiceEvent::execute() throw(El::Exception)
    {
      callback_->notify(this);
    }

  }
}

#endif // _ELEMENTS_EL_THREADPOOL_HPP_
