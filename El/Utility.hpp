/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Utility.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_UTILITY_HPP_
#define _ELEMENTS_EL_UTILITY_HPP_

#include <unistd.h>
 
#include <map>
#include <vector>
#include <iostream>

#include <El/Exception.hpp>

namespace El
{
  namespace Utility
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    
    template <typename TYPE>
    void keys(const TYPE& cont, std::vector<typename TYPE::key_type>& keys)
      throw(El::Exception);

    // Returns size in KB of virtual memory used
    size_t mem_used(pid_t pid = 0) throw(Exception, El::Exception);
    
    void dump_mallinfo(std::ostream& ostr) throw(El::Exception);
  }  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Utility
  {
    template <typename TYPE>
    void
    keys(const TYPE& cont, std::vector<typename TYPE::key_type>& keys)
      throw(El::Exception)
    {
      keys.clear();
      keys.reserve(cont.size());

      typename TYPE::const_iterator prev = cont.end();
      for(typename TYPE::const_iterator it = cont.begin(); it != cont.end();
          it++)
      {
        if(prev == cont.end() || prev->first != it->first)
        {
          keys.push_back(it->first);
          prev = it;
        }
      }
    }
    
  }
}

#endif // _ELEMENTS_EL_UTILITY_HPP_
