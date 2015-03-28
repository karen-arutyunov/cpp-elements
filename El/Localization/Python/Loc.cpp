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
 * @file Elements/El/Localization/Python/Loc.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Lang.hpp>
#include <El/Python/Country.hpp>

#include "Loc.hpp"

namespace El
{
  namespace Loc
  {
    namespace Python
    { 
      Localizer::Type Localizer::Type::instance;
      
      //
      // El::Loc::Localizer class
      //
      Localizer::Localizer(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(type),
            localizer_(0)
      {
        throw Exception("El::Loc::Python::Localizer::Localizer: "
          "unforseen way of object creation");
      }

      Localizer::Localizer(El::Loc::Localizer* localizer)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(&Localizer::Type::instance),
            localizer_(localizer)
      {
      }
      
      PyObject*
      Localizer::py_plural(PyObject* args) throw(El::Exception)
      {
        char* word = 0;
        unsigned long count = 0;
        PyObject* lang = 0;

        if(!PyArg_ParseTuple(args,
                             "skO:el.loc.Localizer.plural",
                             &word,
                             &count,
                             &lang))
        {
          El::Python::handle_error(
            "El::Loc::Python::Localizer::py_plural");
        }

        if(!El::Python::Lang::Type::check_type(lang))
        {
          El::Python::report_error(
            PyExc_TypeError,
            "3rd arguments expected to be of el.Language type",
            "El::Loc::Python::Localizer::py_plural");
        }

        std::ostringstream ostr;
        localizer_->plural(word,
                           count,
                           *static_cast<El::Python::Lang*>(lang),
                           ostr);
        
        return PyString_FromString(ostr.str().c_str());
      }

      PyObject*
      Localizer::py_language(PyObject* args) throw(El::Exception)
      {
        PyObject* lang1 = 0;
        PyObject* lang2 = 0;

        if(!PyArg_ParseTuple(args,
                             "OO:el.loc.Localizer.language",
                             &lang1,
                             &lang2))
        {
          El::Python::handle_error(
            "El::Loc::Python::Localizer::py_language");
        }

        if(!El::Python::Lang::Type::check_type(lang1) ||
           !El::Python::Lang::Type::check_type(lang2))
        {
          El::Python::report_error(
            PyExc_TypeError,
            "arguments of el.Language type expected",
            "El::Loc::Python::Localizer::py_language");
        }

        std::ostringstream ostr;
        localizer_->language(*static_cast<El::Python::Lang*>(lang1),
                             *static_cast<El::Python::Lang*>(lang2),
                             ostr);
        
        return PyString_FromString(ostr.str().c_str());
      }

      PyObject*
      Localizer::py_country(PyObject* args) throw(El::Exception)
      {
        PyObject* country = 0;
        PyObject* lang = 0;

        if(!PyArg_ParseTuple(args,
                             "OO:el.loc.Localizer.country",
                             &country,
                             &lang))
        {
          El::Python::handle_error(
            "El::Loc::Python::Localizer::py_country");
        }

        if(!El::Python::Country::Type::check_type(country))
        {
          El::Python::report_error(
            PyExc_TypeError,
            "1st argument of el.Country type expected",
            "El::Loc::Python::Localizer::py_country");
        }

        if(!El::Python::Lang::Type::check_type(lang))
        {
          El::Python::report_error(
            PyExc_TypeError,
            "2nd argument of el.Language type expected",
            "El::Loc::Python::Localizer::py_country");
        }

        std::ostringstream ostr;
        localizer_->country(*static_cast<El::Python::Country*>(country),
                            *static_cast<El::Python::Lang*>(lang),
                            ostr);
        
        return PyString_FromString(ostr.str().c_str());
      }

      //
      // El::PSP::PyModule class
      //
      class PyModule : public El::Python::ModuleImpl<PyModule>
      {
      public:
        static PyModule instance;
        
        PyModule() throw(El::Exception);
        
        PyObject* py_localizer(PyObject* args) throw(El::Exception);
        
        PY_MODULE_METHOD_VARARGS(
          py_localizer,
          "localizer",
          "Returns Localizer singleton");  
        
      };

      PyModule PyModule::instance;
    
      PyModule::PyModule() throw(El::Exception)
          : El::Python::ModuleImpl<PyModule>(
            "el.loc",
            "Module containing Localization library objects, "
            "types and methods.",
            true)
      {
      }

      PyObject*
      PyModule::py_localizer(PyObject* args) throw(El::Exception)
      {
        return new Localizer(&El::Loc::Localizer::instance());
      }
      
    }
  }
}
