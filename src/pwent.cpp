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

#include <smart-snmpd/pwent.h>
#include <smart-snmpd/log.h>

#if defined(WITH_USER_LOOKUP) && (WITH_USER_LOOKUP != 0)
# if defined(WITH_PAM_USER_LOOKUP) || defined(WITH_FILE_USER_LOOKUP)
#  include <pwd.h>
#  include <grp.h>
# endif

# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
static pthread_mutex_t pw_gr_serializer;
# endif

// decision: don't try to intercept other libraries - create an option
// protect library for those purposes
# if 0
# if defined(WITH_PAM_USER_LOOKUP)
extern "C"
{

/**
 * following replacement to *pwent and *grent, respectively, are made to
 * protect the smart-snmpd against mt-unsafe pam modules as seen in
 * wildlife on AIX 5.3 and above
 */

#  if defined(HAVE_DLSYM_RTLD_NEXT)
#   include <dlfcn.h>
typedef void (*setendfn_t)(void);

static setendfn_t real_setgrent = (setendfn_t)( dlsym(RTLD_NEXT, "setgrent") );
void setgrent(void);
static setendfn_t real_endgrent = (setendfn_t)( dlsym(RTLD_NEXT, "endgrent") );
void endgrent(void);

static setendfn_t real_setpwent = (setendfn_t)( dlsym(RTLD_NEXT, "setpwent") );
void setpwent(void);
static setendfn_t real_endpwent = (setendfn_t)( dlsym(RTLD_NEXT, "endpwent") );
void endpwent(void);
# else
void real_setgrent() { setgrent(); }
void real_endgrent() { endgrent(); }
void real_setpwent() { setpwent(); }
void real_endpwent() { endpwent(); }
#   define setgrent my_setgrent
#   define endgrent my_endgrent
#   define setpwent my_setpwent
#   define endpwent my_endpwent
#  endif

void
setgrent(void)
{
    int rc = pthread_mutex_lock( &pw_gr_serializer );
    assert( rc == 0 );
    real_setgrent();
}

void
endgrent(void)
{
    // before we release resources we shall ensure that we own the mutex (must be busy and lock must return dead lock condition)
    // 2nd: Some Linux distributions seems to return EDEADLK on trylock, too - which breaks XPG6
    int rc = pthread_mutex_trylock( &pw_gr_serializer );
    assert( ( rc == EDEADLK ) || ( ( rc == EBUSY ) && ( pthread_mutex_lock( &pw_gr_serializer ) == EDEADLK ) ) );
    real_endgrent();
    pthread_mutex_unlock( &pw_gr_serializer );
}

void
setpwent(void)
{
    int rc = pthread_mutex_lock( &pw_gr_serializer );
    assert( rc == 0 );
    real_setpwent();
}

void
endpwent(void)
{
    int rc = pthread_mutex_trylock( &pw_gr_serializer );
    assert( ( rc == EDEADLK ) || ( ( rc == EBUSY ) && ( pthread_mutex_lock( &pw_gr_serializer ) == EDEADLK ) ) );
    real_endpwent();
    pthread_mutex_unlock( &pw_gr_serializer );
}

}
/* end if defined(WITH_PAM_USER_LOOKUP) */
# endif
# endif /* if 0 */
#endif /* if defined(WITH_USER_LOOKUP) && (WITH_USER_LOOKUP != 0) */

