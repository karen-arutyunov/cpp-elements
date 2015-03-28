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
 * @file Elements/El/Python/Utility.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_UTILITY_HPP_
#define _ELEMENTS_EL_PYTHON_UTILITY_HPP_

#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#include <Python.h>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>
#include <ace/High_Res_Timer.h>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>
#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Sandbox.hpp>

namespace El
{
  namespace Python
  {
    class Interceptor;
    
    Interceptor* interceptor() throw(Exception, El::Exception);
    void interceptor(Interceptor* val) throw();

    class Use
    {
      friend Interceptor* interceptor() throw(Exception, El::Exception);
      friend void interceptor(Interceptor* val) throw();
      
    public :
      
      Use() throw(Exception, El::Exception);
      ~Use() throw();

      static void start() throw(Exception, El::Exception);
      static void stop() throw();

    private:
      typedef ACE_Thread_Mutex Mutex;
      typedef ACE_Guard<Mutex> Guard;
      
      static Mutex lock_;
      static unsigned long counter_;
      static Interceptor* interceptor_;
    };

    struct AllowOtherThreads
    {
      AllowOtherThreads() throw();
      ~AllowOtherThreads() throw() { disallow(); }

      void disallow() throw();

    private:
      PyThreadState* save_;
    };

    struct EnsureCurrentThread
    {
      EnsureCurrentThread() throw();
      ~EnsureCurrentThread() throw() { release(); }

      void release() throw();

    private:
      PyGILState_STATE save_;
      bool ensured_;
    };

    struct ObjectLess
    {
      bool operator()(const PyObject* o1, const PyObject* o2) const;      
    };
    
    void import_module(const char* name) throw(Exception, El::Exception);
    
    void expose_module(PyObject* module, const char* name)
      throw(Exception, El::Exception);

    unsigned long ulong_from_number(PyObject* number, const char* context = 0)
      throw(Exception, El::Exception);

    unsigned long long ulonglong_from_number(PyObject* number,
                                             const char* context = 0)
      throw(Exception, El::Exception);

    const char* string_from_string(PyObject* string,
                                   size_t& len,
                                   const char* context = 0)
      throw(Exception, El::Exception);

    PyObject* string_from_object(PyObject* object, const char* context = 0)
      throw(Exception, El::Exception);

    void dump_module(const char* name,
                     PyObject* module,
                     std::ostream& ostr,
                     const char* ident = "")
      throw(Exception, El::Exception);

    void dump_dict(const char* name,
                   PyObject* dict,
                   std::ostream& ostr,
                   const char* ident = "")
      throw(Exception, El::Exception);    

    template<PyTypeObject* type, typename TYPE_CLASS>
    class TypeAdapter
    {
    public:

      typedef TYPE_CLASS TypeClass;

      static PyObject* down_cast(PyObject* object, bool add_ref = false)
        throw(Exception, El::Exception);

      static bool check_type(PyObject* ob) throw();
      static bool is_type(PyObject* ob) throw();

      const char* tp_name;

      TypeAdapter(const char* name) throw(El::Exception);

    private:
      std::string name_;
    };

    class PyDictType : public El::Python::TypeAdapter<&PyDict_Type, PyDictType>
    {
    public:
      PyDictType() throw(El::Exception);
          
      static PyDictType instance;
    };    
    
    class AnyType
    {
    public:

      static PyObject* down_cast(PyObject* object, bool add_ref = false)
        throw(Exception, El::Exception);

      static bool check_type(PyObject* ob) throw();
      static bool is_type(PyObject* ob) throw();

      const char* tp_name;

      AnyType() throw(El::Exception);

      static AnyType instance;
    };

  }
}

El::BinaryOutStream& operator<<(El::BinaryOutStream& bstr, PyObject* obj)
  throw(El::Exception);

El::BinaryInStream& operator>>(El::BinaryInStream& bstr,
                               El::Python::Object_var& obj)
  throw(El::Exception);

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

template <typename T>
El::BinaryOutStream&
operator<<(El::BinaryOutStream& bstr, El::Python::SmartPtr<T> ptr)
  throw(El::Exception)
{
  bstr << static_cast<PyObject*>(ptr.in());
  return bstr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& bstr, El::Python::ObjectImpl* obj)
  throw(El::Exception)
{
  bstr << static_cast<PyObject*>(obj);
  return bstr;
}

template <typename T>
El::BinaryInStream&
operator>>(El::BinaryInStream& bstr, El::Python::SmartPtr<T>& ptr)
  throw(El::Exception)
{
  El::Python::Object_var obj;
  bstr >> obj;

  ptr = T::Type::down_cast(obj.in(), true);

  return bstr;
}

inline
std::ostream&
operator<<(std::ostream& ostr, const PyObject& obj) throw(El::Exception)
{
  return El::Python::print(ostr, &obj);
}

