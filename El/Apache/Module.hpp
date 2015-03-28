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
 * @file Elements/El/Apache/Module.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_APACHE_MODULE_HPP_
#define _ELEMENTS_EL_APACHE_MODULE_HPP_

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include <string>
#include <list>
#include <vector>
#include <sstream>
#include <memory>
#include <set>

#include <ext/hash_map>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#define AP_HAVE_DESIGNATED_INITIALIZER

#include <apr_errno.h>
#include <apr_strings.h>
#include <httpd/httpd.h>
#include <httpd/http_config.h>
#include <httpd/http_protocol.h>
#include <httpd/http_log.h>
#include <httpd/mpm_common.h>

#include <El/Exception.hpp>
#include <El/Hash/Hash.hpp>
#include <El/Directive.hpp>
#include <El/String/Manip.hpp>

#include <El/Apache/Exception.hpp>
#include <El/Apache/Request.hpp>

namespace El
{
  namespace Apache
  {
    class Hooks
    {
    public:
      virtual ~Hooks() throw();
      
      virtual void child_init_(apr_pool_t *pchild, server_rec *s) throw() = 0;
      
      virtual int handler_(request_rec *r) throw() = 0;
      
      virtual void* create_server_config_(apr_pool_t *p,
                                          server_rec *s) throw() = 0;
      
      virtual void* merge_server_config_(apr_pool_t *p,
                                         void *base_conf,
                                         void *new_conf)
        throw() = 0;
      
      virtual void* create_dir_config_(apr_pool_t *p, char *dir) throw() = 0;
      
      virtual void* merge_dir_config_(apr_pool_t *p,
                                      void *base_conf,
                                      void *new_conf)
        throw() = 0;
      
      virtual int post_config_(apr_pool_t *pconf,
                               apr_pool_t *plog,
                               apr_pool_t *ptemp,
                               server_rec *s)
        throw() = 0;

      virtual void register_hooks_(apr_pool_t *p) throw() = 0;
      
      virtual void config_cleanup_(apr_pool_t* pool) throw() = 0;

      virtual const char* parse_raw_args_(cmd_parms *parms,
                                          void *mconfig,
                                          const char *args)
        throw() = 0;

      virtual void pre_config_() throw(El::Exception) = 0;
    };
    
    struct ModuleRef
    {
      std::string id;
      module& ap_module;
      Hooks* hooks;

      ModuleRef(const char* id_val, module& ap_module_val) throw();
    };
    
    template<typename CONFIG>
    class Module : public Hooks
    {
    public:
      EL_EXCEPTION(Exception, El::Apache::Exception);
      
    public:
      typedef CONFIG Config;
      
      //
      // handler_quick - if true, then handler will be called at very early
      //                 stage of Apache process handling; as a tradeoff pretty
      //                 many fileds of server_rec will not be defined.
      // handler_order - specify position in request processing chain;
      //                 available values are APR_HOOK_FIRST,
      //                 APR_HOOK_MIDDLE, APR_HOOK_LAST
      // handler_locations_directive - if not 0 specifies locations where
      //                 handler to be invoked. Each location can have * at
      //                 the end. There are 2 forms: /aaa/bbb/* and /aaa/bbb*.
      //                 The first one mean "for all sub-locations", the second
      //                 mean "for all locations with specific prefix".
      //
      Module(ModuleRef& mod_ref,
             bool handler_quick = true,
             int handler_order = APR_HOOK_FIRST,
             const char* handler_locations_directive = 0) throw();
      
      virtual ~Module() throw();

      ModuleRef& mod_ref() throw();
      
      struct Context
      {
        Module* module;
        Request request;

        Context(request_rec* ap_req, Module* mod) throw();

        Config* server_config() throw(El::Exception);
        Config* directory_config() throw(El::Exception);

        // Merges server and directory configurations
        Config* config() throw(El::Exception);

      private:
        request_rec* ap_request_;
        std::auto_ptr<CONFIG> config_;
      };

    protected:

      friend class Request;

      //
      // Valid values for type parameter as described in
      // Elements/El/Directive.hpp for El::Directive::TypeArray::parse method.
      // If NULL, then no type check will be made.
      // Should be called from inside module implementation class constructor.
      //
      // min_occurs_val, max_occurs_val specifies valid number of times
      // directive should appear in server-level configuration of the module.
      // If both values equal 0, then no check will be performed.
      // Please note that no validation will be done for directory-level
      // directives. This is because directory-level configurations are merged
      // after Apache configuration phase, so all required checks should be
      // implemented by module itself.
      // If module directives appear in both server and virtual servers areas,
      // then validation will be performed for merged configurations only.
      //
      void register_directive(const char* directive,
                              const char* type,
                              const char* usage_msg,
                              unsigned long min_occurs_val = 0,
                              unsigned long max_occurs_val = 0,
                              int req_override = OR_ALL)
        throw();

      void register_handler(const char* handler)
        throw(Exception, El::Exception);

      //
      // Overrides
      //

      // Called when directive registered by module is parsed
      virtual void directive(const Directive& directive,
                             Config& config)
        throw(El::Exception);

      // Called before configuraion file is read.
      // Happens in root apache process.
      virtual void pre_config() throw(El::Exception);
      
      // Called after configuraion file is read.
      // Happens in root apache process.
      virtual void post_config() throw(El::Exception);

      // Called:
      // 1) for each created module configuration object
      //    after configuration file is read.
      //    Happens in root apache process;
      // 2) before returning merged during request handling configuration
      //    to caller.
      virtual void post_config(Config& conf, const char* caller)
        throw(El::Exception);
      
      // Called after worker process is initialized.
      virtual void child_init() throw(El::Exception);

      // Called after worker process is initialized for each created
      // module configuration object.
      virtual void child_init(Config& conf) throw(El::Exception);

      // Called right before worker process is terminated.
      virtual void child_cleanup() throw(El::Exception);
      
