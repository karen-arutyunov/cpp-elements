/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Utility.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>
#include <marshal.h>

#include <iostream>
#include <sstream>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/ArrayPtr.hpp>

#include "Utility.hpp"
#include "RefCount.hpp"
#include "Object.hpp"
#include "Moment.hpp"
#include "Module.hpp"
#include "TimeValue.hpp"
#include "Interceptor.hpp"

namespace El
{
  namespace Python
  {
    void
    interceptor(Interceptor* val) throw()
    {
      Use::interceptor_ = val;
    }    
    
    Use::Mutex Use::lock_;
    unsigned long Use::counter_ = 0;
    Interceptor* Use::interceptor_ = 0;

    void
    Use::start() throw(Exception, El::Exception)
    {
      Guard guard(lock_);

      try
      {
        if(counter_++ == 0)
        {
          PyEval_InitThreads();
          Py_InitializeEx(0);
          
          ObjectType::init();
          Module::init();
        }
      }
      catch(const El::Exception& e)
      {
        std::cerr << "El::Python::Use::start: exception caught:\n"
                  << e << std::endl;
      }
      
    }
    
    void
    Use::stop() throw()
    {
      Guard guard(lock_);

      if(counter_ > 0)
      {
        if(--counter_ == 0)
        {
          interceptor_ = 0;
          
          Module::terminate();
          ObjectType::terminate();
          
          Py_Finalize();
        }
      }
    }

    void
    import_module(const char* name) throw(Exception, El::Exception)
    {
      El::Python::Object_var im =
        PyImport_ImportModule(const_cast<char*>(name));

      if(im.in() == 0)
      {
        handle_error(
          "El::Python::import_module: PyImport_ImportModule failed");
      }

      PyObject* main = PyImport_AddModule("__main__");
      
      if(PyModule_AddObject(main, const_cast<char*>(name), im.add_ref()) < 0)
      {
        try
        {
          handle_error(
            "El::Python::import_module: PyModule_AddObject failed");
        }
        catch(...)
        {
          remove_ref(im.in());
        }
      }
    }

    void
    expose_module(PyObject* module, const char* name)
      throw(Exception, El::Exception)
    {
      PyObject* main = PyImport_AddModule("__main__");
          
      if(PyModule_AddObject(main,
                            const_cast<char*>(name),
                            add_ref(module)) < 0)
      {
        try
        {
          handle_error(
            "El::Python::expose_module: PyModule_AddObject failed");
        }
        catch(...)
        {
          remove_ref(module);
        }
      }  
    }

    unsigned long
    ulong_from_number(PyObject* number, const char* context)
      throw(Exception, El::Exception)
    {
      Object_var lval = PyNumber_Long(number);

      if(lval.in() == 0)
      {
        if(context)
        {
          std::ostringstream ostr;
          
          ostr << context <<
            ": in El::Python::ulong_from_number PyNumber_Long failed";
          
          handle_error(ostr.str().c_str());
        }
        else
        {
          handle_error("El::Python::ulong_from_number: PyNumber_Long failed");
        }
      }
        
      unsigned long result = PyLong_AsUnsignedLong(lval);

      if(PyErr_Occurred() != 0)
      {
        if(context)
        {
          std::ostringstream ostr;
          
          ostr << context <<
            ": in El::Python::ulong_from_number PyLong_AsUnsignedLong failed";
          
          handle_error(ostr.str().c_str());
        }
        else
        {
          handle_error("El::Python::ulong_from_number: "
                       "PyLong_AsUnsignedLong failed");
        }
      }
    
      return result;
    }
    
