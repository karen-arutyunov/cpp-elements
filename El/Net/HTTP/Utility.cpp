/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Net/HTTP/Utility.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <string>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/Net/HTTP/URL.hpp>
#include <El/Net/HTTP/Params.hpp>

#include "Utility.hpp"

//
// IMPORTANT: all names should be maximum 20 chars len
//
namespace El
{
  namespace Net
  {
    namespace HTTP
    {
      const char*
      browser(const char* user_agent) throw(El::Exception)
      {
        if(user_agent != 0)
        {
          std::string ua;
          El::String::Manip::to_lower(user_agent, ua);
        
          if(ua.find("msie") != std::string::npos)
          {
            return "msie";
          }
          else if(ua.find("opera") != std::string::npos)
          {
            return "opera";
          }
          else if(ua.find("firefox") != std::string::npos)
          {
            return "firefox";
          }
          else if(ua.find("yabrowser") != std::string::npos)
          {
            return "yabrowser";
          }
          else if(ua.find("chrome") != std::string::npos)
          {
            return "chrome";
          }
          else if(ua.find("safari") != std::string::npos)
          {
            return "safari";
          }
          else if(ua.find("ucweb") != std::string::npos)
          {
            return "ucweb";
          }
          else if(ua.find("gecko") != std::string::npos)
          {
            return "mozilla";
          }
          else if(ua.find("curl") != std::string::npos)
          {
            return "curl";
          }
          else if(ua.find("mozilla/4.0 (compatible;)") != std::string::npos)
          {
            return "msie";
          }
        }

        return "";
      }
    
      const char*
      feed_reader(const char* user_agent) throw(El::Exception)
      {
        if(user_agent != 0)
        {
          std::string ua;
          El::String::Manip::to_lower(user_agent, ua);
        
          if(ua.find("newsbrain") != std::string::npos)
          {
            return "newsbrain";
          }
          else if(ua.find("apple-pubsub") != std::string::npos)
          {
            return "pubsub";
          }
          else if(ua.find("feeddemon") != std::string::npos)
          {
            return "feeddemon";
          }
          else if(ua.find("rssowl") != std::string::npos)
          {
            return "rssowl";
          }
          else if(ua.find("feedparser") != std::string::npos)
          {
            return "feedparser";
          }
          else if(ua.find("tt-rss") != std::string::npos)
          {
            return "tinytinyrss";
          }
          else if(ua.find("feedbooster") != std::string::npos)
          {
            return "feedbooster";
          }
          else if(ua.find("feedspot") != std::string::npos)
          {
            return "feedspot";
          }
          else if(ua.find("rss popper") != std::string::npos)
          {
            return "rsspopper";
          }          
          else if(ua.find("snarfer") != std::string::npos)
          {
            return "snarfer";
          }          
          else if(ua.find("windows-rss-platform") != std::string::npos)
          {
            return "winrss";
          }          
        }

        return "";
      }
    
