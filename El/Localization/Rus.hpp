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
 * @file Elements/El/Localization/Rus.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOCALIZATION_RUS_HPP_
#define _ELEMENTS_EL_LOCALIZATION_RUS_HPP_

#include <iostream>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/Lang.hpp>

#include <El/Localization/LocObject.hpp>

namespace El
{
  namespace Loc
  {
    class Rus : public virtual LocObjectImpl<3>
    {
    public:
      typedef LocObjectImpl<3> Base;
      
    public:
      Rus(const char* countries_file,
          const char* languages_file,
          const char* words_file)
        throw(InvalidArg, Exception, El::Exception);

      virtual ~Rus() throw();
        
      virtual void plural(const char* word,
                          unsigned long count,
                          std::ostream& ostr)
        throw(UnsupportedWord, El::Exception);

    private:
      
      enum Forms
      {
        ONE,
        TWO,
        MANY
      };
      
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Loc
  {
    //
    // Eng class
    //
    inline
    Rus::Rus(const char* countries_file,
             const char* languages_file,
             const char* words_file)
      throw(InvalidArg, Exception, El::Exception)
        : Base(countries_file, languages_file, words_file)
    {
    }

    inline
    Rus::~Rus() throw()
    {
    }
  }
}

#endif // _ELEMENTS_EL_LOCALIZATION_RUS_HPP_
