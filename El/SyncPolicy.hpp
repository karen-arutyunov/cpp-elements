/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   El/SyncPolicy.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_SYNCPOLICY_HPP_
#define _ELEMENTS_EL_SYNCPOLICY_HPP_

#include <ace/Synch.h>
#include <ace/Guard_T.h>

namespace El
{
  namespace Sync
  {
    template <typename AdoptedMutex,
              typename AdoptedReadGuard,
              typename AdoptedWriteGuard>
    struct PolicyAdapter
    {
      typedef AdoptedMutex      Mutex;
      typedef AdoptedReadGuard  ReadGuard;
      typedef AdoptedWriteGuard WriteGuard;
    };

    typedef PolicyAdapter<ACE_Thread_Mutex,
                          ACE_Read_Guard<ACE_Thread_Mutex>,
                          ACE_Write_Guard<ACE_Thread_Mutex> >
    ThreadPolicy;
  
    typedef PolicyAdapter<ACE_RW_Thread_Mutex,
                          ACE_Read_Guard<ACE_RW_Thread_Mutex>,
                          ACE_Write_Guard<ACE_RW_Thread_Mutex> >
    ThreadRWPolicy;
  
    typedef PolicyAdapter<ACE_Recursive_Thread_Mutex,
                          ACE_Guard<ACE_Recursive_Thread_Mutex>,
                          ACE_Guard<ACE_Recursive_Thread_Mutex> >
    ThreadRecursivePolicy;
  }
}

#endif  // _ELEMENTS_EL_SYNCPOLICY_HPP_
