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
 * @file Elements/El/PSP/Context.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_CONTEXT_HPP_
#define _ELEMENTS_EL_PSP_CONTEXT_HPP_

#include <El/Exception.hpp>

#include <El/PSP/Exception.hpp>
#include <El/PSP/Request.hpp>
#include <El/PSP/Localization.hpp>
#include <El/PSP/Config.hpp>
#include <El/PSP/Code.hpp>

namespace El
{
  namespace PSP
  {
    class Context : public El::Python::ObjectImpl
    {
    public:
      
      Context(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      Context(El::PSP::Code* code,
              El::PSP::Request* request,
              El::PSP::Config* config,
              El::PSP::Localization* localization,
              El::Cache::VariablesMapCache* localization_cache,
              const El::PSP::Request::LangMap& valid_languages,
              PyObject* forward_ags,
              PyObject* cache,
              PyObject* loc_dict) throw(El::Exception);
      
      virtual ~Context() throw() {}

      void run_number(unsigned long long val) throw();

      PyObject* py_get_run_number() throw(El::Exception);
      PyObject* py_define_static(PyObject* args) throw(El::Exception);      
      PyObject* py_language(PyObject* args) throw(El::Exception);
      PyObject* py_valid_language(PyObject* args) throw(El::Exception);
      PyObject* py_get_localization(PyObject* args) throw(El::Exception);
      
      class Type : public El::Python::ObjectTypeImpl<Context, Context::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;

        PY_TYPE_MEMBER_OBJECT(request_,
                              Request::Type,
                              "request",
                              "Request object",
                              false);

        PY_TYPE_MEMBER_OBJECT(config_,
                              Config::Type,
                              "config",
                              "Configuration object",
                              false);

        PY_TYPE_MEMBER_OBJECT(localization_,
                              Localization::Type,
                              "localization",
                              "Localization object",
                              false);

        PY_TYPE_MEMBER_OBJECT(cache_,
                              El::Python::PyDictType,
                              "cache",
                              "Cache object",
                              false);

        PY_TYPE_MEMBER_OBJECT(forward_params_,
                              El::Python::AnyType,
                              "forward_params",
                              "Forward parameters object",
                              false);

        PY_TYPE_CONST_MEMBER(py_get_run_number,
                             "run_number",
                             "Return number of completed page runs since "
                             "last modified");

        PY_TYPE_METHOD_VARARGS(
          py_define_static,
          "define_static",
          "Defines variable static for the code until page next "
          "recompilation (when changed).");
        
        PY_TYPE_METHOD_VARARGS(
          py_language,
          "language",
          "Sets request processing language");

        PY_TYPE_METHOD_VARARGS(
          py_valid_language,
          "valid_language",
          "Returns valid UI language for the specified one");

        PY_TYPE_METHOD_VARARGS(
          py_get_localization,
          "get_localization",
          "Gets localization object");
      };
      
    private:
      El::PSP::Code* code_;
      El::PSP::Request_var request_;
      Config_var config_;
      El::PSP::Localization_var localization_;
      El::Cache::VariablesMapCache* localization_cache_;
      El::PSP::Request::LangMap valid_languages_;
      El::Python::Object_var forward_params_;
      El::Python::Object_var cache_;
      El::Python::Object_var loc_dict_;
      unsigned long long run_number_;
    };

    typedef El::Python::SmartPtr<Context> Context_var;
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
    // El::PSP::Context class
    //
    inline
    void
    Context::run_number(unsigned long long val) throw()
    {
      run_number_ = val;
    }
    
    //
    // El::PSP::Context::Type class
    //
    inline
    Context::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Context, Context::Type>(
          "el.psp.Context",
          "Object encapsulating context of request processing "
          "(request, cache)")
    {
      tp_new = 0;
    }    
  }
}

#endif // _ELEMENTS_EL_PSP_CONTEXT_HPP_
