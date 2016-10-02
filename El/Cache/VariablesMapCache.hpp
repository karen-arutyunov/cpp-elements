/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Cache/VariablesMapCache.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CACHE_VARIABLESMAPCACHE_HPP_
#define _ELEMENTS_EL_CACHE_VARIABLESMAPCACHE_HPP_

#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <utility>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/String/Manip.hpp>
#include <El/String/Template.hpp>
#include <El/Lang.hpp>

#include <El/Cache/ObjectCache.hpp>
#include <El/Cache/TextFileCache.hpp>

namespace El
{
  namespace Cache
  {
    class VariablesMap : public TextFile,
                         public El::String::Template::VariablesMap
    {
    public:
      EL_EXCEPTION(VariableNotFound, Exception);
      
    public:
      VariablesMap(Container* container,
                   unsigned long long sequence_number,
                   const char* filename)
        throw(Exception, El::Exception);
        
      ~VariablesMap() throw();

      typedef El::String::Template::VariablesMap::iterator iterator;
      
      typedef El::String::Template::VariablesMap::const_iterator
      const_iterator;

      const std::string& get(const char* variable_id) const
        throw(VariableNotFound, El::Exception);

      virtual bool is_modified() const throw(Exception, El::Exception);
      
    protected:
      virtual void read_chunk(const unsigned char* buff, size_t size)
        throw(Exception, El::Exception);

      void load(std::istream& istr, const std::string& filename)
        throw(Exception, El::Exception);

    protected:

      struct FileStat
      {
        std::string path;
        ACE_Time_Value modified;
        size_t size;

        FileStat(const char* path_val) throw(El::Exception);
        FileStat(const FileStat& v) throw(El::Exception);

        bool is_modified() const throw();
      };

      typedef std::map<std::string, FileStat> FileStatMap;

      FileStatMap included_files_;
      std::string filename_;
    };

    typedef El::RefCount::SmartPtr<VariablesMap> VariablesMap_var;

    class VariablesMapCache : public virtual FileCache<VariablesMap>
    {
    public:
      VariablesMapCache(ACE_Time_Value review_filetime_period =
                        ACE_Time_Value::zero)
        throw(El::Exception);
      
      virtual ~VariablesMapCache() throw();

      VariablesMap* get(const char* file_name, const El::Lang& lang)
        throw(InvalidArg, NotFound, Exception, El::Exception);      

      VariablesMap* get(const char* file_name)
        throw(InvalidArg, NotFound, Exception, El::Exception);      
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
    // VariablesMap::FileStat struct
    //
    inline
    VariablesMap::FileStat::FileStat(const char* path_val)
      throw(El::Exception)
        : path(path_val)
    {
      struct stat64 file_stat;
      
      if(::stat64(path_val, &file_stat) != -1)
      {
        modified = ACE_Time_Value(file_stat.st_mtime);
        size = (size_t)file_stat.st_size;
      }
    }

    inline
    VariablesMap::FileStat::FileStat(const FileStat& v) throw(El::Exception)
        : path(v.path),
          modified(v.modified),
          size(v.size)
    {
    }

    inline
    bool
    VariablesMap::FileStat::is_modified() const throw()
    {
      struct stat64 file_stat;
      if(::stat64(path.c_str(), &file_stat) == -1)
      {
        return true;
      }

      return modified != ACE_Time_Value(file_stat.st_mtime) ||
        size != (size_t)file_stat.st_size;
    }
    
    //
    // VariablesMap class
    //
    inline
    VariablesMap::VariablesMap(Container* container,
                               unsigned long long sequence_number,
                               const char* filename)
      throw(Exception, El::Exception)
        : Object(sequence_number),
          BinaryFile(container, sequence_number, filename),
          TextFile(container, sequence_number, filename),
          filename_(filename)
    {
    }
        
    inline
    VariablesMap::~VariablesMap() throw()
    {
    }
    
    inline
    void
    VariablesMap::read_chunk(const unsigned char* buff, size_t size)
      throw(Exception, El::Exception)
    {
      TextFile::read_chunk(buff, size);

      if(!size)
      {
        std::istringstream istr(text());
        load(istr, filename_);        

        delete [] buff_;
        buff_ = 0;
      }
    }

