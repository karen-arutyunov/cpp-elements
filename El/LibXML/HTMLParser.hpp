/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
