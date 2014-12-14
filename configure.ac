#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# don't require higher autoconf version for win32 support
AC_PREREQ([2.68])
AC_INIT([smart-snmp], [0.3.0], [trash@smart-snmp.org])
SMART_SNMP_MAJOR_VERSION=0
SMART_SNMP_MINOR_VERSION=3
SMART_SNMP_MICRO_VERSION=0
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/agent.cpp])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE([disable])

dnl
dnl Get cannonical host
dnl
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([OS_TYPE], "$host_os",
                   [Define to be the name of the operating system.])

dnl disable unnecessary compiler checks
AC_DEFUN([AC_PROG_F77], [:])
AC_DEFUN([AC_PROG_FC], [:])
AC_DEFUN([AC_PROG_OBJC], [:])
AC_DEFUN([AC_PROG_OBJCCPP], [:])
AC_DEFUN([AC_LIBTOOL_F77], [:])

dnl disable check for GNU compatible malloc - just a working malloc is enough
AC_DEFUN([AC_FUNC_MALLOC_IF], [:])
AC_DEFUN([AC_FUNC_MALLOC], [
  AC_CHECK_FUNCS([malloc realloc])
])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_SED
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_AR

# Check for extra flags to enable some kind of behavior

AC_CHECK_DECL([_AIX],[ac_cv_need_minus_d_linux_source_compat="yes"])
AC_CHECK_DECL([__linux],[ac_cv_need_minus_d_gnu_source="yes"]) dnl probably required by kfreebsd, too?
AC_CHECK_DECL([__sun],[ac_cv_need_minus_d_extensions="yes"])
AC_CHECK_DECL([__hpux],[ac_cv_need_minus_d_hpux_source="yes"])

