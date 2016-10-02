/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/CommonTypes.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_COMMONTYPES_HPP_
#define _ELEMENTS_EL_PSP_COMMONTYPES_HPP_

#include <string>
#include <map>
#include <list>

#include <El/Directive.hpp>

namespace El
{
  namespace PSP
  {
    enum ETagCalc
    {
      ETC_NONE,
      ETC_STATIC
    };    

    typedef std::map<std::string, Directive::ArgArray> ArgArrayMap;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace PSP
  {
  }
}

#endif // _ELEMENTS_EL_PSP_COMMONTYPES_HPP_
