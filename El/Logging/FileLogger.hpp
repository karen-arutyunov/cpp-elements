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
 * @file Elements/El/Logging/FileLogger.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LOGGING_FILELOGGER_HPP_
#define _ELEMENTS_EL_LOGGING_FILELOGGER_HPP_

#include <stdio.h>

#include <string>
#include <list>
#include <iostream>

#include <El/Exception.hpp>
#include <El/Moment.hpp>
#include <El/RefCount/All.hpp>
#include <El/Service/Timer.hpp>

#include <El/Logging/LoggerBase.hpp>
 
namespace El
{
  namespace Logging
  {
    class FileLogger : public virtual LoggerBase,
                       public virtual El::Service::Callback
    {
    public:
      
      class RotatingPolicy : public virtual El::RefCount::Interface
      {
      public:
        virtual bool need_rotate(const FileLogger& logger) const throw() = 0;
      };

      typedef El::RefCount::SmartPtr<RotatingPolicy> RotatingPolicy_var;

      class RotatingByTimePolicy : public virtual RotatingPolicy,
                                   public virtual El::RefCount::DefaultImpl<>
      {
      public:
        RotatingByTimePolicy(const ACE_Time_Value& period) throw();
        virtual bool need_rotate(const FileLogger& logger) const throw();

        virtual ~RotatingByTimePolicy() throw() {}

      protected:
        ACE_Time_Value period_;
      };

      class RotatingBySizePolicy : public virtual RotatingPolicy,
                                   public virtual El::RefCount::DefaultImpl<>
      {
      public:
        RotatingBySizePolicy(unsigned long long size) throw();        
        virtual bool need_rotate(const FileLogger& logger) const throw();
        
      protected:
        unsigned long long size_;
      };

      typedef std::list<RotatingPolicy_var> RotatingPolicyList;
      
    public:
      
      FileLogger(const char* filename,
                 unsigned long level = DEBUG,
                 const char* aspects="*",
                 const RotatingPolicyList* polices = 0,
                 El::Moment::TimeZone zone = El::Moment::TZ_GMT,
                 std::ostream* error_stream = &std::cerr,
                 Formatter* formatter = 0,
                 const ACE_Time_Value& check_rotate_period = ACE_Time_Value(10),
                 unsigned long aspect_threshold = TRACE)
        throw(InvalidArgument, Exception, El::Exception);

      virtual ~FileLogger() throw();

      void stop_rotator() throw(El::Exception);

    protected:
      
      virtual void write(const char* line, bool newline, bool lock)
        throw(Exception, El::Exception);
      
      virtual bool notify(El::Service::Event* event) throw(El::Exception);

      void open() throw(InvalidArgument, Exception, El::Exception);
      void close() throw();
      
      bool write_line(const char* line, bool newline, bool lax_on_failure)
        throw(Exception, El::Exception);
      
      void rotate_if_required() throw(Exception, El::Exception);
      
    protected:
      std::string filename_;
      RotatingPolicyList polices_;
      ACE_Time_Value check_rotate_period_;

      FILE* output_;
      ACE_Time_Value created_;
      
      struct stat64 stat_;

      El::Service::Timer_var rotator_;

    private:
      FileLogger(const FileLogger&);
      void operator=(const FileLogger&);
    };
    
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Logging
  {
    //
    // FileLogger::RotatingByTimePolicy class
    //    
    inline
    FileLogger::RotatingByTimePolicy::RotatingByTimePolicy(
      const ACE_Time_Value& period) throw()
        : period_(period)
    {
    }

    //
    // FileLogger::RotatingBySizePolicy class
    //    
    inline
    FileLogger::RotatingBySizePolicy::RotatingBySizePolicy(
      unsigned long long size) throw()
        : size_(size)
    {
    }
  }
}

#endif // _ELEMENTS_EL_LOGGING_FILELOGGER_HPP_
