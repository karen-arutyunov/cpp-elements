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
 * @file Elements/El/Net/HTTP/Robots.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_ROBOTS_HPP_
#define _ELEMENTS_EL_NET_HTTP_ROBOTS_HPP_

#include <string>
#include <sstream>
#include <iostream>

#include <ext/hash_map>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Net/Exception.hpp>
#include <El/Net/HTTP/URL.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      class RobotsChecker
      {
      public:
        EL_EXCEPTION(Exception, El::Net::Exception);

      public:
        
        RobotsChecker(const ACE_Time_Value& request_timeout,
                      size_t redirects_to_follow,
                      const ACE_Time_Value& entry_timeout,
                      const ACE_Time_Value& cleanup_period) throw();

        bool allowed(const char* url, const char* user_agent)
          throw(Exception, El::Exception);
        
        bool allowed(const El::Net::HTTP::URL* url, const char* user_agent)
          throw(Exception, El::Exception);

        void dump(std::ostream& ostr) const throw(El::Exception);

      private:

        typedef std::vector<std::string> StringArray;
        
        struct Record
        {
          StringArray agents;
          StringArray disallow_paths;
          StringArray allow_paths;
          
          void clear() throw();
          bool filled() const throw();
        };

        typedef std::vector<Record> RecordArray;

        class SiteInfo
        {
        public:
          RecordArray records;
          ACE_Time_Value expiration;

        public:

          bool load(const char* host,
                    const char* user_agent,
                    const ACE_Time_Value& request_timeout,
                    size_t redirects_to_follow)
            throw(El::Exception);

          bool allowed(const char* path, const char* user_agent) const throw();
        };

        typedef __gnu_cxx::hash_map<std::string, SiteInfo, El::Hash::String>
        SiteInfoMap;
        
      private:
        typedef ACE_RW_Mutex           Mutex;
        typedef ACE_Read_Guard<Mutex>  ReadGuard;
        typedef ACE_Write_Guard<Mutex> WriteGuard;
        
        mutable Mutex lock_;
        
        ACE_Time_Value request_timeout_;
        size_t redirects_to_follow_;
        ACE_Time_Value entry_timeout_;
        ACE_Time_Value cleanup_period_;
        ACE_Time_Value next_cleanup_;
        
        SiteInfoMap sites_;
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
      // RobotsChecker::Record class
      //
      inline
      RobotsChecker::RobotsChecker(const ACE_Time_Value& request_timeout,
                                   size_t redirects_to_follow,
                                   const ACE_Time_Value& entry_timeout,
                                   const ACE_Time_Value& cleanup_period)
        throw()
          : request_timeout_(request_timeout),
            redirects_to_follow_(redirects_to_follow),
            entry_timeout_(entry_timeout),
            cleanup_period_(cleanup_period)
      {
      }

      inline
      bool
      RobotsChecker::allowed(const char* url, const char* user_agent)
        throw(Exception, El::Exception)
      {
        El::Net::HTTP::URL_var u = new El::Net::HTTP::URL(url);
        return allowed(u, user_agent);
      }

      //
      // RobotsChecker::Record struct
      //
      inline
      void
      RobotsChecker::Record::clear() throw()
      {
        agents.clear();
        disallow_paths.clear();
        allow_paths.clear();
      }
      
      inline
      bool
      RobotsChecker::Record::filled() const throw()
      {
        return !agents.empty() &&
          (!disallow_paths.empty() || !allow_paths.empty());
      }
    }
  }
}

#endif // _ELEMENTS_EL_NET_HTTP_ROBOTS_HPP_
