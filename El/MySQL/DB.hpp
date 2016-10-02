/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/MySQL/DB.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_MYSQL_DB_HPP_
#define _ELEMENTS_EL_MYSQL_DB_HPP_

#include <stdint.h>

#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <list>
#include <sstream>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>
#include <ace/TSS_T.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/SyncPolicy.hpp>
#include <El/Moment.hpp>

#include <mysql/mysql.h>

namespace El
{
  namespace MySQL
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(IsNull, Exception);
    EL_EXCEPTION(Timeout, Exception);
    EL_EXCEPTION(InvalidArg, Exception);

    class Connection;
    class DB;

    class ConnectionFactory
    {
    public:
      virtual ~ConnectionFactory() throw() {}

      virtual Connection* connect(const ACE_Time_Value* wait_time = 0)
        throw(Timeout, Exception, El::Exception) = 0;
      
      virtual void db(DB* db) throw(Exception, El::Exception) = 0;
    };

    class NewConnectionsFactory : public ConnectionFactory
    {
    public:
      NewConnectionsFactory(const char* initialization = 0,
                            const char* charset = 0) throw(El::Exception);
      
      virtual ~NewConnectionsFactory() throw();

      virtual Connection* connect(const ACE_Time_Value* wait_time = 0)
        throw(Timeout, Exception, El::Exception);

      virtual void db(DB* db) throw(Exception, El::Exception);

    private:
      DB* db_;
      std::string initialization_;
      std::string charset_;
    };

    class ConnectionPoolFactory;
    
    class DB : public virtual ::El::RefCount::DefaultImpl<Sync::ThreadPolicy>
    {
    public:
      EL_EXCEPTION(Exception, El::MySQL::Exception);

    public:
      DB(const char* user,
         const char* passwd,
         const char* db,
         uint16_t port = 0,
         const char* host = 0,
         unsigned long client_flag = 0,
         ConnectionFactory* connection_factory = 0)
        throw(InvalidArg, Exception, El::Exception);

      DB(const char* user,
         const char* passwd,
         const char* db,
         const char* unix_socket,
         unsigned long client_flag = 0,
         ConnectionFactory* connection_factory = 0)
        throw(InvalidArg, Exception, El::Exception);

      const char* user() const throw(El::Exception);
      const char* passwd() const throw(El::Exception);
      const char* db() const throw(El::Exception);
      const char* host() const throw(El::Exception);
      uint16_t port() const throw();
      const char* unix_socket() const throw(El::Exception);
      unsigned long client_flag() const throw();

      Connection* connect(const ACE_Time_Value* wait_time = 0)
        throw(Timeout, Exception, El::Exception);

      static void init_thread() { thread_init_->dummy(); }
      
    protected:
      virtual ~DB() throw();

    private:

      class MySQLThreadInit
      {
      public:
        MySQLThreadInit() throw(Exception, El::Exception);
        ~MySQLThreadInit() throw();

        void dummy() const {}
        
      private:
        bool initialized_;
        void* value_;
      };

      class MySQLLibInit
      {
        friend class MySQLThreadInit;
        
      public:
        MySQLLibInit() throw(Exception, El::Exception);
        ~MySQLLibInit() throw();

      private:
        static bool initialized_;
      };      
      
      static ACE_TSS<MySQLThreadInit> thread_init_;
      static MySQLLibInit lib_init_;      
      
    private:
      std::string user_;
      std::string passwd_;
      std::string db_;
      std::string host_;
      uint16_t port_;
      std::string unix_socket_;
      unsigned long client_flag_;
      std::auto_ptr<ConnectionFactory> connection_factory_;
    };

    typedef RefCount::SmartPtr<DB> DB_var;

    class Result;

