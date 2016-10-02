/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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

