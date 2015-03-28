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
 * @file Elements/El/Localization/Loc.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>
#include <string>
#include <iostream>

#include <El/Lang.hpp>

#include "Loc.hpp"
#include "Eng.hpp"
#include "Rus.hpp"

namespace El
{
  namespace Loc
  {
    El::Loc::Localizer El::Loc::Localizer::instance_;
    
    void
    Localizer::init(const char* localization_dir)
      throw(InvalidArg, Exception, El::Exception)
    {      
      WriteGuard guard(lock_);
      
      loc_objects_[El::Lang::EC_ENG] =
        new Eng(countries_file(El::Lang::EC_ENG, localization_dir).c_str(),
                languages_file(El::Lang::EC_ENG, localization_dir).c_str(),
                words_file(El::Lang::EC_ENG, localization_dir).c_str());

      loc_objects_[El::Lang::EC_RUS] =
        new Rus(countries_file(El::Lang::EC_RUS, localization_dir).c_str(),
                languages_file(El::Lang::EC_RUS, localization_dir).c_str(),
                words_file(El::Lang::EC_RUS, localization_dir).c_str());
    }
    
  }
}
