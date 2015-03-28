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
