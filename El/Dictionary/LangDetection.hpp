/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Dictionary/LangDetection.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_DICTIONARY_LANGDETECTION_HPP_
#define _ELEMENTS_EL_DICTIONARY_LANGDETECTION_HPP_

#include <string>
#include <vector>

#include <El/Exception.hpp>
#include <El/Lang.hpp>
#include <El/String/Manip.hpp>

namespace El
{
  namespace Dictionary
  {
    namespace LangDetection
    {
      EL_EXCEPTION(Exception, El::ExceptionBase);
      EL_EXCEPTION(InvalidArg, Exception);

      El::Lang language(const wchar_t* word,
                        const El::Lang& hint = El::Lang::null)
        throw(El::Exception);
      
      El::Lang language(const char* word,
                        const El::Lang& hint = El::Lang::null)
        throw(El::Exception);

      typedef std::vector<El::Lang> LangArray;
 
      void languages(const wchar_t* word, LangArray& langs)
        throw(El::Exception);

      void languages(const char* word, LangArray& langs) throw(El::Exception);

      // Smaller - more popular
      unsigned long popularity_index(const El::Lang& lang) throw();

      bool supported(const El::Lang& lang) throw();      
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Dictionary
  {
    namespace LangDetection
    {
      inline
      El::Lang
      language(const char* word, const El::Lang& hint) throw(El::Exception)
      {
        std::wstring wword;
        El::String::Manip::utf8_to_wchar(word, wword);
        return language(wword.c_str(), hint);
      }

      inline
      void
      languages(const char* word, LangArray& langs) throw(El::Exception)
      {
        std::wstring wword;
        El::String::Manip::utf8_to_wchar(word, wword);
        languages(wword.c_str(), langs);
      }
    }
  }
}

#endif // _ELEMENTS_EL_DICTIONARY_LANGDETECTION_HPP_
