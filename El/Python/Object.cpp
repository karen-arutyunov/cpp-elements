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
 * @file Elements/El/Python/Object.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <sstream>

#include <frameobject.h>
#include <traceback.h>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>

#include "Object.hpp"
#include "Utility.hpp"

namespace
{
  const char EL_SERIALIZED_KEY[] = "__el_serialized_key__";
  
  PyObject*
  __el_based_object__(PyObject* self)
  {
    return El::Python::add_ref(Py_True);
  }    

  PyObject*
  __el_getstate__(PyObject* self)
  {
    El::Python::Object_var val;
    
    try
    {
      std::ostringstream ostr;
      
      {
        El::BinaryOutStream bstr(ostr);

        El::Python::ObjectImpl* oi = static_cast<El::Python::ObjectImpl*>(self);
        bstr << *oi;
      }

      std::string sval(ostr.str());
      std::string encoded;

      El::String::Manip::base64_encode((const unsigned char*)sval.c_str(),
                                       sval.length(),
                                       encoded);

//      std::cerr << "__el_getstate__: " << encoded.length() << ":"
//                << encoded << "\n";

      val = PyString_FromString(encoded.c_str());
    }
    catch(const El::Exception& e)
    {
      El::Python::set_runtime_error(e.what());      
      return 0;
    }
    
    El::Python::Object_var dictionary = PyDict_New();

    if(dictionary.in() == 0 || val.in() == 0 ||
       PyDict_SetItemString(dictionary.in(), EL_SERIALIZED_KEY, val.in()))
    {
      return 0;
    }

//    std::cerr << "__el_getstate__: 0x" << std::hex << dictionary.in() << "\n";
    
    return dictionary.retn();
  }

  PyObject*
  __el_setstate__(PyObject* self, PyObject* args)
  {
    PyObject* dict = 0;
      
    if(!PyArg_ParseTuple(args, "O:__el_setstate__", &dict))
    {
      return 0;
    }
    
//    std::cerr << "__el_setstate__: 0x" << std::hex << dict << "\n";

    if(!PyDict_Check(dict))
    {
      El::Python::set_type_error("__el_setstate__: object is not a dict");
      return 0;
    }

    PyObject* val = PyDict_GetItemString(dict, EL_SERIALIZED_KEY);

    size_t len = 0;
    const char* str = 0;

    try
    {
      str = El::Python::string_from_string(val, len, "__el_setstate__");

//      std::cerr << "__el_setstate__: " << std::dec << len << ":" << str
//                << "\n";

      std::string decoded;
      El::String::Manip::base64_decode(str, decoded);

      std::istringstream istr(decoded);
      
      {
        El::BinaryInStream bstr(istr);
        El::Python::ObjectImpl* oi = static_cast<El::Python::ObjectImpl*>(self);
        
        bstr >> *oi;
      }
    }
    catch(const El::Exception& e)
    {
      El::Python::set_runtime_error(e.what());      
      return 0;
    }

    return El::Python::add_ref(Py_None);
  }
}

namespace El
{
  namespace Python
  {    
    bool ObjectType::initialized_ = false;
    
    //
    // ObjectType class
    //

    ObjectType::ObjectType(const char* type_name,
                           const char* type_doc,
                           const char* base)
      throw(Exception, El::Exception)
        : method_count_(0),
          member_count_(0),
          base_type_(base ? base : "")
    {
      memset(static_cast<PyTypeObject*>(this), 0, sizeof(PyTypeObject));
        
      PyObject head = { PyObject_HEAD_INIT(NULL) };
      memcpy(static_cast<PyTypeObject*>(this), &head, sizeof(head));

      tp_name = El::String::Manip::duplicate(type_name);
      tp_doc = El::String::Manip::duplicate(type_doc);
      tp_str = object_str;

      tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;

      registry()[type_name] = this;

      add_method(reinterpret_cast<PyCFunction>(__el_based_object__),
                 METH_NOARGS,
                 "__el_based_object__",
                 "Return True confirming this is an Elements based object");

      add_method(reinterpret_cast<PyCFunction>(__el_getstate__),
                 METH_NOARGS,
                 "__getstate__",
                 "Return object state for pickling");

      add_method(reinterpret_cast<PyCFunction>(__el_setstate__),
                 METH_VARARGS,
                 "__setstate__",
                 "Accept object state for unpickling");
    }
        
    ObjectType::~ObjectType() throw()
    {
      if(initialized_)
      {      
// No sense to erase as soon as framework is not stay in consistent way
// if some objects deregistered before application terminate
        clear();
      }
      else
      {
        registry().erase(tp_name);
      }
    }

    ObjectType*
    ObjectType::find(const char* name) throw()
    {
      ObjectTypeMap::const_iterator i = registry().find(name);
      return i == registry().end() ? 0 : i->second;
    }
    
