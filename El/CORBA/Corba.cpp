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
 * @file   Elements/El/CORBA/Corba.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <El/CORBA/Corba.hpp>

namespace El
{
  namespace Corba
  {
    Adapter::Mutex         Adapter::lock_;
    Adapter::OrbAdapterPtr Adapter::orb_adapter_;
    size_t                 Adapter::ref_counter_ = 0;

    //
    // Adapter class
    //
    
    OrbAdapter* 
    Adapter::orb_adapter(int argc, char** argv)
      throw(Exception, CORBA::SystemException, El::Exception)
    {
      Guard guard(lock_);

      if(orb_adapter_.get() == 0)
      {
        orb_adapter_.reset(new OrbAdapter(argc, argv));
      }

      ++ref_counter_;
      
      return orb_adapter_.get();
    }

    void
    Adapter::orb_adapter_cleanup() throw()
    {
      Guard guard(lock_);

      assert(ref_counter_);

      if(--ref_counter_ == 0)
      {
        orb_adapter_.reset(0);
      }
    }
  }
}
