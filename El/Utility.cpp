/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
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
