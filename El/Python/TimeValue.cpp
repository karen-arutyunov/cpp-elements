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
 * @file Elements/El/Python/TimeValue.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include "TimeValue.hpp"

namespace El
{
  namespace Python
  {
    TimeValue::Type TimeValue::Type::instance;
    
    TimeValue::TimeValue(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : ObjectImpl(type ? type : &Type::instance)
    {
      const char *kwlist[] = { "second", "usec", NULL};

      unsigned long second = 0;
      unsigned long usec = 0;

      if(args)
      {
        if(!PyArg_ParseTupleAndKeywords(
             args,
             kwds,
             "|kk:el.TimeValue.TimeValue",
             (char**)kwlist,
             &second,
             &usec))
        {
          handle_error("El::Python::TimeValue::TimeValue");
        }
      }
      
      *this = ACE_Time_Value(second, usec);
    }
  
    PyObject*
    TimeValue::py_sec(PyObject* args) throw(El::Exception)
    {
      if(PyTuple_Check(args) && PyTuple_Size(args) > 0)
      {
        unsigned long secnum = 0;
        
        if(!PyArg_ParseTuple(args, "k:el.TimeValue.sec", &secnum))
        {
          handle_error("El::Python::TimeValue::sec");
        }

        sec(secnum);
      }

      return Py_BuildValue("k", sec());
    }
  
    PyObject*
    TimeValue::py_msec(PyObject* args) throw(El::Exception)
    {
      if(PyTuple_Check(args) && PyTuple_Size(args) > 0)
      {
        long msecnum = 0;
        
        if(!PyArg_ParseTuple(args, "l:el.TimeValue.msec", &msecnum))
        {
          handle_error("El::Python::TimeValue::msec");
        }

        msec(msecnum);
      }

      return Py_BuildValue("k", msec());
    }
  
    PyObject*
    TimeValue::py_usec(PyObject* args) throw(El::Exception)
    {
      if(PyTuple_Check(args) && PyTuple_Size(args) > 0)
      {
        unsigned long usecnum = 0;
        
        if(!PyArg_ParseTuple(args, "k:el.TimeValue.usec", &usecnum))
        {
          handle_error("El::Python::TimeValue::usec");
        }

        usec(usecnum);
      }

      return Py_BuildValue("k", usec());
    }
  }
}
