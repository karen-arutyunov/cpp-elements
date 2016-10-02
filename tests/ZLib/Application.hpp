/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/tests/ZLib/Application.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TESTS_ZLIB_APPLICATION_HPP_
#define _ELEMENTS_TESTS_ZLIB_APPLICATION_HPP_

#include <string>
#include <vector>

#include <El/Exception.hpp>

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
  
  typedef std::vector<Argument> ArgList;

  int help(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  int gzip(const char* file_name)
    throw(InvalidArg, Exception, El::Exception);
  
  int test(const ArgList& arguments)
    throw(InvalidArg, Exception, El::Exception);

  void compress_file(const char* source_name,
                     const char* dest_name,
                     unsigned long level,
                     unsigned long& crc)
    throw(InvalidArg, Exception, El::Exception);

  void uncompress_file(const char* source_name,
                       const char* dest_name,
                       unsigned long crc)
    throw(Exception, El::Exception);
  
  static unsigned char* random_data(size_t data_size)
    throw(Exception, El::Exception);
  
  unsigned char* compress_data(unsigned char* data,
                               size_t data_size,
                               size_t buff_size,
                               unsigned long level,
                               size_t& compressed_data_size)
    throw(InvalidArg, Exception, El::Exception);
  
  void uncompress_data(unsigned char* compressed_data,
                       size_t compressed_data_size,
                       size_t buff_size,
                       unsigned char* source_data,
                       size_t source_data_size)
    throw(Exception, El::Exception);

  private:
  
  class FileWriter : public El::Compress::ZLib::OutStreamCallback
  {
  public:
    EL_EXCEPTION(Exception, El::ExceptionBase);

  public:
    FileWriter(const char* filename, size_t write_chunk = SIZE_MAX)
      throw(Exception, El::Exception);

    virtual ~FileWriter() throw();
  
    virtual size_t write(const char* buff, size_t len);

  private:
    std::fstream file_;
    size_t write_chunk_;
  };
  
  class FileReader : public El::Compress::ZLib::InStreamCallback
  {
  public:
    EL_EXCEPTION(Exception, El::ExceptionBase);

  public:
    FileReader(const char* filename, size_t read_chunk = SIZE_MAX)
      throw(Exception, El::Exception);

    virtual ~FileReader() throw();
  
    virtual size_t read(char* buff, size_t len);
    virtual void putback(char* buff, size_t len);

  private:
    std::fstream file_;
    size_t read_chunk_;
  };

  class DataWriter : public El::Compress::ZLib::OutStreamCallback
  {
  public:
    EL_EXCEPTION(Exception, El::ExceptionBase);

  public:
    DataWriter(size_t write_chunk = SIZE_MAX)
      throw(Exception, El::Exception);

    virtual ~DataWriter() throw();
  
    virtual size_t write(const char* buff, size_t len);

    unsigned char* release(size_t& size) throw();

  private:
    std::auto_ptr<unsigned char> data_;
    unsigned char* ptr_;
    size_t write_chunk_;    
  };
  
  class DataReader : public El::Compress::ZLib::InStreamCallback
  {
  public:
    EL_EXCEPTION(Exception, El::ExceptionBase);

  public:
    DataReader(unsigned char* data,
               size_t data_size,
               size_t read_chunk = SIZE_MAX)
      throw(Exception, El::Exception);

    virtual ~DataReader() throw();
  
    virtual size_t read(char* buff, size_t len);
    virtual void putback(char* buff, size_t len);

  private:
    unsigned char* data_;
    size_t data_size_;
    unsigned char* ptr_;
    size_t read_chunk_;    
  };

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

#endif // _ELEMENTS_TESTS_ZLIB_APPLICATION_HPP_
