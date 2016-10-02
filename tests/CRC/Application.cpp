/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/test/CRC/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

#include <string>
#include <iostream>
#include <sstream>

#include <tr1/functional>

#include <google/sparse_hash_map>

#include <El/CRC.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Stat.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nElTestCRC [help]\n";
}

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
              << "\nRun 'ElTestCRC help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestCRC: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestCRC: unknown exception caught\n";
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
  test_performance(arguments);
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
  {
    const unsigned char data[] = "oi92382nifdsbs1\t1=qwjdqowcnkd\n/\r)`A";

    unsigned long crc1 = 0;
    El::CRC(crc1, data, sizeof(data));

    unsigned long crc2 = 0;
    unsigned long p = sizeof(data) / 3;
    
    El::CRC(crc2, data, p);
    El::CRC(crc2, data + p, sizeof(data) - p);

    if(crc1 != crc2)
    {
      std::ostringstream ostr;
      ostr << "Application::test: crc1 != crc2 (" << crc1 << " != "
           << crc2 << ") for unsigned long";
      
      throw Exception(ostr.str());
    }
  }

  {
    const unsigned char data[] = "pqwfd9/\t32nb2\\kwdlodsanias";

    unsigned short crc1 = 0;
    El::CRC(crc1, data, sizeof(data));

    unsigned short crc2 = 0;
    unsigned long p = sizeof(data) * 3 / 5;
    
    El::CRC(crc2, data, p);
    El::CRC(crc2, data + p, sizeof(data) - p);

    if(crc1 != crc2)
    {
      std::ostringstream ostr;
      ostr << "Application::test: crc1 != crc2 (" << crc1 << " != "
           << crc2 << ") for unsigned short";
      
      throw Exception(ostr.str());
    }
  }

  {
    const unsigned char data[] = "qw09234jrednkjalkwaoiuore'lakJAAJhja92jpoaf";

    unsigned char crc1 = 0;
    El::CRC(crc1, data, sizeof(data));

    unsigned char crc2 = 0;
    unsigned long p = sizeof(data) * 3 / 5;
    
    El::CRC(crc2, data, p);
    El::CRC(crc2, data + p, sizeof(data) - p);

    if(crc1 != crc2)
    {
      std::ostringstream ostr;
      ostr << "Application::test: crc1 != crc2 (" << crc1 << " != "
           << crc2 << ") for unsigned char";
      
      throw Exception(ostr.str());
    }
  }

  {
    const unsigned char data[] =
      "0wekeddoi92\n_kdfdfn3idskJDewr[w}abc082\\dfa0230kjsaIhdakbuqwoqw|sa-23";

    unsigned long long crc1 = 0;
    El::CRC(crc1, data, sizeof(data));

    unsigned long long crc2 = 0;
    unsigned long p = sizeof(data) / 4;
    
    El::CRC(crc2, data, p);
    El::CRC(crc2, data + p, sizeof(data) - p);

    if(crc1 != crc2)
    {
      std::ostringstream ostr;
      ostr << "Application::test: crc1 != crc2 (" << crc1 << " != "
           << crc2 << ") for unsigned long long";
      
      throw Exception(ostr.str());
    }
  }

  return 0;
}

