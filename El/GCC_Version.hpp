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
 * @file GCC_Version.hpp
 * @author askryabin
 * 
 */

#ifndef _ELEMENTS_EL_GCC_VERSION_HPP_
#define _ELEMENTS_EL_GCC_VERSION_HPP_


#  ifndef GCC_VERSION
#   define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#  endif

#  if GCC_VERSION < 40800
#    define CONSTEXPR
#  else
#    define CONSTEXPR constexpr
#  endif

#endif // _ELEMENTS_EL_GCC_VERSION_HPP_
