// file      : Utility/ReferenceCounting/SmartPtr.tpp
// author    : Boris Kolpackov <boris@kolpackov.net>
// copyright : Copyright (c) 2002-2003 Boris Kolpackov
// license   : http://kolpackov.net/license.html

namespace El
{
  namespace RefCount
  {
  // c-tor's & d-tor

    template <typename T>
    SmartPtr<T>::
    SmartPtr () throw ()
        : ptr_ (0)
    {
    }

    template <typename T>
    SmartPtr<T>::
    SmartPtr (Type* ptr) throw ()
        : ptr_ (ptr)
    {
    }

    template <typename T>
    SmartPtr<T>::
    SmartPtr (SmartPtr<Type> const& s_ptr)
      throw (Interface::Exception, Interface::SystemException)
        : ptr_ (El::RefCount::add_ref (s_ptr.in ()))
    {
    }

    template <typename T>
    template <typename Other>
    SmartPtr<T>::
    SmartPtr (SmartPtr<Other> const& s_ptr)
      throw (Interface::Exception, Interface::SystemException)
        : ptr_ (El::RefCount::add_ref (s_ptr.in ()))
    {
    }


    template <typename T>
    SmartPtr<T>::
    ~SmartPtr () throw ()
    {
      // This is an additional catch-all layer to protect from
      // non-conformant Type.
      try
      {
        if (ptr_ != 0) ptr_->remove_ref ();
      }
      catch (...)
      {
      }
    }

  // operator=

    template <typename T>
    SmartPtr<T>& SmartPtr<T>::
    operator= (Type* ptr) throw ()
    {
      if (ptr_ != 0) ptr_->remove_ref ();
      ptr_ = ptr;
      return *this;
    }

    template <typename T>
    SmartPtr<T>& SmartPtr<T>::
    operator= (SmartPtr<Type> const& s_ptr)
      throw (Interface::Exception, Interface::SystemException)
    {
      Type* old_ptr (ptr_);
      Type* new_ptr (El::RefCount::add_ref (s_ptr.in ())); // this can throw
      if (old_ptr != 0) old_ptr->remove_ref ();

      ptr_ = new_ptr; // commit

      return *this;
    }


    template <typename T>
    template <typename Other>
    SmartPtr<T>& SmartPtr<T>::
    operator= (SmartPtr<Other> const& s_ptr)
      throw (Interface::Exception, Interface::SystemException)
    {
      Type*  old_ptr (ptr_);
      Other* new_ptr (El::RefCount::add_ref (s_ptr.in ())); // this can throw
      if (old_ptr != 0) old_ptr->remove_ref ();

      ptr_ = new_ptr; // commit

      return *this;
    }

  // conversions

    template <typename T>
    SmartPtr<T>::
    operator T* () const throw ()
    {
      return ptr_;
    }


  // accessors

    template <typename T>
    T* SmartPtr<T>::
    operator-> () const throw (NotInitialized)
    {
      if (ptr_ == 0)
      {
        throw NotInitialized(
          "El::RefCount::SmartPtr::operator-> : "
          "unable to dereference NULL pointer.");
      }
      return ptr_;
    }

    template <typename T>
    T* SmartPtr<T>::
    in () const throw ()
    {
      return ptr_;
    }

    template <typename T>
    T* SmartPtr<T>::
    retn() throw ()
    {
      Type* ret (ptr_);
      ptr_ = 0;
      return ret;
    }

  // Specialization of add_ref function for SmartPtr<T>
    template <typename T>
    T*
    add_ref (SmartPtr<T> const& ptr)
      throw (Interface::Exception, Interface::SystemException)
    {
      // delegate to generic implementation
      return El::RefCount::add_ref (ptr.in ());
    }

  // Dynamic type conversion function for SmartPtr's
    template <typename D, typename S>
    D*
    smart_cast (SmartPtr<S> const& s)
      throw (Interface::Exception, Interface::SystemException)
    {
      return El::RefCount::add_ref (dynamic_cast<D*>(s.in ()));
    }

  // Acquisition function
    template <typename T>
    SmartPtr<T>
    acquire (T* ptr) throw (Interface::Exception, Interface::SystemException)
    {
      return SmartPtr<T> (ptr);
    }
  }
}
