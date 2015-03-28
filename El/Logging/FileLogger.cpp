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
 * @file Elements/El/Logging/FileLogger.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>

#include <El/Exception.hpp>
#include <El/Service/Timer.hpp>

#include "FileLogger.hpp"

namespace El
{
  namespace Logging
  {
    //
    // Rotate class
    //    

    class Rotate : public El::Service::Timer::ServiceEvent
    {
    public:
      Rotate(El::Service::Callback* callback) throw(El::Exception);
      virtual ~Rotate() throw();
    };
    
    Rotate::Rotate(El::Service::Callback* callback) throw(El::Exception)
        : El__Service__Timer__ServiceEventBase(callback, 0)
    {
    }
    
    Rotate::~Rotate() throw()
    {
    }
    
    //
    // FileLogger class
    //    

    FileLogger::FileLogger(const char* filename,
                           unsigned long level,
                           const char* aspects,
                           const RotatingPolicyList* polices,
                           El::Moment::TimeZone zone,
                           std::ostream* error_stream,
                           Formatter* formatter,
                           const ACE_Time_Value& check_rotate_period,
                           unsigned long aspect_threshold)
      throw(InvalidArgument, Exception, El::Exception)
        : LoggerBase(level,
                     aspects,
                     zone,
                     error_stream,
                     formatter,
                     aspect_threshold),
          filename_(filename ? filename : ""),
          check_rotate_period_(check_rotate_period),
          output_(0)
    {
      if(filename_.empty())
      {
        throw InvalidArgument(
          "El::Logging::FileLogger::FileLogger: file name undefined");
      }

      if(polices)
      {
        polices_ = *polices;
      }

      memset(&stat_, 0, sizeof(stat_));
      if(stat64(filename_.c_str(), &stat_) == 0 && stat_.st_size > 0)
      {
        time_ = ACE_OS::gettimeofday();
        rotate_if_required();
      }

      if(output_ == 0)
      {
        open();
      }
      
    }
    
    FileLogger::~FileLogger() throw()
    {
      try
      {
        stop_rotator();
      }
      catch(const El::Exception& e)
      {
        if(error_stream_)
        {
          *error_stream_ << "El::FileLogger::~FileLogger: "
            "stop_rotator failed. Reason:\n" << e << std::endl;
        }
      }
        
      close();
    }

    void
    FileLogger::write(const char* line, bool newline, bool lock)
      throw(Exception, El::Exception)
    {
      std::auto_ptr<WriteGuard> guard(lock ? new WriteGuard(lock_) : 0);
      
      if(rotator_.in() == 0 && check_rotate_period_ != ACE_Time_Value::zero)
      {
        rotator_ = new El::Service::Timer(this);

        El::Service::Timer::Event_var event = new Rotate(this);
        
        rotator_->set(event.in(),
                      ACE_OS::gettimeofday() + check_rotate_period_);

        rotator_->start();        
      }
        
      rotate_if_required();
      
      if(!write_line(line, newline, true))
      {
        open();
        write_line(line, newline, false);
      }
    }

    void
    FileLogger::stop_rotator() throw(El::Exception)
    {
      if(rotator_.in() != 0)
      {
        rotator_->stop();
        rotator_->wait();
      }
    }
    
    bool
    FileLogger::notify(El::Service::Event* event) throw(El::Exception)
    {
      Rotate* rotate = dynamic_cast<Rotate*>(event);

      if(rotate != 0)
      {
        {
          WriteGuard guard(lock_);
          
          memset(&stat_, 0, sizeof(stat_));
          if(stat64(filename_.c_str(), &stat_) == 0 && stat_.st_size > 0)
          {
            time_ = ACE_OS::gettimeofday();
            rotate_if_required();
          }
        }
        
        rotator_->set(rotate, ACE_OS::gettimeofday() + check_rotate_period_);
        return true;
      }
        
      El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);

      if(error != 0 && error_stream_)
      {
        *error_stream_ << "El::FileLogger::notify: error(" << error->severity
                       << "). Description: " << error->description
                       << std::endl;
        
        return true;
      }

      return false;
    }
    
    void
    FileLogger::open() throw(InvalidArgument, Exception, El::Exception)
    {
      close();

      output_ = ::fopen64(filename_.c_str(), "a");
      
      if(output_ == 0)
      {
        std::ostringstream ostr;
        ostr << "El::Logging::FileLogger: failed to open file '"
             << filename_ << "'";
        
        throw InvalidArgument(ostr.str());
      }

      created_ = ACE_OS::gettimeofday();
      memset(&stat_, 0, sizeof(stat_));
    }
    
    bool
    FileLogger::write_line(const char* line, bool newline, bool lax_on_failure)
      throw(Exception, El::Exception)
    {
      memset(&stat_, 0, sizeof(stat_));
      
      if(output_ && fputs(line, output_) != EOF &&
         (newline ? fputs("\n", output_) != EOF : true) &&
         fflush(output_) != EOF &&
         stat64(filename_.c_str(), &stat_) == 0)
      {
        return true;
      }

      if(lax_on_failure)
      {
        return false;
      }
      
      std::ostringstream ostr;
      ostr << "El::Logging::FileLogger: writing failed for file '"
           << filename_ << "'";
        
      throw Exception(ostr.str());
    }
    
    void
    FileLogger::close() throw()
    {
      if(output_ != 0)
      {
        fclose(output_);
        output_ = 0;
      }
    }

    void
    FileLogger::rotate_if_required() throw(Exception, El::Exception)
    {
      for(RotatingPolicyList::const_iterator it = polices_.begin();
          it != polices_.end(); it++)
      {
        RotatingPolicy_var policy = *it;
        
        if(policy->need_rotate(*this))
        {
          close();

          std::ostringstream ostr;
          ostr << filename_ << "." << time_.dense_format();

          std::string new_name = ostr.str();
      
          if (ACE_OS::rename(filename_.c_str(), new_name.c_str()) != 0)
          {
            std::ostringstream ostr;
            ostr << "El::Logging::FileLogger::rotate_if_required: "
              "failed to rename file '" << filename_ << "'" << " to '"
                 << new_name << "'";

            throw Exception(ostr.str().c_str());
          }

          open();

          if((ACE_Time_Value)time_ != ACE_Time_Value::zero)
          {
            created_ = time_;
          }          
        }
      }
    }
    
    //
    // FileLogger::RotatingByTimePolicy class
    //
    bool
    FileLogger::RotatingByTimePolicy::need_rotate(
      const FileLogger& logger) const throw()
    {
      return (ACE_Time_Value)logger.time_ >= logger.created_ + period_ &&
        logger.stat_.st_size > 0 && period_ != ACE_Time_Value::zero;
    }
    
    //
    // FileLogger::RotatingBySizePolicy class
    //
    bool
    FileLogger::RotatingBySizePolicy::need_rotate(
      const FileLogger& logger) const throw()
    {
      return size_ && (unsigned long long)logger.stat_.st_size >= size_;
    }
    
  }  
}
