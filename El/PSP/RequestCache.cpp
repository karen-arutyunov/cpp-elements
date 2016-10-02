/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/PSP/RequestCache.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sstream>
#include <string>
#include <iostream>

#include <El/Exception.hpp>
#include <El/CRC.hpp>
#include <El/Apache/Request.hpp>
#include <El/Net/HTTP/Utility.hpp>

#include "RequestCache.hpp"

namespace El
{
  namespace PSP
  {
    namespace RequestCache
    {
      const char LOG_ASPECT[] = "PSP_Cache";
      
      //
      // ConditionMap class 
      //
      bool
      ConditionMap::request_cacheable(El::Apache::Request& request) const
        throw(El::Exception)
      {
        if(empty())
        {
          return false;
        }

        const El::Net::HTTP::ParamList* params = 0;
        
        for(const_iterator i(begin()), e(end()); i != e; ++i)
        {
          const CondKey& key = i->first;
            
          switch(key.type)
          {
          case CT_PARAM:
            {
              if(params == 0)
              {
                params = &request.in().parameters(true);
              }
              
              const char* param_val =
                params->find(key.value.c_str());

              const CondValue& cv = i->second;

              if(cv.op == CO_EQ)
              {
                if((param_val == 0 && cv.value != "NULL") ||
                   (param_val && cv.value != param_val))
                {
                  return false;
                }
              }
              else
              {
                if((param_val == 0 && cv.value == "NULL") ||
                   (param_val && cv.value == param_val))
                {
                  return false;
                }
              }
              
              break;
            }
          default: break;
          }
        }
        
        return true;
      }

      //
      // Cache class
      //
      
      Cache::Cache(time_t entry_timeout_delay,
                   time_t entry_unused_check_period,
                   El::Logging::Logger* logger)
        throw(El::Exception)
          : entry_timeout_delay_(entry_timeout_delay),
            entry_unused_check_period_(entry_unused_check_period),
            unused_entry_next_check_(0),
            logger_(logger)
      {
      }
      
