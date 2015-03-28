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
 * @file   Elements/tests/HTMLLightParser/HTMLLightParserMain.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <sstream>
#include <iostream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>

#include <El/HTML/LightParser.hpp>

EL_EXCEPTION(Exception, El::ExceptionBase);

namespace
{
  const char USAGE[] = "Usage: ElTestHTMLLightParser";
}

struct Test
{
  const wchar_t* html;
  const char* text;
  unsigned long flags;
};

const Test positive_tests[] =
{
  { L"<code>Tom, I find your book very useful. I tried many of your examples given in your book. The examples are very much meaningful and educative. When I tried your example (page no. 398.399) on sql loader, I faced a problem. Table: SQL> desc ...",
    "Tom, I find your book very useful. I tried many of your examples given in your book. The examples are very much meaningful and educative. When I tried your example (page no. 398.399) on sql loader, I faced a problem. Table: SQL> desc ..."
  },
  { L" <p> <a href=\"http://us.rd.yahoo.com/dailynews/rss/world/*http://news.yahoo.com/s/ap/20051106/ap_on_re_eu/france_rioting\"><img src=\"http://us.news3.yimg.com/us.i2.yimg.com/p/ap/20051106/capt.par11311061916.france_rioting_par113.jpg?x=93&amp;y=130&amp;sig=0ZPxq9Wz.2jOl0FvU4TeBg--\" align=\"left\" height=\"130\" width=\"93\" alt=\"French President Jacques Chirac addresses the nation at the Elysee Palace in Paris Sunday, Nov. 6, 2005. Chirac said restoring security and public order was an 'absolute priority' after urban violence intensified and spread across France _ with arsonists striking from the Mediterranean to the German border and into central Paris for the first time. (AP Photo/Jacques Brinon)\" border=\"0\" /></a>AP - French President Jacques Chirac on Sunday promised arrests, trials and punishment for those sowing &quot;violence or fear&#x22;&#60; across France &amp;#151; as the urban unrest that has triggered attacks on vehicles, nursery schools and other targets hit central Paris for the first time.</p> <br clear=all> ",
    "AP - French President Jacques Chirac on Sunday promised arrests, trials and punishment for those sowing \"violence or fear\"< across France &#151; as the urban unrest that has triggered attacks on vehicles, nursery schools and other targets hit central Paris for the first time.",
    0
  },
  { L"> just a plain \r\n\"windows\"  text",
    "> just a plain\n\"windows\" text",
    0,
  },
  { L" > just a plain \n 'unix'\n \ntext",
    "> just a plain\n'unix'\ntext",
    0
  },
  { L"<p>simple <span style=\"color:red\"><br clear='all'>HTML</span> text",
    "simple\nHTML text"
  }  
};

const wchar_t* negative_tests[] =
{
  L"<p>broken \n<a",
  L"<p>broken \n<span style=\"color:red",
  L"<p>broken \n<span style=\"color:red>HTML</span> text",
  L"<p>broken \n<span <style=\"color:red\">HTML</span> text"
};

//mbstowcs

int
main(int argc, char** argv)
{
  try
  {
    for(unsigned long i = 0; i < sizeof(positive_tests) /
          sizeof(positive_tests[0]); i++)
    {
      El::HTML::LightParser parser;
      parser.parse(positive_tests[i].html, 0, positive_tests[i].flags);

      if(parser.text != positive_tests[i].text)
      {
        std::string source;
        El::String::Manip::wchar_to_utf8 (positive_tests[i].html, source);
        
        std::cerr << "When parsing:\n" << source
                  << "\nHave got:\n" << parser.text
                  << "\nWhile expected:\n" << positive_tests[i].text
                  << std::endl;
        
        throw Exception("Test failed.");
      }
    }
    
    for(unsigned long i = 0; i < sizeof(negative_tests) /
          sizeof(negative_tests[0]); i++)
    {
      El::HTML::LightParser parser;
      
      try
      {
        parser.parse(negative_tests[i], 0);
      }
      catch(const El::HTML::LightParser::Exception&)
      {
        continue;
      }
      
      std::wcerr << "When parsing:\n" << negative_tests[i]
                 << "\nDidn't got El::HTML::LightParser::Exception\n";
        
      throw Exception("Test failed.");
    }

    return 0;
  }
  catch (const El::Exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "unknown exception caught.\n";
  }
  
  return -1;
}
