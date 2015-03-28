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
 * @file Elements/El/LibXML/HTMLParser.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LIBXML_HTMLPARSER_HPP_
#define _ELEMENTS_EL_LIBXML_HTMLPARSER_HPP_

#include <istream>
#include <iostream>
#include <memory>

#include <libxml/parser.h>
#include <libxml/HTMLtree.h>
#include <libxml/HTMLparser.h>

#include <El/LibXML/Use.hpp>

namespace El
{
  namespace LibXML
  {
    class HTMLParser
    {
    public :
      HTMLParser() throw() : doc_(0) {}
      ~HTMLParser() throw() { if(doc_) xmlFreeDoc(doc_); }
        
      htmlDocPtr document(bool retain = true) throw();

      htmlDocPtr parse(const char* text,
                       unsigned long text_len,
                       const char* url = 0,
                       const char* encoding = 0,
                       ErrorHandler* error_handler = 0,
                       int options = 0)
        throw(Exception, El::Exception);

      htmlDocPtr parse_file(const char* file_path,
                            const char* encoding = 0,
                            ErrorHandler* error_handler = 0,
                            int options = 0)
        throw(Exception, El::Exception);

    private:

      htmlDocPtr doc_;
      
    private:
      HTMLParser(const HTMLParser&);
      void operator=(const HTMLParser&);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace LibXML
  {
    inline
    htmlDocPtr
    HTMLParser::document(bool retain) throw()
    {
      htmlDocPtr doc = doc_;
      
      if(!retain)
      {
        doc_ = 0;
      }
         
      return doc;
    }
  }
}

#endif // _ELEMENTS_EL_LIBXML_HTMLPARSER_HPP_
