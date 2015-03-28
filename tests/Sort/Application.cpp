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
 * @file   Elements/test/Sort/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include <El/Stat.hpp>
#include <El/ArrayPtr.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nElTestSort [help]\n";
  const unsigned long CHUNK_ELEMENTS = 10000;
  const unsigned long CHUNKS_COUNT = 100;

  const unsigned long TOTAL_ELEMENTS = 100000;
  const unsigned long TOP_COUNT = 10;
  const unsigned long RANGES_COUNT = 1000;
}

struct Item
{
  unsigned long weight;
  char data[12];

  bool operator<(const Item& item) const throw();
  bool operator==(const Item& item) const throw();
  static int compare(const void *, const void *);
};
  
inline
bool
Item::operator<(const Item& item) const throw()
{
  return weight > item.weight;
}

inline
bool
Item::operator==(const Item& item) const throw()
{
  return weight == item.weight;
}

inline
int
Item::compare(const void* i1, const void* i2)
{
  unsigned long w1 = ((Item*)i1)->weight;
  unsigned long w2 = ((Item*)i2)->weight;
  
  return w1 > w2 ? -1 : (w1 == w2 ? 0 : 1);
}
  
typedef El::ArrayPtr<Item> ItemLightArray;
typedef std::vector<Item> ItemArray;
typedef std::vector<ItemArray> ItemArrayArray;

struct ItemPtr
{
  ItemArray::iterator cur;
  ItemArray::iterator end;    
};
int
main(int argc, char** argv)
{
  srand(time(0));
  
  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'ElTestSort help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestSort: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestSort: unknown exception caught\n";
  }
  
  return -1;
}

Application::Application() throw(Application::Exception, El::Exception)
{
}

Application::~Application() throw()
{
}

int
Application::run(int& argc, char** argv)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string command;
  
  int i = 0;  

  if(argc > 1)
  {
    command = argv[i++];
  }

  ArgList arguments;

  for(i++; i < argc; i++)
  {
    char* argument = argv[i];
    
    Argument arg;
    const char* eq = strstr(argument, "=");

    if(eq == 0)
    {
      arg.name = argument;
    }
    else
    {
      arg.name.assign(argument, eq - argument);
      arg.value = eq + 1;
    }

    arguments.push_back(arg);
  }

  if(command == "help")
  {
    return help(arguments);
  }

  test(arguments);
  return 0;
}

int
Application::help(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << USAGE;
  return 0;
}

int
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  test_different_sorts(arguments);
  test_taking_top(arguments);
  
  return 0;
}

void
Application::test_different_sorts(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << "* Testing different sorting algorithms ...\n";
  
  ItemArrayArray example_chunks(CHUNKS_COUNT);
  
  for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
  {
    ItemArray& array = example_chunks[i];

    for(unsigned long j = 0; j < CHUNK_ELEMENTS; j++)
    {
      Item item;
      item.weight = rand();
      
      array.push_back(item);
    }
  }

  //
  // Sorting all chunks at once
  //
  El::Stat::TimeMeter sorting_all_at_once("Sorting all at once");

  ItemArray array;
  array.reserve(CHUNKS_COUNT * CHUNK_ELEMENTS);
  
  {
    El::Stat::TimeMeasurement measurement(sorting_all_at_once);
  
    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      ItemArray& ar = example_chunks[i];
      array.insert(array.end(), ar.begin(), ar.end());
    }
  
    {
      std::sort(array.begin(), array.end());
    }
  }
  
  sorting_all_at_once.dump(std::cerr);

  //
  // Sorting each chunk separatelly, then put them together and sort
  //
  array.clear();
  array.reserve(CHUNKS_COUNT * CHUNK_ELEMENTS);
  
  El::Stat::TimeMeter sorting_chunks("Sorting chunks");
  El::Stat::TimeMeter sorting_sorted_chunks("Sorting sorted chunks");
  
  ItemArrayArray source_chunks(example_chunks);
  
  {
    El::Stat::TimeMeasurement measurement(sorting_chunks);
  
    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      ItemArray& array = source_chunks[i];
      std::sort(array.begin(), array.end());
    }

    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      ItemArray& ar = source_chunks[i];
      array.insert(array.end(), ar.begin(), ar.end());
    }
    
    El::Stat::TimeMeasurement measurement2(sorting_sorted_chunks);
    std::sort(array.begin(), array.end());
  }
  
  sorting_chunks.dump(std::cerr);
  sorting_sorted_chunks.dump(std::cerr);
  
  //
  // QSorting each chunk separatelly, then put them together and qsort
  //
  array.clear();
  array.reserve(CHUNKS_COUNT * CHUNK_ELEMENTS);
  
  El::Stat::TimeMeter qsorting_chunks("Sorting chunks (array vs vector)");
  
  El::Stat::TimeMeter qsorting_sorted_chunks(
    "Sorting sorted chunks (array vs vector)");
  
  source_chunks = example_chunks;

  ItemLightArray* item_array_array = new ItemLightArray[CHUNKS_COUNT];
  ItemLightArray item_array(new Item[CHUNK_ELEMENTS * CHUNKS_COUNT]);

  for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
  {
    ItemArray& array = source_chunks[i];

    item_array_array[i].reset(new Item[CHUNK_ELEMENTS]);

    for(unsigned long j = 0; j < CHUNK_ELEMENTS; j++)
    {
      item_array_array[i][j] = array[j];
    }      
  }
  
  {
    El::Stat::TimeMeasurement measurement(qsorting_chunks);
  
    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      std::sort(item_array_array[i].get(), item_array_array[i].get() + CHUNK_ELEMENTS);
//      qsort(item_array_array[i].get(), CHUNK_ELEMENTS, sizeof(Item), Item::compare);
    }

    Item* parr = item_array.get();
    
    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      memcpy(parr, item_array_array[i].get(), CHUNK_ELEMENTS * sizeof(Item));
      parr += CHUNK_ELEMENTS;
    }
    
    El::Stat::TimeMeasurement measurement2(qsorting_sorted_chunks);
