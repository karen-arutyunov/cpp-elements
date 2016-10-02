/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/String/SharedString.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STRING_SHAREDSTRING_HPP_
#define _ELEMENTS_EL_STRING_SHAREDSTRING_HPP_

#include <stdint.h>
#include <iostream>
#include <string>

//#include <ext/hash_fun.h>
#include <google/sparse_hash_set>

#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>
#include <El/ArrayPtr.hpp>
#include <El/BinaryStream.hpp>
#include <El/String/StringPtr.hpp>

namespace El
{
  namespace String
  {
    class SharedStringManager
    {
    public:
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
    public:
      SharedStringManager() throw(El::Exception);
      ~SharedStringManager() throw();

      const char* add(const char* str) throw(El::Exception);
      const char* add_ref(const char* str) throw(El::Exception);
      void remove(const char* str) throw(El::Exception);

      bool empty() const throw();
      
      unsigned long hash(const char* str) const throw();
      bool equal(const char* str1, const char* str2) const throw();
      
      void write_string(El::BinaryOutStream& ostr, const char* str) const
        throw(El::Exception);
      
      const char* read_string(El::BinaryInStream& istr) throw(El::Exception);

      void optimize_mem_usage() throw(El::Exception);
      
      struct Info
      {
        unsigned long strings;
        unsigned long string_refs;
        unsigned long mem_usage;
        float compression;

        void dump(std::ostream& ostr) const throw(El::Exception);

        Info() throw();
      };

      Info info() const throw();

      void dump(std::ostream& ostr, unsigned long dump_strings_count = 0) const
        throw(El::Exception);
      
    private:

      Info info_() const throw();
      
      const char* add(const char* str, size_t len) throw(El::Exception);
      
      typedef ACE_Thread_Mutex Mutex;
      typedef ACE_Read_Guard<Mutex> ReadGuard;
      typedef ACE_Write_Guard<Mutex> WriteGuard;

      mutable Mutex lock_;

      class StringSet :
        public google::sparse_hash_set<StringConstPtr,
                                       El::Hash::StringConstPtr>
      {
      public:
        StringSet() throw(El::Exception) { set_deleted_key(0); }
        ~StringSet() throw();

        static unsigned long* buffer(const char* str) throw();
      };

      StringSet strings_;
      
      El::ArrayPtr<char> buff_;
      size_t buff_len_;
      unsigned long references_;

      static const char EMPTY[];
      
    private:
      void operator=(const SharedStringManager&);
      SharedStringManager(const SharedStringManager&);
    };

    // Made as a template not a class to be able creating several pointer-types
    // working with individual shared string manager singletons.
    // T is semantically a type signature.
    
    template<typename T>
    class SharedStringConstPtr
    {
    public:
      SharedStringConstPtr(const char* str = 0) throw(El::Exception);
      SharedStringConstPtr(const std::string& str) throw(El::Exception);
      
      SharedStringConstPtr(const SharedStringConstPtr& str)
        throw(El::Exception);

      ~SharedStringConstPtr() throw();
      
      const char* c_str() const throw(El::Exception);
      size_t length() const throw();
      
      SharedStringConstPtr& operator=(const char* str) throw(El::Exception);
      
      SharedStringConstPtr& operator=(const std::string& str)
        throw(El::Exception);

      SharedStringConstPtr& operator=(const SharedStringConstPtr& str)
        throw(El::Exception);

      bool operator==(const SharedStringConstPtr& str) const throw();
      bool operator!=(const SharedStringConstPtr& str) const throw();
      bool operator<(const SharedStringConstPtr& str) const throw();
      bool operator>(const SharedStringConstPtr& str) const throw();

      bool operator==(const char* str) const throw();
      bool operator!=(const char* str) const throw();
      bool operator<(const char* str) const throw();
      bool operator>(const char* str) const throw();

      void reset(const char* str = 0) throw(El::Exception);
      void clear() throw(El::Exception);
      bool empty() const throw();

      void swap(SharedStringConstPtr& str) throw();

      unsigned long hash() const throw();

