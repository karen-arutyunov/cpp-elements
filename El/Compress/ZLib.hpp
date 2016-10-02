/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Compress/ZLib.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_COMPRESS_ZLIB_HPP_
#define _ELEMENTS_EL_COMPRESS_ZLIB_HPP_

#include <zlib.h>

#include <string>
#include <streambuf>
#include <iostream>
#include <memory>

#include <ace/OS.h>

#include <El/Exception.hpp>

namespace El
{
  namespace Compress
  {
    namespace ZLib
    {
      EL_EXCEPTION(Exception, El::ExceptionBase);
      EL_EXCEPTION(InvalidArg, El::ExceptionBase);

      class OutStreamCallback
      {
      public:
        virtual size_t write(const char* buff, size_t len) = 0;
        virtual ~OutStreamCallback() throw() {}
      };
      
      class InStreamCallback
      {
      public:
        virtual size_t read(char* buff, size_t len) = 0;
        virtual void putback(char* buff, size_t len) = 0;
        virtual ~InStreamCallback() throw() {}
      };
      
      class OutStreamBuf
        : public std::basic_streambuf<char, std::char_traits<char> >
      {        
      public:
        OutStreamBuf(OutStreamCallback* callback,
                     int level = Z_DEFAULT_COMPRESSION,
                     size_t in_buffer_size = 1024,
                     size_t out_buffer_size = 1024,
                     bool raw_deflate = false, // zlib deflate if false
                     bool calc_crc = false)
          throw(InvalidArg, Exception, El::Exception);
          
        virtual ~OutStreamBuf() throw();

        virtual int_type overflow(int_type c);
        virtual int sync();

        unsigned long long in_bytes() const throw();
        unsigned long long out_bytes() const throw();
        size_t in_crc() const throw();

        const ACE_Time_Value& compression_time() const throw();
        
        int last_error() throw();
        const std::string& last_error_desc() const throw(El::Exception);

        void finalize() throw(El::Exception);
        bool finalized() throw();

      protected:
        int flush_zlib(bool finish) throw(El::Exception);
        bool push_write_pending() throw(El::Exception);
        int deflate(int flush) throw();

      protected:
        z_stream z_stream_;
        
        OutStreamCallback* callback_;

        size_t in_buffer_size_;
        size_t out_buffer_size_;
        size_t write_pending_bytes_;
        
        char* in_buffer_;
        char* out_buffer_;

        unsigned long long in_bytes_;
        unsigned long long out_bytes_;
        size_t in_crc_;
        bool calc_crc_;
        
        ACE_Time_Value compression_time_;
        
        int last_error_;
        std::string last_error_desc_;

        bool deflate_initialized_;
        bool finalized_;
      };

      class InStreamBuf
        : public std::basic_streambuf<char, std::char_traits<char> >
      {        
      public:
        InStreamBuf(InStreamCallback* callback,
                    size_t in_buffer_size = 1024,
                    size_t out_buffer_size = 1024,
                    size_t putback_buffer_size = 1024,
                    bool raw_deflate = false,
                    bool calc_crc = false)
          throw(InvalidArg, Exception, El::Exception);
          
        virtual ~InStreamBuf() throw();

        virtual std::streamsize showmanyc();
        virtual int_type underflow();

        unsigned long long in_bytes() const throw();
        unsigned long long out_bytes() const throw();
        size_t out_crc() const throw();

        const ACE_Time_Value& decompression_time() const throw();
        
        int last_error() throw();
        const std::string& last_error_desc() const throw(El::Exception);

      protected:
        
        int_type read_to_buf(char* eback_ptr,
                             char* read_ptr,
                             size_t bytes_to_read)
          throw(El::Exception);
        
      protected:
        z_stream z_stream_;
        
        InStreamCallback* callback_;

        size_t in_buffer_size_;
        size_t out_buffer_size_;
        size_t putback_buffer_size_;
        size_t out_buffer_total_size_;
        
        char* in_buffer_;
        char* out_buffer_;

        unsigned long long in_bytes_;
        unsigned long long out_bytes_;
        size_t out_crc_;
        bool calc_crc_;
        
        ACE_Time_Value decompression_time_;
        
        int last_error_;
        std::string last_error_desc_;

        bool inflate_initialized_;
        bool end_of_stream_;
      };

