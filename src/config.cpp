/*
 * Copyright 2010,2011 Matthias Haag, Jens Rehsack
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <build-smart-snmpd.h>

#include <smart-snmpd/oids.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/cmndline.h>
#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/mibobject.h>

#include <snmp_pp/usm_v3.h>
#include <snmp_pp/log.h>
#include <agent_pp/snmp_pp_ext.h>
#include <agent_pp/vacm.h>
#include <agent_pp/snmp_pp_ext.h>

#include <stdarg.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.config";

static Oidx rootOid(SM_MIB_OBJECTS);

map<string, Oidx> const & getMapByName();

const char *loglevels[] = { "error", "warning", "event", "info", "debug", "user" };

extern "C"
{
    int validate_root_cfg( cfg_t *cfg ); // no callback

    int cb_validate_root_cfg( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_mibobject( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_inrmibobject( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_extmibobject( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_statgrab_opts( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_usm_user( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_vacm_group( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_vacm_view( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_vacm_access( cfg_t *cfg, cfg_opt_t *opt );
    int cb_validate_v1v2_community( cfg_t *cfg, cfg_opt_t *opt );

    void report_config_error( cfg_t *cfg, const char *fmt, va_list ap );
    int cb_verify_authproto( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_privproto( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_secmodel( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_storagetype( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_seclevel( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_secmatch( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_viewtype( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_oid( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_v1v2_access( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_rlimit( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );
    int cb_verify_onfatal( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result );

    void cb_free_rlimit(void *ptr);
}

static struct cfg_opt_t resource_opts[] = {
    CFG_PTR_CB("core", NULL, CFGF_NONE, &cb_verify_rlimit, &cb_free_rlimit),
    CFG_PTR_CB("cpu", NULL, CFGF_NONE, &cb_verify_rlimit, &cb_free_rlimit),
    CFG_PTR_CB("data", NULL, CFGF_NONE, &cb_verify_rlimit, &cb_free_rlimit),
    CFG_PTR_CB("filesize", NULL, CFGF_NONE, &cb_verify_rlimit, &cb_free_rlimit),
    CFG_PTR_CB("files", NULL, CFGF_NONE, &cb_verify_rlimit, &cb_free_rlimit),
    CFG_PTR_CB("stack", NULL, CFGF_NONE, &cb_verify_rlimit, &cb_free_rlimit),
    CFG_PTR_CB("mem", NULL, CFGF_NONE, &cb_verify_rlimit, &cb_free_rlimit),
    CFG_END()
};

static struct cfg_opt_t mibobject_opts[] = {
    CFG_BOOL("mib-enabled", cfg_true, CFGF_NONE),
    CFG_BOOL("async-update", cfg_false, CFGF_NONE),
    CFG_INT("cache-timeout", 30, CFGF_NONE),
    CFG_END()
};
static struct cfg_opt_t inrmibobject_opts[] = {
    CFG_BOOL("mib-enabled", cfg_true, CFGF_NONE),
    CFG_BOOL("async-update", cfg_false, CFGF_NONE),
    CFG_INT("cache-timeout", 30, CFGF_NONE),
    CFG_INT("mr-interval", 0, CFGF_NONE),
    CFG_END()
};
static struct cfg_opt_t extmibobject_opts[] = {
    CFG_BOOL("mib-enabled", cfg_true, CFGF_NONE),
    CFG_BOOL("async-update", cfg_false, CFGF_NONE),
    CFG_INT("cache-timeout", 30, CFGF_NONE),
    CFG_STR("command", 0, CFGF_NONE),
    CFG_STR_LIST("args", NULL, CFGF_NONE),
#ifdef WITH_SU_CMD
    CFG_STR("user", 0, CFGF_NONE),
#endif
    CFG_INT("sub-oid", 0, CFGF_NONE),
    CFG_SEC("rlimits", resource_opts, CFGF_NONE),
    CFG_END()
};
static struct cfg_opt_t statgrab_opts[] = {
    CFG_STR_LIST("valid-filesystems", 0, CFGF_NONE),
    CFG_END()
};
static struct cfg_opt_t usm_entry_opts[] = {
    CFG_INT_CB("auth-proto", SNMP_AUTHPROTOCOL_HMACSHA, CFGF_NONE, &cb_verify_authproto),
    CFG_STR("auth-key", 0, CFGF_NONE),
    CFG_INT_CB("priv-proto", SNMP_PRIVPROTOCOL_DES, CFGF_NONE, &cb_verify_privproto),
    CFG_STR("priv-key", 0, CFGF_NONE),
    CFG_END()
};
static struct cfg_opt_t vacm_group_opts[] = {
    CFG_INT_CB("security-model", SNMP_SECURITY_MODEL_USM, CFGF_NONE, &cb_verify_secmodel),
    CFG_STR_LIST("security-name", 0, CFGF_NONE),
    CFG_INT_CB("storage-type", storageType_volatile, CFGF_NONE, &cb_verify_storagetype),
    CFG_END()
};
static struct cfg_opt_t vacm_view_opts[] = {
    CFG_STR_CB("sub-tree", 0, CFGF_NONE, &cb_verify_oid),
    CFG_STR("mask", "", CFGF_NONE),
    CFG_INT_CB("view-type", view_included, CFGF_NONE, &cb_verify_viewtype),
    CFG_INT_CB("storage-type", storageType_nonVolatile, CFGF_NONE, &cb_verify_storagetype),
    CFG_END()
};
static struct cfg_opt_t vacm_access_opts[] = {
    CFG_STR("group-name", 0, CFGF_NONE),
    CFG_STR("context", "", CFGF_NONE),
    CFG_INT_CB("security-model", SNMP_SECURITY_MODEL_USM, CFGF_NONE, &cb_verify_secmodel),
    CFG_INT_CB("security-level", SecurityLevel_authPriv, CFGF_NONE, &cb_verify_seclevel),
    CFG_INT_CB("match", match_exact, CFGF_NONE, &cb_verify_secmatch),
    CFG_STR("read-view", "restricted", CFGF_NONE),
    CFG_STR("write-view", "restricted", CFGF_NONE),
    CFG_STR("notify-view", "restricted", CFGF_NONE),
    CFG_INT_CB("storage-type", storageType_nonVolatile, CFGF_NONE, &cb_verify_storagetype),
    CFG_END()
};
static struct cfg_opt_t vacm_v1v2_opts[] = {
    CFG_INT_LIST_CB("access", 0, CFGF_NONE, &cb_verify_v1v2_access),
    CFG_END()
};
#ifndef _NO_LOGGING
static struct cfg_opt_t log_class_opts[] = {
    CFG_INT("error", 15, CFGF_NONE),
    CFG_INT("warning", 15, CFGF_NONE),
    CFG_INT("event", 10, CFGF_NONE),
    CFG_INT("info", 5, CFGF_NONE),
    CFG_INT("debug", 0, CFGF_NONE),
    CFG_INT("user", 0, CFGF_NONE),
    CFG_END()
};
#endif

int
validate_root_cfg( cfg_t *cfg )
{
    char *fn;
    int rc = 0;
#ifdef WITH_SU_CMD
    int n;
#endif

    assert( cfg );

    // XXX documentation mismatch - cfg_size() returns number of elements, regardless whether they're parsed or from defaults
    if( cfg_size( cfg, "status-file" ) != 0 )
    {
        fn = cfg_getstr( cfg, "status-file" );
        if( ( NULL != fn ) && ( 0 != strlen(fn) ) )
        {
	    int fd;
            if( ( fd = open( fn, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP ) ) < 0 )
            {
                cfg_error( cfg, "validate root-configuration: status-file %s: %s\n", fn, strerror(errno) );
                rc = -1;
            }
            close(fd);
        }
    }

#if 0
    if( cfg_size( cfg, "pid-file" ) != 0 )
    {
        fn = cfg_getstr( cfg, "pid-file" );
        if( 0 == strlen(fn) )
        {
            cfg_error( cfg, "validate root-configuration: pid-file must not be empty\n" );
            rc = -1;
        }
    }
#endif

#ifndef _NO_LOGGING
#ifdef WITH_LIBLOG4CPLUS
    if( cfg_size( cfg, "log4cplus-property-file" ) != 0 )
    {
        fn = cfg_getstr( cfg, "log4cplus-property-file" );
        if( ( NULL != fn ) && ( 0 != strlen(fn) ) )
        {
            if( access(fn, R_OK) < 0 )
            {
                cfg_error( cfg, "validate root-configuration: log4cplus-property-file %s: %s\n", fn, strerror(errno) );
                rc = -1;
            }
        }
    }
#else
    if( cfg_size( cfg, "log-file" ) != 0 )
    {
        fn = cfg_getstr( cfg, "log-file" );
        if( ( NULL != fn ) && ( 0 != strlen(fn) ) )
        {
	    int fd;
            if( ( fd = open( fn, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP ) ) < 0 )
            {
                cfg_error( cfg, "validate root-configuration: log-file %s: %s\n", fn, strerror(errno) );
                rc = -1;
            }
            close(fd);
        }
    }
#endif
#endif

    fn = cfg_getstr( cfg, "listen-on" );
    if( fn && strlen( fn ) )
    {
        UdpAddress udpaddr(fn);
        if( !udpaddr.valid() )
        {
            cfg_error( cfg, "validate root-configuration: listen-on %s: invalid udp-address\n", fn );
            rc = -1;
        }

        // XXX check if port is specified ... together with listen-on
        long int port = cfg_getint( cfg, "port" );
        if( port > 0 )
        {
            cfg_error( cfg, "validate root-configuration: specified both 'listen-on' and 'port'\n" );
            rc = -1;
        }
    }

#ifdef AGENTPP_USE_THREAD_POOL
    long int jobpoolthreads = cfg_getint( cfg, "job-threads" );
    if( jobpoolthreads < 0 )
    {
        cfg_error( cfg, "validate root-configuration: invalid value for 'job-threads', must be greater than or equal to 0\n" );
        rc = -1;
    }
#endif

#ifdef WITH_SU_CMD
    // XXX check if su-cmd is executable and su-args contains 2 "%s"
    fn = cfg_getstr( cfg, "su-cmd" );
    if( fn && strlen( fn ) )
    {
        if( 0 != access(fn, X_OK) )
        {
            cfg_error( cfg, "validate root-configuration: su-cmd %s: %s\n", fn, strerror(errno) );
            rc = -1;
        }
    }
    n = cfg_size( cfg, "su-args" );
    if( n != 0 )
    {
        int i;
        int found_u = 0, found_c = 0;

        if( n < 2 )
        {
            cfg_error( cfg, "validate root-configuration: su-args requires at least 2 arguments" );
            rc = -1;
        }

        for( i = 0; i < n; ++i )
        {
            char *buf = cfg_getnstr( cfg, "su-args", i );
            if( buf != NULL )
            {
                string suarg( buf );
                if( suarg == "%u" )
                    ++found_u;
                else if( suarg == "%c" )
                    ++found_c;
            }
        }

        if( found_u != 1 )
        {
            cfg_error( cfg, "validate root-configuration: missing user placeholder in su-args" );
            rc = -1;
        }

        if( found_c != 1 )
        {
            cfg_error( cfg, "validate root-configuration: missing command placeholder in su-args" );
            rc = -1;
        }
    }
#endif

    return rc;
}

int
cb_validate_log_class( cfg_t *cfg, cfg_opt_t *opt )
{
    int rc = 0;
    cfg_t *sec = cfg_opt_getnsec( opt, 0 );
    if( !sec )
    {
        cfg_error( cfg, "validate log-class: section is NULL?!" );
        return -1;
    }

    for( size_t i = 0; i < lengthof(loglevels); ++i )
    {
        long int level = cfg_getint( sec, loglevels[i] );
        if( level < -1 || level > 15 )
        {
            cfg_error( cfg, "validate log-class: invalid log level for log class '%s': %d.", loglevels[i], level );
            cfg_error( cfg, "valid log level must be in (-1 .. 15)." );
            rc = -1;
        }
    }

    return rc;
}

static int
validate_mibobject( cfg_t *cfg, cfg_opt_t *opt, bool check_oid = true )
{
    /* only validate the last mibobject */
    const char *title;
    cfg_t *sec = cfg_opt_getnsec( opt, cfg_opt_size(opt) - 1 );
    if( !sec )
    {
        cfg_error( cfg, "validate mibobject: section is NULL?!" );
        return -1;
    }

    title = cfg_title( sec );
    if( !title )
    {
        cfg_error( cfg, "validate mibobject: missing title" );
        return -1;
    }

    if( check_oid )
    {
        // XXX validate title
        Oidx tmpoid(title);

        if( 0 == tmpoid.len() )
        {
            tmpoid = FindOidForMibName(title);
            if( 0 == tmpoid.len() )
            {
                cfg_error( cfg, "validate mibobject: unknown OID/Name '%s'", title );
                return -1;
            }
        }

        if( !rootOid.is_root_of(tmpoid) )
        {
            cfg_error( cfg, "validate mibobject: mib's oid must be below " SM_MIB_OBJECTS );
            return -1;
        }
    }

    return 0;
}

