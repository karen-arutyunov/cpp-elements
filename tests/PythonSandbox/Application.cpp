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
 * @file   Elements/test/PythonSandbox/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <Python.h>

#include <string>
#include <sstream>
#include <fstream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>

#include <El/Python/Code.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/Sandbox.hpp>
#include <El/Python/InterceptorImpl.hpp>
#include <El/Python/SandboxService.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/Logger.hpp>

#include "Context.hpp"

class Application : public El::Python::SandboxService::Callback
{
public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    
public:
    
  Application() throw(Exception, El::Exception);
  virtual ~Application() throw();

  int run(int& argc, char** argv) throw(Exception, El::Exception);

  virtual bool notify(El::Service::Event* event) throw(El::Exception);
  virtual bool interrupt_execution(std::string& reason) throw(El::Exception);
  
private:
  void add_path(const char* path) throw(Exception, El::Exception);
};

namespace
{
  const char USAGE[] =
  "Usage:\nElTestPythonSandbox --file=<script file> [--max-ticks=<unsigned>] "
    "[--timeout=<unsigned>] [--call-max-depth=<unsigned>] "
    "[--max-mem=<unsigned>] [--debug] [--local]\n";  
}

int
main(int argc, char** argv)
{
  srand(time(0));
  
  El::Python::Use use;
  
  El::Python::InterceptorImpl::Installer installer(
    El::Python::Sandbox::INTERCEPT_FLAGS);
  
  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch(const El::Exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  catch(...)
  {
    std::cerr << "unknown exception caught.\n";
  }
  
  return -1;
}

Application::Application() throw(Exception, El::Exception)
{
}

Application::~Application() throw()
{
}

int
Application::run(int& argc, char** argv) throw(Exception, El::Exception)
{
  if (argc < 2)
  {
    std::ostringstream ostr;
    ostr << "Too few arguments\n" << USAGE;
    throw Exception(ostr.str());
  }

  std::string filename;
  size_t max_ticks = 0;
  size_t timeout = 0;
  size_t call_max_depth = 0;
  size_t max_mem = 0;
  std::ostream* log = 0;
  bool local = false;
  
  for(int i = 1; i < argc; i++)
  {
    const char* arg = argv[i];
    
    if(!strncmp(arg, "--file=", 7))
    {
      filename = arg + 7;
    }
    else if(!strcmp(arg, "--debug"))
    {
      log = &std::cerr;
    }
    else if(!strcmp(arg, "--local"))
    {
      local = true;
    }
    else if(!strncmp(arg, "--max-ticks=", 12))
    {
      if(!El::String::Manip::numeric(arg + 12, max_ticks))
      {
        std::ostringstream ostr;
        ostr << "Invalid --max-ticks value\n" << USAGE;
        throw Exception(ostr.str());
      }        
    }
    else if(!strncmp(arg, "--timeout=", 10))
    {
      if(!El::String::Manip::numeric(arg + 10, timeout))
      {
        std::ostringstream ostr;
        ostr << "Invalid --timeout value\n" << USAGE;
        throw Exception(ostr.str());
      }
    }
    else if(!strncmp(arg, "--call-max-depth=", 17))
    {
      if(!El::String::Manip::numeric(arg + 17, call_max_depth))
      {
        std::ostringstream ostr;
        ostr << "Invalid --call-max-depth value\n" << USAGE;
        throw Exception(ostr.str());
      }
    }
    else if(!strncmp(arg, "--max-mem=", 10))
    {
      if(!El::String::Manip::numeric(arg + 10, max_mem))
      {
        std::ostringstream ostr;
        ostr << "Invalid --max-mem value\n" << USAGE;
        throw Exception(ostr.str());
      }
    }
    else
    {
      std::ostringstream ostr;
      ostr << "Unexpected argument " << arg << std::endl << USAGE;
      throw Exception(ostr.str());
    }
  }

  std::fstream file(filename.c_str(), std::ios::in);

  if(!file.is_open())
  {
    std::ostringstream ostr;
    ostr << "Failed to open file '" << filename << "' for read access";

    throw Exception(ostr.str());
  }

  std::string script;
  
  {    
    std::ostringstream ostr;
  
    while(!file.fail())
    {
      char buff[1024];
      file.read(buff, sizeof(buff));
      ostr.write(buff, file.gcount());
    }

    script = ostr.str();
  }
  
//  add_path("/home/karen_arutyunov/projects/Elements/tests/PythonSandbox");
  
  El::Python::Sandbox sandbox(El::Python::Sandbox::SAFE_MODULES,
                              El::Python::Sandbox::SAFE_BUILTINS,
                              max_ticks,
                              ACE_Time_Value(timeout),
                              call_max_depth,
                              max_mem);
  
  El::Python::Code code(script.c_str(), filename.c_str());
  El::Python::Object_var result;

  Context_var context = new Context();
  context->value = "ABC";
  context->lang = new El::Python::Lang(El::Lang("rus"));
  context->country = new El::Python::Country(El::Country("UKR"));
  context->moment = new El::Python::Moment(El::Moment(ACE_OS::gettimeofday()));

  Interceptor_var interceptor = new Interceptor();

/*
  std::ostringstream ostr;

  {
    El::BinaryOutStream bstr(ostr);
    bstr << context;
  }

  std::istringstream istr(ostr.str());
  El::BinaryInStream bstr(istr);

  bstr >> context;
*/
  if(local)
  {
    El::Python::Object_var dictionary = PyDict_New();

    if(dictionary.in() == 0)
    {
      El::Python::handle_error("Application::run: PyDict_New failed");
    }

    try
    {      
      if(PyDict_SetItemString(dictionary.in(), "context", context.in()))
      {
        El::Python::handle_error(
          "Application::run: PyDict_SetItemString('context') failed");
      }
    
      PyObject* module = PyImport_AddModule("__main__");

      if(module == 0)
      {
        El::Python::handle_error(
          "Application::run: PyImport_AddModule failed");
      }

      PyObject* main_global_dict = PyModule_GetDict(module);

      //
      // Copying to dictionary builtins, name, doc string, imported modules
      //
      if(PyDict_Merge(dictionary.in(), main_global_dict, 0) < 0)
      {
        El::Python::handle_error("Application::run: PyDict_Merge failed");
      }
    
      result = code.run(dictionary.in(), 0, &sandbox, log);
      PyDict_Clear(dictionary.in());
    }
    catch(...)
    {
      PyDict_Clear(dictionary.in());
      throw;
    }
  }
  else
  {
    El::Python::SandboxService::ObjectMap objects;
    objects["context"] = context;
    
    El::Python::SandboxService_var srv =
      new El::Python::SandboxService(this,
                                     "SandboxService",
                                     1,
                                     timeout ? (timeout + 1) * 1000 : 0,
                                     "libElTestPythonSandboxContext.so");
    
    srv->start();

    try
    {
      result = srv->run(code, &sandbox, &objects, interceptor.in(), log);
    }
    catch(...)
    {
      srv->stop();
      srv->wait();
      throw;
    }

    El::Python::Object_var o = objects["context"];
    context = Context::Type::down_cast(o.in(), true);
    
    srv->stop();
    srv->wait();

//    std::cerr << El::Python::ulong_from_number(result.in()) << std::endl;
  }

  if(context->value != "ABC")
  {
    std::cerr << "context changed:\n";
    context->dump(std::cerr);
    std::cerr << std::endl;
  }
  
  if(!interceptor->log.empty())
  {
    std::cerr << "interceptor log:\n" << interceptor->log;
  }

  return 0;
}

void
Application::add_path(const char* path) throw(Exception, El::Exception)
{
  PyObject* module = PyImport_AddModule("sys");

  if(module == 0)
  {
    El::Python::handle_error(
      "Application::add_path: PyImport_AddModule failed");
  }

  El::Python::Object_var paths = PyObject_GetAttrString(module, "path");
        
  if(paths.in() == 0)
  {
    El::Python::handle_error(
      "Application::add_path: PyObject_GetAttrString failed");
  }

  if(!PyList_Check(paths.in()))
  {
    throw Exception("Application::add_path: sys.path not a list");
  }

  El::Python::Object_var p = PyString_FromString(path);
  PyList_Append(paths.in(), p.in());
}

bool
Application::notify(El::Service::Event* event) throw(El::Exception)
{
  El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);
      
  if(error)
  {
    std::cerr << "Application::notify: error " << *error << std::endl;
    return true;
  }

  std::cerr << "Application::notify: unknown " << *event << std::endl;
  return true;
}

bool
Application::interrupt_execution(std::string& reason) throw(El::Exception)
{
  return false;
}

