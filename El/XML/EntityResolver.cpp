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
 * @file Elements/El/XML/EntityResolver.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <stdint.h>
#include <utime.h>
#include <stdio.h>

#include <sstream>
#include <string.h>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/ArrayPtr.hpp>
#include <El/String/Manip.hpp>
#include <El/CRC.hpp>
#include <El/Moment.hpp>
#include <El/XML/InputStream.hpp>
#include <El/Net/HTTP/StatusCodes.hpp>
#include <El/Net/HTTP/Headers.hpp>
#include <El/FileSystem.hpp>

#include "EntityResolver.hpp"

namespace El
{
  namespace XML
  {
    const std::string MUTEX_NAME(
      std::string("ElementsXMLEntityResolverInputStreamLock_") +
      El::String::Manip::string(geteuid()));
    
    EntityResolver::InputSource::InputStream::Mutex
    EntityResolver::InputSource::InputStream::lock_(
      MUTEX_NAME.c_str());

    time_t
    EntityResolver::InputSource::InputStream::next_cache_cleanup_(0);

    //
    // ExpiredFileList class
    //
    class ExpiredFileList : public El::FileSystem::DirectoryReader
    {
    public:
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
    public:
      ExpiredFileList(const char* dir_path, time_t time_threshold)
        throw(Exception, El::Exception);

    private:
      virtual bool select(const struct dirent* dir) throw(El::Exception);

    private:
      time_t time_threshold_;
    };
    
    ExpiredFileList::ExpiredFileList(const char* dir_path,
                                     time_t time_threshold)
      throw(Exception, El::Exception)
        : time_threshold_(time_threshold)
    {
      read(dir_path);
    }
    
    bool
    ExpiredFileList::select(const struct dirent* dir) throw(El::Exception)
    {
      if(dir->d_type != DT_REG && dir->d_type != DT_LNK)
      {
        return false;
      }

      struct stat64 file_stat;
      std::string name = current_directory_ + "/" + dir->d_name;

      return ::stat64(name.c_str(), &file_stat) == 0 &&
        file_stat.st_atime < time_threshold_;
    }

    
    //
    // EntityResolver class
    //
    xercesc::InputSource*
    EntityResolver::resolveEntity(const XMLCh* const publicId,
                                  const XMLCh* const systemId)
      throw(xercesc::SAXException)
    {
      try
      {
        return new InputSource(publicId,
                               systemId,
                               net_strategy_,
                               file_strategy_);
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "El::XML::EntityResolver::resolveEntity: El::Exception caught."
          " Description:\n" << e;
        
        throw xercesc::SAXException(ostr.str().c_str());
      }
    }
    
    //
    // EntityResolver::InputSource class
    //
    EntityResolver::InputSource::InputSource(
      const XMLCh* const publicId,
      const XMLCh* const systemId,
      const NetStrategy& net_strategy,
      const FileStrategy& file_strategy)
      throw(El::Exception)
        : net_strategy_(net_strategy),
          file_strategy_(file_strategy)
    {
      if(publicId)
      {
        El::ArrayPtr<char> val(xercesc::XMLString::transcode(publicId));
        assert(val.get());
        
        pub_id_ = val.get();
      }

      if(systemId)
      {
        El::ArrayPtr<char> val(xercesc::XMLString::transcode(systemId));
        assert(val.get());
        
        sys_id_ = val.get();
      }
    }
    
    xercesc::BinInputStream*
    EntityResolver::InputSource::makeStream() const
      throw(xercesc::SAXException)
    {
      return new InputStream(pub_id_.c_str(),
                             sys_id_.c_str(),
                             net_strategy_,
                             file_strategy_);
    }

    //
    // EntityResolver::InputSource::InputStream class
    //
    EntityResolver::InputSource::InputStream::InputStream(
      const char* pub_id,
      const char* sys_id,
      const NetStrategy& net_strategy,
      const FileStrategy& file_strategy)
      throw(xercesc::SAXException)
    {
//      std::cerr << "IS: " << pub_id << "===" << sys_id << std::endl;

      try
      {
        if(file_strategy.cache_dir.empty())
        {
          create_network_stream(pub_id,
                                sys_id,
                                net_strategy);
        }
        else
        {
          create_file_stream(pub_id,
                             sys_id,
                             net_strategy,
                             file_strategy);
        }
        
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "El::XML::EntityResolver::InputSource::InputStream::"
          "InputStream: El::Exception caught. Description:\n" << e;
        
        throw xercesc::SAXException(ostr.str().c_str());
      }      
    }

