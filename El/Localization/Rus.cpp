/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Localization/Rus.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>

#include "Rus.hpp"

namespace El
{
  namespace Loc
  {
    void
    Rus::plural(const char* word, unsigned long count, std::ostream& ostr)
      throw(UnsupportedWord, El::Exception)
    {
      WordMap::const_iterator it = words_.find(word);

      if(it == words_.end())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::Rus::mult: cant find word " << word;
        throw UnsupportedWord(ostr.str());
      }

      unsigned long cnt = count % 10;
      
      ostr << count << " " << it->second.variants[
             cnt == 1 && (count % 11) ? ONE :
             (cnt > 1 && cnt < 5 && (count / 10) != 1 ? TWO : MANY)];      
    }
  }
}
