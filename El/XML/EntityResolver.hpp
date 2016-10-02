/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/XML/EntityResolver.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_XML_ENTITYRESOLVER_HPP_
#define _ELEMENTS_EL_XML_ENTITYRESOLVER_HPP_

#include <memory>
#include <fstream>
#include <string>

#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/sax/InputSource.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/SAXException.hpp>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>
#include <ace/Process_Mutex.h>

#include <El/Exception.hpp>
#include <El/Net/HTTP/Session.hpp>
#include <El/XML/InputStream.hpp>

namespace El
{
  namespace XML
  {
    class EntityResolver: public xercesc::EntityResolver
    {
    public:

      struct NetStrategy
      {
        std::auto_ptr<ACE_Time_Value> connect_timeout;
        std::auto_ptr<ACE_Time_Value> send_timeout;
        std::auto_ptr<ACE_Time_Value> recv_timeout;
        size_t redirects_to_follow;

        NetStrategy(const ACE_Time_Value* connect_timeout_val = 0,
                    const ACE_Time_Value* send_timeout_val = 0,
                    const ACE_Time_Value* recv_timeout_val = 0,
                    size_t redirects_to_follow_val = 0)
          throw(El::Exception);

        NetStrategy(const NetStrategy& ns) throw(El::Exception);
        void operator=(const NetStrategy& ns) throw(El::Exception);        

      private:
        void copy(const NetStrategy& ns) throw(El::Exception);
      };

      struct FileStrategy
      {
        std::string cache_dir;
        time_t cache_timeout;
        time_t cache_retry_period;
        size_t max_file_size;
        time_t cache_clean_period;
        time_t cache_file_expire;

        FileStrategy(const char* cache_dir_val = "",
                     time_t cache_timeout_val = 86400,
                     time_t cache_retry_period_val = 60,
                     size_t max_file_size_val = 1024 * 1024 * 10,
                     time_t cache_clean_period_val = 60,
                     time_t cache_file_expire_val = 86400 * 3)
          throw(El::Exception);
      };
      
    public:
      
      EntityResolver(const NetStrategy& net_strategy = NetStrategy(),
                     const FileStrategy& file_strategy = FileStrategy())
        throw(El::Exception);

      virtual xercesc::InputSource* resolveEntity(const XMLCh* const publicId,
                                                  const XMLCh* const systemId)
        throw(xercesc::SAXException);

    protected:

      class InputSource: public xercesc::InputSource
      {
      public :
        InputSource(const XMLCh* const publicId,
                    const XMLCh* const systemId,
                    const NetStrategy& net_strategy,
                    const FileStrategy& file_strategy)
          throw(El::Exception);
      
        virtual xercesc::BinInputStream* makeStream() const
          throw(xercesc::SAXException);

      protected:
        
        class InputStream: public xercesc::BinInputStream
        {
        public :
          InputStream(const char* pub_id,
                      const char* sys_id,
                      const NetStrategy& net_strategy,
                      const FileStrategy& file_strategy)
            throw(xercesc::SAXException);
      
          virtual XMLFilePos curPos() const throw();
      
          virtual XMLSize_t readBytes(XMLByte* const toFill,
                                      const XMLSize_t maxToRead) throw();      

          virtual const XMLCh* getContentType() const { return 0; }

        protected:

          void create_session(const char* pub_id,
                              const char* sys_id,
                              const NetStrategy& net_strategy)
            throw(xercesc::SAXException, El::Exception);          

          void create_network_stream(
            const char* pub_id,
            const char* sys_id,
            const NetStrategy& net_strategy)
            throw(xercesc::SAXException, El::Exception);
          
          void create_file_stream(
            const char* pub_id,
            const char* sys_id,
            const NetStrategy& net_strategy,
            const FileStrategy& file_strategy)
            throw(xercesc::SAXException, El::Exception);
          
