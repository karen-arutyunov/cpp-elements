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
 * @file   Elements/test/ProcessPool/Task.cpp
 * @author Karen Arutyunov
 * $Id:$
 */


#include <sstream>

#include <El/Service/ProcessPool.hpp>
#include <tests/ProcessPool/Task.hpp>

// Tasks factory

class TaskFactory : public ::El::Service::ProcessPool::TaskFactoryInterface
{
  EL_EXCEPTION(Exception, El::Service::Exception);
  
  virtual ~TaskFactory() throw() {}
  virtual const char* creation_error() throw() { return 0; }
  
  virtual void release() throw(Exception) { delete this; }
  
  virtual ::El::Service::ProcessPool::Task* create_task(const char* id)
    throw(El::Exception);
};

::El::Service::ProcessPool::Task*
TaskFactory::create_task(const char* id) throw(El::Exception) 
{
  if(strcmp(id, "Dbl") == 0)
  {
    return new DoublingTask();
  }

  std::ostringstream ostr;
  ostr << "TaskFactory::create_task: unknown task id '" << id << "'";
  throw Exception(ostr.str());
}
  
extern "C"
::El::Service::ProcessPool::TaskFactoryInterface*
create_task_factory(const char* args)
{
  return new TaskFactory();
}
