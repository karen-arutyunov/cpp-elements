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
 * @file Elements/tests/PythonSandbox/Context.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_TESTS_PYTHONSANDBOX_CONTEXT_HPP_
#define _ELEMENTS_TESTS_PYTHONSANDBOX_CONTEXT_HPP_

#include <string>
#include <iostream>
#include <memory>
#include <sstream>

#include <Python.h>

#include <El/Exception.hpp>
#include <El/Logging/StreamLogger.hpp>
#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/Lang.hpp>
#include <El/Python/Country.hpp>
#include <El/Python/Guid.hpp>
#include <El/Python/Locale.hpp>
#include <El/Python/Map.hpp>
#include <El/Python/Moment.hpp>
#include <El/Python/NameValueMap.hpp>
#include <El/Python/Sequence.hpp>
#include <El/Python/TimeValue.hpp>
#include <El/Python/SandboxService.hpp>
#include <El/Net/HTTP/Python/Params.hpp>
#include <El/Net/HTTP/Python/Utility.hpp>
#include <El/Net/HTTP/Python/Cookies.hpp>

class Interceptor : public El::Python::SandboxService::Interceptor
{
public:
  std::string log;
  
public:

  Interceptor(PyTypeObject *type = 0, PyObject *args = 0, PyObject *kwds = 0)
    throw(El::Exception);
  
  virtual ~Interceptor() throw() {}

  virtual void pre_run(El::Python::Sandbox* sandbox,
                       El::Python::SandboxService::ObjectMap& objects)
    throw(El::Exception);
  
  virtual void post_run(
    El::Python::SandboxService::ExceptionType exception_type,
    El::Python::SandboxService::ObjectMap& objects,
    El::Python::Object_var& result)
    throw(El::Exception);  

  virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
  virtual void read(El::BinaryInStream& bstr) throw(El::Exception);

  class Type :
    public El::Python::ObjectTypeImpl<Interceptor, Interceptor::Type>
  {
  public:
    Type() throw(El::Python::Exception, El::Exception);
    static Type instance;    
  };

private:
  std::auto_ptr<std::ostringstream> stream_;
  std::auto_ptr<El::Logging::Logger> logger_;
};

typedef El::Python::SmartPtr<Interceptor> Interceptor_var;

class Context : public El::Python::ObjectImpl
{
public:
  std::string value;
  El::Python::Lang_var lang;
  El::Python::Country_var country;
  El::Python::Guid_var guid;
  El::Python::Locale_var locale;
  El::Python::Map_var map;
  El::Python::Moment_var moment;
  El::Python::NameValueMap_var nv_map;
  El::Python::Sequence_var seq;
  El::Python::TimeValue_var time;
  El::Net::HTTP::Python::HeaderSeq_var http_headers;
  El::Net::HTTP::Python::ParamSeq_var http_params;
  El::Net::HTTP::Python::AcceptLanguageSeq_var http_accept_langs;
  El::Net::HTTP::Python::SearchInfo_var http_search_info;
  El::Net::HTTP::Python::RequestParams_var http_request_params;
  El::Net::HTTP::Python::CookieSetter_var http_cookie_setter;
  El::Net::HTTP::Python::CookieSeq_var http_cookies;
  
public:
  Context(PyTypeObject *type = 0, PyObject *args = 0, PyObject *kwds = 0)
    throw(El::Exception);
  
  virtual ~Context() throw() {}

  void dump(std::ostream& ostr) const throw();

