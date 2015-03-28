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
 * @file   Elements/test/HTTPSession/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

#include <El/Net/HTTP/Session.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
  "\nUsage:\nElTestHTTPSession (help|"
  "request (header=\"<name>:<value>\")* [preserve-content-encoding=(0|1)] "
  "[print-headers=(0|1)] url=<url> )* \n";
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
              << "\nRun 'ElTestHTTPSession help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestHTTPSession: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestHTTPSession: unknown exception caught\n";
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
  else
  {
    throw InvalidArg("too few arguments");
  }

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
  else if(command == "request")
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
  El::Net::HTTP::HeaderList headers;
  bool preserve_content_encoding = false;
  bool print_headers = false;
  
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    if(it->name == "url")
    {
      El::Net::HTTP::Session session(it->value.c_str(),
                                     El::Net::HTTP::HTTP_1_1,
                                     0,
                                     preserve_content_encoding);
      
      ACE_Time_Value timeout(20);
      
      El::Net::HTTP::ParamList params;
      
      session.open(&timeout, &timeout, &timeout, 100, 100, 50);
      session.send_request(El::Net::HTTP::GET, params, headers);

      session.recv_response_status();

      El::Net::HTTP::Header header;
      while(session.recv_response_header(header))
      {
        if(print_headers)
        {
          std::cout << header.name << ":" << header.value << std::endl;
        }
      }

      std::istream& body_stream = session.response_body();
      unsigned long read_bytes = 0;

      while(true)
      {
        char buff[1024];
        body_stream.read(buff, sizeof(buff));

        read_bytes = body_stream.gcount();

        if(read_bytes)
        {
          std::cout.write(buff, read_bytes);
        }
        else
        {
          break;
        }
      }

      session.test_completion();

      headers.clear();
      preserve_content_encoding = false;
      print_headers = false;
    }
    else if(it->name == "header")
    {
      std::string::size_type pos = it->value.find(':');

      if(pos != std::string::npos)
      {
        std::string nm = it->value.substr(0, pos);
        std::string vl = it->value.substr(pos + 1);
        
        headers.add(nm.c_str(), vl.c_str());
      }
    }
    else if(it->name == "preserve-content-encoding")
    {
      preserve_content_encoding = it->value == "1";
    }
    else if(it->name == "print-headers")
    {
      print_headers = it->value == "1";
    }
    
  }

  return 0;
}

