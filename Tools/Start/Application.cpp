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
 * @file   Elements/Tools/Start/Application.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <fnmatch.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include <ace/OS.h>

#include <El/String/Manip.hpp>
#include <El/String/Template.hpp>

#include <El/Moment.hpp>

#include "Application.hpp"

namespace
{
  const char USAGE[] =
    "Usage:\nElStart <command> <command arguments>\n\n"
    "Synopsis 1:\n"
    "ElStart help\n\n"
    "Synopsis 2:\nElStart ( (start|stop|status) | list (start|stop|status)? ) "
    "(--nodeps)? "
    "((+|-)?<host wildcard>(:<service wildcard>|^<service tag wildcard>))* "
    "<cluster description file>\n"
    "\nExamples:\nElStart start ./Cluster.xml\n"
    "ElStart status --nodeps server:*manager ./Cluster.xml\n"
    "ElStart stop *^backend ./Cluster.xml\n"
    "ElStart stop server:*manager -*^backend ./Cluster.xml\n";
}

int
main(int argc, char** argv)
{
  try
  {
    srand(time(NULL));
    
    Application app;
    return app.run(argc, argv);
  }
  catch (const xml_schema::exception& e)
  {
    std::cerr << "ElStart: " << e << std::endl;
  }
  catch(const Application::InvalidArg& e)
  {
    std::cerr << "Invalid argument: " << e
              << "\nRun 'ElStart help' for usage details\n";
  }
  catch(const El::Exception& e)
  {
    std::cerr << e << std::endl;
  }
  catch(...)
  {
    std::cerr << "ElStart: unknown exception caught\n";
  }
  
  return -1;
}

//
// ServiceOnHost class
//
struct ServiceOnHost
{
  std::string service;
  std::string host;
  Application::StringSet tags;
  
  ServiceOnHost() throw() {}
  
  ServiceOnHost(const char* srv,
                const char* hst,
                const Application::StringSet& tags) throw(El::Exception);
  
  bool operator<(const ServiceOnHost& val) const throw(El::Exception);
};

inline
ServiceOnHost::ServiceOnHost(const char* srv,
                             const char* hst,
                             const Application::StringSet& tgs)
  throw(El::Exception)
    : service(srv),
      host(hst),
      tags(tgs)
{
}
  
inline
bool
ServiceOnHost::operator<(const ServiceOnHost& val) const throw(El::Exception)
{
  int r = strcmp(service.c_str(), val.service.c_str());

  if(r)
  {
    return r < 0;
  }

  return host < val.host;
}

typedef std::set<ServiceOnHost> ServiceOnHostSet;

//
// Rule class
//
class Rule
{
public:
  EL_EXCEPTION(Exception, El::ExceptionBase);
  
public:
  Rule(const char* text) throw(Exception, El::Exception);
  ~Rule() throw();
  
  bool match(const char* service,
             const char* host,
             const Application::StringSet& tags)
    const throw(El::Exception);

private:
  std::string service_pattern_;
  std::string host_pattern_;
  bool tag;
};

inline
Rule::Rule(const char* text) throw(Exception, El::Exception)
    : tag(false)
{
  std::string txt;
  El::String::Manip::trim(text, txt);

  if(txt.empty())
  {
    throw Exception("empty rule");
  }

  std::string::size_type pos = txt.find_first_of(":^");

  if(pos == std::string::npos)
  {
    host_pattern_ = txt;
    service_pattern_ = "*";
  }
  else
  {
    host_pattern_ = txt.substr(0, pos);
    service_pattern_ = txt.substr(pos + 1);

    if(host_pattern_.empty())
    {
      host_pattern_ = "*";
    }
    
    if(service_pattern_.empty())
    {
      service_pattern_ = "*";
    }

    tag = txt[pos] == '^';
  }
}

inline
Rule::~Rule() throw()
{
}
  
inline
bool
Rule::match(const char* service,
            const char* host,
            const Application::StringSet& tags)
  const throw(El::Exception)
{
  if(fnmatch(host_pattern_.c_str(), host, FNM_CASEFOLD))
  {
    return false;
  }

  if(tag)
  {
    for(Application::StringSet::const_iterator it = tags.begin();
        it != tags.end(); it++)
    {
      if(fnmatch(service_pattern_.c_str(), it->c_str(), FNM_CASEFOLD) == 0)
      {
        return true;
      }

      return false;
    }
  }
  
  return fnmatch(service_pattern_.c_str(), service, FNM_CASEFOLD) == 0;
}

