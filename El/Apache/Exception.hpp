/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Apache/Exception.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_APACHE_EXCEPTION_HPP_
#define _ELEMENTS_EL_APACHE_EXCEPTION_HPP_

#include <El/Exception.hpp>

namespace El
{
  namespace Apache
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, El::ExceptionBase);
  }
}

#define EL_APACHE_VERSION (AP_SERVER_MAJORVERSION_NUMBER * 10 + AP_SERVER_MINORVERSION_NUMBER)

#if EL_APACHE_VERSION < 24
# define EL_APACHE_LOG_MARK APLOG_MARK
#else
# define EL_APACHE_LOG_MARK __FILE__,__LINE__,APLOG_NO_MODULE
#endif

#endif // _ELEMENTS_EL_APACHE_EXCEPTION_HPP_
