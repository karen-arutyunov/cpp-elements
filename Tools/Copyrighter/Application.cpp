/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/Tools/Copyrighter/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <ace/OS.h>

#include <El/String/Manip.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
  "\nUsage:\nElCopyrighter <command> <command arguments>\n\n"
  "Synopsis 1:\nElCopyrighter help\n\n"
  "Synopsis 2:\nElCopyrighter update [skiplines=<numeric>] copyright=<filename>\n"
  "Takes data from stdin, replaces the copyright note, "
  "writes result to stdout\n";
}

int
main(int argc, char** argv)
{
  try
  {
    srand(time(NULL));
    
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << e << "\nRun 'ElCopyrighter help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << e << std::endl;
  }
  catch(...)
  {
    std::cerr << "ElCopyrighter: unknown exception caught\n";
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
    throw InvalidArg("Too few arguments");
  }

  int i = 1;  
  std::string command = argv[i];

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
  else if(command == "update")
  {
    return update(arguments);
  }
  else
  {
    std::ostringstream ostr;
    ostr << "unknown command '" << command << "'";
   
    throw InvalidArg(ostr.str());
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
Application::update(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string copyright_file;
  unsigned long skiplines = 0;
  
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    const std::string& name = it->name;
    const std::string& value = it->value;

    if(name == "copyright")
    {
      copyright_file = value;
    }
    else if(name == "skiplines")
    {
      El::String::Manip::numeric(value.c_str(), skiplines);
    }
  }

  if(copyright_file.empty())
  {
    throw InvalidArg("copyright file not specified");
  }

  std::fstream file(copyright_file.c_str(), std::ios::in);

  if(!file.is_open())
  {
    std::ostringstream ostr;
    ostr << "file '" << copyright_file << "' can't be opened";
    throw InvalidArg(ostr.str());
  }

  std::ostringstream copyright;
  std::string line;
  bool first_line = true;

  std::string first_copyline;
  std::string last_copyline;
  
  while(std::getline(file, line))
  {
    if(first_line)
    {
      first_line = false;
    }
    else
    {
      copyright << std::endl;
    }

    copyright << line;
    
    std::string trimmed;
    El::String::Manip::trim(line.c_str(), trimmed);

    if(first_copyline.empty() && !trimmed.empty())
    {
      first_copyline = trimmed;
    }

    if(!trimmed.empty())
    {
      last_copyline = trimmed;
    }
  }

  copyright << std::endl;
  
  if(first_copyline.empty())
  {
    std::ostringstream ostr;
    ostr << "file '" << copyright_file << "' contains empty copyright notice";
    throw InvalidArg(ostr.str());
  }
  
  enum State
    {
      ST_READ_HEAD,
      ST_READ_COPYRIGHT,
      ST_READ_FILE
    };

  State state = ST_READ_HEAD;
  
  while(std::getline(std::cin, line))
  {
    std::string trimmed;
    El::String::Manip::trim(line.c_str(), trimmed);

    switch(state)
    {
    case ST_READ_HEAD:
      {
        if(skiplines)
        {
          skiplines--;
        }
        else
        {
          if(!trimmed.empty())
          {
            if(first_copyline == trimmed)
            {
              state = ST_READ_COPYRIGHT;
              continue;
            }
            else
            {
              std::cout << copyright.str() << std::endl;
              state = ST_READ_FILE;
            }
          }
        }
        
        break;
      }
    case ST_READ_COPYRIGHT:
      {
        if(last_copyline == trimmed)
        {
          std::cout << copyright.str();
          state = ST_READ_FILE;
        }
          
        continue;
      }
    case ST_READ_FILE:
      {
        break;
      }
    }

    std::cout << line << std::endl;
  }
  
  return -1;
}
