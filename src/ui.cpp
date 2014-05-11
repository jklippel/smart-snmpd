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

#include <smart-snmpd/config.h>
#include <smart-snmpd/cmndline.h>
#include <smart-snmpd/ui.h>

#include <iostream>

#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/file.h>

#ifndef _NO_LOGGING
# ifdef WITH_LIBLOG4CPLUS
using namespace log4cplus;
# endif
#endif

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.ui";

Agent * volatile runningAgent = 0;
UI * volatile runningUI = 0;

#ifndef _NO_LOGGING
# ifdef WITH_LIBLOG4CPLUS
class log4cplus_first_init
{
public:
    log4cplus_first_init()
    {
        BasicConfigurator::doConfigure();
    }
};
# endif
#endif

static const int handled_signals[] =
{
#ifdef SIGQUIT
    SIGQUIT,
#endif
#ifdef SIGHUP
    SIGHUP,
#endif
    SIGTERM,
    SIGINT
};

void
UI::sig(int signo)
{
    switch (signo)
    {
#ifdef SIGQUIT
    case SIGQUIT:
#endif
    case SIGTERM:
    case SIGINT:
        if( runningAgent && runningAgent->isRunning() )
        {
            runningAgent->Stop(signo);
            LOG_BEGIN(loggerModuleName, EVENT_LOG | 1);
            LOG("Terminated");
            LOG_END;
        }
        else
        {
            if( runningUI )
            {
                LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
                LOG("Initialization error");
                LOG_END;
                _exit(255);
            }
            else
            {
                LOG_BEGIN(loggerModuleName, EVENT_LOG | 1);
                LOG("Double termination");
                LOG_END;
                exit(255);
            }
        }

        break;

#ifdef SIGHUP
    case SIGHUP:
        if( runningUI )
        {
            runningUI->reload_config();
        }
        break;
#endif
    }
}

#ifdef HAVE_PTHREAD
void *
UI::sig_watcher(void *arg)
{
    sigset_t sigmask;

    (void)arg;

    sigemptyset(&sigmask);
    for( size_t i = 0; i < lengthof(handled_signals); ++i )
        sigaddset( &sigmask, handled_signals[i] );

    /* mask must stay blocked */

    while( runningUI )
    {
        int rc, signo = 0;

        rc = sigwait(&sigmask, &signo);
        if( rc != 0 )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG(string( string("sigwait failed: ") + strerror(errno)).c_str());
            LOG_END;

            if( runningAgent && runningAgent->isRunning() )
            {
                runningAgent->Stop(SIGTERM);
                LOG_BEGIN(loggerModuleName, EVENT_LOG | 1);
                LOG("Terminated");
                LOG_END;
            }

            return NULL;
        }

        LOG_BEGIN( loggerModuleName, EVENT_LOG | 0 );
        LOG(
          "Catched signal (signo)"
#if HAVE_STRSIGNAL
          "(signame)"
#endif
           );
        LOG( signo );
#if HAVE_STRSIGNAL
        LOG( strsignal(signo) );
#endif
        LOG_END;

        sig(signo);
    }

    return NULL;
}
#else
void
UI::init_signals()
{
#ifndef HAVE_PTHREAD
# ifdef SIGQUIT
    signal (SIGQUIT, UI::sig);
# endif
    signal (SIGTERM, UI::sig);
    signal (SIGINT, UI::sig);
# ifdef SIGHUP
    signal (SIGHUP, UI::sig); 
# endif
#endif
}
#endif

extern "C"
{
//! application defined union for semctl
union semun
{
    //! integer value to use for SETVAL command
    int             val;
    //! semid description pointer for IPC_STAT/IPC_GET commands
    struct semid_ds *buf;
    //! list of values for SETALL/GETALL commands
    unsigned short  *array;
};
}

UI::UI(int argc, char **argv)
    : mPidfileDesc(-1)
    // , mStartSemaphore()
    , mSemId(-1)
#ifndef _NO_LOGGING
# ifdef WITH_LIBLOG4CPLUS
    , mConfigureAndWatchThread(0)
# else
    , mLogfileHandle(0)
# endif
#endif
{
    Options::getInstance(argc, argv);
    int rc = Config::getInstance(0).Read();
    if( -1 == rc )
    {
        exit(255);
    }
    if( -2 == rc ) // ENOENT
        exit(255);

    setup_logging(false);
}

