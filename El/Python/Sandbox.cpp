/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Sandbox.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <iostream>
#include <sstream>
#include <string>

#include <El/Exception.hpp>
#include <El/Moment.hpp>
#include <El/String/Manip.hpp>
#include <El/String/ListParser.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Interceptor.hpp>
#include <El/Python/Utility.hpp>

#include "Sandbox.hpp"

namespace El
{
  namespace Python
  {
    const char Sandbox::SAFE_BUILTINS[] =
      "abs all any ascii basestring bin bool buffer bytearray bytes callable "
      "chr classmethod cmp coerce complex delattr dict dir divmod enumerate "
      "filter float format frozenset getattr globals hasattr hash hex id "
      "int intern isinstance issubclass iter len list long map max "
      "memoryview min next object oct ord pow property range reduce repr "
      "reversed round set setattr slice sorted staticmethod str sum super "
      "tuple type unichr unicode xrange zip"
      "ArithmeticError AssertionError AttributeError DeprecationWarning "
      "Ellipsis EnvironmentError EOFError Exception False FloatingPointError "
      "FutureWarning ImportError IndentationError IndexError IOError "
      "KeyboardInterrupt KeyError LookupError MemoryError NameError None "
      "NotImplemented NotImplementedError OSError OverflowError "
      "OverflowWarning PendingDeprecationWarning ReferenceError RuntimeError "
      "RuntimeWarning StandardError StopIteration SyntaxError SyntaxWarning "
      "SystemError SystemExit TabError True TypeError UnboundLocalError "
      "UnicodeDecodeError UnicodeEncodeError UnicodeError "
      "UnicodeTranslateError UserWarning ValueError Warning ZeroDivisionError "
      "__name__ __doc__ __debug__";
    
    const char Sandbox::SAFE_MODULES[] = "math _random";

    Sandbox::Sandbox(const char* safe_modules,
                     const char* safe_builtins,
                     size_t max_ticks,
                     const ACE_Time_Value& timeout,
                     size_t call_max_depth,
                     size_t max_mem,
                     Callback* callback)
      throw(Exception, El::Exception)
        : max_ticks_(max_ticks),
          timeout_(timeout),
          call_max_depth_(call_max_depth),
          max_mem_(max_mem),
          callback_(callback),
          enabled_(false),
          tick_(0)
    {
      {
        El::String::ListParser parser(safe_modules);
        const char* item = 0;
      
        while((item = parser.next_item()) != 0)
        {
          safe_modules_.insert(item);
        }
      }

      {
        El::String::ListParser parser(safe_builtins);
        const char* item = 0;
      
        while((item = parser.next_item()) != 0)
        {
          safe_builtins_.insert(item);
        }
      }      
    }

    Sandbox::~Sandbox() throw()
    {
      try
      {
        disable();
      }
      catch(const El::Exception&)
      {
      }
    }