AS_IF([test "x$ac_cv_need_minus_d_linux_source_compat" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_LINUX_SOURCE_COMPAT"])

AS_IF([test "x$ac_cv_need_minus_d_gnu_source" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_GNU_SOURCE"])

AS_IF([test "x$ac_cv_need_minus_d_all_source" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_ALL_SOURCE"])

AS_IF([test "x$ac_cv_need_minus_d_extensions" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D__EXTENSIONS__"])

AS_IF([test "x$ac_cv_need_minus_d_hpux_source" = "xyes"],
      [CPPFLAGS="$CPPFLAGS -D_HPUX_SOURCE"])

dnl
dnl Process features.
dnl
AX_CHECK_WITH_LIB([snmp++],[yes])
AX_CHECK_WITH_LIB([agent++],[yes])
AX_CHECK_WITH_LIB([confuse],[yes])
AX_CHECK_WITH_LIB([json],[check])
AC_ARG_WITH([bundled-libjson],
[  --with-bundled-libjson  uses bundled libjson instead of external library],
[BUNDLED_LIBJSON="${withval}"], [BUNDLED_LIBJSON="fallback"])
AX_CHECK_WITH_LIB([statgrab],[yes])
AX_CHECK_WITH_LIB([log4cplus],[check])

AC_ARG_WITH([user-lookup],
[ --with-user-lookup[[=param]] user and group lookup yes/no/file/pamdbs],
[USER_LOOKUP="${withval}"], [USER_LOOKUP="check"])

AC_ARG_ENABLE(threads,
[  --disable-threads       disable thread support],
ENABLE_THREADS=$enableval, ENABLE_THREADS=yes)
AC_ARG_ENABLE([debug],
[  --disable-debug         disable support for debugging output],
[ENABLE_DEBUG=$enableval], [ENABLE_DEBUG=yes])
AS_IF([test "x${ENABLE_DEBUG}" = "xyes"],
      [AC_DEFINE([DEBUG], 1, [Define if you want the debug output support compiled in.])],
      [AC_LIB_APPENDTOVAR([CXXFLAGS], [-D_NDEBUG])]
)
AC_ARG_ENABLE(docbuild,
[  --enable-docbuild       enable build of documentation],
ENABLE_DOCBUILD=$enableval, ENABLE_DOCBUILD=check)

SU_CMD=check
AC_ARG_WITH([su-cmd], [
  --with-su-cmd[[=ARG]]     use su-cmd (default: search for sudo and su)
  --without-su-cmd        do not use an su-cmd to switch user for external commands],
[SU_CMD=$withval],[SU_CMD=check])

SU_ARGS=check
AC_ARG_WITH([su-args],
[  --with-su-args[[=ARG]]    use su-args (default: choose reasonable flags depending
                          on su-cmd)],
[SU_ARGS=$withval],[SU_ARGS=check])

AM_CONDITIONAL(USE_V1_COMMUNITY, true)
AC_ARG_WITH([v1-community],
[  --with-v1-community[[=ARG]]
                          use SNMP v1 community ARG (default: public)
  --without-v1-community  do not use SNMP v1 community ARG (default: public)],
[
  AS_CASE(
    [$withval],
    [[[Yy]][[Ee]][[Ss]]], [
      DEFAULT_V1_COMMUNITY="public"
    ],
    [[[Nn]][[Oo]]], [
      AM_CONDITIONAL(USE_V1_COMMUNITY, false)
      DEFAULT_V1_COMMUNITY="public"
    ],
    [*], [
      DEFAULT_V1_COMMUNITY="${withval}"
    ]
  )
], [
  DEFAULT_V1_COMMUNITY="public"
])
AC_SUBST(DEFAULT_V1_COMMUNITY)

AM_CONDITIONAL(USE_V2_COMMUNITY, true)
AC_ARG_WITH([v2-community],
[  --with-v2-community[[=ARG]]
                          use SNMP v2 community ARG (default: public)
  --without-v2-community  do not use SNMP v2c community ARG (default: public)],
[
  AS_CASE(
    [$withval],
    [[[Yy]][[Ee]][[Ss]]], [
      DEFAULT_V2_COMMUNITY="public"
    ],
    [[[Nn]][[Oo]]], [
      AM_CONDITIONAL(USE_V2_COMMUNITY, false)
      DEFAULT_V2_COMMUNITY="public"
    ],
    [*], [
      DEFAULT_V2_COMMUNITY="${withval}"
    ]
  )
], [
  DEFAULT_V2_COMMUNITY="public"
])
AC_SUBST(DEFAULT_V2_COMMUNITY)

dnl run tests using the c++ compiler - there're differences in the result ...
AC_LANG_PUSH([C++])
AX_CHECK_XOPEN_SOURCE
AS_IF([test -n "$XOPEN_SOURCE_CPPFLAGS"], [CPPFLAGS="$CPPFLAGS $XOPEN_SOURCE_CPPFLAGS"])
AC_LANG_POP([C++])
AS_IF([test "$HAVE_XOPEN_SOURCE_LEVEL" -lt "500"], [
  AC_MSG_WARN([smart-snmpd relies on XPG5 features, but only $HAVE_XOPEN_SOURCE is detected.])
  AC_MSG_NOTICE([Please schedule exessive tests when the compile does not fail to ensure smart-snmpd is working properly])
])

AX_CHECK_VISIBILITY(
[
	SM_EXPORT="$acx_visibility_export"
	SM_IMPORT="$acx_visibility_import"
	SM_PRIVATE="$acx_visibility_private"
	WITH_VISIBILITY_SUPPORT=1
	AC_SUBST(SM_EXPORT)
	AC_SUBST(SM_IMPORT)
	AC_SUBST(SM_PRIVATE)
],[
	WITH_VISIBILITY_SUPPORT=0
])
AC_SUBST(WITH_VISIBILITY_SUPPORT)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
AC_HEADER_STDBOOL
AC_CHECK_HEADERS([ctype.h errno.h fcntl.h float.h limits.h poll.h pwd.h signal.h stdarg.h stdio.h string.h sys/fcntl.h sys/file.h sys/param.h sys/select.h sys/socket.h sys/timeb.h sys/unistd.h sys/wait.h])
AC_CHECK_HEADERS([netdb.h netinet/in.h arpa/inet.h], , ,[
#ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#  include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif
])

dnl Checks for inet libraries:
AC_SEARCH_LIBS([gethostent], [nsl])
AC_SEARCH_LIBS([setsockopt], [socket net network])
AC_SEARCH_LIBS([connect], [inet])
AC_SEARCH_LIBS([strerror], [cposix])

AX_WIN32([
    AC_CHECK_HEADERS([io.h process.h winsock.h winsock2.h wstcpip.h wspiapi.h])
    AC_CHECK_LIB([wsock32],[main])
    AC_CHECK_LIB([ws2_32],[main])], [
    dnl AC_CHECK_HEADERS([libgen.h])
    AC_CHECK_HEADERS([sys/ipc.h sys/sem.h], ,[AC_MSG_FAILURE([missing header for SysV semaphores])], [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#endif
#ifdef HAVE_SYS_IPC_H
# include <sys/ipc.h>
#endif
    ])])
    AC_SEARCH_LIBS([sem_init], [rt])
    AC_CHECK_HEADERS([dlfcn.h])
    AS_IF([test "x$ac_cv_header_dlfcn_h" = "xyes"], [
        AC_MSG_CHECKING([for dlsym(RTLD_NEXT, ...)])
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
#include <dlfcn.h>

    int main() {
        static void* (*real_malloc)(size_t) = NULL;
        if (!real_malloc)
            real_malloc = dlsym(RTLD_NEXT, "malloc");
        return 0;
    }
          ])], [
              AC_MSG_RESULT([yes])
              AC_DEFINE(HAVE_DLSYM_RTLD_NEXT, , [define with dlsym(RTLD_NEXT, ...) works])
              AC_SEARCH_LIBS([dlsym], [dl])
          ], [
              AC_MSG_RESULT([no])
          ])
    ], [
        AC_MSG_WARN([dlsym(RTLD_NEXT, ...) doesn't work, can't intercept malloc() ...])
    ])
])

dnl run tests using the c++ compiler - there're differences in the result ...
AC_LANG_PUSH([C++])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

dnl pull the rabbit out of the hat to avoid regexp in AX_CHECK_LIB_FLAGS searching for winsock2.h
acx_include_sockets="\
#if HAVE_WINSOCK2_H
# include <winsock2.h>
# if HAVE_WS2TCPIP_H
#  include <ws2tcpip.h>
# endif
# if HAVE_WSPIAPI_H
#  include <wspiapi.h>
# endif
#elif HAVE_WINSOCK_H
  /* IIRC winsock.h must be included before windows.h */
# include <winsock.h>
#else
# ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
# endif
# ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
# endif
# ifdef HAVE_NETDB_H
#  include <netdb.h>
# endif
# ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
#endif"

AC_CHECK_TYPES([struct sockaddr_in6],,,[
AC_INCLUDES_DEFAULT
$acx_include_sockets
])

# Checks for thread dependencies
AS_IF([test "x${ENABLE_THREADS}" = "xyes"], [
    AX_WIN32(
    [
        AC_DEFINE([ENABLE_THREADS], 1,
            [Define if you want the thread support compiled in.])
        ac_cv_enable_threads=yes
        WITH_THREADS=1
    ],
    [
        AX_PTHREAD([
            AC_DEFINE([ENABLE_THREADS], 1,
                [Define if you want the thread support compiled in.])
            AC_DEFINE(HAVE_PTHREAD, 1,
                [Define if you have POSIX threads libraries and header files.])
            CC="$PTHREAD_CC"
            CXX="$PTHREAD_CXX"
            AS_IF([test -n "$PTHREAD_LIBS"], [LIBS="$PTHREAD_LIBS $LIBS"])
            AS_IF([test -n "$PTHREAD_CFLAGS"], [
              CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
              CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
            ])
            ac_cv_enable_threads=yes
            WITH_THREADS=1
            AC_SEARCH_LIBS([sem_init], [rt])
        ],
        [])
    ])
], [WITH_THREADS=0]) # endif : --enable-threads

dnl FIXME when fixed in source
AS_IF([test "x$ac_cv_enable_threads" != "xyes"],
      [AC_MSG_FAILURE([This (early) version of smart-snmpd requires threads being available])])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_ALLOCA
AC_CHECK_FUNCS([access fcntl flock getaddrinfo gethostbyaddr gethostbyaddr_r gethostbyname gethostbyname2 gethostbyname_r gethostname gettimeofday inet_aton inet_ntoa inet_pton inet_ntop isdigit localtime_r memset mkdir poll rmdir select socket strcasecmp stricmp strchr strerror strsignal strstr tzset])

# check this separately if it produces different results on Win2k or WinXP
AC_CHECK_DECLS([getaddrinfo],,,[
#if HAVE_WINSOCK2_H
#  include <winsock2.h>
#  if HAVE_WS2TCPIP_H
#    // Fake windows XP for getaddrinfo if IPv6
#    if HAVE_STRUCT_SOCKADDR_IN6
#      define _WIN32_WINNT 0x0501
#    endif
#    include <ws2tcpip.h>
#  endif
#  if HAVE_WSPIAPI_H
#    include <wspiapi.h>
#  endif
#else
#  include <netdb.h>
#endif
])

AC_CHECK_DECL([F_SETLK], [AC_DEFINE([HAVE_DECL_F_SETLK], 1, [define when F_SETLK is declared])], ,[
AC_INCLUDES_DEFAULT
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#endif
#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif
])

AX_CXX_NAMESPACES
AX_CXX_NAMESPACE_STD
AX_CXX_HAVE_STL
AX_CXX_HAVE_STD
AX_CXX_TEMPLATES
AX_CXX_HEADER_STDCXX_98
AX_CXX_HEADER_STDCXX_TR1
AX_CXX_HEADER_STDCXX_0X

AS_IF([test "x$ax_cv_cxx_stdcxx_98" != "xyes" -o "x$ax_cv_cxx_have_std" != "xyes"], [
  AC_MSG_ERROR([ISO C++ 98 (including namespace std) required])])
AM_CONDITIONAL(WITH_STL_CONTAINER,true)

AC_MSG_CHECKING([for sqrt(unsigned long long int)])
AC_LINK_IFELSE(
  [AC_LANG_SOURCE([
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char **argv) {
    unsigned long long int ull; 
    string s = argv[[1]];
    istringstream iss( s );
    iss >> ull;
    unsigned long long int sqrt_ull = sqrt( ull );
    cout << sqrt_ull << endl;
    return 0;
}
  ])],
  [
    AC_MSG_RESULT(yes)
    AC_DEFINE([HAVE_SQRT_LONG_LONG], 1, [define when prototype for sqrt can deal with long long variables])
  ],
  [
    AC_MSG_RESULT(no)
  ]
)

# Do not disable mandatory libraries
AS_IF([test "x${ax_cv_with_libsnmp}" != "xyes"], [AC_MSG_ERROR([libsnmp++ is mandatory and must not be disabled])])
AS_IF([test "x${ax_cv_with_libagent}" != "xyes"], [AC_MSG_ERROR([libagent++ is mandatory and must not be disabled])])
AS_IF([test "x${ax_cv_with_libconfuse}" != "xyes"], [AC_MSG_ERROR([libconfuse is mandatory and must not be disabled])])
AS_IF([test "x${ax_cv_with_libstatgrab}" != "xyes"], [AC_MSG_ERROR([libstatgrab is mandatory and must not be disabled])])

LINKFLAGS=
CLIBFLAGS=

# Checks for libraries.
AX_CHECK_LIB_FLAGS([snmp++], [], [
#ifdef SNMP_PP_NAMESPACE
Snmp_pp::
#endif
DefaultLog::log ()
  ], [
AC_INCLUDES_DEFAULT
$acx_include_sockets

#include <snmp_pp/snmp_pp.h>
  ], [snmp++-3 >= 3.2.25], [
    WITH_LIBSNMP=1
    AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_SNMP})
    AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_SNMP})
    AC_MSG_NOTICE([compiling in support for libsnmp++ (LIBS="$LIBS_SNMP}")])
  ])

