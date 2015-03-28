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
 * @file Elements/El/Net/HTTP/URL.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <ctype.h>

#include <string>
#include <sstream>
#include <iomanip>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>

#include "URL.hpp"

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      URL::URL(const char* url, unsigned long treat_flags)
        throw(InvalidArg, El::Exception)
          : port_(0),
            secure_(false)
      {
        if(url == 0 || *url == '\0')
        {
          throw InvalidArg("El::Net::HTTP::URL::URL: null argument");
        }

        std::string trimmed;

        try
        {
          El::String::Manip::trim(url, trimmed);
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::URL::URL: El::String::Manip::trim failed. "
            "Reason:\n" << e;
          
          throw InvalidArg(ostr.str());
        }

        const char* str = trimmed.c_str();
        
        if(strncasecmp(str, "http://", 7) == 0 ||
           strncasecmp(str, "feed://", 7) == 0)
        {
          secure_ = false;
          str += 7;
        }
        else if(strncasecmp(str, "https://", 8) == 0)
        {
          secure_ = true;
          str += 8;
        }
        else if(strstr(str, "://") != 0)
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::URL::URL: unexpected protocol in '"
               << str << "'";
          
          throw InvalidArg(ostr.str());
        }
        else
        {
          secure_ = false;
        }
        
        const char* p1 = strchr(str, '/');
        const char* p2 = strchr(str, '?');
        const char* p3 = strchr(str, '#');

        const char* host_end = p1;
        host_end = host_end ? (p2 ? std::min(host_end, p2) : host_end) : p2;
        host_end = host_end ? (p3 ? std::min(host_end, p3) : host_end) : p3;        

        std::string host;
        
        if(host_end == 0)
        {
          host.assign(str, strcspn(str, " \t"));
          str = 0;
        }
        else
        {
          host.assign(str, host_end - str);
          str = host_end;
        }

        size_t pos = host.find(':');

        if(pos == std::string::npos)
        {
          port_ = secure_ ? 443 : 80;
        }
        else
        {
          port_ = atol(host.c_str() + pos + 1);
          host.assign(host.c_str(), pos);
        }
        
        try
        {
          El::Net::idna_encode(host.c_str(), idn_host_);        
          El::Net::idna_decode(idn_host_.c_str(), host_);
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::URL::URL: El::Net::idna_* failed. "
            "Reason:\n" << e;
          
          throw InvalidArg(ostr.str());
        }

        if(str)
        {
          std::string treated;
          treat(str, treated, treat_flags);

          str = treated.c_str();

          const char* anchor = strchr(str, '#');
          std::string path_and_query;
            
          if(anchor)
          {
            anchor_ = anchor + 1;
            path_and_query.assign(str, anchor - str);
            str = path_and_query.c_str();
          }
          
          const char* params = strchr(str, '?');

          if(params)
          {
            const char* p = params + 1;
            
            params_.assign(p, strcspn(p, "\t"));
            El::String::Manip::replace(params_, " ", "+");
            
            path_.assign(str, params - str);
          }
          else
          {
            path_.assign(str, strcspn(str, "\t"));
          }

          const char* sp = "%20";
          El::String::Manip::replace(path_, " ", &sp);
        }

        normalize();
      }

      void
      URL::treat(const char* url,
                 std::string& treated,
                 unsigned long flags)
        throw(InvalidArg, El::Exception)
      {
        if(url == 0)
        {
          throw InvalidArg("El::Net::HTTP::URL::treat: url is null");
        }

        const char* str = url;        
        for(; *str != '\0' && isspace(*str); str++);

        if(*str == '\0')
        {
          throw InvalidArg("El::Net::HTTP::URL::treat: url is empty");
        }

        size_t len = strlen(str);
        const char* end = str + len - 1;

        for(; isspace(*end); end--);
        end++;
        
        std::ostringstream ostr;
        
        for(bool param_part = false; str != end; str++)
        {
          char chr = *str;
          
          switch(chr)
          {
          case ' ':
            {
              if(flags & (TFL_SPACE | TFL_NON_MIME))
              {
                ostr << (param_part ? "+" : "%20");
              }
              else
              {
                ostr << chr;
              }
              
              break;
            }
          default:
            {
              if(flags & TFL_NON_MIME)
              {
                if((chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z') ||
                   (chr >= '0' && chr <= '9') || strchr("_*.,-", chr) != 0)
                {
                  ostr << chr;
                  continue;
                }

                if(chr == '%')
                {
                  char buff[3];
                  strncpy(buff, str + 1, 2);
                  buff[2] = '\0';
                  
                  char* endptr = 0;
                  strtoul(buff, &endptr, 16);
                  
                  if(endptr == 0 || *endptr == '\0')
                  {
                    ostr << chr;
                    continue;
                  }
                }

                if(param_part)
                {
                  if(strchr("&=", chr) != 0)
                  {
                    ostr << chr;
                    continue;                
                  }
                }
                else
                {
                  if(strchr("\\/:", chr) != 0)
                  {
                    ostr << chr;
                    continue;                
                  }
                  
                  if(chr == '?')
                  {
                    param_part = true;
                    ostr << chr;
                    continue;
                  }
                }
              }
              else if(flags & TFL_NON_ASCII)
              {
                if((unsigned char)chr < 0x80)
                {
                  if(chr != '\n' && chr != '\r' && chr != '\t')
                  {
                    ostr << chr;
                  }
                  
                  continue;
                }
              }
              else
              {
                ostr << chr;
                continue;
              }
                
              ostr << "%" << std::setw(2) << std::setfill('0') <<
                std::uppercase << std::hex << std::right <<
                (unsigned short)(chr & 0xFF) << std::dec;

              break;
            }
          }
        }
          
        treated = ostr.str();
      }

      std::string
      URL::abs_url(const char* path, bool lax) const
        throw(InvalidArg, El::Exception)
      {
        if(path == 0 || *path == '\0')
        {
          throw InvalidArg("El::Net::HTTP::URL::abs_url: path null or empty");
        }

        const char* proto = strchr(path, ':');
        
        if(proto != 0)
        {
          for(proto--; proto >= path && isalpha(*proto); proto--);

          if(proto < path)
          {
            return path;
          }
        }
        
        if(path[0] == '/')
        {
          if(path[1] == '/')
          {
//            std::cerr << path << " against " << string() << std::endl;
            return schema() + path;
          }
          
          return schema_and_endpoint() + path;
        }

        size_t dirend = path_.rfind('/');
        std::string base = path_.substr(0, dirend);

        const char* ptr = path;
          
        while(*ptr != '\0')
        {
          if(strncmp(ptr, "../", 3) == 0 || strcmp(ptr, "..") == 0)
          {
            ptr += strncmp(ptr, "../", 3) == 0 ? 3 : 2;

            size_t dirend = base.rfind('/');
            
            if(dirend == std::string::npos)
            {
              if(lax)
              {
                continue;
              }
              else
              {
                std::ostringstream ostr;
                ostr << "El::Net::HTTP::URL::abs_url: path '" << path
                     << "' can't be relative for '" << string_ << "'";
                
                throw InvalidArg(ostr.str());
              }
            }

            base = base.substr(0, dirend);
          }
          else if(strncmp(ptr, "./", 2) == 0)
          {
            ptr += 2;
          }
          else if(strcmp(ptr, ".") == 0)
          {
            ptr++;
          }
          else
          {
            break;
          }
        }

        return schema_and_endpoint() + base + "/" + ptr;
      }      

    }
  }
}

