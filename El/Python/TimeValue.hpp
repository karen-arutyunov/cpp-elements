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
 * @file Elements/El/Python/TimeValue.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_TIMEVALUE_HPP_
#define _ELEMENTS_EL_PYTHON_TIMEVALUE_HPP_

#include <ace/OS.h>

#include <El/Exception.hpp>

#include <El/Python/Object.hpp>

namespace El
{
  namespace Python
  {
    class TimeValue : public El::Python::ObjectImpl,
                      public ACE_Time_Value
    {
    public:      
      TimeValue(PyTypeObject *type = 0, PyObject *args = 0, PyObject *kwds = 0)
        throw(El::Exception);

      TimeValue(const ACE_Time_Value& val) throw(El::Exception);

      virtual ~TimeValue() throw() {}
      
      virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      virtual void read(El::BinaryInStream& bstr) throw(El::Exception);

      PyObject* py_sec(PyObject* args) throw(El::Exception);
      PyObject* py_msec(PyObject* args) throw(El::Exception);
      PyObject* py_usec(PyObject* args) throw(El::Exception);
      
      class Type : public El::Python::ObjectTypeImpl<TimeValue,
                                                     TimeValue::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;
        
        PY_TYPE_METHOD_VARARGS(
          py_sec,
          "sec",
          "Returns number of seconds");

        PY_TYPE_METHOD_VARARGS(
          py_msec,
          "msec",
          "Returns number of milliseconds");

        PY_TYPE_METHOD_VARARGS(
          py_usec,
          "usec",
          "Returns number of microseconds");
      };
    };

    typedef SmartPtr<TimeValue> TimeValue_var;    
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    //
    // El::Python::TimeValue::Type class
    //
    inline
    TimeValue::TimeValue(const ACE_Time_Value& val) throw(El::Exception)
        : ObjectImpl(&Type::instance)
    {
      *static_cast<ACE_Time_Value*>(this) = val;
    }
    
    inline
    void
    TimeValue::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      bstr << (uint64_t)sec() << (uint64_t)usec();
    }
    
    inline
    void
    TimeValue::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      uint64_t s = 0;
      uint64_t u = 0;
      bstr >> s >> u;
      set(s, u);
    }
    
    //
    // El::Python::TimeValue class
    //
    inline
    TimeValue::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<TimeValue, TimeValue::Type>(
          "el.TimeValue",
          "Object encapsulating ACE_Time_Value functionality")
    {
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_TIMEVALUE_HPP_
