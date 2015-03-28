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
 * @file Elements/El/Net/HTTP/Params.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_HTTP_PARAMS_HPP_
#define _ELEMENTS_EL_NET_HTTP_PARAMS_HPP_

#include <string>
#include <list>
#include <iostream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/String/ListParser.hpp>
#include <El/Net/HTTP/Exception.hpp>
#include <El/Lang.hpp>
#include <El/Country.hpp>

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      struct Param
      {
        std::string name;
        std::string value;

        Param() throw(El::Exception);
        
        Param(const char* name, const char* value)
          throw(InvalidArg, El::Exception);

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);
      };

      class ParamList : public std::list<Param>
      {
      public:

        ParamList() throw(El::Exception) {}
        ParamList(const char* params, bool lax = false) throw(El::Exception);
        
        void add(const char* name, const char* value)
          throw(InvalidArg, El::Exception);

        // In param is in "arg=value" form
        void add(const char* param) throw(InvalidArg, El::Exception);

        const char* find(const char* name) const throw(El::Exception);
      };

      struct Header
      {
        std::string name;
        std::string value;

        Header() throw(El::Exception);
        
        Header(const char* name, const char* value)
          throw(InvalidArg, El::Exception);

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);
      };

      class HeaderList : public std::list<Header>
      {
      public:

        void add(const char* name, const char* value)
          throw(InvalidArg, El::Exception);

        const char* find(const char* name) const throw(El::Exception);
      };

      struct AcceptLanguage
      {
        El::Lang language;
        El::Country country;
        std::string subtag;
        float qvalue;

        AcceptLanguage() : qvalue(1) {}
        
        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);
      };

      typedef std::list<AcceptLanguage> AcceptLanguageList;

      struct AcceptEncoding
      {
        std::string coding;
        float qvalue;

        AcceptEncoding() : qvalue(1) {}

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);
      };
      
      typedef std::list<AcceptEncoding> AcceptEncodingList;

      bool content_type(const char* header_value, std::string& charset)
        throw(El::Exception);
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
      //
      // Param struct
      //
      inline
      Param::Param() throw(El::Exception)
      {
      }
      
      inline
      Param::Param(const char* nm, const char* vl)
        throw(InvalidArg, El::Exception)
          : name(nm ? nm : ""),
            value(vl ? vl : "")
      {
        if(name.empty())
        {
          throw InvalidArg("El::Net::HTTP::Param::Param: empty name");
        }
      }
      
      inline
      void
      Param::write(El::BinaryOutStream& bstr) const throw(El::Exception)
      {
        bstr << name << value;
      }
      
      inline
      void
      Param::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> name >> value;
      }

      //
      // ParamList class
      //

      inline
      ParamList::ParamList(const char* params, bool lax) throw(El::Exception)
      {
        if(params == 0)
        {
          return;
        }

        El::String::ListParser parser(params, "&");
        const char* item = 0;
        
        while((item = parser.next_item()) != 0)
        {
          try
          {
            add(item);
          }
          catch(const InvalidArg&)
          {
            if(!lax)
            {
              throw;
            }
          }
        }
      }
      
      inline
      void
      ParamList::add(const char* nm, const char* vl)
        throw(InvalidArg, El::Exception)
      {
        if(nm == 0 || *nm == '\0')
        {
          throw InvalidArg("El::Net::HTTP::ParamList::add: empty name");
        }
          
        push_back(Param());

        reverse_iterator it = rbegin();
        it->name = nm;

        if(vl)
        {
          it->value = vl;
        }
      }

      inline
      void
      ParamList::add(const char* param)
        throw(InvalidArg, El::Exception)
      {
        if(param == 0 || *param == '\0')
        {
          throw InvalidArg(
            "El::Net::HTTP::ParamList::add: empty param string");
        }
          
        push_back(Param());

        reverse_iterator it = rbegin();
        const char* eq = strchr(param, '=');

        try
        {
          if(eq == 0)
          {
            El::String::Manip::mime_url_decode(param, it->name);
          }
          else
          {
            std::string name(param, eq - param);
            El::String::Manip::mime_url_decode(name.c_str(), it->name);
            El::String::Manip::mime_url_decode(eq + 1, it->value);
          }
        }
        catch(const El::String::Manip::InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::ParamList::add: "
            "El::String::Manip::InvalidArg caught. Description:\n" << e;
          
          throw InvalidArg(ostr.str());
        }
      }

      inline
      const char* 
      ParamList::find(const char* name) const throw(El::Exception)
      {
        if(name != 0)
        {
          for(const_iterator i(begin()), e(end()); i != e; ++i)
          {
            if(strcasecmp(i->name.c_str(), name) == 0)
            {
              return i->value.c_str();
            }
          }
        }

        return 0;
      }
      
      //
      // Header struct
      //
      inline
      Header::Header() throw(El::Exception)
      {
      }
      
      inline
      Header::Header(const char* nm, const char* vl)
        throw(InvalidArg, El::Exception)
          : name(nm ? nm : ""),
            value(vl ? vl : "")
      {
        if(name.empty())
        {
          throw InvalidArg("El::Net::HTTP::Header::Header: empty name");
        }
      }
      
      inline
      void
      Header::write(El::BinaryOutStream& bstr) const throw(El::Exception)
      {
        bstr << name << value;
      }
      
      inline
      void
      Header::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> name >> value;
      }
      
      //
      // HeaderList class
      //
      inline
      void
      HeaderList::add(const char* nm, const char* vl)
        throw(InvalidArg, El::Exception)
      {
        if(nm == 0 || *nm == '\0')
        {
          throw InvalidArg("El::Net::HTTP::HeaderList::add: empty name");
        }
          
        push_back(Header());

        reverse_iterator it = rbegin();
        it->name = nm;

        if(vl)
        {
          it->value = vl;
        }
      }

      inline
      const char*
      HeaderList::find(const char* name) const throw(El::Exception)
      {
        if(name != 0)
        {
          for(const_iterator i(begin()), e(end()); i != e; ++i)
          {
            if(strcasecmp(i->name.c_str(), name) == 0)
            {
              return i->value.c_str();
            }
          }
        }

        return 0;
      }

      //
      // Class AcceptLanguage
      //
      inline
      void
      AcceptLanguage::write(El::BinaryOutStream& bstr) const
        throw(El::Exception)
      {
        bstr << language << country << subtag << qvalue;
      }
      
      inline
      void
      AcceptLanguage::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> language >> country >> subtag >> qvalue;
      }
      
      //
      // Class AcceptEncoding
      //
      inline
      void
      AcceptEncoding::write(El::BinaryOutStream& bstr) const
        throw(El::Exception)
      {
        bstr << coding << qvalue;
      }
      
      inline
      void
      AcceptEncoding::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> coding >> qvalue;
      }

      inline
      bool
      content_type(const char* header_value, std::string& charset)
        throw(El::Exception)
      {
        const char* ptr = strcasestr(header_value, "charset");

        if(ptr == 0 || (ptr = strchr(ptr + 7, '=')) == 0)
        {
          return false;
        }
        
        const char* end = strchr(++ptr, ';');
        
        if(end == 0)
        {
          El::String::Manip::trim(ptr, charset);
        }
        else
        {
          std::string tmp(ptr, end - ptr);
          El::String::Manip::trim(tmp.c_str(), charset);
        }

        if(!strncasecmp(charset.c_str(), "win-", 4))
        {
          charset = std::string("windows-") + (charset.c_str() + 4);
        }

        return !charset.empty();
      }
    }
  }
}

inline
std::ostream& operator<<(std::ostream& ostr, const El::Net::HTTP::Param& param)
  throw(El::Exception)
{
  El::String::Manip::mime_url_encode(param.name.c_str(), ostr);
  ostr << "=";
  El::String::Manip::mime_url_encode(param.value.c_str(), ostr);
  return ostr;
}

inline
std::ostream& operator<<(std::ostream& ostr,
                         const El::Net::HTTP::Header& header)
  throw(El::Exception)
{
  ostr << header.name << ": " << header.value << "\r\n";
  return ostr;
}

#endif // _ELEMENTS_EL_NET_HTTP_PARAMS_HPP_
