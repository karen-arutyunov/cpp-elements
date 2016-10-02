/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/FileSystem.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>
#include <sstream>

#include <El/Exception.hpp>

#include "FileSystem.hpp"

namespace El
{
  namespace FileSystem
  {
    //
    // DirectoryReader class
    //
    ACE_TSS<DirectoryReader::ThreadStorage> DirectoryReader::thread_storage_;

    int
    DirectoryReader::select_s(const struct dirent* dir)
    {
      int result = 0;

      try
      {
        if(thread_storage_->error.empty())
        {
          if(thread_storage_->reading == 0)
          {
            std::ostringstream ostr;
            ostr << "El::FileSystem::DirectoryReader::select_s: reading_ == 0 "
              "when called for " << dir;

            throw Exception(ostr.str());
          }
      
          result = thread_storage_->reading->select(dir) ? 1 : 0;
        }
      }
      catch(const El::Exception& e)
      {
        thread_storage_->error = e.what();
      }
      
      return result;
    }

#ifdef USE_SCANDIR_VOIDPTR
    int
    DirectoryReader::compare_s(const void* d1, const void* d2)
    {
      const struct dirent** dir1 = (const struct dirent**)d1;
      const struct dirent** dir2 = (const struct dirent**)d2;
#else
    int
    DirectoryReader::compare_s(
      const struct dirent** dir1, const struct dirent** dir2)
    {
#endif
      int result = 0;

      try
      {
        if(thread_storage_->error.empty())
        {
          if(thread_storage_->reading == 0)
          {
            std::ostringstream ostr;
            ostr << "El::FileSystem::DirectoryReader::compare_s: "
              "reading_ == 0 when called for " << dir1 << ", " << dir2;

            throw Exception(ostr.str());
          }
      
          result = thread_storage_->reading->compare(dir1, dir2);
        }
      }
      catch(const El::Exception& e)
      {
        thread_storage_->error = e.what();
      }
      
      return result;
    }
    
    int
    DirectoryReader::compare(const struct dirent** dir1,
                             const struct dirent** dir2)
      throw(El::Exception)
    {
      return alphasort(dir1, dir2);
    }
    
    void
    DirectoryReader::read(const char* name)
      throw(InvalidArg, Exception, El::Exception)
    {
      clear();

      current_directory_ = name;

      thread_storage_->reading = this;
      thread_storage_->error.clear();
      
      int result = ::scandir(name, &directories_, &select_s, &compare_s);

      thread_storage_->reading = 0;
      
      if(result == -1)
      {
        int error = ACE_OS::last_error();

        std::ostringstream ostr;
        ostr << "El::DirectoryReader::read: scandir failed for " << name
             << ". Errno " << error << ". Reason:\n"
             << ACE_OS::strerror(error);

        throw Exception(ostr.str());
      }

      count_ = result;

      if(!thread_storage_->error.empty())
      {
        throw Exception(thread_storage_->error);
      }
    }

    //
    // Urility functions
    //
    
    void
    create_directory(const char* path)
      throw(InvalidArg, Exception, El::Exception)
    {
      struct stat64 stat;
      memset(&stat, 0, sizeof(stat));
      
      if(stat64(path, &stat) == 0)
      {
        if(S_ISDIR(stat.st_mode))
        {
          return;
        }

        std::ostringstream ostr;
        ostr << "El::FileSystem::create_directory: '" << path
             << "' is not a directory";

        throw InvalidArg(ostr.str());
      }

      const char* ptr = path;

      do
      {
        ptr = strchr(ptr, '/');
        
        std::string subpath;

        if(ptr)
        {
          subpath.assign(path, ptr - path + 1);
          ptr++;
        }
        else
        {
          subpath = path;
        }

        memset(&stat, 0, sizeof(stat));
        if(stat64(subpath.c_str(), &stat) == 0 && S_ISDIR(stat.st_mode))
        {
          continue;
        }

        mkdir(subpath.c_str(), S_IRWXU);
        
        memset(&stat, 0, sizeof(stat));
        if(stat64(subpath.c_str(), &stat) != 0 || !S_ISDIR(stat.st_mode))
        {
          std::ostringstream ostr;
          ostr << "El::FileSystem::create_directory: failed to create '"
               << subpath << "'";
          
          throw InvalidArg(ostr.str());
        }
      }
      while(ptr);
    }

    void
    remove_directory(const char* path)
      throw(InvalidArg, Exception, El::Exception)
    {
      std::string prefix = std::string(path) + "/";
      
      DirectoryReader entries(path);

      for(size_t i = 0; i < entries.count(); i++)
      {
        const dirent& entry = entries[i];

        if(strcmp(entry.d_name, ".") && strcmp(entry.d_name, ".."))
        {
          std::string name = prefix + entry.d_name;

          if(entry.d_type == DT_DIR)
          {
            remove_directory(name.c_str());
          }
          else
          {
            unlink(name.c_str());
          }
        }
      }
      
      rmdir(path);
    }
  }
}
