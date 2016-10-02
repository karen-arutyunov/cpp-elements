/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/XML/Use.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include "Use.hpp"

namespace El
{
  namespace XML
  {
    unsigned long Use::counter_ = 0;
    Use::Mutex Use::lock_;
  }
}
