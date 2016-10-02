/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Localization/Eng.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>

#include "Eng.hpp"

namespace El
{
  namespace Loc
  {
    void
    Eng::plural(const char* word, unsigned long count, std::ostream& ostr)
      throw(UnsupportedWord, El::Exception)
    {
      WordMap::const_iterator it = words_.find(word);

      if(it == words_.end())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::Eng::plural: cant find word " << word;
        throw UnsupportedWord(ostr.str());
      }
      
      ostr << count << " " << it->second.variants[
        (count % 10) == 1 && (count % 11) ? SINGLE : MULTIPLE];
    }
  }
}
