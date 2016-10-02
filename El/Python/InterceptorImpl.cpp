/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/InterceptorImpl.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>
#include <pthread.h>

#include <iostream>
#include <sstream>

#include "Utility.hpp"
#include "InterceptorImpl.hpp"

namespace
{
  bool in_dlsym = false;

  template <typename T> 
  inline
  bool
  real_mem_func(T& func, const char* name) throw()
  {
    if(func == 0)
    {
      if(in_dlsym)
      {
        return false;
      }
      
      in_dlsym = true;
      func = reinterpret_cast<T>(dlsym(RTLD_NEXT, name));
      in_dlsym = false;
        
      char* error = dlerror();
      
      if(error != 0)
      {
        fputs(error, stderr);
        abort();
      }
    }

    return true;
  }
  
  typedef void* (*MallocFunc)(size_t size);
  MallocFunc real_malloc = 0;
  
  typedef void* (*VallocFunc)(size_t size);
  VallocFunc real_valloc = 0;
  
  typedef void (*FreeFunc)(void *ptr);
  FreeFunc real_free = 0;

  typedef void* (*CallocFunc)(size_t nmemb, size_t size);
  CallocFunc real_calloc = 0;

  typedef void* (*ReallocFunc)(void *ptr, size_t size);
  ReallocFunc real_realloc = 0;

  typedef void* (*MemalignFunc)(size_t boundary, size_t size);
  MemalignFunc real_memalign = 0;

  typedef void* (*MMapFunc)(void *start, size_t length, int prot, int flags,
                            int fd, off_t offset);
  MMapFunc real_mmap = 0;
  MMapFunc real_mmap2 = 0;

  typedef int (*MUnmapFunc)(void *start, size_t length);
  MUnmapFunc real_munmap = 0;
}

extern "C" int
munmap(void *start, size_t length)
{
  real_mem_func(real_munmap, "munmap");
  int res = real_munmap(start, length);
  El::Python::InterceptorImpl::count_free(start);
  return res;
}
  
extern "C" void*
mmap2(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  return real_mem_func(real_mmap2, "mmap2") ?
    El::Python::InterceptorImpl::count_alloc(real_mmap2(start,
                                                        length,
                                                        prot,
                                                        flags,
                                                        fd,
                                                        offset),
                                             length) :
    0;
}

extern "C" void*
mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
  return real_mem_func(real_mmap, "mmap") ?
    El::Python::InterceptorImpl::count_alloc(real_mmap(start,
                                                       length,
                                                       prot,
                                                       flags,
                                                       fd,
                                                       offset),
                                             length) :
    0;
}

extern "C" void*
realloc(void* ptr, size_t size)
{
  return real_mem_func(real_realloc, "realloc") ?
    El::Python::InterceptorImpl::count_alloc(real_realloc(ptr, size),
                                             size,
                                             ptr) :
    0;
}

extern "C" void*
malloc(size_t size)
{
  return real_mem_func(real_malloc, "malloc") ?
    El::Python::InterceptorImpl::count_alloc(real_malloc(size), size) : 0;
}

extern "C" void*
valloc(size_t size)
{
  return real_mem_func(real_valloc, "valloc") ?
    El::Python::InterceptorImpl::count_alloc(real_valloc(size), size) : 0;
}

extern "C" void
free(void* ptr)
{
  real_mem_func(real_free, "free");
  real_free(ptr);
  El::Python::InterceptorImpl::count_free(ptr);
}

extern "C" void*
calloc(size_t nmemb, size_t size)
{
  return real_mem_func(real_calloc, "calloc") ?
    El::Python::InterceptorImpl::count_alloc(
      real_calloc(nmemb, size), nmemb * size) : 0;
}

extern "C" void*
memalign(size_t boundary, size_t size)
{
  return real_mem_func(real_memalign, "memalign") ?
    El::Python::InterceptorImpl::count_alloc(
      real_memalign(boundary, size), size) : 0;
}

