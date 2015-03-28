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
 * @file Elements/El/Geography/AddressInfo.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <iostream>

#include <El/Exception.hpp>

#include "ImageInfo.hpp"

namespace El
{
  namespace Image
  {
    bool
    ImageInfo::read(std::istream& istr) throw(Exception, El::Exception)
    {
      type = IT_UNKNOWN;
      width = 0;
      height = 0;
      
      unsigned char buff[10];

      istr.read((char*)buff, 2);

      if(istr.fail())
      {
        return false;
      }

      if(memcmp(buff, "\xFF\xD8", 2) == 0)
      {
        return read_jpeg_info(istr);
      }

      if(memcmp(buff, "BM", 2) == 0)
      {
        return read_bmp_info(istr);
      }

      istr.read((char*)buff + 2, 4);

      if(istr.fail())
      {
        return false;
      }
      
      if(memcmp(buff, "GIF87a", 6) == 0 || memcmp(buff, "GIF89a", 6) == 0)
      {
        return read_gif_info(istr);
      }
      
      istr.read((char*)buff + 6, 2);

      if(istr.fail())
      {
        return false;
      }
      
      if(memcmp(buff, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8) == 0)
      {
        return read_png_info(istr);
      }
      
      return false;
    }

    bool
    ImageInfo::read_jpeg_info(std::istream& istr)
      throw(Exception, El::Exception)
    {
      unsigned char buff[128];
      
      while(true)
      {
        istr.read((char*)buff, 2);
    
        if(istr.fail())
        {
          return false;
        }

        unsigned char chr = 0;
        if(buff[0] == 0xFF && (chr = buff[1]) >= 0xC0 && chr <= 0xC3)
        {
          // Skip len and some byte
          istr.read((char*)buff, 3);
      
          if(istr.fail())
          {
            return false;
          }

          istr.read((char*)buff, 2);
      
          if(istr.fail())
          {
            return false;
          }

          height = ushort_be(buff);

          istr.read((char*)buff, 2);
      
          if(istr.fail())
          {
            return false;
          }

          width = ushort_be(buff);

          type = IT_JPEG;
      
          return true;
        }
        else
        {
          istr.read((char*)buff, 2);
      
          if(istr.fail())
          {
            return false;
          }

          size_t len  = (((size_t)buff[0]) << 8) | ((size_t)buff[1]);

          if(len < 2)
          {
            return false;
          }

          len -= 2;

          while(len)
          {
            size_t read_bytes = std::min(len, (size_t)sizeof(buff));
            istr.read((char*)buff, read_bytes);
      
            if(istr.fail())
            {
              return false;
            }

            len -= read_bytes;
          }
        }
      }

      return false;
    }
    
    bool
    ImageInfo::read_png_info(std::istream& istr)
      throw(Exception, El::Exception)
    {
      unsigned char buff[128];

      while(true)
      {
        istr.read((char*)buff, 4);
      
        if(istr.fail())
        {
          return false;
        }

        size_t len = ulong_be(buff);

        istr.read((char*)buff, 4);
      
        if(istr.fail())
        {
          return false;
        }

        if(strncmp((char*)buff, "IHDR", 4) == 0)
        {
          istr.read((char*)buff, 4);
      
          if(istr.fail())
          {
            return false;
          }
      
          width = ulong_be(buff);
      
          istr.read((char*)buff, 4);
      
          if(istr.fail())
          {
            return false;
          }
      
          height = ulong_be(buff);

          type = IT_PNG;
          
          return true;
        }
        else
        {
          while(len)
          {
            size_t read_bytes = std::min(len, (size_t)sizeof(buff));

            istr.read((char*)buff, read_bytes);
      
            if(istr.fail())
            {
              return false;
            }

            len -= read_bytes;
          }

          istr.read((char*)buff, 2); // CRC
      
          if(istr.fail())
          {
            return false;
          }
        }
      }

      return false;
    }
    
    bool
    ImageInfo::read_gif_info(std::istream& istr)
      throw(Exception, El::Exception)
    {
      unsigned char buff[128];

      istr.read((char*)buff, 2);
      
      if(istr.fail())
      {
        return false;
      }

      width = ushort_le(buff);

      istr.read((char*)buff, 2);
      
      if(istr.fail())
      {
        return false;
      }

      height = ushort_le(buff);
      
      type = IT_GIF;
      
      return true;
    }
    
    bool
    ImageInfo::read_bmp_info(std::istream& istr)
      throw(Exception, El::Exception)
    {
      unsigned char buff[128];

      istr.read((char*)buff, 16); // skipping some not interesting data
      
      if(istr.fail())
      {
        return false;
      }

      istr.read((char*)buff, 4);
      
      if(istr.fail())
      {
        return false;
      }

      width = ulong_le(buff);

      istr.read((char*)buff, 2);
      
      if(istr.fail())
      {
        return false;
      }

      height = ulong_le(buff);
      
      type = IT_BMP;
      
      return true;
    }
    
  }
}
