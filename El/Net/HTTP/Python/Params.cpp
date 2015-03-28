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
 * @file Elements/El/Net/HTTP/Python/Params.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/RefCount.hpp>

#include "Params.hpp"

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      namespace Python
      { 
        Header::Type Header::Type::instance;
        HeaderSeq::Type HeaderSeq::Type::instance;

        Param::Type Param::Type::instance;
        ParamSeq::Type ParamSeq::Type::instance;

        AcceptLanguage::Type AcceptLanguage::Type::instance;
        AcceptLanguageSeq::Type AcceptLanguageSeq::Type::instance;

        //
        // Header class
        //
        Header::Header(PyTypeObject *type, PyObject *args, PyObject *kwds)
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
                 "s|s:el.net.http.Header.Header",
                 (char**)kwlist,
                 &nm,
                 &val))
            {
              El::Python::handle_error(
                "El::Net::HTTP::Python::Header::Header");
            }          

            if(nm == 0 || *nm == '\0')
            {
              El::Python::report_error(
                PyExc_TypeError,
                "non-empty header name expected",
                "El::Net::HTTP::Python::Header::Header");
            }
          }

          name = nm ? nm : "";
          value = val ? val : "";
        }

        El::Python::ObjectImpl::CMP_RESULT
        Header::eq(ObjectImpl* o) throw(Exception, El::Exception)
        {
          Header* ob = static_cast<Header*>(o);
          return name == ob->name && value == ob->value ? CR_TRUE : CR_FALSE;
        }
        
        //
        // El::Net::HTTP::Python::HeaderSeq class
        //
        PyObject*
        HeaderSeq::py_find(PyObject* args) throw(El::Exception)
        {
          char* name = 0;
          PyObject* default_val = 0;

          if(!PyArg_ParseTuple(args,
                               "s|O:el.net.http.HeaderSeq.find",
                               &name,
                               &default_val))
          {
            El::Python::handle_error(
              "El::Net::HTTP::Python::HeaderSeq::py_find");
          }
          
          if(name != 0)
          {
            for(const_iterator i(begin()), e(end()); i != e; ++i)
            {
              PyObject* obj = i->in();

              if(Header::Type::check_type(obj))
              {
                Header* header = Header::Type::down_cast(obj);
              
                if(strcasecmp(header->name.c_str(), name) == 0)
                {
                  return PyString_FromString(header->value.c_str());
                }
              }
              
            }
          }
          
          return default_val ? El::Python::add_ref(default_val) :
            PyString_FromString("");
        }

        //
        // Param class
        //
        Param::Param(PyTypeObject *type, PyObject *args, PyObject *kwds)
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
                 "s|s:el.net.http.Param.Param",
                 (char**)kwlist,
                 &nm,
                 &val))
            {
              El::Python::handle_error(
                "El::Net::HTTP::Python::Param::Param");
            }
            
            if(nm == 0 || *nm == '\0')
            {
              El::Python::report_error(
                PyExc_TypeError,
                "non-empty header name expected",
                "El::Net::HTTP::Python::Param::Param");
            }
          }
          
          name = nm ? nm : "";
          value = val ? val : "";
        }

        El::Python::ObjectImpl::CMP_RESULT
        Param::eq(ObjectImpl* o) throw(Exception, El::Exception)
        {
          Param* ob = static_cast<Param*>(o);
          return name == ob->name && value == ob->value ? CR_TRUE : CR_FALSE;
        }
        
        //
        // AcceptLanguage class
        //
        AcceptLanguage::AcceptLanguage(PyTypeObject *type,
                                       PyObject *args,
                                       PyObject *kwds)
          throw(El::Python::Exception, El::Exception)
            : ObjectImpl(type ? type : &Type::instance),
              country_(new El::Python::Country())
        {
          const char *kwlist[] = { "language", "subtag", "qvalue", NULL};

          PyObject* lang = 0;
          char* stag = 0;
          float qval = 0;

          if(args)
          {
            if(!PyArg_ParseTupleAndKeywords(
                 args,
                 kwds,
                 "O|sf:el.net.http.AcceptLanguage.AcceptLanguage",
                 (char**)kwlist,
                 &lang,
                 &stag,
                 &qval))
            {
              El::Python::handle_error(
                "El::Net::HTTP::Python::AcceptLanguage::AcceptLanguage");
            }

            if(!El::Python::Lang::Type::check_type(lang))
            {
              El::Python::report_error(
                PyExc_TypeError,
                "invalid language value",
                "El::Net::HTTP::Python::AcceptLanguage::AcceptLanguage");
            }

            language_ = El::Python::Lang::Type::down_cast(lang, true);
          }
          
          subtag = stag ? stag : "";

          try
          {
            El::Country cnt(subtag.c_str());
            country_ = new El::Python::Country(cnt);
          }
          catch(const El::Country::InvalidArg& )
          {
            country_ = new El::Python::Country();
          }
          
          qvalue = qval;
        }

        AcceptLanguage::AcceptLanguage(
          const El::Net::HTTP::AcceptLanguage& val)
          throw(El::Exception) : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Net::HTTP::AcceptLanguage*>(this) = val;
          language_ = new El::Python::Lang(val.language);
          country_ = new El::Python::Country(val.country);
        }
        
        El::Python::ObjectImpl::CMP_RESULT
        AcceptLanguage::eq(ObjectImpl* o) throw(Exception, El::Exception)
        {          
          AcceptLanguage* ob = static_cast<AcceptLanguage*>(o);
          return *language_ == *ob->language_ && subtag == ob->subtag &&
            qvalue == ob->qvalue ? CR_TRUE : CR_FALSE;
        }        

        //
        // El::Net::HTTP::Python::ParamSeq class
        //
        PyObject*
        ParamSeq::py_find(PyObject* args) throw(El::Exception)
        {
          char* name = 0;
          PyObject* default_val = 0;

          if(!PyArg_ParseTuple(args,
                               "s|O:el.net.http.ParamSeq.find",
                               &name,
                               &default_val))
          {
            El::Python::handle_error(
              "El::Net::HTTP::Python::ParamSeq::py_find");
          }
          
          if(name != 0)
          {
            for(const_iterator i(begin()), e(end()); i != e; ++i)
            {
              PyObject* obj = i->in();

              if(Param::Type::check_type(obj))
              {
                Param* param = Param::Type::down_cast(obj);
              
                if(strcasecmp(param->name.c_str(), name) == 0)
                {
                  return PyString_FromString(param->value.c_str());
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
