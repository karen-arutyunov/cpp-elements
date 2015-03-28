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
 * @file   Elements/Tools/Dict/AOT/Converter/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <locale.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include <ace/OS.h>

#include <El/String/Manip.hpp>

#include "Application.hpp"

struct LangMap
{
  const char* lang;
  const char* lang_aot;
};

namespace
{
  const char USAGE[] =
    "\nUsage:\nElAOTConverter <command> <command arguments>\n\n"
    "Synopsis 1:\nElAOTConverter help\n\n" 
    "Synopsis 2:\nElAOTConverter raw lang=(eng|rus|ger)\n"
    "Dumps lemma raw set for language specified, "
    "writes result to stdout\n\n"
    "Synopsis 3:\nElAOTConverter convert lang=(eng|rus|ger)\n"
    "Convert dictionary for language specified, "
    "writes result to stdout\n\n"
    "Synopsis 4:\nElAOTConverter dump lang=(eng|rus|ger) [word=<word>]\n"
    "Dumps dictionary for language specified, "
    "writes result to stdout\n\n"
    "Note: ensure RML environment variable points to directory parent "
    "for Dicts\n";

  const LangMap LANG_MAP[] =
  {
    { "eng", "English" },
    { "rus", "Russian" },
    { "ger", "German" }
  };

  const unsigned long LEMMA_MAX_COUNT = 1000000;
}

int
main(int argc, char** argv)
{
  try
  {
    srand(time(NULL));
    
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << e << "\nRun 'ElAOTConverter help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << e << std::endl;
  }
  catch(...)
  {
    std::cerr << "ElAOTConverter: unknown exception caught\n";
  }
  
  return -1;
}

Application::Application() throw(Application::Exception, El::Exception)
{
  if (!::setlocale(LC_CTYPE, "en_US.utf8"))
  {
    throw Exception("Application::Application: cannot set en_US.utf8 locale");
  }
}

Application::~Application() throw()
{
}

int
Application::run(int& argc, char** argv)
  throw(InvalidArg, Exception, El::Exception)
{
  if(argc < 2)
  {
    throw InvalidArg("Too few arguments");
  }

  int i = 1;  
  std::string command = argv[i];

  ArgList arguments;

  for(i++; i < argc; i++)
  {
    char* argument = argv[i];
    
    Argument arg;
    const char* eq = strstr(argument, "=");

    if(eq == 0)
    {
      arg.name = argument;
    }
    else
    {
      arg.name.assign(argument, eq - argument);
      arg.value = eq + 1;
    }

    arguments.push_back(arg);
  }

  if(command == "help")
  {
    return help(arguments);
  }
  else if(command == "raw")
  {
    return raw(arguments);
  }
  else if(command == "convert")
  {
    return convert(arguments);
  }
  else if(command == "dump")
  {
    return dump(arguments);
  }
  else
  {
    std::ostringstream ostr;
    ostr << "unknown command '" << command << "'";
   
    throw InvalidArg(ostr.str());
  }

  return 0;
}

int
Application::help(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << USAGE;
  return 0;
}

void
Application::init_lemmatizer(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  try
  {
    MorphLanguageEnum language = morphUnknown;
    
    for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
        it++)
    {
      const std::string& name = it->name;
      const std::string& value = it->value;

      if(name == "lang")
      {
        unsigned long count = sizeof(LANG_MAP) / sizeof(LANG_MAP[0]);
        unsigned long i = 0;
        
        for(; i < count && value != LANG_MAP[i].lang; i++);

        if(i == count)
        {
          std::ostringstream ostr;
          ostr << "unexpected language " << value;          
          throw InvalidArg(ostr.str());
        }
        
        if(!GetLanguageByString(LANG_MAP[i].lang_aot, language))
        {
          language = morphUnknown;
        }

        switch(language)
        {
        case morphRussian: 
          lemmatizer_.reset(new CLemmatizerRussian());
          lang_ = El::Lang(El::Lang::EC_RUS);
          break;
        case morphEnglish : 
          lemmatizer_.reset(new CLemmatizerEnglish());
          lang_ = El::Lang(El::Lang::EC_ENG);
          break;
        case morphGerman: 
          lemmatizer_.reset(new CLemmatizerGerman());
          lang_ = El::Lang(El::Lang::EC_GER);
          break;
        default:
          std::ostringstream ostr;
          ostr << "unrecognized lang argument value << '" << value << "'";
        
          throw InvalidArg(ostr.str());
        }  
      }
    }
  
    if(lemmatizer_.get() == 0)
    {
      throw InvalidArg("lang argument is not provided");
    }

    string error;
    if(!lemmatizer_->LoadDictionariesRegistry(error))
    {
      std::ostringstream ostr;
      ostr << "can't load morphological dictionary; reason: " << error;
      throw InvalidArg(ostr.str());
    }
  }
  catch(const CExpc& e)
  {
    std::ostringstream ostr;
    ostr << "Application::init_lemmatizer: CExpc caught. Description: "
         << e.m_strCause;

    throw Exception(ostr.str());
  }
}