      void write(El::BinaryOutStream& ostr) const
        throw(Exception, El::Exception);

      void read(El::BinaryInStream& istr) throw(Exception, El::Exception);

      const char* add_ref() const throw(El::Exception);
      void remove() const throw(El::Exception);
      
      const char* release() throw(El::Exception);

      static const SharedStringConstPtr null;
      static El::String::SharedStringManager string_manager;

    protected:
      const char* str_;
    };
    
  }
}

namespace El
{
  namespace Hash
  {
    template<typename T>
    struct SharedStringConstPtr
    {
      size_t operator()(const El::String::SharedStringConstPtr<T>& str) const
        throw(El::Exception);
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

template<typename T>
std::ostream&
operator<<(std::ostream& ostr, const El::String::SharedStringConstPtr<T>& str)
  throw(El::Exception)
{
  ostr << (str.c_str() ? str.c_str() : "");
  return ostr;
}

namespace El
{
  namespace String
  {
    //
    // SharedStringManager::Info struct
    //
    inline
    SharedStringManager::Info::Info() throw()
        : strings(0),
          string_refs(0),
          mem_usage(0),
          compression(0)
    {
    }
    
    //
    // SharedStringManager class
    //
    inline
    SharedStringManager::SharedStringManager() throw(El::Exception)
        : buff_len_(0),
          references_(0)
    {
    }
    
    inline
    SharedStringManager::~SharedStringManager() throw()
    {
/*      
      if(!strings_.empty())
      {
        std::cerr << "SharedStringManager::~SharedStringManager: storage "
          "is not empty while being destructed - really bad design:";

        dump(std::cerr, 30);
      }
*/
    }

    inline
    SharedStringManager::Info
    SharedStringManager::info() const throw()
    {
      ReadGuard guard(lock_);
      return info_();
    }
    
    inline
    bool
    SharedStringManager::empty() const throw()
    {
      ReadGuard guard(lock_);
      return strings_.empty();
    }

    inline
    unsigned long
    SharedStringManager::hash(const char* str) const throw()
    {
      return (unsigned long)str;
    }
    
    inline
    bool
    SharedStringManager::equal(const char* str1, const char* str2) const
      throw()
    {      
      return str1 == str2;
    }
    
    inline
    void
    SharedStringManager::write_string(El::BinaryOutStream& ostr,
                                      const char* str) const
      throw(El::Exception)
    {
      ostr.write_string(str);
    }

    inline
    const char*
    SharedStringManager::add(const char* str, size_t len)
      throw(El::Exception)
    {      
      len++;
      references_++;
      
      StringSet::iterator it = strings_.find(str);

      if(it == strings_.end())
      {        
        unsigned long* buff =
          new unsigned long[len / sizeof(unsigned long) +
                            (len % sizeof(unsigned long) ? 2 : 1)];

        *buff = 0;
        char* str_buff = (char*)(buff + 1);
          
        strcpy(str_buff, str);
        it = strings_.insert(str_buff).first;
      }

      ++(*StringSet::buffer(it->c_str()));
      return it->c_str();
    }
    
    inline
    const char*
    SharedStringManager::add_ref(const char* str) throw(El::Exception)
    {
      if(str == 0 || str == EMPTY)
      {
        return str;
      }

      register unsigned long& ref_count = *StringSet::buffer(str);
      
      WriteGuard guard(lock_);

      ++ref_count;
      ++references_;      
      
      return str;
    }
    
    inline
    void
    SharedStringManager::remove(const char* str) throw(El::Exception)
    {
      if(str == 0 || str == EMPTY)
      {
        return;
      }

      register unsigned long& ref_count = *StringSet::buffer(str);

      WriteGuard guard(lock_);

      if(--ref_count == 0)
      {
        strings_.erase(str);
        delete [] StringSet::buffer(str);
      }
      
      --references_;      
    }
    
