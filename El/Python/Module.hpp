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
 * @file Elements/El/Python/Module.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_MODULE_HPP_
#define _ELEMENTS_EL_PYTHON_MODULE_HPP_

#include <list>

#include <Python.h>

#include <El/Exception.hpp>

#include <El/Python/Object.hpp>

namespace El
{
  namespace Python
  {
    class Module
    {
    public:      

      Module(const char* name, const char* doc, bool import) // = false)
        throw(El::Exception);

      virtual ~Module() throw();

      static void init() throw(El::Exception);
      static void terminate() throw(El::Exception);

      const char* name() const throw() { return name_.c_str(); }
      const char* doc() const throw() { return doc_.c_str(); }

      PyObject* py_module() const throw() { return py_module_; }
      PyMethodDef* methods() const throw() { return methods_; }

      bool import_by_default() throw() { return import_; }

      virtual void initialized() throw(El::Exception) {}

      static const Module* find(const char* name) throw(El::Exception);
      static PyCFunction find_function(const char* name) throw(El::Exception);

      PyObject* create_exception(const char* name,
                                 PyObject* base = 0,
                                 PyObject* dict = 0)
        throw(Exception, El::Exception);

    protected:
      
      void add_method(PyCFunction func,
                      int flags,
                      const char* name,
                      const char* doc)
        throw(El::Exception);

      void add_member(PyObject* object, const char* name, bool own_ref = true)
        throw(Exception, El::Exception);

    private:

      typedef std::list<Module*> ModuleList;
/*
      struct ModuleList : public std::list<Module*>
      {
        ModuleList() { std::cerr << "ModuleList\n"; }
        ~ModuleList() { std::cerr << "~ModuleList " << size() << "\n"; }        
      };
*/      
      static ModuleList& registry() throw(El::Exception);

    protected:
      std::string name_;
      std::string doc_;
      bool import_;
      PyObject* py_module_;
      PyMethodDef* methods_;
      unsigned long method_count_;
      
      static bool initialized_;
    };

    template<typename MODULE_CLASS>
    class ModuleImpl : public Module
    {
    public:
      ModuleImpl(const char* name, const char* doc, bool import) // = false
        throw(El::Exception);

      ~ModuleImpl() throw();

    public:

      typedef MODULE_CLASS ModuleClass;

      typedef const char* (*Str)();
      typedef PyObject* (ModuleClass::* NoArgsMethod)();      
      typedef PyObject* (ModuleClass::* VarArgsMethod)(PyObject* args);
      
      typedef PyObject* (ModuleClass::* KwdsMethod)(PyObject* args,
                                                    PyObject* kwds);

      static PyObject* disp_noargs(NoArgsMethod method) throw();

      static PyObject* disp_varargs(VarArgsMethod method,
                                    PyObject* args) throw();
      
      static PyObject* disp_kwds(KwdsMethod method,
                                 PyObject* args,
                                 PyObject* kwds) throw();
      
      template <typename MODULE_CLASS_,
                typename FUNC_TYPE,
                FUNC_TYPE func,
                int flags,
                Str nm,
                Str dc>
      struct MethodUpdater
      {
        MethodUpdater() throw()
        {
          MODULE_CLASS_::instance.add_method(
            reinterpret_cast<PyCFunction>(func),
            flags,
            (*nm)(),
            (*dc)());
        }
      };
    };
    
  }
}

#define PY_MODULE_METHOD_DEF_(func_type, method, flags, py_name, py_doc)  \
  static const char* method##_name__() { return py_name; } \
  static const char* method##_doc__() { return py_doc; } \
  MethodUpdater<ModuleClass, func_type, method##_disp__, flags, \
                method##_name__, method##_doc__> \
  method##_upd__;

#define PY_MODULE_METHOD_NOARGS(method, py_name, py_doc) \
  static PyObject* method##_disp__(PyObject* self)          \
  { \
    return disp_noargs(&ModuleClass::method); \
  } \
  PY_MODULE_METHOD_DEF_(PyNoArgsFunction, method, METH_NOARGS, py_name, py_doc)

