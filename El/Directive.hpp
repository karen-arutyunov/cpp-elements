/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Directive.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_DIRECTIVE_HPP_
#define _ELEMENTS_EL_DIRECTIVE_HPP_

#include <limits.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <El/Exception.hpp>

#include <El/String/ListParser.hpp>
#include <El/String/Manip.hpp>

namespace El
{
  struct Directive
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
    EL_EXCEPTION(ParsingError, Exception);
      
    struct Arg
    {
      EL_EXCEPTION(Exception, El::Directive::Exception);
      EL_EXCEPTION(InvalidTypeSec, Exception);
      EL_EXCEPTION(TypeMismatch, Exception);

      Arg(const char* value) throw(El::Exception);

      void validate(const char* type) const
        throw(TypeMismatch, InvalidTypeSec, El::Exception);

      const char* string() const throw(El::Exception);
      long long numeric() const throw(TypeMismatch, El::Exception);

    private:
      std::string value_;
    };
      
    typedef std::vector<Arg> ArgArray;

    struct TypeArray : public std::vector<std::string>
    {
      //
      // type parameter should be a list of types for each argument, like
      // "string,string,numeric" which mean there should be 2 strings
      // followed by long long.
      //
      // Valid types:
      // string[:class]  - string; class can be "ne" (non empty) or
      //                   "nws" (not empty and not entirely consisting of
      //                   whitespaces)
      // numeric[:A[,B]] - long long; A and be can be a valid long long number
      //                   or "min" (denominating minimal negative value) or
      //                   "max" (maximum positive value)
      
      void parse(const char* types) throw(ParsingError, El::Exception);

      static bool validate(const char* type, unsigned long num_type)
        throw(ParsingError, El::Exception);

      static void num_range(const char* type,
                            long long& min_value,
                            long long& max_value)
        throw(ParsingError, El::Exception);
      
    private:
      
      static bool str_to_num(const char* str, long long& value)
        throw(El::Exception);      
    };

    std::string name;
    ArgArray arguments;

    Directive(const char* nm,
              const char* args,
              const Directive::TypeArray* types = 0)
      throw(ParsingError, El::Exception);

    void dump(std::ostream& ostr) const throw(El::Exception);