//
// Application::HostInfo class
//
void
Application::HostInfo::set_execution(const Service& srv) throw(El::Exception)
{
  execute_service_command = true;
  desc.host_service = name + ":" + srv.name;
  desc.commands.clear();
  desc.tags.clear();
  
  if(srv.flags & Service::SF_START)
  {
    desc.commands += "start";
  }
  
  if(srv.flags & Service::SF_STOP)
  {
    if(!desc.commands.empty())
    {
      desc.commands += " ";
    }
    
    desc.commands += "stop";
  }

  if(srv.flags & Service::SF_STATUS)
  {
    if(!desc.commands.empty())
    {
      desc.commands += " ";
    }
    
    desc.commands += "status";
  }

  const StringSet& tags = srv.tag;

  for(StringSet::const_iterator it = tags.begin(); it != tags.end(); it++)
  {
    if(it != tags.begin())
    {
      desc.tags += " ";
    }

    desc.tags += *it;
  }
  
}

//
// Application class
//
Application::Application() throw(Application::Exception, El::Exception)
    : operation_(OP_START),
      layer_counter_(0),
      verbose_(Start::VerboseType::no),
      list_reverse_(false),
      nodeps_(false),
      max_host_service_len_(0),
      max_commands_len_(0),
      max_tags_len_(0)
{
}

Application::~Application() throw()
{
  if(threads_.in())
  {
    threads_->stop();
    threads_->wait();
  }
}

int
Application::run(int& argc, char** argv)
  throw(InvalidArg, Exception, El::Exception)
{
  if(argc < 2)
  {
    throw InvalidArg("Too few arguments");
  }

  int i = 1;  
  std::string command = argv[i];

  ArgList arguments;

  for(i++; i < argc; i++)
  {
    char* argument = argv[i];
    
    Argument arg;
    const char* eq = strstr(argument, "=");

    if(eq == 0)
    {
      arg.name = argument;
    }
    else
    {
      arg.name.assign(argument, eq - argument);
      arg.value = eq + 1;
    }

    arguments.push_back(arg);
  }

  if(command == "help")
  {
    return help(arguments);
  }

  if(command == "start")
  {
    execute(arguments, OP_START);
  }
  else if(command == "stop")
  {
    execute(arguments, OP_STOP);
  }
  else if(command == "status")
  {
    execute(arguments, OP_STATUS);
  }
  else if(command == "list")
  {
    execute(arguments, OP_LIST);
  }
  else
  {
    std::ostringstream ostr;
    ostr << "unknown command '" << command << "'";
   
    throw InvalidArg(ostr.str());
  }

  return 0;
}

int
Application::help(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  std::cerr << USAGE;
  return 0;
}