UI::~UI()
{
#ifndef _NO_LOGGING
# ifdef WITH_LIBLOG4CPLUS
    delete mConfigureAndWatchThread;
    Logger::shutdown();

    // write now occuring events to stdout
    BasicConfigurator config;
    config.configure();
# else
    if( mLogfileHandle )
        fclose( mLogfileHandle );
# endif
#endif

    if( mPidfileDesc >= 0 )
    {
        close(mPidfileDesc);
        unlink( Config::getInstance().getPidFileName().c_str() );
    }
}

#ifndef _NO_LOGGING
int
UI::setup_logging(bool reopen)
{
    Config const &cfg = Config::getInstance();

#ifdef WITH_LIBLOG4CPLUS
    if( cfg.getLogPropertyFile().length() )
    {
        if( reopen )
        {
#if 0
            mConfigureAndWatchThread = new ConfigureAndWatchThread( Config::getInstance().getLogPropertyFile() );
#else
            PropertyConfigurator::doConfigure( Config::getInstance().getLogPropertyFile() );
#endif
        }
        else
        {
            PropertyConfigurator::doConfigure( Config::getInstance().getLogPropertyFile() );
            AgentLog4CPlus *al = new AgentLog4CPlus();
            if( !al )
            {
                // this goes to stderr using default AgentLogImpl
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG("Out of memory instantiating new AgentLogImpl");
                LOG_END;

                return -1;
            }
            if( al != DefaultLog::init_ts( al ) )
            {
                DefaultLog::init( al );
            }
        }
    }
#else
    if( reopen && cfg.getLogFile().length() )
    {
        int logfileDescriptor = open( cfg.getLogFile().c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP );
        if( -1 == logfileDescriptor )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG(string( string("Can't open log file ") + cfg.getLogFile() + ": " + strerror(errno)).c_str());
            LOG_END;

            return -1;
        }

        FILE *logfileHandle = fdopen( logfileDescriptor, "a" );
        if( !logfileHandle )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG(string( string("Can't fdopen() log-file ") + cfg.getLogFile() + ": " + strerror(errno)).c_str());
            LOG_END;

            return -1;
        }

        AgentLogImpl *al = new AgentLogImpl( logfileHandle );
        if( !al )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG("Out of memory instantiating new AgentLogImpl");
            LOG_END;

            return -1;
        }
        if( al != DefaultLog::init_ts( al ) )
        {
            DefaultLog::init( al );
        }

        if( Config::getInstance().getDaemonize() )
        {
            if( ( dup2( logfileDescriptor, STDOUT_FILENO ) == -1 ) || ( dup2( logfileDescriptor, STDERR_FILENO ) == -1 ) )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG(string( string("Can't dup log-file to stdout/stderr: ") + strerror(errno)).c_str());
                LOG_END;

                return -1;
            }

            al = new AgentLogImpl( stderr );
            if( !al )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG("Out of memory instantiating new AgentLogImpl");
                LOG_END;

                return -1;
            }
            DefaultLog::init( al );
            fclose( logfileHandle );
        }
        else
        {
            if( mLogfileHandle )
                fclose( mLogfileHandle );

            mLogfileHandle = logfileHandle;
        }
    }
#endif

#ifdef WITH_LOG_PROFILES
    if( cfg.getLogProfile() == "individual" )
    {
#endif
        DefaultLog::log ()->set_filter (ERROR_LOG, Config::getInstance().getLogLevel("error"));
        DefaultLog::log ()->set_filter (WARNING_LOG, Config::getInstance().getLogLevel("warning"));
        DefaultLog::log ()->set_filter (EVENT_LOG, Config::getInstance().getLogLevel("event"));
        DefaultLog::log ()->set_filter (INFO_LOG, Config::getInstance().getLogLevel("info"));
        DefaultLog::log ()->set_filter (DEBUG_LOG, Config::getInstance().getLogLevel("debug"));
        DefaultLog::log ()->set_filter (USER_LOG, Config::getInstance().getLogLevel("user"));
#ifdef WITH_LOG_PROFILES
    }
    else
    {
        DefaultLog::log ()->set_profile( cfg.getLogProfile().c_str() );
    }
