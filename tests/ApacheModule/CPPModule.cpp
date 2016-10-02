/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

#include <string>

#include <El/Apache/Module.hpp>

// Commented code prevents module unload in Apache after reading
// configuration phase.
// Actual for CentOS release 6.5 (Final), gcc 4.4.7

#if 0

struct A
{
  virtual int a()
  {
    static int x;
    return x;
  }
};

struct B : A
{
  B();  
};

B::B()
{
}

#endif

struct CPPTest
{
  CPPTest() { std::cerr << "CPPTest ++++++++++++++\n"; }
  ~CPPTest() { std::cerr << "~CPPTest ~~~~~~~~~~~~~~~\n"; }  
};

static CPPTest cpp_test;

struct CPPModuleConfig
{
  static El::Apache::ModuleRef mod_ref;
  
  unsigned long ul_val;
  std::string s_val;

  CPPModuleConfig() throw(El::Exception);

  CPPModuleConfig(CPPModuleConfig& base_conf, CPPModuleConfig& new_conf)
    throw(El::Exception);
};

class CPPModule : public El::Apache::Module<CPPModuleConfig>
{
public:
  EL_EXCEPTION(Exception, El::Apache::Module<CPPModuleConfig>::Exception);
  
public:
  CPPModule(El::Apache::ModuleRef& mod_ref) throw();
  ~CPPModule() throw();

  virtual void directive(const El::Directive& directive,
                         Config& config)
    throw(El::Exception);

  virtual void post_config(Config& conf, const char* caller)
    throw(El::Exception);
  
  virtual void child_init(Config& conf) throw(El::Exception);
  void child_cleanup(Config& conf) throw(El::Exception);
  virtual int handler(Context& context) throw(El::Exception);
};

EL_APACHE_MODULE_INSTANCE(CPPModule, test_module);

//
// CPPModuleConfig struct
//
CPPModuleConfig::CPPModuleConfig() throw(El::Exception)
    : ul_val(0)
{
}

CPPModuleConfig::CPPModuleConfig(CPPModuleConfig& base_conf,
                                 CPPModuleConfig& new_conf)
  throw(El::Exception)
    : ul_val(new_conf.ul_val + base_conf.ul_val),
      s_val(base_conf.s_val.empty() ? new_conf.s_val :
            (base_conf.s_val + "-" + new_conf.s_val))
{
}

//
// CPPModule class
//
CPPModule::CPPModule(El::Apache::ModuleRef& mod_ref) throw()
    : El::Apache::Module<CPPModuleConfig>(mod_ref,
                                          true,
                                          APR_HOOK_FIRST,
                                          "TestModuleLocations")
{
  std::cerr << "CPPModule::CPPModule\n";
  
  register_directive("TestArg",
                     "string;numeric:0,max",
                     "TestArg <name - string> <days - numeric:0,max>.",
                     0,
                     2);
}

CPPModule::~CPPModule() throw()
{
  std::cerr << "CPPModule::~CPPModule\n";
}

void
CPPModule::directive(const El::Directive& directive,
                     Config& config)
  throw(El::Exception)
{
  std::cerr << std::dec << "P: " << getpid() << " ";
  directive.dump(std::cerr);

  if(directive.name == "TestArg")
  {
    config.s_val = directive.arguments[0].string();
    config.ul_val = directive.arguments[1].numeric();
  }
  else
  {
    std::ostringstream ostr;
    ostr << "CPPModule::directive: unexpected directive '"
         << directive.name << "'";
    
    throw Exception(ostr.str());
  } 
}

void
CPPModule::child_init(Config& conf) throw(El::Exception)
{
  std::ostringstream ostr;
  ostr << std::dec << "CPPModule::child_init (" << getpid() << "): "
       << conf.ul_val << " " << conf.s_val << std::endl;

  std::cerr << ostr.str();
}

void
CPPModule::child_cleanup(Config& conf) throw(El::Exception)
{
  std::ostringstream ostr;
  ostr << std::dec << "CPPModule::child_cleanup (" << getpid() << "): "
       << conf.ul_val << " " << conf.s_val << std::endl;

  std::cerr << ostr.str();
}

void
CPPModule::post_config(Config& conf, const char* caller) throw(El::Exception)
{
  std::ostringstream ostr;
  ostr  << std::dec << "CPPModule::post_config (" << caller << ":"
        << getpid() << "): " << conf.ul_val << " " << conf.s_val << std::endl;

  std::cerr << ostr.str();
/*  
  std::cerr << std::dec << "CPPModule::post_config: " << conf.s_val
            << " " << conf.ul_val << std::endl;
*/
}

int
CPPModule::handler(Context& context) throw(El::Exception)
{
//  std::cerr << std::dec << "CPPModule::handler:\n";

  Config* conf = context.server_config();
  
//  std::cerr << "    server_config: " << conf->s_val << " " << conf->ul_val
//            << std::endl;
  
  conf = context.directory_config();
  
//  std::cerr << "    directory_config: " << conf->s_val << " "
//            << conf->ul_val << std::endl;
  
  conf = context.config();

/*  
  std::cerr << "    config: " << conf->s_val << " " << conf->ul_val
            << std::endl;

  std::cerr << "    unparsed_uri: " << request.unparsed_uri() << std::endl;
  std::cerr << "    uri: " << request.uri() << std::endl;
  std::cerr << "    args: " << request.args() << std::endl;
  std::cerr << "    remote_ip: " << request.remote_ip() << std::endl;
  std::cerr << "    method: " << request.method() << std::endl;
  std::cerr << "    method_number: " << request.method_number() << std::endl;
*/
  unsigned long count = 0;
  El::Apache::Request& request = context.request;
  
  const El::Net::HTTP::HeaderList& headers = request.in().headers();

  count += headers.size();

/*
  std::cerr << "    headers:\n";
    
  for(El::Net::HTTP::HeaderList::const_iterator it = headers.begin();
      it != headers.end(); it++)
  {
    std::cerr << "      " << it->name << ": " << it->value << std::endl;
  }
*/
  
  const El::Net::HTTP::CookieList& cookies = request.in().cookies();

  count += cookies.size();

  /*
  std::cerr << "    cookies:\n";
    
  for(El::Net::HTTP::CookieList::const_iterator it = cookies.begin();
      it != cookies.end(); it++)
  {
    std::cerr << "      " << it->name << ": " << it->value << std::endl;
  }
  */
  
  const El::Net::HTTP::ParamList& parameters = request.in().parameters();

  count += parameters.size();

/*
  std::cerr << "    parameters:\n";
    
  for(El::Net::HTTP::ParamList::const_iterator it = parameters.begin();
      it != parameters.end(); it++)
  {
    std::cerr << "      " << it->name << ": " << it->value << std::endl;
  }
*/
  
  request.out().send_header("CustomHeader", "AAAAAAA");
  request.out().content_type("text/html");
    
  std::ostream& out_stream = request.out().stream();

  out_stream << "<html><body>The result page: " << conf->ul_val << " "
             << conf->s_val << "</body></html>\n";

  return OK;
}
