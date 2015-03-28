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
 * @file Elements/El/Python/Sequence.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <sstream>

#include <El/Python/Utility.hpp>
#include <El/Python/RefCount.hpp>

#include "Sequence.hpp"

namespace El
{
  namespace Python
  {
    Sequence::Type Sequence::Type::instance;
    
    //
    // El::Python::Sequence class
    //
    Sequence::Sequence(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception) : ObjectImpl(type)
    {
    }    
  
    Sequence::Sequence() throw(El::Exception) : ObjectImpl(&Type::instance)
    {
    }    

    void
    Sequence::from_sequence(PyObject* seq) throw(Exception, El::Exception)
    {
      if(!PySequence_Check(seq))
      {
        throw Exception("El::Python::Sequence::from_sequence: "
                        "argument of non sequence type");
      }

      clear();
      
      size_t len = PySequence_Length(seq);
      reserve(len);
      
      for(size_t i = 0; i < len; i++)
      {
        push_back(PySequence_GetItem(seq, i));
      }      
    }
    
    void
    Sequence::constant(bool val) throw()
    {
      ObjectImpl::constant(val);

      for(iterator i(begin()), e(end()); i != e; ++i)
      {
        PyObject* p = i->in();

        if(el_based_object(p))
        {
          static_cast<ObjectImpl*>(p)->constant(val);
        }
        
      }
    }
    
    PyObject*
    Sequence::py_size() throw(El::Exception)
    {
      return Py_BuildValue("k", size());
    }

    PyObject*
    Sequence::py_append(PyObject* args) throw(El::Exception)
    {
      if(constant_)
      {
        set_runtime_error("el.Sequence.append failed as object is constant");
        return 0;
      }
        
      PyObject* val = 0;
      if(!PyArg_ParseTuple(args, "O:el.Sequence.append", &val))
      {
        El::Python::handle_error("El::Pythong::Sequence::py_append");
      }

      El::Python::Object_var i(El::Python::add_ref(val));
      push_back(i);
      return i.retn();
    }
    
    PyObject*
    Sequence::py_erase(PyObject* args) throw(El::Exception)
    {
      if(constant_)
      {
        set_runtime_error("el.Sequence.erase failed as object is constant");
        return 0;
      }
        
      unsigned long index = 0;
        
      if(!PyArg_ParseTuple(args, "k:el.Sequence.erase", &index))
      {
        El::Python::handle_error( "El::Pythong::Sequence::py_erase");
      }

      if(index >= size())
      {
        El::Python::report_error(PyExc_ValueError,
                                 "index is out of range",
                                 "El::Python::Sequence::py_erase");
      }

      erase(begin() + index);
      return Py_None;
    }
    
    PyObject*
    Sequence::py_index(PyObject* args) throw(El::Exception)
    {
      PyObject* obj = 0;
      if(!PyArg_ParseTuple(args, "O:el.Sequence.index", &obj))
      {
        El::Python::handle_error( "El::Pythong::Sequence::py_index");
      }

      size_t i = 0;
      for(const_iterator it = begin(); it != end(); it++, i++)
      {
        int cmp = PyObject_RichCompareBool(it->in(), obj, Py_EQ);
        
        if(cmp > 0)
        {
          return PyInt_FromLong(i);
        }
        else if(cmp < 0)
        {
          El::Python::handle_error("El::Python::Sequence::py_index");
        }
      }
      
      El::Python::report_error(PyExc_ValueError,
                               "object not in sequence",
                               "El::Python::Sequence::py_index");
      return Py_None; // Just to avoid compile warning
    }
    
    //
    // El::Python::Sequence::Type class
    //
    Sequence::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Sequence, Sequence::Type>(
          "el.Sequence", "Sequence class")
    {
      memset(&seq_methods_, 0, sizeof(seq_methods_));
      
      seq_methods_.sq_length = &sq_length;
      seq_methods_.sq_item = &sq_item;
      seq_methods_.sq_concat = &sq_concat;
      seq_methods_.sq_inplace_concat = &sq_inplace_concat;
      seq_methods_.sq_repeat = &sq_repeat;
      seq_methods_.sq_inplace_repeat = &sq_inplace_repeat;
      seq_methods_.sq_slice = &sq_slice;
      seq_methods_.sq_contains = &sq_contains;
      seq_methods_.sq_ass_item = &sq_ass_item;
      seq_methods_.sq_ass_slice = &sq_ass_slice;

      tp_as_sequence = &seq_methods_;

      tp_flags |= Py_TPFLAGS_HAVE_SEQUENCE_IN;
    }
    
    Py_ssize_t
    Sequence::Type::sq_length(PyObject* seq)
    {
      return static_cast<Sequence*>(seq)->size();
    }
    
    PyObject*
    Sequence::Type::sq_item(PyObject* seq, Py_ssize_t index)
    {
      Sequence* sequence = static_cast<Sequence*>(seq);

      if(index >= 0 && (size_t)index < sequence->size())
      {
        return (*sequence)[index].add_ref();
      }

      std::ostringstream ostr;
      ostr << "out of range call for object of " << instance.tp_name
           << " type";
      
      set_index_error(ostr.str().c_str());
      return 0;
    }
    
