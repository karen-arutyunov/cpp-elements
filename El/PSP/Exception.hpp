/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Exception.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_EXCEPTION_HPP_
#define _ELEMENTS_EL_PSP_EXCEPTION_HPP_

#include <El/Exception.hpp>

namespace El
{
  namespace PSP
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, El::ExceptionBase);
  }
}

#endif // _ELEMENTS_EL_PSP_EXCEPTION_HPP_
