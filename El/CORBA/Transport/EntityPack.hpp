/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   Elements/El/CORBA/Transport/EntityPack.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_EL_CORBA_TRANSPORT_ENTITYPACK_HPP_
#define _ELEMENTS_EL_CORBA_TRANSPORT_ENTITYPACK_HPP_

#include <stdint.h>

#include <memory>
#include <sstream>

#include <El/Exception.hpp>

#include <El/BinaryStream.hpp>
#include <El/Compress/ZLib.hpp>

#include <El/CORBA/Stream/OctetStream.hpp>
#include <El/CORBA/ZLib/StreamCallback.hpp>

namespace El
{
  namespace Corba
  {
    namespace Transport
    {
      enum TransferEncoding
      {
        TE_IDENTITY,
        TE_GZIP
      };
        
      EL_EXCEPTION(Exception, El::ExceptionBase);
      
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      class Entity : public virtual ENTITY_PACK_BASE,
                     public virtual ValueRefCountBase
//                     public virtual CORBA::DefaultValueRefCountBase
      {
      public:

        Entity(ENTITY* entity = 0) throw(El::Exception);
        virtual ~Entity() throw();

        ENTITY& entity() throw(Exception, El::Exception);
        ENTITY* release() throw(Exception, El::Exception);

        bool serialized() const throw();

        void serialize() throw(CORBA::Exception);
        void serialize(TransferEncoding encoding) throw(CORBA::Exception);
        
        void deserialize() throw(CORBA::Exception);
        
        //
        // IDL:omg.org/CORBA/CustomMarshal/marshal:1.0
        //
        virtual void marshal(CORBA::DataOutputStream* os);

        //
        // IDL:omg.org/CORBA/CustomMarshal/unmarshal:1.0
        //
        virtual void unmarshal(CORBA::DataInputStream* is);

      protected:
        virtual CORBA::ValueBase* _copy_value() throw(CORBA::NO_IMPLEMENT);

      protected:
        static const size_t COMPRESSION_LEVEL = 1;
        static const size_t ZLIB_BUFF_SIZE = 1000000;
        static const size_t MARSHAL_BUFF_SIZE = 1000000;
        
        typedef std::auto_ptr<ENTITY> EntityPtr;

        EntityPtr entity_;
        CORBA::OctetSeq_var packed_entity_;
        bool serialized_;
      };
      
      template<typename ENTITY, typename TRANSPORT_ENTITY>
      class Entity_init : public virtual CORBA::ValueFactoryBase
      {
      public:
        static TRANSPORT_ENTITY* create(ENTITY* entity = 0)
          throw(El::Exception);
      
        virtual CORBA::ValueBase* create_for_unmarshal();
      };
      
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      class EntityPack : public virtual ENTITY_PACK_BASE,
                         public virtual ValueRefCountBase
      {
      public:
        EntityPack(ENTITY_ARRAY* entities = 0) throw(El::Exception);
        virtual ~EntityPack() throw();

        ENTITY_ARRAY& entities() throw(Exception, El::Exception);
        ENTITY_ARRAY* release() throw(Exception, El::Exception);

        bool serialized() const throw();

        void serialize(TransferEncoding encoding)
          throw(CORBA::Exception);
        
        void serialize() throw(CORBA::Exception);
        void deserialize() throw(CORBA::Exception);
        
        //
        // IDL:omg.org/CORBA/CustomMarshal/marshal:1.0
        //
        virtual void marshal(CORBA::DataOutputStream* os);

        //
        // IDL:omg.org/CORBA/CustomMarshal/unmarshal:1.0
        //
        virtual void unmarshal(CORBA::DataInputStream* is);

      protected:
        virtual CORBA::ValueBase* _copy_value() throw(CORBA::NO_IMPLEMENT);

      protected:
        static const size_t COMPRESSION_LEVEL = 1;
        static const size_t ZLIB_BUFF_SIZE = 1000000;
        static const size_t MARSHAL_BUFF_SIZE = 1000000;
        
        typedef std::auto_ptr<ENTITY_ARRAY> EntityArrayPtr;

        EntityArrayPtr entities_;
        CORBA::OctetSeq_var packed_entities_;
        bool serialized_;
      };
      
