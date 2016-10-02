/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/Python/Utility.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/RefCount.hpp>

#include "Utility.hpp"

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      namespace Python
      { 
        SearchInfo::Type SearchInfo::Type::instance;
        RequestParams::Type RequestParams::Type::instance;

        //
        // RequestParams class
        //
        RequestParams::RequestParams(PyTypeObject *type,
                                     PyObject *args,
                                     PyObject *kwds)
          throw(El::Python::Exception, El::Exception)
            : ObjectImpl(type ? type : &Type::instance)
        {
          const char *kwlist[] =
            { "user_agent",
              "referer",
              "timeout",
              "redirects_to_follow",
              "recv_buffer_size",
              NULL
            };

          char* a = 0;
          char* r = 0;
          unsigned long t = 0;
          unsigned long f = 0;
          unsigned long s = 0;

          if(args)
          {
            if(!PyArg_ParseTupleAndKeywords(
                 args,
                 kwds,
                 "|sskkk:el.net.http.RequestParams.RequestParams",
                 (char**)kwlist,
                 &a,
                 &r,
                 &t,
                 &f,
                 &s))
            {
              El::Python::handle_error(
                "El::Net::HTTP::Python::RequestParams::RequestParams");
            }
          }
          
          user_agent = a ? a : "";
          referer = r ? r : "";
          timeout = t;
          redirects_to_follow = f;
          recv_buffer_size = s;
        }

        //
        // SearchInfo class
        //
        SearchInfo::SearchInfo(PyTypeObject *type,
                               PyObject *args,
                               PyObject *kwds)
          throw(El::Python::Exception, El::Exception)
            : ObjectImpl(type ? type : &Type::instance)
        {
          const char *kwlist[] = { "engine", "query", "image", NULL};

          char* e = 0;
          char* q = 0;
          char* i = 0;

          if(args)
          {
            if(!PyArg_ParseTupleAndKeywords(
                 args,
                 kwds,
                 "s|ss:el.net.http.SearchInfo.SearchInfo",
                 (char**)kwlist,
                 &e,
                 &q,
                 &i))
            {
              El::Python::handle_error(
                "El::Net::HTTP::Python::SearchInfo::SearchInfo");
            }

            if(e == 0 || *e == '\0')
            {
              El::Python::report_error(
                PyExc_TypeError,
                "non-empty engine name expected",
                "El::Net::HTTP::Python::SearchInfo::SearchInfo");
            }
          }
          
          engine = e ? e : "";
          query = q ? q : "";
          image = i ? i : "";
        }
      }
      
    }
  }
}