int
cb_validate_mibobject( cfg_t *cfg, cfg_opt_t *opt )
{
    return validate_mibobject( cfg, opt, true );
}

int
cb_validate_inrmibobject( cfg_t *cfg, cfg_opt_t *opt )
{
    long int mr_interval, cache_timeout;
    cfg_t *sec = cfg_opt_getnsec( opt, cfg_opt_size(opt) - 1 );
    if( 0 != validate_mibobject( cfg, opt, true ) )
        return -1;

    if( 0 != ( cache_timeout = cfg_getint( sec, "cache-timeout" ) ) )
    {
        if( !( mr_interval = cfg_getint( sec, "mr-interval" ) ) )
        {
            cfg_error( cfg, "validate inrobject: mr-interval must not be 0" );
            return -1;
        }

        if( mr_interval < cache_timeout )
        {
            cfg_error( cfg, "validate inrobject: mr-interval must not be smaller than cache-timeout" );
            return -1;
        }

        if( ( mr_interval % cache_timeout ) != 0 )
        {
            cfg_error( cfg, "validate inrobject: mr-interval must be a multiple of cache-timeout" );
            return -1;
        }
    }
    /* else assume one comparing item to diff against */
    if( !cfg_getbool( sec, "async-update" ) )
        cfg_error( cfg, "validate inrobject: it's strongly recommended to update interval data asynchronous" );

    return 0;
}

