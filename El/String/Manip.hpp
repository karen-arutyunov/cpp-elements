/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/String/Manip.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STRING_MANIP_HPP_
#define _ELEMENTS_EL_STRING_MANIP_HPP_

#include <iconv.h>
#include <stdint.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/String/Utf16Char.hpp>
#include <El/String/Unicode.hpp>
#include <El/CRC.hpp>

namespace El
{
  namespace String
  {
    typedef std::vector<std::string> Array;
    typedef std::set<std::string> Set;
    
    namespace Manip
    {    
      EL_EXCEPTION(Exception, El::ExceptionBase);
      EL_EXCEPTION(InvalidArg, Exception);

      //
      // Mime transcoding
      //
      void mime_url_encode(const char* from, std::string& to)
        throw(El::Exception);
      
      void mime_url_encode(const char* from, std::ostream& to)
        throw(El::Exception);

      void mime_url_decode(const char* from, std::string& to)
        throw(InvalidArg, El::Exception);

      void mime_url_decode(const char* from, std::ostream& to)
        throw(InvalidArg, El::Exception);

      //
      // UTF-8 encoding transcoding
      //
      void wchar_to_utf8(const wchar_t* src, std::string& dest)
        throw(El::Exception);

      void wchar_to_utf8(const wchar_t* src, std::ostream& ostr)
        throw(El::Exception);

      size_t wchar_to_utf8(const wchar_t src,
                           std::ostream& ostr,
                           uint64_t* crc = 0)
        throw(El::Exception);

      size_t wchar_to_utf8(const wchar_t src, std::string& dest)
        throw(El::Exception);

      size_t utf8_char_len(const wchar_t src) throw();
      
      enum UFT8AdditionalCompliance
      {
        UAC_XML_1_0 = 0x1
      };      

      bool utf8_to_wchar(const char* src,
                         std::wstring& dest,
                         bool lax = false,
                         unsigned long compliance = 0)
        throw(InvalidArg, El::Exception);
      
      bool utf8_to_wchar(const char* src,
                         std::wostream& ostr,
                         bool lax = false,
                         unsigned long compliance = 0)
        throw(InvalidArg, El::Exception);

      size_t utf8_to_wchar(std::istream& src,
                           wchar_t& dest,
                           bool lax = false)
        throw(InvalidArg, El::Exception);

      size_t utf8_to_wchar(const char* src,
                           wchar_t& dest,
                           bool lax = false)
        throw(InvalidArg, El::Exception);

      bool utf8_valid(const char* src, unsigned long compliance = 0)
        throw(El::Exception);

      //
      // UTF-16 encoding transcoding
      //
      void wchar_to_utf16(const wchar_t* src, std::ustring& dest)
        throw(InvalidArg, El::Exception);

      void wchar_to_utf16(const wchar_t src, std::uostream& ostr)
        throw(InvalidArg, El::Exception);

      void utf16_to_wchar(const uchar_t* src, std::wstring& dest)
        throw(InvalidArg, El::Exception);

      void utf16_to_wchar(const uchar_t* src, std::wostream& ostr)
        throw(InvalidArg, El::Exception);

      //
      // Windows-1251 transcoding
      //      
      bool win1251_to_wchar(const char* src,
                            std::wstring& dest,
                            bool lax = false)
        throw(InvalidArg, El::Exception);
      
      bool win1251_to_wchar(const char* src,
                            std::wostream& ostr,
                            bool lax = false)
        throw(InvalidArg, El::Exception);
      
      //
      // Latin 1 transcoding
      //      
      void latin1_to_wchar(const char* src, std::wstring& dest)
        throw(InvalidArg, El::Exception);

      void latin1_to_wchar(const char* src, std::wostream& ostr)
        throw(InvalidArg, El::Exception);

