/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/HTML/LightParser.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_HTML_LIGHTPARSER_HPP_
#define _ELEMENTS_EL_HTML_LIGHTPARSER_HPP_

#include <stdint.h>

#include <string>
#include <vector>

#include <limits.h>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/String/LightString.hpp>
#include <El/String/Unicode.hpp>

#include <El/Net/HTTP/URL.hpp>

namespace El
{
  namespace HTML
  {
    class LightParser
    {
    public:

      struct Image
      {
        El::String::LightString src;
        El::String::LightString alt;
        uint16_t width;
        uint16_t height;
        size_t pos;

        Image() throw(El::Exception);
      };

      typedef std::vector<Image> ImageArray;
    
      struct Link
      {
        El::String::LightString url;
        size_t start_pos;
        size_t end_pos;

        Link() throw(El::Exception);
      };

      typedef std::vector<Link> LinkArray;

      struct Frame
      {
        El::String::LightString url;

        Frame() throw(El::Exception) {}
      };

      typedef std::vector<Frame> FrameArray;
      
      std::string text;
      ImageArray images;
      LinkArray links;
      FrameArray frames;

    public:
      EL_EXCEPTION(Exception, El::ExceptionBase);
      EL_EXCEPTION(MaxLenReached, Exception);

      enum ParsingFlags
      {
        PF_PARSE_IMAGES = 0x1,
        PF_PARSE_LINKS = 0x2,
        PF_PARSE_FRAMES = 0x4,
        PF_LAX = 0x8
      };
    
      LightParser() throw(El::Exception);
    
      void parse(const char* html_text,
                 const char* charset,
                 const char* document_url,
                 unsigned long flags = 0,
                 size_t max_text_len = SIZE_MAX,
                 size_t max_char_len = SIZE_MAX)
        throw(Exception, El::Exception);

      void parse(const wchar_t* html_text,
                 const char* document_url,
                 unsigned long flags = 0,
                 size_t max_text_len = SIZE_MAX,
                 size_t max_char_len = SIZE_MAX)
        throw(Exception, El::Exception);

    private:

      EL_EXCEPTION(CharsetChange, Exception);
    
      enum ParseState
      {
        READING_TEXT,
        READING_TAG,
        READING_ATTRIBUTE
      };

      struct Meta
      {
        std::string http_equiv;
        std::string content;
      };

      struct XML
      {
        std::string encoding;
      };

      void append(wchar_t chr) throw(MaxLenReached, El::Exception);
      void process_tag(const wchar_t* tag) throw(CharsetChange, El::Exception);
    
      void process_attr(const wchar_t* tag,
                        const wchar_t* attr,
                        const wchar_t* attr_value)
        throw(El::Exception);

      void process_img_attr(const wchar_t* attr,
                            const wchar_t* attr_value)
        throw(El::Exception);

      void process_a_attr(const wchar_t* attr,
                          const wchar_t* attr_value)
        throw(El::Exception);

      void process_link_attr(const wchar_t* attr,
                             const wchar_t* attr_value)
        throw(El::Exception);

      void process_base_attr(const wchar_t* attr,
                             const wchar_t* attr_value)
        throw(El::Exception);

      void process_frame_attr(const wchar_t* attr,
                              const wchar_t* attr_value)
        throw(El::Exception);

      void process_meta_attr(const wchar_t* attr,
                             const wchar_t* attr_value)
        throw(El::Exception);

      void process_xml_attr(const wchar_t* attr,
                            const wchar_t* attr_value)
        throw(El::Exception);
      
      size_t rtrim(bool trim_spaces) throw(El::Exception);
    
      std::string url(const wchar_t* value) throw(El::Exception);
      
      void parse(const wchar_t* html_text,
                 const char* document_url,
                 unsigned long flags,
                 size_t max_text_len,
                 size_t max_char_len,
                 const char* charset)
        throw(Exception, CharsetChange, El::Exception);
      
