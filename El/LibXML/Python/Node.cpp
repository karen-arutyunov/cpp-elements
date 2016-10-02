/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/LibXML/Python/Node.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <Python.h>
#include <libxml/xpath.h>

#include <string>
#include <sstream>

#include <El/Python/Utility.hpp>
#include <El/Python/Module.hpp>
#include <El/Python/RefCount.hpp>
#include <El/Python/Sequence.hpp>

#include <El/LibXML/Traverser.hpp>
#include <El/Net/HTTP/URL.hpp>

#include "Node.hpp"

namespace El
{
  namespace LibXML
  {
    namespace Python
    {
      class LibXMLPyModule : public El::Python::ModuleImpl<LibXMLPyModule>
      {
      public:
        static LibXMLPyModule instance;
        
        LibXMLPyModule() throw(El::Exception);
        virtual void initialized() throw(El::Exception);
      };  
  
      LibXMLPyModule::LibXMLPyModule()
        throw(El::Exception)
          : El::Python::ModuleImpl<LibXMLPyModule>(
            "el.libxml",
            "Module containing ElLibXML library types.",
            true)
      {
      }
      
      void
      LibXMLPyModule::initialized() throw(El::Exception)
      {
        add_member(PyLong_FromLong(XML_ELEMENT_NODE), "XML_ELEMENT_NODE");
        add_member(PyLong_FromLong(XML_ATTRIBUTE_NODE), "XML_ATTRIBUTE_NODE");
        add_member(PyLong_FromLong(XML_TEXT_NODE), "XML_TEXT_NODE");
        
        add_member(PyLong_FromLong(XML_CDATA_SECTION_NODE),
                   "XML_CDATA_SECTION_NODE");
        
        add_member(PyLong_FromLong(XML_ENTITY_REF_NODE),
                   "XML_ENTITY_REF_NODE");
        
        add_member(PyLong_FromLong(XML_ENTITY_NODE), "XML_ENTITY_NODE");
        add_member(PyLong_FromLong(XML_PI_NODE), "XML_PI_NODE");
        add_member(PyLong_FromLong(XML_COMMENT_NODE), "XML_COMMENT_NODE");
        add_member(PyLong_FromLong(XML_DOCUMENT_NODE), "XML_DOCUMENT_NODE");
        
        add_member(PyLong_FromLong(XML_DOCUMENT_TYPE_NODE),
                   "XML_DOCUMENT_TYPE_NODE");
        
        add_member(PyLong_FromLong(XML_DOCUMENT_FRAG_NODE),
                   "XML_DOCUMENT_FRAG_NODE");
        
        add_member(PyLong_FromLong(XML_NOTATION_NODE), "XML_NOTATION_NODE");
        
        add_member(PyLong_FromLong(XML_HTML_DOCUMENT_NODE),
                   "XML_HTML_DOCUMENT_NODE");
        
        add_member(PyLong_FromLong(XML_DTD_NODE), "XML_DTD_NODE");
        add_member(PyLong_FromLong(XML_ELEMENT_DECL), "XML_ELEMENT_DECL");
        add_member(PyLong_FromLong(XML_ATTRIBUTE_DECL), "XML_ATTRIBUTE_DECL");
        add_member(PyLong_FromLong(XML_ENTITY_DECL), "XML_ENTITY_DECL");
        add_member(PyLong_FromLong(XML_NAMESPACE_DECL), "XML_NAMESPACE_DECL");
        add_member(PyLong_FromLong(XML_XINCLUDE_START), "XML_XINCLUDE_START");
        add_member(PyLong_FromLong(XML_XINCLUDE_END), "XML_XINCLUDE_END");
        
        add_member(PyLong_FromLong(XML_DOCB_DOCUMENT_NODE),
                   "XML_DOCB_DOCUMENT_NODE");
      }
      
      LibXMLPyModule LibXMLPyModule::instance;
/*      
      El::Python::Module el_libxml_module(
        "el.libxml",
        "Module containing ElLibXML library types.",
        true);
*/
      Document::Type Document::Type::instance;
      DocText::Type DocText::Type::instance;      
      Node::Type Node::Type::instance;
      
      //
      // Document class
      //
      Document::Document(PyTypeObject *type,
                         PyObject *args,
                         PyObject *kwds)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(type),
            doc_(0),
            is_html_(false),
            crc_(0)
      {
      }
      
      Document::Document(xmlDocPtr doc, bool is_html)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(&Type::instance),
            doc_(doc),
            is_html_(is_html),
            crc_(0)
      {
      }

