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

#include <sstream>

#include <ace/OS.h>

#include <El/Moment.hpp>

#include "Timer.hpp"

namespace El
{
  namespace Service
  {
    void
    Timer::run() throw(Exception, El::Exception)
    {
      while(true)
      {
        WriteGuard guard(srv_lock_);

        if(stop_)
        {
          backets_.clear();
          last_backet_ = backets_.end();
          backets_count_ = 0;
          break;
        }
        
        ACE_Time_Value next_event_time;
        ACE_Time_Value cur_time;

        BacketList::iterator bit = backets_.begin();
        
        if(bit != backets_.end())
        {
          Backet& backet = *bit;
          
          EventList& events = backet.events;
          Event_var event = El::RefCount::add_ref(events.begin()->in());
          
          next_event_time = event->time;
          
          cur_time = ACE_OS::gettimeofday();

          if(next_event_time <= cur_time ||
             (minimize_delay_ && (next_event_time - cur_time) <= avg_delay_))
          {
            events.erase(events.begin());

            if(events.empty())
            {
              backets_.erase(bit);
              backets_count_--;

              if(backets_count_ == 0)
              {
                last_backet_ = backets_.end();
              }
            }
            else
            {
              backet.events_count--;
              backet.min_time = events.begin()->in()->time;
              backet.max_time = events.rbegin()->in()->time;
            }

//            avg_delay_ = El::Moment::divide(avg_delay_, 2);

            guard.release();
            event->trigger();
            continue;
          }
          
        }

        bool with_timeout = next_event_time != ACE_Time_Value::zero;

        if(with_timeout && minimize_delay_)
        {
          next_event_time -= avg_delay_;
        }

        if(event_happened_.wait(with_timeout ? &next_event_time : 0))
        {
          int error = ACE_OS::last_error();
          
          if(with_timeout && error == ETIME)
          {
            if(minimize_delay_)
            {
              ACE_Time_Value cur_time = ACE_OS::gettimeofday();

              if(cur_time > next_event_time)
              {
                avg_delay_ =
                  El::Moment::divide(avg_delay_ + (cur_time - next_event_time),
                                     3);
/*
                std::cerr << "DL: " << El::Moment::time(avg_delay_)
                          << std::endl;
*/              
              }
              else
              {
                avg_delay_ = El::Moment::divide(avg_delay_, 3);
/*              
                std::cerr << "BF: " << El::Moment::time(avg_delay_)
                          << std::endl;
*/
              }
            }
            
            continue;
          }
          else
          {
            std::ostringstream ostr;
            
            ostr << "El::Timer::run: event_happened_.wait() failed. "
              "Errno " << error << ". Description:" << std::endl
                 << ACE_OS::strerror(error);
            
            throw Exception(ostr.str());
          }
        }
      }
      
    }

    void
    Timer::set(Event* event, const ACE_Time_Value& time)
      throw(InvalidArg, El::Exception)
    {
      if(event == 0)
      {
        throw InvalidArg("El::Service::Timer::set: event is null");
      }

      event->time = time;

      if(time <= ACE_OS::gettimeofday())
      {
        event->trigger();
        return;
      }

      WriteGuard guard(srv_lock_);

      if(backets_count_ == 0)
      {
        //
        // Is empty; just insert first backet
        //
        
        backets_.push_back(Backet());
        last_backet_ = backets_.begin();
        backets_count_++;
        
        Backet& backet = *backets_.begin();
        
        backet.min_time = time;
        backet.max_time = time;

        Event_var event_ptr(El::RefCount::add_ref(event));
        
        backet.events.push_back(event_ptr);
        backet.events_count++;

        event_happened_.signal();
        
        return;
      }

      BacketList::iterator bit;

      if(time >= last_backet_->min_time)
      {
        //
        // Goes into the last backet
        //
        
        bit = last_backet_;
      }
      else
      {
        //
        // Goes somewhere in the middle (not the last backet)
        //
        
        for(bit = backets_.begin(); time > bit->max_time; bit++);
      }

      if(bit->events_count > backets_count_ && bit->events_count > 4)
      {
        //
        // Split the backet if too long.
        // In general will try to keep backet length same as backet count
        // to optimize search
        //
        
        Backet& backet = *bit;
        
        unsigned long skip = backet.events_count / 2;
        EventList::iterator it = backet.events.begin();
        
        for(unsigned long i = skip; i; i--, it++);

        BacketList::iterator new_bit = backets_.insert(bit, Backet());
        Backet& new_backet = *new_bit;

        new_backet.events.splice(new_backet.events.begin(),
                                 backet.events,
                                 backet.events.begin(),
                                 it);
        
        backets_count_++;

        new_backet.min_time = new_backet.events.begin()->in()->time;
        new_backet.max_time = new_backet.events.rbegin()->in()->time;
        new_backet.events_count = skip;

        backet.min_time = backet.events.begin()->in()->time;
        backet.events_count -= skip;

        if(time <= new_backet.max_time)
        {
          bit = new_bit;
        }
      }

      Backet& backet = *bit;
      
      Event_var event_ptr(El::RefCount::add_ref(event));

      EventList& events = backet.events;

      if(backet.min_time >= time)
      {
        backet.min_time = time;
        events.push_front(event_ptr);
      }
      else if(backet.max_time <= time)
      {
        backet.max_time = time;
        events.push_back(event_ptr);
      }
      else
      {
        EventList::iterator it = events.begin();
        for(; time > it->in()->time; it++);
        
        events.insert(it, event_ptr);
      }
      
      backet.events_count++;

      event_happened_.signal();
    }

    void
    Timer::validate() const throw(ImplementationException, El::Exception)
    {
      ReadGuard guard(srv_lock_);

      unsigned long backets_count = 0;
      ACE_Time_Value time;
      
      BacketList::const_iterator last_backet = backets_.end();
        
      for(BacketList::const_iterator bit = backets_.begin();
          bit != backets_.end(); bit++, backets_count++)
      {
        last_backet = bit;
        
        const Backet& backet = *bit;
        const EventList& events = backet.events;
          
        if(events.empty())
        {
          throw ImplementationException(
            "El::Timer::validate: empty backet encountered");
        }

        unsigned long events_count = 0;
        
        for(EventList::const_iterator it = events.begin(); it != events.end();
            it++, events_count++)
        {
          Event* event = it->in();

          if(event->time < backet.min_time)
          {
            throw ImplementationException(
              "El::Timer::validate: event is out of backet range "
              "(less min_time)");
          }

          if(event->time > backet.max_time)
          {
            throw ImplementationException(
              "El::Timer::validate: event is out of backet range "
              "(more max_time)");
          }

          if(event->time < time)
          {
            throw ImplementationException(
              "El::Timer::validate: wrong event order");
          }
          
          time = event->time;
        }

        if(events_count != backet.events_count)
        {
          throw ImplementationException(
            "El::Timer::validate: wrong backet events_count");
        }
      }

      if(backets_count != backets_count_)
      {
        throw ImplementationException(
          "El::Timer::validate: wrong backets_count");
      }

      if(last_backet_ != last_backet)
      {
        throw ImplementationException(
          "El::Timer::validate: wrong last_backet");
      }
      
    }
    
  }
  
}
