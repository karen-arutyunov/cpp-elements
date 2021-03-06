/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/String/Template.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>

#include "Template.hpp"

namespace El
{
  namespace String
  {
    namespace Template
    {
      namespace Python
      {
        Parser::Type Parser::Type::instance;
    
        El::Python::Module string_module(
          "el.string.template",
          "Module containing El::String::Template namespace types.",
          true);
    
        Parser::Parser(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Exception)
            : ObjectImpl(type)
        {
          char* text = 0;
          char* left_marker = 0;
          char* right_marker = 0;

          if(!PyArg_ParseTuple(
               args,
               "sss:el.string.template.Parser.Parser",
               &text,
               &left_marker,
               &right_marker))
          {
            El::Python::handle_error(
              "El::String::Template::Python::Parser::Parser");
          }

          parse(text, left_marker, right_marker);
        }

        PyObject*
        Parser::py_instantiate(PyObject* args) throw(El::Exception)
        {
          PyObject* map_obj = 0;
          unsigned char lax = 0;

          if(!PyArg_ParseTuple(args,
                               "O|b:el.string.template.Parser.instantiate",
                               &map_obj,
                               &lax))
          {
            El::Python::handle_error("El::String::Template::Python::Parser::"
                                     "py_instantiate: ParseTuple failed");
          }

          VariablesMap variables;
          map_to_vars(map_obj, variables);

          std::ostringstream ostr;
          instantiate(variables, ostr, lax);
            
          return PyString_FromString(ostr.str().c_str());
        }

        void
        Parser::map_to_vars(PyObject* map_obj, VariablesMap& variables)
          throw(El::Python::Exception, El::Exception)
        {
          if(!PyMapping_Check(map_obj))
          {
            El::Python::report_error(
              PyExc_TypeError,
              "map expected as a 1st argument",
              "El::String::Template::Python::Parser::map_to_vars");
          }

          El::Python::Object_var items = PyMapping_Items(map_obj);

          if(items.in() == 0)
          {
            El::Python::handle_error("El::String::Template::Python::Parser::"
                                     "map_to_vars: PyMapping_Items failed");
          }

          unsigned long len = PyList_Size(items.in());

          for(unsigned long i = 0; i < len; i++)
          {
            PyObject* item = PyList_GetItem(items.in(), i);

            char* key = 0;
            PyObject* value = 0;
            
            if(!PyArg_ParseTuple(item,
                                 "sO:El::String::Template::Python::Parser::"
                                 "map_to_vars",
                                 &key,
                                 &value))
            {
              El::Python::handle_error("El::String::Template::Python::Parser::"
                                       "map_to_vars: ParseTuple[2] failed");
            }

            if(PyString_Check(value))
            {
              const char* val = PyString_AsString(value);
              
              if(val == 0) 
              {
                El::Python::handle_error(
                  "El::String::Template::Python::Parser::"
                  "map_to_vars: PyString_AsString[1] failed");
              }

              size_t slen = 0;
              
              variables[key] =
                El::Python::string_from_string(
                  value,
                  slen,
                  "El::String::Template::Python::Parser::map_to_vars");
            }
            else
            {
              El::Python::Object_var str = PyObject_Str(value);

              if(str.in() == 0)
              {
                El::Python::handle_error(
                  "El::String::Template::Python::Parser::"
                  "map_to_vars: PyObject_Str failed");
              }

              size_t slen = 0;
              
              variables[key] =
                El::Python::string_from_string(
                  str.in(),
                  slen,
                  "El::String::Template::Python::Parser::map_to_vars");
            }
          }

        }
        
      }
    }
  }
}
