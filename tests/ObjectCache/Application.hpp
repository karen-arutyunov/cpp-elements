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
 * @file   Elements/tests/ObjectCache/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TESTS_OBJECTCACHE_APPLICATION_HPP_
#define _ELEMENTS_TESTS_OBJECTCACHE_APPLICATION_HPP_

#include <string>
#include <list>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ThreadPool.hpp>
#include <El/Cache/TextFileCache.hpp>
#include <El/Cache/TextTemplateFileCache.hpp>
#include <El/String/Template.hpp>
#include <El/Stat.hpp>

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

  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int test(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  bool test_no_file() throw(El::Exception);
  bool test_existing_file() throw(El::Exception);
  bool test_template_file() throw(El::Exception);
  
  void process_no_file() throw(El::Exception);
  void process_existing_file() throw(El::Exception);
  void process_template_file() throw(El::Exception);

private:
  typedef ACE_RW_Thread_Mutex    Mutex;
  typedef ACE_Read_Guard<Mutex>  ReadGuard;
  typedef ACE_Write_Guard<Mutex> WriteGuard;
  
  mutable Mutex lock_;

  El::Cache::TextFileCache cache_;

  El::String::Template::VariablesMap variables_;
  El::Cache::TextTemplateFileCache template_cache_;
  std::string template_sample_;
  
  El::Stat::TimeMeter stat_;

  enum TestPhase
  {
    TP_INITIAL,
    TP_NO_FILE,
    TP_FILE_EXIST,
    TP_TEMPLATE_FILE
  };
  
  TestPhase phase_;
  bool failed_;
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
    : El__Service__ThreadPool__ServiceEventBase(callback, source, true)
{
}

#endif // _ELEMENTS_TESTS_OBJECTCACHE_APPLICATION_HPP_