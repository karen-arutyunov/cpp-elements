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
 * @file Elements/El/LibXML/Python/HTMLParser.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LIBXML_PYTHON_HTMLPARSER_HPP_
#define _ELEMENTS_EL_LIBXML_PYTHON_HTMLPARSER_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>

#include <El/LibXML/HTMLParser.hpp>
#include <El/LibXML/Python/Node.hpp>

namespace El
{
  namespace LibXML
  {
    namespace Python
    { 
      class HTMLErrorsHolder : public El::Python::ObjectImpl
      {
      public:
        HTMLErrorsHolder(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Python::Exception, El::Exception);

        HTMLErrorsHolder(const ErrorRecorderHandler& error_handler)
          throw(El::Python::Exception, El::Exception);

        virtual ~HTMLErrorsHolder() throw() {}

        void init(const ErrorRecorderHandler& error_handler)
          throw(El::Exception);

        class Type : public El::Python::ObjectTypeImpl<HTMLErrorsHolder,
                                                       HTMLErrorsHolder::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_MEMBER_STRING(fatal_errors,
                                "fatal_errors",
                                "Fatal errors",
                                true);

          PY_TYPE_MEMBER_STRING(errors,
                                "errors",
                                "Errors",
                                true);

          PY_TYPE_MEMBER_STRING(warnings,
                                "warnings",
                                "Warnings",
                                true);
        };
        
      private:
        std::string fatal_errors;
        std::string errors;
        std::string warnings;
      };

      typedef El::Python::SmartPtr<HTMLErrorsHolder> HTMLErrorsHolder_var;
      
      class HTMLParser : public El::Python::ObjectImpl
      {
      public:
        HTMLParser(PyTypeObject *type = 0,
                   PyObject *args = 0,
                   PyObject *kwds = 0)
          throw(El::Python::Exception, El::Exception);

        virtual ~HTMLParser() throw() {}

        PyObject* parse(const char* text,
                        unsigned long text_len,
                        const char* url = 0,
                        const char* encoding = 0,
                        ErrorHandler* error_handler = 0,
                        unsigned long options = 0)
          throw(El::Exception);
        
        PyObject* parse_file(const char* file_path,
                             const char* encoding = 0,
                             ErrorHandler* error_handler = 0,
                             unsigned long options = 0)
          throw(El::Exception);
        
        PyObject* py_parse(PyObject* args) throw(El::Exception);
        PyObject* py_parse_file(PyObject* args) throw(El::Exception);
        
        class Type : public El::Python::ObjectTypeImpl<HTMLParser,
                                                       HTMLParser::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_METHOD_VARARGS(py_parse, "parse", "Parses HTML text");

          PY_TYPE_METHOD_VARARGS(
            py_parse_file,
            "parse_file",
            "Parses HTML file");

          PY_TYPE_STATIC_MEMBER(PARSE_RECOVER_, "PARSE_RECOVER");
          PY_TYPE_STATIC_MEMBER(PARSE_NODEFDTD_, "PARSE_NODEFDTD");
          PY_TYPE_STATIC_MEMBER(PARSE_NOERROR_, "PARSE_NOERROR");
          PY_TYPE_STATIC_MEMBER(PARSE_NOWARNING_, "PARSE_NOWARNING");
          PY_TYPE_STATIC_MEMBER(PARSE_PEDANTIC_, "PARSE_PEDANTIC");
          PY_TYPE_STATIC_MEMBER(PARSE_NOBLANKS_, "PARSE_NOBLANKS");
          PY_TYPE_STATIC_MEMBER(PARSE_NONET_, "PARSE_NONET");
          PY_TYPE_STATIC_MEMBER(PARSE_NOIMPLIED_, "PARSE_NOIMPLIED");
          PY_TYPE_STATIC_MEMBER(PARSE_COMPACT_, "PARSE_COMPACT");

        private:
          El::Python::Object_var PARSE_RECOVER_;
          El::Python::Object_var PARSE_NODEFDTD_;
          El::Python::Object_var PARSE_NOERROR_;
          El::Python::Object_var PARSE_NOWARNING_;
          El::Python::Object_var PARSE_PEDANTIC_;
          El::Python::Object_var PARSE_NOBLANKS_;
          El::Python::Object_var PARSE_NONET_;
          El::Python::Object_var PARSE_NOIMPLIED_;
          El::Python::Object_var PARSE_COMPACT_;
        };

      private:
        Document_var doc_;
      };

      typedef El::Python::SmartPtr<HTMLParser> HTMLParser_var;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace LibXML
  {
    namespace Python
    { 
      //
      // El::LibXML::Python::HTMLErrorsHolder::Type class
      //
      inline
      HTMLErrorsHolder::Type::Type()
        throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<HTMLErrorsHolder,
                                       HTMLErrorsHolder::Type>(
            "el.libxml.HTMLErrorsHolder",
            "Object encapsulating "
            "El::LibXML::ErrorRecorderHandler functionality")
      {
      }
      
      //
      // El::LibXML::Python::HTMLParser::Type class
      //
      inline
      HTMLParser::Type::Type()
        throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<HTMLParser, HTMLParser::Type>(
            "el.libxml.HTMLParser",
            "Object encapsulating El::LibXML::HTMLParser functionality")
      {
        PARSE_RECOVER_ = PyLong_FromLong(HTML_PARSE_RECOVER);
        PARSE_NODEFDTD_ = PyLong_FromLong(HTML_PARSE_NODEFDTD);
        PARSE_NOERROR_ = PyLong_FromLong(HTML_PARSE_NOERROR);
        PARSE_NOWARNING_ = PyLong_FromLong(HTML_PARSE_NOWARNING);
        PARSE_PEDANTIC_ = PyLong_FromLong(HTML_PARSE_PEDANTIC);
        PARSE_NOBLANKS_ = PyLong_FromLong(HTML_PARSE_NOBLANKS);
        PARSE_NONET_ = PyLong_FromLong(HTML_PARSE_NONET);
        PARSE_NOIMPLIED_ = PyLong_FromLong(HTML_PARSE_NOIMPLIED);
        PARSE_COMPACT_ = PyLong_FromLong(HTML_PARSE_COMPACT);
      }
    }
  }
}

#endif // _ELEMENTS_EL_LIBXML_PYTHON_HTMLPARSER_HPP_
