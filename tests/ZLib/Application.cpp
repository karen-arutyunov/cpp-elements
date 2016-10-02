/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/test/ZLib/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#include <zlib.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>

#include <El/Moment.hpp>
#include <El/Compress/ZLib.hpp>
#include <El/Compress/GZip.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] = "\nUsage:\nElTestZLib <command> <args>\n"
  "Synopsis 1: ElTestZLib help\n"
  "Synopsis 2: ElTestZLib run [file=<filename>]\n"
  "Synopsis 3: ElTestZLib gzip file=<filename>\n";

  const size_t BUFF_SIZE = 300000000;
}

int
main(int argc, char** argv)
{
  try
  {
    srand(time(0));
    
    Application app;
    return app.run(argc, argv);
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'ElTestZLib help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "ElTestZLib: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "ElTestZLib: unknown exception caught\n";
  }
  
  return -1;
}

Application::Application() throw(Application::Exception, El::Exception)
{
}

Application::~Application() throw()
{
}

int
Application::run(int& argc, char** argv)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string command;
  
  int i = 1;  

  if(argc > 1)
  {
    command = argv[i++];
  }

  ArgList arguments;

  for(; i < argc; i++)
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
  else if(command == "gzip")
  {
    if(arguments.size() != 1 && arguments[0].name != "file")
    {
      std::cerr << "Invalid arguments for gzip command\n"
        "Run 'ElTestZLib help' for usage details\n";
      
      return -1;
    }

    return gzip(arguments[0].value.c_str());
  }

  test(arguments);

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
Application::gzip(const char* source_name)
  throw(InvalidArg, Exception, El::Exception)
{
  std::fstream infile(source_name, std::ios::in);

  if(!infile.is_open())
  {
    std::ostringstream ostr;
    ostr << "Application::gzip: failed to open file " << source_name;
    
    throw InvalidArg(ostr.str());
  }

  std::string dest_name = std::string(source_name) + ".gz";
  FileWriter writer(dest_name.c_str());

  unsigned long level = 6;
    
  El::Compress::ZLib::GZip gz(&writer, level);
  El::Compress::ZLib::OutStream& zlib_out = gz.stream();

  char buff[1024];

  while(true)
  {
    infile.read(buff, sizeof(buff));
    
    size_t read_bytes = infile.gcount();

    if(read_bytes > 0)
    {
      zlib_out.write(buff, read_bytes);
    }    
    else
    {
      gz.finalize();
      break;
    }
  }


  return 0;
}

int
Application::test(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string source_name;
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    if(it->name == "file")
    {
      source_name = it->value;
      break;
    } 
  }

  if(source_name.empty())
  {
    size_t buff_size = 15;
    
    for(size_t i = 1; buff_size < 100000000;
        buff_size = (buff_size + 1) * i++)
    {
      size_t val1 = (unsigned long long)rand() * 3 /
        ((unsigned long long)RAND_MAX + 1) + 1;
      
      size_t val2 = (unsigned long long)rand() * buff_size /
        ((unsigned long long)RAND_MAX + 1);

      size_t data_size = buff_size * val1 + val2;

      for(size_t level = 0; level <= 9; level++)
      {
        std::auto_ptr<unsigned char> data(random_data(data_size));

        size_t compressed_data_size = 0;
        std::auto_ptr<unsigned char> compressed_data(
          compress_data(data.get(),
                        data_size,
                        buff_size,
                        level,
                        compressed_data_size));

        uncompress_data(compressed_data.get(),
                        compressed_data_size,
                        buff_size,
                        data.get(),
                        data_size);
      }
    }
  }
  else
  {
    for(unsigned long level = 0; level <= 9; level++)
    {
      unsigned long crc = 0;
      compress_file(source_name.c_str(), "TestZLib.deflate", level, crc);
      uncompress_file("TestZLib.deflate", "TestZLib.inflate", crc);
    }    
  }

  return 0;
}

