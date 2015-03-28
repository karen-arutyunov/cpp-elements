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
 * @file Elements/El/Net/URL.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_URL_HPP_
#define _ELEMENTS_EL_NET_URL_HPP_

#include <string.h>

#include <string>

#include <El/Exception.hpp>
#include <El/Net/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/String/Manip.hpp>
#include <El/BinaryStream.hpp>

namespace El
{
  namespace Net
  {
    class URL : virtual public RefCount::DefaultImpl<>
    {
    public:    
      EL_EXCEPTION(Exception, ::El::Net::Exception);
      EL_EXCEPTION(InvalidArg, Exception);
    
    public:
      URL(const char* str = 0, const char* idn_str = 0) throw (El::Exception);
      URL(const URL& url) throw (El::Exception);
      
      virtual ~URL() throw ();
    
      const char* string() const throw(El::Exception);
      void string(const char* str) throw(El::Exception);
    
      const char* idn_string() const throw(El::Exception);
      void idn_string(const char* str) throw(El::Exception);
    
      URL& operator=(const URL& url) throw (El::Exception);

    protected:
      std::string string_;
      std::string idn_string_;
    };

    typedef RefCount::SmartPtr<URL> URL_var;

    struct IpMask
    {
      EL_EXCEPTION(Exception, ::El::Net::Exception);
      EL_EXCEPTION(InvalidArg, Exception);
      
      uint32_t mask;
      uint32_t subnet;
      
      IpMask(const char* str) throw(InvalidArg);
      IpMask() throw() : mask(0), subnet(0) {}
      
      bool match(const char* ip) const throw();
      bool match(uint32_t ip) const throw();

      void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      void read(El::BinaryInStream& bstr) throw(El::Exception);      
    };

    uint16_t domain_level(const char* domain) throw();
    const char* domain(const char* domain, uint16_t level) throw();

    bool ip(const char* str, uint32_t* pip = 0) throw();

    bool company_domain(const char* domain, std::string* pcompany_domain = 0)
      throw(El::Exception);

    bool subdomain(const char* domain, const char* subdomain) throw();
    
    void idna_encode(const char* src, std::string& dest)
      throw(Exception, El::Exception);

    void idna_decode(const char* src, std::string& dest)
      throw(Exception, El::Exception);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Net
  {
    //
    // URL class
    //
    inline
    URL::URL(const char* str, const char* idn_str) throw (El::Exception)
        : string_(str ? str : ""),
          idn_string_(idn_str ? idn_str : "")
    {
    }
    
    inline
    URL::URL(const URL& url) throw (El::Exception)
        : string_(url.string_),
          idn_string_(url.idn_string_)
    {
    }
      
    inline
    URL::~URL() throw ()
    {
    }
    
    inline
    const char*
    URL::string() const throw(El::Exception)
    {
      return string_.c_str();
    }
    
    inline
    void
    URL::string(const char* str) throw(El::Exception)
    {
      string_ = str ? str : "";
    }

    inline
    const char*
    URL::idn_string() const throw(El::Exception)
    {
      return idn_string_.c_str();
    }
    
    inline
    void
    URL::idn_string(const char* str) throw(El::Exception)
    {
      idn_string_ = str ? str : "";
    }

    inline
    URL&
    URL::operator=(const URL& url) throw (El::Exception)
    {
      string_ = url.string_;
      idn_string_ = url.idn_string_;
      
      return *this;
    }

    //
    // IpMask struct
    //

    inline
    IpMask::IpMask(const char* str) throw(InvalidArg)
        : mask(0),
          subnet(0)
    {
      uint32_t i = 0;
      const char* pos = strchr(str, '/');
      uint32_t bits = 0;

      if(pos)
      {
        std::string ips(str, pos - str);
           
        if(!ip(ips.c_str(), &i) ||
           !El::String::Manip::numeric(pos + 1, bits) || bits > 32 || !bits)
        {
          std::ostringstream ostr;
          ostr << "::El::Net::IpMask::IpMask: invalid ip mask (1) " << str;
          throw InvalidArg(ostr.str());
        }
      }
      else
      {
        if(!ip(str, &i))
        {
          std::ostringstream ostr;
          ostr << "::El::Net::IpMask::IpMask: invalid ip mask (2) " << str;
          throw InvalidArg(ostr.str());
        }

        bits = 32;
      }

      while(bits)
      {
        mask |= 0x80000000 >> (--bits);
      }

      subnet = mask & i;
    }
    
    inline
    bool
    IpMask::match(const char* ip_val) const throw()
    {
      uint32_t i = 0;      
      return ip(ip_val, &i) && match(i);
    }
    
    inline
    bool
    IpMask::match(uint32_t ip_val) const throw()
    {
      return (ip_val & mask) == subnet;
    }

    inline
    void
    IpMask::write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      bstr << subnet << mask;
    }
    