    inline
    const char*
    SharedStringManager::read_string(El::BinaryInStream& istr)
      throw(El::Exception)
    {
      uint64_t len = 0;

      if(istr.short_types())
      {
        uint32_t len32 = 0;
        istr >> len32;

        if(len32 == UINT32_MAX)
        {
          return 0;
        }

        len = len32;
      }
      else
      {
        istr >> len;

        if(len == UINT64_MAX)
        {
          return 0;
        }
      }

      if(len == 0)
      {
        return EMPTY;
      }

      WriteGuard guard(lock_);

      if(buff_len_ < len + 1)
      {
        size_t new_len = (size_t)2 * len + 1;
        buff_.reset(new char[new_len]);
        buff_len_ = new_len;
      }
      
      istr.read_raw_bytes((unsigned char*)buff_.get(), len);
      buff_[len] = '\0';
      
      return add(buff_.get(), len);
    }

    //
    // SharedStringManager::StringSet class
    //
    inline
    SharedStringManager::StringSet::~StringSet() throw()
    {
      for(StringSet::iterator it = begin(); it != end(); it++)
      {
        delete [] buffer(it->c_str());
      }
    }    

    inline
    unsigned long*
    SharedStringManager::StringSet::buffer(const char* str) throw()
    {
      return const_cast<unsigned long*>(((const unsigned long*)str) - 1);
    }
    
    //
    // SharedStringConstPtr class
    //

    template<typename T>
    El::String::SharedStringManager SharedStringConstPtr<T>::string_manager;
    
    template<typename T>
    const SharedStringConstPtr<T> SharedStringConstPtr<T>::null;
    
    template<typename T>
    SharedStringConstPtr<T>::SharedStringConstPtr(const char* str)
      throw(El::Exception)
        : str_(string_manager.add(str))
    {
    }
    
    template<typename T>
    SharedStringConstPtr<T>::SharedStringConstPtr(const std::string& str)
      throw(El::Exception)
        : str_(string_manager.add(str.c_str()))
    {
    }
    
    template<typename T>
    SharedStringConstPtr<T>::SharedStringConstPtr(
      const SharedStringConstPtr& str)
      throw(El::Exception)
        : str_(string_manager.add_ref(str.c_str()))
    {
    }

    template<typename T>
    SharedStringConstPtr<T>::~SharedStringConstPtr() throw()
    {
      try
      {
        string_manager.remove(str_);
      }
      catch(const El::Exception& e)
      {
        std::cerr << "SharedStringConstPtr<T>::~SharedStringConstPtr: "
                  << e << std::endl;
      }
    }
    
    template<typename T>
    SharedStringConstPtr<T>&
    SharedStringConstPtr<T>::operator=(const char* str) throw(El::Exception)
    {
      const char* tmp = string_manager.add(str);
      string_manager.remove(str_);
      str_ = tmp;
      return *this;
    }
    
    template<typename T>
    SharedStringConstPtr<T>&
    SharedStringConstPtr<T>::operator=(const std::string& str)
      throw(El::Exception)
    {
      const char* tmp = string_manager.add(str.c_str());
      string_manager.remove(str_);
      str_ = tmp;
      return *this;
    }
    
    template<typename T>
    SharedStringConstPtr<T>&
    SharedStringConstPtr<T>::operator=(const SharedStringConstPtr& str)
      throw(El::Exception)
    {
      const char* tmp = string_manager.add_ref(str.c_str());
      string_manager.remove(str_);
      str_ = tmp;
      return *this;
    }
    
    template<typename T>
    bool
    SharedStringConstPtr<T>::empty() const throw()
    {
      return str_ == 0 || *str_ == '\0';
    }
    
    template<typename T>
    const char*
    SharedStringConstPtr<T>::c_str() const throw(El::Exception)
    {
      return str_;
    }

    template<typename T>
    void
    SharedStringConstPtr<T>::swap(SharedStringConstPtr& str) throw()
    {
      const char* tmp = str_;
      str_ = str.str_;
      str.str_ = tmp; 
    }
    
    template<typename T>
    unsigned long
    SharedStringConstPtr<T>::hash() const throw()
    {
      return string_manager.hash(str_);      
    }
    
