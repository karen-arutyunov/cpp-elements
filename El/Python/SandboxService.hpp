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
 * @file Elements/El/Python/SandboxService.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_SANDBOXSERVICE_HPP_
#define _ELEMENTS_EL_PYTHON_SANDBOXSERVICE_HPP_

#include <Python.h>

#include <string>

#include <El/Exception.hpp>
#include <El/Service/ProcessPool.hpp>
#include <El/Python/Exception.hpp>
#include <El/Python/Interceptor.hpp>
#include <El/Python/Sandbox.hpp>
#include <El/Python/Code.hpp>

namespace El
{
  namespace Python
  {
    class SandboxService : virtual public El::Service::ProcessPool
    {
      friend class SandboxServiceTaskFactory;
      
    public:
      
      class Callback : public virtual El::Service::Callback,
                       public virtual El::Python::Sandbox::Callback
      {
      public:
        virtual ~Callback() throw() {}
      };

      typedef std::map<std::string, El::Python::Object_var> ObjectMap;

      enum ExceptionType
      {
        ET_NONE,
        ET_CODE_NOT_COMPILED,
        ET_EXECUTION_INTERRUPTED,
        ET_ERROR_PROPAGATION,
        ET_PYTHON_EXCEPTION,
        ET_PYTHON_UNKNOWN
      };
      
      class Interceptor : public ObjectImpl
      {
      public:

        Interceptor(PyTypeObject* type) throw() : ObjectImpl(type) {}
        virtual ~Interceptor() throw() {}
        
        virtual void pre_run(Sandbox* sandbox,
                             ObjectMap& objects)
          throw(El::Exception) = 0;
        
        virtual void post_run(ExceptionType exception_type,
                              ObjectMap& objects,
                              El::Python::Object_var& result)
          throw(El::Exception) = 0;
      };

      typedef SmartPtr<Interceptor> Interceptor_var;
      
    public:
      SandboxService(Callback* callback,
                     const char* name = 0,
                     unsigned long processes = 1,
                     unsigned long timeout = 0 /* msec */,
                     const char* libs = 0)
        throw(Exception, El::Exception);
      
      virtual ~SandboxService() throw() {}
      
      PyObject* run(const El::Python::Code& code,
                    const Sandbox* sandbox = 0,
                    ObjectMap* objects = 0,
                    Interceptor* interceptor = 0,
                    std::ostream* log = 0,
                    bool current_proc = false)
        throw(CodeNotCompiled,
              ExecutionInterrupted,
              ErrorPropogation,
              Exception,
              El::Exception);
      
    private:

      struct RunCodeTask : public virtual ::El::Service::ProcessPool::TaskBase
      {
        El::Python::Code code;
        
        uint8_t use_sandbox;
        Sandbox sandbox;

        ObjectMap objects;
        Interceptor_var interceptor;

        uint8_t make_log;
        
        El::Python::Object_var result;
        std::string log;

        ExceptionType exception_type;
        std::string exception_desc;
        std::string exception_extras;
        
        RunCodeTask(const El::Python::Code& code_val,
                    const Sandbox* sandbox_val,
                    ObjectMap* objects_val,
                    Interceptor* interceptor_val,
                    bool make_log_val) throw(El::Exception);
        
        RunCodeTask() throw(El::Exception);
        
        ~RunCodeTask() throw() {}

        void clear_python() throw();

        virtual const char* type_id() const throw(El::Exception);
    
        virtual void execute() throw(Exception, El::Exception);

        virtual void write_arg(El::BinaryOutStream& bstr) const
          throw(El::Exception);
        
        virtual void read_arg(El::BinaryInStream& bstr) throw(El::Exception);
        virtual void write_res(El::BinaryOutStream& bstr) throw(El::Exception);
        
        virtual void read_res(El::BinaryInStream& bstr) throw(El::Exception);
      };

      typedef ::El::RefCount::SmartPtr<RunCodeTask> RunCodeTask_var;
    };

    typedef ::El::RefCount::SmartPtr<SandboxService> SandboxService_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
//
// RunCodeTask class
//
    inline
    SandboxService::RunCodeTask::RunCodeTask(const El::Python::Code& code_val,
                                             const Sandbox* sandbox_val,
                                             ObjectMap* objects_val,
                                             Interceptor* interceptor_val,
                                             bool make_log_val)
      throw(El::Exception)
        : El::Service::ProcessPool::TaskBase(false),
          code(code_val),
          use_sandbox(sandbox_val != 0),
          make_log(make_log_val),
          exception_type(ET_NONE)
    {
      if(sandbox_val)
      {
        sandbox = *sandbox_val;
      }

      if(objects_val)
      {
        objects = *objects_val;
      }

      interceptor = El::Python::add_ref(interceptor_val);
    }

    inline
    SandboxService::RunCodeTask::RunCodeTask()
      throw(El::Exception)
        : El::Service::ProcessPool::TaskBase(false),
          use_sandbox(0),
          make_log(0),
          exception_type(ET_NONE)
    {
    }

    inline
    void
    SandboxService::RunCodeTask::clear_python() throw()
    {
      code.clear();
      sandbox.disable();
      objects.clear();
      interceptor = 0;
      result = 0;
    }
        
    inline
    const char*
    SandboxService::RunCodeTask::type_id() const throw(El::Exception)
    {
      return "SandboxService::RunCodeTask";
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_SANDBOXSERVICE_HPP_
