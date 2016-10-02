/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
