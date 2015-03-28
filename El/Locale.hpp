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
 * @file Elements/El/Locale.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOCALE_HPP_
#define _ELEMENTS_EL_LOCALE_HPP_

#include <stdint.h>

#include <ostream>
#include <sstream>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>
#include <El/Lang.hpp>
#include <El/Country.hpp>

namespace El
{
  class Locale
  {
  public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);

    El::Lang lang;
    El::Country country;
    
    static const Locale null;

    // To make it possible to use class as a key for google hash maps
    static const Locale nonexistent; 
    static const Locale nonexistent2;

    Locale() throw() {}
    Locale(const Locale& loc) throw() : lang(loc.lang), country(loc.country){}
    Locale(const El::Lang& lval, const El::Country& cval) throw();
    
    bool operator==(const Locale& val) const throw();
    bool operator!=(const Locale& val) const throw();

    void write(El::BinaryOutStream& ostr) const
      throw(Exception, El::Exception);

    void read(El::BinaryInStream& istr) throw(Exception, El::Exception);    
  };  
}

namespace El
{
  namespace Hash
  {
    struct Locale
    {
      size_t operator()(const El::Locale& loc) const
        throw(El::Exception);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  inline
  Locale::Locale(const El::Lang& lval, const El::Country& cval) throw()
      : lang(lval),
        country(cval)
  {
  }
  
  inline
  bool
  Locale::operator==(const Locale& val) const throw()
  {
    return lang == val.lang && country == val.country;
  }

  inline
  bool
  Locale::operator!=(const Locale& val) const throw()
  {
    return lang != val.lang || country != val.country;
  }

  inline
  void
  Locale::write(El::BinaryOutStream& ostr) const
    throw(Exception, El::Exception)
  {
    ostr << lang << country;
  }
  
  inline
  void
  Locale::read(El::BinaryInStream& istr) throw(Exception, El::Exception)
  {
    istr >> lang >> country;
  }
  
}

namespace El
{
  namespace Hash
  {
    inline
    size_t
    Locale::operator()(const El::Locale& loc) const
      throw(El::Exception)
    {
      return (((uint32_t)loc.lang.el_code()) << 16) | loc.country.el_code();
    }
  }
}

#endif // _ELEMENTS_EL_LOCALE_HPP_
