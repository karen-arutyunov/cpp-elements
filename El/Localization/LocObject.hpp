/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Localization/LocObject.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOCALIZATION_LOCOBJECT_HPP_
#define _ELEMENTS_EL_LOCALIZATION_LOCOBJECT_HPP_

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <ext/hash_map>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/String/LightString.hpp>
#include <El/String/Manip.hpp>
#include <El/Country.hpp>
#include <El/Lang.hpp>
#include <El/Hash/Hash.hpp>

namespace El
{
  namespace Loc
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    EL_EXCEPTION(UnsupportedLanguage, Exception);
    EL_EXCEPTION(UnsupportedWord, El::Loc::Exception);
    EL_EXCEPTION(UnsupportedCountry, El::Loc::Exception);

    class LocObject : public virtual El::RefCount::Interface
    {
    public:

      virtual void plural(const char* word,
                          unsigned long count,
                          std::ostream& ostr)
        throw(UnsupportedWord, El::Exception) = 0;

      virtual void country(const El::Country& country,
                           std::ostream& ostr) const
        throw(UnsupportedCountry, El::Exception) = 0;
      
      virtual void language(const El::Lang& language,
                            std::ostream& ostr) const
        throw(UnsupportedLanguage, El::Exception) = 0;
      
      virtual ~LocObject() throw();
    };

    typedef El::RefCount::SmartPtr<LocObject> LocObject_var;
    