    inline
    void
    IpMask::read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      bstr >> subnet >> mask;
    }
    
    //
    // Helper functions
    //
    inline
    uint16_t
    domain_level(const char* domain) throw()
    {
      uint16_t level = 1;
      
      while(*domain != '\0')
      {
        if(*domain++ == '.')
        {
          level++;
        }
      }

      return level;
    }

    inline
    const char*
    domain(const char* domain, uint16_t level) throw()
    {
      if(domain == 0 || level == 0)
      {
        return 0;
      }

      for(const char* ptr = domain + strlen(domain) - 1; ptr != domain; ptr--)
      {
        if(*ptr == '.')
        {
          if(--level == 0)
          {
            return ptr + 1;
          }
        }
      }

      if(level == 1)
      {
        return domain;
      }

      return 0;
    }
    
    inline
    bool    
    ip(const char* str, uint32_t* pip) throw()
    {
      unsigned int buff[4];

      if(str == 0 ||
         sscanf(str, "%u.%u.%u.%u", buff, buff + 1, buff + 2, buff + 3) != 4)
      {
        return false;
      }
      
      for(size_t i = 0; i < sizeof(buff) / sizeof(buff[0]); i++)
      {
        if(buff[i] > 255)
        {
          return false;
        }
      }

      if(pip)
      {
        *pip = buff[3] | (buff[2] << 8) | (buff[1] << 16) | (buff[0] << 24);
      }
      
      return true;
    }

    inline
    bool
    company_domain(const char* domain, std::string* pcompany_domain)
      throw(El::Exception)
    {
      if(domain == 0 || ip(domain))
      {
        return false;
      }

      while(*domain == '.')
      {
        domain++;
      }
    
      if(*domain == '\0')
      {
        return false;
      }
      
      uint32_t level = 0;
      const char* root_domain = 0;

      const char* ptr = domain + strlen(domain) - 1;
      
      for(; ptr != domain; ptr--)
      {
        if(*ptr == '.')
        {          
          level++;

          if(level == 1)
          {
            root_domain = ptr;
            continue;
          }
          
          if(level == 2)
          {
            std::string second_level_domain;
            second_level_domain.assign(ptr + 1, root_domain - ptr - 1);

            if(strcasecmp(second_level_domain.c_str(), "co") &&
               strcasecmp(second_level_domain.c_str(), "com"))
            {
              if(pcompany_domain)
              {
                *pcompany_domain = ptr + 1;
              }
              
              return true;
            }

            continue;
          }

          break;
        }
      }

      if(ptr != domain)
      {
        if(pcompany_domain)
        {
          *pcompany_domain = ptr + 1;
        }
        
        return true;
      }

      if(root_domain == 0)
      {
        return false;
      }

      std::string second_level_domain;
      second_level_domain.assign(domain, root_domain - domain);

      if(strcasecmp(second_level_domain.c_str(), "co") &&
         strcasecmp(second_level_domain.c_str(), "com"))
      {
        if(pcompany_domain)
        {
          *pcompany_domain = domain;
        }
              
        return true;
      }     
      
      return false;
    }
  }
}

#endif // _ELEMENTS_EL_NET_URL_HPP_