      const char*
      crawler(const char* user_agent) throw(El::Exception)
      {
        if(user_agent != 0)
        {
          std::string ua;
          El::String::Manip::to_lower(user_agent, ua);
        
          if(ua.find("googlebot") != std::string::npos)
          {
            return "googlebot";
          }
          else if(ua.find("yandex") != std::string::npos)
          {
            return "yandex";
          }
          else if(ua.find("mail.ru") != std::string::npos)
          {
            return "mail.ru";
          }
          else if(ua.find("bing") != std::string::npos ||
                  ua.find("msnbot") != std::string::npos)
          {
            return "msnbot";
          }
          if(ua.find("google web preview") != std::string::npos)
          {
            return "googlewebpreview";
          }
          else if(ua.find("yahoo") != std::string::npos)
          {
            return "slurp";
          }
          else if(ua.find("mediapartners-google") != std::string::npos)
          {
            return "adsensebot";
          }
          else if(ua.find("yadirectbot") != std::string::npos)
          {
            return "yadirect";
          }
          else if(ua.empty())
          {
            return "unknown-app";
          }
          else if(ua.find("360spider") != std::string::npos)
          {
            return "360spider";
          }
          else if(ua.find("ia_archiver") != std::string::npos)
          {
            return "alexa";
          }
          else if(ua.find("feedfetcher-google") != std::string::npos)
          {
            return "googlefeedfetcher";
          }
          else if(ua.find("feedfetcher") != std::string::npos)
          {
            return "feedfetcher";
          }
          else if(ua.find("google wireless transcoder") != std::string::npos)
          {
            return "googlewltranscoder";
          }
          else if(ua.find("adsbot-google") != std::string::npos)
          {
            return "adsbot-google";
          }          
          else if(ua.find("jeeves") != std::string::npos)
          {
            return "jeeves";
          }
          else if(ua.find("ask jeeves") != std::string::npos)
          {
            return "teoma";
          }
          else if(ua.find("scooter") != std::string::npos)
          {
            return "altavista";
          }
          else if(ua.find("lycos") != std::string::npos)
          {
            return "lycos";
          }
          else if(ua.find("rambler") != std::string::npos)
          {
            return "rambler";
          }
          else if(ua.find("aport") != std::string::npos)
          {
            return "aport";
          }
          else if(ua.find("webalta") != std::string::npos)
          {
            return "webalta";
          }
          else if(ua.find("bazquxbot") != std::string::npos)
          {
            return "bazquxbot";
          }
          else if(ua.find("docomo") != std::string::npos)
          {
            return "docomo";
          }
          else if(ua.find("socialradarbot") != std::string::npos)
          {
            return "socialradar";
          }
          else if(ua.find("integromedb.org/crawler") != std::string::npos)
          {
            return "integromedb";
          }
          else if(ua.find("codegator") != std::string::npos)
          {
            return "codegator";
          }
          else if(ua.find("icsbot") != std::string::npos)
          {
            return "icsbot";
          }
          else if(ua.find("affectv robot") != std::string::npos)
          {
            return "affectv";
          }
          else if(ua.find("theoldreader") != std::string::npos)
          {
            return "theoldreader";
          }
          else if(ua.find("unwindfetchor") != std::string::npos)
          {
            return "unwindfetchor";
          }
          else if(ua.find("thumbshots.ru") != std::string::npos)
          {
            return "thumbshots";
          }
          else if(ua.find("diffbot.com") != std::string::npos)
          {
            return "diffbot";
          }
          else if(ua.find("vkshare") != std::string::npos)
          {
            return "vkshare";
          }
          else if(ua.find("rssreader") != std::string::npos)
          {
            return "rssreader";
          }
          else if(ua.find("pr-cy.ru") != std::string::npos)
          {
            return "pr-cy";
          }
          else if(ua.find("jakarta") != std::string::npos)
          {
            return "jakarta";
          }
          else if(ua.find("odklbot") != std::string::npos)
          {
            return "odklbot";
          }
          else if(ua.find("feeddigest") != std::string::npos)
          {
            return "feeddigest";
          }
          else if(ua.find("baiduspider") != std::string::npos)
          {
            return "baidu";
          }
          else if(ua.find("mlbot") != std::string::npos)
          {
            return "mlbot";
          }
          else if(ua.find("voilabot") != std::string::npos)
          {
            return "voilabot";
          }
          else if(ua.find("wordpress") != std::string::npos)
          {
            return "wordpress";
          }
          else if(ua.find("begun") != std::string::npos)
          {
            return "begun";
          }
          else if(ua.find("sogou") != std::string::npos)
          {
            return "sogou";
          }
          else if(ua.find("panscient") != std::string::npos)
          {
            return "panscient";
          }
          else if(ua.find("mj12bot") != std::string::npos)
          {
            return "mj12bot";
          }
          else if(ua.find("comodo-certificates") != std::string::npos)
          {
            return "comodo";
          }
          else if(ua.find("builtwith") != std::string::npos)
          {
            return "builtwith";
          }
          else if(ua.find("ahrefsbot") != std::string::npos)
          {
            return "ahrefsbot";
          }
          else if(ua.find("yodaobot") != std::string::npos)
          {
            return "yodaobot";
          }
          else if(ua.find("ezooms") != std::string::npos)
          {
            return "ezooms";
          }
          else if(ua.find("huaweisymantecspider") != std::string::npos)
          {
            return "huaweisymspider";
          }
          else if(ua.find("edisterbot") != std::string::npos)
          {
            return "edisterbot";
          }
          else if(ua.find("dle_spider") != std::string::npos)
          {
            return "dle_spider";
          }
          else if(ua.find("netcraftsurveyagent") != std::string::npos)
          {
            return "netcraftsurvagent";
          }
          else if(ua.find("discobot") != std::string::npos)
          {
            return "discobot";
          }
          else if(ua.find("archive.org_bot") != std::string::npos)
          {
            return "archive";
          }
          else if(ua.find("aboundex") != std::string::npos)
          {
            return "aboundex";
          }
          else if(ua.find("wbsearchbot") != std::string::npos)
          {
            return "wbsearch";
          }
          else if(ua.find("turnitinbot") != std::string::npos)
          {
            return "turnitinbot";
          }
          else if(ua.find("httrack") != std::string::npos)
          {
            return "httrack";
          }
          else if(ua.find("genieo") != std::string::npos)
          {
            return "genieo";
          }
          else if(ua.find("indy library") != std::string::npos)
          {
            return "indy-app";
          }
          else if(ua.find("superfeedr") != std::string::npos)
          {
            return "superfeedr";
          }
          else if(ua.find("rssgraffiti") != std::string::npos)
          {
            return "rssgraffiti";
          }
          else if(ua.find("ruby") != std::string::npos)
          {
            return "ruby-app";
          }
          else if(ua.find("google-http-java-client") != std::string::npos)
          {
            return "google-java-app";
          }
          else if(ua.find("solomonobot") != std::string::npos)
          {
            return "solomonobot";
          }
          else if(ua.find("addthis") != std::string::npos)
          {
            return "addthis";
          }
          else if(ua.find("liveinternet") != std::string::npos)
          {
            return "liveinternet";
          }
          else if(ua.find("rogerbot") != std::string::npos)
          {
            return "rogerbot";
          }
          else if(ua.find("facebookexternalhit") != std::string::npos)
          {
            return "facebookext";
          }          
          else if(ua.find("nigma") != std::string::npos)
          {
            return "nigma";
          }          
          else if(ua.find("proximic") != std::string::npos)
          {
            return "proximic";
          }          
          else if(ua.find("newsgatoronline") != std::string::npos)
          {
            return "newsgator";
          }
          else if(ua.find("python") != std::string::npos)
          {
            return "python-app";
          }
          else if(ua.find("acoonbot") != std::string::npos)
          {
            return "acoonbot";
          }
          else if(ua.find("sistrix") != std::string::npos)
          {
            return "sistrix";
          }
          else if(ua.find("zend_http_client") != std::string::npos)
          {
            return "zend-app";
          }
          else if(ua.find("panopta.com") != std::string::npos)
          {
            return "panopta";
          }
          else if(ua.find("iteco dummy crawler") != std::string::npos)
          {
            return "iteco";
          }
          else if(ua.find("searchbot") != std::string::npos)
          {
            return "searchbot";
          }          
          else if(ua.find("simplepie") != std::string::npos)
          {
            return "simplepie";
          }          
          else if(ua.find("flipboard") != std::string::npos)
          {
            return "flipboardproxy";
          }          
          else if(ua.find("curl/") != std::string::npos)
          {
            return "curl-app";
          }          
          else if(ua.find("sputnikbot") != std::string::npos)
          {
            return "sputnikbot";
          }
          else if(ua.find("bazqux") != std::string::npos)
          {
            return "bazqux";
          }
          else if(ua.find("bitrixsmrss") != std::string::npos)
          {
            return "bitrixsmrss";
          }
          else if(ua.find("lwp-trivial") != std::string::npos)
          {
            return "perl-app";
          }
          else if(ua.find("admantx") != std::string::npos)
          {
            return "admantx";
          }
          else if(ua.find("g2reader-bot") != std::string::npos)
          {
            return "g2reader-bot";
          }
          else if(ua.find("squider") != std::string::npos)
          {
            return "squider";
          }
          else if(ua.find("developers.google.com") != std::string::npos)
          {
            return "googlewidget";
          }
          else if(ua.find("taptu-downloader") != std::string::npos)
          {
            return "taptu";
          }
          else if(ua.find("site-shot") != std::string::npos)
          {
            return "siteshot";
          }
          else if(ua.find("firephp") != std::string::npos)
          {
            return "firephp";
          } 
          else if(ua.find("omgilibot") != std::string::npos)
          {
            return "omgilibot";
          }
          else if(ua.find("sensikabot") != std::string::npos)
          {
            return "sensikabot";
          }          
          else if(ua.find("dotbot") != std::string::npos)
          {
            return "dotbot";
          }
          else if(ua.find("exabot") != std::string::npos)
          {
            return "exabot";
          }
          else if(ua.find("grapeshotcrawler") != std::string::npos)
          {
            return "grapeshot";
          }
          else if(ua.find("xspider") != std::string::npos)
          {
            return "xspider";
          }
          else if(ua.find("seokicks") != std::string::npos)
          {
            return "seokicks";
          }
          else if(ua.find("webindex") != std::string::npos)
          {
            return "webindex";
          }
          else if(ua.find("compatible; ics)") != std::string::npos)
          {
            return "ics";
          }
          else if(ua.find("feedburner/") != std::string::npos)
          {
            return "feedburner";
          }
          else if(ua.find("infegyatlas") != std::string::npos)
          {
            return "infegy";
          }
          else if(ua.find("netvibes") != std::string::npos)
          {
            return "netvibes";
          }
          else if(ua.find("ltx71") != std::string::npos)
          {
            return "ltx71";
          }
          else if(ua.find("protopage") != std::string::npos)
          {
            return "protopage";
          }
          else if(ua.find("feedlybot") != std::string::npos)
          {
            return "feedlybot";
          }
          else if(ua.find("drupal") != std::string::npos)
          {
            return "drupal";
          }
          else if(ua.find("cliqzbot") != std::string::npos)
          {
            return "cliqzbot";
          }
          else if(ua.find("subscribe.ru") != std::string::npos)
          {
            return "subscribe";
          }
          else if(ua.find("xovibot") != std::string::npos)
          {
            return "xovibot";
          }
          else if(ua.find("linkdexbot") != std::string::npos)
          {
            return "linkdexbot";
          }
          else if(ua.find("twitterfeed") != std::string::npos)
          {
            return "twitterfeed";
          }
          else if(ua.find("java/") != std::string::npos)
          {
            return "java-app";
          }
          else if(ua.find("megaindex") != std::string::npos)
          {
            return "megaindex";
          }
          else if(ua.find("semrush") != std::string::npos)
          {
            return "semrush";
          }
        }

        return "";
      }

