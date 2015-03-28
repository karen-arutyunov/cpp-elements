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
