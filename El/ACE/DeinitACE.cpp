/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
