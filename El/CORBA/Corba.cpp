/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
