/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/Exception.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_EXCEPTION_HPP_
#define _ELEMENTS_EL_NET_HTTP_EXCEPTION_HPP_

#include <El/Exception.hpp>
#include <El/Net/Exception.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      EL_EXCEPTION(Exception, ::El::Net::Exception);
      EL_EXCEPTION(InvalidArg, Exception);
    }
  }
}


#endif // _ELEMENTS_EL_NET_HTTP_EXCEPTION_HPP_