      Entry*
      Cache::entry(El::Apache::Request& request,
                   CacheKeyType key_type,
                   time_t entry_timeout,
                   time_t entry_unused_timeout)
        throw(Exception, El::Exception)
      {
        uint64_t key = 0;

        std::auto_ptr<std::ostringstream> ostr;
        bool log = logger_ && logger_->will_log(El::Logging::DEBUG);

        if(log)
        {
          ostr.reset(new std::ostringstream());
          *ostr << "Cache::entry:\nIP: " << request.remote_ip()
                << ", URI: '" << request.unparsed_uri() << "'";
        }
        
        switch(key_type)
        {
        case CKT_NONE:
        case CKT_UNDEFINED:
          {
            if(log)
            {
              *ostr << "\nCan't determine key type\n"
                "No ready entry, no caching";
              
              logger_->debug(ostr->str(), LOG_ASPECT);
            }
            
            return 0;
          }
        case CKT_URI:
          {
            const char* uri = request.unparsed_uri();
            El::CRC(key, (const unsigned char*)uri, strlen(uri));
            
            if(log)
            {
              *ostr << "\nURI_Key: " << std::uppercase << std::hex << key
                    << std::dec;
            }

            break;
          }
        case CKT_URI_CRAWLER:
          {
            const char* uri = request.unparsed_uri();
            El::CRC(key, (const unsigned char*)uri, strlen(uri));

            const char* ua =
              request.in().headers().find(El::Net::HTTP::HD_USER_AGENT);
            
            if(ua)
            {
              const char* crawler = El::Net::HTTP::crawler(ua);
              El::CRC(key, (const unsigned char*)crawler, strlen(crawler));
            }
            
            if(log)
            {
              *ostr << "\nURI_Crawler_Key: " << std::uppercase
                    << std::hex << key << std::dec;
            }

            break;
          }
        }

        Entry* entry = 0;
        bool check_unused = false;
        time_t curr_time = request.time().sec();
        
        {
          ReadGuard guard(lock_);

          CacheMap::const_iterator it = cache_map_.find(key);

          if(it == cache_map_.end())
          {
            Entry_var e(
              new Entry(this, key, entry_timeout, entry_unused_timeout));
            
            if(log)
            {
              *ostr << " not found\nNew record created (" << std::uppercase
                    << std::hex << e.in() << std::dec << "); TTL:"
                    << entry_timeout << " sec, UTTL: " << entry_unused_timeout
                    << " sec\nNo ready entry, will cache";
            }
            
            request.callback(e.in(), PSP_CACHE_REQUEST_CALLBACK_ID);
          }
          else
          {            
            entry = it->second.in();            
            entry->expire_unused(curr_time + entry->unused_timeout());

            if(log)
            {
              *ostr << " found (" << std::uppercase << std::hex << entry
                    << std::dec << ")\nUTTL: " << entry->unused_timeout()
                    << " sec";
            }
          
            if(entry->expire() <= curr_time)
            {
              if(log)
              {
                *ostr << "\nExpired: keep for " << entry_timeout_delay_
                      << " sec\nNo ready entry, will cache";
              }
              
              entry->expire(curr_time + entry_timeout_delay_);
              entry = 0;

              Entry_var e(
                new Entry(this, key, entry_timeout, entry_unused_timeout));

              if(log)
              {
                *ostr << "\nNew record created (" << std::uppercase
                      << std::hex << e.in() << std::dec << "); TTL:"
                      << entry_timeout << " sec, UTTL: "
                      << entry_unused_timeout << " sec";
              }
              
              request.callback(e.in(), PSP_CACHE_REQUEST_CALLBACK_ID);
            }
            else
            {
              if(log)
              {
                *ostr << "; TTL: " << entry->expire() - curr_time << " sec";
              }
              
              El::Apache::Request::In& in = request.in();
          
              if(in.accept_encoding("gzip") || in.accept_encoding("identity"))
              {
                if(log)
                {
                  *ostr << "\nHave ready entry, no caching";
                }
                
                entry->add_ref();
              }
              else
              {
                if(log)
                {
                  *ostr << "\nNo gzip nor identity encoding supported\n"
                    "No ready entry, no caching";
                }
                
                entry = 0;
              }              
            }
          }          

          check_unused = entry == 0 && unused_entry_next_check_ <= curr_time;
        }

        if(check_unused)
        {
          size_t erased = 0;
          
          if(log)
          {
            *ostr << "\nCheck Unused:";
          }
          
          WriteGuard guard(lock_);
          
          for(CacheMap::iterator i(cache_map_.begin()),
                e(cache_map_.end()); i != e; )
          {
            if(i->second->expire_unused() <= curr_time)
            {
              CacheMap::iterator n(i);
              ++n;

              if(log)
              {
                *ostr << "\n  " << std::uppercase << std::hex << i->first
                      << " (" << std::uppercase << i->second.in() << std::dec
                      << ")";
              }
              
              cache_map_.erase(i);
              i = n;

              ++erased;
            }
            else
            {
              ++i;
            }
          }

          unused_entry_next_check_ = curr_time + entry_unused_check_period_;

          if(log)
          {
            *ostr << "\n" << erased << " erased; next check in "
                  << entry_unused_check_period_ << " sec";
          }
        }
        
        if(log)
        {
          logger_->debug(ostr->str(), LOG_ASPECT);
        }
        
        return entry;
      }

      void
      Cache::entry(Entry* val, uint64_t key) throw(El::Exception)
      {
        std::auto_ptr<std::ostringstream> ostr;
        bool log = logger_ && logger_->will_log(El::Logging::DEBUG);

        if(log)
        {
          ostr.reset(new std::ostringstream());
          *ostr << "Cache::entry: record filled for key "
                << std::uppercase << std::hex << key << std::uppercase
                << " (" << val << ")" << std::dec;
        }

        time_t timeout = val->timeout();

        if(!timeout)
        {
          if(log)
          {
            *ostr << "\nDropped; timeout 0";
            logger_->debug(ostr->str(), LOG_ASPECT);
          }
          
          return;
        }
        
        time_t now = ACE_OS::gettimeofday().sec();
          
        val->expire(now + timeout);
        val->expire_unused(now + val->unused_timeout());

        if(log)
        {
          const char* ct = val->content_type();
          
          *ostr << "\nSaved; TTL: " << timeout << " sec, UTTL: "
                << val->unused_timeout() << "  sec"
                << "\nCode: " << val->result()
                << "\nETag: " << val->etag()
                << "\nDataLen: " << val->data_len()
                << "\nContentType: '" << (ct ? ct : "") << "'"
                << "\nHeaders:";

          for(El::Net::HTTP::HeaderList::const_iterator
                i(val->headers().begin()), e(val->headers().end()); i != e;
              ++i)
          {
            *ostr << "\n" << i->name << ":" << i->value;
          }
          
          logger_->debug(ostr->str(), LOG_ASPECT);
        }
        
        WriteGuard guard(lock_);
        cache_map_[key] = El::RefCount::add_ref(val);
      }
      
