/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/String/Unicode.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STRING_UNICODE_HPP_
#define _ELEMENTS_EL_STRING_UNICODE_HPP_

#include <vector>
#include <string>

#include <ext/hash_map>
#include <google/dense_hash_map>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>

namespace El
{
  namespace String
  {
    namespace Unicode
    {
      EL_EXCEPTION(Exception, El::ExceptionBase);

      enum GeneralCategory
      {
        GC_Lu, // Letter, Uppercase
        GC_Ll, // Letter, Lowercase
        GC_Lt, // Letter, Titlecase
        GC_Mn, // Mark, Non-Spacing
        GC_Mc, // Mark, Spacing Combining
        GC_Me, // Mark, Enclosing
        GC_Nd, // Number, Decimal Digit
        GC_Nl, // Number, Letter
        GC_No, // Number, Other
        GC_Zs, // Separator, Space
        GC_Zl, // Separator, Line
        GC_Zp, // Separator, Paragraph
        GC_Cc, // Other, Control
        GC_Cf, // Other, Format
        GC_Cs, // Other, Surrogate
        GC_Co, // Other, Private Use
        GC_Cn, // Other, Not Assigned
        GC_Lm, // Letter, Modifier
        GC_Lo, // Letter, Other
        GC_Pc, // Punctuation, Connector
        GC_Pd, // Punctuation, Dash
        GC_Ps, // Punctuation, Open
        GC_Pe, // Punctuation, Close
        GC_Pi, // Punctuation, Initial quote
        GC_Pf, // Punctuation, Final quote
        GC_Po, // Punctuation, Other
        GC_Sm, // Symbol, Math
        GC_Sc, // Symbol, Currency
        GC_Sk, // Symbol, Modifier
        GC_So  // Symbol, Other
      };

      enum ElCategory
      {
        EC_SPACE = 0x1,
        EC_STOP = 0x2,
        EC_SINGLE_QUOTE = 0x4,
        EC_MULTI_QUOTE = 0x8,
        EC_BRACKET = 0x10,
        EC_OPENING = 0x20,
        EC_CLOSING = 0x40,
        EC_QUOTE = EC_SINGLE_QUOTE | EC_MULTI_QUOTE,
        EC_LETTER = 0x80,
        EC_NUMBER = 0x100,
        EC_INTERWORD = 0x200,
        EC_INTERNUMBER = 0x400,
        EC_END_OF_SENTENCE = 0x800
      };
      
      struct CharDescriptor
      {
        wchar_t code;
        GeneralCategory general_category;
        const char* decomposition;
        wchar_t upper_case;
        wchar_t lower_case;
        wchar_t title_case;
      };

      struct CharInfo
      {
        unsigned long flags;
        wchar_t symmetric_code;
        const CharDescriptor* desc;

        CharInfo() throw() : flags(0), symmetric_code(0), desc(0) {}
      };

      class CharTable :
        public google::dense_hash_map<wchar_t,
                                      CharInfo,
                                      El::Hash::Numeric<wchar_t> >
      {
      public:
        CharTable() throw(El::Exception);

        static void to_lower(const wchar_t* src, std::wstring& dest)
          throw(El::Exception);

        static wchar_t to_lower(wchar_t chr) throw(El::Exception);

        static void to_upper(const wchar_t* src, std::wstring& dest)
          throw(El::Exception);

        static wchar_t to_upper(wchar_t chr) throw(El::Exception);

        static void to_title(const wchar_t* src, std::wstring& dest)
          throw(El::Exception);

        static wchar_t to_title(wchar_t chr) throw(El::Exception);

        static void to_uniform(const wchar_t* src, std::wstring& dest)
          throw(El::Exception);

        static bool is_space(wchar_t chr) throw(El::Exception);
        static bool is_stop(wchar_t chr) throw(El::Exception);

        static wchar_t is_single_quote(wchar_t chr) throw(El::Exception);
        static wchar_t is_multi_quote(wchar_t chr) throw(El::Exception);
        static wchar_t is_bracket(wchar_t chr) throw(El::Exception);

        static wchar_t is_opening(wchar_t chr) throw(El::Exception);
        static wchar_t is_closing(wchar_t chr) throw(El::Exception);

        static bool is_letter(wchar_t chr) throw(El::Exception);
        static bool is_number(wchar_t chr) throw(El::Exception);
        static bool is_interword(wchar_t chr) throw(El::Exception);
        static bool is_internumber(wchar_t chr) throw(El::Exception);
        
        static GeneralCategory general_category(wchar_t chr)
          throw(El::Exception);
        
        static unsigned long el_categories(wchar_t chr) throw(El::Exception);

      public:
        
        static const CharTable instance;

        struct Range
        {
          wchar_t first;
          wchar_t last;
          const CharInfo* info;

          Range(wchar_t f = L'\0', wchar_t l = L'\0', const CharInfo* i = 0)
            throw();
        };

      private:

        const CharInfo* info(wchar_t chr) const throw();

        bool normalize(std::wstring& norm_repr) throw(El::Exception);
        
      private:

        typedef std::vector<Range> RangeArray;
        
        RangeArray ranges_;
        RangeArray::const_iterator begin_ranges_;
        RangeArray::const_iterator end_ranges_;

        typedef __gnu_cxx::hash_map<wchar_t,
                                    std::wstring,
                                    El::Hash::Numeric<wchar_t> >
        NormalizationMap;

        NormalizationMap normal_representation_;
        
        static const CharDescriptor descriptors_[];
      };
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
    namespace Unicode
    {
      //
      // CharTable class
      //