    PyObject*
    Sandbox::enable(PyObject* global_dict) throw(El::Exception)
    {
      disable();

      if(global_dict == 0)
      {
        throw Exception("El::Python::Sandbox::enable: global dictionary is "
                        "missed");
      }

      Object_var res = PyDict_New();

      if(res.in() == 0)
      {
        El::Python::handle_error(
          "El::Python::Sandbox::enable: PyDict_New failed");
      }

      if(PyDict_Merge(res.in(), global_dict, 0) < 0)
      {
        El::Python::handle_error(
          "El::Python::Sandbox::enable: PyDict_Merge failed (1)");
      }

      for(StringSet::const_iterator i(safe_modules_.begin()),
            e(safe_modules_.end()); i != e; ++i)
      {
        char* name = const_cast<char*>(i->c_str());

        El::Python::Object_var mod = PyImport_ImportModule(name);
        
        if(mod.in() == 0)
        {
          El::Python::handle_error(
            "El::Python::Sandbox::enable: PyImport_ImportModule failed");
        }

        if(PyDict_SetItemString(res.in(), name, mod.in()) != 0)
        {
          El::Python::handle_error(
            "El::Python::Sandbox::enable: PyDict_SetItemString failed (1)");
        }
      }
        
//      int pos = 0;
      Py_ssize_t pos = 0;
      PyObject *key, *value;
      
      while(PyDict_Next(res.in(), &pos, &key, &value))
      {
        El::Python::Object_var key_obj =
          El::Python::string_from_object(key);
          
        size_t len = 0;
        const char* k = El::Python::string_from_string(key_obj.in(), len);

        globals_snapshot_[k] = value;
          
        if(strcmp(k, "__builtins__") == 0)
        {  
          StringSet unsafe_builtins;
          PyObject* builtins_dict = PyModule_GetDict(value);

          El::Python::Object_var new_builtins = PyModule_New("__builtins__");
          PyObject* new_builtins_dict = PyModule_GetDict(new_builtins.in());

          if(PyDict_Merge(new_builtins_dict, builtins_dict, 0) < 0)
          {
            El::Python::handle_error(
              "El::Python::Sandbox::enable: PyDict_Merge failed (2)");
          }
            
          if(PyDict_SetItem(res.in(), key, new_builtins.in()) < 0)
          {
            El::Python::handle_error(
              "El::Python::Sandbox::enable: PyDict_SetItem failed (2)");
          }

          globals_snapshot_[k] = new_builtins.in();
            
//          int pos = 0;
          Py_ssize_t pos = 0;
          PyObject *key, *value;
            
          while(PyDict_Next(new_builtins_dict, &pos, &key, &value))
          {
            El::Python::Object_var key_obj =
              El::Python::string_from_object(key);
          
            len = 0;
              
            const char* k =
              El::Python::string_from_string(key_obj.in(), len);
              
            if(safe_builtins_.find(k) == safe_builtins_.end())
            {
              unsafe_builtins.insert(k);
            }
            else
            {
              builtins_snapshot_[k] = value;
            }
          }

          for(StringSet::const_iterator i(unsafe_builtins.begin()),
                e(unsafe_builtins.end()); i != e; ++i)
          {
            PyDict_DelItemString(new_builtins_dict,
                                 const_cast<char*>(i->c_str()));
          }
        }
      }

      global_dict_ = res;

      tick_ = 0;
      call_stack_ = CallStack();
//      prolongation_ = ACE_Time_Value::zero;      
      
      if(timeout_ != ACE_Time_Value::zero)
      {
        end_ = ACE_OS::gettimeofday() + timeout_;
      }

      El::Python::interceptor()->thread_callback(this, INTERCEPT_FLAGS);
      enabled_ = true;

//      std::cerr << "Sandbox::enable\n";
      
      return res.retn();
    }

