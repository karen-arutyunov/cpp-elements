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
 * @file Elements/El/Net/HTTP/Cookies.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string>

#include "Cookies.hpp"

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      void
      CookieList::add(const char* cookies) throw(InvalidArg, El::Exception)
      {
        El::String::ListParser parser(cookies, ";");

        const char* item = 0;
        while((item = parser.next_item()) != 0)
        {
          Cookie cookie;
          const char* eq = strchr(item, '=');
          
          if(eq == 0)
          {
            El::String::Manip::trim(item, cookie.name);
          }
          else
          {
            std::string name(item, eq - item);
            El::String::Manip::trim(name.c_str(), cookie.name);
            El::String::Manip::trim(eq + 1, cookie.value);
          }

          if(cookie.name.empty())
          {
            continue;
          }

          push_back(cookie);
        }
      }

      const char*
      CookieList::most_specific(const char* cookie_name) const
        throw(El::Exception)
      {
        for(const_iterator it = begin(); it != end(); it++)
        {
          if(it->name == cookie_name)
          {
            return it->value.c_str();
          }
        }

        return 0;
      }
    }
  }
}

