/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
