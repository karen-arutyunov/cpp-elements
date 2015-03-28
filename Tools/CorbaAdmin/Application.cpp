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
 * @file   Elements/Tools/CorbaAdmin/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <El/CORBA/Corba.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <ace/OS.h>

#include <El/CORBA/Control.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
"\nUsage:\nElCorbaAdmin <command> <command arguments>\n\n"
"Synopsis 1:\n"
"ElCorbaAdmin help\n\n"
"Synopsis 2:\nElCorbaAdmin stop <corba reference>\n\n"
"Synopsis 3:\nElCorbaAdmin status <corba reference>\n";  
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
    std::cerr << e << "\nRun 'ElCorbaAdmin help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << e << std::endl;
  }
  catch(...)
  {
    std::cerr << "ElCorbaAdmin: unknown exception caught\n";
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
  orb_ = ::CORBA::ORB::_duplicate(
    El::Corba::Adapter::orb_adapter(argc, argv)->orb());  

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

  int res = 0;

  if(command == "help")
  {
    res = help(arguments);
  }
  else if(command == "stop")
  {
    res = stop(arguments);
  }
  else if(command == "status")
  {
    res = status(arguments);    
  }
  else
  {
    std::ostringstream ostr;
    ostr << "unknown command '" << command << "'";
   
    throw InvalidArg(ostr.str());
  }

  El::Corba::Adapter::orb_adapter_cleanup();
  
  return res;
}

int
Application::help(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << USAGE;
  return 0;
}

int
Application::stop(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  if(arguments.empty())
  {
    throw InvalidArg("CORBA reference should be provided");
  }

  const std::string& ref = arguments.begin()->name;

  try
  {
    CORBA::Object_var obj = orb_->string_to_object(ref.c_str());
      
    if(CORBA::is_nil(obj.in()))
    {
      std::ostringstream ostr;
      ostr << "String '" << ref << "' doesn't look like CORBA reference";
      
      throw InvalidArg(ostr.str().c_str());
    }
    
    El::Corba::ProcessCtl_var prc_ctl =
      El::Corba::ProcessCtl::_narrow(obj.in());
    
    if(CORBA::is_nil(prc_ctl.in()))
    {
      return -1;
    }
    
    prc_ctl->stop();
    return 0;
  }
  catch(const CORBA::Exception& e)
  {
  }
  
  return -1;
}

int
Application::status(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  if(arguments.empty())
  {
    throw InvalidArg("CORBA reference should be provided");
  }

  const std::string& ref = arguments.begin()->name;

  try
  {
    CORBA::Object_var obj = orb_->string_to_object(ref.c_str());
      
    if(CORBA::is_nil(obj.in()))
    {
      std::ostringstream ostr;
      ostr << "String '" << ref << "' doesn't look like CORBA reference";
      
      throw InvalidArg(ostr.str().c_str());
    }
    
    El::Corba::ProcessCtl_var prc_ctl =
      El::Corba::ProcessCtl::_narrow(obj.in());
    
    if(CORBA::is_nil(prc_ctl.in()))
    {
      return -1;
    }
    
    return prc_ctl->status() ? 0 : -1;
  }
  catch(const CORBA::Exception& e)
  {
  }
  
  return -1;
}