    PyObject*
    ObjectType::object_str(PyObject* obj) throw()
    {
      try
      {
        return static_cast<ObjectImpl*>(obj)->str();
      }
      catch(El::Exception& e)
      {
        set_error(e);
      }

      return 0;
    }
    
    void
    ObjectType::ready() throw(Exception, El::Exception)
    {
      if(PyType_Ready(this) < 0) 
      {
        El::Python::handle_error(
          "El::Python::ObjectType::ready: PyType_Ready failed");
      }
    }
    
    void
    ObjectType::clear() throw()
    {
      delete [] tp_name;
      tp_name = 0;
      
      delete [] tp_doc;
      tp_doc = 0;

      if(tp_methods)
      {
        PyMethodDef* mdef = tp_methods;
      
        for(unsigned long i = 0;
            i < method_count_ && mdef->ml_name != 0; i++, mdef++)
        {
          delete [] mdef->ml_name;
          delete [] mdef->ml_doc;
        }
      
        delete [] tp_methods;
        tp_methods = 0;
        method_count_ = 0;
      }

      if(tp_getset)
      {
        PyGetSetDef* mdef = tp_getset;
      
        for(unsigned long i = 0;
            i < member_count_ && mdef->name != 0; i++, mdef++)
        {
          delete [] mdef->name;
          delete [] mdef->doc;
        }
        
        delete [] tp_getset;
        tp_getset = 0;
        member_count_ = 0;
      }
    }
      
    void
    ObjectType::add_member(getter get_func,
                           setter set_func,
                           const char* name,
                           const char* doc)
      throw(El::Exception)
    {
      PyGetSetDef* mdef = tp_getset;
      unsigned long i = 0;
    
      for(; i < member_count_ && mdef->name != 0; i++, mdef++);

      if(i == member_count_)
      {
        unsigned long new_member_count = member_count_ + 3;

        // Additional record is sentinel record
        PyGetSetDef* new_tp_getset = new PyGetSetDef[new_member_count + 1];

        memcpy(new_tp_getset,
               tp_getset,
               member_count_ * sizeof(PyGetSetDef));

        memset(new_tp_getset + member_count_,
               0,
               sizeof(PyGetSetDef) * (new_member_count - member_count_ + 1));

        delete [] tp_getset;
        
        tp_getset = new_tp_getset;
        mdef = tp_getset + member_count_;
        
        member_count_ = new_member_count;
      }
      
      mdef->name = El::String::Manip::duplicate(name);
      mdef->get = reinterpret_cast<getter>(get_func);
      mdef->set = reinterpret_cast<setter>(set_func);
      mdef->doc = El::String::Manip::duplicate(doc);
    }
      
    void
    ObjectType::add_method(PyCFunction func,
                           int flags,
                           const char* name,
                           const char* doc)
      throw(El::Exception)
    {
      PyMethodDef* mdef = tp_methods;
      unsigned long i = 0;
    
      for(; i < method_count_ && mdef->ml_name != 0; i++, mdef++);

      if(i == method_count_)
      {
        unsigned long new_method_count = method_count_ + 3;

        // Additional record is sentinel record
        PyMethodDef* new_tp_methods = new PyMethodDef[new_method_count + 1];

        memcpy(new_tp_methods,
               tp_methods,
               method_count_ * sizeof(PyMethodDef));

        memset(new_tp_methods + method_count_,
               0,
               sizeof(PyMethodDef) * (new_method_count - method_count_ + 1));

        delete [] tp_methods;
        
        tp_methods = new_tp_methods;
        mdef = tp_methods + method_count_;
        
        method_count_ = new_method_count;
      }

      mdef->ml_name = El::String::Manip::duplicate(name);
      mdef->ml_meth = func;
      mdef->ml_flags = flags;
      mdef->ml_doc = El::String::Manip::duplicate(doc);
    }
    
    void
    ObjectType::add_to_module(PyObject* module)
      throw(Exception, El::Exception)
    {
      const char* dot = strrchr(tp_name, '.');
      char* name = const_cast<char*>(dot ? dot + 1 : tp_name);
      
      PyTypeObject* type = this;
        
      if(PyModule_AddObject(
           module,
           name,
           El::Python::add_ref(reinterpret_cast<PyObject*>(type))) < 0)
      {
        El::Python::remove_ref(reinterpret_cast<PyObject*>(type));
        
        El::Python::handle_error(
          "El::Python::ObjectType::add_to_module: "
          "PyModule_AddObject failed");
      }
    }
    
    ObjectType::ObjectTypeMap&
    ObjectType::registry() throw(El::Exception)
    {
      static ObjectTypeMap type_map;
      return type_map;
    }
    