int
cb_validate_extmibobject( cfg_t *cfg, cfg_opt_t *opt )
{
    char *f;
    /* only validate the last mibobject */
    cfg_t *sec = cfg_opt_getnsec( opt, cfg_opt_size(opt) - 1 );
    if( 0 != validate_mibobject( cfg, opt, false ) )
        return -1;

    if( !(f = cfg_getstr( sec, "command" ) ) )
    {
        cfg_error( cfg, "validate extobject: missing command entry" );
        return -1;
    }
    else if( access(f, X_OK) < 0 )
    {
        cfg_error( cfg, "validate extobject: %s: %s", f, strerror(errno) );
        return -1;
    }

#ifdef WITH_SU_CMD
    if( !cfg_getstr( sec, "user" ) ) // XXX getpwnam to verify user - but only if set!
    {
        cfg_error( cfg, "validate extobject: missing user entry" );
        return -1;
    }
#endif

    if( !cfg_getint( sec, "sub-oid" ) )
    {
        cfg_error( cfg, "validate extobject: missing or invalid sub-oid entry" );
        return -1;
    }

    return 0;
}

int
cb_validate_statgrab_opts( cfg_t *cfg, cfg_opt_t *opt )
{
    unsigned int n;
    /* only validate the last statgrab conf */
    cfg_t *sec = cfg_opt_getnsec( opt, cfg_opt_size(opt) - 1 );
    if( !sec )
    {
        cfg_error( cfg, "validate statgrab-conf: section is NULL?!" );
        return -1;
    }

    n = cfg_size( sec, "valid-filesystems" );
    if( ( 0 == n ) || ( ( 1 == n ) && ( 0 == strcmp( "!", cfg_getnstr( sec, "valid-filesystems", 0 ) ) ) ) )
    {
        cfg_error( cfg, "validate statgrab-conf: valid-filesystems must not be empty" );
        return -1;
    }
    // if( 0 != n )
    else
    {
        unsigned int i;

        for( i = 0; i < n; ++i )
        {
            // unsigned int j;

            char *sn = cfg_getnstr( sec, "valid-filesystems", i );
            if( !sn )
            {
                cfg_error( cfg, "validate statgrab-conf: filesystem type must not be empty" );
                return -1;
            }

            /*
            // first entry can be negator
            if( 0 == i && 0 == strcmp( sn, "!" ) )
                continue;

            j = strlen(sn);
            while( j-- > 0 )
            {
                if( !isalphanum(sn[j]) )
                {
                }
            }
            */
        }
    }

    return 0;
}

int
cb_validate_usm_user( cfg_t *cfg, cfg_opt_t *opt )
{
    /* only validate the last user */
    cfg_t *sec = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);
    long int proto;
    char *key;
    if( !sec )
    {
        cfg_error( cfg, "validate user: section is NULL?!" );
        return -1;
    }

    proto = cfg_getint( sec, "auth-proto" );
    key = cfg_getstr( sec, "auth-key" );
    if( proto != SNMP_AUTHPROTOCOL_NONE )
    {
        if( ( key == 0 ) || ( strlen( key ) == 0 ) )
        {
            cfg_error( cfg, "validate user: auth-key must be set when auth-proto != none" );
            return -1;
        }
    }
    else
    {
        if( key != 0 )
        {
            cfg_error( cfg, "validate user: auth-key must not be set when auth-proto == none" );
            return -1;
        }
    }

    proto = cfg_getint( sec, "priv-proto" );
    key = cfg_getstr( sec, "priv-key" );
    if( proto != SNMP_PRIVPROTOCOL_NONE )
    {
        if( ( key == 0 ) || ( strlen( key ) == 0 ) )
        {
            cfg_error( cfg, "validate user: priv-key must be set when priv-proto != none" );
            return -1;
        }
    }
    else
    {
        if( key != 0 )
        {
            cfg_error( cfg, "validate user: priv-key must not be set when priv-proto == none" );
            return -1;
        }
    }

    return 0;
}

int
cb_validate_vacm_group( cfg_t *cfg, cfg_opt_t *opt )
{
    /* only validate the last group */
    const char *title;
    unsigned int i, n;
    cfg_t *sec = cfg_opt_getnsec( opt, cfg_opt_size(opt) - 1 );
    if( !sec )
    {
        cfg_error( cfg, "validate group: section is NULL?!" );
        return -1;
    }

    title = cfg_title( sec );
    if( !title )
    {
        cfg_error( cfg, "validate group: missing title" );
        return -1;
    }

    if( ( strcasecmp( title, "v1group" ) == 0 ) || ( strcasecmp( title, "v2group" ) == 0 ) )
    {
        cfg_error( cfg, "validate group: '%s' is a reserved group name", title );
        return -1;
    }

    n = cfg_size( sec, "security-name" );
    if( 0 == n )
    {
        cfg_error( cfg, "validate group: must add at least one security name" );
        return -1;
    }

    for( i = 0; i < n; ++i )
    {
        char *sn = cfg_getnstr( sec, "security-name", i );
        if( ( sn == 0 ) || ( strlen( sn ) == 0 ) )
        {
            cfg_error( cfg, "validate group: security name must not be empty" );
            return -1;
        }
    }

    return 0;
}

