/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
