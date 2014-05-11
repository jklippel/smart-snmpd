## Process this file with automake to produce Makefile.in

sysconf_DATA=	smart-snmpd.conf.example \
		$(sysconf_log_properties)

if WITH_SU_CMD
cfg_su_edit=	-e 's|@SU_CMD[@]|$(SU_CMD)|g' \
		-e 's|@SU_ARGS[@]|$(SU_ARGS)|g' \
		-e 's|@su-if[@]||g'
else
cfg_su_edit=	-e 's|@SU_CMD[@]||g' \
		-e 's|@SU_ARGS[@]|{}|g' \
		-e 's|@su-if[@]|// |g'
endif

if WITH_LOGGING
if WITH_LIBLOG4CPLUS
cfg_log_edit=	-e 's|@log-if[@]||g' \
		-e 's|@log-endif[@]||g' \
		-e 's|@log-file[@]|log4cplus-property-file|g' \
		-e 's|@log-spec[@]|$(sysconfdir)/smart-snmpd-log.properties|g'
else
cfg_log_edit=	-e 's|@log-if[@]||g' \
		-e 's|@log-endif[@]||g' \
		-e 's|@log-file[@]|log-file|g' \
		-e 's|@log-spec[@]|$(localstatedir)/log/smart-snmpd.log|g'
endif
else
cfg_log_edit=	-e 's|@log-if[@]|/*|g' \
		-e 's|@log-endif[@]|*/|g' \
		-e 's|@log-file[@]|log-file|g' \
		-e 's|@log-spec[@]|$(localstatedir)/log/smart-snmpd.log|g'
endif

if USE_V1_COMMUNITY
cfg_v1_edit=	-e 's|@use-v1-community[@]||g' \
		-e 's|@default-v1-community@|$(DEFAULT_V1_COMMUNITY)|g'
else
cfg_v1_edit=	-e 's|@use-v1-community[@]|// |g' \
		-e 's|@default-v1-community@|$(DEFAULT_V1_COMMUNITY)|g'
endif

if USE_V2_COMMUNITY
cfg_v2_edit=	-e 's|@use-v2-community[@]||g' \
		-e 's|@default-v2-community[@]|$(DEFAULT_V2_COMMUNITY)|g'
else
cfg_v2_edit=	-e 's|@use-v2-community[@]|// |g' \
		-e 's|@default-v2-community[@]|$(DEFAULT_V2_COMMUNITY)|g'
endif

if WITH_AGENT_PP_THREADPOOL
cfg_tp_edit=	-e 's|@if-threadpool[@]||g'
else
cfg_tp_edit=	-e 's|@if-threadpool[@]|// |g'
endif

cfg_edit= sed	-e 's|@localstatedir[@]|$(localstatedir)|g' \
		$(cfg_su_edit) \
		$(cfg_log_edit) \
		$(cfg_v1_edit) \
		$(cfg_v2_edit) \
		$(cfg_tp_edit)
 
smart-snmpd.conf.example: Makefile $(srcdir)/smart-snmpd.conf.example.in
	rm -f $@ $@.tmp
	srcdir=''; \
	  test -f ./$@.in || srcdir=$(srcdir)/; \
	  $(cfg_edit) $${srcdir}$@.in >$@.tmp
 
	chmod +r $@.tmp
	chmod a-wx $@.tmp
	mv $@.tmp $@
 
if WITH_LOGGING
if WITH_LIBLOG4CPLUS
prop_edit= sed	-e 's|@log-file-path[@]|$(localstatedir)/log/smart-snmpd.log|g'

smart-snmpd-log.properties.example: Makefile $(srcdir)/smart-snmpd-log.properties.example.in
	rm -f $@ $@.tmp
	srcdir=''; \
	  test -f ./$@.in || srcdir=$(srcdir)/; \
	  $(prop_edit) $${srcdir}$@.in >$@.tmp
 
	chmod +r $@.tmp
	chmod a-wx $@.tmp
	mv $@.tmp $@

sysconf_log_properties=	smart-snmpd-log.properties.example
endif
endif

EXTRA_DIST =	smart-snmpd.conf.example.in \
		smart-snmpd-log.properties.example.in
