/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Request.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Exception.hpp>
#include <El/Python/Utility.hpp>
#include <El/Net/HTTP/Python/Cookies.hpp>
#include <El/Net/HTTP/Python/Params.hpp>
#include <El/Python/String/Template.hpp>
#include <El/Python/Locale.hpp>

#include "Request.hpp"

namespace El
{
  namespace PSP
  {
    Request::Type Request::Type::instance;
    Request::Forward::Type Request::Forward::Type::instance;
    Request::In::Type Request::In::Type::instance;
    Request::Out::Type Request::Out::Type::instance;
    Request::CacheEntry::Type Request::CacheEntry::Type::instance;
    
    Request::Out::Stream::Type Request::Out::Stream::Type::instance;
    
    Request::Out::LocalizationMarker::Type
    Request::Out::LocalizationMarker::Type::instance;
 
    Request::Out::HexMarker::Type Request::Out::HexMarker::Type::instance;
    Request::Out::DecMarker::Type Request::Out::DecMarker::Type::instance;
    
    Request::Out::UppercaseMarker::Type
    Request::Out::UppercaseMarker::Type::instance;
 
    Request::Out::NouppercaseMarker::Type
    Request::Out::NouppercaseMarker::Type::instance;
  
    Request::Out::JS_EscapeMarker::Type
    Request::Out::JS_EscapeMarker::Type::instance;
 
    Request::Out::NoJS_EscapeMarker::Type
    Request::Out::NoJS_EscapeMarker::Type::instance;

    El::Geography::AddressInfo Request::In::address_info_;
    
    //
    // Request class
    //
    Request::Request(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type),
          request_(0)
    {
      throw Exception(
        "El::PSP::Request::Request: unforseen way of object creation");
    }

    Request::Request(El::Apache::Request& request,
                     const El::Lang& lang,
                     const LangMap& valid_languages,
                     El::PSP::Localization* localization)
      throw(El::Exception)
        : El::Python::ObjectImpl(&Type::instance),
          request_(&request),
          in_(new In(request, lang, valid_languages)),
          out_(new Out(request, localization)),
          localization_(add_ref(localization))
    {
      RequestCache::Entry* entry =
        dynamic_cast<RequestCache::Entry*>(
          request.callback(RequestCache::PSP_CACHE_REQUEST_CALLBACK_ID));

      cache_entry_ = entry ? new CacheEntry(entry) :
        El::Python::add_ref(Py_None);
    }

    void
    Request::language(const El::Lang& lang, 
                      El::PSP::Localization* localization)
      throw(El::Exception)
    {
      in_->lang(lang);
      out_->localization(localization);
      localization_ = El::Python::add_ref(localization);
    }    
    
    //
    // Request::CacheEntry class
    //
    Request::CacheEntry::CacheEntry(PyTypeObject *type,
                                    PyObject *args,
                                    PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type)
    {
      throw Exception(
        "El::PSP::Request::CacheEntry::CacheEntry: unforseen way of "
        "object creation");
    }
    
    Request::CacheEntry::CacheEntry(RequestCache::Entry* entry)
      throw(El::Exception)
        : El::Python::ObjectImpl(&Type::instance),
          entry_(El::RefCount::add_ref(entry))
    {
    }

    PyObject*
    Request::CacheEntry::py_etag(PyObject* args) throw(El::Exception)
    {
      const char* value = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s:el.psp.CacheEntry.etag",
                           &value))
      {
        El::Python::handle_error("El::PSP::Request::CacheEntry::py_etag");
      }

