/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