AC_MSG_CHECKING([if libsnmp++ is built with SNMPv3 support])
acx_cppflags_safe="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $CLIBFLAGS"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
$acx_include_sockets

#include <snmp_pp/config_snmp_pp.h>
#if !defined(_SNMPv3)
#error no SNMPv3
#endif

int main() { return 0; }
  ])], [
    AC_MSG_RESULT([yes])
    acx_libsnmp_snmpv3=yes
  ], [
    AC_MSG_RESULT([no])
    acx_libsnmp_snmpv3=no
  ]
)
CPPFLAGS="$acx_cppflags_safe"

AM_CONDITIONAL(WITH_LOGGING, false)
AC_MSG_CHECKING([if libsnmp++ is built with logging support])
acx_cppflags_safe="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $CLIBFLAGS"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
$acx_include_sockets

#include <snmp_pp/config_snmp_pp.h>
#if defined(_NO_LOGGING)
#error no logging
#endif

int main() { return 0; }
  ])], [
    AC_MSG_RESULT([yes])
    acx_libsnmp_logging=yes
    AM_CONDITIONAL(WITH_LOGGING, true)
  ], [
    AC_MSG_RESULT([no])
    acx_libsnmp_logging=no
  ]
)
CPPFLAGS="$acx_cppflags_safe"

