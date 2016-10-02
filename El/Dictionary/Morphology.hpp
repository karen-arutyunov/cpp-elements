/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Dictionary/Morphology.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_DICTIONARY_MORPHOLOGY_HPP_
#define _ELEMENTS_EL_DICTIONARY_MORPHOLOGY_HPP_

#include <stdint.h>

#include <iostream>
#include <vector>
#include <map>
#include <memory>

#include <ext/hash_map>
#include <ext/hash_set>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/Lang.hpp>
#include <El/String/StringPtr.hpp>
#include <El/String/Manip.hpp>
#include <El/ArrayPtr.hpp>
#include <El/RefCount/All.hpp>
#include <El/Hash/Hash.hpp>
#include <El/CRC.hpp>
#include <El/BinaryStream.hpp>

namespace El
{
  namespace Dictionary
  {
    namespace Morphology
    {
      EL_EXCEPTION(Exception, El::ExceptionBase);
      EL_EXCEPTION(InvalidArg, Exception);
      
      typedef uint32_t WordId;
      
      struct WordForm
      {
        WordId id;
        El::Lang lang;
        bool is_stop_word;

        WordForm() throw() : id(0), is_stop_word(false) {}
        
        WordForm(WordId id_val,
                 const El::Lang& lang_val,
                 bool is_stop_word_val) throw(El::Exception);

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);        
      };
      
      typedef std::vector<WordForm> WordFormArray;
      
      struct WordInfo
      {
        El::Lang lang;
        WordFormArray forms;

        bool contained(const WordInfo& wi) const throw();

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);
      };
      
      typedef std::vector<WordInfo> WordInfoArray;      
      typedef std::vector<WordId> WordIdArray;
    
      typedef std::vector<El::String::StringConstPtr> WordArray;
      
      struct WordNormalForms
      {
        size_t normal_form_count;
        size_t normal_form_offset;

        WordNormalForms() throw(El::Exception);
      };

      typedef std::vector<WordNormalForms> WordNormalFormsArray;
    
      class WordNormalFormsMap :
        public __gnu_cxx::hash_map<El::String::StringConstPtr,
                                   WordNormalForms,
                                   El::Hash::StringConstPtr>,
        virtual public RefCount::DefaultImpl<>
      {        
      public:
        WordNormalFormsMap() throw(El::Exception);
        virtual ~WordNormalFormsMap() throw();

        void load(const char* dict_file,
                  const char* stop_words_file,
                  std::ostream* warnings_stream)
          throw(InvalidArg, Exception, El::Exception);

        El::Lang lang() const throw() { return lang_; }
        uint32_t hash() const throw() { return hash_; }
        
        bool get_normal_forms(const char* word, WordFormArray& forms) const
          throw(El::Exception);

      private:
        
        typedef El::ArrayPtr<char> BuffPtr;

        typedef __gnu_cxx::hash_map<WordId,
                                    El::String::StringConstPtr,
                                    El::Hash::Numeric<WordId> >
        StopWordsMap;
        
        BuffPtr buff_;
        El::Lang lang_;
        uint32_t hash_;
        WordIdArray norm_form_ids_;
        StopWordsMap stop_words_;

      private:
        WordNormalFormsMap(const WordNormalFormsMap&);
        void operator=(const WordNormalFormsMap&);
      };

      typedef RefCount::SmartPtr<WordNormalFormsMap> WordNormalFormsMap_var;

      struct Lemma
      {
        size_t word_form_count;
        size_t word_form_offset;
        El::String::StringConstPtr text;

        Lemma() throw(El::Exception);

        struct Word
        {
          WordId id;
          El::String::StringConstPtr text;
          
          Word() throw(El::Exception) : id(0) {}
        };

        typedef std::vector<Word> WordArray;

        enum GuessStrategy
        {
          GS_NONE,
          GS_SIMILAR,
          GS_TRANSFORM,
          GS_COUNT
        };
      };

      struct LemmaInfo
      {
        struct Word
        {
          WordId id;
          std::string text;
          
          Word() throw(El::Exception) : id(0) {}
          
          void write(El::BinaryOutStream& bstr) const throw(El::Exception);
          void read(El::BinaryInStream& bstr) throw(El::Exception);
        };
        