#endif
    return 0;
}
#endif /* ?_NO_LOGGING */

int
UI::reload_config()
{
    // Config::Read should be reentrant - test one fine day
    int rc = Config::getInstance(0).Read();
    if( -2 == rc )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 5);
        LOG("Error loading config");
        LOG_END;
    }
    else
    {
        setup_logging();

        LOG_BEGIN( loggerModuleName, INFO_LOG | 9);
        LOG("Config loaded");
        LOG_END;

        if( runningAgent && runningAgent->isRunning() )
        {
            rc = runningAgent->RefreshMibConfig();
            if( 0 == rc )
            {
                LOG_BEGIN( loggerModuleName, INFO_LOG | 9);
                LOG("Mib Config updated");
                LOG_END;
            }
            else
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 5);
                LOG("Error updating Mib config");
                LOG_END;
            }
        }
    }

    return rc;
}

#ifdef _WIN32
#define DEVNULL "\\\\null"
#else
#define DEVNULL "/dev/null"
#endif

static const int ui_decoupler_sem = 0;
static const int ui_decoupler_starts = 1;
static const int ui_decoupler_fired = 2;
static const int ui_decoupler_succeeds = 3;
static const int ui_daemon_sem = 1;
static const int ui_daemon_starts = 1;
static const int ui_daemon_runs = 2;

int
UI::wait_for_child( ChildInfo &childInfo, int sleep_seconds )
{
    bool semvalok = false;
    for( ;; )
    {
        int rc;

        rc = waitpid( childInfo.child, &childInfo.procrc, WNOHANG );
        if( childInfo.child == rc )
        {
            if( ( WIFEXITED(childInfo.procrc) && ( WEXITSTATUS(childInfo.procrc) != 0 ) ) || WIFSIGNALED(childInfo.procrc) )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                string s = childInfo.who + "(" + to_string(childInfo.child) + ") dies unexpectedly with ";
                if( WIFEXITED(childInfo.procrc) )
                    s += "exit status=" + to_string(WEXITSTATUS(childInfo.procrc));
                if(WIFSIGNALED(childInfo.procrc))
                    s += "signal=" + to_string(WTERMSIG(childInfo.procrc));
                LOG(s.c_str());
                LOG_END;

                rc = -1;
            }
            else
            {
                rc = childInfo.child;
            }

            if( !semvalok && ( ( childInfo.semrc = semctl( mSemId, childInfo.semnum, GETVAL ) ) < 0 ) )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                // LOG( string( string("Can't query SysV IPC semaphore at '") + mStartSemaphore + "': " + strerror(errno) ).c_str() );
                LOG( string( string("Can't query private SysV IPC semaphore: ") + strerror(errno) ).c_str() );
                LOG_END;
                return -1;
            }

            if( childInfo.semrc != childInfo.semval )
                return -1;

            return rc;
        }
        else if( -1 == rc )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
            string s = "waitpid for " + childInfo.who + "(pid=" + to_string(childInfo.child) + "): errno=" + to_string(errno);
            LOG(s.c_str());
            LOG_END;
            return -1;
        }
        else
        {
            if( semvalok && !childInfo.wantexit )
                break;

            if( ( childInfo.semrc = semctl( mSemId, childInfo.semnum, GETVAL ) ) < 0 )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                // LOG( string( string("Can't query SysV IPC semaphore at '") + mStartSemaphore + "': " + strerror(errno) ).c_str() );
                LOG( string( string("Can't query private SysV IPC semaphore: ") + strerror(errno) ).c_str() );
                LOG_END;
                return -1;
            }

            if( childInfo.semrc == childInfo.semval )
                semvalok = true;
        }

        sleep( sleep_seconds );
    }

    return 0;
}

