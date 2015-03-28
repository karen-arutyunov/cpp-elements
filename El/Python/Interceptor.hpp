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
 * @file Elements/El/Python/Interceptor.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_INTERCEPTOR_HPP_
#define _ELEMENTS_EL_PYTHON_INTERCEPTOR_HPP_

#include <vector>

#include <Python.h>
#include <frameobject.h>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>

namespace El
{
  namespace Python
  {
    class Interceptor
    {
      friend class Code;

    public:
      EL_EXCEPTION(StopExecution, Exception);
      
      enum InterceptionFlags
      {
        IF_IMPORT = 0x1,
        IF_TRACE = 0x2,
        IF_MEM_COUNTING = 0x4,
        IF_ALL = IF_IMPORT | IF_TRACE | IF_MEM_COUNTING
      };

      typedef std::vector<const char*> StringPtrArray;

      class Callback
      {
      public:

        virtual ~Callback() throw() {}
        
        virtual void import(const char* module_name,
                            const StringPtrArray& from_list,
                            size_t import_level)
          throw(StopExecution, El::Exception) = 0;

        virtual void imported(const char* module_name,
                              PyObject *globals,
                              PyObject *locals,
                              const StringPtrArray& from_list,
                              size_t import_level)
          throw(StopExecution, El::Exception) = 0;

        virtual void trace(PyFrameObject *frame,
                           int what,
                           PyObject *arg,
                           size_t import_level,
                           size_t mem_allocated)
          throw(StopExecution, El::Exception) = 0;

        virtual void check_integrity() throw(StopExecution, El::Exception) = 0;
      };

      virtual ~Interceptor() throw() {}
      
      virtual void thread_callback(Callback* callback, unsigned long flags)
        throw() = 0;

      virtual Callback* thread_callback() const throw() = 0;

      class Interruption : public El::Python::ObjectImpl
      {
      public:
        
        std::string reason;
        
        Interruption(PyTypeObject *type = 0,
                     PyObject *args = 0,
                     PyObject *kwds = 0)
          throw(El::Exception);
        
        virtual ~Interruption() throw() {}

        virtual PyObject* str() throw(Exception, El::Exception);
        
        class Type :
          public El::Python::ObjectTypeImpl<Interruption, Interruption::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
        
          static Type instance;        
        };
      };

      typedef SmartPtr<Interruption> Interruption_var;      
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
    //
    // Interceptor::Interruption class
    //
    inline
    Interceptor::Interruption::Interruption(PyTypeObject *type,
                                            PyObject *args,
                                            PyObject *kwds)
      throw(El::Exception)
        : ObjectImpl(type ? type : &Type::instance)
    {
    }

    inline
    PyObject*
    Interceptor::Interruption::str() throw(Exception, El::Exception)
    {
      return PyString_FromString(reason.c_str());
    }
    
    //
    // Interceptor::Interruption::Type class
    //
    inline
    Interceptor::Interruption::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Interruption, Interruption::Type>(
          "el.InterceptorInterruption",
          "Object representing script execution interruption event")
    {
    }

  }
}

#endif // _ELEMENTS_EL_PYTHON_INTERCEPTOR_HPP_
