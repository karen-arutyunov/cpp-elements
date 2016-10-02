/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/NameValueMap.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_NAMEVALUEMAP_HPP_
#define _ELEMENTS_EL_PYTHON_NAMEVALUEMAP_HPP_

#include <El/Exception.hpp>
#include <El/NameValueMap.hpp>

#include <El/Python/Object.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Map.hpp>

namespace El
{
  namespace Python
  {
    class NameValueMap : public El::Python::Map
    {
    public:      
      NameValueMap(PyTypeObject *type = 0,
                   PyObject *args = 0,
                   PyObject *kwds = 0)
        throw(El::Exception);

      virtual ~NameValueMap() throw() {}

      PyObject* py_string() throw(El::Exception);
      PyObject* py_present(PyObject* args) throw(El::Exception);
      PyObject* py_value(PyObject* args) throw(El::Exception);
      
      class Type : public El::Python::ObjectTypeImpl<NameValueMap,
                                                     NameValueMap::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;
        
        PY_TYPE_METHOD_NOARGS(
          py_string,
          "string",
          "Returns stringified representation.");

        PY_TYPE_METHOD_VARARGS(
          py_present,
          "present",
          "Returns name-value pair presence flag.");

        PY_TYPE_METHOD_VARARGS(
          py_value,
          "value",
          "Finds and returns value by name.");
      };

    protected:
      char nvp_separator_;
      char nv_separator_;      
    };

    typedef SmartPtr<NameValueMap> NameValueMap_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    //
    // El::Python::NameValueMap class
    //
    inline
    NameValueMap::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<NameValueMap, NameValueMap::Type>(
          "el.NameValueMap",
          "Object encapsulating El::NameValueMap functionality",
          "el.Map")
    {
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_NAMEVALUEMAP_HPP_
