/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/LightArray.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LIGHTARRAY_HPP_
#define _ELEMENTS_EL_LIGHTARRAY_HPP_

#include <vector>
#include <sstream>
#include <algorithm>

#include <El/Exception.hpp>
#include <El/BinaryStream.hpp>
#include <El/ArrayPtr.hpp>

namespace El
{
  template<typename TYPE, typename SIZE>
  class LightArray
  {
  public:
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);

    typedef SIZE Size;
    typedef TYPE Element;

    typedef TYPE* iterator;
    typedef const TYPE* const_iterator;
    
  public:
    LightArray() throw();
    LightArray(SIZE size) throw(El::Exception);
    LightArray(const std::vector<TYPE>& src) throw(El::Exception);
    LightArray(const LightArray<TYPE, SIZE>& src) throw(El::Exception);

    ~LightArray() throw();

    LightArray<TYPE, SIZE>& operator=(const LightArray<TYPE, SIZE>& src)
      throw(El::Exception);

    LightArray<TYPE, SIZE>& operator=(const std::vector<TYPE>& src)
      throw(El::Exception);

    LightArray<TYPE, SIZE>& raw_copy(const LightArray<TYPE, SIZE>& src)
      throw(El::Exception);

    SIZE size() const throw();
    void resize(SIZE val, bool no_realloc = false) throw(El::Exception);
    void erase(const_iterator it) throw(El::Exception);
    
    bool empty() const throw();
    void clear() throw(El::Exception);

    TYPE& operator[](SIZE index) throw();
    const TYPE& operator[](SIZE index) const throw();

    bool operator==(const LightArray<TYPE, SIZE>& val) const
      throw(El::Exception);

    void copy(const LightArray<TYPE, SIZE>& src,
              SIZE dest_position,
              SIZE count)
      throw(InvalidArg, El::Exception);

    const TYPE* elements() const throw() { return elements_.get(); }
    TYPE* elements() throw() { return elements_.get(); }

    const_iterator begin() const throw() { return elements_.get(); }
    iterator begin() throw() { return elements_.get(); }

    const_iterator end() const throw() { return elements_.get() + size_; }
    iterator end() throw() { return elements_.get() + size_; }

    void reset(TYPE* elements, SIZE size) throw();

    void sort() throw(El::Exception);

    template<typename SOURCE> void init(const SOURCE& src)
      throw(El::Exception);    

    template<typename SOURCE_ITER> void init(const SOURCE_ITER from,
                                             const SOURCE_ITER to)
      throw(El::Exception);    

    bool find(const TYPE& val, SIZE* index = 0) const throw();

    void release(LightArray<TYPE, SIZE>& dest) throw();
    void swap(LightArray<TYPE, SIZE>& value) throw();
    
    void write(El::BinaryOutStream& ostr) const
      throw(Exception, El::Exception);

    void read(El::BinaryInStream& istr) throw(Exception, El::Exception);
    
