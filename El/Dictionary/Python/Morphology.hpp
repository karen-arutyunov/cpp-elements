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
 * @file Elements/El/Dictionary/Python/Morphology.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_DICTIONARY_PYTHON_MORPHOLOGY_HPP_
#define _ELEMENTS_EL_DICTIONARY_PYTHON_MORPHOLOGY_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Sequence.hpp>
#include <El/Python/Lang.hpp>

#include <El/Dictionary/Morphology.hpp>

namespace El
{
  namespace Dictionary
  {
    namespace Morphology
    {
      namespace Python
      { 
        class Lemma : public El::Python::ObjectImpl,
                      public El::Dictionary::Morphology::LemmaInfo
        {
        public:
          Lemma(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Python::Exception, El::Exception);

          Lemma(const El::Dictionary::Morphology::LemmaInfo& val)
            throw(El::Exception);

          virtual ~Lemma() throw() {}

          void sync_py() throw(El::Python::Exception, El::Exception);
          
          class Word : public El::Python::ObjectImpl,
                       public El::Dictionary::Morphology::LemmaInfo::Word
          {
          public:
            Word(PyTypeObject *type, PyObject *args, PyObject *kwds)
              throw(El::Python::Exception, El::Exception);
            
            Word(const El::Dictionary::Morphology::LemmaInfo::Word& val)
              throw(El::Exception);
            
            virtual ~Word() throw() {}
            
            class Type : public El::Python::ObjectTypeImpl<Word, Word::Type>
            {
            public:
              Type() throw(El::Python::Exception, El::Exception);
              static Type instance;

              PY_TYPE_MEMBER_ULONG(id, "id", "Word id");
              PY_TYPE_MEMBER_STRING(text, "text", "Word text", false);
            };
          };

          typedef El::Python::SmartPtr<Word> Word_var;
          
//          PyObject* py_known() throw(El::Exception);
          
          class Type : public El::Python::ObjectTypeImpl<Lemma,
                                                         Lemma::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_MEMBER_OBJECT(lang_,
                                  El::Python::Lang::Type,
                                  "lang",
                                  "Lemma language",
                                  false);
            
            PY_TYPE_MEMBER_OBJECT(
              norm_form_,
              El::Dictionary::Morphology::Python::Lemma::Word::Type,
              "norm_form",
              "Lemma normal form",
              false);
            
            PY_TYPE_MEMBER_OBJECT(
              word_forms_,
              El::Python::Sequence::Type,
              "word_forms",
              "Lemma word forms",
              false);

            PY_TYPE_MEMBER_BOOL(known,
                                "known",
                                "Check if lemma corresponds to known word");

            PY_TYPE_STATIC_MEMBER(GS_NONE_, "GS_NONE");
            PY_TYPE_STATIC_MEMBER(GS_SIMILAR_, "GS_SIMILAR");
            PY_TYPE_STATIC_MEMBER(GS_TRANSFORM_, "GS_TRANSFORM");

          private:
            El::Python::Object_var GS_NONE_;
            El::Python::Object_var GS_SIMILAR_;
            El::Python::Object_var GS_TRANSFORM_;
            /*
            PY_TYPE_METHOD_NOARGS(py_known,
                                  "known",
                                  "Check if lemma corresponds to known word");
            */
          };

        protected:
          El::Dictionary::Morphology::Python::Lemma::Word_var norm_form_;
          El::Python::Sequence_var word_forms_;
          El::Python::Lang_var lang_;
        };
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Dictionary
  {
    namespace Morphology
    {
      namespace Python
      {
        //
        // El::Dictionary::Morphology::Python::Lemma class
        //
        inline
        Lemma::Lemma(const El::Dictionary::Morphology::LemmaInfo& val)
          throw(El::Exception) : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Dictionary::Morphology::LemmaInfo*>(this) = val;
          sync_py();
        }

        //
        // El::Dictionary::Morphology::Python::Lemma::Type class
        //
        inline
        Lemma::Type::Type() throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<Lemma, Lemma::Type>(
              "el.dictionary.morphology.Lemma",
              "Object encapsulating El::Dictionary::Morphology::Lemma "
              "functionality"),
              GS_NONE_(PyLong_FromLong(
                         ::El::Dictionary::Morphology::Lemma::GS_NONE)),
              GS_SIMILAR_(PyLong_FromLong(
                            ::El::Dictionary::Morphology::Lemma::GS_SIMILAR)),
              GS_TRANSFORM_(
                PyLong_FromLong(
                  ::El::Dictionary::Morphology::Lemma::GS_TRANSFORM))
        {
        }

        //
        // El::Dictionary::Morphology::Python::Lemma::Word class
        //
        inline
        Lemma::Word::Word(
          const El::Dictionary::Morphology::LemmaInfo::Word& val)
          throw(El::Exception) : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Dictionary::Morphology::LemmaInfo::Word*>(this) =
            val;
        }
        
        //
        // El::Dictionary::Morphology::Python::Lemma::Word::Type class
        //
        inline
        Lemma::Word::Type::Type()
          throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<Word, Word::Type>(
              "el.dictionary.morphology.LemmaWord",
              "Object encapsulating El::Dictionary::Morphology::LemmaWord "
              "functionality")
        {
        }

      }
    }
  }
}

#endif // _ELEMENTS_EL_DICTIONARY_PYTHON_MORPHOLOGY_HPP_
