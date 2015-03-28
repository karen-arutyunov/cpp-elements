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
 * @file Elements/El/Python/Sequence.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_SEQUENCE_HPP_
#define _ELEMENTS_EL_PYTHON_SEQUENCE_HPP_

#include <vector>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>

#include <El/Python/RefCount.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Utility.hpp>

namespace El
{
  namespace Python
  {
    class Sequence : public El::Python::ObjectImpl,
                     public std::vector<El::Python::Object_var>
    {
    public:      
      Sequence(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      Sequence() throw(El::Exception);
      virtual ~Sequence() throw() {}

      virtual void constant(bool val) throw();      
      virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      virtual void read(El::BinaryInStream& bstr) throw(El::Exception);      

      template<typename T, typename C>
      void from_container(const C& src)
        throw(El::Python::Exception, El::Exception);

      void from_sequence(PyObject* seq) throw(Exception, El::Exception);

      PyObject* py_size() throw(El::Exception);
      PyObject* py_append(PyObject*) throw(El::Exception);
      PyObject* py_erase(PyObject*) throw(El::Exception);
      PyObject* py_index(PyObject*) throw(El::Exception);
      
      class Type : public El::Python::ObjectTypeImpl<Sequence, Sequence::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        
        static Type instance;
        
        PY_TYPE_METHOD_NOARGS(
          py_size,
          "size",
          "Returns container size");

        PY_TYPE_METHOD_VARARGS(
          py_append,
          "append",
          "Appends new element to sequence");

        PY_TYPE_METHOD_VARARGS(
          py_erase,
          "erase",
          "Erases sequence element");

        PY_TYPE_METHOD_VARARGS(
          py_index,
          "index",
          "Finds the first location of element in sequence");

      private:
        static Py_ssize_t sq_length(PyObject* seq);
        static PyObject* sq_item(PyObject* seq, Py_ssize_t index);
        static PyObject* sq_concat(PyObject* s1, PyObject* s2);
        static PyObject* sq_inplace_concat(PyObject* s1, PyObject* s2);
        static PyObject* sq_repeat(PyObject* seq, Py_ssize_t count);
        static PyObject* sq_inplace_repeat(PyObject* seq, Py_ssize_t count);
        
        static PyObject* sq_slice(PyObject* seq,
                                  Py_ssize_t from,
                                  Py_ssize_t to);
        
        static int sq_contains(PyObject* seq, PyObject* val);
        
        static int sq_ass_slice(PyObject* seq,
                                Py_ssize_t from,
                                Py_ssize_t to,
                                PyObject* val);

        static int sq_ass_item(PyObject* seq, Py_ssize_t index, PyObject* val);

      private:
        PySequenceMethods seq_methods_;
      };
    };

    typedef SmartPtr<Sequence> Sequence_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    template<typename T, typename C>
    void
    Sequence::from_container(const C& src)
      throw(El::Python::Exception, El::Exception)
    {
      resize(src.size());

      iterator dit = begin();
          
      for(typename C::const_iterator it = src.begin(); it != src.end();
          it++, dit++)
      {
        *dit = new T(*it);
      }
    }
    
    inline
    void
    Sequence::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      bstr.write_array(*this);
    }

    inline
    void
    Sequence::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      bstr.read_array(*this);
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_SEQUENCE_HPP_
