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
 * @file Elements/El/Python/Locale.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_LOCALE_HPP_
#define _ELEMENTS_EL_PYTHON_LOCALE_HPP_

#include <El/Exception.hpp>

#include <El/Python/Object.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Country.hpp>
#include <El/Python/Lang.hpp>

namespace El
{
  namespace Python
  {
    class Locale : public El::Python::ObjectImpl
    {
    public:      
      Locale(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      Locale() throw(El::Exception);
      virtual ~Locale() throw() {}

      virtual CMP_RESULT eq(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      virtual void read(El::BinaryInStream& bstr) throw(El::Exception);
      
      bool operator==(const Locale& val) const throw();

      class Type : public El::Python::ObjectTypeImpl<Locale, Locale::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;
        
        PY_TYPE_MEMBER_OBJECT(lang,
                              El::Python::Lang::Type,
                              "lang",
                              "Locale language",
                              false);

        PY_TYPE_MEMBER_OBJECT(country,
                              El::Python::Country::Type,
                              "country",
                              "Locale country",
                              false);

        PY_TYPE_STATIC_MEMBER(null_, "null");

      private:
        SmartPtr<Locale> null_;
      };

      Lang_var lang;
      Country_var country;
    };

    typedef SmartPtr<Locale> Locale_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    //
    // El::Python::Locale::Type class
    //
    inline
    Locale::Locale() throw(El::Exception)
        : ObjectImpl(&Type::instance),
          lang(new Lang()),
          country(new Country())
    {
    }

    inline
    void
    Locale::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      bstr << lang << country;
    }
    
    inline
    void
    Locale::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      bstr >> lang >> country;
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_LOCALE_HPP_
