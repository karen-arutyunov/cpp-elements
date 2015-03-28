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
 * @file Elements/El/Python/Map.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <sstream>

#include <El/Python/Utility.hpp>
#include <El/Python/RefCount.hpp>

#include "Map.hpp"

namespace El
{
  namespace Python
  {
    Map::Type Map::Type::instance;
    
    //
    // El::Python::Map class
    //
    Map::Map(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception) : ObjectImpl(type ? type : &Type::instance)
    {
    }    
  
    PyObject*
    Map::py_size() throw(El::Exception)
    {
      return Py_BuildValue("k", size());
    }

    PyObject*
    Map::py_keys() throw(El::Exception)
    {
      size_t len = size();
      El::Python::Object_var list = PyList_New(len);

      if(list.in() == 0)
      {
        return 0;
      }

      size_t i = 0;
      
      for(iterator it = begin(); it != end(); it++, i++)
      {
        El::Python::Object_var key = it->first;
        if(PyList_SetItem(list.in(), i, key.retn()) < 0)
        {
          return 0;
        }
      }
      
      return list.retn();
    }

    //
    // El::Python::Map::Type class
    //
    Map::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Map, Map::Type>(
          "el.Map", "Map class")
    {
      memset(&map_methods_, 0, sizeof(map_methods_));

      map_methods_.mp_length = &mp_length;
      map_methods_.mp_subscript = &mp_subscript;
      map_methods_.mp_ass_subscript = &mp_ass_subscript;

      tp_as_mapping = &map_methods_;
    }
    
    Py_ssize_t
    Map::Type::mp_length(PyObject* m)
    {
      return static_cast<Map*>(m)->size();
    }

    PyObject*
    Map::Type::mp_subscript(PyObject* m, PyObject* k)
    {
      const Map& mp = *static_cast<Map*>(m);
      Object_var key = add_ref(k);
      
      Map::const_iterator it = mp.find(key);

      if(it == mp.end())
      {
        set_key_error("non-existing key provided");
        return 0;
      }
      
      return const_cast<Object_var&>(it->second).add_ref();
    }
    
    int
    Map::Type::mp_ass_subscript(PyObject* m, PyObject* k, PyObject* v)
    {
      Map& mp = *static_cast<Map*>(m);
      Object_var key = add_ref(k);

      if(v == 0)
      {
        mp.erase(key);        
      }
      else
      {
        Object_var val = add_ref(v);
        mp[key] = val;
      }

      return 0;
    }
    
  }
}