//    qsort(item_array.get(), CHUNK_ELEMENTS * CHUNKS_COUNT, sizeof(Item), Item::compare);
    std::sort(item_array.get(), item_array.get() + CHUNK_ELEMENTS * CHUNKS_COUNT);
    
  }
  
  qsorting_chunks.dump(std::cerr);
  qsorting_sorted_chunks.dump(std::cerr);
  
  //
  // Sorting each chunk separatelly, then put them orderly together
  //
  array.clear();
  array.reserve(CHUNKS_COUNT * CHUNK_ELEMENTS);
  
  El::Stat::TimeMeter ordering_chunks("Ordering chunks");
  El::Stat::TimeMeter ordering_sorted_chunks("Ordering sorted chunks");
  
  source_chunks = example_chunks;

  typedef std::list<ItemPtr> ItemPtrList;
  
  {
    El::Stat::TimeMeasurement measurement(ordering_chunks);
  
    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      ItemArray& array = source_chunks[i];
      std::sort(array.begin(), array.end());
    }

    ItemPtrList item_ptrs;
      
    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      ItemArray& ar = source_chunks[i];
      
      ItemPtr item_ptr;
      item_ptr.cur = ar.begin();
      item_ptr.end = ar.end();
      
      item_ptrs.push_back(item_ptr);
    }
    
    El::Stat::TimeMeasurement measurement2(ordering_sorted_chunks);

    ItemPtrList::iterator item_ptrs_begin;
    ItemPtrList::iterator item_ptrs_end;
    
    while((item_ptrs_begin = item_ptrs.begin()) !=
          (item_ptrs_end = item_ptrs.end()))
    {
      Item* min_val = &(*item_ptrs_begin->cur);
      ItemPtrList::iterator min_ptr = item_ptrs_begin;
      
      for(ItemPtrList::iterator it = ++item_ptrs_begin;
          it != item_ptrs_end; it++)
      {
        if(*it->cur < *min_val)
        {
          min_val = &(*it->cur);
          min_ptr = it;
        } 
      }

      array.push_back(*min_val);

      if(++min_ptr->cur == min_ptr->end)
      {
        item_ptrs.erase(min_ptr);
      }
    }
  }
  
  ordering_chunks.dump(std::cerr);
  ordering_sorted_chunks.dump(std::cerr);
  
  //
  // Sorting each chunk separatelly, then merging them together
  //
  array.clear();
  array.resize(CHUNKS_COUNT * CHUNK_ELEMENTS * 2);

  El::Stat::TimeMeter merging_chunks("Merging chunks");
  El::Stat::TimeMeter merging_sorted_chunks("Merging sorted chunks");
  
  source_chunks = example_chunks;
  
  {
    El::Stat::TimeMeasurement measurement(merging_chunks);
  
    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      ItemArray& array = source_chunks[i];
      std::sort(array.begin(), array.end());
    }

    El::Stat::TimeMeasurement measurement2(merging_sorted_chunks);

    ItemArray::iterator ptr = array.begin();
    
    for(unsigned long i = 0; i < CHUNKS_COUNT; i++)
    {
      ItemArray& ar = source_chunks[i];
      ItemArray::iterator dest = ptr + CHUNK_ELEMENTS;
      
      std::merge(array.begin(),
                 ptr,
                 ar.begin(),
                 ar.end(),
                 dest);

      ptr = dest;
    }
  }
  
  merging_chunks.dump(std::cerr);
  merging_sorted_chunks.dump(std::cerr);
}