      //
      // Entry class
      //
      Entry::Entry(Cache* cache,
                   uint64_t key,
                   time_t entry_timeout,
                   time_t entry_unused_timeout) throw(Exception, El::Exception)
          : key_(key),
            buff_(0),
            buff_len_(0),
            data_len_(0),
            content_type_(0),
            result_(0),
            expire_(0),
            expire_unused_(0),
            timeout_(entry_timeout),
            unused_timeout_(entry_unused_timeout),
            cache_(cache)
      {
      }
      
      Entry::~Entry() throw()
      {
        // Need to destruct gzip_ before buff_ is freed as it can flash
        // data in destructor writing to buff_
        
        gzip_.reset(0);
        
        if(buff_)
        {
          free(buff_);
        }

        if(content_type_)
        {
          free(content_type_);
        }
      }

      void
      Entry::content_type(const char* value) throw(El::Exception)
      {
        if(content_type_)
        {
          free(content_type_);
        }

        content_type_ = value && cache_ ? strdup(value) : 0;
      }

      void
      Entry::send_header(const char* name, const char* value)
        throw(El::Exception)
      {
        if(cache_ && strcasecmp(name, El::Net::HTTP::HD_CONTENT_ENCODING))
        {
          headers_.add(name, value);
        }
      }
      
      void
      Entry::start_writing(bool deflate) throw(El::Exception)
      {
        if(!deflate)
        {
          gzip_.reset(
            new El::Compress::ZLib::GZip(this, Z_DEFAULT_COMPRESSION));
        }
      }
      
      void
      Entry::on_write(const char* buffer, size_t size) throw(El::Exception)
      {
        if(cache_ == 0)
        {
          return;
        }

        if(gzip_.get())
        {
          gzip_->stream().write(buffer, size);
        }
        else
        {
          write_buff(buffer, size);
        }
      }

      size_t
      Entry::write(const char* buff, size_t len)
      {
        write_buff(buff, len);
        return len;
      }
      
      void
      Entry::write_buff(const char* buffer, size_t size) throw(El::Exception)
      {
        if(buff_len_ - data_len_ < size)
        {
          size_t tmp_len = std::max(2 * buff_len_, buff_len_ + size);
          unsigned char* tmp = (unsigned char*)realloc(buff_, tmp_len);

          if(tmp == 0)
          {
            cache_ = 0;
            
            if(buff_)
            {
              free(buff_);
              buff_ = 0;
              buff_len_ = 0;
              data_len_ = 0;
            }
            
            throw std::bad_alloc();
          }
          
          buff_ = tmp;
          buff_len_ = tmp_len;
        }
        
        memcpy(buff_ + data_len_, buffer, size);
        data_len_ += size;
      }
      
      void
      Entry::finalize(int result) throw(El::Exception)
      {
        if(result != OK || cache_ == 0)
        {
          gzip_.reset(0);
          return;
        }

        if(gzip_.get())
        {
          gzip_->finalize();
          gzip_.reset(0);
        }
        
        result_ = result;

        if(buff_len_ > data_len_)
        {
          unsigned char* tmp = (unsigned char*)malloc(data_len_);
          
          if(tmp)
          {
            memcpy(tmp, buff_, data_len_);

            free(buff_);
            buff_ = tmp;
            buff_len_ = data_len_;
          }
        }

        cache_->entry(this, key_);
      }

