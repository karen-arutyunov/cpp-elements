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
 * @file   Elements/Tools/MySQLClassGen/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */
#include <string.h>
#include <string>
#include <iostream>
#include <list>
#include <sstream>

#include <El/String/Manip.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
"\nUsage:\nMySQLClassGen <command> <command arguments>\n\n"
"Synopsis 1:\n"
"MySQLClassGen help\n\n"
"Synopsis 2:\n"
"MySQLClassGen gen <command arguments>\n"
"  command arguments ::= (class=<class> query=\"<query>\" "
"[ignore-not-null-flag=(1|0)])+ "
"[ignore-binary-flag=(1|0)])+ "
"[user=<user>] "
"[passwd=<passwd>] "
"[db=<db>] "
"[host=<host>] [port=<port>] [unix_socket=<unix_socket>] "
"[client_flag=<client_flag>]\n";

  const char LICENSE[] =
"// Copyright (C) 2005-2008 Karen Arutyunov\n"
"//\n"
"// This program was generated by MySQL Class Generating Compiler\n"
"// MySQLClassGen (TM)\n"
"//\n"
"// This program is free software; you can redistribute it and/or modify\n"
"// it under the terms of the GNU General Public License version 2 as\n"
"// published by the Free Software Foundation.\n"
"//\n"
"// This program is distributed in the hope that it will be useful,\n"
"// but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"// GNU General Public License for more details.\n"
"//\n"
"// You should have received a copy of the GNU General Public License\n"
"// along with this program; if not, write to the Free Software\n"
"// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA\n";
  
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
    std::cerr << "Invalid argument: " << e
              << "\nRun 'MySQLClassGen help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << "MySQLClassGen: El::Exception caught. "
      "Description:" << std::endl << e << std::endl;    
  }
  catch(...)
  {
    std::cerr << "MySQLClassGen: unknown exception caught\n";
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

  if(command == "gen")
  {
    gen(arguments);
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
Application::gen(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string user;
  std::string passwd;
  std::string db;
  std::string host;
  std::string unix_socket;
  unsigned short port = 0;
  unsigned long client_flag = 0;

  std::string query;
  std::string class_name;
  bool ignore_not_null_flag = false;
  bool ignore_binary_flag = false;

  ClassInfoList class_info;
  
  for(ArgList::const_iterator it = arguments.begin(); it != arguments.end();
      it++)
  {
    if(it->name == "user")
    {
      user = it->value;
    }
    else if(it->name == "passwd")
    {
      passwd = it->value;
    }
    else if(it->name == "db")
    {
      db = it->value;
    }
    else if(it->name == "host")
    {
      host = it->value;
    }
    else if(it->name == "port")
    {
      port = atol(it->value.c_str());
    }
    else if(it->name == "client_flag")
    {
      client_flag = atoll(it->value.c_str());
    }
    else if(it->name == "unix_socket")
    {
      unix_socket = it->value;
    }
    else if(it->name == "class")
    {
      if(!class_name.empty())
      {
        std::ostringstream ostr;
        ostr << "class argument " << class_name
             << " is not followed by a query argument";
        
        throw InvalidArg(ostr.str());
      }
      
      class_name = it->value;
    }
    else if(it->name == "query")
    {
      if(!query.empty())
      {
        std::ostringstream ostr;
        ostr << "query argument \"" << query
             << "\" do not follow class argument";
        
        throw InvalidArg(ostr.str());
      }

      query = it->value;
    }
    else if(it->name == "ignore-not-null-flag")
    {
      if(class_name.empty())
      {
        throw InvalidArg("ignore-not-null-flag argument should "
                         "follow class argument but go before query argument");
      }

      if(!query.empty())
      {
        throw InvalidArg("ignore-not-null-flag argument should go "
                         "before query argument");
      }

      ignore_not_null_flag = atol(it->value.c_str());
    }
    else if(it->name == "ignore-binary-flag")
    {
      if(class_name.empty())
      {
        throw InvalidArg("ignore-binary-flag argument should "
                         "follow class argument but go before query argument");
      }

      if(!query.empty())
      {
        throw InvalidArg("ignore-binary-flag argument should go "
                         "before query argument");
      }

      ignore_binary_flag = atol(it->value.c_str());
    }

    if(!class_name.empty() && !query.empty())
    {
      ClassInfo info;
      
      info.name = class_name;
      info.query = query;
      info.ignore_not_null_flag = ignore_not_null_flag;
      info.ignore_binary_flag = ignore_binary_flag;

      class_name.clear();
      query.clear();
      ignore_not_null_flag = false;
      ignore_binary_flag = false;
      
      class_info.push_back(info);
    }
    
  }

  if(class_info.empty())
  {
    throw InvalidArg("no one class/query information provided");
  }

  std::ostringstream file_stream;
    
  El::MySQL::DB_var dbase = unix_socket.empty() ?
    new El::MySQL::DB(user.c_str(),
                      passwd.c_str(),
                      db.c_str(),
                      port,
                      host.c_str(),
                      client_flag) :
    new El::MySQL::DB(user.c_str(),
                      passwd.c_str(),
                      db.c_str(),
                      unix_socket.c_str(),
                      client_flag);

  connection_ = dbase->connect();

  unsigned long size = class_info.size();
  unsigned long class_num = 0;
  
  std::string file_lock;
  for(ClassInfoList::const_iterator it = class_info.begin();
      it != class_info.end(); it++, class_num++)
  {
    std::string query = it->query;
    std::string class_name = it->name;
    bool ignore_not_null_flag = it->ignore_not_null_flag;
    bool ignore_binary_flag = it->ignore_binary_flag;
    
    El::MySQL::Result_var result = connection_->query(query.c_str());

    if(result->fetch_fields() == 0)
    {
      std::ostringstream ostr;
      ostr << "No columns returned for query:\n" << query;
    
      throw InvalidArg(ostr.str());
    }

    typedef std::list<std::string> StringList;
    StringList namspaces;

    std::ostringstream file_lock_ostr;
  
    for(std::string::size_type pos = class_name.find("::");
        pos != std::string::npos; pos = class_name.find("::"))
    {
      std::string name_space = class_name.substr(0, pos);
      class_name = class_name.substr(pos + 2);
      namspaces.push_back(name_space);
    
      file_lock_ostr << "_" << name_space;
    }
    
    if(!class_num)
    {
      file_lock_ostr << "_" << class_name << "__" << rand() << "_";

      El::String::Manip::utf8_to_upper(file_lock_ostr.str().c_str(),
                                       file_lock);
  
// Writing file header

      file_stream << LICENSE << std::endl
                  << "#ifndef " << file_lock << "\n#define " << file_lock
                  << "\n\n#include <limits.h>\n\n#include <string>\n"
        "#include <sstream>\n\n"
                  << "#include <El/Exception.hpp>\n"
        "#include <El/MySQL/DB.hpp>\n\n";
    }
    
// Opening namespaces
  
    std::string ident;
  
    for(StringList::const_iterator it = namspaces.begin();
        it != namspaces.end(); it++)
    {
      file_stream << ident << "namespace " << it->c_str() << std::endl
                   << ident << "{\n";
      ident += "  ";
    }

// Writing class declaration

    file_stream << "//\n// " << class_name << " class declaration\n//\n"
                << ident << "class " << class_name
                << ": public El::MySQL::Row\n" << ident << "{\n" << ident
                << "public:\n";

    ident += "  ";
    file_stream << ident
                << "EL_EXCEPTION(Exception, El::MySQL::Exception);\n" << ident
                << "EL_EXCEPTION(IsNull, Exception);\n\n";
    
    ident.resize(ident.size() - 2);

    file_stream << ident << "public:\n";
    ident += "  ";

    file_stream << ident << class_name
                << "(El::MySQL::Result* result, unsigned long use_columns = ULONG_MAX)\n"
                << ident << "  throw(Exception, El::Exception);\n\n";
    
    unsigned long num_fields = result->num_fields();
    for(unsigned long i = 0; i < num_fields; i++)
    {
      write_accessor_decl(file_stream, (*result)[i], ident);
    }

    ident.resize(ident.size() - 2);
    file_stream << ident << "};\n\n";

// Writing class definition
  
    file_stream << "//\n// " << class_name << " class definition\n//\n"
                << ident << "inline\n" << ident << class_name << "::"
                << class_name << "(El::MySQL::Result* result, unsigned long use_columns)\n" << ident <<
      "  throw(Exception, El::Exception)\n"
                << ident << "    : Row(result)\n" << ident << "{\n";
    
    ident += "  ";

    file_stream<< ident << "unsigned long num_columns = std::min(use_columns, "
      "(unsigned long)"<< num_fields << ");\n\n"
               << ident << "if(result->num_fields() != num_columns)\n" << ident << "{\n" << ident
                << "  std::ostringstream ostr;\n" << ident
                << "  ostr << \"" << class_name << "::" << class_name
                << ": unexpected number of fields \"\n" << ident
                << "       << result->num_fields() << \" instead of \" << "
      "num_columns;\n\n" << ident
                << "  throw Exception(ostr.str());\n" << ident << "}\n\n";

    for(unsigned long i = 0; i < num_fields; i++)
    {
      write_field_check(file_stream,
                        (*result)[i],
                        class_name.c_str(),
                        ignore_not_null_flag,
                        ignore_binary_flag,
                        i,
                        ident);
    }

    ident.resize(ident.size() - 2);
    file_stream << ident << "}\n\n";

    for(unsigned long i = 0; i < num_fields; i++)
    {
      write_accessor_def(file_stream,
                         (*result)[i],
                         class_name.c_str(),
                         i,
                         ident);
    }

// Closing namespaces
  
    for(StringList::const_iterator it = namspaces.begin();
        it != namspaces.end(); it++)
    {
      ident.resize(ident.size() - 2);
      file_stream << ident << "}\n";
    }

    file_stream << std::endl;

    if(class_num == size - 1)
    {
      
// Writing file footer
  
      file_stream << "#endif // " << file_lock;
    }
    
  }
  
// Producing resulted output

  std::cout << file_stream.str() << std::endl;
  
  return 0;
}

void
Application::write_accessor_decl(std::ostream& ostr,
                                 const MYSQL_FIELD& field,
                                 std::string& ident)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string type;
  
  switch(field.type)
  {
  case FIELD_TYPE_BIT:
    {
      type = "El::MySQL::Bit";
      break;
    }
  case FIELD_TYPE_TINY:
    {
      if(field.flags & UNSIGNED_FLAG)
      {
        type = "El::MySQL::UnsignedChar";
      }
      else
      {
        type = "El::MySQL::Char";
      }
      
      break;
    }
  case FIELD_TYPE_YEAR:
  case FIELD_TYPE_SHORT:
    {
      if(field.flags & UNSIGNED_FLAG)
      {
        type = "El::MySQL::UnsignedShort";
      }
      else
      {
        type = "El::MySQL::Short";
      }

      break;
    }
  case FIELD_TYPE_LONG:
  case FIELD_TYPE_INT24:
    {
      if(field.flags & UNSIGNED_FLAG)
      {
        type = "El::MySQL::UnsignedLong";
      }
      else
      {
        type = "El::MySQL::Long";
      }

      break;
    }
  case FIELD_TYPE_LONGLONG:
    {
      if(field.flags & UNSIGNED_FLAG)
      {
        type = "El::MySQL::UnsignedLongLong";
      }
      else
      {
        type = "El::MySQL::LongLong";
      }

      break;
    }

  case FIELD_TYPE_FLOAT:
    {
      type = "El::MySQL::Float";
      break;
    }

  case FIELD_TYPE_DECIMAL:
  case FIELD_TYPE_NEWDECIMAL:
  case FIELD_TYPE_DOUBLE:
    {
      type = "El::MySQL::Double";
      break;
    }

  case FIELD_TYPE_STRING:
  case FIELD_TYPE_VAR_STRING:
  case FIELD_TYPE_BLOB:
  case FIELD_TYPE_ENUM:
  case FIELD_TYPE_TIME:
    {
      type = "El::MySQL::String";
      break;
    }
  case FIELD_TYPE_TIMESTAMP:
  case FIELD_TYPE_DATE:
  case FIELD_TYPE_DATETIME:
    {
      type = "El::MySQL::DateTime";
      break;
    }
  default:
    {
      type = "UNSUPPORTED MYSQL TYPE";
      
      /*
      std::ostringstream ostr;
      ostr << "Unsupported type " << field.type << " encountered in field "
           << field.name;
      
      throw Exception(ostr.str());
      */
    }
  }
  
  ostr << ident << type << " " << field.name << "() const\n" << ident
       << " throw(Exception, El::Exception);\n\n";
}

void
Application::write_field_check(std::ostream& ostr,
                               const MYSQL_FIELD& field,
                               const char* class_name,
                               bool ignore_not_null_flag,
                               bool ignore_binary_flag,
                               unsigned long index,
                               std::string& ident)
  throw(InvalidArg, Exception, El::Exception)
{
  ostr << ident << "if(use_columns >= " << index << ")\n" << ident
       << "{\n" << ident << "  return;\n" << ident << "}\n\n";

  ostr << ident;
  
  if(index == 0)
  {
    ostr << "enum_field_types ";
  }
  
  ostr << "type = (*result)[" << index <<  "].type;\n\n"
       << ident << "if(type != " << field.type << ")\n" << ident
       << "{\n" << ident  
       << "  std::ostringstream ostr;\n" << ident
       << "  ostr << \"" << class_name << "::" << class_name
       << ": unexpected type \" << type\n" << ident
       << "       << \" instead of "
       << field.type << " for field " << field.name << "\";\n\n" << ident
       << "  throw Exception(ostr.str());\n" << ident << "}\n\n";

  ostr << ident;
  
  if(index == 0)
  {
    ostr << "unsigned int ";
  }

  unsigned int ul_mask = UNSIGNED_FLAG;

  if(!ignore_not_null_flag)
  {
    ul_mask |= NOT_NULL_FLAG;
  }
    
  if(!ignore_binary_flag)
  {
    ul_mask |= BINARY_FLAG;
  }
    
  unsigned int flags = field.flags & ul_mask;

  std::string mask = "UNSIGNED_FLAG";

  if(!ignore_not_null_flag)
  {
    mask += "|NOT_NULL_FLAG";
  }
  
  if(!ignore_binary_flag)
  {
    mask += "|BINARY_FLAG";
  }
  
  ostr << "flags = \n" << ident << "  (*result)[" << index
       << "].flags & (" << mask << ");\n\n"
       << ident << "if(flags != 0x" << std::hex << flags << ")\n"
       << ident << "{\n" << ident << "  std::ostringstream ostr;\n" << ident
       << "  ostr << \"" << class_name << "::" << class_name
       << ": unexpected flags 0x\" << std::hex\n" << ident
       << "       << flags << \" instead of 0x"
       << std::hex << flags << std::dec << " for field " << field.name
       << "\";\n\n" << ident << "  throw Exception(ostr.str());\n"
       << ident << "}\n\n";

  ostr << ident;
  
  if(index == 0)
  {
    ostr << "const char* ";
  }
  
  ostr << "name = (*result)[" << index <<  "].name;\n\n"
       << ident << "if(strcmp(name, \"" << field.name << "\"))\n" << ident
       << "{\n" << ident  
       << "  std::ostringstream ostr;\n" << ident
       << "  ostr << \"" << class_name << "::" << class_name
       << ": unexpected name \" << name\n" << ident
       << "       << \" instead of "
       << field.name << " for field " << field.name << "\";\n\n" << ident
       << "  throw Exception(ostr.str());\n" << ident << "}\n\n";
}

void
Application::write_accessor_def(std::ostream& ostr,
                                const MYSQL_FIELD& field,
                                const char* class_name,
                                unsigned long index,
                                std::string& ident)
  throw(InvalidArg, Exception, El::Exception)
{
  std::string type;
  std::string native_type;
  
  switch(field.type)
  {
  case FIELD_TYPE_BIT:
    {
      type = "El::MySQL::Bit";
      native_type = "unsigned short";
      
      break;
    }
  case FIELD_TYPE_TINY:
    {
      if(field.flags & UNSIGNED_FLAG)
      {
        type = "El::MySQL::UnsignedChar";
        native_type = "unsigned short";
      }
      else
      {
        type = "El::MySQL::Char";
        native_type = "short";
      }
      
      break;
    }
  case FIELD_TYPE_YEAR:
  case FIELD_TYPE_SHORT:
    {
      if(field.flags & UNSIGNED_FLAG)
      {
        type = "El::MySQL::UnsignedShort";
        native_type = "unsigned short";
      }
      else
      {
        type = "El::MySQL::Short";
        native_type = "short";
      }

      break;
    }
  case FIELD_TYPE_LONG:
  case FIELD_TYPE_INT24:
    {
      if(field.flags & UNSIGNED_FLAG)
      {
        type = "El::MySQL::UnsignedLong";
        native_type = "unsigned long";
      }
      else
      {
        type = "El::MySQL::Long";
        native_type = "long";
      }

      break;
    }
  case FIELD_TYPE_LONGLONG:
    {
      if(field.flags & UNSIGNED_FLAG)
      {
        type = "El::MySQL::UnsignedLongLong";
        native_type = "unsigned long long";
      }
      else
      {
        type = "El::MySQL::LongLong";
        native_type = "long long";
      }

      break;
    }

  case FIELD_TYPE_FLOAT:
    {
      type = "El::MySQL::Float";
      native_type = "float";

      break;
    }

  case FIELD_TYPE_DECIMAL:
  case FIELD_TYPE_NEWDECIMAL:
  case FIELD_TYPE_DOUBLE:
    {
      type = "El::MySQL::Double";
      native_type = "double";

      break;
    }
  case FIELD_TYPE_STRING:
  case FIELD_TYPE_VAR_STRING:
  case FIELD_TYPE_BLOB:
  case FIELD_TYPE_ENUM:
  case FIELD_TYPE_TIME:
    {
      type = "El::MySQL::String";
      native_type = "std::string";

      break;
    }
  case FIELD_TYPE_TIMESTAMP:
  case FIELD_TYPE_DATE:
  case FIELD_TYPE_DATETIME:
    {
      type = "El::MySQL::DateTime";
      native_type = "El::Moment";

      break;
    }
  default:
    {
      type = "UNSUPPORTED MYSQL TYPE";
      
      /*
      std::ostringstream ostr;
      ostr << "Unsupported type " << field.type << " encountered in field "
           << field.name;
      
      throw Exception(ostr.str());
      */
    }
  }

  ostr << ident << "inline\n" << ident << type << std::endl << ident
       << class_name << "::" << field.name
       << "() const\n" << ident << "  throw(Exception, El::Exception)\n"
       << ident << "{\n";

  ident += "  ";

  ostr << ident << "El::MySQL::DB::init_thread();\n\n";
    
  switch(field.type)
  {
  case FIELD_TYPE_BIT:
  case FIELD_TYPE_TINY:
  case FIELD_TYPE_SHORT:
  case FIELD_TYPE_LONG:
  case FIELD_TYPE_INT24:
  case FIELD_TYPE_LONGLONG:
  case FIELD_TYPE_FLOAT:
  case FIELD_TYPE_DECIMAL:
  case FIELD_TYPE_NEWDECIMAL:
  case FIELD_TYPE_DOUBLE:
  case FIELD_TYPE_YEAR:
    {
      write_common_check_code(ostr,
                              ident.c_str(),
                              class_name,
                              field.name,
                              index);

      ostr << ident << native_type << " value = 0;\n"
           << ident << "bool is_null = row_[" << index << "] == 0;\n\n"
           << ident << "if(!is_null)\n" << ident << "{\n"
           << ident << "  unsigned long* lengths = "
        "mysql_fetch_lengths(result_->mysql_res());\n" << ident
           << "  std::string tmp;\n" << ident
           << "  tmp.assign(row_[" << index << "], lengths[" << index
           << "]);\n\n" << ident << "  std::istringstream istr(tmp);\n"
           << ident << "  istr >> value;\n\n" << ident
           << "  if(istr.fail())\n" << ident << "  {\n" << ident
           << "    std::ostringstream ostr;\n" << ident
           << "    ostr << \"" << class_name << "::" << field.name
           << ": failed to convert '" << field.name << "' field data\"\n"
           << ident << "      \" to " << native_type << "\";\n\n" << ident
           << "    throw Exception(ostr.str());\n" << ident << "  }\n"
           << ident <<"}\n\n" << ident << "return " << type
           << "(is_null, value);\n";
      
      break;
    }
  case FIELD_TYPE_STRING:
  case FIELD_TYPE_VAR_STRING:
  case FIELD_TYPE_BLOB:
  case FIELD_TYPE_ENUM:
  case FIELD_TYPE_TIMESTAMP:
  case FIELD_TYPE_DATE:
  case FIELD_TYPE_TIME:
  case FIELD_TYPE_DATETIME:
    {
      write_common_check_code(ostr,
                              ident.c_str(),
                              class_name,
                              field.name,
                              index);

      ostr << ident << "if(row_[" << index << "] == 0)\n" << ident << "{\n"
           << ident << "  " << type << " tmp(true, 0, 0);\n" << ident
           << "  return tmp;\n" << ident
           << "}\n\n" << ident << "unsigned long* lengths = "
        "mysql_fetch_lengths(result_->mysql_res());\n" << ident
           << type << " tmp(false, row_[" << index
           << "], lengths[" << index << "]);\n\n" << ident << "return tmp;\n";

      break;
    }
  default:
    {
    }
  }

  ident.resize(ident.size() - 2);
  ostr << ident << "}\n\n";  
}

void
Application::write_common_check_code(std::ostream& ostr,
                                     const char* ident,
                                     const char* class_name,
                                     const char* field_name,
                                     unsigned long index)
  throw(El::Exception)
{
  ostr << ident << "if(row_ == 0)\n" << ident << "{\n" << ident
       << "  throw Exception(\"" << class_name << "::" << field_name
       << ": row_ is 0\");\n" << ident << "}\n\n";
  
  ostr << ident << "if(" << index << " >= result_->num_fields())\n"
       << ident << "{\n" << ident << "  std::ostringstream ostr;\n"
       << ident << "  ostr << \"" << class_name << "::" << field_name
       << ": unexpected index " << index
       << " when number of fileds is \"\n" << ident
       << "       << result_->num_fields();\n\n" << ident
       << "  throw Exception(ostr.str());\n" << ident << "}\n\n";
}