      entry_->etag(value);
      return El::Python::add_ref(Py_None);
    }

    PyObject*
    Request::CacheEntry::py_timeout(PyObject* args) throw(El::Exception)
    {
      unsigned long value = 0;
      
      if(!PyArg_ParseTuple(args,
                           "k:el.psp.CacheEntry.timeout",
                           &value))
      {
        El::Python::handle_error("El::PSP::Request::CacheEntry::py_timeout");
      }

      entry_->timeout(value);
      return El::Python::add_ref(Py_None);
    }

    PyObject*
    Request::CacheEntry::py_unused_timeout(PyObject* args) throw(El::Exception)
    {
      unsigned long value = 0;
      
      if(!PyArg_ParseTuple(args, "k:el.psp.CacheEntry.unused_timeout", &value))
      {
        El::Python::handle_error(
          "El::PSP::Request::CacheEntry::py_unused_timeout");
      }

      entry_->unused_timeout(value);
      return El::Python::add_ref(Py_None);
    }

    //
    // Request::In class
    //
    Request::In::In(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type),
          request_(0)
    {
      throw Exception(
        "El::PSP::Request::In::In: unforseen way of object creation");
    }
    
    Request::In::In(El::Apache::Request& request,
                    const El::Lang& lang,
                    const LangMap& valid_languages)
      throw(El::Exception)
        : El::Python::ObjectImpl(&Type::instance),
          request_(&request),
          valid_languages_(valid_languages)
    {
      for(LangMap::const_iterator it = valid_languages.begin();
          it != valid_languages.end(); it++)
      {
        final_languages_.insert(it->second);
      }      
      
      lang_ = new El::Python::Lang(lang);
    }

    void
    Request::In::lang(const El::Lang& val) throw(El::Exception)
    {
      if(final_languages_.find(val) == final_languages_.end())
      {
        std::ostringstream ostr;
        ostr << "EL::PSP::Request::In::lang: language " << val.l3_code()
             << "is not valid for interface production";

        throw Exception(ostr.str());
      }
      
      lang_ = new El::Python::Lang(val);
    }

    PyObject*
    Request::In::py_locale(PyObject* args, PyObject *kwds) throw(El::Exception)
    {
      const char *kwlist[] = { "lang", "country", NULL};
      
      PyObject* l = 0;
      PyObject* c = 0;
        
      if(!PyArg_ParseTupleAndKeywords(args,
                                      kwds,
                                      "|OO:el.Locale.Locale",
                                      (char **)kwlist,
                                      &l,
                                      &c))
      {
        El::Python::handle_error("El::PSP::Request::In::py_locale");
      }

      El::Lang lang;
      
      if(l)
      {
        lang = *El::Python::Lang::Type::down_cast(l);
      }

      El::Country country;
      
      if(c)
      {
        country = *El::Python::Country::Type::down_cast(c);
      }

      El::Python::Locale_var locale = new El::Python::Locale();

      bool reset_lang = lang == El::Lang::nonexistent;
      bool reset_country = country == El::Country::nonexistent;      
      
      if(lang != El::Lang::null && country != El::Country::null)
      {
        *locale->lang = reset_lang ? El::Lang::null : lang;        
        *locale->country = reset_country ? El::Country::null : country;        
        return locale.retn();
      }

      if(reset_lang)
      {
        lang = El::Lang::null;
      }

      if(reset_country)
      {
        country = El::Country::null;
      }
      
      const El::Net::HTTP::AcceptLanguageList& al =
        request_->in().accept_languages();

      El::Lang locale_lang;
      
      if(!al.empty())
      {
        locale_lang = al.begin()->language;
      }
      
      if(lang != El::Lang::null)
      {
        // Need to detect country

        country = detect_country(lang);        
      }
/*      
      else if(country != El::Country::null)
      {
        // Need to detect language
        
        if(locale_lang == El::Lang::null)
        {
          lang = *lang_;
        }
        
        if(lang == El::Lang::null)
        {
          El::PSP::Request::LangMap::const_iterator i =
            valid_languages_.find(locale_lang.l3_code());
          
          if(i == valid_languages_.end())
          {
            i = valid_languages_.find("*");
          }
          
          if(i != valid_languages_.end())
          {
            lang = i->second;
          }
        }

        if(lang == El::Lang::null)
        {
          lang = locale_lang;
        }
        
        if(lang == El::Lang::null)
        {
          const char* ip = request_->remote_ip();

          if(ip) 
          {
            lang = address_info_.country(ip).lang();
          }
        }
      }
      else
      {
        // Need to detect country and language
        
        if(locale_lang == El::Lang::null)
        {
          lang = *lang_;
        }

        if(lang != El::Lang::null)
        {
          country = detect_country(lang);
        }
        else
        {
          if(locale_lang != El::Lang::null)
          {
            El::PSP::Request::LangMap::const_iterator i =
              valid_languages_.find(locale_lang.l3_code());
          
            if(i == valid_languages_.end())
            {
              i = valid_languages_.find("*");
            }
          
            if(i == valid_languages_.end())
            {
              lang = locale_lang;
            }
            else
            {
              lang = i->second;
            }
            
            country = detect_country(lang, 1);
          }
          else
          {
            country = detect_country(lang, 2);
            lang = country.lang();
          }
        }
      }
*/
      else if(country != El::Country::null)
      {
        // Need to detect language

        if(locale_lang == El::Lang::null ||
           final_languages_.find(locale_lang) != final_languages_.end())
        {
          lang = *lang_;
        }

        if(lang == El::Lang::null)
        {
          El::PSP::Request::LangMap::const_iterator i =
            valid_languages_.find(locale_lang.l3_code());
          
          if(i == valid_languages_.end())
          {
            i = valid_languages_.find("*");
          }
          
          if(i != valid_languages_.end())
          {
            lang = i->second;
          }
        }        

        if(lang == El::Lang::null)
        {
          lang = locale_lang;
        }
        
        if(lang == El::Lang::null)
        {
          const char* ip = request_->remote_ip();

          if(ip) 
          {
            lang = address_info_.country(ip).lang();
          }
        }
      }
      else
      {
        // Need to detect country and language
        
        if(locale_lang == El::Lang::null ||
           final_languages_.find(locale_lang) != final_languages_.end())
        {
          lang = *lang_;
        }

        if(lang != El::Lang::null)
        {
          country = detect_country(lang);
        }
        else
        {
          if(locale_lang != El::Lang::null)
          {
//            lang = locale_lang;

            El::PSP::Request::LangMap::const_iterator i =
              valid_languages_.find(locale_lang.l3_code());
          
            if(i == valid_languages_.end())
            {
              i = valid_languages_.find("*");
            }
          
            if(i == valid_languages_.end())
            {
              lang = locale_lang;
            }
            else
            {
              lang = i->second;
            }
            
            country = detect_country(lang, 1);
          }
          else
          {
            country = detect_country(lang, 2);
            lang = country.lang();
          }
        }
      }
      
      *locale->lang = reset_lang ? El::Lang::null : lang;
      *locale->country = reset_country ? El::Country::null : country;      
      return locale.retn();
    }
      
    El::Country
    Request::In::detect_country(const El::Lang& lang, unsigned long step) const
      throw(El::Exception)
    {
      El::Country country;

      if(step == 0)
      {
        for(unsigned long i = 1; i <= El::Country::countries_count(); i++)
        {
          El::Country c =
            El::Country((El::Country::ElCode)(El::Country::EC_NUL + i));
          
          if(c.lang() == lang)
          {
            if(country == El::Country::null)
            {
              country = c;
            }
            else
            {
              // Ambiguity
              country = El::Country::null;
              break;
            }
          }
        }

        step++;
      }

      if(step == 1)
      {
        if(country == El::Country::null)
        {
          const El::Net::HTTP::AcceptLanguageList& al =
            request_->in().accept_languages();
          
          if(!al.empty())
          {
            country = al.begin()->country;
          }
        }

        step++;
      }

      if(step == 2)
      {
        if(country == El::Country::null)
        {
          const char* ip = request_->remote_ip();
          
          if(ip) 
          {
            country = address_info_.country(ip);
          }
        }
        
        step++;
      }        

      return country;
    }
    
    PyObject*
    Request::In::py_headers() throw(El::Exception)
    {
      if(headers_.in() == 0)
      {
        headers_ = new El::Net::HTTP::Python::HeaderSeq();
        
        headers_->from_container<El::Net::HTTP::Python::Header>(
          request_->in().headers());
      }
      
      return headers_.add_ref();
    }
    
    PyObject*
    Request::In::py_parameters() throw(El::Exception)
    {
      if(parameters_.in() == 0)
      {
        parameters_ = new El::Net::HTTP::Python::ParamSeq();
        
        parameters_->from_container<El::Net::HTTP::Python::Param>(
          request_->in().parameters(true));
      }
      
      return parameters_.add_ref();
    }
    
    PyObject*
    Request::In::py_cookies() throw(El::Exception)
    {
      if(cookies_.in() == 0)
      {
        cookies_ = new El::Net::HTTP::Python::CookieSeq();
        
        cookies_->from_container<El::Net::HTTP::Python::Cookie>(
          request_->in().cookies());
      }
      
      return cookies_.add_ref();
    }

    PyObject*
    Request::In::py_accept_languages() throw(El::Exception)
    {
      if(accept_languages_.in() == 0)
      {
        accept_languages_ = new El::Net::HTTP::Python::AcceptLanguageSeq();
        
        accept_languages_->from_container<El::Net::HTTP::Python::AcceptLanguage>(
          request_->in().accept_languages());
      }
      
      return accept_languages_.add_ref();
    }
    
    //
    // Request::Out class
    //
    Request::Out::Out(PyTypeObject *type, PyObject *args, PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type),
          request_(0)
    {
      throw Exception(
        "El::PSP::Request::Out::Out: unforseen way of object creation");
    }
    
    Request::Out::Out(El::Apache::Request& request,
                      El::PSP::Localization* localization) throw(El::Exception)
        : El::Python::ObjectImpl(&Type::instance),
          request_(&request),
          stream_(new Request::Out::Stream(request, localization))
    {
    }

    void
    Request::Out::localization(El::PSP::Localization* val)
      throw(El::Exception)
    {
      stream_->localization(val);
    }

    PyObject*
    Request::Out::py_content_type(PyObject* args) throw(El::Exception)
    {
      char* value = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s:el.psp.RequestOut.content_type",
                           &value))
      {
        El::Python::handle_error("El::PSP::Request::Out::py_content_type");
      }

      if(value == 0 || *value == '\0')
      {
        El::Python::report_error(
          PyExc_TypeError,
          "Non-empty string expected as a parameter for "
          "el.psp.RequestOut.content_type",
          "El::PSP::Request::Out::py_content_type");
      }
      
      request_->out().content_type(value);
        
      return El::Python::add_ref(Py_None);
    }
    
    PyObject*
    Request::Out::py_send_header(PyObject* args) throw(El::Exception)
    {
      char* name = 0;
      char* value = 0;
      
      if(!PyArg_ParseTuple(args,
                           "ss:el.psp.RequestOut.send_header",
                           &name,
                           &value))
      {
        El::Python::handle_error("El::PSP::Request::Out::py_send_header");
      }

      if(name == 0 || *name == '\0')
      {
        El::Python::report_error(
          PyExc_TypeError,
          "Non-empty string expected as a name parameter for "
          "el.psp.RequestOut.send_header",
          "El::PSP::Request::Out::py_send_header");
      }      
      
      request_->out().send_header(name, value ? value : "");
        
      return El::Python::add_ref(Py_None);
    }

    PyObject*
    Request::Out::py_send_cookie(PyObject* args) throw(El::Exception)
    {
      PyObject* cookie_setter = 0;
      
      if(!PyArg_ParseTuple(args,
                           "O:el.psp.RequestOut.send_cookie",
                           &cookie_setter))
      {
        El::Python::handle_error("El::PSP::Request::Out::py_send_cookie");
      }

      if(cookie_setter == 0)
      {
        El::Python::report_error(
          PyExc_TypeError,
          "Cookie setter should be passed for "
          "el.psp.RequestOut.send_header",
          "El::PSP::Request::Out::py_send_cookie");
      }
      
      El::Net::HTTP::Python::CookieSetter* setter =
        El::Net::HTTP::Python::CookieSetter::Type::down_cast(cookie_setter);
      
      request_->out().send_cookie(*setter);
        
      return El::Python::add_ref(Py_None);
    }

    PyObject*
    Request::Out::py_send_location(PyObject* args) throw(El::Exception)
    {
      char* url = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s:el.psp.RequestOut.send_location",
                           &url))
      {
        El::Python::handle_error("El::PSP::Request::Out::py_send_location");
      }

      request_->out().send_location(url);
      return El::Python::add_ref(Py_None);
    }

    //
    // Request::Out::LocalizationMarker class
    //
    Request::Out::LocalizationMarker::LocalizationMarker(PyTypeObject *type,
                                                         PyObject *args,
                                                         PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type)
    {
    }

    //
    // Request::Out::HexMarker class
    //
    Request::Out::HexMarker::HexMarker(PyTypeObject *type,
                                       PyObject *args,
                                       PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type)
    {
    }

    //
    // Request::Out::DecMarker class
    //
    Request::Out::DecMarker::DecMarker(PyTypeObject *type,
                                       PyObject *args,
                                       PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type)
    {
    }

    //
    // Request::Out::UppercaseMarker class
    //
    Request::Out::UppercaseMarker::UppercaseMarker(PyTypeObject *type,
                                                   PyObject *args,
                                                   PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type)
    {
    }

    //
    // Request::Out::NouppercaseMarker class
    //
    Request::Out::NouppercaseMarker::NouppercaseMarker(PyTypeObject *type,
                                                       PyObject *args,
                                                       PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type)
    {
    }

    //
    // Request::Out::JS_EscapeMarker class
    //
    Request::Out::JS_EscapeMarker::JS_EscapeMarker(PyTypeObject *type,
                                                   PyObject *args,
                                                   PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type)
    {
    }

    //
    // Request::Out::NoJS_EscapeMarker class
    //
    Request::Out::NoJS_EscapeMarker::NoJS_EscapeMarker(PyTypeObject *type,
                                                       PyObject *args,
                                                       PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type)
    {
    }

    //
    // Request::Out::Stream class
    //
    Request::Out::Stream::Stream(PyTypeObject *type,
                                 PyObject *args,
                                 PyObject *kwds)
      throw(El::Exception)
        : El::Python::ObjectImpl(type),
          request_(0)
    {
      throw Exception(
        "El::PSP::Request::Out::Stream::Stream: unforseen way "
        "of object creation");
    }

    Request::Out::Stream::Stream(El::Apache::Request& request,
                                 El::PSP::Localization* localization)
      throw(El::Exception)
        : El::Python::ObjectImpl(&Type::instance),
          request_(&request),
          localization_(localization ? localization->localization.in() : 0),
          js_escape_(false)
    {
    }

    void
    Request::Out::Stream::localization(El::PSP::Localization* val)
      throw(El::Exception)
    {
      localization_ = val ? val->localization.in() : 0;
    }
    
    PyObject*
    Request::Out::Stream::py_printf(PyObject* args) throw(El::Exception)
    {
      if(args)
      {
        unsigned long size = PyTuple_Size(args);

        if(size)
        {
          PyObject* format = PyTuple_GetItem(args, 0);
          El::Python::Object_var new_args = PyTuple_GetSlice(args, 1, size);
          
          El::Python::Object_var res = PyString_Format(format, new_args.in());

          if(res.in() == 0)
          {
            El::Python::handle_error(
              "El::PSP::Request::Out::Stream::py_printf: "
              "PyString_Format failed");
          }

          const char* res_str = PyString_AsString(res.in());

          if(res_str == 0)
          {
            El::Python::handle_error(
              "El::PSP::Request::Out::Stream::py_printf: "
              "PyString_AsString failed");
          }
          
          request_->out().stream() << res_str;
        }
      }
      
      return El::Python::add_ref(Py_None);
    }

    PyObject*
    Request::Out::Stream::py_prn2(PyObject* args) throw(El::Exception)
    {
      return py_prn(args);
    }
    
    PyObject*
    Request::Out::Stream::py_prn(PyObject* args) throw(El::Exception)
    {
      unsigned long size = args ? PyTuple_Size(args) : 0;
      bool localization_mark = false;
      El::String::Template::Python::Parser_var template_parser;

      std::ostream& output = request_->out().stream();
      
      for(unsigned long i = 0; i < size; i++)
      {
        PyObject* obj = PyTuple_GetItem(args, i);

        if(localization_mark && !PyString_Check(obj))
        {
          El::Python::report_error(
            PyExc_TypeError,
            "string expected after el.psp.LocalizationMarker",
            "El::PSP::Localization::py_prn");
        }

        if(template_parser.in() != 0 && !PyMapping_Check(obj))
        {
          El::Python::report_error(
            PyExc_TypeError,
            "map expected after el.string.template.Parser",
            "El::PSP::Localization::py_prn");
        }

        if(Request::Out::LocalizationMarker::Type::check_type(obj) ||
           Request::Out::LocalizationMarker::Type::is_type(obj))
        {
          localization_mark = true;
          continue;
        }
        else if(Request::Out::HexMarker::Type::check_type(obj) ||
                Request::Out::HexMarker::Type::is_type(obj))
        {
          output << std::hex;
          continue;
        }
        else if(Request::Out::DecMarker::Type::check_type(obj) ||
                Request::Out::DecMarker::Type::is_type(obj))
        {
          output << std::dec;
          continue;
        }
        else if(Request::Out::UppercaseMarker::Type::check_type(obj) ||
                Request::Out::UppercaseMarker::Type::is_type(obj))
        {
          output << std::uppercase;
          continue;
        }
        else if(Request::Out::NouppercaseMarker::Type::check_type(obj) ||
                Request::Out::NouppercaseMarker::Type::is_type(obj))
        {
          output << std::nouppercase;
          continue;
        }
        else if(El::String::Template::Python::Parser::Type::check_type(obj))
        {
          template_parser =
            El::String::Template::Python::Parser::Type::down_cast(obj, true);
          
          continue;
        }
        else if(Request::Out::JS_EscapeMarker::Type::check_type(obj) ||
                Request::Out::JS_EscapeMarker::Type::is_type(obj))
        {
          js_escape_ = true;
          continue;
        }
        else if(Request::Out::NoJS_EscapeMarker::Type::check_type(obj) ||
                Request::Out::NoJS_EscapeMarker::Type::is_type(obj))
        {
          js_escape_ = false;
          continue;
        }

        if(template_parser.in() != 0)
        {
          El::String::Template::VariablesMap variables;
          El::String::Template::Python::Parser::map_to_vars(obj, variables);
          
          if(js_escape_)
          {
            std::ostringstream ostr;
            template_parser->instantiate(variables, ostr);
            El::String::Manip::js_escape(ostr.str().c_str(), output);
          }
          else
          {
            template_parser->instantiate(variables, output);
          }
          
          template_parser = 0;
          continue;
        }        

        El::Python::Object_var converted_obj;
        
        if(PyUnicode_Check(obj))
        {
          converted_obj = PyUnicode_AsUTF8String(obj);
          obj = converted_obj.in();
        }
        
        if(PyString_Check(obj))
        {
          size_t slen = 0;
          
          const char* res_str = El::Python::string_from_string(
            obj,
            slen,
            "El::PSP::Request::Out::Stream::py_print[1]");

          if(localization_mark)
          {
            localization_mark = false;

            if(localization_)
            {
              try
              {
                if(js_escape_)
                {
                  El::String::Manip::js_escape(
                    localization_->get(res_str).c_str(), output);
                }
                else
                {
                  output << localization_->get(res_str);
                }
                
                continue;
              }
              catch(const El::Cache::VariablesMap::VariableNotFound& )
              {
              }
            }

            std::ostringstream ostr;
            ostr << "localized string not found for id '" << res_str << "'";
            
            El::Python::report_error(PyExc_LookupError,
                                     ostr.str().c_str(),
                                     "El::PSP::Localization::py_prn");
          }
            
          if(js_escape_)
          {
            El::String::Manip::js_escape(res_str, output);
          }
          else
          {
            output.write(res_str, slen);
          }
          
          continue;
        }

        if(PyNumber_Check(obj))
        {
          El::Python::Object_var long_obj = PyNumber_Long(obj);

          if(long_obj.in() == 0)
          {
            El::Python::handle_error(
              "El::PSP::Request::Out::Stream::py_prn: "
              "PyNumber_Long failed");            
          }

          long long lval = PyLong_AsLongLong(long_obj.in());
          PyObject* err = PyErr_Occurred();

          if(err == 0)
          {
            output << lval;
            continue;
          }
          
          if(PyErr_GivenExceptionMatches(err, PyExc_OverflowError))
          {
            PyErr_Clear();
            
            unsigned long long ulval =
              PyLong_AsUnsignedLongLong(long_obj.in());
            
            err = PyErr_Occurred();

            if(err == 0)
            {
              output << ulval;
              continue;
            }
          }
          
          El::Python::handle_error(
            "El::PSP::Request::Out::Stream::py_prn: "
            "PyLong_AsUnsignedLongLong failed");            
        }
        
        El::Python::Object_var str =
          El::Python::string_from_object(
            obj,
            "El::PSP::Request::Out::Stream::py_print");

        size_t slen = 0;
        const char* res_str =
          El::Python::string_from_string(
            str.in(),
            slen,
            "El::PSP::Request::Out::Stream::py_print[2]");
        
        if(js_escape_)
        {
          El::String::Manip::js_escape(res_str, output);
        }
        else
        {
          output << res_str;
        }
      }
      
      return El::Python::add_ref(Py_None);
    }

    //
    // Request::Forward class
    //
    Request::Forward::Forward(PyTypeObject *type,
                              PyObject *args,
                              PyObject *kwds)
      throw(El::Exception) :
        El::Python::ObjectImpl(type),
        is_script(false)
    {
      char* path = 0;
      PyObject* fw_args = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s|O:el.psp.Forward.Forward",
                           &path,
                           &fw_args))
      {
        El::Python::handle_error("El::PSP::Request::Forward::Forward");
      }

      const char* ext = 0;
      if(path == 0 || (ext = strrchr(path, '.')) == 0 ||
         (!(is_script = strcmp(ext, ".psp") == 0) && strcmp(ext, ".tsp")))
      {
        El::Python::report_error(
          PyExc_TypeError,
          "1st argument of el.psp.Forward.Forward should be a path to a file "
          "with '.psp' or '.tsp' extension",
          "El::PSP::Request::Forward::Forward");
      }

      dest_path = path;
      parameter = El::Python::add_ref(fw_args);
    }
    
  }
}