int
Application::raw(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  init_lemmatizer(arguments);

//  Converter converter(lemmatizer_.get(), lang_);

  for(unsigned long i = 0; i < LEMMA_MAX_COUNT; i++)
  {
    CFormInfo res;
    if(lemmatizer_->CreateParadigmFromID(i, res))
    {
      if(res.GetCount() == 0)
      {
        continue;
      }

      typedef std::vector<std::wstring> WordVector;

      WordVector words;
      words.reserve(res.GetCount());

      for(unsigned long i = 0; i < res.GetCount(); i++)
      {
        std::wstring word = normalize(res.GetWordForm(i).c_str(), lang_);

        if(!word.empty())
        {
          words.push_back(word);
        }
      }

      if(!words.empty())
      {
        for(WordVector::const_iterator it = words.begin(); it != words.end();
            it++)
        {
          if(it != words.begin())
          {
            std::cout << " ";
          }
          
          El::String::Manip::wchar_to_utf8(it->c_str(), std::cout);          
        }

        std::cout << std::endl;
      }
    }
  }
  
  return 0;
}

int
Application::convert(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  init_lemmatizer(arguments);

  Converter converter(lemmatizer_.get(), lang_);
  converter.dump_norm_form_map(std::cout, std::cerr);
  
  return 0;
}

int
Application::dump(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  init_lemmatizer(arguments);

  std::string word;
  
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    const std::string& name = it->name;
    const std::string& value = it->value;
    
    if(name == "word")
    {
      word = value;
    }
  }    

  Converter converter(lemmatizer_.get(), lang_);

  if(word.empty())
  {
    converter.dump_lemmas(std::cout);
  }
  else
  {
    converter.dump_word_lemmas(std::cout, word.c_str());
  }    

  return 0;
}

//
// Application::Converter class
//

Application::Converter::Converter(CLemmatizer* lemmatizer, El::Lang lang)
  throw(Exception, El::Exception)
    : lang_(lang)
{
  try
  {
    WordId word_id = 1;
    NormFormHashMap norm_form_map;
    unsigned long duplicated_lemmas = 0;
    
    for(unsigned long i = 0; i < LEMMA_MAX_COUNT; i++)
    {
      CFormInfo res;
      if(lemmatizer->CreateParadigmFromID(i, res))
      {
        if(res.GetCount() == 0)
        {
          continue;
        }

        WordId id = word_id;
        WordId lemma_id = id;
        
        Lemma lemma;
        lemma.normal_form.text =
          Application::normalize(res.GetWordForm(0).c_str(), lang_);
        
        lemma.normal_form.id = id;

        if(lemma.normal_form.text.empty())
        {
          continue;
        }

        WordArray& word_forms = lemma.word_forms;
          
        for(unsigned long i = 0; i < res.GetCount(); i++)
        {
          Word form;
            
          form.text =
            Application::normalize(res.GetWordForm(i).c_str(), lang_);
          
          form.id = id;

          if(form.text.empty())
          {
            continue;
          }

          WordArray::iterator it = word_forms.begin();
          for(; it != word_forms.end(); it++)
          {
            if(it->text >= form.text)
            {
              break;
            }
          }

          if(it != word_forms.end() && it->text == form.text)
          {
            continue;
          }

          word_forms.insert(it, form);
          id++;
        }

        bool add_lemma = true;
        
        NormFormHashMap::iterator it =
          norm_form_map.find(lemma.normal_form.text);

        if(it != norm_form_map.end())
        {
          WordIdArray& words = it->second;

          for(WordIdArray::iterator wit = words.begin(); wit != words.end(); )
          {
            LemmaMap::iterator lit = lemmas_.find(*wit);

            if(lit != lemmas_.end())
            {
              if(lit->second.sub_lemma(lemma))
              {
                add_lemma = false;
                break;
              }

              if(lemma.sub_lemma(lit->second))
              {
                WordId id = lit->first;
              
                lemmas_.erase(id);
                wit = words.erase(wit);
                duplicated_lemmas++;
              }
              else
              {
                wit++;
              }
            }
          }
        }
        
        if(add_lemma)
        {
          word_id = id;
          lemmas_[lemma_id] = lemma;
          norm_form_map[lemma.normal_form.text].push_back(lemma_id);
        }
        else
        {
          duplicated_lemmas++;
        }
      }
      else
      {
        break;
      }
    }

    std::cerr << "Duplicated lemmas: " << duplicated_lemmas << std::endl;
  }
  catch(const CExpc& e)
  {
    std::ostringstream ostr;
    ostr << "Application::convert: CExpc caught. Description: "
         << e.m_strCause;

    throw Exception(ostr.str());
  }
}

