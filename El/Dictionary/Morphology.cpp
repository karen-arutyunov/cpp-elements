/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Dictionary/Morphology.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>

#include <ext/hash_set>
#include <google/sparse_hash_set>

#include <El/String/Manip.hpp>
#include <El/String/ListParser.hpp>
#include <El/CRC.hpp>

#include <El/Dictionary/LangDetection.hpp>

#include "Morphology.hpp"

//#define TRACE_LANG_DETECT

struct IdBase
{
  El::Lang::ElCode lang;
  El::Dictionary::Morphology::WordId id_base;
};

IdBase ID_BASES[] =
{
  { El::Lang::EC_ENG,         0 }, // ~180K word forms, 1M reserved
  { El::Lang::EC_RUS,   1000000 }, // ~3M word forms, 10M reserved
  { El::Lang::EC_GER,  10000000 }, // ~1M word forms, 4M reserved
  { El::Lang::EC_KOR,  14000000 }, // ~270K word forms, 1M reserved
  { El::Lang::EC_CHI,  15000000 }, // ~65K word forms, 1M reserved
  { El::Lang::EC_JPN,  16000000 }, // ~240K word forms, 1M reserved
  { El::Lang::EC_POR,  17000000 }, // ~360K word forms, 1M reserved
  { El::Lang::EC_ITA,  18000000 }, // ~225K word forms, 1M reserved
  { El::Lang::EC_SPA,  19000000 }, // ~315K word forms, 1M reserved
  { El::Lang::EC_RUM,  20000000 }, // ~2M word forms, 10M reserved
  { El::Lang::EC_SLO,  30000000 }, // ? word forms, 2M reserved
  { El::Lang::EC_TUR,  32000000 }, // ~40K word forms, 1M reserved
  { El::Lang::EC_NUL,  33000000 }  // Ids boundry
};

typedef __gnu_cxx::hash_set<El::Lang, El::Hash::Lang> LangSet;
typedef std::auto_ptr<LangSet> LangSetPtr;

struct LangWeight
{
  El::Lang lang;
  unsigned long weight;
  unsigned long index;
  bool default_lang;

  LangWeight(El::Lang lang_val = El::Lang::null,
             unsigned long weight_val = 0,
             unsigned long index_val = 0,
             bool default_lang_val = false) throw();

  bool operator<(const LangWeight& val) const throw();
};

inline
LangWeight::LangWeight(El::Lang lang_val,
                       unsigned long weight_val,
                       unsigned long index_val,
                       bool default_lang_val) throw()
    : lang(lang_val),
      weight(weight_val),
      index(index_val),
      default_lang(default_lang_val)
{
}

inline
bool
LangWeight::operator<(const LangWeight& val) const throw()
{
  if(weight > val.weight)
  {
    return true;
  }

  if(weight < val.weight)
  {
    return false;
  }

  if(default_lang > val.default_lang)
  {
    return true;
  }

  if(default_lang < val.default_lang)
  {
    return false;
  }

  return index < val.index;
}

typedef std::vector<LangWeight> LangWeightArray;
typedef std::auto_ptr<LangWeightArray> LangWeightArrayPtr;

namespace El
{
  namespace Dictionary
  {
    namespace Morphology
    {      
      struct WordIdSet :
        public google::sparse_hash_set<WordId, El::Hash::Numeric<WordId> >
      {
        WordIdSet() throw(El::Exception) { set_deleted_key(0); }
      };

      
      struct LemmasCrc :
        public google::sparse_hash_set<unsigned long long,
                                       El::Hash::Numeric<unsigned long long> >
      {
        LemmasCrc() throw(El::Exception) { set_deleted_key(0); }
      };
      