AM_CONDITIONAL(WITH_AGENT_PP_THREADPOOL,false)
AX_CHECK_LIB_FLAGS([agent++], [], [AC_LANG_SOURCE([
NS_AGENT Timer timer;
timer.get_life();
])], [AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT
$acx_include_sockets

#include <agent_pp/agent++.h>
#include <agent_pp/tools.h>
  ])], [agent++-3 >= 3.5.31], [
    WITH_LIBAGENT=1
    AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_AGENT})
    AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_AGENT})
    AC_MSG_NOTICE([compiling in support for libagent++ (LIBS="$LIBS_AGENT}")])

    AC_MSG_CHECKING([if libagent++ is built with thread support])
    CPPFLAGS="$CPPFLAGS $CLIBFLAGS"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    AC_INCLUDES_DEFAULT
    $acx_include_sockets

#include <agent_pp/agent++.h>
#include <agent_pp/tools.h>
#if !defined(_THREADS)
#error no threads in agent++
#endif

int main() { return 0; }
      ])], [
        AC_MSG_RESULT([yes])
        acx_libagent_threads=yes

        AC_MSG_CHECKING([if libagent++ is built with threadpool support])
        CPPFLAGS="$CPPFLAGS $CLIBFLAGS"
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([
        AC_INCLUDES_DEFAULT
        $acx_include_sockets

#include <agent_pp/agent++.h>
#include <agent_pp/tools.h>
#if !defined(AGENTPP_USE_THREAD_POOL)
#error no threadpool in agent++
#endif

int main() { return 0; }
          ])], [
            AC_MSG_RESULT([yes])
            acx_libagent_threadpool=yes
            AM_CONDITIONAL(WITH_AGENT_PP_THREADPOOL,true)
          ], [
            AC_MSG_RESULT([no])
            acx_libagent_threadpool=no
          ]
        )
      ], [
        AC_MSG_RESULT([no])
        acx_libagent_threads=no
      ]
    )
  ])
