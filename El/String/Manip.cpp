/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <punycode.h>

#include <string>
#include <sstream>
#include <iomanip>

#include <ace/OS.h>

#include <El/ArrayPtr.hpp>

#include "Manip.hpp"
#include "Unicode.hpp"

const iconv_t El::String::Manip::Transcoder::NULL_HANDLE_ =
  reinterpret_cast<const iconv_t>(-1);

const size_t El::String::Manip::Transcoder::ERROR_ = static_cast<size_t>(-1);
  
struct EntityRef
{
  const char* name;
  wchar_t value;
};
  
namespace
{
  EntityRef ENTITY_REFS[] =
  {
    { "quot", 0x22 },
    { "quote", 0x22 },
    { "amp", 0x26 },
    { "lt", 0x3C },
    { "gt", 0x3E },
    { "nbsp", 0xA0 },
    { "iexcl", 0xA1 },
    { "cent", 0xA2 },
    { "pound", 0xA3 },
    { "curren", 0xA4 },
    { "yen", 0xA5 },
    { "brvbar", 0xA6 },
    { "sect", 0xA7 },
    { "uml", 0xA8 },
    { "copy", 0xA9 },
    { "ordf", 0xAA },
    { "laquo", 0xAB },
    { "not", 0xAC },
    { "shy", 0xAD },
    { "reg", 0xAE },
    { "macr", 0xAF },
    { "deg", 0xB0 },
    { "plusmn", 0xB1 },
    { "sup2", 0xB2 },
    { "sup3", 0xB3 },
    { "acute", 0xB4 },
    { "micro", 0xB5 },
    { "para", 0xB6 },
    { "middot", 0xB7 },
    { "cedil", 0xB8 },
    { "sup1", 0xB9 },
    { "ordm", 0xBA },
    { "raquo", 0xBB },
    { "frac14", 0xBC },
    { "frac12", 0xBD },
    { "frac34", 0xBE },
    { "iquest", 0xBF },
    { "Agrave", 0xC0 },
    { "Aacute", 0xC1 },
    { "Acirc", 0xC2 },
    { "Atilde", 0xC3 },
    { "Auml", 0xC4 },
    { "Aring", 0xC5 },
    { "AElig", 0xC6 },
    { "Ccedil", 0xC7 },
    { "Egrave", 0xC8 },
    { "Eacute", 0xC9 },
    { "Ecirc", 0xCA },
    { "Euml", 0xCB },
    { "Igrave", 0xCC },
    { "Iacute", 0xCD },
    { "Icirc", 0xCE },
    { "Iuml", 0xCF },
    { "ETH", 0xD0 },
    { "Ntilde", 0xD1 },
    { "Ograve", 0xD2 },
    { "Oacute", 0xD3 },
    { "Ocirc", 0xD4 },
    { "Otilde", 0xD5 },
    { "Ouml", 0xD6 },
    { "times", 0xD7 },
    { "Oslash", 0xD8 },
    { "Ugrave", 0xD9 },
    { "Uacute", 0xDA },
    { "Ucirc", 0xDB },
    { "Uuml", 0xDC },
    { "Yacute", 0xDD },
    { "THORN", 0xDE },
    { "szlig", 0xDF },
    { "agrave", 0xE0 },
    { "aacute", 0xE1 },
    { "acirc", 0xE2 },
    { "atilde", 0xE3 },
    { "auml", 0xE4 },
    { "aring", 0xE5 },
    { "aelig", 0xE6 },
    { "ccedil", 0xE7 },
    { "egrave", 0xE8 },
    { "eacute", 0xE9 },
    { "ecirc", 0xEA },
    { "euml", 0xEB },
    { "igrave", 0xEC },
    { "iacute", 0xED },
    { "icirc", 0xEE },
    { "iuml", 0xEF },
    { "eth", 0xF0 },
    { "ntilde", 0xF1 },
    { "ograve", 0xF2 },
    { "oacute", 0xF3 },
    { "ocirc", 0xF4 },
    { "otilde", 0xF5 },
    { "ouml", 0xF6 },
    { "divide", 0xF7 },
    { "oslash", 0xF8 },
    { "ugrave", 0xF9 },
    { "uacute", 0xFA },
    { "ucirc", 0xFB },
    { "uuml", 0xFC },
    { "yacute", 0xFD },
    { "thorn", 0xFE },
    { "yuml", 0xFF },
    { "OElig", 0x152 },
    { "oelig", 0x153 },
    { "Scaron", 0x160 },
    { "scaron", 0x161 },
    { "Yuml", 0x178 },
    { "circ", 0x2C6 },
    { "tilde", 0x2DC },
    { "ensp", 0x2002 },
    { "emsp", 0x2003 },
    { "thinsp", 0x2009 },
    { "zwnj", 0x200C },
    { "zwj", 0x200D },
    { "lrm", 0x200E },
    { "rlm", 0x200F },
    { "ndash", 0x2013 },
    { "mdash", 0x2014 },
    { "lsquo", 0x2018 },
    { "rsquo", 0x2019 },
    { "sbquo", 0x201A },
    { "ldquo", 0x201C },
    { "rdquo", 0x201D },
    { "bdquo", 0x201E },
    { "dagger", 0x2020 },
    { "Dagger", 0x2021 },
    { "permil", 0x2030 },
    { "lsaquo", 0x2039 },
    { "rsaquo", 0x203A },
    { "euro", 0x20AC },
    { "fnof", 0x192 },
    { "Alpha", 0x391 },
    { "Beta", 0x392 },
    { "Gamma", 0x393 },
    { "Delta", 0x394 },
    { "Epsilon", 0x395 },
    { "Zeta", 0x396 },
    { "Eta", 0x397 },
    { "Theta", 0x398 },
    { "Iota", 0x399 },
    { "Kappa", 0x39A },
    { "Lambda", 0x39B },
    { "Mu", 0x39C },
    { "Nu", 0x39D },
    { "Xi", 0x39E },
    { "Omicron", 0x39F },
    { "Pi", 0x3A0 },
    { "Rho", 0x3A1 },
    { "Sigma", 0x3A3 },
    { "Tau", 0x3A4 },
    { "Upsilon", 0x3A5 },
    { "Phi", 0x3A6 },
    { "Chi", 0x3A7 },
    { "Psi", 0x3A8 },
    { "Omega", 0x3A9 },
    { "alpha", 0x3B1 },
    { "beta", 0x3B2 },
    { "gamma", 0x3B3 },
    { "delta", 0x3B4 },
    { "epsilon", 0x3B5 },
    { "zeta", 0x3B6 },
    { "eta", 0x3B7 },
    { "theta", 0x3B8 },
    { "iota", 0x3B9 },
    { "kappa", 0x3BA },
    { "lambda", 0x3BB },
    { "mu", 0x3BC },
    { "nu", 0x3BD },
    { "xi", 0x3BE },
    { "omicron", 0x3BF },
    { "pi", 0x3C0 },
    { "rho", 0x3C1 },
    { "sigmaf", 0x3C2 },
    { "sigma", 0x3C3 },
    { "tau", 0x3C4 },
    { "upsilon", 0x3C5 },
    { "phi", 0x3C6 },
    { "chi", 0x3C7 },
    { "psi", 0x3C8 },
    { "omega", 0x3C9 },
    { "thetasym", 0x3D1 },
    { "upsih", 0x3D2 },
    { "piv", 0x3D6 },
    { "bull", 0x2022 },
    { "hellip", 0x2026 },
    { "prime", 0x2032 },
    { "Prime", 0x2033 },
    { "oline", 0x203E },
    { "frasl", 0x2044 },
    { "weierp", 0x2118 },
    { "image", 0x2111 },
    { "real", 0x211C },
    { "trade", 0x2122 },
    { "alefsym", 0x2135 },
    { "larr", 0x2190 },
    { "uarr", 0x2191 },
    { "rarr", 0x2192 },
    { "darr", 0x2193 },
    { "harr", 0x2194 },
    { "crarr", 0x21B5 },
    { "lArr", 0x21D0 },
    { "uArr", 0x21D1 },
    { "rArr", 0x21D2 },
    { "dArr", 0x21D3 },
    { "hArr", 0x21D4 },
    { "forall", 0x2200 },
    { "part", 0x2202 },
    { "exist", 0x2203 },
    { "empty", 0x2205 },
    { "nabla", 0x2207 },
    { "isin", 0x2208 },
    { "notin", 0x2209 },
    { "ni", 0x220B },
    { "prod", 0x220F },
    { "sum", 0x2211 },
    { "minus", 0x2212 },
    { "lowast", 0x2217 },
    { "radic", 0x221A },
    { "prop", 0x221D },
    { "infin", 0x221E },
    { "ang", 0x2220 },
    { "and", 0x2227 },
    { "or", 0x2228 },
    { "cap", 0x2229 },
    { "cup", 0x222A },
    { "int", 0x222B },
    { "there4", 0x2234 },
    { "sim", 0x223C },
    { "cong", 0x2245 },
    { "asymp", 0x2248 },
    { "ne", 0x2260 },
    { "equiv", 0x2261 },
    { "le", 0x2264 },
    { "ge", 0x2265 },
    { "sub", 0x2282 },
    { "sup", 0x2283 },
    { "nsub", 0x2284 },
    { "sube", 0x2286 },
    { "supe", 0x2287 },
    { "oplus", 0x2295 },
    { "otimes", 0x2297 },
    { "perp", 0x22A5 },
    { "sdot", 0x22C5 },
    { "lceil", 0x2308 },
    { "rceil", 0x2309 },
    { "lfloor", 0x230A },
    { "rfloor", 0x230B },
    { "lang", 0x2329 },
    { "rang", 0x232A },
    { "loz", 0x25CA },
    { "spades", 0x2660 },
    { "clubs", 0x2663 },
    { "hearts", 0x2665 },
    { "diams", 0x2666 }
  };

