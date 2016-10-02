/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
