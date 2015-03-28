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
 * @file   Elements/test/MySQLClassGen/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <string>
#include <iostream>
#include <list>
#include <sstream>

#include <El/Moment.hpp>

#include "Application.hpp"
#include "Record.hpp"

namespace
{
  const char USAGE[] =
"\nUsage:\nTestMySQLClassGen <command> <command arguments>\n\n"
"Synopsis 1:\n"
"TestMySQLClassGen help\n\n"
"Synopsis 2:\n"
"TestMySQLClassGen test <command arguments>\n"
"  command arguments ::= "
"[user=<user>] "
"[passwd=<passwd>] "
"[db=<db>] "
"[host=<host>] [port=<port>] [unix_socket=<unix_socket>] "
"[client_flag=<client_flag>]\n";

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
              << "\nRun 'TestMySQLClassGen help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestMySQLClassGen: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestMySQLClassGen: unknown exception caught\n";
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

  if(command == "test")
  {
    test(arguments);
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
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string user;
  std::string passwd;
  std::string db;
  std::string host;
  std::string unix_socket;
  unsigned short port = 0;
  unsigned long client_flag = 0;

  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    if(it->name == "user")
    {
      user = it->value;
    }
    else if(it->name == "passwd")
    {
      passwd = it->value;
    }
    else if(it->name == "db")
    {
      db = it->value;
    }
    else if(it->name == "host")
    {
      host = it->value;
    }
    else if(it->name == "port")
    {
      port = atol(it->value.c_str());
    }
    else if(it->name == "client_flag")
    {
      client_flag = atoll(it->value.c_str());
    }
    else if(it->name == "unix_socket")
    {
      unix_socket = it->value;
    }
  }
  
  El::MySQL::DB_var dbase = unix_socket.empty() ?
    new El::MySQL::DB(user.c_str(),
                      passwd.c_str(),
                      db.c_str(),
                      port,
                      host.c_str(),
                      client_flag) :
    new El::MySQL::DB(user.c_str(),
                      passwd.c_str(),
                      db.c_str(),
                      unix_socket.c_str(),
                      client_flag);

  connection_ = dbase->connect();

  std::string query = "select * from AllTypes";

  El::MySQL::Result_var result = connection_->query(query.c_str());
  
  Namespace1::Namespace2::Record record(result.in());

  while(record.fetch_row())
  {
    El::Moment created = record.created();
    created.tm_tz = El::Moment::TZ_LOCAL;
    
    std::cerr << "id=" << record.id()
              << ", url=" << record.url()
              << ", created=" << created.rfc0822()
              << ", creator_type=" << record.creator_type()
              << ", creator_name=" << record.creator_name()
              << ", activity=" << record.activity()
              << ", density=" << record.density()
              << ", rate=" << record.rate()
              << ", flags=" << record.flags()
              << ", start_year=" << record.start_year()
              << ", timeX=" << record.timeX()
              << ", rgb=" << record.rgb()
              << ", comment=" << record.comment()
              << std::endl;
  }
  
  return 0;
}

