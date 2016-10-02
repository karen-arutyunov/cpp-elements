/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/NameValueMap.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NAMEVALUEMAP_HPP_
#define _ELEMENTS_EL_NAMEVALUEMAP_HPP_

#include <string>
#include <sstream>

#include <ext/hash_map>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/String/ListParser.hpp>
#include <El/String/Manip.hpp>

namespace El
{
  class NameValueMap : public __gnu_cxx::hash_map<std::string,
                                                  std::string,
                                                  El::Hash::String>
  {
  public:
    NameValueMap(const char* text = 0,
                 char nvp_separator = ',',
                 char nv_separator = '=') throw(El::Exception);
    
    void load(const char* text) throw(El::Exception);
    std::string string() throw(El::Exception);

    bool present(const char* name) const throw(El::Exception);
    const char* value(const char* name) const throw(El::Exception);

  protected:
    char nvp_separator_;
    char nv_separator_;
  };
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  inline
  NameValueMap::NameValueMap(const char* text,
                             char nvp_separator,
                             char nv_separator) throw(El::Exception)
      : nvp_separator_(nvp_separator),
        nv_separator_(nv_separator)
  {
    load(text);
  }
  
  inline
  bool
  NameValueMap::present(const char* name) const throw(El::Exception)
  {
    return find(name) != end();
  }

  inline
  const char*
  NameValueMap::value(const char* name) const throw(El::Exception)
  {
    const_iterator it = find(name);
    return it == end() ? "" : it->second.c_str();
  }
  
  inline
  void
  NameValueMap::load(const char* text) throw(El::Exception)
  {
    clear();

    char sep[2];
    *sep = nvp_separator_;
    sep[1] = '\0';

    El::String::ListParser nvp_list(text, sep);
    
    const char* nvp = 0;
    while((nvp = nvp_list.next_item()) != 0)
    {
      std::string name;
      std::string value;
      
      const char* eq = strchr(nvp, nv_separator_);

      if(eq == 0)
      {
        El::String::Manip::trim(nvp, name);
      }
      else
      {
        El::String::Manip::trim(nvp, name, eq - nvp);
        El::String::Manip::trim(eq + 1, value);
      }

      (*this)[name] = value;
    }
  }

  inline
  std::string
  NameValueMap::string() throw(El::Exception)
  {
    std::stringstream ostr;

    for(const_iterator it = begin(); it != end(); it++)
    {
      if(it != begin())
      {
        ostr << nvp_separator_;
      }
      
      ostr << it->first << nv_separator_ << it->second;
    }    

    return ostr.str();
  }
  
}

#endif // _ELEMENTS_EL_NAMEVALUEMAP_HPP_