  protected:
    ArrayPtr<TYPE> elements_;
    SIZE  size_;
  };
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  template<typename TYPE, typename SIZE>
  LightArray<TYPE, SIZE>::LightArray() throw()
      : size_(0)
  {
  }
  
  template<typename TYPE, typename SIZE>
  LightArray<TYPE, SIZE>::LightArray(SIZE size) throw(El::Exception)
      : elements_(size ? new TYPE[size] : 0),
        size_(size)
  {
  }
  
  template<typename TYPE, typename SIZE>
  LightArray<TYPE, SIZE>::LightArray(const std::vector<TYPE>& src)
    throw(El::Exception)
      : elements_(src.size() ? new TYPE[src.size()] : 0),
        size_(src.size())
  {
    
    for(SIZE i = 0; i < size_; i++)
    {
      elements_[i] = src[i];
    }
  }

  template<typename TYPE, typename SIZE>
  LightArray<TYPE, SIZE>::LightArray(const LightArray<TYPE, SIZE>& src)
    throw(El::Exception)
      : elements_(src.size() ? new TYPE[src.size()] : 0),
        size_(src.size())
  {
    iterator i = begin(), e = end();
    const_iterator is = src.begin();

    for(; i != e; ++i, ++is)
    {
      *i = *is;
    }
  }

  template<typename TYPE, typename SIZE>
  LightArray<TYPE, SIZE>::~LightArray() throw()
  {
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::copy(const LightArray<TYPE, SIZE>& src,
                               SIZE dest_position,
                               SIZE count)
    throw(InvalidArg, El::Exception)
  {
    if(count > src.size())
    {
      std::ostringstream ostr;
      ostr << "El::LightArray::copy: try to copy more elements "
        "than source array size";

      throw InvalidArg(ostr.str());      
    }
    
    if(dest_position + count > size_)
    {
      std::ostringstream ostr;
      ostr << "El::LightArray::copy: try to copy outside array";

      throw InvalidArg(ostr.str());
    }

    iterator i = begin() + dest_position, e = i + count;
    const_iterator is = src.begin();

    for(; i != e; ++i, ++is)
    {
      *i = *is;
    }
  }
  
  template<typename TYPE, typename SIZE>
  LightArray<TYPE, SIZE>&
  LightArray<TYPE, SIZE>::raw_copy(const LightArray<TYPE, SIZE>& src)
    throw(El::Exception)
  {
    size_ = src.size();
    elements_.clear();

    if(size_)
    {
      elements_.reset(new TYPE[size_]);
      memcpy(elements_.get(), src.elements(), size_ * sizeof(TYPE));  
    }

    return *this;
  }
    
  template<typename TYPE, typename SIZE>
  LightArray<TYPE, SIZE>&
  LightArray<TYPE, SIZE>::operator=(const LightArray<TYPE, SIZE>& src)
    throw(El::Exception)
  {
    size_ = src.size();
    elements_.reset(size_ ? new TYPE[size_] : 0);

    iterator i = begin(), e = end();
    const_iterator is = src.begin();

    for(; i != e; ++i, ++is)
    {
      *i = *is;
    }

    return *this;
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::reset(TYPE* elements, SIZE size) throw()
  {
    elements_.reset(elements);
    size_ = size;
  }
  
  template<typename TYPE, typename SIZE>
  LightArray<TYPE, SIZE>&
  LightArray<TYPE, SIZE>::operator=(const std::vector<TYPE>& src)
    throw(El::Exception)
  {
    size_ = src.size();
    elements_.reset(size_ ? new TYPE[size_] : 0);

    iterator i = begin(), e = end();
    typename std::vector<TYPE>::const_iterator is = src.begin();

    for(; i != e; ++i, ++is)
    {
      *i = *is;
    }

    return *this;
  }
  
  template<typename TYPE, typename SIZE>
  bool
  LightArray<TYPE, SIZE>::operator==(const LightArray<TYPE, SIZE>& val) const
    throw(El::Exception)
  {
    if(size_ != val.size_)
    {
      return false;
    }

    const_iterator i = begin(), e = end();
    const_iterator iv = val.begin();

    for(; i != e && *i == *iv; ++i, ++iv);
    return i == e;
  }

  template<typename TYPE, typename SIZE>
  SIZE
  LightArray<TYPE, SIZE>::size() const throw()
  {
    return size_;
  }

  template<typename TYPE, typename SIZE>
  bool
  LightArray<TYPE, SIZE>::empty() const throw()
  {
    return size_ == 0;
  }
  
  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::resize(SIZE val, bool no_realloc)
    throw(El::Exception)
  {
    if(size_ == val)
    {
      return;
    }

    if(no_realloc && val < size_)
    {
      iterator i = begin() + val, e = end();

      for(; i != e; ++i)
      {
        *i = TYPE();
      }
      
      size_ = val;
    }
    else
    {
      ArrayPtr<TYPE> new_elements(val ? new TYPE[val] : 0);      
      SIZE count = std::min(val, size_);
      
      for(SIZE i = 0; i < count; i++)
      {
        new_elements[i] = elements_[i];
      }
      
      size_ = val;
      elements_.reset(new_elements.release());
    }
    
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::clear() throw(El::Exception)
  {
    resize(0);
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::erase(const_iterator it) throw(El::Exception)
  {
    const_iterator e = end();
    
    if(it != e)
    {
      ArrayPtr<Element> elements(new Element[size_ - 1]);
      Element* dest = elements.get();
      const_iterator src = begin();
      
      for(; src != it; src++)
      {
        *dest++ = *src;
      }

      for(src++; src != it; src++)
      {
        *dest++ = *src;
      }

      elements_.reset(elements.release());
      size_--;
    }
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::sort() throw(El::Exception)
  {
    std::sort(elements_.get(), elements_.get() + size_);
  }

  template<typename TYPE, typename SIZE>
  TYPE&
  LightArray<TYPE, SIZE>::operator[](SIZE index) throw()
  {
    return elements_[index];
  }

  template<typename TYPE, typename SIZE>
  const TYPE&
  LightArray<TYPE, SIZE>::operator[](SIZE index) const throw()
  {
    return elements_[index];
  }
  
  template<typename TYPE, typename SIZE>
  template<typename SOURCE>
  void
  LightArray<TYPE, SIZE>::init(const SOURCE& src) throw(El::Exception)
  {
    init(src.begin(), src.end());
/*    
    LightArray<TYPE, SIZE> array(src.size());

    iterator i = array.begin(), e = array.end();
    typename SOURCE::const_iterator is = src.begin();

    for(; i != e; ++i, ++is)
    {
      *i = *is;
    }

    swap(array);
*/
  }

  template<typename TYPE, typename SIZE>
  template<typename SOURCE_ITER>
  void
  LightArray<TYPE, SIZE>::init(const SOURCE_ITER from,
                               const SOURCE_ITER to)
    throw(El::Exception)
  {
    LightArray<TYPE, SIZE> array(to - from);

    iterator i = array.begin(), e = array.end();
    SOURCE_ITER is = from;

    for(; i != e; ++i, ++is)
    {
      *i = *is;
    }

    swap(array);
  }

  template<typename TYPE, typename SIZE>
  bool
  LightArray<TYPE, SIZE>::find(const TYPE& val, SIZE* index) const throw()
  {
    const_iterator i = begin(), e = end();

    for(; i != e; ++i)
    {
      if(*i == val)
      {
        if(index)
        {
          *index = i - begin();
        }
        
        return true;
      }
    }
    
    return false;
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::release(LightArray<TYPE, SIZE>& dest) throw()
  {
    dest.elements_.reset(elements_.release());
    dest.size_ = size_;    
    size_ = 0;
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::swap(LightArray<TYPE, SIZE>& value) throw()
  {
    LightArray<TYPE, SIZE> tmp;

    release(tmp);
    value.release(*this);
    tmp.release(value);
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::write(El::BinaryOutStream& ostr) const
    throw(Exception, El::Exception)
  {
    ostr.write_array(*this);
  }

  template<typename TYPE, typename SIZE>
  void
  LightArray<TYPE, SIZE>::read(El::BinaryInStream& istr)
    throw(Exception, El::Exception)
  {
    istr.read_array(*this);
  }
}

#endif // _ELEMENTS_EL_LIGHTARRAY_HPP_
