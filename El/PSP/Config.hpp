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
