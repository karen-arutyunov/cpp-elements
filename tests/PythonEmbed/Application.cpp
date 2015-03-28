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
 * @file   Elements/test/PythonEmbed/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <Python.h>
#include <pthread.h>

#include <string>
#include <sstream>

#include <El/Exception.hpp>
#include <El/Stat.hpp>
#include <El/String/Manip.hpp>

#include <El/Python/Utility.hpp>
#include <El/Python/Code.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Module.hpp>

#include <El/Service/ThreadPool.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
  "\nUsage:\nElTestPythonEmbed ( help | run ) \n";

  El::Stat::TimeMeter simple_string_meter("PyRun_SimpleString");

  const unsigned long THREADS_COUNT = 1;
  const unsigned long SCRIPT_EXEC_COUNT = 10000;

  const char PYPROG[] =
    "def run(count) :\n"
    "\tobj = myobjects.MyObject('Bob', last='Dilan')\n"
    "\tobj.first = 'Bobs'\n"
    "\tobj.first_last('Albert', 'Gore')\n"
    "\tobj.first_last_num(number=101)\n"
    "\tprint '%d: %s %s (%s)' % (obj.number, obj.first, obj.last, obj.name())\n"
    "\tA, str, i = 1, \"AAAAA\", 1\n"
    "\twhile i < count:\n"
    "\t\tstr = \"assdfasd \" + str\n"
    "\t\ti += 1\n"
    "\t\tA += 1\n"
    "run(count)\n";
}

struct MyObject : public El::Python::ObjectImpl
{
  MyObject(PyTypeObject *type, PyObject *args, PyObject *kwds)
    throw(El::Exception);

  virtual ~MyObject() throw() {}

  PyObject* py_name() throw(El::Exception);

  PyObject* py_first_last(PyObject* args) throw(El::Exception);

  PyObject* py_first_last_num(PyObject* args, PyObject* kwds)
    throw(El::Exception);

  PyObject* py_get_first() throw(El::Exception);
  void      py_set_first(PyObject* value) throw(El::Exception);  

  PyObject* py_get_last() throw(El::Exception);
  void      py_set_last(PyObject* value) throw(El::Exception);  

  PyObject* py_get_number() throw(El::Exception);
  void      py_set_number(PyObject* value) throw(El::Exception);  

private:
  El::Python::Object_var first;
  El::Python::Object_var last;
  El::Python::Object_var number;
};

MyObject::MyObject(PyTypeObject *type, PyObject *args, PyObject *kwds)
    throw(El::Exception)
  : El::Python::ObjectImpl(type)
{
  PyObject* f = 0;
  PyObject* l = 0;
  PyObject* n = 0;
    
  const char *kwlist[] = {"first", "last", "number", NULL};
    
  if(!PyArg_ParseTupleAndKeywords(args,
                                  kwds,
                                  "|OOO:myobjects.MyObject",
                                  (char**)kwlist,
                                  &f,
                                  &l,
                                  &n))
  {
    El::Python::handle_error("MyObject::MyObject");
  }

  first = El::Python::add_ref(f ? f : Py_None);
  last = El::Python::add_ref(l ? l : Py_None);
  number = n ? El::Python::add_ref(n) : PyInt_FromLong(33);
}  

PyObject*
MyObject::py_name() throw(El::Exception)  
{
  El::Python::Object_var args = Py_BuildValue("OO", first.in(), last.in());

  if(args.in() == 0)
  {
    El::Python::handle_error(
      "MyObject::py_name: Py_BuildValue failed");
  }
  
  El::Python::Object_var format = PyString_FromString("%s %s");

  if(format.in() == 0)
  {
    El::Python::handle_error(
      "MyObject::py_name: PyString_FromString failed");
  }
  
  return PyString_Format(format.in(), args.in());
}

