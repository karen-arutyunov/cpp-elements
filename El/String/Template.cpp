/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/String/Template.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string.h>

#include <string>
#include <sstream>

#include <El/Exception.hpp>

#include "Template.hpp"

namespace El
{
  namespace String
  {
    namespace Template
    {
      Parser::Parser(const char* string,
                     const char* var_left_marker,
                     const char* var_right_marker,
                     const ParseInterceptor* interceptor)
        throw(InvalidArgument, ParsingFailed, El::Exception)
      {
        parse(string, var_left_marker, var_right_marker);
      }

      void
      Parser::parse(const char* string,
                    const char* var_left_marker,
                    const char* var_right_marker,
                    const ParseInterceptor* interceptor)
        throw(InvalidArgument, ParsingFailed, El::Exception)
      {
        if(var_left_marker == 0 || *var_left_marker == '\0')
        {
          throw InvalidArgument(
            "El::String::Template::Parser::Parser: var_left_marker undefined");
        }
      
        if(var_right_marker == 0 || *var_right_marker == '\0')
        {
          throw InvalidArgument(
            "El::String::Template::Parser::Parser: "
            "var_right_marker undefined");
        }

        chunks_.clear();

        var_left_marker_ = var_left_marker;
        var_right_marker_ = var_right_marker;

        if(string == 0)
        {
          return;
        }

        size_t var_left_marker_len = strlen(var_left_marker);
        size_t var_right_marker_len = strlen(var_right_marker);

        const char* ptr = string;

        while(true)
        {
          const char* begin = strstr(ptr, var_left_marker);

          if(begin == 0)
          {
            if(*ptr != '\0')
            {
              chunks_.push_back(Chunk());

              Chunk& chunk = *chunks_.rbegin();
              chunk.text = ptr;
//              chunk.offset = ptr - string;

              if(interceptor)
              {
                interceptor->update(chunk);
              }
            }
          
            break;
          }

          if(begin > ptr)
          {
            chunks_.push_back(Chunk());
            Chunk& chunk = *chunks_.rbegin();
            
            chunk.text.assign(ptr, begin - ptr);
//            chunk.offset = ptr - string;
              
            if(interceptor)
            {
              interceptor->update(chunk);
            }
          }
        
          ptr = begin + var_left_marker_len;
          const char* end = strstr(ptr, var_right_marker);

          if(end == 0)
          {
            std::ostringstream ostr;
            ostr << "El::String::Template::Parser::Parser: "
              "can't find right marker '" << var_right_marker <<
              "' for variable '" << ptr << "' in text \n" << string;

            throw ParsingFailed(ostr.str());
          }

          chunks_.push_back(Chunk(true));
          Chunk& chunk = *chunks_.rbegin();

          chunk.text.assign(ptr, end - ptr);
//          chunk.offset = ptr - string;

          if(interceptor)
          {
            interceptor->update(chunk);
          }
          
          ptr = end + var_right_marker_len;
        }
      }
    
      void
      Parser::instantiate(const Variables& variables,
                          std::ostream& output,
                          bool lax) const
        throw(VariableNotFound, El::Exception)
      {
        for(ChunkList::const_iterator it(chunks_.begin()), ie(chunks_.end());
            it != ie; ++it)
        {
          const Chunk& chunk(*it);
          
          if(chunk.is_var)
          {
            if(!variables.write(chunk, output))
            {
              if(lax)
              {
                output << var_left_marker_ << chunk.text << var_right_marker_;
              }
              else
              {
                std::ostringstream ostr;
                ostr << "El::String::Template::Parser::instantiate: "
                  "failed to resolve variable '" << chunk.text
                     << "' in template\n" << string();
                
                throw VariableNotFound(ostr.str());
              }
            }
          }
          else
          {
            variables.chunk(chunk);
            output << chunk.text;
          }
        }
      }
 
      std::string
      Parser::string() const throw(El::Exception)
      {
        std::ostringstream result;

        for(ChunkList::const_iterator it(chunks_.begin()), ie(chunks_.end());
            it != ie; ++it)
        {
          const Chunk& chunk(*it);
          
          if(chunk.is_var)
          {
            result << var_left_marker_ << chunk.text << var_right_marker_;
          }
          else
          {
            result << chunk.text;
          }
        }
      
        return result.str();      
      }
    }
  }
}
