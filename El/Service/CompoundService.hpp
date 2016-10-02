/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/CompoundService.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_COMPOUNDSERVICE_HPP_
#define _ELEMENTS_EL_COMPOUNDSERVICE_HPP_

#include <sstream>
#include <string>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/SyncPolicy.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ThreadPool.hpp>
#include <El/Service/Timer.hpp>

namespace El
{
  namespace Service
  {
    class CompoundServiceMessage :
      public virtual El::Service::ThreadPool::ServiceEvent,
      public virtual El::Service::Timer::ServiceEvent
    {
    public:
      CompoundServiceMessage(Callback* callback,
                             Service* source,
                             bool execution_required = false)
        throw(InvalidArg, Exception, El::Exception);

      virtual ~CompoundServiceMessage() throw();
      
      virtual ThreadPool::TaskQueue::EnqueueStrategy enqueue_strategy()
        throw();
    };
      
    typedef El::RefCount::SmartPtr<CompoundServiceMessage>
    CompoundServiceMessage_var;

    template<typename STATE = El::Service::Service,
             typename CALLBACK = El::Service::Callback>
    class CompoundService :
      public virtual Service,
      public virtual Callback,
      public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
    {
    public:
      EL_EXCEPTION(Exception, El::Service::Exception);

    public:      
      CompoundService(
        CALLBACK* callback,
        const char* name = 0,
        unsigned long thread_pool_threads = 1,
        size_t thread_pool_stack_size = 0,
        size_t thread_pool_queue_size = SIZE_MAX,
        ThreadPool::TaskQueue::EnqueueStrategy enqueue_strategy =
          ThreadPool::TaskQueue::ES_BACK,
        size_t timer_stack_size = 0,
        bool timer_minimize_delay = false)
        throw(InvalidArg, El::Exception);

      virtual ~CompoundService() throw();

      virtual const char* name() const throw(El::Exception);

      virtual bool start() throw(Exception, El::Exception);
      virtual bool stop() throw(Exception, El::Exception);
      virtual void wait() throw(Exception, El::Exception);
      virtual bool started() throw(Exception, El::Exception);

      void deliver_at_time(CompoundServiceMessage* msg,
                           const ACE_Time_Value& time)
        throw(InvalidArg, El::Exception);

      bool deliver_now(
        CompoundServiceMessage* msg,
        const ACE_Time_Value* wait_time = 0,
        ThreadPool::TaskQueue::EnqueueStrategy enqueue_strategy =
        ThreadPool::TaskQueue::ES_DEFAULT)
        throw(InvalidArg, El::Exception);

      void state(STATE* new_state) throw(El::Exception);
      STATE* state() const throw(El::Exception);

      size_t task_queue_size() const throw();
      
      virtual bool notify(Event* event) throw(El::Exception);
      
    protected:

      typedef El::RefCount::SmartPtr<STATE> STATE_var;
      
      struct SetState : public virtual El::Service::ThreadPool::ServiceEvent
      {
        SetState(STATE* new_state, CompoundService* service) throw();
        ~SetState() throw();

        STATE_var state;
      };
      
    protected:
      typedef ACE_RW_Thread_Mutex    Mutex;
      typedef ACE_Read_Guard<Mutex>  ReadGuard;
      typedef ACE_Write_Guard<Mutex> WriteGuard;
      
      mutable Mutex srv_lock_;
      
      CALLBACK* callback_;
      std::string name_;

      bool started_;
      bool stop_;

      ThreadPool_var thread_pool_;
      Timer_var timer_;

      STATE_var state_;
    };

    typedef CompoundService<> GenericService;
    typedef El::RefCount::SmartPtr<GenericService> GenericService_var;
  }
  
}