    unsigned long long
    ulonglong_from_number(PyObject* number, const char* context)
      throw(Exception, El::Exception)
    {
      Object_var lval = PyNumber_Long(number);

      if(lval.in() == 0)
      {
        if(context)
        {
          std::ostringstream ostr;
          
          ostr << context <<
            ": in El::Python::ulonglong_from_number PyNumber_Long failed";
          
          handle_error(ostr.str().c_str());
        }
        else
        {
          handle_error(
            "El::Python::ulonglong_from_number: PyNumber_Long failed");
        }
      }
        
      unsigned long long result = PyLong_AsUnsignedLongLong(lval);

      if(PyErr_Occurred() != 0)
      {
        if(context)
        {
          std::ostringstream ostr;
          
          ostr << context <<
            ": in El::Python::ulonglong_from_number "
            "PyLong_AsUnsignedLongLong failed";
          
          handle_error(ostr.str().c_str());
        }
        else
        {
          handle_error("El::Python::ulonglong_from_number: "
                       "PyLong_AsUnsignedLong failed");
        }
      }
    
      return result;
    }
    const char*
    string_from_string(PyObject* string,
                       size_t& len,
                       const char* context)
      throw(Exception, El::Exception)
    {
      if(string == 0 || !PyString_Check(string))
      {
        if(context)
        {
          std::ostringstream ostr;
          
          ostr << context <<
            ": argument for El::Python::string_from_string is not a string";
          
          report_error(PyExc_RuntimeError, ostr.str().c_str());
        }
        else
        {
          report_error(PyExc_RuntimeError,
                       "El::Python::string_from_string: "
                       "argument is not a string");
        }        
      }
      
      const char* result = PyString_AsString(string);

      if(result == 0)
      {
        if(context)
        {
          std::ostringstream ostr;
          
          ostr << context <<
            ": in El::Python::string_from_string PyString_AsString failed";
          
          handle_error(ostr.str().c_str());
        }
        else
        {
          handle_error("El::Python::string_from_string: "
                       "PyString_AsString failed");
        }
      }

      int slen = PyString_Size(string);

      if(slen < 0)
      {
        if(context)
        {
          std::ostringstream ostr;
          
          ostr << context <<
            ": in El::Python::string_from_string PyString_Size failed";
          
          handle_error(ostr.str().c_str());
        }
        else
        {
          handle_error("El::Python::string_from_string: "
                       "PyString_Size failed");
        }
      }

      len = (size_t)slen;
      return result;
    }

    PyObject*
    string_from_object(PyObject* object, const char* context)
      throw(Exception, El::Exception)
    {
      El::Python::Object_var str = PyObject_Str(object);

      if(str.in() == 0)
      {
        if(context)
        {
          std::ostringstream ostr;
          
          ostr << context <<
            ": in El::Python::object_from_string PyObject_Str failed";
          
          handle_error(ostr.str().c_str());
        }
        else
        {
          El::Python::handle_error(
            "El::Python::object_from_string: "
            "PyObject_Str failed");
        }
      }

      return str.retn();
    }
    
    void
    dump_module(const char* name,
                PyObject* module,
                std::ostream& ostr,
                const char* ident)
      throw(Exception, El::Exception)
    {
      ostr << ident << "module '" << name << "':";

      if(module == 0)
      {
        ostr << " is null\n";
        return;
      }
      else if(!PyModule_Check(module))
      {
        ostr << " not a module\n";
        return;
      }
      else
      {
        ostr << std::endl;
      }      

      PyObject* dict = PyModule_GetDict(module);

      if(dict == 0)
      {
        std::ostringstream ostr;
        ostr << "El::Python::dump_module: '" << name
             << "' module dict is absent";
        
        throw Exception(ostr.str());
      }

      std::string new_ident(ident);
      new_ident += "  ";

      std::ostringstream dict_name;
      dict_name << "dict of module " << name;

      dump_dict(dict_name.str().c_str(), dict, ostr, new_ident.c_str());
    }
    
    void
    dump_dict(const char* name,
              PyObject* dict,
              std::ostream& ostr,
              const char* ident)
      throw(Exception, El::Exception)
    {
      ostr << ident << "dict '" << name << "':";

      if(dict == 0)
      {
        ostr << " is null\n";
        return;
      }
      else if(!PyDict_Check(dict))
      {
        ostr << " not a dict\n";
        return;
      }
      else
      {
        ostr << std::endl;
      }
      
      std::string new_ident(ident);
      new_ident += "  ";

//      int pos = 0;
      Py_ssize_t pos = 0;
      PyObject *key, *value;
      
      while(PyDict_Next(dict, &pos, &key, &value))
      {
        El::Python::Object_var key_obj =
          El::Python::string_from_object(key);
        
        size_t len = 0;
        const char* k = El::Python::string_from_string(key_obj.in(), len);

        const char* tp_name = "<none>";
        
        if(value)
        {
          if(PyModule_Check(value))
          {
            dump_module(k, value, ostr, new_ident.c_str());
            continue;
          }
          
          PyTypeObject* type = value->ob_type;
          
          if(type && type->tp_name && *type->tp_name != '\0')
          {
            tp_name = type->tp_name;
          }
        }
        
        ostr << new_ident << k << " (" << tp_name << ")" << std::endl;
      }      
    }

    class PyModule : public El::Python::ModuleImpl<PyModule>
    {
    public:
      static PyModule instance;