      //
      // WordInfoMap class
      //
      void
      WordNormalFormsMap::load(const char* dict_file,
                               const char* stop_words_file,
                               std::ostream* warnings_stream)
        throw(InvalidArg, Exception, El::Exception)
      {
        clear();
        buff_.reset(0);
        norm_form_ids_.clear();
        
        lang_ = El::Lang::EC_NUL;
        hash_ = 0;
        
        std::fstream file(dict_file, std::ios::in);

        if(!file.is_open())
        {
          std::ostringstream ostr;
          ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
            "failed to open file " << dict_file;

          throw InvalidArg(ostr.str());          
        }

        uint32_t records = 0;
        uint32_t buff_size = 0;
        uint32_t norm_forms_count = 0;
        std::string lang_s;

        file >> lang_s >> records >> buff_size >> norm_forms_count;

        El::CRC(hash_, (const unsigned char*)lang_s.c_str(), lang_s.length());
        El::CRC(hash_, (const unsigned char*)&records, sizeof(records));
        El::CRC(hash_, (const unsigned char*)&buff_size, sizeof(buff_size));
        
        El::CRC(hash_,
                (const unsigned char*)&norm_forms_count,
                sizeof(norm_forms_count));

        try
        {
          lang_ = El::Lang(lang_s.c_str());
        }
        catch(const El::Lang::InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
            "can't process language " << lang_s << ". Reason: " << e;

          throw InvalidArg(ostr.str());          
        }

        unsigned long i = 0;
        unsigned long count = sizeof(ID_BASES) / sizeof(ID_BASES[0]) - 1;
        
        for(; i < count && lang_.el_code() != ID_BASES[i].lang; i++);

        if(i == count)
        {
          std::ostringstream ostr;
          ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
            "unexpected language " << lang_;

          throw InvalidArg(ostr.str());
        }

        unsigned long id_base = ID_BASES[i].id_base;        
        unsigned long id_upper_boundry = ID_BASES[i + 1].id_base;        

        std::string line;
        std::getline(file, line);

        if(file.fail())
        {
          std::ostringstream ostr;
          ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
            "failed to read file " << dict_file;

          throw InvalidArg(ostr.str());          
        }

        norm_form_ids_.resize(norm_forms_count);

        unsigned long position = 0;
        El::ArrayPtr<char> buff(new char[buff_size]);
        unsigned long norm_form_offset = 0;
        
        try
        {
          for(unsigned long i = 0; i < records; i++)
          {
            if(!std::getline(file, line))
            {
              std::ostringstream ostr;
              ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
                "failed to read file " << dict_file << " after " << i
                   << " records";

              throw InvalidArg(ostr.str());            
            }

            std::string::size_type pos = line.find('\t');

            if(pos == std::string::npos)
            {
              std::ostringstream ostr;
              ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
                "word id not found in line '" << line << "'; file "
                   << dict_file;
              
              throw InvalidArg(ostr.str());    
            }

            std::string word = line.substr(0, pos);

            if(find(word.c_str()) != end())
            {
              std::ostringstream ostr;
              ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
                "duplicated word '" << word << "' in file " << dict_file
                   << " line num: " << i + 1;

              throw InvalidArg(ostr.str());            
            }
            
            unsigned long length = word.length();
            if(length + position >= buff_size)
            {
              std::ostringstream ostr;
              ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
                "buffer size " << buff_size << " is insufficient to read "
                "record " << i << " from " << records << "; file "
                   << dict_file;

              throw InvalidArg(ostr.str());
            }

            El::CRC(hash_, (const unsigned char*)word.c_str(), length);

            char* pword = buff.get() + position;
            
            strcpy(pword, word.c_str());
            position += length;
            buff[position++] = '\0';

            El::String::ListParser parser(line.c_str() + pos + 1, " \t");

            unsigned long word_nf_offset = norm_form_offset;
              
            const char* item = 0;
            while((item = parser.next_item()) != 0)
            {
              if(norm_form_offset == norm_form_ids_.size())
              {
                std::ostringstream ostr;
                ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load:"
                  " norm form count " << norm_form_ids_.size()
                     << " is smaller than actual number; file "
                     << dict_file;

                throw InvalidArg(ostr.str());
              }
              
              unsigned long word_id = 0;
              if(!El::String::Manip::numeric(item, word_id))
              {
                std::ostringstream ostr;
                ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load:"
                  " invalid word id in line '" << line << "'; file "
                     << dict_file;
              
                throw InvalidArg(ostr.str());
              }

              WordId id = id_base + word_id;
              
              if(id >= id_upper_boundry)
              {
                std::ostringstream ostr;
                ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load:"
                  " invalid id value in line '" << line << "'; file "
                     << dict_file;
              
                throw InvalidArg(ostr.str());
              }
                
              El::CRC(hash_, (const unsigned char*)&id, sizeof(id));
              norm_form_ids_[norm_form_offset++] = id;
            }

            unsigned long word_nf_count = norm_form_offset - word_nf_offset;
              
            if(word_nf_count == 0)
            {
              std::ostringstream ostr;
              ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
                "no word normal forms specified in line '" << line
                   << "'; file " << dict_file;
              
              throw InvalidArg(ostr.str());
            }

            WordNormalForms& wi = (*this)[pword];
            
            wi.normal_form_offset = word_nf_offset;
            wi.normal_form_count = word_nf_count;
          }
        }
        catch(...)
        {
          clear();
          lang_ = El::Lang::EC_NUL;
          
          throw;
        }
        
        buff_.reset(buff.release());

        if(stop_words_file == 0 || *stop_words_file == '\0')
        {
          return;
        }

        file.close();
        
        file.open(stop_words_file, std::ios::in);

        if(!file.is_open())
        {
          return;
        }

        while(std::getline(file, line))
        {
          std::string trimmed;
          El::String::Manip::trim(line.c_str(), trimmed);

          if(trimmed.empty() || trimmed[0] == '#')
          {
            continue;
          }

          std::string uniformed;
          El::String::Manip::utf8_to_uniform(trimmed.c_str(), uniformed);

          WordNormalFormsMap::const_iterator it = find(uniformed.c_str());
          
          if(it == end())
          {
            std::ostringstream ostr;
            
            ostr << "El::Dictionary::Morphology::WordNormalFormsMap::load: "
              "stop word '" << trimmed << "' of file " << stop_words_file
                 << " is not in dictionary";
              
            throw InvalidArg(ostr.str());
          }

          El::String::StringConstPtr word = it->first;
          
          const WordNormalForms& nf = it->second;
          unsigned long count = nf.normal_form_count;

          bool spare_word = true;
          
          for(unsigned long i = nf.normal_form_offset; count; count--, i++)
          {
            WordId id = norm_form_ids_[i];
            
            StopWordsMap::const_iterator sit = stop_words_.find(id);

            if(sit != stop_words_.end())
            {
              continue;
            }

            El::CRC(hash_, (const unsigned char*)&id, sizeof(id));
            
            stop_words_[id] = word;
            spare_word = false;
          }

          if(warnings_stream && spare_word)
          {
            *warnings_stream << "Spare " << lang_.l3_code() << " stop word '"
                             << uniformed << "'. Covered by";

            unsigned long count = nf.normal_form_count;
            for(unsigned long i = nf.normal_form_offset; count; count--, i++)
            {
              WordId id = norm_form_ids_[i];
            
              StopWordsMap::const_iterator sit = stop_words_.find(id);
              assert(sit != stop_words_.end());

              *warnings_stream << " '" << sit->second << "' (" << id << ")";
            }

            *warnings_stream << std::endl;
          }
          
        }
      }
      
