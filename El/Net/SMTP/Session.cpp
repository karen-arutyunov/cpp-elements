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
 * @file Elements/El/Net/SMTP/Session.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string.h>

#include <string>
#include <sstream>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/Net/Socket/Stream.hpp>

#include "Session.hpp"

namespace
{
  const char CRLF[] = "\r\n";
  const char BOUNDARY[] = "EL_NSMB_2BB6805BB6F15F40EEA0F8B6C00666EA";
};

namespace El
{
  namespace Net
  {
    namespace SMTP
    {
      //
      // Session class
      //
      Session::Session(const char* host,
                       unsigned short port,
                       const ACE_Time_Value* connect_timeout,
                       const ACE_Time_Value* send_timeout,
                       const ACE_Time_Value* recv_timeout)
        throw(InvalidArg, Timeout, CommandFailure, Exception, El::Exception)
      {
        open(host, port, connect_timeout, send_timeout, recv_timeout);
      }

      void
      Session::open(const char* host,
                    unsigned short port,
                    const ACE_Time_Value* connect_timeout,
                    const ACE_Time_Value* send_timeout,
                    const ACE_Time_Value* recv_timeout)
        throw(InvalidArg, Timeout, CommandFailure, Exception, El::Exception)
      {
        close();
        
        try
        {
          stream_.reset(
            new El::Net::Socket::Stream(host,
                                        port,
                                        connect_timeout,
                                        send_timeout,
                                        recv_timeout));

          Response res = get_response(false);

          if(res.code != 220)
          {
            throw CommandFailure("CONNECT", res.code, res.info);
          }

          *stream_ << "EHLO 127.0.0.1" << CRLF;
          res = get_response();

          if(res.code != 250)
          {
            throw CommandFailure("EHLO", res.code, res.info);
          }          
        }
        catch(const El::Net::Socket::InvalidArg& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::SMTP::Session::Session: "
            "El::Net::Socket::InvalidArg caught. Description\n" << e;

          throw InvalidArg(ostr.str());
        }
        catch(const El::Net::Socket::Stream::Timeout& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::SMTP::Session::Session: "
            "El::Net::Socket::Stream::Timeout caught. Description\n" << e;

          throw Timeout(ostr.str());
        }
        catch(const Exception&)
        {
          close();
          throw;
        }
        catch(const El::Exception& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::SMTP::Session::Session: "
            "El::Exception caught. Description\n" << e;

          throw Exception(ostr.str());
        }
      }

      void
      Session::close() throw()
      {
        try
        {
          if(stream_.get())
          {
            *stream_ << "QUIT" << CRLF;
            get_response();
            stream_.reset(0);
          }
        }
        catch(...)
        {
        }
      }