      const char*
      computer(const char* user_agent) throw(El::Exception)
      {
        if(user_agent != 0)
        {
          std::string ua;
          El::String::Manip::to_lower(user_agent, ua);

          if(((ua.find("mozilla/5.0 (windows") != std::string::npos ||
               ua.find("mozilla/4.0 (windows") != std::string::npos) &&
              ua.find("windows phone") == std::string::npos) ||
             
             ua.find("mozilla/5.0 (compatible, msie 11, windows nt 6.") !=
             std::string::npos ||

             ua.find("mozilla/5.0 (compatible; msie 10.6; windows nt 6.") !=
             std::string::npos ||

             ua.find("mozilla/5.0 (compatible; msie 10.0; windows nt 6.") !=
             std::string::npos ||
             
             ua.find("mozilla/5.0 (compatible; msie 9.0; windows nt 6.") !=
             std::string::npos ||
             
             ua.find("mozilla/4.0 (compatible; msie 8.0; windows nt 6.") !=
             std::string::npos ||
             
             ua.find("mozilla/4.0 (compatible; msie 8.0; windows nt 5.") !=
             std::string::npos ||
             
             ua.find("mozilla/4.0 (compatible; msie 7.0; windows nt 6.") !=
             std::string::npos ||
             
             ua.find("mozilla/4.0 (compatible; msie 7.0; windows nt 5.") !=
             std::string::npos ||
             
             ua.find("mozilla/4.0 (compatible; msie 6.0; windows nt 5.") !=
             std::string::npos ||

             (ua.find("mozilla/5.0 (macintosh") != std::string::npos &&
              ua.find("htc") == std::string::npos) ||
             
             ua.find("opera/9.80 (windows nt ") != std::string::npos ||
             ua.find("opera/9.80 (macintosh;") != std::string::npos ||
             ua.find("x11") != std::string::npos ||
             ua.find("curl/") != std::string::npos)
          {
            return "generic";
          }
        }
        
        return "";
      }
      