      //
      // LemmaMap class
      //
      void
      LemmaMap::load(const char* dict_file,
                     const WordNormalFormsMap& norm_form_map)
        throw(InvalidArg, Exception, El::Exception)
      {
        clear();
        word_forms_.clear();
        
        std::fstream file(dict_file, std::ios::in);

        if(!file.is_open())
        {
          std::ostringstream ostr;
          ostr << "El::Dictionary::Morphology::LemmaMap::load: "
            "failed to open file " << dict_file;

          throw InvalidArg(ostr.str());          
        }

        std::string lang_s;
        unsigned long records = 0;
        unsigned long word_forms_count = 0;

        file >> lang_s >> records >> word_forms_count;

        try
        {
          lang_ = El::Lang(lang_s.c_str());
        }
        catch(const El::Lang::InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::Dictionary::Morphology::LemmaMap::load: "
            "can't process language " << lang_s << ". Reason: " << e;

          throw InvalidArg(ostr.str());          
        }

        unsigned long i = 0;
        unsigned long count = sizeof(ID_BASES) / sizeof(ID_BASES[0]) - 1;
        
        for(; i < count && lang_.el_code() != ID_BASES[i].lang; i++);

        if(i == count)
        {
          std::ostringstream ostr;
          ostr << "El::Dictionary::Morphology::LemmaMap::load: "
            "unexpected language " << lang_;

          throw InvalidArg(ostr.str());
        }

        unsigned long id_base = ID_BASES[i].id_base;        
        unsigned long id_upper_boundry = ID_BASES[i + 1].id_base;        

        std::string line;
        std::getline(file, line);

        if(file.fail())
        {
          std::ostringstream ostr;
          ostr << "El::Dictionary::Morphology::LemmaMap::load: "
            "failed to read file " << dict_file;

          throw InvalidArg(ostr.str());          
        }

        word_forms_.resize(word_forms_count);

        unsigned long word_form_offset = 0;
        
        try
        {
          for(unsigned long i = 0; i < records; i++)
          {
            if(!std::getline(file, line))
            {
              std::ostringstream ostr;
              ostr << "El::Dictionary::Morphology::LemmaMap::load: "
                "failed to read file " << dict_file << " after " << i
                   << " records";

              throw InvalidArg(ostr.str());
            }

            El::String::ListParser parser(line.c_str(), " \t");
            
            const char* item = 0;
            Lemma* lemma = 0;
            
            while((item = parser.next_item()) != 0)
            {
              const char* wid = strrchr(item, ':');

              if(wid == 0)
              {
                std::ostringstream ostr;
                ostr << "El::Dictionary::Morphology::LemmaMap::load:"
                  " invalid word form description '" << item << "' in line '"
                     << line << "'; file " << dict_file;
              
                throw InvalidArg(ostr.str());
              }

              unsigned long word_id = 0;
              unsigned long id = 0;
              
              if(!El::String::Manip::numeric(wid + 1, word_id) ||
                 (id = (id_base + word_id)) >= id_upper_boundry)
              {
                std::ostringstream ostr;
                ostr << "El::Dictionary::Morphology::LemmaMap::load:"
                  " invalid id in line '" << line << "'; file "
                     << dict_file;
              
                throw InvalidArg(ostr.str());
              }

              std::string w(item, wid - item);
              WordNormalFormsMap::const_iterator it = norm_form_map.find(w);
              
              if(it == norm_form_map.end())
              {
                std::ostringstream ostr;
                ostr << "El::Dictionary::Morphology::LemmaMap::load:"
                  "can't find norm forms for word '" << w 
                     << "'; line '" << line << "'; file "
                     << dict_file;
                
                throw InvalidArg(ostr.str());
              }

              El::String::StringConstPtr word(it->first);

              if(lemma)
              {
                if(word_form_offset == word_forms_.size())
                {
                  std::ostringstream ostr;
                  ostr << "El::Dictionary::Morphology::LemmaMap::load:"
                    " word form count " << word_forms_.size()
                       << " is smaller than actual number; file "
                       << dict_file;
                  
                  throw InvalidArg(ostr.str());
                }

                Lemma::Word& lemma_word = word_forms_[word_form_offset++];
                
                lemma_word.id = id;
                lemma_word.text = word;
                  
                lemma->word_form_count++;
              }
              else
              {
                lemma = &(*this)[id];
                lemma->text = word;
                lemma->word_form_offset = word_form_offset;
              }
            }
          }
        }
        catch(...)
        {
          clear();
          word_forms_.clear();
          
          throw;
        }
      }

