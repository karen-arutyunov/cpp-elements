/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