void
Application::set_target(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  ArgList rules(arguments.begin(), arguments.begin() + arguments.size() - 1);
  
  if(rules.empty() || rules[0].name[0] == '-')
  {
    rules.insert(rules.begin(), Argument("*"));
  }

  ServiceOnHostSet target;

  for(ArgList::const_iterator it = rules.begin(); it != rules.end(); it++)
  {
    std::string rule_text = it->name;
    char fc = rule_text[0];
    bool negate = fc == '-';
    
    if(fc == '-' || fc == '+')
    {
      rule_text.erase(0, 1);
    }

    Rule rule(rule_text.c_str());
      
    if(negate)
    {
      for(ServiceOnHostSet::const_iterator sit = target.begin();
          sit != target.end(); )
      {
        if(rule.match(sit->service.c_str(), sit->host.c_str(), sit->tags))
        {
          ServiceOnHostSet::const_iterator current = sit++;
          target.erase(current);
        }
        else
        {
          sit++;
        }
      }
    }
    else
    {
      for(ServiceTable::const_iterator sit = services_.begin();
          sit != services_.end(); sit++)
      {
        const HostInfoMap& hosts = sit->second.hosts;

        for(HostInfoMap::const_iterator hit = hosts.begin();
            hit != hosts.end(); hit++)
        {
          const char* srv = sit->first.c_str();
          const char* host = hit->first.c_str();
          
          if(rule.match(srv, host, sit->second.tag))
          {
            target.insert(ServiceOnHost(srv, host, sit->second.tag));
          }
        }
      }
    }
  }
  
  for(ServiceTable::iterator sit = services_.begin(); 
      sit != services_.end(); sit++)
  {
    HostInfoMap& hosts = sit->second.hosts;

    bool execute_prerequisites = false;
    
    for(HostInfoMap::iterator hit = hosts.begin(); hit != hosts.end(); hit++)
    {
      const char* srv = sit->first.c_str();
      const char* host = hit->first.c_str();

      if(target.find(ServiceOnHost(srv, host, sit->second.tag)) !=
         target.end())
      {
        hit->second.set_execution(sit->second);
        execute_prerequisites = !nodeps_;
      }
    }

    if(execute_prerequisites)
    {
      set_execute_flags(sit->second, true);
    }
  }

  max_host_service_len_ = 0;
  max_commands_len_ = 0;
  max_tags_len_ = 0;
  
  for(ServiceTable::iterator sit = services_.begin(); 
      sit != services_.end(); sit++)
  {
    HostInfoMap& hosts = sit->second.hosts;

    for(HostInfoMap::iterator hit = hosts.begin(); hit != hosts.end(); hit++)
    {
      const HostInfo:: Description& desc = hit->second.desc;
      
      max_host_service_len_ =
        std::max(max_host_service_len_,
                 (unsigned long)desc.host_service.length());
      
      max_commands_len_ =
        std::max(max_commands_len_,
                 (unsigned long)desc.commands.length());

      max_tags_len_ =
        std::max(max_tags_len_, (unsigned long)desc.tags.length());
    }
  }

}

void
Application::set_execute_flags(Service& service, bool prerequisites_only)
  throw(InvalidArg, Exception, El::Exception)
{
  if(!prerequisites_only)
  {
    HostInfoMap& hosts = service.hosts;
    
    for(HostInfoMap::iterator hit = hosts.begin(); hit != hosts.end(); hit++)
    {
      hit->second.set_execution(service);
    }
  }

  StringSet& prerequisites = reversed() ? service.before : service.after;

  for(StringSet::iterator it = prerequisites.begin();
      it != prerequisites.end(); it++)
  {
    ServiceTable::iterator sit = services_.find(*it);
    assert(sit != services_.end());
    
    set_execute_flags(sit->second, false);
  }
}

void
Application::parse_options(ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  if(!arguments.empty() && operation_ == OP_LIST)
  {
    std::string options = arguments[0].name;

    if(options == "stop")
    {
      list_reverse_ = true;
      arguments.erase(arguments.begin());
    }
    else if(options == "start" || options == "status")
    {
      arguments.erase(arguments.begin());
    }
  }

  for(ArgList::iterator it = arguments.begin(); it != arguments.end(); )
  {
    const char* name = it->name.c_str();

    if(strncmp(name, "--", 2) == 0)
    {
      if(strcmp(name, "--nodeps") == 0)
      {
        nodeps_ = true;
      }
         
      it = arguments.erase(it);
    }
    else
    {
      ++it;
    }
  }

  /*
  if(!arguments.empty() && arguments[0].name == "--nodeps")
  {
    nodeps_ = true;
    arguments.erase(arguments.begin());
  }
  */
}

int
Application::execute(ArgList& arguments, Operation operation)
  throw(InvalidArg, Exception, El::Exception)
{
  operation_ = operation;

  parse_options(arguments);
  parse_cluster_definition(arguments);
  validate_cluster_definition();
  set_target(arguments);

  if(verbose_ == Start::VerboseType::full)
  {
    std::cerr << "\nRunning:\n";
  }
  
  threads_ =
    new El::Service::ThreadPool(this,
                                "ThreadPool",
                                cluster_->parameters().present() ?
                                cluster_->parameters()->threads() : 10);
  threads_->start();
  
  {
    WriteGuard guard(lock_);
    execute_on_layer();
  }
  
  threads_->wait();

  return 0;
}