CPPFLAGS="$acx_cppflags_safe"

AM_CONDITIONAL(WITH_LIBSTATGRAB,false)
AX_CHECK_LIB_FLAGS([statgrab], [], [sg_init(0)], [
#include <statgrab.h>
  ], [libstatgrab >= 0.18], [
    WITH_LIBSTATGRAB=1
    AM_CONDITIONAL(WITH_LIBSTATGRAB,true)
    AC_DEFINE([WITH_LIBSTATGRAB], 1, [define when libstatgrab is linked])
    AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_STATGRAB})
    AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_STATGRAB})
    AC_MSG_NOTICE([compiling in support for libstatgrab (LIBS="$LIBS_STATGRAB}")])
  ])

AC_MSG_CHECKING([if libstatgrab is built with thread support])
acx_cppflags_safe="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $CLIBFLAGS"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
AC_INCLUDES_DEFAULT

#include <statgrab.h>
#if !defined(SG_SUPPORT_THREADS)
#error no threads in libstatgrab
#endif

int main() { return 0; }
  ])], [
    AC_MSG_RESULT([yes])
    acx_libstatgrab_threads=yes
  ], [
    AC_MSG_RESULT([no])
    acx_libstatgrab_threads=no
  ]
)
CPPFLAGS="$acx_cppflags_safe"

AX_CHECK_LIB_FLAGS([confuse], [], [cfg_init(NULL, 0)], [AC_LANG_SOURCE([
#include <confuse.h>
  ])], [libconfuse >= 2.7], [
    WITH_LIBCONFUSE=1
    AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_CONFUSE})
    AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_CONFUSE})
    AC_MSG_NOTICE([compiling in support for libconfuse (LIBS="$LIBS_CONFUSE}")])
  ])

AM_CONDITIONAL(WITH_LIBLOG4CPLUS, false)
WITH_LIBLOG4CPLUS=0
AS_IF([test "x$acx_libsnmp_logging" = "xyes"], [
  AX_CHECK_LIB_FLAGS([log4cplus], [], [log4cplus::BasicConfigurator config; config.configure()], [
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
    ], [log4cplus >= 1.0.5], [
      AM_CONDITIONAL(WITH_LIBLOG4CPLUS, true)
      WITH_LIBLOG4CPLUS=1
      AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_LOG4CPLUS})
      AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_LOG4CPLUS})
      AC_MSG_NOTICE([compiling in support for liblog4cplus (LIBS="$LIBS_LOG4CPLUS}")])
    ]
  )]
)

AS_IF([test "x$WITH_LIBLOG4CPLUS" = "x1"], [AC_LANG_SOURCE([
  AC_MSG_CHECKING([if liblog4cplus is built with thread support])
  acx_cppflags_safe="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $CLIBFLAGS"
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([
  AC_INCLUDES_DEFAULT

#include <log4cplus/config.hxx>
#if defined(LOG4CPLUS_SINGLE_THREADED)
#error no threads in log4cplus
#endif

  int main() { return 0; }
    ])], [
      AC_MSG_RESULT([yes])
      acx_liblog4cplus_threads=yes
    ], [
      AC_MSG_RESULT([no])
      acx_liblog4cplus_threads=no
    ]
  )
  CPPFLAGS="$acx_cppflags_safe"
])])

