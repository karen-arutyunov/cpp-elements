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
 * @file Elements/El/Python/Map.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_MAP_HPP_
#define _ELEMENTS_EL_PYTHON_MAP_HPP_

#include <map>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>

#include <El/Python/RefCount.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Utility.hpp>

namespace El
{
  namespace Python
  {
    class Map : public El::Python::ObjectImpl,
                public std::map<Object_var, Object_var, ObjectLess>
    {
    public:      
      Map(PyTypeObject *type = 0, PyObject *args = 0, PyObject *kwds = 0)
        throw(El::Exception);

      virtual ~Map() throw() {}

      virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      virtual void read(El::BinaryInStream& bstr) throw(El::Exception);
      
      PyObject* py_size() throw(El::Exception);
      PyObject* py_keys() throw(El::Exception);
      
      class Type : public El::Python::ObjectTypeImpl<Map, Map::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        
        static Type instance;
        
        PY_TYPE_METHOD_NOARGS(
          py_size,
          "size",
          "Returns associative container size");

        PY_TYPE_METHOD_NOARGS(
          py_keys,
          "keys",
          "Returns keys list");

      private:
        static Py_ssize_t mp_length(PyObject* m);
        static PyObject* mp_subscript(PyObject* m, PyObject* k);
        
        static int mp_ass_subscript(PyObject* m,
                                    PyObject* k,
                                    PyObject* v);

      private:
        PyMappingMethods map_methods_;
      };
    };

    typedef SmartPtr<Map> Map_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    inline
    void
    Map::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      bstr.write_map(*this);
    }

    inline
    void
    Map::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      bstr.read_map(*this);
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_MAP_HPP_
