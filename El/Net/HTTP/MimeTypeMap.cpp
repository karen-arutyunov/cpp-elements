/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/MimeTypeMap.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string>
#include <sstream>
#include <fstream>

#include <El/String/ListParser.hpp>

#include "MimeTypeMap.hpp"

namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      MimeTypeMap::MimeTypeMap(const char* file_path)
        throw(Exception, El::Exception)
      {
        std::fstream file(file_path, std::ios::in);

        if(!file.is_open())
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::MimeTypeMap::MimeTypeMap: failed to open '"
               << file_path << "' for read access";

          throw Exception(ostr.str());
        }

        std::string line;
        while(std::getline(file, line))
        {
          El::String::ListParser parser(line.c_str());

          const char* type = parser.next_item();
          
          if(type == 0 || type[0] == '#')
          {
            continue;
          }

          std::string stype = type;
          const char* ext = 0;
          
          while((ext = parser.next_item()) != 0)
          {
            insert(std::make_pair(ext, stype));
          }
        }

        if(!file.eof())
        {
          std::ostringstream ostr;
          ostr << "El::Net::HTTP::MimeTypeMap::MimeTypeMap: failed to read '"
               << file_path << "'";

          throw Exception(ostr.str());
        }
      }
    }
  }
}

