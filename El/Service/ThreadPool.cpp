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
