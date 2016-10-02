/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/String/Template.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_STRING_TEMPLATE_HPP_
#define _ELEMENTS_EL_STRING_TEMPLATE_HPP_

#include <string>
#include <list>
#include <iostream>
#include <sstream>

#include <ext/hash_map>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>

namespace El
{
  namespace String
  {
    namespace Template
    {
      EL_EXCEPTION(Exception, El::ExceptionBase);
      EL_EXCEPTION(InvalidArgument, Exception);
      EL_EXCEPTION(ParsingFailed, Exception);
      EL_EXCEPTION(VariableNotFound, Exception);
      
      struct Chunk
      {
        bool is_var;
        std::string text;
//        size_t offset;
        void* tag;

        Chunk(bool var = false, const char* txt = 0/*, size_t off = 0*/)
          throw(El::Exception);
      };
      
      typedef std::list<Chunk> ChunkList;

      class Variables
      {
      public:
        virtual ~Variables() throw() {}
        
        virtual bool write(const Chunk& chunk, std::ostream& output) const
          throw(El::Exception) = 0;

        virtual void chunk(const Chunk& value) const throw(El::Exception) {}
      };
      
      struct ParseInterceptor
      {
        virtual ~ParseInterceptor() throw() {}

        virtual void update(Chunk& chunk) const
          throw(ParsingFailed, Exception, El::Exception) = 0;
      };
      
      class Parser
      {
      public:
        Parser(const char* string,
               const char* var_left_marker,
               const char* var_right_marker,
               const ParseInterceptor* interceptor = 0)
          throw(InvalidArgument, ParsingFailed, El::Exception);

        Parser() throw(El::Exception);

        virtual ~Parser() throw();

        void parse(const char* string,
                   const char* var_left_marker,
                   const char* var_right_marker,
                   const ParseInterceptor* interceptor = 0)
          throw(InvalidArgument, ParsingFailed, El::Exception);

        //
        // lax if true, then all unresolved variables will go into resulting
        // text as it were in unparsed template
        //
        std::string instantiate(const Variables& variables, bool lax = false)
          const throw(VariableNotFound, El::Exception);

        void instantiate(const Variables& variables,
                         std::ostream& output,
                         bool lax = false) const
          throw(VariableNotFound, El::Exception);

        std::string string() const throw(El::Exception);

        const ChunkList& chunks() const throw() { return chunks_; }
        
      private:
        ChunkList chunks_;
        std::string var_left_marker_;
        std::string var_right_marker_;
      };

      class VariablesMap : public Variables,
                           public __gnu_cxx::hash_map<std::string,
                           std::string,
                           El::Hash::String>
      {
      public:
        VariablesMap() throw(El::Exception);

        virtual ~VariablesMap() throw();

      protected:
        virtual bool write(const Chunk& chunk, std::ostream& output) const
          throw(El::Exception);
      };
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace String
  {
    namespace Template
    {
      //
      // Chunk struct
      //
      inline
      Chunk::Chunk(bool var, const char* txt/*, size_t off*/)
        throw(El::Exception)
          : is_var(var),
            text(txt ? txt : ""),
//            offset(off),
            tag(0)
      {
      }

      //
      // Parser class
      //
      inline
      Parser::Parser() throw(El::Exception)
      {
      }
      
      inline
      Parser::~Parser() throw()
      {
      }

      inline
      std::string
      Parser::instantiate(const Variables& variables, bool lax) const
        throw(VariableNotFound, El::Exception)
      {
        std::ostringstream result;
        instantiate(variables, result, lax);
        return result.str();
      }
      
      //
      // VariablesMap class
      //
      inline
      VariablesMap::VariablesMap() throw(El::Exception)
      {
      }
      
      inline
      VariablesMap::~VariablesMap() throw()
      {
      }

      inline
      bool
      VariablesMap::write(const Chunk& chunk, std::ostream& output) const
        throw(El::Exception)
      {
        const_iterator it = find(chunk.text);

        if(it == end())
        {
          return false;
        }

        output << it->second;
        return true;
      }
      
    }
  }
}

#endif // _ELEMENTS_EL_STRING_TEMPLATE_HPP_