void
Application::parse_cluster_definition(const ArgList& arguments)
  throw(InvalidArg, Exception, El::Exception)
{
  ArgList::const_reverse_iterator it = arguments.rbegin();
  
  if(it == arguments.rend())
  {
    throw InvalidArg("path to cluster description file not specified");
  }

  cluster_ = Start::cluster(it->name);

  if(cluster_->parameters().present())
  {
    verbose_ = cluster_->parameters()->verbose();
  }

  if(verbose_ == Start::VerboseType::full)
  {
    std::cerr << "Environment variables:\n";
  }
  
  for(char** env = environ; *env != 0; ++env)
  {
    const char* env_str = *env;
    const char* val = strchr(env_str, '=');

    if(val == 0)
    {
      continue;
    }

    std::string varname(env_str, val - env_str);
    variables_[std::string("ENV:") + varname] = ++val;

    if(verbose_ == Start::VerboseType::full)
    {
      std::cerr << "  " << varname << "=" << val << std::endl;
    }
  }  

  if(verbose_ == Start::VerboseType::full)
  {
    std::cerr << "Variables:\n";
  }

  variables_[""] = "\\";
    
  Start::ClusterType::var_sequence& vars = cluster_->var();
  for(Start::ClusterType::var_sequence::const_iterator it = vars.begin();
      it != vars.end(); it++)
  {
    const Start::VarType& var = *it;
    parser_.parse(var.c_str(), "\\", "\\");

    std::string value = parser_.instantiate(variables_);
    variables_[std::string("VAR:") + var.name()] = value;
    
    if(verbose_ == Start::VerboseType::full)
    {
      std::cerr << "  " << var.name().c_str() << "=" << value << std::endl;
    }
  }
    
  if(verbose_ == Start::VerboseType::full)
  {
    std::cerr << "\nTransports:\n";
  }
  
  Start::ClusterType::transport_sequence& transports =
    cluster_->transport();
  
  for(Start::ClusterType::transport_sequence::const_iterator it =
        transports.begin(); it != transports.end(); it++)
  {
    const Start::TransportType& transport = *it;

    std::string tmp = transport;
    std::string transport_command;
    
    El::String::Manip::trim(tmp.c_str(), transport_command);

    if(transport_command.empty())
    {
      std::ostringstream ostr;
      ostr << "no command line specified for transport " << transport.name();

      throw Exception(ostr.str());
    }
      
    std::pair<TransportTable::iterator, bool> tit =
      transports_.insert(std::make_pair(transport.name(),
                                        transport_command));

    if(!tit.second)
    {
      std::ostringstream ostr;
      ostr << "transport " << transport.name() << " redefinition";

      throw Exception(ostr.str());
    }
    
    if(verbose_ == Start::VerboseType::full)
    {
      std::cerr << "  " << transport.name().c_str() << "="
                << transport_command  << std::endl;
    }
  }
    
  if(verbose_ == Start::VerboseType::full)
  {
    std::cerr << "\nServices:\n";
  }
  
  Start::ClusterType::service_sequence& services =
    cluster_->service();
  
  for(Start::ClusterType::service_sequence::const_iterator it =
        services.begin(); it != services.end(); it++)
  {
    std::string command;
    bool status_check_delay = false;
    const Start::ServiceType& service = *it;

    if(!service.enabled())
    {
      disabled_services_.insert(service.name());
      continue;
    }
    
    switch(operation_)
    {
    case OP_START:
      {
        if(service.start().present())
        {
          El::String::Manip::trim(service.start()->c_str(), command);
          status_check_delay = service.start()->status_check_delay();
        }
          
        break;
      }
    case OP_STATUS:
      {
        if(service.status().present())
        {
          El::String::Manip::trim(service.status()->c_str(), command);
        }
          
        break;
      }
    case OP_STOP:
      {
        if(service.stop().present())
        {
          El::String::Manip::trim(service.stop()->c_str(), command);
        }
          
        break;
      }
    default: break;
    }

    std::pair<ServiceTable::iterator, bool> sit =
      services_.insert(std::make_pair(service.name(), Service()));

    if(!sit.second)
    {
      std::ostringstream ostr;
      ostr << "service " << service.name() << " redefinition";

      throw Exception(ostr.str());
    }

    Service& svc = sit.first->second;

    svc.name = service.name();
    svc.command = command;
    svc.status_check_delay = status_check_delay;

    if(service.start().present())
    {
      svc.flags |= Service::SF_START;
    }

    if(service.stop().present())
    {
      svc.flags |= Service::SF_STOP;
    }

    if(service.status().present())
    {
      svc.flags |= Service::SF_STATUS;
      El::String::Manip::trim(service.status()->c_str(), svc.status);
    }
    
    TransportTable::iterator tit = transports_.find(service.transport());

    if(tit == transports_.end())
    {
      std::ostringstream ostr;
      ostr << "transport " << service.transport() << " specified for service "
           << service.name() << " is not defined";

      throw Exception(ostr.str());      
    }

    svc.transport = tit->second;
    
    if(verbose_ == Start::VerboseType::full)
    {
      std::cerr << "  " << service.name() << ", transport "
                << service.transport() << " ( " << svc.transport
                << " )\n    command: " << svc.command << std::endl;
    }

    if(verbose_ == Start::VerboseType::full)
    {
      std::cerr << "    Tags:\n";
    }
      
    const Start::ServiceType::tag_sequence& tag =
      service.tag();

    for(Start::ServiceType::tag_sequence::const_iterator sit =
          tag.begin(); sit != tag.end(); sit++)
    {
      const Start::TagServiceType& item = *sit;

      std::pair<StringSet::iterator, bool> ait =
        svc.tag.insert(item.name());

      if(!ait.second)
      {
        std::ostringstream ostr;
        ostr << "for service " << service.name() << " tag "
             << item.name() << " mentioned multiple times";
        
        throw Exception(ostr.str());
      }
      
      if(verbose_ == Start::VerboseType::full)
      {
        std::cerr << "      " << item.name().c_str() << std::endl;
      }        
    }
    
    if(verbose_ == Start::VerboseType::full)
    {
      std::cerr << "    After:\n";
    }
      
    const Start::ServiceType::after_sequence& after =
      service.after();

    for(Start::ServiceType::after_sequence::const_iterator sit =
          after.begin(); sit != after.end(); sit++)
    {
      const Start::AfterServiceType& item = *sit;

      std::pair<StringSet::iterator, bool> ait =
        svc.after.insert(item.service());

      if(!ait.second)
      {
        std::ostringstream ostr;
        ostr << "for service " << service.name() << " prerequisite "
             << item.service() << " mentioned multiple times";
        
        throw Exception(ostr.str());
      }
      
      if(verbose_ == Start::VerboseType::full)
      {
        std::cerr << "      " << item.service().c_str() << std::endl;
      }        
    }
    
  }
  
  if(verbose_ == Start::VerboseType::full)
  {
    std::cerr << "\nHosts:\n";
  }
  
  const Start::ClusterType::host_sequence& hosts = cluster_->host();
  
  for(Start::ClusterType::host_sequence::const_iterator it =
        hosts.begin(); it != hosts.end(); it++)
  {
    const Start::HostType& host = *it;
    
    if(verbose_ == Start::VerboseType::full)
    {
      std::cerr << "  " << host.name() << std::endl;
      std::cerr << "    Services:\n";
    }
    
    const Start::HostType::service_sequence& services =
      host.service();

    for(Start::HostType::service_sequence::const_iterator sit =
          services.begin(); sit != services.end(); sit++)
    {
      const Start::HostServiceType& item = *sit;

      if(disabled_services_.find(item.name()) != disabled_services_.end())
      {
        continue;
      }

      ServiceTable::iterator ssit = services_.find(item.name());

      if(ssit == services_.end())
      {
        std::ostringstream ostr;
        ostr << "service " << item.name() << " to be run on host "
             << host.name() << " is not defined";
        
        throw Exception(ostr.str());
      }

      const char* host_name = host.name().c_str();
      
      std::pair<HostInfoMap::iterator, bool> hit =
        ssit->second.hosts.insert(std::make_pair(host_name,
                                                 HostInfo(host_name)));
      
      if(!hit.second)
      {
        std::ostringstream ostr;
        ostr << "service " << item.name() << " is assigned to host "
             << host.name() << " multiple times";
        
        throw Exception(ostr.str());
      }

      if(verbose_ == Start::VerboseType::full)
      {
        std::cerr << "      " << item.name().c_str() << std::endl;
      }
      
    } 
  }

//  StringSet idle_services;
    
  for(ServiceTable::iterator it = services_.begin();
      it != services_.end(); it++)
  {
    Service& svr = it->second;

    for(StringSet::iterator ait = svr.after.begin(); ait != svr.after.end(); )
    {
      if(disabled_services_.find(*ait) != disabled_services_.end())
      {
        StringSet::iterator current = ait++;
        svr.after.erase(current);
        continue;
      }

      ServiceTable::iterator sit = services_.find(*ait);
        
      if(sit == services_.end())
      {
        std::ostringstream ostr;
        ostr << "service " << *ait << " mentioned as prerequisite of service "
             << it->first << " is not defined";
        
        throw Exception(ostr.str());
      }

      sit->second.before.insert(it->first);
      ait++;
    }
    
    if(svr.hosts.empty())
    {
//      idle_services.insert(it->first);
//      std::cerr << "warning: service " << it->first << " is idle\n";

      std::ostringstream ostr;
      ostr << "no host assigned for " << it->first << " service";
      throw Exception(ostr.str());
    }
  }

/*  
  for(StringSet::iterator it = idle_services.begin();
      it != idle_services.end(); it++)
  {
    services_.erase(*it);
  }
*/
}