int
cb_validate_vacm_view( cfg_t *cfg, cfg_opt_t *opt )
{
    /* only validate the last view */
    const char *title;
    cfg_t *sec = cfg_opt_getnsec( opt, cfg_opt_size(opt) - 1 );
    if( !sec )
    {
        cfg_error( cfg, "validate view: section is NULL?!" );
        return -1;
    }

    title = cfg_title( sec );
    if( !title )
    {
        cfg_error( cfg, "validate view: missing title" );
        return -1;
    }

    if(
        ( strcasecmp( title, "v1v2ReadView" ) == 0 ) || ( strcasecmp( title, "v1v2NoReadView" ) == 0 ) ||
        ( strcasecmp( title, "v1v2WriteView" ) == 0 ) || ( strcasecmp( title, "v1v2NoWriteView" ) == 0 ) ||
        ( strcasecmp( title, "v1v2NotifyView" ) == 0 ) || ( strcasecmp( title, "v1v2NoNotifyView" ) == 0 ) ||
        ( strcasecmp( title, "restricted" ) == 0 ) || ( strcasecmp( title, "all" ) == 0 )
      )
    {
        cfg_error( cfg, "validate group: '%s' is a reserved view name", title );
        return -1;
    }

    if( !cfg_getstr( sec, "sub-tree" ) )
    {
        cfg_error( cfg, "validate view: missing sub-tree entry" );
        return -1;
    }

    return 0;
}

int
cb_validate_vacm_access( cfg_t *cfg, cfg_opt_t *opt )
{
    /* only validate the last access */
    const char *title;
    cfg_t *sec = cfg_opt_getnsec( opt, cfg_opt_size(opt) - 1 );
    if( !sec )
    {
        cfg_error( cfg, "validate access: section is NULL?!" );
        return -1;
    }

    title = cfg_title( sec );
    if( title )
    {
        cfg_error( cfg, "validate access: access sections must be untitled" );
        return -1;
    }

    char *cp = cfg_getstr( sec, "group-name" );
    if( ( cp == 0 ) || ( strlen( cp ) == 0 ) )
    {
        cfg_error( cfg, "validate view: missing group-name entry" );
        return -1;
    }

    return 0;
}

int
cb_validate_v1v2_community( cfg_t *cfg, cfg_opt_t *opt )
{
    /* only validate the last community */
    const char *title;
    cfg_t *sec = cfg_opt_getnsec( opt, cfg_opt_size(opt) - 1 );
    if( !sec )
    {
        cfg_error( cfg, "validate %s: section is NULL?!", cfg_opt_name(opt) );
        return -1;
    }

    title = cfg_title( sec );
    if( !title )
    {
        cfg_error( cfg, "validate %s: missing title (community name)", cfg_opt_name(opt) );
        return -1;
    }

    if( !cfg_size( sec, "access" ) )
    {
        cfg_error( cfg, "validate %s: missing access entry", cfg_opt_name(opt) );
        return -1;
    }

    return 0;
}

void
report_config_error( cfg_t *cfg, const char *fmt, va_list ap )
{
    char buf[1024];
    vsnprintf( buf, sizeof(buf), fmt, ap );
#ifndef _NO_LOGGING
    LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
    LOG(buf);
    if( cfg->filename )
        LOG(cfg->filename);
    if( cfg->line )
        LOG(cfg->line);
    LOG_END;
#else
    cerr << buf;
    if( cfg->filename )
    {
        cerr << " @" << cfg->filename;
        if( cfg->line )
            cerr << ":" cfg->line;
    }
    if( cfg->line )
        cerr << " line " << cfg->line;
    cerr << endl;
#endif
}

