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
 * @file Elements/El/PSP/Context.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>
#include <dictobject.h>

#include <El/Exception.hpp>
#include <El/Python/Object.hpp>

#include "Context.hpp"

namespace El
{
  namespace PSP
  {
    Context::Type Context::Type::instance;

    //
    // Context class
    //
    Context::Context(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type),
          code_(0),
          localization_cache_(0),
          run_number_(0)
    {
      throw Exception(
        "El::PSP::Context::Context: unforseen way of object creation");
    }    
      
    Context::Context(El::PSP::Code* code,
                     El::PSP::Request* request,
                     El::PSP::Config* config,
                     El::PSP::Localization* localization,
                     El::Cache::VariablesMapCache* localization_cache,
                     const El::PSP::Request::LangMap& valid_languages,
                     PyObject* forward_ags,
                     PyObject* cache,
                     PyObject* loc_dict) throw(El::Exception)
        : El::Python::ObjectImpl(&Type::instance),
          code_(code),
          request_(El::Python::add_ref(request)),
          config_(add_ref(config)),
          localization_(add_ref(localization)),
          localization_cache_(localization_cache),
          valid_languages_(valid_languages),
          forward_params_(
            El::Python::add_ref(forward_ags ? forward_ags : Py_None)),
          cache_(El::Python::add_ref(cache)),
          loc_dict_(El::Python::add_ref(loc_dict)),
          run_number_(0)
    {
    }

    PyObject*
    Context::py_language(PyObject* args) throw(El::Exception)
    {
      PyObject* l = 0;
        
      if(!PyArg_ParseTuple(args, "O:el.psp.Context.language", &l))
      {
        El::Python::handle_error("El::PSP::Context::py_language");
      }

      El::Lang lang(*El::Python::Lang::Type::down_cast(l));

      El::Cache::VariablesMap_var loc =
        localization_cache_->get(code_->filename(), lang);
      
      El::PSP::Localization_var localization =
        new El::PSP::Localization(loc.in());      

      request_->language(lang, localization);
      localization_ = localization.retn();
      
      return El::Python::add_ref(Py_None);
    }      
    
    PyObject*
    Context::py_valid_language(PyObject* args) throw(El::Exception)
    {
      PyObject* l = 0;
        
      if(!PyArg_ParseTuple(args, "O:el.psp.Context.valid_language", &l))
      {
        El::Python::handle_error("El::PSP::Context::py_valid_language");
      }

      El::Lang lang(*El::Python::Lang::Type::down_cast(l));

      El::Python::Lang_var valid_lang = new El::Python::Lang();

      El::PSP::Request::LangMap::const_iterator i =
        valid_languages_.find(lang.l3_code());

      if(i == valid_languages_.end())
      {
        i = valid_languages_.find("*");
      }

      if(i != valid_languages_.end())
      {
        *valid_lang = i->second;
      }
      
      return valid_lang.retn();
    }
    
    PyObject*
    Context::py_get_run_number() throw(El::Exception)
    {
      return PyLong_FromUnsignedLongLong(run_number_);
    }

    PyObject*
    Context::py_define_static(PyObject* args) throw(El::Exception)
    {
      char* name = 0;
      PyObject* value = 0;
      
      if(!PyArg_ParseTuple(args,
                           "sO:el.psp.Context.define_static",
                           &name,
                           &value))
      {
        El::Python::handle_error("El::PSP::Context::py_define_static");
      }

      if(PyDict_SetItemString(loc_dict_.in(), name, value))
      {
        El::Python::handle_error(
          "El::PSP::Context::py_define_static: PyDict_SetItemString failed");
      }

      El::Python::Object_var v(El::Python::add_ref(value));
      code_->static_vars_[name] = v;
      
      return El::Python::add_ref(Py_None);
    }
    
    PyObject*
    Context::py_get_localization(PyObject* args) throw(El::Exception)
    {
      char* filename = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s:el.psp.Context.get_localization",
                           &filename))
      {
        El::Python::handle_error("El::PSP::Context::py_get_localization");
      }

      El::Cache::VariablesMap_var loc =
        localization_cache_->get(filename, request_->input()->lang());
      
      return new El::PSP::Localization(loc.in());
    }
  }
}
