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
 * @file Elements/El/LibXML/Traverser.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LIBXML_TRAVERSER_HPP_
#define _ELEMENTS_EL_LIBXML_TRAVERSER_HPP_

#include <string>
#include <iostream>

#include <libxml/tree.h>

namespace El
{
  namespace LibXML
  {
    class Builder
    {
    public:
      virtual ~Builder() throw() {}

      virtual void element_pre_attr(xmlElementPtr node)
        throw(El::Exception) = 0;
      
      virtual void element_post_attr(xmlElementPtr node)
        throw(El::Exception) = 0;
      
      virtual void element_post(xmlElementPtr node) throw(El::Exception) = 0;
      virtual void attribute(xmlAttributePtr node) throw(El::Exception) = 0;
      virtual void text(xmlNodePtr node) throw(El::Exception) = 0;
    };
    
    class TextBuilder : public Builder
    {
    public:

      enum OutputType
      {
        OT_XML,
        OT_TEXT,
        OT_RENDERED_HTML
      };
      
    public:
      TextBuilder(std::ostream& output, OutputType output_type) throw();
      virtual ~TextBuilder() throw() {}

      virtual void element_pre_attr(xmlElementPtr node)
        throw(El::Exception);
      
      virtual void element_post_attr(xmlElementPtr node)
        throw(El::Exception);
      
      virtual void element_post(xmlElementPtr node) throw(El::Exception);
      virtual void attribute(xmlAttributePtr node) throw(El::Exception);
      virtual void text(xmlNodePtr node) throw(El::Exception);

      static bool block_element(const char* name) throw();

    private:
      std::ostream& output_;
      OutputType output_type_;
      bool inside_elem_;
      std::string ident_;
      
      static const char* block_elements_[];
    };

    class Traverser
    {
    public :
      Traverser() throw() {}
        
      void traverse(xmlNodePtr node, Builder& builder) throw(El::Exception);

      void traverse_list(xmlNodePtr node, Builder& builder)
        throw(El::Exception);
      
    protected:

      void traverse_element(xmlElementPtr node, Builder& builder)
        throw(El::Exception);      
    };

  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace XML
  {
  }
}

#endif // _ELEMENTS_EL_LIBXML_TRAVERSER_HPP_
