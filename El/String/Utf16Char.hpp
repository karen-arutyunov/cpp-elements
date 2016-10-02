/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Utf16Char.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_UTF16CHAR_HPP_
#define _ELEMENTS_EL_UTF16CHAR_HPP_

#include <stdint.h>

#include <string>
#include <iostream>

#include <El/Exception.hpp>

typedef uint16_t uchar_t;

namespace std
{
  template<>
  struct char_traits<uchar_t>
  {
    typedef uchar_t         char_type;
    typedef int 	    int_type;
    typedef std::streampos  pos_type;
    typedef std::streamoff  off_type;
    typedef std::mbstate_t  state_type;

    static void assign(char_type& __c1, const char_type& __c2);
    static bool eq(const char_type& __c1, const char_type& __c2);
    static bool lt(const char_type& __c1, const char_type& __c2);

    static int compare(const char_type* __s,
                       const char_type* __s2,
                       size_t __n);

    static size_t length(const char_type* __s);

    static const char_type* find(const char_type* __s,
                                 size_t __n,
                                 const char_type& __a);

    static char_type* move(char_type* __s1,
                           const char_type* __s2,
                           size_t __n);

    static char_type* copy(char_type* __s1,
                           const char_type* __s2,
                           size_t __n);
        
    static char_type* assign(char_type* __s, size_t __n, char_type __a);
    static char_type to_char_type(const int_type& __c);
    static int_type to_int_type(const char_type& __c);
    static bool eq_int_type(const int_type& __c1, const int_type& __c2);
    static int_type eof();
    static int_type not_eof(const int_type& __c);
  };

  typedef basic_ios<uchar_t> 		uios;
  typedef basic_streambuf<uchar_t> 	ustreambuf;
  typedef basic_istream<uchar_t> 	uistream;
  typedef basic_ostream<uchar_t> 	uostream;
  typedef basic_iostream<uchar_t> 	uiostream;
  typedef basic_stringbuf<uchar_t> 	ustringbuf;
  typedef basic_istringstream<uchar_t> 	uistringstream;
  typedef basic_ostringstream<uchar_t> 	uostringstream;
  typedef basic_stringstream<uchar_t> 	ustringstream;
  typedef basic_filebuf<uchar_t> 	ufilebuf;
  typedef basic_ifstream<uchar_t> 	uifstream;
  typedef basic_ofstream<uchar_t> 	uofstream;
  typedef basic_fstream<uchar_t> 	ufstream;
  
  typedef basic_string<uchar_t, char_traits<uchar_t> > ustring;
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace std
{
  inline
  void 
  char_traits<uchar_t>::assign(char_type& __c1, const char_type& __c2)
  {
    __c1 = __c2;
  }

  inline
  bool 
  char_traits<uchar_t>::eq(const char_type& __c1, const char_type& __c2)
  {
    return __c1 == __c2;
  }

  inline
  bool 
  char_traits<uchar_t>::lt(const char_type& __c1, const char_type& __c2)
  {
    return __c1 < __c2;
  }

  inline
  int 
  char_traits<uchar_t>::compare(const char_type* __s1,
                                const char_type* __s2,
                                size_t __n)
  {
    while(true)
    {
      if(lt(*__s1, *__s2))
      {
        return -1;
      }
      else if(!eq(*__s1, *__s2))
      {
        return 1;
      }
      else if(*__s1 == 0)
      {
        break;
      }
    }
        
    return 0;
  }

  inline
  size_t
  char_traits<uchar_t>::length(const char_type* __s)
  {
    size_t size = 0;        
    for(; *__s++ != 0; size++);
    return size;
  }

  inline
  const char_traits<uchar_t>::char_type* 
  char_traits<uchar_t>::find(const char_type* __s,
                             size_t __n,
                             const char_type& __a)
  {
    for (size_t i = 0; i < __n; i++)
    {
      if (__s[i] == __a)
      {
        return __s + i;
      }
    }
        
    return static_cast<const char_type*>(0);
  }

  inline
  char_traits<uchar_t>::char_type* 
  char_traits<uchar_t>::move(char_type* __s1,
                             const char_type* __s2,
                             size_t __n)
  {
    memmove(__s1, __s2, sizeof(char_type) * __n);
    return __s1;
  }

  inline
  char_traits<uchar_t>::char_type* 
  char_traits<uchar_t>::copy(char_type* __s1,
                             const char_type* __s2,
                             size_t __n)
  {
    memmove(__s1, __s2, sizeof(char_type) * __n);
    return __s1;
  }

  inline
  char_traits<uchar_t>::char_type* 
  char_traits<uchar_t>::assign(char_type* __s, size_t __n, char_type __a)
  {
    for (size_t i = 0; i < __n; i++)
    {
      __s[i] = __a;
    }
        
    return __s;
  }

  inline
  char_traits<uchar_t>::char_type 
  char_traits<uchar_t>::to_char_type(const int_type& __c)
  {
    return static_cast<char_type>(__c);
  }
      
  inline
  char_traits<uchar_t>::int_type 
  char_traits<uchar_t>::to_int_type(const char_type& __c)
  {
    return static_cast<int_type>(static_cast<unsigned int>(__c));
  }

  inline
  bool 
  char_traits<uchar_t>::eq_int_type(const int_type& __c1, const int_type& __c2)
  {
    return __c1 == __c2;
  }

  inline
  char_traits<uchar_t>::int_type 
  char_traits<uchar_t>::eof()
  {
    return static_cast<int_type>(EOF);
  }

  inline
  char_traits<uchar_t>::int_type 
  char_traits<uchar_t>::not_eof(const int_type& __c)
  {
    return (__c == eof()) ? 0 : __c;
  }      
}

#endif // _ELEMENTS_EL_UTF16CHAR_HPP_
