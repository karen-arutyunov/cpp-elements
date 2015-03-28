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
 * @file Elements/El/BinaryStream.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_BINARYSTREAM_HPP_
#define _ELEMENTS_EL_BINARYSTREAM_HPP_

#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <wchar.h>

#include <iostream>
#include <sstream>
#include <list>

#include <ace/OS.h>

template <bool>
struct st_assert;

template<>
struct st_assert<true>
{
 typedef int type;
};

template<>
struct st_assert<false>
{
};

typedef st_assert<sizeof(char) == 1>::type dumy1;
typedef st_assert<sizeof(wchar_t) == 4>::type dumy2;
typedef st_assert<sizeof(float) == 4>::type dumy3;
typedef st_assert<sizeof(double) == 8>::type dumy4;
  
#include <El/Exception.hpp>

namespace El
{
  //
  // Currently assumed that machine writing into the stream ans machine reading
  // from the stream are of the same architecture. This restriction can easily
  // be overriden when required by inventing proper stream data header.
  //
  
  class BinaryOutStream
  {
  public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);

    BinaryOutStream(std::ostream& backend) throw(El::Exception);
    virtual ~BinaryOutStream() throw();

    void write_int8(int8_t val) throw(Exception, El::Exception);
    void write_uint8(uint8_t val) throw(Exception, El::Exception);

    void write_int16(int16_t val) throw(Exception, El::Exception);
    void write_uint16(uint16_t val) throw(Exception, El::Exception);    

    void write_int32(int32_t val) throw(Exception, El::Exception);
    void write_uint32(uint32_t val) throw(Exception, El::Exception);

    void write_int64(const int64_t& val) throw(Exception, El::Exception);
    void write_uint64(const uint64_t& val) throw(Exception, El::Exception);
    
    void write_float(float val) throw(Exception, El::Exception);
    void write_double(double val) throw(Exception, El::Exception);

    void write_string(const char* val) throw(Exception, El::Exception);
    void write_string(const std::string& val) throw(Exception, El::Exception);
    void write_string_buff(const char* val) throw(Exception, El::Exception);

    void write_wstring(const wchar_t* val) throw(Exception, El::Exception);
    
    void write_wstring(const std::wstring& val)
      throw(Exception, El::Exception);

    void write_bytes(const unsigned char* val, size_t length)
      throw(Exception, El::Exception);

    void write_raw_bytes(const unsigned char* val, size_t length)
      throw(Exception, El::Exception);

    size_t written_bytes() const throw();

    //
    // Overloaded write methods
    //
    
    template<typename MAP>
    void write_map(const MAP& map) throw(Exception, El::Exception);
    
    template<typename SET>
    void write_set(const SET& set) throw(Exception, El::Exception);
    
    template<typename SET, typename SERIALIZER>
    void write_set(const SET& set, const SERIALIZER& serializer)
      throw(Exception, El::Exception);
    
    template<typename T>
    void write_list(const std::list<T>& val) throw(Exception, El::Exception);
    
    template<typename C>
    void write_container(const C& val) throw(Exception, El::Exception);
    
    template<typename C, typename SERIALIZER>
    void write_container(const C& val, const SERIALIZER& serializer)
      throw(Exception, El::Exception);
    
    template<typename A>
    void write_array(const A& arr) throw(Exception, El::Exception);
    
