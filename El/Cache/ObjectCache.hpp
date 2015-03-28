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
 * @file Elements/El/Cache/ObjectCache.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CACHE_OBJECTCACHE_HPP_
#define _ELEMENTS_EL_CACHE_OBJECTCACHE_HPP_

#include <stdio.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <sstream>

#include <ext/hash_map>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/RefCount/All.hpp>
#include <El/SyncPolicy.hpp>

namespace El
{
  namespace Cache
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    EL_EXCEPTION(NotFound, Exception);

    class Container
    {
    public:
      virtual ~Container() throw();
    };

    class Object : public virtual El::RefCount::Interface
    {
    public:
      //
      // Assigns a unique incarnation number for the object to
      // easily track object changes
      //
      Object(unsigned long long sequence_number) throw();
      virtual ~Object() throw();

      unsigned long long sequence_number() const throw();
      
      unsigned long long hash() const throw();
      void hash(unsigned long long val) throw();
      
      //
      // It is garanteed that read_chunk and reserve function will be
      // called in a thread-safe environment. Actually they will be called
      // as a part of object construction process which prevents an object
      // to be visible for outer world untill completed.
      //
      
      // If size == 0, then reading finished
      virtual void read_chunk(const unsigned char* buff, size_t size)
        throw(Exception, El::Exception) = 0;

      virtual void reserve(size_t size)
        throw(Exception, El::Exception) = 0;

      virtual bool is_modified() const throw(Exception, El::Exception);

    protected:      
      unsigned long long sequence_number_;
      unsigned long long hash_;
    };

    typedef El::RefCount::SmartPtr<Object> Object_var;
    
    class ObjectHolder :
      public El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
    {
    public:
      ObjectHolder() throw(El::Exception);
      ~ObjectHolder() throw();

      ACE_Time_Value reviewed() const throw();
      void reviewed(const ACE_Time_Value& tm) throw();

      Object* object() throw(El::Exception);

      bool is_modified(const char* filename) const
        throw(Exception, El::Exception);

      typedef ACE_Write_Guard<ACE_RW_Thread_Mutex> ContainerGuard;      
        
      ACE_Time_Value accessed() const throw();
      
      Object* load(const char* file_name,
                   Object* object,
                   ContainerGuard& container_guard)
        throw(NotFound, Exception, El::Exception);

    protected:

      enum ObjectState
      {
        OS_LOADING,
        OS_LOADED,
        OS_NOT_FOUND,
        OS_LOADING_ERROR_OCCURED
      };
      
      Object_var object_;
      ObjectState state_;
      ACE_Time_Value modified_;
      size_t size_;
      ACE_Time_Value reviewed_;
      ACE_Time_Value accessed_;
      std::string loading_error_;
    };
      
    typedef El::RefCount::SmartPtr<ObjectHolder> ObjectHolder_var;

    template<typename OBJECT>
    class FileCache : public virtual Container
    {
    public:
      FileCache(const ACE_Time_Value& review_filetime_period =
                ACE_Time_Value::zero,
                const ACE_Time_Value& object_timeout = ACE_Time_Value::zero)
        throw(Exception, El::Exception);

      virtual ~FileCache() throw();

      OBJECT* get(const char* file_name)
        throw(InvalidArg, NotFound, Exception, El::Exception);

      bool modified(const char* file_name, bool force_check = false)
        throw(InvalidArg, NotFound, Exception, El::Exception);

      void erase(const char* file_name) throw(El::Exception);
      void clear() throw(Exception, El::Exception);

    protected:
      OBJECT* downcast(Object* object) throw(Exception, El::Exception);

    protected:      
      typedef ACE_RW_Thread_Mutex Mutex;
      typedef ACE_Read_Guard<Mutex>  ReadGuard;
      typedef ACE_Write_Guard<Mutex> WriteGuard;

      mutable Mutex lock_;

      typedef __gnu_cxx::hash_map<std::string,
                                  ObjectHolder_var,
                                  El::Hash::String>
      ObjectMap;

      ObjectMap objects_;
      ACE_Time_Value review_filetime_period_;
      ACE_Time_Value object_timeout_;
      ACE_Time_Value object_timeout_last_check_;
      
