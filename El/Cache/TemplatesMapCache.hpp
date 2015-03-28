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
 * @file Elements/El/Cache/TemplatesMapCache.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CACHE_TEMPLATESMAPCACHE_HPP_
#define _ELEMENTS_EL_CACHE_TEMPLATESMAPCACHE_HPP_

#include <string>
#include <sstream>
#include <utility>

#include <ext/hash_map>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/String/Template.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Lang.hpp>

#include <El/Cache/ObjectCache.hpp>
#include <El/Cache/TextFileCache.hpp>

namespace El
{
  namespace Cache
  {
    class TemplatesMapCache;

    typedef __gnu_cxx::hash_map<std::string,
                                El::String::Template::Parser,
                                El::Hash::String> TemplatesMapBase;
    
    class TemplatesMap : public TextFile,
                         public TemplatesMapBase
    {
    public:
      EL_EXCEPTION(TemplateNotFound, Exception);
      
    public:
      TemplatesMap(Container* container,
                   unsigned long long sequence_number,
                   const char* filename)
        throw(Exception, El::Exception);
        
      ~TemplatesMap() throw();

      typedef TemplatesMapBase::iterator iterator;
      typedef TemplatesMapBase::const_iterator const_iterator;

      const El::String::Template::Parser& get(const char* tempate_id) const
        throw(TemplateNotFound, El::Exception);

    protected:
      virtual void read_chunk(const unsigned char* buff, size_t size)
        throw(Exception, El::Exception);

    protected:
      TemplatesMapCache* container_;
      std::string filename_;
    };

    typedef El::RefCount::SmartPtr<TemplatesMap> TemplatesMap_var;

    class TemplatesMapCache : public virtual FileCache<TemplatesMap>
    {
    public:
      TemplatesMapCache(
        const char* var_left_marker,
        const char* var_right_marker,
        ACE_Time_Value review_filetime_period = ACE_Time_Value::zero,
        const El::String::Template::ParseInterceptor* interceptor = 0)
        throw(El::Exception);
      
      virtual ~TemplatesMapCache() throw();

      const char* var_left_marker() const throw(El::Exception);
      const char* var_right_marker() const throw(El::Exception);
      
      const El::String::Template::ParseInterceptor* parse_interceptor() const
        throw();

      TemplatesMap* get(const char* file_name, const El::Lang& lang)
        throw(InvalidArg, NotFound, Exception, El::Exception);      

      TemplatesMap* get(const char* file_name)
        throw(InvalidArg, NotFound, Exception, El::Exception);      

    protected:
      std::string var_left_marker_;
      std::string var_right_marker_;
      const El::String::Template::ParseInterceptor* interceptor_;
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
    // TemplatesMap class
    //
    inline
    TemplatesMap::TemplatesMap(Container* container,
                               unsigned long long sequence_number,
                               const char* filename)
      throw(Exception, El::Exception)
        : Object(sequence_number),
          BinaryFile(container, sequence_number, filename),
          TextFile(container, sequence_number, filename),
          container_(dynamic_cast<TemplatesMapCache*>(container)),
          filename_(filename)
    {
    }
        
    inline
    TemplatesMap::~TemplatesMap() throw()
    {
    }
    
    inline
    void
    TemplatesMap::read_chunk(const unsigned char* buff, size_t size)
      throw(Exception, El::Exception)
    {
      TextFile::read_chunk(buff, size);

      if(!size)
      {
        std::istringstream istr(text());
        std::string line;
        
        while(std::getline(istr, line))
        {
          std::string trimmed;
          El::String::Manip::trim(line.c_str(), trimmed);

          if(trimmed.empty() || trimmed[0] == '#')
          {
            continue;
          }

          size_t pos = strcspn(trimmed.c_str(), " \t");
          size_t pos2 = strspn(trimmed.c_str() + pos, " \t");

          iterator it =
            insert(std::make_pair(trimmed.substr(0, pos),
                                  El::String::Template::Parser())).first;

          it->second.parse(trimmed.substr(pos + pos2).c_str(),
                           container_->var_left_marker(),
                           container_->var_right_marker(),
                           container_->parse_interceptor());          
        }

        delete [] buff_;
        buff_ = 0;
      }
    }

    inline
    const El::String::Template::Parser&
    TemplatesMap::get(const char* tempate_id) const
      throw(TemplateNotFound, El::Exception)
    {
      El::Cache::TemplatesMap::const_iterator it = find(tempate_id);

      if(it == end())
      {
        std::ostringstream ostr;
        ostr << "El::Cache::TemplatesMap::get: template for " << tempate_id
             << " can't be found in file " << filename_;

        throw TemplateNotFound(ostr.str());
      }

      return it->second;
    }
    
    //
    // TemplatesMapCache class
    //
    inline
    TemplatesMapCache::TemplatesMapCache(
      const char* var_left_marker,
      const char* var_right_marker,
      ACE_Time_Value review_filetime_period,
      const El::String::Template::ParseInterceptor* interceptor)
      throw(El::Exception)
        : FileCache<TemplatesMap>(review_filetime_period),
          var_left_marker_(var_left_marker),
          var_right_marker_(var_right_marker),
          interceptor_(interceptor)
    {
    }

    inline
    TemplatesMapCache::~TemplatesMapCache() throw()
    {
    }
    
    inline
    const char*
    TemplatesMapCache::var_left_marker() const throw(El::Exception)
    {
      return var_left_marker_.c_str();
    }

    inline
    const char*
    TemplatesMapCache::var_right_marker() const throw(El::Exception)
    {
      return var_right_marker_.c_str();
    }
    
    inline
    const El::String::Template::ParseInterceptor*
    TemplatesMapCache::parse_interceptor() const throw()
    {
      return interceptor_;
    }
    
    inline
    TemplatesMap*
    TemplatesMapCache::get(const char* file_name, const El::Lang& lang)
      throw(InvalidArg, NotFound, Exception, El::Exception)
    {
      std::string loc_file = std::string(file_name) + "." + lang.l3_code();
      return FileCache<TemplatesMap>::get(loc_file.c_str());
    }

    inline
    TemplatesMap*
    TemplatesMapCache::get(const char* file_name)
      throw(InvalidArg, NotFound, Exception, El::Exception)
    {
      return FileCache<TemplatesMap>::get(file_name);
    }
  }
}

#endif // _ELEMENTS_EL_CACHE_TEMPLATESMAPCACHE_HPP_
