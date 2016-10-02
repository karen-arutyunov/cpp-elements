/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/String/LightString.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STRING_LIGHTSTRING_HPP_
#define _ELEMENTS_EL_STRING_LIGHTSTRING_HPP_

#include <stdint.h>
#include <string.h>

#include <string>
#include <iostream>

//#include <ext/hash_fun.h>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>
#include <El/ArrayPtr.hpp>
#include <El/Hash/Hash.hpp>

namespace El
{
  namespace String
  {
    class LightString
    {
    public:
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
    public:
      LightString(char* str) throw(El::Exception);
      LightString(const char* str = "") throw(El::Exception);
      LightString(const std::string& str) throw(El::Exception);
      LightString(const LightString& str) throw(El::Exception);
      
      ~LightString() throw();

      const char* c_str() const throw(El::Exception);
      size_t length() const throw();

      bool empty() const throw();
      
      LightString& operator=(const char* str) throw(El::Exception);
      LightString& operator=(const std::string& str) throw(El::Exception);
      LightString& operator=(const LightString& str) throw(El::Exception);

      bool operator==(const LightString& str) const throw();
      bool operator!=(const LightString& str) const throw();

      bool operator==(const char* str) const throw();
      bool operator!=(const char* str) const throw();

      char* release() throw();
      void reset(char* str = 0) throw();

      void clear() throw();

      void write(El::BinaryOutStream& ostr) const
        throw(Exception, El::Exception);

      void read(El::BinaryInStream& istr) throw(Exception, El::Exception);
      
      static const LightString null;

    protected:
      void set(const char* str) throw(El::Exception);

    protected:
      El::ArrayPtr<char> str_;
    };
  }

  namespace Hash
  {
    struct LightString
    {
      size_t operator()(const El::String::LightString& str) const
        throw(El::Exception);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

inline
std::ostream&
operator<<(std::ostream& ostr, const El::String::LightString& str)
  throw(El::Exception)
{
  ostr << (str.c_str() ? str.c_str() : "<<null>>");
  return ostr;
}

inline
std::istream&
operator>>(std::istream& istr, El::String::LightString& str)
  throw(El::Exception)
{
  std::string tmp;
  istr >> tmp;
  str = tmp;
  return istr;
}

namespace El
{
  namespace String
  {
    inline
    LightString::LightString(char* str) throw(El::Exception) : str_(str)
    {
    }
    
    inline
    LightString::LightString(const char* str) throw(El::Exception)
    {
      set(str);
    }
    
    inline
    LightString::LightString(const std::string& str) throw(El::Exception)
    {
      set(str.c_str());
    }
    
    inline
    LightString::LightString(const LightString& str) throw(El::Exception)
    {
      set(str.c_str());
    }
    
    inline
    LightString::~LightString() throw()
    {
    }
    
    inline
    void
    LightString::set(const char* str) throw(El::Exception)
    {
      if(str != str_.get())
      {
        if(str)
        {
          str_.reset(new char[strlen(str) + 1]);
          strcpy(str_.get(), str);
        }
        else
        {
          str_.reset(0);
        }
      }
      
    }
    
    inline
    LightString&
    LightString::operator=(const char* str) throw(El::Exception)
    {
      set(str);
      return *this;
    }
    
    inline
    LightString&
    LightString::operator=(const std::string& str) throw(El::Exception)
    {
      set(str.c_str());
      return *this;
    }
    
    inline
    LightString&
    LightString::operator=(const LightString& str) throw(El::Exception)
    {
      set(str.c_str());
      return *this;
    }
    
    inline
    const char*
    LightString::c_str() const throw(El::Exception)
    {
      return str_.get();
    }

    inline
    char*
    LightString::release() throw()
    {
      return str_.release();
    }
    
    inline
    void
    LightString::reset(char* str) throw()
    {
      str_.reset(str);
    }
    
    inline
    void
    LightString::clear() throw()
    {
      str_.reset(new char[1]);
      str_[0] = '\0';
    }
    
    inline
    bool
    LightString::empty() const throw()
    {
      return str_.get() == 0 || str_[0] == '\0';
    }
    
    inline
    size_t
    LightString::length() const throw()
    {
      return str_.get() ? strlen(str_.get()) : 0;
    }

    inline
    bool
    LightString::operator==(const LightString& str) const throw()
    {
      return *this == str.str_.get();
    }

    inline
    bool
    LightString::operator!=(const LightString& str) const throw()
    {
      return !(*this == str);
    }
    
    inline
    bool
    LightString::operator==(const char* str) const throw()
    {
      if((str_.get() == 0) != (str == 0))
      {
        return false;
      }

      return str_.get() == 0 ? true : (strcmp(str_.get(), str) == 0);
    }

    inline
    bool
    LightString::operator!=(const char* str) const throw()
    {
      return !(*this == str);
    }

    inline
    void
    LightString::write(El::BinaryOutStream& ostr) const
      throw(Exception, El::Exception)
    {
      uint64_t len = c_str() ? length() : UINT64_MAX;
      ostr << len;
    
      if(len && len != UINT64_MAX)
      {
        ostr.write_raw_bytes((const unsigned char*)c_str(), len);
      }
    }

    inline
    void
    LightString::read(El::BinaryInStream& istr) throw(Exception, El::Exception)
    {
      uint64_t len = 0;

      if(istr.short_types())
      {
        uint32_t len32 = 0;
        istr >> len32;
    
        if(len32 == UINT32_MAX)
        {
          clear();
          return;
        }

        len = len32;
      }
      else
      {
        istr >> len;
    
        if(len == UINT64_MAX)
        {
          clear();
          return;
        }        
      }

      char* buff = new char[len + 1];

      try
      {
        istr.read_raw_bytes((unsigned char*)buff, len);
        buff[len] = '\0';
      }
      catch(...)
      {
        delete [] buff;
        throw;
      }

      reset(buff);
    }
  }

  namespace Hash
  {
    inline
    size_t
    LightString::operator()(const El::String::LightString& str) const
      throw(El::Exception)
    {
      return str.c_str() ? __gnu_cxx::__stl_hash_string(str.c_str()) : 0;
    }
    
  }
}

#endif // _ELEMENTS_EL_STRING_LIGHTSTRING_HPP_
