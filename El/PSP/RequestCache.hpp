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
 * @file Elements/El/PSP/RequestCache.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PSP_REQUESTCACHE_HPP_
#define _ELEMENTS_EL_PSP_REQUESTCACHE_HPP_

#include <stdint.h>

#include <string>
#include <memory>
#include <sstream>

#include <ext/hash_map>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/Hash/Hash.hpp>
#include <El/RefCount/All.hpp>
#include <El/SyncPolicy.hpp>
#include <El/Logging/Logger.hpp>

#include <El/Apache/Request.hpp>

namespace El
{
  namespace PSP
  {
    namespace RequestCache
    {
      const time_t TIME_UNSET = -1;
      const unsigned long PSP_CACHE_REQUEST_CALLBACK_ID = 2;
      
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
      enum CacheKeyType
      {
        CKT_UNDEFINED,
        CKT_NONE,
        CKT_URI,
        CKT_URI_CRAWLER
      };
      
      enum CondType
      {
        CT_UNDEFINED,
        CT_RESET,
        CT_PARAM
      };
      
      enum CondOp
      {
        CO_EQ,
        CO_NE
      };
      
      struct CondKey
      {
        CondType type;
        std::string value;

        CondKey(CondType tp = CT_UNDEFINED) throw() : type(tp) {}
        bool operator==(const CondKey& val) const throw();
      };

      struct CondValue
      {
        CondOp op;
        std::string value;

        CondValue() throw() : op(CO_EQ) {}
      };

      struct CondKeyHash
      {
        size_t operator()(const CondKey& val) const throw();
      };      

      struct ConditionMap :
        public __gnu_cxx::hash_map<CondKey,CondValue,CondKeyHash>
      {
        bool request_cacheable(El::Apache::Request& request) const
          throw(El::Exception);
      };

      class Cache;
      
      class Entry :
        virtual public El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>,
        virtual public El::Apache::Request::Callback,
        virtual public El::Compress::ZLib::OutStreamCallback
      {
      public:
        
        Entry(Cache* cache,
              uint64_t key,
              time_t entry_timeout,
              time_t entry_unused_timeout)
          throw(Exception, El::Exception);
        
        virtual ~Entry() throw();

        time_t expire() const throw();
        void expire(time_t val) throw();

        time_t expire_unused() const throw();
        void expire_unused(time_t val) throw();

        time_t timeout() const throw();
        void timeout(time_t val) throw(Exception, El::Exception);

        time_t unused_timeout() const throw();
        void unused_timeout(time_t val) throw();

        void etag(const char* value) throw(El::Exception);
        const char* etag() const throw(El::Exception);

        size_t data_len() const throw() { return data_len_; }
        int result() const throw() { return result_; }

        const El::Net::HTTP::HeaderList& headers() const throw();
        
        static time_t time(time_t tm) throw();          

        int respond(El::Apache::Request& request,
                    El::Logging::Logger* logger) const
          throw(El::Exception);

        void send_cache_control(El::Apache::Request& request) const
          throw(El::Exception);
        
        virtual void content_type(const char* value) throw(El::Exception);
        const char* content_type() const throw() { return content_type_; }
        
        virtual void send_header(const char* name, const char* value)
          throw(El::Exception);
        
        virtual void start_writing(bool deflate) throw(El::Exception);
        
        virtual void on_write(const char* buffer, size_t size)
          throw(El::Exception);

        virtual void pre_ap_rwrite() throw() {}
        virtual void post_ap_rwrite() throw() {}
        
        virtual void finalize(int result) throw(El::Exception);
        
        virtual size_t write(const char* buff, size_t len);
        
      private:
        void write_buff(const char* buffer, size_t size) throw(El::Exception);

      private:
        
        class GZippedDataReader : public El::Compress::ZLib::InStreamCallback
        {
        public:
          GZippedDataReader(const unsigned char* buff, size_t data_len)
            throw();
        
          virtual size_t read(char* buff, size_t len);
          virtual void putback(char* buff, size_t len);
          virtual ~GZippedDataReader() throw() {}

        private:
          const unsigned char* ptr_;
          const unsigned char* end_;
        };
        
      private:
        uint64_t key_;
        unsigned char* buff_;
        size_t buff_len_;
        size_t data_len_;
        char* content_type_;
        int result_;
        time_t expire_;
        time_t expire_unused_;
        time_t timeout_;
        time_t unused_timeout_;
        std::string etag_;
        El::Net::HTTP::HeaderList headers_;
        Cache* cache_;
        std::auto_ptr<El::Compress::ZLib::GZip> gzip_;

      private:
        Entry(const Entry&);
        void operator=(const Entry&);
      };
      
