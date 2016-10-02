/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Moment.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_MOMENT_HPP_
#define _ELEMENTS_EL_MOMENT_HPP_

#include <time.h>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>

#include <ace/OS.h>

namespace El
{
  class Moment : public tm
  {
  public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);

    static const Moment null;

    enum TimeZone
    {
      TZ_GMT,
      TZ_LOCAL
    };

    enum TimeFormat
    {
      TF_ISO_8601, // Sample: 2005-12-17 16:09:37.10 +03:00
                   // Sample: 2005-12-17T16:09:37.10+03:00
                   // YYYY | YYYY-MM | YYYY-MM-DD |
                   // YYYY-MM-DD( |T)HH:MM[:SS[.s]][ ]ZD |
                   // s - fraction of a second,
                   // ZD - Z (means GMT) or +hh:mm or -hh:mm
      
      TF_RFC_0822, // Sample: "Sat, 17 Dec 2005 16:09:37 +0300"
                   // date-time = [ day "," ] date time ; dd mm yy
                   //                                   ; hh:mm:ss zzz
                   // day = "Mon" / "Tue" / "Wed" / "Thu" / "Fri" / "Sat" /
                   //       "Sun"
                   //
                   // date = 1*2DIGIT month 2DIGIT ; day month year
                   //                              ;  e.g. 20 Jun 82
                   // month = "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" /
                   //         "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec"
                   // time = hour zone ; ANSI and Military
                   // hour = 2DIGIT ":" 2DIGIT [":" 2DIGIT]
                   //                                    ; 00:00:00 - 23:59:59
                   // zone = "UT" / "GMT" ; Universal Time
                   //                     ; North American : UT
                   //         /  "EST" / "EDT" ;  Eastern:  - 5/ - 4
                   //         /  "CST" / "CDT" ;  Central:  - 6/ - 5
                   //         /  "MST" / "MDT" ;  Mountain: - 7/ - 6
                   //         /  "PST" / "PDT" ;  Pacific:  - 8/ - 7
                   //         /  1ALPHA        ; Military: Z = UT;
                   //                          ;  A:-1; (J not used)
                   //                          ;  M:-12; N:+1; Y:+12
                   //         / ( ("+" / "-") 4DIGIT ) ; Local differential
                   //                                  ;  hours+min. (HHMM)
      
      TF_FROM_EPOCH // Sample: 1400148778.023653
    };

    Moment(unsigned long year,  // year from 1900
           uint8_t month, // January is 0
           uint8_t day,
           uint8_t hour = 0,
           uint8_t minute = 0,
           uint8_t second = 0,
           uint32_t usec = 0,
           TimeZone tz = TZ_GMT) throw();
    
    Moment(TimeZone tz = TZ_GMT) throw();
    
    Moment(const ACE_Time_Value& time, TimeZone tz = TZ_GMT)
      throw(InvalidArg, Exception, El::Exception);

    Moment(const char* value, TimeFormat format, TimeZone tz = TZ_GMT)
      throw (InvalidArg, Exception, El::Exception);

    void set(const char* value, TimeFormat format)
      throw(InvalidArg, Exception, El::Exception);

    void set_iso8601(const char* str)
      throw(InvalidArg, Exception, El::Exception);

    void set_time_iso8601(const char* str)
      throw(InvalidArg, Exception, El::Exception);

    void set_rfc0822(const char* str)
      throw(InvalidArg, Exception, El::Exception);
    
    void set_from_epoch(const char* str)
      throw(InvalidArg, Exception, El::Exception);
    
    static uint8_t month(const char* month)
      throw(InvalidArg, Exception, El::Exception);

    static const char* month(uint8_t month)
      throw(InvalidArg, Exception, El::Exception);

    static uint8_t week_day(const char* day)
      throw(InvalidArg, Exception, El::Exception);

    static const char* week_day(uint8_t day)
      throw(InvalidArg, Exception, El::Exception);

    operator ACE_Time_Value() const throw();

    void normalize() throw(Exception, El::Exception);

    bool operator==(const Moment& val) const throw();
    bool operator!=(const Moment& val) const throw();
    bool operator<(const Moment& val) const throw();
    bool operator>(const Moment& val) const throw();
    bool operator<=(const Moment& val) const throw();
    bool operator>=(const Moment& val) const throw();

    Moment operator+(const ACE_Time_Value& val) const throw();
    Moment operator-(const ACE_Time_Value& val) const throw(Exception);

    std::string iso8601(bool timezone = true, bool time = true) const
      throw(Exception, El::Exception);
    
    std::string rfc0822(bool timezone = true, bool usec = false) const
      throw(Exception, El::Exception);

    std::string epoch(bool usec = false) const throw(Exception, El::Exception);

