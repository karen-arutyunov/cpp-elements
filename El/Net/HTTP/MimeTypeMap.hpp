/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
