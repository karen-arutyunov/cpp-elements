/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

#include <iostream>
#include <cstdlib>

#include <dlfcn.h>


static void
__attribute__((__constructor__))
init()
{
  if (!dlopen("libACE.so", RTLD_LAZY | RTLD_LOCAL))
  {
    std::cerr << "PreloadACE: dlopen(): " << dlerror() << std::endl;
    exit(1);
  }
}