  char BASE64_CHARS[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  wchar_t WIN1251[] =
  { 0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 0x20AC, 
    0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F, 0x0452, 0x2018, 
    0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, '_',    0x2122, 0x0459,
    0x203A, 0x045A, 0x045C, 0x045B, 0x045F, 0x00A0, 0x040E, 0x045E, 0x0408,
    0x00A4, 0x0490, 0x00A6, 0x00A7, 0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC,
    0x00AD, 0x00AE, 0x0407, 0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5,
    0x00B6, 0x00B7, 0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455,
    0x0457, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F, 0x0420,
    0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429,
    0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F, 0x0430, 0x0431, 0x0432,
    0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B,
    0x043C, 0x043D, 0x043E, 0x043F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444,
    0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D,
    0x044E, 0x044F
  };

}

namespace El
{
  namespace String
  {
    namespace Manip
    {
      void 
      mime_url_encode(const char* from, std::string& to) throw (El::Exception)
      {
        std::ostringstream ostr;
        mime_url_encode(from, ostr);      
        to = ostr.str();
      }

      void
      mime_url_encode(const char* from, std::ostream& to)
        throw(El::Exception)
      {
        if(from == 0)
        {
          return;
        }

        char ch; 
        for (const char* ptr = from; (ch = *ptr) != '\0'; ptr++)
        {      
          if (ch == ' ')
          {
            to << '+';
            continue;
          }
          else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                   (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' ||
                   ch == '*' || ch == '.' || ch == ',')
          {
            to << ch;
            continue;
          }
                           
          to << "%";
          to << std::setw(2) << std::setfill('0') << std::uppercase
             << std::hex << std::right << (unsigned short) (ch & 0xFF)
             << std::dec;
        }
      }
      
      void 
      mime_url_decode(const char* from, std::string& to)
        throw(InvalidArg, El::Exception)
      {
        std::ostringstream ostr;
        mime_url_decode(from, ostr);      
        to = ostr.str();
      }

      void
      mime_url_decode(const char* from, std::ostream& to)
        throw(InvalidArg, El::Exception)
      {
        if(from == 0)
        {
          return;
        }

        const char* src = from;

        char chr = 0;
        while((chr = *src++) != '\0')
        {
          switch(chr)
          {
          case '+':
            {
              to << " ";
              break;
            }
          case '%':
            {
              if(*src == '\0' || src[1] == '\0')
              {
                std::ostringstream ostr;
                ostr << "El::String::Manip::mime_url_decode: invalid encoding "
                  "at position " << (src - from) << " in line '" << from
                     << "'";
                  
                throw InvalidArg(ostr.str());
              }

              char buff[3];
              strncpy(buff, src, 2);
              buff[2] = '\0';

              char* endptr = 0;
              size_t val = strtoul(buff, &endptr, 16);

              if(endptr != 0 && *endptr != '\0')
              {
                std::ostringstream ostr;
                ostr << "El::String::Manip::mime_url_decode: invalid encoding "
                  "at position " << (src - from) << " in line '" << from
                     << "'";
                  
                throw InvalidArg(ostr.str());
              }
              
              to << (char)val;
              src += 2;
              break;
            }
          default:
            {
              to << chr;
              break;
            }
          }
        }
        
      }
      