  protected:
    std::ostream& backend_;
    size_t written_bytes_;
  };

  class BinaryInStream
  {
  public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);

    BinaryInStream(std::istream& backend) throw(El::Exception);
    virtual ~BinaryInStream() throw();

    void seek(size_t offset, std::ios_base::seekdir dir)
      throw(El::Exception);

    bool short_types() const throw() { return short_types_; }
    void short_types(bool val) throw() { short_types_ = val; }
    
    void read_int8(int8_t& val) throw(Exception, El::Exception);
    void read_uint8(uint8_t& val) throw(Exception, El::Exception);

    void read_int16(int16_t& val) throw(Exception, El::Exception);
    void read_uint16(uint16_t& val) throw(Exception, El::Exception);  

    void read_int32(int32_t& val) throw(Exception, El::Exception);
    void read_uint32(uint32_t& val) throw(Exception, El::Exception);

    void read_int64(int64_t& val) throw(Exception, El::Exception);
    void read_uint64(uint64_t& val) throw(Exception, El::Exception);
    
    void read_float(float& val) throw(Exception, El::Exception);
    void read_double(double& val) throw(Exception, El::Exception);

    void read_string(char*& val) throw(Exception, El::Exception);
    void read_string(std::string& val) throw(Exception, El::Exception);
    
    void read_string_buff(char* val, size_t max_len)
      throw(Exception, El::Exception);

    void read_wstring(wchar_t*& val) throw(Exception, El::Exception);
    
    void read_wstring(std::wstring& val)
      throw(Exception, El::Exception);

    void read_bytes(unsigned char*& val, size_t& length)
      throw(Exception, El::Exception);

    void read_raw_bytes(unsigned char* val, size_t length)
      throw(Exception, El::Exception);

    size_t read_bytes() throw();

    template<typename C>
    void read_container(C& val) throw(Exception, El::Exception);

    template<typename SET, typename SERIALIZER>
    void read_container(SET& set, const SERIALIZER& serializer)
      throw(Exception, El::Exception);    

    template<typename A>
    void read_array(A& arr) throw(Exception, El::Exception);
    
    //
    // Overloaded read methods
    //

    template<typename MAP>
    void read_map(MAP& map) throw(Exception, El::Exception);
    
    template<typename SET>
    void read_set(SET& set) throw(Exception, El::Exception);

    template<typename SET, typename SERIALIZER>
    void read_set(SET& set, const SERIALIZER& serializer)
      throw(Exception, El::Exception);    
    
    template<typename T>
    void read_list(std::list<T>& val) throw(Exception, El::Exception);

  protected:
    std::istream& backend_;
    size_t read_bytes_;
    bool short_types_;
  };

  template<typename TYPE>
  class SerializableSet : public TYPE
  {
  public:
    void write(El::BinaryOutStream& bstr) const throw(El::Exception);
    void read(El::BinaryInStream& bstr) throw(El::Exception);
  };
  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

//
// El::BinaryOutStream& operator<<
//

