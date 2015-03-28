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
 * @file Elements/El/Dictionary/LangDetection.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <stdint.h>

#include <iostream>

#include <google/dense_hash_map>
#include <ext/hash_map>

#include <El/Exception.hpp>
#include <El/Lang.hpp>
#include <El/LightArray.hpp>
#include <El/Utility.hpp>
#include <El/Hash/Hash.hpp>

#include "LangDetection.hpp"

struct CharSubset
{
  wchar_t from;
  wchar_t to;

  CharSubset(wchar_t f, wchar_t t = L'\0') throw();
};

inline
CharSubset::CharSubset(wchar_t f, wchar_t t) throw()
    : from(f),
      to(t ? t : f)
{
}

//
// Taken from:
// http://rocky.uta.edu/doran/charsets/charsets.cgi?map=marc2unicode&table=1
//
CharSubset ALPHABET_ENG[] =
{
  CharSubset(L'\x41', L'\x5A'), CharSubset(L'\x61', L'\x7A'), CharSubset(L'\0')
};

//
// Taken from:
// http://ru.wikipedia.org/wiki/...
//
CharSubset ALPHABET_RUS[] =
{
  CharSubset(L'\x410', L'\x42F'),
  CharSubset(L'\x430', L'\x44F'),
  CharSubset(L'\x401'),
  CharSubset(L'\x451'),
  CharSubset(L'\0')
};

//
// Taken from:
// http://rocky.uta.edu/doran/charsets/charsets.cgi?map=marc2unicode&table=1
// http://www.geocities.com/click2speak/unicode/chars_de.html
//
CharSubset ALPHABET_GER[] =
{
  CharSubset(L'\x41', L'\x5A'),
  CharSubset(L'\x61', L'\x7A'),
  CharSubset(L'\xC4'),
  CharSubset(L'\xD6'),
  CharSubset(L'\xDC'),
  CharSubset(L'\xE4'),
  CharSubset(L'\xF6'),
  CharSubset(L'\xFC'),
  CharSubset(L'\xDF'),
  CharSubset(L'\0')
};

//
// Taken from:
// http://www.tamasoft.co.jp/en/general-info/unicode.html
//
CharSubset ALPHABET_CHI[] =
{
  // CJK
  CharSubset(L'\x31C0', L'\x31FF'),
  CharSubset(L'\x3220', L'\x325F'),
  CharSubset(L'\x3280', L'\x32BF'),
  CharSubset(L'\x337B', L'\x337F'),
  CharSubset(L'\x3400', L'\x4DB5'),
  CharSubset(L'\x4E00', L'\x9FBB'),
  CharSubset(L'\xF900', L'\xFA6B'),

  // Bopomofo
  CharSubset(L'\x3100', L'\x31FF'),
  CharSubset(L'\x31A0', L'\x31BF'),
  
  // Chinese corner tone
  CharSubset(L'\xA700', L'\xA71F'),
  
  CharSubset(L'\0')
};

//
// Taken from:
// http://www.tamasoft.co.jp/en/general-info/unicode.html
//
CharSubset ALPHABET_JPN[] =
{
  // CJK
  CharSubset(L'\x31C0', L'\x31FF'),
  CharSubset(L'\x3220', L'\x325F'),
  CharSubset(L'\x3280', L'\x32BF'),
  CharSubset(L'\x337B', L'\x337F'),
  CharSubset(L'\x3400', L'\x4DB5'),
  CharSubset(L'\x4E00', L'\x9FBB'),
  CharSubset(L'\xF900', L'\xFA6B'),

  // Hiragana
  CharSubset(L'\x3040', L'\x309F'),

  // Katakana
  CharSubset(L'\x30A0', L'\x30FF'),
  CharSubset(L'\x31D0', L'\x32FE'),
  CharSubset(L'\x3300', L'\x3357'),
  CharSubset(L'\xFF66', L'\xFF9F'),

  // Kanbun
  CharSubset(L'\x3180', L'\x319F'),
  
  CharSubset(L'\0')
};

