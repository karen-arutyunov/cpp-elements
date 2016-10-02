/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Geography/AddressInfo.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

#include "AddressInfo.hpp"

namespace El
{
  namespace Geography
  {
    AddressInfo::AddressInfo(GeoIPDBTypes type, unsigned long flags)
      throw(Exception, El::Exception)
        : geoip_(GeoIP_open_type(type, flags))
    {
      if(geoip_ == 0)
      {
        std::ostringstream ostr;
        ostr << "El::Geography::AddressInfo::AddressInfo: GeoIP_open failed "
          "for flags " << flags << ", type " << type;

        throw Exception(ostr.str());
      }
    }
    
    AddressInfo::AddressInfo(const char* filepath, unsigned long flags)
      throw(Exception, El::Exception)
        : geoip_(GeoIP_open(filepath, flags))
    {
      if(geoip_ == 0)
      {
        std::ostringstream ostr;
        ostr << "El::Geography::AddressInfo::AddressInfo: GeoIP_open failed "
          "for flags " << flags << ", file " << filepath;

        throw Exception(ostr.str());
      }
    }

/*    
    // CityEdition edition
    El::Country
    AddressInfo::country(const char* host) throw(Exception, El::Exception)
    {
      Guard guard(lock_);
      
      const char* ccode = GeoIP_country_code3_by_name(geoip_, host);

      try
      {
        return ccode ? El::Country(ccode) : El::Country::null;
      }
      catch(const El::Country::Exception&)
      {
        return El::Country::null;
      }
    }
*/

    // CityEdition: 
    El::Country
    AddressInfo::country(const char* host) throw(Exception, El::Exception)
    {
      El::Country country;
      
      Guard guard(lock_);
      
      GeoIPRecord* rec = GeoIP_record_by_name(geoip_, host);

      if(rec)
      {
        try
        {
          country = El::Country(rec->country_code3);
        }
        catch(const El::Country::Exception&)
        {
        }

        GeoIPRecord_delete(rec);
      }

      return country;
    }
  }
}
