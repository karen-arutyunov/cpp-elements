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
 * @file Elements/El/String/ListParser.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string.h>

#include <string>
#include <istream>
#include <sstream>
//#include <iomanip>

#include "ListParser.hpp"

namespace El
{
  namespace String
  {
    //
    // ListParser class
    //
    ListParser::ListParser(std::istream& stream, const char* separators)
      throw(El::Exception)
        : stream_source_(&stream),
          string_source_(0),
          separators_(separators ? separators : "")
    {
    }
    
    ListParser::ListParser(const char* string, const char* separators)
      throw(El::Exception)
        : stream_source_(0),
          string_source_(string ? string : ""),
          separators_(separators ? separators : "")
    {
    }

    const char*
    ListParser::next_item() throw(El::Exception)
    {
      item_.erase();

      if(stream_source_ != 0)
      {
        next_stream_item();
      }
      else
      {
        next_string_item();
      }
      
      return item_.empty() ? 0 : item_.c_str();
    }

    void
    ListParser::next_stream_item() throw(El::Exception)
    {
      char c = 0;
      while(stream_source_->get(c))
      {
        if(separators_.find(c) == std::string::npos)
        {
          stream_source_->putback(c);
          break;
        }
      }

      while(stream_source_->get(c))
      {
        if(separators_.find(c) != std::string::npos)
        {
          stream_source_->putback(c);
          break;
        }
          
        item_.append(&c, 1);
      }
    }
    
    void
    ListParser::next_string_item() throw(El::Exception)
    {
      string_source_ += strspn(string_source_, separators_.c_str());

      size_t end = strcspn(string_source_, separators_.c_str());
      
      item_.assign(string_source_, end);
      string_source_ += end;
    }

    //
    // WListParser class
    //
    WListParser::WListParser(std::wistream& stream, const wchar_t* separators)
      throw(El::Exception)
        : stream_source_(&stream),
          string_pos_(0),
          string_src_(string_pos_),
          separators_(separators ? separators : L""),
          item_offset_(0)
    {
    }
    
    WListParser::WListParser(const wchar_t* string, const wchar_t* separators)
      throw(El::Exception)
        : stream_source_(0),
          string_pos_(string ? string : L""),
          string_src_(string_pos_),
          separators_(separators ? separators : L""),
          item_offset_(0)
    {
    }

    const wchar_t*
    WListParser::next_item() throw(El::Exception)
    {
      item_.erase();

      if(stream_source_)
      {
        next_stream_item();
      }
      else
      {
        next_string_item();
      }
      
      return item_.empty() ? 0 : item_.c_str();
    }

    void
    WListParser::next_stream_item() throw(El::Exception)
    {
      wchar_t c = 0;
      while(stream_source_->get(c))
      {
        if(separators_.find(c) == std::string::npos)
        {
          stream_source_->putback(c);
          break;
        }
      }

      item_offset_ = (size_t)stream_source_->tellg();

      while(stream_source_->get(c))
      {
        if(separators_.find(c) != std::string::npos)
        {
          stream_source_->putback(c);
          break;
        }
          
        item_.append(&c, 1);
      }
    }
    
    void
    WListParser::next_string_item() throw(El::Exception)
    {
      string_pos_ += wcsspn(string_pos_, separators_.c_str());
      size_t end = wcscspn(string_pos_, separators_.c_str());
      
      item_.assign(string_pos_, end);
      item_offset_ = string_pos_ - string_src_;
      string_pos_ += end;
    }
  }
}
