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
 * @file Elements/El/Python/NameValueMap.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include "NameValueMap.hpp"

namespace El
{
  namespace Python
  {
    NameValueMap::Type NameValueMap::Type::instance;
    
    NameValueMap::NameValueMap(PyTypeObject *type,
                               PyObject *args,
                               PyObject *kwds)
      throw(El::Exception) : Map(type ? type : &Type::instance, args, kwds)
    {
      char* text = 0;
      const char* nvp_separator = ",";
      const char* nv_separator = "=";

      if(args)
      {
        if(!PyArg_ParseTuple(args,
                             "|sss:el.NameValueMap.NameValueMap",
                             &text,
                             &nvp_separator,
                             &nv_separator))
        {
          handle_error("El::Python::NameValueMap::NameValueMap");
        }
      }
      
      nvp_separator_ = nvp_separator[0];
      nv_separator_ = nv_separator[0];

      El::NameValueMap nvm(text, nvp_separator_, nv_separator_);

      for(El::NameValueMap::const_iterator it = nvm.begin();
          it != nvm.end(); it++)
      {
         El::Python::Object_var key = PyString_FromString(it->first.c_str());

         El::Python::Object_var value =
           PyString_FromString(it->second.c_str());
         
         (*this)[key] = value;
      }
      
    }

    PyObject*
    NameValueMap::py_string() throw(El::Exception)
    {
      std::stringstream ostr;

      for(const_iterator it = begin(); it != end(); it++)
      {
        if(it != begin())
        {
          ostr << nvp_separator_;
        }

        El::Python::Object_var key =
          El::Python::string_from_object(
            it->first.in(),
            "El::Python::NameValueMap::py_string");
        

        El::Python::Object_var value =
          El::Python::string_from_object(
            it->second.in(),
            "El::Python::NameValueMap::py_string");

        ostr << PyString_AsString(key.in()) << nv_separator_
             << PyString_AsString(value.in());
        
//        ostr << PyString_AsString(it->first.in()) << nv_separator_
//             << PyString_AsString(it->second.in());
      }    

      return PyString_FromString(ostr.str().c_str());
    }

    PyObject*
    NameValueMap::py_present(PyObject* args) throw(El::Exception)
    {
      PyObject* name = 0;
        
      if(!PyArg_ParseTuple(args, "O:el.NameValueMap.present", &name))
      {
        handle_error("El::Python::NameValueMap::py_present");
      }

      El::Python::Object_var key(add_ref(name));
      return add_ref(find(key) != end() ? Py_True : Py_False);
      
    }
    
    PyObject*
    NameValueMap::py_value(PyObject* args) throw(El::Exception)
    {
      PyObject* name = 0;
      PyObject* default_val = 0;
        
      if(!PyArg_ParseTuple(args,
                           "O|O:el.NameValueMap.present",
                           &name,
                           &default_val))
      {
        handle_error("El::Python::NameValueMap::py_find");
      }

      El::Python::Object_var key(add_ref(name));
      iterator it = find(key);
      
      return it != end() ? it->second.add_ref() :
        (default_val ? add_ref(default_val) : PyString_FromString(""));
    }
  }
}
