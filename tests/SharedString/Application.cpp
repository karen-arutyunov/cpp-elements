/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/test/SharedString/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <El/Stat.hpp>
#include <El/String/SharedString.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nElTestSharedString [help]\n";
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
              << "\nRun 'ElTestSharedString help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestSharedString: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestSharedString: unknown exception caught\n";
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
  test_shared_string(arguments);
  test_performance(arguments);
  
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
  El::String::SharedStringManager string_manager;

  const char* abc = string_manager.add("abc");
  const char* xyz = string_manager.add("xyz");

  const char* abc2 = string_manager.add("abc");
  
  if(abc2 != abc)
  {
    throw Exception("Application::test: second add('abc') failed");
  }
  
  string_manager.remove(abc2);

  abc2 = string_manager.add_ref(abc);
  
  if(abc2 != abc)
  {
    throw Exception("Application::test: add_ref(abc) failed");
  }
  
  string_manager.remove(abc2);
  
  const char* mnl = string_manager.add("mnl");
  const char* A12 = string_manager.add("A12");

  const char* abc3 = string_manager.add("abc");
  
  if(abc3 != abc)
  {
    throw Exception("Application::test: third add('abc') failed");
  }
  
  string_manager.remove(abc);

  const char* nvp = string_manager.add("nvp");
  
  string_manager.remove(abc3);
  string_manager.remove(xyz);
  string_manager.remove(mnl);
  string_manager.remove(A12);

  const char* lex = string_manager.add("lex");

  string_manager.remove(nvp);
  string_manager.remove(lex);

  if(!string_manager.empty())
  {
    throw Exception("Application::test: string_manager is "
                    "unexpectedly non empty");
  }
  
  return 0;
}

int
Application::test_performance(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  El::String::SharedStringManager string_manager;

  El::Stat::TimeMeter adding_meter("  adding");
  El::Stat::TimeMeter accessing_meter("  accessing");
  El::Stat::TimeMeter adding_ref_meter("  adding ref");
  El::Stat::TimeMeter removing_meter("  removing");

  typedef std::vector<const char*> StringVector;

  StringVector strings;
  char buff[2000];
    
  for(unsigned long i = 0; i < 1000000; i++)
  {
    unsigned long len = (unsigned long long)rand() * (sizeof(buff) - 1) /
      ((unsigned long long)RAND_MAX + 1);

    unsigned long j = 0;
    
    for(; j < len; j++)
    {
      buff[j] = (char)((unsigned long long)rand() * 255 /
                       ((unsigned long long)RAND_MAX + 1) + 1);
    }

    buff[j] = '\0';

    const char* ptr = 0;
    
    {
      El::Stat::TimeMeasurement measurement(adding_meter);
      ptr = string_manager.add(buff);
    }
    
    strings.push_back(ptr);
  }

  for(StringVector::const_iterator it = strings.begin(); it != strings.end();
      it++)
  {
    const char* ptr = *it;
    const char* ptr2 = 0;
    
    {
      El::Stat::TimeMeasurement measurement(accessing_meter);
      ptr2 = string_manager.add(ptr);
    }
    
    if(ptr != ptr2)
    {
      std::ostringstream ostr;
      ostr << "Application::test_performance: wrong pointer returned by "
        "string_manager::add for '" << ptr << "'";
      
      throw Exception(ostr.str());
    }
    
    El::Stat::TimeMeasurement measurement(removing_meter);
    string_manager.remove(ptr2);
  }
    
  for(StringVector::const_iterator it = strings.begin(); it != strings.end();
      it++)
  {
    const char* ptr = *it;
    const char* ptr2 = 0;
    
    {
      El::Stat::TimeMeasurement measurement(adding_ref_meter);
      ptr2 = string_manager.add_ref(ptr);
    }
    
    if(ptr != ptr2)
    {
      std::ostringstream ostr;
      ostr << "Application::test_performance: wrong pointer returned by "
        "string_manager::add_ref for '" << ptr << "'";
      
      throw Exception(ostr.str());
    }
    
    El::Stat::TimeMeasurement measurement(removing_meter);
    string_manager.remove(ptr2);
  }
    
  for(StringVector::const_iterator it = strings.begin(); it != strings.end();
      it++)
  {
    El::Stat::TimeMeasurement measurement(removing_meter);
    string_manager.remove(*it);
  }

  if(!string_manager.empty())
  {
    throw Exception("Application::test_performance: string_manager is "
                    "unexpectedly non empty");
  }

  std::cerr << "SharedStringManager operations stat:\n";
  
  adding_meter.dump(std::cerr);
  std::cerr << std::endl;
  
  accessing_meter.dump(std::cerr);
  std::cerr << std::endl;

  adding_ref_meter.dump(std::cerr);
  std::cerr << std::endl;

  removing_meter.dump(std::cerr);
  std::cerr << std::endl;
  
  return 0;
}

int
Application::test_shared_string(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  typedef El::String::SharedStringConstPtr<int> StringConstPtr;

  StringConstPtr val("AAA");

  if(!(val == "AAA"))
  {
    throw Exception("Application::test_shared_string: comparison with "
                    "const char* failed");
  }
    
  StringConstPtr val2("AAA");

  if(!(val == val2))
  {
    throw Exception("Application::test_shared_string: comparison with "
                    "StringConstPtr failed");
  }
    
  return 0;
}

