/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/tests/FileGen/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/ArrayPtr.hpp>
#include <El/FileSystem.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
  "\nUsage:\nElTestFileGen help\n"
    "\nElTestFileGen gen --dir=<dir-prefix> --dir-count=<dir count> "
    "--file=<file prefix> --count=<file per dir count> --size=<file size>";
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
              << "\nRun 'ElTestFileGen help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestFileGen: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestFileGen: unknown exception caught\n";
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
  else if(command == "gen")
  {
    test(arguments);
  }
  else
  {
    std::ostringstream ostr;
    ostr << "unknown command " << command;
    throw Exception(ostr.str());
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
  std::string file_name;
  std::string dir_name = "DR";
  size_t file_count = 0;
  size_t dir_count = 1;
  size_t file_size = 0;
  
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      ++it)
  {
    const std::string name = it->name;
    const char* value = it->value.c_str();

    if(name == "--file")
    {
      file_name = value;
    }
    else if(name == "--dir")
    {
      dir_name = value;
    }
    else if(name == "--count")
    {
      if(!El::String::Manip::numeric(value, file_count))
      {
        throw Exception("invalid value for --count argument");
      }
    }
    else if(name == "--dir-count")
    {
      if(!El::String::Manip::numeric(value, dir_count))
      {
        throw Exception("invalid value for --dir-count argument");
      }
    }
    else if(name == "--size")
    {
      if(!El::String::Manip::numeric(value, file_size))
      {
        throw Exception("invalid value for --size argument");
      }
    }
  }

  if(file_name.empty())
  {
    throw Exception("no file name provided");
  }
  
  El::ArrayPtr<unsigned char> buff(new unsigned char[file_size]);
  memset(buff.get(), 1, file_size);

  for(size_t i = 0; i < dir_count; ++i)
  {
    std::string dname;
    
    {
      std::ostringstream ostr;
      ostr << dir_name << "_" << i;
      dname = ostr.str();
    }

    El::FileSystem::create_directory(dname.c_str());
    
    for(size_t i = 0; i < file_count; ++i)
    {
      std::string name;
      
      {
        std::ostringstream ostr;
        ostr << dname << "/" << file_name << "." << i;
        name = ostr.str();
      }
      
      std::fstream file(name.c_str(), ios::out);
      
      if(!file.is_open())
      {
        std::ostringstream ostr;
        ostr << "failed to open file '" << name << "' for write access";
        throw Exception(ostr.str());
      }
      
      file.write((char*)buff.get(), file_size);
      
      if(file.fail())
      {
        std::ostringstream ostr;
        ostr << "failed to write to file '" << name << "'";
        throw Exception(ostr.str());
      }
    }
  }
  
  return 0;
}
