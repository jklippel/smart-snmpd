## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_builddir)/include -I$(top_srcdir)/include $(PTHREAD_CFLAGS) @CLIBFLAGS@

SUBDIRS = mibutils mibs

sbin_PROGRAMS = smart-snmpd

if WITH_LIBLOG4CPLUS
log4cplus_sources =		log4cplus.cpp
endif

smart_snmpd_SOURCES = 		agent.cpp \
				cmndline.cpp \
				config.cpp \
				datasource.cpp \
				mibobject.cpp \
				pwent.cpp \
				resourcelimits.cpp \
				updatethread.cpp \
				ui.cpp \
				smart-snmpd.cpp \
				tools.cpp \
				$(log4cplus_sources)

if WITH_EXTERNAL_COMMANDS
ext_mibs_ldadd =  $(top_builddir)/src/mibs/extcmd/libext_mibs.la
endif
if WITH_LIBSTATGRAB
sg_mibs_ldadd = $(top_builddir)/src/mibs/statgrab/libsg_mibs.la
endif
LDADDS = $(top_builddir)/src/mibutils/libmibutils.la $(sg_mibs_ldadd) $(ext_mibs_ldadd)

smart_snmpd_LDADD =		$(LDADDS)

smart_snmpd_LDFLAGS =		@LINKFLAGS@
