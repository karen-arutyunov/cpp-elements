/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Cache/TextFileCache.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_CACHE_TEXTFILECACHE_HPP_
#define _ELEMENTS_EL_CACHE_TEXTFILECACHE_HPP_

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/Cache/BinaryFileCache.hpp>

namespace El
{
  namespace Cache
  {
    class TextFile : public virtual BinaryFile
    {
    public:
      TextFile(Container*,
               unsigned long long sequence_number,
               const char* filename) throw();
      
      ~TextFile() throw();

      //
      // Returns text length, which is one byte less
      // than value provided by size method
      //
      size_t length() const throw();
      const char* text() const throw();

      virtual void read_chunk(const unsigned char* buff, size_t size)
        throw(Exception, El::Exception);

      virtual void reserve(size_t size)
        throw(Exception, El::Exception);      
    };

    typedef El::RefCount::SmartPtr<TextFile> TextFile_var;
    typedef FileCache<TextFile> TextFileCache;
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
    TextFile::TextFile(Container* container,
                       unsigned long long sequence_number,
                       const char* filename) throw()
        : Object(sequence_number),
          BinaryFile(container, sequence_number, filename)
    {
    }
    
    inline
    TextFile::~TextFile() throw()
    {
    }

    inline
    void
    TextFile::read_chunk(const unsigned char* buff, size_t size)
      throw(Exception, El::Exception)
    {
      if(size)
      {
        BinaryFile::read_chunk(buff, size);
      }
      else
      {
        unsigned char n = 0;
        BinaryFile::read_chunk(&n, 1);
        BinaryFile::read_chunk(0, 0);
      }
    }

    inline
    void
    TextFile::reserve(size_t size) throw(Exception, El::Exception)
    {
      BinaryFile::reserve(size + 1);
    }

    inline
    size_t
    TextFile::length() const throw()
    {
      return size_ ? size_ - 1 : 0;
    }

    inline
    const char*
    TextFile::text() const throw()
    {
      return (const char*)buff_;
    }
    
  }
}

#endif // _ELEMENTS_EL_CACHE_TEXTFILECACHE_HPP_

