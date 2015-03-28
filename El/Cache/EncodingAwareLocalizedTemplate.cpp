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
 * @file Elements/El/Cache/EncodingAwareLocalizedTemplate.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <iostream>
#include <sstream>

#include <El/Exception.hpp>
#include <El/String/Template.hpp>
#include <El/String/Manip.hpp>

#include "EncodingAwareLocalizedTemplate.hpp"

namespace El
{
  namespace Cache
  {
    //
    // EncodableVariables class
    //
    void
    EncodableVariables::encode(const char* text,
                               std::ostream& output,
                               ValueEncoding encoding)
      throw(El::Exception)
    {
      if(text)
      {
        switch(encoding)
        {
        case VE_NONE: output << text; break;
        case VE_MIME_URL:
          {
            El::String::Manip::mime_url_encode(text, output);
            break;
          }
        case VE_XML:
          {
            El::String::Manip::xml_encode(
              text,
              output,
              El::String::Manip::XE_TEXT_ENCODING |
              El::String::Manip::XE_ATTRIBUTE_ENCODING |
              El::String::Manip::XE_PRESERVE_UTF8);
            
            break;
          }
        case VE_XML_STRICT:
          {
            El::String::Manip::xml_encode(
              text,
              output,
              El::String::Manip::XE_TEXT_ENCODING |
              El::String::Manip::XE_ATTRIBUTE_ENCODING);
            
            break;
          }
        default:
          {
            std::ostringstream ostr;
            ostr << "El::Cache::EncodableVariables::encode: "
              "unexpected encoding " << encoding;
              
            throw El::String::Template::Exception(ostr.str());
          }
          
        }
      }
    }

    //
    // LocalizationVariables class
    //
    bool
    LocalizationVariables::write(const El::String::Template::Chunk& chunk,
                                 std::ostream& output) const
      throw(El::Exception)
    {
      const char* var_name = chunk.text.c_str();

      if(strcmp(var_name, "encoding") == 0)
      {
        // Can make this trick as this object is used just once in
        // El::Cache::LocalizedTemplateFileCache::get method
        const_cast<LocalizationVariables*>(this)->default_encoding_ =
          reinterpret_cast<unsigned long>(chunk.tag);

        return true;
      }

      El::String::Template::Chunk new_chunk = chunk;
        
      if(reinterpret_cast<unsigned long>(new_chunk.tag) == VE_DEFAULT)
      {
        new_chunk.tag = reinterpret_cast<void*>(default_encoding_);
      }
      
      if(strncmp(var_name, "loc ", 4) == 0)
      {
        try
        {
          return encode(localization_.get(var_name + 4).c_str(),
                        output,
                        new_chunk);
        }
        catch(const El::Cache::VariablesMap::VariableNotFound& )
        {
        }
      }
      else
      {
        unsigned long encoding =
          reinterpret_cast<unsigned long>(new_chunk.tag);
        
        std::string encoding_str;
          
        switch(encoding)
        {
        case VE_MIME_URL: encoding_str = "mime-url"; break;
        case VE_XML: encoding_str = "xml"; break;
        case VE_XML_STRICT: encoding_str = "xml-strict"; break;
        case VE_NONE: encoding_str = "none"; break;
        default:
          {
            std::ostringstream ostr;
            ostr << "LocalizationVariables::write: "
              "not valid encoding " << encoding;

            throw El::String::Template::Exception(ostr.str());
          }
        } 
            
        const std::string& text = new_chunk.text;
        std::string::size_type pos = text.find(' ');
            
        output << var_left_marker_ << text.substr(0, pos) << ":"
               << encoding_str;

        if(pos != std::string::npos)
        {
          output << text.substr(pos);
        }

        output << var_right_marker_;
      }
      
      return true;
    }
    
    //
    // LocalizationInstantiationInterceptor class
    //
    El::String::Template::Variables*
    LocalizationInstantiationInterceptor::variables(
      const El::Cache::VariablesMap& localization)
      throw(El::Exception)
    {
      return new LocalizationVariables(var_left_marker_.c_str(),
                                       var_right_marker_.c_str(),
                                       localization);
    }

    //
    // EncodingAwareTemplateParseInterceptor class
    //

    char EncodingAwareTemplateParseInterceptor::separators_[] = " \t\r\n";
    
    void
    EncodingAwareTemplateParseInterceptor::update(
      El::String::Template::Chunk& chunk) const
      throw(El::String::Template::ParsingFailed,
            El::String::Template::Exception,
            El::Exception)
    {
      if(!chunk.is_var)
      {
        return;
      }

      const char* txt = chunk.text.c_str();
      const char* begin = txt + strspn(txt, separators_);
      const char* end = begin + strcspn(begin, separators_);

      std::string tag(begin, end - begin);
      
      std::string value;
      El::String::Manip::trim(end, value);

      Cache::EncodableVariables::ValueEncoding encoding =
        Cache::EncodableVariables::VE_DEFAULT;

      std::string::size_type pos = tag.find(':');

      if(pos != std::string::npos)
      {
        const char* enc = tag.c_str() + pos + 1;

        if(strcasecmp(enc, "mime-url") == 0)
        {
          encoding = Cache::EncodableVariables::VE_MIME_URL;
        }
        else if(strcasecmp(enc, "xml") == 0)
        {
          encoding = Cache::EncodableVariables::VE_XML;
        }
        else if(strcasecmp(enc, "xml-strict") == 0)
        {
          encoding = Cache::EncodableVariables::VE_XML_STRICT;
        }
        else if(strcasecmp(enc, "none") == 0)
        {
          encoding = Cache::EncodableVariables::VE_NONE;
        }
        else
        {
          std::ostringstream ostr;
          ostr << "El::Cache::EncodingAwareTemplateParseInterceptor::update: "
            "not valid encoding '" << enc << "'";

          throw El::String::Template::ParsingFailed(ostr.str());
        }

        tag.resize(pos);
        chunk.tag = reinterpret_cast<void*>(encoding);
      }
      
      if(tag == "loc")
      {
        chunk.text = std::string("loc ") + value;
      }
      else if(tag.empty())
      {
        chunk.text.clear();
      }
      else if(tag == "encoding")
      {
        if(encoding == Cache::EncodableVariables::VE_DEFAULT)
        {
          throw El::String::Template::ParsingFailed(
            "El::Cache::EncodingAwareTemplateParseInterceptor::update: "
            "unexpected encoding for 'encoding' tag");
        }
        
        chunk.text = tag;
      }
      else
      {
        update(tag, value, chunk);
      }
    }
  }
}