    void
    ObjectType::init() throw(Exception, El::Exception)
    {
      ObjectTypeMap& type_map = registry();

      for(ObjectTypeMap::iterator it = type_map.begin(); it != type_map.end();
          it++)
      {
        ObjectType* type = it->second;

        if(!type->base_type_.empty())
        {
          ObjectTypeMap::iterator bit = type_map.find(type->base_type_);

          if(bit == type_map.end())
          {
            std::ostringstream ostr;
            ostr << "El::Python::ObjectType::init: can't find base type '"
                 << type->base_type_ << "' for type '" << type->tp_name
                 << "'";

            throw Exception(ostr.str());
          }

          type->tp_base = bit->second;
        }
      }
      
      for(ObjectTypeMap::iterator it = type_map.begin(); it != type_map.end();
          it++)
      {
        it->second->ready();        
      }

      initialized_ = true;
    }
    
    void
    ObjectType::terminate() throw(El::Exception)
    {
      registry().clear();
      initialized_ = false;
    }
    
    //
    // ObjectImpl class
    //
    PyObject*
    ObjectImpl::str() throw(Exception, El::Exception)
    {
      std::ostringstream ostr;
      ostr << "<" << static_cast<ObjectType*>(ob_type)->tp_name << " at 0x"
           << std::hex << static_cast<PyObject*>(this) << ">";
      
      return PyString_FromString(ostr.str().c_str());
    }

    //
    // Functions
    //
    bool
    el_based_object(PyObject* obj) throw()
    {
      if(obj == 0)
      {
        return false;
      }

      PyTypeObject* type = obj->ob_type;

      if(type && type->tp_methods)
      {
        for(PyMethodDef* p = type->tp_methods; p->ml_name != 0 &&
              *p->ml_name != '\0'; ++p)
        {
          if(strcmp(p->ml_name, "__el_based_object__") == 0)
          {
            return true;
          }
        }
      }

      return false;
    }

    std::ostream&
    print(std::ostream& ostr, const PyObject* obj, const char* text_on_failure)
      throw(El::Exception)
    {
      if(obj != 0)
      {
        El::Python::Object_var str_obj =
          PyObject_Str(const_cast<PyObject*>(obj));

        if(str_obj.in() != 0 && PyString_Check(str_obj))
        {
          ostr << PyString_AsString(str_obj);
          return ostr;
        }
      }

      if(text_on_failure)
      {
        ostr << text_on_failure;
      }
      
      return ostr;
    }    
    
    void
    set_error(El::Exception& e) throw()
    {
      El::Python::ErrorPropogation* pe =
        dynamic_cast<El::Python::ErrorPropogation*>(&e);

      if(pe)
      {
        Object_var value = pe->value;
        
        if(!pe->context.empty())
        {
          std::ostringstream ostr;
          print(ostr, value.in());
          ostr << " (" << pe->context << ")";

          value = Py_BuildValue("s", ostr.str().c_str());
        }
          
        PyErr_Restore(pe->type.add_ref(),
                      value.add_ref(),
                      pe->traceback.add_ref());
        return;
      }

      set_runtime_error(e.what());      
    }
    
    void
    handle_error(const char* context, const char* suffix)
      throw(Exception, El::Exception)
    {
      if(PyErr_Occurred() == 0)
      {
        return;
      }
        
      Object_var type, value, traceback;
    
      PyErr_Fetch(type.out(), value.out(), traceback.out());
      PyErr_Clear();

      if(type.in() == PyExc_SystemExit)
      {
        throw SystemExit(value.in());
      }

      std::ostringstream ostr;

      if(context)
      {
        ostr << context << ":" << std::endl;
      }
      else
      {
        ostr << "El::Python::handle_error:" << std::endl;
      }
        
      ostr << "error type: ";
      
      print(ostr, type.in(), "unknown") << std::endl;
    
      ostr << "error: ";
      print(ostr, value.in());

      if(traceback.in() && PyTraceBack_Check(traceback.in()))
      {
        ostr << std::endl << "traceback:";
        
        for(PyTracebackObject* tb =
              reinterpret_cast<PyTracebackObject*>(traceback.in());
            tb != 0; tb = tb->tb_next)
        {
          ostr << std::endl << "  " << tb->tb_lineno << ":"
               << PyString_AsString(tb->tb_frame->f_code->co_name) << " "
               << PyString_AsString(tb->tb_frame->f_code->co_filename);
        }
      }

//      print(ostr, traceback.in());

      if(suffix)
      {
        ostr << suffix;
      }
    
      throw ErrorPropogation(ostr.str().c_str(),
                             context,
                             type.in(),
                             value.in(),
                             traceback.in());
    }

    void
    report_error(PyObject* type,
                 const char* description,
                 const char* context,
                 const char* suffix)
      throw(Exception, El::Exception)
    {
      PyErr_SetString(type, description); 
      handle_error(context, suffix);
    }    
  }
}

