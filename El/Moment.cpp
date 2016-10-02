/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Moment.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <iomanip>
#include <sstream>

#include <El/String/Manip.hpp>

#include "Moment.hpp"

namespace
{
  static char WHITESPACES[] = " \t\n\r";
  static char WHITESPACES_ZzPM[] = " \t\n\rZz+-";
  static char WHITESPACES_T[] = " \t\n\rT";
  
}

namespace El
{
  const Moment Moment::null;

  Moment::Moment(const ACE_Time_Value& time, TimeZone tz)
    throw (InvalidArg, Exception, El::Exception)
  {
    time_t sec = time.sec();

    if(sec == 0)
    {
      *this = null;
      return;
    }

    if (tz == TZ_GMT)
    {
      if(ACE_OS::gmtime_r(&sec, this) == 0)
      {
        std::ostringstream ostr;
        ostr << "El::Moment::Moment: "
          "gmtime_r(" << sec << ") failed";

        throw InvalidArg(ostr.str().c_str());
      }
    }
    else
    {
      if(ACE_OS::localtime_r(&sec, this) == 0)
      {
        std::ostringstream ostr;
        ostr << "El::Moment::Moment: "
          "localtime_r(" << sec << ") failed";

        throw InvalidArg(ostr.str().c_str());
      }

    }

    tm_usec = time.usec();
    tm_tz = tz;
  }

  Moment::Moment(const char* value, TimeFormat format, TimeZone tz)
    throw (InvalidArg, Exception, El::Exception)
      : tm_tz(tz)
  {
    set(value, format);
  }

  void
  Moment::set(const char* value, TimeFormat format)
    throw (InvalidArg, Exception, El::Exception)
  {
    switch(format)
    {
    case TF_ISO_8601: set_iso8601(value); break;
    case TF_RFC_0822: set_rfc0822(value); break;
    case TF_FROM_EPOCH: set_from_epoch(value); break;
    }
  }

  void
  Moment::set_from_epoch(const char* str)
    throw(InvalidArg, Exception, El::Exception)
  {
    uint64_t sec = 0;
    uint64_t usec = 0;

    const char* p = strchr(str, '.');

    if(p)
    {
      std::string s(str, p - str);

      if(!El::String::Manip::numeric(s.c_str(), sec) ||
         !El::String::Manip::numeric(p + 1, usec))
      {
        std::ostringstream ostr;
        ostr << "El::Moment::set_from_epoch: invalid time '"
             << str << "'";
    
        throw InvalidArg(ostr.str().c_str());
      }
    }
    else
    {
      if(!El::String::Manip::numeric(str, sec))
      {
        std::ostringstream ostr;
        ostr << "El::Moment::set_from_epoch: invalid time '"
             << str << "'";
    
        throw InvalidArg(ostr.str().c_str());
      }
    }

    *this = Moment(ACE_Time_Value(sec, usec), tm_tz);
  }
  