namespace El
{
  namespace Python
  {
    bool InterceptorImpl::mem_alloc_intercept_ = false;
    InterceptorImpl InterceptorImpl::instance;
    
    PyCFunction InterceptorImpl::builtin_import_def_func_ = 0;

    InterceptorImpl::InterceptorImpl()
      throw(Exception, El::Exception)
        : flags_(0),
          mem_counting_(true)
    {
      memset(&hook_def_builtin_import_, 0, sizeof(hook_def_builtin_import_));

      // Initialize all mem hooks here until there is a single main thread
      real_mem_func(real_calloc, "calloc");
      real_mem_func(real_malloc, "malloc");
      real_mem_func(real_free, "free");
      real_mem_func(real_realloc, "realloc");
      real_mem_func(real_valloc, "valloc");
      real_mem_func(real_memalign, "memalign");
      real_mem_func(real_mmap, "mmap");
      real_mem_func(real_mmap2, "mmap2");
      real_mem_func(real_munmap, "munmap");

      srand(time(0));
    }

    void
    InterceptorImpl::on_install(unsigned long flags)
      throw(Exception, El::Exception)
    {
      Guard guard(lock_);
        
      mem_counting_ = false;
      flags_ = flags;

      if(flags & IF_IMPORT)
      {
        instance.register_import_hook();
      }

      if(flags & IF_MEM_COUNTING)
      {
        mem_alloc_intercept_ = true;
      }
        
      mem_counting_ = true;
    }

    void
    InterceptorImpl::on_uninstall() throw()
    {
      Guard guard(lock_);
      mem_counting_ = false;
      
      thread_infos_.clear();
      mem_threads_.clear();
      mem_alloc_intercept_ = false;
      flags_ = 0;
      
      mem_counting_ = true;
    }
    
    void
    InterceptorImpl::install(unsigned long flags)
      throw(Exception, El::Exception)
    {
      if(flags)
      {
        instance.on_install(flags);
        interceptor(&instance);
      }
      else
      {
        uninstall();
      }
    }
      
    void
    InterceptorImpl::uninstall() throw()
    {
      interceptor(0);
      instance.on_uninstall();      
    }

    PyObject*
    InterceptorImpl::get_builtins_dict() throw(Exception, El::Exception)
    {
      PyObject* module = PyImport_AddModule("__main__");
      
      if(module == 0)
      {
        El::Python::handle_error(
          "El::Python::InterceptorImpl::get_builtins_dict: "
          "PyImport_AddModule failed");
      }
        
      PyObject* main_dict = PyModule_GetDict(module);

      if(main_dict == 0)
      {
        throw Exception("InterceptorImpl::get_builtins_dict: "
                        "main module dict is absent");
      }
      
      PyObject* builtins = PyDict_GetItemString(main_dict, "__builtins__");
        
      if(builtins == 0)
      {
        throw Exception("InterceptorImpl::get_builtins_dict: "
                        "__builtins__ module is absent");
      }
      
      PyObject* builtins_dict = PyModule_GetDict(builtins);

      if(builtins_dict == 0)
      {
        throw Exception("InterceptorImpl::get_builtins_dict: "
                        "__builtins__ module dict is absent");
      }
        
      return builtins_dict;
    }
  
    int
    InterceptorImpl::trace_hook(PyObject *obj,
                                PyFrameObject *frame,
                                int what,
                                PyObject *arg) throw()
    {
      try
      {        
        ThreadInfo info;
        
        if(instance.thread_info(info) && (info.flags & IF_TRACE))
        {
          info.callback->check_integrity();
          
          info.callback->trace(frame,
                               what,
                               arg,
                               info.import_level,
                               info.allocated);
        }        
      }
      catch(const StopExecution& e)
      {
        Interruption_var obj = new Interruption();
        obj->reason = e.what();
          
        PyErr_SetObject(PyExc_SystemExit, obj.in());
        instance.thread_callback(0, IF_ALL);
        return -1;
      }
      catch(const El::Exception& e)
      {
        El::Python::set_runtime_error(e.what());
        return -1;
      }
    
      return 0;
    }

