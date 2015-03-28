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
 * @file Elements/El/Geography/Python/AddressInfo.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_GEOGRAPHY_PYTHON_ADDRESSINFO_HPP_
#define _ELEMENTS_EL_GEOGRAPHY_PYTHON_ADDRESSINFO_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>

#include <El/Geography/AddressInfo.hpp>

namespace El
{
  namespace Geography
  {
    namespace Python
    { 
      class AddressInfo : public El::Python::ObjectImpl,
                          public El::Geography::AddressInfo
      {
      public:
        AddressInfo(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Python::Exception, El::Exception);

        virtual ~AddressInfo() throw() {}

        PyObject* py_country(PyObject* args) throw(El::Exception);
        
        class Type : public El::Python::ObjectTypeImpl<AddressInfo,
                                                       AddressInfo::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_METHOD_VARARGS(
            py_country,
            "country",
            "Detects country by host name or ip address");
        };
      };
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Geography
  {
    namespace Python
    { 
      //
      // El::Geography::Python::AddressInfo::Type class
      //
      inline
      AddressInfo::Type::Type()
        throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<AddressInfo, AddressInfo::Type>(
            "el.geography.AddressInfo",
            "Object encapsulating El::Geography::AddressInfo functionality")
      {
      }
    }
  }
}

#endif // _ELEMENTS_EL_GEOGRAPHY_PYTHON_ADDRESSINFO_HPP_
