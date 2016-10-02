/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/TokyoCabinet/BTreeDBM.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_TOKYOCABINET_BTREEDBM_HPP_
#define _ELEMENTS_EL_TOKYOCABINET_BTREEDBM_HPP_

#include <tcbdb.h>

#include <El/Exception.hpp>
#include <El/TokyoCabinet/DBM.hpp>

namespace El
{
  namespace TokyoCabinet
  { 
    class BTreeDBM : public virtual DBM
    {
    public:
      EL_EXCEPTION(Exception, El::TokyoCabinet::DBM::Exception);

    public:
      
      // Parameters description in
      // http://1978th.net/tokyocabinet/spex-en.html#tcbdbapi

      BTreeDBM(const char *path,
              int omode,
              bool thread_safe = true,
              int64_t bnum = 0,
              int8_t apow = -1,
              int8_t fpow = -1,
              uint8_t opts = HDBTLARGE,
              int32_t lcnum = 0,
              int32_t ncnum = 0,
              int64_t xmsiz = -1,
              int32_t dfunit = 0)
        throw(Exception, El::Exception);

      virtual void insert(const void *kbuf,
                          int ksiz,
                          const void *vbuf,
                          int vsiz)
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
      virtual ~BTreeDBM() throw();      
      
    private:
      TCBDB* dbm_;
    };

    typedef RefCount::SmartPtr<BTreeDBM> BTreeDBM_var;

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
    // BTreeDBM class
    //
    inline
    BTreeDBM::~BTreeDBM() throw()
    {
      tcbdbclose(dbm_);
      tcbdbdel(dbm_);
    }
  }
}

#endif // _ELEMENTS_EL_TOKYOCABINET_BTREEDBM_HPP_
