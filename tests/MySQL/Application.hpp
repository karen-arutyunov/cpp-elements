/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/tests/MySQL/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TESTS_MYSQL_APPLICATION_HPP_
#define _ELEMENTS_TESTS_MYSQL_APPLICATION_HPP_

#include <string>
#include <list>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ServiceBase.hpp>

#include <El/MySQL/DB.hpp>

class Application : public virtual El::Service::Callback
{
public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    
public:
    
  Application() throw(Exception, El::Exception);
  virtual ~Application() throw();

  int run(int& argc, char** argv) throw(InvalidArg, Exception, El::Exception);

private:

  struct Argument
  {
    std::string name;
    std::string value;

    Argument(const char* nm = 0, const char* vl = 0)
      throw(El::Exception);
  };
  
  typedef std::list<Argument> ArgList;

  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int test(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  void test_new_connections_factory() throw(Exception, El::Exception);
  void test_pool_connections_factory() throw(Exception, El::Exception);

  virtual bool notify(El::Service::Event* event) throw(El::Exception);
  
  class DBTester :
    public virtual El::Service::ServiceBase<El::Sync::ThreadRWPolicy>,
    public virtual El::RefCount::DefaultImpl<>
  {
  public:
    DBTester(Callback* callback, El::MySQL::DB* dbase)
      throw(InvalidArg, El::Exception);
    
    virtual ~DBTester() throw();
    virtual void run() throw(Exception, El::Exception);

    std::string error;

  private:
    El::MySQL::DB_var dbase_;
  };

  typedef El::RefCount::SmartPtr<DBTester> DBTester_var;
  
private:
  std::string user_;
  std::string passwd_;
  std::string db_;
  std::string host_;
  std::string unix_socket_;
  unsigned short port_;
  unsigned long client_flag_;
  DBTester_var dbtester_;
};

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

//
// Application::Argument class
//
inline
Application::Argument::Argument(const char* nm, const char* vl)
  throw(El::Exception)
    : name(nm ? nm : ""),
      value(vl ? vl : "")
{
}

#endif // _ELEMENTS_TESTS_MYSQL_APPLICATION_HPP_
