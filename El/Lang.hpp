/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Lang.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LANG_HPP_
#define _ELEMENTS_EL_LANG_HPP_

#include <stdint.h>

#include <ostream>
#include <sstream>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>

namespace El
{
  class Lang
  {
  public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);

    static const Lang null;

    // To make it possible to use class as a key for google hash maps
    static const Lang nonexistent; 
    static const Lang nonexistent2;

    enum ElCode
    {
      EC_NUL, // ISO 639-2 language codes:      
      EC_AAR,
      EC_ABK,
      EC_ACE,
      EC_ACH,
      EC_ADA,
      EC_ADY,
      EC_AFA,
      EC_AFH,
      EC_AFR,
      EC_AIN,
      EC_AKA,
      EC_AKK,
      EC_ALB,
      EC_ALE,
      EC_ALG,
      EC_ALT,
      EC_AMH,
      EC_ANG,
      EC_ANP,
      EC_APA,
      EC_ARA,
      EC_ARC,
      EC_ARG,
      EC_ARM,
      EC_ARN,
      EC_ARP,
      EC_ART,
      EC_ARW,
      EC_ASM,
      EC_AST,
      EC_ATH,
      EC_AUS,
      EC_AVA,
      EC_AVE,
      EC_AWA,
      EC_AYM,
      EC_AZE,
      EC_BAD,
      EC_BAI,
      EC_BAK,
      EC_BAL,
      EC_BAM,
      EC_BAN,
      EC_BAQ,
      EC_BAS,
      EC_BAT,
      EC_BEJ,
      EC_BEL,
      EC_BEM,
      EC_BEN,
      EC_BER,
      EC_BHO,
      EC_BIH,
      EC_BIK,
      EC_BIN,
      EC_BIS,
      EC_BLA,
      EC_BNT,
      EC_BOS,
      EC_BRA,
      EC_BRE,
      EC_BTK,
      EC_BUA,
      EC_BUG,
      EC_BUL,
      EC_BUR,
      EC_BYN,
      EC_CAD,
      EC_CAI,
      EC_CAR,
      EC_CAT,
      EC_CAU,
      EC_CEB,
      EC_CEL,
      EC_CHA,
      EC_CHB,
      EC_CHE,
      EC_CHG,
      EC_CHI,
      EC_CHK,
      EC_CHM,
      EC_CHN,
      EC_CHO,
      EC_CHP,
      EC_CHR,
      EC_CHU,
      EC_CHV,
      EC_CHY,
      EC_CMC,
      EC_COP,
      EC_COR,
      EC_COS,
      EC_CPE,
      EC_CPF,
      EC_CPP,
      EC_CRE,
      EC_CRH,
      EC_CRP,
      EC_CSB,
      EC_CUS,
      EC_CZE,
      EC_DAK,
      EC_DAN,
      EC_DAR,
      EC_DAY,
      EC_DEL,
      EC_DEN,
      EC_DGR,
      EC_DIN,
      EC_DIV,
      EC_DOI,
      EC_DRA,
      EC_DSB,
      EC_DUA,
      EC_DUM,
      EC_DUT,
      EC_DYU,
      EC_DZO,
      EC_EFI,
      EC_EGY,
      EC_EKA,
      EC_ELX,
      EC_ENG,
      EC_ENM,
      EC_EPO,
      EC_EST,
      EC_EWE,
      EC_EWO,
      EC_FAN,
      EC_FAO,
      EC_FAT,
      EC_FIJ,
      EC_FIL,
      EC_FIN,
      EC_FIU,
      EC_FON,
      EC_FRE,
      EC_FRM,
      EC_FRO,
      EC_FRR,
      EC_FRS,
      EC_FRY,
      EC_FUL,
      EC_FUR,
      EC_GAA,
      EC_GAY,
      EC_GBA,
      EC_GEM,
      EC_GEO,
      EC_GER,
      EC_GEZ,
      EC_GIL,
      EC_GLA,
      EC_GLE,
      EC_GLG,
      EC_GLV,
      EC_GMH,
      EC_GOH,
      EC_GON,
      EC_GOR,
      EC_GOT,
      EC_GRB,
      EC_GRC,
      EC_GRE,
      EC_GRN,
      EC_GSW,
      EC_GUJ,
      EC_GWI,
      EC_HAI,
      EC_HAT,
      EC_HAU,
      EC_HAW,
      EC_HEB,
      EC_HER,
      EC_HIL,
      EC_HIM,
      EC_HIN,
      EC_HIT,
      EC_HMN,
      EC_HMO,
      EC_HSB,
      EC_HUN,
      EC_HUP,
      EC_IBA,
      EC_IBO,
      EC_ICE,
      EC_IDO,
      EC_III,
      EC_IJO,
      EC_IKU,
      EC_ILE,
      EC_ILO,
      EC_INA,
      EC_INC,
      EC_IND,
      EC_INE,
      EC_INH,
      EC_IPK,
      EC_IRA,
      EC_IRO,
      EC_ITA,
      EC_JAV,
      EC_JBO,
      EC_JPN,
      EC_JPR,
      EC_JRB,
      EC_KAA,
      EC_KAB,
      EC_KAC,
      EC_KAL,
      EC_KAM,
      EC_KAN,
      EC_KAR,
      EC_KAS,
      EC_KAU,
      EC_KAW,
      EC_KAZ,
      EC_KBD,
      EC_KHA,
      EC_KHI,
      EC_KHM,
      EC_KHO,
      EC_KIK,
      EC_KIN,
      EC_KIR,
      EC_KMB,
      EC_KOK,
      EC_KOM,
      EC_KON,
      EC_KOR,
      EC_KOS,
      EC_KPE,
      EC_KRC,
      EC_KRL,
      EC_KRO,
      EC_KRU,
      EC_KUA,
      EC_KUM,
      EC_KUR,
      EC_KUT,
      EC_LAD,
      EC_LAH,
      EC_LAM,
      EC_LAO,
      EC_LAT,
      EC_LAV,
      EC_LEZ,
      EC_LIM,
      EC_LIN,
      EC_LIT,
      EC_LOL,
      EC_LOZ,
      EC_LTZ,
      EC_LUA,
      EC_LUB,
      EC_LUG,
      EC_LUI,
      EC_LUN,
      EC_LUO,
      EC_LUS,
      EC_MAC,
      EC_MAD,
      EC_MAG,
      EC_MAH,
      EC_MAI,
      EC_MAK,
      EC_MAL,
      EC_MAN,
      EC_MAO,
      EC_MAP,
      EC_MAR,
      EC_MAS,
      EC_MAY,
      EC_MDF,
      EC_MDR,
      EC_MEN,
      EC_MGA,
      EC_MIC,
      EC_MIN, //      EC_MIS,
      EC_MKH,
      EC_MLG,
      EC_MLT,
      EC_MNC,
      EC_MNI,
      EC_MNO,
      EC_MOH,
      EC_MOL,
      EC_MON,
      EC_MOS,
      EC_MUL,
      EC_MUN,
      EC_MUS,
      EC_MWL,
      EC_MWR,
      EC_MYN,
      EC_MYV,
      EC_NAH,
      EC_NAI,
      EC_NAP,
      EC_NAU,
      EC_NAV,
      EC_NBL,
      EC_NDE,
      EC_NDO,
      EC_NDS,
      EC_NEP,
      EC_NEW,
      EC_NIA,
      EC_NIC,
      EC_NIU,
      EC_NNO,
      EC_NOB,
      EC_NOG,
      EC_NON,
      EC_NOR,
      EC_NSO,
      EC_NUB,
      EC_NWC,
      EC_NYA,
      EC_NYM,
      EC_NYN,
      EC_NYO,
      EC_NZI,
      EC_OCI,
      EC_OJI,
      EC_ORI,
      EC_ORM,
      EC_OSA,
      EC_OSS,
      EC_OTA,
      EC_OTO,
      EC_PAA,
      EC_PAG,
      EC_PAL,
      EC_PAM,
      EC_PAN,
      EC_PAP,
      EC_PAU,
      EC_PEO,
      EC_PER,
      EC_PHI,
      EC_PHN,
      EC_PLI,
      EC_POL,
      EC_PON,
      EC_POR,
      EC_PRA,
      EC_PRO,
      EC_PUS,
      EC_QUE,
      EC_RAJ,
      EC_RAP,
      EC_RAR,
      EC_ROA,
      EC_ROH,
      EC_ROM,
      EC_RUM,
      EC_RUN,
      EC_RUP,
      EC_RUS,
      EC_SAD,
      EC_SAG,
      EC_SAH,
      EC_SAI,
      EC_SAL,
      EC_SAM,
      EC_SAN,
      EC_SAS,
      EC_SAT,
      EC_SCC,
      EC_SCN,
      EC_SCO,
      EC_SCR,
      EC_SEL,
      EC_SEM,
      EC_SGA, //      EC_SGN,
      EC_SHN,
      EC_SID,
      EC_SIN,
      EC_SIO,
      EC_SIT,
      EC_SLA,
      EC_SLO,
      EC_SLV,
      EC_SMA,
      EC_SME,
      EC_SMI,
      EC_SMJ,
      EC_SMN,
      EC_SMO,
      EC_SMS,
      EC_SNA,
      EC_SND,
      EC_SNK,
      EC_SOG,
      EC_SOM,
      EC_SON,
      EC_SOT,
      EC_SPA,
      EC_SRD,
      EC_SRN,
      EC_SRR,
      EC_SSA,
      EC_SSW,
      EC_SUK,
      EC_SUN,
      EC_SUS,
      EC_SUX,
      EC_SWA,
      EC_SWE,
      EC_SYR,
      EC_TAH,
      EC_TAI,
      EC_TAM,
      EC_TAT,
      EC_TEL,
      EC_TEM,
      EC_TER,
      EC_TET,
      EC_TGK,
      EC_TGL,
      EC_THA,
      EC_TIB,
      EC_TIG,
      EC_TIR,
      EC_TIV,
      EC_TKL,
      EC_TLH,
      EC_TLI,
      EC_TMH,
      EC_TOG,
      EC_TON,
      EC_TPI,
      EC_TSI,
      EC_TSN,
      EC_TSO,
      EC_TUK,
      EC_TUM,
      EC_TUP,
      EC_TUR,
      EC_TUT,
      EC_TVL,
      EC_TWI,
      EC_TYV,
      EC_UDM,
      EC_UGA,
      EC_UIG,
      EC_UKR,
      EC_UMB,
      EC_UND,
      EC_URD,
      EC_UZB,
      EC_VAI,
      EC_VEN,
      EC_VIE,
      EC_VOL,
      EC_VOT,
      EC_WAK,
      EC_WAL,
      EC_WAR,
      EC_WAS,
      EC_WEL,
      EC_WEN,
      EC_WLN,
      EC_WOL,
      EC_XAL,
      EC_XHO,
      EC_YAO,
      EC_YAP,
      EC_YID,
      EC_YOR,
      EC_YPK,
      EC_ZAP,
      EC_ZEN,
      EC_ZHA,
      EC_ZND,
      EC_ZUL,
      EC_ZUN,

