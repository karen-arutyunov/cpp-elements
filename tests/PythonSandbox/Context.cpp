/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/tests/PythonSandbox/Context.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <El/Python/Object.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/Logger.hpp>

#include "Context.hpp"

El::Python::Module test_module("el.PythonSandboxTest",
                               "Module test object types.",
                               true);

Context::Type Context::Type::instance;
Interceptor::Type Interceptor::Type::instance;

void
Interceptor::pre_run(El::Python::Sandbox* sandbox,
                     El::Python::SandboxService::ObjectMap& objects)
  throw(El::Exception)
{
  stream_.reset(new std::ostringstream());
  
  logger_.reset(new El::Logging::StreamLogger(
                  *stream_,
                  El::Logging::TRACE + El::Logging::HIGH));

  El::Python::Object_var obj = new El::Logging::Python::Logger(logger_.get());
  objects["logger"] = obj;    
}
  
void
Interceptor::post_run(El::Python::SandboxService::ExceptionType exception_type,
                      El::Python::SandboxService::ObjectMap& objects,
                      El::Python::Object_var& result)
  throw(El::Exception)
{
  objects.erase("logger");
  logger_.reset(0);

  log = stream_->str();
}

void
Interceptor::write(El::BinaryOutStream& bstr) const throw(El::Exception)
{
  bstr << log;
}

void
Interceptor::read(El::BinaryInStream& bstr) throw(El::Exception)
{
  bstr >> log;
}