AM_CONDITIONAL(WITH_BUNDLED_LIBJSON,false)
AM_CONDITIONAL(WITH_EXTERNAL_COMMANDS,false)
WITH_LIBJSON=0
WITH_BUNDLED_LIBJSON=0
WITH_EXTERNAL_COMMANDS=0

AS_IF(
  [test "x$acx_with_libjson" != "xno"],
  [
    AS_IF(
      [test "x$BUNDLED_LIBJSON" != "xyes"],
      [
        AX_CHECK_LIB_FLAGS([json], [], [json_parser_init(NULL, NULL, NULL, NULL)], [
#include <json.h>
          ], [libjson >= 0.8], [
            WITH_LIBJSON=1
	    WITH_EXTERNAL_COMMANDS=1
            AC_LIB_APPENDTOVAR([LINKFLAGS], ${LIBS_JSON})
            AC_LIB_APPENDTOVAR([CLIBFLAGS], ${INC_JSON})
            AM_CONDITIONAL(WITH_EXTERNAL_COMMANDS,true)
            AC_MSG_NOTICE([compiling in support for libjson (LIBS="$LIBS_JSON}")])
          ], [
            AS_IF(
              [test "x$acx_with_libjson" = "xyes" -a "x$BUNDLED_LIBJSON" = "xfallback"],
              [
                WITH_LIBJSON=1
                WITH_BUNDLED_LIBJSON=1
		WITH_EXTERNAL_COMMANDS=1
                AM_CONDITIONAL(WITH_BUNDLED_LIBJSON,true)
                AM_CONDITIONAL(WITH_EXTERNAL_COMMANDS,true)
                AC_MSG_NOTICE([compiling in support for libjson (LIBS="json.o")])
              ]
            )
          ]
        )
      ], [
        AC_MSG_CHECKING([for json])
        AC_MSG_RESULT([bundled])
        WITH_LIBJSON=1
        WITH_BUNDLED_LIBJSON=1
	WITH_EXTERNAL_COMMANDS=1
        AM_CONDITIONAL(WITH_BUNDLED_LIBJSON,true)
        AM_CONDITIONAL(WITH_EXTERNAL_COMMANDS,true)
        AC_MSG_NOTICE([compiling in support for libjson (LIBS="json.o")])
      ]
    )
  ]
)

AC_LANG_POP([C++])

AM_CONDITIONAL(WITH_SU_CMD,false)
AS_IF([test "x$WITH_EXTERNAL_COMMANDS" = "x1"], [
# su-cmd is only required when libjson is available
    AC_MSG_NOTICE([building with external command support])
    AS_IF([test "x$SU_CMD" != "xno"],
    [
      AS_IF([test "x$SU_CMD" = "xyes" -o "x$SU_CMD" = "xcheck"],
        [
          for cmd in sudo su; do
            AC_PATH_PROG([SU_CMD],[$cmd], [])
            if test "x$SU_CMD" != "x"; then
              break
            fi
          done
        ]
      )

      AS_IF([test "x$SU_CMD" != "x"],
        [
          WITH_SU_CMD=1
          AC_MSG_CHECKING([for $SU_CMD arguments])
          AS_IF([test "x$SU_ARGS" = "xyes" -o "x$SU_ARGS" = "xcheck"],[
            AS_CASE(
              [$SU_CMD],
              [*/sudo],[
              SU_ARGS="{\"-u\", \"%u\", \"%c\"}"
              AC_MSG_RESULT([$SU_ARGS])
              AM_CONDITIONAL(WITH_SU_CMD,true)
              ],
              [*/su],[
              SU_ARGS="{\"%u\", \"-c\", \"%c\"}"
              AC_MSG_RESULT([$SU_ARGS])
              AM_CONDITIONAL(WITH_SU_CMD,true)
              ],
              [
              SU_CMD=
              WITH_SU_CMD=0
              AC_MSG_RESULT([unknown su-cmd - disabling defaults])
              ])
          ],[test "x$SU_ARGS" = "x" -o "x$SU_ARGS" = "xno"],[
            WITH_SU_CMD=0
            AC_MSG_RESULT([none (disables using)])
            SU_ARGS=
          ],[
            AC_MSG_RESULT([$SU_ARGS])
            AM_CONDITIONAL(WITH_SU_CMD,true)
          ])
        ],
        [
          WITH_SU_CMD=0
          SU_ARGS=
        ]
      )
    ],
    [
      SU_CMD=
      SU_ARGS=
      WITH_SU_CMD=0
    ])
  ], [
    AC_MSG_NOTICE([building without external command support])
    SU_CMD=
    SU_ARGS=
    WITH_SU_CMD=0
  ]
)

