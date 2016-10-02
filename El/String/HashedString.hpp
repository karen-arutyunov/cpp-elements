/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/String/HashedString.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STRING_HASHEDSTRING_HPP_
#define _ELEMENTS_EL_STRING_HASHEDSTRING_HPP_

#include <string.h>

#include <string>
#include <iostream>

#include <El/Exception.hpp>
#include <El/CRC.hpp>

#include <El/String/LightString.hpp>

namespace El
{
  namespace String
  {
    class HashedString
    {
    public:
      HashedString(char* str) throw(El::Exception);
      HashedString(const char* str = 0) throw(El::Exception);
      HashedString(const std::string& str) throw(El::Exception);
      HashedString(const HashedString& str) throw(El::Exception);
      
      ~HashedString() throw();

      const char* c_str() const throw(El::Exception);
      size_t length() const throw();

      unsigned long hash() const throw();
      
      HashedString& operator=(const char* str) throw(El::Exception);
      HashedString& operator=(const std::string& str) throw(El::Exception);
      HashedString& operator=(const HashedString& str) throw(El::Exception);

      bool operator==(const HashedString& str) const throw();

      operator const LightString&() const throw();

      char* release() throw();
      void reset(char* str = 0) throw();
      void reset(char* str, unsigned long hash) throw();

      void clear() throw();

    protected:
      LightString lstr_;
      unsigned long hash_;
    };
  }
}

namespace El
{
  namespace Hash
  {
    struct HashedString
    {
      size_t operator()(const El::String::HashedString& str) const
        throw(El::Exception);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

inline
std::ostream&
operator<<(std::ostream& ostr, const El::String::HashedString& str)
  throw(El::Exception)
{
  ostr << (str.c_str() ? str.c_str() : "");
  return ostr;
}

inline
std::istream&
operator>>(std::istream& istr, El::String::HashedString& str)
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
    HashedString::HashedString(char* str) throw(El::Exception)
        : lstr_(str),
          hash_(El::CRC32_init())
    {
      El::CRC32(hash_, (const unsigned char*)c_str(), length());
    }
    
    inline
    HashedString::HashedString(const char* str) throw(El::Exception)
        : hash_(El::CRC32_init())
    {
      lstr_ = str;
      El::CRC32(hash_, (const unsigned char*)c_str(), length());
    }
    
    inline
    HashedString::HashedString(const std::string& str) throw(El::Exception)
        : hash_(El::CRC32_init())
    {
      lstr_ = str;
      El::CRC32(hash_, (const unsigned char*)c_str(), length());
    }
    
    inline
    HashedString::HashedString(const HashedString& str) throw(El::Exception)
        : lstr_(str.c_str()),
          hash_(str.hash_)
    {
    }
    
    inline
    HashedString::~HashedString() throw()
    {
    }
    
    inline
    unsigned long
    HashedString::hash() const throw()
    {
      return hash_;
    }
    
    inline
    HashedString::operator const LightString&() const throw()
    {
      return lstr_;
    }
    
    inline
    HashedString&
    HashedString::operator=(const char* str) throw(El::Exception)
    {
      lstr_ = str;
      
      hash_ = El::CRC32_init();
      El::CRC32(hash_, (const unsigned char*)c_str(), length());

      return *this;
    }
    
    inline
    HashedString&
    HashedString::operator=(const std::string& str)
      throw(El::Exception)
    {
      lstr_ = str;

      hash_ = El::CRC32_init();
      El::CRC32(hash_, (const unsigned char*)c_str(), length());

      return *this;
    }
    
    inline
    HashedString&
    HashedString::operator=(const HashedString& str) throw(El::Exception)
    {
      lstr_ = str.c_str();
      hash_ = str.hash_;

      return *this;
    }
    
    inline
    const char*
    HashedString::c_str() const throw(El::Exception)
    {
      return lstr_.c_str();
    }

    inline
    char*
    HashedString::release() throw()
    {
      hash_ = 0;
      return lstr_.release();
    }
    
    inline
    void
    HashedString::reset(char* str) throw()
    {
      if(str != lstr_.c_str())
      {
        lstr_.reset(str);

        hash_ = El::CRC32_init();
        El::CRC32(hash_, (const unsigned char*)c_str(), length());
      }
    }

    inline
    void
    HashedString::reset(char* str, unsigned long hash) throw()
    {
      lstr_.reset(str);
      hash_ = hash;
    }
    
    inline
    void
    HashedString::clear() throw()
    {
      reset();
    }
    
    inline
    size_t
    HashedString::length() const throw()
    {
      return lstr_.length();
    }

    inline
    bool
    HashedString::operator==(const HashedString& str) const throw()
    {
      return lstr_ == str.lstr_;
    }
  }
}

namespace El
{
  namespace Hash
  {
    inline
    size_t
    HashedString::operator()(const El::String::HashedString& str) const
      throw(El::Exception)
    {
      return str.hash();
    }
    
  }
}

#endif // _ELEMENTS_EL_STRING_HASHEDSTRING_HPP_
