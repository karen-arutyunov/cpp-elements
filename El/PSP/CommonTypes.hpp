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