        typedef std::vector<Word> WordArray;

        uint8_t known;
        WordArray word_forms;
        El::Lang lang;
        Word norm_form;

        LemmaInfo() throw() : known(1) {}

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);
      };

      struct LemmaInfoArray : public std::vector<LemmaInfo>
      {
        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);        
      };
      
      typedef std::vector<LemmaInfoArray> LemmaInfoArrayArray;

      class LemmaMap :
        public __gnu_cxx::hash_map<WordId, Lemma, El::Hash::Numeric<WordId> >,
        virtual public RefCount::DefaultImpl<>
      {        
      public:
        LemmaMap() throw(El::Exception) {}
        virtual ~LemmaMap() throw() {}

        El::Lang lang() const throw() { return lang_; }
        
        void load(const char* dict_file,
                  const WordNormalFormsMap& norm_form_map)
          throw(InvalidArg, Exception, El::Exception);

        bool get_lemma(WordId id, LemmaInfoArray& lemmas) const
          throw(Exception, El::Exception);

        bool guess_lemma(const char* word,
                         Lemma::GuessStrategy strategy,
                         LemmaInfoArray& lemmas) const
          throw(Exception, El::Exception);

      private:

        static long match_length(const wchar_t* word1,
                                 long word1_len,
                                 const wchar_t* word2,
                                 long word2_len) throw();

      private:
        El::Lang lang_;
        Lemma::WordArray word_forms_;

      private:
        LemmaMap(const LemmaMap&);
        void operator=(const LemmaMap&);
      };

      typedef RefCount::SmartPtr<LemmaMap> LemmaMap_var;
      
      class WordInfoManager
      {
      public:

        WordInfoManager(size_t default_lang_validation_level, // 10
                        size_t guessing_default_lang_validation_level, //30
                        size_t lang_validation_level)
          throw(El::Exception);
        
        void load(const char* dict_file, std::ostream* warnings_stream)
          throw(InvalidArg, Exception, El::Exception);

        void normal_form_ids(const WordArray& words,
                             WordInfoArray& word_infos,
                             El::Lang* lang,
                             bool narrow_by_containment) const
          throw(El::Exception);
        
        void get_lemmas(const WordArray& words,
                        const El::Lang* lang,
                        Lemma::GuessStrategy guess_strategy,
                        LemmaInfoArrayArray& lemmas) const
          throw(El::Exception);

        const char* get_normal_form(const char* word,
                                    const El::Lang& lang,
                                    bool& is_stop_word) const
          throw(El::Exception);
        
        unsigned long languages() const throw(El::Exception);
        bool is_loaded(El::Lang lang) throw(El::Exception);
        uint32_t hash() const throw() { return hash_; }
        
      private:
        
        typedef __gnu_cxx::hash_map<El::Lang, unsigned long, El::Hash::Lang>
        LangRates;

        typedef std::auto_ptr<LangRates> LangRatesPtr;
        
        static void add_rate(LangRates& lang_rates, El::Lang lang)
          throw(El::Exception);

        void adjust_lang(El::Lang& lang,
                         unsigned long recornized_words,
                         unsigned long unrecornized_words,
                         unsigned long guessed_words,
                         const LangRates& lang_rates,
                         const LangRates& guessed_lang_rates,
                         size_t default_lang_validation_level) const
          throw(El::Exception);
        
      private:
        
        typedef __gnu_cxx::hash_map<El::Lang,
                                    WordNormalFormsMap_var,
                                    El::Hash::Lang> LangWordNormalFormsMap;

        typedef std::map<unsigned long, WordNormalFormsMap_var>
        PopularityWordNormalFormsMap;

        typedef __gnu_cxx::hash_map<El::Lang,
                                    LemmaMap_var,
                                    El::Hash::Lang> LangLemmaMap;
        
        LangWordNormalFormsMap word_normal_forms_;
        PopularityWordNormalFormsMap popularity_word_normal_forms_;
        
        LangLemmaMap lemmas_;
        
        size_t default_lang_validation_level_;
        size_t guessing_default_lang_validation_level_;
        size_t lang_validation_level_;
        uint32_t hash_;
      };

      WordId pseudo_id(const char* word) throw();

      enum TokenType
      {
        TT_UNDEFINED,
        TT_WORD,     // EC_LETTER + EC_INTERWORD
        TT_NUMBER,   // EC_NUMBER + EC_INTERNUMBER
        TT_SURROGATE // At least 1 EC_LETTER or EC_NUMBER
      };

      TokenType token_type(const wchar_t* token) throw();
      TokenType token_type(const char* token) throw(El::Exception);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Dictionary
  {
    namespace Morphology
    {  
      //
      // WordForm struct
      //
      inline
      WordForm::WordForm(WordId id_val,
                         const El::Lang& lang_val,
                         bool is_stop_word_val)
        throw(El::Exception)
          : id(id_val),
            lang(lang_val),
            is_stop_word(is_stop_word_val)
      {
      }

      inline
      void
      WordForm::write(El::BinaryOutStream& bstr) const throw(El::Exception)
      {
        bstr << id << lang << (uint8_t)is_stop_word;
      }

      inline
      void
      WordForm::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        uint8_t isw = 0;
        bstr >> id >> lang >> isw;
        is_stop_word = isw;
      }
      
      //
      // WordInfo struct
      //

      inline
      void
      WordInfo::write(El::BinaryOutStream& bstr) const throw(El::Exception)
      {
        bstr << lang;
        bstr.write_array(forms);
      }

      inline
      void
      WordInfo::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> lang;
        bstr.read_array(forms);
      }
      
      inline
      bool
      WordInfo::contained(const WordInfo& wi) const throw()
      {
        if(forms.empty() || forms.size() >= wi.forms.size())
        {
          return false;
        }

        WordFormArray::const_iterator wib(wi.forms.begin());
        WordFormArray::const_iterator wie(wi.forms.end());

        for(WordFormArray::const_iterator i(forms.begin()), e(forms.end());
            i != e; ++i)
        {
          WordId id = i->id;
          WordFormArray::const_iterator j(wib);
          
          for(; j != wie && j->id != id; ++j);

          if(j == wie)
          {
            return false;
          }
        }
/*
        std::cerr << lang.l3_code() << " ";

        for(WordFormArray::const_iterator i(forms.begin()), e(forms.end());
            i != e; ++i)
        {
          std::cerr << i->id << " ";
        }

        std::cerr << "IN " << wi.lang.l3_code() << " ";

        for(WordFormArray::const_iterator i(wi.forms.begin()),
            e(wi.forms.end()); i != e; ++i)
        {
          std::cerr << i->id << " ";
        }

        std::cerr << std::endl;
*/
        
        return true;
      }
      
      //
      // WordNormalForms struct
      //
      inline
      WordNormalForms::WordNormalForms() throw(El::Exception)
          : normal_form_count(0),
            normal_form_offset(0)
      {
      }
    
      //
      // Lemma struct
      //
      inline
      Lemma::Lemma() throw(El::Exception)
          : word_form_count(0),
            word_form_offset(0)
      {
      }

      //
      // LemmaInfo::Word struct
      //
      inline
      void
      LemmaInfo::Word::write(El::BinaryOutStream& bstr) const
        throw(El::Exception)
      {
        bstr << id << text;
      }
      
      inline
      void
      LemmaInfo::Word::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> id >> text;
      }
      
      //
      // LemmaInfo struct
      //
      inline
      void
      LemmaInfo::write(El::BinaryOutStream& bstr) const throw(El::Exception)
      {
        bstr << known << lang << norm_form;
        bstr.write_array(word_forms);
      }
      
      inline
      void
      LemmaInfo::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> known >> lang >> norm_form;
        bstr.read_array(word_forms);
      }

      //
      // LemmaInfoArray struct
      //
      inline
      void
      LemmaInfoArray::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr.read_array(*this);
      }
      
      inline
      void
      LemmaInfoArray::write(El::BinaryOutStream& bstr) const
        throw(El::Exception)
      {
        bstr.write_array(*this);
      }
      
      //
      // WordNormalFormsMap class
      //
      inline
      WordNormalFormsMap::WordNormalFormsMap() throw(El::Exception) : hash_(0)
      {
      }
    
      inline
      WordNormalFormsMap::~WordNormalFormsMap() throw()
      {
      }

      inline
      bool
      WordNormalFormsMap::get_normal_forms(const char* word,
                                           WordFormArray& forms) const
        throw(El::Exception)
      {
        const_iterator wit = find(word);

        if(wit == end())
        {
          switch(lang_.el_code())
          {
          case El::Lang::EC_ENG:
            {              
              size_t len = strlen(word);
          
              if(len > 2 && strcasecmp(word + len - 2, "'s") == 0)
              {
                std::string w(word, len - 2);
                wit = find(w);
              }
              else if(len > 1 && word[len - 1] == '\'')
              {
                std::string w(word, len - 1);
                wit = find(w);
              }

              break;
            }  
          case El::Lang::EC_RUS:
            {              
              size_t len = strlen(word);
          
              if(len > 5 &&
                 strcasecmp(word + len - 5, "-\xD1\x82\xD0\xBE") == 0) //-to
              {
                std::string w(word, len - 5);
                wit = find(w);
              }
              else if(len > 9 &&
                      strcasecmp(word + len - 9, //-libo
                                 "-\xD0\xBB\xD0\xB8\xD0\xB1\xD0\xBE") == 0)
              {
                std::string w(word, len - 9);
                wit = find(w);
              }
              else if(len > 13 &&
                      strcasecmp(word + len - 13, //-nibud
                                 "-\xD0\xBD\xD0\xB8\xD0\xB1\xD1\x83\xD0\xB4\xD1\x8C") == 0)
              {
                std::string w(word, len - 13);
                wit = find(w);
              }
              else if(len > 7 &&
                      strncasecmp(word, //koe-
                                  "\xD0\xBA\xD0\xBE\xD0\xB5-",
                                  7) == 0)
              {
                wit = find(word + 7);
              }
              else if(len > 5 &&
                      strncasecmp(word, //po-
                                  "\xD0\xBF\xD0\xBE-",
                                  5) == 0)
              {
                wit = find(word + 5);
              }

              break;
            }
          default:
            {
              break;
            }
          }
        }
        
        if(wit == end())
        {
          return false;
        }

        const WordNormalForms& wi = wit->second;
              
        size_t count = wi.normal_form_count;
        for(size_t i = wi.normal_form_offset; count; count--, i++)
        {
          WordId id = norm_form_ids_[i];
          
          forms.push_back(WordForm(id,
                                   lang_,
                                   stop_words_.find(id) != stop_words_.end()));
        }

        return true;
      }
      
      //
      // WordInfoManager class
      //

      inline
      WordInfoManager::WordInfoManager(
        size_t default_lang_validation_level,
        size_t guessing_default_lang_validation_level,
        size_t lang_validation_level)
        throw(El::Exception)
          : default_lang_validation_level_(default_lang_validation_level),
            guessing_default_lang_validation_level_(
              guessing_default_lang_validation_level),
            lang_validation_level_(lang_validation_level),
            hash_(1) // To ensure hash is "never" equal 0
      {
      }
      
      inline
      unsigned long
      WordInfoManager::languages() const throw(El::Exception)
      {
        return word_normal_forms_.size();
      }

      inline
      bool
      WordInfoManager::is_loaded(El::Lang lang) throw(El::Exception)
      {
        return word_normal_forms_.find(lang) != word_normal_forms_.end();
      }

      inline
      void
      WordInfoManager::add_rate(LangRates& lang_rates, El::Lang lang)
        throw(El::Exception)
      {
        LangRates::iterator it = lang_rates.find(lang);
              
        if(it == lang_rates.end())
        {
          lang_rates[lang] = 1;
        }
        else
        {
          it->second++;
        }
      }
      
      inline
      WordId
      pseudo_id(const char* word) throw()
      {
        WordId crc = 0;
        El::CRC(crc, (const unsigned char*)word, word ? strlen(word) : 0);
        return crc | 0x80000000;
      }

      inline
      TokenType
      token_type(const char* token) throw(El::Exception)
      {
        std::wstring token_w;
        El::String::Manip::utf8_to_wchar(token, token_w);
        return token_type(token_w.c_str());
      }
    }
  }
}

#endif // _ELEMENTS_EL_DICTIONARY_MORPHOLOGY_HPP_
