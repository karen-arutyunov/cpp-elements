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

// file      : Utility/Synch/Policy/Null.hpp
// author    : Boris Kolpackov <boris@kolpackov.net>
// copyright : Copyright (c) 2002-2003 Boris Kolpackov
// license   : http://kolpackov.net/license.html

#ifndef EL_REFCOUNT_NULL_SYNCH_POLICY_HPP
#define EL_REFCOUNT_NULL_SYNCH_POLICY_HPP

namespace El
{
  namespace RefCount
  {
    namespace NullSynchPolicy
    {
      class NullMutex
      {
      };

      class NullGuard
      {
      public:
        explicit NullGuard (NullMutex&) throw ();
        
      private:
        NullGuard (NullGuard const&) throw ();
      
        NullGuard& operator= (NullGuard const&) throw ();
      };

      struct Null
      {
        typedef NullMutex Mutex;
        typedef NullGuard ReadGuard;
        typedef NullGuard WriteGuard;
      };
    }
  }
}

#include <El/RefCount/NullSynchPolicy.ipp>

#endif  // EL_REFCOUNT_NULL_SYNCH_POLICY_HPP