      PyModule() throw(El::Exception);

      PyObject* py_gettimeofday() throw(El::Exception);
      PyObject* py_exit(PyObject* args) throw(El::Exception);
  
      PY_MODULE_METHOD_NOARGS(
        py_gettimeofday,
        "gettimeofday",
        "Returns current time value");  

      PY_MODULE_METHOD_VARARGS(
        py_exit,
        "exit",
        "Raises SystemExit exception terminating script execution");  
    };

    PyModule::PyModule() throw(El::Exception)
        : El::Python::ModuleImpl<PyModule>(
          "el",
          "Module containing Elements library types.",
          true)
    {
    }

    PyObject*
    PyModule::py_gettimeofday() throw(El::Exception)
    {
      return new TimeValue(ACE_OS::gettimeofday());
    }

    PyObject*
    PyModule::py_exit(PyObject* args) throw(El::Exception)
    {
      PyObject* result = 0;
      
      if(!PyArg_ParseTuple(args, "|O:el.exit", &result))
      {
        handle_error("El::PyModule::exit");
      }

      PyErr_SetObject(PyExc_SystemExit, result ? result : Py_None);
      
      return 0;
    }
    
    PyModule PyModule::instance;

    PyDictType PyDictType::instance;
    AnyType AnyType::instance;
  }
}

El::BinaryOutStream&
operator<<(El::BinaryOutStream& bstr, PyObject* obj) throw(El::Exception)
{
  if(obj == 0)
  {
    bstr << (uint8_t)0;
    return bstr;
  }
  
  if(El::Python::el_based_object(obj))
  {
    bstr << (uint8_t)2;
    
    El::Python::ObjectImpl* oi = static_cast<El::Python::ObjectImpl*>(obj);
    bstr << (const char*)obj->ob_type->tp_name << *oi;
  }
  else
  {   
    bstr << (uint8_t)1;
    
    El::Python::Object_var cs = PyMarshal_WriteObjectToString(obj, 1);

    if(cs.in() == 0)
    {
      El::Python::handle_error(
        "operator<<(El::BinaryOutStream&, PyObject*): "
        "PyMarshal_WriteObjectToString failed");
    }

    size_t len = 0;
    const char* str = El::Python::string_from_string(cs.in(), len);

    bstr.write_bytes((const unsigned char*)str, len);
  }
  
  return bstr;
}

El::BinaryInStream&
operator>>(El::BinaryInStream& bstr, El::Python::Object_var& obj)
  throw(El::Exception)
{
  uint8_t impl_type = 0;
  bstr >> impl_type;

  switch(impl_type)
  {
  case 0: obj = 0; break;
  case 1:
    {    
      unsigned char* s = 0;
      size_t len = 0;      
      bstr.read_bytes(s, len);
      
      El::ArrayPtr<unsigned char> str(s);
      obj = PyMarshal_ReadObjectFromString((char*)str.get(), len);
      
      if(obj.in() == 0)
      {
        El::Python::handle_error(
          "operator>>(El::BinaryInStream&, El::Python::Object_var&): "
          "PyMarshal_ReadObjectFromString failed");
      }
      
      break;
    }
  case 2:
    {
      std::string name;
      bstr >> name;
      
      El::Python::ObjectType* tp = El::Python::ObjectType::find(name.c_str());

      if(tp == 0)
      {
        std::ostringstream ostr;
        ostr << "operator>>(El::BinaryInStream&, El::Python::Object_var&): "
          "no implementation found for type " << name;

        throw El::Python::Exception(ostr.str());
      }

//      El::Python::Object_var args = PyTuple_New(0);
//      El::Python::Object_var kws = PyDict_New();
//      El::Python::Object_var o = (*tp->tp_new)(tp, args.in(), kws.in());
      
      if(tp->tp_new == 0)
      {
        std::ostringstream ostr;
        ostr << "operator>>(El::BinaryInStream&, El::Python::Object_var&): "
          "no constructor for type " << name;

        throw El::Python::Exception(ostr.str());
      }
      
      El::Python::Object_var o = (*tp->tp_new)(tp, 0, 0);

      if(o.in() == 0)
      {
        El::Python::handle_error(
          "operator>>(El::BinaryInStream&, El::Python::Object_var&)");
      }
      
      El::Python::ObjectImpl* oi =
        static_cast<El::Python::ObjectImpl*>(o.in());
      
      bstr >> *oi;
      obj = o.retn();
      
      break;
    }
  }

  return bstr;
}


