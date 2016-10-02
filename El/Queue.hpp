/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/Queue.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_QUEUE_HPP_
#define _ELEMENTS_EL_QUEUE_HPP_

#include <limits.h>

#include <deque>
#include <sstream>

#include <ace/OS.h>

#include <El/Exception.hpp>

namespace El
{
  template<typename T>
  class Queue
  {      
  public:
    EL_EXCEPTION(Exception, El::ExceptionBase);

    enum EnqueueStrategy
    {
      ES_DEFAULT,
      ES_BACK,
      ES_FRONT,
      ES_RANDOM
    };

  public:
    Queue(size_t max_size = SIZE_MAX) throw(El::Exception);

    bool enqueue(const T& element,
                 const ACE_Time_Value* wait_time = 0,
                 EnqueueStrategy strategy = ES_BACK)
      throw(Exception, El::Exception);

    bool dequeue(T& element, const ACE_Time_Value* wait_time = 0)
      throw(Exception,El::Exception);

    void awake() throw(El::Exception);

    size_t max_size() const throw();
    void max_size(size_t val) throw();

    size_t size() const throw();

  protected:
    
    void do_enqueue(const T& element, EnqueueStrategy strategy)
      throw(Exception, El::Exception);

  protected:
    typedef ACE_Thread_Mutex Mutex;
    typedef ACE_Guard<Mutex> Guard;
    typedef ACE_Condition<Mutex> Condition;

    mutable Mutex lock_;
    Condition event_happened_;

    typedef std::deque<T> InnerQueue;
    InnerQueue queue_;

    size_t max_size_;
    size_t size_;
    size_t waiting_;
    size_t awaking_;

  private:
    Queue(const Queue&);
    void operator=(const Queue&);
  };
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  template<typename T>
  Queue<T>::Queue(size_t max_size) throw(El::Exception)
      : event_happened_(lock_),
        max_size_(max_size),
        size_(0),
        waiting_(0),
        awaking_(0)
  {
  }

  template<typename T>
  size_t
  Queue<T>::size() const throw()
  {
    Guard guard(lock_);
    return queue_.size();
  }
  
  template<typename T>
  size_t
  Queue<T>::max_size() const throw()
  {
    Guard guard(lock_);
    return max_size_;
  }
  
  template<typename T>
  void
  Queue<T>::max_size(size_t val) throw()
  {
    Guard guard(lock_);
    
    max_size_ = val;
    event_happened_.signal();
  }

  template<typename T>
  void
  Queue<T>::awake() throw(El::Exception)
  {
    {
      Guard guard(lock_);

      if(!waiting_)
      {
        return;
      }

      awaking_++;
    }
    
    while(true)
    {
      event_happened_.signal();

      Guard guard(lock_);

      if(!waiting_)
      {
        awaking_--;
        return;
      }
    }
  }
    
  template<typename T>
  void
  Queue<T>::do_enqueue(const T& element, EnqueueStrategy strategy)
    throw(Exception, El::Exception)
  {
    switch(strategy)
    {
    case ES_BACK:
    case ES_DEFAULT:
      {
        queue_.push_back(element);
        break;
      }
    case ES_FRONT:
      {
        queue_.push_front(element);
        break;
      }
    case ES_RANDOM:
      {
        size_t i = (unsigned long long)rand() *
          size_ / ((unsigned long long)RAND_MAX + 1);
        
        queue_.insert(queue_.begin() + i, element);
        break;
      }
    }
    
    size_++;    
  }
    
  template<typename T>
  bool
  Queue<T>::enqueue(const T& element,
                    const ACE_Time_Value* wait_time,
                    EnqueueStrategy strategy)
    throw(Exception, El::Exception)
  {
    Guard guard(lock_);

    if(size_ < max_size_)
    {
      do_enqueue(element, strategy);
      event_happened_.signal();
      return true;
    }

    if(max_size_ == 0 || (wait_time && *wait_time == ACE_Time_Value::zero))
    {
      return false;
    }
 
    ACE_Time_Value abstime;
    bool with_timeout = wait_time != 0;
      
    if(with_timeout)
    {
      abstime = ACE_OS::gettimeofday() + *wait_time;
    }
    
    waiting_++;
    
    do
    {
      if(awaking_)
      {
        waiting_--;
        return false;
      }
      
      if(event_happened_.wait(with_timeout ? &abstime : 0))
      {
        int error = ACE_OS::last_error();
          
        if(with_timeout && error == ETIME)
        {
          waiting_--;
          return false;
        }
        else
        {
          waiting_--;

          std::ostringstream ostr;
          ostr << "El::Queue<T>::enqueue: event_happened_.wait() failed. "
            "Errno " << error << ". Description:" << std::endl
               << ACE_OS::strerror(error);
            
          throw Exception(ostr.str());
        }
      }
    }
    while(max_size_ && size_ >= max_size_);
    
    waiting_--;

    if(max_size_ != 0)
    {
      do_enqueue(element, strategy);
    }
    
    event_happened_.signal();
    
    return max_size_ != 0;
  }

  template<typename T>
  bool
  Queue<T>::dequeue(T& element, const ACE_Time_Value* wait_time)
    throw(Exception, El::Exception)
  {
    Guard guard(lock_);

    if(size_)
    {
      element = queue_.front();
      queue_.pop_front();
      size_--;

      event_happened_.signal();
      
      return true;
    }

    if(max_size_ == 0 || (wait_time && *wait_time == ACE_Time_Value::zero))
    {
      return false;
    }

    ACE_Time_Value abstime;
    bool with_timeout = wait_time != 0;
      
    if(with_timeout)
    {
      abstime = ACE_OS::gettimeofday() + *wait_time;
    }
    
    waiting_++;
    
    do
    {
      if(awaking_)
      {
        waiting_--;
        return false;
      }
      
      if(event_happened_.wait(with_timeout ? &abstime : 0))
      {
        int error = ACE_OS::last_error();
          
        if(with_timeout && error == ETIME)
        {
          waiting_--;
          return false;
        }
        else
        {
          waiting_--;

          std::ostringstream ostr;
            
          ostr << "El::Queue<T>::dequeue: event_happened_.wait() failed. "
            "Errno " << error << ". Description:" << std::endl
               << ACE_OS::strerror(error);
            
          throw Exception(ostr.str());
        }
      }
    }
    while(max_size_ && size_ == 0);
    
    waiting_--;

    bool dequeued = false;
    
    if(size_)
    {
      element = queue_.front();
      queue_.pop_front();
      size_--;

      dequeued = true;
    }
    
    event_happened_.signal();
    
    return dequeued;
  }
}

#endif // _ELEMENTS_EL_QUEUE_HPP_
