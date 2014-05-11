## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include $(PTHREAD_CFLAGS) @CLIBFLAGS@

noinst_LTLIBRARIES = libmibutils.la

if WITH_STL_CONTAINER
libmibutils_la_SOURCES =	mibcontainer.cpp
else
libmibutils_la_SOURCES =	mibcomposed.cpp
endif
