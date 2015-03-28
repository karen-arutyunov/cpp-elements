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
 * @file Elements/El/Image/ImageInfo.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_IMAGE_IMAGE_INFO_HPP_
#define _ELEMENTS_EL_IMAGE_IMAGE_INFO_HPP_

#include <stdint.h>
#include <iostream>

#include <El/Exception.hpp>

namespace El
{
  namespace Image
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);

    enum ImageType
    {
      IT_UNKNOWN,
      IT_GIF,
      IT_JPEG,
      IT_PNG,
      IT_BMP
    };
    
    struct ImageInfo
    {
    public:
      ImageType type;
      uint32_t width;
      uint32_t height;
      
    public:
      ImageInfo(ImageType t = IT_UNKNOWN, uint32_t w = 0, uint32_t h = 0)
        throw(El::Exception);
      
      ~ImageInfo() throw();

      bool read(std::istream& istr) throw(Exception, El::Exception);

      const char* type_name() const throw(El::Exception);

    private:

      bool read_jpeg_info(std::istream& istr)
        throw(Exception, El::Exception);
      
      bool read_gif_info(std::istream& istr)
        throw(Exception, El::Exception);
      
      bool read_png_info(std::istream& istr)
        throw(Exception, El::Exception);
      
      bool read_bmp_info(std::istream& istr)
        throw(Exception, El::Exception);

    private:

      static uint32_t ulong_le(const unsigned char* buff) throw();
      static uint32_t ulong_be(const unsigned char* buff) throw();
      static uint16_t ushort_le(const unsigned char* buff) throw();
      static uint16_t ushort_be(const unsigned char* buff) throw();
      
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Image
  {
    inline
    ImageInfo::ImageInfo(ImageType t, uint32_t w, uint32_t h)
      throw(El::Exception)
        : type(t), width(w), height(h)
    {
    }
    
    inline
    ImageInfo::~ImageInfo() throw()
    {
    }

    inline
    const char*
    ImageInfo::type_name() const throw(El::Exception)
    {
      switch(type)
      {
      case IT_UNKNOWN: return "unknown";
      case IT_GIF: return "gif";
      case IT_JPEG: return "jpeg";
      case IT_PNG: return "png";
      case IT_BMP: return "bmp";
      }

      return"";
    }

    inline
    uint32_t
    ImageInfo::ulong_le(const unsigned char* buff) throw()
    {
      return (((uint32_t)buff[3]) << 24) | (((uint32_t)buff[2]) << 16) |
        (((uint32_t)buff[1]) << 8) | ((uint32_t)buff[0]);
    }
    
    inline
    uint32_t
    ImageInfo::ulong_be(const unsigned char* buff) throw()
    {
      return (((uint32_t)buff[0]) << 24) | (((uint32_t)buff[1]) << 16) |
        (((uint32_t)buff[2]) << 8) | ((uint32_t)buff[3]);
    }
    
    inline
    uint16_t
    ImageInfo::ushort_le(const unsigned char* buff) throw()
    {
      return (((uint16_t)buff[1]) << 8) | ((uint16_t)buff[0]);
    }
    
    inline
    uint16_t
    ImageInfo::ushort_be(const unsigned char* buff) throw()
    {
      return (((uint16_t)buff[0]) << 8) | ((uint16_t)buff[1]);
    }
  }
}

#endif // _ELEMENTS_EL_IMAGE_IMAGE_INFO_HPP_
