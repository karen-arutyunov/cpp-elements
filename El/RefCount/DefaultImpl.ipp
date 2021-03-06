// file      : Utility/ReferenceCounting/DefaultImpl.ipp
// author    : Boris Kolpackov <boris@kolpackov.net>
// copyright : Copyright (c) 2002-2003 Boris Kolpackov
// license   : http://kolpackov.net/license.html

namespace El
{
  namespace RefCount
  {
  // c-tor & d-tor

    template <typename SynchPolicy>
    DefaultImpl<SynchPolicy>::DefaultImpl() throw (Interface::SystemException)
        : ref_count_ (1)
    {
    }

    template <typename SynchPolicy>
    DefaultImpl<SynchPolicy>::~DefaultImpl() throw ()
    {
    }

  // add_ref, remove_ref and refcount_value member functions

    template <typename SynchPolicy>
    void 
    DefaultImpl<SynchPolicy>::add_ref () const 
      throw(Exception, SystemException)
    {
      WriteGuard_ guard (lock_);

      add_ref_i ();

      // To suppress warning re unused guard
//      WriteGuard_* pguard = &guard;
//      pguard = 0;
    }

    template <typename SynchPolicy>
    void 
    DefaultImpl<SynchPolicy>::remove_ref() const throw ()
    {
      bool destroy (false);
      try
      {
        WriteGuard_ guard (lock_);
        destroy = remove_ref_i ();
        
        // To suppress warning re unused guard
//        WriteGuard_* pguard = &guard;
//        pguard = 0;
      }
      catch (...)
      {
        // there is nothing we can do
      }

      if(destroy) destroy_i();
    }

    template <typename SynchPolicy>
    Interface::count_t 
    DefaultImpl<SynchPolicy>::refcount_value () const 
      throw(Exception, SystemException)
    {
      ReadGuard_ guard (lock_);

      // To suppress warning re unused guard
//      ReadGuard_* pguard = &guard;
//      pguard = 0;

      return refcount_value_i ();
    }

  // add_ref_i, remove_ref_i and refcount_value_i member functions

    template <typename SynchPolicy>
    void 
    DefaultImpl<SynchPolicy>::add_ref_i () const 
      throw(Exception, SystemException)
    {
      ref_count_++;
    }

    template <typename SynchPolicy>
    bool 
    DefaultImpl<SynchPolicy>::remove_ref_i() const 
      throw(Exception, SystemException)
    {
      bool destroy (false);
      if (ref_count_ > 0)
      {
        if (--ref_count_ == 0) destroy = true;
      }
      else
      {
        throw InconsistentState (
          "ReferenceCounting::DefaultImpl::_remove_ref_i: "
          "reference counter is zero.");
      }

      return destroy;
    }

    template <typename SynchPolicy>
    Interface::count_t 
    DefaultImpl<SynchPolicy>::refcount_value_i() const 
      throw(Exception, SystemException)
    {
      return ref_count_;
    }

    template <typename SynchPolicy>
    typename SynchPolicy::Mutex& 
    DefaultImpl<SynchPolicy>::lock_i() const throw()
    {
      return lock_;
    }

    template <typename SynchPolicy>
    void
    DefaultImpl<SynchPolicy>::destroy_i() const throw()
    {
      delete this;
    }
  }
}