      const char*
      tab(const char* user_agent) throw(El::Exception)
      {
        if(user_agent != 0 && *computer(user_agent) == '\0')
        {
          std::string ua;
          El::String::Manip::to_lower(user_agent, ua);

          if(ua.find("ipad") != std::string::npos)
          {
            return "apple";
          }
          else if(ua.find("htc") != std::string::npos &&
                  (ua.find("flyer") != std::string::npos ||
                   ua.find("pg") != std::string::npos))
          {
            return "htc";
          }
          else if(ua.find("sony tablet") != std::string::npos)
          {
            return "sony";
          }
          else if(ua.find("huawei mediapad") != std::string::npos)
          {
            return "huawei";
          }
          else if(ua.find("gt-p") != std::string::npos ||
                  ua.find("gt-n8") != std::string::npos)
          {
            return "samsung";
          }
          else if(ua.find("nexus 7") != std::string::npos ||
                  ua.find("nexus 10") != std::string::npos)
          {
            return "google";
          }
          else if(ua.find("transformer") != std::string::npos)
          {
            return "asus";
          }
          else if(ua.find("playbook") != std::string::npos ||
                  ua.find("rim tablet") != std::string::npos)
          {
            return "blackberry";
          }
          else if(ua.find("a100") != std::string::npos ||
                  ua.find("a101") != std::string::npos ||
                  ua.find("a200") != std::string::npos ||
                  ua.find("a500") != std::string::npos ||
                  ua.find("a501") != std::string::npos ||
                  ua.find("a510") != std::string::npos ||
                  ua.find("a511") != std::string::npos ||
                  ua.find("a701") != std::string::npos)
          {
            return "acer";
          }            
          else if(ua.find("opera tablet") != std::string::npos)
          {
            return "opera";
          }
          else if(ua.find("android") != std::string::npos)
          {
            if(ua.find("tablet") != std::string::npos)
            {
              return "fennec";
            }
          }
        }
        
        return "";
      }
      
