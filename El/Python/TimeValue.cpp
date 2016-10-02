/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
