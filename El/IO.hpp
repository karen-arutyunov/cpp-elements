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