namespace SmartSnmpd
{
static const char * const loggerModuleName = "smartsnmpd.pwent";

static string const emptyString = "";

#ifndef WIN32
# define FALLBACK_BUF_SIZE 16384

size_t
SystemUserInfo::guess_pwent_bufsize()
{
    static int bufsize = 0;
    if( bufsize <= 0 )
        bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if( bufsize <= 0 )
        bufsize = FALLBACK_BUF_SIZE;
    return bufsize;
}
#endif

#if defined(WITH_USER_LOOKUP) && (WITH_USER_LOOKUP != 0)

# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    namespace
    {
        class InitPwGrSerializer
        {
        public:
            InitPwGrSerializer()
            {
                pthread_mutexattr_t attr;
                pthread_mutexattr_init(&attr);
                pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
                pthread_mutex_init( &pw_gr_serializer, &attr );
                pthread_mutexattr_destroy(&attr);
            }

            ~InitPwGrSerializer() { pthread_mutex_destroy( &pw_gr_serializer ); }

        private:
            InitPwGrSerializer(InitPwGrSerializer const &);
            InitPwGrSerializer & operator = (InitPwGrSerializer const &);
        };

        InitPwGrSerializer initPwGrSerializer;
    }
# endif

static int
finduserbyuid(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    int rc;
# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    rc = pthread_mutex_lock( &pw_gr_serializer );
    if( rc != 0 )
        return ENOENT;
# endif

# if defined(WITH_PAM_USER_LOOKUP)
    setpwent();
#  if defined(HAVE_GETPWUID_R)
    rc = getpwuid_r( uid, pwd, buf, buflen, result );
#  elif defined(HAVE_GETPWUID)
#   error non-reentrant function support using getpwuid() currently not implemented
#  else
#   error neither getpwuid_r() nor getpwuid() available
#  endif
    endpwent();
# elif defined(WITH_FILE_USER_LOOKUP)
    FILE *fp = fopen( "/etc/passwd", "r" );
    if( 0 == fp )
    {
        rc = errno;
    }
    else
    {
#  if defined(HAVE_FGETPWENT_R)
        while( 0 == ( rc = fgetpwent_r( fp, pwd, buf, buflen, result ) ) )
        {
            if( uid == pwd->pw_uid )
                break;
        }

        if( ( rc != 0 ) && ( result != 0 ) )
            *result = 0;

#  elif defined(HAVE_FGETPWENT)
#   error non-reentrant function support using fgetpwent() currently not implemented
#  else
#   error neither fgetpwent_r() nor fgetpwent() available
#  endif
        fclose( fp );
    }
# endif

# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    pthread_mutex_unlock( &pw_gr_serializer );
# endif

    return rc;
}

static int
findgroupbygid(gid_t gid, struct group *grp, char *buf, size_t buflen, struct group **result)
{
    int rc;
# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    rc = pthread_mutex_lock( &pw_gr_serializer );
    if( rc != 0 )
        return ENOENT;
# endif

# if defined(WITH_PAM_USER_LOOKUP)
    setgrent();
#  if defined(HAVE_GETGRGID_R)
    rc = getgrgid_r( gid, grp, buf, buflen, result );
#  elif defined(HAVE_GETGRGID)
#   error non-reentrant function support using getgrgid() currently not implemented
#  else
#   error neither getgrgid_r() nor getgrgid() available
#  endif
    endgrent();
# elif defined(WITH_FILE_USER_LOOKUP)
    FILE *fp = fopen( "/etc/group", "r" );
    if( 0 == fp )
    {
        rc = errno;
    }
    else
    {
#  if defined(HAVE_FGETPWENT_R)
        while( 0 == ( rc = fgetgrent_r( fp, grp, buf, buflen, result ) ) )
        {
            if( gid == grp->gr_gid )
                break;
        }

        if( ( rc != 0 ) && ( result != 0 ) )
            *result = 0;

#  elif defined(HAVE_FGETPWENT)
#   error non-reentrant function support using fgetgrent() currently not implemented
#  else
#   error neither fgetgrent_r() nor fgetgrent() available
#  endif
        fclose( fp );
    }
# endif

# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    pthread_mutex_unlock( &pw_gr_serializer );
# endif

    return rc;
}

static int
finduserbyname(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result)
{
    int rc;
# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    rc = pthread_mutex_lock( &pw_gr_serializer );
    if( rc != 0 )
        return ENOENT;
# endif

# if defined(WITH_PAM_USER_LOOKUP)
    setpwent();
#  if defined(HAVE_GETPWNAM_R)
    rc = getpwnam_r( name, pwd, buf, buflen, result );
#  elif defined(HAVE_GETPWNAM)
#   error non-reentrant function support using getpwnam() currently not implemented
#  else
#   error neither getpwnam_r() nor getpwnam() available
#  endif
    endpwent();
# elif defined(WITH_FILE_USER_LOOKUP)
    if( name == 0 )
    {
        if( result )
            *result = 0;

        rc = ENOENT;
    }
    else
    {
        FILE *fp = fopen( "/etc/passwd", "r" );
        if( 0 == fp )
        {
            rc = errno;
        }
        else
        {
#  if defined(HAVE_FGETPWENT_R)
            while( 0 == ( rc = fgetpwent_r( fp, pwd, buf, buflen, result ) ) )
            {
                if( (pwd->pw_name != 0) && strcmp( name, pwd->pw_name ) == 0 )
                    break;
            }

            if( ( rc != 0 ) && ( result != 0 ) )
                *result = 0;

#  elif defined(HAVE_FGETPWENT)
#   error non-reentrant function support using getpwuid() currently not implemented
#  else
#   error neither fgetpwent_r() nor fgetpwent() available
#  endif
            fclose( fp );
        }
    }
# endif

# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    pthread_mutex_unlock( &pw_gr_serializer );
# endif

    return rc;
}

static int
findgroupbyname(const char *name, struct group *grp, char *buf, size_t buflen, struct group **result)
{
    int rc;
# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    rc = pthread_mutex_lock( &pw_gr_serializer );
    if( rc != 0 )
        return ENOENT;
# endif

# if defined(WITH_PAM_USER_LOOKUP)
    setgrent();
#  if defined(HAVE_GETGRGID_R)
    rc = getgrnam_r( name, grp, buf, buflen, result );
#  elif defined(HAVE_GETGRGID)
#   error non-reentrant function support using getgrgid() currently not implemented
#  else
#   error neither getgrgid_r() nor getgrgid() available
#  endif
    endgrent();
# elif defined(WITH_FILE_USER_LOOKUP)
    if( name == 0 )
    {
        if( result )
            *result = 0;

        rc = ENOENT;
    }
    else
    {
        FILE *fp = fopen( "/etc/group", "r" );
        if( 0 == fp )
        {
            rc = errno;
        }
        else
        {
#  if defined(HAVE_FGETGRENT_R)
            while( 0 == ( rc = fgetgrent_r( fp, grp, buf, buflen, result ) ) )
            {
                if( (grp->gr_name != 0) && strcmp( name, grp->gr_name ) == 0 )
                    break;
            }

            if( ( rc != 0 ) && ( result != 0 ) )
                *result = 0;

#  elif defined(HAVE_FGETGRENT)
#   error non-reentrant function support using fgetgrent() currently not implemented
#  else
#   error neither fgetgrent_r() nor fgetgrent() available
#  endif
            fclose( fp );
        }
    }
# endif

# if defined(WITH_PAM_USER_LOOKUP) || \
     ( defined(WITH_FILE_USER_LOOKUP) && \
       !( defined(HAVE_FGETGRENT_R) && defined(HAVE_FGETPWENT_R) ) )
    pthread_mutex_unlock( &pw_gr_serializer );
# endif

    return rc;
}

string const &
SystemUserInfo::getusernamebyuid(uid_t id)
{
    map<uid_t, string>::iterator iter = mPwUidCache.lower_bound( id );
    if( iter != mPwUidCache.end() && iter->first == id )
    {
        return iter->second;
    }
    else
    {
        struct passwd pwent, *result = NULL;
        memset( &pwent, 0, sizeof(pwent) );

        int rc = finduserbyuid(id, &pwent, mSysBuf.getBuffer(), mSysBuf.getBufSize(), &result);

        LOG_BEGIN( loggerModuleName, DEBUG_LOG | 0 );
        LOG( "getusernamebyuid(id): (rc)(pwent.pw_name)" );
        LOG(id);
        LOG(rc);
        LOG(to_string((void *)(pwent.pw_name)).c_str());
        LOG_END;

        if( ( 0 == rc ) && (pwent.pw_name) )
        {
            iter = mPwUidCache.insert( iter, make_pair( id, pwent.pw_name ) );
            mPwNameCache.insert( make_pair( pwent.pw_name, id ) );
        }
        else
        {
            iter = mPwUidCache.insert( iter, make_pair( id, emptyString ) );
        }

        return iter->second;
    }
}

string const &
SystemUserInfo::getgroupnamebygid(gid_t id)
{
    map<uid_t, string>::iterator iter = mGrGidCache.lower_bound( id );
    if( iter != mGrGidCache.end() && iter->first == id )
    {
        return iter->second;
    }
    else
    {
        struct group grent, *result = NULL;
        memset( &grent, 0, sizeof(grent) );

        int rc = findgroupbygid(id, &grent, mSysBuf.getBuffer(), mSysBuf.getBufSize(), &result);

        LOG_BEGIN( loggerModuleName, DEBUG_LOG | 0 );
        LOG( "getgroupnamebygid(id): (rc)(grent.gr_name)" );
        LOG(id);
        LOG(rc);
        LOG(to_string((void *)(grent.gr_name)).c_str());
        LOG_END;

        if( ( 0 == rc ) && (grent.gr_name) )
        {
            iter = mGrGidCache.insert( iter, make_pair( id, grent.gr_name ) );
            mGrNameCache.insert( make_pair( grent.gr_name, id ) );
        }
        else
        {
            iter = mGrGidCache.insert( iter, make_pair( id, emptyString ) );
        }

        return iter->second;
    }
}

uid_t
SystemUserInfo::getuseridbyname(string const &name)
{
    map<string, uid_t>::iterator iter = mPwNameCache.lower_bound( name );
    if( iter != mPwNameCache.end() && iter->first == name )
    {
        return iter->second;
    }
    else
    {
        struct passwd pwent, *result = NULL;
        memset( &pwent, 0, sizeof(pwent) );

        int rc = finduserbyname( name.c_str(), &pwent, mSysBuf.getBuffer(), mSysBuf.getBufSize(), &result );

        LOG_BEGIN( loggerModuleName, DEBUG_LOG | 0 );
        LOG( "getuseridbyname(name): (rc)(uid)" );
        LOG(name.c_str());
        LOG(rc);
        LOG(pwent.pw_uid);
        LOG_END;

        if( 0 == rc )
        {
            mPwUidCache.insert( make_pair( pwent.pw_uid, name ) );
        }
        else
        {
            pwent.pw_uid = (uid_t)-1;
        }

        iter = mPwNameCache.insert( iter, make_pair( name, pwent.pw_uid ) );

        return pwent.pw_uid;
    }
}

gid_t
SystemUserInfo::getgroupidbyname(string const &name)
{
    map<string, gid_t>::iterator iter = mGrNameCache.lower_bound( name );
    if( iter != mGrNameCache.end() && iter->first == name )
    {
        return iter->second;
    }
    else
    {
        struct group grent, *result = NULL;
        memset( &grent, 0, sizeof(grent) );

        int rc = findgroupbyname( name.c_str(), &grent, mSysBuf.getBuffer(), mSysBuf.getBufSize(), &result );

        LOG_BEGIN( loggerModuleName, DEBUG_LOG | 0 );
        LOG( "getgroupidbyname(name): (rc)(gid)" );
        LOG(name.c_str());
        LOG(rc);
        LOG(grent.gr_gid);
        LOG_END;

        if( 0 == rc )
        {
            mGrGidCache.insert( make_pair( grent.gr_gid, name ) );
        }
        else
        {
            grent.gr_gid = (uid_t)-1;
        }

        iter = mGrNameCache.insert( iter, make_pair( name, grent.gr_gid ) );

        return grent.gr_gid;
    }
}

#endif

}
