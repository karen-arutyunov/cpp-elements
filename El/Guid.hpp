/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Guid.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_GUID_HPP_
#define _ELEMENTS_EL_GUID_HPP_

#include <stdint.h>

#include <string>
#include <sstream>

//#include <ext/hash_fun.h>
#include <uuid/uuid.h>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>
#include <El/Hash/Hash.hpp>

namespace El
{
  struct Guid
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);

    static const Guid null;

    Guid() throw();
    Guid(const Guid& guid) throw();

    Guid(const char* value) throw(InvalidArg, Exception, El::Exception);
    Guid(const std::string& value) throw(InvalidArg, Exception, El::Exception);

    Guid& operator=(const char* value)
      throw(InvalidArg, Exception, El::Exception);
    
    Guid& operator=(const Guid& value) throw();

    bool operator==(const Guid& val) const throw();
    bool operator!=(const Guid& val) const throw();
    bool operator<(const Guid& val) const throw();
    bool operator>(const Guid& val) const throw();
    bool operator<=(const Guid& val) const throw();
    bool operator>=(const Guid& val) const throw();

    operator const unsigned char* () const throw ();
    operator unsigned char* ()  throw ();

    void generate() throw(Exception, El::Exception);
    
    enum GuidFormat
    {
      GF_CLASSIC,
      GF_DENSE
    };
    
    std::string string(GuidFormat format = GF_CLASSIC) const
      throw(El::Exception);

    void write(El::BinaryOutStream& bstr) const throw(El::Exception);
    void read(El::BinaryInStream& bstr) throw(El::Exception);
    
  private:
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_hi_and_version;
    uint16_t clock_seq;
    uint8_t  node[6];
  };

  namespace Hash
  {
    struct Guid
    {
      size_t operator()(const El::Guid& guid) const throw(El::Exception);      
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  inline
  Guid::Guid() throw()
  {
    memset(this, 0, sizeof(*this));
  }
  
  inline
  Guid::Guid(const Guid& guid) throw()
  {
    *this = guid;
  }
  
  inline
  Guid::Guid(const std::string& value)
    throw(InvalidArg, Exception, El::Exception)
  {
    *this = value.c_str();
  }
    
  inline
  Guid::Guid(const char* value)
    throw(InvalidArg, Exception, El::Exception)
  {
    *this = value;
  }

  inline
  void
  Guid::write(El::BinaryOutStream& bstr) const throw(El::Exception)
  {
    bstr << time_low << time_mid << time_hi_and_version << clock_seq;

    for(const uint8_t *i(node), *e(node + 6); i != e; ++i)
    {
      bstr << *i;
    }
  }
  
  inline
  void
  Guid::read(El::BinaryInStream& bstr) throw(El::Exception)
  {
    bstr >> time_low >> time_mid >> time_hi_and_version >> clock_seq;

    for(uint8_t *i(node), *e(node + 6); i != e; ++i)
    {
      bstr >> *i;
    }
  }
  
  inline
  Guid::operator const unsigned char* () const throw ()
  {
    return (const unsigned char*)this;
  }
  
  inline
  Guid::operator unsigned char* () throw ()
  {
    return (unsigned char*)this;
  }
  
  inline
  Guid&
  Guid::operator=(const Guid& value) throw()
  {
    memcpy(this, &value, sizeof(value));
    return *this;
  }
  
  inline
  Guid&
  Guid::operator=(const char* value)
    throw(InvalidArg, Exception, El::Exception)
  {
    unsigned int time_low_val;
    unsigned int time_mid_val;
    unsigned int time_hi_and_version_val;
    unsigned int clock_seq_val;
    unsigned int node_val[6];

    if(value == 0 || *value == '\0')
    {
      throw InvalidArg("El::Guid::Guid: argument is null or empty");
    }

    size_t fields = 0;
    
    if(*value == '{')
    {
      fields = sscanf(value,
                      "{%08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X}",
                      &time_low_val,
                      &time_mid_val,
                      &time_hi_and_version_val,
                      &clock_seq_val,
                      node_val,
                      node_val + 1,
                      node_val + 2,
                      node_val + 3,
                      node_val + 4,
                      node_val + 5);
    }
    else
    {
      fields = sscanf(value,
                      "%08X%04X%04X%04X%02X%02X%02X%02X%02X%02X",
                      &time_low_val,
                      &time_mid_val,
                      &time_hi_and_version_val,
                      &clock_seq_val,
                      node_val,
                      node_val + 1,
                      node_val + 2,
                      node_val + 3,
                      node_val + 4,
                      node_val + 5);
    }

    if(fields == 10)
    {
      time_low = time_low_val;
      time_mid = time_mid_val;
      time_hi_and_version = time_hi_and_version_val;
      clock_seq = clock_seq_val;

      for(size_t i = 0; i < 6; i++)
      {
        node[i] = node_val[i];
      }

      return *this;
    }

    std::ostringstream ostr;
    ostr << "El::Guid::operator=: invalid format of '" << value << "'";
      
    throw InvalidArg(ostr.str());
  }
  
  inline
  bool
  Guid::operator==(const Guid& val) const throw()
  {
    return uuid_compare(*this, val) == 0;
  }
  
  inline
  bool
  Guid::operator!=(const Guid& val) const throw()
  {
    return uuid_compare(*this, val) != 0;
  }
  
  inline
  bool
  Guid::operator<(const Guid& val) const throw()
  {
    return uuid_compare(*this, val) < 0;
  }
  
  inline
  bool
  Guid::operator>(const Guid& val) const throw()
  {
    return uuid_compare(*this, val) > 0;
  }
  
  inline
  bool
  Guid::operator<=(const Guid& val) const throw()
  {
    return uuid_compare(*this, val) <= 0;
  }
  
  inline
  bool
  Guid::operator>=(const Guid& val) const throw()
  {
    return uuid_compare(*this, val) >= 0;
  }
  
  inline
  std::string
  Guid::string(GuidFormat format) const throw(El::Exception) 
  {
    char str[100];
    
    switch(format)
    {
    case GF_CLASSIC:
      {
        sprintf(str,
                "{%08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X}",
                (unsigned int)time_low,
                time_mid,
                time_hi_and_version,
                clock_seq,
                node[0],
                node[1],
                node[2],
                node[3],
                node[4],
                node[5]);
        break;
      }
    case GF_DENSE:
      {
        sprintf(str,
                "%08X%04X%04X%04X%02X%02X%02X%02X%02X%02X",
                (unsigned int)time_low,
                time_mid,
                time_hi_and_version,
                clock_seq,
                node[0],
                node[1],
                node[2],
                node[3],
                node[4],
                node[5]);
        break;
      }
    }
    
    return str;
  }

  inline
  void
  Guid::generate() throw(Exception, El::Exception)
  {
//    const char* file_error = 0;
    
    {
//      Guard guard(lock_);
      uuid_generate(*this);
      
//      file_error = uuid_create(this);
    }
/*    
    if(file_error != 0)
    {
      std::ostringstream ostr;
      ostr << "El::Guid::generate: GUID generation failed. Can't open file '"
           << file_error << "' for write access";
      
      throw Exception(ostr.str());
    }
*/
  }

  namespace Hash
  {
    inline
    size_t
    Guid::operator()(const El::Guid& guid) const throw(El::Exception)
    {
      return __gnu_cxx::__stl_hash_string(
        guid.string(El::Guid::GF_DENSE).c_str());
    }
  }
}

#endif // _ELEMENTS_EL_GUID_HPP_
