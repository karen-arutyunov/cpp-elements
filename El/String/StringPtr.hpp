/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/String/StringPtr.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STRING_STRINGPTR_HPP_
#define _ELEMENTS_EL_STRING_STRINGPTR_HPP_

#include <string.h>

#include <string>
#include <iostream>

//#include <ext/hash_fun.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>

namespace El
{
  namespace String
  {
    class StringConstPtr
    {
    public:
      StringConstPtr(const char* str = 0) throw(El::Exception);
      StringConstPtr(const std::string& str) throw(El::Exception);
      StringConstPtr(const StringConstPtr& str) throw(El::Exception);
      
      const char* c_str() const throw(El::Exception);
      size_t length() const throw();
      
      StringConstPtr& operator=(const char* str) throw(El::Exception);
      StringConstPtr& operator=(const std::string& str) throw(El::Exception);

      StringConstPtr& operator=(const StringConstPtr& str)
        throw(El::Exception);

      bool operator==(const StringConstPtr& str) const throw();
      bool operator<(const StringConstPtr& str) const throw();

      void reset(const char* str = 0) throw();
      void clear() throw();
      bool empty() const throw();

      static const StringConstPtr null;

    protected:
      const char* str_;
    };
  }
}

namespace El
{
  namespace Hash
  {
    struct StringConstPtr
    {
      size_t operator()(const El::String::StringConstPtr& str) const
        throw(El::Exception);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

inline
std::ostream&
operator<<(std::ostream& ostr, const El::String::StringConstPtr& str)
  throw(El::Exception)
{
  ostr << (str.c_str() ? str.c_str() : "");
  return ostr;
}

namespace El
{
  namespace String
  {
    inline
    StringConstPtr::StringConstPtr(const char* str) throw(El::Exception)
        : str_(str)
    {
    }
    
    inline
    StringConstPtr::StringConstPtr(const std::string& str) throw(El::Exception)
        : str_(str.c_str())
    {
    }
    
    inline
    StringConstPtr::StringConstPtr(const StringConstPtr& str)
      throw(El::Exception)
        : str_(str.c_str())
    {
    }
    
    inline
    StringConstPtr&
    StringConstPtr::operator=(const char* str) throw(El::Exception)
    {
      str_ = str;
      return *this;
    }
    
    inline
    StringConstPtr&
    StringConstPtr::operator=(const std::string& str)
      throw(El::Exception)
    {
      str_ = str.c_str();
      return *this;
    }
    
    inline
    StringConstPtr&
    StringConstPtr::operator=(const StringConstPtr& str) throw(El::Exception)
    {
      str_ = str.c_str();
      return *this;
    }
    
    inline
    const char*
    StringConstPtr::c_str() const throw(El::Exception)
    {
      return str_;
    }

    inline
    void
    StringConstPtr::reset(const char* str) throw()
    {
      str_ = str;
    }
    
    inline
    void
    StringConstPtr::clear() throw()
    {
      reset();
    }
    
    inline
    size_t
    StringConstPtr::length() const throw()
    {
      return str_ ? strlen(str_) : 0;
    }

    inline
    bool
    StringConstPtr::operator==(const StringConstPtr& str) const throw()
    {
      if((str_ == 0) != (str.str_ == 0))
      {
        return false;
      }

      return str_ == 0 ? true : (strcmp(str_, str.str_) == 0);
    }

    inline
    bool
    StringConstPtr::operator<(const StringConstPtr& str) const throw()
    {
      if((str_ == 0) != (str.str_ == 0))
      {
        return str_ == 0;
      }

      return str_ == 0 ? false : strcmp(str_, str.str_) < 0;
    }

    inline
    bool
    StringConstPtr::empty() const throw()
    {
      return str_ == 0 || *str_ == '\0';
    }
  }
}

namespace El
{
  namespace Hash
  {
    inline
    size_t
    StringConstPtr::operator()(const El::String::StringConstPtr& str) const
      throw(El::Exception)
    {
      return str.c_str() ? __gnu_cxx::__stl_hash_string(str.c_str()) : 0;
    }
    
  }
}

#endif // _ELEMENTS_EL_STRING_STRINGPTR_HPP_
