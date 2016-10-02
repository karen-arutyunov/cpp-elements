/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Service/Service.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_SERVICE_HPP_
#define _ELEMENTS_EL_SERVICE_HPP_

#include <string>
#include <ostream>
#include <sstream>

#include <El/Exception.hpp>
#include <El/RefCount/Interface.hpp>
#include <El/RefCount/SmartPtr.hpp>

#include <El/Logging/Logger.hpp>

namespace El
{
  namespace Service
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    
    class Service : public virtual El::RefCount::Interface
    {
    public:

      virtual const char* name() const throw(El::Exception) = 0;
      
      //
      // true if started, false if already started
      //
      virtual bool start() throw(Exception, El::Exception) = 0;
      
      //
      // true if stopped, false if already stopped
      //
      virtual bool stop() throw(Exception, El::Exception) = 0;

      virtual void wait() throw(Exception, El::Exception) = 0;

      virtual bool started() throw(Exception, El::Exception) = 0;

      virtual ~Service() throw() {}
    };

    typedef El::RefCount::SmartPtr<Service> Service_var;

    struct Event
    {
      std::string source_name;

      Event(const Service* src) throw();
      virtual ~Event() throw() {}
    };
  
    struct Error : public virtual Event
    {
    public:
      
      enum Severity
      {
        CRITICAL,
        ALERT,
        WARNING,
        NOTICE
      };

      Severity severity;
      char description[10 * 1024];
      
      Error(const char* descr, Service* src, Severity svr = CRITICAL) throw();
      
      Error(const std::string& descr, Service* src, Severity svr = CRITICAL)
        throw();
      
      virtual ~Error() throw() {}
    };  

    class Callback
    {
    public:

      // Implementor should return true if it assume that it have fully
      // processed an event and no futher processing required
      virtual bool notify(Event* event) throw(El::Exception) = 0;

      virtual ~Callback() throw() {}
    };
  
    class ServiceEvent : public virtual Event
    {
    public:
      ServiceEvent(Callback* callback, Service* source)
        throw(InvalidArg, Exception, El::Exception);
      
      virtual ~ServiceEvent() throw() {}
        
    protected:
      Callback* callback_;
    };
    
  }
  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

inline
std::ostream&
operator<<(std::ostream& ostr, const El::Service::Error& error)
  throw(El::Exception)
{
  switch(error.severity)
  {
  case El::Service::Error::CRITICAL: ostr << "critical error"; break;
  case El::Service::Error::ALERT: ostr << "alert"; break;
  case El::Service::Error::WARNING: ostr << "warning"; break;
  case El::Service::Error::NOTICE: ostr << "notice"; break;
  }
  
  ostr << " received from " << error.source_name << ". Description:\n"
       << error.description;
  
  return ostr;
}        

inline
std::ostream&
operator<<(std::ostream& ostr, const El::Service::Event& event)
  throw(El::Exception)
{
  ostr << "event received from " << event.source_name;
  return ostr;
}

namespace El
{
  namespace Service
  {    
    //
    // ServiceEvent class
    //
    inline
    Event::Event(const Service* src) throw()
        : source_name(src && src->name() ? src->name() : "Unknown")
    {
    }
    
    //
    // Error struct
    //
    inline
    Error::Error(const char* descr, Service* src, Severity svr) throw()
        : Event(src),
          severity(svr)
    {
      strncpy(description, descr ? descr : "", sizeof(description) - 1);
      description[sizeof(description) - 1] = '\0';
    }

    inline
    Error::Error(const std::string& descr, Service* src, Severity svr) throw()
        : Event(src),
          severity(svr)
    {
      strncpy(description, descr.c_str(), sizeof(description) - 1);
      description[sizeof(description) - 1] = '\0';
    }

    //
    // ServiceEvent class
    //
    inline
    ServiceEvent::ServiceEvent(Callback* callback, Service* src)
      throw(InvalidArg, Exception, El::Exception)
        : Event(src),
          callback_(callback)
    {
    }    

    //
    // Helpers
    //
    
    inline
    bool
    is_error(Event* event) throw()
    {
      return dynamic_cast<El::Service::Error*>(event) != 0;
    }
    
    inline
    std::string
    error_message(Event* event,
                  const char* prefix = 0,
                  unsigned long* level = 0) throw(El::Exception)
    {
      std::ostringstream ostr;
      El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);

      if(prefix)
      {
        ostr << prefix;
      }
      
      if(error == 0)
      {
        ostr << "unknown " << *event;

        if(level)
        {
          *level = El::Logging::EMERGENCY;
        }
      }
      else
      {
        if(level)
        {
          switch(error->severity)
          {
          case El::Service::Error::CRITICAL:
            *level = El::Logging::CRITICAL;
            break;
          case El::Service::Error::ALERT:
            *level = El::Logging::ALERT;
            break;
          case El::Service::Error::WARNING:
            *level = El::Logging::WARNING;
            break;
          case El::Service::Error::NOTICE:
            *level = El::Logging::NOTICE;
            break;
          }
        }
        
        ostr << *error;
      }

      return ostr.str();
    }
    
    inline
    void
    log(Event* event,
        const char* prefix,
        El::Logging::Logger* logger,
        const char* aspect = 0) throw()
    {
      try
      {
        unsigned long level = 0;
        std::string msg = error_message(event, prefix, &level);
        logger->log(msg, level, aspect);
      }
      catch(...)
      {
        logger->log("El::Service::log: unexpected exception caught",
                    El::Logging::EMERGENCY,
                    aspect);
      }
    }
    
  }
}

#endif // _ELEMENTS_EL_SERVICE_HPP_