      void
      wchar_to_utf8(const wchar_t* src, std::string& dest)
        throw(El::Exception)
      {
        if(src == 0 || *src == L'\0')
        {
          dest.erase();
          return;
        }

        std::ostringstream ostr;
        for(const wchar_t* ptr = src; *ptr != L'\0'; ptr++)
        {
          wchar_to_utf8(*ptr, ostr);
        }

        dest = ostr.str();
      }

      bool
      utf8_valid(const char* src, unsigned long compliance)
        throw(El::Exception)
      {
        if(src == 0 || *src == '\0')
        {
          return true;
        }

        for(const char* ptr = src; *ptr != '\0'; )
        {
          unsigned long chr = *ptr;
          size_t len = 0;
          unsigned long wchr = 0;

          if((chr & 0x80) == 0)
          {
            len = 0;
            wchr = chr & 0x7F;
          }
          else if((chr & 0xE0) == 0xC0)
          {
            len = 1;
            wchr = chr & 0x1F;
          }
          else if((chr & 0xF0) == 0xE0)
          {
            len = 2;
            wchr = chr & 0xF;
          }
          else if((chr & 0xF8) == 0xF0)
          {
            len = 3;
            wchr = chr & 0x7;
          }
          else if((chr & 0xFC) == 0xF8)
          {
            len = 4;
            wchr = chr & 0x3;
          }
          else if((chr & 0xFE) == 0xFC)
          {
            len = 5;
            wchr = chr & 0x1;
          }
          else
          {
            return false;
          }
          
          for(ptr++; len && *ptr != '\0'; len--)
          {
            chr = *ptr++;
            
            if ((chr & 0xC0) != 0x80)
            {
              return false;
            }

            wchr = (wchr << 6) | (chr & 0x3F);
          }

          if(len)
          {
            return false;
          }
          
          if(compliance & UAC_XML_1_0)
          {            
            if(!uac_xml_1_0_compliant(wchr))
            {
              return false;
            }
          }
        }

        return true;
      }
      
      bool
      utf8_to_wchar(const char* src,
                    std::wostream& ostr,
                    bool lax,
                    unsigned long compliance)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0 || *src == '\0')
        {
          return true;
        }

        bool res = true;

        for(const unsigned char* ptr = (const unsigned char*)src;
            *ptr != '\0'; )
        {
          unsigned long chr = *ptr;
          size_t len = 0;
          unsigned long wchr = 0;

          if((chr & 0x80) == 0)
          {
            len = 0;
            wchr = chr & 0x7F;
          }
          else if((chr & 0xE0) == 0xC0)
          {
            len = 1;
            wchr = chr & 0x1F;
          }
          else if((chr & 0xF0) == 0xE0)
          {
            len = 2;
            wchr = chr & 0xF;
          }
          else if((chr & 0xF8) == 0xF0)
          {
            len = 3;
            wchr = chr & 0x7;
          }
          else if((chr & 0xFC) == 0xF8)
          {
            len = 4;
            wchr = chr & 0x3;
          }
          else if((chr & 0xFE) == 0xFC)
          {
            len = 5;
            wchr = chr & 0x1;
          }
          else if(lax)
          {
            res = false;
            wchr = (wchar_t)chr;

            if(compliant(wchr, compliance))
            {
              ostr << wchr;
            }          
            
            ptr++;
            continue;
          }
          else
          {
            std::ostringstream ostr;
            ostr << "El::String::Manip::utf8_to_wchar: bad string (1) '"
                 << src << "'";
            
            throw InvalidArg(ostr.str());
          }
          
          for(ptr++; len && *ptr != '\0'; len--)
          {
            chr = *ptr++;
            
            if ((chr & 0xC0) != 0x80)
            {
              if(lax)
              {
                res = false;

                if(compliant(wchr, compliance))
                {
                  ostr << (wchar_t)wchr;
                }

                wchr = (wchar_t)chr;
                
                if(compliant(wchr, compliance))
                {
                  ostr << (wchar_t)wchr;
                }
                
                break;
              }
              else
              {
                std::ostringstream ostr;
                ostr << "El::String::Manip::utf8_to_wchar: bad string (2) '"
                     << src << "'";
                
                throw InvalidArg(ostr.str());
              }
            }
        
            wchr = (wchr << 6) | (chr & 0x3F);
          }

          if(len)
          {
            if(lax)
            {
              res = false;
              continue;
            }
            else
            {
              std::ostringstream ostr;
              ostr << "El::String::Manip::utf8_to_wchar: bad string (3) '"
                   << src << "'";
            
              throw InvalidArg(ostr.str());
            }
          }
      
          ostr << (wchar_t)wchr;
        }

        return res;
      }

      size_t
      utf8_to_wchar(std::istream& src,
                    wchar_t& dest,
                    bool lax)
        throw(InvalidArg, El::Exception)
      {
        dest = 0;
        char c = 0;
        src.get(c);

        if(src.fail())
        {
          return 0;
        }

        size_t read_bytes = 1;        
        unsigned long chr = c;
        size_t len = 0;
        unsigned long wchr = 0;

        if((chr & 0x80) == 0)
        {
          len = 0;
          wchr = chr & 0x7F;
        }
        else if((chr & 0xE0) == 0xC0)
        {
          len = 1;
          wchr = chr & 0x1F;
        }
        else if((chr & 0xF0) == 0xE0)
        {
          len = 2;
          wchr = chr & 0xF;
        }
        else if((chr & 0xF8) == 0xF0)
        {
          len = 3;
          wchr = chr & 0x7;
        }
        else if((chr & 0xFC) == 0xF8)
        {
          len = 4;
          wchr = chr & 0x3;
        }
        else if((chr & 0xFE) == 0xFC)
        {
          len = 5;
          wchr = chr & 0x1;
        }
        else if(lax)
        {
          return 1;
        }
        else
        {
          std::ostringstream ostr;
          ostr << "El::String::Manip::utf8_to_wchar: bad char (1) '"
               << c << "'";
            
          throw InvalidArg(ostr.str());
        }

        for(; len; len--)
        {
          src.get(c);

          if(src.fail())
          {
            return read_bytes;
          }

          chr = c;
            
          if((chr & 0xC0) != 0x80)
          {
            if(lax)
            {
              src.putback(c);
              return read_bytes;
            }
            else
            {
              std::ostringstream ostr;
              ostr << "El::String::Manip::utf8_to_wchar: bad char (2) '"
                   << c << "'";
                
              throw InvalidArg(ostr.str());
            }
          }
        
          wchr = (wchr << 6) | (chr & 0x3F);
          ++read_bytes;
        }

        dest = (wchar_t)wchr;
        return read_bytes;
      }

