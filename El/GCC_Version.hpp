/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
