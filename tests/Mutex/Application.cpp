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
 * @file   Elements/test/Moment/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string>
#include <iostream>

#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestMutex [help]\n";

  const unsigned long MUTEXT_COUNT = 10000000;
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
              << "\nRun 'TestMutex help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestMutex: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestMutex: unknown exception caught\n";
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
//  std::cerr << "\nPress Enter to start ...\n";
//  getchar();

//  typedef ACE_Thread_Mutex Mutex;
  typedef ACE_RW_Thread_Mutex Mutex;
  
  Mutex* mutexes = new Mutex[MUTEXT_COUNT];

  for(unsigned long i = 0; i < MUTEXT_COUNT; i++)
  {
    mutexes[i].lock();
  }
  
//  std::cerr << "\nPress Enter to exit ...\n";
//  getchar();

  delete [] mutexes;
  
  return 0;
}

