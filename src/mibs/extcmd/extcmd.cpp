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

#include <smart-snmpd/mibs/extcmd/extcmd.h>
#include <smart-snmpd/log.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.extcmd";

ExternalCommand::~ExternalCommand()
{
    if( mChildPid >= 0 )
        kill(SIGTERM);
    finish();
}

int
ExternalCommand::start()
{
    if( mChildPid >= 0 )
        return EBADF;

    int out[2], err[2];
    if( pipe(out) < 0 )
    {
        int errno_code = errno;
        LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 );
        LOG("ExternalCommand::start() (command) (stdout pipe error)");
        LOG(mExecutable.c_str());
        LOG(errno); // strerror isn't thread safe over all platforms - borrow new code from libstatgrab here for text error
        LOG_END;
        return errno_code;
    }
    if( pipe(err) < 0 )
    {
        int errno_code = errno;
        LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 );
        LOG("ExternalCommand::start() (command) (stderr pipe error)");
        LOG(mExecutable.c_str());
        LOG(errno); // strerror isn't thread safe over all platforms - borrow new code from libstatgrab here for text error
        LOG_END;
        return errno_code;
    }

    mChildPid = fork();
    if( mChildPid < 0 )
    {
        int errno_code = errno;
        LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 );
        LOG("ExternalCommand::start() (command) (fork error)");
        LOG(mExecutable.c_str());
        LOG(errno_code); // strerror isn't thread safe over all platforms - borrow new code from libstatgrab here for text error
        LOG_END;
        close( out[0] ); close( out[1] );
        close( err[0] ); close( err[1] );
        return errno_code;
    }
    else if( 0 == mChildPid )
    {
        /* XXX Unix only */
        int maxfd = sysconf(_SC_OPEN_MAX);
        if( maxfd <= 0 )
#if defined(OPEN_MAX)
            maxfd = OPEN_MAX;
#elif defined(_POSIX_OPEN_MAX)
            maxfd = _POSIX_OPEN_MAX;
#else
            maxfd = 16;
#endif

        for( ResourceLimits::const_iterator ci = mResourceLimits.begin(); ci != mResourceLimits.end(); ++ci )
        {
            ci->commit();
        }

        // child
        close( out[0] ); close( err[0] ); // close write ends of pipes
        if( dup2( out[1], STDOUT_FILENO ) != STDOUT_FILENO )
        {
            int errno_code = errno;
            LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 );
            LOG("ExternalCommand::start() (command) (dup2 stdout)");
            LOG(mExecutable.c_str());
            LOG(errno); // strerror isn't thread safe over all platforms - borrow new code from libstatgrab here for text error
            LOG_END;
            exit( errno_code );
        }
        if( dup2( err[1], STDERR_FILENO ) != STDERR_FILENO )
        {
            int errno_code = errno;
            LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 );
            LOG("ExternalCommand::start() (command) (dup2 stderr)");
            LOG(mExecutable.c_str());
            LOG(errno); // strerror isn't thread safe over all platforms - borrow new code from libstatgrab here for text error
            LOG_END;
            exit( errno_code );
        }

        for( int fd = STDERR_FILENO + 1; fd < maxfd; ++fd )
        {
            int flags = fcntl( fd, F_GETFD );
            if( flags < 0 )
                continue; // probably not open (EBADF) ...
            flags |= FD_CLOEXEC;
            if( fcntl( fd, F_SETFD, flags ) < 0 )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
                LOG("ExternalCommand::start() (command) (fcntl( fd, F_SETFD, +FD_CLOEXEC) error)");
                LOG(mExecutable.c_str());
                LOG(errno);
                LOG_END;
            }
        }

        const char **argv = (const char **)alloca( sizeof(char *) * (mArguments.size() + 2) );
        argv[0] = mExecutable.c_str();
        for( size_t i = 0; i < mArguments.size(); ++i )
            argv[i + 1] = mArguments[i].c_str();
        argv[mArguments.size() + 1] = NULL;

        execv( mExecutable.c_str(), (char **)argv );

        int errno_code = errno;
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
        LOG("ExternalCommand::start() (command) (execv)");
        LOG(mExecutable.c_str());
        LOG(errno_code); // strerror isn't thread safe over all platforms - borrow new code from libstatgrab here for text error
        LOG_END;

        exit( errno_code );
    }
    else
    {
        // parent
        close( out[1] ); close( err[1] ); // close read ends of pipes
        mFileno[0] = out[0];
        mFileno[1] = err[0];
        mBuffers[0].clear();
        mBuffers[1].clear();
    }

    return 0;
}

bool
ExternalCommand::poll(unsigned seconds)
{
    if( (mChildPid == -1) && (mFileno[0] == -1) && (mFileno[1] == -1) )
        return false;

    struct pollfd fds[2] = { { mFileno[0], POLLIN, 0 }, { mFileno[1], POLLIN, 0 } };

    int ready = ::poll( fds, 2, seconds * 1000 );
    if( ready )
    {
        for( unsigned i = 0; i < lengthof(fds); ++i )
        {
            if( ( fds[i].revents & POLLIN ) != 0 )
            {
                char buf[4096];
                ssize_t bytes;
                bytes = read( mFileno[i], buf, sizeof(buf) );
                if( 0 != bytes )
                {
                    string s( buf, bytes );
                    mBuffers[i].append(s);
                }
                else
                {
                    close(mFileno[i]);
                    mFileno[i] = -1;
                }
            }
        }
    }

    if( mChildPid > 0 )
    {
        int status;
        pid_t pid = waitpid( mChildPid, &status, WNOHANG );
        if( pid > 0 )
        {
            // status
            if( WIFEXITED(status) )
            {
                mExitCode = WEXITSTATUS(status);
                mChildPid = -1;
            }
            else if( WIFSIGNALED(status) )
            {
                mExitSignal = WTERMSIG(status);
                mChildPid = -1;
            }
        }

        // read left data from pipes
        for( unsigned i = 0; i < lengthof(mFileno); ++i )
        {
            while( mFileno[i] > 0 )
            {
                char buf[4096];
                ssize_t bytes;
                bytes = read( mFileno[i], buf, sizeof(buf) );
                if( 0 != bytes )
                {
                    string s( buf, bytes );
                    mBuffers[i].append(s);
                }
                else
                {
                    close(mFileno[i]);
                    mFileno[i] = -1;
                }
            }
        }
    }

    return (mChildPid != -1) || (mFileno[0] != -1) || (mFileno[1] != -1);
}

int
ExternalCommand::kill(int signal)
{
    if( mChildPid <= 0 )
        return EBADF;

    return ::kill( mChildPid, signal );
}


ExternalCommandAsUser::ExternalCommandAsUser(ExternalCommandConfig const &cmdcfg)
    : ExternalCommand(cmdcfg)
    , mUser(cmdcfg.User)
{
}

}