      // Called right before worker process is terminated for each created
      // module configuration object (right before child_cleanup() invocation).
      virtual void child_cleanup(Config& conf) throw(El::Exception);

      // Called to handle request.
      virtual int handler(Context& context) throw(El::Exception);

    private:

      //
      // Static hook methods
      //
      
      static void* create_dir_config_static(apr_pool_t *p, char *dir);
      
      static void* merge_dir_config_static(apr_pool_t *p,
                                           void *base_conf,
                                           void *new_conf);

      static void* create_server_config_static(apr_pool_t *p, server_rec *s);

      static void* merge_server_config_static(apr_pool_t *p,
                                              void *base_conf,
                                              void *new_conf);
      
      static int pre_config_static(apr_pool_t *pconf,
                                   apr_pool_t *plog,
                                   apr_pool_t *ptemp);
      
      static int post_config_static(apr_pool_t *pconf,
                                    apr_pool_t *plog,
                                    apr_pool_t *ptemp,
                                    server_rec *s);

      static void register_hooks_static(apr_pool_t *p);
      static void child_init_static(apr_pool_t *pchild, server_rec *s);
      
      static const char* parse_raw_args_static(cmd_parms *parms,
                                               void *mconfig,
                                               const char *args);

      static apr_status_t child_cleanup_static(void* data);
      static int handler_static(request_rec *r);
      static int config_holder_creator_static(request_rec *r, int lookup_uri);
      static int quick_handler_static(request_rec *r, int lookup_uri);
      static apr_status_t config_cleanup_static(void* data);
      static apr_status_t config_cleanup_empty(void* data);
      
      //
      // Hook methods implementation
      //
      
      virtual void child_init_(apr_pool_t *pchild, server_rec *s) throw();
      virtual void child_cleanup_() throw(El::Exception);
      
      virtual int handler_(request_rec *r) throw();
      
      virtual void* create_server_config_(apr_pool_t *p,
                                          server_rec *s) throw();
      
      virtual void* merge_server_config_(apr_pool_t *p,
                                         void *base_conf,
                                         void *new_conf)
        throw();
      
      virtual void* create_dir_config_(apr_pool_t *p, char *dir) throw();
      
      virtual void* merge_dir_config_(apr_pool_t *p,
                                      void *base_conf,
                                      void *new_conf)
        throw();
      
      virtual int post_config_(apr_pool_t *pconf,
                               apr_pool_t *plog,
                               apr_pool_t *ptemp,
                               server_rec *s)
        throw();

      virtual void register_hooks_(apr_pool_t *p) throw();
      
      virtual void config_cleanup_(apr_pool_t* pool) throw();      

      virtual const char* parse_raw_args_(cmd_parms *parms,
                                          void *mconfig,
                                          const char *args)
        throw();

      virtual void pre_config_() throw(El::Exception);
      
      //
      // Other
      //
      
      void cleanup_configured() throw();
      void validate_directive_presense() throw(El::Exception);

      typedef __gnu_cxx::hash_map<std::string, unsigned long, El::Hash::String>
      DirectiveCounterTable;

      struct ConfigHolder
      {
        enum ConfigOrigin
        {
          CO_SERVER,
          CO_DIRECTORY,
          CO_MIX,
          CO_SERVER_MIX,
          CO_DIRECTORY_MIX
        };
        
        std::auto_ptr<Config> conf;
        bool empty;
        ConfigOrigin origin;
        bool post_configured;
        DirectiveCounterTable directive_count;

        ConfigHolder(Config* conf_val, bool empty_val, ConfigOrigin origin_val)
          throw();

        bool server_origin() const throw();
        bool directory_origin() const throw();

        static ConfigOrigin mixed_origin(const ConfigHolder* base_cf,
                                         const ConfigHolder* new_cf)
          throw();

      private:
        ConfigHolder(const ConfigHolder&);
        void operator=(const ConfigHolder&);
      };
      
      struct ConfigHolderPtr
      {
        ConfigHolder* holder;
      };
      
      void save_config(apr_pool_t* p, ConfigHolderPtr* holder_ptr) throw();

    protected:

      class ConfigHolderPtrList : public std::list<ConfigHolderPtr>
      {
      public:
        /*
        ConfigHolderPtrList() throw()
        {
          std::ostringstream ostr;
          ostr << std::dec << "ConfigHolderPtrList: ("
               << getpid() << ")" << std::endl;
          
          std::cerr << ostr.str();
        }
        */
        
        ~ConfigHolderPtrList() throw();
        
        void clear() throw();
      };

      struct DirectiveDef
      {
        unsigned long min_occurs;
        unsigned long max_occurs;
        std::string usage;
        
        Directive::TypeArray types;
      };

      typedef __gnu_cxx::hash_map<std::string,
                                  DirectiveDef,
                                  El::Hash::String>
      DirectiveDefTable;
      
      typedef __gnu_cxx::hash_map<apr_pool_t*,
                                  ConfigHolderPtrList,
                                  El::Hash::Numeric<apr_pool_t*> >
      RequestConfigTable;


      enum MatchType
      {
        MT_EXACT = 0x1,
        MT_PREFIX = 0x2
      };
      
      typedef __gnu_cxx::hash_map<std::string, unsigned long, El::Hash::String>
      LocationTable;

      ModuleRef& mod_ref_;
      bool handler_quick_;
      int handler_order_;
      std::string handler_locations_directive_;
      
      bool initialized_;
      bool is_worker_;
      bool post_configured_;
      
      unsigned long cmds_reserved;
      unsigned long cmds_count;

      DirectiveDefTable directive_definitions_;
      ConfigHolderPtrList config_holders_;
      LocationTable locations_;

      typedef std::set<std::string> HandlerSet;

      HandlerSet handlers_;
      
      typedef ACE_Thread_Mutex RCT_Mutex;
      typedef ACE_Guard<RCT_Mutex> RCT_Guard;