void
Application::validate_cluster_definition() throw(Exception, El::Exception)
{
  ServiceTable services = services_;

  while(!services.empty())
  {
    StringSet layer_services;
      
    for(ServiceTable::iterator it = services.begin();
        it != services.end(); it++)
    {
      if(it->second.after.empty())
      {
        layer_services.insert(it->first);
      }
    }

    if(layer_services.empty())
    {
      std::stringstream ostr;
      ostr << "can't execute requested operation because "
        "following services are all interdependent: ";
    
      for(ServiceTable::iterator it = services.begin();
          it != services.end(); it++)
      {
        ostr << (it == services.begin() ? "" : ", ") << it->first;

        const StringSet& prereq = it->second.after;

        ostr << " [ ";
      
        for(StringSet::const_iterator dit = prereq.begin();
            dit != prereq.end(); dit++)
        {
          ostr << (dit == prereq.begin() ? "" : ", ") << *dit;
        }
      
        ostr << " ]";
      }

      throw Exception(ostr.str());
    }

    for(StringSet::const_iterator it = layer_services.begin();
        it != layer_services.end(); it++)
    {
      const std::string& service = *it;
      
      for(ServiceTable::iterator sit = services.begin();
          sit != services.end(); sit++)
      {
        sit->second.after.erase(service);
      }

      services.erase(service);
    }
  }  
}

