/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
