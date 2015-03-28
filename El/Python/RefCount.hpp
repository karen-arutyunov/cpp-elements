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
 * @file Elements/El/Python/RefCount.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_REFCOUNT_HPP_
#define _ELEMENTS_EL_PYTHON_REFCOUNT_HPP_

#include <Python.h>

#include <El/Exception.hpp>

namespace El
{
  namespace Python
  {
    template <typename T>
    class SmartPtr
    {
    public:
      typedef T Type;

      EL_EXCEPTION(NotInitialized, El::ExceptionBase);

    public:
      // c-tor's

      SmartPtr() throw ();
      SmartPtr(Type* ptr) throw ();
      SmartPtr(SmartPtr<Type> const& s_ptr) throw();
      
      template <typename Other>
      SmartPtr (SmartPtr<Other> const& s_ptr) throw();

      // d-tor

      ~SmartPtr() throw ();

      // assignment & copy-assignment operators

      SmartPtr<Type>&
      operator=(Type* ptr) throw ();

      SmartPtr<Type>&
      operator=(SmartPtr<Type> const& s_ptr) throw();

      template <typename Other>
      SmartPtr<Type>&
      operator=(SmartPtr<Other> const& s_ptr) throw();

      //conversions

      operator Type*() const throw();

      // accessors

      Type* operator->() const throw(NotInitialized);
      Type* in() const throw();
      Type** out() throw();      
      Type* retn() throw();
      Type* add_ref() throw();

    private:
      Type* ptr_;
    };

    template<typename Type> Type* add_ref(Type* ptr) throw();
    template<typename Type> void remove_ref(Type* ptr) throw();
    
    typedef SmartPtr<PyObject> Object_var;
    typedef SmartPtr<PyCodeObject> CodeObject_var;
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
    // SmartPtr
    //
    template <typename T>
    SmartPtr<T>::SmartPtr() throw()
        : ptr_ (0)
    {
    }

    template <typename T>
    SmartPtr<T>::SmartPtr(Type* ptr) throw()
        : ptr_ (ptr)
    {
    }

    template <typename T>
    SmartPtr<T>::SmartPtr(SmartPtr<Type> const& s_ptr) throw()
        : ptr_(s_ptr.in ())
    {
      Py_XINCREF(ptr_);
    }

    template <typename T>
    template <typename Other>
    SmartPtr<T>::SmartPtr(SmartPtr<Other> const& s_ptr) throw()
        : ptr_(s_ptr.in())
    {
      Py_XINCREF(ptr_);
    }

    template <typename T>
    SmartPtr<T>::~SmartPtr() throw()
    {
      Py_XDECREF(ptr_);
    }
    
    template <typename T>
    SmartPtr<T>&
    SmartPtr<T>::operator=(Type* ptr) throw()
    {
      Type* old_ptr(ptr_);
      
      ptr_ = ptr;
      Py_XDECREF(old_ptr);
      
      return *this;
    }

    template <typename T>
    SmartPtr<T>&
    SmartPtr<T>::operator=(SmartPtr<Type> const& s_ptr) throw()
    {
      Type* old_ptr(ptr_);
      
      ptr_ = s_ptr.in();
      Py_XINCREF(ptr_);
      
      Py_XDECREF(old_ptr);

      return *this;
    }


    template <typename T>
    template <typename Other>
    SmartPtr<T>&
    SmartPtr<T>::operator=(SmartPtr<Other> const& s_ptr) throw()
    {
      Type* old_ptr(ptr_);
      
      ptr_ = s_ptr.in();
      Py_XINCREF(ptr_);
      
      Py_XDECREF(old_ptr);

      return *this;
    }

  // conversions

    template <typename T>
    SmartPtr<T>::operator T*() const throw()
    {
      return ptr_;
    }

  // accessors

    template <typename T>
    T*
    SmartPtr<T>::operator->() const throw(NotInitialized)
    {
      if (ptr_ == 0)
      {
        throw NotInitialized(
          "El::Python::SmartPtr::operator-> : "
          "unable to dereference NULL pointer.");
      }
      
      return ptr_;
    }

    template <typename T>
    T*
    SmartPtr<T>::in() const throw()
    {
      return reinterpret_cast<T*>(ptr_);
    }

    template <typename T>
    T**
    SmartPtr<T>::out() throw()
    {
      return &ptr_;
    }

    template <typename T>
    T*
    SmartPtr<T>::retn() throw()
    {
      Type* ret(ptr_);
      ptr_ = 0;
      return ret;
    }

    template <typename T>
    T*
    SmartPtr<T>::add_ref() throw()
    {
      Py_XINCREF(ptr_);
      return ptr_;
    }
    
    //
    // Other
    //
    template<typename Type>
    Type*
    add_ref(Type* ptr) throw()
    {
      Py_XINCREF(ptr);
      return ptr;
    }

    template<typename Type>
    void
    remove_ref(Type* ptr) throw()
    {
      Py_XDECREF(ptr);
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_REFCOUNT_HPP_
