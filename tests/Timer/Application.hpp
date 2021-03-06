/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/tests/Timer/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TESTS_TIMER_APPLICATION_HPP_
#define _ELEMENTS_TESTS_TIMER_APPLICATION_HPP_

#include <string>
#include <list>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ThreadPool.hpp>
#include <El/Service/Timer.hpp>

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

  struct TestEvent : public virtual El::Service::ThreadPool::ServiceEvent,
                     public virtual El::Service::Timer::ServiceEvent
  {
    TestEvent(El::Service::Callback* callback, El::Service::Service* source)
      throw(InvalidArg, Exception, El::Exception);
  };

  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int test(const ArgList& arguments, bool minimize_delay)
    throw(InvalidArg, Exception, El::Exception);

private:
  typedef ACE_RW_Thread_Mutex    Mutex;
  typedef ACE_Read_Guard<Mutex>  ReadGuard;
  typedef ACE_Write_Guard<Mutex> WriteGuard;
  
  mutable Mutex lock_;

  unsigned long intimes_;
  unsigned long delays_;
  unsigned long earliers_;

  ACE_Time_Value delay_total_time_;
  ACE_Time_Value earlier_total_time_;
  ACE_Time_Value last_time_;
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
    : El__Service__ThreadPool__ServiceEventBase(callback, source, false),
      El::Service::Timer::ServiceEvent(callback, source)
{
}


#endif // _ELEMENTS_TESTS_TIMER_APPLICATION_HPP_