dnl check whether libsnmp++/libagent++ and libstatgrab have the same thread setting as "we"
AC_MSG_CHECKING([if thread flags are sane])
acx_thread_flags_sane="yes"
AS_IF([test "x$ac_cv_enable_threads" = "xyes"], [
  dnl prove if all required libraries have thread support on
  AS_IF([test "x$acx_libagent_threads" != "xyes"], [
    AS_IF([test "x$acx_thread_flags_sane" != "xno"], [
      AC_MSG_RESULT([no])
      acx_thread_flags_sane="no"
    ])
    AC_MSG_ERROR([threads enabled, but snmp++/agent++ built without thread support])
  ])
  AS_IF([test "x$acx_libstatgrab_threads" != "xyes"], [
    AS_IF([test "x$acx_thread_flags_sane" != "xno"], [
      AC_MSG_RESULT([no])
      acx_thread_flags_sane="no"
    ])
    AC_MSG_ERROR([threads enabled, but libstatgrab built without thread support])
  ])
  AS_IF([test "x$WITH_LIBLOG4CPLUS" = "x1"], [
    AS_IF([test "x$acx_liblog4cplus_threads" != "xyes"], [
      AS_IF([test "x$acx_thread_flags_sane" != "xno"], [
	AC_MSG_RESULT([no])
	acx_thread_flags_sane="no"
      ])
      AC_MSG_ERROR([threads enabled, but liblog4cplus built without thread support])
    ])
  ])
], [
  AS_IF([test "x$acx_libagent_threads" = "xyes"], [
    AS_IF([test "x$acx_thread_flags_sane" = "xyes"], [
      AC_MSG_RESULT([no])
      acx_thread_flags_sane="no"
    ])
    AC_MSG_WARN([threads disabled, but snmp++/agent++ built with thread support - behavior might be unpredictable])
  ])
  AS_IF([test "x$acx_libstatgrab_threads" = "xyes"], [
    AS_IF([test "x$acx_thread_flags_sane" = "xyes"], [
      AC_MSG_RESULT([no])
      acx_thread_flags_sane="no"
    ])
    AC_MSG_WARN([threads disabled, but libstatgrab built with thread support - behavior might be unpredictable])
  ])
  AS_IF([test "x$WITH_LIBLOG4CPLUS" = "x1"], [
    AS_IF([test "x$acx_liblog4cplus_threads" = "xyes"], [
      AS_IF([test "x$acx_thread_flags_sane" = "xyes"], [
	AC_MSG_RESULT([no])
	acx_thread_flags_sane="no"
      ])
      AC_MSG_WARN([threads disabled, but liblog4cplus built with thread support - behavior might be unpredictable])
    ])
  ])
])
AS_IF([test "x$acx_thread_flags_sane" = "xyes"], [
  AC_MSG_RESULT([yes])
])

HAVE_USER_LOOKUP=0
AS_IF([test "x$USER_LOOKUP" != "xno"], [
    AS_IF([test "x$USER_LOOKUP" = "xpam" -o "x$USER_LOOKUP" = "xyes" -o "x$USER_LOOKUP" = "xcheck"], [
        AC_CHECK_FUNCS([setpwent endpwent setgrent endgrent])
        AS_IF([test "$ac_cv_func_setpwent" = "yes" -a "$ac_cv_func_endpwent" = "yes" -a "$ac_cv_func_setgrent" = "yes" -a "$ac_cv_func_endgrent" = "yes"], [
            AC_CHECK_FUNCS([getpwnam_r getpwuid_r getgrnam_r getgrgid_r])
            AS_IF([test "$ac_cv_func_getpwnam_r" = "yes" -a "$ac_cv_func_getpwuid_r" = "yes" -a "$ac_cv_func_getgrnam_r" = "yes" -a "$ac_cv_func_getgrgid_r" = "yes"], [
                USER_LOOKUP_WITH_PAM=1
                HAVE_USER_LOOKUP=1
                AC_MSG_NOTICE(using user/group password databases to lookup for user/group details)
            ], [
                AC_CHECK_FUNCS([getpwnam getpwuid getgrnam getgrgid])
                AS_IF([test "$ac_cv_func_getpwnam" = "yes" -a "$ac_cv_func_getpwuid" = "yes" -a "$ac_cv_func_getgrnam" = "yes" -a "$ac_cv_func_getgrgid" = "yes"], [
                    USER_LOOKUP_WITH_PAM=1
                    HAVE_USER_LOOKUP=1
                    AC_MSG_NOTICE(using user/group password databases to lookup for user/group details)
                ])
            ])
        ])
    ])
    AS_IF([test "x$HAVE_USER_LOOKUP" = "x1"], [], [test "x$USER_LOOKUP" = "xfile" -o "x$USER_LOOKUP" = "xyes" -o "x$USER_LOOKUP" = "xcheck"], [
        AC_CHECK_FUNCS([fgetpwent_r fgetgrent_r])
        AS_IF([test "$ac_cv_func_fgetpwent_r" = "yes" -a "$ac_cv_func_fgetgrent_r" = "yes"], [
            USER_LOOKUP_WITH_FILE=1
            HAVE_USER_LOOKUP=1
            AC_MSG_NOTICE(using user/group password files to lookup for user/group details)
        ], [
            AC_CHECK_FUNCS([fgetpwent fgetgrent])
            AS_IF([test "$ac_cv_func_fgetpwent" = "yes" -a "$ac_cv_func_fgetgrent" = "yes"], [
                USER_LOOKUP_WITH_FILE=1
                HAVE_USER_LOOKUP=1
                AC_MSG_NOTICE(using user/group password files to lookup for user/group details)
            ])
        ])
    ])
])
AS_IF([test "$HAVE_USER_LOOKUP" != "1"], [
    AS_IF([test "x$USER_LOOKUP" = "xcheck" -o "x$USER_LOOKUP" = "xno"], [
        AC_MSG_NOTICE(user/group details lookup turned off)
    ], [
        AC_MSG_FAILURE([no usable lookup method for user/group details])
    ])
])

