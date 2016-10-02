/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

#include <El/BinaryStream.hpp>

#include "Guid.hpp"

typedef st_assert<sizeof(uuid_t) == sizeof(El::Guid)>::type dumy;

namespace El
{
  const Guid Guid::null;
}
