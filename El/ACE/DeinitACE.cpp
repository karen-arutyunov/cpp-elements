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

#include <ace/Object_Manager.h>

#include <iostream>

#ifndef GCC_VERSION
# define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif /* GCC_VERSION */

static void
__attribute__((__destructor__))
deinit()
{
#if GCC_VERSION < 40300
  ACE_Object_Manager::instance()->fini();
#endif  
}
/*
struct DeinitACE
{
  DeinitACE() 
  {
    std::cerr << "DeinitACE\n";
  }
  
  ~DeinitACE() 
  {
    std::cerr << "~DeinitACE\n"; 
  }
};

static DeinitACE dia;
*/
