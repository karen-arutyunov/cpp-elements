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
 * @file   Elements/Tools/PoolProcess/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <limits.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <sstream>
#include <utility>

#include <ace/OS.h>

#include <El/BinaryStream.hpp>
#include <El/String/Manip.hpp>
#include <El/String/ListParser.hpp>
#include <El/IO.hpp>

//#include <tests/ProcessPool/Task.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
    "Usage:\nElPoolProcess <command> <command arguments>\n\n"
    "Synopsis 1:\nElPoolProcess help\n\n"
    "Synopsis 2:\nElPoolProcess exec <service name> <in fileno> <out fileno>\n";

  int IN_FILENO = STDIN_FILENO;
  int OUT_FILENO = STDOUT_FILENO;
}

int
main(int argc, char** argv)
{
  /*
  rlimit limit;
  memset(&limit, 0, sizeof(limit));
  getrlimit(RLIMIT_NOFILE, &limit);

  int max_fd = limit.rlim_cur;              

  for(int fd = 0; fd < max_fd; fd++)
  {
    switch(fd)
    {
    case STDIN_FILENO:
    case STDOUT_FILENO:
    case STDERR_FILENO: break;
    default:
      {
        if(fd != IN_FILENO && fd != OUT_FILENO)
        {
          close(fd);
        }
      }
    }
  }
  */

  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'ElPoolProcess help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << e << std::endl;
  }
  catch(...)
  {
    std::cerr << "ElPoolProcess: unknown exception caught\n";
  }
  
  return -1;
}

//
// Application class
//
Application::Application() throw(Application::Exception, El::Exception)
    : lib_handle_(0),
      task_factory_(0)
{
}

Application::~Application() throw()
{
  release_lib();
}

void
Application::release_lib() throw()
{
  if(lib_handle_)
  {
    if(task_factory_)
    {
      task_factory_->release();
      task_factory_ = 0;
    }

// TODO: uncomment    
    dlclose(lib_handle_);
    lib_handle_ = 0;
  }

  for(LibHandleArray::reverse_iterator i(extra_libs_.rbegin()),
        e(extra_libs_.rend()); i != e;  ++i)
  {
// TODO: uncomment    
    dlclose(*i);
  }

  extra_libs_.clear();
}

int
Application::run(int& argc, char** argv)
  throw(InvalidArg, Exception, El::Exception)
{
  if(argc < 2)
  {
    throw InvalidArg("Too few arguments");
  }

  int i = 1;  
  std::string command = argv[i];

  ArgList arguments;

  for(i++; i < argc; i++)
  {
    char* argument = argv[i];
    
    Argument arg;
    const char* eq = strstr(argument, "=");

    if(eq == 0)
    {
      arg.name = argument;
    }
    else
    {
      arg.name.assign(argument, eq - argument);
      arg.value = eq + 1;
    }

    arguments.push_back(arg);
  }

  int res = 0;
  
  if(command == "help")
  {
    res = help(arguments);
  }
  else if(command == "exec")
  {
    res = execute(arguments);
  }
  else
  {
    std::ostringstream ostr;
    ostr << "unknown command '" << command << "'";
   
    throw InvalidArg(ostr.str());
  }

  return res;
}

int
Application::help(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << USAGE;
  return 0;
}

