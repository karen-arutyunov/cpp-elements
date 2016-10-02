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

#include <stdlib.h>

#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

#include <El/Moment.hpp>
#include <El/Service/ThreadPool.hpp>
#include <El/Service/Timer.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestTimer [help]\n";

  const unsigned long EVENTS_COUNT = 10000;
  const unsigned long TIME_RANGE = 10;
}

int
main(int argc, char** argv)
{
  srand(time(0));
  
  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'TestTimer help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestTimer: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestTimer: unknown exception caught\n";
  }
  
  return -1;
}

Application::Application() throw(Application::Exception, El::Exception)
    : intimes_(0),
      delays_(0),
      earliers_(0)
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

  test(arguments, false);
  test(arguments, true);
  
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
Application::test(const ArgList& arguments, bool minimize_delay)
  throw(InvalidArg, Exception, El::Exception)
{
  if(minimize_delay)
  {
    std::cerr << "\nDO minimizing delayes ...\n";
  }
  else
  {
    std::cerr << "\nNOT minimizing delayes ...\n";
  }
  
  intimes_ = 0;
  delays_ = 0;
  earliers_ = 0;

  delay_total_time_ = ACE_Time_Value::zero;
  earlier_total_time_ = ACE_Time_Value::zero;
  last_time_ = ACE_Time_Value::zero;

  El::Service::Timer_var timer(
    new El::Service::Timer(this, "Timer", 0, minimize_delay));

  timer->start();

  ACE_Time_Value cur_time = ACE_OS::gettimeofday();
  
  El::Service::Timer::Event_var event;

  unsigned long shift = 0;

  for(unsigned long i = 0; i < EVENTS_COUNT; i++)
  {
    unsigned long long usec = (unsigned long long)rand() *
      TIME_RANGE * 1000000 / ((unsigned long long)RAND_MAX + 1);

    ACE_Time_Value tm = cur_time +
      ACE_Time_Value(usec / 1000000 + shift, usec % 1000000);

    event = new TestEvent(this, 0);

    timer->set(event.in(), tm);
  }

  timer->validate();

  ACE_OS::sleep(shift);

  for(unsigned long i = 0; i < TIME_RANGE; i++)
  {
    ACE_OS::sleep(1);
    timer->validate();
  }

  ACE_OS::sleep(3);
  
  timer->stop();
  timer->wait();

  timer->validate();

  unsigned long events = intimes_ + delays_ + earliers_;
  
  if(events != EVENTS_COUNT)
  {
    std::ostringstream ostr;
    ostr << "Application::test: unexpected number of events received - "
         << events << " instead of " << EVENTS_COUNT;
    
    throw Exception(ostr.str()); 
  }

  ACE_Time_Value avg_delay;

  if(delays_)
  {
    avg_delay = El::Moment::divide(delay_total_time_ , delays_);
  }
  
  ACE_Time_Value avg_earlier;

  if(earliers_)
  {
    avg_earlier = El::Moment::divide(earlier_total_time_, earliers_);
  }
    
  std::cerr << "In time: " << intimes_ << "\nDelays: " << delays_
            << ", avg " << El::Moment::time(avg_delay)
            << "\nEarliers: " << earliers_ << ", avg "
            << El::Moment::time(avg_earlier) << std::endl;

  return 0;
}

bool
Application::notify(El::Service::Event* event) throw(El::Exception)
{
  ACE_Time_Value cur_time = ACE_OS::gettimeofday();
  
  El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);

  if(error)
  {
    std::cerr << "Application::notify: " << *error << std::endl;
    return true;
  }

  TestEvent* test_event = dynamic_cast<TestEvent*>(event);

  if(test_event)
  {
    {
      WriteGuard guard(lock_);

      if(last_time_ > test_event->time)
      {
        std::cerr << "Application::notify: wrong event order\n";
      }

      if(cur_time > test_event->time)
      {
        delays_++;
        delay_total_time_ += cur_time - test_event->time;
      }
      else if(cur_time < test_event->time)
      {
        earliers_++;
        earlier_total_time_ += test_event->time - cur_time;
      }
      else
      {
        intimes_++;
      }
    }
    
    return true;
  }

  std::cerr << "Application::notify: unknown " << *event << std::endl;

  return false;
}