#define PY_MODULE_METHOD_VARARGS(method, py_name, py_doc) \
  static PyObject* method##_disp__(PyObject* self, PyObject* args)           \
  { \
    return disp_varargs(&ModuleClass::method, args); \
  } \
  PY_MODULE_METHOD_DEF_(PyCFunction, method, METH_VARARGS, py_name, py_doc)

#define PY_MODULE_METHOD_KWDS(method, py_name, py_doc) \
  static PyObject* method##_disp__(PyObject* self, PyObject* args, \
                                   PyObject* kwds)                 \
  { \
    return disp_kwds(&ModuleClass::method, args, kwds); \
  } \
  PY_MODULE_METHOD_DEF_(PyCFunctionWithKeywords, method, \
                        METH_VARARGS | METH_KEYWORDS, py_name, py_doc)

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    //
    // El::Python::Module class
    //
    inline
    Module::Module(const char* name, const char* doc, bool import)
      throw(El::Exception)
        : name_(name),
          doc_(doc),
          import_(import),
          py_module_(0),
          methods_(0),
          method_count_(0)
    {
      ModuleList& reg = registry();
      ModuleList::iterator it = reg.begin();
/*
      std::cerr << std::dec << "reg: " << name << "; " << reg.size()
                << std::endl;
*/    
      for(; it != reg.end() && strcmp((*it)->name(), name) < 0; it++);

//      std::cerr << "inserting: " << name << std::endl;

      reg.insert(it, this);
    }    

    inline
    Module::~Module() throw()
    {
      if(initialized_)
      {      
// No sense to remove as soon as framework is not stay in consistent way
// if some objects deregistered before application terminate
        registry().clear();
      }
      else
      {
        registry().remove(this);
      }
      
//      std::cerr << "deleting " << name_ << " (" << initialized_ << ")\n";
      
      if(methods_)
      {
        PyMethodDef* mdef = methods_;
      
        for(unsigned long i = 0;
            i < method_count_ && mdef->ml_name != 0; i++, mdef++)
        {
          delete [] mdef->ml_name;
          delete [] mdef->ml_doc;
        }
      
        delete [] methods_;
      }
    }
    
    //
    // El::Python::ModuleImpl class
    //
    template<typename MODULE_CLASS>
    ModuleImpl<MODULE_CLASS>::ModuleImpl(const char* name,
                                         const char* doc,
                                         bool import)
      throw(El::Exception)
        : Module(name, doc, import)
          
    {
    }
    
    template<typename MODULE_CLASS>
    ModuleImpl<MODULE_CLASS>::~ModuleImpl() throw()
    {
    }
    
    template<typename MODULE_CLASS>
    PyObject*
    ModuleImpl<MODULE_CLASS>::disp_noargs(NoArgsMethod method) throw()
    {
      try
      {
        return ((&MODULE_CLASS::instance)->*method)();
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error(
          "El::Python::ObjectTypeImpl::disp_noarg: unknown error");
      }

      return 0;      
    }

    template<typename MODULE_CLASS>
    PyObject*
    ModuleImpl<MODULE_CLASS>::disp_varargs(
      VarArgsMethod method,
      PyObject* args) throw()
    {
      try
      {
        return ((&MODULE_CLASS::instance)->*method)(args);
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error(
          "El::Python::ObjectTypeImpl::disp_varargs: unknown error");
      }

      return 0;      
    }
    
    template<typename MODULE_CLASS>
    PyObject*
    ModuleImpl<MODULE_CLASS>::disp_kwds(KwdsMethod method,
                                        PyObject* args,
                                        PyObject* kwds) throw()
    {
      try
      {
        return ((&MODULE_CLASS::instance)->*method)(args, kwds);
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error(
          "El::Python::ObjectTypeImpl::disp_kwds: unknown error");
      }

      return 0;      
    }
    
  }
}

#endif // _ELEMENTS_EL_PYTHON_MODULE_HPP_
