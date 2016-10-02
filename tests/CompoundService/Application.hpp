/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/tests/ThreadPool/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TESTS_THREADPOOL_APPLICATION_HPP_
#define _ELEMENTS_TESTS_THREADPOOL_APPLICATION_HPP_

#include <string>
#include <list>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/Service/Service.hpp>
#include <El/Service/CompoundService.hpp>

class Application : public virtual El::Service::Callback
{
public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    
public:
    
  Application() throw(Exception, El::Exception);
  virtual ~Application() throw();

  int run(int& argc, char** argv) throw(InvalidArg, Exception, El::Exception);

  virtual bool notify(El::Service::Event* event) throw(El::Exception);
  
private:

  struct Argument
  {
    std::string name;
    std::string value;

    Argument(const char* nm = 0, const char* vl = 0)
      throw(El::Exception);
  };
  
  typedef std::list<Argument> ArgList;

  struct TestEvent : public virtual El::Service::ThreadPool::ServiceEvent
  {
    TestEvent(El::Service::Callback* callback, El::Service::Service* source)
      throw(InvalidArg, Exception, El::Exception);
  };

  class TestService : public virtual El::Service::CompoundService<>
  {
  public:
    TestService(Callback* callback) throw(InvalidArg, El::Exception);
  };

  typedef El::RefCount::SmartPtr<TestService> TestService_var;
  
  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int test(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

private:
  typedef ACE_RW_Thread_Mutex     Mutex;
  typedef ACE_Read_Guard<Mutex>  ReadGuard;
  typedef ACE_Write_Guard<Mutex> WriteGuard;
  
  mutable Mutex lock_;

  unsigned long counter_;
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

//
// Application::Argument class
//
inline
Application::TestEvent::TestEvent(El::Service::Callback* callback,
                                  El::Service::Service* source)
  throw(InvalidArg, Exception, El::Exception)
    : El__Service__ThreadPool__ServiceEventBase(callback, source, false)
{
}


#endif // _ELEMENTS_TESTS_THREADPOOL_APPLICATION_HPP_
