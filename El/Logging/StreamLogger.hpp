/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Logging/StreamLogger.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOGGING_STREAMLOGGER_HPP_
#define _ELEMENTS_EL_LOGGING_STREAMLOGGER_HPP_

#include <iostream>
#include <memory>

#include <El/Exception.hpp>
#include <El/Moment.hpp>

#include <El/Logging/LoggerBase.hpp>
 
namespace El
{
  namespace Logging
  {
    class StreamLogger : public virtual LoggerBase
    {
    public:

      StreamLogger(std::ostream& output,
                   unsigned long level = DEBUG,
                   const char* aspects="*",
                   El::Moment::TimeZone zone = El::Moment::TZ_GMT,
                   std::ostream* error_stream = &std::cerr,
                   Formatter* formatter = 0,
                   unsigned long aspect_threshold = TRACE)
        throw(Exception, El::Exception);


      virtual ~StreamLogger() throw();

      virtual void write(const char* line, bool newline, bool lock)
        throw(Exception, El::Exception);
      
    protected:
      std::ostream& output_;
    };
    
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
    StreamLogger::StreamLogger(std::ostream& output,
                               unsigned long level,
                               const char* aspects,
                               El::Moment::TimeZone zone,
                               std::ostream* error_stream,
                               Formatter* formatter,
                               unsigned long aspect_threshold)
      throw(Exception, El::Exception)
        : LoggerBase(level,
                     aspects,
                     zone,
                     error_stream,
                     formatter,
                     aspect_threshold),
          output_(output)
    {
    }
    
    inline
    StreamLogger::~StreamLogger() throw()
    {
    }
    
    inline
    void
    StreamLogger::write(const char* line, bool newline, bool lock)
      throw(Exception, El::Exception)
    {
      std::auto_ptr<WriteGuard> guard(lock ? new WriteGuard(lock_) : 0);
      
      output_ << line;

      if(newline)
      {
        output_ << std::endl;
      }

      output_ << std::flush;

      if(output_.fail() || output_.bad())
      {
        throw Exception("El::Logging::StreamLogger::write: operation failed");
      }
      
    }
  }
}

#endif // _ELEMENTS_EL_LOGGING_STREAMLOGGER_HPP_
