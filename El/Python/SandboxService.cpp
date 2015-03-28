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
 * @file Elements/El/Python/SandboxService.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>
#include <dlfcn.h>

#include <iostream>
#include <sstream>
#include <string>

#include <El/Exception.hpp>
#include <El/Moment.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Interceptor.hpp>
#include <El/Python/Utility.hpp>
#include <El/Python/InterceptorImpl.hpp>

#include "SandboxService.hpp"

namespace El
{
  namespace Python
  {
    SandboxService::SandboxService(Callback* callback,
                                   const char* name,
                                   unsigned long processes,
                                   unsigned long timeout,
                                   const char* libs)
      throw(Exception, El::Exception)
        : El::Service::ServiceBase<El::Sync::ThreadRWPolicy>(callback,
                                                             name,
                                                             processes,
                                                             1024 * 1024),
          El::Service::ProcessPool(callback,
                                   "libElPythonSandboxService.so",
                                   "create_task_factory",
                                   0,
                                   libs,
                                   name,
                                   processes,
                                   timeout)
    {
    }

    PyObject*
    SandboxService::run(const El::Python::Code& code,
                        const Sandbox* sandbox,
                        ObjectMap* objects,
                        Interceptor* interceptor,
                        std::ostream* log,
                        bool current_proc)
      throw(CodeNotCompiled,
            ExecutionInterrupted,
            ErrorPropogation,
            Exception,
            El::Exception)
    {
      assert(!sandbox->enabled());
      
      RunCodeTask_var task = new RunCodeTask(code,
                                             sandbox,
                                             objects,
                                             interceptor,
                                             log != 0);

      El::Python::Object_var result;
      ObjectMap result_objects;

      bool res = true;

      if(current_proc)
      {
        task->execute();
      }
      else
      {
        res = execute(task.in());
      }
      
      if(res && started())
      {
        if(!current_proc)
        {
          El::Python::AllowOtherThreads guard;
          task->wait();
        }

        result = task->result.retn();
        task->objects.swap(result_objects);

        // Need clear python aware members of task object before released.
        // Otherwise it can be deleted in python unaware process poll worker
        // thread which will lead to crash.
        task->clear_python();
        
        if(task->timeout_occured())
        {
          std::ostringstream ostr;
          ostr << "El::Python::SandboxService::run: execution interrupted. "
            "Reason:\nallowed execution time ("
               << El::Moment::time(ACE_Time_Value(timeout_ / 1000,
                                                  (timeout_ % 1000) * 1000))
               << ") expired";

          throw ExecutionInterrupted(ostr.str());
        }

        if(log)
        {
          *log << task->log;
        }

        if(objects)
        {
          result_objects.swap(*objects);
        }

        switch(task->exception_type)
        {
        case ET_CODE_NOT_COMPILED:
          {
            throw CodeNotCompiled(task->exception_desc);
          }
        case ET_EXECUTION_INTERRUPTED:
          {
            throw ExecutionInterrupted(task->exception_desc);
          }
        case ET_ERROR_PROPAGATION:
          {
            std::istringstream istr(task->exception_extras);        
            El::BinaryInStream bstr(istr);

            std::string context;
            Object_var type;
            Object_var value;
            Object_var traceback;

            // e.type.in(), e.traceback.in() are not marshalable
            bstr >> context >> value;
            
            ErrorPropogation e(task->exception_desc.c_str(),
                               context.c_str(),
                               type.in(),
                               value.in(),
                               traceback.in());
            
            throw e;
          }
        case ET_PYTHON_EXCEPTION:
          {
            throw Exception(task->exception_desc);
          }
        default:
          {
            // keep going
          }
        }

        std::string error = task->error();

        if(!error.empty())
        {
          std::ostringstream ostr;
          ostr << "El::Python::SandboxService::run: execution error. "
            "Description:\n" << error;

          throw Exception(ostr.str());
        }
        
        return result.retn();
      }
      else
      {
        task->clear_python();
        
        throw ExecutionInterrupted(
          "El::Python::SandboxService::run: can't enqueue task; "
          "probably terminating ...");
      }
    }

    //
    // SandboxServiceTaskFactory class
    //
    class SandboxServiceTaskFactory :
      public ::El::Service::ProcessPool::TaskFactoryInterface
    {
    public:
      EL_EXCEPTION(Exception, El::Service::Exception);

    public:
      SandboxServiceTaskFactory() throw(Exception, El::Exception);

    private:
      virtual ~SandboxServiceTaskFactory() throw() {}
      virtual const char* creation_error() throw() { return 0; }
      virtual void release() throw(Exception) { delete this; }
  
      virtual ::El::Service::ProcessPool::Task* create_task(const char* id)
        throw(El::Exception);

    private:
      El::Python::Use python_use_;
      El::Python::InterceptorImpl::Installer installer_;
    };

    SandboxServiceTaskFactory::SandboxServiceTaskFactory()
      throw(Exception, El::Exception)
        : installer_(El::Python::Sandbox::INTERCEPT_FLAGS)
    {
      rlimit limit;
      memset(&limit, 0, sizeof(limit));
      setrlimit(RLIMIT_CORE, &limit);
    }

    ::El::Service::ProcessPool::Task*
    SandboxServiceTaskFactory::create_task(const char* id)
      throw(El::Exception)
    {
      if(strcmp(id, "SandboxService::RunCodeTask") == 0)
      {
        return new SandboxService::RunCodeTask();
      }

      std::ostringstream ostr;
      ostr << "SandboxServiceTaskFactory::create_task: unknown task id '"
           << id << "'";
      
      throw Exception(ostr.str());
    }
    
//
// RunCodeTask class
//
    void
    SandboxService::RunCodeTask::write_arg(El::BinaryOutStream& bstr) const
      throw(El::Exception)
    {
      El::Python::EnsureCurrentThread guard;
      
      bstr << code << make_log << use_sandbox;

      if(use_sandbox)
      {
        bstr << sandbox;
      }

      bstr.write_map(objects);
      bstr << interceptor;
    }

