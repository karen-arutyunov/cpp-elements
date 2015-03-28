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
 * @file Elements/El/Python/Country.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include "Country.hpp"

namespace El
{
  namespace Python
  {
    Country::Type Country::Type::instance;
    
    Country::Country(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : ObjectImpl(type)
    {
      char* country = 0;
      unsigned long cnum = ULONG_MAX;

      if(args)
      {
        if(!PyArg_ParseTuple(args, "|s:el.Country.Country", &country))
        {
          PyErr_Clear();
          
          if(!PyArg_ParseTuple(args, "k:el.Country.Country", &cnum))
          {
            PyErr_Clear();
            
            std::ostringstream ostr;
            throw Exception("El::Python::Country: argument should be of "
                            "string or int type");
          }
        }
      }

      if(cnum != ULONG_MAX)
      {
        *this = El::Country((El::Country::ElCode)cnum);
      }
      else if(country && *country != '\0')
      {
        *this = El::Country(country);
      }
    }
  
    PyObject*
    Country::py_el_code() throw(El::Exception)
    {
      return PyLong_FromLong(el_code());
    }

    PyObject*
    Country::py_l2_code(PyObject* args) throw(El::Exception)
    {
      unsigned char zz = 0;

      if(!PyArg_ParseTuple(args, "|b:el.Country.l2_code", &zz))
      {
        handle_error("El::Python::Country::l2_code");
      }
      
      return PyString_FromString(l2_code(zz));
    }

    PyObject*
    Country::py_l3_code(PyObject* args) throw(El::Exception)
    {
      unsigned char zzz = 0;

      if(!PyArg_ParseTuple(args, "|b:el.Country.l3_code", &zzz))
      {
        handle_error("El::Python::Country::l3_code");
      }
      
      return PyString_FromString(l3_code(zzz));
    }

    PyObject*
    Country::py_name() throw(El::Exception)
    {
      return PyString_FromString(name());
    }

    PyObject*
    Country::py_description() throw(El::Exception)
    {
      std::ostringstream ostr;
      ostr << static_cast<const El::Country&>(*this);
      
      return PyString_FromString(ostr.str().c_str());
    }

    El::Python::ObjectImpl::CMP_RESULT
    Country::eq(ObjectImpl* o) throw(Exception, El::Exception)
    {
      El::Python::Country* ob = static_cast<El::Python::Country*>(o);
      return *ob == *this ? CR_TRUE : CR_FALSE;
    }

    //
    // El::Python::Country class
    //
    Country::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Country, Country::Type>(
          "el.Country",
          "Object encapsulating El::Country functionality"),
          null_(new Country()),
          nonexistent_(new Country(El::Country::EC_NONEXISTENT)),
          all_(new El::Python::Sequence())
    {
      all_->reserve(El::Country::countries_count());

      for(unsigned long i = 1; i <= El::Country::countries_count(); i++)
      {
        El::Python::Country_var country =
          new El::Python::Country(
            El::Country((El::Country::ElCode)(EC_NUL + i)));
        
        all_->push_back(country);
      }

      all_->constant(true);
    }
  }
}
