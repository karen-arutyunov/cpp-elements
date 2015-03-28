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
 * @file Elements/El/Net/HTTP/MimeTypeMap.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_MIMETYPEMAP_HPP_
#define _ELEMENTS_EL_NET_HTTP_MIMETYPEMAP_HPP_

#include <string>

#include <ext/hash_map>

#include <El/Exception.hpp>
#include <El/Net/HTTP/Exception.hpp>
#include <El/Hash/Hash.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      class MimeTypeMap :
        public __gnu_cxx::hash_map<std::string, std::string, El::Hash::String>
      {
      public:
        
        MimeTypeMap() throw(Exception, El::Exception) {}
        MimeTypeMap(const char* file) throw(Exception, El::Exception);        
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
    }
  }
}
#endif // _ELEMENTS_EL_NET_HTTP_MIMETYPEMAP_HPP_