    private:
      size_t length_;
      size_t char_count_;
      unsigned long flags_;
      std::wstring skip_content_until_tag_;
      bool last_space_;
      bool inside_comment_;
      bool inside_pre_tag_;
      size_t max_text_len_;
      size_t max_char_len_;
      std::string charset_;
      Image current_image_;
      Link current_link_;
      Frame current_frame_;
      Meta current_meta_;
      XML current_xml_;
      El::Net::HTTP::URL_var url_base_;
    };
  
  }
}

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace HTML
  {
    //
    // LightParser::Image struct
    //
    inline
    LightParser::Image::Image() throw(El::Exception)
        : width(UINT16_MAX),
          height(UINT16_MAX),
          pos(0)
    {
    }

    //
    // LightParser::Link struct
    //
    inline
    LightParser::Link::Link() throw(El::Exception)
        : start_pos(SIZE_MAX),
          end_pos(SIZE_MAX)
    {
    }
        
    //
    // LightParser class
    //
    inline
    LightParser::LightParser() throw(El::Exception)
        : length_(0),
          char_count_(0),
          flags_(0),
          last_space_(true),
          inside_comment_(false),
          inside_pre_tag_(false),
          max_text_len_(0),
          max_char_len_(0)
    {
      images.reserve(10);
    }

    inline
    void
    LightParser::append(wchar_t chr) throw(MaxLenReached, El::Exception)
    {
      if(!skip_content_until_tag_.empty())
      {
        return;
      }
      
      bool is_space = El::String::Unicode::CharTable::is_space(chr);

      if(is_space && (length_ > max_text_len_ ||
                      char_count_ > max_char_len_))
      {
        throw MaxLenReached("");
      }

      if(is_space)
      {
        if(chr != L'\n')
        {
          chr = L' ';
        }
          
        if(last_space_)
        {
          if(chr == L'\n' && length_)
          {
            // rewrite latest space character with line feed
            text[length_ - 1] = '\n';
          }
            
          return;
        }
      }
      
      last_space_ = is_space;
      length_ += El::String::Manip::wchar_to_utf8(chr, text);
      ++char_count_;
    }    
  
    inline
    size_t
    LightParser::rtrim(bool trim_spaces) throw(El::Exception)
    {
      size_t trimmed = 0;
      
      if(length_)
      {
        unsigned i = length_ - 1;
        
        for(; i > 0 && ((bool)std::isspace(text[i])) == trim_spaces; i--)
        {
          ++trimmed;
        }

        if(i != length_ - 1)
        {
          text.resize(i + 1);
          length_ = i + 1;
        }
      }

      return trimmed;
    }
  
    inline
    void
    LightParser::process_tag(const wchar_t* tag)
      throw(CharsetChange, El::Exception)
    {
      if(skip_content_until_tag_ == tag)
      {
        skip_content_until_tag_.clear();
      }
      
      if(wcscmp(tag, L"p") == 0 || wcscmp(tag, L"p/") == 0 ||
         wcscmp(tag, L"/p") == 0 || wcscmp(tag, L"br") == 0 ||
         wcscmp(tag, L"br/") == 0 || wcscmp(tag, L"/br") == 0 ||
         wcscmp(tag, L"div") == 0 || wcscmp(tag, L"/div") == 0 ||
         wcscmp(tag, L"ol") == 0 || wcscmp(tag, L"/ol") == 0 ||
         wcscmp(tag, L"ul") == 0 || wcscmp(tag, L"/ul") == 0 ||
         wcscmp(tag, L"table") == 0 || wcscmp(tag, L"/table") == 0 ||
         wcscmp(tag, L"/li") == 0)
      {
        append(L'\n');
      }
      else if(wcscmp(tag, L"li") == 0)
      {
        append(L'\n');
        append(L'\x2022'); // add bullet
        append(L' ');
      }
      else if(wcscmp(tag, L"pre") == 0)
      {
        append(L'\n');
        inside_pre_tag_ = true;
      }
      else if(wcscmp(tag, L"/pre") == 0)
      {
        append(L'\n');
        inside_pre_tag_ = false;
      }
      else if(wcscmp(tag, L"script") == 0)
      {
        skip_content_until_tag_ = L"/script";
      }
      else if(wcscmp(tag, L"textarea") == 0)
      {
        skip_content_until_tag_ = L"/textarea";
      }
      else if(wcscmp(tag, L"style") == 0)
      {
        skip_content_until_tag_ = L"/style";
      }
      else if((flags_ & PF_PARSE_IMAGES) && wcscmp(tag, L"img") == 0)
      {
        images.push_back(current_image_);
        current_image_ = Image();
      }
      else if((flags_ & PF_PARSE_LINKS) && (wcscmp(tag, L"a") == 0 ||
               wcscmp(tag, L"link") == 0))
      {
        links.push_back(current_link_);
        current_link_ = Link();
      }
      else if((flags_ & PF_PARSE_FRAMES) && wcscmp(tag, L"frame") == 0)
      {
        frames.push_back(current_frame_);
        current_frame_ = Frame();
      }
      else if(wcscmp(tag, L"meta") == 0)
      {
        if(!charset_.empty() &&
           strcasecmp(current_meta_.http_equiv.c_str(), "content-type") == 0)
        {
          std::string charset;
          
          const char* ptr = strcasestr(current_meta_.content.c_str(),
                                       "charset");

          if(ptr && (ptr = strchr(ptr + 7, '=')) != 0)
          {
            El::String::Manip::trim(ptr + 1, charset);

            if(!charset.empty() && charset_ != charset)
            {
              throw CharsetChange(charset);
            }  
          }

          current_meta_ = Meta();
        }
      }
      else if(wcscmp(tag, L"?xml") == 0)
      {
        if(!charset_.empty() && !current_xml_.encoding.empty() &&
           current_xml_.encoding != charset_)
        {
          throw CharsetChange(current_xml_.encoding);
        }  
        
        current_xml_ = XML();
      }

    }

    inline
    void
    LightParser::process_attr(const wchar_t* tag,
                              const wchar_t* attr,
                              const wchar_t* attr_value)
      throw(El::Exception)
    {
//    std::wcerr << L"=== " << tag << "." << attr << L"=\n" << attr_value
//               << std::endl;

      if((flags_ & PF_PARSE_IMAGES) && wcscmp(tag, L"img") == 0)
      {
        process_img_attr(attr, attr_value);
      }
      else if((flags_ & PF_PARSE_LINKS) && wcscmp(tag, L"a") == 0)
      {
        process_a_attr(attr, attr_value);
      }
      else if((flags_ & PF_PARSE_LINKS) && wcscmp(tag, L"link") == 0)
      {
        process_link_attr(attr, attr_value);
      }
      else if(wcscmp(tag, L"base") == 0)
      {
        process_base_attr(attr, attr_value);
      }
      else if((flags_ & PF_PARSE_FRAMES) && wcscmp(tag, L"frame") == 0)
      {
        process_frame_attr(attr, attr_value);
      }
      else if(wcscmp(tag, L"meta") == 0)
      {
        process_meta_attr(attr, attr_value);
      }
      else if(wcscmp(tag, L"?xml") == 0)
      {
        process_xml_attr(attr, attr_value);
      }
    
    }
  
    inline
    void
    LightParser::process_xml_attr(const wchar_t* attr,
                                  const wchar_t* attr_value)
      throw(El::Exception)
    {
      if(wcscmp(attr, L"encoding") == 0)
      {
        std::string val;
        El::String::Manip::wchar_to_utf8(attr_value, val);
        El::String::Manip::trim(val.c_str(), current_xml_.encoding);
      }
    }

    inline
    void
    LightParser::process_meta_attr(const wchar_t* attr,
                                   const wchar_t* attr_value)
      throw(El::Exception)
    {
      if(wcscmp(attr, L"http-equiv") == 0)
      {
        std::string val;
        El::String::Manip::wchar_to_utf8(attr_value, val);
        El::String::Manip::trim(val.c_str(), current_meta_.http_equiv);
      }
      else if(wcscmp(attr, L"content") == 0)
      {
        El::String::Manip::wchar_to_utf8(attr_value, current_meta_.content);
      }
    }

    inline
    void
    LightParser::process_a_attr(const wchar_t* attr,
                                const wchar_t* attr_value)
      throw(El::Exception)
    {
      if(wcscmp(attr, L"href") == 0)
      {
        current_link_.url = url(attr_value);          
      }
    }

    inline
    void
    LightParser::process_link_attr(const wchar_t* attr,
                                   const wchar_t* attr_value)
      throw(El::Exception)
    {
      if(wcscmp(attr, L"href") == 0)
      {
        current_link_.url = url(attr_value);          
      }
    }

    inline
    void
    LightParser::process_base_attr(const wchar_t* attr,
                                   const wchar_t* attr_value)
      throw(El::Exception)
    {
      if(wcscmp(attr, L"href") == 0)
      {
        try
        {
          url_base_ = new El::Net::HTTP::URL(url(attr_value).c_str());
        }
        catch(...)
        {
        }
      }
    }

    inline
    void
    LightParser::process_frame_attr(const wchar_t* attr,
                                    const wchar_t* attr_value)
      throw(El::Exception)
    {
      if(wcscmp(attr, L"src") == 0)
      {
        current_frame_.url = url(attr_value);          
      }
    }
    
    inline
    void
    LightParser::process_img_attr(const wchar_t* attr,
                                  const wchar_t* attr_value)
      throw(El::Exception)
    {
      if(wcscmp(attr, L"alt") == 0)
      {
        std::string sval;
        
        try
        {
          El::String::Manip::xml_decode(attr_value,  sval);
        }
        catch(...)
        {
          El::String::Manip::wchar_to_utf8(attr_value, sval);
        }

        std::string trimmed;
        El::String::Manip::trim(sval.c_str(), trimmed);
        current_image_.alt = trimmed;
      }
      else if(wcscmp(attr, L"src") == 0)
      {
        current_image_.src = url(attr_value);          
      }
      else if(wcscmp(attr, L"width") == 0)
      {
        unsigned short val = 0;
        El::String::Manip::numeric(attr_value, val);
        current_image_.width = val;
      }
      else if(wcscmp(attr, L"height") == 0)
      {
        unsigned short val = 0;
        El::String::Manip::numeric(attr_value, val);
        current_image_.height = val;
      }
    }

    inline
    std::string
    LightParser::url(const wchar_t* value) throw(El::Exception)
    {
      std::string sval;

      try
      {
        El::String::Manip::xml_decode(value,  sval);
      }
      catch(...)
      {
        El::String::Manip::wchar_to_utf8(value, sval);
      }

      std::string trimmed;
      El::String::Manip::trim(sval.c_str(), trimmed);
      
      if(url_base_.in() != 0)
      {
        try
        {
          return url_base_->abs_url(trimmed.c_str());
        }
        catch(...)
        {
        }
      }
      
      return trimmed;
    }

    inline
    void
    LightParser::parse(const wchar_t* html_text,
                       const char* document_url,
                       unsigned long flags,
                       size_t max_text_len,
                       size_t max_char_len)
      throw(Exception, El::Exception)
    {
      parse(html_text, document_url, flags, max_text_len, max_char_len, 0);
    }
  }
}

#endif // _ELEMENTS_EL_HTML_LIGHTPARSER_HPP_
