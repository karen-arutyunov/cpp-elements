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