    class Connection :
      public virtual ::El::RefCount::DefaultImpl<Sync::ThreadPolicy>
    {
      friend class NewConnectionsFactory;
      
    public:      
      EL_EXCEPTION(Exception, El::MySQL::Exception);
      EL_EXCEPTION(InvalidArg, Exception);

      Result* query(const char *query,
                    size_t length = 0,
                    bool store_result = true)
        throw (Exception, El::Exception);

      MYSQL* mysql() const throw();

      DB* db() const throw();
      
      std::string escape(const char *text, size_t length = 0)
        throw (Exception, El::Exception);

      static std::string escape_for_load(const char *text,
                                         size_t length = 0)
        throw (Exception, El::Exception);

      static void escape_for_load(std::ostream& dst,
                                  const char *text,
                                  size_t length = 0)
        throw (Exception, El::Exception);

      unsigned long affected_rows() throw();
      unsigned long long insert_id() throw();

    protected:
      Connection(DB* db,
                 const char* initialization = 0,
                 const char* charset = 0)
        throw(Exception, El::Exception);
      
      virtual ~Connection() throw();
        
      void close() throw();

    protected:
      DB_var db_;
      MYSQL* mysql_;

    private:
      Connection(const Connection& );
      Connection& operator=(const Connection& );      
    };

    typedef RefCount::SmartPtr<Connection> Connection_var;  

    class Result :
      public virtual ::El::RefCount::DefaultImpl<Sync::ThreadPolicy>
    {
      friend class Connection;
      
    public:      
      EL_EXCEPTION(Exception, El::MySQL::Exception);
      EL_EXCEPTION(InvalidArg, Exception);

      MYSQL_RES* mysql_res() const throw();

      Connection* connection() const throw();
      DB* db() const throw();
      
      bool stored() const throw();

      unsigned long num_fields() throw();
      unsigned long long num_rows() throw();
      
      MYSQL_FIELD* fetch_fields() throw();

      MYSQL_FIELD& operator[](size_t index)
        throw(InvalidArg, Exception, El::Exception);
      
    protected:
      Result(MYSQL_RES* result, Connection* connection, bool stored) throw();
      virtual ~Result() throw();

      void release() throw ();

    private:
      MYSQL_RES* result_;
      Connection_var connection_;
      bool stored_;
      unsigned long num_fields_;
      MYSQL_FIELD* fields_;

    private:
      Result(const Result& );
      Result& operator=(const Result& );
    };

    typedef RefCount::SmartPtr<Result> Result_var;

    class ConnectionPoolFactory : public ConnectionFactory
    {
    public:
      ConnectionPoolFactory(unsigned long min_connections,
                            unsigned long max_connections,
                            const char* initialization = 0,
                            const char* charset = 0)
        throw(InvalidArg, Exception, El::Exception);
      
      virtual ~ConnectionPoolFactory() throw();

      virtual Connection* connect(const ACE_Time_Value* wait_time = 0)
        throw(Timeout, Exception, El::Exception);

      virtual void db(DB* db) throw(Exception, El::Exception);

    private:

      class ReusableConnection : public virtual Connection
      {
        friend class ConnectionPoolFactory;
        
      public:
        virtual ~ReusableConnection() throw();
        
      protected:
        ReusableConnection(DB* db,
                           ConnectionPoolFactory* factory,
                           const char* initialization = 0,
                           const char* charset = 0)
          throw(Exception, El::Exception);
        
        virtual void destroy_i() const throw();

      protected:
        ConnectionPoolFactory* factory_;
      };

      friend class ReusableConnection;
      
      typedef RefCount::SmartPtr<ReusableConnection> ReusableConnection_var;
      typedef std::vector<ReusableConnection_var> ReusableConnectionArray;

    private:
      void released(const ReusableConnection* connection) throw();
      ReusableConnection* reuse_connection() throw(Exception, El::Exception);
      
    private:      
      
      DB* db_;
      unsigned long min_connections_;
      unsigned long max_connections_;
      std::string initialization_;
      std::string charset_;
      
      unsigned long used_connections_;
      ReusableConnectionArray connections_;
      unsigned long waiting_connection_;

      typedef ACE_Thread_Mutex Mutex;
      typedef ACE_Guard<Mutex> Guard;
      typedef ACE_Condition<Mutex> Condition;

      Mutex lock_;
      Condition connection_released_;
    };

    class Type
    {
    public:
      EL_EXCEPTION(Exception, El::MySQL::Exception);
      
    public:
      Type(bool is_null) throw();

      bool is_null() const throw();

    protected:
      bool is_null_;
    };
    
