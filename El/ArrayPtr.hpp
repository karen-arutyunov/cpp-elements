/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/ArrayPtr.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_ARRAYPTR_HPP_
#define _ELEMENTS_EL_ARRAYPTR_HPP_

#include <El/Exception.hpp>

namespace El
{
  template<typename T>
  class ArrayPtr
  {
  public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);

  public:

    typedef T ElemType;
    
    ArrayPtr(ElemType* array = 0) throw();
    ArrayPtr(ArrayPtr& src) throw();
    
    ~ArrayPtr() throw();

    ArrayPtr& operator=(ArrayPtr& src) throw();
    void reset(ElemType* array) throw();
    
    ElemType* release() throw();
    ElemType* get() const throw();

    ElemType* operator->() const throw();
    ElemType& operator*() const throw();
    
    ElemType& operator[](size_t i) const throw();

    ElemType*& out() throw();

  protected:
    ElemType* array_;
  };  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  template<typename T>
  ArrayPtr<T>::ArrayPtr(ElemType* array) throw()
      : array_(array)
  {
  }
    
  template<typename T>
  ArrayPtr<T>::ArrayPtr(ArrayPtr& src) throw()
      : array_(src.release())
  {
  }
  
  template<typename T>
  ArrayPtr<T>::~ArrayPtr() throw()
  {
    delete [] array_;
  }

  template<typename T>
  ArrayPtr<T>&
  ArrayPtr<T>::operator=(ArrayPtr& src) throw()
  {
    reset(src.release());
    return *this;
  }
  
  template<typename T>
  void
  ArrayPtr<T>::reset(ElemType* array) throw()
  {
    if(array != array_)
    {
      delete [] array_;
      array_ = array;
    }
  }
    
  template<typename T>
  typename ArrayPtr<T>::ElemType*
  ArrayPtr<T>::release() throw()
  {
    ElemType* tmp = array_;
    array_ = 0;
    return tmp;
  }
  
  template<typename T>
  typename ArrayPtr<T>::ElemType*
  ArrayPtr<T>::get() const throw()
  {
    return array_;
  }

  template<typename T>
  typename ArrayPtr<T>::ElemType*
  ArrayPtr<T>::operator->() const throw()
  {
    return array_;    
  }
  
  template<typename T>
  typename ArrayPtr<T>::ElemType&
  ArrayPtr<T>::operator*() const throw()
  {
    return *array_;
  }
  
    
  template<typename T>
  typename ArrayPtr<T>::ElemType&
  ArrayPtr<T>::operator[](size_t i) const throw()
  {    
    return array_[i];
  }

  template<typename T>
  typename ArrayPtr<T>::ElemType*&
  ArrayPtr<T>::out() throw()
  {
    delete [] array_;
    array_ = 0;
    return array_;
  }
}

#endif // _ELEMENTS_EL_ARRAYPTR_HPP_
