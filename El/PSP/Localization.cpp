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
 * @file Elements/El/PSP/Localization.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <sstream>

#include "Localization.hpp"

namespace El
{
  namespace PSP
  {
    Localization::Type Localization::Type::instance;

    Localization::Localization(PyTypeObject *type,
                               PyObject *args,
                               PyObject *kwds)
      throw(El::Exception) : El::Python::ObjectImpl(type)
    {
/*      
      char* kwlist[] =
        { "path",
          NULL
        };

      const char* path = 0;
      
      if(!PyArg_ParseTupleAndKeywords(
           args,
           kwds,
           "|s:el.psp.Localization.Localization",
           kwlist,
           &path))
      {
        El::Python::handle_error("El::PSP::Localization::Localization");
      }
*/
    }
        
    PyObject*
    Localization::get(const char* key) throw(Exception, El::Exception)
    {
      try
      {
        return PyString_FromString(localization->get(key).c_str());
      }
      catch(const El::Cache::VariablesMap::VariableNotFound& )
      {
      }
      
      return 0;
    }
    
    PyObject*
    Localization::py_get(PyObject* args) throw(El::Exception)
    {
      if(localization.in() == 0)
      {
        El::Python::report_error(
          PyExc_TypeError,
          "no localization object present for the resource",
          "El::PSP::Localization::py_get");
      }
      
      char* key = 0;
      char* def_val = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s|s:el.psp.Localization.get",
                           &key,
                           &def_val))
      {
        El::Python::handle_error("El::PSP::Localization::py_get");
      }

      if(key == 0 || *key == '\0')
      {
        El::Python::report_error(
          PyExc_TypeError,
          "Non-empty string expected as a parameter for "
          "el.psp.Localization.get",
          "El::PSP::Localization::py_get");
      }

      PyObject* res = get(key);

      if(res == 0)
      {
        if(def_val == 0)
        {
          std::ostringstream ostr;
          ostr << "localized string not found for id '" << key << "'";
          
          El::Python::report_error(PyExc_LookupError,
                                   ostr.str().c_str(),
                                   "El::PSP::Localization::py_get");
        }
        else
        {
          res = PyString_FromString(def_val);
        }
      }
      
      return res;
    }
    
  }
}
