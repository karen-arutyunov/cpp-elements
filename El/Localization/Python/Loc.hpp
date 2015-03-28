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
 * @file Elements/El/Localization/Python/Loc.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOCALIZATION_PYTHON_LOC_HPP_
#define _ELEMENTS_EL_LOCALIZATION_PYTHON_LOC_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Lang.hpp>

#include <El/Localization/Loc.hpp>

namespace El
{
  namespace Loc
  {
    namespace Python
    { 
      class Localizer : public El::Python::ObjectImpl
      {
      public:
        Localizer(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Python::Exception, El::Exception);

        Localizer(El::Loc::Localizer* localizer)
          throw(El::Python::Exception, El::Exception);

        virtual ~Localizer() throw() {}

        PyObject* py_language(PyObject* args) throw(El::Exception);
        PyObject* py_country(PyObject* args) throw(El::Exception);
        PyObject* py_plural(PyObject* args) throw(El::Exception);
          
        class Type : public El::Python::ObjectTypeImpl<Localizer,
                                                       Localizer::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_METHOD_VARARGS(
            py_plural,
            "plural",
            "Provides plural representation of word in a language specified");

          PY_TYPE_METHOD_VARARGS(
            py_language,
            "language",
            "Provides language (1-st arg) representation in a language "
            "specified (2-nd) arg");

          PY_TYPE_METHOD_VARARGS(
            py_country,
            "country",
            "Provides country representation in a language specified");
        };

      private:
        El::Loc::Localizer* localizer_;
      };
    }
      
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Loc
  {
    namespace Python
    { 
      //
      // El::Loc::Python::Localizer::Type class
      //
      inline
      Localizer::Type::Type() throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<Localizer, Localizer::Type>(
            "el.loc.Localizer",
            "Object encapsulating El::Loc::Localizer functionality")
      {
        tp_new = 0;
      }      
    }
  }
}

#endif // _ELEMENTS_EL_LOCALIZATION_PYTHON_LOC_HPP_
