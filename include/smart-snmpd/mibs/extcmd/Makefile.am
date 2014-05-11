## Process this file with automake to produce Makefile.in

smartsnmpdincdir = $(includedir)/smart-snmpd/mibs/extcmd

if WITH_BUNDLED_LIBJSON
bundled_json_header =	json.h
endif

smartsnmpdinc_HEADERS =	datasourceextcmd.h \
			mibextcmd.h \
			extcmd.h \
			parseextjson.h \
			$(bundled_json_header)
