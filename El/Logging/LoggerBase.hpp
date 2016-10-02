/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Logging/LoggerBase.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOGGING_LOGGERBASE_HPP_
#define _ELEMENTS_EL_LOGGING_LOGGERBASE_HPP_

#include <string>
#include <iostream>

#include <ext/hash_set>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Moment.hpp>
#include <El/RefCount/All.hpp>

#include <El/Logging/Logger.hpp>

namespace El
{
  namespace Logging
  {
    class Formatter : public virtual El::RefCount::Interface
    {
    public:
      virtual std::string line(const char* text,
                               unsigned long severity,
                               const char* aspect,
                               const El::Moment& time)
        throw(El::Exception) = 0;

      virtual ~Formatter() throw();
    };

    typedef El::RefCount::SmartPtr<Formatter> Formatter_var;
    
    class SimpleFormatter : public virtual Formatter,
                            public virtual El::RefCount::DefaultImpl<>
    {
    public:

      enum FieldPresence
      {
        FP_TIME = 0x1,
        FP_SEVERITY = 0x2,
        FP_ASPECT = 0x4,
        FP_MESSAGE = 0x8,
        FP_LINE_IDENT = 0x10,
        FP_ALL = FP_TIME | FP_SEVERITY | FP_ASPECT | FP_MESSAGE | FP_LINE_IDENT
      };

      SimpleFormatter(unsigned long field_presence_mask = FP_ALL) throw();
        
      virtual std::string line(const char* text,
                               unsigned long severity,
                               const char* aspect,
                               const El::Moment& time)
        throw(El::Exception);

      virtual ~SimpleFormatter() throw();

    protected:
      unsigned long field_presence_mask_;
    };

    class LoggerBase : public virtual Logger
    {
    public:
      LoggerBase(unsigned long level,
                 const char* aspects,
                 El::Moment::TimeZone zone,
                 std::ostream* error_stream,
                 Formatter* formatter,
                 unsigned long aspect_threshold)
        throw(Exception, El::Exception);

      virtual unsigned long level() const throw();
      virtual void level(unsigned long val) throw();

      virtual const char* aspects() const throw(El::Exception);
      virtual void aspects(const char* aspects) throw(El::Exception);

      virtual void log(const char* text,
                       unsigned long severity,
                       const char* aspect)
        throw();

      virtual void raw_log(const char* text) throw();
      
      virtual ~LoggerBase() throw();
      
      //
      // Overrides
      //
      virtual void write(const char* line, bool newline, bool lock)
        throw(Exception, El::Exception) = 0;

    protected:
      typedef ACE_RW_Thread_Mutex    Mutex;
      typedef ACE_Read_Guard<Mutex>  ReadGuard;
      typedef ACE_Write_Guard<Mutex> WriteGuard;

      mutable Mutex lock_;

      typedef __gnu_cxx::hash_set<std::string, El::Hash::String> AspectTable;

      unsigned long level_;
      std::string aspects_;
      El::Moment::TimeZone zone_;
      std::ostream* error_stream_;
      Formatter_var formatter_;
      unsigned long aspect_threshold_;
      
      AspectTable aspect_table_;
      El::Moment time_;
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
    //
    // Formatter class
    //

    inline
    Formatter::~Formatter() throw()
    {
    }

    //
    // LoggerBase class
    //
    inline
    LoggerBase::~LoggerBase() throw()
    {
    }
    
    inline
    unsigned long
    LoggerBase::level() const throw()
    {
      ReadGuard guard(lock_);
      return level_;
    }

    inline
    void
    LoggerBase::level(unsigned long val) throw()
    {
      WriteGuard guard(lock_);
      level_ = val;
    }

    inline
    const char*
    LoggerBase::aspects() const throw(El::Exception)
    {
      ReadGuard guard(lock_);
      return aspects_.c_str();
    }

    //
    // SimpleFormatter class
    //
    
    inline
    SimpleFormatter::SimpleFormatter(
      unsigned long field_presence_mask) throw()
        : field_presence_mask_(field_presence_mask)
    {
    }
    
    inline
    SimpleFormatter::~SimpleFormatter() throw()
    {
    }    
  }
}

#endif // _ELEMENTS_EL_LOGGING_LOGGERBASE_HPP_