      RCT_Mutex rct_lock_;
      RequestConfigTable request_config_table_;

      std::string parse_args_last_error_;

    private:
      Module(const Module&);
      void operator=(const Module&);      
    };
    
  }
}

#define EL_APACHE_MODULE_INSTANCE(ModuleClass, module_name) \
  module module_name; \
  El::Apache::ModuleRef ModuleClass::Config::mod_ref(#module_name, module_name); \
  ModuleClass module_name##_impl(ModuleClass::Config::mod_ref);

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Apache
  {
    //
    // Hook class
    //

    inline
    Hooks::~Hooks() throw()
    {
    }

    //
    // ModuleRef struct
    //
    
    inline
    ModuleRef::ModuleRef(const char* id_val,
                         module& ap_module_val)
      throw()
        : id(id_val),
          ap_module(ap_module_val),
          hooks(0)
    {
    }
    
    //
    // Module<CONFIG>::Context class
    //
    template<typename CONFIG>
    Module<CONFIG>::Context::Context(request_rec* ap_req, Module* mod) throw()
        : module(mod),
          request(ap_req),
          ap_request_(ap_req)
    {
    }                          

    template<typename CONFIG>
    CONFIG*
    Module<CONFIG>::Context::server_config() throw(El::Exception)
    {
      ConfigHolderPtr* holder_ptr =
        (ConfigHolderPtr*)ap_get_module_config(
          ap_request_->server->module_config,
          &module->mod_ref().ap_module);
      
      ConfigHolder* holder = holder_ptr->holder;

      if(!holder->post_configured)
      {
        module->post_config(*holder->conf, "server_config");
        holder->post_configured = true;
      }
      
      return holder->conf.get();
    }
    
    template<typename CONFIG>
    CONFIG*
    Module<CONFIG>::Context::directory_config() throw(El::Exception)
    {
      ConfigHolderPtr* holder_ptr =
        (ConfigHolderPtr*)ap_get_module_config(ap_request_->per_dir_config,
                                               &module->mod_ref().ap_module);

      ConfigHolder* holder = holder_ptr->holder;

      if(!holder->post_configured)
      {
        module->post_config(*holder->conf, "directory_config");
        holder->post_configured = true;
      }
      
      return holder->conf.get();
    }

    template<typename CONFIG>
    CONFIG*
    Module<CONFIG>::Context::config() throw(El::Exception)
    {
      if(module->handler_quick_)
      {
        return server_config();
      }

      if(config_.get() == 0)
      {
        config_.reset(new Config(*server_config(), *directory_config()));
        module->post_config(*config_, "config");
      }
      
      return config_.get();
    }

    //
    // Module<CONFIG>::ConfigHolder class
    //
    template<typename CONFIG>
    Module<CONFIG>::ConfigHolder::ConfigHolder(Config* conf_val,
                                               bool empty_val,
                                               ConfigOrigin origin_val) throw()
        : conf(conf_val),
          empty(empty_val),
          origin(origin_val),
          post_configured(false)
    {
    }

    template<typename CONFIG>
    bool
    Module<CONFIG>::ConfigHolder::server_origin()  const throw()
    {
      return origin == CO_SERVER || origin == CO_SERVER_MIX;
    }

    template<typename CONFIG>
    bool
    Module<CONFIG>::ConfigHolder::directory_origin() const throw()
    {
      return origin == CO_DIRECTORY || origin == CO_DIRECTORY_MIX;
    }

    template<typename CONFIG>
    typename Module<CONFIG>::ConfigHolder::ConfigOrigin
    Module<CONFIG>::ConfigHolder::mixed_origin(const ConfigHolder* base_cf,
                                               const ConfigHolder* new_cf)
      throw()
    {
      if(base_cf->server_origin() && new_cf->server_origin())
      {
        return CO_SERVER_MIX;
      }

      if(base_cf->directory_origin() && new_cf->directory_origin())
      {
        return CO_DIRECTORY_MIX;
      }

      return CO_MIX;
    }
    
    //
    // Module<CONFIG>::ConfigHolderPtrList class
    //
    template<typename CONFIG>
    Module<CONFIG>::ConfigHolderPtrList::~ConfigHolderPtrList() throw()
    {
/*      
      std::ostringstream ostr;
      ostr << "~ConfigHolderPtrList: " << this->size()
           << " (" << getpid() << ")" << std::endl;
*/    
      clear();
      
//      std::cerr << ostr.str() << std::endl;
    }

    template<typename CONFIG>
    void
    Module<CONFIG>::ConfigHolderPtrList::clear() throw()
    {
      /*
      std::ostringstream ostr;
      ostr << "ConfigHolderPtrList::clear: " << this->size()
           << " (" << getpid() << ")" << std::endl;
      */
      
      for(typename std::list<ConfigHolderPtr>::iterator it =
            std::list<ConfigHolderPtr>::begin();
          it != std::list<ConfigHolderPtr>::end(); it++)
      {
        delete it->holder;
      }

      std::list<ConfigHolderPtr>::clear();
      
//      std::cerr << ostr.str() << std::endl;
    }

    //
    // Module<CONFIG> class template
    //
    template<typename CONFIG>
    Module<CONFIG>::Module(ModuleRef& mod_ref,
                           bool handler_quick,
                           int handler_order,
                           const char* handler_locations_directive) throw()
        : mod_ref_(mod_ref),
          handler_quick_(handler_quick),
          handler_order_(handler_order),
          handler_locations_directive_(handler_locations_directive ?
                                       handler_locations_directive : ""),
          initialized_(false),
          is_worker_(false),
          post_configured_(false),
          cmds_reserved(0),
          cmds_count(0)
    {
      mod_ref.hooks = this;
      
      mod_ref.ap_module.version = MODULE_MAGIC_NUMBER_MAJOR;
      mod_ref.ap_module.minor_version = MODULE_MAGIC_NUMBER_MINOR;
      mod_ref.ap_module.module_index = -1;
      mod_ref.ap_module.name = __FILE__;
      mod_ref.ap_module.dynamic_load_handle = 0;
      mod_ref.ap_module.next = 0;
      mod_ref.ap_module.magic = MODULE_MAGIC_COOKIE;
      mod_ref.ap_module.rewrite_args = 0;
      
      mod_ref.ap_module.create_dir_config = create_dir_config_static;
      mod_ref.ap_module.merge_dir_config = merge_dir_config_static;
      
      mod_ref.ap_module.create_server_config = create_server_config_static;
      mod_ref.ap_module.merge_server_config = merge_server_config_static;
      
      mod_ref.ap_module.cmds = 0;
      mod_ref.ap_module.register_hooks = register_hooks_static;

      if(handler_locations_directive)
      {
        std::ostringstream ostr;
        ostr << handler_locations_directive << " should be followed with a "
          "list of locations to be handled by " << mod_ref.id << " module";
          
        register_directive(handler_locations_directive,
                           "string:nws;...",
                           ostr.str().c_str());
      }
    }
    
    template<typename CONFIG>
    Module<CONFIG>::~Module() throw()
    {
      for(command_rec *cmd(const_cast<command_rec*>(mod_ref_.ap_module.cmds)),
            *end(cmd + cmds_count); cmd != end; ++cmd)
      {
        assert(cmd->name != 0);
        assert(cmd->errmsg != 0);
        
        free(const_cast<char*>(cmd->name));
        free(const_cast<char*>(cmd->errmsg));
      }
  
      delete [] mod_ref_.ap_module.cmds;
      mod_ref_.ap_module.cmds = 0;
    }

    template<typename CONFIG>
    void
    Module<CONFIG>::cleanup_configured() throw()
    {
      if(post_configured_)
      {
        /*
        std::ostringstream ostr;
        ostr << std::dec << "El::Apache::Module<CONFIG>::cleanup_configured("
             << getpid() << ")\n";
        
        std::cerr << ostr.str();
        */
        
        config_holders_.clear();
        post_configured_ = false;
      }
    }      
    
    template<typename CONFIG>
    ModuleRef&
    Module<CONFIG>::mod_ref() throw()
    {
      return mod_ref_;
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::directive(const Directive& directive,
                              Config& config)
      throw(El::Exception)
    {
    }

    template<typename CONFIG>
    void
    Module<CONFIG>::pre_config() throw(El::Exception)
    {
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::post_config() throw(El::Exception)
    {
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::post_config(Config& conf, const char* caller)
      throw(El::Exception)
    {
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::child_init() throw(El::Exception)
    {
    }

    template<typename CONFIG>
    void
    Module<CONFIG>::child_init(Config& conf) throw(El::Exception)
    {
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::child_cleanup(Config& conf) throw(El::Exception)
    {
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::child_cleanup() throw(El::Exception)
    {
    }
    
    template<typename CONFIG>
    int
    Module<CONFIG>::handler(Context& context) throw(El::Exception)
    {
      return DECLINED;
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::register_handler(const char* handler)
      throw(Exception, El::Exception)
    {
      if(handler_quick_)
      {
        std::ostringstream ostr;
        ostr << "El::Apache::Module<CONFIG>::register_handler: "
             << "to support AddHandler, SetHandler, ... directives "
          "module should not be a \"quick\" handler";

        throw Exception(ostr.str());
      }
      
      std::string lowered;
      El::String::Manip::to_lower(handler, lowered);
      
      handlers_.insert(lowered);
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::register_directive(const char* directive,
                                       const char* type,
                                       const char* usage_msg,
                                       unsigned long min_occurs_val,
                                       unsigned long max_occurs_val,
                                       int req_override) throw()
    {
      try
      {
        if(cmds_reserved == cmds_count)
        {
          if(mod_ref_.ap_module.cmds == 0)
          {
            cmds_reserved = 10;
            mod_ref_.ap_module.cmds = new command_rec[cmds_reserved + 1];
            memset((command_rec*)mod_ref_.ap_module.cmds,
                   0,
                   sizeof(mod_ref_.ap_module.cmds[0]) * (cmds_reserved + 1));
          }
          else
          {
            unsigned long new_cmds_reserved = cmds_reserved + 10;
            command_rec* new_cmds = new command_rec[new_cmds_reserved + 1];
          
            memcpy(new_cmds,
                   mod_ref_.ap_module.cmds,
                   sizeof(new_cmds[0]) * cmds_reserved);
          
            memset(new_cmds + cmds_reserved, 0, sizeof(new_cmds[0]) * 11);

            delete [] mod_ref_.ap_module.cmds;
            mod_ref_.ap_module.cmds = new_cmds;
            cmds_reserved = new_cmds_reserved;
          }
        }

        command_rec& rec = (command_rec&)mod_ref_.ap_module.cmds[cmds_count++];

        typename DirectiveDefTable::iterator it =
          directive_definitions_.insert(
            std::make_pair(directive, DirectiveDef())).first;

        DirectiveDef& dir_def = it->second;

        dir_def.min_occurs = min_occurs_val;
        dir_def.max_occurs = max_occurs_val;
          
        if(usage_msg)
        {
          dir_def.usage = usage_msg;
        }

        rec.name = strdup(it->first.c_str());
        //SHIT
        rec.func.raw_args = parse_raw_args_static;
        rec.cmd_data = this;
        rec.req_override = req_override;
        rec.args_how = RAW_ARGS;
        rec.errmsg = strdup(dir_def.usage.c_str());

        if(type && *type != '\0')
        {
          it->second.types.parse(type);
        }
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "Registration directive " << directive
             << " failed for module " << mod_ref_.id << ". Reason: "
             << e << std::endl;
        
        std::cerr << ostr.str();
        
        exit(-1);
      }
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::save_config(apr_pool_t* p, ConfigHolderPtr* holder_ptr)
      throw()
    {
      if(is_worker_)
      {
        if(handler_quick_)
        {
          delete holder_ptr->holder;
          
          ap_log_perror(EL_APACHE_LOG_MARK,
                        APLOG_EMERG,
                        0,
                        p,
                        "%s",
                        "El::Apache::Module<CONFIG>::save_config: "
                        "shouldn't be here in \"quick handler\" mode");
        }
        else
        {
          RCT_Guard guard(rct_lock_);
          request_config_table_[p].push_back(*holder_ptr);
        }
      }
      else
      {
        config_holders_.push_back(*holder_ptr);
/*
        std::ostringstream ostr;
        ostr << std::dec << "El::Apache::Module<CONFIG>::save_config("
             << getpid() << "):  configs " << config_holders_.size()
             << std::endl;
        
        std::cerr << ostr.str();
*/      
      }
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::child_init_static(apr_pool_t *pchild, server_rec *s)
    {
/*      
      {
        std::ostringstream ostr;
        ostr << std::dec << "El::Apache::Module<CONFIG>::child_init_static("
             << getpid() << "):  pool " << std::hex
             << pchild << std::endl;
        
        std::cerr << ostr.str();
      }
*/

      Config::mod_ref.hooks->child_init_(pchild, s);
    }
    
    template<typename CONFIG>
    const char*
    Module<CONFIG>::parse_raw_args_static(cmd_parms *parms,
                                          void *mconfig,
                                          const char *args)
    {
      Module* mod = (Module*)parms->cmd->cmd_data;
      return mod->parse_raw_args_(parms, mconfig, args);
    }

    template<typename CONFIG>
    int
    Module<CONFIG>::handler_static(request_rec *r)
    {
/*      
      {
        std::ostringstream ostr;
        ostr << std::dec << "El::Apache::Module<CONFIG>::handler_static("
             << getpid() << "):  pool " << std::hex
             << r->pool << std::endl;
        
        std::cerr << ostr.str();
      }
*/ 
      return Config::mod_ref.hooks->handler_(r);
    }

    template<typename CONFIG>
    int
    Module<CONFIG>::quick_handler_static(request_rec *r, int lookup_uri)
    {
/*      
      {
        std::ostringstream ostr;
        ostr << std::dec << "El::Apache::Module<CONFIG>::"
          "quick_handler_static(" << getpid() << "):  pool "
             << std::hex << r->pool << std::endl;
        
        std::cerr << ostr.str();
      }
*/ 
      return Config::mod_ref.hooks->handler_(r);
    }

    template<typename CONFIG>
    const char*
    Module<CONFIG>::parse_raw_args_(cmd_parms *parms,
                                    void *mconfig,
                                    const char *args)
      throw()
    {
//      std::cerr << "parse_raw_args_: " << parms->cmd->name << " " << args
//                << std::endl;
      
//      static std::string last_error;
//      last_error.erase();
      
      parse_args_last_error_.clear();
      
      try
      {
        const char* dname = parms->cmd->name;
        
        typename DirectiveDefTable::const_iterator it =
          directive_definitions_.find(dname);

        if(it == directive_definitions_.end())
        {
          std::ostringstream ostr;
          ostr << "El::Apache::Module<CONFIG>::parse_raw_args_: "
            "unexpected directive '" << dname << "' for module "
               << Config::mod_ref.id;

          throw Exception(ostr.str());
        }

        El::Directive d(dname,
                        args,
                        it->second.types.empty() ? 0 : &it->second.types);

        if(handler_locations_directive_ == dname)
        {
          const El::Directive::ArgArray& args = d.arguments;
          
          for(El::Directive::ArgArray::const_iterator it = args.begin();
              it != args.end(); it++)
          {
            MatchType type = MT_EXACT;
            
            std::string location = it->string();
            unsigned long len = location.length();
            
            if(len > 1 && location[len - 1] == '*')
            {
              type = MT_PREFIX;
              location.resize(len - 1);
            }
              
            LocationTable::iterator lit = locations_.find(location);

            if(lit == locations_.end())
            {
              locations_[location] = type;
            }
            else
            {
              lit->second |= type;
            }
          }
        }
        else
        {
          ConfigHolderPtr* holder_ptr = parms->path != 0 ?
            (ConfigHolderPtr*)mconfig :
            (ConfigHolderPtr*)ap_get_module_config(
              parms->server->module_config,
              &Config::mod_ref.ap_module);

          if(holder_ptr->holder->server_origin())
          {
            DirectiveCounterTable& directive_count =
              holder_ptr->holder->directive_count;

            DirectiveCounterTable::iterator it = directive_count.find(dname);
          
            if(it == directive_count.end())
            {
              directive_count[dname] = 1;
            }
            else
            {
              it->second++;
            }
          }
          
          directive(d, *holder_ptr->holder->conf);
        }
        
        return 0;
      }
      catch(const El::Exception& e)
      {
/*        
        last_error = e.what();

        if(parms->cmd->errmsg)
        {
          last_error += ". Usage: ";
          last_error += parms->cmd->errmsg;
        }
*/
        
        parse_args_last_error_ = e.what();

        if(parms->cmd->errmsg)
        {
          parse_args_last_error_ += ". Usage: ";
          parse_args_last_error_ += parms->cmd->errmsg;
        }
      }
      
//      return last_error.c_str();
      return parse_args_last_error_.c_str();
    }
    
    template<typename CONFIG>
    void*
    Module<CONFIG>::create_server_config_static(apr_pool_t *p, server_rec *s)
    {
      return Config::mod_ref.hooks->create_server_config_(p, s);
    }

    template<typename CONFIG>
    void*
    Module<CONFIG>::merge_server_config_static(apr_pool_t *p,
                                               void *base_conf,
                                               void *new_conf)
    {
      return Config::mod_ref.hooks->
        merge_server_config_(p, base_conf, new_conf);
    }
    
    template<typename CONFIG>
    void*
    Module<CONFIG>::create_dir_config_static(apr_pool_t *p, char *dir)
    {
      return Config::mod_ref.hooks->create_dir_config_(p, dir);
    }

    template<typename CONFIG>
    void*
    Module<CONFIG>::merge_dir_config_static(apr_pool_t *p,
                                            void *base_conf,
                                            void *new_conf)
    {
      return Config::mod_ref.hooks->merge_dir_config_(p, base_conf, new_conf);
    }
    
    template<typename CONFIG>
    int
    Module<CONFIG>::pre_config_static(apr_pool_t *pconf,
                                      apr_pool_t *plog,
                                      apr_pool_t *ptemp)
    {
      try
      {
/*        
        {
          std::ostringstream ostr;
          ostr << std::dec
               << "El::Apache::Module<CONFIG>::pre_config_static("
               << getpid() << "):  pool " << std::hex << pconf
               << std::endl;
        
          std::cerr << ostr.str();
        }
*/
        Config::mod_ref.hooks->pre_config_();
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "Preconfiguration of module " << Config::mod_ref.id
             << " failed. Reason: " << e;

        ap_log_perror(EL_APACHE_LOG_MARK,
                      APLOG_EMERG,
                      0,
                      pconf,
                      "%s",
                      ostr.str().c_str());
        
        return 1;
      }
      
      return OK;
    }

    template<typename CONFIG>
    int
    Module<CONFIG>::post_config_static(apr_pool_t *pconf,
                                       apr_pool_t *plog,
                                       apr_pool_t *ptemp,
                                       server_rec *s)
    {
/*      
      {
        std::ostringstream ostr;
        ostr << std::dec
             << "El::Apache::Module<CONFIG>::post_config_static("
             << getpid() << "):  pool " << std::hex << pconf
             << std::endl;
        
        std::cerr << ostr.str();
      }
*/
      return Config::mod_ref.hooks->post_config_(pconf, plog, ptemp, s);
    }
  
    template<typename CONFIG>
    void
    Module<CONFIG>::register_hooks_static(apr_pool_t *p)
    {
/*      
      std::cerr << std::dec << "test_module_register_hooks(" << id << ", "
                << getpid() << ") pool " << std::hex << p << std::endl;
*/
      Config::mod_ref.hooks->register_hooks_(p);
    }
    
    template<typename CONFIG>
    int
    Module<CONFIG>::config_holder_creator_static(request_rec *r,
                                                 int lookup_uri)
    {
//      std::cerr << std::dec << "config_holder_creator_static(" << getpid()
//                << ") invoked, pool " << std::hex << r->pool << std::endl;

      apr_pool_cleanup_register(r->pool,
                                r->pool,
                                config_cleanup_static,
                                config_cleanup_empty);

      return DECLINED;
    }

    template<typename CONFIG>
    apr_status_t
    Module<CONFIG>::config_cleanup_empty(void* data)
    {
      return OK;
    }    

    template<typename CONFIG>
    apr_status_t
    Module<CONFIG>::config_cleanup_static(void* data)
    {
//      std::cerr << std::dec << "config_cleanup_static(" << getpid()
//                << ") invoked, pool " << std::hex << data << std::endl;

      Config::mod_ref.hooks->config_cleanup_((apr_pool_t*)data);
      return OK;
    }

    template<typename CONFIG>
    void
    Module<CONFIG>::register_hooks_(apr_pool_t *p) throw()
    {
      if(handler_quick_)
      {
        ap_hook_quick_handler(quick_handler_static,
                              NULL,
                              NULL,
                              handler_order_);
      }
      else
      {
        ap_hook_quick_handler(config_holder_creator_static,
                              NULL,
                              NULL,
                              APR_HOOK_FIRST);
      
        ap_hook_handler(handler_static, NULL, NULL, handler_order_);
      }
      
      ap_hook_pre_config(pre_config_static, NULL, NULL, APR_HOOK_FIRST);  
      ap_hook_post_config(post_config_static, NULL, NULL, APR_HOOK_LAST);      
      ap_hook_child_init(child_init_static, NULL, NULL, APR_HOOK_LAST);
    }

    template<typename CONFIG>
    void*
    Module<CONFIG>::create_server_config_(apr_pool_t *p, server_rec *s) throw()
    {
/*      
      {
        std::ostringstream ostr;
        ostr << std::dec
             << "El::Apache::Module<CONFIG>::create_server_config_("
             << getpid() << "):  pool " << std::hex << p
             << " configs: " << config_holders_.size()
             << std::endl;
        
        std::cerr << ostr.str();
      }
*/
      if(is_worker_ && handler_quick_)
      {
        return 0;
      }

      // Test if module cave already been configured and cleanup is so.
      // This happens if module have not been properly unloaded by apache
      // during 2-passes configuration phase.
      cleanup_configured();

      ConfigHolderPtr* holder_ptr =
        (ConfigHolderPtr*)apr_pcalloc(p, sizeof(ConfigHolderPtr));

      holder_ptr->holder =
        new ConfigHolder(new Config(), false, ConfigHolder::CO_SERVER);
      
      save_config(p, holder_ptr);
      
      return holder_ptr;
    }

    template<typename CONFIG>
    void
    Module<CONFIG>::child_cleanup_() throw(El::Exception)
    {
      for(typename ConfigHolderPtrList::iterator
            it = config_holders_.begin();
          it != config_holders_.end(); it++)
      {
        if(!it->holder->empty)
        {
            child_cleanup(*it->holder->conf);
        }
      }

      child_cleanup();
    } 
    
    template<typename CONFIG>
    apr_status_t
    Module<CONFIG>::child_cleanup_static(void* data)
    {
/*      
      std::cerr << std::dec
                << "El::Apache::Module<CONFIG>::child_cleanup_static("
                << getpid() << ") invoked, module " << std::hex << data
                << std::endl;
*/
      try
      {
        
        ((Module<Config>*)data)->child_cleanup_();
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "Child process cleanup phase of module " << Config::mod_ref.id
             << " failed. Reason: " << e << std::endl;

        std::cerr << ostr.str();

        return DECLINED;
      }
        
      return OK;
    }

    template<typename CONFIG>
    void*
    Module<CONFIG>::merge_server_config_(apr_pool_t *p,
                                         void *base_conf,
                                         void *new_conf) throw()
    {
/*
      {
        std::ostringstream ostr;
        std::cerr << std::dec
                  << "El::Apache::Module<CONFIG>::merge_server_config_("
                  << getpid() << "):  pool " << std::hex << p
                  << " configs: " << config_holders_.size()
                  << std::endl;
        
        std::cerr << ostr.str();
      }
*/
      if(is_worker_ && handler_quick_)
      {
        return 0;
      }

      ConfigHolderPtr *base_cfg = (ConfigHolderPtr*)base_conf;
      ConfigHolderPtr *new_cfg = (ConfigHolderPtr*)new_conf;
      
      ConfigHolderPtr* holder_ptr =
        (ConfigHolderPtr*)apr_pcalloc(p, sizeof(ConfigHolderPtr));

      holder_ptr->holder =
        new ConfigHolder(new Config(*base_cfg->holder->conf,
                                    *new_cfg->holder->conf),
                         false,
                         ConfigHolder::mixed_origin(base_cfg->holder,
                                                    new_cfg->holder));

      DirectiveCounterTable& dc_table = holder_ptr->holder->directive_count;
      dc_table = base_cfg->holder->directive_count;

      const DirectiveCounterTable& new_dc_table =
        new_cfg->holder->directive_count;

      for(DirectiveCounterTable::const_iterator it = new_dc_table.begin();
          it != new_dc_table.end(); it++)
      {
        DirectiveCounterTable::iterator dit = dc_table.find(it->first);

        if(dit == dc_table.end())
        {
          dc_table[it->first] = it->second;
        }
        else
        {
          dit->second += it->second;
        }
      }
      
      save_config(p, holder_ptr);

      return holder_ptr;
    }
    
    template<typename CONFIG>
    void*
    Module<CONFIG>::create_dir_config_(apr_pool_t *p, char *dir) throw()
    {
/*      
      std::cerr << std::dec
                << "El::Apache::Module<CONFIG>::create_dir_config_("
                << getpid() << ", " << is_worker_ << "):  pool "
                << std::hex << p
                << " configs: " << config_holders_.size()
                << std::endl;
*/
      if(is_worker_ && handler_quick_)
      {
        return 0;
      }

      // Test if module cave already been configured and cleanup is so.
      // This happens if module have not been properly unloaded by apache
      // during 2-passes configuration phase.
      cleanup_configured();

      ConfigHolderPtr* holder_ptr =
        (ConfigHolderPtr*)apr_pcalloc(p, sizeof(ConfigHolderPtr));
      
      holder_ptr->holder =
        new ConfigHolder(new Config(), dir == 0, ConfigHolder::CO_DIRECTORY);
      
      save_config(p, holder_ptr);

      return holder_ptr;
    }

    template<typename CONFIG>
    void*
    Module<CONFIG>::merge_dir_config_(apr_pool_t *p,
                                      void *base_conf,
                                      void *new_conf) throw()
    {
/*      
      std::cerr << std::dec
                << "El::Apache::Module<CONFIG>::merge_dir_config_("
                << getpid() << "):  pool " << std::hex << p
                << " configs: " << config_holders_.size()
                << std::endl;
*/
      if(is_worker_ && handler_quick_)
      {
        return 0;
      }

      ConfigHolderPtr *base_cfg = (ConfigHolderPtr*)base_conf;
      ConfigHolderPtr *new_cfg = (ConfigHolderPtr*)new_conf;
      
      ConfigHolderPtr* holder_ptr =
        (ConfigHolderPtr*)apr_pcalloc(p, sizeof(ConfigHolderPtr));

      holder_ptr->holder =
        new ConfigHolder(new Config(*base_cfg->holder->conf,
                                    *new_cfg->holder->conf),
                         base_cfg->holder->empty && new_cfg->holder->empty,
                         ConfigHolder::mixed_origin(base_cfg->holder,
                                                    new_cfg->holder));

      save_config(p, holder_ptr);
      
      return holder_ptr;
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::pre_config_() throw(El::Exception)
    {      
      pre_config();
    }
    
    template<typename CONFIG>
    int
    Module<CONFIG>::post_config_(apr_pool_t *pconf,
                                 apr_pool_t *plog,
                                 apr_pool_t *ptemp,
                                 server_rec *s) throw()
    {
      try
      {
        validate_directive_presense();
        
        for(typename ConfigHolderPtrList::iterator
              it = config_holders_.begin();
            it != config_holders_.end(); it++)
        {
          ConfigHolder* holder = it->holder;
            
          if(!holder->empty)
          {
            post_config(*holder->conf, "post_config_");
          }
          
          holder->post_configured = true;
        }
        
        post_config();
        post_configured_ = true;        
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "Postconfiguration of module " << Config::mod_ref.id
             << " failed. Reason: " << e;

        ap_log_perror(EL_APACHE_LOG_MARK,
                      APLOG_EMERG,
                      0,
                      pconf,
                      "%s",
                      ostr.str().c_str());

        return 1;
      }
      
      return OK;
    }
     
    template<typename CONFIG>
    void
    Module<CONFIG>::validate_directive_presense() throw(El::Exception)
    {
      bool has_srv_origins = false;
      bool has_mixed = false;
      
      for(typename ConfigHolderPtrList::iterator
            it = config_holders_.begin();
          it != config_holders_.end(); it++)
      {        
        ConfigHolder* holder = it->holder;
            
        if(!holder->empty && holder->server_origin())
        {
          has_srv_origins = true;

          if(holder->origin == ConfigHolder::CO_SERVER_MIX)
          {
            has_mixed = true;
          }
        }
      }

      if(!has_srv_origins)
      {
        return;
      }
      
      for(typename ConfigHolderPtrList::iterator
            it = config_holders_.begin();
          it != config_holders_.end(); it++)
      {
        ConfigHolder* holder = it->holder;
            
        if(!holder->empty && holder->server_origin())
        {
          if(has_mixed && holder->origin == ConfigHolder::CO_SERVER)
          {
            continue;
          }
                
          for(typename DirectiveDefTable::const_iterator
                dit = directive_definitions_.begin();
              dit != directive_definitions_.end(); dit++)
          {
            const DirectiveDef& dir_def = dit->second;
              
            if(dir_def.min_occurs == 0 && dir_def.max_occurs == 0)
            {
              continue;
            }
              
            const DirectiveCounterTable& directive_count =
              holder->directive_count;
                
            DirectiveCounterTable::const_iterator dc_it =
              directive_count.find(dit->first);
              
            unsigned long count =
              dc_it == directive_count.end() ? 0 : dc_it->second;
            
            if(count < dir_def.min_occurs || count > dir_def.max_occurs)
            {
              std::ostringstream ostr;
              ostr << "directive '" << dit->first << "' should appear ";

              if(dir_def.min_occurs == dir_def.max_occurs)
              {
                ostr << dir_def.min_occurs;
              }
              else
              {
                ostr << "from " << dir_def.min_occurs << " to "
                     << dir_def.max_occurs << " number of";
              }

              ostr << " times; appeared " << count << " times instead";

              throw Exception(ostr.str());
            }
          }
            
        }
      }
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::child_init_(apr_pool_t *pchild, server_rec *s) throw()
    {
/*      
      std::cerr << std::dec
                << "El::Apache::Module<CONFIG>::child_init_("
                << getpid() << ") configs: " << config_holders_.size()
                << std::endl;
*/    
      try
      {
        is_worker_ = true;
        
        for(typename ConfigHolderPtrList::iterator
              it = config_holders_.begin();
            it != config_holders_.end(); it++)
        {
//          std::cerr << "  " << it->holder->empty << std::endl;
          
          if(!it->holder->empty)
          {
            child_init(*it->holder->conf);
          }
        }
        
        child_init();

        apr_pool_cleanup_register(pchild,
                                  this,
                                  child_cleanup_static,
                                  child_cleanup_static);
        
        initialized_ = true;
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "Child process initialization for module "
             << Config::mod_ref.id << " failed. Reason: " << e;
        
        ap_log_error(EL_APACHE_LOG_MARK,
                     APLOG_EMERG,
                     0,
                     s,
                     "%s",
                     ostr.str().c_str());
        
        child_cleanup_();
        kill(getppid(), SIGTERM);
      }
    }

    template<typename CONFIG>
    int
    Module<CONFIG>::handler_(request_rec *r) throw()
    {
      /*
      ap_log_rerror(APLOG_MARK,
                    APLOG_ALERT,
                    0,
                    r,
                    "%s",
                    "rec handling");
      */
      
      if(!initialized_)
      {
        std::ostringstream ostr;
        ostr << "Unexpected execution path: module " << Config::mod_ref.id
             << " is not initialized";
        
        ap_log_rerror(EL_APACHE_LOG_MARK,
                      APLOG_ALERT,
                      0,
                      r,
                      "%s",
                      ostr.str().c_str());

        return DECLINED;
      }

      if(!handler_locations_directive_.empty())
      {
        bool matched = false;
        
        for(LocationTable::const_iterator it = locations_.begin();
            it != locations_.end(); it++)
        {
          const std::string url = it->first;
          
          if((it->second & MT_EXACT) && (matched = (url == r->uri)))
          {
            break;
          }          

          if(it->second & MT_PREFIX)
          {
            unsigned long url_len = url.length();
            unsigned long rurl_len = strlen(r->uri);
              
            if((matched = url_len < rurl_len &&
                strncmp(r->uri, url.c_str(), url_len) == 0))
            {
              break;
            }
          }
        }
        
        if(!matched)
        {
          return DECLINED;
        }
      }

      if(!handler_quick_ && r->handler != 0 && !handlers_.empty())
      {
        std::string lowered;
        El::String::Manip::to_lower(r->handler, lowered);

        if(handlers_.find(lowered) == handlers_.end())
        {
          return DECLINED;
        }
      }

      try
      {
        Context context(r, this);

        int result = 0;

        try
        {
          result = handler(context);
        }
        catch(...)
        {
          context.request.discard();
          throw;
        }
        
        context.request.finalize(result);
        
        return result;
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;
        ostr << "Request handling by module " << Config::mod_ref.id
             << " failed. Reason: " << e;

        ap_log_rerror(EL_APACHE_LOG_MARK,
                      APLOG_ALERT,
                      0,
                      r,
                      "%s",
                      ostr.str().c_str());
      }
      
      return HTTP_INTERNAL_SERVER_ERROR;
    }
    
    template<typename CONFIG>
    void
    Module<CONFIG>::config_cleanup_(apr_pool_t* pool) throw()
    {
      ConfigHolderPtrList hlist;
      
      {
        RCT_Guard guard(rct_lock_);

        typename RequestConfigTable::iterator i =
          request_config_table_.find(pool);

        if(i != request_config_table_.end())
        {
          // To call object destructors out of hash map guard
          hlist.swap(i->second);
          request_config_table_.erase(i);
        }
      }
      
    }
  }
}

#endif // _ELEMENTS_EL_APACHE_MODULE_HPP_
