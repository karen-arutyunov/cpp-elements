/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