namespace El
{
  namespace Python
  {
    inline
    Interceptor*
    interceptor() throw(Exception, El::Exception)
    {
      if(Use::interceptor_ == 0)
      {
        throw Exception("El::Python::interceptor: "
                        "interceptor not initialized");        
      }

      return Use::interceptor_;
    }

    //
    // Use class
    //
    inline
    Use::Use() throw(Exception, El::Exception)
    {
      start();
    }
    
    inline
    Use::~Use() throw()
    {
      stop();
    }

    //
    // ObjectLess class
    //
    inline
    bool
    ObjectLess::operator()(const PyObject* o1, const PyObject* o2) const
    {
      if(o1 && o2)
      {
        int res = 0;
          
        if(PyObject_Cmp(const_cast<PyObject*>(o1),
                        const_cast<PyObject*>(o2),
                        &res) < 0)
        {
          handle_error(
            "El::Python::ObjectLess::operator(): PyObject_Cmp failed");
        }

        return res < 0;
      }
      else if(o1 == 0 && o2 == 0)
      {
        return false;
      }

      return o1 == 0;
    }
    
    //
    // AllowOtherThreads class
    //
    inline
    AllowOtherThreads::AllowOtherThreads() throw()
        : save_(0)
    { 
      ACE_High_Res_Timer timer;
      Sandbox* sandbox = Sandbox::thread_sandboxed();

      if(sandbox)
      {
        timer.start();
      }
      
      save_ = PyEval_SaveThread();

      if(sandbox)
      {
        timer.stop();
        
        ACE_Time_Value tm;
        timer.elapsed_time(tm);
        sandbox->prolong_timeout(tm);
      }
    }

    inline
    void
    AllowOtherThreads::disallow() throw()
    {
      if(save_)
      {
        ACE_High_Res_Timer timer;
        Sandbox* sandbox = Sandbox::thread_sandboxed();

        if(sandbox)
        {
          timer.start();
        }
        
        PyEval_RestoreThread(save_);
        save_ = 0;

        if(sandbox)
        {
          timer.stop();

          ACE_Time_Value tm;
          timer.elapsed_time(tm);
          sandbox->prolong_timeout(tm);
        }        
      }
    }

    //
    // AllowOtherThreads class
    //
    inline
    EnsureCurrentThread::EnsureCurrentThread() throw()
        : save_(PyGILState_Ensure()),
          ensured_(true)
    {
    }

    inline
    void
    EnsureCurrentThread::release() throw()
    {
      if(ensured_)
      {
        PyGILState_Release(save_);
        ensured_ = false;
      }
    }

    //
    // Other
    //

    //
    // TypeAdapter template class
    //
    template<PyTypeObject* type, typename TYPE_CLASS>
    TypeAdapter<type, TYPE_CLASS>::TypeAdapter(const char* name)
      throw(El::Exception)
        : name_(name)
    {
      tp_name = name_.c_str();
    }
    
    template<PyTypeObject* type, typename TYPE_CLASS>
    bool
    TypeAdapter<type, TYPE_CLASS>::check_type(PyObject* ob) throw()
    {
      return ob && ob->ob_type == type;
    }
      
    template<PyTypeObject* type, typename TYPE_CLASS>
    bool
    TypeAdapter<type, TYPE_CLASS>::is_type(PyObject* ob) throw()
    {
      return ob && ob == reinterpret_cast<PyObject*>(type);
    }

    template<PyTypeObject* type, typename TYPE_CLASS>
    PyObject*
    TypeAdapter<type, TYPE_CLASS>::down_cast(PyObject* object, bool add_ref)
      throw(Exception, El::Exception)
    {
      if(object && !check_type(object))
      {
        std::ostringstream ostr;
        ostr << "object is not of '" << TypeClass::instance.tp_name
             << "' type";

        report_error(PyExc_TypeError,
                     ostr.str().c_str(),
                     "El::Python::TypeAdapter::down_cast");
      }

      if(add_ref)
      {
        El::Python::add_ref(object);
      }
        
      return object;
    }

    //
    // AnyType class
    //
    inline
    AnyType::AnyType() throw(El::Exception)
    {
      tp_name = "AnyType";
    }
    
    inline
    bool
    AnyType::check_type(PyObject* ob) throw()
    {
      return ob && ob->ob_type;
    }
      
    inline
    bool
    AnyType::is_type(PyObject* ob) throw()
    {
      return ob && ob->ob_type == &PyType_Type;
    }

    inline
    PyObject*
    AnyType::down_cast(PyObject* object, bool add_ref)
      throw(Exception, El::Exception)
    {
      if(object && !check_type(object))
      {
        report_error(PyExc_TypeError,
                     "object do not have proper type",
                     "El::Python::AnyType::down_cast");
      }

      if(add_ref)
      {
        El::Python::add_ref(object);
      }
        
      return object;
    }

    //
    // PyDictType class
    //
    inline
    PyDictType::PyDictType() throw(El::Exception) 
        : El::Python::TypeAdapter<&PyDict_Type, PyDictType>("PyDict_Type")
    {
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_UTILITY_HPP_
