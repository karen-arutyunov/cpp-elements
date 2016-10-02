/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/Tools/Dict/AOT/Converter/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TOOLS_DICT_AOT_CONVERTER_APPLICATION_HPP_
#define _ELEMENTS_TOOLS_DICT_AOT_CONVERTER_APPLICATION_HPP_

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>

#include <ext/hash_map>

#include <ace/OS.h>

#include <Lemmatizers.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Lang.hpp>

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

  int raw(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int convert(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int dump(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  void init_lemmatizer(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);
  
private:
  
  typedef unsigned long WordId;

  struct Word
  {
    std::wstring text;
    WordId id;

    Word() throw(El::Exception) : id(0) {}
  };
  
  typedef std::vector<Word> WordArray;
  typedef std::vector<WordId> WordIdArray;
  typedef std::set<std::wstring> WordSet;
  typedef std::set<WordId> WordIdSet;

  typedef std::map<std::string, WordIdArray> NormFormMap;
  typedef std::map<unsigned long, unsigned long> WordStatMap;

  typedef __gnu_cxx::hash_map<std::wstring, WordIdArray, El::Hash::String>
  NormFormHashMap;
  
  struct Lemma
  {
    Word normal_form;
    WordArray word_forms;

    void dump(std::ostream& ostr) const throw(El::Exception);

    bool sub_lemma(const Lemma& lemma) const throw(El::Exception);
  };

  typedef std::map<WordId, Lemma> LemmaMap;  

  static std::wstring normalize(const char* word, El::Lang lang)
    throw(Exception, El::Exception);
    
  class Converter
  {
  public:
    EL_EXCEPTION(Exception, El::ExceptionBase);
    
  public:
    Converter(CLemmatizer* lemmatizer, El::Lang lang)
      throw(Exception, El::Exception);

    void dump_lemmas(std::ostream& ostr) const throw(El::Exception);

    void dump_word_lemmas(std::ostream& ostr, const char* word) const
      throw(El::Exception);
    
    void dump_norm_form_map(std::ostream& ostr, std::ostream& stat_str) const
      throw(El::Exception);

  private:
    El::Lang lang_;
    LemmaMap lemmas_;
  };
  
private:
  El::Lang lang_;
  
  typedef std::auto_ptr<CLemmatizer> CLemmatizerPtr;
  CLemmatizerPtr lemmatizer_;
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
  El::String::Manip::wchar_to_utf8(normal_form.text.c_str(), ostr);
  ostr << " >";
  
  for(WordArray::const_iterator it = word_forms.begin();
      it != word_forms.end(); it++)
  {
    ostr << " ";
    El::String::Manip::wchar_to_utf8(it->text.c_str(), ostr);
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
/*
  std::cerr << "'";
  lemma.dump(std::cerr);
  std::cerr << "' subset of '";
  dump(std::cerr);
  std::cerr << "'" << std::endl;
*/  
  return true;
}

inline
std::wstring 
Application::normalize(const char* word, El::Lang lang)
  throw(Exception, El::Exception)
{
  std::wstring wword;
  switch(lang.el_code())
  {
  case El::Lang::EC_RUS:
    El::String::Manip::win1251_to_wchar(word, wword);
    break;
  case El::Lang::EC_ENG:
  case El::Lang::EC_GER:
    El::String::Manip::latin1_to_wchar(word, wword);
    break;
  default:
    {
      std::ostringstream ostr;
      ostr << "Application::normalize: unsupported language " << lang;
      throw Exception(ostr.str());
    }
  }

  std::wstring lw;
  El::String::Manip::to_lower(wword.c_str(), lw);

  std::wstring tr;
  El::String::Manip::trim(lw.c_str(), tr);
  
  return tr;
}

#endif // _ELEMENTS_TOOLS_DICT_AOT_CONVERTER_APPLICATION_HPP_
