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

#if !defined(HAVE_STRCASECMP) && !defined(HAVE_STRICMP)
int
strcasecmp(const char *s1, const char *s2)
{
    int rc;

    if( NULL == s1 && NULL == s2 )
        return 0;
    if( NULL != s1 && NULL == s2 )
        return 1;
    if( NULL == s1 && NULL != s2 )
        return -1;

    while( *s1 && *s2 )
    {
        int c1 = *s1, c2 = *s2;
        if( c1 >= 'A' && c1 <= 'Z' )
            c1 += 'a' - 'A';
        if( c2 >= 'A' && c1 <= 'Z' )
            c2 += 'a' - 'A';
        rc = c1 - c2;
        if( rc != 0 )
            return rc;
    }

    return *s1 - *s2;
}
#endif

#if !defined(HAVE_FLOCK) && defined(HAVE_FCNTL) && defined(HAVE_DECL_F_SETLK)
int
flock(int fd, int op)
{
    int try_lock = (0 != (op & LOCK_NB));
    struct flock fl;

    op &= ~LOCK_NB;
    switch(op)
    {
    case LOCK_SH:
        fl.l_type = F_RDLCK;
        break;

    case LOCK_EX:
        fl.l_type = F_WRLCK;
        break;

    case LOCK_UN:
        fl.l_type = F_UNLCK;
        break;

    default:
        errno = EINVAL;
        return -1;
    }

    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_whence = SEEK_SET;

    return fcntl( fd, try_lock ? F_SETLK : F_SETLKW, &fl );
}
#endif