    inline
    bool
    VariablesMap::is_modified() const throw(Exception, El::Exception)
    {
      for(FileStatMap::const_iterator i(included_files_.begin()),
            e(included_files_.end()); i != e; ++i)
      {
        if(i->second.is_modified())
        {
          return true;
        }
      }

      return false;
    }
    
    inline
    void
    VariablesMap::load(std::istream& istr, const std::string& filename)
      throw(Exception, El::Exception)
    {
      std::string line;        
      std::string key;
      std::string value;
        
      while(std::getline(istr, line))
      {
        if(key.empty())
        {
          size_t pos = strcspn(line.c_str(), " \t");
          size_t pos2 = strspn(line.c_str() + pos, " \t");

          key = line.substr(0, pos);
          value = line.substr(pos + pos2);

          if(key == "#include")
          {
            key.clear();
            
            std::string path;
            El::String::Manip::trim(value.c_str(), path);

            if(path[0] != '/')
            {
              size_t pos = filename.rfind('/');

              if(pos != std::string::npos)
              {
                path = filename.substr(0, pos + 1) + path;
              }
            }
              
            if(included_files_.insert(
                 std::make_pair(path, FileStat(path.c_str()))).second)
            {
              std::fstream file(path.c_str(), std::ios::in);
            
              if(!file.is_open())
              {
                std::ostringstream ostr;
                ostr << "El::Cache::VariablesMap::load: failed to open '"
                     << path << "' for read access while processing '"
                     << filename << "'";
                
                throw NotFound(ostr.str());
              }
              
              load(file, path);
            }
            
            continue;
          }

          if(line[0] == '#')
          {
            continue;
          }
            
          size_t len = value.length();
          bool bs_escaped = false;

          if(len > 0 && value[len - 1] == '\\' &&
             (len == 1 || (bs_escaped = value[len - 2]) != '\\'))
          {
            value = value.substr(0, len - 1) + "\n";
          }
          else if(!key.empty())
          {
            (*this)[key] = bs_escaped ? value.substr(0, len - 1) : value;
            
//            insert(
//              std::make_pair(key,
//                             bs_escaped ? value.substr(0, len - 1) : value));
            
            key.clear();
          } 
        }
        else
        {
          size_t len = line.length();

          if(len > 0 && line[len - 1] == '\\' &&
             (len == 1 || line[len - 2] != '\\'))
          {
            value += line.substr(0, len - 1) + "\n";
          }
          else
          {
            value += line;
            (*this)[key] = value;
            
//            insert(std::make_pair(key, value));
              
            key.clear();
          }
        }
      }
    }
    
    inline
    const std::string&
    VariablesMap::get(const char* variable_id) const
      throw(VariableNotFound, El::Exception)
    {
      const_iterator it = find(variable_id);

      if(it == end())
      {
        std::ostringstream ostr;
        ostr << "El::Cache::VariablesMap::get: template for " << variable_id
             << " can't be found in file " << filename_;

        throw VariableNotFound(ostr.str());
      }

      return it->second;
    }
    
    //
    // VariablesMapCache class
    //
    inline
    VariablesMapCache::VariablesMapCache(ACE_Time_Value review_filetime_period)
      throw(El::Exception)
        : FileCache<VariablesMap>(review_filetime_period)
    {
    }

    inline
    VariablesMapCache::~VariablesMapCache() throw()
    {
    }
    
    inline
    VariablesMap*
    VariablesMapCache::get(const char* file_name, const El::Lang& lang)
      throw(InvalidArg, NotFound, Exception, El::Exception)
    {
      std::string loc_file = std::string(file_name) + "." + lang.l3_code();
      return FileCache<VariablesMap>::get(loc_file.c_str());
    }

    inline
    VariablesMap*
    VariablesMapCache::get(const char* file_name)
      throw(InvalidArg, NotFound, Exception, El::Exception)
    {
      return FileCache<VariablesMap>::get(file_name);
    }
  }
}

#endif // _ELEMENTS_EL_CACHE_VARIABLESMAPCACHE_HPP_
