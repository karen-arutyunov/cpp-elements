# @file   configure.ac
# @author Karen Arutyunov
# $Id:$

AC_INIT([configure.ac])

m4_include([OpenSBE.ac])

OSBE_ENABLE_FEATURE([CXX/CXX])
OSBE_ENABLE_FEATURE([CXX/So])
OSBE_ENABLE_FEATURE([CXX/Ex])

OSBE_ENABLE_FEATURE([CXX/XML/XSD])
OSBE_ENABLE_FEATURE([CXX/Corba])

python_ver=`python -V 2>&1 | sed -n -e "s/Python \(.*\)\.\(.*\)\..*/\1.\2/p"`

osbe_cxx_use_lib "Python" \
                 "Python" \
                 "include/python$python_ver" \
                 "lib" \
                 "python$python_ver" \
                 "" \
                 "External/Python.pre.rules"

OSBE_CXX_USE_LIB([UUID],
                 [UUID],
                 [include],
                 [lib],
                 [uuid],
                 [],
                 [External/UUID.pre.rules])

OSBE_CXX_USE_LIB([DL],
                 [DL],
                 [include],
                 [lib],
                 [dl],
                 [],
                 [External/DL.pre.rules])

OSBE_CXX_USE_LIB([MD5],
                 [MD5],
                 [include/openssl],
                 [lib],
                 [ssl],
                 [],
                 [External/MD5.pre.rules])

OSBE_CXX_USE_LIB([ZLib],
                 [ZLib],
                 [include],
                 [lib],
                 [z],
                 [],
                 [External/ZLib.pre.rules])

OSBE_CXX_USE_LIB([LibXML],
                 [LibXML],
                 [include/libxml2],
                 [lib],
                 [xml2],
                 [],
                 [External/LibXML.pre.rules])

OSBE_CXX_USE_LIB([IDN],
                 [IDN],
                 [include],
                 [lib],
                 [idn],
                 [],
                 [External/IDN.pre.rules])

OSBE_CXX_USE_LIB([MySQL],
                 [MySQL],
                 [include],
                 [lib/mysql],
                 [mysqlclient_r z],
                 [],
                 [External/MySQL.pre.rules])

OSBE_CXX_USE_LIB([Google],
                 [Google],
                 [include],
                 [],
                 [],
                 [],
                 [External/Google.pre.rules])

OSBE_CXX_USE_LIB(ACE,
                 [ACE],
                 [include],
                 [lib],
                 [ACE],
                 [],
                 [External/ACE.pre.rules])

OSBE_CXX_USE_LIB([Xerces],
                 [Xerces],
                 [include],
                 [lib],
                 [xerces-c],
                 [],
                 [External/Xerces.pre.rules])

OSBE_CXX_USE_LIB([GeoIP],
                 [GeoIP],
                 [include],
                 [lib],
                 [GeoIP],
                 [],
                 [External/GeoIP.pre.rules])

dnl include/httpd - apr directory, can vary on different platforms

OSBE_CXX_USE_LIB([Apache],
                 [Apache],
                 [include include/httpd],
                 [],
                 [],
                 [],
                 [External/Apache.pre.rules])

OSBE_CXX_USE_LIB([TokyoCabinet],
                 [TokyoCabinet],
                 [include],
                 [lib],
                 [tokyocabinet],
                 [],
                 [External/TokyoCabinet.pre.rules])

option_ids="No Yes"

osbe_ask_for_option "Complile AOT Conversion Tool" \
                    "Compile AOT Tool" \
                    "$option_ids" \
                    "" \
                    option_id \
                    "option.aot_conversion_tool" \
                    "No"

aot_conversion_tool=""

if test "$option_id" = "Yes"; then
  OSBE_CXX_USE_LIB([AOTLemmatizer],
                   [AOTLemmatizer],
                   [include],
                   [lib],
                   [],
                   [Lemmatizerrst Graphanrst StructDictrst MorphWizardrst Agramtabrst pcrecpp pcre],
                   [External/AOTLemmatizer.pre.rules])

  aot_conversion_tool="AOT"
fi

AC_SUBST([aot_conversion_tool])

m4_include([dir.ac])

OSBE_OUTPUT
