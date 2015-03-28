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
 * @file Elements/El/Country.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_COUNTRY_HPP_
#define _ELEMENTS_EL_COUNTRY_HPP_

#include <stdint.h>
#include <ctype.h>

#include <ostream>
#include <sstream>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>
#include <El/Lang.hpp>

namespace El
{
  class Country
  {
  public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);

    static const Country null;

    // To make it possible to use class as a key for google hash maps
    static const Country nonexistent; 
    static const Country nonexistent2;

    enum ElCode
    {
      EC_NUL,

//    ISO 3166 country codes:
      
      EC_AFG,
      EC_ALB,
      EC_DZA,
      EC_ASM,
      EC_AND,
      EC_AGO,
      EC_AIA,
      EC_ATA,
      EC_ATG,
      EC_ARG,
      EC_ARM,
      EC_ABW,
      EC_AUS,
      EC_AUT,
      EC_AZE,
      EC_BHS,
      EC_BHR,
      EC_BGD,
      EC_BRB,
      EC_BLR,
      EC_BEL,
      EC_BLZ,
      EC_BEN,
      EC_BMU,
      EC_BTN,
      EC_BOL,
      EC_BIH,
      EC_BWA,
      EC_BVT,
      EC_BRA,
      EC_IOT,
      EC_VGB,
      EC_BRN,
      EC_BGR,
      EC_BFA,
      EC_BDI,
      EC_KHM,
      EC_CMR,
      EC_CAN,
      EC_CPV,
      EC_CYM,
      EC_CAF,
      EC_TCD,
      EC_CHL,
      EC_CHN,
      EC_CXR,
      EC_CCK,
      EC_COL,
      EC_COM,
      EC_COD,
      EC_COG,
      EC_COK,
      EC_CRI,
      EC_CIV,
      EC_CUB,
      EC_CYP,
      EC_CZE,
      EC_DNK,
      EC_DJI,
      EC_DMA,
      EC_DOM,
      EC_ECU,
      EC_EGY,
      EC_SLV,
      EC_GNQ,
      EC_ERI,
      EC_EST,
      EC_ETH,
      EC_FRO,
      EC_FLK,
      EC_FJI,
      EC_FIN,
      EC_FRA,
      EC_GUF,
      EC_PYF,
      EC_ATF,
      EC_GAB,
      EC_GMB,
      EC_GEO,
      EC_DEU,
      EC_GHA,
      EC_GIB,
      EC_GRC,
      EC_GRL,
      EC_GRD,
      EC_GLP,
      EC_GUM,
      EC_GTM,
      EC_GIN,
      EC_GNB,
      EC_GUY,
      EC_HTI,
      EC_HMD,
      EC_VAT,
      EC_HND,
      EC_HKG,
      EC_HRV,
      EC_HUN,
      EC_ISL,
      EC_IND,
      EC_IDN,
      EC_IRN,
      EC_IRQ,
      EC_IRL,
      EC_ISR,
      EC_ITA,
      EC_JAM,
      EC_JPN,
      EC_JOR,
      EC_KAZ,
      EC_KEN,
      EC_KIR,
      EC_PRK,
      EC_KOR,
      EC_KWT,
      EC_KGZ,
      EC_LAO,
      EC_LVA,
      EC_LBN,
      EC_LSO,
      EC_LBR,
      EC_LBY,
      EC_LIE,
      EC_LTU,
      EC_LUX,
      EC_MAC,
      EC_MKD,
      EC_MDG,
      EC_MWI,
      EC_MYS,
      EC_MDV,
      EC_MLI,
      EC_MLT,
      EC_MHL,
      EC_MTQ,
      EC_MRT,
      EC_MUS,
      EC_MYT,
      EC_MEX,
      EC_FSM,
      EC_MDA,
      EC_MCO,
      EC_MNG,
      EC_MSR,
      EC_MAR,
      EC_MOZ,
      EC_MMR,
      EC_NAM,
      EC_NRU,
      EC_NPL,
      EC_ANT,
      EC_NLD,
      EC_NCL,
      EC_NZL,
      EC_NIC,
      EC_NER,
      EC_NGA,
      EC_NIU,
      EC_NFK,
      EC_MNP,
      EC_NOR,
      EC_OMN,
      EC_PAK,
      EC_PLW,
      EC_PSE,
      EC_PAN,
      EC_PNG,
      EC_PRY,
      EC_PER,
      EC_PHL,
      EC_PCN,
      EC_POL,
      EC_PRT,
      EC_PRI,
      EC_QAT,
      EC_REU,
      EC_ROU,
      EC_RUS,
      EC_RWA,
      EC_SHN,
      EC_KNA,
      EC_LCA,
      EC_SPM,
      EC_VCT,
      EC_WSM,
      EC_SMR,
      EC_STP,
      EC_SAU,
      EC_SEN,
      EC_SCG,
      EC_SYC,
      EC_SLE,
      EC_SGP,
      EC_SVK,
      EC_SVN,
      EC_SLB,
      EC_SOM,
      EC_ZAF,
      EC_SGS,
      EC_ESP,
      EC_LKA,
      EC_SDN,
      EC_SUR,
      EC_SJM,
      EC_SWZ,
      EC_SWE,
      EC_CHE,
      EC_SYR,
      EC_TWN,
      EC_TJK,
      EC_TZA,
      EC_THA,
      EC_TLS,
      EC_TGO,
      EC_TKL,
      EC_TON,
      EC_TTO,
      EC_TUN,
      EC_TUR,
      EC_TKM,
      EC_TCA,
      EC_TUV,
      EC_VIR,
      EC_UGA,
      EC_UKR,
      EC_ARE,
      EC_GBR,
      EC_UMI,
      EC_USA,
      EC_URY,
      EC_UZB,
      EC_VUT,
      EC_VEN,
      EC_VNM,
      EC_WLF,
      EC_ESH,
      EC_YEM,
      EC_ZMB,
      EC_ZWE,