void
Application::uncompress_data(unsigned char* compressed_data,
                             size_t compressed_data_size, 
                             size_t buff_size,
                             unsigned char* source_data,
                             size_t source_data_size)
  throw(Exception, El::Exception)
{
  try
  {

    size_t fraction1 = (unsigned long long)rand() * 10 /
      ((unsigned long long)RAND_MAX + 1) + 1;
  
    size_t chunk_size = buff_size / fraction1;

    if(chunk_size == 0)
    {
      chunk_size = 1;
    }
    
    size_t fraction2 = (unsigned long long)rand() * 10 /
      ((unsigned long long)RAND_MAX + 1) + 1;

    size_t read_chunk_size = buff_size / fraction2;

    if(read_chunk_size == 0)
    {
      read_chunk_size = SIZE_MAX;
    }

    std::cout << "uncompressing buff_size=" << buff_size << ", chunk_size="
              << chunk_size << ", read_chunk_size=" << read_chunk_size
              << " ...";

    std::cout.flush();

    DataReader reader(compressed_data, compressed_data_size, read_chunk_size);
    DataWriter writer;

    El::Compress::ZLib::InStream zlib_in(&reader, buff_size, buff_size);

    std::auto_ptr<char> buff(new char[chunk_size]);
    
    while(true)
    {
      zlib_in.read(buff.get(), chunk_size);
    
      size_t read_bytes = zlib_in.gcount();

      if(read_bytes > 0)
      {
        writer.write(buff.get(), read_bytes);
      }
      else
      {
        break;
      }
    }

    if(!zlib_in.last_error_desc().empty())
    {
      std::ostringstream ostr;
      ostr << "Application::uncompress_data: data uncompression failed. "
        "Error:\n" << zlib_in.last_error_desc();

      throw Exception(ostr.str());
    }

    size_t uncompressed_data_size = 0;
    std::auto_ptr<unsigned char>
      uncompressed_data(writer.release(uncompressed_data_size));

    if(source_data_size != uncompressed_data_size)
    {
      std::ostringstream ostr;
      ostr << "Application::uncompress_data: sizes of source data and "
        "uncompressed data are different";

      throw Exception(ostr.str());
    }

    if(memcmp(source_data, uncompressed_data.get(), source_data_size))
    {
      std::ostringstream ostr;
      ostr << "Application::uncompress_data: source data and "
        "uncompressed data are different";

      throw Exception(ostr.str());
    }

    std::cout << " done\n";
  }
  catch(...)
  {
    std::cout << " failed\n";
    throw;
  }
  
}

unsigned char*
Application::compress_data(unsigned char* data,
                           size_t data_size,
                           size_t buff_size,
                           unsigned long level,
                           size_t& compressed_data_size)
  throw(InvalidArg, Exception, El::Exception)
{
  size_t fraction1 = (unsigned long long)rand() * 10 /
    ((unsigned long long)RAND_MAX + 1) + 1;
  
  size_t chunk_size = buff_size / fraction1;

  if(chunk_size == 0)
  {
    chunk_size = 1;
  }
    
  size_t fraction2 = (unsigned long long)rand() * 10 /
    ((unsigned long long)RAND_MAX + 1) + 1;

  size_t write_chunk_size = buff_size / fraction2;

  if(write_chunk_size == 0)
  {
    write_chunk_size = SIZE_MAX;
  }

  DataWriter writer(write_chunk_size);

  std::cout << "compressing data_size=" << data_size
            << ", buff_size=" << buff_size << ", chunk_size="
            << chunk_size << ", write_chunk_size=" << write_chunk_size
            << ", level=" << level << " ...";

  std::cout.flush();

  try
  {
    std::auto_ptr<unsigned char> buff(new unsigned char[chunk_size]);
    ACE_Time_Value compression_time;
  
    {
      El::Compress::ZLib::OutStream zlib_out(&writer,
                                             level,
                                             buff_size,
                                             buff_size);

      size_t bytes_left = data_size;
      size_t portion = 0;
  
      for(unsigned char* ptr = data; bytes_left; ptr += portion)
      {
        portion = chunk_size;

        if(portion > bytes_left)
        {
          portion = bytes_left ;
        }
      
        zlib_out.write((char*)ptr, portion);

        bytes_left -= portion;
      }

      zlib_out.finalize();

      if(!zlib_out.last_error_desc().empty())
      {
        std::ostringstream ostr;
        ostr << "Application::compress_data: data compression failed. Error:\n"
             << zlib_out.last_error_desc();

        throw Exception(ostr.str());
      }

      compression_time = zlib_out.compression_time();
    }

    unsigned char* res = writer.release(compressed_data_size);
    
    std::cout << " done (" << data_size << "->" << compressed_data_size
              << ":" << (data_size < compressed_data_size ? 0 :
                         compressed_data_size * 100 / data_size)
              << "%, " << El::Moment::time(compression_time) << ")\n";
    
    return res;
  }
  catch(...)
  {
    std::cout << " failed\n";
    throw;
  }
  
}

