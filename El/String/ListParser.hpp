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
 * @file Elements/El/String/ListParser.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STRING_LISTPARSER_HPP_
#define _ELEMENTS_EL_STRING_LISTPARSER_HPP_

#include <string>
#include <istream>

#include <El/Exception.hpp>

namespace El
{
  namespace String
  {
    class ListParser
    {
    public:
      ListParser(std::istream& stream, const char* separators = " \t\r\n")
        throw(El::Exception);

      ListParser(const char* string, const char* separators = " \t\r\n")
        throw(El::Exception);

      const char* next_item() throw(El::Exception);

    protected:
      void next_stream_item() throw(El::Exception);
      void next_string_item() throw(El::Exception);

    protected:
      std::istream* stream_source_;
      const char*   string_source_;
      std::string   separators_;
      std::string   item_;

    private:
      ListParser(const ListParser&);
      void operator=(const ListParser&);
    };

    class WListParser
    {
    public:
      WListParser(std::wistream& stream,
                  const wchar_t* separators = L" \t\r\n")
        throw(El::Exception);

      WListParser(const wchar_t* string,
                  const wchar_t* separators = L" \t\r\n")
        throw(El::Exception);

      const wchar_t* next_item() throw(El::Exception);
      
      size_t item_offset() const throw() { return item_offset_; }

    protected:
      void next_stream_item() throw(El::Exception);
      void next_string_item() throw(El::Exception);

    protected:
      std::wistream* stream_source_;
      const wchar_t* string_pos_;
      const wchar_t* string_src_;
      std::wstring   separators_;
      std::wstring   item_;
      size_t item_offset_;

    private:
      WListParser(const WListParser&);
      void operator=(const WListParser&);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace String
  {
  }
}


#endif // _ELEMENTS_EL_STRING_LISTPARSER_HPP_
