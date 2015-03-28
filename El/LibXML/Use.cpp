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