unsigned char*
Application::random_data(size_t data_size)
  throw(Exception, El::Exception)
{
  unsigned char* data = new unsigned char[data_size];
  unsigned char rand_char = 0;
  
  for(size_t i = 0; i < data_size; i++)
  {
    if((i % 5) == 0)
    {
      rand_char = (unsigned long long)rand() * 255 /
        ((unsigned long long)RAND_MAX + 1) + 1;
    }
    
    data[i] = rand_char;
  }

  return data;
}

void
Application::uncompress_file(const char* source_name,
                             const char* dest_name,
                             unsigned long crc)
  throw(Exception, El::Exception)
{
  FileReader reader(source_name);

  std::fstream outfile(dest_name, std::ios::out);

  if(!outfile.is_open())
  {
    std::ostringstream ostr;
    ostr << "Application::uncompress_file: failed to open file " << dest_name;
    
    throw Exception(ostr.str());
  }
  
      
  El::Compress::ZLib::InStream zlib_in(&reader);

  char buff[1024 * 10];
  uLong adler = crc32(0L, Z_NULL, 0);

  while(true)
  {
    zlib_in.read(buff, sizeof(buff));
    
    size_t read_bytes = zlib_in.gcount();

    if(read_bytes > 0)
    {
      outfile.write(buff, read_bytes);
      
      adler = crc32(adler, (const Bytef*)buff, read_bytes);
    }    
    else
    {
      outfile.flush();
      break;
    }
  }

  if(!zlib_in.last_error_desc().empty())
  {
    std::ostringstream ostr;
    ostr << "Application::uncompress_file: uncompression of file "
         << source_name << " failed. Error:\n" << zlib_in.last_error_desc();

    throw Exception(ostr.str());
  }

  if(adler != crc)
  {
    std::ostringstream ostr;
    ostr << "Application::uncompress_file: file " << source_name << " CRC ("
         << std::hex << adler << ") differes from source file CRC ("
         << std::hex << crc << ")";

    throw Exception(ostr.str());
  }

  std::cout << "Uncompression time "
            << El::Moment::time(zlib_in.decompression_time()) << std::endl;  
}

void
Application::compress_file(const char* source_name,
                           const char* dest_name,
                           unsigned long level,
                           unsigned long& crc)
  throw(InvalidArg, Exception, El::Exception)
{
  FileWriter writer(dest_name);
  
  std::fstream infile(source_name, std::ios::in);

  if(!infile.is_open())
  {
    std::ostringstream ostr;
    ostr << "Application::compress_file: failed to open file " << source_name;
    
    throw InvalidArg(ostr.str());
  }
    
  El::Compress::ZLib::OutStream zlib_out(&writer, level);

  char buff[1024 * 10];
  
  uLong adler = crc32(0L, Z_NULL, 0);

  while(true)
  {
    infile.read(buff, sizeof(buff));
    
    size_t read_bytes = infile.gcount();

    if(read_bytes > 0)
    {
      zlib_out.write(buff, read_bytes);

      adler = crc32(adler, (const Bytef*)buff, read_bytes);
    }    
    else
    {
      zlib_out.flush();
      break;
    }
  }

  if(!zlib_out.last_error_desc().empty())
  {
    std::ostringstream ostr;
    ostr << "Application::compress_file: compression of file " << source_name
         << " failed. Error:\n" << zlib_out.last_error_desc();

    throw Exception(ostr.str());
  }

  crc = adler;

  std::cout << "Compression level " << level << " of '" << source_name << "' "
            << (zlib_out.out_bytes() < zlib_out.in_bytes() ?
                (zlib_out.in_bytes() - zlib_out.out_bytes()) * 100 /
                zlib_out.in_bytes() : 0) << "%, time "
            << El::Moment::time(zlib_out.compression_time()) << std::endl;  
}