    template<const unsigned long WORD_VARIANTS>
    class LocObjectImpl : public virtual LocObject,
                          public virtual El::RefCount::DefaultImpl<>
    {
    public:
      LocObjectImpl(const char* countries_file,
                    const char* languages_file,
                    const char* words_file)
        throw(InvalidArg, Exception, El::Exception);

      virtual ~LocObjectImpl() throw();

      virtual void country(const El::Country& country,
                           std::ostream& ostr) const
        throw(UnsupportedCountry, El::Exception);

      virtual void language(const El::Lang& language,
                            std::ostream& ostr) const
        throw(UnsupportedLanguage, El::Exception);

    protected:
      void read_countries(const char* countries_file)
        throw(InvalidArg, Exception, El::Exception);
      
      void read_languages(const char* languages_file)
        throw(InvalidArg, Exception, El::Exception);
      
      void read_words(const char* words_file)
        throw(InvalidArg, Exception, El::Exception);

    protected:

      typedef __gnu_cxx::hash_map<El::Country::ElCode,
                                  El::String::LightString,
                                  El::Hash::Numeric<El::Country::ElCode> >
      CountryMap;
      
      typedef __gnu_cxx::hash_map<El::Lang::ElCode,
                                  El::String::LightString,
                                  El::Hash::Numeric<El::Lang::ElCode> >
      LanguageMap;
      
      struct Word
      {
        El::String::LightString variants[WORD_VARIANTS];
      };
      
      typedef __gnu_cxx::hash_map<El::String::LightString,
                                  Word,
                                  El::Hash::LightString>
      WordMap;

      CountryMap countries_;
      LanguageMap languages_;
      WordMap words_;
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
    // LocObject class
    //
    inline
    LocObject::~LocObject() throw()
    {
    }
    
    //
    // LocObjectImpl class
    //
    template<const unsigned long WORD_VARIANTS>
    LocObjectImpl<WORD_VARIANTS>::LocObjectImpl(const char* countries_file,
                                                const char* languages_file,
                                                const char* words_file)
      throw(InvalidArg, Exception, El::Exception)
    {
      read_countries(countries_file);
      read_languages(languages_file);
      read_words(words_file);
    }

    template<const unsigned long WORD_VARIANTS>
    void
    LocObjectImpl<WORD_VARIANTS>::read_countries(
      const char* countries_file)
      throw(InvalidArg, Exception, El::Exception)
    {
      std::fstream istr(countries_file, std::ios::in);

      if(!istr.is_open())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::LocObjectImpl::read_countries: failed to open "
             << countries_file;

        throw InvalidArg(ostr.str());
      }

      std::string bad_line;
      std::string line;
      while(bad_line.empty() && std::getline(istr, line))
      {
        std::string trimmed;
        El::String::Manip::trim(line.c_str(), trimmed);

        if(trimmed.empty() || trimmed[0] == '#')
        {
          continue;
        }

        size_t pos = strcspn(trimmed.c_str(), " \t");
        size_t pos2 = strspn(trimmed.c_str() + pos, " \t");

        El::Country country;

        try
        {
          country = El::Country(trimmed.substr(0, pos).c_str());
        }
        catch(...)
        {
          bad_line = line;
          break;
        }

        countries_[country.el_code()] = trimmed.substr(pos + pos2);
      }

      if(!bad_line.empty())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::LocObjectImpl::read_countries: bad line in file "
             << countries_file << ": " << bad_line;

        throw Exception(ostr.str());
      }
    }

    template<const unsigned long WORD_VARIANTS>
    void
    LocObjectImpl<WORD_VARIANTS>::read_languages(
      const char* languages_file)
      throw(InvalidArg, Exception, El::Exception)
    {
      std::fstream istr(languages_file, std::ios::in);

      if(!istr.is_open())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::LocObjectImpl::read_languages: failed to open "
             << languages_file;

        throw InvalidArg(ostr.str());
      }

      std::string bad_line;
      std::string line;
      while(bad_line.empty() && std::getline(istr, line))
      {
        std::string trimmed;
        El::String::Manip::trim(line.c_str(), trimmed);

        if(trimmed.empty() || trimmed[0] == '#')
        {
          continue;
        }

        size_t pos = strcspn(trimmed.c_str(), " \t");
        size_t pos2 = strspn(trimmed.c_str() + pos, " \t");

        El::Lang language;

        try
        {
          language = El::Lang(trimmed.substr(0, pos).c_str());
        }
        catch(...)
        {
          bad_line = line;
          break;
        }

        languages_[language.el_code()] = trimmed.substr(pos + pos2);
      }

      if(!bad_line.empty())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::LocObjectImpl::read_languages: bad line in file "
             << languages_file << ": " << bad_line;

        throw Exception(ostr.str());
      }
    }

    template<const unsigned long WORD_VARIANTS>
    void
    LocObjectImpl<WORD_VARIANTS>::read_words(const char* words_file)
      throw(InvalidArg, Exception, El::Exception)
    {
      std::fstream istr(words_file, std::ios::in);

      if(!istr.is_open())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::LocObjectImpl::read_words: failed to open "
             << words_file;

        throw InvalidArg(ostr.str());
      }

      std::string bad_line;
      std::string line;
      while(bad_line.empty() && std::getline(istr, line))
      {
        std::string trimmed;
        El::String::Manip::trim(line.c_str(), trimmed);

        if(trimmed.empty() || trimmed[0] == '#')
        {
          continue;
        }

        size_t pos = strcspn(trimmed.c_str(), " \t");
        size_t pos2 = strspn(trimmed.c_str() + pos, " \t");

        std::string word_id = trimmed.substr(0, pos);
        
        if(word_id.empty())
        {
          bad_line = line;
          break;
        }

        std::string tmp = trimmed.substr(pos + pos2);

        typename WordMap::iterator it = words_.insert(
          std::make_pair(word_id.c_str(), Word())).first;
        
        Word& word = it->second;
        
        for(unsigned long i = 0; i < WORD_VARIANTS; i++)
        {
          pos = strcspn(tmp.c_str(), " \t");
          pos2 = strspn(tmp.c_str() + pos, " \t");
          
          std::string str = tmp.substr(0, pos);
          tmp = tmp.substr(pos + pos2);

          if(str.empty())
          {
            bad_line = line;
            break;
          }

          word.variants[i] = str.c_str();
        }

        if(!tmp.empty())
        {
          bad_line = line;
          break;
        }
      }

      if(!bad_line.empty())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::LocObjectImpl::read_words: bad line in file "
             << words_file << ": " << bad_line;

        throw Exception(ostr.str());
      }
    }

    template<const unsigned long WORD_VARIANTS>
    void
    LocObjectImpl<WORD_VARIANTS>::country(const El::Country& country,
                                          std::ostream& ostr) const
      throw(UnsupportedCountry, El::Exception)
    {
      CountryMap::const_iterator it = countries_.find(country.el_code());
      
      if(it == countries_.end())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::LocObjectImpl::country: can't find country "
             << country.l3_code();
        
        throw UnsupportedCountry(ostr.str());
      }
      
      ostr << it->second;
    }

    template<const unsigned long WORD_VARIANTS>
    void
    LocObjectImpl<WORD_VARIANTS>::language(const El::Lang& language,
                                           std::ostream& ostr) const
      throw(UnsupportedLanguage, El::Exception)
    {
      LanguageMap::const_iterator it = languages_.find(language.el_code());
      
      if(it == languages_.end())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::LocObjectImpl::language: can't find language "
             << language.l3_code();
        
        throw UnsupportedLanguage(ostr.str());
      }
      
      ostr << it->second;
    }

    template<const unsigned long WORD_VARIANTS>
    LocObjectImpl<WORD_VARIANTS>::~LocObjectImpl() throw()
    {
    }
  }
}

#endif // _ELEMENTS_EL_LOCALIZATION_LOCOBJECT_HPP_