void
Application::execute_on_layer() throw(Exception, El::Exception)
{
  if(layer_counter_)
  {
    std::stringstream ostr;
    ostr << "Application::execute_on_layer: layer_counter_ unexpectedly = "
         << layer_counter_;

    throw Exception(ostr.str());
  }
    
  if(verbose_ == Start::VerboseType::full)
  {
    std::cerr << "Executing service layer\n";
  }
  
  for(ServiceTable::iterator it = services_.begin();
      it != services_.end(); it++)
  {
    Service& svc = it->second;

    if(reversed() ? !svc.before.empty() : !svc.after.empty())
    {
      continue;
    }

    HostInfoMap& hosts = svc.hosts;
    for(HostInfoMap::iterator hit = hosts.begin(); hit != hosts.end(); hit++)
    {
      El::Service::ThreadPool::Task_var task =
        new ExecuteOnHost(it->first.c_str(), hit->first.c_str(), this);
      
      threads_->execute(task.in());
      layer_counter_++;
    }
  }

  if(layer_counter_ == 0)
  {
    //
    // Shouldn't be here if validate_cluster_definition works properly
    //
    throw Exception("cluster definition validation internal error");
  }
}

bool
Application::notify(El::Service::Event* event) throw(El::Exception)
{
  ExecuteOnHost* execute = dynamic_cast<ExecuteOnHost*>(event);

  if(execute != 0)
  {
    if(!execute_service_operation(execute->service.c_str(),
                                  execute->host.c_str()))
    {    
      threads_->stop();
      return true;
    }      

    WriteGuard guard(lock_);

    ServiceTable::iterator it = services_.find(execute->service);

    if(it == services_.end())
    {
      std::ostringstream ostr;
      ostr << "Application::notify: service " << execute->service
           << " unexpectedly not found" << std::endl;
        
      std::cerr << ostr.str();
      threads_->stop();

      return true;
    }

    HostInfoMap& hosts = it->second.hosts;
    hosts.erase(execute->host);

    if(hosts.empty())
    {
      services_.erase(execute->service);

      for(ServiceTable::iterator it = services_.begin();
          it != services_.end(); it++)
      {
        if(reversed())
        {
          it->second.before.erase(execute->service);
        }
        else
        {
          it->second.after.erase(execute->service);
        }
        
      }
    }
    
    layer_counter_--;
    
    if(layer_counter_ == 0)
    {
      if(services_.empty())
      {
        threads_->stop();
      }
      else
      {
        try
        {
          execute_on_layer();
        }
        catch(const El::Exception& e)
        {
          std::cerr << e << std::endl;
          threads_->stop();
        }
      }
    }
    
    return true;
  }
    
  std::cerr
    << El::Service::error_message(event, "Application::notify: ")
    << std::endl;

  threads_->stop();
  
  return true;
}

