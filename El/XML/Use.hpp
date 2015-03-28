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
 * @file Elements/El/XML/Use.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_XML_USE_HPP_
#define _ELEMENTS_EL_XML_USE_HPP_

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <xercesc/util/PlatformUtils.hpp>

namespace El
{
  namespace XML
  {
    class Use
    {
    public :
      Use() throw();
      ~Use() throw();

      static void start() throw();
      static void stop() throw();

    private:
      typedef ACE_Thread_Mutex Mutex;
      typedef ACE_Guard<Mutex> Guard;
      
      static unsigned long counter_;
      static Mutex lock_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace XML
  {
    inline
    Use::Use() throw()
    {
      start();
    }
    
    inline
    Use::~Use() throw()
    {
      stop();
    }

    inline
    void
    Use::start() throw()
    {
      Guard guard(lock_);

      if(counter_++ == 0)
      {
        xercesc::XMLPlatformUtils::Initialize();
      }
    }
    
    inline
    void
    Use::stop() throw()
    {
      Guard guard(lock_);

      if(counter_ > 0)
      {
        if(--counter_ == 0)
        {
          xercesc::XMLPlatformUtils::Terminate();
        }
      }
    }
  }
}

#endif // _ELEMENTS_EL_XML_USE_HPP_
 
