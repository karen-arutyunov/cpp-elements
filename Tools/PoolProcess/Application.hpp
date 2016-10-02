/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/Tools/PoolProcess/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TOOLS_POOLPROCESS_APPLICATION_HPP_
#define _ELEMENTS_TOOLS_POOLPROCESS_APPLICATION_HPP_

#include <string>
#include <vector>

#include <El/Exception.hpp>
#include <El/Service/ProcessPool.hpp>

class Application
{
public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    
public:
    
  Application() throw(Exception, El::Exception);
  virtual ~Application() throw();

  int run(int& argc, char** argv) throw(InvalidArg, Exception, El::Exception);

//  typedef __gnu_cxx::hash_set<std::string, El::Hash::String>
//  StringSet;

private:

  struct Argument
  {
    std::string name;
    std::string value;

    Argument(const char* nm = 0, const char* vl = 0)
      throw(El::Exception);
  };

  typedef std::vector<Argument> ArgList;

  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int execute(ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  std::string init(El::BinaryInStream& input, El::BinaryOutStream& output)
    throw(Exception, El::Exception);
  
  void release_lib() throw();
  
  std::string execute_task(El::BinaryInStream& input,
                           El::BinaryOutStream& output) const
    throw(Exception, El::Exception);
  
  static bool read_data(uint32_t& signature, std::string& data)
    throw(Exception, El::Exception);
  
  static bool write_data(uint32_t signature, const std::string& data)
    throw(Exception, El::Exception);
  
private:
  
  void* lib_handle_;
  ::El::Service::ProcessPool::TaskFactoryInterface* task_factory_;

  typedef std::vector<void*> LibHandleArray;
  LibHandleArray extra_libs_;
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

#endif // _ELEMENTS_TOOLS_POOLPROCESS_APPLICATION_HPP_