  class Type :
    public El::Python::ObjectTypeImpl<Context, Context::Type>
  {
  public:
    Type() throw(El::Python::Exception, El::Exception);
    static Type instance;
    
    PY_TYPE_MEMBER_STRING(value, "value", "Object value", true);

    PY_TYPE_MEMBER_OBJECT(lang,
                          El::Python::Lang::Type,
                          "lang",
                          "Language",
                          false);
    
    PY_TYPE_MEMBER_OBJECT(country,
                          El::Python::Country::Type,
                          "country",
                          "Country",
                          false);

    PY_TYPE_MEMBER_OBJECT(guid,
                          El::Python::Guid::Type,
                          "guid",
                          "Guid",
                          false);

    PY_TYPE_MEMBER_OBJECT(locale,
                          El::Python::Locale::Type,
                          "locale",
                          "Locale",
                          false);

    PY_TYPE_MEMBER_OBJECT(map,
                          El::Python::Map::Type,
                          "map",
                          "Map",
                          false);

    PY_TYPE_MEMBER_OBJECT(moment,
                          El::Python::Moment::Type,
                          "moment",
                          "Moment",
                          false);
    
    PY_TYPE_MEMBER_OBJECT(nv_map,
                          El::Python::NameValueMap::Type,
                          "nv_map",
                          "NameValueMap",
                          false);    

    PY_TYPE_MEMBER_OBJECT(seq,
                          El::Python::Sequence::Type,
                          "seq",
                          "Sequence",
                          false);    

    PY_TYPE_MEMBER_OBJECT(time,
                          El::Python::TimeValue::Type,
                          "time",
                          "TimeValue",
                          false);

    PY_TYPE_MEMBER_OBJECT(http_headers,
                          El::Net::HTTP::Python::HeaderSeq::Type,
                          "http_headers",
                          "HTTP headers",
                          false);    

    PY_TYPE_MEMBER_OBJECT(http_params,
                          El::Net::HTTP::Python::ParamSeq::Type,
                          "http_params",
                          "HTTP params",
                          false);    

    PY_TYPE_MEMBER_OBJECT(http_accept_langs,
                          El::Net::HTTP::Python::AcceptLanguageSeq::Type,
                          "http_accept_langs",
                          "HTTP accept langs",
                          false);    

    PY_TYPE_MEMBER_OBJECT(http_search_info,
                          El::Net::HTTP::Python::SearchInfo::Type,
                          "http_search_info",
                          "HTTP search info",
                          false);    

    PY_TYPE_MEMBER_OBJECT(http_request_params,
                          El::Net::HTTP::Python::RequestParams::Type,
                          "http_request_params",
                          "HTTP request params",
                          false);

    PY_TYPE_MEMBER_OBJECT(http_cookies,
                          El::Net::HTTP::Python::CookieSeq::Type,
                          "http_cookies",
                          "HTTP cookies",
                          false);

    PY_TYPE_MEMBER_OBJECT(http_cookie_setter,
                          El::Net::HTTP::Python::CookieSetter::Type,
                          "http_cookie_setter",
                          "HTTP cookie setter",
                          false);
  };
    
  virtual void write(El::BinaryOutStream& bstr) const throw(El::Exception);
  virtual void read(El::BinaryInStream& bstr) throw(El::Exception);
};
      
typedef El::Python::SmartPtr<Context> Context_var;

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

//
// Interceptor class
//

inline
Interceptor::Interceptor(PyTypeObject *type, PyObject *args, PyObject *kwds)
  throw(El::Exception) :
    El::Python::SandboxService::Interceptor(type ? type : &Type::instance)
{
}

inline
Interceptor::Type::Type() throw(El::Python::Exception, El::Exception)
    : El::Python::ObjectTypeImpl<Interceptor, Interceptor::Type>(
      "el.PythonSandboxTest.Interceptor",
      "Interceptor object")
{
}

//
// Context class
//
inline
Context::Context(PyTypeObject *type, PyObject *args, PyObject *kwds)
  throw(El::Exception) :
    El::Python::ObjectImpl(type ? type : &Type::instance),
    lang(new El::Python::Lang()),
    country(new El::Python::Country()),
    guid(new El::Python::Guid()),
    locale(new El::Python::Locale()),
    map(new El::Python::Map()),
    moment(new El::Python::Moment()),
    nv_map(new El::Python::NameValueMap()),
    seq(new El::Python::Sequence()),
    time(new El::Python::TimeValue()),
    http_headers(new El::Net::HTTP::Python::HeaderSeq()),
    http_params(new El::Net::HTTP::Python::ParamSeq()),
    http_accept_langs(new El::Net::HTTP::Python::AcceptLanguageSeq()),
    http_search_info(new El::Net::HTTP::Python::SearchInfo()),
    http_request_params(new El::Net::HTTP::Python::RequestParams()),
    http_cookie_setter(new El::Net::HTTP::Python::CookieSetter()),
    http_cookies(new El::Net::HTTP::Python::CookieSeq())
{
}

