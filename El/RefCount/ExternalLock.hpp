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

