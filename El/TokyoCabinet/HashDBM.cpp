/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/TokyoCabinet/HashDBM.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <sstream>

#include <El/Exception.hpp>

/*
#include <string.h>

#include <ace/OS.h>

#include <El/String/Manip.hpp>
*/

#include "HashDBM.hpp"

namespace El
{
  namespace TokyoCabinet
  {
    HashDBM::HashDBM(const char *path,
                     int omode,
                     bool thread_safe,
                     int64_t bnum,
                     int8_t apow,
                     int8_t fpow,
                     uint8_t opts,
                     int32_t rcnum,
                     int64_t xmsiz,
                     int32_t dfunit)
      throw(Exception, El::Exception)
        : dbm_(0)
    {
      dbm_ = tchdbnew();

      if(dbm_ == 0)
      {
        throw Exception("::El::TokyoCabinet::HashDBM::HashDBM: "
                        "tchdbnew failed.");
      }

      try
      {
        if(thread_safe && !tchdbsetmutex(dbm_))
        {
          int ecode = tchdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::HashDBM::HashDBM: tchdbsetmutex "
            "failed (" << ecode << "). Description:\n" << tchdberrmsg(ecode);

          throw Exception(ostr.str());
        }

        if((bnum > 0 || apow >= 0 || fpow >= 0 || opts != 0) &&
           !tchdbtune(dbm_, bnum, apow, fpow, opts))
        {
          int ecode = tchdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::HashDBM::HashDBM: tchdbtune failed ("
               << ecode << "). Description:\n" << tchdberrmsg(ecode);

          throw Exception(ostr.str());
        }

        if(rcnum > 0 && !tchdbsetcache(dbm_, rcnum))
        {
          int ecode = tchdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::HashDBM::HashDBM: tchdbsetcache "
            "failed (" << ecode << "). Description:\n" << tchdberrmsg(ecode);

          throw Exception(ostr.str());
        }
            
        if(xmsiz >= 0 && !tchdbsetxmsiz(dbm_, xmsiz))
        {
          int ecode = tchdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::HashDBM::HashDBM: tchdbsetxmsiz "
            "failed (" << ecode << "). Description:\n" << tchdberrmsg(ecode);

          throw Exception(ostr.str());
        }

        if(dfunit > 0 && !tchdbsetdfunit(dbm_, dfunit))
        {
          int ecode = tchdbecode(dbm_);   
          std::ostringstream ostr;

          ostr << "::El::TokyoCabinet::HashDBM::HashDBM: "
            "tchdbsetdfunit failed (" << ecode
               << "). Description:\n" << tchdberrmsg(ecode);

          throw Exception(ostr.str());
        }


        if(!tchdbopen(dbm_, path, omode))
        {
          int ecode = tchdbecode(dbm_);   
          std::ostringstream ostr;
          
          ostr << "::El::TokyoCabinet::HashDBM::HashDBM: "
            "tchdbopen failed (" << ecode
               << "). Description:\n" << tchdberrmsg(ecode);
          
          throw Exception(ostr.str());
        }
      }
      catch(...)
      {
        tchdbdel(dbm_);
        throw;
      }
    }
    
    void
    HashDBM::insert(const void *kbuf, int ksiz, const void *vbuf, int vsiz)
      throw(Exception, El::Exception)
    {
      if(!tchdbput(dbm_, kbuf, ksiz, vbuf, vsiz))
      {
        int ecode = tchdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::HashDBM::insert: "
          "tchdbput failed (" << ecode
             << "). Description:\n" << tchdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }
    
    void
    HashDBM::insert_async(const void *kbuf,
                          int ksiz,
                          const void *vbuf,
                          int vsiz)
      throw(Exception, El::Exception)
    {
      if(!tchdbputasync(dbm_, kbuf, ksiz, vbuf, vsiz))
      {
        int ecode = tchdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::HashDBM::insert_async: "
          "tchdbputasync failed (" << ecode
             << "). Description:\n" << tchdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }

    int
    HashDBM::add_int(const void *kbuf, int ksiz, int num)
      throw(Exception, El::Exception)
    {
      int res = tchdbaddint(dbm_, kbuf, ksiz, num);
      
      if(res == INT_MIN)
      {
        int ecode = tchdbecode(dbm_);
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::HashDBM::add_int: "
          "tchdbaddint failed (" << ecode
             << "). Description:\n" << tchdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
      
      return res;
    }
      
    void
    HashDBM::erase(const void *kbuf, int ksiz) throw(Exception, El::Exception)
    {
      if(!tchdbout(dbm_, kbuf, ksiz))
      {
        int ecode = tchdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::HashDBM::erase: "
          "tchdbout failed (" << ecode
             << "). Description:\n" << tchdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }

    void*
    HashDBM::find(const void *kbuf, int ksiz, int* vsiz) const
      throw(Exception, El::Exception)
    {
      int size = 0;
      return tchdbget(dbm_, kbuf, ksiz, vsiz ? vsiz : &size);
    }
    
    void
    HashDBM::clear() throw(Exception, El::Exception)
    {
      if(!tchdbvanish(dbm_))
      {
        int ecode = tchdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::HashDBM::clear: "
          "tchdbvanish failed ("
             << ecode << "). Description:\n" << tchdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }

    void
    HashDBM::copy(const char* path) const throw(Exception, El::Exception)
    {
      if(!tchdbcopy(dbm_, path))
      {
        int ecode = tchdbecode(dbm_);   
        std::ostringstream ostr;

        ostr << "::El::TokyoCabinet::HashDBM::copy: "
          "tchdbcopy failed (" << ecode
             << "). Description:\n" << tchdberrmsg(ecode);
        
        throw Exception(ostr.str());
      }
    }

    const char*
    HashDBM::path() const throw()
    {
      return tchdbpath(dbm_);
    }
    
  }
}