PyObject*
MyObject::py_first_last(PyObject *args) throw(El::Exception)
{
  PyObject* f = 0;
  PyObject* l = 0;
    
  if(!PyArg_ParseTuple(args, "|OO:myobjects.MyObject.first_last", &f, &l))
  {
    El::Python::handle_error("MyObject::py_first_last");
  }

  if(f)
  {
    first = El::Python::add_ref(f);
  }
  
  if(l)
  {
    last = El::Python::add_ref(l);
  }
  
  return py_name();
}

PyObject*
MyObject::py_first_last_num(PyObject* args, PyObject* kwds)
  throw(El::Exception)
{
  PyObject* f = 0;
  PyObject* l = 0;
  PyObject* n = 0;
    
  const char *kwlist[] = {"first", "last", "number", NULL};
    
  if(!PyArg_ParseTupleAndKeywords(args,
                                  kwds,
                                  "|OOO:myobjects.MyObject.py_first_last_num",
                                  (char**)kwlist,
                                  &f,
                                  &l,
                                  &n))
  {
    El::Python::handle_error("MyObject::py_first_last_num");
  }

  if(f)
  {
    first = El::Python::add_ref(f);
  }

  if(l)
  {
    last = El::Python::add_ref(l);
  }

  if(n)
  {
    number = El::Python::add_ref(n);
  }
  
  return py_name();
}

PyObject*
MyObject::py_get_first() throw(El::Exception)
{
  return first.add_ref();
}

void
MyObject::py_set_first(PyObject *value) throw(El::Exception)
{
  if(value == NULL)
  {
    El::Python::report_error(PyExc_TypeError,
                             "Cannot delete the first attribute",
                             "MyObject::py_set_first");
  } 

  if(!PyString_Check(value))
  { 
    El::Python::report_error(PyExc_TypeError,
                             "The first attribute value must be a string",
                             "MyObject::py_set_first");
  }
  
  first = El::Python::add_ref(value); 
}

PyObject*
MyObject::py_get_last() throw(El::Exception)
{
  return last.add_ref();
}

void
MyObject::py_set_last(PyObject *value) throw(El::Exception)
{
  if(value == NULL)
  {
    El::Python::report_error(PyExc_TypeError,
                             "Cannot delete the last attribute",
                             "MyObject::py_set_last");
  } 

  if(!PyString_Check(value))
  { 
    El::Python::report_error(PyExc_TypeError,
                             "The last attribute value must be a string",
                             "MyObject::py_set_last");
  }
  
  last = El::Python::add_ref(value); 
}

PyObject*
MyObject::py_get_number() throw(El::Exception)
{
  return number.add_ref();
}

void
MyObject::py_set_number(PyObject *value) throw(El::Exception)
{
  if(value == NULL)
  {
    El::Python::report_error(PyExc_TypeError,
                             "Cannot delete the number attribute",
                             "MyObject::py_set_number");
  } 

  if(!PyInt_Check(value))
  { 
    El::Python::report_error(PyExc_TypeError,
                             "The last attribute value must be an integer",
                             "MyObject::py_set_number");
  }
  
  number = El::Python::add_ref(value); 
}

class MyObjectType : public El::Python::ObjectTypeImpl<MyObject, MyObjectType>
{
public:
  MyObjectType() : El::Python::ObjectTypeImpl<MyObject, MyObjectType>(
    "myobjects.MyObject", "My objects"){}

  static MyObjectType instance;

  PY_TYPE_METHOD_NOARGS(py_name, "name",
                        "Return first and last name combined");
  
  PY_TYPE_METHOD_VARARGS(py_first_last, "first_last",
                        "Sets first and last names");

  PY_TYPE_METHOD_KWDS(py_first_last_num, "first_last_num",
                      "Sets first and last names and number");

  PY_TYPE_MEMBER(py_get_first, py_set_first, "first",
                 "first name");

  PY_TYPE_MEMBER(py_get_last, py_set_last, "last",
                 "last name");

  PY_TYPE_MEMBER(py_get_number, py_set_number, "number",
                 "object number");
};