int
UI::create_sysv_sem()
{
#if 0
    string pidpath = Config::getInstance().getPidFileName( true );
    char *cstr = strdup( pidpath.c_str() );
    if( NULL == cstr )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        LOG( string("Can't allocate memory for SysV IPC creation").c_str() );
        LOG_END;
        return -1;
    }
    string sempath = dirname( cstr );
    free( cstr ); cstr = 0;
    if( sempath[ sempath.size() - 1 ] == '/' )
    {
        sempath += "/";
    }
    sempath += "smart-snmpd.startX";
    cstr = strdup( sempath.c_str() );
    if( NULL == cstr )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        LOG( string("Can't allocate memory for SysV IPC creation").c_str() );
        LOG_END;
        return -1;
    }
    int semfd = mkstemp( cstr );
    if( semfd < 0 )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        LOG( string( string("Can't get temporary file for SysV IPC (as '") + sempath + "'): " + strerror(errno) ).c_str() );
        LOG_END;
        return -1;
    }
    close( semfd );

    mStartSemaphore = cstr;
    free(cstr); cstr = 0;

    key_t semkey;
    semkey = ftok( mStartSemaphore.c_str(), mStartSemaphore[ mStartSemaphore.size() - 1 ] );
    if( (key_t)-1 == semkey )
    {
        unlink( mStartSemaphore.c_str() );
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        LOG( string( string("Can't generate SysV IPC key from '") + mStartSemaphore + "': " + strerror(errno) ).c_str() );
        LOG_END;
        return -1;
    }
#endif

    mSemId = semget( IPC_PRIVATE /* semkey */, 2, S_IWUSR | S_IRUSR | IPC_CREAT | IPC_EXCL );
    if( mSemId < 0 )
    {
        // unlink( mStartSemaphore.c_str() );
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        // LOG( string( string("Can't open private SysV IPC semaphore at '") + mStartSemaphore + "': " + strerror(errno) ).c_str() );
        LOG( string( string("Can't open private SysV IPC semaphore: ") + strerror(errno) ).c_str() );
        LOG_END;
        return -1;
    }

    // init semaphore counters ...
    unsigned short seminit[2] = { 0, 0 };
    union semun arg;
    arg.array = seminit;
    if( semctl( mSemId, 0, SETALL, arg ) < 0 )
    {
        int errno_setall = errno;

        cleanup_sysv_sem();

        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        // LOG( string( string("Can't initialize SysV IPC semaphore at '") + mStartSemaphore + "': " + strerror(errno_setall) ).c_str() );
        LOG( string( string("Can't initialize private SysV IPC semaphore: ") + strerror(errno_setall) ).c_str() );
        LOG_END;
        return -1;
    }

    return mSemId;
}

int
UI::cleanup_sysv_sem()
{
    int rc = 0;

    if( semctl( mSemId, 0, IPC_RMID ) < 0 )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
        // LOG( string( string("Can't remove SysV IPC semaphore at '") + mStartSemaphore + "': " + strerror(errno) ).c_str() );
        LOG( string( string("Can't remove private SysV IPC semaphore at: ") + strerror(errno) ).c_str() );
        LOG_END;
        rc = -1;
    }
    // unlink( mStartSemaphore.c_str() );

    return rc;
}

