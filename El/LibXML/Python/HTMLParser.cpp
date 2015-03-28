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
 * @file Elements/El/LibXML/Python/HTMLParser.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>
#include <libxml/xpath.h>

#include <string>
#include <sstream>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Sequence.hpp>
#include <El/Python/Sandbox.hpp>

#include "HTMLParser.hpp"

namespace El
{
  namespace LibXML
  {
    namespace Python
    {
      HTMLErrorsHolder::Type HTMLErrorsHolder::Type::instance;
      HTMLParser::Type HTMLParser::Type::instance;
      
      //
      // HTMLErrorsHolder class
      //
      HTMLErrorsHolder::HTMLErrorsHolder(PyTypeObject *type,
                                         PyObject *args,
                                         PyObject *kwds)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(type)
      {
      }
      
      HTMLErrorsHolder::HTMLErrorsHolder(
        const ErrorRecorderHandler& error_handler)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(&Type::instance)
      {
        init(error_handler);
      }

      void
      HTMLErrorsHolder::init(const ErrorRecorderHandler& error_handler)
        throw(El::Exception)
      {
        fatal_errors = error_handler.fatal_errors.str();
        errors = error_handler.errors.str();
        warnings = error_handler.warnings.str();
      }
      
      //
      // HTMLParser class
      //
      HTMLParser::HTMLParser(PyTypeObject *type,
                             PyObject *args,
                             PyObject *kwds)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(type ? type : &Type::instance)
      {
      }

      PyObject*
      HTMLParser::parse(const char* text,
                        unsigned long text_len,
                        const char* url,
                        const char* encoding,
                        ErrorHandler* error_handler,
                        unsigned long options)
        throw(El::Exception)
      {
        El::LibXML::HTMLParser parser;

        htmlDocPtr doc = 0;

        {
          El::Python::AllowOtherThreads guard;
          
          doc = parser.parse(text,
                             text_len,
                             url,
                             encoding,
                             error_handler,
                             options);
        }

        if(doc)
        {
          doc_ = new Document(parser.document(false), true);
          return El::Python::add_ref(doc_.in());
        }

        doc_ = 0;
        return El::Python::add_ref(Py_None);
      }
      
      PyObject*
      HTMLParser::parse_file(const char* file_path,
                             const char* encoding,
                             ErrorHandler* error_handler,
                             unsigned long options)
        throw(El::Exception)
      {
        El::LibXML::HTMLParser parser;

        htmlDocPtr doc = 0;

        {
          El::Python::AllowOtherThreads guard;
        
          doc = parser.parse_file(file_path,
                                  encoding,
                                  error_handler,
                                  options);
        }

        if(doc)
        {
          doc_ = new Document(parser.document(false), true);
          return El::Python::add_ref(doc_.in());
        }

        doc_ = 0;
        return El::Python::add_ref(Py_None);
      }
      
      PyObject*
      HTMLParser::py_parse(PyObject* args) throw(El::Exception)
      {
        char* text = 0;
        unsigned long len = ULONG_MAX;
        char* url = 0;
        char* encoding = 0;
        PyObject* errors_holder = 0;
        unsigned long options = 0;

        if(!PyArg_ParseTuple(args,
                             "s|kssOk:el.libxml.HTMLParser.parse",
                             &text,
                             &len,
                             &url,
                             &encoding,
                             &errors_holder,
                             &options))
        {
          El::Python::handle_error(
            "El::LibXML::Python::HTMLParser::py_parse");
        }

        if(len == ULONG_MAX)
        {
          len = strlen(text);
        }

        HTMLErrorsHolder* eh = errors_holder && errors_holder != Py_None ? 
          HTMLErrorsHolder::Type::down_cast(errors_holder) : 0;

        ErrorRecorderHandler error_handler;
        
        PyObject* doc = parse(text,
                              len,
                              url,
                              encoding,
                              eh ? &error_handler : 0,
                              options);
        if(eh)
        {
          eh->init(error_handler);
        }

        return doc;
      }
      
      PyObject*
      HTMLParser::py_parse_file(PyObject* args) throw(El::Exception)
      {
        if(El::Python::Sandbox::thread_sandboxed())
        {
          El::Python::Interceptor::Interruption_var obj =
            new El::Python::Interceptor::Interruption();
          
          obj->reason = "el.libxml.HTMLParser.parse_file is not safe";
          PyErr_SetObject(PyExc_SystemExit, obj.in());
          return 0;
        }
        
        char* file_path = 0;
        char* encoding = 0;
        PyObject* errors_holder = 0;
        unsigned long options = 0;

        if(!PyArg_ParseTuple(args,
                             "s|sOk:el.libxml.HTMLParser.parse_file",
                             &file_path,
                             &encoding,
                             &errors_holder,
                             &options))
        {
          El::Python::handle_error(
            "El::LibXML::Python::HTMLParser::py_parse_file");
        }

        HTMLErrorsHolder* eh = errors_holder && errors_holder != Py_None ? 
          HTMLErrorsHolder::Type::down_cast(errors_holder) : 0;

        ErrorRecorderHandler error_handler;
        
        PyObject* doc = parse_file(file_path,
                                   encoding,
                                   eh ? &error_handler : 0,
                                   options);
        if(eh)
        {
          eh->init(error_handler);
        }

        return doc;
      }
      
    }
  }
}
