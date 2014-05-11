## Process this file with automake to produce Makefile.in

SUBDIRS=mibutils mibs
smartsnmpdincdir = $(includedir)/smart-snmpd

if WITH_LIBLOG4CPLUS
log4cplus_headers =	log4cplus.h
endif

smartsnmpdinc_HEADERS =	agent.h \
			cmndline.h \
			config.h \
			datadiff.h \
			datasource.h \
			functional.h \
			log.h \
			mibobject.h \
			oids.h \
			property.h \
			pwent.h \
			resourcelimits.h \
			updatethread.h \
			ui.h \
			smart-snmpd.h \
			sysbuf.h \
			$(log4cplus_headers)

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(smartsnmpdincdir)

EXTRA_DIST = smart-snmpd.h.in
