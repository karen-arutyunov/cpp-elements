/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/test/ApacheModule/Module.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#define AP_HAVE_DESIGNATED_INITIALIZER

#include <httpd/httpd.h>
#include <httpd/http_config.h>
/*
// Commented code prevents module unload in Apache after reading
// configuration phase.
// Actual for CentOS release 6.5 (Final), gcc 4.4.7

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
*/
struct test_module_config
{
  char value[100];
  
  test_module_config() { memset(value, 0, sizeof(value)); value[0] = '-'; };
};

inline
const char* str(const char* val)
{
  return val ? val : "null";
}

inline
std::ostream&
dump_uri(const apr_uri_t& uri, std::ostream& ostr)
{
  ostr << std::dec << str(uri.scheme) << "|" << str(uri.hostinfo)
       << "|" << str(uri.user) << "|" << str(uri.password) << "|"
       << str(uri.hostname) << "|" << str(uri.port_str) << "|"
       << str(uri.path) << "|" << str(uri.query) << "|" << str(uri.fragment)
       << "|" << uri.port;
  
  return ostr;
}

inline
std::ostream&
dump_array(const char** elts, unsigned long nelts, std::ostream& ostr)
{
  for(unsigned long i = 0; i < nelts; i++)
  {
    ostr << "    " << elts[i] << std::endl;
  }

  return ostr;
}

inline
std::ostream&
dump_connection(conn_rec* conn, std::ostream& ostr)
{
  if(conn == 0)
  {
    return ostr;
  }

  ostr << std::dec << "  connection:\n    remote_ip: "
       << str(conn->remote_ip)<< "\n    remote_host: "
       << str(conn->remote_host) << "\n    keepalive: " << conn->keepalive
       << "\n    keepalives: " << conn->keepalives << "\n    id: "
       << conn->id << std::endl;
  
  return ostr;
}

std::ostream& dump_server_list(server_rec* srv, std::ostream& ostr);
std::ostream& dump_server(server_rec* srv, std::ostream& ostr);
std::ostream& dump_request(request_rec* r, std::ostream& ostr);

static int
test_module_handler(request_rec *r)
{
  std::cerr << std::dec << "test_module_handler(" << getpid()
            << ") invoked, pool " << std::hex << r->pool << std::endl;

  dump_request(r, std::cerr);  

  return DECLINED;
}

static apr_status_t
test_module_config_cleanup(void* data)
{
  std::cerr << std::dec << "test_module_config_cleanup("
            << getpid() << ") invoked, pool " << std::hex << data << std::endl;
  
  return OK;
}

static int
test_module_config_holder_creator(request_rec *r, int lookup_uri)
{
  std::cerr << std::dec << "test_module_config_holder_creator(" << getpid()
            << ") invoked, pool " << std::hex << r->pool << std::endl;
  
  dump_request(r, std::cerr);  

  apr_pool_cleanup_register(r->pool,
                            r->pool,
                            test_module_config_cleanup,
                            0);

  return DECLINED;
}

static int
test_module_quick_handler(request_rec *r, int lookup_uri)
{
  std::cerr << std::dec << "test_module_quick_handler(" << getpid()
            << ") invoked, pool " << std::hex << r->pool << std::endl;
  
  dump_request(r, std::cerr);  

  return DECLINED;
}

static apr_status_t
test_module_child_cleanup(void* data)
{
  std::cerr << std::dec << "test_module_child_cleanup(" << getpid()
            << ") invoked, pool " << std::hex << data << std::endl;

  return OK;
}

static void
test_module_child_init(apr_pool_t *pchild, server_rec *s)
{
  std::cerr << std::dec << "test_module_child_init(" << getpid()
            << ") invoked, pool " << std::hex << pchild << ":\n";

  dump_server_list(s, std::cerr);

  apr_pool_cleanup_register(pchild,
                            pchild,
                            test_module_child_cleanup,
                            test_module_child_cleanup);
}

static int
test_module_pre_config(apr_pool_t *pconf,apr_pool_t *plog, apr_pool_t *ptemp)
{
  std::cerr << std::dec << "test_module_pre_config(" << getpid()
            << ") invoked, pool " << std::hex << pconf << std::endl;
  
  return OK;
}