      long
      LemmaMap::match_length(const wchar_t* word1,
                             long word1_len,
                             const wchar_t* word2,
                             long word2_len) throw()
      {
        long i = 0;
        long len = std::min(word1_len, word2_len);
        
        const wchar_t* word1_end = word1 + word1_len - 1;
        const wchar_t* word2_end = word2 + word2_len - 1;
        
        for(; i < len && *(word1_end - i) == *(word2_end - i); i++);
        return i;
      }
      
      bool
      LemmaMap::guess_lemma(const char* word,
                            Lemma::GuessStrategy strategy,
                            LemmaInfoArray& lemmas) const
        throw(Exception, El::Exception)
      {
        if(strategy == Lemma::GS_NONE || strategy == Lemma::GS_COUNT)
        {
          return false;
        }

        std::wstring wword;
        El::String::Manip::utf8_to_wchar(word, wword);
        
        long word_len = wword.length();

        if(!word_len)
        {
          return false;
        }        

        WordIdSet ids;
        long max_match = 0;
        
        for(LemmaMap::const_iterator it = begin(); it != end(); it++)
        {
          const Lemma& lemma = it->second;
          
          std::wstring lemma_text;
          El::String::Manip::utf8_to_wchar(lemma.text.c_str(), lemma_text);

          long match = match_length(wword.c_str(),
                                    word_len,
                                    lemma_text.c_str(),
                                    lemma_text.length());

          if(match < 2)
          {
            continue;
          }
          
          if(match > max_match)
          {
            max_match = match;
            ids.clear();
          }

          if(match == max_match)
          {
            ids.insert(it->first);
          }
        }

        if(ids.empty())
        {
          return false;
        }

        long min_diff = LONG_MAX;

        for(WordIdSet::const_iterator it = ids.begin(); it != ids.end(); it++)
        {
          LemmaMap::const_iterator lit = find(*it);
          assert(lit != end());

          const Lemma& lemma = lit->second;

          std::wstring lemma_text;
          El::String::Manip::utf8_to_wchar(lemma.text.c_str(), lemma_text);

          min_diff = std::min(min_diff,
                              std::abs(word_len - (long)lemma_text.length()));
        }
        
        WordId word_pesudo_id = pseudo_id(word);
        LemmasCrc lemmas_crc;
        
        for(WordIdSet::const_iterator it = ids.begin(); it != ids.end(); it++)
        {
          LemmaMap::const_iterator lit = find(*it);
          assert(lit != end());

          const Lemma& lemma = lit->second;

          std::wstring lemma_text;
          El::String::Manip::utf8_to_wchar(lemma.text.c_str(), lemma_text);
          
          long lemma_len = lemma_text.length();

          if(std::abs(word_len - (long)lemma_len) > min_diff)
          {
            continue;
          }

          LemmaInfo lemma_info;

          lemma_info.known = false;
          lemma_info.lang = lang_;
          
          LemmaInfo::WordArray& word_forms = lemma_info.word_forms;
          word_forms.resize(lemma.word_form_count);
            
          if(strategy == Lemma::GS_SIMILAR)
          {
            lemma_info.norm_form.id = *it;
            lemma_info.norm_form.text = lemma.text.c_str();

            for(unsigned long i = 0; i < lemma.word_form_count; i++)
            {
              const Lemma::Word& wfs = word_forms_[lemma.word_form_offset + i];
              LemmaInfo::Word& wfd = word_forms[i];
              
              wfd.id = wfs.id;
              wfd.text = wfs.text.c_str();
            }

          }
          else
          {
            long match =
              match_length(wword.c_str(),
                           word_len,
                           lemma_text.c_str(),
                           lemma_len);

            unsigned long lemma_prefix = lemma_len - match;
            
            lemma_info.norm_form.id = word_pesudo_id;
            lemma_info.norm_form.text = word;

            typedef std::vector<std::wstring> StringArray;
            StringArray guessed_forms;
            
            unsigned long i = 0;
            for(; i < lemma.word_form_count; i++)
            {
              const Lemma::Word& wfs =
                word_forms_[lemma.word_form_offset + i];
              
              std::wstring wfs_text;
              El::String::Manip::utf8_to_wchar(wfs.text.c_str(), wfs_text);

              if(wfs_text.length() < lemma_prefix)
              {
                break;
              }
              
              std::wstring guessed_form =
                std::wstring(wword.c_str(), word_len - match) +
                std::wstring(wfs_text.c_str() + lemma_prefix);

              guessed_forms.push_back(guessed_form);
            }

            if(i < lemma.word_form_count)
            {
              continue;
            }

            std::sort(guessed_forms.begin(), guessed_forms.end());

            unsigned long long lemma_crc = 0;
            
            for(i = 0; i < lemma.word_form_count; i++)
            {
              std::string guessed_form;
              
              El::String::Manip::wchar_to_utf8(guessed_forms[i].c_str(),
                                               guessed_form);
              
              LemmaInfo::Word& wfd = word_forms[i];

              wfd.id = pseudo_id(guessed_form.c_str());
              wfd.text = guessed_form;

              El::CRC(lemma_crc,
                      (const unsigned char*)wfd.text.c_str(),
                      wfd.text.length());

              El::CRC(lemma_crc, (const unsigned char*)"\n", 1);
            }

            if(lemmas_crc.find(lemma_crc) != lemmas_crc.end())
            {
              continue;
            }

            lemmas_crc.insert(lemma_crc);
          }

          lemmas.push_back(lemma_info);
        }
        
        return !lemmas.empty();
      }
      
