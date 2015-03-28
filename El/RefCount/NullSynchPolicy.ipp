// file      : Utility/Synch/Policy/Null.ipp
// author    : Boris Kolpackov <boris@kolpackov.net>
// copyright : Copyright (c) 2002-2003 Boris Kolpackov
// license   : http://kolpackov.net/license.html

namespace El
{
  namespace RefCount
  {
    namespace NullSynchPolicy
    {
      inline NullGuard::
      NullGuard (NullMutex& lock) throw ()
      {
      }
    }
  }
}
