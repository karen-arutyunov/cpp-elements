/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/LibXML/HTMLParser.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>

#include <El/Exception.hpp>

#include "HTMLParser.hpp"

namespace El
{
  namespace LibXML
  {
    htmlDocPtr
    HTMLParser::parse(const char* text,
                      unsigned long text_len,
                      const char* url,
                      const char* encoding,
                      ErrorHandler* error_handler,
                      int options)
      throw(Exception, El::Exception)
    {
      if(text == 0 || *text == '\0' || text_len == 0)
      {
        // Otherwise htmlReadMemory stupidly fails to allocate 0 bytes
        text = " ";
        text_len = 1;
      }
        
      if(doc_)
      {
        xmlFreeDoc(doc_);
        doc_ = 0;
      }

      std::auto_ptr<ErrorRecorderHandler> def_handler;
      
      if(error_handler == 0)
      {
        def_handler.reset(new ErrorRecorderHandler());
        error_handler = def_handler.get();
      }
        
      Use::set_error_handler(error_handler);
      
      doc_ = htmlReadMemory(text,
                            text_len,
                            url,
                            encoding && *encoding ? encoding : 0,
                            options);
        
      Use::set_error_handler(0);
      
      if(doc_ == 0)
      {
        std::ostringstream ostr;
        
        ostr << "El::LibXML::HTMLParser::parse: "
          "htmlReadMemory failed";

        if(url && *url != '\0')
        {
          ostr << " for url '" << url << "'";
        }

        if(encoding && *encoding != '\0')
        {
          ostr << ", encoding '" << encoding << "'";
        }

        ErrorRecorderHandler* handler =
          dynamic_cast<ErrorRecorderHandler*>(error_handler);

        if(handler != 0)
        {
          std::string fatal_errors = handler->fatal_errors.str();

          if(!fatal_errors.empty())
          {
            ostr << "; fatal errors:\n" << fatal_errors;
          }
        }
        
        throw Exception(ostr.str());      
      }

      return doc_;
    }

    htmlDocPtr
    HTMLParser::parse_file(const char* file_path,
                           const char* encoding,
                           ErrorHandler* error_handler,
                           int options)
      throw(Exception, El::Exception)
    {
      if(doc_)
      {
        xmlFreeDoc(doc_);
        doc_ = 0;
      }

      std::auto_ptr<ErrorRecorderHandler> def_handler;
      
      if(error_handler == 0)
      {
        def_handler.reset(new ErrorRecorderHandler());
        error_handler = def_handler.get();
      }
        
      Use::set_error_handler(error_handler);
      
      doc_ = htmlReadFile(file_path,
                          encoding && *encoding ? encoding : 0,
                          options);
      
      Use::set_error_handler(0);
      
      if(doc_ == 0)
      {
        std::ostringstream ostr;
        
        ostr << "El::LibXML::HTMLParser::parse_file: "
          "htmlReadFile failed for '" << file_path << "', encoding '"
             << (encoding ? encoding : "<null>") << "'";

        ErrorRecorderHandler* handler =
          dynamic_cast<ErrorRecorderHandler*>(error_handler);

        if(handler != 0)
        {
          std::string fatal_errors = handler->fatal_errors.str();

          if(!fatal_errors.empty())
          {
            ostr << "; fatal errors:\n" << fatal_errors;
          }
        }
        
        throw Exception(ostr.str());      
      }

      return doc_;
    }
  }
}
