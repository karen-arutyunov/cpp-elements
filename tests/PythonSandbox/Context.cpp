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