    void
    SandboxService::RunCodeTask::read_arg(El::BinaryInStream& bstr)
      throw(El::Exception)
    {
      // SandboxServiceTaskFactory ensured Python execution context
      
      bstr >> code >> make_log >> use_sandbox;

      if(use_sandbox)
      {
        bstr >> sandbox;
      }
      else
      {
        sandbox.disable();
      }

      bstr.read_map(objects);

      El::Python::Object_var obj;
      bstr >> obj;
      
      interceptor = static_cast<Interceptor*>(obj.in());
      interceptor.add_ref();
    }
    
    void
    SandboxService::RunCodeTask::write_res(El::BinaryOutStream& bstr)
      throw(El::Exception)
    {
      // SandboxServiceTaskFactory ensured Python execution context
      bstr << result.in() << (uint32_t)exception_type << exception_desc
           << exception_extras << log;

      bstr.write_map(objects);

      if(interceptor.in())
      {
        interceptor->write(bstr);
      }

      objects.clear();
      result = 0;
      interceptor = 0;
    }
        
    void
    SandboxService::RunCodeTask::read_res(El::BinaryInStream& bstr)
      throw(El::Exception)
    {
      El::Python::EnsureCurrentThread guard;
      
      uint32_t et = ET_NONE;
      bstr >> result >> et >> exception_desc >> exception_extras >> log;
      exception_type = (ExceptionType)et;

      bstr.read_map(objects);

      if(interceptor.in())
      {
        interceptor->read(bstr);
      }
      
/*      
      El::Python::Object_var obj;
      bstr >> obj;
      
      interceptor = static_cast<Interceptor*>(obj.in());
      interceptor.add_ref();
*/
    }
    
    void
    SandboxService::RunCodeTask::execute() throw(Exception, El::Exception)
    {
      El::Python::Object_var dictionary;
      bool interceptor_prerun = false;

      try
      {
        if(interceptor.in())
        {
          interceptor->pre_run(use_sandbox ? &sandbox : 0, objects);
          interceptor_prerun = true;
        }
            
        if(!objects.empty())
        {
          dictionary = PyDict_New();

          if(dictionary.in() == 0)
          {
            El::Python::handle_error(
              "El::Python::SandboxService::RunCodeTask::execute: "
              "PyDict_New failed");
          }

          for(ObjectMap::iterator i(objects.begin()), e(objects.end()); i != e;
              ++i)
          { 
            if(PyDict_SetItemString(dictionary.in(),
                                    i->first.c_str(),
                                    i->second.in()))
            {
              std::ostringstream ostr;
              ostr << "El::Python::SandboxService::RunCodeTask::execute: "
                "PyDict_SetItemString('" << i->first << "') failed";
              
              El::Python::handle_error(ostr.str().c_str());
            }
          }
          
          PyObject* module = PyImport_AddModule("__main__");

          if(module == 0)
          {
            El::Python::handle_error(
              "El::Python::SandboxService::RunCodeTask::execute: "
              "PyImport_AddModule failed");
          }

          PyObject* main_global_dict = PyModule_GetDict(module);

          //
          // Copying to dictionary builtins, name, doc string, imported modules
          //
          if(PyDict_Merge(dictionary.in(), main_global_dict, 0) < 0)
          {
            El::Python::handle_error(
              "El::Python::SandboxService::RunCodeTask::execute: "
              "PyDict_Merge failed");
          }
        }
        
        std::auto_ptr<std::ostringstream> ostr(make_log ?
                                               new std::ostringstream() : 0);
        
        try
        {
          result = code.run(dictionary.in(),
                            0,
                            use_sandbox ? &sandbox : 0,
                            ostr.get());

          if(dictionary.in())
          {
            PyDict_Clear(dictionary.in());
            dictionary = 0;
          }
        }
        catch(const CodeNotCompiled& e)
        {
          exception_type = ET_CODE_NOT_COMPILED;
          exception_desc = e.what();
        }
        catch(const ExecutionInterrupted& e)
        {
          exception_type = ET_EXECUTION_INTERRUPTED;
          exception_desc = e.what();
        }
        catch(const ErrorPropogation& e)
        {
          exception_type = ET_ERROR_PROPAGATION;
          exception_desc = e.what();
          
          std::ostringstream ostr;
          
          {
            El::BinaryOutStream bstr(ostr);
            // e.type.in(), e.traceback.in() are not marshalable
            bstr << e.context << e.value.in();
          }
          
          exception_extras = ostr.str();
        }
        catch(const El::Python::Exception& e)
        {
          exception_type = ET_PYTHON_EXCEPTION;
          exception_desc = e.what();
        }

        if(make_log)
        {
          log = ostr->str();
        }

        if(interceptor.in())
        {
          interceptor->post_run(exception_type, objects, result);
        }
      }
      catch(...)
      {
        if(dictionary.in())
        {
          PyDict_Clear(dictionary.in());
        }
        
        if(interceptor_prerun && interceptor.in())
        {
          interceptor->post_run(
            exception_type == ET_NONE ? ET_PYTHON_UNKNOWN : exception_type,
            objects,
            result);
        }
        
        throw;
      }
    }    
  }
}

extern "C"
::El::Service::ProcessPool::TaskFactoryInterface*
create_task_factory(const char* args)
{
  return new El::Python::SandboxServiceTaskFactory();
}