      EC_NONEXISTENT = UINT16_MAX,
      EC_NONEXISTENT2 = UINT16_MAX - 1
    };

    Lang(const ElCode& code = EC_NUL) throw();
    Lang(const Lang& val) throw();
    Lang(const char* val) throw(InvalidArg, El::Exception);
    
    Lang& operator=(const Lang& val) throw();

    bool operator==(const Lang& val) const throw();
    bool operator!=(const Lang& val) const throw();

    ElCode el_code() const throw();
    const char* l2_code(bool zz = false) const throw();
    const char* l3_code(bool zzz = false) const throw();
    unsigned short num_code() const throw();
    const char* name() const throw();

    static ElCode el_code(unsigned long code) throw(InvalidArg, El::Exception);
    
    static unsigned long languages_count() throw();
    
    void write(El::BinaryOutStream& ostr) const
      throw(Exception, El::Exception);

    void read(El::BinaryInStream& istr) throw(Exception, El::Exception);
    
  private:
    
    uint16_t code_;

    struct Record
    {
      ElCode      el_code;
      const char* l3_code;
      const char* l2_code;
      const char* name;
    };
    
    static const Record languages_[];
    static const unsigned long languages_count_;
  };  
}

namespace El
{
  namespace Hash
  {
    struct Lang
    {
      size_t operator()(const El::Lang& lang) const
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
  Lang::Lang(const char* val) throw(InvalidArg, El::Exception)
      : code_(EC_NUL)
  {
    if(val == 0 || *val == '\0' || val[1] == '\0')
    {
      throw InvalidArg("El::Lang::Lang: empty argument");
    }

    if(val[2] == '\0')
    {
      const Record* rec = languages_;

      do
      {
        if(*rec->l2_code != '\0' && strcasecmp(val, rec->l2_code) == 0)
        {
          code_ = rec->el_code;
          return;
        }
      }
      while(rec++->el_code != EC_NUL);

      std::ostringstream ostr;
      ostr << "El::Lang::Lang: unexpected lang 2 letter code '" << val << "'";
        
      throw InvalidArg(ostr.str());
    }
    else
    {
      const Record* rec = languages_;

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
      
      std::ostringstream ostr;
      ostr << "El::Lang::Lang: unexpected language specification '" << val
           << "'";

      throw InvalidArg(ostr.str());
    }
  }

