/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
