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
 * @file Elements/El/PSP/Config.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>

#include <string>
#include <sstream>

#include <El/String/Manip.hpp>

#include "Config.hpp"

namespace El
{
  namespace PSP
  {
    Config::Type Config::Type::instance;

    PyObject*
    Config::get(const char* key) throw(Exception, El::Exception)
    {
      El::Python::Map_var owner = find_owner(key);

      if(owner)
      {
        El::Python::Object_var ob_key = short_key(key);
        El::Python::Map::iterator it = owner->find(ob_key);

        return it == owner->end() ? 0 : it->second.add_ref();
      }

      return 0;
    }
    
    void
    Config::set(const char* key, PyObject* val)
      throw(Exception, El::Exception)
    {
      if(val == 0)
      {
        El::Python::Map_var owner = find_owner(key);

        if(owner.in() != 0)
        {
          El::Python::Object_var ob_key = short_key(key);
          owner->erase(ob_key);
        }

        return;
      }
      
      El::Python::Map_var owner = find_owner(key, true);
      El::Python::Object_var ob_key = short_key(key);

      El::Python::Object_var value = El::Python::add_ref(val);
      (*owner)[ob_key] = value;
    }

    Config*
    Config::find_owner(const char* key, bool create)
      throw(Exception, El::Exception)
    {
      if(key == 0)
      {
        return 0;
      }
         
      const char* next_key = strchr(key, '.');

      if(next_key == 0)
      {
        return add_ref(this);
      }

      El::Python::Object_var ob_key =
        short_key(std::string(key, next_key - key).c_str());

      El::Python::Map::iterator it = find(ob_key);
      
      if(it == end())
      {
        if(create)
        {
          Config_var conf = new Config();
          (*this)[ob_key] = conf;

          return conf->find_owner(next_key + 1, true);
        }
        else
        {
          return 0;
        }
      }

      Config* conf = Config::Type::down_cast(it->second.in());
      return conf->find_owner(next_key + 1, create);
    }

    PyObject*
    Config::short_key(const char* key) throw(El::Exception)
    {
      const char* last_key = strrchr(key, '.');

      if(last_key == 0)
      {
        last_key = key;
      }
      else
      {
        last_key++;
      }
      
      return PyString_FromString(last_key);
    }

    PyObject*
    Config::py_get(PyObject* args) throw(El::Exception)
    {
      char* key = 0;
      PyObject* default_val = 0;
      
      if(!PyArg_ParseTuple(args,
                           "s|O:el.psp.Config.get",
                           &key,
                           &default_val))
      {
        El::Python::handle_error("El::PSP::Config::py_get");
      }

      if(key == 0 || *key == '\0')
      {
        El::Python::report_error(
          PyExc_TypeError,
          "Non-empty string expected as a parameter for "
          "el.psp.Config.get",
          "El::PSP::Config::py_get");
      }

      PyObject* res = Config::get(key);
      return res ? res :
        El::Python::add_ref(default_val ? default_val : Py_None);
    }

    std::string
    Config::string(const char* key) throw(Exception, El::Exception)
    {
      El::Python::Object_var ob = get(key);

      if(ob.in() == 0)
      {
        std::ostringstream ostr;
        ostr << "El::PSP::Config::string: value for key '" << key
             << "' not exist";

        throw Exception(ostr.str().c_str());
      }
    
      std::ostringstream ostr;
      ostr << "El::PSP::Config::string('" << key << "')";

      size_t slen = 0;
      const char* value =
        El::Python::string_from_string(ob.in(), slen, ostr.str().c_str());
    
      return value;
    }

    long long
    Config::number(const char* key) throw(Exception, El::Exception)
    {
      El::Python::Object_var ob = get(key);

      if(ob.in() == 0)
      {
        std::ostringstream ostr;
        ostr << "El::PSP::Config::number: value for key '" << key
             << "' not exist";

        throw Exception(ostr.str().c_str());
      }
    
      std::ostringstream ostr;
      ostr << "El::PSP::Config::number('" << key << "')";

      size_t slen = 0;
      const char* value =
        El::Python::string_from_string(ob.in(), slen, ostr.str().c_str());

      long long result = 0;
      
      if(!El::String::Manip::numeric(value, result))
      {
        std::ostringstream ostr;
        ostr << "El::PSP::Config::number: value for key '" << key
             << "' not a number";

        throw Exception(ostr.str().c_str());        
      }

      return result;
    }
    
    double
    Config::double_number(const char* key) throw(Exception, El::Exception)
    {
      El::Python::Object_var ob = get(key);

      if(ob.in() == 0)
      {
        std::ostringstream ostr;
        ostr << "El::PSP::Config::double_number: value for key '" << key
             << "' not exist";

        throw Exception(ostr.str().c_str());
      }
    
      std::ostringstream ostr;
      ostr << "El::PSP::Config::double_number('" << key << "')";

      size_t slen = 0;
      const char* value =
        El::Python::string_from_string(ob.in(), slen, ostr.str().c_str());

      double result = 0;
      
      if(!El::String::Manip::numeric(value, result))
      {
        std::ostringstream ostr;
        ostr << "El::PSP::Config::double_number: value for key '" << key
             << "' not a number";

        throw Exception(ostr.str().c_str());        
      }

      return result;
    }
    
    El::PSP::Config*
    Config::config(const char* key) throw(Exception, El::Exception)
    {
      El::Python::Object_var ob = get(key);

      if(ob.in() == 0)
      {
        std::ostringstream ostr;
        ostr << "El::PSP::Config::config: value for key '" << key
             << "' not exist";

        throw Exception(ostr.str());
      }
    
      if(!El::PSP::Config::Type::check_type(ob.in()))
      {
        std::ostringstream ostr;
        ostr << "El::PSP::Config::config: value for key '" << key
             << "' is not of El::PSP::Config type";

        throw Exception(ostr.str());
      }

      El::PSP::Config* conf = static_cast<El::PSP::Config*>(ob.in());
      ob.add_ref();
      
      return conf;
    }
    
  }
}
