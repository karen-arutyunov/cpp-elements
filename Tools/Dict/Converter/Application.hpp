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
 * @file   Elements/Tools/Dict/Converter/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TOOLS_DICT_CONVERTER_APPLICATION_HPP_
#define _ELEMENTS_TOOLS_DICT_CONVERTER_APPLICATION_HPP_

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <queue>

#include <ext/hash_map>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Lang.hpp>
#include <El/Moment.hpp>

class Application
{
public:    
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(InvalidArg, Exception);
    
public:
    
  Application() throw(Exception, El::Exception);
  virtual ~Application() throw();

  int run(int& argc, char** argv) throw(InvalidArg, Exception, El::Exception);

private:

  struct Argument
  {
    std::string name;
    std::string value;

    Argument(const char* nm = 0, const char* vl = 0)
      throw(El::Exception);
  };

  typedef std::list<Argument> ArgList;

  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int convert(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

private:
  
  typedef unsigned long WordId;

  struct Word
  {
    std::string text;
    WordId id;

    Word() throw(El::Exception) : id(0) {}
  };
  
  typedef std::vector<Word> WordArray;
  typedef std::multimap<std::string, WordId> NormFormMultiMap;  
  typedef std::map<unsigned long, unsigned long> WordStatMap;

  struct Lemma
  {
    Word normal_form;
    WordArray word_forms;

    void dump(std::ostream& ostr) const throw(El::Exception);
    bool sub_lemma(const Lemma& lemma) const throw(El::Exception);
  };

  typedef std::map<WordId, Lemma> LemmaMap;
  typedef std::multimap<std::string, Lemma> LemmaMultiMap;

  static std::string normalize(const char* word)
    throw(Exception, El::Exception);
    
  class Converter
  {
  public:
    EL_EXCEPTION(Exception, El::ExceptionBase);
    
  public:
    
    Converter(std::istream& src, const char* dest, El::Lang lang)
      throw(Exception, El::Exception);
    
    void dump_lemmas() throw(El::Exception);
    
    void dump_norm_form_map(unsigned long dump_multiform)
      throw(El::Exception);
    
    void dump_stat() const throw(El::Exception);
    
  private:

    struct Stat
    {
      WordStatMap norm_forms_distr;
      unsigned long duplicated_lemmas;
      unsigned long norm_forms;
      unsigned long known_lemmas;
      unsigned long removed_lemmas;
      
      Stat() throw(El::Exception);
    };

    class IdManager
    {
    public:
      IdManager() throw() : id_(1) {}

      void id(WordId val) throw() { id_ = val; }
      WordId id() const throw() { return id_; }

      WordId get_next_id() throw(El::Exception);
      void release_id(WordId id) throw(El::Exception);

    private:
      typedef std::queue<WordId> WordIdQueue;
      
      WordId id_;
      WordIdQueue free_ids_;
    };
    
    void load_prev_dict(LemmaMultiMap& dict, IdManager& id_manager) const
      throw(Exception, El::Exception);

    void rename_if_exist(const char* filename) const
      throw(Exception, El::Exception);
    
  private:
    std::string dest_mrf_;
    std::string dest_nrm_;
    std::string dest_stt_;
    El::Lang lang_;
    LemmaMap lemmas_;
    Stat stat_;
    El::Moment time_;
  };
  
private:
  El::Lang lang_;  
};

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

//
// Application::Argument class
//

inline
Application::Argument::Argument(const char* nm, const char* vl)
  throw(El::Exception)
    : name(nm ? nm : ""),
      value(vl ? vl : "")
{
}

//
// Application::Lemma class
//

inline
void
Application::Lemma::dump(std::ostream& ostr) const throw(El::Exception)
{
  ostr << normal_form.text << " :";
  
  for(WordArray::const_iterator it = word_forms.begin();
      it != word_forms.end(); it++)
  {
    ostr << " " << it->text;
  }
}

inline
bool
Application::Lemma::sub_lemma(const Lemma& lemma) const throw(El::Exception)
{
  if(lemma.normal_form.text != normal_form.text ||
     lemma.word_forms.size() > word_forms.size())
  {
    return false;
  }

  WordArray::const_iterator wit = word_forms.begin();

  for(WordArray::const_iterator it = lemma.word_forms.begin();
      it != lemma.word_forms.end(); it++, wit++)
  {
    for(; wit != word_forms.end() && it->text != wit->text; wit++);

    if(wit == word_forms.end())
    {
      return false;
    }
  }

  return true;
}

//
// Application class
//

inline
std::string 
Application::normalize(const char* word) throw(Exception, El::Exception)
{
  std::wstring wword;
  El::String::Manip::utf8_to_wchar(word, wword);

  std::wstring lw;
  El::String::Manip::to_lower(wword.c_str(), lw);

  std::wstring tr;
  El::String::Manip::trim(lw.c_str(), tr);
  
  std::string sword;
  El::String::Manip::wchar_to_utf8(wword.c_str(), sword);
  return sword;
}

//
// Application::Converter::IdManager class
//

inline
Application::WordId
Application::Converter::IdManager::get_next_id() throw(El::Exception)
{
  if(free_ids_.empty())
  {
    return id_++;
  }
  else
  {
    WordId id = free_ids_.front();
    free_ids_.pop();
    return id;
  }
}

inline
void
Application::Converter::IdManager::release_id(WordId id) throw(El::Exception)
{
  free_ids_.push(id);
}

#endif // _ELEMENTS_TOOLS_DICT_CONVERTER_APPLICATION_HPP_