      template<typename ENTITY, typename ENTITY_ARRAY, typename ENTITY_PACK>
      class EntityPack_init : public virtual CORBA::ValueFactoryBase
      {
      public:
        static ENTITY_PACK* create(ENTITY_ARRAY* entities = 0)
          throw(El::Exception);
      
        virtual CORBA::ValueBase* create_for_unmarshal();
      };
      
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Corba
  {
    namespace Transport
    {
      //
      // Entity class template
      //
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::Entity(
        ENTITY* entity) throw(El::Exception)
          : serialized_(false)
      {
        entity_.reset(entity);
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::~Entity() throw()
      {
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      ENTITY&
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::entity()
        throw(Exception, El::Exception)
      {
        try
        {
          deserialize();
        }
        catch(const CORBA::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::Entity::entity: "
            "CORBA::Exception caught. Description:\n" << e;
          
          throw Exception(ostr.str());
        }
        
        if(entity_.get() == 0)
        {
          throw Exception(
            "El::Corba::Transport::Entity::entity: entity_ is 0");
        }
        
        return *entity_;
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      bool
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::serialized() const
        throw()
      {
        return serialized_;
      }
      
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      ENTITY*
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::release()
        throw(Exception, El::Exception)
      {
        try
        {
          deserialize();
          return entity_.release();
        }
        catch(const CORBA::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::Entity::release: "
            "CORBA::Exception caught. Description:\n" << e;
          
          throw Exception(ostr.str());
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::Entity::release: "
            "El::Exception caught. Description:\n" << e << std::endl;
          
          throw Exception(ostr.str());
        }        
      }
      
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      CORBA::ValueBase*
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::_copy_value()
        throw(CORBA::NO_IMPLEMENT)
      {
        typedef Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING> Type;
        El::Corba::ValueVar<Type> res(new Type());

        if(serialized_)
        {
          res->packed_entity_ = new CORBA::OctetSeq();
          res->packed_entity_.inout() = packed_entity_.in();
        }
        else
        {
          res->entity_.reset(new ENTITY());
          *res->entity_ = *entity_;
        }

        return res._retn();
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::serialize()
        throw(CORBA::Exception)
      {
        serialize(DEFAULT_ENCODING);
      }
      
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::serialize(
        TransferEncoding encoding) throw(CORBA::Exception)
      {
        try
        {
          if(serialized())
          {
            return;
          }
          
          if(encoding == TE_GZIP)
          {  
            El::Corba::ZLib::OctetArrayWriter writer(MARSHAL_BUFF_SIZE);
        
            uint8_t enc = encoding;
            writer.write((const char*)&enc, sizeof(uint8_t));
        
            {
              El::Compress::ZLib::OutStream zstr(&writer,
                                                 COMPRESSION_LEVEL,
                                                 ZLIB_BUFF_SIZE,
                                                 ZLIB_BUFF_SIZE);
              
              El::BinaryOutStream bstr(zstr);              
              entity_->write(bstr);
              
              zstr.finalize();
            }

            size_t len = writer.length();
            
            packed_entity_ =
              new CORBA::OctetSeq(len, len, writer.release(), 1);
          }
          else
          {
            El::Corba::Stream::OutOctetStream ostr(MARSHAL_BUFF_SIZE);
            El::BinaryOutStream bstr(ostr);
            
            bstr << (uint8_t)encoding;
            entity_->write(bstr);

            ostr.flush();

            size_t len = ostr.length();
            packed_entity_ = new CORBA::OctetSeq(len, len, ostr.release(), 1);
          }
          
          entity_.reset(0);
          serialized_ = true;
        }
        catch(const CORBA::Exception&)
        {
          throw;
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::Entity::serialize: "
            "El::Exception caught. Description:\n" << e;
        
          throw El::Corba::MARSHAL(ostr.str().c_str());
        }
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::deserialize()
        throw(CORBA::Exception)
      {
        try
        {
          if(!serialized())
          {
            return;
          }

          size_t len = packed_entity_->length();
        
          if(!len)
          {
            throw
              El::Corba::MARSHAL("El::Corba::Transport::Entity::deserialize: "
                                 "encoding information is missed");
          }

          const CORBA::Octet* buff = packed_entity_->get_buffer();
          uint8_t encoding = buff[0];

          if(encoding > TE_GZIP)
          {
            std::ostringstream ostr;
            ostr << "El::Corba::Transport::Entity::deserialize: unexpected "
              "encoding " << (size_t)encoding << " (data len " << len
                 << ")";
        
            throw El::Corba::MARSHAL(ostr.str().c_str());
          }
            
          entity_.reset(new ENTITY());

          switch(encoding)
          {
          case TE_GZIP:
            {
              El::Corba::ZLib::OctetArrayReader
                reader(buff + 1, packed_entity_->length() - 1);
              
              El::Compress::ZLib::InStream zstr(&reader,
                                                ZLIB_BUFF_SIZE,
                                                ZLIB_BUFF_SIZE);
      
              El::BinaryInStream bstr(zstr);
              entity_->read(bstr);
              
              break;
            }
          case TE_IDENTITY:
            {
              El::Corba::Stream::InOctetStream
                istr(buff + 1, packed_entity_->length() - 1);

              El::BinaryInStream bstr(istr);
              entity_->read(bstr);
              
              break;
            }
          }
            
          packed_entity_ = 0;
          serialized_ = false;
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::Entity::deserialize: "
            "El::Exception caught. Description:\n" << e << std::endl;

          throw El::Corba::MARSHAL(ostr.str().c_str());
        }
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::marshal(
        CORBA::DataOutputStream* os)
      {
        serialize();
          
        os->write_ulong(packed_entity_->length());        
        os->write_octet_array(packed_entity_.inout(), 0, packed_entity_->length());
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      Entity<ENTITY_PACK_BASE, ENTITY, DEFAULT_ENCODING>::unmarshal(
        CORBA::DataInputStream* is)
      {
        try
        {
          CORBA::ULong length = is->read_ulong();
        
          packed_entity_ = new CORBA::OctetSeq(length);
          packed_entity_->length(length);
        
          is->read_octet_array(packed_entity_.inout(), 0, length);
        }
        catch(...)
        {
          packed_entity_ = 0;
          throw;
        }
        
        serialized_ = true;
      }      

      //
      // Entity_init class tempate
      //
      template<typename ENTITY, typename TRANSPORT_ENTITY>
      TRANSPORT_ENTITY*
      Entity_init<ENTITY, TRANSPORT_ENTITY>::
      create(ENTITY* entity) throw(El::Exception)
      {
        return new TRANSPORT_ENTITY(entity);
      }

      template<typename ENTITY, typename TRANSPORT_ENTITY>
      CORBA::ValueBase* 
      Entity_init<ENTITY, TRANSPORT_ENTITY>::create_for_unmarshal()
      {
        return create();
      }

      //
      // EntityPack class template
      //
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      EntityPack(ENTITY_ARRAY* entities)
        throw(El::Exception) : serialized_(false)
      {
        entities_.reset(entities);
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      ~EntityPack() throw()
      {
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      ENTITY_ARRAY&
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      entities() throw(Exception, El::Exception)
      {
        try
        {
          deserialize();
        }
        catch(const CORBA::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::EntityPack::entities: "
            "CORBA::Exception caught. Description:\n" << e;
          
          throw Exception(ostr.str());
        }
        
        if(entities_.get() == 0)
        {
          throw Exception("El::Corba::Transport::EntityPack::"
                          "entities: entities_ is 0");
        }
        
        return *entities_;
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      bool
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      serialized() const throw()
      {
        return serialized_;
      }
      
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      ENTITY_ARRAY*
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      release() throw(Exception, El::Exception)
      {
        try
        {
          deserialize();
          return entities_.release();
        }
        catch(const CORBA::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::EntityPack::release: "
            "CORBA::Exception caught. Description:\n" << e << std::endl;

          throw Exception(ostr.str());
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::EntityPack::release: "
            "El::Exception caught. Description:\n" << e << std::endl;
          
          throw Exception(ostr.str());
        }
      }
      
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      CORBA::ValueBase*
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      _copy_value() throw(CORBA::NO_IMPLEMENT)
      {
        typedef EntityPack<ENTITY_PACK_BASE,
          ENTITY,
          ENTITY_ARRAY,
          DEFAULT_ENCODING> Type;
        
        El::Corba::ValueVar<Type> res(new Type());

        if(serialized_)
        {
          res->packed_entities_ = new CORBA::OctetSeq();
          res->packed_entities_.inout() = packed_entities_.in();
        }
        else
        {
          res->entities_.reset(new ENTITY_ARRAY());
          *res->entities_ = *entities_;
        }

        return res._retn();
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      serialize() throw(CORBA::Exception)
      {
        serialize(DEFAULT_ENCODING);
      }
      
      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      serialize(TransferEncoding encoding) throw(CORBA::Exception)
      {
        try
        {
          if(serialized())
          {
            return;
          }
          
          if(encoding == TE_GZIP)
          {  
            El::Corba::ZLib::OctetArrayWriter writer(MARSHAL_BUFF_SIZE);

            uint8_t enc = encoding;
            writer.write((const char*)&enc, sizeof(uint8_t));
        
            {
              El::Compress::ZLib::OutStream zstr(&writer,
                                                 COMPRESSION_LEVEL,
                                                 ZLIB_BUFF_SIZE,
                                                 ZLIB_BUFF_SIZE);
              
              El::BinaryOutStream bstr(zstr);
              bstr.write_array(*entities_);

/*  
              bstr << (uint64_t)entities_->size();
              
              for(typename ENTITY_ARRAY::const_iterator it =
                    entities_->begin(); it != entities_->end(); it++)
              {
                bstr << *it;
              }
*/
              
              zstr.finalize();
            }

            size_t len = writer.length();
            
            packed_entities_ =
              new CORBA::OctetSeq(len, len, writer.release(), 1);
          }
          else
          {
            El::Corba::Stream::OutOctetStream ostr(MARSHAL_BUFF_SIZE);
            El::BinaryOutStream bstr(ostr);

            bstr << (uint8_t)encoding;
            bstr.write_array(*entities_);
            
/*            
            bstr << (uint8_t)encoding << (uint64_t)entities_->size();
            
            for(typename ENTITY_ARRAY::const_iterator it =
                  entities_->begin(); it != entities_->end(); it++)
            {
              bstr << *it;
            }
*/

            ostr.flush();

            size_t len = ostr.length();
            
            packed_entities_ =
              new CORBA::OctetSeq(len, len, ostr.release(), 1);
          }
          
          entities_.reset(0);
          serialized_ = true;
        }
        catch(const CORBA::Exception&)
        {
          throw;
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::EntityPack::serialize: "
            "El::Exception caught. Description:\n" << e;
        
          throw El::Corba::MARSHAL(ostr.str().c_str());
        }
        
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      deserialize() throw(CORBA::Exception)
      {
        try
        {
          if(!serialized())
          {
            return;
          }

          size_t len = packed_entities_->length();
          
          if(!len)
          {
            throw El::Corba::MARSHAL(
              "El::Corba::Transport::EntityPack::"
              "deserialize: encoding information is missed");
          }

          const CORBA::Octet* buff = packed_entities_->get_buffer();
          uint8_t encoding = buff[0];

          if(encoding > TE_GZIP)
          {
            std::ostringstream ostr;
            ostr << "El::Corba::Transport::EntityPack::deserialize: "
              "unexpected encoding " << (size_t)encoding
                 << " (data len " << len << ")";
        
            throw El::Corba::MARSHAL(ostr.str().c_str());
          }
            
          entities_.reset(new ENTITY_ARRAY());

          switch(encoding)
          {
          case TE_GZIP:
            {
              El::Corba::ZLib::OctetArrayReader
                reader(buff + 1, packed_entities_->length() - 1);
              
              El::Compress::ZLib::InStream zstr(&reader,
                                                ZLIB_BUFF_SIZE,
                                                ZLIB_BUFF_SIZE);
      
              El::BinaryInStream bstr(zstr);
              bstr.read_array(*entities_);
/*
              uint64_t size = 0;
      
              bstr >> size;
              entities_->resize(size);
      
              for(typename ENTITY_ARRAY::iterator it = entities_->begin();
                  it != entities_->end(); it++)
              {
                bstr >> *it;
              }
*/
              break;
            }
          case TE_IDENTITY:
            {
              El::Corba::Stream::InOctetStream
                istr(buff + 1, packed_entities_->length() - 1);

              El::BinaryInStream bstr(istr);
              bstr.read_array(*entities_);

/*              
              uint64_t size = 0;
      
              bstr >> size;
              entities_->resize(size);
      
              for(typename ENTITY_ARRAY::iterator it = entities_->begin();
                  it != entities_->end(); it++)
              {
                bstr >> *it;
              }
*/              
              break;
            }
          }
            
          packed_entities_ = 0;
          serialized_ = false;
        }
        catch(const CORBA::Exception&)
        {
          throw;
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Corba::Transport::EntityPack::deserialize: "
            "El::Exception caught. Description:\n" << e;
        
          throw El::Corba::MARSHAL(ostr.str().c_str());
        }
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      marshal(CORBA::DataOutputStream* os)
      {
        serialize();
          
        os->write_ulong(packed_entities_->length());
        
        os->write_octet_array(packed_entities_.inout(),
                              0,
                              packed_entities_->length());        
      }

      template<typename ENTITY_PACK_BASE,
               typename ENTITY,
               typename ENTITY_ARRAY,
               const TransferEncoding DEFAULT_ENCODING>
      void
      EntityPack<ENTITY_PACK_BASE, ENTITY, ENTITY_ARRAY, DEFAULT_ENCODING>::
      unmarshal(CORBA::DataInputStream* is)
      {
        try
        {
          CORBA::ULong length = is->read_ulong();
        
          packed_entities_ = new CORBA::OctetSeq(length);
          packed_entities_->length(length);
          
          is->read_octet_array(packed_entities_.inout(), 0, length);
        }
        catch(...)
        {
          packed_entities_ = 0;
          throw;
        }
        
        serialized_ = true;
      }      

      //
      // EntityPack_init class tempate
      //
      template<typename ENTITY, typename ENTITY_ARRAY, typename ENTITY_PACK>
      ENTITY_PACK*
      EntityPack_init<ENTITY, ENTITY_ARRAY, ENTITY_PACK>::
      create(ENTITY_ARRAY* entities) throw(El::Exception)
      {
        return new ENTITY_PACK(entities);
      }

      template<typename ENTITY, typename ENTITY_ARRAY, typename ENTITY_PACK>
      CORBA::ValueBase* 
      EntityPack_init<ENTITY, ENTITY_ARRAY, ENTITY_PACK>::
      create_for_unmarshal()
      {
        return create();
      }
    }
  }
}

#endif // _ELEMENTS_EL_CORBA_TRANSPORT_ENTITYPACK_HPP_
