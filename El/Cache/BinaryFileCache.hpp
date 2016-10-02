/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Cache/BinaryFileCache.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CACHE_BINARYFILECACHE_HPP_
#define _ELEMENTS_EL_CACHE_BINARYFILECACHE_HPP_

#include <El/RefCount/All.hpp>
#include <El/Cache/ObjectCache.hpp>

namespace El
{
  namespace Cache
  {
    class BinaryFile :
      public virtual Object,
      public virtual El::RefCount::DefaultImpl<El::Sync::ThreadPolicy>
    {
    public:
      BinaryFile(Container*,
                 unsigned long long sequence_number,
                 const char* filename) throw();
      ~BinaryFile() throw();

      const unsigned char* buff() const throw(El::Exception);
      size_t size() const throw();

      virtual void read_chunk(const unsigned char* buff, size_t size)
        throw(Exception, El::Exception);

      virtual void reserve(size_t size)
        throw(Exception, El::Exception);
      
    protected:
      unsigned char* buff_;
      size_t size_;
      size_t reserved_;
    };

    typedef El::RefCount::SmartPtr<BinaryFile> BinaryFile_var;

    typedef FileCache<BinaryFile> BinaryFileCache;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Cache
  {
    inline
    BinaryFile::BinaryFile(Container*,
                           unsigned long long sequence_number,
                           const char* filename) throw()
        : Object(sequence_number),
          buff_(0),
          size_(0),
          reserved_(0)
    {
    }
    
    inline
    BinaryFile::~BinaryFile() throw()
    {
      delete [] buff_;
    }

    inline
    const unsigned char*
    BinaryFile::buff() const throw(El::Exception)
    {
      return buff_;
    }
    
    inline
    size_t
    BinaryFile::size() const throw()
    {
      return size_;
    }
    
    inline
    void
    BinaryFile::read_chunk(const unsigned char* buff, size_t size)
      throw(Exception, El::Exception)
    {
      if(reserved_ < size_ + size)
      {
        size_t reserved = size_ + 2 * size;
        unsigned char* new_buff = new unsigned char[reserved];

        if(size_)
        {
          memcpy(new_buff, buff_, size_);
        }

        reserved_ = reserved;
        delete [] buff_;
        buff_ = new_buff;
      }

      if(size)
      {
        memcpy(buff_ + size_, buff, size);
        size_ += size;
      }
    }

    inline
    void
    BinaryFile::reserve(size_t size) throw(Exception, El::Exception)
    {
      if(buff_ != 0)
      {
        throw Exception(
          "El::Cache::BinaryFile::reserve: unexpected call sequence");
      }

      buff_ = new unsigned char[size];
      reserved_ = size;
    }
  }
}

#endif // _ELEMENTS_EL_CACHE_BINARYFILECACHE_HPP_