      size_t
      utf8_to_wchar(const char* src, wchar_t& dest, bool lax)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0)
        {
          return 0;
        }
        
        dest = 0;
        char c = *src++;

        if(c == '\0')
        {
          return 0;
        }

        size_t read_bytes = 1;        
        unsigned long chr = c;
        size_t len = 0;
        unsigned long wchr = 0;

        if((chr & 0x80) == 0)
        {
          len = 0;
          wchr = chr & 0x7F;
        }
        else if((chr & 0xE0) == 0xC0)
        {
          len = 1;
          wchr = chr & 0x1F;
        }
        else if((chr & 0xF0) == 0xE0)
        {
          len = 2;
          wchr = chr & 0xF;
        }
        else if((chr & 0xF8) == 0xF0)
        {
          len = 3;
          wchr = chr & 0x7;
        }
        else if((chr & 0xFC) == 0xF8)
        {
          len = 4;
          wchr = chr & 0x3;
        }
        else if((chr & 0xFE) == 0xFC)
        {
          len = 5;
          wchr = chr & 0x1;
        }
        else if(lax)
        {
          return 1;
        }
        else
        {
          std::ostringstream ostr;
          ostr << "El::String::Manip::utf8_to_wchar2: bad char (1) '"
               << c << "'";
            
          throw InvalidArg(ostr.str());
        }

        for(; len; len--)
        {
          c = *src++;

          if(c == '\0')
          {
            return read_bytes;
          }

          chr = c;
            
          if((chr & 0xC0) != 0x80)
          {
            if(lax)
            {
              return read_bytes;
            }
            else
            {
              std::ostringstream ostr;
              ostr << "El::String::Manip::utf8_to_wchar2: bad char (2) '"
                   << c << "'";
                
              throw InvalidArg(ostr.str());
            }
          }
        
          wchr = (wchr << 6) | (chr & 0x3F);
          ++read_bytes;
        }