    template<typename T>
    void
    SharedStringConstPtr<T>::reset(const char* str) throw(El::Exception)
    {
      const char* tmp = string_manager.add(str);
      string_manager.remove(str_);
      str_ = tmp;
    }
    
    template<typename T>
    void
    SharedStringConstPtr<T>::clear() throw(El::Exception)
    {
      reset();
    }
    
    template<typename T>
    size_t
    SharedStringConstPtr<T>::length() const throw()
    {
      return str_ ? strlen(str_) : 0;
    }

    template<typename T>
    bool
    SharedStringConstPtr<T>::operator==(const SharedStringConstPtr& str) const
      throw()
    {
      return string_manager.equal(str_, str.str_);
    }

    template<typename T>
    bool
    SharedStringConstPtr<T>::operator!=(const SharedStringConstPtr& str) const
      throw()
    {
      return !string_manager.equal(str_, str.str_);
    }

    template<typename T>
    bool
    SharedStringConstPtr<T>::operator<(
      const SharedStringConstPtr<T>& str) const throw()
    {
      if((str_ == 0) != (str.str_ == 0))
      {
        return str_ == 0;
      }

      return str_ == 0 ? false : strcmp(str_, str.str_) < 0;
    }    

    template<typename T>
    bool
    SharedStringConstPtr<T>::operator>(
      const SharedStringConstPtr<T>& str) const throw()
    {
      if((str_ == 0) != (str.str_ == 0))
      {
        return str_ == 0;
      }

      return str_ == 0 ? false : strcmp(str_, str.str_) > 0;
    }    

    template<typename T>
    bool
    SharedStringConstPtr<T>::operator==(const char* str) const throw()
    {
      if((str_ == 0) != (str == 0))
      {
        return false;
      }

      return str_ == 0 ? true : (strcmp(str_, str) == 0);
    }

    template<typename T>
    bool
    SharedStringConstPtr<T>::operator!=(const char* str) const throw()
    {
      if((str_ == 0) != (str == 0))
      {
        return true;
      }

      return str_ == 0 ? false : (strcmp(str_, str) != 0);
    }

    template<typename T>
    bool
    SharedStringConstPtr<T>::operator<(const char* str) const throw()
    {
      if((str_ == 0) != (str == 0))
      {
        return str_ == 0;
      }

      return str_ == 0 ? false : strcmp(str_, str) < 0;
    }

    template<typename T>
    bool
    SharedStringConstPtr<T>::operator>(const char* str) const throw()
    {
      if((str_ == 0) != (str == 0))
      {
        return str_ == 0;
      }

      return str_ == 0 ? false : strcmp(str_, str) > 0;
    }

    template<typename T>
    void
    SharedStringConstPtr<T>::write(El::BinaryOutStream& ostr) const
      throw(Exception, El::Exception)
    {
      string_manager.write_string(ostr, str_);
    }

    template<typename T>
    void
    SharedStringConstPtr<T>::read(El::BinaryInStream& istr)
      throw(Exception, El::Exception)
    {
      const char* tmp = string_manager.read_string(istr);
      string_manager.remove(str_);
      str_ = tmp;
    }
    
    template<typename T>
    const char*
    SharedStringConstPtr<T>::add_ref() const throw(El::Exception)
    {
      return string_manager.add_ref(str_);
    }
    
    template<typename T>
    void
    SharedStringConstPtr<T>::remove() const throw(El::Exception)
    {
      string_manager.remove(str_);
    }

    template<typename T>
    const char*
    SharedStringConstPtr<T>::release() throw(El::Exception)
    {
      const char* str = str_;
      str_ = 0;
      return str;
    }
  }
}

namespace El
{
  namespace Hash
  {
    template<typename T> 
    size_t
    SharedStringConstPtr<T>::operator()(
      const El::String::SharedStringConstPtr<T>& str) const
      throw(El::Exception)
    {
      return str.hash();
    }    
  }
}

#endif // _ELEMENTS_EL_STRING_SHAREDSTRING_HPP_
