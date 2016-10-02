/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/Tools/Copyrighter/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TOOLS_COPYRIGHTER_APPLICATION_HPP_
#define _ELEMENTS_TOOLS_COPYRIGHTER_APPLICATION_HPP_

#include <string>
#include <list>

#include <ace/OS.h>

#include <El/Exception.hpp>

class Application
{
public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    
public:
    
  Application() throw(Exception, El::Exception);
  virtual ~Application() throw();

  int run(int& argc, char** argv) throw(InvalidArg, Exception, El::Exception);

private:

  struct Argument
  {
    std::string name;
    std::string value;

    Argument(const char* nm = 0, const char* vl = 0)
      throw(El::Exception);
  };

  typedef std::list<Argument> ArgList;

  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int update(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);
};

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

//
// Application::Argument class
//

inline
Application::Argument::Argument(const char* nm, const char* vl)
  throw(El::Exception)
    : name(nm ? nm : ""),
      value(vl ? vl : "")
{
}

#endif // _ELEMENTS_TOOLS_COPYRIGHTER_APPLICATION_HPP_