int
UI::fork_daemon()
{
    int rc = 0;
    if( -1 == create_sysv_sem() )
    {
        return -1;
    }

    switch( pid_t child = fork() )
    {
    case -1: // error
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        LOG( string( string("Can't fork: ") + strerror(errno) ).c_str() );
        LOG_END;
        cleanup_sysv_sem();
        return -1;

    case 0: // child
        {
            union semun arg;
            arg.val = ui_decoupler_starts;
            if( semctl( mSemId, ui_decoupler_sem, SETVAL, arg ) < 0 )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG( string( string("Can't set private SysV IPC semaphore ") + to_string(ui_decoupler_sem) +
                             " to " + to_string(ui_decoupler_starts) +
                             // " at '" + mStartSemaphore + "': "
                             + strerror(errno) ).c_str() );
                LOG_END;

                return -1;
            }

            setsid();
            if( ( child = fork() ) < 0 )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG(string( string("Can't fork: ") + strerror(errno)).c_str());
                LOG_END;

                return -1;
            }
            else if( child != 0 )
            {
                arg.val = ui_decoupler_fired;
                if( semctl( mSemId, ui_decoupler_sem, SETVAL, arg ) < 0 )
                {
                    LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                    LOG( string( string("Can't set private SysV IPC semaphore ") + to_string(ui_decoupler_sem) +
                                 " to " + to_string(ui_decoupler_fired) +
                                 // " at '" + mStartSemaphore + "': "
                                 + strerror(errno) ).c_str() );
                    LOG_END;
                    return -1;
                }

                ChildInfo childInfo;
                childInfo.child = child;
                childInfo.who = "Daemon";
                childInfo.semnum = ui_daemon_sem;
                childInfo.semval = ui_daemon_runs;
                childInfo.wantexit = false;

                if( 0 == wait_for_child( childInfo ) )
                {
                    arg.val = ui_decoupler_succeeds;
                    if( semctl( mSemId, ui_decoupler_sem, SETVAL, arg ) < 0 )
                    {
                        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                        LOG( string( string("Can't set private SysV IPC semaphore ") + to_string(ui_decoupler_sem) +
                                     " to " + to_string(ui_decoupler_succeeds) +
                                     // " at '" + mStartSemaphore + "': "
                                     + strerror(errno) ).c_str() );
                        LOG_END;
                        return -1;
                    }
                    _exit( 0 );
                }
                else
                    exit(255);
            }

            arg.val = ui_daemon_starts;
            if( semctl( mSemId, ui_daemon_sem, SETVAL, arg ) < 0 )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG( string( string("Can't set private SysV IPC semaphore ") + to_string(ui_daemon_sem) +
                             " to " + to_string(ui_daemon_starts) +
                             // " at '" + mStartSemaphore + "': "
                             + strerror(errno) ).c_str() );
                LOG_END;

                return -1;
            }

            if( chdir( "/" ) < 0 ) // chdir to /, so we don't prevent umounting of file systems
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG(string( string("Can't chdir(\"/\"): ") + strerror(errno)).c_str());
                LOG_END;

                return -1;
            }

            if( 0 != hold_daemon_pid() )
            {
                return -1;
            }

            if( -1 == close(STDIN_FILENO) || -1 == open( DEVNULL, O_RDWR ) )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG(string( string("Can't open ") + DEVNULL + " as stdin: " + strerror(errno)).c_str());
                LOG_END;

                return -1;
            }

#ifndef _NO_LOGGING
            if( 0 != setup_logging(true) )
            {
                return -1;
            }
#else
            if( ( dup2( STDIN_FILENO, STDOUT_FILENO ) == -1 ) || ( dup2( STDIN_FILENO, STDERR_FILENO ) == -1 ) )
            {
                return -1;
            }
#endif
        }

        break;

    default: // parent
        ChildInfo childInfo;
        childInfo.child = child;
        childInfo.who = "Decoupler";
        childInfo.semnum = ui_decoupler_sem;
        childInfo.semval = ui_decoupler_succeeds;
        childInfo.wantexit = true;

        if( wait_for_child( childInfo ) <= 0 )
        {
            rc = -1;
        }
        else
        {
            rc = child;
        }

        cleanup_sysv_sem();
    }

    return rc;
}

pid_t
UI::get_daemon_pid()
{
    pid_t daemonPid = -1;
    string const &pidFileName = Config::getInstance().getPidFileName(true);
    FILE *pidfileHandle;

    /* fcntl() related bug in at least AIX will release any lock when reopening a file
       and close the reopened file descriptor */
    if( mPidfileDesc < 0 )
        pidfileHandle = fopen( pidFileName.c_str(), "r" );
    else
        pidfileHandle = fdopen( mPidfileDesc, "r" );

    if( !pidfileHandle )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
        LOG(string( string("Can't open ") + pidFileName + ": " + strerror(errno)).c_str());
        LOG_END;

        return -1;
    }

    if( mPidfileDesc >= 0 )
    {
        if( fseek( pidfileHandle, 0, SEEK_SET ) < 0 )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
            LOG(string( string("Can't rewind ") + pidFileName + ": " + strerror(errno)).c_str());
            LOG_END;

            return -1;
        }
    }

    int rc = fscanf( pidfileHandle, "%d", &daemonPid );

    if( mPidfileDesc < 0 )
        fclose( pidfileHandle );

    if( rc <= 0 )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
        LOG(string( string("Can't parse ") + pidFileName + ": " + strerror(errno)).c_str());
        LOG_END;

        return -1;
    }

    return daemonPid;
}

