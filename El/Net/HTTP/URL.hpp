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
 * @file Elements/El/Net/HTTP/URL.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_URL_HPP_
#define _ELEMENTS_EL_NET_HTTP_URL_HPP_

#include <stdint.h>

#include <strings.h>
#include <string.h>

#include <string>
#include <sstream>

#include <El/Exception.hpp>

#include <El/RefCount/All.hpp>

#include <El/Net/URL.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      class URL : virtual public El::Net::URL
      {
      public:
        EL_EXCEPTION(Exception, El::Net::URL::Exception);
        EL_EXCEPTION(InvalidArg, El::Net::URL::InvalidArg);

      public:

        enum TreatFlags
        {
          TFL_SPACE = 0x1,
          TFL_NON_ASCII = 0x2,
          TFL_NON_MIME = 0x4
        };
        
        URL(const char* str,
            unsigned long treat_flags = TFL_SPACE | TFL_NON_ASCII)
          throw(InvalidArg, El::Exception);

        URL(const char* host,
            const char* path,
            const char* params = 0,
            const char* anchor = 0,
            uint16_t port = 0,
            bool secure = false)
          throw(InvalidArg, El::Exception);

        URL(const URL& url) throw(El::Exception);

        virtual ~URL() throw();

        const char* host() const throw(El::Exception);
        const char* idn_host() const throw(El::Exception);
        uint16_t port() const throw();
        const char* path() const throw(El::Exception);
        const char* params() const throw(El::Exception);
        const char* anchor() const throw(El::Exception);
        bool secure() const throw();

        std::string schema() const throw(El::Exception);
        std::string schema_and_endpoint() const throw(El::Exception);
        
        std::string abs_url(const char* path, bool lax = true) const
          throw(InvalidArg, El::Exception);

        bool operator==(const URL& url) const throw(El::Exception);
        bool operator!=(const URL& url) const throw(El::Exception);

        URL& operator=(const URL& url) throw(El::Exception);

      protected:
        void normalize() throw(InvalidArg, El::Exception);

        void set_full_url(std::string& url, const std::string& host) const
          throw(El::Exception);
        
        static void treat(const char* url,
                          std::string& treated,
                          unsigned long flags)
          throw(InvalidArg, El::Exception);
    
      protected:
        std::string host_;
        std::string idn_host_;
        std::string path_;
        std::string params_;
        std::string anchor_;
        uint16_t port_;
        bool secure_;
      };

      typedef RefCount::SmartPtr<URL> URL_var;
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
      inline
      URL::URL(const char* host,
               const char* path,
               const char* params,
               const char* anchor,
               uint16_t port,
               bool secure)
        throw(InvalidArg, El::Exception)
          : host_(host ? host : ""),
            path_(path ? path : ""),
            params_(params ? params : ""),
            anchor_(anchor ? anchor : ""),
            port_(port),
            secure_(secure)
      {
        normalize();
      }
      
      inline
      URL::URL(const URL& url) throw(El::Exception)
          : El::Net::URL(url),
            host_(url.host_),
            idn_host_(url.idn_host_),
            path_(url.path_),
            params_(url.params_),
            anchor_(url.anchor_),
            port_(url.port_),
            secure_(url.secure_)
      {
      }

      inline
      URL::~URL() throw()
      {
      }

      inline
      std::string
      URL::schema() const throw(El::Exception)
      {
        return secure_ ? "https:" : "http:";
      }
      
      inline
      std::string
      URL::schema_and_endpoint() const throw(El::Exception)
      {
        std::ostringstream ostr;
        ostr << (secure_ ? "https://" : "http://") << host_;
        
        if(port_ != (secure_ ? 443 : 80))
        {
          ostr << ":" << port_;
        }
        
        return ostr.str();
      }
      
      inline
      const char*
      URL::host() const throw(El::Exception)
      {
        return host_.c_str();
      }

      inline
      const char*
      URL::idn_host() const throw(El::Exception)
      {
        return idn_host_.c_str();
      }

      inline
      uint16_t
      URL::port() const throw()
      {
        return port_;
      }
      
      inline
      const char*
      URL::path() const throw(El::Exception)
      {
        return path_.c_str();
      }
        
      inline
      const char*
      URL::params() const throw(El::Exception)
      {
        return params_.c_str();
      }
        
      inline
      const char*
      URL::anchor() const throw(El::Exception)
      {
        return anchor_.c_str();
      }
        
      inline
      bool
      URL::secure() const throw()
      {
        return secure_;
      }
      
      inline
      void
      URL::normalize() throw(InvalidArg, El::Exception)
      {
        if(host_.empty())
        {
          throw InvalidArg("El::Net::HTTP::URL::normalize: host undefined");
        }

        if(path_.empty())
        {
          path_ = "/";
        }

        if(!port_)
        {
          port_ = secure_ ? 443 : 80;
        }

        set_full_url(string_, host_);
        set_full_url(idn_string_, idn_host_);        
      }

      inline
      void
      URL::set_full_url(std::string& url, const std::string& host) const
        throw(El::Exception)
      {
        std::ostringstream ostr;
        ostr << (secure_ ? std::string("https://") : std::string("http://"))
             << host;

        if(secure_ ? (port_ != 443) : (port_ != 80))
        {
          ostr << ":" << port_;
        }

        ostr << path_;

        if(!params_.empty())
        {
          ostr << "?" << params_;          
        }
        
        if(!anchor_.empty())
        {
          ostr << "#" << anchor_;
        }
        
        url = ostr.str();
      }
      
      inline
      bool
      URL::operator==(const URL& url) const throw(El::Exception)
      {
        return !strcmp(url.string(), string());
      }
      
      inline
      bool
      URL::operator!=(const URL& url) const throw(El::Exception)
      {
        return strcmp(url.string(), string()) != 0;
      }

      inline
      URL&
      URL::operator=(const URL& url) throw(El::Exception)
      {
        El::Net::URL::operator=(url);
        
        host_ = url.host_;
        idn_host_ = url.idn_host_;
        path_ = url.path_;
        params_ = url.params_;
        anchor_ = url.anchor_;
        port_ = url.port_;
        secure_ = url.secure_;

        return *this;
      }

    }
  }
}

#endif // _ELEMENTS_EL_NET_HTTP_URL_HPP_
