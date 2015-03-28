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
 * @file Elements/El/CORBA/ZLib/StreamCallback.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CORBA_ZLIB_STREAMCALLBACK_HPP_
#define _ELEMENTS_EL_CORBA_ZLIB_STREAMCALLBACK_HPP_

#include <El/Exception.hpp>
#include <El/Compress/ZLib.hpp>
#include <El/BinaryStream.hpp>

namespace El
{
  namespace Corba
  {
    namespace ZLib
    {
      class OctetArrayWriter : public El::Compress::ZLib::OutStreamCallback
      {
      public:
        EL_EXCEPTION(Exception, El::ExceptionBase);
  
      public:
        OctetArrayWriter(size_t reserve)
          throw(Exception, El::Exception);
        
        virtual ~OctetArrayWriter() throw();
  
        virtual size_t write(const char* buff, size_t len);

        size_t length() const throw();
        const ::CORBA::Octet* data() const throw();

        ::CORBA::Octet* release() throw();

      protected:
        ::CORBA::Octet* buff_;
        size_t buff_size_;
        ::CORBA::Octet* ptr_;
      };

      class OctetArrayReader : public El::Compress::ZLib::InStreamCallback
      {
      public:
        EL_EXCEPTION(Exception, El::ExceptionBase);
  
      public:
        OctetArrayReader(const ::CORBA::Octet* data, size_t length)
          throw(Exception, El::Exception);
  
        virtual ~OctetArrayReader() throw();
  
        virtual size_t read(char* buff, size_t len);
        virtual void putback(char* buff, size_t len);

      private:
        const ::CORBA::Octet* data_;
        size_t length_;
        const ::CORBA::Octet* ptr_;
        const ::CORBA::Octet* data_end_;
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
    namespace ZLib
    {
      //
      // OctetArrayWriter class
      //
      inline
      ::CORBA::Octet*
      OctetArrayWriter::release() throw() 
      {
        ::CORBA::Octet* ret = buff_;

        buff_ = 0;
        ptr_ = 0;
        buff_size_ = 0;

        return ret;
      }

      inline
      OctetArrayWriter::OctetArrayWriter(size_t reserve)
        throw(Exception, El::Exception)
          : buff_(new ::CORBA::Octet[reserve]),
            buff_size_(reserve),
            ptr_(buff_)
      {
      }

      inline
      OctetArrayWriter::~OctetArrayWriter() throw()
      {
        delete [] buff_;
      }

      inline
      size_t
      OctetArrayWriter::length() const throw()
      {
        return ptr_ - buff_;
      }
      
      inline
      const ::CORBA::Octet*
      OctetArrayWriter::data() const throw()
      {
        return buff_;
      }

      inline
      size_t
      OctetArrayWriter::write(const char* buff, size_t len)
      {
        if(ptr_ + len > buff_ + buff_size_)
        {
          size_t written_bytes = ptr_ - buff_;  
          size_t new_len = written_bytes + len;
    
          if(new_len < buff_size_ * 2)
          {
            new_len = buff_size_ * 2;
          }

          ::CORBA::Octet* new_buff = new ::CORBA::Octet[new_len];

          memcpy(new_buff, buff_, written_bytes);
    
          delete [] buff_;
          buff_ = new_buff;
    
          ptr_ = buff_ + written_bytes;
        }

        memcpy(ptr_, buff, len);
        ptr_ += len;
  
        return len;
      }

      //
      // OctetArrayReader class
      //
      inline
      OctetArrayReader::OctetArrayReader(const ::CORBA::Octet* data,
                                         size_t length)
        throw(Exception, El::Exception)
          : data_(data),
            length_(length),
            ptr_(data),
            data_end_(data + length)
      {
      }

      inline
      OctetArrayReader::~OctetArrayReader() throw()
      {
      }

      inline
      size_t
      OctetArrayReader::read(char* buff, size_t len)
      {
        if(ptr_ + len > data_end_)
        {
          len = data_end_ - ptr_;
        }
  
        memcpy(buff, ptr_, len);
        ptr_ += len;
  
        return len;
      }

      inline
      void
      OctetArrayReader::putback(char* buff, size_t len)
      {
        size_t shift = ptr_ - data_;

        if(len > shift)
        {
          len = shift;
        }

        ptr_ -= len;
      }
      
    }
  }
}

#endif // _ELEMENTS_EL_CORBA_ZLIB_STREAMCALLBACK_HPP_