      //
      // XML transcoding
      //
      enum XmlEncoding
      {
        XE_TEXT_ENCODING = 0x1,
        XE_ATTRIBUTE_ENCODING = 0x2,
        XE_PRESERVE_UTF8 = 0x4,
        XE_FORCE_NUMERIC_ENCODING = 0x8,
        XE_LAX_ENCODING = 0x10
      };
      
      void xml_encode(
        const wchar_t* src,
        std::string& dst,
        unsigned long flags = XE_TEXT_ENCODING | XE_ATTRIBUTE_ENCODING)
        throw(El::Exception);

      void xml_encode(
        const wchar_t* src,
        std::ostream& dest,
        unsigned long flags = XE_TEXT_ENCODING | XE_ATTRIBUTE_ENCODING)
        throw(El::Exception);
        
      void xml_encode(
        const char* src,
        std::ostream& dest,
        unsigned long flags = XE_TEXT_ENCODING | XE_ATTRIBUTE_ENCODING)
        throw(El::Exception);
        
      void xml_encode(
        const char* src,
        std::string& dest,
        unsigned long flags = XE_TEXT_ENCODING | XE_ATTRIBUTE_ENCODING)
        throw(El::Exception);
        
      void xml_decode(const wchar_t* src, std::string& dest)
        throw(InvalidArg, El::Exception);

      const wchar_t* xml_decode_entity(const wchar_t* src, wchar_t& chr)
        throw(InvalidArg, El::Exception);

      void xml_decode(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception);

      bool uac_xml_1_0_compliant(wchar_t chr) throw();

      //
      // Base64 encoding
      //

      void base64_encode(const unsigned char* src,
                         size_t src_len,
                         std::string& dest)
        throw(El::Exception);
      
      void base64_encode(const unsigned char* src,
                         size_t src_len,
                         std::ostream& dest,
                         size_t chunk_per_line = SIZE_MAX)
        throw(El::Exception);
      
      void base64_decode(const char* src,
                         unsigned char* dest,
                         size_t dest_len)
        throw(InvalidArg, El::Exception);

      void base64_decode(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception);

      //
      // Word encoding
      //

      void utf8_word_encode(const char* src, std::ostream& dest)
        throw(El::Exception);

      //
      // Quoted printable
      
      void quoted_printable_encode(const unsigned char* src,
                                   size_t src_len,
                                   std::ostream& dest,
                                   size_t char_per_line = 76)
        throw(El::Exception);
      
      //
      // Punycode encoding
      //

      enum CaseFlag
      {
        CF_NOCHANGE,
        CF_LOWERCASE,
        CF_UPPERCASE
      };
      
      void punycode_encode(const wchar_t* src,
                           std::string& dest,
                           CaseFlag case_flag = CF_NOCHANGE)
        throw(InvalidArg, El::Exception);

      void punycode_encode(const char* src,
                           std::string& dest,
                           CaseFlag case_flag = CF_NOCHANGE)
        throw(InvalidArg, El::Exception);

      void punycode_decode(const char* src, std::wstring& dest)
        throw(InvalidArg, El::Exception);
      
      void punycode_decode(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception);
      
      //
      // Universal transcoder
      //

      class Transcoder
      {
      public:
        Transcoder(const char* src_encoding,
                   const char* dest_encoding = "utf-8")
          throw(InvalidArg, El::Exception);

        ~Transcoder() throw();

        void encode(const char* src, std::ostream& dest, bool lax = false)
          throw(InvalidArg, El::Exception);

        void encode(const char* src, std::string& dest, bool lax = false)
          throw(InvalidArg, El::Exception);

        void decode(const char* src, std::wstring& dest, bool lax = false)
          throw(InvalidArg, El::Exception);
        
        void decode(const char* src, std::wostream& dest, bool lax = false)
          throw(InvalidArg, El::Exception);
        
      private:

        static const iconv_t NULL_HANDLE_;
        static const size_t ERROR_;

        iconv_t handle_;

      private:
        Transcoder(const Transcoder&);
        void operator=(const Transcoder&);
      };
      
