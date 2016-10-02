/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Logger.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <sstream>

#include <El/Python/Module.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/RefCount.hpp>

#include "Logger.hpp"

namespace El
{
  namespace Logging
  {
    namespace Python
    {
      Logger::Type Logger::Type::instance;

      class PyLoggerModule : public El::Python::ModuleImpl<PyLoggerModule>
      {
      public:
        static PyLoggerModule instance;

        PyLoggerModule() throw(El::Exception);

        virtual void initialized() throw(El::Exception);
      };

      PyLoggerModule::PyLoggerModule() throw(El::Exception)
        : El::Python::ModuleImpl<PyLoggerModule>(
          "el.logging",
          "Module containing El::Logging types.",
          true)
      {
      }
      
      void
      PyLoggerModule::initialized() throw(El::Exception)
      {
        add_member(PyLong_FromLong(El::Logging::EMERGENCY), "EMERGENCY");
        add_member(PyLong_FromLong(El::Logging::ALERT), "ALERT");
        add_member(PyLong_FromLong(El::Logging::ERROR), "ERROR");
        add_member(PyLong_FromLong(El::Logging::CRITICAL), "CRITICAL");
        add_member(PyLong_FromLong(El::Logging::WARNING), "WARNING");
        add_member(PyLong_FromLong(El::Logging::NOTICE), "NOTICE");
        add_member(PyLong_FromLong(El::Logging::INFO), "INFO");
        add_member(PyLong_FromLong(El::Logging::DEBUG), "DEBUG");
        add_member(PyLong_FromLong(El::Logging::TRACE), "TRACE");
        
        add_member(PyLong_FromLong(El::Logging::LOW), "LOW");
        add_member(PyLong_FromLong(El::Logging::MIDDLE), "MIDDLE");
        add_member(PyLong_FromLong(El::Logging::HIGH), "HIGH");
      }
    
      PyLoggerModule PyLoggerModule::instance;
      
      //
      // El::Logging::Python::Logger class
      //
      Logger::Logger(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception)
          : ObjectImpl(type ? type : &Type::instance),
            backend_(0),
            own_(false)
      {
      }
  
      PyObject*
      Logger::py_log(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 ULONG_MAX,
                                 "El::Logging::Python::Logger::py_log");
      }
      
      PyObject*
      Logger::py_emergency(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 El::Logging::EMERGENCY,
                                 "El::Logging::Python::Logger::py_emergency");
      }
      
