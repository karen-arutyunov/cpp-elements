/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

#include <sstream>
#include <string.h>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>

#include "DB.hpp"

extern pthread_key_t THR_KEY_mysys;

namespace El
{
  namespace MySQL
  {
    bool DB::MySQLLibInit::initialized_(false);
    ACE_TSS<DB::MySQLThreadInit> DB::thread_init_;
    DB::MySQLLibInit DB::lib_init_;
    
    //
    // DB::MySQLThreadInit class
    //
    
    DB::MySQLThreadInit::MySQLThreadInit() throw(Exception, El::Exception)
        : initialized_(false),
          value_(0)
    {
      if(MySQLLibInit::initialized_)
      {
        if(mysql_thread_init())
        {
          throw Exception(
            "DB::MySQLThreadInit::MySQLThreadInit: mysql_thread_init failed");
        }

        value_ = pthread_getspecific(THR_KEY_mysys);
        initialized_ = true;
      }
/*
      std::cerr << "MySQLThreadInit: " << MySQLLibInit::initialized_
                << "/" << value_ << "/" << initialized_
                << std::endl;
*/
    }
    
    DB::MySQLThreadInit::~MySQLThreadInit() throw()
    {
      bool keys = pthread_getspecific(THR_KEY_mysys) == 0;
      
      if(initialized_)
      {
        if(keys)
          pthread_setspecific(THR_KEY_mysys, value_);
        
        mysql_thread_end();
      }
/*
      std::cerr << "~MySQLThreadInit: " << MySQLLibInit::initialized_
                << "/" << value_ << "/" << initialized_ << "/" << keys
                << std::endl;
*/
    }
    
    //
    // DB class
    //
    DB::DB(const char* user,
           const char* passwd,
           const char* db,
           unsigned short port,
           const char* host,
           unsigned long client_flag,
           ConnectionFactory* connection_factory)
      throw(InvalidArg, Exception, El::Exception)
        :  user_(user ? user : ""),
           passwd_(passwd ? passwd : ""),
           db_(db ? db : ""),
           host_(host ? host : ""),
           port_(port),
           unix_socket_(""),
           client_flag_(client_flag),
           connection_factory_(connection_factory)
    {
      DB::init_thread();

      if(connection_factory == 0)
      {
        connection_factory_.reset(new NewConnectionsFactory(0, 0));
      }

      connection_factory_->db(this);
    }

    DB::DB(const char* user,
           const char* passwd,
           const char* db,
           const char* unix_socket,
           unsigned long client_flag,
           ConnectionFactory* connection_factory)
      throw(InvalidArg, Exception, El::Exception)
        :  user_(user ? user : ""),
           passwd_(passwd ? passwd : ""),
           db_(db ? db : ""),
           host_(""),
           port_(0),
           unix_socket_(unix_socket ? unix_socket : ""),
           client_flag_(client_flag),
           connection_factory_(connection_factory)
    {
      DB::init_thread();

      if(connection_factory == 0)
      {
        connection_factory_.reset(new NewConnectionsFactory(0, 0));
      }

      connection_factory_->db(this);
    }
    
    //
    // Connection class
    //
    Connection::Connection(DB* db,
                           const char* initialization,
                           const char* charset)
      throw(Exception, El::Exception)
        : db_(RefCount::add_ref<DB>(db)),
          mysql_(0)
          
