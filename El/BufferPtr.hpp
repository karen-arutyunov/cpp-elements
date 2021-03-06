/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/BufferPtr.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_BUFFERPTR_HPP_
#define _ELEMENTS_EL_BUFFERPTR_HPP_

#include <El/Exception.hpp>

namespace El
{
  template<typename T>
  class BufferPtr
  {
  public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);

  public:

    typedef T ElemType;
    
    BufferPtr(ElemType* ptr = 0) throw();
    BufferPtr(BufferPtr& src) throw();
    
    ~BufferPtr() throw();

    BufferPtr& operator=(BufferPtr& src) throw();
    void reset(ElemType* ptr) throw();
    
    ElemType* release() throw();
    ElemType* get() const throw();

    ElemType* operator->() const throw();
    ElemType& operator*() const throw();
    
    ElemType*& out() throw();

    ElemType& operator[](size_t i) const throw();

  protected:
    ElemType* ptr_;
  };  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  template<typename T>
  BufferPtr<T>::BufferPtr(ElemType* ptr) throw()
      : ptr_(ptr)
  {
  }
    
  template<typename T>
  BufferPtr<T>::BufferPtr(BufferPtr& src) throw()
      : ptr_(src.release())
  {
  }
  
  template<typename T>
  BufferPtr<T>::~BufferPtr() throw()
  {
    if(ptr_)
    {
      free(ptr_);
    }
  }

  template<typename T>
  BufferPtr<T>&
  BufferPtr<T>::operator=(BufferPtr& src) throw()
  {
    reset(src.release());
    return *this;
  }
  
  template<typename T>
  void
  BufferPtr<T>::reset(ElemType* ptr) throw()
  {
    if(ptr != ptr_)
    {
      if(ptr_)
      {
        free(ptr_);
      }
      
      ptr_ = ptr;
    }
  }
    
  template<typename T>
  typename BufferPtr<T>::ElemType*
  BufferPtr<T>::release() throw()
  {
    ElemType* tmp = ptr_;
    ptr_ = 0;
    return tmp;
  }
  
  template<typename T>
  typename BufferPtr<T>::ElemType*
  BufferPtr<T>::get() const throw()
  {
    return ptr_;
  }

  template<typename T>
  typename BufferPtr<T>::ElemType*
  BufferPtr<T>::operator->() const throw()
  {
    return ptr_;    
  }
  
  template<typename T>
  typename BufferPtr<T>::ElemType&
  BufferPtr<T>::operator*() const throw()
  {
    return *ptr_;
  }
    
  template<typename T>
  typename BufferPtr<T>::ElemType&
  BufferPtr<T>::operator[](size_t i) const throw()
  {    
    return ptr_[i];
  }

  template<typename T>
  typename BufferPtr<T>::ElemType*&
  BufferPtr<T>::out() throw()
  {
    if(ptr_)
    {
      free(ptr_);
    }
    
    ptr_ = 0;
    return ptr_;
  }
}

#endif // _ELEMENTS_EL_BUFFERPTR_HPP_