    class String : public Type
    {
    public:
      EL_EXCEPTION(Exception, Type::Exception);

    public:
      String(bool is_null, const char* str, size_t length)
        throw(Exception, El::Exception);

      String(String& src) throw();

      ~String() throw();

      operator std::string() const throw(IsNull, El::Exception);
      const char* c_str() const throw(El::Exception);
      const char* value() const throw(El::Exception);

      size_t length() const throw();

      String& operator=(String& src) throw();
      char operator[](size_t index) const throw();
      
    protected:
      char* value_;
      size_t length_;
    };

    class DateTime : public String
    {
    public:
      EL_EXCEPTION(Exception, String::Exception);

    public:  
      DateTime(bool is_null, const char* str, size_t length)
        throw(Exception, El::Exception);

      DateTime(DateTime& src) throw();

      DateTime& operator=(DateTime& src) throw();

      operator const El::Moment() const throw(IsNull, El::Exception);
      const El::Moment moment() const throw(IsNull, El::Exception);
      
      operator const El::Moment*() const throw();
      const El::Moment* moment_ptr() const throw();

    protected:
      El::Moment moment_;
    };

    template <class TYPE>
    class Numeric : public Type
    {
    public:
      EL_EXCEPTION(Exception, Type::Exception);

    public:
      Numeric(bool is_null, TYPE value) throw();

      operator TYPE() const throw(IsNull);
      TYPE value() const throw(IsNull);

      operator const TYPE*() const throw();
      const TYPE* pvalue() const throw();

      Numeric& operator=(const Numeric& src) throw();

    protected:
      TYPE value_;
    };

    typedef Numeric<uint64_t> Bit;

    typedef Numeric<char> Char;
    typedef Numeric<unsigned char> UnsignedChar;

    typedef Numeric<int16_t> Short;
    typedef Numeric<uint16_t> UnsignedShort;

    typedef Numeric<int32_t> Long;
    typedef Numeric<uint32_t> UnsignedLong;

    typedef Numeric<int64_t> LongLong;
    typedef Numeric<uint64_t> UnsignedLongLong;

    typedef Numeric<float> Float;
    typedef Numeric<double> Double;

    class Row
    {
    public:
      EL_EXCEPTION(Exception, El::MySQL::Exception);
      EL_EXCEPTION(InvalidArg, Exception);

      Row(Result* result) throw(InvalidArg, Exception, El::Exception);

      bool fetch_row() throw(Exception, El::Exception);
      
      void data_seek(unsigned long long offset)
        throw(Exception, El::Exception);

      Result* result() const throw();
      Connection* connection() const throw();
      DB* db() const throw();
      
      El::MySQL::String string(size_t index) const
        throw(Exception, El::Exception);
      
