/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Module.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Exception.hpp>

#include "Object.hpp"
#include "Module.hpp"
#include "Utility.hpp"

namespace El
{
  namespace Python
  {
    bool Module::initialized_ = false;
    
    void
    Module::init() throw(El::Exception)
    {
      ModuleList& modules = registry();
/*
      for(ModuleList::iterator it = modules.begin(); it != modules.end(); it++)
      {
        Module* module = *it;
        std::cerr << "Module " << module->name() << std::endl;
      }
*/      
      for(ModuleList::iterator it = modules.begin(); it != modules.end(); it++)
      {
        Module* module = *it;
        
        const char* module_name = module->name();
        
        PyObject* m =
          Py_InitModule3(const_cast<char*>(module_name),
                         module->methods(),
                         const_cast<char*>(module->doc()));

        module->py_module_ = m;

        const char* s = strrchr(module->name(), '.');

        if(s)
        {
          std::string parent(module_name, s - module_name);
          const char* mod_short_name = s + 1;

          ModuleList::iterator it2 = modules.begin();
          
          for(; it2 != it && parent != (*it2)->name(); it2++);

          if(it2 != it)
          {
            if(PyObject_SetAttrString((*it2)->py_module_,
                                      const_cast<char*>(mod_short_name),
                                      m) < 0)
            {
              handle_error(
                "El::Python::Module::init: PyObject_SetAttrString failed");
              
            }
          }
        }

        ObjectType::ObjectTypeMap& type_map = ObjectType::registry();

        for(ObjectType::ObjectTypeMap::iterator itt = type_map.begin();
            itt != type_map.end(); itt++)
        {
          ObjectType* type = itt->second;
          const char* tptr = 0;
          
          if(type->tp_name && (tptr = strrchr(type->tp_name, '.')) != 0)
          {
            std::string modname(type->tp_name, tptr - type->tp_name);

            if(modname == module->name())
            {
              type->add_to_module(m);
            }
          }

          type->initialized();
        }

        if(module->import_by_default())
        {
          El::Python::import_module(module->name());
        }
        else
        {
          El::Python::expose_module(m, module->name());
        }

        module->initialized();
      }

      initialized_ = true;
    }
    
    void
    Module::terminate() throw(El::Exception)
    {
      registry().clear();
      initialized_ = false;
    }
    
    Module::ModuleList&
    Module::registry() throw(El::Exception)
    {
      static ModuleList modules;
      return modules;
    }
    
    void
    Module::add_method(PyCFunction func,
                       int flags,
                       const char* name,
                       const char* doc)
      throw(El::Exception)
    {
      PyMethodDef* mdef = methods_;
      unsigned long i = 0;
    
      for(; i < method_count_ && mdef->ml_name != 0; i++, mdef++);

      if(i == method_count_)
      {
        unsigned long new_method_count = method_count_ + 3;

        // Additional record is sentinel record
        PyMethodDef* new_methods = new PyMethodDef[new_method_count + 1];

        memcpy(new_methods,
               methods_,
               method_count_ * sizeof(PyMethodDef));

        memset(new_methods + method_count_,
               0,
               sizeof(PyMethodDef) * (new_method_count - method_count_ + 1));

        delete [] methods_;
        
        methods_ = new_methods;
        mdef = methods_ + method_count_;
        
        method_count_ = new_method_count;
      }

      mdef->ml_name = El::String::Manip::duplicate(name);
      mdef->ml_meth = func;
      mdef->ml_flags = flags;
      mdef->ml_doc = El::String::Manip::duplicate(doc);
    }

    PyObject*
    Module::create_exception(const char* name,
                             PyObject* base,
                             PyObject* dict)
      throw(Exception, El::Exception)
    {
      if(py_module_ == 0)
      {
        throw Exception("El::Python::Module::create_exception: module "
                        "not initialized");
      }

      std::string full_name = name_ + "." + name;
      
      El::Python::Object_var exception =
        PyErr_NewException(const_cast<char*>(full_name.c_str()), base, dict);

      if(PyModule_AddObject(py_module_,
                            const_cast<char*>(name),
                            exception.in()) < 0)
      {
        El::Python::handle_error("El::Python::Module::create_exception");
      }

      exception.add_ref();
      return exception.retn();
    }
    
    void
    Module::add_member(PyObject* object,
                       const char* name,
                       bool own_ref)
      throw(Exception, El::Exception)
    {
      if(py_module_ == 0)
      {
        throw Exception("El::Python::Module::add_member: module "
                        "not initialized");
      }

      PyObject* dict = PyModule_GetDict(py_module_);
      
      if(PyDict_SetItemString(dict, name, object))
      {
        El::Python::handle_error(
          "El::Python::Module::add_member: PyDict_SetItemString failed");
      }

      if(own_ref)
      {
        remove_ref(object);
      }
    }
    
    const Module*
    Module::find(const char* name) throw(El::Exception)
    {
      const ModuleList& modules = registry();

      for(ModuleList::const_iterator it = modules.begin();
          it != modules.end(); it++)
      {
        const Module* module = *it;
        if(strcmp(module->name(), name) == 0)
        {
          return module;
        }
      }

      return 0;
    }

    PyCFunction
    Module::find_function(const char* name) throw(El::Exception)
    {
      const char* fname = strrchr(name, '.');

      if(fname)
      {        
        std::string modname(name, fname++ - name);

        const Module* mod = find(modname.c_str());

        if(mod)
        {
          PyMethodDef* mdef = mod->methods_;
          
          for(; mdef->ml_name != 0 && strcmp(mdef->ml_name, fname); mdef++);
          return mdef->ml_meth;
        }
      }

      return 0;
    }
    
  }
}
