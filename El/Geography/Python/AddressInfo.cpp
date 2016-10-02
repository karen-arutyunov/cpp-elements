/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Geography/Python/AddressInfo.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Country.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/RefCount.hpp>

#include "AddressInfo.hpp"

namespace El
{
  namespace Geography
  {
    namespace Python
    { 
      AddressInfo::Type AddressInfo::Type::instance;

      El::Python::Module el_geography_module(
        "el.geography",
        "Module containing ElGeography library types.",
        true);
      
      //
      // AddressInfo class
      //
      AddressInfo::AddressInfo(PyTypeObject *type,
                               PyObject *args,
                               PyObject *kwds)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(type)
      {
      }
      
      PyObject*
      AddressInfo::py_country(PyObject* args) throw(El::Exception)
      {
        char* host = 0;

        if(!PyArg_ParseTuple(args,
                             "s:el.geography.AddressInfo.country",
                             &host))
        {
          El::Python::handle_error(
            "El::Geography::Python::AddressInfo::py_country");
        }

        return new El::Python::Country(country(host));
      }
    }
  }
}
