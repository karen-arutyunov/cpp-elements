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
 * @file Elements/El/Stat.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STAT_HPP_
#define _ELEMENTS_EL_STAT_HPP_

#include <limits.h>

#include <string>
#include <iostream>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>
#include <ace/TSS_T.h>

//#define USE_HIRES

#ifdef USE_HIRES
# include <ace/High_Res_Timer.h>
#endif

#include <El/Exception.hpp>
#include <El/Moment.hpp>

namespace El
{
  namespace Stat
  {
    class TimeMeter
    {
    public:    
      EL_EXCEPTION(Exception, El::ExceptionBase);

      TimeMeter(const char* dump_header = 0, bool active = true) throw();

      void start() throw();
      void stop() throw();
      void reset() throw();

      //
      // active functions are thread unsafe not to impact
      // start and stop functions performance. Assumed to be set
      // prior meterings.
      //
      void active(bool val) throw();
      bool active() throw();

      ACE_Time_Value average_time() const throw();
      ACE_Time_Value max_time() const throw();
      ACE_Time_Value min_time() const throw();
      ACE_Time_Value total_time() const throw();
      unsigned long long meterings() const throw();

      void dump(std::ostream& ostr) const throw();

      void dump_header(const char* header) throw(El::Exception);

    private:
      ACE_Time_Value average_time_() const throw();
      ACE_Time_Value max_time_() const throw();
      ACE_Time_Value min_time_() const throw();
      ACE_Time_Value total_time_() const throw();      
      
    private:
      typedef ACE_Thread_Mutex  Mutex;
      typedef ACE_Guard<Mutex>  ReadGuard;
      typedef ACE_Guard<Mutex>  WriteGuard;

      mutable Mutex lock_;

      bool active_;
      
#ifdef USE_HIRES
      ACE_TSS<ACE_High_Res_Timer> timer_;
#else
      ACE_TSS<ACE_Time_Value> start_time_;      
#endif
      
      unsigned long long meterings_;
      unsigned long long time_; //usec
      unsigned long long max_tm_;
      unsigned long long min_tm_;

      std::string dump_header_;
    };

    class TimeMeasurement
    {
    public:
      TimeMeasurement(TimeMeter& meter) throw();
      ~TimeMeasurement() throw();

      void stop() throw();

    private:
      TimeMeter& meter_;
      bool started_;
    };
  }
  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Stat
  {
    //
    // TimeMeter
    //
    inline
    TimeMeter::TimeMeter(const char* dump_header, bool active) throw()
        : active_(active),
          meterings_(0),
          time_(0),
          max_tm_(0),
          min_tm_(ULONG_LONG_MAX),
          dump_header_(dump_header ? dump_header : "")
    {
    }
  
    inline
    void
    TimeMeter::active(bool val) throw()
    {
      active_ = val;
    }
    
    inline
    bool
    TimeMeter::active() throw()
    {
      return active_;
    }
    
    inline
    void
    TimeMeter::reset() throw()
    {
      WriteGuard guard(lock_);
      meterings_ = 0;
      time_ = 0;
      max_tm_ = 0;
      min_tm_ = ULONG_LONG_MAX;
    }
    
    inline
    void
    TimeMeter::start() throw()
    {
      if(active_)
      {
#ifdef USE_HIRES
        (*timer_).start();
#else
        (*start_time_) = ACE_OS::gettimeofday();
#endif        
      }
    }
    
    inline
    void
    TimeMeter::stop() throw()
    {
      if(active_)
      {

#ifdef USE_HIRES
        (*timer_).stop();
      
        ACE_Time_Value tm;
        (*timer_).elapsed_time(tm);
#else
        ACE_Time_Value tm = ACE_OS::gettimeofday() - (*start_time_);
#endif

        unsigned long long time = (unsigned long long)tm.sec() * 1000000 +
          tm.usec();
        
        WriteGuard guard(lock_);

        if(max_tm_ < time)
        {
          max_tm_ = time;
        }

        if(min_tm_ > time)
        {
          min_tm_ = time;
        }
        
        time_ += time;          
        meterings_++;
      }
    }

    inline
    ACE_Time_Value
    TimeMeter::average_time() const throw()
    {
      ReadGuard guard(lock_);
      return average_time_();
    }
      
    inline
    ACE_Time_Value
    TimeMeter::average_time_() const throw()
    {
      if(!meterings_) return ACE_Time_Value::zero;
      
      unsigned long long avr = time_ / meterings_;
      return ACE_Time_Value(avr / 1000000, avr % 1000000);
    }
    
    inline
    ACE_Time_Value
    TimeMeter::max_time() const throw()
    {
      ReadGuard guard(lock_);
      return max_time_();
    }
      
    inline
    ACE_Time_Value
    TimeMeter::max_time_() const throw()
    {
      unsigned long long sec = max_tm_ / 1000000;
      unsigned long long usec = max_tm_ % 1000000;
      return ACE_Time_Value(sec, usec);
    }
    
    inline
    ACE_Time_Value
    TimeMeter::min_time() const throw()
    {
      ReadGuard guard(lock_);
      return min_time_();
    }
      
    inline
    ACE_Time_Value
    TimeMeter::min_time_() const throw()
    {
      return ACE_Time_Value(min_tm_ / 1000000, min_tm_ % 1000000);
    }
    
    inline
    ACE_Time_Value
    TimeMeter::total_time() const throw()
    {
      ReadGuard guard(lock_);
      return total_time_();
    }
      
    inline
    ACE_Time_Value
    TimeMeter::total_time_() const throw()
    {
      return ACE_Time_Value(time_ / 1000000, time_ % 1000000);
    }
    
    inline
    unsigned long long
    TimeMeter::meterings() const throw()
    {
      ReadGuard guard(lock_);      
      return meterings_;
    }

    inline
    void
    TimeMeter::dump(std::ostream& ostr) const throw()
    {
      ReadGuard guard(lock_);
      
      if(!dump_header_.empty())
      {
        ostr << dump_header_ << std::endl;
      }
      
      ostr << "  Average time: " << El::Moment::time(average_time_())
           << std::endl
           << "  Max time    : " << El::Moment::time(max_time_()) << std::endl
           << "  Min time    : " << El::Moment::time(
             min_tm_ == ULONG_LONG_MAX ? ACE_Time_Value::zero : min_time_())
           << std::endl
           << "  Total time  : " << El::Moment::time(total_time_())
           << std::endl
           << "  Meterings   : " << meterings_ << std::endl;
    }
    
    inline
    void
    TimeMeter::dump_header(const char* header) throw(El::Exception)
    {
      WriteGuard guard(lock_);
      dump_header_ = header;
    }
    
    //
    // TimeMeasurement
    //
    inline
    TimeMeasurement::TimeMeasurement(TimeMeter& meter) throw()
        : meter_(meter),
          started_(true)
    {
      meter.start();
    }
    
    inline
    TimeMeasurement::~TimeMeasurement() throw()
    {
      stop();
    }

    inline
    void
    TimeMeasurement::stop() throw()
    {
      if(started_)
      {
        meter_.stop();
        started_ = false;
      }
    }
  }
}

#endif // _ELEMENTS_EL_STAT_HPP_
