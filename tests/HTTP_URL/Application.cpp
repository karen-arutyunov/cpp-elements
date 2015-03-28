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
 * @file   Elements/test/HTTP_URL/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

#include <El/Net/HTTP/URL.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestHTTP_URL [help]\n";
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
              << "\nRun 'TestHHTP_URL help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestHTTP_URL: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestHTTP_URL: unknown exception caught\n";
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
  std::string company_domain;
  const char* domain = 0;
  
  if(!El::Net::company_domain(domain = "abc.com", &company_domain) ||
     company_domain != "abc.com")
  {
    std::ostringstream ostr;
    ostr << "Application::test: failed to correctly recognize company "
      "domain for '" << domain << "'";
      
    throw Exception(ostr.str());    
  }

  if(!El::Net::company_domain(domain = "www.abc.com", &company_domain) ||
     company_domain != "abc.com")
  {
    std::ostringstream ostr;
    ostr << "Application::test: failed to correctly recognize company "
      "domain for '" << domain << "'";
      
    throw Exception(ostr.str());    
  }
  
  if(!El::Net::company_domain(domain = "www.abc.co.uk", &company_domain) ||
     company_domain != "abc.co.uk")
  {
    std::ostringstream ostr;
    ostr << "Application::test: failed to correctly recognize company "
      "domain for '" << domain << "'";
      
    throw Exception(ostr.str());    
  }
  
  if(!El::Net::company_domain(domain = "abc.co.uk", &company_domain) ||
     company_domain != "abc.co.uk")
  {
    std::ostringstream ostr;
    ostr << "Application::test: failed to correctly recognize company "
      "domain for '" << domain << "'";
      
    throw Exception(ostr.str());    
  }
  
  if(El::Net::company_domain(domain = ".co.uk", &company_domain) ||
     El::Net::company_domain(domain = "co.uk", &company_domain) ||
     El::Net::company_domain(domain = "uk", &company_domain) ||
     El::Net::company_domain(domain = "", &company_domain))
  {
    std::ostringstream ostr;
    ostr << "Application::test: failed to correctly recognize company "
      "domain for '" << domain << "'";
      
    throw Exception(ostr.str());    
  }
  
  const char* ip = "www.abc.com";
  
  if(El::Net::ip(ip, 0) || El::Net::ip(ip = "www.abc.co.uk", 0) ||
     El::Net::ip(ip = "1ww.2bc.3co.4uk", 0) ||
     El::Net::ip(ip = "1233.233.123.012", 0) ||
     El::Net::ip(ip = "233.233.123", 0))
  {
    std::ostringstream ostr;
    ostr << "Application::test: host address '" << ip << "' is unexpectedly "
         << "recognised as an IP address";
      
    throw Exception(ostr.str());    
  }

  unsigned int ipnum = 0;
  
  if(!El::Net::ip(ip = "123.1.1.0", &ipnum) || ipnum != 0x7B010100 ||
     !El::Net::ip(ip = "255.255.255.255", &ipnum) || ipnum != 0xFFFFFFFF)
  {
    std::ostringstream ostr;
    ostr << "Application::test: host address '" << ip << "' is not "
         << "recognised as an IP address";
      
    throw Exception(ostr.str());
  }

  El::Net::HTTP::URL_var url1;
  El::Net::HTTP::URL_var url2;

  El::Net::HTTP::URL_var base_url =
    new El::Net::HTTP::URL("http://www.google.com:80/A/B/C.html");  

  const char* str_url = "http://www.google.com:81/A/B/D/E.pdf?a=x&b=y#X?Y/Z";
  
  url1 = new El::Net::HTTP::URL(str_url);

  if(strcmp(url1->host(), "www.google.com") || url1->port() != 81 ||
     strcmp(url1->path(), "/A/B/D/E.pdf") ||
     strcmp(url1->params(), "a=x&b=y") || strcmp(url1->anchor(), "X?Y/Z") ||
     strcmp(url1->string(), str_url))
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' parsing is broken";
      
    throw Exception(ostr.str());
  }  

  url1 = new El::Net::HTTP::URL(base_url->abs_url("../..").c_str());
  url2 = new El::Net::HTTP::URL("http://www.google.com:80/");

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL(base_url->abs_url("D/E.pdf").c_str());
  url2 = new El::Net::HTTP::URL("http://www.google.com:80/A/B/D/E.pdf");

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL(base_url->abs_url(".././D/E.pdf").c_str());
  url2 = new El::Net::HTTP::URL("http://www.google.com:80/A/D/E.pdf");

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL(base_url->abs_url(".././../D/E.pdf").c_str());
  url2 = new El::Net::HTTP::URL("http://www.google.com:80/D/E.pdf");

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL(base_url->abs_url("/D/E.pdf").c_str());
  url2 = new El::Net::HTTP::URL("http://www.google.com:80/D/E.pdf");

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL(base_url->abs_url("D/E.pdf").c_str());
  url2 = new El::Net::HTTP::URL("http://www.google.com:80/A/B/D/E.pdf");

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  base_url = new El::Net::HTTP::URL("http://www.google.com:80/A/B/");
  
  url1 = new El::Net::HTTP::URL(base_url->abs_url(".././../D/E.pdf").c_str());
  url2 = new El::Net::HTTP::URL("http://www.google.com:80/D/E.pdf");

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL("http://www.google.com:80");  
  url2 = new El::Net::HTTP::URL("www.google.com", "/");

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL("  https://www.google.com/A/B?x=1 \t");
  url2 = new El::Net::HTTP::URL("www.google.com", "/A/B", "x=1", "", 0, true);

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL("www.google.com/A/B");
  url2 = new El::Net::HTTP::URL("www.google.com", "/A/B", "", "", 80);

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL("https://www.google.com:443?y=1");
  url2 = new El::Net::HTTP::URL("www.google.com", "", "y=1", 0, 0, true);

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }

  url1 = new El::Net::HTTP::URL("https://www.google.com/?y=1");
  url2 = new El::Net::HTTP::URL("www.google.com", "/", "y=1", 0, 0, true);

  if(*url1 != *url2)
  {
    std::ostringstream ostr;
    ostr << "Application::test: url '" << url1->string()
         << "' unexpectedly != '" << url2->string() << "'";
      
    throw Exception(ostr.str());
  }
  
  return 0;
}