    {
      if(db == 0)
      {
        throw Exception("El::MySQL::Connection::Connection: db is 0");
      }
      
      DB::init_thread();
      
      mysql_ = mysql_init(NULL);

      if (!mysql_)
      {
        std::ostringstream ostr;
        ostr << "El::MySQL::Connection::Connection: mysql_init() "
          "failed. DB info:\n" << *db;
          
        throw Exception(ostr.str());
      }
      
      try
      {
/*
  Implement properly
      
  if(mysql_options(mysql_, MYSQL_READ_DEFAULT_GROUP,"client"))
  {
  std::ostringstream ostr;
  ostr << "El::MySQL::Connection::Connection: "
  "mysql_options(MYSQL_READ_DEFAULT_GROUP) failed. DB info:\n" << *db;
          
  close();

  throw Exception(ostr.str());
  }        
*/
        my_bool my_true = true;
   
        if(mysql_options(mysql_, MYSQL_OPT_RECONNECT, &my_true))
        {
          std::ostringstream ostr;
          ostr << "El::MySQL::Connection::Connection: "
            "mysql_options(MYSQL_OPT_RECONNECT) failed. DB info:\n" << *db;
          
          close();

          throw Exception(ostr.str());
        }

        if(initialization && *initialization != '\0' &&
           mysql_options(mysql_, MYSQL_INIT_COMMAND, initialization))
        {
          std::ostringstream ostr;
          ostr << "El::MySQL::Connection::Connection: "
            "mysql_options(MYSQL_INIT_COMMAND) failed. DB info:\n" << *db;
          
          close();

          throw Exception(ostr.str());
        }
      
        if(charset && *charset != '\0' &&
           mysql_options(mysql_, MYSQL_SET_CHARSET_NAME, charset))
        {
          std::ostringstream ostr;
          ostr << "El::MySQL::Connection::Connection: "
            "mysql_options(MYSQL_SET_CHARSET_NAME) failed. DB info:\n" << *db;
          
          close();

          throw Exception(ostr.str());
        }

        if (!mysql_real_connect(mysql_,
                                db_->host(),
                                db_->user(),
                                db_->passwd(),
                                db_->db(),
                                db_->port(),
                                db_->unix_socket(),
                                db_->client_flag()))
        {
          std::ostringstream ostr;
          ostr << "El::MySQL::Connection::Connection: mysql_real_connect failed."
            " DB info:\n" << *db_.in() << "\nError: code "  << std::dec
               << mysql_errno(mysql_) << ", description:\n"
               << mysql_error(mysql_);
      
          close();
        
          throw Exception(ostr.str());
        }
      }
      catch(...)
      {
        mysql_close(mysql_);
        throw;
      }
    }

    Result*
    Connection::query(const char *query,
                      size_t length,
                      bool store_result)
      throw(Exception, El::Exception)
    {
      DB::init_thread();

      int res = length > 0 ?  mysql_real_query(mysql_, query,  length) :
        mysql_query(mysql_, query);
      
      if(res)
      {      
        std::ostringstream ostr;
        ostr << "El::MySQL::Connection::query: mysql_real_query failed. "
          "DB info:\n" << *db_.in() << "\nError: code " << std::dec
             << mysql_errno(mysql_) << ", description:\n"
             << mysql_error(mysql_) << std::endl << "\nQuery: " << query;
        
        throw Exception(ostr.str());
      }

      MYSQL_RES* result = store_result ?
        mysql_store_result(mysql_) : mysql_use_result(mysql_);
      
      if(mysql_field_count(mysql_) > 0 && result == NULL)
      {
        std::ostringstream ostr;
        ostr << "El::MySQL::Connection::query: mysql_real_query failed. "
          "DB info:\n" << *db_.in() << "\nError: code " << std::dec
             << mysql_errno(mysql_) << ", description:\n"
             << mysql_error(mysql_) << std::endl << "\nQuery: " << query;
        
        throw Exception(ostr.str());
      }

      return new Result(result, this, store_result);
    }

    std::string
    Connection::escape(const char *text, size_t length)
      throw (Exception, El::Exception)
    {
      DB::init_thread();

      if(text == 0)
      {
        text = "";
      }
      
      if(length == 0)
      {
        length = strlen(text);
      }

      if(length == 0)
      {
        return "";
      }

      std::string result;
      char* to = new char[2 * length + 1];

      unsigned long len = mysql_real_escape_string(mysql_, to, text, length);

      if(len < length)
      {
        delete [] to;
        
        std::ostringstream ostr;
        ostr << "El::MySQL::Connection::escape: failed to encode string '"
             << text << "'";
        
        throw Exception(ostr.str());
      }
      
      try
      {
        result = to;
      }
      catch(...)
      {
        delete [] to;
        throw;
      }
      
      delete [] to;
      return result;
    }

    void
    Connection::escape_for_load(std::ostream& dst,
                                const char *text,
                                size_t length)
      throw (Exception, El::Exception)
    {      
      if(length == 0)
      {
        length = strlen(text);
      }

      if(length == 0)
      {
        return;
      }
      
      const char* src = text;
      
      for(size_t i = length; i; i--, src++)
      {
        switch(*src)
        {
        case '\t':
        case '\n':
        case '\\': dst << '\\'; break;
        default: break;
        }
          
        dst << *src;
      }
    }    

    std::string
    Connection::escape_for_load(const char *text, size_t length)
      throw (Exception, El::Exception)
    {
      if(length == 0)
      {
        length = strlen(text);
      }

      if(length == 0)
      {
        return "";
      }
      
      char* to = new char[2 * length + 1];

      const char* src = text;
      char* dst = to;
      
      for(size_t i = length; i; i--, src++)
      {
        switch(*src)
        {
        case '\t':
        case '\n':
        case '\\': *dst++ = '\\'; break;
        default: break;
        }
          
        *dst++ = *src;
      }

      size_t len = dst - to;
      std::string result;

      try
      {
        result.assign(to, len);
      }
      catch(...)
      {
        delete [] to;
        throw;
      }

      delete [] to;
      return result;      
    }
    
