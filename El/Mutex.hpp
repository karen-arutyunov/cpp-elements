/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   El/Mutex.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_MUTEX_HPP_
#define _ELEMENTS_EL_MUTEX_HPP_

#include <memory>

#include <ace/Synch.h>

namespace El
{
  namespace Sync
  {
    class AdaptingMutex
    {
    public:
      
      AdaptingMutex(size_t check_period = 1000,
                    size_t tries = 20,
                    const ACE_Time_Value& wait_time = ACE_Time_Value(0, 20000))
        throw();
 
      int acquire_read() const throw();
      int acquire_write() const throw();
      void release() const throw();

      int tryacquire_write() const throw();
      
    private:
      typedef ACE_Guard<ACE_Thread_Mutex> Guard;
      
      mutable ACE_Thread_Mutex service_mutex_;
      mutable ACE_RW_Thread_Mutex main_mutex_;

      mutable volatile _Atomic_word pending_writers_;
      mutable size_t counter_;
      
      size_t check_period_;
      size_t tries_;
      ACE_Time_Value wait_time_;
    };

    class AdaptingReadGuard
    {
    public:
      AdaptingReadGuard(const AdaptingMutex& mutex) throw();
      ~AdaptingReadGuard() throw();

      void release() throw();
      
    private:
      const AdaptingMutex& mutex_;
      bool release_;
    };

    class AdaptingWriteGuard
    {
    public:
      AdaptingWriteGuard(const AdaptingMutex& mutex) throw();
      ~AdaptingWriteGuard() throw();

      void release() throw();

    private:
      const AdaptingMutex& mutex_;
      bool release_;
    };
  }
}

//////////////////////////////////////////////////////////////////

namespace El
{
  namespace Sync
  {
    //
    // AdaptingMutex
    //

    inline
    AdaptingMutex::AdaptingMutex(size_t check_period,
                                 size_t tries,
                                 const ACE_Time_Value& wait_time) throw()
        : pending_writers_(0),
          counter_(0),
          check_period_(check_period),
          tries_(tries),
          wait_time_(wait_time)
    {
    }
    
    inline
    int
    AdaptingMutex::acquire_read() const throw()
    {
      if(check_period_)
      {
        Guard guard(service_mutex_);

        if(counter_)
        {
          --counter_;
        }
        else
        {
          int pending_writers = pending_writers_;
          
          if(pending_writers)
          {
            size_t r = tries_;
            
            for(size_t i = 0; i < tries_; ++i)
            {
              ACE_OS::sleep(wait_time_);

              if(pending_writers_ < pending_writers)
              {
                r = i;
                break;
              }
            }
            
//            std::cerr << "W" << r << std::endl;
          }
          
          counter_ = check_period_;
        }
      }
      
      return main_mutex_.acquire_read();
    }
    
    inline
    int
    AdaptingMutex::acquire_write() const throw()
    {
      if(check_period_)
      {
        {
          Guard guard(service_mutex_);
          __gnu_cxx::__atomic_add(&pending_writers_, 1);
        }
      
        int res = main_mutex_.acquire_write();

        __gnu_cxx::__atomic_add(&pending_writers_, -1);
      
        return res;
      }
      else
      {
        return main_mutex_.acquire_write();
      }
    }
    
    inline
    void
    AdaptingMutex::release() const throw()
    {
      main_mutex_.release();
    }

    inline
    int
    AdaptingMutex::tryacquire_write() const throw()
    {
      return main_mutex_.tryacquire_write();
    }

    //
    // AdaptingReadGuard
    //
    inline
    AdaptingReadGuard::AdaptingReadGuard(const AdaptingMutex& mutex) throw()
        : mutex_(mutex),
          release_(true)
    {
      mutex_.acquire_read();
    }
    
    inline
    AdaptingReadGuard::~AdaptingReadGuard() throw()
    {
      if(release_)
      {
        mutex_.release();
      }
    }

    inline
    void
    AdaptingReadGuard::release() throw()
    {
      assert(release_);
      mutex_.release();
      release_ = false;
    }
    
    //
    // AdaptingWriteGuard
    //
    inline
    AdaptingWriteGuard::AdaptingWriteGuard(const AdaptingMutex& mutex) throw()
        : mutex_(mutex),
          release_(true)
    {
      mutex_.acquire_write();
    }
    
    inline
    AdaptingWriteGuard::~AdaptingWriteGuard() throw()
    {
      if(release_)
      {
        mutex_.release();
      }
    }

    inline
    void
    AdaptingWriteGuard::release() throw()
    {
      assert(release_);
      mutex_.release();
      release_ = false;
    }    
  }
}

#endif  // _ELEMENTS_EL_MUTEX_HPP_
