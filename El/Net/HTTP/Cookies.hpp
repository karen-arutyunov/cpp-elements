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
 * @file Elements/El/Net/HTTP/Cookies.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_COOKIES_HPP_
#define _ELEMENTS_EL_NET_HTTP_COOKIES_HPP_

#include <string>
#include <list>
#include <iostream>

#include <El/Exception.hpp>
#include <El/Moment.hpp>
#include <El/String/Manip.hpp>
#include <El/String/ListParser.hpp>

#include <El/Net/HTTP/Exception.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      struct Cookie
      {
        std::string name;
        std::string value;

        Cookie() throw(El::Exception);
        
        Cookie(const char* name, const char* value)
          throw(InvalidArg, El::Exception);

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);
      };

      class CookieList : public std::list<Cookie>
      {
      public:
        // In cookies is in "name=value;name=value;..." form
        void add(const char* cookies) throw(InvalidArg, El::Exception);

        const char* most_specific(const char* cookie_name) const
          throw(El::Exception);
      };

      struct CookieSetter
      {
        std::string name;
        std::string value;
        El::Moment  expiration;
        std::string path;
        std::string domain;
        bool        secure;
        bool        session;

        CookieSetter() throw(El::Exception);
        
        CookieSetter(const char* name,
                     const char* value = 0,
                     const El::Moment* expiration = 0,
                     const char* path = 0,
                     const char* domain = 0,
                     bool secure = false)
          throw(InvalidArg, El::Exception);

        void write(std::ostream& ostr) const throw(El::Exception);

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);

        std::string string() const throw(El::Exception);
      };

    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      //
      // Cookie struct
      //
      inline
      Cookie::Cookie() throw(El::Exception)
      {
      }
      
      inline
      Cookie::Cookie(const char* nm, const char* vl)
        throw(InvalidArg, El::Exception)
          : name(nm ? nm : ""),
            value(vl ? vl : "")
      {
        if(name.empty())
        {
          throw InvalidArg("El::Net::HTTP::Cookie::Cookie: empty name");
        }
      }

      inline
      void
      Cookie::write(El::BinaryOutStream& bstr) const throw(El::Exception)
      {
        bstr << name << value;
      }

      inline
      void
      Cookie::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> name >> value;
      }

      //
      // CookieSetter struct
      //
      inline
      CookieSetter::CookieSetter() throw(El::Exception)
          : secure(false),
            session(false)
      {
      }
        
      inline
      CookieSetter::CookieSetter(const char* nm,
                                 const char* vl,
                                 const El::Moment* ex,
                                 const char* pt,
                                 const char* dm,
                                 bool sc)
        throw(InvalidArg, El::Exception)
          : name(nm ? nm : ""),
            value(vl ? vl : ""),
            expiration(ex ? *ex : El::Moment::null),
            path(pt ? pt : ""),
            domain(dm ? dm : ""),
            secure(sc),
            session(ex == 0)
      {
        std::size_t pos = domain.find(':');

        if(pos != std::string::npos)
        {
          domain.resize(pos);
        }
      }

      inline
      void
      CookieSetter::write(El::BinaryOutStream& bstr) const throw(El::Exception)
      {
        bstr << name << value << expiration << path << domain
             << (uint8_t)secure << (uint8_t)session;
      }

      inline
      void
      CookieSetter::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        uint8_t sec = 0;
        uint8_t ses = 0;
        bstr >> name >> value >> expiration >> path >> domain >> sec >> ses;
        secure = sec;
        session = ses;
      }

      inline
      void
      CookieSetter::write(std::ostream& ostr) const throw(El::Exception)
      {
        if(name.empty())
        {
          throw Exception("El::Net::HTTP::CookieSetter::write: empty name");
        }

        ostr << name << "=" << value;

        if(!session)
        {
          ostr << "; expires=" << expiration.http_cookie_expiration();
        }

        if(!domain.empty())
        {
          ostr << "; domain=" << domain;
        }

        if(!path.empty())
        {
          ostr << "; path=" << path;
        }

        if(secure)
        {
          ostr << "; secure";
        }  
      }

      inline
      std::string
      CookieSetter::string() const throw(El::Exception)
      {
        std::ostringstream ostr;
        write(ostr);
        return ostr.str();
      }
    }
  }
}

inline
std::ostream& operator<<(std::ostream& ostr,
                         const El::Net::HTTP::Cookie& cookie)
  throw(El::Exception)
{
  ostr << cookie.name << "=" << cookie.value;
  return ostr;
}

#endif // _ELEMENTS_EL_NET_HTTP_COOKIES_HPP_
