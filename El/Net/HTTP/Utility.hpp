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
 * @file Elements/El/Net/HTTP/Utility.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_UTILITY_HPP_
#define _ELEMENTS_EL_NET_HTTP_UTILITY_HPP_

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>
#include <El/Net/HTTP/Session.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      const char* browser(const char* user_agent) throw(El::Exception);
      const char* feed_reader(const char* user_agent) throw(El::Exception);
      const char* crawler(const char* user_agent) throw(El::Exception);
      const char* os(const char* user_agent) throw(El::Exception);
      const char* computer(const char* user_agent) throw(El::Exception);
      const char* tab(const char* user_agent) throw(El::Exception);
      const char* phone(const char* user_agent) throw(El::Exception);

      struct SearchInfo
      {
        std::string engine;
        std::string query;
        std::string image;

        void write(El::BinaryOutStream& ostr) const
          throw(Exception, El::Exception);
        
        void read(El::BinaryInStream& istr) throw(Exception, El::Exception);
      };

      SearchInfo search_info(const char* url) throw(El::Exception);

      struct RequestParams
      {
        std::string user_agent;
        std::string referer;
        unsigned long timeout;
        unsigned long redirects_to_follow;
        unsigned long recv_buffer_size;

        Session::Interceptor* interceptor;

        RequestParams() throw();

        void write(El::BinaryOutStream& ostr) const
          throw(Exception, El::Exception);
        
        void read(El::BinaryInStream& istr) throw(Exception, El::Exception);
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
      // SearchInfo struct
      //
      inline
      void
      SearchInfo::write(El::BinaryOutStream& ostr) const
        throw(Exception, El::Exception)
      {
        ostr << engine << query << image;
      }
        
      inline
      void
      SearchInfo::read(El::BinaryInStream& istr)
        throw(Exception, El::Exception)
      {
        istr >> engine >> query >> image;
      }      
      
      //
      // RequestParams struct
      //
      inline
      RequestParams::RequestParams() throw()
          : timeout(0),
            redirects_to_follow(0),
            recv_buffer_size(0),
            interceptor(0)
      {
      }

      inline
      void
      RequestParams::write(El::BinaryOutStream& ostr) const
        throw(Exception, El::Exception)
      {
        ostr << user_agent << referer << (uint32_t)timeout
             << (uint32_t)redirects_to_follow << (uint32_t)recv_buffer_size;
      }
        
      inline
      void
      RequestParams::read(El::BinaryInStream& istr)
        throw(Exception, El::Exception)
      {
        uint32_t tm = 0;
        uint32_t rtf = 0;
        uint32_t rbs = 0;
        
        istr >> user_agent >> referer >> tm >> rtf >> rbs;

        timeout = tm;
        redirects_to_follow = rtf;
        recv_buffer_size = rbs;

        interceptor = 0;
      }
    }
  }
}

#endif // _ELEMENTS_EL_NET_HTTP_UTILITY_HPP_
