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
 * @file Elements/El/Geography/AddressInfo.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_GEOGRAPHY_ADDRESS_INFO_HPP_
#define _ELEMENTS_EL_GEOGRAPHY_ADDRESS_INFO_HPP_

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <GeoIP.h>

#include <El/Exception.hpp>
#include <El/Country.hpp>

namespace El
{
  namespace Geography
  {
    class AddressInfo
    {
    public:    
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
    public:
      // CityEdition: GEOIP_CITY_EDITION_REV1
      // CountryEdition: GEOIP_COUNTRY_EDITION
      AddressInfo(GeoIPDBTypes type = GEOIP_CITY_EDITION_REV1,
                  unsigned long flags = GEOIP_STANDARD)
        throw(Exception, El::Exception);
      
      AddressInfo(const char* filepath,
                  unsigned long flags = GEOIP_STANDARD)
        throw(Exception, El::Exception);

      ~AddressInfo() throw();

      El::Country country(const char* host) throw(Exception, El::Exception);

    private:
      typedef ACE_Thread_Mutex Mutex;
      typedef ACE_Guard<Mutex> Guard;
      
      Mutex lock_;

      GeoIP* geoip_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Geography
  {
    inline
    AddressInfo::~AddressInfo() throw()
    {
      if(geoip_)
      {
        GeoIP_delete(geoip_);
      }
    }
  }
}

#endif // _ELEMENTS_EL_GEOGRAPHY_ADDRESS_INFO_HPP_