template<typename TYPE>
inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, const TYPE& val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  val.write(ostr);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, int8_t val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_int8(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, uint8_t val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_uint8(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, int16_t val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_int16(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, uint16_t val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_uint16(val);
  return ostr;
}
          
inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, float val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_float(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, double val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_double(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, const int64_t& val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_int64(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, const uint64_t& val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_uint64(val);
  return ostr;
}
    
inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, const char* val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_string(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, const std::string& val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_string(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, const wchar_t* val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_wstring(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, const std::wstring& val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_wstring(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, int32_t val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_int32(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, uint32_t val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_uint32(val);
  return ostr;
}

inline
El::BinaryOutStream&
operator<<(El::BinaryOutStream& ostr, const ACE_Time_Value& val)
  throw(El::BinaryOutStream::Exception, El::Exception)
{
  ostr.write_uint64(val.sec());
  ostr.write_uint64(val.usec());
  return ostr;
}

//
// El::BinaryInStream& operator>>
//

template<typename TYPE>
inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, TYPE& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  val.read(istr);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, int8_t& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_int8(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, uint8_t& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_uint8(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, int16_t& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_int16(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, uint16_t& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_uint16(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, float& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_float(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, double& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_double(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, int64_t& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_int64(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, uint64_t& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_uint64(val);
  return istr;
}
    
inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, char*& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_string(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, std::string& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_string(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, wchar_t*& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_wstring(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, std::wstring& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_wstring(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, int32_t& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_int32(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, uint32_t& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  istr.read_uint32(val);
  return istr;
}

inline
El::BinaryInStream&
operator>>(El::BinaryInStream& istr, ACE_Time_Value& val)
  throw(El::BinaryInStream::Exception, El::Exception)
{
  uint64_t sec = 0;
  uint64_t usec = 0;
  
  istr.read_uint64(sec);
  istr.read_uint64(usec);
  
  val = ACE_Time_Value(sec, usec);
  return istr;
}

namespace El
{
  //
  // BinaryOutStream class
  //
  inline
  BinaryOutStream::BinaryOutStream(std::ostream& backend) throw(El::Exception)
      : backend_(backend),
        written_bytes_(0)
  {
  }
  
  inline
  BinaryOutStream::~BinaryOutStream() throw()
  {
  }

  inline
  void
  BinaryOutStream::write_int8(int8_t val) throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_int8 failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_int16(int16_t val) throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_int16 failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_uint16(uint16_t val)
    throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_uint16 failed");
    }
  }

  inline
  void
  BinaryOutStream::write_int32(int32_t val) throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_int32 failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_uint32(uint32_t val)
    throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_uint32 failed");
    }
  }

  inline
  void
  BinaryOutStream::write_float(float val)
    throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_float failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_double(double val)
    throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_double failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_int64(const int64_t& val)
    throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_int64 failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_uint64(const uint64_t& val)
    throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_uint64 failed");
    }
  }

  inline
  void
  BinaryOutStream::write_uint8(uint8_t val) throw(Exception, El::Exception)
  {
    backend_.write((const char*)&val, sizeof(val));
    written_bytes_ += sizeof(val);

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_uint8 failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_bytes(const unsigned char* val, size_t length)
    throw(Exception, El::Exception)
  {
    uint64_t len = val ? length : UINT64_MAX;
    
    *this << len;
    
    if(len && len != UINT64_MAX)
    {
      backend_.write((const char*)val, len);
      written_bytes_ += len;
    }

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_bytes failed");
    }
  }
    
  inline
  void
  BinaryOutStream::write_raw_bytes(const unsigned char* val, size_t length)
    throw(Exception, El::Exception)
  {
    backend_.write((const char*)val, length);
    written_bytes_ += length;

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_raw_bytes failed");
    }
  }
    
  template<typename MAP>
  void
  BinaryOutStream::write_map(const MAP& map)
    throw(Exception, El::Exception)
  {
    *this << (uint64_t)map.size();

    for(typename MAP::const_iterator it = map.begin(); it != map.end(); it++)
    {
      *this << it->first << it->second;
    }
  }

  template<typename SET>
  void
  BinaryOutStream::write_set(const SET& set)
    throw(Exception, El::Exception)
  {
    write_container(set);
  }

  template<typename SET, typename SERIALIZER>
  void
  BinaryOutStream::write_set(const SET& set, const SERIALIZER& serializer)
    throw(Exception, El::Exception)
  {
    write_container(set, serializer);
  }
  
  template<typename T>
  void
  BinaryOutStream::write_list(const std::list<T>& val)
    throw(Exception, El::Exception)
  {
    *this << (uint64_t)val.size();

    for(typename std::list<T>::const_iterator it = val.begin();
        it != val.end(); it++)
    {
      *this << *it;
    }
  }

  template<typename C>
  void
  BinaryOutStream::write_container(const C& val)
    throw(Exception, El::Exception)
  {
    *this << (uint64_t)val.size();

    for(typename C::const_iterator i(val.begin()), e(val.end()); i != e; ++i)
    {
      *this << *i;
    }
  }

  template<typename C, typename SERIALIZER>
  void
  BinaryOutStream::write_container(const C& val, const SERIALIZER& serializer)
    throw(Exception, El::Exception)
  {
    *this << (uint64_t)val.size();

    for(typename C::const_iterator i(val.begin()), e(val.end()); i != e; ++i)
    {
      serializer.write(*i, *this);
    }
  }

  template<typename A>
  void
  BinaryOutStream::write_array(const A& arr) throw(Exception, El::Exception)
  {
    uint64_t length = arr.size();

    *this << length;
    
    for(size_t i = 0; i < length; i++)
    {
      *this << arr[i];
    }
  }

  inline
  void
  BinaryOutStream::write_string(const char* val)
    throw(Exception, El::Exception)
  {
    uint64_t len = val ? strlen(val) : UINT64_MAX;
    
    *this << len;
    
    if(len && len != UINT64_MAX)
    {
      backend_.write(val, len);
      written_bytes_ += len;
    }

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_string failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_string_buff(const char* val)
    throw(Exception, El::Exception)
  {
    uint64_t len = strlen(val);
    *this << len;
    write_raw_bytes((const unsigned char*)val, len);
  }
  
  inline
  void
  BinaryOutStream::write_string(const std::string& val)
    throw(Exception, El::Exception)
  {
    uint64_t len = val.length();
    
    *this << len;
    
    if(len)
    {
      backend_.write(val.c_str(), len);
      written_bytes_ += len;
    }

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_string failed");
    }
  }

  inline
  void
  BinaryOutStream::write_wstring(const wchar_t* val)
    throw(Exception, El::Exception)
  {    
    uint64_t len = val ? wcslen(val) : UINT64_MAX;
    
    *this << len;
    
    if(len && len != UINT64_MAX)
    {
      len *= sizeof(wchar_t);      
      backend_.write((const char*)val, len);
      written_bytes_ += len;
    }

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_wstring failed");
    }
  }
  
  inline
  void
  BinaryOutStream::write_wstring(const std::wstring& val)
    throw(Exception, El::Exception)
  {    
    uint64_t len = val.length();
    
    *this << len;
    
    if(len)
    {
      len *= sizeof(wchar_t);      
      backend_.write((const char*)val.c_str(), len);
      written_bytes_ += len;
    }

    if(backend_.fail())
    {
      throw Exception("BinaryOutStream::write_wstring failed");
    }
  }

  inline
  size_t
  BinaryOutStream::written_bytes() const throw()
  {
    return written_bytes_;
  }
  
  //
  // BinaryInStream class
  //
  
  inline
  BinaryInStream::BinaryInStream(std::istream& backend) throw(El::Exception)
      : backend_(backend),
        read_bytes_(0),
        short_types_(false)
  {
  }

  inline
  BinaryInStream::~BinaryInStream() throw()
  {
  }

  inline
  void
  BinaryInStream::seek(size_t offset, std::ios_base::seekdir dir)
    throw(El::Exception)
  {
    backend_.seekg(offset, dir);

    if(backend_.fail())
    {
      std::ostringstream ostr;
      ostr << "El::BinaryInStream::seek(" << offset << ") failed";
      
      throw Exception(ostr.str());
    }
    
    read_bytes_ = backend_.tellg();
  }
  
  inline
  void
  BinaryInStream::read_int8(int8_t& val) throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_int8 failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_int16(int16_t& val) throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_int16 failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_uint16(uint16_t& val)
    throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_uint16 failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_int32(int32_t& val) throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_int32 failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_uint32(uint32_t& val) throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_uint32 failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_float(float& val)
    throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_float failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_double(double& val)
    throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_double failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_int64(int64_t& val) throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_int64 failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_uint64(uint64_t& val) throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_uint64 failed");
    }

    read_bytes_ += sizeof(val);
  }

  inline
  void
  BinaryInStream::read_uint8(uint8_t& val) throw(Exception, El::Exception)
  {
    backend_.read((char*)&val, sizeof(val));

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_uint8 failed");
    }

    read_bytes_ += sizeof(val);
  }
    
  inline
  void
  BinaryInStream::read_string(char*& val) throw(Exception, El::Exception)
  {
    uint64_t len = 0;
    
    if(short_types_)
    {
      uint32_t len32 = 0;
      *this >> len32;
    
      if(len32 == UINT32_MAX)
      {
        val = 0;
        return;
      }

      len = len32;
    }
    else
    {
      *this >> len;

      if(len == UINT64_MAX)
      {
        val = 0;
        return;
      }
    }

    val = new char[len + 1];

    try
    {
      backend_.read(val, len);

      if(backend_.fail())
      {
        throw Exception("BinaryInStream::read_string failed");
      }

      read_bytes_ += len;
    }
    catch(...)
    {
      delete [] val;
      val = 0;
      
      throw;
    }

    val[len] = '\0';
  }

  inline
  void
  BinaryInStream::read_string_buff(char* val, size_t max_len)
    throw(Exception, El::Exception)
  {
    uint64_t len = 0;
    *this >> len;

    if(len >= max_len)
    {
      throw Exception("BinaryInStream::read_string_buff: too long string");
    }
    
    read_raw_bytes((unsigned char*)val, len);
    val[len] = '\0';
  }

  inline
  void
  BinaryInStream::read_bytes(unsigned char*& val, size_t& length)
    throw(Exception, El::Exception)
  {
    uint64_t len = 0;
    
    if(short_types_)
    { 
      uint32_t len32 = 0;
      *this >> len32;
    
      if(len32 == UINT32_MAX)
      {
        val = 0;
        length = 0;
        return;
      }

      len = len32;
    }
    else
    {
      *this >> len;
    
      if(len == UINT64_MAX)
      {
        val = 0;
        length = 0;
        return;
      }

      if(len > SIZE_MAX)
      {
        throw Exception(
          "El::BinaryOutStream::read_bytes: long arrays not supported");
      }
    }

    val = new unsigned char[len];

    try
    {
      backend_.read((char*)val, len);

      if(backend_.fail())
      {
        throw Exception("BinaryInStream::read_bytes failed");
      }

      read_bytes_ += len;
    }
    catch(...)
    {
      delete [] val;
      val = 0;
      
      throw;
    }
    
    length = len;
  }

  inline
  void
  BinaryInStream::read_raw_bytes(unsigned char* val, size_t length)
    throw(Exception, El::Exception)
  {
    backend_.read((char*)val, length);

    if(backend_.fail())
    {
      throw Exception("BinaryInStream::read_raw_bytes failed");
    }

    read_bytes_ += length;
  }

  template<typename C>
  void
  BinaryInStream::read_container(C& val)
    throw(Exception, El::Exception)
  {
    uint64_t size = 0;
    
    if(short_types_)
    {
      uint32_t size32 = 0;
      *this >> size32;
      
      size = size32;
    }
    else
    {
      *this >> size;
    }

    typename C::value_type empty;
    for(size_t i = 0; i < size; i++)
    {
      *this >> *val.insert(val.end(), empty);
    }    
  }

  template<typename C, typename SERIALIZER>
  void
  BinaryInStream::read_container(C& val, const SERIALIZER& serializer)
    throw(Exception, El::Exception)
  {
    uint64_t size = 0;
    
    if(short_types_)
    {
      uint32_t size32 = 0;
      *this >> size32;
      
      size = size32;
    }
    else
    {
      *this >> size;
    }

    typename C::value_type item;
    
    for(size_t i = 0; i < size; i++)
    {
      serializer.read(item, *this);
      val.insert(val.end(), item);
    }    
  }  

  template<typename SET>
  void
  BinaryInStream::read_set(SET& set) throw(Exception, El::Exception)
  {
    uint64_t size = 0;

    if(short_types_)
    {
      uint32_t size32 = 0;
      *this >> size32;
      
      size = size32;
    }
    else
    {
      *this >> size;
    }

    typename SET::value_type val;
    
    for(size_t i = 0; i < size; i++)
    {
      *this >> val;
      set.insert(val);
    }    
  }

  template<typename SET, typename SERIALIZER>
  void
  BinaryInStream::read_set(SET& set, const SERIALIZER& serializer)
    throw(Exception, El::Exception)
  {
    uint64_t size = 0;

    if(short_types_)
    {
      uint32_t size32 = 0;
      *this >> size32;
      
      size = size32;
    }
    else
    {
      *this >> size;
    }

    typename SET::value_type val;
    
    for(size_t i = 0; i < size; i++)
    {
      serializer.read(val, *this);
      set.insert(val);
    }    
  }  
  
  template<typename A>
  void
  BinaryInStream::read_array(A& arr) throw(Exception, El::Exception)
  {
    uint64_t size = 0;

    if(short_types_)
    {
      uint32_t size32 = 0;
      *this >> size32;
      
      size = size32;
    }
    else
    {
      *this >> size;      
    }
    
    arr.resize(size);

    for(size_t i = 0; i < size; i++)
    {
      *this >> arr[i];
    }
  }
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#  pragma GCC diagnostic push
#endif
  
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
#  pragma GCC diagnostic ignored "-Wuninitialized"      
#endif
  
  template<typename MAP>
  void
  BinaryInStream::read_map(MAP& map) throw(Exception, El::Exception)
  {
    uint64_t size = 0;

    if(short_types_)
    {
      uint32_t size32 = 0;
      *this >> size32;

      size = size32;
    }
    else
    {
      *this >> size;
    }

    typename MAP::mapped_type val;

    for(size_t i = 0; i < size; i++)
    {
      typename MAP::key_type key;
      
      *this >> key;
      *this >> map.insert(std::make_pair(key, val)).first->second;      
    }
  }

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#  pragma GCC diagnostic pop
#endif

  template<typename T>
  void
  BinaryInStream::read_list(std::list<T>& val)
    throw(Exception, El::Exception)
  {
    uint64_t size = 0;

    if(short_types_)
    {
      uint32_t size32 = 0;
      *this >> size32;

      size = size32;
    }
    else
    {
      *this >> size;
    }

    for(size_t i = 0; i < size; i++)
    {
      val.push_back(T());
      *this >> *val.rbegin();
    }
  }

  inline
  void
  BinaryInStream::read_string(std::string& val) throw(Exception, El::Exception)
  {
    uint64_t len = 0;

    if(short_types_)
    {
      uint32_t len32 = 0;
      *this >> len32;
    
      if(len32 == UINT32_MAX)
      {
        val.clear();
        return;
      }

      len = len32;
    }
    else
    {
      *this >> len;

      if(len == UINT64_MAX)
      {
        val.clear();
        return;
      }
    }

    char* buff = new char[len];

    try
    {
      backend_.read(buff, len);

      if(backend_.fail())
      {
        throw Exception("BinaryInStream::read_string failed");
      }

      read_bytes_ += len;
      val.assign(buff, len);
    }
    catch(...)
    {
      delete [] buff;
      throw;
    }

    delete [] buff;
  }

  inline
  void
  BinaryInStream::read_wstring(wchar_t*& val) throw(Exception, El::Exception)
  {
    uint64_t len = 0;

    if(short_types_)
    {
      uint32_t len32 = 0;
      *this >> len32;
    
      if(len32 == UINT32_MAX)
      {
        val = 0;
        return;
      }

      len = len32;
    }
    else
    {
      *this >> len;

      if(len == UINT64_MAX)
      {
        val = 0;
        return;
      }
    }

    val = new wchar_t[len + 1];

    try
    {
      unsigned long size = sizeof(wchar_t) * len;
      backend_.read((char*)val, size);

      if(backend_.fail())
      {
        throw Exception("BinaryInStream::read_wstring failed");
      }

      read_bytes_ += size;
    }
    catch(...)
    {
      delete [] val;
      throw;
    }

    val[len] = L'\0';
  }

  inline
  void
  BinaryInStream::read_wstring(std::wstring& val)
    throw(Exception, El::Exception)
  {
    uint64_t len = 0;

    if(short_types_)
    {
      uint32_t len32 = 0;
      *this >> len32;
    
      if(len32 == UINT32_MAX)
      {
        val.clear();
        return;
      }

      len = len32;
    }
    else
    {
      *this >> len;
    
      if(len == UINT64_MAX)
      {
        val.clear();
        return;
      }
    }

    wchar_t* buff = new wchar_t[len];

    try
    {
      size_t size = sizeof(wchar_t) * len;
      backend_.read((char*)buff, size);

      if(backend_.fail())
      {
        throw Exception("BinaryInStream::read_wstring failed");
      }

      read_bytes_ += size;
      val.assign(buff, len);
    }
    catch(...)
    {
      delete [] buff;
      throw;
    }

    delete [] buff;
  }

  inline
  size_t
  BinaryInStream::read_bytes() throw()
  {
    return read_bytes_;
  }
  
//
// SerializableSet class template
//
  template<typename TYPE>
  void
  SerializableSet<TYPE>::write(El::BinaryOutStream& bstr) const
    throw(El::Exception)
  {
    bstr.write_set(*this);
  }
  
  template<typename TYPE>
  void
  SerializableSet<TYPE>::read(El::BinaryInStream& bstr)
    throw(El::Exception)
  {
    bstr.read_set(*this);
  }
  
}


#endif // _ELEMENTS_EL_BINARYSTREAM_HPP_
