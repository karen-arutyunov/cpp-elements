/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