static int
test_module_post_config(apr_pool_t *pconf, apr_pool_t *plog,
                        apr_pool_t *ptemp, server_rec *s)
{
  std::cerr << std::dec << "test_module_post_config(" << getpid()
            << ") invoked, pool " << std::hex << pconf << std::endl;
  
  dump_server_list(s, std::cerr);  
  return OK;
}
  
static void
test_module_register_hooks(apr_pool_t *p)
{
  std::cerr << std::dec << "test_module_register_hooks(" << getpid()
            << ") pool " << std::hex << p << std::endl;

  ap_hook_quick_handler(test_module_config_holder_creator, NULL, NULL, APR_HOOK_FIRST);
  ap_hook_quick_handler(test_module_quick_handler, NULL, NULL, APR_HOOK_FIRST);
  ap_hook_handler(test_module_handler, NULL, NULL, APR_HOOK_FIRST);
  ap_hook_child_init(test_module_child_init, NULL, NULL, APR_HOOK_FIRST);
  ap_hook_pre_config(test_module_pre_config, NULL, NULL, APR_HOOK_FIRST);  
  ap_hook_post_config(test_module_post_config, NULL, NULL, APR_HOOK_LAST);  
}

static const char*
parse_command(cmd_parms *parms, void *mconfig, const char *args);

typedef const char *(*raw_args_func)(
  cmd_parms *parms, void *mconfig, const char *args);

cmd_func
raw_args(raw_args_func func)
{
  cmd_func tmp;
  tmp.raw_args = func;
  return tmp;
}

command_rec test_module_commands[] =
{ 
  { "TestArg", raw_args(parse_command), 0, OR_ALL, RAW_ARGS, "this is TestArg" },
  { "TestModuleLocations", raw_args(parse_command), 0, OR_ALL, RAW_ARGS, "this is TestModuleLocations" },  
  { 0, raw_args(0), 0, 0, (cmd_how)0, 0 }
};

void*
test_module_create_dir_config(apr_pool_t *p, char *dir)
{
  test_module_config *cfg =
    (test_module_config*)apr_pcalloc(p, sizeof(test_module_config));

  strcat(cfg->value, "D");

  std::cerr << std::dec << "test_module_create_dir_config(" << getpid()
            << "): " << cfg->value << " ( " << str(dir) << " ), pool "
            << std::hex << p << std::endl;

  return cfg;
}

void*
test_module_merge_dir_config(apr_pool_t *p, void *base_conf, void *new_conf)
{
  test_module_config *base_cfg = (test_module_config*)base_conf;
  test_module_config *new_cfg = (test_module_config*)new_conf;

  test_module_config *cfg =
    (test_module_config*)apr_pcalloc(p, sizeof(test_module_config));

  cfg->value[0] = '{';
  
  strcat(cfg->value, base_cfg ? base_cfg->value : "0");
  strcat(cfg->value, "+");
  strcat(cfg->value, new_cfg ? new_cfg->value : "0");
  strcat(cfg->value, "}");
  
  std::cerr << std::dec << "test_module_merge_dir_config(" << getpid() << ") "
            << cfg->value << ", pool " << std::hex << p << std::endl;

  return cfg;
}

void*
test_module_create_server_config(apr_pool_t *p, server_rec *s)
{
  test_module_config *cfg =
    (test_module_config*)apr_pcalloc(p, sizeof(test_module_config));

  strcat(cfg->value, "S");

  std::cerr << std::dec << "test_module_create_server_config(" << getpid()
            << "): " << cfg->value << ", pool " << std::hex << p << std::endl;

  return cfg;
}

void*
test_module_merge_server_config(apr_pool_t *p, void *base_conf, void *new_conf)
{
  test_module_config *base_cfg = (test_module_config*)base_conf;
  test_module_config *new_cfg = (test_module_config*)new_conf;
  

  test_module_config *cfg =
    (test_module_config*)apr_pcalloc(p, sizeof(test_module_config));

  cfg->value[0] = '{';

  strcat(cfg->value, base_cfg ? base_cfg->value : "0");
  strcat(cfg->value, "+");
  strcat(cfg->value, new_cfg ? new_cfg->value : "0");
  strcat(cfg->value, "}");

  std::cerr << std::dec << "test_module_merge_server_config(" << getpid()
            << "): " << cfg->value << ", pool " << std::hex << p << std::endl;

  return cfg;
}

