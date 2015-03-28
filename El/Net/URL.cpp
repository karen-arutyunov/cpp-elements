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
 * @file Elements/El/Net/URL.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/String/ListParser.hpp>

#include "URL.hpp"

namespace El
{
  namespace Net
  {
    bool
    subdomain(const char* domain, const char* subdomain) throw()
    {
      if(domain == 0 || subdomain == 0 || ip(domain) || ip(subdomain))
      {
        return false;
      }

      size_t dlen = strlen(domain);
      size_t slen = strlen(subdomain);

      if(slen <= dlen || !dlen || !slen)
      {
        return false;
      }

      size_t offset = slen - dlen;
      
      return strcmp(domain, subdomain + offset) == 0 &&
        subdomain[offset - 1] == '.';
    }    
    
    void
    idna_encode(const char* src, std::string& dest)
      throw(Exception, El::Exception)
    {
      std::ostringstream ostr;
      El::String::ListParser parser(src, ".");

      bool is_first = true;
      const char* item = 0;
      
      while((item = parser.next_item()) != 0)
      {
        if(is_first)
        {
          is_first = false;
        }
        else
        {
          ostr << ".";
        }
        
        const unsigned char* p = (const unsigned char*)item;
        for(; *p != '\0' && *p < 0x80; p++);

        if(*p == 0)
        {
          ostr << item;
        }
        else
        { 
          std::string encoded;
        
          El::String::Manip::punycode_encode(item,
                                             encoded,
                                             El::String::Manip::CF_LOWERCASE);
          ostr << "xn--" << encoded;
        }
      }

      dest = ostr.str();
    }

    void
    idna_decode(const char* src, std::string& dest)
      throw(Exception, El::Exception)
    {
      std::ostringstream ostr;
      El::String::ListParser parser(src, ".");

      bool is_first = true;
      const char* item = 0;
      
      while((item = parser.next_item()) != 0)
      {
        if(is_first)
        {
          is_first = false;
        }
        else
        {
          ostr << ".";
        }

        if(strncmp(item, "xn--", 4))
        {
          ostr << item;
        }
        else
        {
          std::string decoded;
          El::String::Manip::punycode_decode(item + 4, decoded);
          ostr << decoded;
        }
        
      }

      dest = ostr.str();
    }
  }
}