  private:
    void push_argument(std::string& arg,
                       bool& quote,
                       const Directive::TypeArray* types,
                       unsigned long& type_num)
      throw(Arg::Exception, El::Exception);
  };
    
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  //
  // Directive class
  //
  inline
  Directive::Directive(const char* nm,
                       const char* args,
                       const Directive::TypeArray* types)
    throw(ParsingError, El::Exception)
      : name(nm ? nm : "")
  {
    if(args == 0)
    {
      return;
    }

    std::string arg;
    bool quote = false;
    const char* ptr = args;
    unsigned long type_num = 0;

    try
    {
      for(; *ptr != '\0'; ptr++)
      {
        bool isspace = std::isspace(*ptr);
        
        if(isspace)
        {
          if(quote)
          {
            arg.append(1, *ptr);
            continue;
          }
          
          if(arg.empty())
          {
            continue;
          }
            
          push_argument(arg, quote, types, type_num);
          continue;
        }

        switch(*ptr)
        {
        case '"':
          {
            if(quote)
            {
              push_argument(arg, quote, types, type_num);

              if(ptr[1] != '\0' && !std::isspace(ptr[1]))
              {
                // Erroneous situation; advance ptr to point at problem
                ptr++;
              }
              else
              {
                break;
              }
            }
            else if(arg.empty())
            {
              quote = true;
              break;
            }

            throw Arg::Exception(
              "unexpected \" character; probably you meant \\\" ?");
          }
        case '\\':
          {
            ptr++;
            
            if(*ptr == '\\' || *ptr == '"')
            {
              arg.append(1, *ptr);
              break;
            }
            
            if(*ptr == '\0')
            {
              throw Arg::Exception(
                "unexpected end of line after \\ character");
            }
            
            std::ostringstream ostr;
            ostr << "unexpected '" << *ptr << "' character";
            throw Arg::Exception(ostr.str());
          }
        default:
          {
            arg.append(1, *ptr);
          }
        }
      }

      if(quote)
      {
        throw Arg::Exception("expected closing \" character");
      }
        
      if(!arg.empty())
      {
        push_argument(arg, quote, types, type_num);
      }

      unsigned long types_count = types ? types->size() : 0;
      
      if(types_count &&
         ((*types)[types_count - 1] == "..." ?
          (types_count - 1) > type_num : types_count > type_num))
      {  
        std::ostringstream ostr;
        ostr << "up to " << types->size() << " arguments expected";
        throw Arg::Exception(ostr.str());
      }
        
    }
    catch(const Arg::Exception& e)
    {
      std::ostringstream ostr;
      ostr << "Parsing arguments of directive " << name <<
        " failed at position " << (ptr - args) + 1 << ". Reason: " << e.what()
           << ". Arguments: " << args;

      throw ParsingError(ostr.str());
    }
  }

  inline
  void
  Directive::push_argument(std::string& arg,
                           bool& quote,
                           const Directive::TypeArray* types,
                           unsigned long& type_num)
    throw(Arg::Exception, El::Exception)
  {
    Arg argument(arg.c_str());

    unsigned long types_count = types ? types->size() : 0;
    
    if(types_count)
    {
      if((*types)[types_count - 1] == "...")
      {
        if(types_count == 1)
        {
          throw Arg::Exception("'...' is unexpectedly alone");
        }

        const std::string& type =
          (*types)[type_num < types_count - 1 ? type_num : (types_count - 2)];
        
        argument.validate(type.c_str());
      }
      else
      {
        if(type_num >= types_count)
        {
          std::ostringstream ostr;
          ostr << "number of arguments should be " << types_count;
          
          throw Arg::Exception(ostr.str());
        }

        const std::string& type = (*types)[type_num];
        argument.validate(type.c_str());
      }
      
    }

    type_num++;
    arguments.push_back(argument);
    arg.clear();
    quote = false;
  }
    
  inline
  void
  Directive::dump(std::ostream& ostr) const throw(El::Exception)
  {
    ostr << name;

    for(ArgArray::const_iterator it = arguments.begin();
        it != arguments.end(); it++)
    {
      ostr << " \"" << it->string() << "\"";
    }

    ostr << std::endl;
  }
    
  //
  // Directive::Arg class
  //
  inline
  Directive::Arg::Arg(const char* value) throw(El::Exception)
      : value_(value ? value : "")
  {
  }

  inline
  const char*
  Directive::Arg::string() const throw(El::Exception)
  {
    return value_.c_str();
  }

  inline
  long long
  Directive::Arg::numeric() const throw(TypeMismatch, El::Exception)
  {
    long long val = 0;
    
    if(!String::Manip::numeric(value_.c_str(), val))
    {
      std::ostringstream ostr;
      ostr << "conversion to numeric failed for '" << value_ << "'";
          
      throw TypeMismatch(ostr.str());
    }

    return val;
  }
  
  inline
  void
  Directive::Arg::validate(const char* type) const
    throw(TypeMismatch, InvalidTypeSec, El::Exception)
  {
    try
    {
      TypeArray::validate(type, 0);
    
      if(strncmp(type, "numeric", 7) == 0)
      {
        long long val = numeric();

        if(type[7] == '\0')
        {
          return;
        }

        long long min_value = 0;
        long long max_value = 0;

        TypeArray::num_range(type, min_value, max_value);      

        if(val < min_value || val > max_value)
        {
          std::ostringstream ostr;
          ostr << "value " << val << " is out of range [" << min_value
               << "," << max_value << "]";
          
          throw TypeMismatch(ostr.str());
        }

        return;
      }
    }
    catch(const ParsingError& e)
    {
      throw InvalidTypeSec(e.what());
    }
  }

  //
  // Directive::TypeArray class
  //
  inline
  void
  Directive::TypeArray::parse(const char* types)
    throw(ParsingError, El::Exception)
  {
    clear();
      
    El::String::ListParser parser(types, "; \t\n\r");

    const char* item = 0;
    unsigned long num_type = 0;
    bool last_type = false;
    
    while((item = parser.next_item()) != 0)
    {
      if(last_type)
      {
        throw ParsingError("no type can follow '...' specification");
      }
      
      last_type = validate(item, num_type++);
      push_back(item);
    }
  }

  inline
  bool
  Directive::TypeArray::str_to_num(const char* str, long long& value)
    throw(El::Exception)
  {
    bool result = true;
    
    if(strcmp(str, "max") == 0)
    {
      value = LONG_LONG_MAX;
    }
    else if(strcmp(str, "min") == 0)
    {
      value = LONG_LONG_MIN;
    }
    else
    {
      result = String::Manip::numeric(str, value);
    }

    return result;
  }
        
  inline
  bool
  Directive::TypeArray::validate(const char* type, unsigned long num_type)
    throw(ParsingError, El::Exception)
  {
    if(strncmp(type, "string", 6) == 0)
    {
      if(type[6] == '\0' || strcmp(type, "string:ne") == 0 ||
         strcmp(type, "string:nws") == 0)
      {
        return false;
      }
    }
    else if(strncmp(type, "numeric", 7) == 0)
    {
      if(type[7] == '\0')
      {
        return false;
      }

      if(type[7] == ':')
      {
        long long min_value = 0;
        long long max_value = 0;
        TypeArray::num_range(type, min_value, max_value);
        
        return false;
      }
    }
    else if(strcmp(type, "...") == 0)
    {
      if(!num_type)
      {
        throw ParsingError("'...' can only follow type specification");
      }
      
      return true;
    }
  
    std::ostringstream ostr;
    ostr << "'" << type << "' is not a valid type specification";
    throw ParsingError(ostr.str());
  }

  inline
  void
  Directive::TypeArray::num_range(const char* type,
                                  long long& min_value,
                                  long long& max_value)
    throw(ParsingError, El::Exception)
  {
    if(strlen(type) >= 8 && type[7] == ':')
    {
      const char* range = type + 8;
      const char* max_val = strchr(range, ',');
      const char* min_val = range;
      
      std::string tmp;
        
      if(max_val)
      {
        tmp.assign(range, max_val - range);
        min_val = tmp.c_str();
        max_val++;          
      }
      else
      {
        max_val = "max";
      }

      if(!str_to_num(min_val, min_value))
      {
        throw ParsingError(
          "unexpected numeric minimum value specification");
      }        
          
      if(!str_to_num(max_val, max_value))
      {
        throw ParsingError(
          "unexpected numeric maximum value specification");
      }
    }
  }
  
}

#endif // _ELEMENTS_EL_DIRECTIVE_HPP_