      const char*
      phone(const char* user_agent) throw(El::Exception)
      {
        if(user_agent != 0 && *computer(user_agent) == '\0' &&
           *tab(user_agent) == '\0')
        {
          std::string ua;
          El::String::Manip::to_lower(user_agent, ua);

          if(ua.find("(iphone;") != std::string::npos ||
             ua.find("(ipod;") != std::string::npos)
          {
            return "apple";
          }
          else if(ua.find("sonyericsson") != std::string::npos)
          {
            return "sonyericsson";
          }
          else if(ua.find("sony") != std::string::npos)
          {
            return "sony";
          }
          else if(ua.find("huawei") != std::string::npos)
          {
            return "huawei";
          }
          else if(ua.find("blackberry") != std::string::npos)
          {
            return "blackberry";
          }
          else if(ua.find("lg-") != std::string::npos)
          {
            return "lg";
          }
          else if(ua.find("htc") != std::string::npos)
          {
            return "htc";
          }
          else if(ua.find("gt-") != std::string::npos ||
                  ua.find("samsung") != std::string::npos ||
                  ua.find("galaxy") != std::string::npos)
          {
            return "samsung";
          }          
          else if(ua.find("nexus") != std::string::npos)
          {
            return "google";
          }
          else if(ua.find("opera mobi") != std::string::npos ||
                  ua.find("opera mini") != std::string::npos)
          {
            return "opera";
          }
          else if(ua.find("nokia") != std::string::npos ||
                  ua.find("symbian") != std::string::npos)
          {
            return "nokia";
          }
          else if(ua.find("android") != std::string::npos)
          {
            return "android";
          }
          else if(ua.find("windows phone") != std::string::npos)
          {
            return "microsoft";
          }
        }

        return "";
      }
      
      const char*
      os(const char* user_agent) throw(El::Exception)
      {
        if(user_agent != 0)
        {
          std::string ua;
          El::String::Manip::to_lower(user_agent, ua);
        
          if(ua.find("windows") != std::string::npos)
          {
            return "windows";
          }
          else if(ua.find("macintosh") != std::string::npos)
          {
            return "macintosh";
          }
          else if(ua.find("linux") != std::string::npos)
          {
            return "linux";
          }
        }

        return "";
      }
    
