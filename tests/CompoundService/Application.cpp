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
 * @file   Elements/test/ThreadPool/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

#include <El/Service/CompoundService.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestCompoundService [help]\n";
}

int
main(int argc, char** argv)
{
  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'TestCompoundService help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestCompoundService: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestCompoundService: unknown exception caught\n";
  }
  
  return -1;
}

//
// Application
//
Application::Application() throw(Application::Exception, El::Exception)
    : counter_(0)
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

  test(arguments);
  return 0;
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
  TestService_var test_service(new TestService(this));
    
  /*
  El::Service::ThreadPool_var thr_pool(
    new El::Service::ThreadPool(this, "ThreadPool", 10, 0, 30));

  El::Service::ThreadPool::Task_var event;
  
  for(unsigned long i = 0; i < 30; i++)
  {
    event = new TestEvent(this, 0);

    thr_pool->execute(event, &ACE_Time_Value::zero);
  }

  event = new TestEvent(this, 0);
  
  if(thr_pool->execute(event, &ACE_Time_Value::zero))
  {
    throw Exception(
      "Application::test: task unexpectedly taken for execution");
  }

  std::cerr << "Starting ...\n";
  
  thr_pool->start();

  for(unsigned long i = 0; i < 30; i++)
  {
    event = new TestEvent(this, 0);
    
    thr_pool->execute(event);
  }

  ACE_OS::sleep(3);
  
  std::cerr << "Stopping ...\n";
  thr_pool->stop();

  std::cerr << "Waiting ...\n";
  thr_pool->wait();

  if(counter_ != 60)
  {
    throw Exception(
      "Application::test: unexpected number of tasks executed"); 
  }
  */
  
  return 0;
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

  TestEvent* test_event = dynamic_cast<TestEvent*>(event);

  if(test_event)
  {
    {
      WriteGuard guard(lock_);
      counter_++;
    }
    
    ACE_OS::sleep(ACE_Time_Value(1));

    return true;
  }

  std::cerr << "Application::notify: unknown " << *event << std::endl;

  return false;
}

//
// Application::TestService class
//
Application::TestService::TestService(Callback* callback)
  throw(InvalidArg, El::Exception)
    : El::Service::CompoundService<>(callback, "Application::TestService")
{
}
