/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
