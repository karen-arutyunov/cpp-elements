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
 * @file   Elements/test/Image/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <El/Exception.hpp>
#include <El/Image/ImageInfo.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
  "\nUsage:\nElTestImage ( help | info (<filename>)* ) \n";
}

int
main(int argc, char** argv)
{
  try
  {
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'ElTestImage help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestImage: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestImage: unknown exception caught\n";
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
  if(argc < 2)
  {
    throw InvalidArg("too few arguments");
  }
  
  std::string command;
  
  int i = 1;
  command = argv[i++];

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
  else if(command == "info")
  {
    test(arguments);
  }
  else
  {
    throw InvalidArg("unknown command");
  }
  
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
  for(ArgList::const_iterator it = arguments.begin();
      it != arguments.end(); it++)
  {
    image_info(it->name.c_str());
  }
  
  return 0;
}

void
Application::image_info(const char* filename) const
  throw(InvalidArg, Exception, El::Exception)
{
  std::fstream file(filename, std::ios::in);

  if(!file.is_open())
  {
    std::ostringstream ostr;
    ostr << "can't open file " << filename;
    throw InvalidArg(ostr.str());
  }

  El::Image::ImageInfo info;

  info.read(file);

  std::cout << filename << std::endl << "  " << info.type_name()
            << " width " << info.width << " height " << info.height
            << std::endl;

/*  
  unsigned char buff[256];

  file.read((char*)buff, 6);

  if(file.fail() || memcmp(buff, "GIF87a", 6) && memcmp(buff, "GIF89a", 6))
  {
    return false;
  }


  file.read((char*)buff, 2);
      
  if(file.fail())
  {
    return false;
  }

  width = (((unsigned long)buff[1]) << 8) | ((unsigned long)buff[0]);

  file.read((char*)buff, 2);
      
  if(file.fail())
  {
    return false;
  }

  height = (((unsigned long)buff[1]) << 8) | ((unsigned long)buff[0]);

  return true;
*/
  
/* PNG
  file.read((char*)buff, 8);

  if(file.fail() || memcmp(buff, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8))
  {
    return false;
  }

  while(true)
  {
    file.read((char*)buff, 4);
      
    if(file.fail())
    {
      return false;
    }

    unsigned long len =
      (((unsigned long)buff[0]) << 24) | (((unsigned long)buff[1]) << 16) |
      (((unsigned long)buff[2]) << 8) | ((unsigned long)buff[3]);

    file.read((char*)buff, 4);
      
    if(file.fail())
    {
      return false;
    }

    if(strncmp((char*)buff, "IHDR", 4) == 0)
    {
      file.read((char*)buff, 4);
      
      if(file.fail())
      {
        return false;
      }
      
      width =
        (((unsigned long)buff[0]) << 24) | (((unsigned long)buff[1]) << 16) |
        (((unsigned long)buff[2]) << 8) | ((unsigned long)buff[3]);
      
      file.read((char*)buff, 4);
      
      if(file.fail())
      {
        return false;
      }
      
      height =
        (((unsigned long)buff[0]) << 24) | (((unsigned long)buff[1]) << 16) |
        (((unsigned long)buff[2]) << 8) | ((unsigned long)buff[3]);

      return true;
    }
    else
    {
      while(len)
      {
        unsigned long read_bytes = std::min(len, (unsigned long)sizeof(buff));

        file.read((char*)buff, read_bytes);
      
        if(file.fail())
        {
          return false;
        }

        len -= read_bytes;
      }

      file.read((char*)buff, 2); // CRC
      
      if(file.fail())
      {
        return false;
      }
    }
  }
*/  
/*
  // JPEG
  
  file.read((char*)buff, 2);

  if(file.fail() || buff[0] != 0xFF || buff[1] != 0xD8)
  {
    return false;
  }

  width = 0;
  height = 0;
    
  while(true)
  {
    file.read((char*)buff, 2);
    
    if(file.fail())
    {
      return false;
    }

    unsigned char chr = 0;
    if(buff[0] == 0xFF && (chr = buff[1]) >= 0xC0 && chr <= 0xC3)
    {
      // Skip len and some byte
      file.read((char*)buff, 3);
      
      if(file.fail())
      {
        return false;
      }

      file.read((char*)buff, 2);
      
      if(file.fail())
      {
        return false;
      }

      height = (((unsigned long)buff[0]) << 8) | ((unsigned long)buff[1]);

      file.read((char*)buff, 2);
      
      if(file.fail())
      {
        return false;
      }

      width = (((unsigned long)buff[0]) << 8) | ((unsigned long)buff[1]);
      
      return true;
    }
    else
    {
      file.read((char*)buff, 2);
      
      if(file.fail())
      {
        return false;
      }

      unsigned long len  =
        (((unsigned long)buff[0]) << 8) | ((unsigned long)buff[1]);

      if(len < 2)
      {
        return false;
      }

      len -= 2;

      while(len)
      {
        unsigned long read_bytes = std::min(len, (unsigned long)sizeof(buff));

        file.read((char*)buff, read_bytes);
      
        if(file.fail())
        {
          return false;
        }

        len -= read_bytes;
      }
      
    }
*/
  
}
