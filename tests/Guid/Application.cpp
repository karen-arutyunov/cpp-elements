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
 * @file   Elements/test/Guid/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <El/Guid.hpp>
#include <El/Stat.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nElTestGuid [help]\n";
}

int
main(int argc, char** argv)
{
  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'ElTestGuid help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestGuid: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestGuid: unknown exception caught\n";
  }
  
  return -1;
}

Application::Application() throw(Application::Exception, El::Exception)
    : guid_generate_counter_(1000000)
{
}

Application::~Application() throw()
{
}

int
Application::run(int& argc, char** argv)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string command;
  
  int i = 0;  

  if(argc > 1)
  {
    command = argv[i++];
  }

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

  if(command == "help")
  {
    return help(arguments);
  }

  test(arguments);
  return 0;
}

int
Application::help(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << USAGE;
  return 0;
}

int
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{

  std::cerr << "Machine level uniqueness check ...\n";

  pid_t child_pid = fork();

  if(child_pid == 0)
  {
    //
    // Child process goes here
    //

    std::string filename = guids_file(getpid());
    generate_guids_file(filename.c_str());
    
    std::exit(EXIT_SUCCESS);
  }

  std::string filename = guids_file(getpid());
  generate_guids_file(filename.c_str());
  
  int status = 0;
  pid_t ecode = waitpid(child_pid, &status, 0);

  if(ecode == child_pid)
  {
    if(!WIFEXITED(status))
    {
      throw Exception("Application::test: abnormal child termination");
    }
  }
  else
  {
    int error = ACE_OS::last_error();    

    std::ostringstream ostr;
    ostr << "Application::test: waitpid failed with "
      "code " << error << " when executing. Description:\n"
         << ACE_OS::strerror(error);

    throw Exception(ostr.str());
  }

  std::string child_filename = guids_file(child_pid);
  
  if(load_guids(filename.c_str(), guids_) &&
     load_guids(child_filename.c_str(), guids_))
  {
    unlink(filename.c_str());
    unlink(child_filename.c_str());    
  }

  std::cerr << "Process level uniqueness check ...\n";

  guids_.clear();
  
  thread_pool_ = new El::Service::ThreadPool(this, "ThreadPool", 50);
  
  for(unsigned long i = 0; i < 100; i++)  
  {
    El::Service::ThreadPool::Task_var task =
      new GenerateGUIDTask(this, thread_pool_.in());

    thread_pool_->execute(task.in());
  }

  thread_pool_->start();
  thread_pool_->wait();
  
  std::cerr << "done\n";

  El::Stat::TimeMeter guid_generating("Guid generating");
  
  for(unsigned long i = 0; i < 100000; i++)  
  {
    El::Guid guid;

    {
      El::Stat::TimeMeasurement measurement(guid_generating);
      guid.generate();
    }
    
    std::string guid_str = guid.string();
    
    if(guid != El::Guid(guid_str))
    {
      std::ostringstream ostr;
      ostr << "Application::test: guid differes from it's classic "
        "string representation" << guid_str;
      
      throw Exception(ostr.str());
    }

    guid_str = guid.string(El::Guid::GF_DENSE);

    if(guid != El::Guid(guid_str))
    {
      std::ostringstream ostr;
      ostr << "Application::test: guid differes from it's dense "
        "string representation" << guid_str;
      
      throw Exception(ostr.str());
    }

    El::Guid guid2(guid_str);
    
    if(!(guid == guid2))
    {
      std::ostringstream ostr;
      ostr << "Application::test: guid differes from it's classic "
        "string representation" << guid_str;
      
      throw Exception(ostr.str());
    }

    guid2.generate();

    El::Guid small(guid < guid2 ? guid : guid2);
    El::Guid big(guid < guid2 ? guid2 : guid);

    if(!(small < big))
    {
      std::ostringstream ostr;
      ostr << "Application::test: Guid::operator< failed";
      
      throw Exception(ostr.str());
    }
    
    if(small >= big)
    {
      std::ostringstream ostr;
      ostr << "Application::test: Guid::operator>= failed";
      
      throw Exception(ostr.str());
    }    
    
    if(!(big > small))
    {
      std::ostringstream ostr;
      ostr << "Application::test: Guid::operator> failed";
      
      throw Exception(ostr.str());
    }
    
    if(big <= small)
    {
      std::ostringstream ostr;
      ostr << "Application::test: Guid::operator<= failed";
      
      throw Exception(ostr.str());
    }    
  }

  guid_generating.dump(std::cerr);
  
  return 0;
}

bool
Application::notify(El::Service::Event* event)
  throw(El::Exception)
{
  GenerateGUIDTask* gum = dynamic_cast<GenerateGUIDTask*>(event);
        
  if(gum != 0)
  {
    El::Guid guid;
    guid.generate();
    
    std::string guid_str = guid.string(El::Guid::GF_DENSE);

    WriteGuard guard(lock_);

    if(!guid_generate_counter_ || !thread_pool_->started())
    {
      return true;
    }

    StringSet::const_iterator it = guids_.find(guid_str);

    if(it != guids_.end())
    {
      thread_pool_->stop();

      std::cerr << "Application::notify: duplicated GUID " << guid_str
                << std::endl;
    }

    if(--guid_generate_counter_)
    {
      guids_.insert(guid_str);
//      std::cerr << guid_str << std::endl;   
//      std::cerr << guid_generate_counter_ << " left (" << guids_.size()
//                << ")\n";
      
      El::Service::ThreadPool::Task_var task =
        new GenerateGUIDTask(this, thread_pool_.in());
      
      thread_pool_->execute(task.in());      
    }
    else
    {
      thread_pool_->stop();      
    }
      
    return true;
  }
    
  El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);
  
  if(error)
  {
    std::cerr << "Application::notify: error received. Description:\n"
              << *error << std::endl;
    
    return true;
  }

  std::cerr << "Application::notify: unknown event from "
            << event->source_name << " received. Description:\n"
            << *error << std::endl;

  return false;
}

std::string
Application::guids_file(pid_t pid) throw(El::Exception)
{
  std::ostringstream ostr;
  ostr << "guids." << pid;
  return ostr.str();
}

void
Application::generate_guids_file(const char* filename)
  throw(El::Exception)
{
  std::fstream file(filename, std::ios::out);

  if(!file.is_open())
  {
    std::ostringstream ostr;
    ostr << "Application::generate_guids_file: failed to open for write "
      "access file " << filename;
      
    throw Exception(ostr.str());
  }

  for(size_t i = 0; i < 1000000; i++)
  {
    El::Guid guid;
    guid.generate();
    
    file << guid.string(El::Guid::GF_DENSE) << std::endl;
  }
}

bool
Application::load_guids(const char* filename, StringSet& guids)
  throw(El::Exception)
{
  static size_t dups_counter = 10;
  
  std::fstream file(filename, std::ios::in);

  if(!file.is_open())
  {
    std::ostringstream ostr;
    ostr << "Application::generate_guids_file: failed to open for read "
      "access file " << filename;
      
    throw Exception(ostr.str());
  }

  bool success = true;
  std::string guid_str;
  
  while(std::getline(file, guid_str) && dups_counter > 0)
  {
    StringSet::const_iterator it = guids.find(guid_str);

    if(it != guids.end())
    {
      std::cerr << "Application::load_guids: duplicated GUID " << guid_str
                << std::endl;
      
      success = false;
      dups_counter--;
    }

    guids.insert(guid_str);
  }

  return success;
}