  void
  Moment::set_iso8601(const char* str)
    throw(InvalidArg, Exception, El::Exception)
  {
    *this = null;
    
    bool valid_time = false;
    const char* ptr = str + strspn(str, WHITESPACES);

    size_t pos = strcspn(ptr, WHITESPACES_T);

    int year = 0;
    int month = 0;
    int day = 0;
    
    bool month_read = false;
    bool day_read = false;
    
    if(pos >= 4)
    {
      //
      // Reading year
      //
      std::string val;
      val.assign(ptr, 4);

      char* end = 0;
      year = strtol(val.c_str(), &end, 10);

      valid_time = (*end == '\0' && year > 1900) || year == 0;

      ptr += 4;
      pos -= 4;

      if(pos)
      {
        if(*ptr == '-')
        {
          ptr++;
          pos--;
        }
        else
        {
          valid_time = false;
        }  
      }
    }
    else
    {
      valid_time = false;
    }

    if(valid_time && pos)
    {
      if(pos >= 2)
      {
        std::string val;
        val.assign(ptr, 2);
        
        char* end = 0;
        month = strtol(val.c_str(), &end, 10);
        
        month_read = true;
        
        valid_time = *end == '\0' &&
          ((month >= 1 && month <= 12) || (!year && !month));
      }
      
      ptr += 2;
      pos -= 2;

      if(pos)
      {
        if(*ptr == '-')
        {
          ptr++;
          pos--;
        }
        else
        {
          valid_time = false;
        }  
      }
    }
    
    if(valid_time && pos)
    {
      if(pos == 2)
      {
        std::string val;
        val.assign(ptr, pos);

        char* end = 0;
        day = strtol(val.c_str(), &end, 10);
        
        day_read = true;
        
        valid_time = *end == '\0' && ((day >= 1 && day <= 31) ||
                                      (!year && !month && !day));
      }
      else
      {
        valid_time = false;
      }
    }
    
    valid_time = valid_time && (!month_read || !month == !year) &&
      (!day_read || !day == !year);

    if(!valid_time)
    {
      std::ostringstream ostr;
      ostr << "El::Moment::set_iso8601: invalid time '"
           << str << "'";
    
      throw InvalidArg(ostr.str().c_str());
    }

    tm_year = year ? year - 1900 : 0;

    if(month_read)
    {
      tm_mon = month ? month - 1 : 0;

      if(day_read)
      {
        tm_mday = day;
      }
    }

    ptr += pos;
    ptr += strspn(ptr, WHITESPACES);

    if(*ptr == '\0')
    {
      return;
    }

    set_time_iso8601(ptr);
  }
  
  void
  Moment::set_time_iso8601(const char* str)
    throw(InvalidArg, Exception, El::Exception)
  {
    bool valid_time = false;
    const char* ptr = str + strspn(str, WHITESPACES_T);

    size_t pos = strcspn(ptr, WHITESPACES_ZzPM);

    int hour = 0;
    int minute = 0;
    int sec = 0;
    int usec = 0;

//    bool sec_read = false;
//    bool usec_read = false;
    
    if(pos == 5 || (pos > 5 && ptr[5] == ':'))
    {
      if(ptr[2] == ':')
      {
        std::string hval;
        hval.assign(ptr, 2);

        char* hend = 0;
        hour = strtol(hval.c_str(), &hend, 10);

        std::string mval;
        mval.assign(ptr + 3, 2);

        char* mend = 0;
        minute = strtol(mval.c_str(), &mend, 10);

        valid_time = *hend == '\0' && *mend == '\0' && hour >= 0 &&
          hour < 24 && minute >= 0 && minute < 60;
      }
      else
      {
        std::ostringstream ostr;
        ostr << "El::Moment::set_time_iso8601: invalid time '"
             << str << "'";
    
        throw InvalidArg(ostr.str().c_str());      
      }
      
      if(pos > 5)
      {
        ptr += 6;
        pos -= 6;

        if(pos == 2 || (pos > 2 && ptr[2] == '.'))
        {
          std::string val;
          val.assign(ptr, 2);

          char* end = 0;
          sec = strtol(val.c_str(), &end, 10);

//          sec_read = true;

          valid_time = valid_time && *end == '\0' && sec >= 0 && sec < 60;
        }
        else
        {
          std::ostringstream ostr;
          ostr << "El::Moment::set_time_iso8601: invalid time '"
               << str << "'";
    
          throw InvalidArg(ostr.str().c_str());      
        }

        if(pos > 2)
        {
          ptr += 2;
          pos -= 2;

          double second_frac = atof(ptr);
          usec = (int)(1000000.0 * second_frac);
//          usec_read = true;

          valid_time = valid_time && usec >= 0 && usec < 1000000;
        }
      }
    }
    else
    {
      std::ostringstream ostr;
      ostr << "El::Moment::set_time_iso8601: invalid time '"
           << str << "'";
    
      throw InvalidArg(ostr.str().c_str());      
    }
    
    if(!valid_time)
    {
      std::ostringstream ostr;
      ostr << "El::Moment::set_time_iso8601: invalid time '"
           << str << "'";
    
      throw InvalidArg(ostr.str().c_str());
    }

    tm_hour = hour;
    tm_min = minute;
    tm_sec = sec;
    tm_usec = usec;

    ptr += pos;
    ptr += strspn(ptr, WHITESPACES);

    if(*ptr == '\0')
    {
      normalize();
      return;
    }

    valid_time = false;
    pos = strcspn(ptr, WHITESPACES);

    if(pos == 1)
    {
      if(*ptr == 'Z' || *ptr == 'z')
      {
        tm_tz = TZ_GMT;
        normalize();
        return;
      }
    }
    else if(pos == 6)
    {
      std::string hval;
      hval.assign(ptr + 1, 2);

      char* hend = 0;
      hour = strtol(hval.c_str(), &hend, 10);

      std::string mval;
      mval.assign(ptr + 4, 2);

      char* mend = 0;
      minute = strtol(mval.c_str(), &mend, 10);

      valid_time = *hend == '\0' && *mend == '\0' &&
        (*ptr == '+' || *ptr == '-') &&
        hour >= 0 && hour < 24 && minute >= 0 && minute < 60;

      if(valid_time)
      {
        int shift = (*ptr == '-' ? -1 : 1) * (60 * hour + minute);

        ACE_Time_Value tmp = *this;
        tmp -= ACE_Time_Value(shift * 60);
        *this = Moment(tmp, TZ_GMT);
        normalize();
        return;
      }
    }
    
    std::ostringstream ostr;
    ostr << "El::Moment::set_time_iso8601: invalid time '"
         << str << "'";
    
    throw InvalidArg(ostr.str().c_str());    
  }
  