int
cb_verify_authproto( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if(strcasecmp(value, "none") == 0)
        *(long int *)result = SNMP_AUTHPROTOCOL_NONE;
    else if(strcasecmp(value, "md5") == 0)
        *(long int *)result = SNMP_AUTHPROTOCOL_HMACMD5;
    else if(strcasecmp(value, "sha") == 0)
        *(long int *)result = SNMP_AUTHPROTOCOL_HMACSHA;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

int
cb_verify_privproto( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if(strcasecmp(value, "none") == 0)
        *(long int *)result = SNMP_PRIVPROTOCOL_NONE;
    else if(strcasecmp(value, "des") == 0)
        *(long int *)result = SNMP_PRIVPROTOCOL_DES;
    else if(strcasecmp(value, "3des") == 0)
        *(long int *)result = SNMP_PRIVPROTOCOL_3DESEDE;
    else if(strcasecmp(value, "idea") == 0)
        *(long int *)result = SNMP_PRIVPROTOCOL_IDEA;
    else if((strcasecmp(value, "aes") == 0) || (strcasecmp(value, "aes128") == 0))
        *(long int *)result = SNMP_PRIVPROTOCOL_AES128;
    else if(strcasecmp(value, "aes192") == 0)
        *(long int *)result = SNMP_PRIVPROTOCOL_AES192;
    else if(strcasecmp(value, "aes256") == 0)
        *(long int *)result = SNMP_PRIVPROTOCOL_AES256;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

int
cb_verify_secmodel( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if(strcasecmp(value, "usm") == 0)
        *(long int *)result = SNMP_SECURITY_MODEL_USM;
    else if(strcasecmp(value, "v1") == 0)
        *(long int *)result = SNMP_SECURITY_MODEL_V1;
    else if(strcasecmp(value, "v2") == 0)
        *(long int *)result = SNMP_SECURITY_MODEL_V2;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

int
cb_verify_storagetype( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if(strcasecmp(value, "other") == 0)
        *(long int *)result = storageType_other;
    else if(strcasecmp(value, "volatile") == 0)
        *(long int *)result = storageType_volatile;
    else if(strcasecmp(value, "nonvolatile") == 0)
        *(long int *)result = storageType_nonVolatile;
    else if(strcasecmp(value, "permanent") == 0)
        *(long int *)result = storageType_permanent;
    else if(strcasecmp(value, "readonly") == 0)
        *(long int *)result = storageType_readOnly;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

int
cb_verify_seclevel( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if((strcasecmp(value, "none") == 0) || (strcasecmp(value, "noauth,nopriv") == 0))
        *(long int *)result = SNMP_SECURITY_LEVEL_NOAUTH_NOPRIV;
    else if(strcasecmp(value, "auth,nopriv") == 0)
        *(long int *)result = SNMP_SECURITY_LEVEL_AUTH_NOPRIV;
    else if(strcasecmp(value, "auth,priv") == 0)
        *(long int *)result = SNMP_SECURITY_LEVEL_AUTH_PRIV;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

int
cb_verify_secmatch( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if(strcasecmp(value, "exact") == 0)
        *(long int *)result = match_exact;
    else if(strcasecmp(value, "prefix") == 0)
        *(long int *)result = match_prefix;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

int
cb_verify_viewtype( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if(strcasecmp(value, "included") == 0)
        *(long int *)result = view_included;
    else if(strcasecmp(value, "excluded") == 0)
        *(long int *)result = view_excluded;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

class Oid4Parse
    : public Oidx
{
public:
    Oid4Parse()
        : Oidx()
    {}

    using Oidx::StrToOid; // make this public available
    using Oidx::operator =;
};

int
cb_verify_oid( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    Oidx oid( FindOidForMibName( value ) );
    Oid4Parse o4p;
    SmiOID dstOid;

    if( oid.len() )
        *(const char **)result = value;
    else if( o4p.StrToOid( value, value ? strlen(value) : 0, &dstOid ) )
    {
        *(const char **)result = value;
        delete [] dstOid.ptr;
    }
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

int
cb_verify_v1v2_access( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if(strcasecmp(value, "read") == 0)
        *(long int *)result = 1;
    else if(strcasecmp(value, "write") == 0)
        *(long int *)result = 2;
    else if(strcasecmp(value, "notify") == 0)
        *(long int *)result = 3;
    else if(strcasecmp(value, "all") == 0)
        *(long int *)result = 4;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

int
get_resource_id_from_name( const char *resource_name )
{
    if( strcasecmp(resource_name, "core") == 0 )
        return RLIMIT_CORE;
    else if( strcasecmp(resource_name, "cpu") == 0 )
        return RLIMIT_CPU;
    else if( strcasecmp(resource_name, "data") == 0 )
        return RLIMIT_DATA;
    else if( strcasecmp(resource_name, "filesize") == 0 )
        return RLIMIT_FSIZE;
    else if( strcasecmp(resource_name, "files") == 0 )
        return RLIMIT_NOFILE;
    else if( strcasecmp(resource_name, "stack") == 0 )
        return RLIMIT_STACK;
    else if( strcasecmp(resource_name, "mem") == 0 )
        return RLIMIT_AS;
    else
        return -1;
}

int
cb_verify_rlimit( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    int resource_id = get_resource_id_from_name(opt->name);
    unsigned long limit;

    if( resource_id < 0 )
    {
        cfg_error(cfg, "Invalid option entry %s for rlimit", opt->name);
        return -1;
    }

    if(strcasecmp(value, "soft") == 0)
        *(ResourceLimit **)result = new ResourceLimit(resource_id, ResourceSoft);
    else if(strcasecmp(value, "hard") == 0)
        *(ResourceLimit **)result = new ResourceLimit(resource_id, ResourceHard);
    else if( (strcasecmp(value, "inf") == 0) || (strcasecmp(value, "infinite") == 0) || (strcasecmp(value, "unlimited") == 0) )
    {
        *(ResourceLimit **)result = new ResourceLimit(resource_id, ResourceFixSoft, RLIM_INFINITY);
    }
    else if(1 == sscanf( value, "%lu", &limit ) )
    {
        rlim_t rlim = limit;
        *(ResourceLimit **)result = new ResourceLimit(resource_id, ResourceFixSoft, rlim);
    }
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }

    return 0;
}

void
cb_free_rlimit(void *ptr)
{
    delete static_cast<ResourceLimit *>(ptr);
}

int
cb_verify_onfatal( cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result )
{
    if(strcasecmp(value, "ignore") == 0)
        *(long int *)result = Config::onfIgnore;
    else if(strcasecmp(value, "raise") == 0)
        *(long int *)result = Config::onfRaise;
    else if(strcasecmp(value, "kill") == 0)
        *(long int *)result = Config::onfKill;
    else if(strcasecmp(value, "exit") == 0)
        *(long int *)result = Config::onfExit;
    else if(strcasecmp(value, "_exit") == 0)
        *(long int *)result = Config::onf_Exit;
    else if(strcasecmp(value, "abort") == 0)
        *(long int *)result = Config::onfAbort;
    else
    {
        cfg_error(cfg, "Invalid value for option %s: %s", opt->name, value);
        return -1;
    }
    return 0;
}

Config *Config::mInstance = 0;

Config::~Config()
{
    cfg_free(mCfg); mCfg = 0;
}

void
Config::createInstance()
{
    static Config instance;
    mInstance = &instance;
}

static void
ReadResourceLimits( cfg_t *sec, ResourceLimits &resourceLimits )
{
    ResourceLimit *rlptr;

    rlptr = static_cast<ResourceLimit *>( cfg_getptr( sec, "core" ) );
    if( rlptr )
    {
        resourceLimits.set_core_limit( *rlptr );
    }

    rlptr = static_cast<ResourceLimit *>( cfg_getptr( sec, "cpu" ) );
    if( rlptr )
    {
        resourceLimits.set_cpu_limit( *rlptr );
    }

    rlptr = static_cast<ResourceLimit *>( cfg_getptr( sec, "data" ) );
    if( rlptr )
    {
        resourceLimits.set_data_limit( *rlptr );
    }

    rlptr = static_cast<ResourceLimit *>( cfg_getptr( sec, "filesize" ) );
    if( rlptr )
    {
        resourceLimits.set_fsize_limit( *rlptr );
    }

    rlptr = static_cast<ResourceLimit *>( cfg_getptr( sec, "files" ) );
    if( rlptr )
    {
        resourceLimits.set_nofile_limit( *rlptr );
    }

    rlptr = static_cast<ResourceLimit *>( cfg_getptr( sec, "stack" ) );
    if( rlptr )
    {
        resourceLimits.set_stack_limit( *rlptr );
    }

    rlptr = static_cast<ResourceLimit *>( cfg_getptr( sec, "mem" ) );
    if( rlptr )
    {
        resourceLimits.set_as_limit( *rlptr );
    }
}

int
Config::Read()
{
    unsigned int i, n;
    cfg_t *cfg;
    int ret;
    char *fn;
    Options const &options = Options::getInstance();

    const struct cfg_opt_t opts[] = {
        CFG_BOOL("daemonize", cfg_true, CFGF_NONE),
        CFG_INT("port", 0, CFGF_NONE),
        CFG_STR("listen-on", "", CFGF_NONE),
        CFG_STR("status-file", 0, CFGF_NONE),
        CFG_STR("pid-file", 0, CFGF_NONE),
#ifndef _NO_LOGGING
#ifdef WITH_LIBLOG4CPLUS
        CFG_STR("log4cplus-property-file", 0, CFGF_NONE),
#else
        CFG_STR("log-file", 0, CFGF_NONE),
#endif
#ifdef WITH_LOG_PROFILES
        CFG_STR("log-profile", "individual", CFGF_NONE),
#endif
        CFG_SEC("log-class", log_class_opts, CFGF_NONE),
#endif
#ifdef WITH_SU_CMD
        CFG_STR("su-cmd", SU_CMD, CFGF_NONE),
        CFG_STR_LIST("su-args", NULL, CFGF_NONE),
#endif
#ifdef AGENTPP_USE_THREAD_POOL
        CFG_INT("job-threads", 16, CFGF_NONE),
#endif
        CFG_SEC("rlimits", resource_opts, CFGF_NONE),
        CFG_INT_CB("on-fatal", onfKill, CFGF_NONE, &cb_verify_onfatal),
        CFG_SEC("mibobject", mibobject_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_SEC("inrobject", inrmibobject_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_SEC("extobject", extmibobject_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_SEC("statgrab", statgrab_opts, CFGF_NONE),
        CFG_SEC("user", usm_entry_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_SEC("group", vacm_group_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_SEC("view", vacm_view_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_SEC("access", vacm_access_opts, CFGF_MULTI),
        CFG_SEC("v1-community", vacm_v1v2_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_SEC("v2-community", vacm_v1v2_opts, CFGF_MULTI | CFGF_TITLE),
        CFG_END()
    };

    cfg = cfg_init( opts, CFGF_NOCASE );

    cfg_set_error_function( cfg, &report_config_error );

    cfg_set_validate_func( cfg, "log-class", &cb_validate_log_class );
    cfg_set_validate_func( cfg, "mibobject", &cb_validate_mibobject );
    cfg_set_validate_func( cfg, "inrobject", &cb_validate_inrmibobject );
    cfg_set_validate_func( cfg, "extobject", &cb_validate_extmibobject );
    cfg_set_validate_func( cfg, "user", &cb_validate_usm_user );
    cfg_set_validate_func( cfg, "group", &cb_validate_vacm_group );
    cfg_set_validate_func( cfg, "view", &cb_validate_vacm_view );
    cfg_set_validate_func( cfg, "access", &cb_validate_vacm_access );
    cfg_set_validate_func( cfg, "v1-community", &cb_validate_v1v2_community );
    cfg_set_validate_func( cfg, "v2-community", &cb_validate_v1v2_community );

    ret = cfg_parse(cfg, options.getConfigFile().c_str() );
    if( CFG_FILE_ERROR  == ret )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 5);
        LOG(string(string("Error parsing '") + options.getConfigFile() + "': " + strerror(errno)).c_str());
        LOG_END;

        return -1;
    }
    else if( ( CFG_PARSE_ERROR == ret ) || ( validate_root_cfg( cfg ) < 0 ) )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 5);
        LOG(string(string("Error parsing '") + options.getConfigFile() + "': parse error").c_str());
        LOG_END;

        return -2;
    }

    mDaemonize = cfg_getbool( cfg, "daemonize" ) == cfg_true;
    if( 0 == ( mPort = cfg_getint( cfg, "port" ) ) )
    {
        mPort = 161;
    }
    mListenOn = cfg_getstr( cfg, "listen-on" );
    fn = cfg_getstr( cfg, "status-file" );
    if( fn )
        mStatusFile = fn;
    fn = cfg_getstr( cfg, "pid-file" );
    if( fn )
        mPidFile = fn;

#ifdef WITH_SU_CMD
    // XXX su-cmd, su-args
    mSuCmd = cfg_getstr( cfg, "su-cmd" );
    mSuArgs.clear();
    n = cfg_size( cfg, "su-args" );
    if( 0 != n )
    {
        for( i = 0; i < n; ++i )
        {
            string suarg( cfg_getnstr( cfg, "su-args", i ) );
            mSuArgs.push_back( suarg );
        }
    }
    else
    {
        const char *su_args[] = SU_ARGS;
        for( i = 0; i < lengthof(su_args); ++i )
        {
            mSuArgs.push_back( su_args[i] );
        }
    }
#endif

#ifndef _NO_LOGGING
#ifdef WITH_LIBLOG4CPLUS
    fn = cfg_getstr( cfg, "log4cplus-property-file" );
    if( fn )
        mLogPropertyFile = fn;
#else
    fn = cfg_getstr( cfg, "log-file" );
    if( fn )
        mLogFile = fn;
#endif

#ifdef WITH_LOG_PROFILES
    mLogProfile = cfg_getstr( cfg, "log-profile" );
#endif

    {
        cfg_t *sec = cfg_getsec( cfg, "log-class" );

        for( size_t i = 0; i < lengthof(loglevels); ++i )
        {
            long int level = cfg_getint( sec, loglevels[i] );
            mLogConfig[loglevels[i]] = -1 == level ? 0xff : level;
        }
    }
#endif

#ifdef AGENTPP_USE_THREAD_POOL
    mNumberOfJobThreads = cfg_getint( cfg, "job-threads" );
#endif

    {
        cfg_t *sec = cfg_getsec( cfg, "rlimits" );
        if( sec )
            ReadResourceLimits( sec, mDaemonResourceLimits );
    }

    mOnFatalError = (OnFatalError)(cfg_getint( cfg, "on-fatal" ));

    mMibObjectConfigs.clear();

    n = cfg_size( cfg, "mibobject" );
    for( i = 0; i < n; ++i )
    {
        MibObjectConfig mibObjCfg;
        cfg_t *cfge = cfg_getnsec( cfg, "mibobject", i );

        Oidx tmpoid(cfg_title( cfge ));
        if( 0 == tmpoid.len() )
        {
            tmpoid = FindOidForMibName(cfg_title( cfge ));
        }
        string mibOid = tmpoid.get_printable();

        mibObjCfg.AsyncUpdate = cfg_getbool( cfge, "async-update" );
        mibObjCfg.MibEnabled = cfg_getbool( cfge, "mib-enabled" );
        if( !mibObjCfg.MibEnabled )
            mibObjCfg.AsyncUpdate = false;
        mibObjCfg.CacheTime = cfg_getint( cfge, "cache-timeout" );
        map<string, MibObjectConfig>::iterator iter = mMibObjectConfigs.find(mibOid);
        if( iter != mMibObjectConfigs.end() )
            iter->second = mibObjCfg;
        else
            mMibObjectConfigs[mibOid] = mibObjCfg;
    }

    n = cfg_size( cfg, "inrobject" );
    for( i = 0; i < n; ++i )
    {
        MibObjectConfig mibObjCfg;
        cfg_t *cfge = cfg_getnsec( cfg, "inrobject", i );

        Oidx tmpoid(cfg_title( cfge ));
        if( 0 == tmpoid.len() )
        {
            tmpoid = FindOidForMibName(cfg_title( cfge ));
        }
        string mibOid = tmpoid.get_printable();

        mibObjCfg.AsyncUpdate = cfg_getbool( cfge, "async-update" );
        mibObjCfg.MibEnabled = cfg_getbool( cfge, "mib-enabled" );
        if( !mibObjCfg.MibEnabled )
            mibObjCfg.AsyncUpdate = false;
        mibObjCfg.CacheTime = cfg_getint( cfge, "cache-timeout" );
        mibObjCfg.MostRecentIntervalTime = cfg_getint( cfge, "mr-interval" );
        map<string, MibObjectConfig>::iterator iter = mMibObjectConfigs.find(mibOid);
        if( iter != mMibObjectConfigs.end() )
            iter->second = mibObjCfg;
        else
            mMibObjectConfigs[mibOid] = mibObjCfg;
    }

    n = cfg_size( cfg, "extobject" );
    for( i = 0; i < n; ++i )
    {
        MibObjectConfig extMibObjCfg;
        cfg_t *cfge = cfg_getnsec( cfg, "extobject", i );
        string mibOid;

        Oidx tmpoid(cfg_title( cfge ));
        extMibObjCfg.SubOid = cfg_getint( cfge, "sub-oid" );
        if( 0 == tmpoid.len() )
        {
            tmpoid = FindOidForMibName(cfg_title( cfge ));
            if( /* still */ 0 == tmpoid.len() )
            {
                stringstream ss;
                mibOid = SM_EXTERNAL_COMMANDS;
                mibOid += ".";
                ss << extMibObjCfg.SubOid;
                mibOid += ss.str();
                tmpoid = mibOid.c_str();
            }
        }

        if( mibOid.empty() )
            mibOid = tmpoid.get_printable();

        extMibObjCfg.AsyncUpdate = cfg_getbool( cfge, "async-update" );
        extMibObjCfg.MibEnabled = cfg_getbool( cfge, "mib-enabled" );
        if( !extMibObjCfg.MibEnabled )
            extMibObjCfg.AsyncUpdate = false;
        extMibObjCfg.CacheTime = cfg_getint( cfge, "cache-timeout" );

        extMibObjCfg.ExternalCommand.Executable = cfg_getstr( cfge, "command" );

        extMibObjCfg.ExternalCommand.Arguments.clear();
        int k = cfg_size( cfge, "args" );
        for( int j = 0; j < k; ++j )
        {
            string arg( cfg_getnstr( cfge, "args", j ) );
            extMibObjCfg.ExternalCommand.Arguments.push_back( arg );
        }

#ifdef WITH_SU_CMD
        extMibObjCfg.ExternalCommand.User = cfg_getstr( cfge, "user" );
#endif

        {
            cfg_t *sec = cfg_getsec( cfge, "rlimits" );
            if( sec )
                ReadResourceLimits( sec, extMibObjCfg.ExternalCommand.ResourceLimits );
        }

        map<string, MibObjectConfig>::iterator iter = mMibObjectConfigs.find(mibOid);
        if( iter != mMibObjectConfigs.end() )
            iter->second = extMibObjCfg;
        else
            mMibObjectConfigs[mibOid] = extMibObjCfg;
    }

    map<string, Oidx> const &mapByName = getMapByName();
    for( map<string, Oidx>::const_iterator iter = mapByName.begin();
         iter != mapByName.end();
         ++iter )
    {
        if( !iter->second.in_subtree_of(SM_MIB_OBJECTS) )
            continue;
        const char * const po = iter->second.get_printable();
        map<string, MibObjectConfig>::const_iterator exists = mMibObjectConfigs.find(po);
        if( exists == mMibObjectConfigs.end() )
        {
            MibObjectConfig stdMibObjCfg;
            mMibObjectConfigs[po] = stdMibObjCfg;
        }
    }

    mStatgrabSettings.ValidFilesystems.clear();
    mStatgrabSettings.RemoveFilesystems = false;

    {
        cfg_t *sec = cfg_getsec( cfg, "statgrab" );

        n = cfg_size( sec, "valid-filesystems" );
        if( 0 != n )
        {
            for( i = 0; i < n; ++i )
            {
                char *sn = cfg_getnstr( sec, "valid-filesystems", i );
                if( 0 == i && 0 == strcmp( sn, "!" ) )
                {
                    mStatgrabSettings.RemoveFilesystems = true;
                }
                else
                {
                    mStatgrabSettings.ValidFilesystems.push_back(sn);
                }
            }
        }
    }

    mUsmEntries.clear();
    n = cfg_size( cfg, "user" );
    for( i = 0; i < n; ++i )
    {
        UsmEntry usmEntry;
        cfg_t *cfge = cfg_getnsec( cfg, "user", i );
        usmEntry.Username = cfg_title( cfge );
        usmEntry.AuthProto = cfg_getint( cfge, "auth-proto" );
        usmEntry.AuthKey = cfg_getstr( cfge, "auth-key" );
        usmEntry.PrivProto = cfg_getint( cfge, "priv-proto" );
        usmEntry.PrivKey = cfg_getstr( cfge, "priv-key" );
        mUsmEntries.push_back( usmEntry );
    }

    mVacmGroupEntries.clear();
    n = cfg_size( cfg, "group" );
    for( i = 0; i < n; ++i )
    {
        unsigned int j, m;
        VacmGroupEntry groupEntry;
        cfg_t *cfge = cfg_getnsec( cfg, "group", i );
        groupEntry.GroupName = cfg_title( cfge );

        m = cfg_size( cfge, "security-name" );
        for( j = 0; j < m; ++j )
        {
            groupEntry.SecurityNames.push_back( cfg_getnstr( cfge, "security-name", j ) );
        }

        groupEntry.SecurityModel = cfg_getint( cfge, "security-model" );
        groupEntry.StorageType = cfg_getint( cfge, "storage-type" );
        mVacmGroupEntries.push_back( groupEntry );
    }

    mVacmViewEntries.clear();
    n = cfg_size( cfg, "view" );
    for( i = 0; i < n; ++i )
    {
        VacmViewEntry viewEntry;
        cfg_t *cfge = cfg_getnsec( cfg, "view", i );
        viewEntry.ViewName = cfg_title( cfge );
        viewEntry.SubTree = cfg_getstr( cfge, "sub-tree" );
        char *cp = cfg_getstr( cfge, "mask" );
        viewEntry.Mask = cp ? cp : "";
        viewEntry.ViewType = cfg_getint( cfge, "view-type" );
        viewEntry.StorageType = cfg_getint( cfge, "storage-type" );
        mVacmViewEntries.push_back( viewEntry );
    }

    mVacmContextEntries.clear();
    n = cfg_size( cfg, "access" );
    for( i = 0; i < n; ++i )
    {
        VacmAccessEntry accessEntry;
        cfg_t *cfge = cfg_getnsec( cfg, "access", i );
        char *cp;

        cp = cfg_getstr( cfge, "group-name" );
        accessEntry.GroupName = cp ? cp : "";
        cp = cfg_getstr( cfge, "context" );
        accessEntry.Context = cp ? cp : "";
        accessEntry.SecurityModel = cfg_getint( cfge, "security-model" );
        accessEntry.SecurityLevel = cfg_getint( cfge, "security-level" );
        accessEntry.Match = cfg_getint( cfge, "match" );
        cp = cfg_getstr( cfge, "read-view" );
        accessEntry.ReadView = cp ? cp : "";
        cp = cfg_getstr( cfge, "write-view" );
        accessEntry.WriteView = cp ? cp : "";
        cp = cfg_getstr( cfge, "notify-view" );
        accessEntry.NotifyView = cp ? cp : "";
        accessEntry.StorageType = cfg_getint( cfge, "storage-type" );
        mVacmAccessEntries.push_back( accessEntry );
        mVacmContextEntries[accessEntry.Context] = true;
    }

    mVacmV1V2ShortCutEntries.clear();
    n = cfg_size( cfg, "v1-community" );
    for( i = 0; i < n; ++i )
    {
        unsigned int j, m;
        VacmV1V2ShortCutEntry v1v2Entry;
        cfg_t *cfge = cfg_getnsec( cfg, "v1-community", i );
        v1v2Entry.CommunityName = cfg_title( cfge );
        v1v2Entry.V1 = true;
        v1v2Entry.Access = v1v2_community_no_access;

        m = cfg_size( cfge, "access" );
        for( j = 0; j < m; ++j )
        {
            long int perm = cfg_getnint( cfge, "access", j );
            switch( perm )
            {
            case 1:
                v1v2Entry.Access |= v1v2_community_read_access;
                break;
            case 2:
                v1v2Entry.Access |= v1v2_community_write_access;
                break;
            case 3:
                v1v2Entry.Access |= v1v2_community_notify_access;
                break;
            case 4:
                if( j != 0 || m != 1 )
                {
                    LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
                    LOG(string(string("Invalid access combination for v1 community '")
                        + v1v2Entry.CommunityName
                        + "': "
                        + "full must not be combined with other access types").c_str());
                    LOG_END;

                    return -2;
                }
                v1v2Entry.Access |= v1v2_community_full_access;
                break;
            default:
                LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
                LOG(string(string("Invalid access value for v2 community '")
                    + v1v2Entry.CommunityName
                    + "': "
                    + "internal error - please report").c_str());
                LOG_END;

                return -2;
            }
        }
        mVacmV1V2ShortCutEntries.push_back( v1v2Entry );
    }

    n = cfg_size( cfg, "v2-community" );
    for( i = 0; i < n; ++i )
    {
        unsigned int j, m;
        VacmV1V2ShortCutEntry v1v2Entry;
        cfg_t *cfge = cfg_getnsec( cfg, "v2-community", i );
        v1v2Entry.CommunityName = cfg_title( cfge );
        v1v2Entry.V1 = false;
        v1v2Entry.Access = v1v2_community_no_access;

        m = cfg_size( cfge, "access" );
        for( j = 0; j < m; ++j )
        {
            long int perm = cfg_getnint( cfge, "access", j );
            switch( perm )
            {
            case 1:
                v1v2Entry.Access |= v1v2_community_read_access;
                break;
            case 2:
                v1v2Entry.Access |= v1v2_community_write_access;
                break;
            case 3:
                v1v2Entry.Access |= v1v2_community_notify_access;
                break;
            case 4:
                if( j != 0 || m != 1 )
                {
                    LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
                    LOG(string(string("Invalid access combination for v2 community '")
                        + v1v2Entry.CommunityName
                        + "': "
                        + "full must not be combined with other access types").c_str());
                    LOG_END;

                    return -2;
                }
                v1v2Entry.Access |= v1v2_community_full_access;
                break;
            default:
                LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
                LOG(string(string("Invalid access value for v2 community '")
                    + v1v2Entry.CommunityName
                    + "': "
                    + "internal error - please report").c_str());
                LOG_END;

                return -2;
            }
        }
        mVacmV1V2ShortCutEntries.push_back( v1v2Entry );
    }

    cfg_t *oldcfg = mCfg;
    mCfg = cfg;
    delete oldcfg;

    return 0;
}

string const Config::mEmptyString = "";
string const Config::mDefaultStatusFile = DEFAULT_STATUS_FILE;
#ifndef _NO_LOGGING
#ifdef WITH_LIBLOG4CPLUS
string const Config::mDefaultLogPropertyFile = DEFAULT_LOG_PROPERTY_FILE;
#else
string const Config::mDefaultLogFile = DEFAULT_LOG_FILE;
#endif
#ifdef WITH_LOG_PROFILES
string const Config::mIndividualLogClass = "individual";
#endif
#endif
string const Config::mDefaultPidFile = DEFAULT_PID_FILE;

static const MibObjectConfig DefaultConstMibObjectConfig;
static MibObjectConfig DefaultMibObjectConfig;

MibObjectConfig &
Config::getMibObjectConfig(string const &mibOid)
{
    map<string, MibObjectConfig>::iterator iter = mMibObjectConfigs.find(mibOid);
    if( iter != mMibObjectConfigs.end() )
        return iter->second;

    LOG_BEGIN(loggerModuleName, ERROR_LOG | 5);
    LOG("Didn't find object for (oid)");
    LOG(mibOid.c_str());
    LOG_END;

    return DefaultMibObjectConfig;
}

MibObjectConfig const &
Config::getMibObjectConfig(string const &mibOid) const
{
    map<string, MibObjectConfig>::const_iterator iter = mMibObjectConfigs.find(mibOid);
    if( iter != mMibObjectConfigs.end() )
        return iter->second;

    LOG_BEGIN(loggerModuleName, ERROR_LOG | 5);
    LOG("Didn't find object for (oid)");
    LOG(mibOid.c_str());
    LOG_END;

    return DefaultConstMibObjectConfig;
}

}
