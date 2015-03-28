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
 * @file   Elements/test/ProcessPool/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <string.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <El/Service/ProcessPool.hpp>

#include <tests/ProcessPool/Task.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestProcessPool [help]\n";
}

int
main(int argc, char** argv)
{
  try
  {
    Application app;
    int res = app.run(argc, argv);
    return res;
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'TestProcessPool help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestProcessPool: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestProcessPool: unknown exception caught\n";
  }
  
  return -1;
}

Application::Application() throw(Application::Exception, El::Exception)
{
}

Application::~Application() throw()
{
}

int
Application::run(int& argc, char** argv)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string command;
  
  int i = 0;  

  if(argc > 1)
  {
    command = argv[i++];
  }

  ArgList arguments;

  for(i++; i < argc; i++)
  {
    char* argument = argv[i];
    
    Argument arg;
    const char* eq = strstr(argument, "=");

    if(eq == 0)
    {
      arg.name = argument;
    }
    else
    {
      arg.name.assign(argument, eq - argument);
      arg.value = eq + 1;
    }

    arguments.push_back(arg);
  }

  if(command == "help")
  {
    return help(arguments);
  }
  
  return test(arguments);
}

int
Application::help(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << USAGE;
  return 0;
}

int
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  typedef std::vector<DoublingTask_var> DoublingTaskArray;

  DoublingTaskArray tasks;
  
  El::Service::ProcessPool_var prc_pool(
    new El::Service::ProcessPool(this,
                                 "libElTestProcessPoolTask.so",
                                 "create_task_factory",
                                 0,
                                 0,
                                 "ProcessPool",
                                 10,
                                 300,
                                 1024 * 20,
                                 30));

  DoublingTask_var task;
  
  for(unsigned long i = 0; i < 30; i++)
  {
    task = new DoublingTask(i);
    prc_pool->execute(task, &ACE_Time_Value::zero);

    tasks.push_back(task);
  }

  task = new DoublingTask(0);
  
  if(prc_pool->execute(task, &ACE_Time_Value::zero))
  {
    throw Exception(
      "Application::test: task unexpectedly taken for execution");
  }

  std::cerr << "Starting ...\n";
  
  prc_pool->start();
  
  unsigned long base = tasks.size();

  for(unsigned long i = 0; i < 30; i++)
  {
    task = new DoublingTask(base + i);
    prc_pool->execute(task);
    
    tasks.push_back(task);    
  }

  for(DoublingTaskArray::const_iterator it = tasks.begin(); it != tasks.end();
      it++)
  {
    DoublingTask_var task = *it;
    task->wait();
  }

  std::cerr << "Stopping ...\n";
  prc_pool->stop();

  std::cerr << "Waiting ...\n";
  prc_pool->wait();

  int result = 0;

  for(DoublingTaskArray::const_iterator it = tasks.begin(); it != tasks.end();
      it++)
  {
    DoublingTask_var task = *it;
    
    std::string error = task->error();

    if(!error.empty())
    {
      std::cerr << "Task error: " << error << std::endl;
      result = 1;
    }
    else
    {
      DoublingTask_var check_task = new DoublingTask(task->param);
      check_task->execute();

      if(check_task->result != task->result)
      {
        std::cerr << "Task result error: " << task->result
                  << " instead of " << check_task->result
                  << "; param " << task->param << std::endl;

        result = 1;
      }
    }
    
  }
  
  return result;
}

bool
Application::notify(El::Service::Event* event)
  throw(El::Exception)
{
  El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);

  if(error)
  {
    std::cerr << "Application::notify: " << *error << std::endl;
    return true;
  }

  std::cerr << "Application::notify: unknown " << *event << std::endl;

  return false;
}

