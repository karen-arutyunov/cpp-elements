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

#include <Python.h>

#include <string>
#include <memory>
#include <map>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/Logger.hpp>

#include <El/PSP/Config.hpp>

class PSPTest : public El::Python::ObjectImpl
{
public:
  EL_EXCEPTION(Exception, El::ExceptionBase);
    
public:
  
  PSPTest(PyTypeObject *type, PyObject *args, PyObject *kwds)
    throw(Exception, El::Exception);
    
  PSPTest(PyObject* args) throw(Exception, El::Exception);
      
  virtual ~PSPTest() throw() {}  

  PyObject* py_echo(PyObject* args) throw(El::Exception);      

  class Type : public El::Python::ObjectTypeImpl<PSPTest, PSPTest::Type>
  {
  public:
    Type() throw(El::Python::Exception, El::Exception);
    static Type instance;
      
    PY_TYPE_METHOD_VARARGS(py_echo, "echo", "Echo function");
  };

private:
  El::PSP::Config_var config_;
  El::Logging::Python::Logger_var logger_;
};  

inline
PSPTest::Type::Type() throw(El::Python::Exception, El::Exception)
    : El::Python::ObjectTypeImpl<PSPTest, PSPTest::Type>(
        "el.psp_test.PSPTest",
        "Object providing PSP object test functionality")
{
  tp_new = 0;
}

PSPTest::PSPTest(PyTypeObject *type, PyObject *args, PyObject *kwds)
  throw(Exception, El::Exception)
    : El::Python::ObjectImpl(&Type::instance)
{
}
    
PSPTest::PSPTest(PyObject* args) throw(Exception, El::Exception)
    : El::Python::ObjectImpl(&Type::instance)
{
  PyObject* config = 0;
  PyObject* logger = 0;
    
  if(!PyArg_ParseTuple(args,
                       "OO:newsgate.ad.serving.AdServer",
                       &config,
                       &logger))
  {
    El::Python::handle_error("PSPTest::PSPTest");
  }

  if(!El::PSP::Config::Type::check_type(config))
  {
    El::Python::report_error(PyExc_TypeError,
                             "1st argument of el.psp.Config expected",
                             "PSPTest::PSPTest");
  }

  if(!El::Logging::Python::Logger::Type::check_type(logger))
  {
    El::Python::report_error(PyExc_TypeError,
                             "2nd argument of el.logging.Logger expected",
                             "PSPTest::PSPTest");
  }

  config_ = El::PSP::Config::Type::down_cast(config, true);
  logger_ = El::Logging::Python::Logger::Type::down_cast(logger, true);
}

PSPTest::Type PSPTest::Type::instance;

class PSPTestPyModule : public El::Python::ModuleImpl<PSPTestPyModule>
{
public:
  static PSPTestPyModule instance;

  PSPTestPyModule() throw(El::Exception);

  virtual void initialized() throw(El::Exception);

  PyObject* py_create_object(PyObject* args) throw(El::Exception);
  PyObject* py_cleanup_object(PyObject* args) throw(El::Exception);
  
  PY_MODULE_METHOD_VARARGS(
    py_create_object,
    "create_object",
    "Creates test object");

  PY_MODULE_METHOD_VARARGS(
    py_cleanup_object,
    "cleanup_object",
    "Cleanups test object");  

  El::Python::Object_var not_found_ex;
};

PSPTestPyModule PSPTestPyModule::instance;
    
PSPTestPyModule::PSPTestPyModule() throw(El::Exception)
    : El::Python::ModuleImpl<PSPTestPyModule>(
      "el.psp_test",
      "Module containing test object factory method.",
      true)
{
}

void
PSPTestPyModule::initialized() throw(El::Exception)
{
  not_found_ex = create_exception("NotFound");
}
  
PyObject*
PSPTestPyModule::py_create_object(PyObject* args) throw(El::Exception)
{
  return new PSPTest(args);
}

PyObject*
PSPTestPyModule::py_cleanup_object(PyObject* args) throw(El::Exception)
{
  return El::Python::add_ref(Py_None);
}

PyObject*
PSPTest::py_echo(PyObject* args) throw(El::Exception)
{
  char* str = 0;
    
  if(!PyArg_ParseTuple(args,
                       "s:el.PSPTest.echo",
                       &str))
  {
    El::Python::handle_error("PSPTest::py_echo");
  }

  std::string text = config_->string("name");
  text += std::string(": ") + str;
  
  return PyString_FromString(text.c_str());
}
