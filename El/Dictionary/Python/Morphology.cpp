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
 * @file Elements/El/Dictionary/Python/Morphology.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/RefCount.hpp>

#include "Morphology.hpp"

namespace El
{
  namespace Dictionary
  {
    namespace Morphology
    {
      namespace Python
      { 
        Lemma::Type Lemma::Type::instance;
        Lemma::Word::Type Lemma::Word::Type::instance;

        El::Python::Module el_dictionary_module(
          "el.dictionary",
          "Module containing ElDictionary library types.",
          true);
    
        El::Python::Module el_dictionary_morphology_module(
          "el.dictionary.morphology",
          "Module containing ElDictionary morphology library types.",
          true);
    
        //
        // Lemma class
        //
        Lemma::Lemma(PyTypeObject *type,
                             PyObject *args,
                             PyObject *kwds)
          throw(El::Python::Exception, El::Exception)
            : ObjectImpl(type)
        {
          throw El::Python::Exception(
            "El::Dictionary::Morphology::Python::Lemma::Lemma: not "
            "implemented yet");
          
          // TODO: Complete implementation when required
/*          
          char *kwlist[] = { "norm_form", "word_forms", "lang", NULL};

          PyObject* nf = 0;
          PyObject* wf = 0;
          PyObject* ln = 0;
        
          if(!PyArg_ParseTupleAndKeywords(
               args,
               kwds,
               "OO|O:el.dictionary.morphology.Lemma.Lemma",
               kwlist,
               &nf,
               &wf,
               &ln))
          {
            El::Python::handle_error(
              "El::Dictionary::Morphology::Python::Lemma::Lemma");
          }

          if(!El::Dictionary::Morphology::Python::Lemma::Word::Type::
             check_type(nf))
          {
            El::Python::report_error(
              PyExc_TypeError,
              "norm_form arg expected to be of "
              "el.dictionary.morphology.LemmaWord type",
              "El::Dictionary::Morphology::Python::Lemma::Lemma");
          }

          norm_form_ =
            El::Dictionary::Morphology::Python::Lemma::Word::Type::down_cast(
              nf, true);
          
          if(!PySequence_Check(wf))
          {
            El::Python::report_error(
              PyExc_TypeError,
              "word_forms arg expected to be of sequence type",
              "El::Dictionary::Morphology::Python::Lemma::Lemma");
          }

          sync();
*/
        }

        void
        Lemma::sync_py() throw(El::Python::Exception, El::Exception)
        {
          lang_ = new El::Python::Lang(lang);
          
          norm_form_ =
            new El::Dictionary::Morphology::Python::Lemma::Word(norm_form);
          
          word_forms_ = new El::Python::Sequence();
          word_forms_->reserve(word_forms.size());

          for(El::Dictionary::Morphology::LemmaInfo::WordArray::const_iterator
                it = word_forms.begin(); it != word_forms.end(); it++)
          {
            word_forms_->push_back(
              new El::Dictionary::Morphology::Python::Lemma::Word(*it));
          }
        }
/*
        PyObject*
        Lemma::py_known() throw(El::Exception)
        {
          return PyBool_FromLong(known());
        }
*/        
        //
        // Lemma::Word class
        //
        Lemma::Word::Word(PyTypeObject *type,
                          PyObject *args,
                          PyObject *kwds)
          throw(El::Python::Exception, El::Exception)
            : ObjectImpl(type)
        {
          throw El::Python::Exception(
            "El::Dictionary::Morphology::Python::Lemma::Word::Word: not "
            "implemented yet");
          
          // TODO: Complete implementation when required
        }
        
      }  
    }
  }
}
