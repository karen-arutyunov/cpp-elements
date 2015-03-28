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
 * @file Elements/El/Python/String/Template.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_STRING_TEMPLATE_HPP_
#define _ELEMENTS_EL_PYTHON_STRING_TEMPLATE_HPP_

#include <El/Exception.hpp>
#include <El/String/Template.hpp>

#include <El/Python/Object.hpp>
#include <El/Python/RefCount.hpp>

namespace El
{
  namespace String
  {
    namespace Template
    {
      namespace Python
      {
        class Parser : public El::Python::ObjectImpl,
                       public El::String::Template::Parser
        {
        public:      
          Parser(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);
        
          virtual ~Parser() throw() {}
        
          PyObject* py_instantiate(PyObject* args) throw(El::Exception);
          
          static void map_to_vars(PyObject* map_obj, VariablesMap& variables)
            throw(El::Python::Exception, El::Exception);
        
          class Type : public El::Python::ObjectTypeImpl<Parser, Parser::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
          
            PY_TYPE_METHOD_VARARGS(
              py_instantiate,
              "instantiate",
              "Instantiates template");       
          };
        };

        typedef El::Python::SmartPtr<Parser> Parser_var;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace String
  {
    namespace Template
    {
      namespace Python
      {
        //
        // El::String::Template::Python::Parser class
        //
        inline
        Parser::Type::Type() throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<Parser, Parser::Type>(
              "el.string.template.Parser",
              "Object encapsulating El::String::Template::Parser "
              "functionality")
        {
        }
      }
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_STRING_TEMPLATE_HPP_
