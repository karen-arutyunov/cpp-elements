/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/CORBA/Process/ControlImpl.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_CORBA_PROCESS_CONTROLIMPL_HPP_
#define _ELEMENTS_EL_CORBA_PROCESS_CONTROLIMPL_HPP_

#include <string>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/Service/ThreadPool.hpp>

#include <El/CORBA/Control.hpp>
#include <El/CORBA/Control_s.hpp>

namespace El
{
  namespace Corba
  {
    class ProcessCtlImpl : public virtual POA_El::Corba::ProcessCtl,
                           public virtual El::Service::Callback
    {
    public:
      EL_EXCEPTION(Exception, El::ExceptionBase);

      ProcessCtlImpl(::CORBA::ORB_ptr orb = 0) throw(Exception, El::Exception);

      virtual ~ProcessCtlImpl() throw();
      
      void orb(::CORBA::ORB_ptr new_orb) throw();
      ::CORBA::ORB_ptr orb() const throw();
      
      void wait() throw();

      bool shutting_down() const throw();      

    protected:

      //
      // IDL:El/Corba/ProcessCtl/status:1.0
      //
      virtual ::CORBA::Boolean status() throw(::CORBA::SystemException);

      //
      // IDL:El/Corba/ProcessCtl/stop:1.0
      //
      virtual void stop() throw(::CORBA::SystemException);

      virtual bool notify(El::Service::Event* event) throw(El::Exception);

      virtual void terminate_process() throw(El::Exception);

    private:
      typedef ACE_Thread_Mutex SD_Mutex;
      typedef ACE_Guard<SD_Mutex> SD_Guard;

      mutable SD_Mutex shutdown_lock_;

      ::CORBA::ORB_var orb_;
      El::Service::ThreadPool_var thread_pool_;

      bool shutting_down_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Corba
  {
    //
    // ProcessCtlImpl class
    //
    
    inline
    ProcessCtlImpl::~ProcessCtlImpl() throw()
    {
    }
    
    inline
    ::CORBA::Boolean
    ProcessCtlImpl::status() throw(::CORBA::SystemException)
    {
      return 1;
    }

    inline
    void
    ProcessCtlImpl::terminate_process() throw(El::Exception)
    {
    }
    
    inline
    ::CORBA::ORB_ptr
    ProcessCtlImpl::orb() const throw()
    {
      return orb_.in();
    }

    inline
    void
    ProcessCtlImpl::orb(::CORBA::ORB_ptr new_orb) throw()
    {
      orb_ = ::CORBA::ORB::_duplicate(new_orb);
    }

    inline
    bool
    ProcessCtlImpl::shutting_down() const throw()
    {
      SD_Guard guard(shutdown_lock_);
      return shutting_down_;
    }
  }
}

#endif // _ELEMENTS_EL_CORBA_PROCESS_CONTROLIMPL_HPP_
