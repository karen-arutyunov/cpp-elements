/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Code.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_CODE_HPP_
#define _ELEMENTS_EL_PYTHON_CODE_HPP_

#include <Python.h>

#include <string>
#include <iostream>

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Sandbox.hpp>

namespace El
{
  namespace Python
  {
    class Code
    {      
    public:
      Code() throw(El::Exception) {}
      
      Code(const char* text,
           const char* name = 0)
        throw(InvalidArg, Exception, El::Exception);

      Code(const Code& src) throw(El::Exception);

      ~Code() throw();

      Code& operator=(const Code& src) throw(El::Exception);

      void compile(const char* text,
                   const char* name = 0)
        throw(InvalidArg, Exception, El::Exception);

      PyObject* run(PyObject* global_dictionary = 0,
                    PyObject* local_dictionary = 0,
                    Sandbox* sandbox = 0,
                    std::ostream* log = 0) const
        throw(CodeNotCompiled,
              ExecutionInterrupted,
              ErrorPropogation,
              Exception,
              El::Exception);

      void clear() throw();
      bool compiled() const throw() { return code_.in() != 0; }

      PyCodeObject* code() const throw() { return code_.in(); }

      void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      void read(El::BinaryInStream& bstr) throw(El::Exception);

    private:
      void set_global_dict() throw(Exception, El::Exception);

    protected:
      std::string text_;
      std::string name_;
      Object_var global_dictionary_;
      CodeObject_var code_;
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
    Code::Code(const char* text,
               const char* name)
      throw(InvalidArg, Exception, El::Exception)
    {
      compile(text, name);
    }

    inline
    Code::~Code() throw()
    {
      clear();
    }

    inline
    void
    Code::clear() throw()
    {
      text_.empty();
      name_.empty();
      global_dictionary_ = 0;
      code_ = 0;
    }

  }
}

#endif // _ELEMENTS_EL_PYTHON_CODE_HPP_
