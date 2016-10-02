/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   HTMLParserMain.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <sstream>
#include <fstream>
#include <iostream>
#include <list>
#include <fstream>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xpath.h>

#include <El/Exception.hpp>

#include <El/String/Manip.hpp>
#include <El/Guid.hpp>
#include <El/Net/HTTP/Session.hpp>
#include <El/Net/HTTP/StatusCodes.hpp>
#include <El/Net/HTTP/Headers.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ThreadPool.hpp>

#include <El/LibXML/Use.hpp>
#include <El/LibXML/HTMLParser.hpp>
#include <El/LibXML/Traverser.hpp>

namespace
{
  const char USAGE[] =
  "Usage: ElTestHTMLParser --uri=<file path or url> "
    "--xpath=<xpath expression>? --agent=<user-agent>? --encoding=<encoding>? "
    "--threads=<thread count>? --iterations=<iteration count>? "
    "--max-size=<max downlowd size>? --timeout=<request timeout in sec>? "
    "--redirects=<redirects count>? --dump=(xml|plain|render-html)?";

  const char USER_AGENT[] =
    "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)";
}

class Application : public virtual El::Service::Callback
{
public:    
  EL_EXCEPTION(Exception, El::ExceptionBase);
    
public:
  Application() throw() : dump_format_(DF_NONE), iterations_(1) {}
  virtual ~Application() throw() {}
  
  int run(int& argc, char** argv) throw();

private:
  virtual bool notify(El::Service::Event* event) throw(El::Exception);  

private:

  typedef ACE_Thread_Mutex  Mutex;
  typedef ACE_Guard<Mutex>  Guard;
  
  mutable Mutex lock_;

  enum DumpFormat
  {
    DF_NONE,
    DF_XML,
    DF_PLAIN,
    DF_RENDER_HTML
  };    

  std::string uri_;
  std::string encoding_;
  std::string xpath_;
  DumpFormat dump_format_;
  size_t iterations_;
  
  El::Service::ThreadPool_var thread_pool_;

  struct Parse : public El::Service::ThreadPool::ServiceEvent
  {
    Parse(Callback* callback, El::Service::Service* source)
      throw(El::Exception);
  };
};

inline
Application::Parse::Parse(Callback* callback,
                          El::Service::Service* source)
  throw(El::Exception)
    : El__Service__ThreadPool__ServiceEventBase(callback, source, false)
{ 
}

int
main(int argc, char** argv)
{
  Application app;
  return app.run(argc, argv);
}

