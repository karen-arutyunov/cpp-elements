/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/test/Base64/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

/*
#include <string.h>
#include <zlib.h>


#include <ext/hash_fun.h>

#include <google/sparse_hash_map>

#include <El/CRC.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Stat.hpp>
*/

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/ArrayPtr.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nTestBase64 decode <file> | help\n";
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
              << "\nRun 'TestBase64 help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "TestBase64: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "TestBase64: unknown exception caught\n";
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
  
  int i = 1;  

  if(argc > 1)
  {
    command = argv[i++];
  }

  ArgList arguments;

  for(; i < argc; i++)
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
  else if(command == "decode")
  {
    return decode(arguments);
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
Application::decode(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  for(ArgList::const_iterator i(arguments.begin()), e(arguments.end());
      i != e; ++i)
  {
    std::string name = i->name;
    std::fstream f(name.c_str(), std::ios::in);

    if(!f.is_open())
    {
      std::ostringstream ostr;
      ostr << "Failed to open " << name;
      throw Exception(ostr.str());
    }

    std::string text;
    std::string line;
    while(std::getline(f, line))
    {
      text += line;
    }

    f.close();

    name += ".decoded";
    
    f.open(name.c_str(), std::ios::out);

    if(!f.is_open())
    {
      std::ostringstream ostr;
      ostr << "Failed to open " << name;
      throw Exception(ostr.str());
    }

    std::string output;
    El::String::Manip::base64_decode(text.c_str(), output);

    f << output;

    if(f.fail())
    {
      std::ostringstream ostr;
      ostr << "Failed to write " << name;
      throw Exception(ostr.str());
    }
  }

  return 0;
}

int
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
/*  
  const char* src = "U2VhcmNoIHJlc3VsdHMgLi4uLi4KCkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKQUFBQUFBQQpodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9tP21zZz1QTHMzV2p1UXdJNCUzRApBQUFBQUFBCmh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL20/bXNnPVBMczNXanVRd0k0JTNECkFBQUFBQUEKaHR0cDovL25nZGV2Lm9jc2xhYi5jb206NzE4MC9wL3MvbT9tc2c9UExzM1dqdVF3STQlM0QKCgpFZGl0OiBodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9lP3NtX289dCZzbV9pPTI4QjNCM0RDRTYzQjYzNEQ1REEzMUQ5NEVDRjI1OTExJnNtX2U9a2FyZW4lNDBvY3NsYWIuY29tCgpTdXNwZW5kOiBodHRwOi8vbmdkZXYub2NzbGFiLmNvbTo3MTgwL3Avcy9lP3NtX289ZCZzbV9pPTI4QjNCM0RDRTYzQjYzNEQ1REEzMUQ5NEVDRjI1OTExCgpSZXN1bWU6IGh0dHA6Ly9uZ2Rldi5vY3NsYWIuY29tOjcxODAvcC9zL2U/c21fbz1lJnNtX2k9MjhCM0IzRENFNjNCNjM0RDVEQTMxRDk0RUNGMjU5MTEK";
  std::string dest;
  El::String::Manip::base64_decode(src, dest);
  std::cerr << dest;
  return 0;
*/
  
  typedef El::ArrayPtr<unsigned char> UCharArrayPtr;
  
  for(unsigned long i = 0; i < 100000; i++)
  {
    unsigned long len = (unsigned long long)rand() * 1000 /
      ((unsigned long long)RAND_MAX + 1);
    
    UCharArrayPtr src(new unsigned char[len]);
    UCharArrayPtr dest(new unsigned char[len]);

    for(unsigned long i = 0; i < len; i++)
    {
      src[i] = (char)((unsigned long long)rand() * 256 /
                      ((unsigned long long)RAND_MAX + 1));

    }
    
    std::string encoded;
    El::String::Manip::base64_encode(src.get(), len, encoded);
    
    El::String::Manip::base64_decode(encoded.c_str(), dest.get(), len);
  
    if(memcmp(src.get(), dest.get(), len))
    {
      std::ostringstream ostr;
      ostr << "Application::test: data differs after decoding";
      
      throw Exception(ostr.str());
    }
  }

  return 0;
}

