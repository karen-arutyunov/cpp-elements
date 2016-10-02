/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Exception.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_EXCEPTION_HPP_
#define _ELEMENTS_EL_EXCEPTION_HPP_

#include <string.h>

#include <stdexcept>
#include <string>
#include <iostream>

#define EL_EXCEPTION(NAME, BASE) \
  class NAME##_tag_; \
  typedef El::Composite<NAME##_tag_, BASE> NAME

namespace El
{
  typedef std::exception Exception;

  class ExceptionBase : public virtual Exception
  {
  public:
    explicit ExceptionBase(const char* desc) throw ();

    explicit ExceptionBase(const std::string& desc) throw ();

    ~ExceptionBase() throw ();

    virtual const char* what() const throw ();

  protected:
    ExceptionBase() throw ();

    void init(const char* desc) throw ();
    void init(const std::string& desc) throw ();

  protected:
//    char desc_[10240];
    char desc_[5120];
  };

  template<typename Tag, typename Base>
  class Composite : public virtual Base
  {
  public:
    explicit Composite(const char* desc) throw ();
    explicit Composite(const std::string& desc) throw ();

  protected:
    Composite() throw ();
  };
  

  //
  // Implementation
  //

  inline
  ExceptionBase::ExceptionBase() throw ()
  {
    init(0);
  }
  
  inline
  ExceptionBase::ExceptionBase(const char* desc) throw ()
  {
    init(desc);
  }

  inline
  ExceptionBase::ExceptionBase(const std::string& desc) throw ()
  {
    init(desc.c_str());
  }

  inline
  ExceptionBase::~ExceptionBase() throw ()
  {
  }

  inline
  const char*
  ExceptionBase::what() const throw ()
  {
    return desc_;
  }
  
  inline
  void
  ExceptionBase::init(const char* desc) throw ()
  {
    strncpy(desc_, desc ? desc : "", sizeof(desc_) - 1);
    desc_[sizeof(desc_) - 1] = '\0';
  }

  inline
  void
  ExceptionBase::init(const std::string& desc) throw ()
  {
    init(desc.c_str());
  }
  
  //
  // Template members
  //

  template<typename Tag, typename Base>
  Composite<Tag, Base>::Composite() throw ()
  {}
  
  template<typename Tag, typename Base>
  Composite<Tag, Base>::Composite(const char* desc) throw ()
  {
    Base::init(desc);
  }

  template<typename Tag, typename Base>
  Composite<Tag, Base>::Composite(const std::string& desc) throw ()
  {
    Base::init(desc);
  }
}

inline
std::ostream&
operator<<(std::ostream& ostr, const El::Exception& e) throw(El::Exception)
{
  ostr << e.what();
  return ostr;
}

#endif // _ELEMENTS_EL_EXCEPTION_HPP_
