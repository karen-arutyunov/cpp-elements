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

// file      : ReferenceCounting/DefaultImpl.hpp
// author    : Boris Kolpackov <boris@kolpackov.net>
// copyright : Copyright (c) 2002-2003 Boris Kolpackov
// license   : http://kolpackov.net/license.html

#ifndef EL_REFCOUNT_DEFAULT_IMPL_HPP
#define EL_REFCOUNT_DEFAULT_IMPL_HPP

#include <El/Exception.hpp>

#include <El/RefCount/NullSynchPolicy.hpp>
#include <El/RefCount/Interface.hpp>

namespace El
{
  namespace RefCount
  {
    // Default reference counter implementation with parameterised
    // synchronization policy. It is assumed that none of the SynchPolicy
    // types throw any logic exceptions. If in fact they do then these
    // exceptions won't be handled and will be automatically converted
    // to system exceptions.

    template <typename SynchPolicy = RefCount::NullSynchPolicy::Null>
    class DefaultImpl : public virtual Interface
    {
    public:
      EL_EXCEPTION(InconsistentState, El::ExceptionBase);

    public:
      DefaultImpl () throw (SystemException);

      virtual
      ~DefaultImpl () throw ();

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

      virtual void destroy_i() const throw();

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
      mutable Mutex_  lock_;

    private:
      // Copy semanic is not supported.
      DefaultImpl (DefaultImpl const&) throw ();
      DefaultImpl&
      operator= (DefaultImpl const&) throw ();
    };
  }
}

#include <El/RefCount/DefaultImpl.ipp>

#endif  // EL_REFCOUNT_DEFAULT_IMPL_HPP

