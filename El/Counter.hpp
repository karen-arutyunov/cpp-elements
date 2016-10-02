/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Counter.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_COUNTER_HPP_
#define _ELEMENTS_EL_COUNTER_HPP_

//#include <limits.h>

#include <string>
#include <iostream>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>

namespace El
{
  namespace Stat
  {
    class Counter
    {
    public:    
      EL_EXCEPTION(Exception, El::ExceptionBase);

      Counter(const char* dump_header = 0, bool active = true) throw();

      void increment() throw();
      void decrement() throw();
      
      void reset() throw();

      //
      // active functions are thread unsafe not to impact
      // increment function performance. Assumed to be set
      // prior counting.
      //
      void active(bool val) throw();
      bool active() throw();

      unsigned long long value() const throw();
      unsigned long long max_value() const throw();

      void dump(std::ostream& ostr) const throw();
      void dump_header(const char* header) throw(El::Exception);
      
    private:
      typedef ACE_Thread_Mutex  Mutex;
      typedef ACE_Guard<Mutex>  ReadGuard;
      typedef ACE_Guard<Mutex>  WriteGuard;

      mutable Mutex lock_;

      bool active_;      
      unsigned long long value_;
      unsigned long long max_value_;

      std::string dump_header_;
    };

    class CounterIncDec
    {
    public:
      CounterIncDec(Counter& counter) throw();
      ~CounterIncDec() throw();

    private:
      Counter& counter_;
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
    Counter::Counter(const char* dump_header, bool active) throw()
        : active_(active),
          value_(0),
          max_value_(0),
          dump_header_(dump_header ? dump_header : "")
    {
    }
  
    inline
    void
    Counter::active(bool val) throw()
    {
      active_ = val;
    }
    
    inline
    bool
    Counter::active() throw()
    {
      return active_;
    }
    
    inline
    void
    Counter::reset() throw()
    {
      WriteGuard guard(lock_);
      value_ = 0;
    }
    
    inline
    void
    Counter::increment() throw()
    {
      if(active_)
      {
        WriteGuard guard(lock_);
        
        ++value_;

        if(max_value_ < value_)
        {
          max_value_ = value_;
        }
        
      }
    }
    
    inline
    void
    Counter::decrement() throw()
    {
      if(active_)
      {
        WriteGuard guard(lock_);

        assert(value_);
        --value_;
      }
    }
    
    inline
    unsigned long long
    Counter::value() const throw()
    {
      ReadGuard guard(lock_);
      return value_;
    }

    inline
    unsigned long long
    Counter::max_value() const throw()
    {
      ReadGuard guard(lock_);
      return max_value_;
    }

    inline
    void
    Counter::dump(std::ostream& ostr) const throw()
    {
      if(!dump_header_.empty())
      {
        ostr << dump_header_;
      }

      unsigned long long val = 0;
      unsigned long long max_val = 0;
      
      {
        ReadGuard guard(lock_);
        val = value_;
        max_val = max_value_;
      }
      
      
      ostr << " " << val << " (max " << max_val << ")";
    }
    
    inline
    void
    Counter::dump_header(const char* header) throw(El::Exception)
    {
      WriteGuard guard(lock_);
      dump_header_ = header;
    }
    
    //
    // CounterIncDec
    //
    inline
    CounterIncDec::CounterIncDec(Counter& counter) throw()
        : counter_(counter)
    {
      counter_.increment();
    }
    
    inline
    CounterIncDec::~CounterIncDec() throw()
    {
      counter_.decrement();
    }

  }
}

#endif // _ELEMENTS_EL_COUNTER_HPP_
