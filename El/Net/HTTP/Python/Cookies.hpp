/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/Python/Cookies.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_PYTHON_COOKIES_HPP_
#define _ELEMENTS_EL_NET_HTTP_PYTHON_COOKIES_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Sequence.hpp>

#include <El/Net/HTTP/Cookies.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      namespace Python
      { 
        class CookieSetter : public El::Python::ObjectImpl,
                             public El::Net::HTTP::CookieSetter
        {
        public:
          CookieSetter(PyTypeObject *type = 0,
                       PyObject *args = 0,
                       PyObject *kwds = 0)
            throw(El::Python::Exception, El::Exception);

          CookieSetter(const El::Net::HTTP::CookieSetter& val)
            throw(El::Exception);

          virtual ~CookieSetter() throw() {}

          virtual void write(El::BinaryOutStream& bstr) const
            throw(El::Exception);
          
          virtual void read(El::BinaryInStream& bstr) throw(El::Exception);
          
          PyObject* py_string() throw(El::Exception);
          
          class Type :
            public El::Python::ObjectTypeImpl<CookieSetter, CookieSetter::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_METHOD_NOARGS(
              py_string,
              "string",
              "Returns string that can be used as a value for "
              "Set-Cookie header");
            
          };
        };

        typedef El::Python::SmartPtr<CookieSetter> CookieSetter_var;

        class Cookie : public El::Python::ObjectImpl,
                       public El::Net::HTTP::Cookie
        {
        public:
          Cookie(PyTypeObject *type = 0,
                 PyObject *args = 0,
                 PyObject *kwds = 0)
            throw(El::Python::Exception, El::Exception);

          Cookie(const El::Net::HTTP::Cookie& val) throw(El::Exception);

          virtual ~Cookie() throw() {}

          virtual CMP_RESULT eq(ObjectImpl* ob)
            throw(Exception, El::Exception);
          
          virtual void write(El::BinaryOutStream& bstr) const
            throw(El::Exception);
          
          virtual void read(El::BinaryInStream& bstr) throw(El::Exception);
          
          class Type : public El::Python::ObjectTypeImpl<Cookie, Cookie::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_MEMBER_STRING(name, "name", "Cookie name", false);
            PY_TYPE_MEMBER_STRING(value, "value", "Cookie value", true);
          };
        };

        typedef El::Python::SmartPtr<Cookie> Cookie_var;

        class CookieSeq : public El::Python::Sequence
        {
        public:
          CookieSeq(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);

          CookieSeq() throw(El::Exception) : Sequence(&Type::instance, 0, 0) {}

          virtual ~CookieSeq() throw() {}

          PyObject* py_most_specific(PyObject* args) throw(El::Exception);
          
          class Type : public El::Python::ObjectTypeImpl<CookieSeq,
                                                         CookieSeq::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
        
            PY_TYPE_METHOD_VARARGS(
              py_most_specific,
              "most_specific",
              "Finds the most specific occurance of cookie requested");
          };
        };
        
        typedef El::Python::SmartPtr<CookieSeq> CookieSeq_var;
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
        // El::Net::HTTP::Python::CookieSetter class
        //
        inline
        CookieSetter::CookieSetter(const El::Net::HTTP::CookieSetter& val)
          throw(El::Exception)
            : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Net::HTTP::CookieSetter*>(this) = val;
        }

        inline
        void
        CookieSetter::write(El::BinaryOutStream& bstr) const
          throw(El::Exception)
        {
          El::Net::HTTP::CookieSetter::write(bstr);
        }

        inline
        void
        CookieSetter::read(El::BinaryInStream& bstr) throw(El::Exception)
        {
          El::Net::HTTP::CookieSetter::read(bstr);
        }
          
        //
        // El::Net::HTTP::Python::CookieSetter::Type class
        //
        inline
        El::Net::HTTP::Python::CookieSetter::Type::Type()
          throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<CookieSetter, CookieSetter::Type>(
              "el.net.http.CookieSetter",
              "Object encapsulating El::Net::HTTP::CookieSetter functionality")
        {
        }

        //
        // El::Net::HTTP::Python::Cookie class
        //
        inline
        Cookie::Cookie(const El::Net::HTTP::Cookie& val)
          throw(El::Exception)
            : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Net::HTTP::Cookie*>(this) = val;
        }
        
        inline
        void
        Cookie::write(El::BinaryOutStream& bstr) const
          throw(El::Exception)
        {
          El::Net::HTTP::Cookie::write(bstr);
        }

        inline
        void
        Cookie::read(El::BinaryInStream& bstr) throw(El::Exception)
        {
          El::Net::HTTP::Cookie::read(bstr);
        }
          
        //
        // El::Net::HTTP::Python::Cookie::Type class
        //
        inline
        El::Net::HTTP::Python::Cookie::Type::Type()
          throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<Cookie, Cookie::Type>(
              "el.net.http.Cookie",
              "Object encapsulating El::Net::HTTP::Cookie functionality")
        {
        }

        //
        // El::Net::HTTP::Python::CookieSeq class
        //
        inline
        CookieSeq::CookieSeq(PyTypeObject *type,
                             PyObject *args,
                             PyObject *kwds)
          throw(El::Exception) : El::Python::Sequence(type, args, kwds)
        {
        }
        
        //
        // El::Net::HTTP::Python::CookieSeq::Type class
        //
        inline
        CookieSeq::Type::Type() throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<
          CookieSeq,
          CookieSeq::Type>(
            "el.net.http.CookieSeq",
            "Object encapsulating El::Net::HTTP::CookieList "
            "functionality",
            "el.Sequence")
        {
        }
        
      }
    }
  }
}

#endif // _ELEMENTS_EL_NET_HTTP_PYTHON_COOKIES_HPP_
