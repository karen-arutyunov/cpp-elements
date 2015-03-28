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
 * @file   Elements/test/CRC/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <stdlib.h>

#include <string>
#include <iostream>
#include <sstream>

#include <El/String/Manip.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nElTestStringManip [help]\n";
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
              << "\nRun 'ElTestStringManip help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestStringManip: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestStringManip: unknown exception caught\n";
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

struct TrimSample
{
  const char* text;
  const char* trimmed;
  unsigned long len;
};

struct PunycodeSample
{
  const char* src;
  const char* dest;
  El::String::Manip::CaseFlag case_flag;
};

static TrimSample TRIM_SAMPLES[] =
{
  { " \tABC  ", "ABC", 0 },
  { " \nABC  = XYZ  ", "ABC", 7 }
};

static PunycodeSample PUNYCODE_SAMPLES[] =
{
  {
    "\xD0\xA0\xD1\x84",
    "s0a6e",
    El::String::Manip::CF_NOCHANGE
  },
  {
    "\xD1\x80\xD1\x84",
    "p1ai",
    El::String::Manip::CF_NOCHANGE
  },
  {
    "\xD0\x9F\xD1\x80\xD0\xB5\xD0\xB7\xD0\xB8\xD0\xB4\xD0\xB5\xD0\xBD\xD1\x82.\xD0\xA0\xD1\x84",
    ".-8rbe2febmi1cya2a0a",
    El::String::Manip::CF_NOCHANGE
  },    
  { "\xD0\xBF\xD1\x80\xD0\xB5\xD0\xB7\xD0\xB8\xD0\xB4\xD0\xB5\xD0\xBD\xD1\x82.\xD1\x80\xD1\x84",
    ".-gtbcbig1bkjhu0a",
    El::String::Manip::CF_NOCHANGE
  },
  { "\xD1\x80\xD1\x84", "p1ai", El::String::Manip::CF_NOCHANGE },
  { "Abc", "Abc-", El::String::Manip::CF_NOCHANGE },
  { "Abc", "ABC-", El::String::Manip::CF_UPPERCASE },
  { "Abc", "abc-", El::String::Manip::CF_LOWERCASE },
};

int
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  for(unsigned long i = 0; i < sizeof(TRIM_SAMPLES) / sizeof(TRIM_SAMPLES[0]);
      i++)
  {
    TrimSample& ts = TRIM_SAMPLES[i];
    
    std::string res;
    El::String::Manip::trim(ts.text, res, ts.len);

    if(res != ts.trimmed)
    {
      std::ostringstream ostr;
      ostr << "Application::test: when trimmed '" << ts.text
           << "' have got unexpected result '" << res << "' while expected '"
           << ts.trimmed;
      
      throw Exception(ostr.str());
    }
  }
  
  for(unsigned long i = 0;
      i < sizeof(PUNYCODE_SAMPLES) / sizeof(PUNYCODE_SAMPLES[0]); i++)
  {
    PunycodeSample& ps = PUNYCODE_SAMPLES[i];
    
    std::string res;
    El::String::Manip::punycode_encode(ps.src, res, ps.case_flag);

    if(res != ps.dest)
    {
      std::ostringstream ostr;
      ostr << "Application::test: when punycode encoded '" << ps.src
           << "' have got unexpected result '" << res << "' while expected '"
           << ps.dest << "'";
      
      throw Exception(ostr.str());
    }

    if(ps.case_flag == El::String::Manip::CF_NOCHANGE)
    {
      std::string decoded;
      El::String::Manip::punycode_decode(res.c_str(), decoded);

      if(decoded != ps.src)
      {
        std::ostringstream ostr;
        ostr << "Application::test: when punycode decoded '" << res
             << "' have got unexpected result '" << decoded
             << "' while expected '" << ps.src << "'";
      
        throw Exception(ostr.str());
      }
    }
  }
  
  return 0;
}