  inline
  Lang::Lang(const ElCode& code) throw() : code_(code)
  {
  }
  
  inline
  Lang::Lang(const Lang& val) throw() : code_(val.code_)
  {
  }
  
  inline
  Lang&
  Lang::operator=(const Lang& val) throw()
  {
    code_ = val.code_;
    return *this;
  }
  
  inline
  bool
  Lang::operator==(const Lang& val) const throw()
  {
    return code_ == val.code_;
  }

  inline
  bool
  Lang::operator!=(const Lang& val) const throw()
  {
    return code_ != val.code_;
  }

  inline
  Lang::ElCode
  Lang::el_code(unsigned long code) throw(InvalidArg, El::Exception)
  {
    bool valid = (code >= EC_NUL && code <= EC_ZUN) || code == EC_NONEXISTENT ||
      code == EC_NONEXISTENT2;

    if(!valid)
    {
      std::ostringstream ostr;
      ostr << "El::Lang::el_code: invalid language code " << code;
      throw InvalidArg(ostr.str());
    }

    return (ElCode)code;
  }
  
  inline
  Lang::ElCode
  Lang::el_code() const throw()
  {
    return (Lang::ElCode)code_;
  }

  inline
  unsigned short
  Lang::num_code() const throw()
  {
    const char* code = l2_code();

    return code_ == EC_NUL || code[0] == '\0' ?
      0 : (1070 + 30 * (code[0] - 'A' + 1) + (code[1] - 'A' + 1));
  }
  