int
UI::hold_daemon_pid()
{
    string const &pidFileName = Config::getInstance().getPidFileName();
    if( ( mPidfileDesc < 0 ) && ( pidFileName.length() != 0 ) )
    {
        /* open RDWR to allow file descriptor reusing in get_daemon_pid() because of a fcntl() bug in at least AIX */
        mPidfileDesc = open( pidFileName.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP );
        if( -1 == mPidfileDesc )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG(string( string("Can't open ") + pidFileName + ": " + strerror(errno)).c_str());
            LOG_END;

            return -1;
        }

        if( -1 == flock( mPidfileDesc, LOCK_EX | LOCK_NB ) )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG(string( string("Can't aquire lock on ") + pidFileName + ": " + strerror(errno)).c_str());
            LOG_END;

            close(mPidfileDesc);
            mPidfileDesc = -1;

            return -1;
        }

        if( -1 == ftruncate( mPidfileDesc, 0 ) )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG(string( string("Can't ftruncate ") + pidFileName + ": " + strerror(errno)).c_str());
            LOG_END;

            close(mPidfileDesc);
            mPidfileDesc = -1;

            return -1;
        }

        char buf[64];
        snprintf( buf, sizeof(buf), "%d\n", getpid() );

        if( ( write( mPidfileDesc, buf, strlen(buf) ) != ((ssize_t)(strlen(buf))) ) || ( fsync( mPidfileDesc ) < 0 ) )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG(string( string("Can't write pid to pid-file ") + pidFileName + ": " + strerror(errno)).c_str());
            LOG_END;

            close(mPidfileDesc);
            mPidfileDesc = -1;

            return -1;
        }

        if( get_daemon_pid() != getpid() )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG(string( string("Invalid pid written to pid-file ") + pidFileName + ": " + strerror(errno)).c_str());
            LOG_END;

            close(mPidfileDesc);
            mPidfileDesc = -1;

            return -1;
        }
    }

    return 0;
}

int
UI::start_daemon()
{
#ifdef HAVE_PTHREAD
    int err;
    sigset_t sigmask;
#endif

    Options const &cmndline = Options::getInstance();
    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( string( string("Starting with start directory: ") + cmndline.getStartDirectory() ).c_str() );
    LOG_END;

    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( string( string("Starting with config file: ") + cmndline.getConfigFile() ).c_str() );
    LOG_END;

    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( string( string("Starting with pid file: ") + cmndline.getPidFile() ).c_str() );
    LOG_END;

#ifndef _NO_LOGGING
# ifdef WITH_LOG_PROFILES
    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( string( string("Starting with log profile: ") + cmndline.getLogProfile() ).c_str() );
    LOG_END;
# endif
# ifdef WITH_LIBLOG4CPLUS
    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( string( string("Starting with log property file: ") + cmndline.getLogPropertyFile() ).c_str() );
    LOG_END;
# else
    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( string( string("Starting with log file: ") + cmndline.getLogFile() ).c_str() );
    LOG_END;
# endif
#endif

    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( string( string("Starting with status file: ") + cmndline.getStatusFile() ).c_str() );
    LOG_END;

    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( string( string("Starting with daemonize default: ") + string( cmndline.getDaemonizeDefault() ? "true" : "false" )
               + string(", daemonize: ") + string( cmndline.getDaemonize() ? "true" : "false" ) ).c_str() );
    LOG_END;

    if( Config::getInstance().getDaemonize() )
    {
        pid_t d = fork_daemon();
        if( d < 0 )
            return 255;
        if( d > 0 )
            return 0;
    }
    else
    {
        if( ( hold_daemon_pid() == -1 ) || setup_logging(true) == -1 )
            return 255;
    }

    ResourceLimits const &rlims = Config::getInstance().getDaemonResourceLimits();
    for( ResourceLimits::const_iterator ci = rlims.begin(); ci != rlims.end(); ++ci )
    {
        ci->commit();
    }

    runningUI = this;

#ifdef HAVE_PTHREAD
    sigemptyset(&sigmask);
    for( size_t i = 0; i < lengthof(handled_signals); ++i )
        sigaddset( &sigmask, handled_signals[i] );
    if( ( err = pthread_sigmask( SIG_BLOCK, &sigmask, &mOldSigMask ) ) != 0 )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        LOG(string( string("Can't block signals:") + strerror(err)).c_str());
        LOG_END;

        return 255;
    }

    pthread_t sig_thr;
    if( ( err = pthread_create( &sig_thr, NULL, &sig_watcher, 0 ) ) != 0 )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
        LOG(string( string("Can't create signal watcher (thread):") + strerror(err)).c_str());
        LOG_END;

        return 255;
    }