      void
      Entry::send_cache_control(El::Apache::Request& request) const
        throw(El::Exception)
      {
        time_t max_age = expire() - request.time().sec();

        if(max_age > 0)
        {
          request.out().send_header(
            El::Net::HTTP::HD_CACHE_CONTROL,
            (std::string("max-age=") +
             El::String::Manip::string(max_age)).c_str());
        }          
      }
      
      int
      Entry::respond(El::Apache::Request& request,
                     El::Logging::Logger* logger) const throw(El::Exception)
      {
        std::auto_ptr<std::ostringstream> ostr;
        bool log = logger && logger->will_log(El::Logging::DEBUG);

        if(log)
        {
          const char* inm =
            request.in().headers().find(El::Net::HTTP::HD_IF_NONE_MATCH);
          
          ostr.reset(new std::ostringstream());
          *ostr << "Entry::respond: responding for key "
                << std::uppercase << std::hex << key_ << " ("
                << std::uppercase << this << std::dec << "); TTL: "
                << expire() - request.time().sec() << "; UTTL: "
                << expire_unused() - request.time().sec()
                << "\n" << El::Net::HTTP::HD_IF_NONE_MATCH << ":"
                << (inm ? inm : "");
        }

        if(!etag_.empty())
        {  
          const char* inm =
            request.in().headers().find(El::Net::HTTP::HD_IF_NONE_MATCH);
          
          if(inm && etag_ == inm)
          {
            request.out().send_header(El::Net::HTTP::HD_ETAG, inm);
            send_cache_control(request);
            
            if(log)
            {
              *ostr << "\nNot modified; responded with " << HTTP_NOT_MODIFIED;
              logger->debug(ostr->str(), LOG_ASPECT);
            }
            
            return HTTP_NOT_MODIFIED;
          }
        }
        
        El::Apache::Request::Out& out = request.out();
        
        if(content_type_)
        {
          out.content_type(content_type_);
        }

        bool do_cache_control = true;
        
        for(El::Net::HTTP::HeaderList::const_iterator i(headers_.begin()),
              e(headers_.end()); i != e; ++i)
        {
          const char* name = i->name.c_str();
          out.send_header(name, i->value.c_str());

          if(strcasecmp(name, El::Net::HTTP::HD_CACHE_CONTROL) == 0)
          {
            do_cache_control = false;
          }
        }

        if(do_cache_control)
        {
          send_cache_control(request);
        }
        
        if(data_len_)
        {
          El::Apache::Request::In& in = request.in();
          
          if(in.accept_encoding("gzip"))
          {
            if(log)
            {
              *ostr << "\nWriting gzip-ed content";
            }
            
            out.send_header(El::Net::HTTP::HD_CONTENT_ENCODING, "gzip");  
            out.stream().write((const char*)buff_, data_len_);
          }
          else
          {
            if(log)
            {
              *ostr << "\nWriting plain content";
            }
            
            out.send_header(El::Net::HTTP::HD_CONTENT_ENCODING, "identity");
            
            GZippedDataReader reader(buff_, data_len_);
            El::Compress::ZLib::GUnzip gunzip(&reader);
            El::Compress::ZLib::InStream& istr = gunzip.stream();
            std::ostream& ostr = out.stream();
            
            char buff[1024];
            
            while(true)
            {
              istr.read(buff, sizeof(buff));
              size_t read_bytes = istr.gcount();

              if(read_bytes)
              {
                ostr.write(buff, read_bytes);
              }
              else
              {
                break;
              }
            }
          }
        }
        
        if(log)
        {
          *ostr << "\nServed from cache; responded with " << result_;
          logger->debug(ostr->str(), LOG_ASPECT);
        }
            
        return result_;
      }
      
      void
      Entry::timeout(time_t val) throw(Exception, El::Exception)
      {
        if(val && cache_ == 0)
        {
          throw Exception(
            "El::PSP:RequestCache::Entry::timeout: can't set non zero "
            "timeout as caching have been already canceled");
        }
        
        timeout_ = val;

        if(timeout_ == 0)
        {
          cache_ = 0;
        }
      }

    }
  }
}
