## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include $(PTHREAD_CFLAGS) @CLIBFLAGS@

noinst_LTLIBRARIES = libsg_mibs.la

libsg_mibs_la_SOURCES =	datasourcestatgrab.cpp \
			datasourcecpu.cpp \
			datasourcedaemonstatus.cpp \
			datasourcediskio.cpp \
			datasourcefilesystem.cpp \
			datasourcehostinfo.cpp \
			datasourceload.cpp \
			datasourcememory.cpp \
			datasourcenetworkio.cpp \
			datasourceprocess.cpp \
			datasourceswapio.cpp \
			datasourceuserlogin.cpp \
			mibmodule_sg.cpp
