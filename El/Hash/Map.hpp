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
 * @file Elements/El/Hash/Map.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_HASH_MAP_HPP_
#define _ELEMENTS_EL_HASH_MAP_HPP_

#include <utility>

#include <El/Exception.hpp>
#include <El/LightArray.hpp>
#include <El/BinaryStream.hpp>

namespace El
{
  namespace Hash
  {
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY = KEY,
             typename INSERTED_VALUE = VALUE>
    class Map
    {
    public:
      EL_EXCEPTION(Exception, El::ExceptionBase);

      typedef std::pair<KEY, VALUE> KeyValue;
      typedef LightArray<KeyValue, SIZE> KeyValueArray;
      typedef SIZE Size;
      
    public:
      Map(const KEY& empty_key, SIZE elements_count = 0)
        throw(El::Exception);

      Map(const Map& src) throw(El::Exception);

      void resize(SIZE elements_count) throw(El::Exception);
      
      void clear() throw();
      SIZE size() const throw();

      KeyValue& insert(const INSERTED_KEY& key, const INSERTED_VALUE& value)
        throw(Exception, El::Exception);

      KeyValue& insert(SIZE index,
                       const INSERTED_KEY& key,
                       const INSERTED_VALUE& value)
        throw(Exception, El::Exception);

      KeyValue& force_in(const INSERTED_KEY& key,
                         const INSERTED_VALUE& value)
        throw(Exception, El::Exception);

      void force_in(const Map& src) throw(Exception, El::Exception);

      void swap(Map& value) throw(Exception, El::Exception);

      KeyValue& operator[](SIZE index) throw();
      const KeyValue& operator[](SIZE index) const throw();

      Map& operator=(const Map& src) throw(El::Exception);

      KeyValue* find(const KEY& key) throw(El::Exception);
      KeyValue* find(const KEY& key, size_t hash) throw(El::Exception);
      
      const KeyValue* find(const KEY& key) const throw(El::Exception);

      const KeyValue* find(const KEY& key, size_t hash) const
        throw(El::Exception);

      const KEY& empty_key() const throw() { return empty_key_; }

      void write(El::BinaryOutStream& bstr) const throw(El::Exception);
      void read(El::BinaryInStream& bstr) throw(El::Exception);

