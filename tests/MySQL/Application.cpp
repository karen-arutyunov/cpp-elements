/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/test/MySQL/Application.cpp
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

namespace
{
  const char USAGE[] =
"\nUsage:\nTestMySQL <command> <command arguments>\n\n"
"Synopsis 1:\n"
"TestMySQL help\n\n"
"Synopsis 2:\n"
"TestMySQL test <command arguments>\n"
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

    int res = app.run(argc, argv);

    std::cerr << "Test " << (res ? "failed" : "succeeded") << std::endl;
    return  res;

  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'TestMySQL help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestMySQL: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestMySQL: unknown exception caught\n";
  }
  
  return -1;
}

//
// Application class
//
Application::Application() throw(Application::Exception, El::Exception)
    : port_(0),
      client_flag_(0)
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
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    if(it->name == "user")
    {
      user_ = it->value;
    }
    else if(it->name == "passwd")
    {
      passwd_ = it->value;
    }
    else if(it->name == "db")
    {
      db_ = it->value;
    }
    else if(it->name == "host")
    {
      host_ = it->value;
    }
    else if(it->name == "port")
    {
      port_ = atol(it->value.c_str());
    }
    else if(it->name == "client_flag")
    {
      client_flag_ = atoll(it->value.c_str());
    }
    else if(it->name == "unix_socket")
    {
      unix_socket_ = it->value;
    }
  }

  test_new_connections_factory();
  test_pool_connections_factory();
  return 0;
}

void
Application::test_new_connections_factory()
  throw(Exception, El::Exception)
{
  El::MySQL::DB_var dbase = unix_socket_.empty() ?
    new El::MySQL::DB(user_.c_str(),
                      passwd_.c_str(),
                      db_.c_str(),
                      port_,
                      host_.c_str(),
                      client_flag_) :
    new El::MySQL::DB(user_.c_str(),
                      passwd_.c_str(),
                      db_.c_str(),
                      unix_socket_.c_str(),
                      client_flag_);

  El::MySQL::Connection_var connection = dbase->connect();

  El::MySQL::Result_var result = connection->query("drop table TestMySQL");

  result =
    connection->query("create table TestMySQL (name VARCHAR(255) NOT NULL)");

  result =
    connection->query("insert into TestMySQL set name='TestName'");

  result = connection->query("select * from TestMySQL");

  El::MySQL::Row row(result.in());
  
  while(row.fetch_row())
  {
    std::string value = row.string(0);
    
    if(value != "TestName")
    {
      std::ostringstream ostr;
      ostr << "Application::test_new_connections_factory: unexpected "
        "field value " << value;

      throw Exception(ostr.str());
    }
    
  }  
}

void
Application::test_pool_connections_factory()
  throw(Exception, El::Exception)
{
  El::MySQL::ConnectionPoolFactory* factory =
    new El::MySQL::ConnectionPoolFactory(2, 4, 0, 0);
        
  El::MySQL::DB_var dbase = unix_socket_.empty() ?
    new El::MySQL::DB(user_.c_str(),
                      passwd_.c_str(),
                      db_.c_str(),
                      port_,
                      host_.c_str(),
                      client_flag_,
                      factory) :
    new El::MySQL::DB(user_.c_str(),
                      passwd_.c_str(),
                      db_.c_str(),
                      unix_socket_.c_str(),
                      client_flag_,
                      factory);

  El::MySQL::Connection_var connection1 = dbase->connect();

  El::MySQL::Result_var result = connection1->query("drop table TestMySQL");

  El::MySQL::Connection_var connection2 = dbase->connect();

  result =
    connection2->query("create table TestMySQL (name VARCHAR(255) NOT NULL)");

  El::MySQL::Connection_var connection3 = dbase->connect();

  result =
    connection3->query("insert into TestMySQL set name='TestName'");

  El::MySQL::Connection_var connection4 = dbase->connect();

  result = connection4->query("select * from TestMySQL");
  
  {
    El::MySQL::Row row(result.in());
  
    while(row.fetch_row())
    {
      std::string value = row.string(0);
    
      if(value != "TestName")
      {
        std::ostringstream ostr;
        ostr << "Application::test_pool_connections_factory: unexpected "
          "field value " << value;

        throw Exception(ostr.str());
      }
    }
  }
  
  result = 0;

  bool timeout = false;
  
  try
  {
    ACE_Time_Value to(3);
    
    El::MySQL::Connection_var connection5 = dbase->connect(&to);
    result = connection5->query("select * from TestMySQL");
  }
  catch(const El::MySQL::Timeout&)
  {
    timeout = true;
  }

  if(!timeout)
  {
    throw Exception("Application::test_pool_connections_factory: connection "
                    "unexpectedly provided");
  }

  dbtester_ = new DBTester(this, dbase);
  dbtester_->start();  

  ACE_OS::sleep(3);
  connection4 = 0;
  connection3 = 0;

  dbtester_->stop();
  dbtester_->wait();

  if(!dbtester_->error.empty())
  {
    std::ostringstream ostr;
    ostr << "Application::test_pool_connections_factory: dbtester failed "
         << dbtester_->error;

    throw Exception(ostr.str());
  }
  
  connection1 = 0;
  connection2 = 0;
}

bool
Application::notify(El::Service::Event* event) throw(El::Exception)
{
  El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);

  std::ostringstream ostr;
  
  if(error)
  {
    std::ostringstream ostr;
    ostr << "Application::notify: " << *error;
    dbtester_->error = ostr.str();
  }
  else
  {
    std::cerr << "Application::notify: " << *event;
  }

  return true;
}

//
// Application::DBTester class
//
Application::DBTester::DBTester(Callback* callback, El::MySQL::DB* dbase)
  throw(InvalidArg, El::Exception)
    : El::Service::ServiceBase<El::Sync::ThreadRWPolicy>(callback, "DBTester"),
      dbase_(El::RefCount::add_ref(dbase))
{
}

Application::DBTester::~DBTester() throw()
{
}

void
Application::DBTester::run() throw(Exception, El::Exception)
{
  bool timeout = false;
  
  try
  {
    ACE_Time_Value to(2);
    
    El::MySQL::Connection_var connection = dbase_->connect(&to);
    
    El::MySQL::Result_var result =
      connection->query("select * from TestMySQL");
  }
  catch(const El::MySQL::Timeout&)
  {
    timeout = true;
  }

  if(!timeout)
  {
    El::Service::Error error(
      "Application::DBTester::run: connection (1) "
                "unexpectedly provided", this);

    callback_->notify(&error);
  }
  
  ACE_OS::sleep(3);

  ACE_Time_Value to(2);
  
  El::MySQL::Connection_var connection1 = dbase_->connect(&to);
  El::MySQL::Result_var result = connection1->query("select * from TestMySQL");

  El::MySQL::Connection_var connection2 = dbase_->connect(&to);
  result = connection2->query("select * from TestMySQL");

  timeout = false;
  
  try
  {
    ACE_Time_Value to(2);
    
    El::MySQL::Connection_var connection = dbase_->connect(&to);
    
    El::MySQL::Result_var result =
      connection->query("select * from TestMySQL");
  }
  catch(const El::MySQL::Timeout&)
  {
    timeout = true;
  }

  if(!timeout)
  {
    El::Service::Error error("Application::DBTester::run: connection (2) "
                             "unexpectedly provided", this);

    callback_->notify(&error);
  }
}

