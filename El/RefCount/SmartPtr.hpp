/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

// file      : ReferenceCounting/SmartPtr.hpp
// author    : Boris Kolpackov <boris@kolpackov.net>
// copyright : Copyright (c) 2002-2003 Boris Kolpackov
// license   : http://kolpackov.net/license.html

#ifndef EL_REFCOUNT_SMART_PTR_HPP
#define EL_REFCOUNT_SMART_PTR_HPP

#include <El/Exception.hpp>

#include <El/RefCount/Interface.hpp>

namespace El
{
  namespace RefCount
  {
    template <typename T>
    class SmartPtr
    {
    public:
      typedef T Type;

      EL_EXCEPTION(NotInitialized, El::ExceptionBase);

    public:
      // c-tor's

      SmartPtr () throw ();
      SmartPtr (Type* ptr) throw ();
      SmartPtr (SmartPtr<Type> const& s_ptr)
        throw (Interface::Exception, Interface::SystemException);

      template <typename Other>
      SmartPtr (SmartPtr<Other> const& s_ptr)
        throw (Interface::Exception, Interface::SystemException);

      // d-tor

      ~SmartPtr () throw ();

      // assignment & copy-assignment operators

      SmartPtr<Type>&
      operator= (Type* ptr) throw ();

      SmartPtr<Type>&
      operator= (SmartPtr<Type> const& s_ptr)
        throw (Interface::Exception, Interface::SystemException);

      template <typename Other>
      SmartPtr<Type>&
      operator= (SmartPtr<Other> const& s_ptr)
        throw (Interface::Exception, Interface::SystemException);

      //conversions

      operator Type* () const throw ();

      // accessors

      Type*
      operator-> () const throw (NotInitialized);

      Type*
      in () const throw ();

      Type*
      retn() throw ();

    private:
      Type* ptr_;
    };

    // Specialization of add_ref function for SmartPtr<T>
    template <typename T>
    T*
    add_ref (SmartPtr<T> const& ptr)
      throw (Interface::Exception, Interface::SystemException);


    // Dynamic type conversion function for SmartPtr's
    template <typename D, typename S>
    D*
    smart_cast (SmartPtr<S> const& s)
      throw (Interface::Exception, Interface::SystemException);

    // Acquisition function
    template <typename T>
    SmartPtr<T>
    acquire (T* ptr) throw (Interface::Exception, Interface::SystemException);
  }
}

#include <El/RefCount/SmartPtr.tpp>

#endif  // EL_REFCOUNT_SMART_PTR_HPP
