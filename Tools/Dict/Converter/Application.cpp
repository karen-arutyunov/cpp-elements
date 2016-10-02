/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/Tools/Dict/Converter/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <stdio.h>
#include <locale.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include <ace/OS.h>

#include <El/String/Manip.hpp>
#include <El/String/ListParser.hpp>
#include <El/Utility.hpp>

#include "Application.hpp"

struct LangMap
{
  const char* lang;
  const char* lang_aot;
};

namespace
{
  const char USAGE[] =
    "\nUsage:\nElDictConverter <command> <command arguments>\n\n"
    "Synopsis 1:\nElDictConverter help\n\n" 
    "Synopsis 2:\nElDictConverter convert lang=(chi|eng|ger|ita|jpn|kor|por|rum|rus|slo|spa|tur) "
    "dest=<dir-path> [dump-multiform=<norm form count>]\n"
    "Reads lemmas from standard input and outputs 3 files in "
    "destination directory:\n"
    " * <lang>.mrf - morphological dictionary enriched with IDs. "
    "If file already exist, word IDs it contan will be used for lemmas "
    "which were not changed from the last conversion.\n"
    " * <lang>.nrm - mapping of words to their normal form IDs.\n"
    " * <lang>.stt - dictionary and conversion statistics.\n"
    "Use dump-multiform param to dump words having normal forms count "
    "not less then value specified.\n";
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
    std::cerr << e << "\nRun 'ElDictConverter help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << e << std::endl;
  }
  catch(...)
  {
    std::cerr << "ElDictConverter: unknown exception caught\n";
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
  else if(command == "convert")
  {
    return convert(arguments);
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

int
Application::convert(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  El::Lang lang;
  std::string dest;
  unsigned long dump_multiform = ULONG_MAX;
  
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    const std::string& name = it->name;
    const std::string& value = it->value;

    if(name == "lang")
    {
      lang = El::Lang(value.c_str()); 
    }
    else if(name == "dest")
    {
      dest = value;
    }
    else if(name == "dump-multiform")
    {
      if(!El::String::Manip::numeric(value.c_str(), dump_multiform))
      {
        std::ostringstream ostr;
        ostr << "invalid dump-multiform argument value '" << value << "'";      
        throw InvalidArg(ostr.str());
      }
    }
    else
    {
      std::ostringstream ostr;
      ostr << "unknown argument '" << name << "=" << value
           << "' for 'convert' command";
      
      throw InvalidArg(ostr.str());
    }
  }

  if(lang == El::Lang::null)
  {
    throw InvalidArg("language undefined");
  }
    
  if(dest.empty())
  {
    throw InvalidArg("directory for destination dictionaries undefined");
  }
  
  Converter converter(std::cin, dest.c_str(), lang);
  
  converter.dump_lemmas();
  converter.dump_norm_form_map(dump_multiform);
  converter.dump_stat();
  
  return 0;
}

//
// Application::Converter class
//

Application::Converter::Converter(std::istream& src,
                                  const char* dest,
                                  El::Lang lang)
  throw(Exception, El::Exception)
    : dest_mrf_(std::string(dest) + "/" + lang.l3_code() + ".mrf"),
      dest_nrm_(std::string(dest) + "/" + lang.l3_code() + ".nrm"),
      dest_stt_(std::string(dest) + "/" + lang.l3_code() + ".stt"),
      lang_(lang),
      time_(ACE_OS::gettimeofday())
{
  LemmaMultiMap prev_dict;
  IdManager id_manager;
  
  load_prev_dict(prev_dict, id_manager);
    
  NormFormMultiMap norm_form_map;
  
  std::string line;
  while(std::getline(src, line))
  {

    if(line[0] == '#')
    {
      continue;
    }
    
    std::string uniform;
    El::String::Manip::utf8_to_uniform(line.c_str(), uniform);
      
    El::String::ListParser parser(uniform.c_str());

    Lemma lemma;    
    WordArray& word_forms = lemma.word_forms;

    const char* word = 0;
    while((word = parser.next_item()) != 0)
    {      
      Word form;
      form.text = Application::normalize(word);

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

      form.id = id_manager.get_next_id();

      if(lemma.normal_form.id == 0)
      {
        lemma.normal_form = form;
      }

      word_forms.insert(it, form);
    }

    if(word_forms.empty())
    {
      continue;
    }

    bool add_lemma = true;

    std::pair<NormFormMultiMap::iterator, NormFormMultiMap::iterator> ip =
      norm_form_map.equal_range(lemma.normal_form.text);

    if(ip.first != norm_form_map.end())
    {
      for(NormFormMultiMap::iterator it = ip.first; it != ip.second; )
      {
        LemmaMap::iterator lit = lemmas_.find(it->second);
        assert(lit != lemmas_.end());

        const Lemma& existing_lemma = lit->second;
          
        if(existing_lemma.sub_lemma(lemma))
        {
          add_lemma = false;
          break;
        }
        
        if(lemma.sub_lemma(existing_lemma))
        {
/*          
          std::cerr << "Lemma: ";
          lemma.dump(std::cerr);
          std::cerr << std::endl;
          std::cerr << "SubLemma: ";
          existing_lemma.dump(std::cerr);
          std::cerr << std::endl;
*/        
          const WordArray& word_forms = existing_lemma.word_forms;
          
          for(WordArray::const_iterator wit = word_forms.begin();
              wit != word_forms.end(); wit++)
          {
            id_manager.release_id(wit->id);
          }
            
          WordId id = lit->first;
            
          lemmas_.erase(id);
          norm_form_map.erase(it++);
          stat_.duplicated_lemmas++;
        }
        else
        {
          it++;
        }
      }
    }

    if(add_lemma)
    {
      lemmas_[lemma.normal_form.id] = lemma;
      norm_form_map.insert(std::make_pair(lemma.normal_form.text,
                                          lemma.normal_form.id));
    }
    else
    {
      const WordArray& word_forms = lemma.word_forms;
      
      for(WordArray::const_iterator wit = word_forms.begin();
          wit != word_forms.end(); wit++)
      {
        id_manager.release_id(wit->id);
      }
      
      stat_.duplicated_lemmas++;
    }
  }

  //
  // Restoring IDs from old dictionary
  //

  for(NormFormMultiMap::const_iterator nit = norm_form_map.begin();
      nit != norm_form_map.end(); nit++)
  {
    LemmaMap::iterator lit = lemmas_.find(nit->second);
    assert(lit != lemmas_.end());

    Lemma& lemma = lit->second;
      
    //
    // Check if can find lemma in old dictionary best matching new one.
    // Will use it ids if found.
    //
    std::pair<LemmaMultiMap::iterator, LemmaMultiMap::iterator> pi =
      prev_dict.equal_range(lemma.normal_form.text);

    if(pi.first == prev_dict.end())
    {
      continue;
    }
      
    unsigned long msize = ULONG_MAX;
    LemmaMultiMap::iterator mit = prev_dict.end();
        
    for(LemmaMultiMap::iterator it = pi.first; it != pi.second; it++)
    {
      const Lemma& old_lemma = it->second;
          
      if(old_lemma.sub_lemma(lemma) && msize > old_lemma.word_forms.size())
      {
        mit = it;
        msize = old_lemma.word_forms.size();
      }
    }

    if(mit == prev_dict.end())
    {
      msize = 0;

      for(LemmaMultiMap::iterator it = pi.first; it != pi.second; it++)
      {
        const Lemma& old_lemma = it->second;
            
        if(lemma.sub_lemma(old_lemma) &&
           msize < old_lemma.word_forms.size())
        {
          mit = it;
          msize = old_lemma.word_forms.size();
        }
      } 
    }

    if(mit == prev_dict.end())
    {
      continue;
    }
      
    const Lemma& old_lemma = mit->second;
    lemma.normal_form.id = old_lemma.normal_form.id;

    WordArray& word_forms = lemma.word_forms;
    const WordArray& old_word_forms = old_lemma.word_forms;

    WordArray::const_iterator oit = old_word_forms.begin();
          
    for(WordArray::iterator it = word_forms.begin();
        it != word_forms.end(); it++)
    {
      for(; oit != old_word_forms.end() && oit->text < it->text; oit++);

      if(oit != old_word_forms.end() && oit->text == it->text)
      {
        id_manager.release_id(it->id);
        it->id = oit->id;
        oit++;
      }
      else
      {
        it->id = id_manager.get_next_id();
      }
    }

    lemmas_[lemma.normal_form.id] = lemma;
    lemmas_.erase(nit->second);
    prev_dict.erase(mit);
    stat_.known_lemmas++;
  }
  
  stat_.removed_lemmas = prev_dict.size();
}

void
Application::Converter::load_prev_dict(LemmaMultiMap& dict,
                                       IdManager& id_manager) const
  throw(Exception, El::Exception)
{
  dict.clear();
  
  std::fstream file(dest_mrf_.c_str(), std::ios::in);

  if(!file.is_open())
  {
    return;
  }

  typedef std::set<WordId> WordIdSet;

  WordIdSet used_ids;
  WordId max_word_id = 0;

  std::string line;
  
  // Skip information line
  std::getline(file, line);
  
  while(std::getline(file, line))
  {
    El::String::ListParser parser(line.c_str());

    const char* word = parser.next_item();

    if(word == 0)
    {
      continue;
    }

    Lemma lemma;
    
    do
    {
      const char* sep = strrchr(word, ':');

      if(sep == 0)
      {
        std::ostringstream ostr;
        ostr << "invalid file '" << dest_mrf_ << "' format. Line is broken:\n"
             << line;

        throw InvalidArg(ostr.str());
      }

      unsigned long id = 0;
      if(!El::String::Manip::numeric(sep + 1, id))
      {
        std::ostringstream ostr;
        ostr << "invalid file '" << dest_mrf_ << "' format. "
          "Numeric expected after ':' in word '" << word << "', line :\n"
             << line;
        
        throw InvalidArg(ostr.str());
      }

      if(max_word_id < id)
      {
        max_word_id = id;
      }

      used_ids.insert(id);

      if(lemma.normal_form.id)
      {
        lemma.word_forms.push_back(Word());
        Word& w = *lemma.word_forms.rbegin();
        
        w.text.assign(word, sep - word);
        w.id = id;
      }
      else
      {
        lemma.normal_form.text.assign(word, sep - word);
        lemma.normal_form.id = id;
      } 
    }
    while((word = parser.next_item()) != 0);

    dict.insert(std::make_pair(lemma.normal_form.text, lemma));
  }
  
  id_manager.id(max_word_id + 1);

  WordId next = 1;
  
  for(WordIdSet::const_iterator it = used_ids.begin(); it != used_ids.end();
      it++)
  {
    WordId current = *it;

    if(current != next)
    {
      for(WordId id = next; id < current; id++)
      {
        id_manager.release_id(id);
      }
    }

    next = current + 1;
  }
  
  return;
}

void
Application::Converter::dump_lemmas() throw(El::Exception)
{
  rename_if_exist(dest_mrf_.c_str());
  
  std::fstream file(dest_mrf_.c_str(), std::ios::out);

  if(!file.is_open())
  {
    std::ostringstream ostr;
    ostr << "can not open file '" << dest_mrf_ << "' for write access";
    throw InvalidArg(ostr.str());    
  }

  unsigned long word_form_buffer_size = 0;

  for(LemmaMap::const_iterator it = lemmas_.begin(); it != lemmas_.end(); it++)
  {
    word_form_buffer_size += it->second.word_forms.size();
  }
  
  file << lang_.l3_code() << "\t" << lemmas_.size() << "\t"
       << word_form_buffer_size << std::endl;

  for(LemmaMap::const_iterator it = lemmas_.begin(); it != lemmas_.end(); it++)
  {
    const Word& normal_form = it->second.normal_form;
    file << normal_form.text << ":" << normal_form.id;
      
    const WordArray& word_forms = it->second.word_forms;
    
    for(WordArray::const_iterator wit = word_forms.begin();
        wit != word_forms.end(); wit++)
    {
      file << " " << wit->text << ":" << wit->id;
    }

    file << std::endl;
  }
}

void
Application::Converter::dump_norm_form_map(unsigned long dump_multiform)
  throw(El::Exception)
{
  rename_if_exist(dest_nrm_.c_str());

  std::fstream file(dest_nrm_.c_str(), std::ios::out);

  if(!file.is_open())
  {
    std::ostringstream ostr;
    ostr << "can not open file '" << dest_nrm_ << "' for write access";
    throw InvalidArg(ostr.str());    
  }

  stat_.norm_forms_distr.clear();

  NormFormMultiMap norm_forms;
    
  unsigned long word_buffer_size = 0;
  unsigned long norm_form_buffer_size = 0;

  for(LemmaMap::const_iterator it = lemmas_.begin(); it != lemmas_.end(); it++)
  {    
    const WordArray& word_forms = it->second.word_forms;

    for(WordArray::const_iterator wit = word_forms.begin();
        wit != word_forms.end(); wit++)
    {
      const std::string& word = wit->text;
      
      if(norm_forms.find(word) == norm_forms.end())
      {
        word_buffer_size += word.length() + 1;
      }
      
      norm_forms.insert(std::make_pair(word, it->first));
      norm_form_buffer_size++;
    }
  }

  typedef std::vector<std::string> StringArray;
  StringArray keys;
    
  El::Utility::keys(norm_forms, keys);    
  stat_.norm_forms = keys.size();
    
  file << lang_.l3_code() << "\t" << stat_.norm_forms << "\t"
       << word_buffer_size << "\t" << norm_form_buffer_size << std::endl;

  WordStatMap& stat = stat_.norm_forms_distr;
  
  for(StringArray::const_iterator it = keys.begin(); it != keys.end(); it++)
  {
    file << *it;

    std::pair<NormFormMultiMap::const_iterator,
      NormFormMultiMap::const_iterator> ip = norm_forms.equal_range(*it);

    unsigned long count = 0;
    for(NormFormMultiMap::const_iterator nit = ip.first; nit != ip.second;
        nit++, count++)
    {
      file << "\t" << nit->second;
    }
    
    file << std::endl;
    
    WordStatMap::iterator sit = stat.find(count);

    if(sit == stat.end())
    {
      stat[count] = 1;
    }
    else
    {
      sit->second++;
    }

    if(count >= dump_multiform)
    {
      std::cout << *it << " " << count << std::endl;
    }
  }
}

void
Application::Converter::dump_stat() const
  throw(El::Exception)
{
  El::Moment prev_build;
  
  std::fstream file(dest_stt_.c_str(), std::ios::in);

  if(file.is_open())
  {
    std::string line;
    const char* date = 0;
    
    if(getline(file, line) && (date = strchr(line.c_str(), ':')) != 0)
    {
      std::string date_str;
      El::String::Manip::trim(date + 1, date_str);
      
      try
      {
        prev_build.set_rfc0822(date_str.c_str());
      }
      catch(...)
      {
      }
    }

    file.close();
  }

  rename_if_exist(dest_stt_.c_str());

  file.open(dest_stt_.c_str(), std::ios::out);

  if(!file.is_open())
  {
    std::ostringstream ostr;
    ostr << "can not open file '" << dest_stt_ << "' for write access";
    throw InvalidArg(ostr.str());    
  }

  file << "Built: " << time_.rfc0822() << std::endl;

  if(prev_build != El::Moment::null)
  {
    file << "Prev. built: " << prev_build.rfc0822() << std::endl;
  }

  file << std::endl << stat_.norm_forms << " words in " << lemmas_.size()
       << " lemmas.\nNormal forms -> words with that number "
    "of normal forms:\n";
  
  const WordStatMap& stat = stat_.norm_forms_distr;
  
  for(WordStatMap::const_iterator it = stat.begin(); it != stat.end(); it++)
  {
    file << it->first << " " << it->second << std::endl;
  }

  file << "\nDuplicated lemmas: " << stat_.duplicated_lemmas << std::endl
       << "Added lemmas: " << lemmas_.size() - stat_.known_lemmas << std::endl
       << "Removed lemmas: " << stat_.removed_lemmas << std::endl;
}

void
Application::Converter::rename_if_exist(const char* filename) const
  throw(Exception, El::Exception)
{
  struct stat64 stat;
  memset(&stat, 0, sizeof(stat));
  
  if(stat64(filename, &stat) != 0)
  {
    return;
  }

  std::string new_name = std::string(filename) + "." + time_.dense_format();
      
  if(ACE_OS::rename(filename, new_name.c_str()) != 0)
  {
    std::ostringstream ostr;
    ostr << "Application::Converter::rename_if_exist: "
      "failed to rename file '" << filename << "'" << " to '"
         << new_name << "'";
    
    throw Exception(ostr.str().c_str());
  }
}

//
// Application::Converter::Stat class
//
Application::Converter::Stat::Stat() throw(El::Exception)
    : duplicated_lemmas(0),
      norm_forms(0),
      known_lemmas(0),
      removed_lemmas(0)
{
}
