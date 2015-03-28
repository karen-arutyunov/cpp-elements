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
 * @file Elements/El/Python/Guid.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include "Guid.hpp"

namespace El
{
  namespace Python
  {
    Guid::Type Guid::Type::instance;
    
    Guid::Guid(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception) : ObjectImpl(type)
    {   
      if(args)
      {
        if(PyTuple_Check(args) && PyTuple_Size(args) > 0)
        {
          char* str = 0;
          
          if(!PyArg_ParseTuple(args, "s:el.Guid.Guid", &str))
          {
            handle_error("El::Python::Guid::Guid");
          }
          
          static_cast<El::Guid&>(*this) = El::Guid(str);
        }
      }
    }
    
    El::Python::ObjectImpl::CMP_RESULT
    Guid::eq(ObjectImpl* o) throw(Exception, El::Exception)
    {
      El::Python::Guid* ob = static_cast<El::Python::Guid*>(o);
      return *ob == *this ? CR_TRUE : CR_FALSE;
    }

    PyObject*
    Guid::py_generate() throw(El::Exception)
    {
      generate();
      return El::Python::add_ref(Py_None);
    }  

    PyObject*
    Guid::py_string(PyObject* args) throw(El::Exception)
    {
      unsigned long format = El::Guid::GF_CLASSIC;
        
      if(!PyArg_ParseTuple(args, "|k:el.Guid.string", &format))
      {
        handle_error("El::Python::Guid::string");
      }

      switch(format)
      {
      case El::Guid::GF_CLASSIC:
      case El::Guid::GF_DENSE:

        return PyString_FromString(
          string((El::Guid::GuidFormat)format).c_str());
        
      default:
        throw Exception("El::Python::Guid::string: argument should be of "
                        "el.Guid.GF_CLASSIC or GF_DENSE values");
      }
    }
    
    void
    Guid::Type::ready() throw(El::Python::Exception, El::Exception)
    {
      GF_CLASSIC_ = PyLong_FromLong(El::Guid::GF_CLASSIC);
      GF_DENSE_ = PyLong_FromLong(El::Guid::GF_DENSE);

      El::Python::ObjectTypeImpl<Guid, Guid::Type>::ready();
    }
  }
}
