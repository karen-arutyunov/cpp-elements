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
 * @file Elements/El/Net/SMTP/Session.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_NET_SMTP_SESSION_HPP_
#define _ELEMENTS_EL_NET_SMTP_SESSION_HPP_

#include <vector>
#include <string>
#include <memory>
#include <sstream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/BinaryStream.hpp>
#include <El/Net/Socket/Stream.hpp>

namespace El
{
  namespace Net
  {
    namespace SMTP
    {
      EL_EXCEPTION(Exception, ::El::Net::Exception);
      EL_EXCEPTION(InvalidArg, Exception);
      EL_EXCEPTION(Timeout, Exception);

      struct CommandFailure : public Exception
      {
        CommandFailure(const char* command_val,
                       int code_val,
                       const std::string& info_val) throw();

        ~CommandFailure() throw() {}

        int code;
        std::string command;
        std::string info;
      };
      
      struct Message
      {
        typedef std::vector<std::string> RecipientArray;
        
        std::string from;
        RecipientArray recipients;
        std::string subject;
        std::string text;
        std::string html;

        void write(El::BinaryOutStream& bstr) const throw(El::Exception);
        void read(El::BinaryInStream& bstr) throw(El::Exception);        
      };

      class Session
      {
      public:

        enum BodyEncoding
        {
          BE_NONE,
          BE_RAW,
          BE_BASE64,
          BE_QUOTED_PRINTABLE
        };
        
      public:
        
        Session(const char* host,
                unsigned short port = 25,
                const ACE_Time_Value* connect_timeout = 0,
                const ACE_Time_Value* send_timeout = 0,
                const ACE_Time_Value* recv_timeout = 0)
          throw(InvalidArg, Timeout, CommandFailure, Exception, El::Exception);

        Session() throw() {}
        
        virtual ~Session() throw() { close(); }

        void open(const char* host,
                  unsigned short port = 25,
                  const ACE_Time_Value* connect_timeout = 0,
                  const ACE_Time_Value* send_timeout = 0,
                  const ACE_Time_Value* recv_timeout = 0)
          throw(InvalidArg, Timeout, CommandFailure, Exception, El::Exception);

        void close() throw();
        
        void send(const Message& msg, BodyEncoding encoding = BE_BASE64)
          throw(InvalidArg, Timeout, Exception, El::Exception);        

      private:

        struct Response
        {
          int code;
          std::string info;

          Response() : code(0) {}
        };

        Response get_response(bool flush = true)
          throw(Timeout, Exception, El::Exception);
        
        bool parse_response(const char* line, Session::Response& res)
          throw(Exception, El::Exception);

        static bool valid_email(const char* email) throw();
        
      private:

        typedef std::auto_ptr<El::Net::Socket::Stream> SocketStreamPtr;        
        SocketStreamPtr stream_;
        
      private:
        Session(const Session&);
        void operator=(const Session&);
      };
    } 
  }  
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Net
  {
    namespace SMTP
    {
      //
      // CommandFailure struct
      //
      inline
      CommandFailure::CommandFailure(const char* command_val,
                                     int code_val,
                                     const std::string& info_val) throw()
          : Exception(""),
            code(code_val),
            command(command_val),
            info(info_val)
      {
        std::ostringstream ostr;
        ostr << "SMTP command failed with code " << code << "; " << info;
        init(ostr.str());
      }

      //
      // Session struct
      //
      inline
      bool
      Session::valid_email(const char* email) throw()
      {
        if(email)
        {
          const char* p = strchr(email, '@');

          if(p)
          {
            return p != email && *(p + 1) != '\0';
          }
        }

        return false;
      }

      //
      // Message struct
      //
      inline
      void
      Message::write(El::BinaryOutStream& bstr) const throw(El::Exception)
      {
        bstr << from << subject << text << html;
        bstr.write_array(recipients);
      }
      
      inline
      void
      Message::read(El::BinaryInStream& bstr) throw(El::Exception)
      {
        bstr >> from >> subject >> text >> html;
        bstr.read_array(recipients);
      }      
    }
  }
}

#endif // _ELEMENTS_EL_NET_SMTP_SESSION_HPP_
