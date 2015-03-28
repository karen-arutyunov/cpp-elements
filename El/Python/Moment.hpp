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
 * @file Elements/El/Python/Moment.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_MOMENT_HPP_
#define _ELEMENTS_EL_PYTHON_MOMENT_HPP_

#include <El/Exception.hpp>
#include <El/Moment.hpp>

#include <El/Python/Object.hpp>

namespace El
{
  namespace Python
  {
    class Moment : public El::Python::ObjectImpl,
                   public El::Moment
    {
    public:      
      Moment(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      Moment(const El::Moment& val) throw(El::Exception);
      Moment() throw(El::Exception);

      virtual ~Moment() throw() {}

      virtual CMP_RESULT eq(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      virtual void read(El::BinaryInStream& bstr) throw(El::Exception);

      PyObject* py_iso8601(PyObject* args) throw(El::Exception);
      PyObject* py_rfc0822(PyObject* args) throw(El::Exception);
      PyObject* py_time_value() throw(El::Exception);
      PyObject* py_dense_format() throw(El::Exception);
      
      class Type : public El::Python::ObjectTypeImpl<Moment, Moment::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;
        
        PY_TYPE_METHOD_VARARGS(
          py_iso8601,
          "iso8601",
          "Returns moment value in iso8601 format");
        
        PY_TYPE_METHOD_VARARGS(
          py_rfc0822,
          "rfc0822",
          "Returns moment value in rfc0822 format");
        
        PY_TYPE_METHOD_NOARGS(
          py_time_value,
          "time_value",
          "Returns time value representation");

        PY_TYPE_METHOD_NOARGS(
          py_dense_format,
          "dense_format",
          "Returns time in yyyymmdd.hhmmssuuuuuu");

        PY_TYPE_STATIC_MEMBER(null_, "null");  

      private:
        SmartPtr<Moment> null_;
      };
    };

    typedef SmartPtr<Moment> Moment_var;
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
    // El::Python::Moment::Type class
    //
    inline
    Moment::Moment(const El::Moment& val) throw(El::Exception)
        : ObjectImpl(&Type::instance)
    {
      *static_cast<El::Moment*>(this) = val;
    }
    
    inline
    Moment::Moment() throw(El::Exception) : ObjectImpl(&Type::instance)
    {
    }
    
    inline
    void
    Moment::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      ::El::Moment::write(bstr);
    }
    
    inline
    void
    Moment::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      ::El::Moment::read(bstr);
    }

    //
    // El::Python::Moment class
    //
    inline
    Moment::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Moment, Moment::Type>(
          "el.Moment",
          "Object encapsulating El::Moment functionality"),
          null_(new Moment())
    {
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_MOMENT_HPP_