    XMLFilePos
    EntityResolver::InputSource::InputStream::curPos() const throw()
    {
      return stream_->curPos();
    }

    XMLSize_t
    EntityResolver::InputSource::InputStream::readBytes(
      XMLByte* const toFill,
      const XMLSize_t maxToRead) throw()
    {
      return stream_->readBytes(toFill, maxToRead);
    }

    void
    EntityResolver::InputSource::InputStream::create_session(
      const char* pub_id,
      const char* sys_id,
      const NetStrategy& net_strategy)
      throw(xercesc::SAXException, El::Exception)
    {
      El::Net::HTTP::URL_var url = new El::Net::HTTP::URL(sys_id);
          
      session_.reset(new El::Net::HTTP::Session(url.in()));

      El::Net::HTTP::HeaderList headers;
      headers.add(El::Net::HTTP::HD_ACCEPT_ENCODING, "gzip,deflate");

      session_->open(net_strategy.connect_timeout.get(),
                     net_strategy.send_timeout.get(),
                     net_strategy.recv_timeout.get());

      session_->send_request(El::Net::HTTP::GET,
                             El::Net::HTTP::ParamList(),
                             headers,
                             0,
                             0,
                             net_strategy.redirects_to_follow);

      session_->recv_response_status();

      uint32_t status_code = session_->status_code();
        
      if(status_code != El::Net::HTTP::SC_OK)
      {
        std::string source_url = url->string();
        std::string curr_url = session_->url()->string();
          
        std::ostringstream ostr;
        ostr << "El::XML::EntityResolver::InputSource::InputStream::"
          "create_session: bad status for '" << curr_url << "'";

        if(source_url != curr_url)
        {
          ostr << " original url '" << source_url << "'";
        }

        ostr << ": " << status_code << ", " << session_->status_text();
        throw xercesc::SAXException(ostr.str().c_str());
      }

      El::Net::HTTP::Header header;  
      while(session_->recv_response_header(header));      
    }

    void
    EntityResolver::InputSource::InputStream::create_network_stream(
      const char* pub_id,
      const char* sys_id,
      const NetStrategy& net_strategy)
      throw(xercesc::SAXException, El::Exception)
    {
      create_session(pub_id,
                     sys_id,
                     net_strategy);
      
      stream_.reset(new El::XML::InputStream(session_->response_body()));
    }

