/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/CORBA/Adapter/ORBacus/Corba.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_CORBA_ADAPTER_ORBACUS_CORBA_HPP_
#define _ELEMENTS_EL_CORBA_ADAPTER_ORBACUS_CORBA_HPP_

#include <CORBA.h>
#include <OB/BootManager.h>
#include <OB/OCI_IIOP.h>

#include <set>
#include <string>
#include <sstream>
#include <memory>

#include <ace/OS.h>
#include <ace/Synch_T.h>

#include <El/Exception.hpp>
#include <El/ArrayPtr.hpp>
#include <El/String/Manip.hpp>

namespace El
{
  namespace Corba
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);

    typedef CORBA::DefaultValueRefCountBase ValueRefCountBase;

    template<typename T>
    struct ObjectVar : public ::OB::ObjVar<T>
    {
      ObjectVar() : ::OB::ObjVar<T>() {}
      ObjectVar(T* ptr) : ::OB::ObjVar<T>(ptr) {}
    };
    
    template<typename T>
    struct ObjectOut : public ::OB::ObjOut<T>
    {
    };
    
    template<typename T>
    struct ValueVar : public ::OB::ObjVar<T>
    {
      ValueVar() : ::OB::ObjVar<T>() {}
      ValueVar(T* ptr) : ::OB::ObjVar<T>(ptr) {}
    };
    
    template<typename T>
    struct ValueOut : public ::OB::ObjOut<T>
    {
    };
    
    template<typename T>
    struct ValueFactory : public virtual CORBA::ValueFactoryBase
    {
      struct T_Impl : public T,
                      public virtual ValueRefCountBase
//                      public virtual CORBA::DefaultValueRefCountBase
      {
      };

      virtual CORBA::ValueBase* create_for_unmarshal() { return new T_Impl(); }

      static T_Impl* create() { return new T_Impl(); }
    };
    
    struct MARSHAL : public CORBA::MARSHAL
    {
      MARSHAL(const char* desc = "",
              CORBA::ULong minor = 0,
              CORBA::CompletionStatus status = CORBA::COMPLETED_NO);
/* 
      SystemException(const char* = "");
      SystemException(ULong, CompletionStatus);
      SystemException(const char*, ULong, CompletionStatus);
      SystemException(const SystemException&);
      SystemException& operator=(const SystemException&);
*/
    };

    class OrbAdapter
    {
    public:

      CORBA::ORB* orb() const throw() { return orb_.in(); }

      void orb_run() throw(CORBA::SystemException);

      std::string add_binding(const char* key,
                              CORBA::Object_ptr object,
                              const char* poa_name)
        throw(Exception, CORBA::SystemException, El::Exception);

      void clear_bindings() throw() { boot_manager_->clear(); }

      ~OrbAdapter() throw();
    
    private:

      friend class ::El::Corba::Adapter;

      OrbAdapter(int argc, char** argv)
        throw(Exception, CORBA::SystemException, El::Exception);


      class BootManager
      {
      public:
        BootManager(CORBA::ORB_ptr orb)
          throw(Exception, CORBA::SystemException, El::Exception);
        
        ~BootManager() throw() { clear(); }
        
        void clear() throw();
        
        void add_binding(const char* key, CORBA::Object_ptr object)
          throw(Exception, CORBA::SystemException, El::Exception);
        
      private:
        typedef std::set<PortableServer::ObjectId_var> ObjectIdSet;
        
        OB::BootManager_var boot_manager_;
        ObjectIdSet ids_;
      };      
      
    private:
      CORBA::ORB_var orb_;
      std::string host_;
      unsigned long port_;
      std::auto_ptr<BootManager> boot_manager_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Corba
  {
    //
    // MARSHAL struct
    //
    inline
    MARSHAL::MARSHAL(const char* desc,
                     CORBA::ULong minor,
                     CORBA::CompletionStatus status)
        : CORBA::MARSHAL(desc, minor, status)
    {
    }

    //
    // OrbAdapter class
    //
    inline
    void
    OrbAdapter::orb_run() throw(CORBA::SystemException)
    {
      orb_->run();
    }

    //
    // OrbAdapter::BootManager class
    //
    inline
    OrbAdapter::BootManager::BootManager(CORBA::ORB_ptr orb)
      throw(Exception, CORBA::SystemException, El::Exception)
    {
      CORBA::Object_var bmgr_obj = 
        orb->resolve_initial_references("BootManager");
      
      if(CORBA::is_nil(bmgr_obj))
      {        
        throw Exception("El::Corba::BootManager::BootManager: "
                        "unable to obtain BootManager object: "
                        "resolve_initial_references() failed.");
      }
      
      boot_manager_ = OB::BootManager::_narrow(bmgr_obj.in());
    }
    
    inline
    void
    OrbAdapter::BootManager::add_binding(const char* key,
                                         CORBA::Object_ptr object)
      throw(Exception, CORBA::SystemException, El::Exception)
    {
      PortableServer::ObjectId_var obj_id =
        PortableServer::string_to_ObjectId(key);
      
      boot_manager_->add_binding(obj_id, object);
    }

    inline
    void
    OrbAdapter::BootManager::clear() throw()
    {
      for(ObjectIdSet::iterator it = ids_.begin(); it != ids_.end(); it++)
      {
        boot_manager_->remove_binding(*it);
      }
    }

    //
    // OrbAdapter class
    //
    inline
    OrbAdapter::OrbAdapter(int argc, char** argv)
      throw(Exception, CORBA::SystemException, El::Exception)
        : port_(0)
    {
      //
      // Note: very limited number of ORB arguments supported
      //
      int orb_argc = 0;
      El::ArrayPtr<char*> orb_argv(new char* [argc + 1]);
      
      for (int i = 0; i < argc; i++)
      {
//        std::cerr << " *" << argv[i];
        
        if(strcmp(argv[i], "--corba-host") == 0)
        {
          if(i + 1 < argc)
          {
            orb_argv[orb_argc++] = "-IIOPhost";

            char* host = argv[++i];
            orb_argv[orb_argc++] = host;
            host_ = host;
          }
          else
          {
            throw Exception("--corba-host should be followed with "
                            "actual hostname(s)");
          }
        }
        else if(strcmp(argv[i], "--corba-port") == 0)
        {
          if(i + 1 < argc)
          {
            orb_argv[orb_argc++] = "-IIOPport";

            char* port = argv[++i];
            orb_argv[orb_argc++] = port;

            if(!El::String::Manip::numeric(port, port_))
            {
              throw Exception("--corba-port should be followed with "
                              "unsigned integer");
            }
          }
          else
          {
            throw Exception("--corba-port should be followed with "
                            "actual port number");
          }
        }
        else if(strcmp(argv[i], "--corba-reactive") == 0)
        {
          orb_argv[orb_argc++] = "-ORBreactive";
        }        
        else if(strcmp(argv[i], "--corba-thread-pool") == 0)
        {
//          std::cerr << " -" << i << ":" << argc << "-";
          
          if(i + 1 < argc)
          {
            orb_argv[orb_argc++] = "-OAthread_pool";
            orb_argv[orb_argc++] = argv[++i];
          }
          else
          {
            throw Exception("--corba-thread-pool should be followed with "
                            "a thread count");
          }
        } 
      }

      if(port_)
      {
        if(host_.empty())
        {
          char buff[1024];
          
          if(::gethostname(buff, sizeof(buff)) != 0)
          {
            int error = ACE_OS::last_error();
            
            std::ostringstream ostr;
            ostr << "El::Corba::Adapter(ORBacus): gethostname failed. Errno "
                 << error << ". Description:\n" << ACE_OS::strerror(error);
            
            throw Exception(ostr.str().c_str());
          }
          
          host_ = buff;
        }
      }
      
      orb_argv[orb_argc] = 0;
/*
      std::cerr << "\n================\n";
      
      for(long i = 0; i < orb_argc; i++)
      {
        std::cerr << " " << orb_argv[i];
      }
*/
      orb_ = CORBA::ORB_init(orb_argc, orb_argv.get());

      if(CORBA::is_nil(orb_.in()))
      {
        throw Exception("El::Corba::Adapter(ORBacus): CORBA::ORB_init failed");
      }
      
      boot_manager_.reset(new BootManager(orb_.in()));
    }

    inline
    OrbAdapter::~OrbAdapter() throw()
    {
      clear_bindings();
      boot_manager_.reset(0);

      orb_->destroy();
    }

    inline
    std::string
    OrbAdapter::add_binding(const char* key,
                            CORBA::Object_ptr object,
                            const char* /*poa_name*/)
      throw(Exception, CORBA::SystemException, El::Exception)
    {
      if(!port_)
      {
        throw Exception("El::Corba::OrbAdapter(ORBacus)::add_binding: "
                        "valid endpoint were not specified");
      }
      
      boot_manager_->add_binding(key, object);

      std::ostringstream ostr;
      ostr << "corbaloc:iiop:" << host_ << ":" << port_ << "/" << key;
      
      return ostr.str();
    }
  }
}

#endif // _ELEMENTS_EL_CORBA_ADAPTER_ORBACUS_CORBA_HPP_
