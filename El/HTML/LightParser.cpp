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
 * @file Elements/El/HTML/LightParser.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <wchar.h>

#include <sstream>
#include <iostream>

#include <El/String/Manip.hpp>

#include "LightParser.hpp"

namespace El
{
  namespace HTML
  {
    void
    LightParser::parse(const char* html_text,
                       const char* charset,
                       const char* document_url,
                       unsigned long flags,
                       size_t max_text_len,
                       size_t max_char_len)
      throw(Exception, El::Exception)
    {
      std::string encoding;
      El::String::Manip::to_lower(charset, encoding);
      
      if(encoding.empty())
      {
        encoding = "iso-8859-1"; 
      }

      for(size_t i = 0; i < 3; i++)
      {
        std::wstring wtext;

        try
        {
          if(encoding == "iso-8859-1")
          {
            El::String::Manip::latin1_to_wchar(html_text, wtext);
          }
          else if(encoding == "utf-8")
          {
            El::String::Manip::utf8_to_wchar(html_text, wtext, true);
          }
          else if(encoding == "windows-1251")
          {
            El::String::Manip::win1251_to_wchar(html_text, wtext, true);
          }
          else
          {
            try
            {
              std::auto_ptr<El::String::Manip::Transcoder> transcoder;

              try
              {
                transcoder.reset(
                  new El::String::Manip::Transcoder(encoding.c_str()));
              }
              catch(...)
              {
                El::String::Manip::replace(encoding, "_", '-');
                  
                try
                {
                  transcoder.reset(
                    new El::String::Manip::Transcoder(encoding.c_str()));
                }
                catch(...)
                {
                }

                if(transcoder.get() == 0)
                {
                  throw;
                }
              } 
              
              transcoder->decode(html_text, wtext, flags & PF_LAX);
            }
            catch(const El::Exception& e)
            {
              std::ostringstream ostr;
              ostr << "El::HTML::LightParser::parse: decoding from "
                   << encoding << " failed. Reason:\n" << e;
            
              throw Exception(ostr.str());
            }
          }
        }
        catch(const El::String::Manip::InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::HTML::LightParser::parse: can not parse according "
            "to charset " << charset;
        
          throw Exception(ostr.str());
        }

        try
        {
          parse(wtext.c_str(),
                document_url,
                flags,
                max_text_len,
                max_char_len);
          
          return;
        }
        catch(const CharsetChange& e)
        {
          encoding = e.what();
        }
      }
    }
    
