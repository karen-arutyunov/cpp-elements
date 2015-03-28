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
 * @file   Elements/El/TokyoCabinet/DBM.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_TOKYOCABINET_DBM_HPP_
#define _ELEMENTS_EL_TOKYOCABINET_DBM_HPP_

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/SyncPolicy.hpp>

namespace El
{
  namespace TokyoCabinet
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);

    class DBM : public virtual ::El::RefCount::DefaultImpl<Sync::ThreadPolicy>
    {      
    public:
      EL_EXCEPTION(Exception, El::TokyoCabinet::Exception);

      virtual void insert(const void *kbuf,
                          int ksiz,
                          const void *vbuf,
                          int vsiz)
        throw(Exception, El::Exception) = 0;

      virtual int add_int(const void *kbuf, int ksiz, int num)
        throw(Exception, El::Exception) = 0;
      
      virtual void erase(const void *kbuf, int ksiz)
        throw(Exception, El::Exception) = 0;

      virtual void* find(const void *kbuf, int ksiz, int* vsiz = 0) const
        throw(Exception, El::Exception) = 0;

      virtual void clear() throw(Exception, El::Exception) = 0;

      virtual void copy(const char* path) const
        throw(Exception, El::Exception) = 0;

      virtual const char* path() const throw() = 0;

    protected:
      virtual ~DBM() throw() {}
    };

    typedef RefCount::SmartPtr<DBM> DBM_var;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace TokyoCabinet
  {
  }
}

#endif // _ELEMENTS_EL_TOKYOCABINET_DBM_HPP_
