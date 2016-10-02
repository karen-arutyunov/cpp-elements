/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/XML/Use.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_XML_USE_HPP_
#define _ELEMENTS_EL_XML_USE_HPP_

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <xercesc/util/PlatformUtils.hpp>

namespace El
{
  namespace XML
  {
    class Use
    {
    public :
      Use() throw();
      ~Use() throw();

      static void start() throw();
      static void stop() throw();

    private:
      typedef ACE_Thread_Mutex Mutex;
      typedef ACE_Guard<Mutex> Guard;
      
      static unsigned long counter_;
      static Mutex lock_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace XML
  {
    inline
    Use::Use() throw()
    {
      start();
    }
    
    inline
    Use::~Use() throw()
    {
      stop();
    }

    inline
    void
    Use::start() throw()
    {
      Guard guard(lock_);

      if(counter_++ == 0)
      {
        xercesc::XMLPlatformUtils::Initialize();
      }
    }
    
    inline
    void
    Use::stop() throw()
    {
      Guard guard(lock_);

      if(counter_ > 0)
      {
        if(--counter_ == 0)
        {
          xercesc::XMLPlatformUtils::Terminate();
        }
      }
    }
  }
}

#endif // _ELEMENTS_EL_XML_USE_HPP_
 