bool
Application::execute_service_operation(const char* service, const char* host)
  throw(Exception, El::Exception)
{
  try
  {
    Service svr;
  
    {
      WriteGuard guard(lock_);
    
      ServiceTable::const_iterator sit = services_.find(service);

      if(sit == services_.end())
      {
        std::ostringstream ostr;
        ostr << "Application::execute_service_operation: failed to find "
          "service " << service << " when running on host " << host;
      
        throw Exception(ostr.str());
      }

      svr = sit->second;
    }

    HostInfoMap::const_iterator it = svr.hosts.find(host);
    assert(it != svr.hosts.end());

    if(!it->second.execute_service_command)
    {
      return true;
    }

    if(operation_ == OP_LIST)
    {
      WriteGuard guard(lock_);

      ServiceTable::const_iterator sit = services_.find(service);
      assert(sit != services_.end());

      const Service& srv = sit->second;
      const HostInfoMap& hosts = srv.hosts;

      HostInfoMap::const_iterator hit = hosts.find(host);

      assert(hit != hosts.end());      

      const HostInfo::Description& desc = hit->second.desc;
          
      std::string host_srv = hit->first + ":" + sit->first;
      
      std::cerr << "| " << std::setw(max_host_service_len_)
                << std::setfill(' ') << std::left
                << desc.host_service << " | "
                << std::setw(max_tags_len_) << std::setfill(' ')
                << std::left << desc.tags << " | "
                << std::setw(max_commands_len_) << std::setfill(' ')
                << std::left << desc.commands
                << " |" << std::endl;
        
      return true;
    }
      
    if(svr.command.empty())
    {
      return true;
    }

    std::string command = prepare_command(service,
                                          host,
                                          svr.transport.c_str(),
                                          svr.command.c_str());

    {
      WriteGuard guard(lock_);

      switch(operation_)
      {
      case OP_START:
        {
          std::cerr << "Starting " << service << " on " << host
                    << " ..." << std::endl;
          break;
        }
      case OP_STATUS:
        {
/*          
          std::cerr << "Checking " << service << " on " << host
                    << " ..." << std::endl;
*/
          break;
        }
      case OP_STOP:
        {
          std::cerr << "Stopping " << service << " on " << host
                    << " ..." << std::endl;
          break;
        }
      default: break;
      }

      std::cerr.flush();
    }

    int status = 0;
    bool check_status = false;

    std::string status_check;
  
    if(operation_ != OP_STATUS && !svr.status.empty())
    {
      check_status = true;
    
      status_check = prepare_command(service,
                                     host,
                                     svr.transport.c_str(),
                                     svr.status.c_str());
    
      status = execute_command(status_check.c_str());    
    }

    {
      WriteGuard guard(lock_);

      if(check_status && (status == 0) == (operation_ == OP_START))
      {
        if(operation_ == OP_START)
        {
          std::cerr << service << " is already running on " << host
                    << std::endl;
        }
        else
        {
          std::cerr << service << " is not running on " << host << std::endl;
        }

        std::cerr.flush();
        
        return true;
      }
    
      if(verbose_ >= Start::VerboseType::commands)
      {
        std::cerr << "Command " << command << std::endl;
        std::cerr.flush();
      }
    }

    status = execute_command(command.c_str());

    if(status == 0)
    {
      if(operation_ != OP_STATUS && !status_check.empty())
      {
        unsigned long service_probes = cluster_->parameters().present() ?
          cluster_->parameters()->service_probes() : 1;

        ACE_Time_Value service_probes_period(
          cluster_->parameters().present() ?
          cluster_->parameters()->service_probes_period() : 2);

        unsigned long i = 0;
      
        for(; i < service_probes; i++)
        {
          if(i || svr.status_check_delay)
          {            
            ACE_OS::sleep(service_probes_period);
            
            service_probes_period =
              ACE_Time_Value(service_probes_period.sec() * 2);
          }

          status = execute_command(status_check.c_str(), true);
          
          if((status == 0) == (operation_ == OP_START))
          {
            break;
          }          

          WriteGuard guard(lock_);
          std::cerr << service
                    << (operation_ == OP_START ? " is not started yet on " :
                        " is not stopped yet on ") << host << std::endl;

          std::cerr.flush();
        }

        if(i == service_probes)
        {
          WriteGuard guard(lock_);
        
          std::cerr << "ElStart: can't ensure " << service
                    << (operation_ == OP_START ? " is started on " :
                        " is stopped on ") << host << "; giving up"
                    << std::endl;
          
          std::cerr.flush();
          
          return operation_ == OP_STOP;
        }
      } 

      WriteGuard guard(lock_);
    
      switch(operation_)
      {
      case OP_START:
        {
          std::cerr << service << " is started on " << host << std::endl;
          break;
        }
      case OP_STATUS:
        {
          std::cerr << service << " is running on " << host << std::endl;
          break;
        }
      case OP_STOP:
        {
          std::cerr << service << " is stopped on " << host << std::endl;
          break;
        }
      default: break;
      }
      
      std::cerr.flush();
    }
    else
    {
      WriteGuard guard(lock_);
      
      if(operation_ == OP_STATUS)
      {
        std::cerr << service << " is NOT running on " << host << std::endl;
        std::cerr.flush();
      }
      else
      {
        std::cerr << "ElStart: exit status " << status << " for command '"
                  << command << "'\n";

        std::cerr.flush();
        
        return false;
      }
    }

    if(verbose_ == Start::VerboseType::full)
    {
      std::cerr << std::endl;
      std::cerr.flush();
    }
  }
  catch(const El::String::Template::Exception& e)
  {
    WriteGuard guard(lock_);
    
    std::cerr << e << std::endl;
    return false;
  }

  return true;
}