      EC_NONEXISTENT = UINT16_MAX,
      EC_NONEXISTENT2 = UINT16_MAX - 1
    };

    Country(const ElCode& code = EC_NUL) throw();
    Country(const Country& val) throw();
    Country(const char* val) throw(InvalidArg, El::Exception);
    
    Country& operator=(const Country& val) throw();

    bool operator==(const Country& val) const throw();
    bool operator!=(const Country& val) const throw();

    ElCode el_code() const throw();
    const char* l2_code(bool zz = false) const throw();
    const char* l3_code(bool zzz = false) const throw();
    const char* d3_code() const throw();
    const char* domain() const throw();
    const char* name() const throw();
    El::Lang lang() const throw();

    static ElCode el_code(unsigned long code) throw(InvalidArg, El::Exception);

    static unsigned long countries_count() throw();

    void write(El::BinaryOutStream& ostr) const
      throw(Exception, El::Exception);

    void read(El::BinaryInStream& istr) throw(Exception, El::Exception);
    
  private:

    uint16_t code_;
    
    struct Record
    {
      ElCode       el_code;
      const char*  l2_code;
      const char*  l3_code;
      const char*  d3_code;
      const char*  domain;
      Lang::ElCode language;
      const char*  name;
    };
    
    static const Record countries_[];
    static const unsigned long countries_count_;
  };  
}

namespace El
{
  namespace Hash
  {
    struct Country
    {
      size_t operator()(const El::Country& country) const
        throw(El::Exception);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  inline
  Country::Country(const char* val) throw(InvalidArg, El::Exception)
      : code_(EC_NUL)
  {
    if(val == 0 || *val == '\0' || val[1] == '\0')
    {
      throw InvalidArg("El::Country::Country: empty argument");
    }

    if(val[2] == '\0')
    {
      const Record* rec = countries_;

      do
      {
        if(strcasecmp(val, rec->l2_code) == 0)
        {
          code_ = rec->el_code;
          return;
        }
      }
      while(rec++->el_code != EC_NUL);

      std::ostringstream ostr;
      ostr << "El::Country::Country: unexpected country 2 letter code '"
           << val << "'";
        
      throw InvalidArg(ostr.str());
    }
    else if(val[0] == '.')
    {
      const Record* rec = countries_;

      do
      {
        if(strcasecmp(val, rec->domain) == 0)
        {
          code_ = rec->el_code;
          return;
        }
      }
      while(rec++->el_code != EC_NUL);

      std::ostringstream ostr;
      ostr << "El::Country::Country: unexpected country domain '"
           << val << "'";
        
      throw InvalidArg(ostr.str());
    }
    else
    {
      const Record* rec = countries_;

      if(val[3] != '\0')
      {
        do
        {
          if(strcasecmp(val, rec->name) == 0)
          {
            code_ = rec->el_code;
            return;
          }
        }
        while(rec++->el_code != EC_NUL);
      }
      else
      {
        if(isdigit(*val))
        {
          do
          {
            if(strcmp(val, rec->d3_code) == 0)
            {
              code_ = rec->el_code;
              return;
            }
          }
          while(rec++->el_code != EC_NUL);
        }
        else
        {
          do
          {
            if(strcasecmp(val, rec->l3_code) == 0)
            {
              code_ = rec->el_code;
              return;
            }
          }
          while(rec++->el_code != EC_NUL);
        }
      }
      
      std::ostringstream ostr;
      ostr << "El::Country::Country: unexpected country specification '" << val
           << "'";

      throw InvalidArg(ostr.str());
    }
  }