    void
    Sandbox::check_integrity() throw(Interceptor::StopExecution, El::Exception)
    {
      if(global_dict_.in() == 0)
      {
        throw Interceptor::StopExecution(
          "El::Python::Sandbox::check_integrity: no globals found");
      }

//      int pos = 0;
      Py_ssize_t pos = 0;
      PyObject *key, *value;
      ObjectMap current_snapshot;
      
      while(PyDict_Next(global_dict_.in(), &pos, &key, &value))
      {
        El::Python::Object_var key_obj = El::Python::string_from_object(key);
          
        size_t len = 0;
        const char* k = El::Python::string_from_string(key_obj.in(), len);
        current_snapshot[k] = value;
      }
      
      for(ObjectMap::const_iterator i(globals_snapshot_.begin()),
            e(globals_snapshot_.end()); i != e; ++i)
      {
        ObjectMap::const_iterator it = current_snapshot.find(i->first);
        
        if(it == current_snapshot.end())
        {
          std::ostringstream ostr;
          ostr << "El::Python::Sandbox::check_integrity: global value '"
               << i->first << "' is removed";
            
          throw Interceptor::StopExecution(ostr.str());          
        }

        if(it->second != i->second)
        {
          std::ostringstream ostr;
          ostr << "El::Python::Sandbox::check_integrity: global value '"
               << i->first << "' is substituted";
            
          throw Interceptor::StopExecution(ostr.str());
        }
      }

      PyObject* builtins =
        PyDict_GetItemString(global_dict_.in(), "__builtins__");
      
      if(builtins == 0)
      {
        throw Interceptor::StopExecution(
          "El::Python::Sandbox::check_integrity: no builtins found");
      }

      PyObject* builtins_dict = PyModule_GetDict(builtins);
      
      pos = 0;
      current_snapshot.clear();
      
      while(PyDict_Next(builtins_dict, &pos, &key, &value))
      {
        El::Python::Object_var key_obj = El::Python::string_from_object(key);
          
        size_t len = 0;
        const char* k = El::Python::string_from_string(key_obj.in(), len);

        ObjectMap::const_iterator i = builtins_snapshot_.find(k);

        if(i == builtins_snapshot_.end())
        {
          std::ostringstream ostr;
          ostr << "El::Python::Sandbox::check_integrity: unexpected builtin '"
               << k << "'";
            
          throw Interceptor::StopExecution(ostr.str());
        }

        if(i->second != value)
        {
          std::ostringstream ostr;
          ostr << "El::Python::Sandbox::check_integrity: builtin '"
               << k << "' is substituted";
            
          throw Interceptor::StopExecution(ostr.str());
        }

        current_snapshot[k] = value;
      }

      for(ObjectMap::const_iterator i(builtins_snapshot_.begin()),
            e(builtins_snapshot_.end()); i != e; ++i)
      {
        if(current_snapshot.find(i->first) == current_snapshot.end())
        {
          std::ostringstream ostr;
          ostr << "El::Python::Sandbox::check_integrity: builtin '"
               << i->first << "' is removed";
            
          throw Interceptor::StopExecution(ostr.str());          
        }
      }
    }    
    
    void
    Sandbox::disable() throw(El::Exception)
    {
      if(enabled_)
      {
        globals_snapshot_.clear();
        builtins_snapshot_.clear();
        
        global_dict_ = 0;
        
        El::Python::interceptor()->thread_callback(0, INTERCEPT_FLAGS);      
        enabled_ = false;
/*
        if(prolongation_ != ACE_Time_Value::zero)
        {
          std::cerr << "Sandbox::disable: prolongation "
                    << El::Moment::time(prolongation_) << std::endl;
        }
        else
        {
          std::cerr << "Sandbox::disable\n";
        }
*/
      }
    }

    Sandbox*
    Sandbox::thread_sandboxed() throw()
    {
      try
      {
        Interceptor::Callback* callback =
          El::Python::interceptor()->thread_callback();

        return dynamic_cast<Sandbox*>(callback);
      }
      catch(...)
      {
      }

      return 0;
    }
    
    void
    Sandbox::import(const char* module_name,
                    const Interceptor::StringPtrArray& from_list,
                    size_t import_level)
      throw(Interceptor::StopExecution, El::Exception)
    {
    }

    void
    Sandbox::imported(const char* module_name,
                      PyObject *globals,
                      PyObject *locals,
                      const Interceptor::StringPtrArray& from_list,
                      size_t import_level)
      throw(Interceptor::StopExecution, El::Exception)
    {      
    }

    std::string
    Sandbox::mod_name(const char* file_path) throw(El::Exception)
    {
      if(file_path == 0)
      {
        return "";
      }
      
      PyObject* module = PyImport_AddModule("sys");

      if(module == 0)
      {
        El::Python::handle_error(
          "Sandbox::mod_name: PyImport_AddModule failed");
      }

      El::Python::Object_var paths = PyObject_GetAttrString(module, "path");
      
      if(paths.in() == 0)
      {
        El::Python::handle_error(
          "Sandbox::mod_name: PyObject_GetAttrString failed");
      }
      
      if(!PyList_Check(paths.in()))
      {
        throw Exception("Sandbox::mod_name: sys.path not a list");
      }

      int len = PyList_Size(paths.in());
      for(int i = 0; i < len; ++i)
      {
        PyObject* item = PyList_GetItem(paths.in(), i);

        if(!PyString_Check(item))
        {
          throw Exception("Sandbox::mod_name: sys.path list contain not "
                          "string");
        }

        const char* path = PyString_AsString(item);
        size_t path_len = strlen(path);

        if(path_len && strncmp(file_path, path, path_len) == 0)
        {
          const char* name = file_path + path_len;

          if(path[path_len - 1] != '/')
          {
            if(*name++ != '/')
            {
              continue;
            }
          }

          const char* ext = strrchr(name, '.');

          if(ext == 0 ||
             (strcmp(ext, ".py") && strcmp(ext, ".pyc") &&
              strcmp(ext, ".pyo")))
          {
            continue;
          }

          std::string mod_name(name, ext - name);
          El::String::Manip::replace(mod_name, "/", ".");
          return mod_name;
        }
      }

      return "";
    }