      unsigned long long sequence_number_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Cache
  {
    //
    // Container class
    //
    inline
    Container::~Container() throw()
    {
    }
    
    //
    // Object class
    //
    inline
    Object::Object(unsigned long long sequence_number) throw()
        : sequence_number_(sequence_number),
          hash_(0)
    {
    }

    inline
    Object::~Object() throw()
    {
    }

    inline
    bool
    Object::is_modified() const throw(Exception, El::Exception)
    {
      return false;
    }
    
    inline
    unsigned long long
    Object::sequence_number() const throw()
    {
      return sequence_number_;
    }
    
    inline
    unsigned long long
    Object::hash() const throw()
    {
      return hash_;
    }

    inline
    void
    Object::hash(unsigned long long val) throw()
    {
      hash_ = val;
    }
    
    //
    // ObjectHolder class
    //
    inline
    ObjectHolder::ObjectHolder() throw(El::Exception)
        : state_(OS_LOADING),
          size_(0)
    {
    }

    inline
    ObjectHolder::~ObjectHolder() throw()
    {
    }
    
    inline
    ACE_Time_Value
    ObjectHolder::reviewed() const throw()
    {
      ReadGuard_ guard(lock_i());
      return reviewed_;
    }

    inline
    void
    ObjectHolder::reviewed(const ACE_Time_Value& tm) throw()
    {
      WriteGuard_ guard(lock_i());
      reviewed_ = tm;
    }

    inline
    ACE_Time_Value
    ObjectHolder::accessed() const throw()
    {
      ReadGuard_ guard(lock_i());
      return accessed_;
    }
    
    inline
    bool
    ObjectHolder::is_modified(const char* filename) const
      throw(Exception, El::Exception)
    {
      ReadGuard_ guard(lock_i());

      switch(state_)
      {
      case OS_NOT_FOUND:
      case OS_LOADING_ERROR_OCCURED:
        {
          return true;
        }
      case OS_LOADING:
        {
          throw Exception("ObjectHolder::is_modified: unexpected state");
        }
      case OS_LOADED:
        {
          // Continue processing
          break;
        }
      }
      
      struct stat64 file_stat;
      if(::stat64(filename, &file_stat) == -1)
      {
        return true;
      }

      return modified_ != ACE_Time_Value(file_stat.st_mtime) ||
        size_ != (size_t)file_stat.st_size;
    }
    
    //
    // FileCache class
    //
    template<typename OBJECT>
    FileCache<OBJECT>::FileCache(const ACE_Time_Value& review_filetime_period,
                                 const ACE_Time_Value& object_timeout)
      throw(Exception, El::Exception)
        : review_filetime_period_(review_filetime_period),
          object_timeout_(object_timeout),
          sequence_number_(1)
    {
    }

    template<typename OBJECT>
    FileCache<OBJECT>::~FileCache() throw()
    {
    }
    
    template<typename OBJECT>
    bool
    FileCache<OBJECT>::modified(const char* file_name, bool force_check)
      throw(InvalidArg, NotFound, Exception, El::Exception)
    { 
      if(file_name == 0 || *file_name == '\0')
      {
        throw InvalidArg(
          "El::Cache::FileCache::modified: file_name undefined");
      }

      ACE_Time_Value cur_time = ACE_OS::gettimeofday();
      
      ReadGuard guard(lock_);

      ObjectMap::const_iterator it = objects_.find(file_name);

      if(it == objects_.end())
      {
        return true;
      }
        
      El::Cache::ObjectHolder_var holder = it->second;
          
      if(!force_check && review_filetime_period_ != ACE_Time_Value::zero &&
         holder->reviewed() + review_filetime_period_ > cur_time)
      {
        return false;
      }

      if(!holder->is_modified(file_name))
      {
        guard.release();

        Object_var object = holder->object();

        if(!object->is_modified())
        {
          holder->reviewed(cur_time);  
          return false;
        }
      }
      
      return true;
    }
    
    template<typename OBJECT>
    void
    FileCache<OBJECT>::clear() throw(Exception, El::Exception)
    {
      WriteGuard guard(lock_);
      objects_.clear();
    }

    template<typename OBJECT>
    void
    FileCache<OBJECT>::erase(const char* file_name) throw(El::Exception)
    {
      if(file_name && *file_name != '\0')
      {
        WriteGuard guard(lock_);
        objects_.erase(file_name);
      }
    }
    
    template<typename OBJECT>
    OBJECT*
    FileCache<OBJECT>::get(const char* file_name)
      throw(InvalidArg, NotFound, Exception, El::Exception)
    {
      if(file_name == 0 || *file_name == '\0')
      {
        throw InvalidArg("El::Cache::FileCache::get: file_name undefined");
      }

      ACE_Time_Value cur_time = ACE_OS::gettimeofday();

      if(object_timeout_ != ACE_Time_Value::zero)
      {
        ReadGuard guard(lock_);

        if(object_timeout_last_check_ + object_timeout_ < cur_time)
        {
          guard.release();
          WriteGuard guard2(lock_);
          
          if(object_timeout_last_check_ + object_timeout_ < cur_time)
          {
            for(ObjectMap::iterator it = objects_.begin();
                it != objects_.end(); )
            {
              ObjectMap::iterator current = it++;
              
              if(current->second->accessed() + object_timeout_ < cur_time &&
                 current->first != file_name)
              {
                objects_.erase(current);
              }
            }

            object_timeout_last_check_ = cur_time;
          }
        }
      }
      
      {
        ReadGuard guard(lock_);

        ObjectMap::const_iterator it = objects_.find(file_name);

        if(it != objects_.end())
        {
          El::Cache::ObjectHolder_var holder = it->second;
          
          if(review_filetime_period_ != ACE_Time_Value::zero &&
             holder->reviewed() + review_filetime_period_ > cur_time)
          {
            guard.release();

            Object_var object = holder->object();
            return downcast(object.in());
          }

          if(!holder->is_modified(file_name))
          {
            holder->reviewed(cur_time);
              
            guard.release();
              
            Object_var object = holder->object();

            if(!object->is_modified())
            {
              return downcast(object.in());
            }
          }
        }
      }
      
      WriteGuard guard(lock_);

      ObjectMap::const_iterator it = objects_.find(file_name);

      if(it != objects_.end())
      {
        ObjectHolder_var holder = it->second;
        
        if(!holder->is_modified(file_name))
        {
          holder->reviewed(cur_time);
          guard.release();
          
          Object_var object = holder->object();

          if(!object->is_modified())
          {
            return downcast(object.in());
          }
        }
      }

      Object_var object = new OBJECT(this, sequence_number_++, file_name);
      ObjectHolder_var holder = new ObjectHolder();
      objects_[file_name] = holder;

      object = holder->load(file_name, object.in(), guard);
      return downcast(object.in());
    }

    template<typename OBJECT>
    OBJECT*
    FileCache<OBJECT>::downcast(Object* object) throw(Exception, El::Exception)
    {
      OBJECT* ob = dynamic_cast<OBJECT*>(object);

      if(ob == 0)
      {
        throw Exception(
          "FileCache<OBJECT>::downcast: dynamic_cast<OBJECT*> failed");
      }
      
      return El::RefCount::add_ref(ob);
    }
    
  }
}

#endif // _ELEMENTS_EL_CACHE_OBJECTCACHE_HPP_