//
// Taken from:
// http://www.tamasoft.co.jp/en/general-info/unicode.html
//
CharSubset ALPHABET_KOR[] =
{
  // CJK
  CharSubset(L'\x31C0', L'\x31FF'),
  CharSubset(L'\x3220', L'\x325F'),
  CharSubset(L'\x3280', L'\x32BF'),
  CharSubset(L'\x337B', L'\x337F'),
  CharSubset(L'\x3400', L'\x4DB5'),
  CharSubset(L'\x4E00', L'\x9FBB'),
  CharSubset(L'\xF900', L'\xFA6B'),

  // Hangul
  CharSubset(L'\x1100', L'\x11FF'),
  CharSubset(L'\x3130', L'\x317F'),
  CharSubset(L'\x3200', L'\x321F'),
  CharSubset(L'\x3260', L'\x327F'),
  CharSubset(L'\xAC00', L'\xD7A3'),
  CharSubset(L'\xFFA0', L'\xFFDF'),

  CharSubset(L'\0')
};

//
// Taken from:
// http://en.wikipedia.org/wiki/Portuguese_alphabet
//
CharSubset ALPHABET_POR[] =
{
  CharSubset(L'\x41', L'\x5A'),
  CharSubset(L'\x61', L'\x7A'),
  CharSubset(L'\xC0', L'\xC3'),
  CharSubset(L'\xE0', L'\xE3'),
  CharSubset(L'\xC7'),
  CharSubset(L'\xE7'),
  CharSubset(L'\xC9', L'\xCA'),
  CharSubset(L'\xE9', L'\xEA'),
  CharSubset(L'\xCD'),
  CharSubset(L'\xED'),
  CharSubset(L'\xD3', L'\xD5'),
  CharSubset(L'\xF3', L'\xF5'),
  CharSubset(L'\xDA'),
  CharSubset(L'\xFA'),
  CharSubset(L'\0')
};

//
// Taken from:
// http://en.wikipedia.org/wiki/Italian_alphabet
//
CharSubset ALPHABET_ITA[] =
{
  CharSubset(L'\x41', L'\x49'),
  CharSubset(L'\x4C', L'\x56'),
  CharSubset(L'\x5A'),
  CharSubset(L'\x61', L'\x69'),
  CharSubset(L'\x6C', L'\x76'),
  CharSubset(L'\x7A'),
  CharSubset(L'\xC8'),
  CharSubset(L'\xE8'),
  CharSubset(L'\xD3'),
  CharSubset(L'\xF3'),
  CharSubset(L'\xCE'),
  CharSubset(L'\xEE'),
  CharSubset(L'\xCC'),
  CharSubset(L'\xEC'),
  CharSubset(L'\0')
};

//
// Taken from:
// http://en.wikipedia.org/wiki/Spanish_orthography
//
CharSubset ALPHABET_SPA[] =
{
  CharSubset(L'\x41', L'\x5A'),
  CharSubset(L'\x61', L'\x7A'),
  CharSubset(L'\xC1'),
  CharSubset(L'\xE1'),
  CharSubset(L'\xC9'),
  CharSubset(L'\xE9'),
  CharSubset(L'\xCD'),
  CharSubset(L'\xED'),
  CharSubset(L'\xD3'),
  CharSubset(L'\xF3'),
  CharSubset(L'\xDA'),
  CharSubset(L'\xFA'),
  CharSubset(L'\xDC'),
  CharSubset(L'\xFC'),
  CharSubset(L'\xD1'),
  CharSubset(L'\xF1'),
  CharSubset(L'\0')
};

//
// Taken from:
// http://en.wikipedia.org/wiki/Romanian_alphabet
//
CharSubset ALPHABET_RUM[] =
{
  CharSubset(L'\x41', L'\x5A'),
  CharSubset(L'\x61', L'\x7A'),
  CharSubset(L'\x102', L'\x103'),
  CharSubset(L'\xC2'),
  CharSubset(L'\xE2'),
  CharSubset(L'\xCE'),
  CharSubset(L'\xEE'),
  CharSubset(L'\x218', L'\x21B'),
  CharSubset(L'\x15E', L'\x15F'),
  CharSubset(L'\x162', L'\x163'),
  CharSubset(L'\0')
};

