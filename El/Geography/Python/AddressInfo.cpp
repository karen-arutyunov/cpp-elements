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
