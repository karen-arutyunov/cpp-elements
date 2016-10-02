/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/LibXML/Use.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LIBXML_USE_HPP_
#define _ELEMENTS_EL_LIBXML_USE_HPP_

#include <iostream>
#include <sstream>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <El/Exception.hpp>

namespace El
{
  namespace LibXML
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);

    class ErrorHandler
    {
    public:
      virtual ~ErrorHandler() throw() {}
      virtual void handle(xmlErrorPtr error) throw() = 0;
    };
    
    struct ErrorRecorderHandler : public ErrorHandler
    {
      virtual ~ErrorRecorderHandler() throw() {}
      virtual void handle(xmlErrorPtr error) throw();

      void dump(ostream& ostr) throw(El::Exception);

      std::ostringstream fatal_errors;
      std::ostringstream errors;
      std::ostringstream warnings;
    };

    class Use
    {
      friend void XMLCALL error_handler(void *userData, xmlErrorPtr error);
      
    public :
      Use() throw();
      ~Use() throw();

      static void start() throw();
      static void stop() throw();

      static void set_error_handler(ErrorHandler* handler) throw();

    private:
      
      static void XMLCALL error_handler(void *userData, xmlErrorPtr error);

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
  namespace LibXML
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
        xmlInitParser();
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
          xmlCleanupParser();
        }
      }
    }

    inline
    void
    Use::set_error_handler(ErrorHandler* handler) throw()
    {
      xmlSetStructuredErrorFunc(handler, handler ? error_handler : 0);
    }
  }
}

#endif // _ELEMENTS_EL_LIBXML_USE_HPP_
 