//
// Taken from:
// http://www.phespirit.info/alphabet/slovak.htm
//
CharSubset ALPHABET_SLO[] =
{
  CharSubset(L'\x41', L'\x5A'),
  CharSubset(L'\x61', L'\x7A'),
  CharSubset(L'\xC1'),
  CharSubset(L'\xE1'),
  CharSubset(L'\xC4'),
  CharSubset(L'\xE4'),
  CharSubset(L'\x10C', L'\x10F'),
  CharSubset(L'\xC9'),
  CharSubset(L'\xE9'),
  CharSubset(L'\x139', L'\x13A'),
  CharSubset(L'\x13D', L'\x13E'),
  CharSubset(L'\x147', L'\x148'),
  CharSubset(L'\xD3', L'\xD4'),
  CharSubset(L'\xF3', L'\xF4'),
  CharSubset(L'\x154', L'\x155'),
  CharSubset(L'\x160', L'\x161'),
  CharSubset(L'\x164', L'\x165'),
  CharSubset(L'\xDA'),
  CharSubset(L'\xFA'),
  CharSubset(L'\xDD'),
  CharSubset(L'\xFD'),
  CharSubset(L'\x17D', L'\x17E'),
  CharSubset(L'\x1F1', L'\x1F3'),
  CharSubset(L'\x1C4', L'\x1C4'),
  CharSubset(L'\0')
};

//
// Taken from:
// http://en.wikipedia.org/wiki/Turkish_alphabet
//
CharSubset ALPHABET_TUR[] =
{
  CharSubset(L'\x41', L'\x50'),
  CharSubset(L'\x52', L'\x56'),
  CharSubset(L'\x59'),
  CharSubset(L'\x5A'),
  CharSubset(L'\x61', L'\x70'),  
  CharSubset(L'\x72', L'\x76'),
  CharSubset(L'\x79'),
  CharSubset(L'\x7A'),
  CharSubset(L'\xC7'),
  CharSubset(L'\xD6'),
  CharSubset(L'\xDC'),
  CharSubset(L'\xE7'),
  CharSubset(L'\xF6'),
  CharSubset(L'\xFC'),
  CharSubset(L'\x11E', L'\x11F'),
  CharSubset(L'\x130', L'\x131'),
  CharSubset(L'\x15E', L'\x15F'),
  CharSubset(L'\0')
};

struct Alphabet
{
  El::Lang lang;
  CharSubset* charset;
};

Alphabet ALPHABETS[] =
{
  //
  // Order matter: should go in order of popularity decrease
  // http://www.nicemice.net/amc/tmp/lang-pop.var
  //
  { El::Lang(El::Lang::EC_CHI), ALPHABET_CHI },
  { El::Lang(El::Lang::EC_ENG), ALPHABET_ENG },
  { El::Lang(El::Lang::EC_SPA), ALPHABET_SPA },
  { El::Lang(El::Lang::EC_RUS), ALPHABET_RUS },
  { El::Lang(El::Lang::EC_POR), ALPHABET_POR },  
  { El::Lang(El::Lang::EC_JPN), ALPHABET_JPN },
  { El::Lang(El::Lang::EC_GER), ALPHABET_GER },  
  { El::Lang(El::Lang::EC_KOR), ALPHABET_KOR },
  { El::Lang(El::Lang::EC_TUR), ALPHABET_TUR },
  { El::Lang(El::Lang::EC_ITA), ALPHABET_ITA },
  { El::Lang(El::Lang::EC_RUM), ALPHABET_RUM },
  { El::Lang(El::Lang::EC_SLO), ALPHABET_SLO },
  
  { El::Lang::null, 0 }
};

typedef El::LightArray<El::Lang, uint16_t> LangArray;

class CharLanguageMap : public __gnu_cxx::hash_map<wchar_t,
                                                   LangArray,
                                                   El::Hash::Numeric<wchar_t> >
{
public:
  CharLanguageMap(Alphabet* alphabets) throw(El::Exception);
};

CharLanguageMap::CharLanguageMap(Alphabet* alphabets) throw(El::Exception)
{
  while(alphabets->charset != 0)
  {
    const El::Lang& lang = alphabets->lang;
    const CharSubset* charset = alphabets->charset;

    while(charset->from != L'\0')
    {
      for(wchar_t chr = charset->from; chr <= charset->to; chr++)
      {
        LangArray& langs = (*this)[chr];
        unsigned short i = 0;
        
        for(; i < langs.size() && langs[i] != lang; i++);

        if(i == langs.size())
        {
          langs.resize(i + 1);
          langs[i] = lang;
        }
      }
      
      charset++;
    }
    
    alphabets++;
  } 
}

