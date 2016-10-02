/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/LibXML/Python/Node.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_LIBXML_PYTHON_NODE_HPP_
#define _ELEMENTS_EL_LIBXML_PYTHON_NODE_HPP_

#include <stdint.h>
#include <string>

#include <Python.h>
#include <libxml/xpath.h>
#include <libxml/tree.h>

#include <El/Exception.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/Object.hpp>
#include <El/Python/Sequence.hpp>

namespace El
{
  namespace LibXML
  {
    namespace Python
    { 
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
      class Document : public El::Python::ObjectImpl
      {
      public:
        Document(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Python::Exception, El::Exception);

        Document(xmlDocPtr doc, bool is_html)
          throw(El::Python::Exception, El::Exception);

        virtual ~Document() throw() { if(doc_) xmlFreeDoc(doc_); }

        xmlDocPtr document() throw() { return doc_; }
        bool is_html() const throw() { return is_html_; }

        const char* url() const throw() { return url_.c_str(); }
        void url(const char* val) throw() { url_ = val; }

        El::String::Array& all_urls() throw() { return all_urls_; }
        const El::String::Array& all_urls() const throw() { return all_urls_; }
        void all_urls(const El::String::Array& urls) throw(El::Exception);

        void crc(uint64_t val) throw() { crc_ = val; }
        uint64_t crc() const throw() { return crc_; }

        const char* base_url() throw(El::Exception);
        
        El::Python::Sequence* create_xpath_result(
          xmlXPathObjectPtr xpath_result) throw(El::Exception);        

        PyObject* find(const char* xpath) throw(El::Exception);
        PyObject* py_find(PyObject* args) throw(El::Exception);
        
        std::string abs_url(const char* url, bool rel_base_url)
          throw(El::Exception);
        
        PyObject* py_abs_url(PyObject* args) throw(El::Exception);

        PyObject* text(PyObject* seq) throw(El::Exception);
        PyObject* py_text(PyObject* args) throw(El::Exception);        
        
        class Type : public El::Python::ObjectTypeImpl<Document,
                                                       Document::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;
          
          PY_TYPE_METHOD_VARARGS(
            py_find,
            "find",
            "Find nodes using XPath expression");

          PY_TYPE_METHOD_VARARGS(
            py_abs_url,
            "abs_url",
            "Absolute url from relative to the document url");
          
          PY_TYPE_METHOD_VARARGS(
            py_text,
            "text",
            "Text from nodes referenced by DocText sequence");
        };

      private:
        xmlDocPtr doc_;
        bool is_html_;
        std::string url_;
        std::string base_url_;
        El::String::Array all_urls_;
        uint64_t crc_;
      };

      typedef El::Python::SmartPtr<Document> Document_var;

      class DocText : public El::Python::ObjectImpl
      {
      public:
        DocText(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Exception);

        DocText(const char* xp = 0,
                bool concat = false,
                unsigned long ml = ULONG_MAX)
          throw(El::Exception);

        virtual ~DocText() throw() { }

        class Type :
          public El::Python::ObjectTypeImpl<DocText, DocText::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;

          PY_TYPE_MEMBER_STRING(xpath, "xpath", "XPath expression", false);
            
          PY_TYPE_MEMBER_BOOL(concatenate,
                              "concatenate",
                              "If nodes text to be concatenated");
            
          PY_TYPE_MEMBER_ULONG(max_len, "max_len", "Text max length");
        };

      public:
        std::string xpath;
        bool concatenate;
        unsigned long max_len;        
      };      

      typedef El::Python::SmartPtr<DocText> DocText_var;
      
      class Node : public El::Python::ObjectImpl
      {
        friend class Document;
        
      public:
        Node(PyTypeObject *type, PyObject *args, PyObject *kwds)
          throw(El::Python::Exception, El::Exception);

        Node(xmlNodePtr node, Document* doc)
          throw(El::Python::Exception, El::Exception);

        virtual ~Node() throw() { }

        Document* document() const throw() { return doc_.in(); }
        
        std::string text(bool is_html) throw(El::Exception);
        
        PyObject* py_name() throw(El::Exception);
        PyObject* py_type() throw(El::Exception);
        PyObject* py_attr(PyObject* args) throw(El::Exception);
        PyObject* py_text(PyObject* args) throw(El::Exception);

        PyObject* find(const char* xpath) throw(El::Exception);        
        PyObject* py_find(PyObject* args) throw(El::Exception);

        std::string name() const throw();
        
        class Type : public El::Python::ObjectTypeImpl<Node, Node::Type>
        {
        public:
          Type() throw(El::Python::Exception, El::Exception);
          static Type instance;
          
          PY_TYPE_METHOD_NOARGS(py_name, "name", "Node name");
          PY_TYPE_METHOD_NOARGS(py_type, "type", "Node type");
          
          PY_TYPE_METHOD_VARARGS(
            py_attr,
            "attr",
            "Attribute value");
          
          PY_TYPE_METHOD_VARARGS(
            py_text,
            "text",
            "Produces plain text representation");
          
          PY_TYPE_METHOD_VARARGS(
            py_find,
            "find",
            "Find nodes using XPath expression");
        };

        const char* attr(const char* name,
                         bool case_sensitive) const throw();
        
      private:
        xmlNodePtr node_;
        Document_var doc_;
      };

      typedef El::Python::SmartPtr<Node> Node_var;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace LibXML
  {
    namespace Python
    { 
      //
      // El::LibXML::Python::Document class
      //
      inline
      void
      Document::all_urls(const El::String::Array& urls) throw(El::Exception)
      {
        all_urls_ = urls;
      }

      //
      // El::LibXML::Python::Document::Type class
      //
      inline
      Document::Type::Type() throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<Document, Document::Type>(
            "el.libxml.Document",
            "Object encapsulating libxml xmlDoc functionality")
      {
      }
      
      //
      // El::LibXML::Python::Type class
      //
      inline
      DocText::Type::Type() throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<DocText, DocText::Type>(
            "el.libxml.DocText",
            "Document nodes text descriptor")
      {
      }
        
      //
      // El::LibXML::Python::Node::Type class
      //
      inline
      Node::Type::Type() throw(El::Python::Exception, El::Exception)
          : El::Python::ObjectTypeImpl<Node, Node::Type>(
            "el.libxml.Node",
            "Object encapsulating libxml xmlNode functionality")
      {
      }
    }
  }
}

#endif // _ELEMENTS_EL_LIBXML_PYTHON_NODE_HPP_
