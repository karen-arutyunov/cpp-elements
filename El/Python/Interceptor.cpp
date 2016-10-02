/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Interceptor.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>
#include "Interceptor.hpp"

namespace El
{
  namespace Python
  {
    Interceptor::Interruption::Type Interceptor::Interruption::Type::instance;
  }
}
