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
 * @file   Elements/test/SMTP/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <El/Net/Socket/Stream.hpp>
#include <El/Net/SMTP/Session.hpp>
#include <El/String/Manip.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
  "\nUsage:\nElTestSMTP (help|"
    "send from=<from> (to=<to>)+ (subj=<subject>)? (text=<text>)? "
    "(html=<text>)? (html-file=<path>)?)\n";
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
              << "\nRun 'ElTestSMTP help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestSMTP: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestSMTP: unknown exception caught\n";
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
  else if(command == "send")
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
  ::El::Net::SMTP::Message msg;

  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    if(it->name == "subj")
    {
      msg.subject = it->value;
    }
    else if(it->name == "text")
    {
      msg.text = it->value;
    }
    else if(it->name == "html")
    {
      msg.html = it->value;
    }
    else if(it->name == "html-file")
    {
      std::fstream istr(it->value, std::ios::in);

      if(!istr.is_open())
      {
        std::ostringstream ostr;
        ostr << "failed to open " << it->value;
        throw Exception(ostr.str());
      }
      
      std::getline(istr, msg.html, char(-1));
    }
    else if(it->name == "to")
    {
      msg.recipients.push_back(it->value); 
    }
    else if(it->name == "from")
    {
      msg.from = it->value; 
    }
  }
  
  ::El::Net::SMTP::Session session("localhost");
  session.send(msg);
  
  return 0;
}
/*
int
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string to = "karen@ocslab.com";
  std::string subject = "Test subj";
  std::string text = "Text message";
  std::string html;
  std::string boundary = "----=YYYkjadsasidasdaosdasajsdhaSJDDsadasXXX";
  
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    if(it->name == "subj")
    {
      subject = it->value;
    }
    else if(it->name == "text")
    {
      text = it->value;
    }
    else if(it->name == "html")
    {
      html = it->value;
    }
    else if(it->name == "to")
    {
      to = it->value;
    }
  }
    
  std::string line;

  std::cerr << "Opening\n";
    
  El::Net::Socket::Stream sock("localhost", 25);

  if(std::getline(sock, line))
  {
    std::cerr << line << std::endl;
  }
  else
  {
    throw Exception("Failed to open session");
  }

  std::cerr << "Sending EHLO\n";
  
  sock << "EHLO 127.0.0.1\r\n";
  sock.flush();
  
  while(std::getline(sock, line))
  {
    std::cerr << line << std::endl;

    if(line.find('-') == std::string::npos)
    {
      break;
    }
  }

  std::cerr << "Sending MAIL FROM\n";

  sock << "MAIL FROM: <mailbox@newsfiber.com>\r\n";
  sock.flush();
  
  while(std::getline(sock, line))
  {
    std::cerr << line << std::endl;

    if(line.find('-') == std::string::npos)
    {
      break;
    }
  }
    
  std::cerr << "Sending RCPT TO\n";

  sock << "RCPT TO: <" << to << ">\r\n";
  sock.flush();
  
  while(std::getline(sock, line))
  {
    std::cerr << line << std::endl;

    if(line.find('-') == std::string::npos)
    {
      break;
    }
  }
    
  std::cerr << "Sending DATA\n";

  sock << "DATA\r\n";
  sock.flush();
  
  while(std::getline(sock, line))
  {
    std::cerr << line << std::endl;

    if(line.find('-') == std::string::npos)
    {
      break;
    }
  }
    
  std::cerr << "Sending body\n";

  sock << "From: karen_arutyunov@newsfiber.com\r\n"
    "To: " << to << "\r\nSubject: ";

  El::String::Manip::word_encode((const unsigned char*)subject.c_str(),
                                 subject.length(),
                                 sock);
  
  sock << "\r\n";

  sock << "Content-Type: multipart/alternative; boundary=\""
       << boundary << "\"\r\n\r\n";  

  sock << "--" << boundary
       << "\r\nContent-Type: text/plain; charset=\"utf-8\"\r\n"
    "Content-Transfer-Encoding: base64\r\n";

  El::String::Manip::base64_encode((const unsigned char*)text.c_str(),
                                   text.length(),
                                   sock);

  if(!html.empty())
  {
    sock << "\r\n--" << boundary
         << "\r\nContent-Type: text/html; charset=\"utf-8\"\r\n"
      "Content-Transfer-Encoding: base64\r\n";
    
    El::String::Manip::base64_encode((const unsigned char*)html.c_str(),
                                     html.length(),
                                     sock);
  }
  
  sock << "\r\n--" << boundary << "--\r\n.\r\n";  
  
  sock.flush();

  while(std::getline(sock, line))
  {
    std::cerr << line << std::endl;

    if(line.find('-') == std::string::npos)
    {
      break;
    }
  }

  std::cerr << "Sending RSET\n";

  sock << "RSET\r\n";
  sock.flush();
  
  while(std::getline(sock, line))
  {
    std::cerr << line << std::endl;

    if(line.find('-') == std::string::npos)
    {
      break;
    }
  }
  
  std::cerr << "Sending QUIT\n";

  sock << "QUIT\r\n";
  sock.flush();
  
  while(std::getline(sock, line))
  {
    std::cerr << line << std::endl;

    if(line.find('-') == std::string::npos)
    {
      break;
    }
  }    
  
  return 0;
}
*/
