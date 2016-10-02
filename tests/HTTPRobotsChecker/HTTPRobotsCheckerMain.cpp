/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file   HTTPRobotsCheckerMain.cpp
 * @author Karen Arutyunov
 * $Id:$
 */

#include <El/Exception.hpp>
#include <El/Net/HTTP/Robots.hpp>

namespace
{
  const char USAGE[] = "Usage: ElTestHTTPRobotsChecker <user-agent> <url>+";

  const char USER_AGENT[] =
    "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322)";
}

class Application
{
public:    
  EL_EXCEPTION(Exception, El::ExceptionBase);
    
public:
  Application() throw() {}
  virtual ~Application() throw() {}
  
  int run(int& argc, char** argv) throw();
};

int
main(int argc, char** argv)
{
  Application app;
  return app.run(argc, argv);
}

int
Application::run(int& argc, char** argv) throw()
{
  try
  {
    // Checking params
    if (argc < 3)
    {
      std::ostringstream ostr;
      ostr << "Too few arguments\n" << USAGE;
      throw Exception(ostr.str());
    }

    std::string user_agent = argv[1];

    El::Net::HTTP::RobotsChecker robots_checker(ACE_Time_Value(30),
                                                10,
                                                ACE_Time_Value(60 * 60),
                                                ACE_Time_Value(60 * 10));

    std::cerr << "User-agent: " << user_agent << std::endl;
    
    for(int i = 2; i < argc; i++)
    {
      const char* url = argv[i];
      bool allowed = robots_checker.allowed(url, user_agent.c_str());

      std::cout << "  " << (allowed ? "Allowed" : "Disallowed") << " for "
                << url << std::endl;
    }

    std::cerr << "RobotsChecker state:\n";
    robots_checker.dump(std::cerr);

    return 0;
  }
  catch(const El::Exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
  catch(...)
  {
    std::cerr << "unknown exception caught.\n";
  }

  return -1;
}
 