  void
  Moment::set_rfc0822(const char* str)
    throw(InvalidArg, Exception, El::Exception)
  {
    *this = null;

    bool valid_time = false;
    const char* ptr = str + strspn(str, WHITESPACES);

    size_t pos = strcspn(str, WHITESPACES);

    if(pos == 4 && ptr[pos - 1] == ',')
    {
      ptr += pos + 1;
      ptr += strspn(ptr, WHITESPACES);
    }

    pos = strcspn(ptr, WHITESPACES);
    std::string val;
    val.assign(ptr, pos);

    int day = atol(val.c_str());

    if(day > 0 && day <= 31)
    {
      ptr += pos + 1;
      ptr += strspn(ptr, WHITESPACES);
      pos = strcspn(ptr, WHITESPACES);
      val.assign(ptr, pos);
        
      try
      {
        int month = El::Moment::month(val.c_str());

        ptr += pos + 1;
        ptr += strspn(ptr, WHITESPACES);
        pos = strcspn(ptr, WHITESPACES);
        val.assign(ptr, pos);
          
        int year = atol(val.c_str());

        if(year >= 1900)
        {
          ptr += pos + 1;
          ptr += strspn(ptr, WHITESPACES);
          pos = strcspn(ptr, WHITESPACES);

          if(pos >= 5 && ptr[2] == ':')
          {
            val.assign(ptr, 2);
            int hour = atol(val.c_str());

            val.assign(ptr + 3, 2);
            int min = atol(val.c_str());
            int sec = 0;
              
            if(pos == 8)
            {
              sec = atol(ptr + 6);
            }

            if(hour >= 0 && hour <= 24 && min >= 0 && min <= 60 &&
               sec >= 0 && sec <= 60)
            {
              Moment tm;
              tm.tm_year = year - 1900; 
              tm.tm_mon = month;
              tm.tm_mday = day;
              tm.tm_hour = hour;
              tm.tm_min = min;
              tm.tm_sec = sec;
              tm.tm_usec = 0;
              tm.tm_tz = TZ_GMT;
                
              ptr += pos + 1;
              ptr += strspn(ptr, WHITESPACES);
              pos = strcspn(ptr, WHITESPACES);

              val.assign(ptr, pos);

              int shift = 0;
                
              valid_time = true;
                
              if(val == "UT" || val == "GMT")
              {
                // do nothing
              }
              else if(val == "EST" || val == "CDT")
              {
                shift = -5 * 60;
              }
              else if(val == "EDT")
              {
                shift = -4 * 60;
              }
              else if(val == "CST" || val == "MDT")
              {
                shift = -6 * 60;
              }
              else if(val == "MST" || val == "PDT")
              {
                shift = -7 * 60;
              }
              else if(val == "PST")
              {
                shift = -8 * 60;
              }
              else if(pos == 5)
              {
                val.assign(ptr + 1, 2);
                  
                shift = (*ptr == '-' ? -1 : 1) *
                  (60 * atol(val.c_str()) + atol(ptr + 3));
              }
              else if(pos == 1)
              {
                if(*ptr >= 'A' && *ptr <= 'M')
                {
                  if(*ptr == 'J')
                  {
                    valid_time = false;
                  }
                  else
                  {
                    shift = -60 * (*ptr - 'A' + 1) + (*ptr > 'J' ? 1 : 0);
                  }
                }
                else if(*ptr >= 'N' && *ptr <= 'Y')
                {
                  shift = 60 * (*ptr - 'N' + 1);
                }
                else if(*ptr != 'Z')
                {
                  valid_time = false;
                }
              }
              else
              {
                valid_time = false;
              }

              if(valid_time)
              {
                if(shift == 0)
                {
                  *this = tm;
                }
                else
                {
                  ACE_Time_Value tmp = tm;
                  tmp -= ACE_Time_Value(shift * 60);
                  *this = Moment(tmp, TZ_GMT);
                }

                normalize();
              }
                
            }
          }
        }
      }
      catch(...)
      {
      }
    }

    if(!valid_time)
    {
      std::ostringstream ostr;
      ostr << "El::Moment::set_rfc0822: invalid time '"
           << str << "'";
    
      throw InvalidArg(ostr.str().c_str());
    }      
  }
  
