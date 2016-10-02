/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/Request.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_REQUEST_HPP_
#define _ELEMENTS_EL_PSP_REQUEST_HPP_

#include <string>
#include <map>

#include <ext/hash_set>

#include <El/Exception.hpp>

#include <El/Lang.hpp>
#include <El/Geography/AddressInfo.hpp>
#include <El/Apache/Request.hpp>

#include <El/Python/RefCount.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Sequence.hpp>
#include <El/Python/Lang.hpp>
#include <El/Python/TimeValue.hpp>

#include <El/PSP/Exception.hpp>
#include <El/PSP/Localization.hpp>
#include <El/PSP/RequestCache.hpp>

namespace El
{
  namespace PSP
  {
    class Request : public El::Python::ObjectImpl
    {
    public:
      typedef __gnu_cxx::hash_set<El::Lang, El::Hash::Lang> LangSet;
      typedef std::map<std::string, El::Lang> LangMap;

    public:      
      Request(El::Apache::Request& request,
              const El::Lang& lang,
//              const LangSet& valid_languages,
              const LangMap& valid_languages,
              El::PSP::Localization* localization)
        throw(El::Exception);
      
      Request(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);

      virtual ~Request() throw() {}

      void language(const El::Lang& lang, 
                    El::PSP::Localization* localization)
        throw(El::Exception);

      El::Apache::Request* request() const throw() { return request_; }
      
      class Forward : public El::Python::ObjectImpl
      {
      public:
          
        Forward(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Exception);
          
        virtual ~Forward() throw() {}

        class Type : public El::Python::ObjectTypeImpl<Forward,
                                                       Forward::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;
        };

        std::string dest_path;
        bool is_script;
        El::Python::Object_var parameter;
      };

      typedef El::Python::SmartPtr<Forward> Forward_var;
      
    private:

      class In : public El::Python::ObjectImpl
      {
      public:

        In(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Exception);
        
        In(El::Apache::Request& request,
           const El::Lang& lang,
           const LangMap& valid_languages)
          throw(El::Exception);        

        virtual ~In() throw() {}

        void lang(const El::Lang& val) throw(El::Exception);
        const El::Lang& lang() const throw() { return *lang_; }
    
        PyObject* py_headers() throw(El::Exception);
        PyObject* py_parameters() throw(El::Exception);
        PyObject* py_cookies() throw(El::Exception);
        PyObject* py_accept_languages() throw(El::Exception);
        
        PyObject* py_locale(PyObject* args, PyObject *kwds)
          throw(El::Exception);
        
        class Type : public El::Python::ObjectTypeImpl<In, In::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_METHOD_NOARGS(
            py_headers,
            "headers",
            "Returns http request headers");

          PY_TYPE_METHOD_NOARGS(
            py_parameters,
            "parameters",
            "Returns http request parameters");

          PY_TYPE_METHOD_NOARGS(
            py_cookies,
            "cookies",
            "Returns http request cookies");

          PY_TYPE_METHOD_NOARGS(
            py_accept_languages,
            "accept_languages",
            "Returns http request accept language sequence");

          PY_TYPE_METHOD_KWDS(py_locale, "locale", "User locale");
          
          PY_TYPE_MEMBER_OBJECT(lang_,
                                El::Python::Lang::Type,
                                "lang",
                                "Requested resource language",
                                false);
        };

      private:

        El::Country detect_country(const El::Lang& lang,
                                   unsigned long step = 0)
          const throw(El::Exception);

      private:
        El::Apache::Request* request_;
        El::Python::Sequence_var headers_;
        El::Python::Sequence_var parameters_;
        El::Python::Sequence_var cookies_;
        El::Python::Sequence_var accept_languages_;
        El::Python::Lang_var lang_;
        LangMap valid_languages_;
        LangSet final_languages_;

        static El::Geography::AddressInfo address_info_;
      };

      typedef El::Python::SmartPtr<In> In_var;
      
      class Out : public El::Python::ObjectImpl
      {
      public:

        Out(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Exception);
        
        Out(El::Apache::Request& request,
            El::PSP::Localization* localization) throw(El::Exception);        

        virtual ~Out() throw() {}

        void localization(El::PSP::Localization* val) throw(El::Exception);

        class Stream : public El::Python::ObjectImpl
        {
        public:
          
          Stream(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);
          
          Stream(El::Apache::Request& request,
                 El::PSP::Localization* localization) throw(El::Exception);

          virtual ~Stream() throw() {}

          void localization(El::PSP::Localization* val) throw(El::Exception);
          
          PyObject* py_printf(PyObject* args) throw(El::Exception);
          PyObject* py_prn(PyObject* args) throw(El::Exception);
          PyObject* py_prn2(PyObject* args) throw(El::Exception);
          