int
Application::run(int& argc, char** argv) throw()
{
  El::LibXML::Use use;  
  std::string temp_file_path;
   
  try
  {
    // Checking params
    if (argc < 2)
    {
      std::ostringstream ostr;
      ostr << "Too few arguments\n" << USAGE;
      throw Exception(ostr.str());
    }

    size_t redirect_count = 10;
    std::string user_agent = USER_AGENT;
    size_t timeout = 60;
    size_t max_size = ULONG_MAX;
    size_t threads = 1;

    for(int i = 1; i < argc; i++)
    {
      const char* arg = argv[i];

      if(!strncmp(arg, "--uri=", 6))
      {
        uri_ = arg + 6;
      }
      else if(!strncmp(arg, "--encoding=", 11))
      {
        encoding_ = arg + 11;
      }
      else if(!strncmp(arg, "--xpath=", 8))
      {
        xpath_ = arg + 8;
      }
      else if(!strncmp(arg, "--agent=", 8))
      {
        user_agent = arg + 8;
      }
      else if(!strncmp(arg, "--timeout=", 10))
      {
        if(!El::String::Manip::numeric(arg + 10, timeout))
        {
          std::ostringstream ostr;
          ostr << "Invalid --timeout value\n" << USAGE;
          throw Exception(ostr.str());
        }        
      }
      else if(!strncmp(arg, "--redirects=", 12))
      {
        if(!El::String::Manip::numeric(arg + 12, redirect_count))
        {
          std::ostringstream ostr;
          ostr << "Invalid --redirects value\n" << USAGE;
          throw Exception(ostr.str());
        }        
      }
      else if(!strncmp(arg, "--max-size=", 11))
      {
        if(!El::String::Manip::numeric(arg + 11, max_size))
        {
          std::ostringstream ostr;
          ostr << "Invalid --max-size value\n" << USAGE;
          throw Exception(ostr.str());
        }        
      }
      else if(!strncmp(arg, "--threads=", 10))
      {
        if(!El::String::Manip::numeric(arg + 10, threads))
        {
          std::ostringstream ostr;
          ostr << "Invalid --threads value\n" << USAGE;
          throw Exception(ostr.str());
        }        
      }
      else if(!strncmp(arg, "--iterations=", 13))
      {
        if(!El::String::Manip::numeric(arg + 13, iterations_))
        {
          std::ostringstream ostr;
          ostr << "Invalid --iterations value\n" << USAGE;
          throw Exception(ostr.str());
        }        
      }
      else if(!strncmp(arg, "--dump=", 7))
      {
        const char* format = arg + 7;

        if(strcmp(format, "xml") == 0)
        {
          dump_format_ = DF_XML;
        }
        else if(strcmp(format, "plain") == 0)
        {
          dump_format_ = DF_PLAIN;
        }
        else if(strcmp(format, "render-html") == 0)
        {
          dump_format_ = DF_RENDER_HTML;
        }
      } 
    }

    if(!strncasecmp(uri_.c_str(), "http://", 7))
    {
      El::Net::HTTP::HeaderList headers;

      headers.add(El::Net::HTTP::HD_ACCEPT, "*/*");
      headers.add(El::Net::HTTP::HD_ACCEPT_ENCODING, "gzip,deflate");
      headers.add(El::Net::HTTP::HD_ACCEPT_LANGUAGE, "en-us");
      
      headers.add(El::Net::HTTP::HD_USER_AGENT, user_agent.c_str());
      
      El::Net::HTTP::Session session(uri_.c_str());

      ACE_Time_Value req_timeout(timeout);
      session.open(&req_timeout, &req_timeout, &req_timeout);
      
      session.send_request(El::Net::HTTP::GET,
                           El::Net::HTTP::ParamList(),
                           headers,
                           0,
                           0,
                           redirect_count);

      session.recv_response_status();
      
      if(session.status_code() != El::Net::HTTP::SC_OK)
      {
        std::ostringstream ostr;
        ostr << "Unexpected response status " << session.status_code()
             <<  " for " << uri_ << std::endl << USAGE;
        
        throw Exception(ostr.str());        
      }
      
      El::Net::HTTP::Header header;
      while(session.recv_response_header(header))
      {
        if(encoding_.empty() &&
           strcasecmp(header.name.c_str(),
                      El::Net::HTTP::HD_CONTENT_TYPE) == 0)
        {
          El::Net::HTTP::content_type(header.value.c_str(), encoding_);
        }
      }
 
      {
        El::Guid guid;
        guid.generate();
          
        std::ostringstream ostr;
        ostr << "/tmp/hpt-" << guid.string(El::Guid::GF_DENSE);
        temp_file_path = ostr.str();
      }

      session.save_body(temp_file_path.c_str(),
                        max_size,
                        0,
                        encoding_.c_str());
      
      uri_ = temp_file_path;
    }

    thread_pool_ = new El::Service::ThreadPool(this, "ThreadPool", threads);

    for(size_t i = 0; i < threads; ++i)
    {
      El::Service::ThreadPool::Task_var task =
        new Parse(this, thread_pool_.in());

      thread_pool_->execute(task.in());
    }

    thread_pool_->start();
    thread_pool_->wait();

    if(!temp_file_path.empty())
    {
      unlink(temp_file_path.c_str());
    }

    std::cerr << "Test finished\n";

    return 0;
  }
  catch(const El::Exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  catch(...)
  {
    std::cerr << "unknown exception caught.\n";
  }

  if(!temp_file_path.empty())
  {
    unlink(temp_file_path.c_str());
  }
  
  return -1;
}
 
bool
Application::notify(El::Service::Event* event)
  throw(El::Exception)
{
  El::Service::Error* error = dynamic_cast<El::Service::Error*>(event);
  
  if(error)
  {
    Guard guard(lock_);
    
    std::cerr << "Application::notify: error received. Description:\n"
              << *error << std::endl;
    
    return true;
  }

  Parse* parse_task = dynamic_cast<Parse*>(event);

  if(parse_task == 0)
  {
    Guard guard(lock_);
    
    std::cerr << "Application::notify: unknown event from "
              << event->source_name << " received. Description:\n"
              << *error << std::endl;

    return false;
  }
  
  {
    Guard guard(lock_);
    
    if(iterations_ == 0)
    {
      thread_pool_->stop();
      return true;
    }

    --iterations_;
  }

  std::ostringstream estr;
  std::ostringstream ostr;
  
  try
  {
    El::LibXML::ErrorRecorderHandler error_handler;
    El::LibXML::HTMLParser parser;

    htmlDocPtr doc = parser.parse_file(uri_.c_str(),
                                       encoding_.c_str(),
                                       &error_handler,
                                       HTML_PARSE_NONET);

    error_handler.dump(estr);
    
    if(dump_format_ != DF_NONE)
    {
      El::LibXML::Traverser traverser;
      
      El::LibXML::TextBuilder builder(
        ostr,
        dump_format_ == DF_XML ? El::LibXML::TextBuilder::OT_XML :
        (dump_format_ == DF_PLAIN ? El::LibXML::TextBuilder::OT_TEXT :
         El::LibXML::TextBuilder::OT_RENDERED_HTML));
      
      traverser.traverse_list(doc->children, builder);
    }

    std::string search_res;
    
    if(!xpath_.empty())
    {
      xmlXPathContextPtr xpc = xmlXPathNewContext(doc);
      
      if(xpc == 0)
      {
        std::ostringstream ostr;
        ostr << "xmlXPathNewContext failed\n" << USAGE;
        throw Exception(ostr.str());      
      }
      
      xmlXPathObjectPtr result =
        xmlXPathEvalExpression((xmlChar*)xpath_.c_str(), xpc);
      
      xmlXPathFreeContext(xpc);
      
      if(result == 0)
      {
        std::ostringstream ostr;
        ostr << "xmlXPathEvalExpression failed\n" << USAGE;
        throw Exception(ostr.str());      
      }
      
      xmlNodeSetPtr node_set = result->nodesetval;

      try
      {
        if(xmlXPathNodeSetIsEmpty(node_set))
        {
          estr << "Empty result set for XPath '" << xpath_ << "'\n";
        }
        else
        {
          for(int i = 0; i < node_set->nodeNr; ++i)
          {
            xmlNodePtr node = node_set->nodeTab[i];
            
            ostr << "\nXPath search result: ";
            
            El::LibXML::Traverser traverser;
            
            El::LibXML::TextBuilder builder(ostr,
                                            El::LibXML::TextBuilder::OT_XML);
            
            traverser.traverse(node, builder);
            
            ostr << std::endl;
          }
        }
        
      }
      catch(...)
      {        
        xmlXPathFreeObject(result);

        {
          Guard guard(lock_);
          std::cerr << estr.str();
          std::cout << ostr.str();
        }
        
        throw;
      }
      
      xmlXPathFreeObject(result);
    }

    {
      Guard guard(lock_);
      std::cerr << estr.str();
      std::cout << ostr.str();
    }

    thread_pool_->execute(parse_task);
  }
  catch(const El::Exception& e)
  {
    std::cerr << e.what() << std::endl;
    thread_pool_->stop();
  }
  
  return true;
}
