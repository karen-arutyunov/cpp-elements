/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Localization/Loc.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOCALIZATION_LOC_HPP_
#define _ELEMENTS_EL_LOCALIZATION_LOC_HPP_

#include <iostream>

#include <ext/hash_map>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/Lang.hpp>
#include <El/Country.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Localization/LocObject.hpp>

namespace El
{
  namespace Loc
  {
    class Localizer
    {
    public:
      Localizer() throw(El::Exception);

      void init(const char* localization_dir)
        throw(InvalidArg, Exception, El::Exception);
      
      bool supported(const Lang& lang) throw();

      std::ostream&
      plural(const char* word,
             unsigned long count,
             const Lang& lang,
             std::ostream& ostr) const
        throw(UnsupportedLanguage, UnsupportedWord, El::Exception);

      std::ostream&
      country(const Country& country,
              const Lang& lang,
              std::ostream& ostr) const
        throw(UnsupportedLanguage, UnsupportedCountry, El::Exception);

      std::ostream&
      language(const Lang& language,
               const Lang& lang,
               std::ostream& ostr) const
        throw(UnsupportedLanguage, El::Exception);

      static El::Loc::Localizer& instance() throw();      

    private:

      static std::string words_file(const Lang& lang,
                                    const char* dir) throw(El::Exception);

      static std::string countries_file(const Lang& lang,
                                        const char* dir) throw(El::Exception);
      
      static std::string languages_file(const El::Lang& lang,
                                        const char* dir) throw(El::Exception);
      
    private:

      typedef ACE_RW_Thread_Mutex    Mutex;
      typedef ACE_Read_Guard<Mutex>  ReadGuard;
      typedef ACE_Write_Guard<Mutex> WriteGuard;
      
      mutable Mutex lock_;

      static El::Loc::Localizer instance_;

      typedef __gnu_cxx::hash_map<Lang::ElCode,
                                  LocObject_var,
                                  Hash::Numeric<El::Lang::ElCode> >
      LocObjectMap;

      LocObjectMap loc_objects_;
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
    inline
    Localizer::Localizer() throw(El::Exception)
    {
    }

    inline
    std::ostream&
    Localizer::plural(const char* word,
                      unsigned long count,
                      const Lang& lang,
                      std::ostream& ostr) const
      throw(UnsupportedLanguage, UnsupportedWord, El::Exception)
    {
      ReadGuard guard(lock_);
      
      LocObjectMap::const_iterator it = loc_objects_.find(lang.el_code());
      
      if(it == loc_objects_.end())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::Localizer::plural: language " << lang.l3_code()
             << " is not supported";
        
        throw UnsupportedLanguage(ostr.str());
      }

      it->second->plural(word, count, ostr);
      return ostr;
    }

    inline
    std::ostream&
    Localizer::country(const Country& country,
                       const Lang& lang,
                       std::ostream& ostr) const
      throw(UnsupportedLanguage, UnsupportedCountry, El::Exception)
    {
      ReadGuard guard(lock_);
      
      LocObjectMap::const_iterator it = loc_objects_.find(lang.el_code());
      
      if(it == loc_objects_.end())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::Localizer::country: language " << lang.l3_code()
             << " is not supported";
        
        throw UnsupportedCountry(ostr.str());
      }

      it->second->country(country, ostr);
      return ostr;
    }

    inline
    std::ostream&
    Localizer::language(const Lang& language,
                        const Lang& lang,
                        std::ostream& ostr) const
      throw(UnsupportedLanguage, El::Exception)
    {
      ReadGuard guard(lock_);
      
      LocObjectMap::const_iterator it = loc_objects_.find(lang.el_code());
      
      if(it == loc_objects_.end())
      {
        std::ostringstream ostr;
        ostr << "El::Loc::Localizer::language: language " << lang.l3_code()
             << " is not supported";
        
        throw UnsupportedLanguage(ostr.str());
      }

      it->second->language(language, ostr);
      return ostr;
    }
    
    inline
    bool
    Localizer::supported(const Lang& lang) throw()
    {
      ReadGuard guard(lock_);
      return loc_objects_.find(lang.el_code()) != loc_objects_.end();
    }

    inline
    std::string
    Localizer::words_file(const Lang& lang,
                          const char* dir) throw(El::Exception)
    {
      return std::string(dir) + "/" + lang.l3_code() + "/words.loc";
    }
    
    inline
    std::string
    Localizer::countries_file(const Lang& lang,
                              const char* dir) throw(El::Exception)
    {
      return std::string(dir) + "/" + lang.l3_code() + "/countries.loc";
    }
    
    inline
    std::string
    Localizer::languages_file(const Lang& lang,
                              const char* dir) throw(El::Exception)
    {
      return std::string(dir) + "/" + lang.l3_code() + "/languages.loc";
    }
    
    inline
    El::Loc::Localizer&
    Localizer::instance() throw()
    {
      return instance_;
    }
  }
}

#endif // _ELEMENTS_EL_LOCALIZATION_LOC_HPP_
