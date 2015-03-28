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
 * @file   Elements/El/CORBA/Adapter/TAO/Corba.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_CORBA_ADAPTER_TAO_CORBA_HPP_
#define _ELEMENTS_EL_CORBA_ADAPTER_TAO_CORBA_HPP_

#include <tao/corba.h>
#include <tao/Objref_VarOut_T.h>
#include <tao/Valuetype/Value_VarOut_T.h>
#include <tao/Valuetype/ValueBase.h>
#include <tao/Valuetype/ValueFactory.h>
#include <tao/Valuetype/CustomValuetypeC.h>
#include <tao/IORTable/IORTable.h>

#include <El/Exception.hpp>
#include <El/ArrayPtr.hpp>
#include <El/String/Manip.hpp>
#include <El/Service/ServiceBase.hpp>
#include <El/RefCount/DefaultImpl.hpp>

namespace El
{
  namespace Corba
  {
    EL_EXCEPTION(Exception, El::ExceptionBase);

    typedef CORBA::DefaultValueRefCountBase ValueRefCountBase;

    template<typename T>
    struct ObjectVar : public ::TAO_Objref_Var_T<T>
    {
      ObjectVar() : ::TAO_Objref_Var_T<T>() {}
      ObjectVar(T* ptr) : ::TAO_Objref_Var_T<T>(ptr) {}
    };

    template<typename T>
    struct ObjectOut : public ::TAO_Objref_Out_T<T>
    {
    };

    template<typename T>
    class ValueVar
    {
    public:
      ValueVar() : ptr_(0) {}
//      ValueVar(T* ptr) : ptr_(ptr) { ::CORBA::add_ref(ptr); }
      ValueVar(T* ptr) : ptr_(ptr) { }
      ValueVar(const ValueVar& ptr) : ptr_(ptr.ptr_) { ::CORBA::add_ref(ptr_); }
      ~ValueVar() { ::CORBA::remove_ref(ptr_); }
//      ValueVar& operator =(T* ptr) { ::CORBA::add_ref(ptr); ::CORBA::remove_ref(ptr_); ptr_ = ptr; return *this; }
      ValueVar& operator =(T* ptr) { ::CORBA::remove_ref(ptr_); ptr_ = ptr; return *this; }
      ValueVar& operator =(const ValueVar& ptr) { ::CORBA::add_ref(ptr.ptr_); ::CORBA::remove_ref(ptr_); ptr_ = ptr.ptr_; return *this; }
      T* operator ->() const { return ptr_; }
      operator const T * () const { return ptr_; }
      operator T *& () { return ptr_; };

      typedef T *  _in_type;
      typedef T *& _inout_type;
      typedef T *& _out_type;
      typedef T *  _retn_type;

      _in_type in (void) const { return ptr_; }
      _inout_type inout (void) { return ptr_; }
      _out_type out (void) { return ptr_; }
      _retn_type _retn (void) { T* tmp = ptr_; ptr_ = 0; return tmp; }

    private:
      T* ptr_;
    };

    template<typename T>
    struct ValueOut
    {
      ValueOut(T*& ptr) : ptr_(ptr) { ptr_ = 0; }
      ValueOut(ValueVar<T>& ptr) : ptr_(ptr.out()) { ::CORBA::remove_ref(ptr_); ptr_ = 0; }
      ValueOut(const ValueOut<T>& ptr) : ptr_(const_cast<ValueOut&>(ptr).ptr_) {}
      ValueOut& operator =(T* ptr) { ptr_ = ptr; return *this; }
      ValueOut& operator =(const ValueOut& ptr) { ptr_ = const_cast<ValueOut&>(ptr).ptr_; return *this; }
      T* operator ->() const { return ptr_; }
      operator T *& () { return ptr_; };

    private:
      T*& ptr_;
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