    protected:
      Result_var result_;
      MYSQL_ROW row_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace MySQL
  {
    //
    // DB class
    //
    inline
    DB::~DB() throw()
    {
//      DB::init_thread();
    }

    inline
    Connection*
    DB::connect(const ACE_Time_Value* wait_time)
      throw(Timeout, Exception, El::Exception)
    {
      DB::init_thread();
      return connection_factory_->connect(wait_time);
    }

    inline
    const char*
    DB::user() const throw(El::Exception)
    {
      return user_.c_str();
    }
    
    inline
    const char*
    DB::passwd() const throw(El::Exception)
    {
      return passwd_.c_str();
    }
    
    inline
    const char*
    DB::db() const throw(El::Exception)
    {
      return db_.c_str();
    }
    
    inline
    const char*
    DB::host() const throw(El::Exception)
    {
      return host_.c_str();
    }
    
    inline
    uint16_t
    DB::port() const throw()
    {
      return port_;
    }
    
    inline
    const char*
    DB::unix_socket() const throw(El::Exception)
    {
      return unix_socket_.empty() ? 0 : unix_socket_.c_str();
    }
    
    inline
    unsigned long
    DB::client_flag() const throw()
    {
      return client_flag_;
    }

    //
    // NewConnectionsFactory class
    //
    inline
    NewConnectionsFactory::NewConnectionsFactory(const char* initialization,
                                                 const char* charset)
      throw(El::Exception)
        : db_(0),
          initialization_(initialization ? initialization : ""),
          charset_(charset ? charset : "")
    {
    }
    
    inline
    NewConnectionsFactory::~NewConnectionsFactory() throw()
    {
    }
    
    inline
    Connection*
    NewConnectionsFactory::connect(const ACE_Time_Value* )
      throw(Timeout, Exception, El::Exception)
    {
      DB::init_thread();
      return new Connection(db_, initialization_.c_str(), charset_.c_str());
    }
    
    inline
    void
    NewConnectionsFactory::db(DB* db) throw(Exception, El::Exception)
    {
      db_ = db;
    }
    
    //
    // Connection class
    //
    inline
    MYSQL*
    Connection::mysql() const throw()
    {
      DB::init_thread();
      return mysql_;
    }

    inline
    Connection::~Connection() throw()
    {
      close();
    }

    inline
    void
    Connection::close() throw ()
    {
      if(mysql_)
      {
        DB::init_thread();
        
        mysql_close(mysql_);
        mysql_ = 0;
      }    
    }

    inline
    DB*
    Connection::db() const throw()
    {
      DB::init_thread();
      return db_.in();
    }
    
    inline
    unsigned long
    Connection::affected_rows() throw()
    {
      DB::init_thread();
      return mysql_affected_rows(mysql_);
    }

    inline
    unsigned long long
    Connection::insert_id() throw()
    {
      DB::init_thread();
      return mysql_insert_id(mysql_);      
    }

    //
    // ConnectionPoolFactory class
    //
    inline
    void
    ConnectionPoolFactory::db(DB* db) throw(Exception, El::Exception)
    {      
      DB::init_thread();

      db_ = db;

      connections_.resize(min_connections_);
      
      for(size_t i = 0; i < min_connections_; i++)
      {
        ReusableConnection_var connection =
          new ReusableConnection(db,
                                 0,
                                 initialization_.c_str(),
                                 charset_.c_str());
        
        connection->db_ = 0;
        connections_[i] = connection;
      }      
    }
    
    //
    // ConnectionPoolFactory::ReusableConnection class
    //
    inline
    ConnectionPoolFactory::ReusableConnection::ReusableConnection(
      DB* db,
      ConnectionPoolFactory* factory,
      const char* initialization,
      const char* charset)
      throw(Exception, El::Exception)
        : Connection(db, initialization, charset),
          factory_(factory)
    {
    }
    
    inline
    ConnectionPoolFactory::ReusableConnection::~ReusableConnection() throw()
    {
    }
    
    inline
    void
    ConnectionPoolFactory::ReusableConnection::destroy_i() const throw()
    {
      DB::init_thread();

      if(factory_)
      {
        factory_->released(this);
      }
      else
      {
        delete this;
      }
    }
    
    //
    // DB::MySQLLibInit class
    //

    inline
    DB::MySQLLibInit::MySQLLibInit() throw(Exception, El::Exception)
    {
      if(mysql_library_init(0, 0, 0))
      {
        std::cerr << "El::MySQL::DB::MySQLLibInit::"
          "MySQLLibInit: mysql_library_init failed\n";
        
        abort ();
      }

      thread_init_->dummy();
      
      MySQLLibInit::initialized_ = true;
    }
    
    inline
    DB::MySQLLibInit::~MySQLLibInit() throw()
    {
      mysql_library_end();
    }

    //
    // Result class
    //
    inline
    Result::Result(MYSQL_RES* result, Connection* connection, bool stored)
      throw()
        : result_(result),
          connection_(RefCount::add_ref(connection)),
          stored_(stored),
          num_fields_(0),
          fields_(0)      
    {
    }
    
    inline
    Result::~Result() throw()
    {
      release();
    }

    inline
    Connection*
    Result::connection() const throw()
    {
      DB::init_thread();
      return connection_.in();
    }
    
    inline
    DB*
    Result::db() const throw()
    {
      DB::init_thread();
      
      return connection()->db();
    }
    
    inline
    bool
    Result::stored() const throw()
    {
      return stored_;
    }
    
    inline
    MYSQL_RES*
    Result::mysql_res() const throw()
    {
      DB::init_thread();
      return result_;
    }
    
    inline
    void
    Result::release() throw ()
    {
      if(result_)
      {
        DB::init_thread();
        
        mysql_free_result(result_);
        result_ = NULL;
      }
    }
  
    inline
    unsigned long
    Result::num_fields() throw()
    {
      DB::init_thread();

      return result_ ? (num_fields_ ? num_fields_ :
                        (num_fields_ = mysql_num_fields(result_))) : 0;
    }
    
    inline
    unsigned long long
    Result::num_rows() throw()
    {
      DB::init_thread();
      return result_ ? mysql_num_rows(result_) : 0;
    }
    
    inline
    MYSQL_FIELD*
    Result::fetch_fields() throw()
    {
      DB::init_thread();
      
      return result_ ? (fields_ ? fields_ :
                        (fields_ = mysql_fetch_fields(result_))) : 0;
    }

    inline
    MYSQL_FIELD&
    Result::operator[](size_t index)
      throw(InvalidArg, Exception, El::Exception)
    {
      DB::init_thread();
      
      if(!num_fields() || fetch_fields() == 0)
      {
        throw Exception("El::MySQL::Result::operator[]: no fields");
      }

      if(index >= num_fields_)
      {
        std::ostringstream ostr;
        ostr << "El::MySQL::Result::operator[]: unexpected index "
             << index << " when number of fileds is " << num_fields_;

        throw InvalidArg(ostr.str());
      }

      return fields_[index];
    } 
  
    //
    // Row class
    //
    inline
    Row::Row(Result* result) throw(InvalidArg, Exception, El::Exception)
        : result_(RefCount::add_ref(result)),
          row_(0)
    {
      if(result == 0)
      {
        throw InvalidArg("El::MySQL::Roe::Row: result is 0");
      }
    }

    inline
    Result*
    Row::result() const throw()
    {
      return result_.in();
    }
    
    inline
    Connection*
    Row::connection() const throw()
    {
      DB::init_thread();
      
      return result()->connection();
    }
    
    inline
    DB*
    Row::db() const throw()
    {
      DB::init_thread();
      
      return result()->db();
    }
    
    inline
    El::MySQL::String
    Row::string(size_t index) const throw(Exception, El::Exception)
    {
      DB::init_thread();

      if(row_ == 0)
      {
        throw Exception("El::MySQL::Row::string: row_ is 0");
      }

      if(result_ == 0)
      {
        throw Exception("El::MySQL::Row::string: result_ is 0");
      }

      if(index >= result_->num_fields())
      {
        std::ostringstream ostr;
        ostr << "El::MySQL::Row::string: unexpected index " << index <<
          " when number of fileds is " << result_->num_fields();

        throw Exception(ostr.str());
      }

      if(row_[index] == 0)
      {
        El::MySQL::String tmp(true, 0, 0);
        return tmp;
      }

      unsigned long* lengths = mysql_fetch_lengths(result_->mysql_res());
      El::MySQL::String tmp(false, row_[index], lengths[index]);

      return tmp;
    }
    
    //
    // Type class
    //
    inline
    Type::Type(bool is_null) throw() : is_null_(is_null)
    {
    }
    
    inline
    bool
    Type::is_null() const throw()
    {
      return is_null_;
    }
    
    //
    // String class
    //
    inline
    String::String(bool is_null, const char* str, size_t length)
      throw(Exception, El::Exception)
        : Type(is_null),
          value_(0),
          length_(length)
    {
      if(!is_null)
      {
        value_ = new char[length + 1];
        value_[length] = '\0';
        memcpy(value_, str, length);
      }
    }

    inline
    String::String(String& src) throw()
        : Type(src.is_null_),
          value_(src.value_),
          length_(src.length_)
      
    {
      src.value_ = 0;
      src.length_ = 0;
      src.is_null_ = true;
    }
    
    inline
    String::~String() throw()
    {
      delete [] value_;
    }

    inline
    String&
    String::operator=(String& src) throw()
    {
      delete [] value_;
      
      value_ = src.value_;
      length_ = src.length_;
      is_null_ = src.is_null_;

      src.value_ = 0;
      src.length_ = 0;
      src.is_null_ = true;
      
      return *this;
    }

    inline
    char
    String::operator[](size_t index) const throw()
    {
      return value_[index];
    }

    inline
    String::operator std::string() const
      throw(IsNull, El::Exception)
    {
      if(is_null_)
      {
        throw IsNull(
          "El::MySQL::String::operator std::string: value is null");
      }

      std::string result;
      result.assign(value_, length_);

      return result;
    }

    inline
    const char*
    String::value() const throw(El::Exception)
    {
      return is_null() ? "(null)" : c_str();
    }
    
    inline
    size_t
    String::length() const throw()
    {
      return length_;
    }

    inline
    const char*
    String::c_str() const throw(El::Exception)
    {
      return value_;
    }

    //
    // DateTime class
    //
    inline
    DateTime::DateTime(bool is_null, const char* str, size_t length)
      throw(Exception, El::Exception)
        : String(is_null, str, length)
    {
      if(!is_null)
      {
        moment_.set_iso8601(value_);
      }
    }
    
    inline
    DateTime::DateTime(DateTime& src) throw()
        : String(src),
          moment_(src.moment_)
    {
    }
    
    inline
    DateTime&
    DateTime::operator=(DateTime& src) throw()
    {
      operator=(src);
      moment_ = src.moment_;
      
      return *this;
    }
    
    inline
    DateTime::operator const El::Moment() const throw(IsNull, El::Exception)
    {
      if(is_null_)
      {
        throw IsNull(
          "El::MySQL::DateTime::operator El::Moment: value is null");
      }

      return moment_;
    }

    inline
    El::Moment
    const DateTime::moment() const throw(IsNull, El::Exception)
    {
      if(is_null_)
      {
        throw IsNull(
          "El::MySQL::DateTime::monent: value is null");
      }

      return moment_;
    }
    
    inline
    DateTime::operator const El::Moment*() const throw()
    {
      return moment_ptr();
    }
    
    inline
    const El::Moment*
    DateTime::moment_ptr() const throw()
    {
      if(is_null())
      {
        return 0;
      }

      return &moment_;
    }

    //
    // Numeric template class
    //
    template <class TYPE>
    Numeric<TYPE>::Numeric(bool is_null, TYPE value) throw()
        : Type(is_null),
          value_(value)
    {
    }
    
    template <class TYPE>
    Numeric<TYPE>::operator TYPE() const throw(IsNull)
    {
      if(is_null_)
      {
        throw IsNull(
          "El::MySQL::Numeric<TYPE>::operator TYPE: value is null");
      }
      
      return value_;
    }
    
    template <class TYPE>
    TYPE
    Numeric<TYPE>::value() const throw(IsNull)
    {
      if(is_null_)
      {
        throw IsNull(
          "El::MySQL::Numeric<TYPE>::value: value is null");
      }
      
      return value_;
    }
    
    template <class TYPE>
    Numeric<TYPE>::operator const TYPE*() const throw()
    {
      return pvalue();
    }
    
    template <class TYPE>
    const TYPE*
    Numeric<TYPE>::pvalue() const throw()
    {
      if(is_null())
      {
        return 0;
      }

      return &value_;
    }
  }
}

inline
std::ostream&
operator<<(std::ostream& ostr, const El::MySQL::DB& db) throw(El::Exception)
{
  ostr << "user " << db.user() << ", db " << db.db() << ", host " << db.host()
       << ", port " <<  db.port() << ", unix_socket "
       << (db.unix_socket() ? db.unix_socket() : "")
       << ", client_flag " << std::hex << db.client_flag();

  return ostr;
}

inline
std::ostream&
operator<<(std::ostream& ostr, const El::MySQL::String& str)
  throw(El::Exception)
{
  ostr << str.value();
  return ostr;
}

template <class TYPE>
std::ostream&
operator<<(std::ostream& ostr, const El::MySQL::Numeric<TYPE>& val)
  throw(El::Exception)
{
  if(val.is_null())
  {
    ostr << "null";
  }
  else
  {
    ostr << val.value();
  }
  
  return ostr;
}

#endif // _ELEMENTS_EL_MYSQL_DB_HPP_
