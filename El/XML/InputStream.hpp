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
 * @file Elements/El/XML/InputStream.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_XML_INPUTSTREAM_HPP_
#define _ELEMENTS_EL_XML_INPUTSTREAM_HPP_

#include <istream>
#include <iostream>
#include <memory>

#include <xercesc/sax/InputSource.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/XMLString.hpp>

//#include <El/Net/HTTP/Session.hpp>

namespace El
{
  namespace XML
  {
    class InputStream: public xercesc::BinInputStream
    {
    public :
      InputStream (std::istream& istr,
                   size_t max_read_portion = 0) throw();
      
      virtual XMLFilePos curPos() const throw();
      
      virtual XMLSize_t readBytes(XMLByte* const toFill,
                                  const XMLSize_t maxToRead) throw();      

      virtual const XMLCh* getContentType() const { return 0; }
      
    protected :
      std::istream& istr_;
      size_t max_read_portion_;
    };

    class InputSource: public xercesc::InputSource
    {
    public :
      InputSource(std::istream& istr,
                  size_t max_read_portion = 0,
                  const char* encoding = 0) throw();
      
      virtual xercesc::BinInputStream* makeStream () const throw();

    protected:
      std::istream& istr_;
      size_t max_read_portion_;
    };
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace XML
  {
    //
    // InputStream class
    //
    inline
    InputStream::InputStream(std::istream& istr,
                             size_t max_read_portion) throw()
        : istr_(istr),
          max_read_portion_(max_read_portion)
    {
    }

    inline
    XMLFilePos
    InputStream::curPos() const throw()
    {
      return istr_.tellg();
    }
    
    inline
    XMLSize_t
    InputStream::readBytes(XMLByte* const toFill,
                           const XMLSize_t maxToRead)
      throw()
    {
      istr_.read(reinterpret_cast<char*>(toFill),
                 max_read_portion_ && maxToRead > max_read_portion_ ?
                 max_read_portion_ : maxToRead);

      return istr_.gcount();
    }
    
    //
    // InputSource class
    //
    inline
    InputSource::InputSource(std::istream& istr,
                             size_t max_read_portion,
                             const char* encoding) throw()
        : istr_(istr),
          max_read_portion_(max_read_portion)
    {
      if(encoding && *encoding)
      {
        try
        {
          std::auto_ptr<XMLCh> enc(xercesc::XMLString::transcode(encoding));
          setEncoding(enc.get());
        }
        catch(...)
        {
        }
      }      
    }
/*
    inline
    const XMLCh*
    InputSource::getEncoding() const throw()
    {
      const XMLCh* enc = xercesc::InputSource::getEncoding();

      std::cerr << "getEncoding: ";

      if(enc)
      { 
        El::ArrayPtr<char> enc2(xercesc::XMLString::transcode(enc));
        std::cerr << enc2.get();
      }
      else
      {
        std::cerr << "none";
      }

      std::cerr << std::endl;

      return enc;
//      return enc && *enc != 0 ? enc : default_encoding_.get();
    }
      
    inline
    void
    InputSource::setEncoding(const XMLCh* const encodingStr)
    {
      El::ArrayPtr<char> enc(xercesc::XMLString::transcode(encodingStr));

      std::cerr << "setEncoding: " << (enc.get() ? enc.get() : "none") << std::endl;
      
      xercesc::InputSource::setEncoding(encodingStr);
    }
*/    
    inline
    xercesc::BinInputStream*
    InputSource::makeStream () const throw()
    {
//      std::cerr << "makeStream\n";
      
      return new InputStream(istr_, max_read_portion_);
    }
  }
}

#endif // _ELEMENTS_EL_XML_INPUTSTREAM_HPP_
 
