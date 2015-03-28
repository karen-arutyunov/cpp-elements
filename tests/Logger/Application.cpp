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
 * @file   Elements/test/Logger/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <unistd.h>

#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

#include <El/Moment.hpp>
#include <El/Stat.hpp>
#include <El/FileSystem.hpp>

#include <El/Logging/StreamLogger.hpp>
#include <El/Logging/FileLogger.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestLogger [help]\n";
}

class DirReader : public El::FileSystem::DirectoryReader
{
public:
  virtual bool select(const struct dirent* dir) throw(El::Exception);
};
    
bool
DirReader::select(const struct dirent* dir) throw(El::Exception)
{
  static const char prefix[] = "Test.log";

  return strncmp(dir->d_name, prefix, sizeof(prefix) - 1) == 0;
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
              << "\nRun 'TestLogger help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestLogger: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestLogger: unknown exception caught\n";
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
  DirReader dir;
  dir.read(".");

  for(unsigned long i = 0; i < dir.count(); i++)
  {
    unlink(dir[i].d_name);
  }
  
  El::Logging::Formatter_var formatter =
    new El::Logging::SimpleFormatter(
      El::Logging::SimpleFormatter::FP_ALL &
      ~El::Logging::SimpleFormatter::FP_TIME);
  
  {
    std::ostringstream output;
  
    El::Logging::StreamLogger logger(output,
                                     El::Logging::TRACE + 3,
                                     "Test1,Test1.1",
                                     El::Moment::TZ_GMT,
                                     &std::cerr,
                                     formatter.in());
    
    logger.trace("Line 1", "Test1", 0);
    logger.trace("Line 2", "TestX", 1);
    logger.trace("Line 3", "Test1.1", 2);

    const char* expected =
      "[TRACE 0] (Test1) : Line 1\n[TRACE 2] (Test1.1) : Line 3\n";
    
    std::string result = output.str();
    
    if(result != expected)
    {
      std::stringstream ostr;
      ostr << "For Test1 output is '" << result << "' instead of '" << expected
           << "'";
      
      throw Exception(ostr.str());
    }
  }

  {
    std::ostringstream output;
  
    El::Logging::StreamLogger logger(output,
                                     El::Logging::DEBUG,
                                     "*",
                                     El::Moment::TZ_GMT,
                                     &std::cerr,
                                     formatter.in());
    
    logger.alert("Line 1", "Test2");
    logger.trace("Line 2", "TestX");
    logger.alert("Line 3", "Test2.1");

    const char* expected =
      "[ALERT] (Test2) : Line 1\n[ALERT] (Test2.1) : Line 3\n";
    
    std::string result = output.str();
    
    if(result != expected)
    {
      std::stringstream ostr;
      ostr << "For Test2 output is '" << result << "' instead of '" << expected
           << "'";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    std::ostringstream output;
  
    El::Logging::StreamLogger logger(output,
                                     El::Logging::TRACE + 3,
                                     "Test3,Test3.1",
                                     El::Moment::TZ_GMT,
                                     &std::cerr,
                                     formatter.in());

    El::Stat::TimeMeter meter("StreamLogger::log");
    
    for(unsigned long i = 0; i < 100000; i++)
    {
      meter.start();
      logger.trace("This is some text message.", "Test3", 0);
      meter.stop();
    }

    meter.dump(std::cerr);
  }

  {
    El::Logging::FileLogger logger("Test.log",
                                   El::Logging::TRACE + 3,
                                   "Test4,Test4.1");
    
    logger.trace("Line 1", "Test4", 0);
    logger.trace("Line 2", "TestX", 1);
    logger.trace("Line 3", "Test4.1", 2);
  }
  
  {
    El::Logging::FileLogger::RotatingPolicyList policies;

    policies.push_back(new El::Logging::FileLogger::RotatingBySizePolicy(10));
    
    El::Logging::FileLogger logger("Test.log",
                                   El::Logging::WARNING,
                                   "*",
                                   &policies);
    
    logger.emergency("Line 1", "Test5");
    logger.emergency("Line 2", "TestX");
    logger.emergency("Line 3", "Test5.1");

    DirReader dir;
    dir.read(".");

    if(dir.count() == 4)
    {
      for(unsigned long i = 0; i < dir.count(); i++)
      {
        unlink(dir[i].d_name);
      }
    }
    else
    {
      std::stringstream ostr;
      ostr << "For Test5 output splits into " << dir.count()
           << " files instead of 4";
    
      throw Exception(ostr.str());
    }
  }

  {
    El::Logging::FileLogger::RotatingPolicyList policies;

    policies.push_back(
      new El::Logging::FileLogger::RotatingByTimePolicy(ACE_Time_Value(1)));
    
    El::Logging::FileLogger logger("Test.log",
                                   El::Logging::WARNING,
                                   "*",
                                   &policies,
                                   El::Moment::TZ_GMT,
                                   &std::cerr,
                                   0,
                                   ACE_Time_Value(1));

    for(unsigned long i = 0; i < 10; i++)
    {
      std::ostringstream ostr;
      ostr << "Line " << i;
      
      logger.emergency(ostr.str(), "Test6");
      ACE_OS::sleep(ACE_Time_Value(0, 300000));
    }

    ACE_OS::sleep(2);

    DirReader dir;
    dir.read(".");

    if(dir.count() == 4)
    {
      for(unsigned long i = 0; i < dir.count(); i++)
      {
        unlink(dir[i].d_name);
      }
    }
    else
    {
      std::stringstream ostr;
      ostr << "For Test6 output splits into " << dir.count()
           << " files instead of 4";
    
      throw Exception(ostr.str());
    }

  }

  {
    El::Logging::FileLogger logger("Test.log",
                                   El::Logging::TRACE + 3,
                                   "Test7,Test7.1");
    
    El::Stat::TimeMeter meter("FileLogger::log");
    
    for(unsigned long i = 0; i < 100000; i++)
    {
      meter.start();
      logger.trace("This is some text message.", "Test7", 0);
      meter.stop();
    }

    meter.dump(std::cerr);
  }
  
  dir.read(".");

  for(unsigned long i = 0; i < dir.count(); i++)
  {
    unlink(dir[i].d_name);
  }

  return 0;
}