MyObjectType MyObjectType::instance;

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
              << "\nRun 'ElTestPythonEmbed help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestPythonEmbed: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestPythonEmbed: unknown exception caught\n";
  }
  
  return -1;
}

Application::Application() throw(Application::Exception, El::Exception)
    : counter_(0)
{
}

Application::~Application() throw()
{
}

int
Application::run(int& argc, char** argv)
  throw(InvalidArg, Exception, El::Exception)
{
  if(argc < 2)
  {
    throw InvalidArg("too few arguments");
  }
  
  std::string command;
  
  int i = 1;
  command = argv[i++];

  ArgList arguments;

  for(; i < argc; i++)
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
  else if(command == "run")
  {
    test(arguments);
  }
  else
  {
    throw InvalidArg("unknown command");
  }
  
  return 0;
}

int
Application::help(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << USAGE;
  return 0;
}

struct PythonScript : public El::Service::ThreadPool::ServiceEvent
{
  PythonScript(El::Service::Callback* callback)
    throw(El::Service::ThreadPool::InvalidArg,
          El::Service::ThreadPool::Exception,
          El::Exception)
      : El__Service__ThreadPool__ServiceEventBase(callback, 0, false)
  {
  }
  
  virtual ~PythonScript() throw() 
  {
  }  
};

bool
Application::notify(El::Service::Event* event) throw(El::Exception)
{  
  if(El::Service::is_error(event))
  {
    std::cerr << El::Service::error_message(event, "Application::notify: ")
              << std::endl;
      
    thread_pool_->stop();
    return true;
  }

  if(dynamic_cast<PythonScript*>(event) == 0)
  {
    return false;
  }
  
  {
    WriteGuard guard(lock_);
    
    if(counter_ < SCRIPT_EXEC_COUNT)
    {
      counter_++;

      El::Service::ThreadPool::Task_var task = new PythonScript(this);
      thread_pool_->execute(task.in());
    }
    else
    {
      thread_pool_->stop();
      return true;
    }
  }    

  El::Python::EnsureCurrentThread ensure;

  try
  {
    El::Stat::TimeMeasurement tm(simple_string_meter);

    El::Python::Object_var long_val = PyInt_FromLong(100);
    El::Python::Object_var loc_dict = PyDict_New();    

    if(PyDict_SetItemString(loc_dict, "count", long_val))
    {
      El::Python::handle_error(
        "Application::notify: PyDict_SetItemString failed");
    }
/*
    El::Python::Object_var noddy = Noddy_new(noddy_type, 0, 0);

    if(PyDict_SetItemString(loc_dict, "noddy", noddy))
    {
      El::Python::handle_error(
        "Application::notify: PyDict_SetItemString failed\n");
    }
*/

    El::Python::Object_var res = code_.run(0, loc_dict);
  }
  catch(const El::Exception& e)
  {
    std::cerr << "Application::notify: failed to run program. Reason:\n"
              << e << std::endl;
    
    thread_pool_->stop();
  }

  return true;
}

El::Python::Module mymodule("myobjects",
                            "Example module that creates an extension type.",
                            true);

int
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{  
  El::Python::Use use;

  code_.compile(PYPROG, "TestProgram");

  try
  {
    { 
      El::Python::AllowOtherThreads allow_threads;

      thread_pool_ =
        new El::Service::ThreadPool(this, "ThreadPool", THREADS_COUNT);

      thread_pool_->start();

      for(unsigned long i = 0; i < THREADS_COUNT * 2; i++)
      {
        El::Service::ThreadPool::Task_var task = new PythonScript(this);
        thread_pool_->execute(task.in());  
      }

      thread_pool_->wait();
      thread_pool_ = 0;
    }

    code_.clear();
    
    std::cout << std::dec;
    simple_string_meter.dump(std::cout);
  }
  catch(...)
  {
    code_.clear();
    throw;
  }
  
  return 0;
}
