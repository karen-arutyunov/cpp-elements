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
 * @file Elements/El/Net/HTTP/Python/Utility.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_PYTHON_UTILITY_HPP_
#define _ELEMENTS_EL_NET_HTTP_PYTHON_UTILITY_HPP_

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>

#include <El/Net/HTTP/Utility.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      namespace Python
      { 
        class SearchInfo : public El::Python::ObjectImpl,
                           public El::Net::HTTP::SearchInfo
        {
        public:
          SearchInfo(PyTypeObject *type = 0,
                     PyObject *args = 0,
                     PyObject *kwds = 0)
            throw(El::Python::Exception, El::Exception);

          SearchInfo(const El::Net::HTTP::SearchInfo& val)
            throw(El::Exception);

          virtual ~SearchInfo() throw() {}

          virtual void write(El::BinaryOutStream& bstr) const
            throw(El::Exception);
          
          virtual void read(El::BinaryInStream& bstr) throw(El::Exception);

          class Type : public El::Python::ObjectTypeImpl<SearchInfo,
                                                         SearchInfo::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_MEMBER_STRING(engine,
                                  "engine",
                                  "Search engine name",
                                  false);
            
            PY_TYPE_MEMBER_STRING(query, "query", "Search query", true);
            PY_TYPE_MEMBER_STRING(image, "image", "Found image", true);
          };
        };

        typedef El::Python::SmartPtr<SearchInfo> SearchInfo_var;
        
        class RequestParams : public El::Python::ObjectImpl,
                              public El::Net::HTTP::RequestParams
        {
        public:
          RequestParams(PyTypeObject *type = 0,
                        PyObject *args = 0,
                        PyObject *kwds = 0)
            throw(El::Python::Exception, El::Exception);

          RequestParams(const El::Net::HTTP::RequestParams& val)
            throw(El::Exception);
          
          virtual ~RequestParams() throw() {}

          virtual void write(El::BinaryOutStream& bstr) const
            throw(El::Exception);
          
          virtual void read(El::BinaryInStream& bstr) throw(El::Exception);

          class Type : public El::Python::ObjectTypeImpl<RequestParams,
                                                         RequestParams::Type>
          {
          public:
            Type() throw(El::Python::Exception, El::Exception);
            static Type instance;

            PY_TYPE_MEMBER_STRING(user_agent,
                                  "user_agent",
                                  "User-Agent header",
                                  true);

            PY_TYPE_MEMBER_STRING(referer,
                                  "referer",
                                  "Referrer header",
                                  true);
            
            PY_TYPE_MEMBER_ULONG(timeout,
                                 "timeout",
                                 "Socket operation timeout");

            PY_TYPE_MEMBER_ULONG(redirects_to_follow,
                                 "redirects_to_follow",
                                 "Number of HTTP redirects to follow");
            
            PY_TYPE_MEMBER_ULONG(recv_buffer_size,
                                 "recv_buffer_size",
                                 "Size of data receiving buffer");
          };
        };
        
        typedef El::Python::SmartPtr<RequestParams> RequestParams_var;
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
        // El::Net::HTTP::Python::SearchInfo class
        //
        inline
        SearchInfo::SearchInfo(const El::Net::HTTP::SearchInfo& val)
          throw(El::Exception)
            : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Net::HTTP::SearchInfo*>(this) = val;
        }

        inline
        void
        SearchInfo::write(El::BinaryOutStream& bstr) const throw(El::Exception)
        {
          El::Net::HTTP::SearchInfo::write(bstr);
        }
        
        inline
        void
        SearchInfo::read(El::BinaryInStream& bstr) throw(El::Exception)
        {
          El::Net::HTTP::SearchInfo::read(bstr);
        }
        
        //
        // El::Net::HTTP::Python::SearchInfo::Type class
        //
        inline
        SearchInfo::Type::Type() throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<SearchInfo, SearchInfo::Type>(
              "el.net.http.SearchInfo",
              "Object encapsulating El::Net::HTTP::SearchInfo functionality")
        {
        }

        //
        // El::Net::HTTP::Python::RequestParams class
        //
        inline
        RequestParams::RequestParams(const El::Net::HTTP::RequestParams& val)
          throw(El::Exception)
            : ObjectImpl(&Type::instance)
        {
          *static_cast<El::Net::HTTP::RequestParams*>(this) = val;
        }

        inline
        void
        RequestParams::write(El::BinaryOutStream& bstr) const
          throw(El::Exception)
        {
          El::Net::HTTP::RequestParams::write(bstr);
        }

        inline
        void
        RequestParams::read(El::BinaryInStream& bstr) throw(El::Exception)
        {
          El::Net::HTTP::RequestParams::read(bstr);
        }
        
        //
        // El::Net::HTTP::Python::RequestParams::Type class
        //
        inline
        RequestParams::Type::Type() throw(El::Python::Exception, El::Exception)
            : El::Python::ObjectTypeImpl<RequestParams, RequestParams::Type>(
              "el.net.http.RequestParams",
              "Object encapsulating "
                "El::Net::HTTP::RequestParams functionality")
        {
        }

      }
    }
  }
}

#endif // _ELEMENTS_EL_NET_HTTP_PYTHON_UTILITY_HPP_
