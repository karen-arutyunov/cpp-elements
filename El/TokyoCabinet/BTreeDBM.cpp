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
 * @file   Elements/El/TokyoCabinet/BTreeDBM.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <sstream>

#include <El/Exception.hpp>

#include "BTreeDBM.hpp"

namespace El
{
  namespace TokyoCabinet
  {
    BTreeDBM::BTreeDBM(const char *path,
                     int omode,
                     bool thread_safe,
                     int64_t bnum,
                     int8_t apow,
                     int8_t fpow,
                     uint8_t opts,
                     int32_t lcnum,
                     int32_t ncnum,
                     int64_t xmsiz,
                     int32_t dfunit)
      throw(Exception, El::Exception)
        : dbm_(0)
    {
      dbm_ = tcbdbnew();

      if(dbm_ == 0)
      {
        throw Exception("::El::TokyoCabinet::BTreeDBM::BTreeDBM: "
                        "tcbdbnew failed.");
      }

      try
      {
        if(thread_safe && !tcbdbsetmutex(dbm_))
        {
          int ecode = tcbdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::BTreeDBM::BTreeDBM: tcbdbsetmutex "
            "failed (" << ecode << "). Description:\n" << tcbdberrmsg(ecode);

          throw Exception(ostr.str());
        }

        if((bnum > 0 || apow >= 0 || fpow >= 0 || opts != 0) &&
           !tcbdbtune(dbm_, 0, 0, bnum, apow, fpow, opts))
        {
          int ecode = tcbdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::BTreeDBM::BTreeDBM: tcbdbtune failed ("
               << ecode << "). Description:\n" << tcbdberrmsg(ecode);

          throw Exception(ostr.str());
        }

        if((lcnum > 0 || ncnum > 0) && !tcbdbsetcache(dbm_, lcnum, ncnum))
        {
          int ecode = tcbdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::BTreeDBM::BTreeDBM: tcbdbsetcache "
            "failed (" << ecode << "). Description:\n" << tcbdberrmsg(ecode);

          throw Exception(ostr.str());
        }
            
        if(xmsiz >= 0 && !tcbdbsetxmsiz(dbm_, xmsiz))
        {
          int ecode = tcbdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::BTreeDBM::BTreeDBM: tcbdbsetxmsiz "
            "failed (" << ecode << "). Description:\n" << tcbdberrmsg(ecode);

          throw Exception(ostr.str());
        }

        if(dfunit > 0 && !tcbdbsetdfunit(dbm_, dfunit))
        {
          int ecode = tcbdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::BTreeDBM::BTreeDBM: "
            "tcbdbsetdfunit failed (" << ecode
               << "). Description:\n" << tcbdberrmsg(ecode);

          throw Exception(ostr.str());
        }


        if(!tcbdbopen(dbm_, path, omode))
        {
          int ecode = tcbdbecode(dbm_);   
          std::ostringstream ostr;
          
          ostr << "::El::TokyoCabinet::BTreeDBM::BTreeDBM: "
            "tcbdbopen failed (" << ecode
               << "). Description:\n" << tcbdberrmsg(ecode);
          
          throw Exception(ostr.str());
        }
      }
      catch(...)
      {
        tcbdbdel(dbm_);
        throw;
      }
    }
    
    void
    BTreeDBM::insert(const void *kbuf, int ksiz, const void *vbuf, int vsiz)
      throw(Exception, El::Exception)
    {
      if(!tcbdbput(dbm_, kbuf, ksiz, vbuf, vsiz))
      {
        int ecode = tcbdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::BTreeDBM::insert: "
          "tcbdbput failed (" << ecode
             << "). Description:\n" << tcbdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }
    
    int
    BTreeDBM::add_int(const void *kbuf, int ksiz, int num)
      throw(Exception, El::Exception)
    {
      int res = tcbdbaddint(dbm_, kbuf, ksiz, num);
      
      if(res == INT_MIN)
      {
        int ecode = tcbdbecode(dbm_);
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::BTreeDBM::add_int: "
          "tcbdbaddint failed (" << ecode
             << "). Description:\n" << tcbdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
      
      return res;
    }
      
    void
    BTreeDBM::erase(const void *kbuf, int ksiz) throw(Exception, El::Exception)
    {
      if(!tcbdbout(dbm_, kbuf, ksiz))
      {
        int ecode = tcbdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::BTreeDBM::erase: "
          "tcbdbout failed (" << ecode
             << "). Description:\n" << tcbdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }

    void*
    BTreeDBM::find(const void *kbuf, int ksiz, int* vsiz) const
      throw(Exception, El::Exception)
    {
      int size = 0;
      return tcbdbget(dbm_, kbuf, ksiz, vsiz ? vsiz : &size);
    }
    
    void
    BTreeDBM::clear() throw(Exception, El::Exception)
    {
      if(!tcbdbvanish(dbm_))
      {
        int ecode = tcbdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::BTreeDBM::clear: "
          "tcbdbvanish failed ("
             << ecode << "). Description:\n" << tcbdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }

    void
    BTreeDBM::copy(const char* path) const throw(Exception, El::Exception)
    {
      if(!tcbdbcopy(dbm_, path))
      {
        int ecode = tcbdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::BTreeDBM::copy: "
          "tcbdbcopy failed (" << ecode
             << "). Description:\n" << tcbdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }

    const char*
    BTreeDBM::path() const throw()
    {
      return tcbdbpath(dbm_);
    }
    
  }
}