  inline
  Country::Country(const ElCode& code) throw()
    : code_(code)
  {
  }
  
  inline
  Country::Country(const Country& val) throw()
    : code_(val.code_)
  {
  }
  
  inline
  Country&
  Country::operator=(const Country& val) throw()
  {
    code_ = val.code_;
    return *this;
  }
  
  inline
  bool
  Country::operator==(const Country& val) const throw()
  {
    return code_ == val.code_;
  }

  inline
  bool
  Country::operator!=(const Country& val) const throw()
  {
    return code_ != val.code_;
  }

  inline
  Country::ElCode
  Country::el_code(unsigned long code) throw(InvalidArg, El::Exception)
  {
    bool valid = (code >= EC_NUL && code <= EC_ZWE) || code == EC_NONEXISTENT ||
      code == EC_NONEXISTENT2;

    if(!valid)
    {
      std::ostringstream ostr;
      ostr << "El::Country::el_code: invalid country code " << code;
      throw InvalidArg(ostr.str());
    }

    return (ElCode)code;
  }
  
  inline
  Country::ElCode
  Country::el_code() const throw()
  {
    return (Country::ElCode)code_;
  }

  inline
  const char*
  Country::l2_code(bool zz) const throw()
  {
    const Record* rec = countries_;
    const Record* end = countries_ + countries_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? (zz && code_ == EC_NUL ? "ZZ" : "") : rec->l2_code;
  }

  inline
  const char*
  Country::l3_code(bool zzz) const throw()
  {
    const Record* rec = countries_;
    const Record* end = countries_ + countries_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? (zzz && code_ == EC_NUL ? "ZZZ" : "") : rec->l3_code;
  }
  
  inline
  const char*
  Country::d3_code() const throw()
  {
    const Record* rec = countries_;
    const Record* end = countries_ + countries_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? "" : rec->d3_code;
  }
  
  inline
  const char*
  Country::domain() const throw()
  {
    const Record* rec = countries_;
    const Record* end = countries_ + countries_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? "" : rec->domain;
  }
  
  inline
  const char*
  Country::name() const throw()
  {
    const Record* rec = countries_;
    const Record* end = countries_ + countries_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? "" : rec->name;
  }

  inline
  El::Lang
  Country::lang() const throw()
  {
    const Record* rec = countries_;
    const Record* end = countries_ + countries_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? Lang::EC_NUL : rec->language;
  }
  
  inline
  unsigned long
  Country::countries_count() throw()
  {
    return countries_count_;
  }
  
  inline
  void
  Country::write(El::BinaryOutStream& ostr) const
    throw(Exception, El::Exception)
  {
    ostr << code_;
  }
  
  inline
  void
  Country::read(El::BinaryInStream& istr) throw(Exception, El::Exception)
  {
    istr >> code_;
  }
}

inline
std::ostream&
operator<<(std::ostream& ostr, const El::Country& country) throw(El::Exception)
{
  ostr << country.el_code() << " (";
  
  if(country == El::Country::null)
  {
    ostr << "null";
  }
  else
  {
    const char* code = country.l3_code();

    if(code == 0)
    {
      ostr << "unknown";
    }
    else
    {
      ostr << code << ", " << country.name();
    } 
  }

  ostr << ")";

  return ostr;
}
  
namespace El
{
  namespace Hash
  {
    inline
    size_t
    Country::operator()(const El::Country& country) const
      throw(El::Exception)
    {
      return country.el_code();
    }
  }
}

#endif // _ELEMENTS_EL_COUNTRY_HPP_