module AP_MODULE_DECLARE_DATA test_module =
{
  // Only one callback function is provided.  Real
  // modules will need to declare callback functions for
  // server/directory configuration, configuration merging
  // and other tasks.
  STANDARD20_MODULE_STUFF,
  test_module_create_dir_config,
  test_module_merge_dir_config,
  test_module_create_server_config,
  test_module_merge_server_config,
  test_module_commands,
  test_module_register_hooks
};

static const char*
parse_command(cmd_parms *parms, void *mconfig, const char *args)
{
  test_module_config* conf = parms->path != 0 ?
    (test_module_config*)mconfig :
    (test_module_config*)ap_get_module_config(parms->server->module_config,
                                              &test_module);
    
  std::cerr << std::dec << parms->cmd->name << ": " << args;

  if(conf)
  {
    std::cerr << " | mconfig: " << conf->value;

    char tmp[2];
    tmp[0] = args[0];
    tmp[1] = '\0';
    
    strcat(conf->value, tmp);
  }

  std::cerr << std::endl;
  
  return 0;
}

std::ostream&
dump_request(request_rec* r, std::ostream& ostr)
{
  test_module_config* dir_conf =
    (test_module_config*)ap_get_module_config(r->per_dir_config,
                                              &test_module);
  ostr << std::dec << "  the_request: "
       << str(r->the_request) << "\n  protocol: " << str(r->protocol)
       << "\n  proto_num: " << r->proto_num << "\n  hostname: "
       << str(r->hostname) << "\n  status_line: " << str(r->status_line)
       << "\n  status: " << r->status << "\n  method: " << str(r->method)
       << "\n  method_number: " << r->method_number << "\n  content_type: "
       << str(r->content_type) << "\n  handler: " << str(r->handler)
       << "\n  content_encoding: " << str(r->content_encoding)
       << "\n  unparsed_uri: " << str(r->unparsed_uri) << "\n  uri: "
       << str(r->uri) << "\n  filename: " << str(r->filename)
       << "\n  canonical_filename: " << str(r->canonical_filename)
       << "\n  path_info: " << str(r->path_info) << "\n  args: "
       << str(r->args)
       << "\n  dir conf: " << (dir_conf ? dir_conf->value : "null")
       << "\n  parsed_uri: ";

  dump_uri(r->parsed_uri, ostr) << std::endl;

  if(r->content_languages)
  {
    ostr << "\n  content_languages:\n";

    dump_array((const char **)r->content_languages->elts,
               r->content_languages->nelts,
               ostr);
  }

  dump_connection(r->connection, ostr);

  dump_server(r->server, ostr);
  
  ostr << std::endl;

  // struct ap_conf_vector_t *per_dir_config;
  // struct ap_conf_vector_t *request_config;

  return ostr;
}

std::ostream&
dump_server(server_rec* srv, std::ostream& ostr)
{
  if(srv == 0)
  {
    return ostr;
  }

  test_module_config* srv_conf =
    (test_module_config*)ap_get_module_config(srv->module_config,
                                              &test_module);
  
  ostr << std::dec << "  server:\n    defn_name: " << str(srv->defn_name)
       << "\n    defn_line_number: " << srv->defn_line_number
       << "\n    server_admin: " << str(srv->server_admin)
       << "\n    server_hostname: " << str(srv->server_hostname)
       << "\n    port: " << srv->port << "\n    error_fname: "
       << str(srv->error_fname) << "\n    loglevel: " << srv->loglevel
       << "\n    is_virtual: " << srv->is_virtual
       << "\n    srv conf: " << (srv_conf ? srv_conf->value : "null")
       << std::endl;

  return ostr;
}

std::ostream&
dump_server_list(server_rec* srv, std::ostream& ostr)
{
  unsigned long num = 1;
  for(server_rec* i = srv; i != 0; i = i->next, num++)
  {
    ostr << "  Server " << std::dec << num << ":\n";
    dump_server(i, ostr);
  }
  
  return ostr;
}

