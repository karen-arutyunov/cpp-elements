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
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

#include <El/Moment.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestMoment [help]\n";
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
              << "\nRun 'TestMoment help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestMoment: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestMoment: unknown exception caught\n";
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
  
  int i = 1;  

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
  {
    const char time[] = "Sat, 17 Dec 2005 16:09:37 +0300";
    
    El::Moment moment(time, El::Moment::TF_RFC_0822);

    if(moment != El::Moment(105, 11, 17, 13, 9, 37, 0, El::Moment::TZ_GMT))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_RFC_0822 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = "2005";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 0, 0))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = "0000";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(0, 0, 0))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = "0000-00";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(0, 0, 0))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = "0000-00-00";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(0, 0, 0))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = "2005-12";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 0))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = " 2005-12-17 ";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = " 2005-12-17 12:15";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17, 12, 15))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = " 2005-12-17 12:15 Z";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17, 12, 15))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = " 2005-12-17 12:15:35";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17, 12, 15, 35))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = " 2005-12-17 12:15:35 +03:00";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17, 9, 15, 35))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = " 2005-12-17 12:15:35+03:00";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17, 9, 15, 35))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    const char time[] = " 2005-12-17 12:15:35.02";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17, 12, 15, 35, 20000))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }

    moment.tm_tz = El::Moment::TZ_LOCAL;
    moment.iso8601();
  }
  
  {
    const char time[] = " 2005-12-17T12:15:35.02";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17, 12, 15, 35, 20000))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }

    moment.tm_tz = El::Moment::TZ_LOCAL;
    moment.iso8601();
  }
  
  {
    const char time[] = " 2005-12-17 12:15:35.02 -05:15";
    
    El::Moment moment(time, El::Moment::TF_ISO_8601);

    if(moment != El::Moment(105, 11, 17, 17, 30, 35, 20000))
    {
      std::ostringstream ostr;
      ostr << "Application::test: TF_ISO_8601 parsing of '" << time
           << "' failed";
      
      throw Exception(ostr.str());
    }

    if(El::Moment("2005-12-17 12:15:35.02 +03:00", El::Moment::TF_ISO_8601) !=
       El::Moment("2005-12-17 09:15:35.02 Z", El::Moment::TF_ISO_8601))
    {
      throw Exception("Application::test: El::Moment::operator== failed");
    }

    if(El::Moment("2005-12-17 12:15:35.02 +03:00", El::Moment::TF_ISO_8601) >=
       El::Moment("2005-12-17 10:15:35.02 Z", El::Moment::TF_ISO_8601))
    {
      throw Exception("Application::test: El::Moment::operator< failed");
    }
  }

  {
    El::Moment moment1("2005-12-17 12:15:35.02", El::Moment::TF_ISO_8601);
    El::Moment moment2("2005-12-16 12:15:40", El::Moment::TF_ISO_8601);

    if(moment1.day_time() >= moment2.day_time())
    {
      std::ostringstream ostr;
      ostr << "Application::test: time of '" << moment1.iso8601()
           << "' is unexpectedly >= time of '" << moment2.iso8601()
           << "'";
      
      throw Exception(ostr.str());
    }
  }

  {
    El::Moment moment1("2005-12-17 12:15:30 +03:00", El::Moment::TF_ISO_8601);
    El::Moment moment2("2005-12-16 09:15:30 Z", El::Moment::TF_ISO_8601);

    if(moment1.day_time() != moment2.day_time())
    {
      std::ostringstream ostr;
      ostr << "Application::test: time of '" << moment1.iso8601()
           << "' is unexpectedly != time of '" << moment2.iso8601()
           << "'";
      
      throw Exception(ostr.str());
    }
  }
  
  {
    El::Moment date("1400148778.023653",
                    El::Moment::TF_FROM_EPOCH);

    if(date.epoch(true) != "1400148778.023653")
    {
      std::ostringstream ostr;
      ostr << "Application::test: moment of '1400148778.023653' is "
        "unexpectedly != moment of '" << date.epoch(true) << "'";
      
      throw Exception(ostr.str());
    }
    
  }

  return 0;
}