  inline
  const char*
  Lang::l2_code(bool zz) const throw()
  {
    const Record* rec = languages_;
    const Record* end = languages_ + languages_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? (zz && code_ == EC_NUL ? "zz" : "") : rec->l2_code;
  }

  inline
  const char*
  Lang::l3_code(bool zzz) const throw()
  {
    const Record* rec = languages_;
    const Record* end = languages_ + languages_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? (zzz && code_ == EC_NUL ? "zzz" : "") : rec->l3_code;
  }
  
  inline
  const char*
  Lang::name() const throw()
  {
    const Record* rec = languages_;
    const Record* end = languages_ + languages_count_;
    for(; rec != end && rec->el_code != code_; ++rec);
    return rec == end ? "" : rec->name;
  }
  
  inline
  unsigned long
  Lang::languages_count() throw()
  {
    return languages_count_;
  }

  inline
  void
  Lang::write(El::BinaryOutStream& ostr) const
    throw(Exception, El::Exception)
  {
    ostr << code_;
  }
  
  inline
  void
  Lang::read(El::BinaryInStream& istr) throw(Exception, El::Exception)
  {
    istr >> code_;
  }  
}

inline
std::ostream&
operator<<(std::ostream& ostr, const El::Lang& lang) throw(El::Exception)
{
  ostr << lang.el_code() << " (";
  
  if(lang == El::Lang::null)
  {
    ostr << "null";
  }
  else
  {
    const char* code = lang.l3_code();

    if(code == 0)
    {
      ostr << "unknown";
    }
    else
    {
      ostr << code << ", " << lang.name();
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
    Lang::operator()(const El::Lang& lang) const
      throw(El::Exception)
    {
      return lang.el_code();
    }
  }
}

#endif // _ELEMENTS_EL_LANG_HPP_
