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
 * @file Elements/El/LibXML/Traverser.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <assert.h>

#include <sstream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>

#include "Traverser.hpp"

namespace El
{
  namespace LibXML
  {
    //
    // Traverser class
    //
    void
    Traverser::traverse(xmlNodePtr node, Builder& builder)
      throw(El::Exception)
    {
      switch(node->type)
      {
      case XML_ELEMENT_NODE:
        {
          traverse_element((xmlElementPtr)node, builder);
          break;
        }
      case XML_TEXT_NODE:
        {
          builder.text(node);
          break;
        }
      case XML_ATTRIBUTE_NODE:
        {
          builder.attribute((xmlAttributePtr)node);
          break;
        }
      default:
        break;
      }
    }

    void
    Traverser::traverse_list(xmlNodePtr node, Builder& builder)
      throw(El::Exception)
    {
      for(xmlNodePtr cur = node; cur; cur = cur->next)
      {
        traverse(cur, builder);
      }
    }
    
    void
    Traverser::traverse_element(xmlElementPtr node, Builder& builder)
      throw(El::Exception)
    {
      builder.element_pre_attr(node);
      
      for(xmlAttributePtr attr = node->attributes; attr != 0;
          attr = (xmlAttributePtr)attr->next)
      {
        builder.attribute(attr);
      }
      
      builder.element_post_attr(node);
      traverse_list(((xmlNodePtr)node)->children, builder);
      builder.element_post(node);
    }
    
    //
    // TextBuilder class
    //

    const char* TextBuilder::block_elements_[] =
    {
      "ADDRESS",
      "BLOCKQUOTE",
      "BR",
      "DIV",
      "DL",
      "FIELDSET",
      "FORM",
      "H1",
      "H2",
      "H3",
      "H4",
      "H5",
      "H6",
      "HR",
      "NOSCRIPT",
      "OL",
      "P",
      "PRE",
      "TABLE",
      "UL",
      "DD",
      "DT",
      "LI",
      "TBODY",
      "TD",
      "TFOOT",
      "TH",
      "THEAD",
      "TR",
    };
    
    TextBuilder::TextBuilder(std::ostream& output, OutputType output_type)
      throw()
        : output_(output),
          output_type_(output_type),
          inside_elem_(false)
    {
    }
    
    void
    TextBuilder::element_pre_attr(xmlElementPtr node)
      throw(El::Exception)
    {
      if(output_type_ == OT_XML)
      {
        assert(node->name);
        output_ << std::endl << ident_ << "<";

        El::String::Manip::xml_encode(
          (const char*)node->name,
          output_,
          El::String::Manip::XE_TEXT_ENCODING |
          El::String::Manip::XE_PRESERVE_UTF8);
      }
      else if(output_type_ == OT_RENDERED_HTML)
      {
        if(block_element((const char*)node->name))
        {
          output_ << std::endl;
        }

        if(strcasecmp((const char*)node->name, "LI") == 0)
        {
          El::String::Manip::wchar_to_utf8(L"\x2022 ", output_);
        }
      }
      
      inside_elem_ = true;
    }
      
    void
    TextBuilder::element_post_attr(xmlElementPtr node)
      throw(El::Exception)
    {
      inside_elem_ = false;
      
      if(output_type_ == OT_XML)
      {        
        if(node->children == 0)
        {
          output_ << "/>";
        }
        else
        {
          output_ << ">";
          ident_ += "  ";
        }
      }
    }
      
    void
    TextBuilder::element_post(xmlElementPtr node) throw(El::Exception)
    {
      if(node->children && output_type_ == OT_XML)
      {
        ident_.resize(ident_.length() - 2);        
        output_ << std::endl << ident_ << "</";

        El::String::Manip::xml_encode(
          (const char*)node->name,
          output_,
          El::String::Manip::XE_TEXT_ENCODING |
          El::String::Manip::XE_PRESERVE_UTF8);
        
        output_ << ">";
      }
      else if(output_type_ == OT_RENDERED_HTML)
      {
        if(block_element((const char*)node->name))
        {
          output_ << std::endl;
        }
      }      
    }
    
    void
    TextBuilder::attribute(xmlAttributePtr node) throw(El::Exception)
    {
      if(output_type_ == OT_XML && inside_elem_)
      {
        output_ << " ";

        El::String::Manip::xml_encode(
          (const char*)node->name,
          output_,
          El::String::Manip::XE_TEXT_ENCODING |
          El::String::Manip::XE_PRESERVE_UTF8);

        output_ << "=\"";
      }

      xmlNodePtr child = ((xmlNodePtr)node)->children;

      if(output_type_ == OT_XML)
      {
        El::String::Manip::xml_encode(
          child ? (const char*)child->content : "",
          output_,
          El::String::Manip::XE_ATTRIBUTE_ENCODING |
          El::String::Manip::XE_PRESERVE_UTF8);
      }
      else if(!inside_elem_)
      {
        output_ << child->content;
      }
      
      if(output_type_ == OT_XML && inside_elem_)
      {
        output_  << "\"";
      }
    }
    
    void
    TextBuilder::text(xmlNodePtr node) throw(El::Exception)
    {
      if(output_type_ == OT_XML)
      {
        El::String::Manip::xml_encode(
          (const char*)node->content,
          output_,
          El::String::Manip::XE_TEXT_ENCODING |
          El::String::Manip::XE_PRESERVE_UTF8);
      }
      else
      {
        output_ << node->content;
      }
    }

    bool
    TextBuilder::block_element(const char* name) throw()
    {
      for(size_t i = 0;
          i < sizeof(block_elements_) / sizeof(block_elements_[0]); ++i)
      {
        if(strcasecmp(name, block_elements_[i]) == 0)
        {
          return true;
        }
      }

      return false;
    }
  }
}