    class OrbAdapter :
      private El::Service::Callback,
      private El::Service::ServiceBase<El::Sync::ThreadPolicy>,
      private El::RefCount::DefaultImpl<>
    {
    public:
      CORBA::ORB* orb() const throw() { return orb_.in(); }

      void orb_run() throw(CORBA::SystemException, El::Exception);

      std::string add_binding(const char* key,
                              CORBA::Object_ptr object,
                              const char* poa_name)
        throw(Exception, CORBA::SystemException, El::Exception);

      void clear_bindings() throw() { }

      ~OrbAdapter() throw();

    private:

      virtual bool notify(El::Service::Event* event) throw(El::Exception);

      virtual void run() throw(Exception, El::Exception);

    private:

      friend class ::El::Corba::Adapter;

      OrbAdapter(int argc, char** argv)
        throw(Exception, CORBA::SystemException, El::Exception);

    private:
      CORBA::ORB_var orb_;
      IORTable::Table_var table_;
      std::string host_;
      unsigned long port_;
    };
  }
}

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
        : El::Service::ServiceBase<El::Sync::ThreadPolicy>(this),
          port_(0)
    {
      //
      // Note: very limited number of ORB arguments supported
      //
      int orb_argc = 0;
      El::ArrayPtr<const char*> orb_argv(new const char* [argc + 8]);

      orb_argv[orb_argc++] = "";

      for (int i = 0; i < argc; i++)
      {
        if (strcmp(argv[i], "--corba-host") == 0)
        {
          if (i + 1 < argc)
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
            if(!El::String::Manip::numeric(argv[++i], threads_))
            {
              throw Exception("--corba-thread-pool should be followed with "
                              "unsigned integer");
            }
          }
          else
          {
            throw Exception("--corba-thread-pool should be followed with "
                            "a thread count");
          }
        }
      }

      //TAO_debug_level = 100;
      //orb_argv[orb_argc++] = "-ORBGestalt";
      //orb_argv[orb_argc++] = "Local";
      orb_argv[orb_argc++] = "-ORBKeepalive";
      orb_argv[orb_argc++] = "1";
      orb_argv[orb_argc++] = "-ORBUseLocalMemoryPool";
      orb_argv[orb_argc++] = "0";

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
            ostr << "El::Corba::Adapter(TAO): gethostname failed. Errno "
                 << error << ". Description:\n" << ACE_OS::strerror(error);

            throw Exception(ostr.str().c_str());
          }

          host_ = buff;
        }

        std::ostringstream ostr;
        ostr << "iiop://" << host_ << ":" << port_;

        addr = ostr.str();

        orb_argv[orb_argc++] = "-ORBListenEndpoints";
        orb_argv[orb_argc++] = addr.c_str();
      }

      orb_argv[orb_argc] = 0;
      try
      {
        orb_ = CORBA::ORB_init(orb_argc, const_cast<char**>(orb_argv.get()));
      }
      catch (const CORBA::Exception&)
      {
      }

      if(CORBA::is_nil(orb_.in()))
      {
        throw Exception("El::Corba::Adapter(TAO): CORBA::ORB_init failed");
      }

      if (port_)
      {
        try
        {
          CORBA::Object_var tobj =
            orb_->resolve_initial_references("IORTable");
          table_ = IORTable::Table::_narrow(tobj.in());
        }
        catch (const CORBA::Exception&)
        {
        }
        if (CORBA::is_nil(table_.in()))
        {
          throw Exception("El::Corba::Adapter(TAO): "
            "IORTable::Table::_narrow failed");
        }
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
    OrbAdapter::orb_run() throw(CORBA::SystemException, El::Exception)
    {
      start();
      stop();
      wait();
    }

    inline
    bool
    OrbAdapter::notify(El::Service::Event* event) throw(El::Exception)
    {
      return false;
    }

    inline
    void
    OrbAdapter::run() throw(Exception, El::Exception)
    {
      try
      {
        orb_->run();
      }
      catch (const CORBA::SystemException&)
      {
        throw Exception("");
      }
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
        throw Exception("El::Corba::OrbAdapter(TAO)::add_binding: "
                        "valid endpoint were not specified");
      }

      CORBA::String_var ior_string = orb_->object_to_string(object);
      table_->bind(key, ior_string.in());

      std::ostringstream ostr;
      ostr << "corbaloc:iiop:" << host_ << ":" << port_ << "/" << key;

      return ostr.str();
    }
  }
}

#endif // _ELEMENTS_EL_CORBA_ADAPTER_TAO_CORBA_HPP_
