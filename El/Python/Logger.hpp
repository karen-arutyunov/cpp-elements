/*
 * Elements - useful abstractions library.
 * Copyright (C) 2005-2015 Karen Arutyunov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/**
 * @file Elements/El/Python/Logger.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_LOGGER_HPP_
#define _ELEMENTS_EL_PYTHON_LOGGER_HPP_

#include <El/Exception.hpp>
#include <El/Logging/Logger.hpp>

#include <El/Python/RefCount.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Utility.hpp>

namespace El
{
  namespace Logging
  {
    namespace Python
    {
      class Logger : public El::Python::ObjectImpl,
                     public El::Logging::Logger
      {
      public:      
        Logger(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Exception);

        Logger(El::Logging::Logger* backend = 0, bool own = false)
          throw(El::Exception);
        
        virtual ~Logger() throw() { if(own_) delete backend_; }

        virtual unsigned long level() const throw();
        virtual void level(unsigned long val) throw();

        virtual const char* aspects() const throw(El::Exception);
        virtual void aspects(const char* aspect) throw(El::Exception);

        virtual void log(const char* text,
                         unsigned long severity,
                         const char* aspect)
          throw();

        virtual void raw_log(const char* text) throw();
        
        PyObject* py_log(PyObject* args) throw(El::Exception);
        PyObject* py_emergency(PyObject* args) throw(El::Exception);
        PyObject* py_alert(PyObject* args) throw(El::Exception);
        PyObject* py_error(PyObject* args) throw(El::Exception);
        PyObject* py_critical(PyObject* args) throw(El::Exception);
        PyObject* py_warning(PyObject* args) throw(El::Exception);
        PyObject* py_notice(PyObject* args) throw(El::Exception);
        PyObject* py_info(PyObject* args) throw(El::Exception);
        PyObject* py_debug(PyObject* args) throw(El::Exception);
        PyObject* py_trace(PyObject* args) throw(El::Exception);

        PyObject* py_will_log(PyObject* args) throw(El::Exception);
        PyObject* py_will_trace(PyObject* args) throw(El::Exception);

        void py_set_level(PyObject* value) throw(El::Exception);
        PyObject* py_get_level() throw(El::Exception);
      
        void py_set_aspects(PyObject* value) throw(El::Exception);
        PyObject* py_get_aspects() throw(El::Exception);

        class Type : public El::Python::ObjectTypeImpl<Logger, Logger::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          
          static Type instance;
          
          PY_TYPE_METHOD_VARARGS(py_log, "log", "Logs objects");
          
          PY_TYPE_METHOD_VARARGS(py_emergency,
                                 "emergency",
                                 "Logs objects with emergency severity");
          
          PY_TYPE_METHOD_VARARGS(py_alert,
                                 "alert",
                                 "Logs objects with alert severity");
          
          PY_TYPE_METHOD_VARARGS(py_error,
                                 "error",
                                 "Logs objects with error severity");
          
          PY_TYPE_METHOD_VARARGS(py_critical,
                                 "critical",
                                 "Logs objects with critical severity");
          
          PY_TYPE_METHOD_VARARGS(py_warning,
                                 "warning",
                                 "Logs objects with warning severity");
          
          PY_TYPE_METHOD_VARARGS(py_notice,
                                 "notice",
                                 "Logs objects with notice severity");
          
          PY_TYPE_METHOD_VARARGS(py_info,
                                 "info",
                                 "Logs objects with info severity");
          
          PY_TYPE_METHOD_VARARGS(py_debug,
                                 "debug",
                                 "Logs objects with debug severity");
          
          PY_TYPE_METHOD_VARARGS(py_trace,
                                 "trace",
                                 "Logs objects with trace severity");
          
          PY_TYPE_METHOD_VARARGS(
            py_will_log,
            "will_log",
            "Tells if logger will log for the spacified log level");

          PY_TYPE_METHOD_VARARGS(
            py_will_trace,
            "will_trace",
            "Tells if logger will trace for the spacified trace level");

          PY_TYPE_MEMBER(py_get_level,
                         py_set_level,
                         "level",
                         "Set logging level");
          
          PY_TYPE_MEMBER(py_get_aspects,
                         py_set_aspects,
                         "aspects",
                         "Set aspects");
        };

      protected:

        PyObject* log_with_severity(PyObject* args,
                                    unsigned long svr,
                                    const char* context,
                                    bool do_trace = false)
          throw(El::Exception);
          
      protected:
        El::Logging::Logger* backend_;
        bool own_;
      };

      typedef El::Python::SmartPtr<Logger> Logger_var;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Logging
  {
    namespace Python
    {      
      //
      // El::Logging::Python::Logger class
      //
      inline
      Logger::Logger(El::Logging::Logger* backend, bool own)
        throw(El::Exception)
          : ObjectImpl(&Type::instance),
            backend_(backend),
            own_(own)
      {
      }

      //
      // El::Logging::Python::Logger::Type class
      //
      inline
      Logger::Type::Type() throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<Logger, Logger::Type>(
            "el.logging.Logger", "Logger class")
      {
      }
      
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_LOGGER_HPP_
