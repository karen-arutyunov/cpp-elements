/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/CORBA/Adapter/MICO/Corba.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_CORBA_ADAPTER_MICO_CORBA_HPP_
#define _ELEMENTS_EL_CORBA_ADAPTER_MICO_CORBA_HPP_

#include <CORBA.h>

#include <unistd.h>

#include <mico/throw.h>
#include <mico/value.h>

#include <string>
#include <sstream>

#include <El/Exception.hpp>
#include <El/ArrayPtr.hpp>
#include <El/String/Manip.hpp>

namespace El
{
  namespace Corba
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);
      
    typedef CORBA::SimpleValueRefCountBase ValueRefCountBase;
//    typedef CORBA::DefaultValueRefCountBase ValueRefCountBase;

    template<typename T>
    struct ObjectVar : public ::ObjVar<T>
    {
      ObjectVar() : ::ObjVar<T>() {}
      ObjectVar(T* ptr) : ::ObjVar<T>(ptr) {}
    };
    
    template<typename T>
    struct ObjectOut : public ::ObjOut<T>
    {
    };
    
    template<typename T>
    struct ValueVar : public ::ValueVar<T>
    {
      ValueVar() : ::ValueVar<T>() {}
      ValueVar(T* ptr) : ::ValueVar<T>(ptr) {}
    };
    
    template<typename T>
    struct ValueOut : public ::ValueOut<T>
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

      void clear_bindings() throw() { }

      ~OrbAdapter() throw();
      
    private:
      
      friend class ::El::Corba::Adapter;

      OrbAdapter(int argc, char** argv)
        throw(Exception, CORBA::SystemException, El::Exception);

    private:
      CORBA::ORB_var orb_;
      std::string host_;
      unsigned long port_;
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
        : CORBA::MARSHAL(minor, status)
    {
    }

    //
    // BootManager class
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
      El::ArrayPtr<char*> orb_argv(new char* [argc + 2]);
        
      for (int i = 0; i < argc; i++)
      {
        if(strcmp(argv[i], "--corba-host") == 0)
        {
          if(i + 1 < argc)
          {
            host_ = argv[++i];
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
            if(!El::String::Manip::numeric(argv[++i], port_))
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
        else if(strcmp(argv[i], "--corba-thread-pool") == 0)
        {
          if(i + 1 < argc)
          {
            orb_argv[orb_argc++] = "-ORBThreadPool";
            orb_argv[orb_argc++] = "-ORBRequestLimit";
            orb_argv[orb_argc++] = argv[++i];
            
          }
          else
          {
            throw Exception("--corba-thread-pool should be followed with "
                            "a thread count");
          }
        }
      }

      std::string addr;

      if(port_)
      {
        if(host_.empty())
        {
          char buff[1024];
        
          if(::gethostname(buff, sizeof(buff)) != 0)
          {
            int error = ACE_OS::last_error();
            
            std::ostringstream ostr;
            ostr << "El::Corba::Adapter(MICO): gethostname failed. Errno "
                 << error << ". Description:\n" << ACE_OS::strerror(error);
            
            throw Exception(ostr.str().c_str());
          }
          
          host_ = buff;
        }

        std::ostringstream ostr;
        ostr << "inet:" << host_ << ":" << port_;

        addr = ostr.str();

        orb_argv[orb_argc++] = "-ORBIIOPAddr";
        orb_argv[orb_argc++] = const_cast<char*>(addr.c_str());
      }
      
      orb_argv[orb_argc] = 0;
      orb_ = CORBA::ORB_init(orb_argc, orb_argv.get());

      if(CORBA::is_nil(orb_.in()))
      {
        throw Exception("El::Corba::Adapter(MICO): CORBA::ORB_init failed");
      }      
    }

    inline
    OrbAdapter::~OrbAdapter() throw()
    {
      clear_bindings();
      orb_->destroy();
    }

    inline
    void
    OrbAdapter::orb_run() throw(CORBA::SystemException)
    {
      orb_->run();
    }

    inline
    std::string
    OrbAdapter::add_binding(const char* key,
                            CORBA::Object_ptr object,
                            const char* poa_name)
      throw(Exception, CORBA::SystemException, El::Exception)
    {
      if(!port_)
      {
        throw Exception("El::Corba::OrbAdapter(MICO)::add_binding: "
                        "valid endpoint were not specified");
      }
      
      std::ostringstream ostr;
      ostr << "corbaloc:iiop:" << host_ << ":" << port_ << "/Default/"
           << poa_name << "/" << key;
      
      return ostr.str();
    }
  }
}

#endif // _ELEMENTS_EL_CORBA_ADAPTER_MICO_CORBA_HPP_
