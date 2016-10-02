/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

// file      : ReferenceCounting/ExternalLock.hpp
// author    : Boris Kolpackov <boris@kolpackov.net>
// copyright : Copyright (c) 2002-2003 Boris Kolpackov
// license   : http://kolpackov.net/license.html

#ifndef EL_REFCOUNT_EXTERNALLOCK_HPP
#define EL_REFCOUNT_EXTERNALLOCK_HPP

#include <El/Exception.hpp>

#include <El/RefCount/NullSynchPolicy.hpp>
#include <El/RefCount/Interface.hpp>

namespace El
{
  namespace RefCount
  {
    template <typename SynchPolicy>
    class ExternalLock : public virtual Interface
    {
    public:
      EL_EXCEPTION(InconsistentState, El::ExceptionBase);

    public:
      ExternalLock (SynchPolicy::Mutex& mutex) throw (SystemException);

      virtual
      ~ExternalLock () throw ();

    public:
      virtual void
      add_ref () const throw (Exception, SystemException);

      virtual void
      remove_ref () const throw ();

      virtual count_t
      refcount_value () const throw (Exception, SystemException);

    protected:
      virtual void
      add_ref_i () const throw (Exception, SystemException);

      virtual bool
      remove_ref_i () const throw (Exception, SystemException);

      virtual count_t
      refcount_value_i () const throw (Exception, SystemException);

      typename SynchPolicy::Mutex&
      lock_i () const throw ();

    protected:
      typedef
      typename SynchPolicy::Mutex
      Mutex_;

      typedef
      typename SynchPolicy::ReadGuard
      ReadGuard_;

      typedef
      typename SynchPolicy::WriteGuard
      WriteGuard_;

    protected:
      mutable count_t ref_count_;
    
    private:
      mutable Mutex_& lock_;

    private:
      // Copy semanic is not supported.
      ExternalLock (ExternalLock const&) throw ();
      ExternalLock&
      operator= (ExternalLock const&) throw ();
    };
  }
}

#include <El/RefCount/ExternalLock.ipp>

#endif  // EL_REFCOUNT_EXTERNALLOCK_HPP