      class OutStream :
        public std::basic_ostream<char, std::char_traits<char> >
      {
      public:
        OutStream(OutStreamCallback* callback,
                  int level = Z_DEFAULT_COMPRESSION,
                  size_t in_buffer_size = 1024,
                  size_t out_buffer_size = 1024,
                  bool raw_deflate = false,
                  bool calc_crc = false)
          throw(InvalidArg, Exception, El::Exception);

        virtual ~OutStream() throw();

        unsigned long long in_bytes() const throw();
        unsigned long long out_bytes() const throw();
        size_t in_crc() const throw();

        const ACE_Time_Value& compression_time() const throw();

        OutStreamBuf& zlib_streambuf() const throw();

        int last_error() throw();
        const std::string& last_error_desc() const throw(El::Exception);

        void finalize() throw(El::Exception);

      protected:

        typedef std::auto_ptr<OutStreamBuf> OutStreamBufPtr;

        OutStreamBufPtr streambuf_;
      };

      class InStream :
        public std::basic_istream<char, std::char_traits<char> >
      {
      public:
        InStream(InStreamCallback* callback,
                 size_t in_buffer_size = 1024,
                 size_t out_buffer_size = 1024,
                 size_t putback_buffer_size = 1024,
                 bool raw_deflate = false,
                 bool calc_crc = false)
          throw(InvalidArg, Exception, El::Exception);

        virtual ~InStream() throw();

        unsigned long long in_bytes() const throw();
        unsigned long long out_bytes() const throw();
        size_t out_crc() const throw();

        const ACE_Time_Value& decompression_time() const throw();

        InStreamBuf& zlib_streambuf() const throw();

        int last_error() throw();
        const std::string& last_error_desc() const throw(El::Exception);

      protected:

        typedef std::auto_ptr<InStreamBuf> InStreamBufPtr;

        InStreamBufPtr streambuf_;
      };

      class InputStreamReader : public virtual InStreamCallback
      {
      public:
        InputStreamReader(std::istream& istr) throw(El::Exception);

        virtual size_t read(char* buff, size_t len);
        virtual void putback(char* buff, size_t len);

      protected:
        std::istream& istream_;
      };

      class OutStreamWriter : public OutStreamCallback
      {
      public:
        OutStreamWriter(std::ostream& ostr) throw(El::Exception);
  
        virtual size_t write(const char* buff, size_t len);

