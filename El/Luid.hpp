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
 * @file Elements/El/Luid.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LUID_HPP_
#define _ELEMENTS_EL_LUID_HPP_

#include <stdint.h>

#include <sstream>
#include <string>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>
#include <El/String/Manip.hpp>

#include <ace/OS.h>

namespace El
{
  struct Luid
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);

    static const Luid null;
    static const Luid nonexistent;
    
    uint64_t data;

    Luid() throw();
    Luid(const Luid& luid) throw();
    Luid(uint64_t data_val) throw();

    Luid(const char* value, bool dense = false)
      throw(InvalidArg, Exception, El::Exception);
    
    Luid(const std::string& value, bool dense = false)
      throw(InvalidArg, Exception, El::Exception);

//    Luid& operator=(const char* value)
//      throw(InvalidArg, Exception, El::Exception);    
    
    Luid& operator=(const Luid& value) throw();

    bool operator==(const Luid& val) const throw();
    bool operator!=(const Luid& val) const throw();
    bool operator<(const Luid& val) const throw();
    bool operator>(const Luid& val) const throw();
    bool operator<=(const Luid& val) const throw();
    bool operator>=(const Luid& val) const throw();

    void generate() throw();

    std::string string(bool dense = false) const throw(El::Exception);

    void write(El::BinaryOutStream& bstr) const throw(El::Exception);
    void read(El::BinaryInStream& bstr) throw(El::Exception);

  private:
    
    void init(const char* value, bool dense)
      throw(InvalidArg, Exception, El::Exception);
  };
  
  namespace Hash
  {
    struct Luid
    {
      size_t operator()(const El::Luid& luid) const throw();
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{

  inline
  Luid::Luid() throw() : data(0)
  {
  }
  
  inline
  Luid::Luid(const Luid& luid) throw()
  {
    data = luid.data;
  }
  
  inline
  Luid::Luid(uint64_t data_val) throw() : data(data_val)
  {
  }
  
  inline
  Luid::Luid(const std::string& value, bool dense)
    throw(InvalidArg, Exception, El::Exception)
  {
//    *this = value.c_str();
    init(value.c_str(), dense);
  }
    
  inline
  Luid::Luid(const char* value, bool dense)
    throw(InvalidArg, Exception, El::Exception)
  {
//    *this = value;
    init(value, dense);
  }

  inline
  void
  Luid::init(const char* value, bool dense)
    throw(InvalidArg, Exception, El::Exception)
  {
    if(value == 0 || *value == '\0')
    {
      throw InvalidArg("El::Luid::init: argument is null or empty");
    }

    if(dense)
    {
      try
      {
        El::String::Manip::base64_decode(value,
                                         (unsigned char*)&data,
                                         sizeof(data));
      }
      catch(const El::String::Manip::InvalidArg& e)
      {
        std::ostringstream ostr;
        ostr << "El::Luid::init: invalid format of '"
             << value << "'. Details: " << e;
        
        throw InvalidArg(ostr.str());
      }

      return;
    }
    
    unsigned long long new_data = 0;
    
    if(sscanf(value, "%016llX", &new_data) == 1)
    {
      data = new_data;
      return;
    }

    std::ostringstream ostr;
    ostr << "El::Luid::init: invalid format of '" << value << "'";
      
    throw InvalidArg(ostr.str());
  }

  inline
  void
  Luid::generate() throw()
  {
    do
    {
      ACE_Time_Value tm = ACE_OS::gettimeofday();
        
      data = ((uint64_t)rand()) |
        ((((uint64_t)tm.usec()) & 0xFFFFF) <<
         (4 * 8 - 1)) |
        ((((uint64_t)tm.sec()) & 0x1FFF) <<
         (4 * 8 - 1 + 20));
        
//        std::cerr << std::hex << data << std::endl;
    }
    while(data < 2);
  }

  inline
  Luid&
  Luid::operator=(const Luid& value) throw()
  {
    data = value.data;
    return *this;
  }
/*  
  inline
  Luid&
  Luid::operator=(const char* value)
    throw(InvalidArg, Exception, El::Exception)
  {
    if(value == 0 || *value == '\0')
    {
      throw InvalidArg("El::Luid::Luid: argument is null or empty");
    }

    unsigned long long new_data = 0;
    
    if(sscanf(value, "%016llX", &new_data) == 1)
    {
      data = new_data;
      return *this;
    }

    std::ostringstream ostr;
    ostr << "El::Luid::operator=: invalid format of '" << value << "'";
      
    throw InvalidArg(ostr.str());
  }
*/
  inline
  bool
  Luid::operator==(const Luid& val) const throw()
  {
    return data == val.data;
  }
  
  inline
  bool
  Luid::operator!=(const Luid& val) const throw()
  {
    return data != val.data;
  }
  
  inline
  bool
  Luid::operator<(const Luid& val) const throw()
  {
    return data < val.data;
  }
  
  inline
  bool
  Luid::operator>(const Luid& val) const throw()
  {
    return data > val.data;
  }
  
  inline
  bool
  Luid::operator<=(const Luid& val) const throw()
  {
    return data <= val.data;
  }
  
  inline
  bool
  Luid::operator>=(const Luid& val) const throw()
  {
    return data >= val.data;
  }
  
  inline
  std::string
  Luid::string(bool dense) const throw(El::Exception) 
  {
    if(dense)
    {
      std::string val;

      El::String::Manip::base64_encode((const unsigned char*)&data,
                                       sizeof(data),
                                       val);
      return val;
    }
    
    char str[100];
    sprintf(str, "%016llX", (unsigned long long)data);
    
    return str;
  }

  inline
  void
  Luid::write(El::BinaryOutStream& bstr) const throw(El::Exception)
  {
    bstr << data;
  }

  inline
  void
  Luid::read(El::BinaryInStream& bstr) throw(El::Exception)
  {
    bstr >> data;
  }

  namespace Hash
  {
    inline
    size_t
    Luid::operator()(const El::Luid& luid) const throw()
    {
      return luid.data;
    }
  }
}

#endif // _ELEMENTS_EL_LUID_HPP_