void
Application::Converter::dump_lemmas(std::ostream& ostr) const
  throw(El::Exception)
{
  for(LemmaMap::const_iterator it = lemmas_.begin(); it != lemmas_.end(); it++)
  {
    ostr << it->first << " : ";
    it->second.dump(ostr);
    ostr << std::endl;
  }
}

void
Application::Converter::dump_word_lemmas(std::ostream& ostr,
                                         const char* word) const
  throw(El::Exception)
{
  std::wstring wword;
  El::String::Manip::utf8_to_wchar(word, wword);

  WordIdSet lemmas_id;
  WordSet lemmas;
    
  for(LemmaMap::const_iterator it = lemmas_.begin(); it != lemmas_.end(); it++)
  {
    const WordArray& word_forms = it->second.word_forms;
    
    for(WordArray::const_iterator wit = word_forms.begin();
        wit != word_forms.end(); wit++)
    {
      if(wword == wit->text)
      {
        ostr << it->first << " : ";
        it->second.dump(ostr);
        ostr << std::endl;

        lemmas.insert(it->second.normal_form.text);
        lemmas_id.insert(it->first);
      }
    }
  }

  bool header = true;
  
  for(LemmaMap::const_iterator it = lemmas_.begin(); it != lemmas_.end(); it++)
  {
    if(lemmas.find(it->second.normal_form.text) != lemmas.end() &&
       lemmas_id.find(it->first) == lemmas_id.end())
    {
      if(header)
      {
        header = false;
        ostr << "\nSimilar:\n";
      }
      
      ostr << it->first << " : ";
      it->second.dump(ostr);
      ostr << std::endl;
    }
  }
}

void
Application::Converter::dump_norm_form_map(std::ostream& ostr,
                                           std::ostream& stat_str) const
  throw(El::Exception)
{
  NormFormMap norm_forms;
  unsigned long word_buffer_size = 0;
  unsigned long norm_form_buffer_size = 0;
  
  for(LemmaMap::const_iterator it = lemmas_.begin(); it != lemmas_.end(); it++)
  {    
    const WordArray& word_forms = it->second.word_forms;

    for(WordArray::const_iterator wit = word_forms.begin();
        wit != word_forms.end(); wit++)
    {
      std::string word;
      El::String::Manip::wchar_to_utf8(wit->text.c_str(), word);
      
      NormFormMap::iterator nit = norm_forms.find(word);
      
      if(nit == norm_forms.end())
      {
        word_buffer_size += word.length() + 1;
        norm_forms[word].push_back(it->first);
      }
      else
      {
        nit->second.push_back(it->first);
      }

      norm_form_buffer_size++;
    }
  }

  WordStatMap stat;
  
  ostr << lang_.l3_code() << "\t" << norm_forms.size() << "\t"
       << word_buffer_size << "\t" << norm_form_buffer_size << std::endl;

  for(NormFormMap::const_iterator it = norm_forms.begin();
      it != norm_forms.end(); it++)
  {
    ostr << it->first;

    const WordIdArray& word_array = it->second;

    for(WordIdArray::const_iterator wit = word_array.begin();
        wit != word_array.end(); wit++)
    {
      ostr << "\t" << *wit;
    }

    ostr << std::endl;

    
    WordStatMap::iterator sit = stat.find(word_array.size());

    if(sit == stat.end())
    {
      stat[word_array.size()] = 1;
    }
    else
    {
      sit->second++;
    }
  }

  stat_str << norm_forms.size() << " words in " << lemmas_.size()
           << " lemmas. Normal forms -> word with that number of normal "
    "forms:\n";
  
  for(WordStatMap::const_iterator it = stat.begin(); it != stat.end(); it++)
  {
    stat_str << it->first << " " << it->second << std::endl;
  }
  
}
