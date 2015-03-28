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
 * @file Elements/El/PSP/Logger.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_LOGGER_HPP_
#define _ELEMENTS_EL_PSP_LOGGER_HPP_

#include <El/Exception.hpp>
#include <El/Logging/Logger.hpp>

#include <El/Python/RefCount.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/Logger.hpp>

namespace El
{
  namespace PSP
  {
    class Logger : public El::Logging::Python::Logger
    {
    public:      
      Logger(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception);      
        
      virtual ~Logger() throw() {}

      class Type : public El::Python::ObjectTypeImpl<Logger, Logger::Type>
      {
      public:
        Type() throw(El::Python::Exception, El::Exception);
        
        static Type instance;
      };
    };
    
    typedef El::Python::SmartPtr<Logger> Logger_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace PSP
  {
    //
    // El::PSP::Logger::Type class
    //
    inline
    Logger::Type::Type() throw(El::Python::Exception, El::Exception)
        : El::Python::ObjectTypeImpl<Logger, Logger::Type>(
          "el.psp.Logger", "PSP Logger class", "el.logging.Logger")
    {
    }
  }
}

#endif // _ELEMENTS_EL_PSP_LOGGER_HPP_