          class Type : public El::Python::ObjectTypeImpl<Stream, Stream::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_METHOD_VARARGS(
              py_printf,
              "printf",
              "Perform formatting output into http responce body stream");

            PY_TYPE_METHOD_VARARGS(
              py_prn,
              "prn",
              "Perform output into http responce body stream");

            PY_TYPE_METHOD_VARARGS(
              py_prn2,
              "prn2",
              "Just calls prn. Usefeul or debuging purposes.");
          };

        private:
          El::Apache::Request* request_;
          El::Cache::VariablesMap* localization_;
          bool js_escape_;
        };
    
        typedef El::Python::SmartPtr<Stream> Stream_var;
        
        class LocalizationMarker : public El::Python::ObjectImpl
        {
        public:
          
          LocalizationMarker(PyTypeObject *type,
                             PyObject *args,
                             PyObject *kwds)
            throw(El::Exception);
          
          virtual ~LocalizationMarker() throw() {}

          class Type :
            public El::Python::ObjectTypeImpl<LocalizationMarker,
                                              LocalizationMarker::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
          };
        };
    
        class HexMarker : public El::Python::ObjectImpl
        {
        public:
          
          HexMarker(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);
          
          virtual ~HexMarker() throw() {}

          class Type :
            public El::Python::ObjectTypeImpl<HexMarker, HexMarker::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
          };
        };
    
        class DecMarker : public El::Python::ObjectImpl
        {
        public:
          
          DecMarker(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);
          
          virtual ~DecMarker() throw() {}

          class Type :
            public El::Python::ObjectTypeImpl<DecMarker, DecMarker::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
          };
        };
    
        class UppercaseMarker : public El::Python::ObjectImpl
        {
        public:
          
          UppercaseMarker(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);
          
          virtual ~UppercaseMarker() throw() {}

          class Type :
            public El::Python::ObjectTypeImpl<UppercaseMarker,
                                              UppercaseMarker::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
          };
        };
    
        class NouppercaseMarker : public El::Python::ObjectImpl
        {
        public:
          
          NouppercaseMarker(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);
          
          virtual ~NouppercaseMarker() throw() {}

          class Type :
            public El::Python::ObjectTypeImpl<NouppercaseMarker,
                                              NouppercaseMarker::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
          };
        };
    
        class JS_EscapeMarker : public El::Python::ObjectImpl
        {
        public:
          
          JS_EscapeMarker(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);
          
          virtual ~JS_EscapeMarker() throw() {}

          class Type :
            public El::Python::ObjectTypeImpl<JS_EscapeMarker,
                                              JS_EscapeMarker::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
          };
        };
    
        class NoJS_EscapeMarker : public El::Python::ObjectImpl
        {
        public:
          
          NoJS_EscapeMarker(PyTypeObject *type, PyObject *args, PyObject *kwds)
            throw(El::Exception);
          
          virtual ~NoJS_EscapeMarker() throw() {}

          class Type :
            public El::Python::ObjectTypeImpl<NoJS_EscapeMarker,
                                              NoJS_EscapeMarker::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;
          };
        };
    
        PyObject* py_content_type(PyObject* args) throw(El::Exception);
        PyObject* py_send_header(PyObject* args) throw(El::Exception);
        PyObject* py_send_cookie(PyObject* args) throw(El::Exception);
        PyObject* py_send_location(PyObject* args) throw(El::Exception);
        
        class Type : public El::Python::ObjectTypeImpl<Out, Out::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_MEMBER_OBJECT(stream_,
                                Stream::Type,
                                "stream",
                                "Request output stream object",
                                false);

          PY_TYPE_METHOD_VARARGS(
            py_content_type,
            "content_type",
            "Sets http response content type");

          PY_TYPE_METHOD_VARARGS(
            py_send_header,
            "send_header",
            "Sends http response header");

          PY_TYPE_METHOD_VARARGS(
            py_send_cookie,
            "send_cookie",
            "Sends http response cookie");

          PY_TYPE_METHOD_VARARGS(
            py_send_location,
            "send_location",
            "Sends Location header with URL normalized");
        };

      private:
        El::Apache::Request* request_;
        Stream_var stream_;
      };

      typedef El::Python::SmartPtr<Out> Out_var;

      class CacheEntry : public El::Python::ObjectImpl
      {
      public:

        CacheEntry(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Exception);
        
        CacheEntry(RequestCache::Entry* entry) throw(El::Exception);

        virtual ~CacheEntry() throw() {}
    
        PyObject* py_timeout(PyObject* args) throw(El::Exception);
        PyObject* py_unused_timeout(PyObject* args) throw(El::Exception);
        PyObject* py_etag(PyObject* args) throw(El::Exception);
        
        class Type :
          public El::Python::ObjectTypeImpl<CacheEntry, CacheEntry::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_METHOD_VARARGS(py_timeout,
                                 "timeout",
                                 "Sets cache entry timeout");

          PY_TYPE_METHOD_VARARGS(py_unused_timeout,
                                 "unused_timeout",
                                 "Sets cache entry unused timeout");
          
          PY_TYPE_METHOD_VARARGS(py_etag,
                                 "etag",
                                 "Sets cache entry etag");
        };

      private:
        RequestCache::Entry_var entry_;
      };
    
      PyObject* py_unparsed_uri() throw(El::Exception);
      PyObject* py_uri() throw(El::Exception);
      PyObject* py_args() throw(El::Exception);
      PyObject* py_remote_ip() throw(El::Exception);
      PyObject* py_host() throw(El::Exception);
      PyObject* py_endpoint() throw(El::Exception);
      PyObject* py_method() throw(El::Exception);
      PyObject* py_port() throw(El::Exception);
      PyObject* py_method_number() throw(El::Exception);
      PyObject* py_time() throw(El::Exception);
      PyObject* py_secure() throw(El::Exception);

    public:
      
      In* input() throw() { return in_.in(); }
      Out* output() throw() { return out_.in(); }

      class Type : public El::Python::ObjectTypeImpl<Request, Request::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        static Type instance;

        PY_TYPE_MEMBER_OBJECT(cache_entry_,
                              CacheEntry::Type,
                              "cache_entry",
                              "Request cache entry object",
                              false);
        
        PY_TYPE_MEMBER_OBJECT(in_,
                              In::Type,
                              "input",
                              "Request input object",
                              false);
        
        PY_TYPE_MEMBER_OBJECT(out_,
                              Out::Type,
                              "output",
                              "Request output object",
                              false);

        PY_TYPE_METHOD_NOARGS(py_unparsed_uri,
                              "unparsed_uri",
                              "Unparsed request URI");
        
        PY_TYPE_METHOD_NOARGS(py_uri,
                              "uri",
                              "Request URI");
        
        PY_TYPE_METHOD_NOARGS(py_args,
                              "args",
                              "Request arguments");
        
        PY_TYPE_METHOD_NOARGS(py_remote_ip,
                              "remote_ip",
                              "Remote IP address");
        
        PY_TYPE_METHOD_NOARGS(py_host,
                              "host",
                              "Requested hostname");
        
        PY_TYPE_METHOD_NOARGS(py_endpoint,
                              "endpoint",
                              "Requested endpoint");
        
        PY_TYPE_METHOD_NOARGS(py_method,
                              "method",
                              "Request HTTP-method");
        
        PY_TYPE_METHOD_NOARGS(py_port,
                              "port",
                              "Request port");
        
        PY_TYPE_METHOD_NOARGS(py_method_number,
                              "method_number",
                              "Requst method number");
        
        PY_TYPE_METHOD_NOARGS(py_time,
                              "time",
                              "Returns http request time value");

        PY_TYPE_METHOD_NOARGS(py_secure,
                              "secure",
                              "Checks if SSL is used");
      };
      
    private:
      El::Apache::Request* request_;
      In_var in_;
      Out_var out_;
      El::PSP::Localization_var localization_;
      El::Python::Object_var cache_entry_;
    };

    typedef El::Python::SmartPtr<Request> Request_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace PSP
  {
    //
    // El::PSP::Request class
    //
    inline
    PyObject*
    Request::py_secure() throw(El::Exception)
    {
      return PyBool_FromLong(request_->secure());
    }
    
    inline
    PyObject*
    Request::py_time() throw(El::Exception)
    {
      return new El::Python::TimeValue(request_->time());
    }
    
    inline
    PyObject*
    Request::py_unparsed_uri() throw(El::Exception)
    {
      return PyString_FromString(request_->unparsed_uri());
    }
    
    inline
    PyObject*
    Request::py_uri() throw(El::Exception)
    {
      return PyString_FromString(request_->uri());
    }
    
    inline
    PyObject*
    Request::py_args() throw(El::Exception)
    {
      return PyString_FromString(request_->args());
    }
    
    inline
    PyObject*
    Request::py_remote_ip() throw(El::Exception)
    {
      return PyString_FromString(request_->remote_ip());
    }
    
    inline
    PyObject*
    Request::py_host() throw(El::Exception)
    {
      return PyString_FromString(request_->host());
    }
    
    inline
    PyObject*
    Request::py_endpoint() throw(El::Exception)
    {
      return PyString_FromString(request_->endpoint());
    }
    
    inline
    PyObject*
    Request::py_method() throw(El::Exception)
    {
      return PyString_FromString(request_->method());
    }
    
    inline
    PyObject*
    Request::py_port() throw(El::Exception)
    {
      return PyInt_FromLong(request_->port());
    }
    
    inline
    PyObject*
    Request::py_method_number() throw(El::Exception)
    {
      return PyInt_FromLong(request_->method_number());
    }
    
    //
    // El::PSP::Request::Type class
    //
    inline
    Request::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Request, Request::Type>(
          "el.psp.Request",
          "Object encapsulating El::Apache::Request functionality")
    {
      tp_new = 0;
    }

    //
    // El::PSP::Request::In::Type class
    //
    inline
    Request::In::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<In, In::Type>(
          "el.psp.RequestIn",
          "Object encapsulating El::Apache::Request::In functionality")
    {
      tp_new = 0;
    }

    //
    // El::PSP::Request::Out::Type class
    //
    inline
    Request::Out::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Out, Out::Type>(
          "el.psp.RequestOut",
          "Object encapsulating El::Apache::Request::Out functionality")
    {
      tp_new = 0;
    }

    //
    // El::PSP::Request::Out::Stream::Type class
    //
    inline
    Request::Out::Stream::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Stream, Stream::Type>(
          "el.psp.RequestOutStream",
          "Object encapsulating El::Apache::OutStream functionality")
    {
      tp_new = 0;
    }
    
    //
    // El::PSP::Request::Out::LocalizationMarker::Type class
    //
    inline
    Request::Out::LocalizationMarker::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<LocalizationMarker,
                                     LocalizationMarker::Type>(
          "el.psp.LocalizationMarker",
          "Object denoting localized string identifier coming next in "
          "el.psp.RequestOutStream.prn arguments list")
    {
    }

    //
    // El::PSP::Request::Out::HexMarker::Type class
    //
    inline
    Request::Out::HexMarker::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<HexMarker, HexMarker::Type>(
          "el.psp.HexMarker",
          "Object setting hexdecimal representation for numeric arguments of "
          "el.psp.RequestOutStream.prn")
    {
    }

    //
    // El::PSP::Request::Out::DecMarker::Type class
    //
    inline
    Request::Out::DecMarker::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<DecMarker, DecMarker::Type>(
          "el.psp.DecMarker",
          "Object setting decimal representation for numeric arguments of "
          "el.psp.RequestOutStream.prn")
    {
    }

    //
    // El::PSP::Request::Out::UppercaseMarker::Type class
    //
    inline
    Request::Out::UppercaseMarker::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<UppercaseMarker, UppercaseMarker::Type>(
          "el.psp.UppercaseMarker",
          "Object setting uppercase representation for hexdecimal "
          "arguments of el.psp.RequestOutStream.prn")
    {
    }

    //
    // El::PSP::Request::Out::NouppercaseMarker::Type class
    //
    inline
    Request::Out::NouppercaseMarker::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<NouppercaseMarker,
                                     NouppercaseMarker::Type>(
          "el.psp.NouppercaseMarker",
          "Object resetting uppercese representation for hexdecimal "
          "arguments of el.psp.RequestOutStream.prn")
    {
    }

    //
    // El::PSP::Request::Out::JS_EscapeMarker::Type class
    //
    inline
    Request::Out::JS_EscapeMarker::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<JS_EscapeMarker, JS_EscapeMarker::Type>(
          "el.psp.JS_EscapeMarker",
          "Object telling el.psp.RequestOutStream.prn to JS-escape strings")
    {
    }    
    
    //
    // El::PSP::Request::Out::NoJS_EscapeMarker::Type class
    //
    inline
    Request::Out::NoJS_EscapeMarker::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<NoJS_EscapeMarker,
                                     NoJS_EscapeMarker::Type>(
          "el.psp.NoJS_EscapeMarker",
          "Object telling el.psp.RequestOutStream.prn not "
          "to JS-escape strings")
    {
    }    
    
    //
    // El::PSP::Request::Forward::Type class
    //
    inline
    Request::Forward::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Forward, Forward::Type>(
          "el.psp.Forward",
          "Being returned by PSP-script inform PSP-engine to forward request "
          "execution to the specified PSP-script or PSP-template")
    {
    }

    //
    // El::PSP::Request::CacheEntry::Type class
    //
    inline
    Request::CacheEntry::Type::Type()
      throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<CacheEntry, CacheEntry::Type>(
          "el.psp.CacheEntry",
          "Object wrapping around El::PSP::RequestCache::Entry")
    {
    }
  }
}

#endif // _ELEMENTS_EL_PSP_REQUEST_HPP_