      El::Python::Sequence*
      Document::create_xpath_result(xmlXPathObjectPtr xpath_result)
        throw(El::Exception)
      {
        El::Python::Sequence_var result = new El::Python::Sequence();
        xmlNodeSetPtr node_set = xpath_result->nodesetval;

        if(!xmlXPathNodeSetIsEmpty(node_set))
        {
          result->reserve(node_set->nodeNr);
          
          for(int j = 0; j < node_set->nodeNr; ++j)
          {
            xmlNodePtr node = node_set->nodeTab[j];
            result->push_back(new Node(node, this));
          }
        }

        return result.retn();
      }

      const char*
      Document::base_url() throw(El::Exception)
      {
        if(!base_url_.empty() || !is_html_ || doc_ == 0)
        {
          return base_url_.c_str();
        }

        base_url_ = url_;
        
        try
        {
          El::Python::Object_var res = find("/html/head/base/@href");

          El::Python::Sequence* seq =
            El::Python::Sequence::Type::down_cast(res.in());

          if(seq && !seq->empty())
          {
            Node* node = Node::Type::down_cast((*seq)[0].in());

            if(node)
            {
              std::string bu = node->text(false);
              
              try
              {
                El::Net::HTTP::URL_var url =
                  new El::Net::HTTP::URL(bu.c_str());
                
                base_url_ = url->string();
              }
              catch(const El::Net::HTTP::URL::InvalidArg&)
              {
                El::Net::HTTP::URL_var url =
                  new El::Net::HTTP::URL(url_.c_str());
                
                base_url_ = url->abs_url(bu.c_str());
              }
            }
          }
        }
        catch(const El::Exception& )
        {
        }
        
        return base_url_.c_str();
      }
      
      PyObject*
      Document::find(const char* xpath) throw(El::Exception)
      {
        if(doc_ == 0)
        {
          return El::Python::add_ref(Py_None);
        }        

        El::Python::Sequence_var result;
        xmlXPathObjectPtr xpath_result = 0;

        try
        {
          xmlXPathContextPtr xpc = xmlXPathNewContext(doc_);
      
          if(xpc == 0)
          {
            std::ostringstream ostr;
            ostr << "El::LibXML::Python::Document::find: "
              "xmlXPathNewContext failed";
            
            throw Exception(ostr.str());      
          }
      
          xpath_result =
            xmlXPathEvalExpression((xmlChar*)xpath, xpc);
      
          xmlXPathFreeContext(xpc);
      
          if(xpath_result == 0)
          {
            std::ostringstream ostr;
            ostr << "El::LibXML::Python::Document::find: "
              "xmlXPathEvalExpression failed for '" << xpath << "'";
                
            throw Exception(ostr.str());      
          }

          result = create_xpath_result(xpath_result);            
          xmlXPathFreeObject(xpath_result);
        }
        catch(const El::Exception& e)
        {
          if(xpath_result)
          {
            xmlXPathFreeObject(xpath_result);
          }

          throw;
        }

        return result.retn();
      }
      
      PyObject*
      Document::py_find(PyObject* args) throw(El::Exception)
      {
        char* xpath = 0;

        if(!PyArg_ParseTuple(args, "s:el.libxml.Document.find", &xpath))
        {
          El::Python::handle_error(
            "El::LibXML::Python::Document::py_find");
        }

        return find(xpath);
      }

      PyObject*
      Document::py_abs_url(PyObject* args) throw(El::Exception)
      {
        char* url = 0;
        unsigned char rel_base_url = 1;

        if(!PyArg_ParseTuple(args,
                             "s|B:el.libxml.Document.abs_url",
                             &url,
                             &rel_base_url))
        {
          El::Python::handle_error(
            "El::LibXML::Python::Document::py_abs_url");
        }

        std::string result = abs_url(url, rel_base_url);
        
        return result.empty() ? El::Python::add_ref(Py_None) :
          PyString_FromString(result.c_str());
      }

      std::string
      Document::abs_url(const char* url, bool rel_base_url)
        throw(El::Exception)
      {
        std::string result;
        
        try
        {
          const char* bu = rel_base_url ? base_url() : url_.c_str();
          
          El::Net::HTTP::URL_var u =
            new El::Net::HTTP::URL(*bu == '\0' ? url_.c_str() : bu);
          
          result = u->abs_url(url);
        }
        catch(const El::Exception&)
        {
        }

        return result;
      }

