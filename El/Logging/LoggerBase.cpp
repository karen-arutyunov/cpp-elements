/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Logging/LoggerBase.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string>
#include <sstream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/String/ListParser.hpp>

#include "LoggerBase.hpp"

namespace El
{
  namespace Logging
  {
    LoggerBase::LoggerBase(unsigned long level,
                           const char* aspects_val,
                           El::Moment::TimeZone zone,
                           std::ostream* error_stream,
                           Formatter* formatter,
                           unsigned long aspect_threshold)
      throw(Exception, El::Exception)
        : level_(level),
          zone_(zone),
          error_stream_(error_stream),
          formatter_(El::RefCount::add_ref(formatter)),
          aspect_threshold_(aspect_threshold)
    {
      if(formatter_.in() == 0)
      {
        formatter_ = new SimpleFormatter();
      }
             
      aspects(aspects_val);
    }

    void
    LoggerBase::aspects(const char* aspects) throw(El::Exception)
    {
      El::String::Manip::trim(aspects, aspects_);
      aspect_table_.clear();

      if(aspects_ != "*")
      {
        El::String::ListParser parser(aspects_.c_str(), ", \t\r\n");

        const char* item = 0;
        while((item = parser.next_item()) != 0)
        {
          aspect_table_.insert(item);
        }
      }
    }

    void
    LoggerBase::raw_log(const char* text) throw()
    {
      try
      {
        WriteGuard guard(lock_);
        write(text, false, false);
      }
      catch(const El::Exception& e)
      {
        if(error_stream_)
        {
          try
          {
            *error_stream_ << "El::Logging::LoggerBase::log: "
              "eh::Exception caught. Description:" << std::endl << e
                      << std::endl;
          }
          catch(...)
          {
            // Nothing we can do here
          }
        }
      }
    }
    
    void
    LoggerBase::log(const char* text,
                    unsigned long severity,
                    const char* aspect)
      throw()
    {
      try
      {
        WriteGuard guard(lock_);

        if(severity > level_ ||
           (aspects_ != "*" &&
            severity >= aspect_threshold_ && 
            aspect_table_.find(aspect) == aspect_table_.end()))
        {
          return;
        }

        time_ = El::Moment(ACE_OS::gettimeofday(), zone_);
        
        std::string line = formatter_->line(text, severity, aspect, time_);

        if(!line.empty())
        {
          write(line.c_str(), true, false);
        }
        
      }
      catch(const El::Exception& e)
      {
        if(error_stream_)
        {
          try
          {
            *error_stream_ << "El::Logging::LoggerBase::log: "
              "eh::Exception caught. Description:" << std::endl << e
                      << std::endl;
          }
          catch(...)
          {
            // Nothing we can do here
          }
        }
      }
    }

    //
    // LoggerBase::SimpleFormatter class
    //
    std::string
    SimpleFormatter::line(const char* text,
                          unsigned long severity,
                          const char* aspect,
                          const El::Moment& time)
      throw(El::Exception)
    {
      std::ostringstream ostr;
      
      if(field_presence_mask_ & FP_TIME)
      {
        ostr << time.rfc0822(false, true) << " ";
      }

      if(field_presence_mask_ & FP_SEVERITY)
      {
        switch(severity)
        {
        case EMERGENCY: ostr << "[EMERGENCY] "; break;
        case ALERT: ostr << "[ALERT] "; break;
        case ERROR: ostr << "[ERROR] "; break;
        case CRITICAL: ostr << "[CRITICAL] "; break;
        case WARNING: ostr << "[WARNING] "; break;
        case NOTICE: ostr << "[NOTICE] "; break;
        case INFO: ostr << "[INFO] "; break;
        case DEBUG: ostr << "[DEBUG] "; break;
        default: ostr << "[TRACE " << severity - TRACE << "] ";
        }
      }

      if(field_presence_mask_ & FP_ASPECT)
      {
        ostr << "(" << (aspect ? aspect : "") << ") ";
      }
      
      if(field_presence_mask_ & FP_MESSAGE)
      {
        if(field_presence_mask_ & ~FP_MESSAGE)
        {
          ostr << ": ";
        }

        if((field_presence_mask_ & FP_LINE_IDENT) != 0 && text)
        {
          char c = 0;
          for(const char* p = text; (c = *p++) != '\0';)
          {
            ostr << c;
            
            if(c == '\n' && (c = *p) != '\n' && c != '\0' && c != '\r')
            {
              ostr << " ";
            }
          }
        }
        else
        {
          ostr << (text ? text : "");
        }
      }
      
      return ostr.str();
    }
    
  }
}