inline
void
Context::dump(std::ostream& ostr) const throw()
{
  ostr << "value " << value << "\nlang " << lang->l3_code()
       << "\ncountry " << country->l3_code() << "\nguid " << guid->string()
       << "\nlocale " << locale->lang->l3_code() << "/"
       << locale->country->l3_code() << "\nmap";

  for(El::Python::Map::const_iterator i(map->begin()), e(map->end()); i != e;
      ++i)
  {
    try
    {
      El::Python::Object_var key =
        El::Python::string_from_object(i->first.in());
      
      El::Python::Object_var value =
        El::Python::string_from_object(i->second.in());

      size_t len = 0;
      
      ostr << " '" << El::Python::string_from_string(key.in(), len)
           << "' -> '" << El::Python::string_from_string(value.in(), len)
           << "'";
    }
    catch(...)
    {
    }
  }

  ostr << "\nmoment " << moment->rfc0822(true, true) << "\nnv_map";

  for(El::Python::Map::const_iterator i(nv_map->begin()), e(nv_map->end());
      i != e; ++i)
  {
    try
    {
      El::Python::Object_var key =
        El::Python::string_from_object(i->first.in());
      
      El::Python::Object_var value =
        El::Python::string_from_object(i->second.in());

      size_t len = 0;
      
      ostr << " '" << El::Python::string_from_string(key.in(), len)
           << "' -> '" << El::Python::string_from_string(value.in(), len)
           << "'";
    }
    catch(...)
    {
    }
  }  

  ostr << "\nseq";

  for(El::Python::Sequence::const_iterator i(seq->begin()), e(seq->end());
      i != e; ++i)
  {
    try
    {
      El::Python::Object_var obj = El::Python::string_from_object(i->in());
      
      size_t len = 0;
      ostr << " '" << El::Python::string_from_string(obj.in(), len) << "'";
    }
    catch(...)
    {
    }
  }

  ostr << "\ntime " << El::Moment::time(*time, true) << "\nhttp_headers";

  for(El::Net::HTTP::Python::HeaderSeq::const_iterator
        i(http_headers->begin()), e(http_headers->end()); i != e; ++i)
  {
    try
    {
      El::Net::HTTP::Python::Header* header =
        El::Net::HTTP::Python::Header::Type::down_cast(i->in());
      
      ostr << " " << header->name << ":" << header->value;
    }
    catch(...)
    {
    }
  }

  ostr << "\nhttp_params";

  for(El::Net::HTTP::Python::ParamSeq::const_iterator
        i(http_params->begin()), e(http_params->end()); i != e; ++i)
  {
    try
    {
      El::Net::HTTP::Python::Param* param =
        El::Net::HTTP::Python::Param::Type::down_cast(i->in());
      
      ostr << " " << param->name << "=" << param->value;
    }
    catch(...)
    {
    }
  }

  ostr << "\nhttp_accept_langs";

  for(El::Net::HTTP::Python::AcceptLanguageSeq::const_iterator
        i(http_accept_langs->begin()), e(http_accept_langs->end()); i != e;
      ++i)
  {
    try
    {
      El::Net::HTTP::Python::AcceptLanguage* al =
        El::Net::HTTP::Python::AcceptLanguage::Type::down_cast(i->in());

      al->sync_from_py();
      
      ostr << " " << al->language.l3_code() << "/" << al->country.l3_code()
           << "/" << al->subtag << "/" << al->qvalue;
    }
    catch(...)
    {
    }
  }

  ostr << "\nsearch info " << http_search_info->engine << "/"
       << http_search_info->query << "/" << http_search_info->engine
       << "\nrequest params " << http_request_params->user_agent << "/"
       << http_request_params->referer << "/"
       << http_request_params->timeout << "/"
       << http_request_params->redirects_to_follow << "/"
       << http_request_params->recv_buffer_size << "\nhttp_cookie_setter "
       << http_cookie_setter->name << ":" << http_cookie_setter->value << ":"
       << http_cookie_setter->expiration.rfc0822(true, true) << ":"
       << http_cookie_setter->path << ":" << http_cookie_setter->domain << ":"
       << http_cookie_setter->secure << ":" << http_cookie_setter->session
       << "\nhttp_cookies";
  
  for(El::Net::HTTP::Python::CookieSeq::const_iterator
        i(http_cookies->begin()), e(http_cookies->end()); i != e; ++i)
  {
    try
    {
      El::Net::HTTP::Python::Cookie* c =
        El::Net::HTTP::Python::Cookie::Type::down_cast(i->in());
      
      ostr << " " << c->name << "=" << c->value;
    }
    catch(...)
    {
    }
  }
}

inline
void
Context::write(El::BinaryOutStream& bstr) const throw(El::Exception)
{
  bstr << value << lang << country << guid << locale << map << moment
       << nv_map << seq << time << http_headers << http_params
       << http_accept_langs << http_search_info << http_request_params
       << http_cookies << http_cookie_setter;
}

inline
void
Context::read(El::BinaryInStream& bstr) throw(El::Exception)
{
  bstr >> value >> lang >> country >> guid >> locale >> map >> moment
       >> nv_map >> seq >> time >> http_headers >> http_params
       >> http_accept_langs >> http_search_info >> http_request_params
       >> http_cookies >> http_cookie_setter;
}

inline
Context::Type::Type() throw(El::Python::Exception, El::Exception)
    : El::Python::ObjectTypeImpl<Context, Context::Type>(
      "el.PythonSandboxTest.Context",
      "Context object")
{
}

#endif // _ELEMENTS_TESTS_PYTHONSANDBOX_CONTEXT_HPP_
