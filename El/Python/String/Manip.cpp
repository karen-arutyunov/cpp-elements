/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/String/Manip.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <iostream>
#include <sstream>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>

namespace El
{
  namespace Python
  {
    El::Python::Module string_module(
      "el.string",
      "Module containing El::String namespace types.",
      true);
    
    class PyStringManipModule :
      public El::Python::ModuleImpl<PyStringManipModule>
    {
    public:
      static PyStringManipModule instance;

      PyStringManipModule() throw(El::Exception);

      PyObject* py_mime_url_encode(PyObject* args) throw(El::Exception);
      PyObject* py_mime_url_decode(PyObject* args) throw(El::Exception);
      PyObject* py_utf8_valid(PyObject* args) throw(El::Exception);
      PyObject* py_xml_encode(PyObject* args) throw(El::Exception);
      PyObject* py_xml_decode(PyObject* args) throw(El::Exception);
      PyObject* py_suppress(PyObject* args) throw(El::Exception);
      PyObject* py_truncate(PyObject* args) throw(El::Exception);
      PyObject* py_js_escape(PyObject* args) throw(El::Exception);
      PyObject* py_base64_to_ulong(PyObject* args) throw(El::Exception);
  
      PY_MODULE_METHOD_VARARGS(
        py_mime_url_encode,
        "mime_url_encode",
        "Mime-url encodes string");

      PY_MODULE_METHOD_VARARGS(
        py_mime_url_decode,
        "mime_url_decode",
        "Mime-url decodes string");

      PY_MODULE_METHOD_VARARGS(
        py_utf8_valid,
        "utf8_valid",
        "Check if string is validly UTF-8 encoded");

      PY_MODULE_METHOD_VARARGS(
        py_xml_encode,
        "xml_encode",
        "XML-encodes string");

      PY_MODULE_METHOD_VARARGS(
        py_xml_decode,
        "xml_decode",
        "XML-decodes UTF-8 string");

      PY_MODULE_METHOD_VARARGS(
        py_truncate,
        "truncate",
        "Truncate text specified");
      
      PY_MODULE_METHOD_VARARGS(
        py_suppress,
        "suppress",
        "Suppresses characters specified");

      PY_MODULE_METHOD_VARARGS(
        py_js_escape,
        "js_escape",
        "Escapes string character as required for JavaScript literals");
      
      PY_MODULE_METHOD_VARARGS(
        py_base64_to_ulong,
        "base64_to_ulong",
        "Restore base64-encoded unsigned long");

    private:
      virtual void initialized() throw(El::Exception);
    };

    PyStringManipModule::PyStringManipModule() throw(El::Exception)
        : El::Python::ModuleImpl<PyStringManipModule>(
          "el.string.manip",
          "Module containing El::String::Manip namespace types.",
          true)
    {
    }

    void
    PyStringManipModule::initialized() throw(El::Exception)
    {
      add_member(PyLong_FromLong(El::String::Manip::XE_TEXT_ENCODING),
                 "XE_TEXT_ENCODING");
      
      add_member(PyLong_FromLong(El::String::Manip::XE_ATTRIBUTE_ENCODING),
                 "XE_ATTRIBUTE_ENCODING");
      
      add_member(PyLong_FromLong(El::String::Manip::XE_PRESERVE_UTF8),
                 "XE_PRESERVE_UTF8");
      
      add_member(PyLong_FromLong(El::String::Manip::XE_FORCE_NUMERIC_ENCODING),
                 "XE_FORCE_NUMERIC_ENCODING");

      add_member(PyLong_FromLong(El::String::Manip::XE_LAX_ENCODING),
                 "XE_LAX_ENCODING");

      add_member(PyLong_FromLong(El::String::Manip::UAC_XML_1_0),
                 "UAC_XML_1_0");
    }
    
    PyStringManipModule PyStringManipModule::instance;