      PyObject*
      Document::py_text(PyObject* args) throw(El::Exception)
      {
        PyObject* seq = 0;
        
        if(!PyArg_ParseTuple(args,
                             "O:el.libxml.Document.text",
                             &seq))
        {
          El::Python::handle_error("El::LibXML::Python::Document::py_text");
        }

        return text(seq);
      }
      
      PyObject*
      Document::text(PyObject* seq) throw(El::Exception)
      {
        if(!is_html_ || doc_ == 0)
        {
          throw Exception("El::LibXML::Python::Document::text: "
                          "can be applied to HTML document only");
        }
        
        if(!PySequence_Check(seq))
        {
          throw Exception("El::LibXML::Python::Document::text: "
                          "argument of sequence type expected");
        }

        int len = PySequence_Size(seq);

        for(int i = 0; i < len; ++i)
        {
          El::Python::Object_var obj = PySequence_GetItem(seq, i);
          
          if(obj.in() == 0)
          {
            El::Python::handle_error("El::LibXML::Python::Document::text");
          }
          
          if(PyString_Check(obj.in()))
          {
            return obj.add_ref();
          }
          
          DocText& doc_text = *DocText::Type::down_cast(obj.in());
          
          El::Python::Object_var nd = find(doc_text.xpath.c_str());
            
          const El::Python::Sequence& nodes =
            *El::Python::Sequence::Type::down_cast(nd.in());
          
          std::string text;
          
          for(El::Python::Sequence::const_iterator i(nodes.begin()),
                e(nodes.end()); i != e; ++i)
          {
            El::LibXML::Python::Node* node =
              El::LibXML::Python::Node::Type::down_cast(i->in());
            
            std::string trimmed;
            El::String::Manip::trim(node->text(true).c_str(), trimmed);
            
            if(trimmed.empty())
            {
              continue;
            }
            
            if(doc_text.concatenate)
            {
              if(!text.empty())
              {
                text += " ";
              }
              
              text += trimmed;
            }
            else
            {
              text = trimmed;
              break;
            }  
          }

          if(!text.empty())
          {
            if(doc_text.max_len != ULONG_MAX)
            {
              std::string truncated;
          
              El::String::Manip::truncate(text.c_str(),
                                          truncated,
                                          doc_text.max_len,
                                          true,
                                          El::String::Manip::UAC_XML_1_0);

              return PyString_FromString(truncated.c_str());
            }
            else
            {
              return PyString_FromString(text.c_str());
            }
          }
        }
        
        return PyString_FromString("");
      }
      
      //
      // DocText class
      //
      DocText::DocText(PyTypeObject *type, PyObject *args, PyObject *kwds)
        throw(El::Exception)
          : El::Python::ObjectImpl(type),
            concatenate(false),
            max_len(ULONG_MAX)
      {
        const char* kwlist[] =
          { "xpath",
            "concatenate",
            "max_len",
            NULL
          };
        
        char* xp = 0;
        unsigned char concat = 0;
        PyObject* max_length = Py_None;
        
        if(!PyArg_ParseTupleAndKeywords(
             args,
             kwds,
             "s|BO:el.libxml.DocText.DocText",
             (char**)kwlist,
             &xp,
             &concat,
             &max_length))
        {
          El::Python::handle_error("El::LibXML::Python::DocText.DocText");
        }

        xpath = xp;
        concatenate = concat;
        
        max_len = max_length == Py_None ? ULONG_MAX :
          El::Python::ulong_from_number(
            max_length,
            "El::LibXML::Python::DocText.DocText");
      }
      
      DocText::DocText(const char* xp, bool concat, unsigned long ml)
        throw(El::Exception)
          : El::Python::ObjectImpl(&Type::instance),
            xpath(xp ? xp : ""),
            concatenate(concat),
            max_len(ml)
      {
      }

      //
      // Node class
      //
      Node::Node(PyTypeObject *type,
                 PyObject *args,
                 PyObject *kwds)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(type),
            node_(0)
      {
      }
      
      Node::Node(xmlNodePtr node, Document* doc)
        throw(El::Python::Exception, El::Exception)
          : ObjectImpl(&Type::instance),
            node_(node),
            doc_(El::Python::add_ref(doc))
      {
      }

      std::string
      Node::name() const throw()
      {
        return node_ && node_->name ? (const char*)node_->name : "";
      }
      
      PyObject*
      Node::py_name() throw(El::Exception)
      {
        return node_ && node_->name ?
          PyString_FromString((const char*)node_->name) :
          El::Python::add_ref(Py_None);
      }
      
