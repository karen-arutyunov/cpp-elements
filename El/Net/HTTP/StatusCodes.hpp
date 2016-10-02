/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/StatusCodes.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_STATUSCODES_HPP_
#define _ELEMENTS_EL_NET_HTTP_STATUSCODES_HPP_

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      enum StatusCode
      {
        SC_CONTINUE = 100,
        SC_SWITCHING_PROTOCOLS = 101,

        SC_OK = 200,
        SC_CREATED = 201,
        SC_ACCEPTED = 202,
        SC_NON_AUTH_INFO = 203,
        SC_NO_CONTENT = 204,
        SC_RESET_CONTENT = 205,
        SC_PARTIAL_CONTENT = 206,

        SC_MULTIPLE_CHOICES = 300,
        SC_MOVED_PERMANENTLY = 301,
        SC_FOUND = 302,
        SC_SEE_OTHER = 303,
        SC_NOT_MODIFIED = 304,
        SC_USE_PROXY = 305,
        SC_TEMPORARY_REDIRECT = 307,

        SC_BAD_REQUEST = 400,
        SC_UNAUTHORIZED = 401,
        SC_PAYMENT_REQUIRED = 402,
        SC_FORBIDDEN = 403,
        SC_NOT_FOUND = 404,
        SC_METHOD_NOT_ALLOWED = 405,
        SC_NOT_ACCEPTABLE = 406,
        SC_PROXY_AUTH_REQUIRED = 407,
        SC_REQUEST_TIMEOUT = 408,
        SC_CONFLICT = 409,
        SC_GONE = 410,
        SC_LENGTH_REQUIRED = 411,
        SC_PRECONDITION_FAILED = 412,
        SC_REQUEST_ENTITY_TOO_LARGE = 413,
        SC_REQUEST_URI_TOO_LONG = 414,
        SC_UNSUPPORTED_MEDIA_TYPE = 415,
        SC_REQUEST_RANGE_NOT_SAT = 416,
        SC_EXPECTATION_FAILED = 417,

        SC_INTERNAL_SERVER_ERROR = 500,
        SC_NOT_IMPLEMENTED = 501,
        SC_BAD_GATEWAY = 502,
        SC_SERVICE_UNAVAILABLE = 503,
        SC_GATEWAY_TIMEOUT = 504,
        SC_HTTP_VERSION_NOT_SUPPORTED = 505
      };

    }
  }
}

#endif // _ELEMENTS_EL_NET_HTTP_STATUSCODES_HPP_