    PyObject*
    PyStringManipModule::py_base64_to_ulong(PyObject* args)
      throw(El::Exception)
    {
      char* param = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s:el.string.manip.base64_to_ulong",
                           &param))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_base64_to_ulong");
      }

      uint64_t data = 0;
      
      El::String::Manip::base64_decode(param,
                                       (unsigned char*)&data,
                                       sizeof(data));
      
      return PyLong_FromUnsignedLongLong(data);
    }
    
    PyObject*
    PyStringManipModule::py_mime_url_encode(PyObject* args)
      throw(El::Exception)
    {
      char* param = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s:el.string.manip.mime_url_encode",
                           &param))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_mime_url_encode");
      }

      std::string encoded;
      El::String::Manip::mime_url_encode(param, encoded);
      return PyString_FromString(encoded.c_str());
    }
    
    PyObject*
    PyStringManipModule::py_mime_url_decode(PyObject* args)
      throw(El::Exception)
    {
      char* param = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s:el.string.manip.mime_url_decode",
                           &param))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_mime_url_decode");
      }

      std::string decoded;
      El::String::Manip::mime_url_decode(param, decoded);
      return PyString_FromString(decoded.c_str());
    }

    PyObject*
    PyStringManipModule::py_utf8_valid(PyObject* args)
      throw(El::Exception)
    {
      char* str = 0;
      unsigned long flags = El::String::Manip::UAC_XML_1_0;
        
      if(!PyArg_ParseTuple(args,
                           "s|k:el.string.manip.py_utf8_valid",
                           &str,
                           &flags))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_utf8_valid");
      }

      bool res = El::String::Manip::utf8_valid(str, flags);
      return El::Python::add_ref(res ? Py_True : Py_False);
    }
    
    PyObject*
    PyStringManipModule::py_xml_encode(PyObject* args)
      throw(El::Exception)
    {
      char* str = 0;
      
      unsigned long flags =
        El::String::Manip::XE_TEXT_ENCODING |
        El::String::Manip::XE_ATTRIBUTE_ENCODING |
        El::String::Manip::XE_PRESERVE_UTF8;
        
      if(!PyArg_ParseTuple(args,
                           "s|k:el.string.manip.py_xml_encode",
                           &str,
                           &flags))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_xml_encode");
      }
      
      std::string encoded;
      El::String::Manip::xml_encode(str, encoded, flags);
      return PyString_FromString(encoded.c_str());
    }
    
    PyObject*
    PyStringManipModule::py_xml_decode(PyObject* args)
      throw(El::Exception)
    {
      char* str = 0;
        
      if(!PyArg_ParseTuple(args, "s:el.string.manip.py_xml_encode", &str))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_xml_decode");
      }
      
      std::string decoded;
      El::String::Manip::xml_decode(str, decoded);
      return PyString_FromString(decoded.c_str());
    }
    
    PyObject*
    PyStringManipModule::py_suppress(PyObject* args)
      throw(El::Exception)
    {
      char* param = 0;
      char* sup_chars = 0;
      
      if(!PyArg_ParseTuple(args,
                           "ss:el.string.manip.suppress",
                           &param,
                           &sup_chars))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_suppress");
      }

      std::string suppressed;
      El::String::Manip::suppress(param, suppressed, sup_chars);
      return PyString_FromString(suppressed.c_str());
    }

    PyObject*
    PyStringManipModule::py_truncate(PyObject* args)
      throw(El::Exception)
    {
      char* text = 0;
      unsigned long max_len = 0;
      
      if(!PyArg_ParseTuple(args,
                           "sk:el.string.manip.truncate",
                           &text,
                           &max_len))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_truncate");
      }

      std::string truncated;
          
      El::String::Manip::truncate(text,
                                  truncated,
                                  max_len,
                                  true,
                                  El::String::Manip::UAC_XML_1_0);
      

      return PyString_FromString(truncated.c_str());
    }    
    
    PyObject*
    PyStringManipModule::py_js_escape(PyObject* args)
      throw(El::Exception)
    {
      char* param = 0;
      
      if(!PyArg_ParseTuple(args, "s:el.string.manip.js_escape", &param))
      {
        El::Python::handle_error(
          "El::Python::PyStringManipModule::py_escape");
      }

      std::ostringstream escaped;
      El::String::Manip::js_escape(param, escaped);
      return PyString_FromString(escaped.str().c_str());
    }
    
  }
}

