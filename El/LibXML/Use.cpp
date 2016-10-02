/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/LibXML/Use.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <assert.h>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include "Use.hpp"

namespace El
{
  namespace LibXML
  {
    unsigned long Use::counter_ = 0;
    Use::Mutex Use::lock_;

    //
    // Use class
    //
    void XMLCALL
    Use::error_handler(void *userData, xmlErrorPtr error)
    {
      ErrorHandler* handler = (ErrorHandler*)userData;
      assert(handler != 0);
      handler->handle(error);
    }

    //
    // ErrorRecorderHandler class
    //
    void
    ErrorRecorderHandler::handle(xmlErrorPtr error) throw()
    {
      std::ostringstream* stream = 0;
      
      switch(error->level)
      {
      case XML_ERR_WARNING:
        {
          warnings << "Warning";
          stream = &warnings;
          break;
        }
      case XML_ERR_ERROR:
        {
          errors << "Error";
          stream = &errors;
          break;
        }
      case XML_ERR_FATAL:
        {
          fatal_errors << "Fatal Error";
          stream = &fatal_errors;
          break;
        }
      default: return;
      }

      *stream << " [" << error->line << ":" << error->int2
              << ", " << error->domain << ":" << error->code << "]: "
              << error->message << std::endl;
    }
    
    void
    ErrorRecorderHandler::dump(ostream& ostr) throw(El::Exception)
    {
      std::string text = fatal_errors.str();

      if(!text.empty())
      {
        ostr << text;
      }
      
      text = errors.str();

      if(!text.empty())
      {
        ostr << text;
      }
      
      text = warnings.str();

      if(!text.empty())
      {
        ostr << text;
      }
    }
    
  }
}
