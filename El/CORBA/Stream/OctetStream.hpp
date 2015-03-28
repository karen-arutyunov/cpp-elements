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
 * @file Elements/El/CORBA/Stream/OctetStream.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CORBA_STREAM_OCTETSTREAM_HPP_
#define _ELEMENTS_EL_CORBA_STREAM_OCTETSTREAM_HPP_

#include <streambuf>
#include <iostream>
#include <memory>

#include <El/Exception.hpp>

namespace El
{
  namespace Corba
  {
    namespace Stream
    {
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
      class OutOctetStreamBuf
        : public std::basic_streambuf<char, std::char_traits<char> >
      {
      public:
        OutOctetStreamBuf(size_t reserve = 0) throw(El::Exception);
        virtual ~OutOctetStreamBuf() throw();

        virtual int_type overflow(int_type c);
        virtual int sync();

        size_t length() const throw();
        const ::CORBA::Octet* data() const throw();
        
        ::CORBA::Octet* release() throw();
        
      protected:
        size_t reserve_;
        ::CORBA::Octet* buff_;
        size_t buff_size_;
      };

      class OutOctetStream :
        public std::basic_ostream<char, std::char_traits<char> >
      {
      public:
        OutOctetStream(size_t reserve)
          throw(Exception, El::Exception);

        virtual ~OutOctetStream() throw();

        size_t length() const throw();
        const ::CORBA::Octet* data() const throw();
        
        ::CORBA::Octet* release() throw();        

      protected:
        typedef std::auto_ptr<OutOctetStreamBuf> OutOctetStreamBufPtr;
        OutOctetStreamBufPtr streambuf_;

      private:
        OutOctetStream(const OutOctetStream& );
        void operator=(const OutOctetStream& );
      };

      class InOctetStreamBuf
        : public std::basic_streambuf<char, std::char_traits<char> >
      {
      public:
        InOctetStreamBuf(const ::CORBA::Octet* data, size_t length)
          throw(El::Exception);
        
        virtual ~InOctetStreamBuf() throw();

        virtual std::streamsize showmanyc();
        virtual int_type underflow();
        
      protected:
        const ::CORBA::Octet* data_;
        size_t length_;
     };

      class InOctetStream :
        public std::basic_istream<char, std::char_traits<char> >
      {
      public:
        InOctetStream(const ::CORBA::Octet* data, size_t length)
          throw(Exception, El::Exception);

        virtual ~InOctetStream() throw();

      protected:
        typedef std::auto_ptr<InOctetStreamBuf> InOctetStreamBufPtr;
        InOctetStreamBufPtr streambuf_;

      private:
        InOctetStream(const InOctetStream& );
        void operator=(const InOctetStream& );
      };

    } 
  }  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Corba
  {
    namespace Stream
    {
      //
      // OutOctetStreamBuf class
      //
      inline
      OutOctetStreamBuf::OutOctetStreamBuf(size_t reserve)
        throw(El::Exception)
          : reserve_(reserve ? reserve : 1024),
            buff_(0),
            buff_size_(0)
      {
      }
      
      inline
      OutOctetStreamBuf::~OutOctetStreamBuf() throw()
      {
        delete [] buff_;
      }
  
      inline
      size_t
      OutOctetStreamBuf::length() const throw()
      {
        return pptr() ? pptr() - (char*)buff_ : 0;
      }
      
      inline
      const ::CORBA::Octet*
      OutOctetStreamBuf::data() const throw()
      {
        return buff_;
      }

      inline
      ::CORBA::Octet*
      OutOctetStreamBuf::release() throw()
      {
        ::CORBA::Octet* ret = buff_;

        buff_ = 0;
        buff_size_ = 0;
        setp(0, 0);

        return ret;
      }
      
      inline
      OutOctetStreamBuf::int_type
      OutOctetStreamBuf::overflow(int_type c)
      {
        if(pptr() == 0)
        {
          buff_ = new ::CORBA::Octet[reserve_];
          buff_size_ = reserve_;
            
          buff_[0] = c;
          setp((char*)buff_ + 1, (char*)buff_ + buff_size_);
          return c;
        }

        ::CORBA::Octet* buff = buff_;

        buff_ = new ::CORBA::Octet[buff_size_ * 2];
        buff_size_ *= 2;

        size_t written_bytes = pptr() - (char*)buff;

        memcpy(buff_, buff, written_bytes);
        delete [] buff;
        
        buff_[written_bytes] = c;
        
        setp((char*)buff_ + written_bytes + 1, (char*)buff_ + buff_size_);
        
        return c;
      }
      
      inline
      int
      OutOctetStreamBuf::sync()
      {
        return 0;
      }

      //
      // InOctetStreamBuf class
      //
      inline
      InOctetStreamBuf::InOctetStreamBuf(const ::CORBA::Octet* data,
                                         size_t length)
        throw(El::Exception)
          : data_(data),
            length_(length)
      {
      }
      
      inline
      InOctetStreamBuf::~InOctetStreamBuf() throw()
      {
      }

      inline
      std::streamsize
      InOctetStreamBuf::showmanyc()
      {
        return egptr() - gptr();
      }
      
      inline
      InOctetStreamBuf::int_type
      InOctetStreamBuf::underflow()
      {
        if (gptr() < egptr())
        {
          unsigned char uc = *gptr();
          return uc;
        }

        if(egptr())
        {
          return traits_type::eof();
        }
        
        setg((char*)data_, (char*)data_, (char*)data_ + length_);
        unsigned char uc = *gptr();
        return uc;        
      }
      
      //
      // OutOctetStream class
      //
      inline
      OutOctetStream::OutOctetStream(size_t reserve)
        throw(Exception, El::Exception)
          : std::basic_ostream<char, std::char_traits<char> >(0)
      {
        streambuf_.reset(new OutOctetStreamBuf(reserve));
        init(streambuf_.get());
      }
      
      inline
      OutOctetStream::~OutOctetStream() throw()
      {
      }

      inline
      size_t
      OutOctetStream::length() const throw()
      {
        return streambuf_->length();
      }

      inline
      const ::CORBA::Octet*
      OutOctetStream::data() const throw()
      {
        return streambuf_->data();
      }
        
      inline
      ::CORBA::Octet*
      OutOctetStream::release() throw()
      {
        return streambuf_->release();
      }

      //
      // InOctetStream class
      //
      inline
      InOctetStream::InOctetStream(const ::CORBA::Octet* data,
                                   size_t length)
        throw(Exception, El::Exception)
          : std::basic_istream<char, std::char_traits<char> >(0)
      {
        streambuf_.reset(new InOctetStreamBuf(data, length));
        init(streambuf_.get());
      }
      
      inline
      InOctetStream::~InOctetStream() throw()
      {
      }
    }
  }
}

#endif // _ELEMENTS_EL_CORBA_STREAM_OCTETSTREAM_HPP_