    void
    Sandbox::write_mod_name(std::ostringstream& ostr, const std::string& mod)
      throw(El::Exception)
    {
      if(!mod.empty())
      {
        ostr << "; module '" << mod << "'";
      }
    }
    
    void
    Sandbox::trace(PyFrameObject *frame,
                   int what,
                   PyObject *arg,
                   size_t import_level,
                   size_t mem_allocated)
      throw(Interceptor::StopExecution, El::Exception)
    {
      std::string mod = mod_name(frame);

      if(callback_)
      {
        std::string reason;
        
        if(callback_->interrupt_execution(reason))
        {
          std::ostringstream ostr;
          ostr << "El::Python::Sandbox::trace: execution interrupted";

          if(!reason.empty())
          {
            ostr << "; reason: " << reason;
          }          

          write_mod_name(ostr, mod);
        
          throw Interceptor::StopExecution(ostr.str());          
        }
      }      
      
      if(max_mem_ && mem_allocated > max_mem_)
      {
        std::ostringstream ostr;
        ostr << "El::Python::Sandbox::trace: "
          "allowed memory usage (" << max_mem_ << ") exceeded";

        write_mod_name(ostr, mod);
        
        throw Interceptor::StopExecution(ostr.str());
      }  
      
      if(end_ != ACE_Time_Value::zero)
      {
        if(ACE_OS::gettimeofday() > end_)
        {
          std::ostringstream ostr;
          ostr << "El::Python::Sandbox::trace: "
            "allowed execution time ("
               << El::Moment::time(timeout_) << ") expired";
          
          write_mod_name(ostr, mod);
        
          throw Interceptor::StopExecution(ostr.str());
        }
      }        

      switch(what)
      {
      case PyTrace_CALL:
        {
          if(call_max_depth_ && call_stack_.size() == call_max_depth_)
          {
            std::ostringstream ostr;
            ostr << "El::Python::Sandbox::trace: max call depth ("
                 << call_max_depth_ << ") exceeded";

            write_mod_name(ostr, mod);
            
            throw Interceptor::StopExecution(ostr.str());
          }
            
          call_stack_.push(mod);
          break;
        }
      case PyTrace_RETURN:
        {
          if(call_stack_.empty())
          {
            std::ostringstream ostr;
            ostr << "El::Python::Sandbox::trace: call stack empty";

            write_mod_name(ostr, mod);
            
            throw Exception(ostr.str());
          }

          if(call_stack_.top() != mod)
          {
            std::ostringstream ostr;
            ostr << "El::Python::Sandbox::trace: stack top '"
                 << call_stack_.top() << "' do not match '" << mod
                 << "'";            
            
            throw Exception(ostr.str());
          }
          
          call_stack_.pop();
          break;
        }
      case PyTrace_LINE:
        {
          if(max_ticks_ && tick_++ >= max_ticks_)
          {
            std::ostringstream ostr;          
            ostr << "El::Python::Sandbox::trace: "
              "allowed number of execution ticks (" << max_ticks_
                 << ") exceeded";

            write_mod_name(ostr, mod);
            
            throw Interceptor::StopExecution(ostr.str());
          }
          break;
        }
      }      
    }
  }
}