    enum DenseFormat
    {
      DF_DATE = 0x1,
      DF_TIME = 0x2,
      DF_USEC = 0x4,
      DF_ALL = DF_DATE | DF_TIME | DF_USEC
    };
      
    std::string dense_format(unsigned long flags = DF_ALL) const
      throw(Exception, El::Exception);

    std::string http_cookie_expiration() const
      throw(Exception, El::Exception);

    static std::string time(const ACE_Time_Value& val, bool days = false)
      throw(El::Exception);

    static ACE_Time_Value divide(const ACE_Time_Value& p1, unsigned long p2)
      throw();

    Moment date() const throw();
    Moment day_time() const throw();

    void write(El::BinaryOutStream& bstr) const throw(El::Exception);
    void read(El::BinaryInStream& bstr) throw(El::Exception);    

  public:
    int tm_usec;
    TimeZone tm_tz;
  };  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  inline
  Moment::Moment(TimeZone tz) throw()
  {
    tm_hour = 0;
    tm_isdst = 0;
    tm_mday = 0;
    tm_min = 0;
    tm_mon = 0;
    tm_sec = 0;
    tm_wday = 0;
    tm_yday = 0;
    tm_year = 0;
    tm_usec = 0;
    tm_tz = tz;
    tm_zone = 0;
    tm_gmtoff = 0;
  }
  
  inline
  Moment::Moment(unsigned long year,
                 uint8_t month,
                 uint8_t day,
                 uint8_t hour,
                 uint8_t minute,
                 uint8_t second,
                 uint32_t usec,
                 TimeZone tz) throw()
      : tm_usec(usec),
        tm_tz(tz)
  {
    tm_year = year;
    tm_mon = month;
    tm_mday = day;
    tm_hour = hour;
    tm_min = minute;
    tm_sec = second;
    tm_zone = 0;
    tm_gmtoff = 0;
    normalize();
  }

  inline
  Moment
  Moment::date() const throw()
  {
    Moment moment = *this;
    
    moment.tm_hour = 0;
    moment.tm_min = 0;
    moment.tm_sec = 0;
    moment.tm_usec = 0;

    return moment;
  }
  
  inline
  Moment
  Moment::day_time() const throw()
  {
    Moment moment = *this;
    
    moment.tm_year = 0;
    moment.tm_mon = 0;
    moment.tm_mday = 0;
    moment.tm_isdst = 0;
    moment.tm_wday = 0;
    moment.tm_yday = 0;
    
    return moment;
  }
  
  inline
  bool
  Moment::operator==(const Moment& val) const throw()
  {
    return (ACE_Time_Value)(*this) == (ACE_Time_Value)(val);
  }
  
  inline
  bool
  Moment::operator!=(const Moment& val) const throw()
  {
    return !(*this == val);
  }

  inline
  bool
  Moment::operator<(const Moment& val) const throw()
  {
    return (ACE_Time_Value)(*this) < (ACE_Time_Value)(val);
  }
  
  inline
  bool
  Moment::operator>(const Moment& val) const throw()
  {
    return (ACE_Time_Value)(*this) > (ACE_Time_Value)(val);
  }
  
  inline
  bool
  Moment::operator<=(const Moment& val) const throw()
  {
    return (ACE_Time_Value)(*this) <= (ACE_Time_Value)(val);
  }
  
  inline
  bool
  Moment::operator>=(const Moment& val) const throw()
  {
    return (ACE_Time_Value)(*this) >= (ACE_Time_Value)(val);
  }
  
  inline
  Moment
  Moment::operator+(const ACE_Time_Value& val) const throw()
  {
    return Moment(ACE_Time_Value(*this) + val);
  }

  inline
  Moment
  Moment::operator-(const ACE_Time_Value& val) const throw(Exception)
  {
    if(val > *this)
    {
      throw
        Exception("El::Moment::operator- : second argument > than first one");
    }
    
    return Moment(ACE_Time_Value(*this) - val);
  }

  inline
  ACE_Time_Value
  Moment::divide(const ACE_Time_Value& p1, unsigned long p2) throw()
  {
    unsigned long long usec = ((unsigned long long)p1.sec() * 1000000 +
      p1.usec()) / p2;

    return ACE_Time_Value(usec / 1000000, usec % 1000000);
  }
  
  inline
  void
  Moment::write(El::BinaryOutStream& bstr) const throw(El::Exception)
  {
    bstr.write_raw_bytes((unsigned char*)this, sizeof(*this));    
  }
  
  inline
  void
  Moment::read(El::BinaryInStream& bstr) throw(El::Exception)
  {
    bstr.read_raw_bytes((unsigned char*)this, sizeof(*this));
  }
}

#endif // _ELEMENTS_EL_MOMENT_HPP_