    PyObject*
    InterceptorImpl::builtin_import_hook(PyObject *self, PyObject *args)
      throw()
    {
      // Look import_module_ex (Python/import.c) for reference
      char* name = 0;
      PyObject* globals = NULL;
      PyObject* locals = NULL;
      PyObject* fromlist = NULL;
      StringPtrArray from_list;
      
      try
      {
        ThreadInfo info;
        bool imp_level_dec = false;
        
        if(instance.thread_info(info) && (info.flags & IF_IMPORT))
        {          
          info.callback->check_integrity();
          
          if(PyArg_ParseTuple(
               args,
               "s|OOO:El::Python::InterceptorImpl::builtin_import_hook",
               &name,
               &globals,
               &locals,
               &fromlist))
          {          
            if(fromlist && PySequence_Check(fromlist))
            {
              int size = PySequence_Size(fromlist);
              from_list.reserve(size);
            
              for(int i = 0; i < size; ++i)
              {
                Object_var item = PySequence_GetItem(fromlist, i);
              
                if(PyString_Check(item))
                {
                  const char* str = PyString_AS_STRING(item.in());
                  
                  if(str[0] == '*')
                  {
// TODO: expand *                    
//                Object_var all = PyObject_GetAttrString(mod, "__all__");
                  }
                  
                  from_list.push_back(str);
                }
              }
            }

            info.callback->import(name,
                                  from_list,
                                  info.import_level);
            
            instance.update_thread_import_level(true);
            imp_level_dec = true;
          }
        }
      
        PyObject* res = (*builtin_import_def_func_)(self, args);

        if(imp_level_dec)
        {
          instance.update_thread_import_level(false);

          info.callback->check_integrity();
          
          if(res)
          {
            info.callback->imported(name,
                                    globals,
                                    locals,
                                    from_list,
                                    info.import_level);
          }
        }

        return res;
      }
      catch(const StopExecution& e)
      {
        Interruption_var obj = new Interruption();
        obj->reason = e.what();
        
        PyErr_SetObject(PyExc_RuntimeError, obj.in());
        instance.thread_callback(0, IF_ALL);
        return 0;
      }
      catch(const El::Exception& e)
      {
        El::Python::set_runtime_error(e.what());
        return 0;
      }
    }

    void
    InterceptorImpl::register_trace_hook(bool on)
      throw(Exception, El::Exception)
    {
      PyEval_SetTrace(on ? trace_hook : 0, 0);
    }
    
    void
    InterceptorImpl::register_import_hook()
      throw(Exception, El::Exception)
    {
      PyObject* builtins_dict = get_builtins_dict();
      PyObject* import = PyDict_GetItemString(builtins_dict, "__import__");
          
      if(import)
      {
        PyTypeObject* type = import->ob_type;
        
        if(type == &PyCFunction_Type)
        {
          PyCFunctionObject* io = (PyCFunctionObject*)import;
          
          assert(io->m_ml);
          hook_def_builtin_import_ = *io->m_ml;
          builtin_import_def_func_ = hook_def_builtin_import_.ml_meth;
          hook_def_builtin_import_.ml_meth = &builtin_import_hook;
          
          El::Python::Object_var import_func =
            PyCFunction_NewEx(&hook_def_builtin_import_,
                              io->m_self,
                              io->m_module);
          
          if(import_func.in() == 0)
          {
            El::Python::handle_error(
              "El::Python::InterceptorImpl::register_import_hook: "
              "PyCFunction_NewEx for import_hook failed");
          }
          
          if(PyDict_SetItemString(builtins_dict,
                                  hook_def_builtin_import_.ml_name,
                                  import_func.in()) != 0)
          {
            El::Python::handle_error(
              "El::Python::InterceptorImpl::register_import_hook: "
              "PyDict_SetItemString for import_hook failed");
          }
        }
      }
    }
  }
}

