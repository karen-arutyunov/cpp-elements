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
 * @file Elements/El/Timer.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_TIMER_HPP_
#define _ELEMENTS_EL_TIMER_HPP_

#include <limits.h>

#include <list>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/SyncPolicy.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ServiceBase.hpp>

namespace El
{
  namespace Service
  {
    class Timer :
      public virtual ServiceBase<El::Sync::ThreadPolicy>,
      public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
    {
    public:
      EL_EXCEPTION(Exception, El::Service::Exception);
      EL_EXCEPTION(InvalidArg, Exception);
      EL_EXCEPTION(ImplementationException, Exception);
      
    public:
      
      class Event :
        public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
        
      {
      public:
        virtual ~Event() throw();
        virtual void trigger() throw(El::Exception) = 0;

      public:
        ACE_Time_Value time;
      };

      typedef El::RefCount::SmartPtr<Event> Event_var;

      class ServiceEvent : public virtual Event,
                           public virtual El::Service::ServiceEvent
      {
      public:
        ServiceEvent(Callback* callback, Service* source)
          throw(InvalidArg, Exception, El::Exception);
        
        virtual ~ServiceEvent() throw();
        
        virtual void trigger() throw(El::Exception);
      };

    public:
      
      Timer(Callback* callback,
            const char* name = 0,
            size_t stack_size = 0,
            bool minimize_delay = false)
        throw(InvalidArg, El::Exception);

      virtual ~Timer() throw();

      void set(Event* event, const ACE_Time_Value& time)
        throw(InvalidArg, El::Exception);

      virtual bool stop() throw(Exception, El::Exception);

      void validate() const throw(ImplementationException, El::Exception);

      void clear_buckets() throw(El::Exception);

    protected:
      virtual void run() throw(Exception, El::Exception);

    protected:

      typedef std::list<Event_var> EventList;
      
      struct Backet
      {
        ACE_Time_Value min_time;
        ACE_Time_Value max_time;

        EventList events;
        unsigned long events_count;

        Backet() throw(El::Exception);
      };

      typedef std::list<Backet> BacketList;

      BacketList backets_;
      BacketList::iterator last_backet_;
      unsigned long backets_count_;

      typedef ACE_Condition<ACE_Thread_Mutex> Condition;
      Condition event_happened_;

      bool minimize_delay_;
      ACE_Time_Value avg_delay_;
    };

    typedef El::RefCount::SmartPtr<Timer> Timer_var;
  }
  
}

#define El__Service__Timer__ServiceEventBase(callback, source) \
          El::Service::Event(source), \
          El::Service::ServiceEvent(callback, source), \
          El::Service::Timer::ServiceEvent(callback, source) \

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Service
  {
    //
    // ThreadPool class
    //
    inline
    Timer::Timer(Callback* callback,
                 const char* name,
                 size_t stack_size,
                 bool minimize_delay)
      throw(InvalidArg, El::Exception)
        : ServiceBase<El::Sync::ThreadPolicy>(callback, name, 1, stack_size),
          backets_count_(0),
          event_happened_(srv_lock_),
          minimize_delay_(minimize_delay)
    {
      last_backet_ = backets_.end();
    }
    
    inline
    Timer::~Timer() throw()
    {
    }
    
    inline
    bool
    Timer::stop() throw(Exception, El::Exception)
    {
      bool ret = ServiceBase<El::Sync::ThreadPolicy>::stop();
      event_happened_.signal();
      
      return ret;
    }

    inline
    void
    Timer::clear_buckets() throw(El::Exception)
    {
      WriteGuard guard(srv_lock_);
      
      backets_.clear();
      last_backet_ = backets_.end();
      backets_count_ = 0;
    }
    
    //
    // Timer::Event class
    //
    inline
    Timer::Event::~Event() throw()
    {
    }

    //
    // Timer::ServiceEvent class
    //
    inline
    Timer::ServiceEvent::ServiceEvent(Callback* callback, Service* source)
      throw(InvalidArg, Exception, El::Exception)
        : El::Service::Event(source),
          El::Service::ServiceEvent(callback, source)
    {
      if(callback == 0)
      {
        throw InvalidArg(
          "El::Service::Timer::ServiceEvent::ServiceEvent: "
          "callback is null");
      }
    }
    
    inline
    Timer::ServiceEvent::~ServiceEvent() throw()
    {
    }
    
    inline
    void 
    Timer::ServiceEvent::trigger() throw(El::Exception)
    {
      callback_->notify(this);
    }

    //
    // Timer::Backet class
    //
    inline
    Timer::Backet::Backet() throw(El::Exception)
        : events_count(0)
    {
    }
    
  }
}

#endif // _ELEMENTS_EL_TIMER_HPP_
