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
 * @file Elements/El/Net/Python/URL.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_PYTHON_URL_HPP_
#define _ELEMENTS_EL_NET_PYTHON_URL_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>

#include <El/Net/URL.hpp>

namespace El
{
  namespace Net
  {
    namespace Python
    { 
      class IpMask : public El::Python::ObjectImpl,
                     public El::Net::IpMask
      {
      public:
        IpMask(PyTypeObject *type = 0,
               PyObject *args = 0,
               PyObject *kwds = 0)
          throw(El::Python::Exception, El::Exception);

        IpMask(const El::Net::IpMask& val) throw(El::Exception);
        
        virtual ~IpMask() throw() {}

        PyObject* py_match(PyObject* args) throw(El::Exception);
          
        class Type : public El::Python::ObjectTypeImpl<IpMask, IpMask::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_METHOD_VARARGS(py_match, "match", "Match IP against mask");

          PY_TYPE_MEMBER_ULONG(mask, "mask", "Mask");
          PY_TYPE_MEMBER_ULONG(subnet, "subnet", "Subnet");
        };
      };

      typedef El::Python::SmartPtr<IpMask> IpMask_var;
    }      
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Net
  {
    namespace Python
    {
      //
      // El::Net::Python::IpMask class
      //
      inline
      IpMask::IpMask(const El::Net::IpMask& val) throw(El::Exception)
          : ObjectImpl(&Type::instance)
      {
        *static_cast<El::Net::IpMask*>(this) = val;
      }
      
      //
      // El::Net::Python::IpMask::Type class
      //
      inline
      El::Net::Python::IpMask::Type::Type()
        throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<IpMask, IpMask::Type>(
            "el.net.IpMask",
            "Object encapsulating El::Net::IpMask")
      {
      }
    }
  }
}

#endif // _ELEMENTS_EL_NET_PYTHON_URL_HPP_