      //
      // Other
      //
      void replace(std::string& str,
                   const char* src_chars,
                   const char* replacement_chars)
        throw(InvalidArg, El::Exception);
      
      void replace(std::wstring& str,
                   const wchar_t* src_chars,
                   const wchar_t* replacement_chars)
        throw(InvalidArg, El::Exception);
      
      void replace(wchar_t* str,
                   size_t len,
                   const wchar_t* src_chars,
                   const wchar_t* replacement_chars)
        throw(InvalidArg, El::Exception);
      
      void replace(std::string& str,
                   const char* src_chars,
                   char replacement_char)
        throw(InvalidArg, El::Exception);

      void suppress(const char* src,
                    std::string& dest,
                    const char* suppress_chars)
        throw(El::Exception);

      bool replace(const char* src,
                   const char* src_chars,
                   const char** replacement_strs,
                   std::ostream& ostr)
        throw(El::Exception);

      void replace(std::string& str,
                   const char* src_chars,
                   const char** replacement_strs)
        throw(InvalidArg, El::Exception);
      
      bool replace(const char* src,
                   const char* pattern,
                   const char* replacement,
                   std::ostream& ostr)
        throw(El::Exception);

      void js_escape(const char* src, std::ostream& ostr) throw(El::Exception);

      void xpath_escape(const char* src, char quote, std::ostream& ostr)
        throw(El::Exception);

      void xpath_escape(const char* src, char quote, std::string& dest)
        throw(El::Exception);

      void compact(const wchar_t* src, std::wostream& ostr)
        throw(El::Exception);
      
      void compact(const wchar_t* src, std::wstring& dest)
        throw(El::Exception);
      
      void compact(const char* src,
                   std::ostream& ostr,
                   bool lax_utf8 = false)
        throw(El::Exception);
      
      void compact(const char* src,
                   std::string& dest,
                   bool lax_utf8 = false) throw(El::Exception);
      
      void trim(const char* src, std::string& dest, size_t src_len = 0)
        throw(El::Exception);
      
      void trim(const wchar_t* src, std::wstring& dest) throw(El::Exception);

      void truncate(const char* src,
                    std::string& dest,
                    size_t max_len,
                    bool lax = false,
                    unsigned long compliance = 0,
                    size_t max_utf8_len = SIZE_MAX)
        throw(El::Exception);

      void truncate(const wchar_t* src,
                    std::wstring& dest,
                    size_t max_len,
                    size_t max_utf8_len = SIZE_MAX)
        throw(El::Exception);

      void to_lower(const wchar_t* src, std::wstring& dest)
        throw(El::Exception);

      wchar_t to_lower(wchar_t chr) throw(El::Exception);
      
      void to_upper(const wchar_t* src, std::wstring& dest)
        throw(El::Exception);

      wchar_t to_upper(wchar_t chr) throw(El::Exception);

      void to_title(const wchar_t* src, std::wstring& dest)
        throw(El::Exception);

      wchar_t to_title(wchar_t chr) throw(El::Exception);

      void to_uniform(const wchar_t* src, std::wstring& dest)
        throw(El::Exception);

      void to_lower(const char* src, std::string& dest)
        throw(El::Exception);

      void to_upper(const char* src, std::string& dest)
        throw(El::Exception);

      void utf8_to_lower(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception);

      void utf8_to_upper(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception);

      void utf8_to_title(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception);

      void utf8_to_uniform(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception);

      bool compliant(wchar_t wchr, unsigned long compliance) throw();

      char* duplicate(const char* src) throw(El::Exception);

      enum NumFormat
      {
        NF_DEC,
        NF_OCT,
        NF_HEX
      };
      
      template <typename TYPE>
      bool numeric(const char* str, TYPE& val, NumFormat format = NF_DEC)
        throw(El::Exception);

      template <typename TYPE>
      bool numeric(const wchar_t* str, TYPE& val, NumFormat format = NF_DEC)
        throw(El::Exception);

