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
 * @file Elements/El/String/SharedString.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <assert.h>

#include <sstream>
#include <utility>

#include "SharedString.hpp"

namespace El
{
  namespace String
  {
    const char SharedStringManager::EMPTY[] = "";
    
    void
    SharedStringManager::Info::dump(std::ostream& ostr) const
      throw(El::Exception)
    {
      ostr << "\n  strings: " << strings << "\n  string_refs: "
           << string_refs << "\n  mem_usage: " << mem_usage / 1024
           << " KB\n  compression: " << compression;
    }

    //
    // SharedStringManager
    //
    const char*
    SharedStringManager::add(const char* str) throw(El::Exception)
    {
      if(str == 0)
      {
        return 0;
      }

      if(*str == '\0')
      {
        return EMPTY;
      }
      
      size_t len = strlen(str);

      WriteGuard guard(lock_);
      return add(str, len);
    }    
      
    SharedStringManager::Info
    SharedStringManager::info_() const throw()
    {
      Info info;

      // info.mem_usage calculation is too optimistic as do not
      // count hash set overhead
      info.mem_usage = sizeof(*this);
      
      size_t total_string_len = 0;
        
      info.strings = strings_.size();

      for(StringSet::const_iterator it = strings_.begin();
          it != strings_.end(); it++)
      {
        unsigned long refs = *StringSet::buffer(it->c_str());
          
        info.string_refs += refs;
          
        size_t len = it->length() + 1;
        
        info.mem_usage += (len / sizeof(unsigned long) +
                           (len % sizeof(unsigned long) ? 2 : 1)) *
          sizeof(unsigned long);
        
        total_string_len += len * refs;
      }

      assert(info.string_refs == references_);

      info.compression = (float)total_string_len / info.mem_usage;
        
      return info;
    }
    
    void
    SharedStringManager::dump(std::ostream& ostr,
                              unsigned long dump_strings_count)
      const throw(El::Exception)
    {
      ReadGuard guard(lock_);

      Info info = info_();
      info.dump(ostr);
      
      if(dump_strings_count)
      {
        ostr << "\n  values:";
          
        unsigned long dumped = 0;

        for(StringSet::const_iterator it = strings_.begin();
            it != strings_.end() && dumped < dump_strings_count; it++)
        {
          ostr << "\n  " << it->c_str() << ":"
               << *StringSet::buffer(it->c_str());
          
          dumped++;
        }
        
        if(info.strings > dump_strings_count)
        {
          ostr << "\n...";
        }
        
      }
    }
      
    void
    SharedStringManager::optimize_mem_usage() throw(El::Exception)
    {
      WriteGuard guard(lock_);
      strings_.resize(0);
    }
    
  }
}
