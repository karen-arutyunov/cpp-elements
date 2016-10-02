/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/test/ThreadPool/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <string.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

#include <El/Service/ThreadPool.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestObjectCache [help]\n";
  const char FILE_NAME[] = "TestObjectCache.tmp";
  const char CACHE_CONTENT[] =
  "This is content\nfor cache test <<VAR1>>.\n"
  "This is content\nfor cache test <<VAR2>>.\n"
  "This is content\nfor cache test <<VAR3>>.\n"
  "This is content\nfor cache test <<VAR4>>.\n"
  "This is content\nfor cache test <<VAR5>>.\n"
  "This is content\nfor cache test <<VAR6>>.\n"
  "This is content\nfor cache test <<VAR7>>.\n";
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
              << "\nRun 'TestObjectCache help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestObjectCache: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestObjectCache: unknown exception caught\n";
  }
  
  return -1;
}

Application::Application() throw(Application::Exception, El::Exception)
    : cache_(ACE_Time_Value(1)),
      template_cache_("<<", ">>", ACE_Time_Value(1)),
      phase_(TP_NO_FILE),
      failed_(false)
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
  int res = test_no_file() && test_existing_file() && test_template_file() ?
    0 : -1;
  
  unlink(FILE_NAME);
  return res; 
}

bool
Application::test_no_file() throw(El::Exception)
{
  std::cerr << "Starting \"no file\" phase ...\n";
  phase_ = TP_NO_FILE;

  unlink(FILE_NAME);

  stat_.dump_header("Not existing file access");
  stat_.reset();

  El::Service::ThreadPool_var thr_pool(
    new El::Service::ThreadPool(this, "ThreadPool", 100));

  El::Service::ThreadPool::Task_var event;
  
  for(unsigned long i = 0; i < 10000; i++)
  {
    event = new TestEvent(this, 0);
    thr_pool->execute(event);
  }

  thr_pool->start();

  ACE_OS::sleep(3);
  
  std::cerr << "Stopping ...\n";
  thr_pool->stop();

  std::cerr << "Waiting ...\n";
  thr_pool->wait();

  stat_.dump(std::cerr);

  return !failed_;
}

bool
Application::test_existing_file() throw(El::Exception)
{
  std::cerr << "Starting \"existing file\" phase ...\n";
  phase_ = TP_FILE_EXIST;

  ACE_OS::sleep(ACE_Time_Value(3));
  
  {
    std::fstream file(FILE_NAME, std::ios::out);
    file << CACHE_CONTENT;
  }
  
  stat_.dump_header("Existing file access");
  stat_.reset();

  El::Service::ThreadPool_var thr_pool(
    new El::Service::ThreadPool(this, "ThreadPool", 100));

  El::Service::ThreadPool::Task_var event;
  
  for(unsigned long i = 0; i < 10000; i++)
  {
    event = new TestEvent(this, 0);
    thr_pool->execute(event);
  }

  thr_pool->start();

  ACE_OS::sleep(3);
  
  std::cerr << "Stopping ...\n";
  thr_pool->stop();

  std::cerr << "Waiting ...\n";
  thr_pool->wait();

  stat_.dump(std::cerr);

  return !failed_;
}

bool
Application::test_template_file() throw(El::Exception)
{
  std::cerr << "Starting \"template file\" phase ...\n";
  phase_ = TP_TEMPLATE_FILE;

  ACE_OS::sleep(ACE_Time_Value(3));
  
  {
    std::fstream file(FILE_NAME, std::ios::out);
    file << CACHE_CONTENT;
  }

  variables_["VAR1"] = "1";
  variables_["VAR2"] = "22";
  variables_["VAR3"] = "333";
  variables_["VAR4"] = "4444";
  variables_["VAR5"] = "55555";
  variables_["VAR6"] = "666666";
  variables_["VAR7"] = "7777777";

  El::String::Template::Parser parser(CACHE_CONTENT, "<<", ">>");
  
  template_sample_ = parser.instantiate(variables_);
  
  stat_.dump_header("Template file access");
  stat_.reset();

  El::Service::ThreadPool_var thr_pool(
    new El::Service::ThreadPool(this, "ThreadPool", 100));

  El::Service::ThreadPool::Task_var event;
  
  for(unsigned long i = 0; i < 10000; i++)
  {
    event = new TestEvent(this, 0);
    thr_pool->execute(event);
  }

  thr_pool->start();

  ACE_OS::sleep(3);
  
  std::cerr << "Stopping ...\n";
  thr_pool->stop();

  std::cerr << "Waiting ...\n";
  thr_pool->wait();

  stat_.dump(std::cerr);

  return !failed_;
}

bool
Application::notify(El::Service::Event* event)
  throw(El::Exception)
{
  El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);

  if(error)
  {
    std::cerr << "Application::notify: " << *error;
    return true;
  }

  TestEvent* test_event = dynamic_cast<TestEvent*>(event);

  if(test_event)
  {
    switch(phase_)
    {
    case TP_NO_FILE:
      {
        process_no_file();
        break;
      }
    case TP_FILE_EXIST:
      {
        process_existing_file();
        break;
      }
    case TP_TEMPLATE_FILE:
      {
        process_template_file();
        break;
      }
    default: break;
    }
    
    return true;
  }

  std::cerr << "Application::notify: unknown " << *event << std::endl;

  return false;
}

void
Application::process_no_file() throw(El::Exception)
{
  try
  {
    stat_.start();
    El::Cache::TextFile_var object = cache_.get(FILE_NAME);
    stat_.stop();
  }
  catch(const El::Cache::NotFound& )
  {
    stat_.stop();
    return;
  }
  catch(const El::Cache::Exception& e)
  {
    stat_.stop();

    WriteGuard guard(lock_);
    std::cerr << "Application::test_no_file: El::Cache::Exception caught."
      " Description: " << e << std::endl;
    
    failed_ = true;
  }
  
  WriteGuard guard(lock_);
  std::cerr << "Application::test_no_file: NotFound exception was expected\n"; 

  failed_ = true;
}

void
Application::process_existing_file() throw(El::Exception)
{
  try
  {
    stat_.start();
    El::Cache::TextFile_var object = cache_.get(FILE_NAME);
    stat_.stop();

    if(!object->size() || strcmp(object->text(), CACHE_CONTENT))
    {
      WriteGuard guard(lock_);
      std::cerr << "Application::process_existing_file: object '"
                << (object->text() ? object->text() : "null")
                << "' received; expected ' " << CACHE_CONTENT << "'\n";
      
      failed_ = true;
    }
  }
  catch(const El::Cache::Exception& e)
  {
    stat_.stop();

    WriteGuard guard(lock_);
    std::cerr << "Application::process_existing_file: "
      "El::Cache::Exception caught. Description: " << e << std::endl;
    
    failed_ = true;
  }  
}

void
Application::process_template_file() throw(El::Exception)
{
  try
  {
    stat_.start();

    El::Cache::TextTemplateFile_var object = template_cache_.get(FILE_NAME);
    std::string text = object->instantiate(variables_);
    
    stat_.stop();

    if(text != template_sample_)
    {
      WriteGuard guard(lock_);
      std::cerr << "Application::process_template_file: '"
                << text << "' instantiated; expected ' " << template_sample_
                << "'\n";
      
      failed_ = true;
    }
  }
  catch(const El::Cache::Exception& e)
  {
    stat_.stop();

    WriteGuard guard(lock_);
    std::cerr << "Application::process_existing_file: "
      "El::Cache::Exception caught. Description: " << e << std::endl;
    
    failed_ = true;
  }  
}
