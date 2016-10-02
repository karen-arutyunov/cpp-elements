/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