  inline
  uint8_t
  Moment::month(const char* mon) throw(InvalidArg, Exception, El::Exception)
  {
    if(!mon)
    {
      throw InvalidArg("El::Moment::month: nil month specified");
    }

    if(!strcasecmp(mon, "Jan"))
    {
      return 0;
    }
    else if(!strcasecmp(mon, "Feb"))
    {
      return 1;
    }
    else if(!strcasecmp(mon, "Mar"))
    {
      return 2;
    }
    else if(!strcasecmp(mon, "Apr"))
    {
      return 3;
    }
    else if(!strcasecmp(mon, "May"))
    {
      return 4;
    }
    else if(!strcasecmp(mon, "Jun"))
    {
      return 5;
    }
    else if(!strcasecmp(mon, "Jul"))
    {
      return 6;
    }
    else if(!strcasecmp(mon, "Aug"))
    {
      return 7;
    }
    else if(!strcasecmp(mon, "Sep"))
    {
      return 8;
    }
    else if(!strcasecmp(mon, "Oct"))
    {
      return 9;
    }
    else if(!strcasecmp(mon, "Nov"))
    {
      return 10;
    }
    else if(!strcasecmp(mon, "Dec"))
    {
      return 11;
    }
    else
    {
      std::ostringstream ostr;
      ostr << "El::Moment::month: invalid month specified '" << mon << "'";

      throw InvalidArg(ostr.str());
    }    
  }

  inline
  const char*
  Moment::month(uint8_t month) throw(InvalidArg, Exception, El::Exception)
  {
    if(month > 11)
    {
      std::ostringstream ostr;
      ostr << "El::Moment::month: invalid month specified '" <<
        month << "'";

      throw InvalidArg(ostr.str());
    }

    static const char* months[] = 
      {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
      };

    return months[month];
  }

