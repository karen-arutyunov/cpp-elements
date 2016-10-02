/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Code.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <sstream>
#include <string>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/Python/Utility.hpp>

#include "RefCount.hpp"
#include "Code.hpp"
#include "Interceptor.hpp"

namespace El
{
  namespace Python
  {
    Code::Code(const Code& src) throw(El::Exception)
    {
      *this = src;
    }

    Code&
    Code::operator=(const Code& src) throw(El::Exception)
    {
      text_ = src.text_;
      name_ = src.name_;
      global_dictionary_ = src.global_dictionary_;
      code_ = src.code_;
      return *this;
    }

    void
    Code::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      bstr << text_ << name_ << (PyObject*)code_.in();
    }

    void
    Code::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      clear();
      
      El::Python::Object_var code;
      bstr >> text_ >> name_ >> code;
  
      if(code.in())
      {
        if(!PyCode_Check(code.in()))
        {
          throw Exception("El::Python::Code::read: unexpected object type");
        }
        
        code_ = (PyCodeObject*)code.retn();
        set_global_dict();
      }
    }
    
    void
    Code::compile(const char* text, const char* name)
      throw(InvalidArg, Exception, El::Exception)
    {
      clear();
      
      if(text == 0 || *text == '\0')
      {
        throw InvalidArg("El::Python::Code::compile: text is empty or NULL");
      }

      try
      {
//        std::cerr << "CODE:\n'" << text << "'\n";
        
        El::String::Manip::suppress(text, text_, "\r");
        name_ = name && *name != '\0' ? name : "<no name>";

        set_global_dict();
        
        code_ = (PyCodeObject*)Py_CompileString(text_.c_str(),
                                                name_.c_str(),
                                                Py_file_input);
        
        if(code_.in() == 0)
        {
          std::ostringstream ostr;
          ostr << std::endl << "name: " << name_ << std::endl        
               << "code:" << std::endl << text_;
        
          handle_error("El::Python::Code::compile: Py_CompileString failed",
                       ostr.str().c_str());
        }
      }
      catch(...)
      {
        clear();
        throw;
      }
    }

    void
    Code::set_global_dict() throw(Exception, El::Exception)
    {
      PyObject* module = PyImport_AddModule("__main__");
      
      if(module == 0)
      {
        handle_error(
          "El::Python::Code::set_global_dict: PyImport_AddModule failed");
      }

      global_dictionary_ = add_ref(PyModule_GetDict(module));
    }

    PyObject*
    Code::run(PyObject* global_dictionary,
              PyObject* local_dictionary,
              Sandbox* sandbox,
              std::ostream* log) const
      throw(CodeNotCompiled,
            ExecutionInterrupted,
            ErrorPropogation,
            Exception,
            El::Exception)
    {
      if(code_.in() == 0)
      {
        throw CodeNotCompiled("El::Python::Code::run: code not compiled");
      }
        
      Object_var global_dict;

      if(global_dictionary)
      {
        global_dict = add_ref(global_dictionary);
      }
      else
      {
        global_dict = global_dictionary_;
      }

      if(sandbox)
      {
        global_dict = sandbox->enable(global_dict.in());
      }

      Object_var local_dict;

      if(local_dictionary)
      {
        local_dict = add_ref(local_dictionary);
      }
      else
      {
// If locals == 0 then PyFrame_New (being called from PyEval_EvalCode)
// will set it to globals
      }

      if(log)
      {
        *log << "* PyEval_EvalCode pre run *\n";
        dump_dict("Globals", global_dict.in(), *log);
        dump_dict("Locals", local_dict.in(), *log);
      }
      
      PyObject* res =
        PyEval_EvalCode(code_.in(), global_dict.in(), local_dict.in());

      if(sandbox)
      {
        sandbox->disable();
      }      
    
      if(res == 0)
      {
        {
          Object_var type, value, traceback;
    
          PyErr_Fetch(type.out(), value.out(), traceback.out());

          if(type.in() == PyExc_SystemExit)
          {
            if(Interceptor::Interruption::Type::check_type(value.in()))
            {
              Interceptor::Interruption* interruption =
                Interceptor::Interruption::Type::down_cast(value.in());

              std::ostringstream ostr;
              ostr << "El::Python::Code::run: execution interrupted. Reason:\n"
                   << interruption->reason;

              PyErr_Clear();
              throw ExecutionInterrupted(ostr.str());
            }
              
            PyErr_Clear();
            return value.retn();
          }

          PyErr_Restore(type.retn(), value.retn(), traceback.retn());
        }
        
        std::ostringstream ostr;
        ostr << std::endl << "name: " << name_ << std::endl        
             << "code:" << std::endl << text_;
        
        handle_error("El::Python::Code::run: PyEval_EvalCode failed",
                     ostr.str().c_str());
      }

      if(log)
      {
        *log << "* End *\n";
      }
      
      return res;
    }

  }
}

