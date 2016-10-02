/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Logging/Logger.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOGGING_LOGGER_HPP_
#define _ELEMENTS_EL_LOGGING_LOGGER_HPP_

#include <string>
#include <memory>

#include <El/Exception.hpp>

namespace El
{
  namespace Logging
  {
    enum Severity
    {
      EMERGENCY,
      ALERT,
      ERROR,
      CRITICAL,
      WARNING,
      NOTICE,
      INFO,
      DEBUG,
      TRACE
    };

    enum TraceLevel
    {
      LOW,
      MIDDLE,
      HIGH
    };

    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArgument, Exception);

    class Logger
    {
    public:

      Logger() throw();

      virtual unsigned long level() const throw() = 0;
      virtual void level(unsigned long val) throw() = 0;

      bool will_log(unsigned long level) throw();
      bool will_trace(unsigned long level) throw();

      virtual const char* aspects() const throw(El::Exception) = 0;
      virtual void aspects(const char* aspect) throw(El::Exception) = 0;

      virtual void log(const char* text,
                       unsigned long severity,
                       const char* aspect)
        throw() = 0;

      void log(const std::string& text,
               unsigned long severity,
               const char* aspect)
        throw();

      virtual void raw_log(const char* text) throw() = 0;
      void raw_log(const std::string& text) throw();

      void emergency(const char* text, const char* aspect) throw();
      void alert(const char* text, const char* aspect) throw();
      void error(const char* text, const char* aspect) throw();
      void critical(const char* text, const char* aspect) throw();
      void warning(const char* text, const char* aspect) throw();
      void notice(const char* text, const char* aspect) throw();
      void info(const char* text, const char* aspect) throw();
      void debug(const char* text, const char* aspect) throw();
      
      void trace(const char* text, const char* aspect, unsigned long level = 0)
        throw();
      
      void emergency(const std::string& text, const char* aspect) throw();
      void alert(const std::string& text, const char* aspect) throw();
      void error(const std::string& text, const char* aspect) throw();
      void critical(const std::string& text, const char* aspect) throw();
      void warning(const std::string& text, const char* aspect) throw();
      void notice(const std::string& text, const char* aspect) throw();
      void info(const std::string& text, const char* aspect) throw();
      void debug(const std::string& text, const char* aspect) throw();
      
      void trace(const std::string& text,
                 const char* aspect,
                 unsigned long level = 0)
        throw();

      virtual ~Logger() throw();

    private:
      Logger(const Logger&);
      void operator=(const Logger&);
    };

    typedef std::auto_ptr<Logger> LoggerPtr;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Logging
  {
    inline
    Logger::Logger() throw()
    {
    }
    
    inline
    Logger::~Logger() throw()
    {
    }
    
    inline
    void
    Logger::log(const std::string& text,
                unsigned long severity,
                const char* aspect)
      throw()
    {
      log(text.c_str(), severity, aspect);
    }
    
    inline
    void
    Logger::raw_log(const std::string& text) throw()
    {
      raw_log(text.c_str());
    }
    
    inline
    bool
    Logger::will_log(unsigned long lv) throw()
    {
      return lv <= level();
    }
    
    inline
    bool
    Logger::will_trace(unsigned long lv) throw()
    {
      return will_log(TRACE + lv);
    }
    
    inline
    void
    Logger::emergency(const char* text, const char* aspect) throw()
    {
      log(text, EMERGENCY, aspect);
    }

    inline
    void
    Logger::alert(const char* text, const char* aspect) throw()
    {
      log(text, ALERT, aspect);
    }

    inline
    void
    Logger::error(const char* text, const char* aspect) throw()
    {
      log(text, ERROR, aspect);
    }
    
    inline
    void
    Logger::critical(const char* text, const char* aspect) throw()
    {
      log(text, CRITICAL, aspect);
    }

    inline
    void
    Logger::warning(const char* text, const char* aspect) throw()
    {
      log(text, WARNING, aspect);
    }

    inline
    void
    Logger::notice(const char* text, const char* aspect) throw()
    {
      log(text, NOTICE, aspect);
    }

    inline
    void
    Logger::info(const char* text, const char* aspect) throw()
    {
      log(text, INFO, aspect);
    }

    inline
    void
    Logger::debug(const char* text, const char* aspect) throw()
    {
      log(text, DEBUG, aspect);
    }

    inline
    void
    Logger::trace(const char* text,
                  const char* aspect,
                  unsigned long trace_level) throw()
    {
      log(text, TRACE + trace_level, aspect);
    }

    inline
    void
    Logger::emergency(const std::string& text, const char* aspect) throw()
    {
      log(text, EMERGENCY, aspect);
    }

    inline
    void
    Logger::alert(const std::string& text, const char* aspect) throw()
    {
      log(text, ALERT, aspect);
    }

    inline
    void
    Logger::error(const std::string& text, const char* aspect) throw()
    {
      log(text, ERROR, aspect);
    }
    
    inline
    void
    Logger::critical(const std::string& text, const char* aspect) throw()
    {
      log(text, CRITICAL, aspect);
    }

    inline
    void
    Logger::warning(const std::string& text, const char* aspect) throw()
    {
      log(text, WARNING, aspect);
    }

    inline
    void
    Logger::notice(const std::string& text, const char* aspect) throw()
    {
      log(text, NOTICE, aspect);
    }

    inline
    void
    Logger::info(const std::string& text, const char* aspect) throw()
    {
      log(text, INFO, aspect);
    }

    inline
    void
    Logger::debug(const std::string& text, const char* aspect) throw()
    {
      log(text, DEBUG, aspect);
    }

    inline
    void
    Logger::trace(const std::string& text,
                  const char* aspect,
                  unsigned long trace_level) throw()
    {
      log(text, TRACE + trace_level, aspect);
    }

  }
}

#endif // _ELEMENTS_EL_LOGGING_LOGGER_HPP_