#else
    init_signals();
#endif

    do
    {
        Agent agent;
        runningAgent = &agent;

        agent.Init();

        if( Config::getInstance().getDaemonize() && (mSemId >= 0) )
        {
            union semun arg;
            arg.val = ui_daemon_runs;
            if( semctl( mSemId, ui_daemon_sem, SETVAL, arg ) < 0 )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG( string( string("Can't set private SysV IPC semaphore ") + to_string(ui_daemon_sem) +
                             " to " + to_string(ui_daemon_runs) +
                             // " at '" + mStartSemaphore + "': "
                             + strerror(errno) ).c_str() );
                LOG_END;

                return 255;
            }
        }
        mSemId = -1;

        agent.Run();

        runningAgent = NULL;
        runningUI = NULL;
    } while(0);

    LOG_BEGIN( loggerModuleName, EVENT_LOG | 1);
    LOG("Agent finished");
    LOG_END;

#ifdef HAVE_PTHREAD
    // send SIGHUP to enforce signal watching thread to end hisself
    pthread_kill( sig_thr, SIGHUP );
    pthread_join( sig_thr, NULL );
    if( ( err = pthread_sigmask( SIG_SETMASK, &mOldSigMask, &sigmask ) ) != 0 )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
        LOG(string( string("Can't restore signals:") + strerror(err)).c_str());
        LOG_END;
    }
#endif

    return 0;
}

int
UI::stop_daemon()
{
    pid_t daemon_pid = get_daemon_pid();
    if( -1 != daemon_pid )
    {
        kill( daemon_pid, SIGQUIT );
    }
    else
        return 255;

    return 0;
}

int
UI::restart_daemon()
{
    pid_t daemon_pid = get_daemon_pid();
    if( -1 != daemon_pid )
    {
        kill( daemon_pid, SIGQUIT );

        LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
        LOG("Waiting current daemon ends ...");
        LOG_END;

        string const &pidFileName = Config::getInstance().getPidFileName(true);
        while( -1 != access( pidFileName.c_str(), F_OK ) )
            sleep( 1 );

        return start_daemon();
    }
    else
        return 255;

    return 0;
}

int
UI::reload_daemon()
{
    pid_t daemon_pid = get_daemon_pid();
    if( -1 != daemon_pid )
    {
        kill( daemon_pid, SIGHUP );
    }
    else
        return 255;

    return 0;
}


int
UI::graceful_stop_daemon()
{
    pid_t daemon_pid = get_daemon_pid();
    if( -1 != daemon_pid )
    {
        kill( daemon_pid, SIGTERM );
    }
    else
        return 255;

    return 0;
}

int
UI::graceful_restart_daemon()
{
    pid_t daemon_pid = get_daemon_pid();
    if( -1 != daemon_pid )
    {
        kill( daemon_pid, SIGTERM );

        LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
        LOG("Waiting current daemon ends ...");
        LOG_END;

        string const &pidFileName = Config::getInstance().getPidFileName(true);
        while( -1 != access( pidFileName.c_str(), F_OK ) )
            sleep( 1 );

        return start_daemon();
    }
    else
        return 255;

    return 0;
}

int
UI::kill_daemon()
{
    pid_t daemon_pid = get_daemon_pid();
    if( -1 != daemon_pid )
    {
        kill( daemon_pid, SIGKILL );
    }
    else
        return 255;

    return 0;
}

int
UI::run()
{
    switch( Options::getInstance().getAction() )
    {
    case Options::saStart:
        return start_daemon();

    case Options::saStop:
        return stop_daemon();

    case Options::saRestart:
        return restart_daemon();

    case Options::saReload:
        return reload_daemon();

    case Options::saCheck:
        LOG_BEGIN( loggerModuleName, INFO_LOG | 1);
        LOG("Config ok");
        LOG_END;
        return 0;

    default:
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1);
        LOG("Unimplemented");
        LOG_END;
        break;
    }

    return 255;
}

}
