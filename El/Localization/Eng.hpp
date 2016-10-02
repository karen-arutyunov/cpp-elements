/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Localization/Eng.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOCALIZATION_ENG_HPP_
#define _ELEMENTS_EL_LOCALIZATION_ENG_HPP_

#include <iostream>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/Lang.hpp>

#include <El/Localization/LocObject.hpp>

namespace El
{
  namespace Loc
  {
    class Eng : public virtual LocObjectImpl<2>
    {
    public:
      typedef LocObjectImpl<2> Base;
      
    public:
      Eng(const char* countries_file,
          const char* languages_file,
          const char* words_file)
        throw(InvalidArg, Exception, El::Exception);

      virtual ~Eng() throw();
        
      virtual void plural(const char* word,
                          unsigned long count,
                          std::ostream& ostr)
        throw(UnsupportedWord, El::Exception);

    private:
      
      enum Forms
      {
        SINGLE,
        MULTIPLE
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
    Eng::Eng(const char* countries_file,
             const char* languages_file,
             const char* words_file)
      throw(InvalidArg, Exception, El::Exception)
        : Base(countries_file, languages_file, words_file)
    {
    }

    inline
    Eng::~Eng() throw()
    {
    }
  }
}

#endif // _ELEMENTS_EL_LOCALIZATION_ENG_HPP_