      PyObject*
      Logger::py_alert(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 El::Logging::ALERT,
                                 "El::Logging::Python::Logger::py_alert");
      }
      
      PyObject*
      Logger::py_error(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 El::Logging::ERROR,
                                 "El::Logging::Python::Logger::py_error");
      }
      
      PyObject*
      Logger::py_critical(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 El::Logging::CRITICAL,
                                 "El::Logging::Python::Logger::py_critical");
      }
      
      PyObject*
      Logger::py_warning(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 El::Logging::WARNING,
                                 "El::Logging::Python::Logger::py_warning");
      }
      
      PyObject*
      Logger::py_notice(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 El::Logging::NOTICE,
                                 "El::Logging::Python::Logger::py_notice");
      }
      
      PyObject*
      Logger::py_info(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 El::Logging::INFO,
                                 "El::Logging::Python::Logger::py_info");
      }
      
      PyObject*
      Logger::py_debug(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 El::Logging::DEBUG,
                                 "El::Logging::Python::Logger::py_debug");
      }
      
      PyObject*
      Logger::py_trace(PyObject* args) throw(El::Exception)
      {
        return log_with_severity(args,
                                 ULONG_MAX,
                                 "El::Logging::Python::Logger::py_trace",
                                 true);
      }
      
      PyObject*
      Logger::log_with_severity(PyObject* args,
                                unsigned long svr,
                                const char* context,
                                bool do_trace)
        throw(El::Exception)
      {
        unsigned long size = args ? PyTuple_Size(args) : 0;

        if(size < (svr == ULONG_MAX ? 3 : 2))
        {
          El::Python::report_error(PyExc_LookupError,
                                   "invalid number of arguments",
                                   context);
        }

        unsigned long start_index = 0;

        unsigned long severity = svr != ULONG_MAX ? svr :
          El::Python::ulong_from_number(PyTuple_GetItem(args, start_index++),
                                        context);

        size_t slen = 0;
        const char* aspect =
          El::Python::string_from_string(PyTuple_GetItem(args, start_index++),
                                         slen,
                                         context);

        std::ostringstream ostr;
        
        for(unsigned long i = start_index; i < size; i++)
        {
          PyObject* obj = PyTuple_GetItem(args, i);
          
          if(PyString_Check(obj))
          {
            const char* str = PyString_AsString(obj);
            
            if(str == 0) 
            {
              std::ostringstream ostr;
              ostr << context << ": PyString_AsString[2] failed";
              El::Python::handle_error(ostr.str().c_str());
            }

            ostr << str;
            continue;
          }
            
          El::Python::Object_var ob = PyObject_Str(obj);

          if(ob.in() == 0)
          {
            std::ostringstream ostr;
            ostr << context << ": PyObject_Str failed";
            El::Python::handle_error(ostr.str().c_str());
          }

          const char* str = PyString_AsString(ob.in());
            
          if(str == 0) 
          {
            std::ostringstream ostr;
            ostr << context << ": PyString_AsString[3] failed";
            El::Python::handle_error(ostr.str().c_str());
          }
          
          ostr << str;
        }

        if(do_trace)
        {
          trace(ostr.str().c_str(), aspect, severity);
        }
        else
        {
          log(ostr.str().c_str(), severity, aspect);
        }
        
        return El::Python::add_ref(Py_None);
      }

      void
      Logger::py_set_level(PyObject* value) throw(El::Exception)
      {
        level(El::Python::ulong_from_number(value));
      }
      
      PyObject*
      Logger::py_get_level() throw(El::Exception)
      {
        return PyLong_FromLong(level());
      }
      
      void
      Logger::py_set_aspects(PyObject* value) throw(El::Exception)
      {
        size_t slen = 0;
        
        aspects(El::Python::string_from_string(
                  value,
                  slen,
                  "El::Logging::Python::Logger::py_set_aspects"));
      }
      
      PyObject*
      Logger::py_get_aspects() throw(El::Exception)
      {
        return PyString_FromString(aspects());
      }
      
      PyObject*
      Logger::py_will_log(PyObject* args) throw(El::Exception)
      {
        unsigned long level = 0;
        
        if(!PyArg_ParseTuple(args,
                             "k:el.logging.Logger.will_log",
                             &level))
        {
          El::Python::handle_error("El::Logging::Logger::py_will_log");
        }

        return PyBool_FromLong(will_log(level));
      }
      
      PyObject*
      Logger::py_will_trace(PyObject* args) throw(El::Exception)
      {
        unsigned long level = 0;
        
        if(!PyArg_ParseTuple(args,
                             "k:el.logging.Logger.will_trace",
                             &level))
        {
          El::Python::handle_error("El::Logging::Logger::py_will_trace");
        }

        return PyBool_FromLong(will_trace(level));
      }
      
      unsigned long
      Logger::level() const throw()
      {
        return backend_ ? backend_->level() : 0;
      }  

      void
      Logger::level(unsigned long val) throw()
      {
        if(backend_)
        {
          backend_->level(val);
        }
      }

      const char*
      Logger::aspects() const throw(El::Exception)
      {
        return backend_ ? backend_->aspects() : "*";
      }
      
      void
      Logger::aspects(const char* aspect) throw(El::Exception)
      {
        if(backend_)
        {
          backend_->aspects(aspect);
        }
      }

      void
      Logger::log(const char* text, unsigned long severity, const char* aspect)
        throw()
      {
        if(backend_)
        {
          backend_->log(text, severity, aspect);
        }
      }

      void
      Logger::raw_log(const char* text) throw()
      {
        if(backend_)
        {
          backend_->raw_log(text);
        }
      }
    }
  }
}