      void
      Session::send(const Message& msg,
                    BodyEncoding encoding)
        throw(InvalidArg, Timeout, Exception, El::Exception)
      {
        if(stream_.get() == 0)
        {
          throw Exception("El::Net::SMTP::Session::send: session not opened");
        }

        if(msg.subject.empty() && msg.text.empty() && msg.html.empty())
        {
          throw InvalidArg("El::Net::SMTP::Session::send: "
                          "no message subject no body specified");
        }

        std::string trimmed_from;
        El::String::Manip::trim(msg.from.c_str(), trimmed_from);
        
        if(!valid_email(trimmed_from.c_str()))
        {
          std::ostringstream ostr;
          ostr << "El::Net::SMTP::Session::send: source email '" << msg.from
               << "' is invalid";

          throw InvalidArg(ostr.str());
        }

        if(msg.recipients.empty())
        {
          throw InvalidArg("El::Net::SMTP::Session::send: "
                          "no recipients specified");
        }

        Message::RecipientArray recipients;
        recipients.reserve(msg.recipients.size());

        for(Message::RecipientArray::const_iterator i(msg.recipients.begin()),
              e(msg.recipients.end()); i != e; ++i)
        {
          std::string trimmed;
          El::String::Manip::trim(i->c_str(), trimmed);
        
          if(!valid_email(trimmed.c_str()))
          {
            std::ostringstream ostr;
            ostr << "El::Net::SMTP::Session::send: destination email '"
                 << *i << "' is invalid";
            
            throw InvalidArg(ostr.str());
          }

          recipients.push_back(trimmed);
        }        
        
        try
        {
          *stream_ << "RSET" << CRLF;
          Response res = get_response();          

          if(res.code != 250)
          {
            throw CommandFailure("RSET", res.code, res.info);
          }

          *stream_ << "MAIL FROM: <" << trimmed_from << ">" << CRLF;
          res = get_response();          

          if(res.code != 250)
          {
            throw CommandFailure("MAIL", res.code, res.info);
          }

          for(Message::RecipientArray::const_iterator i(recipients.begin()),
                e(recipients.end()); i != e; ++i)
          {
            *stream_ << "RCPT TO: <" << *i << ">" << CRLF;
            res = get_response();          
            
            if(res.code != 250)
            {
              throw CommandFailure("RCPT", res.code, res.info);
            } 
          }

          *stream_ << "DATA" << CRLF;
          res = get_response();          

          if(res.code != 354)
          {
            throw CommandFailure("DATA", res.code, res.info);
          }          

          *stream_ << "From: " << trimmed_from << CRLF << "To:";

          for(Message::RecipientArray::const_iterator b(recipients.begin()),
                i(b), e(recipients.end()); i != e; ++i)
          {
            if(i != b)
            {
              *stream_ << ",";
            }
            
            *stream_ << " " << *i;
          }

          *stream_ << CRLF;

          if(!msg.subject.empty())
          {
            *stream_ << "Subject: ";
            
            El::String::Manip::utf8_word_encode(msg.subject.c_str(), *stream_);
            *stream_ << CRLF;
          }

          bool multipart = !msg.text.empty() && !msg.html.empty();

          if(multipart)
          {
            *stream_ << "Content-Type: multipart/alternative; boundary=\""
                     << BOUNDARY << "\"" << CRLF;
          }

          if(!msg.text.empty())
          {
            if(multipart)
            {
              *stream_ << "--" << BOUNDARY << CRLF;
            }
            
            switch(encoding)
            {
            case BE_NONE:
              {
                *stream_  << "Content-Type: text/plain; charset=\"utf-8\""
                          << CRLF << CRLF << msg.text;
                break;
              }
            case BE_RAW:
              {
                *stream_ << msg.text;
                break;
              }
            case BE_BASE64:
              {
                *stream_  << "Content-Type: text/plain; charset=\"utf-8\""
                          << CRLF << "Content-Transfer-Encoding: base64"
                          << CRLF << CRLF;
                
                El::String::Manip::base64_encode(
                  (const unsigned char*)msg.text.c_str(),
                  msg.text.length(),
                  *stream_,
                  19);
                
                break;
              }
            case BE_QUOTED_PRINTABLE:
              {
                *stream_<< "Content-Type: text/plain; charset=\"utf-8\""
                        << CRLF << "Content-Transfer-Encoding: quoted-printable"
                        << CRLF << CRLF;
                
                El::String::Manip::quoted_printable_encode(
                  (const unsigned char*)msg.text.c_str(),
                  msg.text.length(),
                  *stream_);

                break;
              }
            }
            
            *stream_ << CRLF;
          }

          if(!msg.html.empty())
          {
            if(multipart)
            {
              *stream_ << "--" << BOUNDARY << CRLF;
            }            

            switch(encoding)
            {
            case BE_NONE:
              {
                *stream_  << "Content-Type: text/html; charset=\"utf-8\""
                          << CRLF << CRLF << msg.html;
                break;
              }
            case BE_RAW:
              {
                *stream_ << msg.html;
                break;
              }
            case BE_BASE64:
              {
                *stream_ << "Content-Type: text/html; charset=\"utf-8\""
                         << CRLF << "Content-Transfer-Encoding: base64" << CRLF
                         << CRLF;
                
                El::String::Manip::base64_encode(
                  (const unsigned char*)msg.html.c_str(),
                  msg.html.length(),
                  *stream_,
                  19);
                
                break;
              }
            case BE_QUOTED_PRINTABLE:
              {
                *stream_  << "Content-Type: text/html; charset=\"utf-8\""
                          << CRLF
                          << "Content-Transfer-Encoding: quoted-printable"
                          << CRLF << CRLF;
                
                El::String::Manip::quoted_printable_encode(
                  (const unsigned char*)msg.html.c_str(),
                  msg.html.length(),
                  *stream_);

                break;
              }
            }

            *stream_  << CRLF;
          }
          
          if(multipart)
          {
            *stream_ << "--" << BOUNDARY << "--" << CRLF;
          }
          
          *stream_ << "." << CRLF;  

          res = get_response();

          if(res.code != 250)
          {
            throw CommandFailure("BODY", res.code, res.info);
          }          
        }
        catch(const El::Net::Socket::Stream::Timeout& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::SMTP::Session::send: "
            "El::Net::Socket::Stream::Timeout caught. Description\n" << e;

          throw Timeout(ostr.str());
        }
      }
      
      Session::Response
      Session::get_response(bool flush)
        throw(Timeout, Exception, El::Exception)
      {
        Response result;
        
        try
        {
          if(flush)
          {
            stream_->flush();

            if(stream_->last_error())
            {
              std::ostringstream ostr;
              ostr << "El::Net::SMTP::Session::get_response: "
                "failed to write to server. Error description:\n"
                   << stream_->last_error_desc();
              
              throw Exception(ostr.str());
            }
          }
        
          std::string line;
          bool cont = true;
        
          while(cont)
          {
            if(!std::getline(*stream_, line))
            {
              std::ostringstream ostr;
              ostr << "El::Net::SMTP::Session::get_response: "
                "failed to read server response. Error description:\n"
                   << stream_->last_error_desc();
            
              throw Exception(ostr.str());
            }

            Response res;
            cont = parse_response(line.c_str(), res);

            if(result.code == 0)
            {
              result.code = res.code;
              result.info.swap(res.info);
            }
          }
        }
        catch(const El::Net::Socket::Stream::Timeout& e)
        {
          std::ostringstream ostr;
          ostr << "El::Net::SMTP::Session::get_response: "
            "El::Net::Socket::Stream::Timeout caught. Description\n" << e;

          throw Timeout(ostr.str());
        }
        
        return result;
      }

      bool
      Session::parse_response(const char* line, Session::Response& res)
        throw(Exception, El::Exception)
      {
        size_t p = strcspn(line, "- ");
        char sep = line[p];

        if(sep == '\0')
        {
          std::ostringstream ostr;
          ostr << "El::Net::SMTP::Session::parse_response: "
            "no end of code in response line:\n" << line;
          
          throw Exception(ostr.str());
        }

        bool cont = sep == '-';
        std::string code_str(line, p);
        
        if(!El::String::Manip::numeric(code_str.c_str(), res.code))
        {
          std::ostringstream ostr;
          ostr << "El::Net::SMTP::Session::parse_response: "
            "no int code in response line:\n" << line;
          
          throw Exception(ostr.str());
        }

        El::String::Manip::trim(line + p + 1, res.info);
        return cont;
      }
    }
  }
}