  inline
  uint8_t
  Moment::week_day(const char* day) throw(InvalidArg, Exception, El::Exception)
  {
    if(!day)
    {
      throw InvalidArg("El::Moment::week_day: day is null");
    }

    if(!strcasecmp(day, "Sun"))
    {
      return 0;
    }
    else if(!strcasecmp(day, "Mon"))
    {
      return 1;
    }
    else if(!strcasecmp(day, "Tue"))
    {
      return 2;
    }
    else if(!strcasecmp(day, "Wed"))
    {
      return 3;
    }
    else if(!strcasecmp(day, "Thu"))
    {
      return 4;
    }
    else if(!strcasecmp(day, "Fri"))
    {
      return 5;
    }
    else if(!strcasecmp(day, "Sat"))
    {
      return 6;
    }
    else
    {
      std::ostringstream ostr;
      ostr << "El::Moment::week_day: invalid day specified '" << day << "'";

      throw InvalidArg(ostr.str());
    }    
  }

  const char*
  Moment::week_day(uint8_t day) throw(InvalidArg, Exception, El::Exception)
  {
    if(day > 6)
    {
      std::ostringstream ostr;
      ostr << "El::Moment::week_day: invalid day specified '" << day << "'";

      throw InvalidArg(ostr.str());
    }

    static const char* days[] = 
      {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
      };

    return days[day];
  }
  
  Moment::operator ACE_Time_Value() const throw ()
  {
    time_t sec = 0;
    Moment tmp = *this;

    bool is_time = false;

    if(!tmp.tm_year && !tmp.tm_mon && !tmp.tm_mday)
    {
      is_time = true;
      tmp = Moment(100, 1, 1, tm_hour, tm_min, tm_sec, tm_usec, tm_tz);
    }
    
    switch (tm_tz)
    {
    case TZ_LOCAL:
      sec = ::mktime(&tmp);
      break;
    case TZ_GMT:
      sec = ::timegm(&tmp);
      break;
    }

    if(!is_time)
    {
      return ACE_Time_Value(sec, tm_usec);
    }

    ACE_Time_Value tm(sec, tm_usec);
    
    return tm - ACE_Time_Value(Moment(tm).date());
  }

  void
  Moment::normalize() throw(Exception, El::Exception)
  {
    if(tm_year && tm_mday)
    {      
      time_t res;
      if (tm_tz == TZ_GMT)
      {
        res = timegm(this);
      }
      else
      {
        res = mktime(this);
      }
    
      if (res == (time_t)(-1))
      {
        throw Exception("El::Moment::normalize: can't normalize");
      }
    }
    
  }
  
  std::string
  Moment::iso8601(bool timezone, bool time) const
    throw(Exception, El::Exception)
  {
    std::ostringstream ostr;

    if(tm_year)
    {
      ostr << tm_year + 1900 << "-";

      ostr.width(2);
      ostr.fill('0');
      ostr << tm_mon + 1 << "-";
    
      ostr.width(2);
      ostr.fill('0');
      ostr << tm_mday;
    }
    else
    {
      ostr << "0000-00-00";
    }

    if(time)
    {
      ostr << " ";
      
      ostr.width(2);
      ostr.fill('0');
      ostr << tm_hour << ":";
    
      ostr.width(2);
      ostr.fill('0');
      ostr << tm_min << ":";

      ostr.width(2);
      ostr.fill('0');
      ostr << tm_sec;

      if(tm_usec)
      {
        std::ostringstream usec_ostr;
      
        usec_ostr.width(6);
        usec_ostr.fill('0');      
        usec_ostr << tm_usec;

        std::string usec = usec_ostr.str();
        int i = usec.length() - 1;
      
        for(; i > 0 && usec[i] == '0'; i--);

        if(i >= 0)
        {
          usec.resize(i + 1);
        }

        ostr << "." << usec;
      }
    }
    
    if(timezone)
    {  
      if(tm_tz == TZ_GMT)
      {
        ostr << " Z";
      }
      else
      {
        tm tmp;
        time_t sec = 0;
      
        if(ACE_OS::localtime_r(&sec, &tmp) == 0)
        {
          throw Exception("El::Moment::iso8601: localtime_r failed");
        }
      
        ostr << " " << (tmp.tm_gmtoff < 0 ? "-" : "+");

        ostr.width(2);
        ostr.fill('0');
        ostr << tmp.tm_gmtoff / 3600 << ":";
      
        ostr.width(2);
        ostr.fill('0');
        ostr << tmp.tm_gmtoff / 60 % 60;
      }
    }
    
    return ostr.str();
  }