      bool
      LemmaMap::get_lemma(WordId id, LemmaInfoArray& lemmas)
        const throw(Exception, El::Exception)
      {
        LemmaMap::const_iterator it = find(id);

        if(it == end())
        {
          return false;
        }

        const Lemma& lemma = it->second;

        lemmas.push_back(LemmaInfo());
        LemmaInfo& lemma_info = *lemmas.rbegin();

        lemma_info.lang = lang_;
        lemma_info.norm_form.id = id;
        lemma_info.norm_form.text = lemma.text.c_str();

        LemmaInfo::WordArray& word_forms = lemma_info.word_forms;

        word_forms.resize(lemma.word_form_count);

        for(unsigned long i = 0; i < lemma.word_form_count; i++)
        {
          const Lemma::Word& wfs = word_forms_[lemma.word_form_offset + i];
          LemmaInfo::Word& wfd = word_forms[i];
          
          wfd.id = wfs.id;
          wfd.text = wfs.text.c_str();
        }

        return true;
      }
      
      //
      // WordInfoManager class
      //
      void
      WordInfoManager::load(const char* dict_file,
                            std::ostream* warnings_stream)
        throw(InvalidArg, Exception, El::Exception)
      {
        std::string df = dict_file;
        
        WordNormalFormsMap_var words = new WordNormalFormsMap();
        
        words->load(std::string(df + ".nrm").c_str(),
                    std::string(df + ".stp").c_str(),
                    warnings_stream);

        uint32_t hash = words->hash();
        El::CRC(hash_, (const unsigned char*)&hash, sizeof(hash));
        
        LemmaMap_var lemmas = new LemmaMap();
        lemmas->load(std::string(df + ".mrf").c_str(), *words);

        El::Lang lang = words->lang();
        
        word_normal_forms_[lang] = words;
        
        popularity_word_normal_forms_[LangDetection::popularity_index(lang)] =
          words.retn();
          
        lemmas_[lang] = lemmas.retn();
      }

      const char*
      WordInfoManager::get_normal_form(
        const char* word,
        const El::Lang& lang,
        bool& is_stop_word) const
        throw(El::Exception)
      {
        LangWordNormalFormsMap::const_iterator it =
          word_normal_forms_.find(lang);
        
        if(it == word_normal_forms_.end())
        {
          return 0;
        }

        const WordNormalFormsMap& forms_map = *(it->second);

        WordFormArray forms;
        forms_map.get_normal_forms(word, forms);
        
        if(forms.empty())
        {
          return 0;
        }

        //take first
        WordId word_id = forms[0].id;
        is_stop_word = forms[0].is_stop_word;

        LangLemmaMap::const_iterator lemmas_it = lemmas_.find(lang);
        assert(lemmas_it != lemmas_.end());

        const LemmaMap& lemmas = *(lemmas_it->second);
        LemmaMap::const_iterator lemma_it = lemmas.find(word_id);
        assert(lemma_it != lemmas.end());
        
        return lemma_it->second.text.c_str();
      }      

