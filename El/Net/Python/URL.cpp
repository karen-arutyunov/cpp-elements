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
 * @file Elements/El/Net/Python/URL.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>

#include "URL.hpp"

namespace El
{
  namespace Net
  {
    namespace Python
    { 
      IpMask::Type IpMask::Type::instance;

      El::Python::Module el_net_module(
        "el.net",
        "Module containing ElNet library types.",
        true);
    
      IpMask::IpMask(PyTypeObject *type,
                     PyObject *args,
                     PyObject *kwds)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(type ? type : &Type::instance)
      {
        char* msk = 0;
        
        if(!PyArg_ParseTuple(args, "|s:el.net.IpMask.IpMask", &msk))
        {
          El::Python::handle_error("El::Net::Python::IpMask::IpMask");
        }

        if(msk)
        {
          try
          {
            IpMask m(msk);
            *this = m;
          }
          catch(const InvalidArg&)
          {
            std::ostringstream ostr;
            ostr << "invalid IP mask '" << msk << "'";
          
            El::Python::report_error(
              PyExc_ValueError,
              ostr.str().c_str(),
              "El::Net::Python::IpMask::IpMask");
          }
        }
      }
      
      PyObject*
      IpMask::py_match(PyObject* args) throw(El::Exception)
      {
        char* ip = 0;
        
        if(!PyArg_ParseTuple(args, "s:el.net.IpMask.match", &ip))
        {
          El::Python::handle_error("El::Net::Python::IpMask::py_match");
        }
          
        return El::Python::add_ref(match(ip) ? Py_True : Py_False);
      }
    }
  }
}