      PyObject*
      Node::py_type() throw(El::Exception)
      {
        return node_ ? PyLong_FromLong(node_->type) :
          El::Python::add_ref(Py_None);
      }
      
      PyObject*
      Node::py_attr(PyObject* args) throw(El::Exception)
      {
        char* name = 0;
        PyObject* def_val = 0;
        unsigned char case_sensitive = 1;
        
        if(!PyArg_ParseTuple(args,
                             "s|OB:el.libxml.Node.attr",
                             &name,
                             &def_val,
                             &case_sensitive))
        {
          El::Python::handle_error("El::LibXML::Python::Node::py_attr");
        }

        if(node_ == 0 || node_->type != XML_ELEMENT_NODE)
        {
          return El::Python::add_ref(Py_None);
        }

        const char* val = attr(name, case_sensitive);

        return val ? PyString_FromString(val) :
          (def_val ? El::Python::add_ref(def_val) : PyString_FromString(""));
      }

      const char*
      Node::attr(const char* name, bool case_sensitive) const throw()
      {
        for(xmlAttributePtr attr = ((xmlElementPtr)node_)->attributes;
            attr != 0; attr = (xmlAttributePtr)attr->next)
        {
          bool match = case_sensitive ?
            strcmp((const char*)attr->name, name) == 0 :
            strcasecmp((const char*)attr->name, name) == 0;

          if(match)
          { 
            xmlNodePtr child = ((xmlNodePtr)attr)->children;

            if(child && child->content)
            {
              return (const char*)child->content;
            }
          }
        }

        return 0;
      }

      std::string
      Node::text(bool is_html) throw(El::Exception)
      {
        if(node_ == 0)
        {
          return "";
        }
        
        std::ostringstream ostr;
        El::LibXML::Traverser traverser;

        El::LibXML::TextBuilder::OutputType output_type = is_html ?
          El::LibXML::TextBuilder::OT_RENDERED_HTML :
          El::LibXML::TextBuilder::OT_TEXT;
          
        El::LibXML::TextBuilder builder(ostr, output_type);
        traverser.traverse(node_, builder);
        return ostr.str();
      }
      
      PyObject*
      Node::py_text(PyObject* args) throw(El::Exception)
      {
        if(doc_ == 0 || node_ == 0)
        {
          return PyString_FromString("");
        }
        
        unsigned char is_html = doc_->is_html();

        if(!PyArg_ParseTuple(args,
                             "|B:el.libxml.Node.text",
                             &is_html))
        {
          El::Python::handle_error("El::LibXML::Python::Node::py_text");
        }

        return PyString_FromString(text(is_html).c_str());        
      }

      PyObject*
      Node::find(const char* xpath) throw(El::Exception)
      {
        xmlDocPtr doc = doc_.in() == 0 ? 0 : doc_->document();
        
        if(doc == 0)
        {
          return El::Python::add_ref(Py_None);
        }
        
        El::Python::Sequence_var result;
        xmlXPathObjectPtr xpath_result = 0;

        try
        {
          xmlXPathContextPtr xpc = xmlXPathNewContext(doc);
      
          if(xpc == 0)
          {
            std::ostringstream ostr;
            ostr << "El::LibXML::Python::Node::find: "
              "xmlXPathNewContext failed";
            
            throw Exception(ostr.str());      
          }

          xpc->node = node_;
      
          xpath_result =
            xmlXPathEvalExpression((xmlChar*)xpath, xpc);
      
          xmlXPathFreeContext(xpc);
      
          if(xpath_result == 0)
          {
            std::ostringstream ostr;
            ostr << "El::LibXML::Python::Node::find: "
              "xmlXPathEvalExpression failed for '" << xpath << "'";
                
            throw Exception(ostr.str());
          }

          result = doc_->create_xpath_result(xpath_result);            
          xmlXPathFreeObject(xpath_result);
        }
        catch(const El::Exception& e)
        {
          if(xpath_result)
          {
            xmlXPathFreeObject(xpath_result);
          }

          throw;
        }

        return result.retn();
      }
      
      PyObject*
      Node::py_find(PyObject* args) throw(El::Exception)
      {
        char* xpath = 0;

        if(!PyArg_ParseTuple(args, "s:el.libxml.Node.find", &xpath))
        {
          El::Python::handle_error(
            "El::LibXML::Python::Node::py_find");
        }

        return find(xpath);
      }
    }
  }
}
