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
 * @file Elements/El/Python/Locale.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include "Locale.hpp"

namespace El
{
  namespace Python
  {
    Locale::Type Locale::Type::instance;
    
    Locale::Locale(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : ObjectImpl(type),
          lang(new Lang()),
          country(new Country())
    {
      const char *kwlist[] = { "lang", "country", NULL};
      
      PyObject* l = 0;
      PyObject* c = 0;
        
      if(args)
      {
        if(!PyArg_ParseTupleAndKeywords(args,
                                        kwds,
                                        "|OO:el.Locale.Locale",
                                        (char**)kwlist,
                                        &l,
                                        &c))
        {
          handle_error("El::Python::Locale::Locale");
        }
      }
      
      if(l)
      {
        *lang = *Lang::Type::down_cast(l);
      }

      if(c)
      {
        *country = *Country::Type::down_cast(c);
      }
    }
  
    El::Python::ObjectImpl::CMP_RESULT
    Locale::eq(ObjectImpl* o) throw(Exception, El::Exception)
    {
      Locale* ob = static_cast<Locale*>(o);
      return *ob == *this ? CR_TRUE : CR_FALSE;
    }

    bool
    Locale::operator==(const Locale& val) const throw()
    {
      return *lang == *val.lang && *country == *val.country;
    }
    
    //
    // El::Python::Locale class
    //
    Locale::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Locale, Locale::Type>(
          "el.Locale",
          "Object encapsulating locale properties"),
          null_(new Locale())
    {
    }
  }
}