  std::string
  Moment::http_cookie_expiration() const throw(Exception, El::Exception)
  {
    if(tm_tz == TZ_LOCAL)
    {
      return Moment(ACE_Time_Value(*this), TZ_GMT).http_cookie_expiration();
    }
      
    std::ostringstream ostr;
    ostr << week_day(tm_wday) << ", " << tm_mday << "-" << month(tm_mon)
         << "-" << 1900 + tm_year << " " << std::setw(2) << std::setfill('0')
         << tm_hour << ":" << std::setw(2) << tm_min << ":" << std::setw(2)
         << tm_sec << " GMT";

    return ostr.str();
  }
  
  std::string
  Moment::dense_format(unsigned long flags) const
    throw(Exception, El::Exception)
  {
    std::ostringstream ostr;

    if(flags & DF_DATE)
    {
      ostr << tm_year + 1900 << std::setw(2) << std::setfill('0')
           << tm_mon + 1 << std::setw(2) << tm_mday;
      
      if(flags & DF_TIME)
      {
        ostr << ".";
      }
    }
    
    if(flags & DF_TIME)
    {
      ostr << std::setw(2) << tm_hour << std::setw(2) << tm_min
           << std::setw(2) << tm_sec;
      
      if(flags & DF_USEC)
      {
        ostr << std::setw(6) << tm_usec;
      }
    }

    return ostr.str();
  }

  std::string
  Moment::epoch(bool usec) const throw(Exception, El::Exception)
  {
    ACE_Time_Value tm(*this);
    
    std::ostringstream ostr;
    ostr << tm.sec();

    if(usec)
    {
      ostr << ".";
      ostr.width(6);
      ostr.fill('0');
      ostr << tm.usec();
    }

    return ostr.str();
  }
  
  std::string
  Moment::rfc0822(bool timezone, bool usec) const
    throw(Exception, El::Exception)
  {
    std::ostringstream ostr;
    ostr << week_day(tm_wday) << ", " << tm_mday << " " << month(tm_mon)
         << " " << 1900 + tm_year << " ";
    
    ostr.width(2);
    ostr.fill('0');
    ostr << tm_hour << ":";
    
    ostr.width(2);
    ostr.fill('0');
    ostr << tm_min << ":";

    ostr.width(2);
    ostr.fill('0');
    ostr << tm_sec;

    if(usec)
    {
      ostr << "." << std::setw(6) << std::setfill('0') << tm_usec;
    }

    if(timezone)
    {
      ostr << " ";

      if(tm_tz == TZ_GMT)
      {
        ostr << "GMT";
      }
      else
      {
        tm tmp;
        time_t sec = 0;
      
        if(ACE_OS::localtime_r(&sec, &tmp) == 0)
        {
          throw Exception("El::Moment::iso8601: localtime_r failed");
        }
      
        ostr << (tmp.tm_gmtoff < 0 ? "-" : "+");

        ostr.width(2);
        ostr.fill('0');
        ostr << abs(tmp.tm_gmtoff) / 3600;
      
        ostr.width(2);
        ostr.fill('0');
        ostr << abs(tmp.tm_gmtoff) / 60 % 60;
      }
    }
    
    return ostr.str();
  }
  
  std::string
  Moment::time(const ACE_Time_Value& val, bool days) throw(El::Exception)
  {
    std::ostringstream ostr;
    uint64_t sec = val.sec();

    if(days)
    {
      uint64_t d = sec / 86400;
      sec -= d * 86400;
      ostr << d << "-";
    }
    
    ostr.width(2);
    ostr.fill('0');
    ostr << sec / 3600 << ":";
    
    ostr.width(2);
    ostr.fill('0');
    ostr << (sec % 3600) / 60 << ":";

    ostr.width(2);
    ostr.fill('0');
    ostr << sec % 60 << ".";

    ostr.width(6);
    ostr.fill('0');      
    ostr << val.usec();

    return ostr.str();
  }
}
