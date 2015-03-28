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
 * @file   Elements/test/Geography/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include <El/Geography/AddressInfo.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nElGeography (help|host <hostname>)\n";
}

int
main(int argc, char** argv)
{
  srand(time(0));
  
  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'ElGeography help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElGeography: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElGeography: unknown exception caught\n";
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
  if(argc < 2)
  {
    throw InvalidArg("too few arguments");
  }
  
  std::string command;
  
  int i = 1;
  command = argv[i++];

  ArgList arguments;

  for(; i < argc; i++)
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
  else if(command == "host")
  {
    test(arguments);
  }
  else
  {
    std::cerr << "no valid command specified. " << USAGE;
  }
  
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
  if(arguments.empty())
  {
    throw Exception("no hosnames specified");
  }

  El::Geography::AddressInfo info;

  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    const char* host = it->name.c_str();
    El::Country country = info.country(host);
    std::cout << host << " in " << country << std::endl;    
  }
  
  return 0;
}
