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
 * @file Elements/El/Utility.cpp
 * @author Karen Arutyunov
 * $id:$
 */
#include <malloc.h>

#include <fstream>
#include <sstream>
#include <string>

#include <El/String/Manip.hpp>

#include "Utility.hpp"

namespace El
{
  namespace Utility
  {
    size_t
    mem_used(pid_t pid) throw(Exception, El::Exception)
    {
      if(pid == 0)
      {
        pid = getpid();
      }
      
      std::string filename;
      
      {
        std::ostringstream ostr;
        ostr << "/proc/" << pid << "/status";
        filename = ostr.str();
      }
      
      std::fstream file(filename.c_str(), std::ios::in);
      
      if(!file.is_open())
      {
        std::ostringstream ostr;
        ostr << "El::Utility::mem_used: failed to open file " << filename;
        throw Exception(ostr.str());
      }
      
      std::string line;
      while(std::getline(file, line))
      {
        if(strncasecmp(line.c_str(), "VmSize:", 7) == 0)
        {
          std::string part;
          El::String::Manip::trim(line.c_str() + 7, part);
          const char* val = part.c_str();
          const char* end = strchr(val, ' ');
          
          std::string value(val, end - val);
          
          unsigned long vmsize = 0;
          if(!El::String::Manip::numeric(value.c_str(), vmsize))
          {
            std::ostringstream ostr;
            ostr << "El::Utility::mem_used: invalid VmSize record in file "
                 << filename;
            
            throw Exception(ostr.str());
          }
          
          return vmsize;
        }
      }
      
      std::ostringstream ostr;
      ostr << "El::Utility::mem_used: failed to find VmSize record in file "
           << filename;
      
      throw Exception(ostr.str());
    }

    void
    dump_mallinfo(std::ostream& ostr) throw(El::Exception)
    {
      struct mallinfo mi = mallinfo();

      ostr << "\nEl::Utility::dump_mallinfo:"
              "\n  Non-mmapped space allocated from system (arena):"
           << mi.arena
           << "\n  Number of free chunks (ordblks): " << mi.ordblks
           << "\n  Number of fastbin blocks (smblks): " << mi.smblks
           << "\n  Number of mmapped regions (hblks): " << mi.hblks
           << "\n  Space in mmapped regions (hblkhd): " << mi.hblkhd
           << "\n  Maximum total allocated space (usmblks): " << mi.usmblks
           << "\n  Space available in freed fastbin blocks (fsmblks): "
           << mi.fsmblks
           << "\n  Total allocated space (uordblks): " << mi.uordblks
           << "\n  Total free space (fordblks): " << mi.fordblks
           << "\n  Top-most, releasable (via malloc_trim) space (keepcost): "
           << mi.keepcost;
    }
  }
}
