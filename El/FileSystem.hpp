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
 * @file Elements/El/FileSystem.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_FILESYSTEM_HPP_
#define _ELEMENTS_EL_FILESYSTEM_HPP_

//taken from here: http://stackoverflow.com/a/1424093
#define USE_SCANDIR_VOIDPTR
#if defined( __GLIBC_PREREQ  )
# if  __GLIBC_PREREQ(2,10)
#  undef USE_SCANDIR_VOIDPTR
# endif
#endif

#include <dirent.h>

#include <string>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>
#include <ace/TSS_T.h>

#include <El/Exception.hpp>

namespace El
{
  namespace FileSystem
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);

    class DirectoryReader
    {
    public:
      DirectoryReader(const char* name = 0)
        throw(InvalidArg, Exception, El::Exception);

      virtual ~DirectoryReader() throw();

      void read(const char* name) throw(InvalidArg, Exception, El::Exception);
      void clear() throw();

      size_t count() const throw();

      const dirent& operator[](size_t index) const
        throw(Exception, El::Exception);
      
      dirent& operator[](size_t index) throw(Exception, El::Exception);

    protected:
      static int select_s(const struct dirent* dir);

#ifdef USE_SCANDIR_VOIDPTR
      static int compare_s(const void* dir1, const void* dir2);
#else
      static int compare_s(const struct dirent** dir1,
                           const struct dirent** dir2);
#endif

      //
      // Overrides
      //
      virtual bool select(const struct dirent* dir) throw(El::Exception);

      virtual int compare(const struct dirent** dir1,
                          const struct dirent** dir2)
        throw(El::Exception);
      
    protected:
      dirent** directories_;
      size_t count_;
      std::string current_directory_;

      struct ThreadStorage
      {
        DirectoryReader* reading;
        std::string error;

        ThreadStorage() throw();
      };
      
      static ACE_TSS<ThreadStorage> thread_storage_;      
    };

    void create_directory(const char* path)
      throw(InvalidArg, Exception, El::Exception);

    void remove_directory(const char* path)
      throw(InvalidArg, Exception, El::Exception);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace FileSystem
  {
    //
    // DirectoryReader::ThreadStorages truct
    //
    inline
    DirectoryReader::ThreadStorage::ThreadStorage() throw()
        : reading(0)
    {
    }

    //
    // DirectoryReader class
    //
    inline
    DirectoryReader::DirectoryReader(const char* name)
      throw(InvalidArg, Exception, El::Exception)
        : directories_(0),
          count_(0)
    {
      if(name)
      {
        read(name);
      }
    }
    
    inline
    DirectoryReader::~DirectoryReader() throw()
    {
      clear();
    }

    inline
    size_t
    DirectoryReader::count() const throw()
    {
      return count_;
    }

    inline
    bool
    DirectoryReader::select(const struct dirent* dir) throw(El::Exception)
    {
      return true;
    }

    inline
    void
    DirectoryReader::clear() throw()
    {
      if(count_)
      {
        for(size_t i = 0; i < count_; i++)
        {
          free(directories_[i]);
        }

        free(directories_);  
        
        directories_ = 0;
        count_ = 0;
      }
    }

    inline
    const dirent&
    DirectoryReader::operator[](size_t index) const
      throw(Exception, El::Exception)
    {
      if(index >= count_)
      {
        throw Exception("El::FileSystem::DirectoryReader::operator[]: index "
                        "is out of range");
      }

      return *directories_[index];
    }
      
    inline
    dirent&
    DirectoryReader::operator[](size_t index)
      throw(Exception, El::Exception)
    {
      if(index >= count_)
      {
        throw Exception("El::FileSystem::DirectoryReader::operator[]: index "
                        "is out of range");
      }

      return *directories_[index];
    }
  }
}

#endif // _ELEMENTS_EL_FILESYSTEM_HPP_
