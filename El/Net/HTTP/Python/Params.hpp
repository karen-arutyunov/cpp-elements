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
 * @file Elements/El/Net/HTTP/Python/Params.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_PYTHON_PARAMS_HPP_
#define _ELEMENTS_EL_NET_HTTP_PYTHON_PARAMS_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Sequence.hpp>
#include <El/Python/Lang.hpp>
#include <El/Python/Country.hpp>
#include <El/Python/RefCount.hpp>

#include <El/Net/HTTP/Params.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      namespace Python
      { 
        class Header : public El::Python::ObjectImpl,
                       public El::Net::HTTP::Header
        {
        public:
          Header(PyTypeObject *type = 0,
                 PyObject *args = 0,
                 PyObject *kwds = 0)
            throw(El::Python::Exception, El::Exception);

          Header(const El::Net::HTTP::Header& val) throw(El::Exception);

          virtual ~Header() throw() {}

          virtual CMP_RESULT eq(ObjectImpl* ob)
            throw(Exception, El::Exception);

          virtual void write(El::BinaryOutStream& bstr) const
            throw(El::Exception);
          
          virtual void read(El::BinaryInStream& bstr) throw(El::Exception);
          
          class Type : public El::Python::ObjectTypeImpl<Header, Header::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_MEMBER_STRING(name, "name", "Header name", false);
            PY_TYPE_MEMBER_STRING(value, "value", "Header value", true);
          };
        };

        typedef El::Python::SmartPtr<Header> Header_var;

        class HeaderSeq : public El::Python::Sequence
        {
        public:
          HeaderSeq(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);

          HeaderSeq() throw(El::Exception) : Sequence(&Type::instance, 0, 0) {}

          virtual ~HeaderSeq() throw() {}

          PyObject* py_find(PyObject* args) throw(El::Exception);

          class Type : public El::Python::ObjectTypeImpl<HeaderSeq,
                                                         HeaderSeq::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
        
            PY_TYPE_METHOD_VARARGS(
              py_find,
              "find",
              "Finds the first occurance of header specified");
          };
        };

        typedef El::Python::SmartPtr<HeaderSeq> HeaderSeq_var;        
        
        class Param : public El::Python::ObjectImpl,
                      public El::Net::HTTP::Param
        {
        public:
          Param(PyTypeObject *type = 0, PyObject *args = 0, PyObject *kwds = 0)
            throw(El::Python::Exception, El::Exception);

          Param(const El::Net::HTTP::Param& val) throw(El::Exception);

          virtual ~Param() throw() {}

          virtual CMP_RESULT eq(ObjectImpl* ob)
            throw(Exception, El::Exception);
          
          virtual void write(El::BinaryOutStream& bstr) const
            throw(El::Exception);
          
          virtual void read(El::BinaryInStream& bstr) throw(El::Exception);
          
          class Type : public El::Python::ObjectTypeImpl<Param, Param::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_MEMBER_STRING(name, "name", "Param name", false);
            PY_TYPE_MEMBER_STRING(value, "value", "Param value", true);
          };
        };

        typedef El::Python::SmartPtr<Param> Param_var;        
        
        class ParamSeq : public El::Python::Sequence
        {
        public:
          ParamSeq(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);

          ParamSeq() throw(El::Exception) : Sequence(&Type::instance, 0, 0) {}

          virtual ~ParamSeq() throw() {}

          PyObject* py_find(PyObject* args) throw(El::Exception);
          
          class Type : public El::Python::ObjectTypeImpl<ParamSeq,
                                                         ParamSeq::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
        
            PY_TYPE_METHOD_VARARGS(
              py_find,
              "find",
              "Finds the first occurance of parameter specified");
          };
        };

        typedef El::Python::SmartPtr<ParamSeq> ParamSeq_var;        

        class AcceptLanguage : public El::Python::ObjectImpl,
                               public El::Net::HTTP::AcceptLanguage
        {
        public:
          AcceptLanguage(PyTypeObject *type = 0,
                         PyObject *args = 0,
                         PyObject *kwds = 0)
            throw(El::Python::Exception, El::Exception);

          AcceptLanguage(const El::Net::HTTP::AcceptLanguage& val)
            throw(El::Exception);

          virtual ~AcceptLanguage() throw() {}

          virtual CMP_RESULT eq(ObjectImpl* ob)
            throw(Exception, El::Exception);
          
          virtual void write(El::BinaryOutStream& bstr) const
            throw(El::Exception);
          
          virtual void read(El::BinaryInStream& bstr) throw(El::Exception);

          void sync_from_py() throw();
          
          class Type : public El::Python::ObjectTypeImpl<AcceptLanguage,
                                                         AcceptLanguage::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_MEMBER_OBJECT(language_,
                                  El::Python::Lang::Type,
                                  "language",
                                  "Language object",
                                  false);

            PY_TYPE_MEMBER_OBJECT(country_,
                                  El::Python::Country::Type,
                                  "country",
                                  "Country object",
                                  false);

            PY_TYPE_MEMBER_STRING(subtag, "subtag", "Subtag", true);

            PY_TYPE_MEMBER_FLOAT(qvalue, "qvalue", "Qvalue");
          };

        protected:
          El::Python::Lang_var language_;
          El::Python::Country_var country_;
        };

        typedef El::Python::SmartPtr<AcceptLanguage> AcceptLanguage_var;

        class AcceptLanguageSeq : public El::Python::Sequence
        {
        public:
          AcceptLanguageSeq(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);

          AcceptLanguageSeq() throw(El::Exception);

          virtual ~AcceptLanguageSeq() throw() {}

          class Type : public El::Python::ObjectTypeImpl<
            AcceptLanguageSeq,
            AcceptLanguageSeq::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
        
          };
        };

        typedef El::Python::SmartPtr<AcceptLanguageSeq> AcceptLanguageSeq_var;
      }
      
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      namespace Python
      {
        //
        // El::Net::HTTP::Python::Header class
        //
        inline
        Header::Header(const El::Net::HTTP::Header& val)
          throw(El::Exception)
            : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Net::HTTP::Header*>(this) = val;
        }

        inline
        void
        Header::write(El::BinaryOutStream& bstr) const throw(El::Exception)
        {
          El::Net::HTTP::Header::write(bstr);
        }
          
        inline
        void
        Header::read(El::BinaryInStream& bstr) throw(El::Exception)
        {
          El::Net::HTTP::Header::read(bstr);
        }
          
        //
        // El::Net::HTTP::Python::Header::Type class
        //
        inline
        Header::Type::Type() throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<Header, Header::Type>(
              "el.net.http.Header",
              "Object encapsulating El::Net::HTTP::Header functionality")
        {
        }

        //
        // El::Net::HTTP::Python::HeaderSeq class
        //
        inline
        HeaderSeq::HeaderSeq(PyTypeObject *type,
                             PyObject *args,
                             PyObject *kwds)
          throw(El::Exception) : El::Python::Sequence(type, args, kwds)
        {
        }
        
        //
        // El::Net::HTTP::Python::HeaderSeq::Type class
        //
        inline
        HeaderSeq::Type::Type() throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<
                HeaderSeq,
                HeaderSeq::Type>(
                  "el.net.http.HeaderSeq",
                  "Object encapsulating El::Net::HTTP::HeaderList "
                    "functionality",
                  "el.Sequence")
        {
        }
        
        //
        // El::Net::HTTP::Python::Param class
        //
        inline
        Param::Param(const El::Net::HTTP::Param& val)
          throw(El::Exception)
            : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Net::HTTP::Param*>(this) = val;
        }
        
        inline
        void
        Param::write(El::BinaryOutStream& bstr) const throw(El::Exception)
        {
          El::Net::HTTP::Param::write(bstr);
        }
          
        inline
        void
        Param::read(El::BinaryInStream& bstr) throw(El::Exception)
        {
          El::Net::HTTP::Param::read(bstr);
        }
          
        //
        // El::Net::HTTP::Python::Param::Type class
        //
        inline
        El::Net::HTTP::Python::Param::Type::Type()
          throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<Param, Param::Type>(
              "el.net.http.Param",
              "Object encapsulating El::Net::HTTP::Param functionality")
        {
        }

        //
        // El::Net::HTTP::Python::ParamSeq class
        //
        inline
        ParamSeq::ParamSeq(PyTypeObject *type,
                           PyObject *args,
                           PyObject *kwds)
          throw(El::Exception) : El::Python::Sequence(type, args, kwds)
        {
        }
        
        //
        // El::Net::HTTP::Python::ParamSeq::Type class
        //
        inline
        ParamSeq::Type::Type() throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<
                ParamSeq,
                ParamSeq::Type>(
                  "el.net.http.ParamSeq",
                  "Object encapsulating El::Net::HTTP::ParamList "
                    "functionality",
                  "el.Sequence")
        {
        }
        
        //
        // El::Net::HTTP::Python::AcceptLanguage::Type class
        //
        inline
        El::Net::HTTP::Python::AcceptLanguage::Type::Type()
          throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<AcceptLanguage, AcceptLanguage::Type>(
              "el.net.http.AcceptLanguage",
              "Object encapsulating El::Net::HTTP::AcceptLanguage "
              "functionality")
        {
        }

        inline
        void
        AcceptLanguage::write(El::BinaryOutStream& bstr) const
          throw(El::Exception)
        {
          El::Net::HTTP::AcceptLanguage::write(bstr);
          bstr << language_ << country_;
        }
          
        inline
        void
        AcceptLanguage::read(El::BinaryInStream& bstr) throw(El::Exception)
        {
          El::Net::HTTP::AcceptLanguage::read(bstr);
          bstr >> language_ >> country_;
        }

        inline
        void
        AcceptLanguage::sync_from_py() throw()
        {
          language = *language_;
          country = *country_;
        }

        //
        // El::Net::HTTP::Python::AcceptLanguageSeq::Type class
        //
        inline
        AcceptLanguageSeq::AcceptLanguageSeq(PyTypeObject *type,
                                             PyObject *args,
                                             PyObject *kwds)
          throw(El::Exception) : El::Python::Sequence(type, args, kwds)
        {
        }

        inline
        AcceptLanguageSeq::AcceptLanguageSeq()
          throw(El::Exception) : Sequence(&Type::instance, 0, 0)
        {
        }

        //
        // El::Net::HTTP::Python::AcceptLanguageSeq::Type class
        //
        inline
        AcceptLanguageSeq::Type::Type()
          throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<
                AcceptLanguageSeq,
                AcceptLanguageSeq::Type>(
                  "el.net.http.AcceptLanguageSeq",
                  "Object encapsulating El::Net::HTTP::AcceptLanguageList "
                    "functionality",
                  "el.Sequence")
        {
        }
        
      }
    }
  }
}

#endif // _ELEMENTS_EL_NET_HTTP_PYTHON_PARAMS_HPP_