namespace
{
  const CharLanguageMap CHAR_LANGUAGE_MAP(ALPHABETS);
}

class LangCounterMap :
  public google::dense_hash_map<El::Lang, unsigned long, El::Hash::Lang>
{
public:
  LangCounterMap() throw(El::Exception);
};

LangCounterMap::LangCounterMap() throw(El::Exception)
{
  set_deleted_key(El::Lang::nonexistent);
  set_empty_key(El::Lang::nonexistent2);
}
      
namespace El
{
  namespace Dictionary
  {
    namespace LangDetection
    {
      bool
      supported(const El::Lang& lang) throw()
      {
        unsigned long i = 0;
        unsigned long count = sizeof(ALPHABETS) / sizeof(ALPHABETS[0]);
        
        for(; i < count && ALPHABETS[i].lang != lang; i++);
        return i < count;        
      }
        
      unsigned long
      popularity_index(const El::Lang& lang) throw()
      {
        unsigned long i = 0;
        unsigned long count = sizeof(ALPHABETS) / sizeof(ALPHABETS[0]);
        
        for(; i < count && ALPHABETS[i].lang != lang; i++);
        return i < count ? i : (i + lang.el_code());
      }

      static
      bool
      language_counters(const wchar_t* word, LangCounterMap& lang_counters)
        throw(El::Exception)
      {
        if(word == 0 || *word == L'\0')
        {
          return false;
        }

        unsigned long interword_chars = 0;
          
        for(const wchar_t* ptr = word; *ptr != L'\0'; ptr++)
        {
          CharLanguageMap::const_iterator it = CHAR_LANGUAGE_MAP.find(*ptr);
            
          if(it == CHAR_LANGUAGE_MAP.end())
          {
            if((El::String::Unicode::CharTable::el_categories(*ptr) &
                El::String::Unicode::EC_INTERWORD) == 0)
            {  
              return false;
            }

            interword_chars++;
          }
          else
          {
            const ::LangArray& langs = it->second;

            for(unsigned long i = 0; i < langs.size(); i++)
            {
              const El::Lang& lang = langs[i];
              LangCounterMap::iterator lit = lang_counters.find(lang);

              if(lit == lang_counters.end())
              {
                lang_counters[lang] = 1;
              }
              else
              {
                lit->second++;
              }
            }
          }
        }

        unsigned long chars_required = wcslen(word) - interword_chars;
        
        if(chars_required == 0)
        {
          return false;
        }

        for(LangCounterMap::iterator lit = lang_counters.begin();
            lit != lang_counters.end(); lit++)
        {
          if(lit->second < chars_required)
          {
            lang_counters.erase(lit);
          }
        }

        return true;
      }

      void
      languages(const wchar_t* word, LangArray& langs) throw(El::Exception)
      {
        langs.clear();
        
        LangCounterMap lang_counters;

        if(language_counters(word, lang_counters))
        {
          langs.reserve(lang_counters.size());

          for(LangCounterMap::const_iterator it = lang_counters.begin();
              it != lang_counters.end(); ++it)
          {
            langs.push_back(it->first);
          }
        }
      }
      
      El::Lang
      language(const wchar_t* word, const El::Lang& hint) throw(El::Exception)
      {
        LangCounterMap lang_counters;

        if(!language_counters(word, lang_counters))
        {
          return El::Lang::null;
        }

        if(lang_counters.size() == 1)
        {
          return lang_counters.begin()->first;
        }

        if(hint != El::Lang::null)
        {
          LangCounterMap::iterator lit = lang_counters.find(hint);

          if(lit != lang_counters.end())
          {
            return hint;
          }
        }
        
        if(lang_counters.size())
        {
          // Considering most popular

          Alphabet* alphabet = ALPHABETS;
          
          for(; alphabet->charset != 0 &&
                lang_counters.find(alphabet->lang) == lang_counters.end();
              alphabet++);

          if(alphabet->charset != 0)
          {
            return alphabet->lang;
          }
        }

        return El::Lang::null;
      }
    }
  }
}
