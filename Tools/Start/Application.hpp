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
 * @file   Elements/Tools/Start/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TOOLS_START_APPLICATION_HPP_
#define _ELEMENTS_TOOLS_START_APPLICATION_HPP_

#include <string>
#include <vector>
#include <memory>
#include <set>

#include <ext/hash_map>
#include <ext/hash_set>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ThreadPool.hpp>

#include <xsd/Start.hpp>

class Application : public El::Service::Callback
{
public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    
public:
    
  Application() throw(Exception, El::Exception);
  virtual ~Application() throw();

  int run(int& argc, char** argv) throw(InvalidArg, Exception, El::Exception);

  typedef __gnu_cxx::hash_set<std::string, El::Hash::String>
  StringSet;

private:

  struct Argument
  {
    std::string name;
    std::string value;

    Argument(const char* nm = 0, const char* vl = 0)
      throw(El::Exception);
  };

  enum Operation
  {
    OP_START,
    OP_STOP,
    OP_STATUS,
    OP_LIST
  };
  
  typedef std::vector<Argument> ArgList;

  struct Service;
  
  struct HostInfo
  {
    struct Description
    {
      std::string host_service;
      std::string commands;
      std::string tags;
    };

    std::string name;
    bool execute_service_command;
    Description desc;
    
    HostInfo(const char* nm) throw(El::Exception);

    void set_execution(const Service& srv) throw(El::Exception);
  };

  typedef std::map<std::string, HostInfo> HostInfoMap;

  struct Service
  {
    enum Flags
    {
      SF_START = 0x1,
      SF_STOP = 0x2,
      SF_STATUS = 0x4
    };
    
    std::string name;
    std::string transport;
    std::string command;
    std::string status;
    bool status_check_delay;
    unsigned long flags;
    
    StringSet tag;
    StringSet after;
    StringSet before;
    HostInfoMap hosts;

    Service() throw(El::Exception);
  };

  typedef __gnu_cxx::hash_map<std::string, Service, El::Hash::String>
  ServiceTable;  

  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int execute(ArgList& arguments, Operation operation)
    throw(InvalidArg, Exception, El::Exception);

  void parse_options(ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);
  
  void parse_cluster_definition(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  void validate_cluster_definition() throw(Exception, El::Exception);
  
  void set_target(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);
  
  void set_execute_flags(Service& service, bool prerequisites_only)
    throw(InvalidArg, Exception, El::Exception);
  
  void execute_on_layer() throw(Exception, El::Exception);

  bool execute_service_operation(const char* service,
                                 const char* host)
    throw(Exception, El::Exception);
  
  int execute_command(const char* command,
                      bool describe_error = true)
    throw(Exception, El::Exception);
  
  std::string prepare_command(const char* service,
                              const char* host,
                              const char* transport,
                              const char* command_template)
    throw(Exception, El::Exception);

  bool reversed() const throw();
  
  //
  // Overrides
  //
  virtual bool notify(El::Service::Event* event) throw(El::Exception);
  
private:
  
  typedef std::auto_ptr<class Start::ClusterType> ClusterPtr;
  ClusterPtr cluster_;

  typedef __gnu_cxx::hash_map<std::string, std::string, El::Hash::String>
  TransportTable;

  struct ExecuteOnHost : public El::Service::ThreadPool::ServiceEvent
  {
    std::string service;
    std::string host;
      
    ExecuteOnHost(const char* service_val,
                  const char* host_val,
                  Callback* callback) throw(El::Exception);
    
    virtual ~ExecuteOnHost() throw();
  };

  typedef ACE_RW_Thread_Mutex    Mutex;
  typedef ACE_Read_Guard<Mutex>  ReadGuard;
  typedef ACE_Write_Guard<Mutex> WriteGuard;
  
  mutable Mutex lock_;

  El::String::Template::Parser parser_;
  El::String::Template::VariablesMap variables_;
  TransportTable transports_;
  ServiceTable services_;
  StringSet disabled_services_;

  Operation operation_;
  El::Service::ThreadPool_var threads_;
  unsigned long layer_counter_;
  Start::VerboseType verbose_;
  bool list_reverse_;
  bool nodeps_;
  unsigned long max_host_service_len_;
  unsigned long max_commands_len_;
  unsigned long max_tags_len_;
};

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

//
// Application::Service class
//

inline
Application::Service::Service() throw(El::Exception)
    : status_check_delay(false),
      flags(0)
{
}

//
// Application::HostInfo class
//

inline
Application::HostInfo::HostInfo(const char* nm) throw(El::Exception)
    : name(nm),
      execute_service_command(false)
{
}

//
// Application::ExecuteOnHost class
//

inline
Application::ExecuteOnHost::ExecuteOnHost(const char* service_val,
                                          const char* host_val,
                                          Callback* callback)
  throw(El::Exception)
    : El__Service__ThreadPool__ServiceEventBase(callback, 0, false),
      service(service_val),
      host(host_val)
{
}

inline
Application::ExecuteOnHost::~ExecuteOnHost() throw()
{
}

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

//
// Application class
//

inline
bool
Application::reversed() const throw()
{
  return operation_ == OP_STOP || list_reverse_;
}

#endif // _ELEMENTS_TOOLS_START_APPLICATION_HPP_