std::string
Application::prepare_command(const char* service,
                             const char* host,
                             const char* transport,
                             const char* command_template)
  throw(Exception, El::Exception)
{
  El::String::Template::VariablesMap variables = variables_;

  variables["HOST"] = host;
  variables["NAME"] = service;
  
  El::String::Template::Parser parser(command_template, "\\", "\\");

  std::string command = parser.instantiate(variables);

  variables.erase("NAME");
  variables["COMMAND"] = command;

  parser.parse(transport, "\\", "\\");
  return parser.instantiate(variables);
}

int
Application::execute_command(const char* command,
                             bool describe_error)
  throw(Exception, El::Exception)
{
  pid_t pid = fork();

  if(pid == 0)
  {
    //
    // Child process goes here
    //
    const char* argv[4] = 
      {
        "sh",
        "-c",
        "",
        0
      };

    argv[2] = strdup(command);
  
    int result = execvp(argv[0], (char* const*)argv);

    free((void*)argv[2]);

    if(result != 0)
    {
      if(describe_error)
      {
        int error = ACE_OS::last_error();

        std::cerr << "ElStart: execvp failed with code " << error
                  << " trying to execute command sh -c " << command
                  << "\nError Description: " << ACE_OS::strerror(error)
                  << std::endl;
      }
      
      _exit(-1);
    }

    std::cerr << "ElStart: shouldn't be here !\n";
    _exit(-1);
  }

  int status = 0;
  pid_t ecode = waitpid(pid, &status, 0);

  if(ecode == pid)
  {
    if(!WIFEXITED(status))
    {
      std::ostringstream ostr;
      ostr << "ElStart: abnormal termination of command " << command;
        
      throw Exception(ostr.str());
    }
  }
  else
  {
    int error = ACE_OS::last_error();    

    std::ostringstream ostr;
    ostr << "ElStart: waitpid failed with code " << error
         << " when executing command " << command << "\n Error Description: "
         << ACE_OS::strerror(error);

    throw Exception(ostr.str());
  }

  return WEXITSTATUS(status);
}
