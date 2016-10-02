/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Sandbox.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_SANDBOX_HPP_
#define _ELEMENTS_EL_PYTHON_SANDBOX_HPP_

#include <vector>
#include <stack>
#include <ext/hash_set>
#include <ext/hash_map>

#include <Python.h>
#include <frameobject.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Python/Exception.hpp>
#include <El/Python/Interceptor.hpp>

namespace El
{
  namespace Python
  {
    class Sandbox : virtual public Interceptor::Callback
    {
      friend class Code;

    public:

      class Callback
      {
      public:

        virtual ~Callback() throw() {}

        virtual bool interrupt_execution(std::string& reason)
          throw(El::Exception) = 0;
      };
      
    public:
      Sandbox(const char* safe_modules = SAFE_MODULES,
              const char* safe_builtins = SAFE_BUILTINS,
              size_t max_ticks = 0,
              const ACE_Time_Value& timeout = ACE_Time_Value::zero,
              size_t call_max_depth = 0,
              size_t max_mem = 0,
              Callback* callback = 0)
        throw(Exception, El::Exception);
      
      virtual ~Sandbox() throw();
      
      static const unsigned long INTERCEPT_FLAGS =
        Interceptor::IF_TRACE | Interceptor::IF_MEM_COUNTING;

      static const char SAFE_BUILTINS[];
      static const char SAFE_MODULES[];

      static Sandbox* thread_sandboxed() throw();

      void prolong_timeout(const ACE_Time_Value& timeout) throw();

      void write(El::BinaryOutStream& bstr) const throw(El::Exception);  
      void read(El::BinaryInStream& bstr) throw(El::Exception);

      void disable() throw(El::Exception);
      bool enabled() const throw() { return enabled_; }

      const ACE_Time_Value& timeout() const throw() { return timeout_; }
      
    private:
      
      virtual void import(const char* module_name,
                          const Interceptor::StringPtrArray& from_list,
                          size_t import_level)
        throw(Interceptor::StopExecution, El::Exception);

      virtual void imported(const char* module_name,
                            PyObject *globals,
                            PyObject *locals,
                            const Interceptor::StringPtrArray& from_list,
                            size_t import_level)
        throw(Interceptor::StopExecution, El::Exception);
      
      virtual void trace(PyFrameObject *frame,
                         int what,
                         PyObject *arg,
                         size_t import_level,
                         size_t mem_allocated)
        throw(Interceptor::StopExecution, El::Exception);

      virtual void check_integrity()
        throw(Interceptor::StopExecution, El::Exception);
      
      PyObject* enable(PyObject* global_dict) throw(El::Exception);      

      std::string mod_name(const char* file_path) throw(El::Exception);
      std::string mod_name(PyFrameObject *frame) throw(El::Exception);
      
      void write_mod_name(std::ostringstream& ostr, const std::string& mod)
        throw(El::Exception);

    private:
      typedef __gnu_cxx::hash_set<std::string, El::Hash::String> StringSet;
      typedef std::stack<std::string> CallStack;

      size_t max_ticks_;
      ACE_Time_Value timeout_;
      StringSet safe_modules_;
      StringSet safe_builtins_;
      size_t call_max_depth_;
      size_t max_mem_;
      Callback* callback_;

      bool enabled_;
      size_t tick_;
      ACE_Time_Value end_;
//      ACE_Time_Value prolongation_;
      CallStack call_stack_;

      typedef __gnu_cxx::hash_map<std::string,
                                  PyObject*,
                                  El::Hash::String> ObjectMap;
      
      El::Python::Object_var global_dict_;
      ObjectMap globals_snapshot_;
      ObjectMap builtins_snapshot_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    inline
    std::string
    Sandbox::mod_name(PyFrameObject *frame) throw(El::Exception)
    {
      return frame && frame->f_code && frame->f_code->co_filename ?
        mod_name(PyString_AsString(frame->f_code->co_filename)) :
        std::string();
    }

    inline
    void
    Sandbox::prolong_timeout(const ACE_Time_Value& timeout) throw()
    {
      if(end_ != ACE_Time_Value::zero)
      {
        end_ += timeout;
//        prolongation_ += timeout;
      }
    }

    inline
    void
    Sandbox::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      bstr.write_set(safe_modules_);
      bstr.write_set(safe_builtins_);

      bstr << (uint64_t)max_ticks_ << timeout_ << (uint64_t)call_max_depth_
           << (uint64_t)max_mem_;
    }

    inline
    void
    Sandbox::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      disable();
      
      bstr.read_set(safe_modules_);
      bstr.read_set(safe_builtins_);

      uint64_t max_ticks = 0;
      uint64_t call_max_depth = 0;
      uint64_t max_mem = 0;
      
      bstr >> max_ticks >> timeout_ >> call_max_depth >> max_mem;

      max_ticks_ = max_ticks;
      call_max_depth_ = call_max_depth;
      max_mem_ = max_mem;
    }
    
  }
}

#endif // _ELEMENTS_EL_PYTHON_SANDBOX_HPP_
