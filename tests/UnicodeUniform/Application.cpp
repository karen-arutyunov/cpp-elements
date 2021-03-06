/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/tests/UnicodeUniform/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string>
#include <iostream>

#include <El/Exception.hpp>

#include <El/String/Unicode.hpp>
#include <El/String/Manip.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
  "\nUsage:\nElTestUnicodeUniform [help]\n";
}

struct Sample
{
  const wchar_t* src;
  const wchar_t* dest;
};

const Sample SAMPLES[] =
{
  { L"\xB5\x451\x401\xFF18\x32B1\x2493", L"\x3BC\x435\x435" L"83612" },
  { L"ABC", L"abc" },
  { L"\x21A\x21B\x162\x163", L"\x21B\x21B\x163\x163" }
};

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
              << "\nRun 'ElTestUnicodeUniform help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestUnicodeUniform: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestUnicodeUniform: unknown exception caught\n";
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
  else
  {
    test(arguments);
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
  for(unsigned long i = 0; i < sizeof(SAMPLES)/sizeof(SAMPLES[0]); i++)
  {
    const Sample& sample = SAMPLES[i];

    std::wstring dest;
    El::String::Unicode::CharTable::to_uniform(sample.src, dest);

    if(dest != sample.dest)
    {
      std::ostringstream ostr;
      ostr << "Application::test: unexpected to_uniform('";
      El::String::Manip::wchar_to_utf8(sample.src, ostr);
      ostr << "') result '";
      El::String::Manip::wchar_to_utf8(dest.c_str(), ostr);
      ostr << "' instead of '";
      El::String::Manip::wchar_to_utf8(sample.dest, ostr);
      ostr << "'";

      throw Exception(ostr.str());
    }
  }
  
  return 0;
}
