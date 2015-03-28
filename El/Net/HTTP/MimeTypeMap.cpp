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