AC_DEFINE_UNQUOTED([WITH_USER_LOOKUP], $HAVE_USER_LOOKUP, [set to 0 or 1, depending if user lookup should be enabled or not])
AS_IF([test "x$USER_LOOKUP_WITH_PAM" = "x1"], [
    AC_DEFINE_UNQUOTED([WITH_PAM_USER_LOOKUP], $USER_LOOKUP_WITH_PAM, [defined if pluggable database modules should be used to get user/group details])
])
AS_IF([test "x$USER_LOOKUP_WITH_FILE" = "x1"], [
    AC_DEFINE_UNQUOTED([WITH_FILE_USER_LOOKUP], $USER_LOOKUP_WITH_FILE, [defined if local files should be used to get user/group details])
])

AS_IF([test "x$ENABLE_DOCBUILD" = "xcheck"], [
  ENABLE_DOCBUILD="$USE_MAINTAINER_MODE"
])

AM_CONDITIONAL(ENABLE_DOC_RULES,false)
AS_IF([test "x$ENABLE_DOCBUILD" = "xyes"], [
  AC_MSG_NOTICE([Build rules for documentation will be enabled])
  AM_CONDITIONAL(ENABLE_DOC_RULES,true)
], [
  AC_MSG_NOTICE([Build rules for documentation will be disabled])
])

dnl Libtool setup.
dnl must be latest to catch rpath compiled in by changes made during checks before

LT_INIT
AC_PROG_LIBTOOL

CPPFLAGS="$CPPFLAGS -DLOCALSTATEDIR=\"\\\"$localstatedir\\\"\" -DSYSCONFDIR=\"\\\"$sysconfdir\\\"\""

AC_SUBST(SMART_SNMP_MAJOR_VERSION)
AC_SUBST(SMART_SNMP_MINOR_VERSION)
AC_SUBST(SMART_SNMP_MICRO_VERSION)

AC_SUBST(WITH_LIBJSON)
AC_SUBST(WITH_BUNDLED_LIBJSON)
AC_SUBST(WITH_LIBLOG4CPLUS)

AC_SUBST(LINKFLAGS)
AC_SUBST(CLIBFLAGS)

AC_SUBST(WITH_SU_CMD)
AC_SUBST(SU_CMD)
AC_SUBST(SU_ARGS)

AC_CONFIG_FILES([Makefile
                 etc/Makefile
                 include/Makefile
                 include/smart-snmpd/Makefile
                 include/smart-snmpd/mibutils/Makefile
                 include/smart-snmpd/mibs/Makefile
                 include/smart-snmpd/mibs/statgrab/Makefile
                 include/smart-snmpd/mibs/extcmd/Makefile
                 src/Makefile
                 src/mibutils/Makefile
                 src/mibs/Makefile
                 src/mibs/statgrab/Makefile
                 src/mibs/extcmd/Makefile
])
AC_OUTPUT(smart-snmpd.pc include/smart-snmpd/smart-snmpd.h)
