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
 * @file Elements/El/Hash/Hash.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_HASH_HPP_
#define _ELEMENTS_EL_HASH_HPP_

#include <stdint.h>

#include <string>

#include <El/GCC_Version.hpp>

#if GCC_VERSION >= 40300
#  include <backward/hash_fun.h>
#else
#  include <ext/hash_fun.h>
#endif

#if GCC_VERSION >= 40800
#  include <tr1/functional_hash.h>
#  define FNV_HASH_TYPENAME _Fnv_hash_base
#elif GCC_VERSION >= 40400
#  include <tr1/functional_hash.h>
#  define FNV_HASH_TYPENAME _Fnv_hash
#else
#  include <tr1/functional>
#  define FNV_HASH_TYPENAME Fnv_hash
#endif

#include <El/Exception.hpp>

namespace El
{
  namespace Hash
  {
    struct String
    {
      size_t operator()(const std::string& str) const throw(El::Exception);
      size_t operator()(const std::wstring& str) const throw(El::Exception);
    };

    template<typename TYPE>
    struct Numeric
    {
      size_t operator()(const TYPE& val) const throw();
    };    

    template<std::size_t S>
    struct Fnv_hash : public std::tr1::FNV_HASH_TYPENAME<S>
    {
    };
  }
}

#undef FNV_HASH_TYPENAME

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Hash
  {
    inline
    size_t
    String::operator()(const std::string& str) const throw(El::Exception)
    {
      return __gnu_cxx::__stl_hash_string(str.c_str());
    }
    
    inline
    size_t
    String::operator()(const std::wstring& str) const throw(El::Exception)
    {
      const wchar_t* __s = str.c_str();
      size_t __h = 0;
      
      for ( ; *__s; ++__s)
      {
        __h = 5*__h + *__s;
      }
  
      return size_t(__h);
    }
    
    template<typename TYPE>
    size_t
    Numeric<TYPE>::operator()(const TYPE& val) const throw()
    {
      return ((size_t)val) & SIZE_MAX;
    }
  }
}

#endif // _ELEMENTS_EL_HASH_HPP_
