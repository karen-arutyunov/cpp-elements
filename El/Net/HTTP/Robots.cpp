/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/RobotsChecker.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <ctype.h>

#include <string>
#include <sstream>
#include <iomanip>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>

#include <El/Net/HTTP/Headers.hpp>
#include <El/Net/HTTP/Session.hpp>
#include <El/Net/HTTP/StatusCodes.hpp>

#include "Robots.hpp"

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      //
      // RobotsChecker class
      //
      bool
      RobotsChecker::allowed(const El::Net::HTTP::URL* url,
                             const char* user_agent)
        throw(Exception, El::Exception)
      {
        {
          ReadGuard guard(lock_);

          SiteInfoMap::const_iterator i = sites_.find(url->host());
          
          if(i != sites_.end() &&
             i->second.expiration > ACE_OS::gettimeofday())
          {
            return i->second.allowed(url->path(), user_agent);
          }
        }

        SiteInfo site_info;
        
        if(!site_info.load(url->host(),
                           user_agent,
                           request_timeout_,
                           redirects_to_follow_))
        {
          return true;
        }

        {
          WriteGuard guard(lock_);

          ACE_Time_Value current_time = ACE_OS::gettimeofday();

          if(current_time >= next_cleanup_)
          {
            for(SiteInfoMap::iterator i(sites_.begin()), e(sites_.end());
                i != e; )
            {
              SiteInfoMap::iterator cur = i++;

              if(cur->second.expiration <= current_time)
              {
                sites_.erase(cur);
              }
            }

            next_cleanup_ = current_time + cleanup_period_;
          }
          
          site_info.expiration = current_time + entry_timeout_;
          sites_[url->host()] = site_info;
        }
          
        return site_info.allowed(url->path(), user_agent);
      }

      void
      RobotsChecker::dump(std::ostream& ostr) const throw(El::Exception)
      {
        ReadGuard guard(lock_);
        
        for(SiteInfoMap::const_iterator i(sites_.begin()), e(sites_.end());
            i != e; ++i)
        {
          ostr << i->first << std::endl;

          const RecordArray& records = i->second.records;
          
          for(RecordArray::const_iterator j(records.begin()),
                je(records.end()); j != je; ++j)
          {
            const StringArray& agents(j->agents);
            
            for(StringArray::const_iterator i(agents.begin()), e(agents.end());
                i != e; ++i)
            {
              ostr << "  User-agent: " << *i << std::endl;
            }

            const StringArray& disallow_paths(j->disallow_paths);
            
            for(StringArray::const_iterator i(disallow_paths.begin()),
                  e(disallow_paths.end()); i != e; ++i)
            {
              ostr << "  Disallow: " << *i << std::endl;
            }

            const StringArray& allow_paths(j->allow_paths);
            
            for(StringArray::const_iterator i(allow_paths.begin()),
                  e(allow_paths.end()); i != e; ++i)
            {
              ostr << "  Allow: " << *i << std::endl;
            }

            ostr << std::endl;
          }
        }
      }

      //
      // RobotsChecker::SiteInfo class
      //

      bool
      RobotsChecker::SiteInfo::load(const char* host,
                                    const char* user_agent,
                                    const ACE_Time_Value& request_timeout,
                                    size_t redirects_to_follow)
        throw(El::Exception)
      {
        El::Net::HTTP::HeaderList headers;

        headers.add(El::Net::HTTP::HD_ACCEPT, "*/*");
        headers.add(El::Net::HTTP::HD_ACCEPT_ENCODING, "gzip,deflate");
        headers.add(El::Net::HTTP::HD_ACCEPT_LANGUAGE, "en-us");
        headers.add(El::Net::HTTP::HD_USER_AGENT, user_agent);
        
        std::string url = std::string("http://") + host + "/robots.txt";

        try
        {
          El::Net::HTTP::Session session(url.c_str());

          session.open(&request_timeout,
                       &request_timeout,
                       &request_timeout);
          
          session.send_request(El::Net::HTTP::GET,
                               El::Net::HTTP::ParamList(),
                               headers,
                               0,
                               0,
                               redirects_to_follow);
          
          session.recv_response_status();

          if(session.status_code() != El::Net::HTTP::SC_OK)
          {
            return false;
          }

          El::Net::HTTP::Header header;
          while(session.recv_response_header(header));

          std::istream& istr = session.response_body();

          Record record;
          std::string line;
          std::string prev_field;
          std::string unknown_field;
          
          while(std::getline(istr, line))
          {
            std::string::size_type pos = line.find('#');

            if(pos != std::string::npos)
            {
              line.resize(pos);
            }
            
            std::string trimmed;
            El::String::Manip::trim(line.c_str(), trimmed);

            pos = line.find(':');
            
            if(pos == std::string::npos || !pos)
            {
              continue;
            }

            std::string field;
            El::String::Manip::trim(line.c_str(), field, pos);
            
            std::string value;
            El::String::Manip::trim(line.c_str() + pos + 1, value);

            if(strcasecmp(field.c_str(), "User-agent") == 0)
            {
              if(strcasecmp(prev_field.c_str(), "User-agent") &&
                 (record.filled() || !unknown_field.empty()))
              {
                records.push_back(record);
                record.clear();
                unknown_field.clear();
              }
              
              record.agents.push_back(value.empty() ? "*" : value.c_str());
            }
            else if(strcasecmp(field.c_str(), "Disallow") == 0)
            {
              record.disallow_paths.push_back(value);
            }
            else if(strcasecmp(field.c_str(), "Allow") == 0)
            {
              record.allow_paths.push_back(value);
            }
            else
            {
              unknown_field = field;
            }

            prev_field = field;
          }

          if(record.filled())
          {          
            records.push_back(record);
          }
        }
        catch(const El::Net::Exception&)
        {
          return false;
        }
        
        return !records.empty();
      }      
      
      bool
      RobotsChecker::SiteInfo::allowed(const char* path,
                                       const char* user_agent) const throw()
      {
        bool disallowed = false;
        
        for(RecordArray::const_iterator j(records.begin()),
              je(records.end()); j != je && !disallowed; ++j)
        {
          bool agent_match = false;
          const StringArray& agents(j->agents);
            
          for(StringArray::const_iterator i(agents.begin()), e(agents.end());
              i != e && !agent_match; ++i)
          {
            const char* agent = i->c_str();

            agent_match = strcmp(agent, "*") == 0 ||
              strcasestr(user_agent, agent) != 0;
          }

          if(agent_match)
          {
            const StringArray& disallow_paths(j->disallow_paths);
            
            for(StringArray::const_iterator i(disallow_paths.begin()),
                  e(disallow_paths.end()); i != e && !disallowed; ++i)
            {
              const char* sub_path = i->c_str();
              
              disallowed = *sub_path != '\0' &&
                strncasecmp(path, sub_path, strlen(sub_path)) == 0;
            }

            if(disallowed)
            {
              const StringArray& allow_paths(j->allow_paths);
            
              for(StringArray::const_iterator i(allow_paths.begin()),
                    e(allow_paths.end()); i != e; ++i)
              {
                const char* sub_path = i->c_str();

                if(*sub_path == '\0')
                {
                  disallowed = true;
                }
                else if(strncasecmp(path, sub_path, strlen(sub_path)) == 0)
                {
                  disallowed = false;
                }
              }
            }
          }
        }
        
        return !disallowed;
      }      
    }
  }
}

