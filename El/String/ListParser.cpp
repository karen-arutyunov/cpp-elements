/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
