/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

#include "Locale.hpp"

namespace El
{
  const Locale Locale::null(El::Lang(El::Lang::EC_NUL),
                            El::Country(El::Country::EC_NUL));

  const Locale Locale::nonexistent(El::Lang(El::Lang::EC_NONEXISTENT),
                                   El::Country(El::Country::EC_NONEXISTENT));
  
  const Locale Locale::nonexistent2(El::Lang(El::Lang::EC_NONEXISTENT2),
                                    El::Country(El::Country::EC_NONEXISTENT2));
}