    void
    EntityResolver::InputSource::InputStream::create_file_stream(
      const char* pub_id,
      const char* sys_id,
      const NetStrategy& net_strategy,
      const FileStrategy& file_strategy)
      throw(xercesc::SAXException, El::Exception)
    {
      std::string cache_file;
      std::string safe_url;
        
      {
        El::Net::HTTP::URL_var url = new El::Net::HTTP::URL(sys_id);
          
        std::ostringstream ostr;
        const char* chars = " ";
        El::String::Manip::replace(url->string(), "\n", &chars, ostr);
        safe_url = ostr.str();
      }

      uint64_t crc = 0;
      El::CRC(crc, (unsigned char*)safe_url.c_str(), safe_url.length());

      char name[17];
      sprintf(name, "%016llX", (unsigned long long)crc);
        
      cache_file = file_strategy.cache_dir + "/" + name;

      {
        Guard guard(lock_);
        
        struct stat64 file_stat;
        
        if(::stat64(cache_file.c_str(), &file_stat) == 0)
        {
          time_t now = ACE_OS::gettimeofday().sec();
          
          if(file_stat.st_mtime + file_strategy.cache_timeout > now)
          {
            file_.open(cache_file.c_str(), std::ios::in);

            if(!file_.is_open())
            {
              std::ostringstream ostr;
              ostr << "El::XML::EntityResolver::InputSource::InputStream::"
                "create_file_stream: can't open '" << cache_file
                   << "' for read access";
            
              throw xercesc::SAXException(ostr.str().c_str());
            }

            stream_.reset(new El::XML::InputStream(file_));
            return;
          }

          struct utimbuf utime_buf;
          utime_buf.actime = now;
          
          utime_buf.modtime =
            std::min(now,
                     now + file_strategy.cache_retry_period -
                     file_strategy.cache_timeout);
          
          utime(cache_file.c_str(), &utime_buf);
        }
      }
      
      std::string safe_pub_id;

      {
        const char* chars = " ";
        std::ostringstream ostr; 
        El::String::Manip::replace(pub_id, "\n", &chars, ostr);
        safe_pub_id = ostr.str();
      }        
 
      std::string cache_tmp_file;
      std::fstream file;
        
      try
      {
        create_session(pub_id,
                       sys_id,
                       net_strategy);
      
        std::istream& body_str(session_->response_body());

        {
          std::ostringstream ostr;
          ostr << cache_file << ".tmp."
               << El::Moment(ACE_OS::gettimeofday()).dense_format()
               << rand();

          cache_tmp_file = ostr.str();
        }
      
        file.open(cache_tmp_file.c_str(), std::ios::out);

        if(!file.is_open())
        {
          std::ostringstream ostr;
          ostr << "El::XML::EntityResolver::InputSource::InputStream::"
            "create_file_stream: can't open '" << cache_tmp_file
               << "' for write access";
            
          throw xercesc::SAXException(ostr.str().c_str());
        }

        size_t size = 0;
          
        while(!body_str.fail())
        {
          char buff[1024];
        
          body_str.read(buff, sizeof(buff));
          file.write(buff, body_str.gcount());
        
          if(file.fail())
          {
            std::ostringstream ostr;
            ostr << "El::XML::EntityResolver::InputSource::InputStream::"
              "create_file_stream: failed to write to '" << cache_tmp_file
                 << "'";
          
            throw xercesc::SAXException(ostr.str().c_str());
          }

          size += body_str.gcount();

          if(size > file_strategy.max_file_size)
          {
            file.close();
            unlink(cache_tmp_file.c_str());
              
            std::ostringstream ostr;
            ostr << "El::XML::EntityResolver::InputSource::InputStream::"
              "create_file_stream: file '" << cache_tmp_file
                 << "' size > " << file_strategy.max_file_size;
          
            throw xercesc::SAXException(ostr.str().c_str());
          }
        }

        session_->test_completion();
        session_.reset(0);

        if(file.bad() || file.fail())
        {
          std::ostringstream ostr;
          ostr << "El::XML::EntityResolver::InputSource::InputStream::"
            "create_file_stream: failed to write to '" << cache_tmp_file
               << "'";
        
          throw xercesc::SAXException(ostr.str().c_str());
        }
          
        file.close();
        rename(cache_tmp_file.c_str(), cache_file.c_str());
      }
      catch(...)
      {
        if(file.is_open())
        {
          file.close();
          unlink(cache_tmp_file.c_str());
        }

        {
          Guard guard(lock_);
        
          struct stat64 file_stat;
        
          if(::stat64(cache_file.c_str(), &file_stat) == 0)
          {
            file_.open(cache_file.c_str(), std::ios::in);

            if(file_.is_open())
            {
              stream_.reset(new El::XML::InputStream(file_));
              return;
            }
          }
        }
          
        throw;
      }

      file_.open(cache_file.c_str(), std::ios::in);

      if(!file_.is_open())
      {
        std::ostringstream ostr;
        ostr << "El::XML::EntityResolver::InputSource::InputStream::"
          "create_file_stream: can't open '" << cache_file
             << "' for read access";
            
        throw xercesc::SAXException(ostr.str().c_str());
      }

      stream_.reset(new El::XML::InputStream(file_));
      
      clean_cache(file_strategy.cache_dir.c_str(),
                  file_strategy.cache_clean_period,
                  file_strategy.cache_file_expire);
    }

    void
    EntityResolver::InputSource::InputStream::clean_cache(
      const char* cache_dir,
      time_t cache_clean_period,
      time_t cache_file_expire) const
      throw(xercesc::SAXException, El::Exception)
    {
      time_t now = ACE_OS::gettimeofday().sec();
      
      {
        Guard guard(lock_);
        
        if(next_cache_cleanup_ && next_cache_cleanup_ <= now)
        {
          std::string dir(cache_dir);
          dir += "/";
        
          ExpiredFileList files(cache_dir, now - cache_file_expire);
        
          for(size_t i(0), e(files.count()); i != e; ++i)
          {
            unlink((dir + files[i].d_name).c_str());
          }   
        }
      }
      
      next_cache_cleanup_ = ACE_OS::gettimeofday().sec() + cache_clean_period;
    }
    
  }
}
