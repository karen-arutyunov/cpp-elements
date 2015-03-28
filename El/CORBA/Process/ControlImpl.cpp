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
 * @file   Elements/El/CORBA/Process/ControlImpl.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <El/CORBA/Corba.hpp>

#include <iostream>

#include <El/Exception.hpp>

#include "ControlImpl.hpp"

namespace El
{
  namespace Corba
  {
    //
    // Rotate class
    //    

    class ShutDown : public El::Service::ThreadPool::ServiceEvent
    {
    public:
      ShutDown(El::Service::Callback* callback) throw(El::Exception);
      virtual ~ShutDown() throw();
    };
    
    ShutDown::ShutDown(El::Service::Callback* callback) throw(El::Exception)
        : El__Service__ThreadPool__ServiceEventBase(callback, 0, true)
    {
    }
    
    ShutDown::~ShutDown() throw()
    {
    }
    
    //
    // ProcessCtlImpl class
    //    

    ProcessCtlImpl::ProcessCtlImpl(CORBA::ORB_ptr orb)
      throw(ProcessCtlImpl::Exception, El::Exception)
        : orb_(CORBA::ORB::_duplicate(orb)),
          shutting_down_(false)
    {
    }
    
    void
    ProcessCtlImpl::stop() throw(CORBA::SystemException)
    {
      try
      {
        {
          SD_Guard guard(shutdown_lock_);
        
          if(shutting_down_)
          {
            return;
          }

          shutting_down_ = true;
        }

        terminate_process();
    
        {
          SD_Guard guard(shutdown_lock_);

          if(thread_pool_.in() == 0)
          {
            thread_pool_ = new El::Service::ThreadPool(this);
            thread_pool_->start();
      
            El::Service::ThreadPool::Task_var task = new ShutDown(this);
            thread_pool_->execute(task.in());
          }
        }
        
      }
      catch(const El::Exception& e)
      {
        std::cerr << "El::Corba::ProcessCtlImpl::stop: El::Exception caught. "
          " Description:\n" << e << std::endl;

        {
          SD_Guard guard(shutdown_lock_);
        
          if(thread_pool_.in() != 0)
          {
            thread_pool_->stop();
          }
        }
        
      }
    }
    
    bool
    ProcessCtlImpl::notify(El::Service::Event* event) throw(El::Exception)
    {
      ShutDown* shutdown = dynamic_cast<ShutDown*>(event);

      if(shutdown != 0)
      {
/*        
        CORBA::Object_var poa_object =
          orb_->resolve_initial_references("RootPOA");

        PortableServer::POA_var root_poa =
          PortableServer::POA::_narrow(poa_object.in ());
        
        PortableServer::POAManager_var poa_manager = 
          root_poa->the_POAManager();
        
        poa_manager->deactivate(0, 1);
*/      
        orb_->shutdown(1);
//        orb_->shutdown(0);
        return true;
      }

      El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);

      if(error != 0)
      {
        std::cerr << El::Service::error_message(
          error,
          "El::Corba::ProcessCtlImpl::notify: ")
                  << std::endl;
        
        return true;
      }

      return false;
    }
    
    void
    ProcessCtlImpl::wait() throw()
    {
      try
      {
        SD_Guard guard(shutdown_lock_);
        
        if(thread_pool_.in() != 0)
        {
          thread_pool_->stop();
          thread_pool_->wait();
        }
      }
      catch(const El::Exception& e)
      {
        std::cerr << "El::Corba::ProcessCtlImpl::wait: El::Exception caught. "
          " Description:\n" << e << std::endl;
      }      
    }
  }
}
