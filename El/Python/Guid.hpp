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
 * @file Elements/El/Python/Guid.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_GUID_HPP_
#define _ELEMENTS_EL_PYTHON_GUID_HPP_

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/Guid.hpp>

#include <El/Python/Object.hpp>

namespace El
{
  namespace Python
  {
    class Guid : public El::Python::ObjectImpl,
                 public El::Guid
    {
    public:      
      Guid(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      Guid(const Guid& val) throw(El::Exception);
      Guid() throw();

      virtual ~Guid() throw() {}

      virtual CMP_RESULT eq(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      virtual void read(El::BinaryInStream& bstr) throw(El::Exception);

      PyObject* py_generate() throw(El::Exception);
      PyObject* py_string(PyObject* args) throw(El::Exception);

      class Type : public El::Python::ObjectTypeImpl<Guid, Guid::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;
        
        PY_TYPE_METHOD_NOARGS(
          py_generate,
          "generate",
          "Generates new GUID");

        PY_TYPE_METHOD_VARARGS(
          py_string,
          "string",
          "Returns guid string representation");

        PY_TYPE_STATIC_MEMBER(null_, "null");
        PY_TYPE_STATIC_MEMBER(GF_CLASSIC_, "GF_CLASSIC");
        PY_TYPE_STATIC_MEMBER(GF_DENSE_, "GF_DENSE");

        virtual void ready() throw(El::Python::Exception, El::Exception);
        
      private:
        
        El::Python::SmartPtr<Guid> null_;
        El::Python::Object_var GF_CLASSIC_;
        El::Python::Object_var GF_DENSE_;
      };
    };

    typedef El::Python::SmartPtr<Guid> Guid_var;
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
    // El::Python::Guid::Type class
    //
    inline
    Guid::Guid(const Guid& val) throw(El::Exception)
        : ObjectImpl(&Type::instance)
    {
      *static_cast<Guid*>(this) = val;
    }
    
    inline
    Guid::Guid() throw() : ObjectImpl(&Type::instance)
    {
    }

    inline
    void
    Guid::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      ::El::Guid::write(bstr);
    }
    
    inline
    void
    Guid::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      ::El::Guid::read(bstr);
    }
    
    //
    // El::Python::TimeValue class
    //
    inline
    Guid::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Guid, Guid::Type>(
          "el.Guid",
          "Object encapsulating Guid functionality"),
          null_(new Guid())
    {
    }

  }
}

#endif // _ELEMENTS_EL_PYTHON_GUID_HPP_
