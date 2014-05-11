## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)/include $(PTHREAD_CFLAGS)
SUBDIRS = include src etc
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST =	smart-snmpd.pc.in build-smart-snmpd.h \
		INSTALL.tex INSTALL.pdf INSTALL.html INSTALL.css \
		EXTENDING.tex EXTENDING.pdf EXTENDING.html EXTENDING.css \
		OPERATION.tex OPERATION.pdf OPERATION.html OPERATION.css

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = smart-snmpd.pc

dist_doc_DATA = AUTHORS CHANGES COPYING EXTENDING INSTALL NOTICE README OPERATION

if ENABLE_DOC_RULES
INSTALL_MAINTAINER_CLEAN=	INSTALL INSTALL.pdf INSTALL.html INSTALL.css \
				EXTENDING EXTENDING.pdf EXTENDING.html EXTENDING.css \
				OPERATION OPERATION.pdf OPERATION.html OPERATION.css

SUFFIXES = .tex .htex .ptex .htm .html .pdf

EXTENDING.html: EXTENDING.pdf

EXTENDING: EXTENDING.htm
	w3m -O iso-8859-1 -dump $< >$@

OPERATION.html: OPERATION.pdf

OPERATION: OPERATION.htm
	w3m -O iso-8859-1 -dump $< >$@

INSTALL.html: INSTALL.pdf

INSTALL: INSTALL.htm
	w3m -O iso-8859-1 -dump $< >$@

.tex.htex:
	echo "\\documentclass{article}" >$@
	echo "" >>$@
	echo "\\input{$<}" >>$@

.tex.ptex:
	echo "\\documentclass[paper=a4]{akktecdoc}" >$@
	echo "" >>$@
	echo "\\usepackage[textheight=245mm]{geometry}" >>$@
	echo "" >>$@
	echo "\\input{$<}" >>$@

.htex.html:
	htlatex $< "html,1,info"

.html.htm:
	sed -e 's/&#8221\;/\&quot\;/g' <$< | sed -e "s/\&#8217\;/'/g" >$@

.ptex.pdf:
	pdflatex $<
	pdflatex $<
	pdflatex $<

clean-local:
	rm -f *.htm *.htex *.4ct *.idv *.lg
	rm -f *.aux *.dvi *.toc *.tmp *.ptex
	rm -f INSTALL.log INSTALL.out
	rm -f EXTENDING.log EXTENDING.out
	rm -f OPERATION.log OPERATION.out
	rm -f *.xref $(INSTALL_MAINTAINER_CLEAN)
	rm -f *~
endif