      void
      WordInfoManager::get_lemmas(const WordArray& words,
                                  const El::Lang* lang,
                                  Lemma::GuessStrategy guess_strategy,
                                  LemmaInfoArrayArray& lemmas) const
        throw(El::Exception)
      {
        unsigned long words_count = words.size();
        
        lemmas.clear();
        lemmas.resize(words_count);

        WordInfoArray word_infos;
        
        word_infos.clear();
        word_infos.resize(words.size());

        typedef std::vector<std::string> StringArray;

        StringArray lowered_words;
        lowered_words.resize(words_count);

        for(unsigned long i = 0; i < words_count; i++)
        {
          El::String::Manip::utf8_to_uniform(words[i].c_str(),
                                             lowered_words[i]);
        }

        for(PopularityWordNormalFormsMap::const_iterator
              it = popularity_word_normal_forms_.begin();
            it != popularity_word_normal_forms_.end(); it++)
        {
          WordNormalFormsMap* wimap = it->second;

          if(lang && wimap->lang() != *lang)
          {
            continue;
          }

          for(unsigned long i = 0; i < words_count; i++)
          {
            const char* word = lowered_words[i].c_str();
            
            wimap->get_normal_forms(word, word_infos[i].forms);
            word_infos[i].lang = wimap->lang();
          } 
        }

        for(unsigned long i = 0; i < words_count; i++)
        {
          LemmaInfoArray& word_lemmas = lemmas[i];
          const WordFormArray& word_forms = word_infos[i].forms;

          for(WordFormArray::const_iterator it = word_forms.begin();
              it != word_forms.end(); it++)
          {
            const WordForm& wf = *it;

            LangLemmaMap::const_iterator lit = lemmas_.find(wf.lang);

            assert(lit != lemmas_.end());
            lit->second->get_lemma(wf.id, word_lemmas);
          }

          if(word_lemmas.empty())
          {
            const char* word = lowered_words[i].c_str();
            
            El::Lang guessed_lang =
              El::Dictionary::LangDetection::language(
                word,
                lang ? *lang : El::Lang::null);

            bool lemma_guessed = false;
            
            if(guess_strategy != Lemma::GS_NONE)
            {
              El::Lang guess_lang;

              if(guessed_lang == El::Lang::null)
              {
                std::wstring wword;
                El::String::Manip::utf8_to_wchar(word, wword);

                std::string last_char;
                
                El::String::Manip::wchar_to_utf8(
                  wword.c_str() + wword.length() - 1,
                  last_char);
                
                guess_lang = El::Dictionary::LangDetection::language(
                  last_char.c_str(),
                  lang ? *lang : El::Lang::null);
              }
              else
              {
                guess_lang = guessed_lang;
              }
                
              LangLemmaMap::const_iterator lit = lemmas_.find(guess_lang);
              
              if(lit != lemmas_.end())
              {
                lemma_guessed =
                  lit->second->guess_lemma(word, guess_strategy, word_lemmas);

                if(lemma_guessed)
                {
                  guessed_lang = guess_lang;
                }
              }
            }
            
            if(!lemma_guessed)
            {
              word_lemmas.push_back(LemmaInfo());
              LemmaInfo& lemma_info = *word_lemmas.rbegin();
              
              lemma_info.known = false;
              
              lemma_info.norm_form.id = pseudo_id(word);
              lemma_info.norm_form.text = word;

              lemma_info.lang = guessed_lang;
            }
/*
            LemmaInfo::Word w;
            w.text = "ABC";
            
            lemma_info.word_forms.push_back(w);
*/
          }
        }

        
      }
        
      void
      WordInfoManager::normal_form_ids(const WordArray& words,
                                       WordInfoArray& word_infos,
                                       El::Lang* lang,
                                       bool narrow_by_containment) const
        throw(El::Exception)
      {
#ifdef TRACE_LANG_DETECT
        std::cerr << "*************************\nnormal_form_ids "
                  << (lang ? lang->l3_code() : "null")
                  << ":\n------------------------\n";
#endif
        word_infos.clear();
        word_infos.resize(words.size());

        LangRatesPtr lang_rates;
        LangRatesPtr guessed_lang_rates;
        LangSetPtr word_langs;

        if(lang)
        {
          lang_rates.reset(new LangRates());
          guessed_lang_rates.reset(new LangRates());
          word_langs.reset(new LangSet(word_normal_forms_.size()));
        }

        unsigned long recornized_words = 0;
        unsigned long unrecornized_words = 0;
        unsigned long guessed_words = 0;

        for(unsigned long i = 0; i < words.size(); i++)
        {
          const char* word = words[i].c_str();

#ifdef TRACE_LANG_DETECT
          std::cerr << word;
#endif
          if(word_langs.get() != 0)
          {
            word_langs->clear();
          }
          
          for(LangWordNormalFormsMap::const_iterator
                it = word_normal_forms_.begin();
              it != word_normal_forms_.end(); it++)
          {
            WordNormalFormsMap* wimap = it->second;

            if(wimap->get_normal_forms(word, word_infos[i].forms) && lang)
            {
              word_langs->insert(it->first);
            } 
          }

          if(lang)
          {
#ifdef TRACE_LANG_DETECT
            std::cerr << " - ";
#endif
            if(word_langs->empty())
            {
              // Unrecognized word

              std::wstring wword;
              El::String::Manip::utf8_to_wchar(word, wword);

              if(token_type(wword.c_str()) == TT_WORD)
              {                
#ifdef TRACE_LANG_DETECT
                std::cerr << " UNKNOWN";
#endif
                El::Dictionary::LangDetection::LangArray langs;
                El::Dictionary::LangDetection::languages(wword.c_str(), langs);

                if(langs.empty())
                {
                  unrecornized_words++;
                }
                else
                {
                  guessed_words++;

                  for(El::Dictionary::LangDetection::LangArray::const_iterator
                        it = langs.begin(); it != langs.end(); ++it)
                  {
                    add_rate(*guessed_lang_rates, *it);
                    
#ifdef TRACE_LANG_DETECT
                    std::cerr << " " << it->l3_code();
#endif
                    
                  }
                }
              }
              else
              {
#ifdef TRACE_LANG_DETECT
                std::cerr << " NOTWORD";
#endif
              }
            }
            else
            {
              // Word belongs to one or several supported language

              recornized_words++;
                
              for(LangSet::iterator it = word_langs->begin();
                  it != word_langs->end(); it++)
              {
                add_rate(*lang_rates, *it);

#ifdef TRACE_LANG_DETECT
                std::cerr << " " << it->l3_code();
#endif
              }
            }

          }
          
#ifdef TRACE_LANG_DETECT
          std::cerr << std::endl;
#endif
        }

        if(lang)
        {
          adjust_lang(*lang,
                      recornized_words,
                      unrecornized_words,
                      guessed_words,
                      *lang_rates,
                      *guessed_lang_rates,
                      default_lang_validation_level_);
        
          for(WordInfoArray::iterator it(word_infos.begin()),
                ie(word_infos.end()); it != ie; ++it)
          {
            WordFormArray& word_forms = it->forms;
            WordFormArray::iterator wit = word_forms.begin();
            
            for(; wit != word_forms.end() && wit->lang != *lang; wit++);

            if(wit != word_forms.end())
            {
              //
              // Word forms for the document language exist,
              // so leave just these forms
              //
              for(wit = word_forms.begin(); wit != word_forms.end(); )
              {
                if(wit->lang == *lang)
                {
                  wit++;
                }
                else
                {
                  wit = word_forms.erase(wit);
                }
              }
            }
          }
        }

        if(narrow_by_containment)
        {
          for(WordInfoArray::iterator i(word_infos.begin()),
                e(word_infos.end()); i != e; ++i)
          {
            WordInfo& wi = *i;
            
            for(WordInfoArray::iterator j(word_infos.begin()); j != e; ++j)
            {
              WordInfo& wj = *j;
              
              // If word forms contain another word forms
              // then consider both words as a same narrowest word
              
              if(wi.contained(wj))
              {
                wj = wi;
              }
            }
          }
        }
        
        for(unsigned long i = 0; i < words.size(); i++)
        {
          WordInfo& wi = word_infos[i];
          WordFormArray& word_forms = wi.forms;

          El::Lang word_lang;
          unsigned long min_index = ULONG_MAX;

          for(WordFormArray::iterator wit = word_forms.begin();
              wit != word_forms.end(); wit++)
          {
            El::Lang lang = wit->lang;
            unsigned long index = LangDetection::popularity_index(lang);
            
            if(index < min_index)
            {
              min_index = index;
              word_lang = lang;
            }
          }
          
          if(min_index < ULONG_MAX)
          {
            wi.lang = word_lang;
            continue;
          }
          
/*          
          WordFormArray::iterator wit = word_forms.begin();

          if(wit != word_forms.end())
          {
            El::Lang lang = wit->lang;
            for(wit++; wit != word_forms.end() && wit->lang == lang; wit++);

            if(wit == word_forms.end())
            {
              wi.lang = lang;
              continue;
            }
          }
*/
/*
          std::cerr << "WORD :" << words[i].c_str() << " " << word_forms.size()
                    << " " << (lang ? lang->l3_code() : "null") << std::endl;
          
          std::cerr.flush();
*/          
          wi.lang = El::Dictionary::LangDetection::language(
            words[i].c_str(), lang ? *lang : El::Lang::null);
/*
          std::cerr << "LANG :" << wi.lang.l3_code() << std::endl;
          std::cerr.flush();
*/        
        }
     
      }