    void
    LightParser::parse(const wchar_t* html_text,
                       const char* document_url,
                       unsigned long flags,
                       size_t max_text_len,
                       size_t max_char_len,
                       const char* charset)
      throw(Exception, CharsetChange, El::Exception)
    {
      text.clear();
      images.clear();
      flags_ = flags;
      last_space_ = true;
      inside_comment_ = false;
      inside_pre_tag_ = false;
      max_text_len_ = max_text_len > 4 ? max_text_len - 4 : max_text_len;
      max_char_len_ = max_char_len > 4 ? max_char_len - 4 : max_char_len;
      length_ = 0;
      char_count_ = 0;

      charset_ = charset ? charset : "";
      current_image_ = Image();
      current_link_ = Link();
      current_frame_ = Frame();
      current_meta_ = Meta();
      current_xml_ = XML();

      url_base_ = document_url && *document_url != '\0' ?
        new El::Net::HTTP::URL(document_url) : 0;

      ParseState state = READING_TEXT;
      size_t line = 1;
      size_t pos = 1;
      wchar_t attr_quote = L'\0';
      const wchar_t* ptag = 0;
      const wchar_t* pattr = 0;
      const wchar_t* pattr_value = 0;
      
      std::wstring tag;
      std::wstring attr;

      try
      {
        for (const wchar_t* ptr = html_text; *ptr != L'\0'; ptr++, pos++)
        {
          if(inside_comment_)
          {
            switch(*ptr)
            {
            case L'-':
              {
                if(wcsncmp(ptr + 1, L"->", 2) == 0)
                {
                  inside_comment_ = false;
                  ptr += 2;
                  pos += 2;
                }

                break;
              }
            case L'\r':
              {
                if(ptr[1] == L'\n')
                {
                  ptr++;
                }
              
                line++;
                pos = 0;
                break;
              }
            case L'\n':
              {
                if(ptr[1] == L'\r')
                {
                  ptr++;
                }
              
                line++;
                pos = 0;
                break;
              }
            default:
              {
                break;
              }
            }

            continue;
          }
        
          switch(*ptr)
          {
          case L'<':
            {
              if(!inside_comment_ && wcsncmp(ptr + 1, L"!--", 3) == 0)
              {
                inside_comment_ = true;
                break;
              }

              switch(state)
              {
              case READING_TAG:
                {
                  // Unexpected <
                  
                  if(flags_ & PF_LAX)
                  {
                  // Just automatically close current tag.
                    
                    state = READING_TEXT;

                    if(ptag)
                    {
                      std::wstring tg(ptag, ptr - ptag);
                      ptag = 0;
                
                      El::String::Manip::to_lower(tg.c_str(), tag);
                    }

                    pattr = 0;
                    pattr_value = 0;

                    process_tag(tag.c_str());

                    ptr--;
                    pos--;
                  }
                  else
                  {
                    std::ostringstream ostr;
                    ostr << "Error line " << line << " pos " << pos
                         << " : unexpected '<' character";
                    
                    throw Exception(ostr.str().c_str());
                  }
                  
                
                  break;                
                }
              case READING_TEXT:
                {
                  wchar_t ch = towlower(ptr[1]);
                
                  if(ch == '/' || ch == '?' || ch == '!' ||
                     (ch >= L'a' && ch <= 'z'))
                  {
                    state = READING_TAG;
                    ptag = ptr + 1;
                  }
                  else
                  {
                    append(*ptr);
                  }
                
                  break;
                }
              case READING_ATTRIBUTE:
                {
                  break;
                }
              }
          
              break;
            }
          case L'\"':
          case L'\'':
            {
              switch(state)
              {
              case READING_TEXT:
                {
                  append(*ptr);
                  break;
                }
              case READING_ATTRIBUTE:
                {
                  if(attr_quote == *ptr)
                  {
                    std::wstring attr_val(pattr_value, ptr - pattr_value);

                    state = READING_TAG;
                    pattr_value = 0;
                
                    process_attr(tag.c_str(), attr.c_str(), attr_val.c_str());
                
                    attr.clear();
                  }
                
                  break;
                }
              case READING_TAG:
                {
                  state = READING_ATTRIBUTE;
                  attr_quote = *ptr;
                  pattr_value = ptr + 1;
              
                  break;
                }
              }
          
              break;
            }
          case L' ':
            {
              switch(state)
              {
              case READING_TAG:
                {
                  if(ptag)
                  {
                    if(ptr == ptag)
                    {
                      std::ostringstream ostr;
                      ostr << "Error line " << line << " pos " << pos
                           << " : unexpected ' ' character";
              
                      throw Exception(ostr.str().c_str());
                    }
                
                    std::wstring tg(ptag, ptr - ptag);
                    ptag = 0;

                    El::String::Manip::to_lower(tg.c_str(), tag);
                  }
                  else if(pattr)
                  {
                    std::wstring at(pattr, ptr - pattr);
                    pattr = 0;

                    El::String::Manip::to_lower(at.c_str(), attr);
                  }
              
                  break;
                }
              case READING_TEXT:
                {
                  append(*ptr);
                  break;
                }
              case READING_ATTRIBUTE:
                {
                  if(attr_quote == L'\0')
                  {
                    std::wstring attr_val(pattr_value, ptr - pattr_value);

                    state = READING_TAG;
                    pattr_value = 0;
                
                    process_attr(tag.c_str(), attr.c_str(), attr_val.c_str());
                    attr.clear();
                  }
                
                  break;
                } 
              default: break;
              }
          
              break;
            }
        
          case L'>':
            {
              switch(state)
              {
              case READING_TEXT:
                {
                  append(*ptr);
                  break;
                }
              case READING_ATTRIBUTE:
                {
                  if(attr_quote == L'\0')
                  {
                    std::wstring attr_val(pattr_value, ptr - pattr_value);

                    state = READING_TAG;
                    pattr_value = 0;
                
                    process_attr(tag.c_str(), attr.c_str(), attr_val.c_str());
                    attr.clear();

                    ptr--;
                    pos--;
                  }
                
                  break;
                }
              case READING_TAG:
                {
                  state = READING_TEXT;

                  if(ptag)
                  {
                    std::wstring tg(ptag, ptr - ptag);
                    ptag = 0;
                
                    El::String::Manip::to_lower(tg.c_str(), tag);
                  }

                  pattr = 0;
                  pattr_value = 0;

                  process_tag(tag.c_str());              
                  break;
                }
              }
          
              break;
            }

          case L'/':
            {
              switch(state)
              {
              case READING_TEXT:
                {
                  append(*ptr);
                  break;
                }
              case READING_ATTRIBUTE:
                {
                  if(attr_quote == L'\0' && ptr[1] == L'>')
                  {
                    std::wstring attr_val(pattr_value, ptr - pattr_value);

                    state = READING_TAG;
                    pattr_value = 0;
                
                    process_attr(tag.c_str(), attr.c_str(), attr_val.c_str());
                    attr.clear();

                    ptr--;
                    pos--;                  
                  }
                
                  break;
                }
              case READING_TAG:
                {
                  break;
                }
              }
          
              break;
            }
          
          case L'=':
            {
              switch(state)
              {
              case READING_TAG:
                {
                  if(pattr)
                  {
                    std::wstring at(pattr, ptr - pattr);
                    pattr = 0;

                    El::String::Manip::to_lower(at.c_str(), attr);
                  }

                  wchar_t chr = ptr[1];
                
                  if(chr != L'\'' && chr != L'\"' &&
                     !El::String::Unicode::CharTable::is_space(chr))
                  {
                    state = READING_ATTRIBUTE;
                    attr_quote = L'\0';
                    pattr_value = ptr + 1;
                  }
                
                  break;
                }
              case READING_TEXT:
                {
                  append(*ptr);
                  break;
                }
              case READING_ATTRIBUTE:
                {
                  break;
                }

                break;
              }

              break;
            }
          case L'\r':
            {
              if(ptr[1] == L'\n')
              {
                if(state == READING_TEXT)
                {
                  append(*ptr);
                }
                
                ptr++;
              }
              
              if(state == READING_TEXT)
              {
                append(*ptr);
              }
              
              line++;
              pos = 0;
              break;
            }
          case L'\n':
            {
              if(ptr[1] == L'\r')
              {
                if(state == READING_TEXT)
                {
                  append(*ptr);
                }
                
                ptr++;
              }
              
              if(state == READING_TEXT)
              {
                append(*ptr);
              }
              
              line++;
              pos = 0;
              break;
            }
          case L'&':
            {
              if(state == READING_TEXT)
              {
                wchar_t character = L'\0';

                try
                {
                  const wchar_t* ptr2 =
                    El::String::Manip::xml_decode_entity(ptr, character);

                  pos += ptr2 - ptr - 1;
                  ptr = ptr2 - 1;
                } 
                catch(const El::Exception& e)
                {
                  if(flags_ & PF_LAX)
                  {
                    append(*ptr);
                    continue;
                  }
                  else
                  {
                    std::ostringstream ostr;
                    ostr << "Error line " << line << " pos " << pos
                         << " : " << e;
              
                    throw Exception(ostr.str());
                  }

                }

                append(character);
              }
              
              break;
            }
          default:
            {
              switch(state)
              {
              case READING_TEXT:
                {
                  append(*ptr);
                  break;
                }
              case READING_TAG:
                {
                  if(pattr == 0 && ptag == 0)
                  {
                    pattr = ptr;
                  }
                    
                  break;
                }
              default:
                {
                  break;
                }
              }
              
              break;
            }
          }
        }
      }
      catch(const MaxLenReached& )
      {
      }
    
      if(state != READING_TEXT)
      {
        if(flags_ & PF_LAX)
        {
          // Just silently stop.
        }
        else
        {
          std::ostringstream ostr;
          ostr << "Error line " << line << " pos " << pos
               << " : end of " << (state == READING_TAG ? "tag" : "attribute")
               << " expected";
              
          throw Exception(ostr.str().c_str());
        }
      }

      char_count_ -= rtrim(true);

      if(length_ > max_text_len_ || char_count_ > max_char_len_)
      {
        rtrim(false);      
        rtrim(true);
        text += " ...";
      }
    }  
  }
}