      SearchInfo
      search_info(const char* url) throw(El::Exception)
      {
        SearchInfo res;
        
        try
        {
          URL_var u = new URL(url);

          const char* host = u->host();

          std::string domain;
          
          if(!company_domain(host, &domain))
          {
            return res;
          }
          
          std::size_t pos = domain.find('.');
          
          if(pos == std::string::npos)
          {
            return res;
          }
          
          std::string name(domain.c_str(), pos);

          if(name == "google")
          {
            res.engine = "google";

            ParamList params(u->params(), true);
            
            const char* p = params.find("q");

            if(p && El::String::Manip::utf8_valid(p))
            {
              res.query = p;
            }

            p = params.find("imgurl");

            if(p && El::String::Manip::utf8_valid(p))
            {
              res.image = p;
            }

            return res;
          }

          if(name == "yandex")
          {
            res.engine = "yandex";

            ParamList params(u->params(), true);
            
            const char* p = params.find("text");
            
            if(p == 0)
            {
              p = params.find("query");
            }

            if(p)
            {
              if(El::String::Manip::utf8_valid(p))
              {
                res.query = p;
              }
              else
              {
                try
                {
                  std::wstring dest;
                  El::String::Manip::win1251_to_wchar(p, dest);
                  El::String::Manip::wchar_to_utf8(dest.c_str(), res.query);
                }
                catch(const El::Exception&)
                {
                  // Didn't divine encoding
                }
              }
            }

            p = params.find("img_url");

            if(p && El::String::Manip::utf8_valid(p))
            {
              res.image = p;
            }

            return res;
          }

          if(name == "yahoo")
          {
            res.engine = "yahoo";

            ParamList params(u->params(), true);
            const char* q = params.find("p");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(name == "rambler")
          {
            res.engine = "rambler";

            ParamList params(u->params(), true);
            const char* q = params.find("query");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(name == "bing")
          {
            res.engine = "bing";

            ParamList params(u->params(), true);
            const char* q = params.find("q");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(strcmp(host, "go.mail.ru") == 0)
          {
            res.engine = "mail.ru";

            ParamList params(u->params(), true);
            const char* q = params.find("q");

            if(q)
            {
              const char* utf8 = params.find("utf8in");

              if(utf8 && strcmp(utf8, "1") == 0)
              {
                if(El::String::Manip::utf8_valid(q))
                {
                  res.query = q;
                }
              }
              else
              {
                if(El::String::Manip::utf8_valid(q))
                {
                  res.query = q;
                }
                else
                {
                  std::wstring wq;
                  El::String::Manip::win1251_to_wchar(q, wq);
                  El::String::Manip::wchar_to_utf8(wq.c_str(), res.query);
                }
              }
            }

            return res;
          }
          
          if(domain == "conduit.com")
          {
            res.engine = "conduit";

            ParamList params(u->params(), true);
            const char* q = params.find("q");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(domain == "nigma.ru")
          {
            res.engine = "nigma";

            ParamList params(u->params(), true);
            const char* q = params.find("s");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(domain == "ask.com")
          {
            res.engine = "ask";

            ParamList params(u->params(), true);
            const char* q = params.find("q");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(strcmp(host, "search.icq.com") == 0)
          {
            res.engine = "icq";

            ParamList params(u->params(), true);
            const char* q = params.find("q");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }
          
          if(strcmp(u->host(), "search.qip.ru") == 0)
          {
            res.engine = "qip";

            ParamList params(u->params(), true);
            const char* q = params.find("query");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(domain == "webalta.ru")
          {
            res.engine = "webalta";

            ParamList params(u->params(), true);
            const char* q = params.find("q");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(domain == "youtube.com")
          {
            res.engine = "youtube";

            ParamList params(u->params(), true);
            const char* q = params.find("search_query");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }

          if(domain == "avg.com")
          {
            res.engine = "avg";

            ParamList params(u->params(), true);
            const char* p = params.find("q");

            if(p && El::String::Manip::utf8_valid(p))
            {
              res.query = p;
            }

            return res;
          }

          if(strcmp(host, "search.tut.by") == 0)
          {
            res.engine = "tut";

            ParamList params(u->params(), true);
            const char* q = params.find("query");

            if(q && El::String::Manip::utf8_valid(q))
            {
              res.query = q;
            }

            return res;
          }          
        }
        catch(const URL::InvalidArg&)
        {
        }
        catch(const El::String::Manip::InvalidArg&)
        {
        }

        return res;
      }
    }
  }
}
