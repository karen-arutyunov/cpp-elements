/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/NoCopy.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NOCOPY_HPP_
#define _ELEMENTS_EL_NOCOPY_HPP_

namespace El
{
  class NoCopy
  {
  public:
    NoCopy() throw() {}

  private:
    NoCopy(const NoCopy& src);
    void operator=(const NoCopy& src);
  };
}

#endif // _ELEMENTS_EL_NOCOPY_HPP_