    PyObject*
    Sequence::Type::sq_concat(PyObject* s1, PyObject* s2)
    {
      if(!PySequence_Check(s2))
      {
        set_index_error("non sequence type of second parameter");
        return 0;
      }

      Sequence* seq1 = static_cast<Sequence*>(s1);
        
      Sequence* seq = static_cast<Sequence*>(
        (*s1->ob_type->tp_new)(s1->ob_type, 0, 0));

      size_t len1 = seq1->size();
      size_t len2 = PySequence_Length(s2);
      
      seq->resize(len1 + len2);

      copy(seq1->begin(), seq1->end(), seq->begin());

      for(size_t i = 0; i < len2; i++)
      {
        (*seq)[len1++] = PySequence_GetItem(s2, i);
      }
      
      return seq;
    }
    
    PyObject*
    Sequence::Type::sq_inplace_concat(PyObject* s1, PyObject* s2)
    {
      if(!PySequence_Check(s2))
      {
        set_index_error("non sequence type of second parameter");
        return 0;
      }

      Sequence* seq1 = static_cast<Sequence*>(s1);

      if(seq1->is_constant())
      {
        set_runtime_error("el.Sequence inplace concatenation failed as object "
                          "is constant");
        return 0;
      }

      size_t len1 = seq1->size();
      size_t len2 = PySequence_Length(s2);

      seq1->resize(len1 + len2);

      for(size_t i = 0; i < len2; i++)
      {
        (*seq1)[len1++] = PySequence_GetItem(s2, i);
      }

      return El::Python::add_ref(seq1);
    }
    
    PyObject*
    Sequence::Type::sq_repeat(PyObject* s, Py_ssize_t count)
    {
      Sequence* new_seq = static_cast<Sequence*>(
        (*s->ob_type->tp_new)(s->ob_type, 0, 0));

      if(count > 0)
      {
        Sequence* seq = static_cast<Sequence*>(s);

        new_seq->resize(count * seq->size());

        Sequence::iterator it = new_seq->begin();
        
        for(size_t i = 0; i < (size_t)count;
            i++, it += seq->size())
        {
          copy(seq->begin(), seq->end(), it);
        }
      }
      
      return new_seq;
    }
    
    PyObject*
    Sequence::Type::sq_inplace_repeat(PyObject* s, Py_ssize_t count)
    {
      Sequence* seq = static_cast<Sequence*>(s);

      if(seq->is_constant())
      {
        set_runtime_error("el.Sequence inplace repeat failed as object "
                          "is constant");
        return 0;
      }
      
      if(count < 1)
      {
        seq->clear();
      }

      if(count > 1)
      {
        size_t size = seq->size();
        seq->resize(count * size);

        Sequence::iterator end = seq->begin() + size;
        Sequence::iterator it = end;
        
        for(size_t i = 1; i < (size_t)count; i++, it += size)
        {
          copy(seq->begin(), end, it);
        }
      }
      
      return El::Python::add_ref(seq);
    }
    
    PyObject*
    Sequence::Type::sq_slice(PyObject* s, Py_ssize_t from, Py_ssize_t to)
    {
      Sequence* seq = static_cast<Sequence*>(s);

      if(from < 0)
      {
        from = 0;
      }
      else if((size_t)from > seq->size())
      {
        from = seq->size();
      }

      if(to < from)
      {
        to = from;
      }
      else if((size_t)to > seq->size())
      {
        to = seq->size();
      }

      size_t len = to - from;

      Sequence* new_seq = static_cast<Sequence*>(
        (*seq->ob_type->tp_new)(seq->ob_type, 0, 0));

      if(len)
      {
        new_seq->resize(len);
        std::copy(seq->begin() + from, seq->begin() + to, new_seq->begin());
      }

      return new_seq;
    }

    int
    Sequence::Type::sq_contains(PyObject* s, PyObject* val)
    {
      Sequence* seq = static_cast<Sequence*>(s);

      Sequence::iterator it = seq->begin();
      
      for(; it != seq->end() &&
            !PyObject_RichCompareBool(val, it->in(), Py_EQ); it++);

      return it != seq->end();
    }

    int
    Sequence::Type::sq_ass_slice(PyObject* s,
                                 Py_ssize_t from,
                                 Py_ssize_t to,
                                 PyObject* val)
    {
      Sequence* seq = static_cast<Sequence*>(s);

      if(seq->is_constant())
      {
        set_runtime_error("el.Sequence assign slice failed as object "
                          "is constant");
        return -1;
      }

      if(from < 0)
      {
        from = 0;
      }
      else if((size_t)from > seq->size())
      {
        from = seq->size();
      }

      if(to < from)
      {
        to = from;
      }
      else if((size_t)to > seq->size())
      {
        to = seq->size();
      }

      for(size_t i = from; i < (size_t)to; i++)
      {
        (*seq)[i] = El::Python::add_ref(val);
      }

      return 0;
    }

    int
    Sequence::Type::sq_ass_item(PyObject* s, Py_ssize_t index, PyObject* val)
    {
      Sequence* seq = static_cast<Sequence*>(s);
      
      if(seq->is_constant())
      {
        set_runtime_error("el.Sequence assign item failed as object "
                          "is constant");
        return -1;
      }

      if(index >= 0 && (size_t)index < seq->size())
      {
        (*seq)[index] = El::Python::add_ref(val);
        return 0;
      }

      std::ostringstream ostr;
      ostr << "out of range call for object of " << instance.tp_name
           << " type";
      
      set_index_error(ostr.str().c_str());
      return -1;
    }
    
  }
}