    protected:
      KeyValueArray key_values_;
      KEY empty_key_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Hash
  {
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::Map(
      const KEY& empty_key, SIZE elements_count) throw(El::Exception)
        : key_values_(elements_count),
          empty_key_(empty_key)
    {
      for(SIZE i = 0; i < elements_count; i++)
      {
        key_values_[i].first = empty_key_;
      }      
    }

    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::Map(
      const Map& src) throw(El::Exception)
        : key_values_(src.key_values_),
          empty_key_(src.empty_key_)
    {
    }

    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    void
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    write(El::BinaryOutStream& bstr) const throw(El::Exception)
    {
      bstr << size();

      for(SIZE i = 0; i < size(); i++)
      {
        const KeyValue& kw = key_values_[i];
        bstr << kw.first << kw.second;
      }
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    void
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    read(El::BinaryInStream& bstr) throw(El::Exception)
    {
      SIZE size = 0;
      bstr >> size;
      resize(size);

      for(SIZE i = 0; i < size; i++)
      {
        KeyValue& kw = key_values_[i];
        bstr >> kw.first >> kw.second;
      }
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>&
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::operator=(
      const Map& src) throw(El::Exception)
    {
      key_values_ = src.key_values_;
      empty_key_ = src.empty_key_;
      return *this;
    }
  
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    void
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    resize(SIZE elements_count) throw(El::Exception)
    {
      key_values_.resize(elements_count);

      for(SIZE i = 0; i < elements_count; i++)
      {        
        key_values_[i].first = empty_key_;
      }
    }

    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    void
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    swap(Map& value) throw(Exception, El::Exception)
    {
      key_values_.swap(value.key_values_);

      KEY empty_key = empty_key_;
      empty_key_ = value.empty_key_;
      value.empty_key_ = empty_key;
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    void
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    force_in(const Map& src) throw(Exception, El::Exception)
    {
      SIZE size_increment = 0;
      
      for(SIZE i = 0; i < src.size(); i++)
      {
        if(find(src[i].first) == 0)
        {
          size_increment++;
        }
      }

      KeyValueArray current_kv;
      key_values_.release(current_kv);

      resize(current_kv.size() + size_increment);

      for(SIZE i = 0; i < src.size(); i++)
      {
        insert(src[i].first, src[i].second);
      }

      for(SIZE i = 0; i < current_kv.size(); i++)
      {
        const KeyValue& kv = current_kv[i];
        
        if(find(kv.first) == 0)
        {
          insert(kv.first, kv.second);
        }
      }
      
    }
      
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    typename Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    KeyValue&
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    force_in(const INSERTED_KEY& key,
             const INSERTED_VALUE& value)
      throw(Exception, El::Exception)
    {
      KeyValue* kv = find(key);
      
      if(kv != 0)
      {
        kv->second = value;
        return *kv;
      }

      KeyValueArray current_kv;
      key_values_.release(current_kv);

      resize(current_kv.size() + 1);

      for(SIZE i = 0; i < current_kv.size(); i++)
      {        
        insert(current_kv[i].first, current_kv[i].second);
      }

      return insert(key, value);
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    typename Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    KeyValue& 
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    insert(const INSERTED_KEY& key, const INSERTED_VALUE& value)
      throw(Exception, El::Exception)
    {
      if(!key_values_.size())
      {
        throw Exception("El::Hash::Map::insert: no elements reserved");
      }
      
      HASH_FUNC hash_func;
      SIZE index = hash_func(key) % key_values_.size();
      
      KeyValue& element = key_values_[index];

      if(element.first == empty_key_)
      {
        element.first = key;
        element.second = value;
        return element;
      }
      
      for(SIZE i = index + 1; i < key_values_.size(); i++)
      {
        KeyValue& element = key_values_[i];
          
        if(element.first == empty_key_)
        {
          element.first = key;
          element.second = value;
          return element;
        }
      }

      for(SIZE i = 0; i < index; i++)
      {
        KeyValue& element = key_values_[i];
          
        if(element.first == empty_key_)
        {
          element.first = key;
          element.second = value;
          return element;
        }
      }

      throw Exception("El::Hash::Map::insert: no more place");
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    typename Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    KeyValue& 
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    insert(SIZE index, const INSERTED_KEY& key, const INSERTED_VALUE& value)
      throw(Exception, El::Exception)
    {
      if(index >= size())
      {
        throw Exception("El::Hash::Map::insert: index out of range");
      }

      KeyValue& element = key_values_[index];
      
      if(element.first == empty_key_)
      {      
        element.first = key;
        element.second = value;
        return element;
      }
      
      throw Exception("El::Hash::Map::insert: position is already taken");
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    SIZE 
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::size()
      const throw()
    {
      return key_values_.size();
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    typename Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    KeyValue&
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    operator[](SIZE index) throw()
    {
      return key_values_[index];
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    const typename
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::KeyValue&
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    operator[](SIZE index) const throw()
    {
      return key_values_[index];
    }

    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    typename Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    KeyValue*
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::find(
      const KEY& key) throw(El::Exception)
    {
      HASH_FUNC hash_func;
      return find(key, hash_func(key));
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    typename Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::
    KeyValue*
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::find(
      const KEY& key, size_t hash) throw(El::Exception)
    {
      if(key_values_.size() == 0)
      {
        return 0;
      }
      
      SIZE index = hash % key_values_.size();
      
      KeyValue& element = key_values_[index];

      if(element.first == key)
      {
        return &element;
      }
      
      for(SIZE i = index + 1; i < key_values_.size(); i++)
      {
        KeyValue& element = key_values_[i];
          
        if(element.first == key)
        {
          return &element;
        }
      }

      for(SIZE i = 0; i < index; i++)
      {
        KeyValue& element = key_values_[i];
          
        if(element.first == key)
        {
          return &element;
        }
      }

      return 0;
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    const typename
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::KeyValue*
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::find(
      const KEY& key) const throw(El::Exception)
    {
      HASH_FUNC hash_func;
      return find(key, hash_func(key));
    }
    
    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    const typename
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::KeyValue*
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::find(
      const KEY& key, size_t hash) const throw(El::Exception)
    {
      if(key_values_.size() == 0)
      {
        return 0;
      }
      
      SIZE index = hash % key_values_.size();
      
      const KeyValue& element = key_values_[index];

      if(element.first == key)
      {
        return &element;
      }
      
      for(SIZE i = index + 1; i < key_values_.size(); i++)
      {
        const KeyValue& element = key_values_[i];
          
        if(element.first == key)
        {
          return &element;
        }
      }

      for(SIZE i = 0; i < index; i++)
      {
        const KeyValue& element = key_values_[i];
          
        if(element.first == key)
        {
          return &element;
        }
      }

      return 0;
    }

    template<typename KEY,
             typename VALUE,
             typename HASH_FUNC,
             typename SIZE,
             typename INSERTED_KEY,
             typename INSERTED_VALUE>
    void
    Map<KEY, VALUE, HASH_FUNC, SIZE, INSERTED_KEY, INSERTED_VALUE>::clear()
      throw()
    {
      key_values_.clear();
    }
  }
}

#endif // _ELEMENTS_EL_HASH_MAP_HPP_