      void
      WordInfoManager::adjust_lang(El::Lang& lang,
                                   unsigned long recornized_words,
                                   unsigned long unrecornized_words,
                                   unsigned long guessed_words,
                                   const LangRates& lang_rates,
                                   const LangRates& guessed_lang_rates,
                                   size_t default_lang_validation_level)
        const throw(El::Exception)
      {
        unsigned long total_words =
          recornized_words + guessed_words + unrecornized_words;

        if(!total_words)
        {
#ifdef TRACE_LANG_DETECT
          std::cerr << "No words; default " << lang.l3_code() << " left\n";
#endif
          return;
        }

#ifdef TRACE_LANG_DETECT
        std::cerr << "------------------------\ndef " << lang.l3_code()
                  << ", total " << total_words
                  << ", rec " << recornized_words << "/"
                  << recornized_words * 100 / total_words << "%, guess "
                  << guessed_words << "/"
                  << guessed_words * 100 / total_words << "%, unrec "
                  << unrecornized_words << "/"
                  << unrecornized_words * 100 / total_words << "%\n";
#endif

        unsigned long unknown_words = guessed_words + unrecornized_words;
        unsigned long count = sizeof(ID_BASES) / sizeof(ID_BASES[0]) - 1;

        LangWeightArray lang_weights;
        lang_weights.reserve(count);

        for(unsigned long i = 0; i < count; i++)
        {
          El::Lang curr_lang = ID_BASES[i].lang;
          LangRates::const_iterator it = lang_rates.find(curr_lang);

          if(it != lang_rates.end())
          {
            unsigned long prc = it->second * 100 / total_words;
            
            if(lang == curr_lang && prc >= default_lang_validation_level)
            {

#ifdef TRACE_LANG_DETECT
              std::cerr << "Def " << lang.l3_code() << " wins ("
                        << it->second << " of " << total_words << " = "
                        << prc << "% >= " << default_lang_validation_level
                        << "%)\n";
#endif
              return;
            }
                
            lang_weights.push_back(
              LangWeight(curr_lang,
                         it->second,
                         LangDetection::popularity_index(curr_lang),
                         curr_lang == lang));
          }
        }

        unsigned long unrecognized_prc = 0;
        unsigned long max_weight_prc = 0;
        LangWeight max_weight;
        
        if(!lang_weights.empty())
        {
          std::sort(lang_weights.begin(), lang_weights.end());

#ifdef TRACE_LANG_DETECT
          
          std::cerr << "Weights:";

          for(LangWeightArray::const_iterator it = lang_weights.begin();
              it != lang_weights.end(); it++)
          {            
            std::cerr << " " << it->lang.l3_code() << "/" << it->weight
                      << "/" << it->weight * 100 / total_words << "%/"
                      << it->default_lang << "/" << it->index;
          }

          std::cerr << std::endl;
#endif
          
          max_weight = *lang_weights.begin();

          bool default_language_supported =
            word_normal_forms_.find(lang) != word_normal_forms_.end();

          if(!default_language_supported && lang != El::Lang::null &&
             unknown_words > max_weight.weight)
          {
            // Just leave default language
           
#ifdef TRACE_LANG_DETECT
            
            std::cerr << "Def unrecognized " << lang.l3_code() << " wins ("
                      << unknown_words << ">" <<  max_weight.weight
                      << ")\n";
#endif
            return;
          }

          max_weight_prc = max_weight.weight * 100 / total_words;          
          unrecognized_prc = unknown_words * 100 / total_words;
        }

        LangRates try_futher_rates;
        
        if(El::Dictionary::LangDetection::supported(lang) &&
           !guessed_lang_rates.empty())
        {
          try_futher_rates = lang_rates;
          try_futher_rates.insert(std::make_pair(lang, 0));

          bool try_futher = false;
            
          for(LangRates::iterator it = try_futher_rates.begin();
              it != try_futher_rates.end(); ++it)
          {
            LangRates::const_iterator git =
              guessed_lang_rates.find(it->first);
              
            if(git != guessed_lang_rates.end())
            {
              it->second += git->second;
              try_futher = true;
            }
          }

          if(!try_futher)
          {
            try_futher_rates.clear();
          }
        }
        
        if(!try_futher_rates.empty())
        {
#ifdef TRACE_LANG_DETECT
          std::cerr << "Will guess futher ...\n";
#endif
          adjust_lang(lang,
                      recornized_words + guessed_words,
                      unrecornized_words,
                      0,
                      try_futher_rates,
                      LangRates(),
                      guessing_default_lang_validation_level_);
          return;
        }

        if(max_weight_prc >= lang_validation_level_)
        {
          lang = max_weight.lang;
          
#ifdef TRACE_LANG_DETECT
          
          std::cerr << lang.l3_code() << " wins ("
                    << max_weight_prc << ">=" << lang_validation_level_
                    << "%)\n";      
#endif
          return;
        }

        if(unrecognized_prc <
           std::min(lang_validation_level_, (size_t)max_weight_prc))
        {
          lang = max_weight.lang;
          
#ifdef TRACE_LANG_DETECT
          
          std::cerr << "Unrec loose to " << lang.l3_code() << " ("
                      << unrecognized_prc << "<min(" << lang_validation_level_
                    << "%, " << max_weight_prc <<"%/"
                    << max_weight.weight << ")\n";
          
#endif
          return;
        }  
        
#ifdef TRACE_LANG_DETECT
        std::cerr << "Can't decide; default " << lang.l3_code() << " left\n";
#endif
      }      
      