int
Application::test_performance(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  typedef google::sparse_hash_map<unsigned long long,
                                  unsigned long,
                                  El::Hash::Numeric<unsigned long> >
  CRC_Distribution;

  CRC_Distribution crc_distr;
  CRC_Distribution crc_ull_distr;
  CRC_Distribution hash_string_distr;
  CRC_Distribution crc32_distr;
  CRC_Distribution adler32_distr;
  CRC_Distribution fnv_hash_distr;

  El::Stat::TimeMeter crc_meter("El::CRC");
  El::Stat::TimeMeter crc_ull_meter("El::CRC<unsigned long long>");
  El::Stat::TimeMeter hash_meter("hash_string");
  El::Stat::TimeMeter crc32_meter("crc32");
  El::Stat::TimeMeter adler32_meter("adler32");
  El::Stat::TimeMeter fnv_hash_meter("fnv_hash<4>");
    
  for(unsigned long i = 0; i < 1000000; i++)
  {
    char rand_str[16];
    unsigned long j = 0;
    
    for(; j < sizeof(rand_str) - 1; j++)
    {
      rand_str[j] = (char)((unsigned long long)rand() * 255 /
                           ((unsigned long long)RAND_MAX + 1) + 1);
    }

    rand_str[j] = '\0';

    //
    // El::CRC
    //
    
    unsigned long crc = 0;
    
    {
      El::Stat::TimeMeasurement measurement(crc_meter);
      El::CRC(crc, (unsigned char*)rand_str, sizeof(rand_str) - 1);
    }

//    crc |= ~0x7FFFFFFF;

    CRC_Distribution::iterator it = crc_distr.find(crc);

    if(it == crc_distr.end())
    {
      crc_distr[crc] = 1;
    }
    else
    {
      it->second++;
    }

    //
    // El::CRC<unsigned long long>
    //
    
    unsigned long long crc_ull = 0;
    
    {
      El::Stat::TimeMeasurement measurement(crc_ull_meter);
      El::CRC(crc_ull, (unsigned char*)rand_str, sizeof(rand_str) - 1);
    }

    it = crc_ull_distr.find(crc_ull);

    if(it == crc_ull_distr.end())
    {
      crc_ull_distr[crc_ull] = 1;
    }
    else
    {
      it->second++;
    }

    //
    // GNU FNV hash
    //
    
    {
      El::Stat::TimeMeasurement measurement(fnv_hash_meter);
//      crc = std::tr1::Fnv_hash<4>::hash(rand_str, sizeof(rand_str) - 1);
      crc = El::Hash::Fnv_hash<4>::hash(rand_str, sizeof(rand_str) - 1);      
    }
    
    it = fnv_hash_distr.find(crc);

    if(it == fnv_hash_distr.end())
    {
      fnv_hash_distr[crc] = 1;
    }
    else
    {
      it->second++;
    }

    //
    // GNU hash string
    //
    
    {
      El::Stat::TimeMeasurement measurement(hash_meter);
      crc = __gnu_cxx::__stl_hash_string(rand_str);
    }
    
    it = hash_string_distr.find(crc);

    if(it == hash_string_distr.end())
    {
      hash_string_distr[crc] = 1;
    }
    else
    {
      it->second++;
    }

    //
    // crc32
    //
    uLong adler;

    {
      El::Stat::TimeMeasurement measurement(crc32_meter);
      adler = crc32(0L, Z_NULL, 0);    
      adler = crc32(adler, (const Bytef*)rand_str, sizeof(rand_str) - 1);
    }

    it = crc32_distr.find(adler);

    if(it == crc32_distr.end())
    {
      crc32_distr[adler] = 1;
    }
    else
    {
      it->second++;
    }
    
    //
    // adler32
    //
    
    {
      El::Stat::TimeMeasurement measurement(adler32_meter);
      adler = adler32(0L, Z_NULL, 0);
      adler = adler32(adler, (const Bytef*)rand_str, sizeof(rand_str) - 1);
    }

    it = adler32_distr.find(adler);

    if(it == adler32_distr.end())
    {
      adler32_distr[adler] = 1;
    }
    else
    {
      it->second++;
    }
    
  }

  //
  // El::CRC
  //
  crc_meter.dump(std::cerr);

  unsigned long failures = 0;
  for(CRC_Distribution::iterator it = crc_distr.begin(); it != crc_distr.end();
      it++)
  {
    if(it->second > 1)
    {
      failures += it->second;  
    }
  }

  std::cerr << "  Failures: " << failures << std::endl;

  //
  // El::CRC<unsigned long long>
  //
  crc_ull_meter.dump(std::cerr);

  failures = 0;
  for(CRC_Distribution::iterator it = crc_ull_distr.begin();
      it != crc_ull_distr.end(); it++)
  {
    if(it->second > 1)
    {
      failures += it->second;  
    }
  }

  std::cerr << "  Failures: " << failures << std::endl;

  //
  // GNU hash string
  //
  hash_meter.dump(std::cerr);
  
  failures = 0;
  for(CRC_Distribution::iterator it = hash_string_distr.begin();
      it != hash_string_distr.end(); it++)
  {
    if(it->second > 1)
    {
      failures += it->second;  
    }
  }

  std::cerr << "  Failures: " << failures << std::endl;

  //
  // GNU FNV hash
  //
  fnv_hash_meter.dump(std::cerr);
  
  failures = 0;
  for(CRC_Distribution::iterator it = fnv_hash_distr.begin();
      it != fnv_hash_distr.end(); it++)
  {
    if(it->second > 1)
    {
      failures += it->second;  
    }
  }

  std::cerr << "  Failures: " << failures << std::endl;

  //
  // crc32
  //
  crc32_meter.dump(std::cerr);
  
  failures = 0;
  for(CRC_Distribution::iterator it = crc32_distr.begin();
      it != crc32_distr.end(); it++)
  {
    if(it->second > 1)
    {
      failures += it->second;  
    }
  }

  std::cerr << "  Failures: " << failures << std::endl;

  //
  // adler32
  //
  adler32_meter.dump(std::cerr);
  
  failures = 0;
  for(CRC_Distribution::iterator it = adler32_distr.begin();
      it != adler32_distr.end(); it++)
  {
    if(it->second > 1)
    {
      failures += it->second;  
    }
  }

  std::cerr << "  Failures: " << failures << std::endl;

  return 0;
}
