/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/Python/Cookies.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/Moment.hpp>

#include "Cookies.hpp"

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      namespace Python
      { 
        CookieSetter::Type CookieSetter::Type::instance;

        Cookie::Type Cookie::Type::instance;
        CookieSeq::Type CookieSeq::Type::instance;

        El::Python::Module el_net_http_module(
          "el.net.http",
          "Module containing ElNet HTTP library types.",
          true);
    
        CookieSetter::CookieSetter(PyTypeObject *type,
                                   PyObject *args,
                                   PyObject *kwds)
          throw(El::Python::Exception, El::Exception)
            : ObjectImpl(type ? type : &Type::instance)
        {
          const char *kwlist[] =
            { "name", "value", "expiration", "path", "domain", "secure", NULL};

          char* name = 0;
          char* val = 0;
          PyObject* expiration = 0;
          char* path = 0;
          char* domain = 0;
          unsigned char secure = 0;

          if(args)
          {
            if(!PyArg_ParseTupleAndKeywords(
                 args,
                 kwds,
                 "s|sOssb:el.net.http.CookieSetter.CookieSetter",
                 (char**)kwlist,
                 &name,
                 &val,
                 &expiration,
                 &path,
                 &domain,
                 &secure))
            {
              El::Python::handle_error(
                "El::Net::HTTP::Python::CookieSetter::CookieSetter");
            }
          }
          
          El::Python::Moment* moment =
            El::Python::Moment::Type::down_cast(expiration);

          *this =
            El::Net::HTTP::CookieSetter(
              name,
              val,
              moment ? moment : NULL,
              path,
              domain,
              secure);
        }

        PyObject*
        CookieSetter::py_string() throw(El::Exception)
        {
          return PyString_FromString(string().c_str());
        }
        
        //
        // Cookie class
        //
        Cookie::Cookie(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Python::Exception, El::Exception)
            : ObjectImpl(type ? type : &Type::instance)
        {
          const char *kwlist[] = { "name", "value", NULL};

          char* nm = 0;
          char* val = 0;

          if(args)
          {
            if(!PyArg_ParseTupleAndKeywords(
                 args,
                 kwds,
                 "s|s:el.net.http.Cookie.Cookie",
                 (char**)kwlist,
                 &nm,
                 &val))
            {
              El::Python::handle_error(
                "El::Net::HTTP::Python::Cookie::Cookie");
            }

            if(nm == 0 || *nm == '\0')
            {
              El::Python::report_error(
                PyExc_TypeError,
                "non-empty header name expected",
                "El::Net::HTTP::Python::Cookie::Cookie");
            }
          }
          
          name = nm ? nm : "";
          value = val ? val : "";
        }

        El::Python::ObjectImpl::CMP_RESULT
        Cookie::eq(ObjectImpl* o) throw(Exception, El::Exception)
        {
          Cookie* ob = static_cast<Cookie*>(o);
          return name == ob->name && value == ob->value ? CR_TRUE : CR_FALSE;
        }
        
        //
        // El::Net::HTTP::Python::CookieSeq class
        //
        PyObject*
        CookieSeq::py_most_specific(PyObject* args) throw(El::Exception)
        {
          char* name = 0;
          PyObject* default_val = 0;

          if(!PyArg_ParseTuple(args,
                               "s|O:el.net.http.CookieSeq.most_specific",
                               &name,
                               &default_val))
          {
            El::Python::handle_error(
              "El::Net::HTTP::Python::CookieSeq::py_most_specific");
          }
          
          if(name != 0)
          {
            for(const_iterator it = begin(); it != end(); it++)
            {
              PyObject* obj = it->in();

              if(Cookie::Type::check_type(obj))
              {
                Cookie* cookie = Cookie::Type::down_cast(obj);
              
                if(strcasecmp(cookie->name.c_str(), name) == 0)
                {
                  return PyString_FromString(cookie->value.c_str());
                }
              }
              
            }
          }
          
          return default_val ? El::Python::add_ref(default_val) :
            PyString_FromString("");
        }
      }
    }
  }
}