          void clean_cache(const char* cache_dir,
                           time_t cache_clean_period,
                           time_t cache_file_expire) const
            throw(xercesc::SAXException, El::Exception);
          
        protected:
          std::fstream file_;
          std::auto_ptr<El::Net::HTTP::Session> session_;
          std::auto_ptr<El::XML::InputStream> stream_;

        private:
          typedef ACE_Process_Mutex Mutex;
          typedef ACE_Guard<Mutex>  Guard;
    
          static Mutex lock_;
          static time_t next_cache_cleanup_;
          
        private:
          void operator=(const InputStream&);
          InputStream(const InputStream&);        
        };

      protected:
        std::string pub_id_;
        std::string sys_id_;
        NetStrategy net_strategy_;      
        FileStrategy file_strategy_;

      private:
        void operator=(const InputSource&);
        InputSource(const InputSource&);
      };

    protected:
      NetStrategy net_strategy_;
      FileStrategy file_strategy_;

    private:
      void operator=(const EntityResolver&);
      EntityResolver(const EntityResolver&);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace XML
  {
    //
    // EntityResolver
    //
    inline
    EntityResolver::EntityResolver(const NetStrategy& net_strategy,
                                   const FileStrategy& file_strategy)
      throw(El::Exception)
        : net_strategy_(net_strategy),
          file_strategy_(file_strategy)
    {
    }

    //
    // EntityResolver::NetStrategy
    //
    inline
    EntityResolver::NetStrategy::NetStrategy(
      const ACE_Time_Value* connect_timeout_val,
      const ACE_Time_Value* send_timeout_val,
      const ACE_Time_Value* recv_timeout_val,
      size_t redirects_to_follow_val)
      throw(El::Exception)
        : connect_timeout(connect_timeout_val ?
                          new ACE_Time_Value(*connect_timeout_val) : 0),
          send_timeout(send_timeout_val ?
                       new ACE_Time_Value(*send_timeout_val) : 0),
          recv_timeout(recv_timeout_val ?
                       new ACE_Time_Value(*recv_timeout_val) : 0),
          redirects_to_follow(redirects_to_follow_val)
    {
    }

    inline
    EntityResolver::NetStrategy::NetStrategy(const NetStrategy& ns)
      throw(El::Exception) : redirects_to_follow(0)
    {
      copy(ns);
    }
    
    inline
    void
    EntityResolver::NetStrategy::operator=(const NetStrategy& ns)
      throw(El::Exception)
    {
      copy(ns);
    }
    
    inline
    void
    EntityResolver::NetStrategy::copy(const NetStrategy& ns)
      throw(El::Exception)
    {
      connect_timeout.reset(ns.connect_timeout.get() ?
                            new ACE_Time_Value(*ns.connect_timeout) : 0);
        
      send_timeout.reset(ns.send_timeout.get() ?
                         new ACE_Time_Value(*ns.send_timeout) : 0);
        
      recv_timeout.reset(ns.recv_timeout.get() ?
                         new ACE_Time_Value(*ns.recv_timeout) : 0);

      redirects_to_follow = ns.redirects_to_follow;
    }

    //
    // EntityResolver::FileStrategy
    //
    inline
    EntityResolver::FileStrategy::FileStrategy(const char* cache_dir_val,
                                               time_t cache_timeout_val,
                                               time_t cache_retry_period_val,
                                               size_t max_file_size_val,
                                               time_t cache_clean_period_val,
                                               time_t cache_file_expire_val)
      throw(El::Exception)
        : cache_dir(cache_dir_val ? cache_dir_val : ""),
          cache_timeout(cache_timeout_val),
          cache_retry_period(cache_retry_period_val),
          max_file_size(max_file_size_val),
          cache_clean_period(cache_clean_period_val),
          cache_file_expire(cache_file_expire_val)
    {
    }
  }
}

#endif // _ELEMENTS_EL_XML_ENTITYRESOLVER_HPP_