    //
    // El::MySQL::Row class
    //

    void
    Row::data_seek(unsigned long long offset)
      throw(Exception, El::Exception)
    {
      DB::init_thread();
      
      mysql_data_seek(result_->mysql_res(), offset);
    }

    bool
    Row::fetch_row() throw(Exception, El::Exception)
    {
      DB::init_thread();
      
      row_ = mysql_fetch_row(result_->mysql_res());

      if(row_ == 0)
      {
        MYSQL* mysql = connection()->mysql();

        if(mysql_errno(mysql))
        {
          std::ostringstream ostr;
          ostr << "Row::fetch_row: mysql_fetch_row failed."
          " DB info:\n" << *db() << "\nError: code "  << std::dec
               << mysql_errno(mysql) << ", description:\n"
               << mysql_error(mysql);
      
          throw Exception(ostr.str());
        }
      }
      
      return row_ != 0;
    }

    //
    // ConnectionPoolFactory class
    //
    ConnectionPoolFactory::ConnectionPoolFactory(unsigned long min_connections,
                                                 unsigned long max_connections,
                                                 const char* initialization,
                                                 const char* charset)
      throw(InvalidArg, Exception, El::Exception)
        : db_(0),
          min_connections_(min_connections),
          max_connections_(max_connections),
          initialization_(initialization ? initialization : ""),
          charset_(charset ? charset : ""),
          used_connections_(0),
          waiting_connection_(0),
          connection_released_(lock_)
    {
      if(min_connections_ > max_connections_)
      {
        throw InvalidArg("El::MySQL::DB: min_connections should not be "
                         "greater than max_connections");
      }
    }
      
    ConnectionPoolFactory::~ConnectionPoolFactory() throw()
    {
      DB::init_thread();
      connections_.clear();
    }
    
    Connection*
    ConnectionPoolFactory::connect(const ACE_Time_Value* wait_time)
      throw(Timeout, Exception, El::Exception)
    {
      DB::init_thread();

      Guard guard(lock_);

      ReusableConnection_var connection = reuse_connection();

      if(connection.in() != 0)
      {
        return connection.retn();
      }

      ACE_Time_Value abstime;
      bool with_timeout = wait_time != 0;
      
      if(with_timeout)
      {
        abstime = ACE_OS::gettimeofday() + *wait_time;
      }

      waiting_connection_++;
      
      do
      {
        if(connection_released_.wait(with_timeout ? &abstime : 0))
        {
          int error = ACE_OS::last_error();
          
          if(with_timeout && error == ETIME)
          {
            waiting_connection_--;

            throw Timeout("El::DB::ConnectionPoolFactory::connect: "
                          "timeout expired");
          }
          else
          {
            waiting_connection_--;

            std::ostringstream ostr;
            
            ostr << "El::DB::ConnectionPoolFactory::connect: "
              "connection_released_.wait() failed. Errno " << error <<
              ". Description:" << std::endl << ACE_OS::strerror(error);
            
            throw Exception(ostr.str());
          }
          
        }
      }
      while(used_connections_ == max_connections_);

      waiting_connection_--;
      return reuse_connection();
    }

    ConnectionPoolFactory::ReusableConnection*
    ConnectionPoolFactory::reuse_connection() throw(Exception, El::Exception)
    {
      DB::init_thread();

      if(!connections_.empty())
      {
        used_connections_++;
        
        ReusableConnection_var connection = *connections_.rbegin();
        connections_.resize(connections_.size() - 1);

        connection->factory_ = this;
        connection->db_ = RefCount::add_ref<DB>(db_);
        
        return connection.retn();
      }

      if(used_connections_ < max_connections_)
      {
        std::auto_ptr<ReusableConnection>
          connection(new ReusableConnection(db_,
                                            this,
                                            initialization_.c_str(),
                                            charset_.c_str()));
        used_connections_++;
        return connection.release();
      }

      return 0;
    }

    void
    ConnectionPoolFactory::released(const ReusableConnection* connection)
      throw()
    {
      DB::init_thread();

      ReusableConnection* conn = const_cast<ReusableConnection*>(connection);
      conn->factory_ = 0;
      conn->db_ = 0;

      Guard guard(lock_);

      bool recycle = waiting_connection_ == 0 &&
        (used_connections_ + connections_.size()) > min_connections_;

      used_connections_--;
      
      if(recycle)
      {
        delete conn;
      }
      else
      {
        conn->ref_count_ = 1;
      
        ReusableConnection_var c = conn;
        connections_.push_back(c);
      }
      
      connection_released_.signal();
    }
    
  }
}
