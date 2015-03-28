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
 * @file Elements/El/Cache/ObjectCache.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/CRC.hpp>

#include "ObjectCache.hpp"

namespace El
{
  namespace Cache
  {
    Object*
    ObjectHolder::object() throw(El::Exception)
    {
      WriteGuard_ guard(lock_i());

      switch(state_)
      {
      case OS_NOT_FOUND:
        {
          throw NotFound(loading_error_);
        }
      case OS_LOADING_ERROR_OCCURED:
        {
          throw Exception(loading_error_);
        }
      case OS_LOADING:
        {
          throw Exception("ObjectHolder::object: unexpected state");
        }
      case OS_LOADED:
        {
          // Return the object

          accessed_ = ACE_OS::gettimeofday();
          break;
        }
      }
      
      return El::RefCount::add_ref(object_.in());  
    }

    Object*    
    ObjectHolder::load(const char* file_name,
                       Object* object,
                       ContainerGuard& container_guard)
      throw(NotFound, Exception, El::Exception)
    {
      WriteGuard_ guard(lock_i());
      container_guard.release();

      state_ = OS_LOADING;
      loading_error_.clear();
      reviewed_ = ACE_OS::gettimeofday();
        
      FILE* file = ::fopen64(file_name, "r");
      
      if(file == 0)
      {
        int error = ACE_OS::last_error();
        
        state_ = OS_NOT_FOUND;
          
        std::ostringstream ostr;
        ostr << "El::Cache::ObjectHolder::load: fopen64 failed for '"
             << file_name << "'.\nReason: " << ACE_OS::strerror(error);

        loading_error_ = ostr.str();
        throw NotFound(loading_error_);
      }

      struct stat64 file_stat;
      if(::stat64(file_name, &file_stat) == -1)
      {
        fclose(file);

        state_ = OS_NOT_FOUND;

        std::ostringstream ostr;
        ostr << "El::Cache::ObjectHolder::load: stat64 failed for '"
             << file_name << "'";

        loading_error_ = ostr.str();
        throw NotFound(loading_error_);
      }
        
      try
      {
        size_t read_bytes = 0;
        unsigned char buff[1024];

        unsigned long long hash = 0;
        object->reserve(file_stat.st_size);

        while((read_bytes = fread(buff, 1, sizeof(buff), file)) > 0)
        {
          object->read_chunk(buff, read_bytes);
          CRC(hash, buff, read_bytes);
        }

        object->hash(hash);

        int error = ferror(file);
          
        fclose(file);
        file = 0;
        
        if(error != 0)
        {
          std::ostringstream ostr;
          ostr << "El::Cache::ObjectHolder::load: fread failed for '"
               << file_name << "'. Error code " << error << ", reason: "
               << ACE_OS::strerror(error);

          throw Exception(ostr.str());
        }

        object->read_chunk(buff, 0);
      }
      catch(const El::Exception& e)
      {
        state_ = OS_LOADING_ERROR_OCCURED;

        std::ostringstream ostr;
        ostr << "El::Cache::ObjectHolder::load: El::Exception caught while "
          "reading '" << file_name << "'. Description:\n" << e.what();
        
        loading_error_ = ostr.str();
        
        if(file)
        {
          fclose(file);
        }
        
        throw;
      }
        
      state_ = OS_LOADED;      
      modified_ = file_stat.st_mtime;
      size_ = file_stat.st_size;
      reviewed_ = ACE_OS::gettimeofday();
      accessed_ = reviewed_;
      
      object_ = El::RefCount::add_ref(object);
      return  El::RefCount::add_ref(object_.in());
    }
    
  }
}
