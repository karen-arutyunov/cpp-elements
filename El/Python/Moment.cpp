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
 * @file Elements/El/Python/Moment.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include "Moment.hpp"
#include "TimeValue.hpp"

namespace El
{
  namespace Python
  {
    Moment::Type Moment::Type::instance;
    
    Moment::Moment(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : ObjectImpl(type)
    {
      PyObject* time_value = 0;

      char* string_val = 0;
      char* tm_format = 0;
        
      unsigned long year = 0;
      unsigned long month = 0;
      unsigned long day = 0;
      unsigned long hour = 0;
      unsigned long minute = 0;
      unsigned long second = 0;
      unsigned long usec = 0;
      char* tz = 0;

      if(args)
      {
        if(PyTuple_Size(args) >= 1)
        {
          PyObject* obj = PyTuple_GetItem(args, 0);
          
          if(El::Python::TimeValue::Type::check_type(obj))
          {
            if(!PyArg_ParseTuple(
                 args,
                 "O|s:el.Moment.Moment",
                 &time_value,
                 &tz))
            {
              handle_error("El::Python::Moment::Moment");
            }
          }
          else if(PyString_Check(obj))
          {
            if(!PyArg_ParseTuple(
                 args,
                 "ss|s:el.Moment.Moment",
                 &string_val,
                 &tm_format,
                 &tz))
            {
              handle_error("El::Python::Moment::Moment");
            }
          }
        }
        
        if(time_value == 0 && tm_format == 0)
        {  
          const char *kwlist[] = { "year", "month", "day", "hour", "minute",
                                   "second", "usec", "tz", NULL};        
          
          if(!PyArg_ParseTupleAndKeywords(
               args,
               kwds,
               "|kkkkkkks:el.Moment.Moment",
               (char**)kwlist,
               &year,
               &month,
               &day,
               &hour,
               &minute,
               &second,
               &usec,
               &tz))
          {
            handle_error("El::Python::Moment::Moment");
          }
        }
      }
      
      El::Moment::TimeZone zone = El::Moment::TZ_GMT;
      
      if(tz)
      {
        if(strcasecmp(tz, "LOCAL") == 0)
        {
          zone = El::Moment::TZ_LOCAL;
        }
        else if(strcasecmp(tz, "GMT"))
        {
          report_error(
            PyExc_TypeError,
            "tz parameter should be one of 'LOCAL', 'GMT' (default)",
            "El::Python::Moment::Moment");
        }
      }

      El::Moment::TimeFormat time_format = El::Moment::TF_ISO_8601;
        
      if(tm_format)
      {
        if(strcasecmp(tm_format, "TF_RFC_0822") == 0)
        {
          time_format = El::Moment::TF_RFC_0822;
        }
        if(strcasecmp(tm_format, "ISO_8601"))
        {
          report_error(
            PyExc_TypeError,
            "tm_format parameter should be one of 'TF_RFC_0822', 'ISO_8601' "
            "(default)", "El::Python::Moment::Moment");
        }
      }

      if(time_value)
      {  
        *this =
          El::Moment(*El::Python::TimeValue::Type::down_cast(time_value),
                     zone);        
      }
      else if(tm_format)
      {  
        *this = El::Moment(string_val, time_format, zone);        
      }
      else
      {
        *this = El::Moment(year, month, day, hour, minute, second, usec, zone);
      }
    }
  
    El::Python::ObjectImpl::CMP_RESULT
    Moment::eq(ObjectImpl* o) throw(Exception, El::Exception)
    {
      El::Python::Moment* ob = static_cast<El::Python::Moment*>(o);
      return *ob == *this ? CR_TRUE : CR_FALSE;
    }

    PyObject*
    Moment::py_iso8601(PyObject* args) throw(El::Exception)
    {
      unsigned char timezone = 1;

      if(!PyArg_ParseTuple(args, "|b:el.Moment.iso8601", &timezone))
      {
        handle_error("El::Python::Moment::iso8601");
      }

      return PyString_FromString(iso8601(timezone).c_str());
    }

    PyObject*
    Moment::py_rfc0822(PyObject* args) throw(El::Exception)
    {
      unsigned char timezone = 1;
      unsigned char usec = 0;

      if(!PyArg_ParseTuple(args, "|bb:el.Moment.rfc0822", &timezone, &usec))
      {
        handle_error("El::Python::Moment::rfc0822");
      }

      return PyString_FromString(rfc0822(timezone, usec).c_str());
    }

    PyObject*
    Moment::py_time_value() throw(El::Exception)
    {
      ACE_Time_Value tv = *this;
      return new TimeValue(tv);
    }
    
    PyObject*
    Moment::py_dense_format() throw(El::Exception)
    {
      return PyString_FromString(dense_format().c_str());      
    }    
  }
}
