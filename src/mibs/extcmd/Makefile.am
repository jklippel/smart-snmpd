## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include $(PTHREAD_CFLAGS) @CLIBFLAGS@

noinst_LTLIBRARIES = libext_mibs.la

if WITH_BUNDLED_LIBJSON
json_sources =	json.c
endif

libext_mibs_la_SOURCES =	datasourceextcmd.cpp \
				extcmd.cpp \
				parseextjson.cpp \
				mibmodule_ext.cpp \
				$(json_sources)