#define El__Service__CompoundServiceMessageBase(callback, source, execution_required) \
          El::Service::ThreadPool::TaskBase(execution_required), \
          El::Service::Event(source), \
          El::Service::ServiceEvent(callback, source), \
          El::Service::ThreadPool::ServiceEvent(callback, source), \
          El::Service::Timer::ServiceEvent(callback, source)

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Service
  {
    //
    // CompoundService::Message class
    //
    inline
    CompoundServiceMessage::CompoundServiceMessage(
      Callback* callback,
      Service* source,
      bool execution_required)
      throw(InvalidArg, Exception, El::Exception)
        : El__Service__CompoundServiceMessageBase(callback,
                                                  source,
                                                  execution_required)
    {
    }

    inline
    CompoundServiceMessage::~CompoundServiceMessage() throw()
    {
    }

    inline
    ThreadPool::TaskQueue::EnqueueStrategy
    CompoundServiceMessage::enqueue_strategy() throw()
    {
      return ThreadPool::TaskQueue::ES_DEFAULT;
    }

    //
    // CompoundService::SetState class
    //
    template<typename STATE, typename CALLBACK>
    CompoundService<STATE, CALLBACK>::SetState::SetState(
      STATE* new_state,
      CompoundService<STATE, CALLBACK>* service)
      throw()
        : El::Service::ThreadPool::TaskBase(false),
          El::Service::Event(service),
          El::Service::ServiceEvent(service, service),
          El::Service::ThreadPool::ServiceEvent(service, service),
          state(El::RefCount::add_ref(new_state))
    {
    }

    template<typename STATE, typename CALLBACK>
    CompoundService<STATE, CALLBACK>::SetState::~SetState() throw()
    {
    }

    //
    // CompoundService class
    //
    template<typename STATE, typename CALLBACK>
    CompoundService<STATE, CALLBACK>::CompoundService(
      CALLBACK* callback,
      const char* name,
      unsigned long thread_pool_threads,
      size_t thread_pool_stack_size,
      size_t thread_pool_queue_size,
      ThreadPool::TaskQueue::EnqueueStrategy enqueue_strategy,
      size_t timer_stack_size,
      bool timer_minimize_delay)
      throw(InvalidArg, El::Exception)
        : callback_(callback),
          name_(name ? name : ""),
          started_(false),
          stop_(true)
    {
      thread_pool_ =
        new ThreadPool(this,
                       std::string(name_ + "_thread_pool").c_str(),
                       thread_pool_threads,
                       thread_pool_stack_size,
                       thread_pool_queue_size,
                       enqueue_strategy);

      timer_ = new Timer(this,
                         std::string(name_ + "_timer").c_str(),
                         timer_stack_size,
                         timer_minimize_delay);
    }
    
    template<typename STATE, typename CALLBACK>
    CompoundService<STATE, CALLBACK>::~CompoundService() throw()
    {
      try
      {
        if(state_.in())
        {
          state_->stop();
          state_->wait();
        }
      }
      catch(...)
      {
        // Can do nothing
      }
      
    }

    template<typename STATE, typename CALLBACK>
    size_t
    CompoundService<STATE, CALLBACK>::task_queue_size() const throw()
    {
      return thread_pool_->queue_size();
    }
    
    template<typename STATE, typename CALLBACK>
    void
    CompoundService<STATE, CALLBACK>::deliver_at_time(
      CompoundServiceMessage* msg,
      const ACE_Time_Value& time)
      throw(InvalidArg, El::Exception)
    {
      if(time == ACE_Time_Value::zero)
      {
        thread_pool_->execute(msg);
      }
      else
      {
        timer_->set(msg, time);
      }
    }
    
    template<typename STATE, typename CALLBACK>
    bool
    CompoundService<STATE, CALLBACK>::deliver_now(
      CompoundServiceMessage* msg,
      const ACE_Time_Value* wait_time,
      ThreadPool::TaskQueue::EnqueueStrategy enqueue_strategy)
      throw(InvalidArg, El::Exception)
    {
      msg->time = ACE_Time_Value::zero;
      return thread_pool_->execute(msg, wait_time, enqueue_strategy);
    }
    
    template<typename STATE, typename CALLBACK>
    const char*
    CompoundService<STATE, CALLBACK>::name() const throw(El::Exception)
    {
      return name_.c_str();
    }
    
    template<typename STATE, typename CALLBACK>
    bool
    CompoundService<STATE, CALLBACK>::started() throw(Exception, El::Exception)
    {
      ReadGuard guard(srv_lock_);
      return started_ && !stop_;
    }

    template<typename STATE, typename CALLBACK>
    bool
    CompoundService<STATE, CALLBACK>::start() throw(Exception, El::Exception)
    {
      STATE_var state;
      {
        WriteGuard guard(srv_lock_);

        if(started_)
        {
          return false;
        }

        thread_pool_->start();
        timer_->start();

        started_ = true;  
        stop_ = false;

        state = state_;        
      }

      if(state.in() != 0)
      {
        state->start();
      }

      return true;
    }
    
    template<typename STATE, typename CALLBACK>
    bool
    CompoundService<STATE, CALLBACK>::stop() throw(Exception, El::Exception)
    {
      STATE_var state;
      
      {
        WriteGuard guard(srv_lock_);

        if(!started_ || stop_)
        {
          return false;
        }
        
        timer_->stop();
        thread_pool_->stop();

        state = state_;
        stop_ = true;
      }

      if(state.in() != 0)
      {
        state->stop();
      }
      
      return true;
    }
    
    template<typename STATE, typename CALLBACK>
    void
    CompoundService<STATE, CALLBACK>::wait() throw(Exception, El::Exception)
    {
      timer_->wait();
      thread_pool_->wait();

      STATE_var st = state();
      
      if(st.in() != 0)
      {
        st->wait();
      }

      timer_->clear_buckets();
      
      WriteGuard guard(srv_lock_);      
      started_ = false;
    }

    template<typename STATE, typename CALLBACK>
    void
    CompoundService<STATE, CALLBACK>::state(STATE* new_state)
      throw(El::Exception)
    {
      STATE_var state;
      
      {  
        WriteGuard guard(srv_lock_);

        if(state_.in() == 0)
        {
          state_ = El::RefCount::add_ref<STATE>(new_state);

          if(state_.in() != 0 && started_ && !stop_)
          {
            state_->start();
          }
        
          return;
        }

        state = state_;
      }
      
      state->stop();

      El::Service::ThreadPool::Task_var msg = new SetState(new_state, this);
      thread_pool_->execute(msg.in());
    }
    
    template<typename STATE, typename CALLBACK>
    STATE*
    CompoundService<STATE, CALLBACK>::state() const throw(El::Exception)
    {
      ReadGuard guard(srv_lock_);
      return El::RefCount::add_ref(state_.in());
    }

    template<typename STATE, typename CALLBACK>
    bool
    CompoundService<STATE, CALLBACK>::notify(Event* event)
      throw(El::Exception)
    {
      CompoundServiceMessage* msg =
        dynamic_cast<CompoundServiceMessage*>(event);

      if(msg)
      {
        if(msg->time != ACE_Time_Value::zero)
        {
          msg->time = ACE_Time_Value::zero;
          thread_pool_->execute(msg, 0, msg->enqueue_strategy());
          return true;
        }

        //
        // Message came from thread pool; passing to derived class for handling
        //
        return false;
      }
        
      SetState* set_state = dynamic_cast<SetState*>(event);

      if(set_state != 0)
      {
        STATE_var state;
        
        {
          ReadGuard guard(srv_lock_);
          state = state_;
        }

        if(state.in() != 0)
        {
          state->stop();
          state->wait();
        }
        
        WriteGuard guard(srv_lock_);

        if(state.in() == state_.in())
        {
          state_ = El::RefCount::add_ref(set_state->state);
        
          if(state_.in() != 0 && started_ && !stop_)
          {
            state_->start();
          }
        }
        
        return true;
      }

      if(callback_)
      {
        El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);
      
        if(error)
        {
          std::ostringstream ostr;
          ostr << "CompoundService::notify: forwarding " << *error;
            
          El::Service::Error fwerr(ostr.str(), this, error->severity);        
          callback_->notify(&fwerr);
          
          return true;
        }
      }
      
      return false;
    }
  }
}

#endif // _ELEMENTS_EL_COMPOUNDSERVICE_HPP_
