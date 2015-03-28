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
 * @file   Elements/test/OctetStream/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <El/CORBA/Corba.hpp>

#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#include <string>
#include <iostream>
#include <sstream>

#include <El/CORBA/Stream/OctetStream.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nElTestOctetStream [help]\n";
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
              << "\nRun 'ElTestOctetStream help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestOctetStream: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestOctetStream: unknown exception caught\n";
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
  char rand_str[1000000];
  
  for(unsigned long i = 0; i < sizeof(rand_str); i++)
  {
    rand_str[i] = (char)((unsigned long long)rand() * 255 /
                         ((unsigned long long)RAND_MAX + 1) + 1);
  }

  El::Corba::Stream::OutOctetStream out(100000);
  out.write(rand_str, sizeof(rand_str));

  if(out.fail())
  {
    throw Exception("Application::test: failed to write data");
  }

  out.flush();

  if(out.length() != sizeof(rand_str))
  {
    throw Exception("Application::test: unexpected data length");
  }

  El::Corba::Stream::InOctetStream in(out.data(), out.length());

  char test_str[1000000];

  in.read(test_str, sizeof(test_str));

  if(in.fail())
  {
    throw Exception("Application::test: failed to read data");
  }

  if(memcmp(rand_str, test_str, sizeof(test_str)) != 0)
  {
    throw Exception("Application::test: written and read data don't match");
  }

  return 0;
}
