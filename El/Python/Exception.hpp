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
 * @file Elements/El/Python/Exception.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_EXCEPTION_HPP_
#define _ELEMENTS_EL_PYTHON_EXCEPTION_HPP_

#include <string>

#include <El/Exception.hpp>

#include <El/Python/RefCount.hpp>

namespace El
{
  namespace Python
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);

    struct SystemExit : public Exception
    {
      Object_var value;

      SystemExit(PyObject* val, const char* desc = 0) throw();
      ~SystemExit() throw() {}
    };

    struct ErrorPropogation : public Exception
    {
      std::string context;
      Object_var type;
      Object_var value;
      Object_var traceback;

      ErrorPropogation(const char* desc,
                       const char* context_val,
                       PyObject* type_val,
                       PyObject* value_val,
                       PyObject* traceback_val) throw();

      ~ErrorPropogation() throw() {}
    };
    
    EL_EXCEPTION(InvalidArg, Exception);
    EL_EXCEPTION(CodeNotCompiled, Exception);
    EL_EXCEPTION(ExecutionInterrupted, Exception);
    EL_EXCEPTION(NotImplemented, Exception);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    inline
    ErrorPropogation::ErrorPropogation(const char* desc,
                                       const char* context_val,
                                       PyObject* type_val,
                                       PyObject* value_val,
                                       PyObject* traceback_val) throw()
        : Exception(desc),
          context(context_val ? context_val : ""),
          type(add_ref(type_val)),
          value(add_ref(value_val)),
          traceback(add_ref(traceback_val))
    {
    }

    inline
    SystemExit::SystemExit(PyObject* val, const char* desc) throw()
        : Exception(desc ? desc : "Python script terminated"),
          value(add_ref(val))
    {
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_UTILITY_HPP_