void
Application::test_taking_top(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << "* Testing \"take top\" approaches ...\n";

  ItemArray example_elems(TOTAL_ELEMENTS);
  
  for(unsigned long i = 0; i < TOTAL_ELEMENTS; i++)
  {
    example_elems[i].weight = rand();
  }

  El::Stat::TimeMeter taking_top("Taking top simple");

  unsigned long result_items =
    TOP_COUNT < TOTAL_ELEMENTS ? TOP_COUNT : TOTAL_ELEMENTS;
  
  ItemLightArray item_array_simple(new Item[TOTAL_ELEMENTS]);
  
  {
    El::Stat::TimeMeasurement measurement(taking_top);

    item_array_simple.reset(new Item[TOTAL_ELEMENTS]);
    
    for(unsigned long i = 0; i < TOTAL_ELEMENTS; i++)
    {
      item_array_simple[i] = example_elems[i];
    }
    
    std::sort(item_array_simple.get(),
              item_array_simple.get() + TOTAL_ELEMENTS);
  }
  
  taking_top.dump(std::cerr);

  El::Stat::TimeMeter taking_top_smart("Taking top smart");
  
  ItemArray result_array;
    
  {
    El::Stat::TimeMeasurement measurement(taking_top_smart);

    unsigned long max_weight = 0;
    unsigned long min_weight = ULONG_MAX;

    for(unsigned long i = 0; i < TOTAL_ELEMENTS; i++)
    {
      unsigned long weight = example_elems[i].weight;

      if(weight > max_weight)
      {
        max_weight = weight;
      }
      
      if(weight < min_weight)
      {
        min_weight = weight;
      }
    }

    unsigned long diapason = max_weight - min_weight + 1;

    ItemArrayArray ranges(RANGES_COUNT);

    for(unsigned long i = 0; i < TOTAL_ELEMENTS; i++)
    {
      unsigned long index =
        (unsigned long long)(max_weight - example_elems[i].weight) *
        RANGES_COUNT / diapason;

      if(index >= ranges.size())
      {
        continue;
      }

      unsigned long item_count = 0;
      unsigned long j = 0;
      
      for(unsigned long j = 0; j <= index && item_count < TOP_COUNT; j++)
      {
        item_count += ranges[j].size();
      }

      if(j < index)
      {
        ranges.resize(index);
        continue;
      }

      ItemArray& array = ranges[index];

      if(array.size() == 0)
      {
        array.reserve(TOTAL_ELEMENTS / RANGES_COUNT + 1);
      }

      array.push_back(example_elems[i]);
    }

    unsigned long elem_to_sort = 0;
    unsigned long range_count = ranges.size();
    
    for(unsigned long i = 0; i < range_count && elem_to_sort < TOP_COUNT; i++)
    {
      const ItemArray& array = ranges[i];
      unsigned long size = array.size();

      if(size)
      {
        if(!result_array.size())
        {
          result_array.reserve(size > TOP_COUNT ? size : TOP_COUNT);
        }
      
        result_array.insert(result_array.end(), array.begin(), array.end());
        
        elem_to_sort += size;
      }
    }

    std::sort(result_array.begin(), result_array.end());

    result_array.resize(result_items);
  }

  bool success = true;
  for(unsigned long i = 0; i < result_items; i++)
  {
    success = result_array[i] == item_array_simple[i];

    if(!success)
    {
      std::cerr << "Broken sort: element " << i << " is unexpectedly "
                << result_array[i].weight << " instead of "
                << item_array_simple[i].weight
                << std::endl;

      break;
    }
  }
  
  taking_top_smart.dump(std::cerr);
}