      inline
      const CharInfo*
      CharTable::info(wchar_t chr) const throw()
      {
        CharTable::const_iterator it = find(chr);
        
        if(it != end())
        {
          return &it->second;
        }

        for(RangeArray::const_iterator it = begin_ranges_; it != end_ranges_;
            ++it)
        {
          if(chr >= it->first && chr <= it->last)
          {
            return it->info;
          }
        }

        return 0;
      }
      
      inline
      GeneralCategory
      CharTable::general_category(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);

        const CharInfo* i = instance.info(chr);
        return i ? i->desc->general_category : GC_Cn;
      }

      inline
      unsigned long
      CharTable::el_categories(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);

        const CharInfo* i = instance.info(chr);
        return i ? i->flags : 0;
      }

      inline
      bool
      CharTable::is_space(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        return it != instance.end() && (it->second.flags & EC_SPACE);
      }

      inline
      bool
      CharTable::is_letter(wchar_t chr) throw(El::Exception)
      {
        const CharInfo* i = instance.info(chr);
        return i ? (i->flags & EC_LETTER) : false;
      }
      
      inline
      bool
      CharTable::is_number(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        return it != instance.end() && (it->second.flags & EC_NUMBER);
      }
      
      inline
      bool
      CharTable::is_interword(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        return it != instance.end() && (it->second.flags & EC_INTERWORD);
      }
      
      inline
      bool
      CharTable::is_internumber(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        return it != instance.end() && (it->second.flags & EC_INTERNUMBER);
      }
      
      inline
      bool
      CharTable::is_stop(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        return it != instance.end() && (it->second.flags & EC_STOP);
      }

      inline
      wchar_t
      CharTable::is_single_quote(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        
        return it != instance.end() && (it->second.flags & EC_SINGLE_QUOTE) ?
          it->second.symmetric_code : 0;
      }
      
      inline
      wchar_t
      CharTable::is_multi_quote(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        
        return it != instance.end() && (it->second.flags & EC_MULTI_QUOTE) ?
          it->second.symmetric_code : 0;
      }
      
      inline
      wchar_t
      CharTable::is_bracket(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        
        return it != instance.end() && (it->second.flags & EC_BRACKET) ?
          it->second.symmetric_code : 0;
      }
      
      inline
      wchar_t
      CharTable::is_opening(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        
        return it != instance.end() && (it->second.flags & EC_OPENING) ?
          it->second.symmetric_code : 0;
      }
      
      inline
      wchar_t
      CharTable::is_closing(wchar_t chr) throw(El::Exception)
      {
        CharTable::const_iterator it = instance.find(chr);
        
        return it != instance.end() && (it->second.flags & EC_CLOSING) ?
          it->second.symmetric_code : 0;
      }

      inline
      wchar_t
      CharTable::to_upper(wchar_t chr) throw(El::Exception)
      {
        wchar_t uc;
        CharTable::const_iterator it = instance.find(chr);
        
        return it != instance.end() &&
          (uc = it->second.desc->upper_case) ? uc : chr; 
      }
      
      inline
      wchar_t
      CharTable::to_lower(wchar_t chr) throw(El::Exception)
      {
        wchar_t lc;
        CharTable::const_iterator it = instance.find(chr);
        
        return it != instance.end() &&
          (lc = it->second.desc->lower_case) ? lc : chr; 
      }
      
      inline
      wchar_t
      CharTable::to_title(wchar_t chr) throw(El::Exception)
      {
        wchar_t tc;
        CharTable::const_iterator it = instance.find(chr);
        
        return it != instance.end() &&
          (tc = it->second.desc->title_case) ? tc : chr; 
      }
      
      //
      // CharTable::Range struct
      //
      inline
      CharTable::Range::Range(wchar_t f, wchar_t l, const CharInfo* i) throw()
          : first(f), last(l), info(i)
      {
      }
    }
  }
}

#endif // _ELEMENTS_EL_STRING_UNICODE_HPP_