      typedef El::RefCount::SmartPtr<Entry> Entry_var;

      class Cache
      {
      public:
        Cache(time_t entry_timeout_delay,
              time_t entry_unused_check_period,
              El::Logging::Logger* logger)
          throw(El::Exception);
        
        Entry* entry(El::Apache::Request& request,
                     CacheKeyType key_type,
                     time_t entry_timeout,
                     time_t entry_unused_timeout)
          throw(Exception, El::Exception);

        void entry(Entry* val, uint64_t key) throw(El::Exception);

      private:

        typedef __gnu_cxx::hash_map<uint64_t,
                                    Entry_var,
                                    El::Hash::Numeric<uint64_t> >
        CacheMap;

        typedef ACE_RW_Thread_Mutex    Mutex;
        typedef ACE_Read_Guard<Mutex>  ReadGuard;
        typedef ACE_Write_Guard<Mutex> WriteGuard;

        Mutex lock_;
        time_t entry_timeout_delay_;
        time_t entry_unused_check_period_;
        time_t unused_entry_next_check_;
        
        CacheMap cache_map_;
        El::Logging::Logger* logger_;
      };
    };
    
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace PSP
  {    
    namespace RequestCache
    {
      //
      // CondKeyHash struct
      //
      inline
      size_t
      CondKeyHash::operator()(const CondKey& val) const throw()
      {
        return __gnu_cxx::__stl_hash_string(val.value.c_str());
      }

      //
      // CondKey struct
      //
      inline
      bool
      CondKey::operator==(const CondKey& val) const throw()
      {
        return type == val.type && value == val.value;
      }

      //
      // Entry struct
      //
      inline
      const El::Net::HTTP::HeaderList&
      Entry::headers() const throw()
      {
        return headers_;
      }
      
      inline
      time_t
      Entry::expire() const throw()
      {
        ReadGuard_ guard(lock_);
        return expire_;
      }
      
      inline
      void
      Entry::expire(time_t val) throw()
      {
        WriteGuard_ guard(lock_);
        expire_ = val;
      }
      
      inline
      time_t
      Entry::expire_unused() const throw()
      {
        ReadGuard_ guard(lock_);
        return expire_unused_;
      }
      
      inline
      void
      Entry::expire_unused(time_t val) throw()
      {
        WriteGuard_ guard(lock_);
        expire_unused_ = val;
      }
      
      inline
      time_t
      Entry::timeout() const throw()
      {
        return time(timeout_);
      }
      
      inline
      time_t
      Entry::unused_timeout() const throw()
      {
        return time(unused_timeout_);
      }
      
      inline
      void
      Entry::unused_timeout(time_t val) throw()
      {
        unused_timeout_ = val;
      }

      inline
      time_t
      Entry::time(time_t tm) throw()
      {
        return tm == TIME_UNSET ? 0 : tm;
      }

      inline
      void
      Entry::etag(const char* value) throw(El::Exception)
      {
        etag_ = value;
      }

      inline
      const char*
      Entry::etag() const throw(El::Exception)
      {
        return etag_.c_str();
      }

      //
      // Entry::GZippedDataReader class
      //
      inline
      Entry::GZippedDataReader::GZippedDataReader(const unsigned char* buff,
                                                  size_t data_len)
        throw()
          : ptr_(buff),
            end_(buff + data_len)
      {
      }
      
      inline  
      size_t
      Entry::GZippedDataReader::read(char* buff, size_t len)
      {
        size_t read_bytes = std::min(len, (size_t)(end_ - ptr_));
        memcpy(buff, ptr_, read_bytes);
        ptr_ += read_bytes;
        return read_bytes;
      }      
        
      inline  
      void
      Entry::GZippedDataReader::putback(char* buff, size_t len)
      {
        ptr_ -= len;
      }      
    }
  }
}

#endif // _ELEMENTS_EL_PSP_REQUESTCACHE_HPP_
