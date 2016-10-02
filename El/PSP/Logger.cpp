/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Logger.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <sstream>

#include <El/Logging/LoggerBase.hpp>

#include <El/Python/Module.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/RefCount.hpp>

#include <El/PSP/PSPModule.hpp>

#include "Logger.hpp"

extern El::PSP::Module psp_module_impl;

namespace El
{
  namespace PSP
  {
    Logger::Type Logger::Type::instance;

    //
    // ProxyLogger class
    //
    class ProxyLogger : public virtual El::Logging::LoggerBase
    {
    public:

      ProxyLogger(El::Logging::LoggerBase* logger,
                  unsigned long level,
                  const char* aspects,
                  El::Moment::TimeZone zone,
                  El::Logging::Formatter* formatter,
                  unsigned long aspect_threshold)
        throw(Exception, El::Exception);

      virtual ~ProxyLogger() throw() {}

      virtual void write(const char* line, bool newline, bool lock)
        throw(Exception, El::Exception);
      
    protected:
      El::Logging::LoggerBase* logger_;
    };
    
    ProxyLogger::ProxyLogger(El::Logging::LoggerBase* logger,
                             unsigned long level,
                             const char* aspects,
                             El::Moment::TimeZone zone,
                             El::Logging::Formatter* formatter,
                             unsigned long aspect_threshold)
      throw(Exception, El::Exception)
        : LoggerBase(level,
                     aspects,
                     zone,
                     &std::cerr,
                     formatter,
                     aspect_threshold),
          logger_(logger)
    {
    }

    void
    ProxyLogger::write(const char* line, bool newline, bool lock)
      throw(Exception, El::Exception)
    {
      logger_->write(line, newline, true);
    }
    
    //
    // El::PSP::Logger class
    //
    Logger::Logger(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : El::Logging::Python::Logger(type, args, kwds)
    {
      const char *kwlist[] = { "level", "aspects", NULL};
      
      unsigned long level = El::Logging::DEBUG;
      char* aspects = 0;
      
      if(!PyArg_ParseTupleAndKeywords(
           args,
           kwds,
           "|ks:el.psp.Logger.Logger",
           (char**)kwlist,
           &level,
           &aspects))
      {
        El::Python::handle_error("El::PSP::Logger::Logger");
      }

      backend_ = new ProxyLogger(psp_module_impl.logger(),
                                 level,
                                 aspects && *aspects != '\0' ? aspects : "*",
                                 El::Moment::TZ_GMT,
                                 0,
                                 El::Logging::TRACE);

      own_ = true;
    }
    
  }
}