      TokenType
      token_type(const wchar_t* token) throw()
      {
        if(token == 0)
        {
          return TT_UNDEFINED;
        }

        enum DetectionStatus
        {
          DS_YES,
          DS_NO,
          DS_MAYBE
        };

        DetectionStatus word = DS_MAYBE;
        DetectionStatus number = DS_MAYBE;
        DetectionStatus surrogate = DS_MAYBE;

        for(const wchar_t* ptr = token; *ptr != L'\0'; ptr++)
        {
          unsigned long cat =
            El::String::Unicode::CharTable::el_categories(*ptr);

//          std::cerr << std::hex << cat << " ";

          if(cat & El::String::Unicode::EC_LETTER)
          {
            if(word != DS_NO)
            {
              word = DS_YES;
            }
          }
          else if((cat & El::String::Unicode::EC_INTERWORD) == 0)
          {
            word = DS_NO;
          }
          
          if(cat & El::String::Unicode::EC_NUMBER)
          {
            if(number != DS_NO)
            {
              number = DS_YES;
            }
          }
          else if((cat & El::String::Unicode::EC_INTERNUMBER) == 0)
          {
            number = DS_NO;
          }

          if(cat & (El::String::Unicode::EC_LETTER |
                    El::String::Unicode::EC_NUMBER))
          {
            surrogate = DS_YES;  
          }
          
          if(surrogate == DS_NO && word == DS_NO && number == DS_NO)
          {
            return TT_UNDEFINED;
          }
        }

        if(number == DS_YES)
        {
          return TT_NUMBER;
        }
        
        if(word == DS_YES)
        {
          return TT_WORD;
        }

        if(surrogate == DS_YES)
        {
          return TT_SURROGATE;
        }

        return TT_UNDEFINED;
      }
/*
      TokenType
      token_type(const wchar_t* token) throw()
      {
        TokenType tt = token_type2(token);
        
        std::string tk;
        El::String::Manip::wchar_to_utf8(token, tk);

        std::cerr << tk << " : " << tt << std::endl;
        return tt;
      }
*/
    }
  }
}