      private:
        std::ostream& ostr_;
      };      
      
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Compress
  {
    namespace ZLib
    {
      //
      // OutStreamBuf class
      //
      inline
      OutStreamBuf::~OutStreamBuf() throw()
      {
//        std::cerr << "OutStreamBuf::~OutStreamBuf()\n";

        if(deflate_initialized_)
        {
          try
          {
            finalize();
          }
          catch(...)
          {
            // can do nothing here
          }
          
          deflateEnd(&z_stream_);
        }

        delete [] in_buffer_;
        delete [] out_buffer_;        
      }
  
      inline
      unsigned long long
      OutStreamBuf::in_bytes() const throw()
      {
        return in_bytes_;
      }
      
      inline
      unsigned long long
      OutStreamBuf::out_bytes() const throw()
      {
        return out_bytes_;
      }
      
      inline
      const ACE_Time_Value&
      OutStreamBuf::compression_time() const throw()
      {
        return compression_time_;
      }

      inline
      int
      OutStreamBuf::last_error() throw()
      {
        return last_error_;
      }
      
      inline
      const std::string&
      OutStreamBuf::last_error_desc() const throw(El::Exception)
      {
        return last_error_desc_;
      }
      
      inline
      bool
      OutStreamBuf::push_write_pending() throw(El::Exception)
      {
        for(char* ptr = out_buffer_; write_pending_bytes_; )
        {
          size_t written_bytes =
            callback_->write(ptr, write_pending_bytes_);

          if(written_bytes)
          {
            ptr += written_bytes;
            write_pending_bytes_ -= written_bytes;
            out_bytes_ += written_bytes;
          }
          else
          {
            if(ptr > out_buffer_)
            {
              memmove(out_buffer_, ptr, write_pending_bytes_);
            }
            
            return false;
          }
        }

        return true;
      }

      inline
      void
      OutStreamBuf::finalize() throw(El::Exception)
      {
        if(deflate_initialized_ && !finalized_)
        {
          try
          {
            sync();
            flush_zlib(true);

            finalized_ = true;
          }
          catch(...)
          {
            finalized_ = true;
            throw;
          }
        }
        
      }
      
      inline
      bool
      OutStreamBuf::finalized() throw()
      {
        return finalized_;
      }
      
      inline
      size_t
      OutStreamBuf::in_crc() const throw()
      {
        return in_crc_;
      }

      //
      // InStreamBuf class
      //
      inline
      InStreamBuf::~InStreamBuf() throw()
      {
//        std::cerr << "InStreamBuf::~InStreamBuf()\n";

        if(inflate_initialized_)
        {
          inflateEnd(&z_stream_);
        }

        delete [] in_buffer_;
        delete [] out_buffer_;        
      }
  
      inline
      unsigned long long
      InStreamBuf::in_bytes() const throw()
      {
        return in_bytes_;
      }
      
      inline
      unsigned long long
      InStreamBuf::out_bytes() const throw()
      {
        return out_bytes_;
      }
      
      inline
      size_t
      InStreamBuf::out_crc() const throw()
      {
        return out_crc_;
      }

      inline
      const ACE_Time_Value&
      InStreamBuf::decompression_time() const throw()
      {
        return decompression_time_;
      }

      inline
      int
      InStreamBuf::last_error() throw()
      {
        return last_error_;
      }
      
      inline
      const std::string&
      InStreamBuf::last_error_desc() const throw(El::Exception)
      {
        return last_error_desc_;
      }
      
      //
      // OutStream class
      //
      inline
      OutStream::~OutStream() throw()
      {
//        std::cerr << "OutStream::~OutStream()\n";
      }
      
      inline
      unsigned long long
      OutStream::in_bytes() const throw()
      {
        return streambuf_->in_bytes();
      }
      
      inline
      unsigned long long
      OutStream::out_bytes() const throw()
      {
        return streambuf_->out_bytes();
      }
      
      inline
      const ACE_Time_Value&
      OutStream::compression_time() const throw()
      {
        return streambuf_->compression_time();
      }

      inline
      OutStreamBuf&
      OutStream::zlib_streambuf() const throw()
      {
        return *streambuf_;
      }

      inline
      int
      OutStream::last_error() throw()
      {
        return streambuf_->last_error();
      }
      
      inline
      const std::string&
      OutStream::last_error_desc() const throw(El::Exception)
      {
        return streambuf_->last_error_desc();
      }
      
      inline
      void
      OutStream::finalize() throw(El::Exception)
      {
        if(!streambuf_->finalized())
        {
          flush();
          streambuf_->finalize();
        }
      }
      
      inline
      size_t
      OutStream::in_crc() const throw()
      {
        return streambuf_->in_crc();
      }
      
      //
      // InStream class
      //
      inline
      InStream::~InStream() throw()
      {
//        std::cerr << "InStream::~InStream()\n";
      }
      
      inline
      unsigned long long
      InStream::in_bytes() const throw()
      {
        return streambuf_->in_bytes();
      }
      
      inline
      unsigned long long
      InStream::out_bytes() const throw()
      {
        return streambuf_->out_bytes();
      }
      
      inline
      size_t
      InStream::out_crc() const throw()
      {
        return streambuf_->out_crc();
      }
      
      inline
      const ACE_Time_Value&
      InStream::decompression_time() const throw()
      {
        return streambuf_->decompression_time();
      }

      inline
      InStreamBuf&
      InStream::zlib_streambuf() const throw()
      {
        return *streambuf_;
      }

      inline
      int
      InStream::last_error() throw()
      {
        return streambuf_->last_error();
      }
      
      inline
      const std::string&
      InStream::last_error_desc() const throw(El::Exception)
      {
        return streambuf_->last_error_desc();
      }

      //
      // InputStreamReader class
      //
      inline
      InputStreamReader::InputStreamReader(std::istream& istr)
        throw(El::Exception)
          : istream_(istr)
      {
      }

      inline
      size_t
      InputStreamReader::read(char* buff, size_t len)
      {
        istream_.read(buff, len);
        return istream_.gcount();
      }

      inline
      void
      InputStreamReader::putback(char* buff, size_t len)
      {
        if(istream_.rdstate() & std::ios::eofbit)
        {
          istream_.clear(istream_.rdstate() & ~std::ios::eofbit);
        }
          
        if(istream_.rdstate() & std::ios_base::failbit)
        {
          istream_.clear(istream_.rdstate() & ~std::ios::failbit);
        }

        while(len--)
        {
          istream_.putback(buff[len]);
        }
      }

      //
      // OutStreamWriter class
      //

      inline
      OutStreamWriter::OutStreamWriter(std::ostream& ostr) throw(El::Exception)
          : ostr_(ostr)
      {
      }
  
      inline
      size_t
      OutStreamWriter::write(const char* buff, size_t len)
      {
        ostr_.write(buff, len);
        return len;
      }
      
    }
  }
}

#endif // _ELEMENTS_EL_COMPRESS_ZLIB_HPP_