      template <typename TYPE>
      std::string string(const TYPE& val) throw(El::Exception);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace String
  {
    namespace Manip
    {
      inline
      void
      replace(std::string& str,
              const char* src_chars,
              const char** replacement_strs)
        throw(InvalidArg, El::Exception)
      {
        std::ostringstream ostr;
        
        if(replace(str.c_str(), src_chars, replacement_strs, ostr))
        {
          str = ostr.str();
        }
      }
      
      inline
      void
      wchar_to_utf8(const wchar_t* src, std::ostream& ostr)
        throw(El::Exception)
      {
        if(src)
        {
          while(*src != L'\0')
          {
            wchar_to_utf8(*src++, ostr);
          }
        }
      }

      inline
      void
      write_char(char chr, std::ostream& ostr, uint64_t* crc) throw()
      {
        ostr << chr;
        
        if(crc)
        {
          El::CRC(*crc, (const unsigned char*)&chr, 1);
        }
      }

      inline
      size_t
      utf8_char_len(const wchar_t src) throw()
      {
        uint32_t chr = src;
          
        if(chr < 0x00000080) 
        {
          return 1;
        } 
        else if(chr < 0x00000800) 
        {
          return 2;
        } 
        else if(chr < 0x00010000) 
        {
          return 3;
        } 
        else if(chr < 0x00200000) 
        {
          return 4;
        }
        else if(chr < 0x04000000) 
        {
          return 5;
        }
        else // chr < 0x80000000
        {
          return 6;
        }
      }
      
      inline
      size_t
      wchar_to_utf8(const wchar_t src,
                    std::ostream& ostr,
                    uint64_t* crc)
        throw(El::Exception)
      {
        uint32_t chr = src;
          
        if(chr < 0x00000080) 
        {
          write_char(chr & 0x0000007F, ostr, crc);
          return 1;
        } 
        else if(chr < 0x00000800) 
        {
          write_char(((chr >> 6) & 0x0000001F) | 0x000000C0, ostr, crc);
          write_char((chr & 0x0000003F) | 0x00000080, ostr, crc);
          return 2;
        } 
        else if(chr < 0x00010000) 
        {
          write_char(((chr >> 12) & 0x0000000F) | 0x000000E0, ostr, crc);
          write_char(((chr >> 6) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char((chr & 0x0000003F) | 0x00000080, ostr, crc);
          return 3;
        } 
        else if(chr < 0x00200000) 
        {
          write_char(((chr >> 18) & 0x00000007) | 0x000000F0, ostr, crc);
          write_char(((chr >> 12) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char(((chr >> 6) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char((chr & 0x0000003F) | 0x00000080, ostr, crc);
          return 4;
        }
        else if(chr < 0x04000000) 
        {
          write_char(((chr >> 24) & 0x00000003) | 0x000000F8, ostr, crc);
          write_char(((chr >> 18) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char(((chr >> 12) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char(((chr >> 6) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char((chr & 0x0000003F) | 0x00000080, ostr, crc);
          return 5;
        }
        else // chr < 0x80000000
        {
          write_char(((chr >> 30) & 0x00000001) | 0x000000FC, ostr, crc);
          write_char(((chr >> 24) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char(((chr >> 18) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char(((chr >> 12) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char(((chr >> 6) & 0x0000003F) | 0x00000080, ostr, crc);
          write_char((chr & 0x0000003F) | 0x00000080, ostr, crc);
          return 6;
        }
      }

      inline
      size_t
      wchar_to_utf8(const wchar_t src, std::string& dest)
        throw(El::Exception)
      {
        uint32_t chr = src;
        char ch;
        
        if(chr < 0x00000080) 
        {
          dest.append(&(ch = char(chr & 0x0000007F)), 1);
          return 1;
        } 
        else if(chr < 0x00000800) 
        {
          dest.append(&(ch = char(((chr >> 6) & 0x0000001F) | 0x000000C0)), 1);
          dest.append(&(ch = char((chr & 0x0000003F) | 0x00000080)), 1);
          return 2;
        } 
        else if(chr < 0x00010000) 
        {
          dest.append(&(ch = char(((chr >> 12) & 0x0000000F) | 0x000000E0)), 1);
          dest.append(&(ch = char(((chr >> 6) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char((chr & 0x0000003F) | 0x00000080)), 1);
          return 3;
        } 
        else if(chr < 0x00200000) 
        {
          dest.append(&(ch = char(((chr >> 18) & 0x00000007) | 0x000000F0)), 1);
          dest.append(&(ch = char(((chr >> 12) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char(((chr >> 6) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char((chr & 0x0000003F) | 0x00000080)), 1);
          return 4;
        } 
        else if(chr < 0x04000000) 
        {
          dest.append(&(ch = char(((chr >> 24) & 0x00000003) | 0x000000F8)), 1);
          dest.append(&(ch = char(((chr >> 18) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char(((chr >> 12) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char(((chr >> 6) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char((chr & 0x0000003F) | 0x00000080)), 1);
          return 5;
        } 
        else // chr < 0x80000000
        {
          dest.append(&(ch = char(((chr >> 30) & 0x00000001) | 0x000000FC)), 1);
          dest.append(&(ch = char(((chr >> 24) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char(((chr >> 18) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char(((chr >> 12) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char(((chr >> 6) & 0x0000003F) | 0x00000080)), 1);
          dest.append(&(ch = char((chr & 0x0000003F) | 0x00000080)), 1);
          return 6;
        }
      }

      inline
      bool
      utf8_to_wchar(const char* src,
                    std::wstring& dest,
                    bool lax,
                    unsigned long compliance)
        throw(InvalidArg, El::Exception)
      {
        std::wostringstream ostr;
        bool res = utf8_to_wchar(src, ostr, lax, compliance);
        dest = ostr.str();

        return res;
      }
      
      inline
      void
      utf16_to_wchar(const uchar_t* src, std::wstring& dest)
        throw(InvalidArg, El::Exception)
      {
        std::wostringstream ostr;
        utf16_to_wchar(src, ostr);
        dest = ostr.str();
      }
      
      inline
      void
      wchar_to_utf16(const wchar_t src, std::uostream& ostr)
        throw(InvalidArg, El::Exception)
      {
        if(src > 0x10FFFF || (src >= 0xD800 && src <= 0xDFFF))
        {
          std::ostringstream ostr;
          ostr << "El::StringManip::wchar_to_utf16: invalid Unicode character "
            "encountered with a code 0x" << std::hex << (unsigned long)src;
          
          throw InvalidArg(ostr.str());
        }

        if(src <= 0xFFFF)
        {
          ostr.put(src);
          return;
        }

        unsigned long shift = (unsigned long)src - 0x10000;

        ostr << (uchar_t)(0xD800 + ((shift >> 10) & 0x3FF))
             << (uchar_t)(0xDC00 + (shift & 0x3FF));
      }

      inline
      void
      base64_encode(const unsigned char* src,
                    size_t src_len,
                    std::string& dest)
        throw(El::Exception)
      {
        std::ostringstream ostr;
        base64_encode(src, src_len, ostr);
        dest = ostr.str();
      }

      inline
      bool
      win1251_to_wchar(const char* src, std::wstring& dest, bool lax)
        throw(InvalidArg, El::Exception)
      {
        std::wostringstream ostr;
        bool res = win1251_to_wchar(src, ostr, lax);
        dest = ostr.str();

        return res;
      }
      
      inline
      void
      latin1_to_wchar(const char* src, std::wstring& dest)
        throw(InvalidArg, El::Exception)
      {
        std::wostringstream ostr;
        latin1_to_wchar(src, ostr);
        dest = ostr.str();
      }
      
      inline
      void
      xml_encode(const wchar_t* src, std::string& dest, unsigned long flags)
        throw(El::Exception)
      {
        std::ostringstream ostr;
        xml_encode(src, ostr, flags);        
        dest = ostr.str();
      }

      inline
      void
      xml_encode(const char* src, std::ostream& dest, unsigned long flags)
        throw(El::Exception)
      {
        std::wstring val;

        utf8_to_wchar(src,
                      val,
                      flags & El::String::Manip::XE_LAX_ENCODING,
                      UAC_XML_1_0);
        
        xml_encode(val.c_str(), dest, flags);
      }

      inline
      void
      xml_encode(const char* src, std::string& dest, unsigned long flags)
        throw(El::Exception)
      {
        std::ostringstream ostr;
        xml_encode(src, ostr, flags);
        dest = ostr.str();
      }

      inline
      void
      xml_decode(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception)
      {
        std::wstring val;
        utf8_to_wchar(src, val);
        xml_decode(val.c_str(), dest);
      }

      inline
      bool
      uac_xml_1_0_compliant(wchar_t wchr) throw()
      {
        return (wchr >= 0x20 && wchr <= 0xD7FF) || wchr == 0xA ||
          wchr == 0xD || wchr == 0x9 || (wchr >= 0xE000 && wchr <= 0xFFFD) ||
          (wchr >= 0x10000 && wchr <= 0x10FFFF);
      }

      inline
      void
      punycode_encode(const char* src, std::string& dest, CaseFlag case_flag)
        throw(InvalidArg, El::Exception)
      {
        std::wstring val;
        utf8_to_wchar(src, val);
        punycode_encode(val.c_str(), dest, case_flag);
      }

      inline
      void
      punycode_decode(const char* src, std::string& dest)
        throw(InvalidArg, El::Exception)
      {
        std::wstring val;
        punycode_decode(src, val);
        El::String::Manip::wchar_to_utf8(val.c_str(), dest);
      }

      inline
      void
      to_lower(const wchar_t* src, std::wstring& dest) throw(El::Exception)
      {
        El::String::Unicode::CharTable::to_lower(src, dest);
      }

      inline
      void
      to_uniform(const wchar_t* src, std::wstring& dest) throw(El::Exception)
      {
        El::String::Unicode::CharTable::to_uniform(src, dest);
      }

      inline
      void
      to_upper(const wchar_t* src, std::wstring& dest) throw(El::Exception)
      {
        El::String::Unicode::CharTable::to_upper(src, dest);
      }

      inline
      void
      to_title(const wchar_t* src, std::wstring& dest) throw(El::Exception)
      {
        El::String::Unicode::CharTable::to_title(src, dest);
      }

      inline
      wchar_t
      to_lower(wchar_t chr) throw(El::Exception)
      {
        return El::String::Unicode::CharTable::to_lower(chr);
      }
      
      inline
      wchar_t
      to_title(wchar_t chr) throw(El::Exception)
      {
        return El::String::Unicode::CharTable::to_title(chr);
      }
      
      inline
      wchar_t
      to_upper(wchar_t chr) throw(El::Exception)
      {
        return El::String::Unicode::CharTable::to_upper(chr);
      }

      inline
      void
      compact(const char* src,
              std::string& dest,
              bool lax_utf8) throw(El::Exception)
      {
        std::ostringstream ostr;
        compact(src, ostr, lax_utf8);
        dest = ostr.str();
      }

      inline
      void
      compact(const char* src,
              std::ostream& ostr,
              bool lax_utf8) throw(El::Exception)
      {
        std::wstring dest;
        utf8_to_wchar(src, dest, lax_utf8);

        std::wstring compacted;
        compact(dest.c_str(), compacted);
        wchar_to_utf8(compacted.c_str(), ostr);
      }

      inline
      void
      compact(const wchar_t* src, std::wstring& dest)
        throw(El::Exception)
      {
        std::wostringstream ostr;
        compact(src, ostr);
        dest = ostr.str();
      }

      inline
      char*
      duplicate(const char* src) throw(El::Exception)
      {
        if(src == 0)
        {
          return 0;
        }
        
        char* dup = new char[strlen(src) + 1];
        strcpy(dup, src);
        
        return dup;
      }

      template <typename TYPE>
      bool
      numeric(const char* str, TYPE& val, NumFormat format)
        throw(El::Exception)
      {
        val = 0;

        std::string trimmed;
        trim(str, trimmed);
      
        TYPE tmp;
        std::istringstream istr(trimmed);

        switch(format)
        {
        case NF_OCT: istr >> std::oct; break;
        case NF_HEX: istr >> std::hex; break;
        default: break;
        } 
          
        istr >> tmp;

        if(istr.fail() || !istr.eof())
        {
          return false;
        }

        val = tmp;
        return true;
      }

      template <typename TYPE>
      bool
      numeric(const wchar_t* str, TYPE& val, NumFormat format)
        throw(El::Exception)
      {
        val = 0;

        std::wstring trimmed;
        trim(str, trimmed);
      
        TYPE tmp;
        std::wistringstream istr(trimmed);

        switch(format)
        {
        case NF_OCT: istr >> std::oct; break;
        case NF_HEX: istr >> std::hex; break;
        default: break;
        } 
          
        istr >> tmp;

        if(istr.fail() || !istr.eof())
        {
          return false;
        }

        val = tmp;
        return true;
      }

      template <typename TYPE>
      std::string
      string(const TYPE& val) throw(El::Exception)
      {
        std::ostringstream ostr;
        ostr << val;
        return ostr.str();
      }

/*      
      inline
      bool
      check_utf8_lowercasing() throw()
      {
        const char A[] = "\xD0\x90";
        const char a[] = "\xD0\xB0";
        
        std::string sa;
        utf8_to_lower(A, sa);
        
        return sa == a;
      }
*/
      //
      // Transcoder class
      //

      inline
      Transcoder::Transcoder(const char* src_encoding,
                             const char* dest_encoding)
        throw(InvalidArg, El::Exception)
          : handle_(iconv_open(dest_encoding, src_encoding))
      {
        if(handle_ == NULL_HANDLE_)
        {
          std::ostringstream ostr;
          ostr << "El::String::Manip::Transcoder::Transcoder: iconv_open "
            "fails to convert from '" << src_encoding << "' to '"
               << dest_encoding << "'. Reason: " << ACE_OS::strerror(errno);

          throw InvalidArg(ostr.str());
        }
      }

      inline
      Transcoder::~Transcoder() throw()
      {
        if(handle_ != NULL_HANDLE_)
        {
          iconv_close(handle_);
        }
      }

      inline
      void
      Transcoder::encode(const char* src, std::string& dest, bool lax)
        throw(InvalidArg, El::Exception)
      {
        std::ostringstream ostr;
        encode(src, ostr, lax);
        dest = ostr.str();
      }
      
      inline
      void
      Transcoder::decode(const char* src, std::wstring& dest, bool lax)
        throw(InvalidArg, El::Exception)
      {
        std::string utf8;
        encode(src, utf8, lax);
        utf8_to_wchar(utf8.c_str(), dest, lax);
      }
        
      inline
      void
      Transcoder::decode(const char* src, std::wostream& dest, bool lax)
        throw(InvalidArg, El::Exception)
      {
        std::string utf8;
        encode(src, utf8, lax);
        utf8_to_wchar(utf8.c_str(), dest, lax);
      }

      inline
      bool
      compliant(wchar_t wchr, unsigned long compliance) throw()
      {
        if(compliance & UAC_XML_1_0)
        {
          return uac_xml_1_0_compliant(wchr);
        }

        return true;
      }
    }
  }
}

#endif // _ELEMENTS_EL_STRING_MANIP_HPP_