int
Application::execute(ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  if(arguments.size() != 3)
  {
    std::cerr << "ElPoolProcess: invalid number of arguments; " << USAGE;
    return -1;
  }

  std::string service = arguments[0].name;

  if(!El::String::Manip::numeric(arguments[1].name.c_str(), IN_FILENO) ||
     !El::String::Manip::numeric(arguments[2].name.c_str(), OUT_FILENO))
  {
    std::cerr << "ElPoolProcess: invalid in/out file descriptor(s); " << USAGE;
    return -1;
  }

//  std::cerr << "+ " << IN_FILENO << " / " << OUT_FILENO << std::endl;  
  
  rlimit limit;
  memset(&limit, 0, sizeof(limit));
  getrlimit(RLIMIT_NOFILE, &limit);

  int max_fd = limit.rlim_cur;              

  for(int fd = 0; fd < max_fd; fd++)
  {
    switch(fd)
    {
    case STDIN_FILENO:
    case STDOUT_FILENO:
    case STDERR_FILENO: break;
    default:
      {
        if(fd != IN_FILENO && fd != OUT_FILENO)
        {
          close(fd);
        }        
      }
    }
  }
  
  uint32_t signature = 0;
  std::string data;

  try
  {
    while(read_data(signature, data))
    {
      std::istringstream istr(data);
      El::BinaryInStream bistr(istr);

      std::string command;
      bistr >> command;

      std::string error;
      std::ostringstream ostr;
      El::BinaryOutStream bostr(ostr);
    
      if(command == "INIT")
      {
        error = init(bistr, bostr);
      }
      else if(command == "TASK")
      {
        error = execute_task(bistr, bostr);
      }
      else
      {
        error = "Unknown command";
      }

      bool res = true;
      
      if(error.empty())
      {
        res = write_data(signature, ostr.str());
      }
      else
      {
        std::ostringstream ostr;        
        El::BinaryOutStream bostr(ostr);
        bostr << "E" << error;
        
        res = write_data(signature, ostr.str());        
      }
      
      if(!res)
      {
        break;
      }
    }
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElPoolProcess(" << service
              << "): El::Exception caught. Description:\n" << e << std::endl;
    
    return -1;
  }
  
  return 0;
}

std::string
Application::init(El::BinaryInStream& input, El::BinaryOutStream& output)
  throw(Exception, El::Exception)
{
  std::string factory_lib;
  std::string factory_func;
  std::string factory_args;
  std::string extra_libs;
  
  input >> factory_lib >> factory_func >> factory_args >> extra_libs;

  if(lib_handle_)
  {
    return "already initialized";
  }

  El::String::ListParser parser(extra_libs.c_str());
  const char* item = 0;

  while((item = parser.next_item()) != 0)  
  {
    void* lib_handle = dlopen(item, RTLD_NOW | RTLD_GLOBAL);
      
    if(!lib_handle)
    {
      std::ostringstream ostr;
      ostr << "dlopen failed for '" << item << "'. Reason:\n" << dlerror();
      
      release_lib();      
      return ostr.str();
    }

    extra_libs_.push_back(lib_handle);
  }

  lib_handle_ = dlopen(factory_lib.c_str(), RTLD_NOW | RTLD_GLOBAL);
      
  if(!lib_handle_)
  {
    std::ostringstream ostr;
    
    ostr << "dlopen failed for '" << factory_lib << "'. Reason:\n"
         << dlerror();

    release_lib();
    return ostr.str();
  }

  CreateProcessPoolTaskFactory create_factory =
    (CreateProcessPoolTaskFactory)dlsym(lib_handle_, factory_func.c_str());

  const char* error = dlerror();
      
  if(error)
  {
    std::ostringstream ostr;
    
    ostr << "dlsym failed for function '" << factory_func << "', library '"
         << factory_lib << "'. Reason:\n" << error;
        
    release_lib();
    return ostr.str();
  }

  if(create_factory == 0)
  {
    std::ostringstream ostr;
    
    ostr << "dlsym returned 0 for function '" << factory_func
         << "', library '" << factory_lib << "'. Reason:\n" << error;
        
    release_lib();
    return ostr.str();
  }

  task_factory_ = create_factory(factory_args.c_str());

  if(task_factory_ == 0)
  {
    std::ostringstream ostr;
    
    ostr << "function call '"
         << factory_func << "(\"" << factory_args
         << "\")' returned 0, library '"
         << factory_lib << "'";        
        
    release_lib();
    return ostr.str();
  }

  error = task_factory_->creation_error();
      
  if(error && *error != '\0')
  {
    std::ostringstream ostr;
    
    ostr << "task factory returned by '" << factory_func << "(\""
         << factory_args << "\")', library '"
         << factory_lib << "' is not functional. Reason:\n" << error;        
        
    release_lib();
    return ostr.str();
  }  
  
  output << "S";
  return "";
}

std::string
Application::execute_task(El::BinaryInStream& input,
                          El::BinaryOutStream& output) const
  throw(Exception, El::Exception)
{
  if(task_factory_ == 0)
  {
    return "not initialized";
  }

  try
  {
    std::string id;
    input >> id;
    
    El::Service::ProcessPool::Task_var task =
      task_factory_->create_task(id.c_str());
    
    task->read_arg(input);
    task->execute();
    
    output << "S";
    task->write_res(output);
  }
  catch(const El::Exception& e)
  {
    std::ostringstream ostr;
    ostr << "task processing failed. Reason:\n" << e;
    
    return ostr.str();
  }

  return "";
}

bool
Application::read_data(uint32_t& signature, std::string& data)
  throw(Exception, El::Exception)
{
  ssize_t size = El::read(IN_FILENO, &signature, sizeof(signature));

  if(size != sizeof(signature))
  {
    return false;
  }

  char buff[PIPE_BUF];  
  size_t data_len = 0;

  size = El::read(IN_FILENO, &data_len, sizeof(data_len));

  if(size != sizeof(data_len))
  {
    return false;
  }

  std::ostringstream ostr;

  while(data_len &&
        (size = El::read(IN_FILENO, buff, std::min(data_len, sizeof(buff)))) >
        0)
  {
    ostr.write((const char*)buff, size);
    data_len -= size;
  }

  if(size < 0)
  {
    int e = ACE_OS::last_error();    

    std::ostringstream ostr;    
    ostr << "Application::read_data: write failed with code " << e;

    if(e)
    {
      ostr << ". Description:\n" << ACE_OS::strerror(e);
    }

    throw Exception(ostr.str());
  }

  data = ostr.str();
  return true;
}

bool
Application::write_data(uint32_t signature, const std::string& data)
  throw(Exception, El::Exception)
{
  size_t data_len = data.length();

  const char* ptr = data.c_str();
  const char* end = ptr + data_len;

  bool success = El::write(OUT_FILENO, &signature, sizeof(signature)) &&
    El::write(OUT_FILENO, &data_len, sizeof(data_len));
              
  while(success && ptr < end)
  {
    ssize_t size =
      El::write(OUT_FILENO,
                ptr,
                std::min((size_t)PIPE_BUF, (size_t)(end - ptr)));
    
    if(size < 0)
    {
      success = false;
    }
    else
    {
      ptr += size;
    }
  }
    
  if(!success)
  {
    int e = ACE_OS::last_error();    
    
    std::ostringstream ostr;
    
    ostr << "Application::write_data: write failed with code " << e;

    if(e)
    {
      ostr << ". Description:\n" << ACE_OS::strerror(e);
    }

    throw Exception(ostr.str());
  }

  return true;
}
