/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/IO.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_IO_HPP_
#define _ELEMENTS_EL_IO_HPP_

#include <unistd.h> 

namespace El
{
  ssize_t read(int fd, void *buf, size_t count) throw();
  ssize_t write(int fd, const void *buf, size_t count) throw();
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  inline
  ssize_t
  read(int fd, void *buf, size_t count) throw()
  {
    while(true)
    {
      ssize_t size = ::read(fd, buf, count);

      if(size < 0 && errno == EINTR)
      {
        continue;
      }

      return size;
    }
  }

  inline
  ssize_t
  write(int fd, const void *buf, size_t count) throw()
  {
    while(true)
    {
      ssize_t size = ::write(fd, buf, count);

      if(size < 0 && errno == EINTR)
      {
        continue;
      }

      return size;
    }
  }
}

#endif // _ELEMENTS_EL_IO_HPP_
