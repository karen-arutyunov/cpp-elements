/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
