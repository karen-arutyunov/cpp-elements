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
 * @file Elements/El/Python/Lang.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include "Lang.hpp"

namespace El
{
  namespace Python
  {
    Lang::Type Lang::Type::instance;
    
    Lang::Lang(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : ObjectImpl(type)
    {
      if(args == 0)
      {
        return;
      }
      
      char* lang = 0;
      unsigned long lnum = ULONG_MAX;
      
      if(args)
      {
        if(!PyArg_ParseTuple(args, "|s:el.Lang.Lang", &lang))
        {
          PyErr_Clear();
          
          if(!PyArg_ParseTuple(args, "k:el.Lang.Lang", &lnum))
          {
            PyErr_Clear();
            
            throw Exception("El::Python::Lang: argument should be of "
                            "string or int type");
          }
        }
      }

      if(lnum != ULONG_MAX)
      {
        *this = El::Lang((El::Lang::ElCode)lnum);
      }
      else if(lang && *lang != '\0')
      {
        *this = El::Lang(lang);
      }
    }
  
    PyObject*
    Lang::py_el_code() throw(El::Exception)
    {
      return PyLong_FromLong(el_code());
    }

    PyObject*
    Lang::py_l2_code(PyObject* args) throw(El::Exception)
    {
      unsigned char zz = 0;

      if(!PyArg_ParseTuple(args, "|b:el.Lang.l2_code", &zz))
      {
        handle_error("El::Python::Lang::l2_code");
      }

      return PyString_FromString(l2_code(zz));
    }

    PyObject*
    Lang::py_l3_code(PyObject* args) throw(El::Exception)
    {
      unsigned char zzz = 0;

      if(!PyArg_ParseTuple(args, "|b:el.Lang.l3_code", &zzz))
      {
        handle_error("El::Python::Lang::l3_code");
      }
      
      return PyString_FromString(l3_code(zzz));
    }

    PyObject*
    Lang::py_num_code() throw(El::Exception)
    {
      return PyLong_FromLong(num_code());
    }

    PyObject*
    Lang::py_name() throw(El::Exception)
    {
      return PyString_FromString(name());
    }

    El::Python::ObjectImpl::CMP_RESULT
    Lang::eq(ObjectImpl* o) throw(Exception, El::Exception)
    {
      El::Python::Lang* ob = static_cast<El::Python::Lang*>(o);
      return *ob == *this ? CR_TRUE : CR_FALSE;
    }

    PyObject*
    Lang::py_description() throw(El::Exception)
    {
      std::ostringstream ostr;
      ostr << static_cast<const El::Lang&>(*this);
      
      return PyString_FromString(ostr.str().c_str());
    }
    
    //
    // El::Python::Lang class
    //
    Lang::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Lang, Lang::Type>(
          "el.Lang",
          "Object encapsulating El::Lang functionality"),
          null_(new Lang()),
          nonexistent_(new Lang(El::Lang::EC_NONEXISTENT)),
          all_(new El::Python::Sequence())
    {
      all_->reserve(El::Lang::languages_count());

      for(unsigned long i = 1; i <= El::Lang::languages_count(); i++)
      {
        El::Python::Lang_var lang =
          new El::Python::Lang(El::Lang((El::Lang::ElCode)(EC_NUL + i)));
        
        all_->push_back(lang);
      }

      all_->constant(true);
    }
  }
}
