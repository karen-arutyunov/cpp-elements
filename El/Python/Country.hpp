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
 * @file Elements/El/Python/Country.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_COUNTRY_HPP_
#define _ELEMENTS_EL_PYTHON_COUNTRY_HPP_

#include <El/Exception.hpp>
#include <El/Country.hpp>

#include <El/Python/Object.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Sequence.hpp>

namespace El
{
  namespace Python
  {
    class Country : public El::Python::ObjectImpl,
                    public El::Country
    {
    public:      
      Country(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      Country(const El::Country& val) throw(El::Exception);
      Country() throw(El::Exception);

      virtual ~Country() throw() {}

      virtual CMP_RESULT eq(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      virtual void read(El::BinaryInStream& bstr) throw(El::Exception);

      PyObject* py_el_code() throw(El::Exception);
      PyObject* py_l2_code(PyObject* args) throw(El::Exception);
      PyObject* py_l3_code(PyObject* args) throw(El::Exception);
      PyObject* py_name() throw(El::Exception);
      PyObject* py_description() throw(El::Exception);
      
      class Type : public El::Python::ObjectTypeImpl<Country, Country::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;
        
        PY_TYPE_METHOD_NOARGS(
          py_el_code,
          "el_code",
          "Returns country el-code");

        PY_TYPE_METHOD_VARARGS(
          py_l2_code,
          "l2_code",
          "Returns country in 2-letter form");

        PY_TYPE_METHOD_VARARGS(
          py_l3_code,
          "l3_code",
          "Returns country in 3-letter form");

        PY_TYPE_METHOD_NOARGS(
          py_name,
          "name",
          "Returns country name");

        PY_TYPE_METHOD_NOARGS(
          py_description,
          "description",
          "Returns country in <int code> (<l3 code>, <name>) form");

        PY_TYPE_STATIC_MEMBER(null_, "null");
        PY_TYPE_STATIC_MEMBER(nonexistent_, "nonexistent");
        PY_TYPE_STATIC_MEMBER(all_, "all");

      private:
        SmartPtr<Country> null_;
        SmartPtr<Country> nonexistent_;
        El::Python::Sequence_var all_;
      };

    };

    typedef SmartPtr<Country> Country_var;
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
    // El::Python::Country::Type class
    //
    inline
    Country::Country(const El::Country& val) throw(El::Exception)
        : ObjectImpl(&Type::instance)
    {
      *static_cast<El::Country*>(this) = val;
    }
    
    inline
    Country::Country() throw(El::Exception) : ObjectImpl(&Type::instance)
    {
    }
    
    inline
    void
    Country::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      ::El::Country::write(bstr);
    }
    
    inline
    void
    Country::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      ::El::Country::read(bstr);
    }    
  }
}

#endif // _ELEMENTS_EL_PYTHON_COUNTRY_HPP_