        dest = (wchar_t)wchr;
        return read_bytes;
      }

      void
      utf16_to_wchar(const uchar_t* src, std::wostream& ostr)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0 || *src == 0)
        {
          return;
        }

        for(const uchar_t* ptr = src; *ptr != 0; ptr++)
        {
          uchar_t chr = *ptr;

          if(chr < 0xD800 || chr > 0xDFFF)
          {
            ostr << (wchar_t)chr;
            continue;
          }

          if(chr >= 0xDC00)
          {
            std::ostringstream ostr;
            ostr << "El::StringManip::utf16_to_wchar: unexpected value of the "
              "first surrogate in a pair 0x" << std::hex << (unsigned long)chr;
          
            throw InvalidArg(ostr.str());
          }

          unsigned long s1 = (chr - 0xD800) << 10;            
          chr = *(++ptr);

          if(chr < 0xDC00)
          {
            std::ostringstream ostr;
            ostr << "El::StringManip::utf16_to_wchar: unexpected value "
              "of the second surrogate in a pair 0x" << std::hex
                 << (unsigned long)chr;
              
            throw InvalidArg(ostr.str());
          }

          ostr << (wchar_t)(s1 | (chr - 0xDC00));
        }
      }

      void
      wchar_to_utf16(const wchar_t* src, std::ustring& dest)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0 || *src == L'\0')
        {
          dest.erase();
          return;
        }

        std::uostringstream ostr;
        for(const wchar_t* ptr = src; *ptr != 0; ptr++)
        {
          wchar_to_utf16(*ptr, ostr);
        }

        dest = ostr.str();
      }
      
      void
      xml_encode(const wchar_t* src, std::ostream& dest, unsigned long flags)
        throw(El::Exception)
      {
        if(src == 0 || *src == '\0')
        {
          return;
        }

        wchar_t chr = L'\0';
        
        while((chr = *src++) != L'\0')
        {        
          if((chr & ~0xFF) == 0 && chr >= 0x20 && chr <= 0x7E)
          {
            if(flags & XE_FORCE_NUMERIC_ENCODING)
            {
              switch(chr)
              {
              case L'<':
              case L'>':
              case L'&':
                dest << "&#x" << std::hex << chr << ";" << std::dec;
                continue;
              }
              
              if(flags & XE_ATTRIBUTE_ENCODING)
              {
                switch (chr)
                {
                case L'\'':
                case L'"':
                  dest << "&#x" << std::hex << chr << ";" << std::dec;
                  continue;
                }
              }
            }
            else
            {
              switch(chr)
              {
              case L'<':
                dest << "&lt;";
                continue;
              case L'>':
                dest << "&gt;";
                continue;
              case L'&':
                dest << "&amp;";
                continue;
              }
          
              if(flags & XE_ATTRIBUTE_ENCODING)
              {
                switch (chr)
                {
                case L'\'':
                  dest << "&#x27;";
                  continue;
                case L'"':
                  dest << "&quot;";
                  continue;
                }
              }
            }
          
            dest << (char)(chr & 0xFF);
          }
          else
          {
            if((flags & XE_PRESERVE_UTF8) != 0 && chr >= 0x370)
            {
              wchar_to_utf8(chr, dest);
            }
            else
            {
              if(chr == 0x0A && (flags & XE_ATTRIBUTE_ENCODING) == 0)
              {
                dest << (char)chr;
              }
              else
              {
                dest << "&#x" << std::hex << chr << ";" << std::dec;
              }
              
            }
            
          }      
        }      
      }

      const wchar_t*
      xml_decode_entity(const wchar_t* src, wchar_t& character)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0)
        {
          throw InvalidArg("can't decode from null pointer");
        }
        
        std::string entity;
        const wchar_t* ptr = src;
                
        for(ptr++; *ptr != L'\0' && *ptr != L'\n' && *ptr != L'\r' &&
              *ptr != L';'; ptr++)
        {
          if(*ptr > 127)
          {
            std::ostringstream ostr;
            ostr << "unexpected entity reference starting with " << entity;
              
            throw InvalidArg(ostr.str());
          }
          
          entity.append(1, (char)*ptr);
        }
                
        if(*ptr != L';')
        {
          std::ostringstream ostr;
          ostr << "unexpected end of entity reference " << entity;
              
          throw InvalidArg(ostr.str());
        }

        bool bad_entity = entity.length() < 2;
                
        if(!bad_entity)
        {
          const char* ent_ref = entity.c_str();
                  
          if(*ent_ref == '#')
          {
            char* entptr = 0;
            
            character = ent_ref[1] == 'x' ?
              strtol(ent_ref + 2, &entptr, 16) :
              strtol(ent_ref + 1, &entptr, 10);

            bad_entity = entptr && *entptr != '\0';
          }
          else
          {
            size_t refs =
              sizeof(ENTITY_REFS) / sizeof(ENTITY_REFS[0]);

            size_t i = 0;
                    
            for(; i < refs && strcmp(ent_ref, ENTITY_REFS[i].name) != 0; i++);
                    
            if(i < refs)
            {
              character = ENTITY_REFS[i].value;
            }
            else
            {
              bad_entity = true;
            }
          } 
        }

        if(bad_entity)
        {
          std::ostringstream ostr;
          ostr << "unexpected entity reference " << entity;
              
          throw InvalidArg(ostr.str());
        }

        return ptr + 1;
      }

      bool
      win1251_to_wchar(const char* src, std::wostream& ostr, bool lax)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0)
        {
          return true;
        }

        bool res = true;
        
        unsigned char chr = 0;
        while((chr = (unsigned char)*src++) != '\0')
        {
          if(chr == 0x98)
          {
            if(lax)
            {
              res = false;
            }
            else
            {
              throw InvalidArg("El::String::Manip::win1251_to_wchar: "
                               "unexpected character 0x98");
            }
          }
          
          ostr << (chr < 128 ? (wchar_t)chr : WIN1251[chr - 128]);
        }
        
        return res;
      }
      
      void
      latin1_to_wchar(const char* src, std::wostream& ostr)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0)
        {
          return;
        }

        unsigned char chr;
        while((chr = (unsigned char)*src++) != '\0')
        {
          ostr << (wchar_t)chr;
        }
      }
      
      void
      xml_decode(const wchar_t* src, std::string& dest)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0)
        {
          dest.clear();
          return;
        }

        std::ostringstream ostr;
        wchar_t chr;
        
        while((chr = *src) != L'\0')
        {
          if(chr == L'&')
          {
            src = xml_decode_entity(src, chr);
            wchar_to_utf8(chr, ostr);            
          }
          else
          {
            wchar_to_utf8(chr, ostr);
            src++;
          }
        }

        dest = ostr.str();
      }

      void
      quoted_printable_encode(const unsigned char* src,
                              size_t src_len,
                              std::ostream& dest,
                              size_t char_per_line)
        throw(El::Exception)
      {
        size_t len = 0;
        
        for(const unsigned char *p(src), *e(src + src_len); p != e; ++p)
        {
          char ch = *p;
          
          bool as_is = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') || ch == '<' || ch == '>' || ch == '"' ||
            ch == '\'' || ch == '/';
          
          len += as_is ? 1 : 3;

          if(len > char_per_line)
          {
            dest << "=\r\n";
            len = as_is ? 1 : 3;
          }

          if(as_is)
          {
            dest << ch;
          }          
          else
          {
            dest << '=' << std::setw(2) << std::setfill('0') << std::uppercase
                 << std::hex << std::right << (unsigned short)*p;
          }
        }
      }

      void
      utf8_word_encode(const char* src, std::ostream& dest)
        throw(El::Exception)
      {
        bool first = true;
        
        while(true)
        {
          if(first)
          {
            first = false;
          }
          else
          {
            dest << "\r\n ";
          }

          wchar_t wc = 0;
          const char* ptr = src;
          const char* we_ptr = 0;

          while(true)
          {            
            size_t cl = utf8_to_wchar(ptr, wc);

            if(!cl || ptr - src + cl > 45)
            {
              break;
            }

            ptr += cl;

            if(El::String::Unicode::CharTable::is_space(wc))
            {
              we_ptr = ptr;
            }
          }

          size_t len = we_ptr ? (we_ptr - src) : (ptr - src);
          
          if(!len)
          {
            break;
          }
          
          dest << "=?utf-8?B?";
          base64_encode((unsigned char*)src, len, dest);
          dest << "?=";

          src += len;
        }
      }
      
      void
      base64_encode(const unsigned char* src,
                    size_t src_len,
                    std::ostream& dest,
                    size_t chunk_per_line)
        throw(El::Exception)
      {
        if(src == 0 || src_len == 0)
        {
          return;
        }
        
        size_t triple_count = src_len / 3;
        size_t chunks = 0;
        
        for(size_t i = 0; i < triple_count; ++i, ++chunks)
        {
          if(chunks == chunk_per_line)
          {
            dest << "\r\n";
            chunks = 0;
          }
          
          unsigned long i1 = ((*src) >> 2) & 0x3F;
          unsigned long i2 = (((*src) << 4) & 0x30) | ((src[1] >> 4) & 0xF);
          src++;
          
          unsigned long i3 = (((*src) << 2) & 0x3C) | ((src[1] >> 6) & 0x3);
          src++;
          
          unsigned long i4 = (*src++) & 0x3F;
          
          dest << BASE64_CHARS[i1] << BASE64_CHARS[i2] << BASE64_CHARS[i3]
               << BASE64_CHARS[i4];
        }
        
        switch(src_len % 3)
        {
        case 1:
          {
            if(chunks == chunk_per_line)
            {
              dest << "\r\n";
            }
            
            dest << BASE64_CHARS[((*src) >> 2) & 0x3F]
                 << BASE64_CHARS[(((*src) << 4) & 0x30)]
                 << "==";
            
            break;
          }
        case 2:
          {
            if(chunks == chunk_per_line)
            {
              dest << "\r\n";
            }

            unsigned long i1 = ((*src) >> 2) & 0x3F;
            unsigned long i2 = (((*src) << 4) & 0x30) | ((src[1] >> 4) & 0xF);
            src++;

            unsigned long i3 = (((*src++) << 2) & 0x3C);
              
            dest << BASE64_CHARS[i1] << BASE64_CHARS[i2] << BASE64_CHARS[i3]
                 << "=";
            
            break;
          }
        }
        
      }

      static
      unsigned char
      base64_index(char chr) throw()
      {
        switch(chr)
        {
        case '=': return 64;
        case '/': return 63;
        case '+': return 62;
        default:
          {
            if(chr >= 'A' && chr <= 'Z')
            {
              return chr - 'A';
            }
            
            if(chr >= 'a' && chr <= 'z')
            {
              return chr - 'a' + 26;
            }

            if(chr >= '0' && chr <= '9')
            {
              return chr - '0' + 52;
            }
          }
        }

        return 0xFF;
      }

      void
      base64_decode(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0)
        {
          throw InvalidArg("El::String::Manip::base64_decode: src is 0");
        }

        size_t len = strlen(src);
        
        if(len % 4)
        {
          std::ostringstream ostr;
          ostr << "El::String::Manip::base64_decode: unexpected length "
               << len << " of string '" << src << "'";
            
          throw InvalidArg(ostr.str());
        }

        size_t dest_len = len / 4 * 3;

        if(len)
        {
          if(src[len - 1] == '=')
          {
            dest_len--;
          }
          
          if(len > 1 && src[len - 2] == '=')
          {
            dest_len--;
          }
        }
        
        El::ArrayPtr<char> buff(new char[dest_len]);
        base64_decode(src, (unsigned char*)buff.get(), dest_len);
        dest.assign(buff.get(), dest_len);
      }

      void
      base64_decode(const char* src,
                    unsigned char* dest,
                    size_t dest_len)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0)
        {
          throw InvalidArg("El::String::Manip::base64_decode: src is 0");
        }

        size_t len = strlen(src);
        
        if(len % 4)
        {
          std::ostringstream ostr;
          ostr << "El::String::Manip::base64_decode: unexpected length "
               << len << " of string '" << src << "'";
            
          throw InvalidArg(ostr.str());
        }

        size_t real_dest_len = len / 4 * 3;

        if(len)
        {
          if(src[len - 1] == '=')
          {
            real_dest_len--;
          }
          
          if(len > 1 && src[len - 2] == '=')
          {
            real_dest_len--;
          }
        }

        if(real_dest_len != dest_len)
        {
          std::ostringstream ostr;
          ostr << "El::String::Manip::base64_decode: invalid dest buffer size "
               << dest_len << " instead of " << real_dest_len
               << " for string '" << src << "'";
            
          throw InvalidArg(ostr.str());          
        }

        const char* ptr = src;
        
        while(*ptr != '\0')
        {
          unsigned char i1, i2, i3, i4;
          
          if((i1 = base64_index(*ptr++)) == 0xFF ||
             (i2 = base64_index(*ptr++)) == 0xFF ||
             (i3 = base64_index(*ptr++)) == 0xFF ||
             (i4 = base64_index(*ptr++)) == 0xFF)
          {
            std::ostringstream ostr;
            ostr << "El::String::Manip::base64_decode: invalid character with "
              "code 0x" << std::hex << (unsigned long)(*(ptr - 1))
                 << std::dec << " at position " << ptr - src - 1
                 << " for string '" << src << "'";
            
            throw InvalidArg(ostr.str());          
          }

          if(i1 == 64 || i2 == 64 || (*ptr != '\0' && (i3 == 64 || i4 == 64)))
          {
            std::ostringstream ostr;
            ostr << "El::String::Manip::base64_decode: unexpected = character "
              " in the middle of string '" << src << "'";
            
            throw InvalidArg(ostr.str()); 
          }

          if(i3 == 64 && i4 != 64)
          {
            std::ostringstream ostr;
            ostr << "El::String::Manip::base64_decode: unexpected last "
              "character of string '" << src << "'";
            
            throw InvalidArg(ostr.str());            
          }

          *dest++ = ((i1 << 2) & 0xFC) | ((i2 >> 4) & 0x3);

          if(i3 < 64)
          {
            *dest++ = ((i2 << 4) & 0xF0) | ((i3 >> 2)& 0xF);
          }

          if(i4 < 64)
          {
            *dest++ = ((i3 << 6) & 0xC0) | (i4 & 0x3F);
          }
        }
      }
      
      void
      utf8_to_lower(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception)
      {        
        std::wstring wsrc;
        utf8_to_wchar(src, wsrc);

        std::wstring lower;
        to_lower(wsrc.c_str(), lower);

        El::String::Manip::wchar_to_utf8(lower.c_str(), dest);
      }
      
      void
      utf8_to_upper(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception)
      {
        std::wstring wsrc;
        utf8_to_wchar(src, wsrc);

        std::wstring upper;
        to_upper(wsrc.c_str(), upper);

        El::String::Manip::wchar_to_utf8(upper.c_str(), dest);
      }
      
      void
      utf8_to_title(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception)
      {
        std::wstring wsrc;
        utf8_to_wchar(src, wsrc);

        std::wstring upper;
        to_title(wsrc.c_str(), upper);

        El::String::Manip::wchar_to_utf8(upper.c_str(), dest);
      }
      
      void
      utf8_to_uniform(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception)
      {        
        std::wstring wsrc;
        utf8_to_wchar(src, wsrc);

        std::wstring uniform;
        to_uniform(wsrc.c_str(), uniform);

        El::String::Manip::wchar_to_utf8(uniform.c_str(), dest);
      }

      bool
      replace(const char* src,
              const char* pattern,
              const char* replacement,
              std::ostream& ostr)
        throw(El::Exception)
      {
        if(src == 0 || *src =='\0')
        {
          return false;
        }

        if(pattern == 0 || *pattern == '\0')
        {
          ostr << src;
          return false;
        }

        bool res = false;

        const char* s = src;
        size_t len = strlen(pattern);
        
        while(true)
        {
          const char* p = strstr(s, pattern);

          if(p)
          {
            ostr.write(s, p - s);
            ostr << replacement;
            s = p + len;
            res = true;
          }
          else
          {
            ostr << s;
            break;
          }
        }
        
        return res;
      }
      
      void
      replace(std::string& str,
              const char* src_chars,
              const char* replacement_chars)
        throw(InvalidArg, El::Exception)
      {
        if(strlen(src_chars) != strlen(replacement_chars))
        {
          throw InvalidArg("El::String::Manip::replace: length of src_chars "
                           "and replacement_chars differes");
        }
        
        for(std::string::iterator it = str.begin(); it != str.end(); it++)
        {
          char& chr = *it;
          const char* ptr = strchr(src_chars, chr);
          
          if(ptr)
          {
            chr = replacement_chars[ptr - src_chars];
          }
        }
      }
      
      void
      replace(std::wstring& str,
              const wchar_t* src_chars,
              const wchar_t* replacement_chars)
        throw(InvalidArg, El::Exception)
      {
        if(wcslen(src_chars) != wcslen(replacement_chars))
        {
          throw InvalidArg("El::String::Manip::replace: length of src_chars "
                           "and replacement_chars differes");
        }
        
        for(std::wstring::iterator it = str.begin(); it != str.end(); it++)
        {
          wchar_t& chr = *it;
          const wchar_t* ptr = wcschr(src_chars, chr);
          
          if(ptr)
          {
            chr = replacement_chars[ptr - src_chars];
          }
        }
      }
      
      void
      replace(wchar_t* str,
              size_t len,
              const wchar_t* src_chars,
              const wchar_t* replacement_chars)
        throw(InvalidArg, El::Exception)
      {
        if(wcslen(src_chars) != wcslen(replacement_chars))
        {
          throw InvalidArg("El::String::Manip::replace: length of src_chars "
                           "and replacement_chars differes");
        }

        if(len == 0)
        {
          len = wcslen(str);
        }
        
        for(size_t i = 0; i < len; i++)
        {
          wchar_t& chr = str[i];
          const wchar_t* ptr = wcschr(src_chars, chr);
          
          if(ptr)
          {
            chr = replacement_chars[ptr - src_chars];
          }
        }
      }
      
      void
      replace(std::string& str, const char* src_chars, char replacement_char)
        throw(InvalidArg, El::Exception)
      {
        for(std::string::iterator it = str.begin(); it != str.end(); it++)
        {
          char& chr = *it;

          if(strchr(src_chars, chr))
          {
            chr = replacement_char;
          }
        }    
      }

      bool
      replace(const char* src,
              const char* src_chars,
              const char** replacement_strs,
              std::ostream& ostr)
        throw(El::Exception)
      {
        if(src == 0)
        {
          return false;
        }

        bool res = false;
        char chr = 0;
        
        for(; (chr = *src) != '\0'; src++)
        {
          const char* ptr = strchr(src_chars, chr);
          
          if(ptr)
          {
            ostr << replacement_strs[ptr - src_chars];
            res = true;
          }
          else
          {
            ostr << chr;
          }
        }

        return res;
      }

      static const char js_escape_chars[] = "\b\f\n\r\t\'\"\\";
      
      static const char* js_escape_strs[] =
      {
        "\\b",
        "\\f",
        "\\n",
        "\\r",
        "\\t",
        "\\'",
        "\\\"",
        "\\\\"
      };
      
      void
      js_escape(const char* src, std::ostream& ostr) throw(El::Exception)
      {
        replace(src, js_escape_chars, js_escape_strs, ostr);
      }

      void
      xpath_escape(const char* src, char quote, std::string& dest)
        throw(El::Exception)
      {
        std::ostringstream ostr;
        xpath_escape(src, quote, ostr);
        dest = ostr.str();
      }

      void
      xpath_escape(const char* src, char quote, std::ostream& ostr)
        throw(El::Exception)
      {
        if(src == 0)
        {
          src = "";
        }
        
        const char* ptr = strchr(src, quote);

        if(ptr == 0)
        {
          ostr << quote << src << quote;
          return;
        }

        char quote2 = quote == '\'' ? '"' : '\'';
        ostr << "concat(";
        
        const char* begin = src;
        
        while(ptr)
        {
          if(begin != src)
          {
            ostr << ",";
          }

          if(ptr != begin)
          {
            ostr << quote;
            ostr.write(begin, ptr - begin);
            ostr << quote << ",";
          }

          ostr << quote2 << quote << quote2;

          begin = ptr + 1;
          ptr = strchr(begin, quote);
        }

        if(*begin != '\0')
        {
          ostr << "," << quote << begin << quote;
        }

        ostr << ")";
      }

      void
      compact(const wchar_t* src, std::wostream& ostr) throw(El::Exception)
      {
        if(src == 0 || *src == L'\0')
        {
          return;
        }

        const wchar_t* end = src + wcslen(src) - 1;
        while(end >= src && Unicode::CharTable::is_space(*end))
        {
          --end;
        }
        
        ++end;

        bool whitespace_last = true;
        for(; src != end; ++src)
        {
          wchar_t chr = *src;

          if(Unicode::CharTable::is_space(chr))
          {
            if(!whitespace_last)
            {
              ostr << " ";
              whitespace_last = true;
            }
          }
          else
          {
            whitespace_last = false;
            ostr << chr;
          }
        }
      }
      
      void
      suppress(const char* src,
               std::string& dest,
               const char* suppress_chars)
        throw(El::Exception)
      {
        size_t len = strlen(src);
        char* buff = new char[len];
        size_t i = 0;

        for(; *src != '\0'; ++src)
        {
          char chr = *src;
          if(strchr(suppress_chars, chr) == 0)
          {
            buff[i++] = chr;
          }
        }

        try
        {
          dest.assign(buff, i);
        }
        catch(...)
        {
          delete [] buff;
          throw;
        }
        
        delete [] buff;
      }
      
      void
      trim(const char* src, std::string& dest, size_t src_len)
        throw(El::Exception)
      {
        if(src == 0)
        {
          dest.clear();
          return;
        }

        if(src_len == 0)
        {
          src_len = strlen(src);
        }

        const char* ptr = src;
        
        size_t i = 0;
        for(; i < src_len && isspace(*ptr); i++, ptr++);
        
        if(i == src_len)
        {
          dest.clear();
          return;
        }

        i = src_len - 1 - (ptr - src);

        while(i > 0 && isspace(ptr[i]))
        {
          i--;
        }
        
        dest.assign(ptr, i + 1);
      }

      void
      trim(const wchar_t* src, std::wstring& dest) throw(El::Exception)
      {
        if(src == 0)
        {
          dest.clear();
          return;
        }

        while(*src != L'\0' && Unicode::CharTable::is_space(*src))
        {
          src++;
        }
        
        if(*src == L'\0')
        {
          dest.clear();
          return;
        }

        size_t i = wcslen(src) - 1;

        while(i > 0 && Unicode::CharTable::is_space(src[i]))
        {
          i--;
        }
        
        dest.assign(src, i + 1);
      }

      void
      truncate(const char* src,
               std::string& dest,
               size_t max_len,
               bool lax,
               unsigned long compliance,
               size_t max_utf8_len)
        throw(El::Exception)
      {
        std::wstring wsrc;
        utf8_to_wchar(src, wsrc, lax, compliance);

        std::wstring wdest;
        truncate(wsrc.c_str(), wdest, max_len, max_utf8_len);

        wchar_to_utf8(wdest.c_str(), dest);
      }

      void
      truncate(const wchar_t* src,
               std::wstring& dest,
               size_t max_len,
               size_t max_utf8_len)
        throw(El::Exception)
      {
        if(max_len > 4)
        {
          max_len -= 4;
        }

        if(max_utf8_len > 4)
        {
          max_utf8_len -= 4;
        }

        El::String::Manip::compact(src, dest);

        std::wstring::const_iterator i(dest.begin());
        std::wstring::const_iterator e(dest.end());

        size_t words_len = 0;
        size_t len = 0;
        size_t utf8_len = 0;
          
        for(; i != e; ++i, ++len)
        {
          wchar_t chr = *i;
            
          if(El::String::Unicode::CharTable::is_space(chr))
          {
            if(len > max_len || utf8_len > max_utf8_len)
            {
              break;
            }
            else
            {
              words_len = len;
            }
          }

          utf8_len += El::String::Manip::utf8_char_len(chr);          
        }

        if(i != e)
        {
          dest.resize(words_len);
          dest += L" ...";
        }        
      }

      void
      to_lower(const char* src, std::string& dest) throw(El::Exception)
      {
        if(src == 0 || *src == '\0')
        {
          dest.clear();
          return;
        }

        dest.resize(strlen(src));

        char chr;
        for(size_t i = 0; (chr = *src++) != '\0'; i++)
        {
          dest[i] = tolower(chr);
        }
      }

      void
      to_upper(const char* src, std::string& dest) throw(El::Exception)
      {
        if(src == 0 || *src == '\0')
        {
          dest.clear();
          return;
        }

        dest.resize(strlen(src));

        char chr;
        for(size_t i = 0; (chr = *src++) != '\0'; i++)
        {
          dest[i] = toupper(chr);
        }
      }

      void
      punycode_encode(const wchar_t* src,
                      std::string& dest,
                      CaseFlag case_flag)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0 || *src == L'\0')
        {
          dest.clear();
          return;
        }

        size_t input_length = wcslen(src);
        El::ArrayPtr<punycode_uint> input(new punycode_uint[input_length]);

        for(uint32_t *i(input.get()), *e(i + input_length); i != e; ++i)
        {
          *i = *src++;
        }

        El::ArrayPtr<unsigned char> case_flags;
        
        if(case_flag != CF_NOCHANGE)
        {
          case_flags.reset(new unsigned char[input_length]);
          
          memset(case_flags.get(),
                 case_flag == CF_LOWERCASE ? 0 : 1,
                 input_length);
        }

        size_t output_buff_size = input_length * 2;

        while(true)
        {
          El::ArrayPtr<char> output(new char[output_buff_size]);
          size_t output_length = output_buff_size;
        
          Punycode_status res =
            (Punycode_status)::punycode_encode(input_length,
                                               input.get(),
                                               case_flags.get(),
                                               &output_length,
                                               output.get());
          
          switch(res)
          {
          case punycode_success:
            {
              dest.assign(output.get(), output_length);
              return;
            }
          case punycode_big_output:
            {
              output_buff_size *= 2;
              continue;
            }
          default:
            {
              std::ostringstream ostr;
              ostr << "El::String::Manip::punycode_encode: "
                "::punycode_encode failed. Reason: "
                   << punycode_strerror(res);
              
              throw InvalidArg(ostr.str());              
            }
          }
        }
      }
      
      void
      punycode_decode(const char* src, std::wstring& dest)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0 || *src == '\0')
        {
          dest.clear();
          return;
        }

        size_t input_length = strlen(src);
        El::ArrayPtr<punycode_uint> output(new punycode_uint[input_length]);
        
        size_t output_length = input_length;
        
        Punycode_status res =
          (Punycode_status)::punycode_decode(input_length,
                                             src,
                                             &output_length,
                                             output.get(),
                                             0);

        if(res != PUNYCODE_SUCCESS)
        {
          std::ostringstream ostr;
          ostr << "El::String::Manip::punycode_decode: "
            "::punycode_encode failed. Reason: " << punycode_strerror(res);
              
          throw InvalidArg(ostr.str());              
        }

        dest.clear();
        dest.reserve(output_length);

        for(punycode_uint *p(output.get()), *e(p + output_length); p != e; ++p)
        {
          dest.push_back(*p);
        }
      }
      
      void
      Transcoder::encode(const char* src, std::ostream& dest, bool lax)
        throw(InvalidArg, El::Exception)
      {
        if(src == 0 || *src == '\0')
        {
          return;
        }

        char* inbuff = const_cast<char*>(src);
        size_t inleft = strlen(src);

        char buff[10];
        size_t buffsize = sizeof(buff);
        
        char* outbuff = buff;
        size_t outleft = buffsize;

        while(iconv(handle_, &inbuff, &inleft, &outbuff, &outleft) == ERROR_)
        {
          if(errno != E2BIG)
          {
            if(lax && (errno == EILSEQ || errno == EINVAL))
            {
              dest.write(buff, buffsize - outleft);
              outbuff = buff;
              outleft = buffsize;

              if(inleft)
              {
                dest.write(inbuff, 1);
                inbuff++;
                inleft--;
              }
              else
              {
                break;
              }
              
              continue;
            }
            
            std::ostringstream ostr;
            ostr << "El::String::Manip::Transcoder::encode: iconv failed. "
              "Reason: " << ACE_OS::strerror(errno);

            throw InvalidArg(ostr.str());
          }

          if(outleft == buffsize)
          {
            throw Exception("El::String::Manip::Transcoder::encode: "
                            "internal buffer too small");
          }

          dest.write(buff, buffsize - outleft);
          outbuff = buff;
          outleft = buffsize;
        }
      }
      
    }
  }
}
