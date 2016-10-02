/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/Tools/MySQLClassGen/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TOOLS_MYSQLCLASSGEN_APPLICATION_HPP_
#define _ELEMENTS_TOOLS_MYSQLCLASSGEN_APPLICATION_HPP_

#include <string>
#include <list>

#include <El/Exception.hpp>
#include <El/MySQL/DB.hpp>

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

  int gen(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  static
  void write_accessor_decl(std::ostream& ostr,
                           const MYSQL_FIELD& field,
                           std::string& ident)
    throw(InvalidArg, Exception, El::Exception);
  
  static
  void write_accessor_def(std::ostream& ostr,
                          const MYSQL_FIELD& field,
                          const char* class_name,
                          unsigned long index,
                          std::string& ident)
    throw(InvalidArg, Exception, El::Exception);
  
  static
  void write_field_check(std::ostream& ostr,
                         const MYSQL_FIELD& field,
                         const char* class_name,
                         bool ignore_not_null_flag,
                         bool ignore_binary_flag,
                         unsigned long index,
                         std::string& ident)
    throw(InvalidArg, Exception, El::Exception);
  
  static
  void write_common_check_code(std::ostream& ostr,
                               const char* ident,
                               const char* class_name,
                               const char* field_name,
                               unsigned long index)
    throw(El::Exception);

  struct ClassInfo
  {
    std::string name;
    std::string query;
    bool ignore_not_null_flag;
    bool ignore_binary_flag;
  };

  typedef std::list<ClassInfo> ClassInfoList;
  
private:
  El::MySQL::Connection_var connection_;
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

#endif // _ELEMENTS_TOOLS_MYSQLCLASSGEN_APPLICATION_HPP_
