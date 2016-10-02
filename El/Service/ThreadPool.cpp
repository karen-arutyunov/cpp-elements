/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/ThreadPool.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>

#include "ThreadPool.hpp"

namespace El
{
  namespace Service
  {
    void
    ThreadPool::run() throw(Exception, El::Exception)
    {
      while(true)
      {
        Task_var task;

        if(tasks_.dequeue(task))
        {
          bool to_execute = task->execution_required();

          if(!to_execute)
          {
            ReadGuard guard(srv_lock_);

            to_execute = !stop_;
          }

          if(to_execute)
          {
            task->execute();
          }
        }
        else
        {
          break;
        }
      }
    }

    bool
    ThreadPool::execute(Task* task,
                        const ACE_Time_Value* wait_time,
                        TaskQueue::EnqueueStrategy enqueue_strategy)
      throw(InvalidArg, El::Exception)
    {
      if(task == 0)
      {
        throw InvalidArg("El::Service::ThreadPool::execute: task is null");
      }

      Task_var task_ptr(El::RefCount::add_ref(task));
      
      ReadGuard guard(srv_lock_);

      if(started_ && stop_)
      {
        return false;
      }

      return
        tasks_.enqueue(task_ptr,
                       wait_time,
                       enqueue_strategy == TaskQueue::ES_DEFAULT ?
                       enqueue_strategy_ : enqueue_strategy);
    }
  }
  
}