//
// FileWriter class
//
Application::FileWriter::FileWriter(const char* filename,
                                    size_t write_chunk)
  throw(Exception, El::Exception)
    : write_chunk_(write_chunk)
{
  file_.open(filename, std::ios::out);

  if(!file_.is_open())
  {
    std::ostringstream ostr;
    ostr << "FileWriter::FileWriter: failed to open file " << filename;

    throw Exception(ostr.str());
  }
}

Application::FileWriter::~FileWriter() throw()
{
}

size_t
Application::FileWriter::write(const char* buff, size_t len)
{
  if(len > write_chunk_)
  {
    len = write_chunk_;
  }

  file_.write(buff, len);
  return len;
}

//
// FileReader class
//
Application::FileReader::FileReader(const char* filename,
                                    size_t read_chunk)
  throw(Exception, El::Exception)
    : read_chunk_(read_chunk)
{
  file_.open(filename, std::ios::in);

  if(!file_.is_open())
  {
    std::ostringstream ostr;
    ostr << "FileReader::FileReader: failed to open file " << filename;

    throw Exception(ostr.str());
  }
}

Application::FileReader::~FileReader() throw()
{
}

size_t
Application::FileReader::read(char* buff, size_t len)
{
  if(len > read_chunk_)
  {
    len = read_chunk_;
  }

  file_.read(buff, len);
  return file_.gcount();
}

void
Application::FileReader::putback(char* buff, size_t len)
{
  while(len--)
  {
    file_.putback(buff[len]);
  }
}

//
// DataWriter class
//
Application::DataWriter::DataWriter(size_t write_chunk)
  throw(Exception, El::Exception)
    : data_(new unsigned char [BUFF_SIZE]),
      ptr_(data_.get()),
      write_chunk_(write_chunk)
{
}

Application::DataWriter::~DataWriter() throw()
{
}

size_t
Application::DataWriter::write(const char* buff, size_t len)
{
  if(data_.get() == 0)
  {
    throw Exception("Application::DataWriter::write: buffer released");
  }
  
  if(len > write_chunk_)
  {
    len = write_chunk_;
  }

  if(ptr_ + len > data_.get() + BUFF_SIZE)
  {
    throw Exception("Application::DataWriter::write: buffer too small");
  }
  
  memcpy(ptr_, buff, len);
  ptr_ += len;

  return len;
}

unsigned char*
Application::DataWriter::release(size_t& size) throw()
{
  size = ptr_ - data_.get();
  return data_.release();
}

//
// DataWriter class
//
Application::DataReader::DataReader(unsigned char* data,
                                    size_t data_size,
                                    size_t read_chunk)
  throw(Exception, El::Exception)
    : data_(data),
      data_size_(data_size),
      ptr_(data_),
      read_chunk_(read_chunk)
{
}

Application::DataReader::~DataReader() throw()
{
}

size_t
Application::DataReader::read(char* buff, size_t len)
{
  if(len > read_chunk_)
  {
    len = read_chunk_;
  }

  if(ptr_ + len > data_ + data_size_)
  {
    len = data_ + data_size_ - ptr_;
  }
     
  memcpy(buff, ptr_, len);
  ptr_ += len;

  return len;
}

void
Application::DataReader::putback(char* buff, size_t len)
{
  size_t shift = ptr_ - data_;

  if(len > shift)
  {
    len = shift;
  }
  
  ptr_ -= len;
}
