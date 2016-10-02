/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
