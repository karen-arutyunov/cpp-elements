/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Localization.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_LOCALIZATION_HPP_
#define _ELEMENTS_EL_PSP_LOCALIZATION_HPP_

#include <El/Exception.hpp>

#include <El/Cache/VariablesMapCache.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/RefCount.hpp>

namespace El
{
  namespace PSP
  {
    class Localization : public El::Python::ObjectImpl
    {
    public:
      Localization(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      Localization(El::Cache::VariablesMap* localization) throw(El::Exception);

      virtual ~Localization() throw() {}

      PyObject* get(const char* key) throw(Exception, El::Exception);
      
      PyObject* py_get(PyObject* args) throw(El::Exception);
      
      class Type :
        public El::Python::ObjectTypeImpl<Localization, Localization::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;

        PY_TYPE_METHOD_VARARGS(py_get, "get", "Finds localized string by id");
      };

      El::Cache::VariablesMap_var localization;
    };

    typedef El::Python::SmartPtr<Localization> Localization_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace PSP
  {
    //
    // El::PSP::Localization class
    //
    inline
    Localization::Localization(El::Cache::VariablesMap* localization)
      throw(El::Exception) :
        El::Python::ObjectImpl(&Type::instance),
        localization(El::RefCount::add_ref(localization))
    {
    }
    
    //
    // El::PSP::Localization::Type class
    //
    inline
    Localization::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Localization, Localization::Type>(
          "el.psp.Localization",
          "Object providing access to localized string")
    {
    }
  }
}

#endif // _ELEMENTS_EL_PSP_LOCALIZATION_HPP_
