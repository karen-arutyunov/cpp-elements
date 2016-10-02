/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Config.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_CONFIG_HPP_
#define _ELEMENTS_EL_PSP_CONFIG_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Map.hpp>

#include <El/PSP/Exception.hpp>

namespace El
{
  namespace PSP
  {
    class Config : public El::Python::Map
    {
    public:
      Config(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      Config() throw(El::Exception) : Map(&Type::instance, 0, 0) {}

      virtual ~Config() throw() {}

      PyObject* get(const char* key) throw(Exception, El::Exception);
      void set(const char* key, PyObject* val) throw(Exception, El::Exception);
      
      Config* find_owner(const char* key, bool create = false)
        throw(Exception, El::Exception);

      static PyObject* short_key(const char* key) throw(El::Exception);      

      PyObject* py_get(PyObject* args) throw(El::Exception);

      std::string string(const char* key) throw(Exception, El::Exception);
      long long number(const char* key) throw(Exception, El::Exception);
      double double_number(const char* key) throw(Exception, El::Exception);
      El::PSP::Config* config(const char* key) throw(Exception, El::Exception);

      bool present(const char* key) throw(Exception, El::Exception);
      
      class Type : public El::Python::ObjectTypeImpl<Config, Config::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;

        PY_TYPE_METHOD_VARARGS(
          py_get,
          "get",
          "Finds configuration option by it full name");
      };      
    };

    typedef El::Python::SmartPtr<Config> Config_var;
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
    // El::PSP::Config class
    //
    inline
    Config::Config(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception) : El::Python::Map(type, args, kwds)
    {
    }
        
    inline
    bool
    Config::present(const char* key) throw(Exception, El::Exception)
    {
      El::Python::Object_var obj = get(key);
      return obj.in() != 0;
    }
      
    //
    // El::PSP::Config::Type class
    //
    inline
    Config::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Config, Config::Type>(
          "el.psp.Config",
          "Object providing access to PSP configuration",
          "el.Map")
    {
    }
  }
}

#endif // _ELEMENTS_EL_PSP_CONFIG_HPP_
