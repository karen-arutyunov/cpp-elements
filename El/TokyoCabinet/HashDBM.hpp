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
 * @file   Elements/El/TokyoCabinet/HashDBM.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_TOKYOCABINET_HASHDBM_HPP_
#define _ELEMENTS_EL_TOKYOCABINET_HASHDBM_HPP_

#include <tchdb.h>

#include <El/Exception.hpp>
#include <El/TokyoCabinet/DBM.hpp>

namespace El
{
  namespace TokyoCabinet
  { 
    class HashDBM : public virtual DBM
    {
    public:
      EL_EXCEPTION(Exception, El::TokyoCabinet::DBM::Exception);

    public:
      
      // Parameters description in
      // http://1978th.net/tokyocabinet/spex-en.html#tchdbapi

      HashDBM(const char *path,
              int omode,
              bool thread_safe = true,
              int64_t bnum = 0,
              int8_t apow = -1,
              int8_t fpow = -1,
              uint8_t opts = HDBTLARGE,
              int32_t rcnum = 0,
              int64_t xmsiz = -1,
              int32_t dfunit = 0)
        throw(Exception, El::Exception);

      virtual void insert(const void *kbuf,
                          int ksiz,
                          const void *vbuf,
                          int vsiz)
        throw(Exception, El::Exception);
      
      void insert_async(const void *kbuf, int ksiz, const void *vbuf, int vsiz)
        throw(Exception, El::Exception);
      
      virtual int add_int(const void *kbuf, int ksiz, int num)
        throw(Exception, El::Exception);
      
      virtual void erase(const void *kbuf, int ksiz)
        throw(Exception, El::Exception);

      virtual void* find(const void *kbuf, int ksiz, int* vsiz = 0) const
        throw(Exception, El::Exception);
      
      virtual void clear() throw(Exception, El::Exception);

      virtual void copy(const char* path) const
        throw(Exception, El::Exception);
      
      virtual const char* path() const throw();
      
    protected:
      virtual ~HashDBM() throw();      
      
    private:
      TCHDB* dbm_;
    };

    typedef RefCount::SmartPtr<HashDBM> HashDBM_var;

  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace TokyoCabinet
  {
    //
    // HashDBM class
    //
    inline
    HashDBM::~HashDBM() throw()
    {
      tchdbclose(dbm_);
      tchdbdel(dbm_);
    }
  }
}

#endif // _ELEMENTS_EL_TOKYOCABINET_HASHDBM_HPP_
