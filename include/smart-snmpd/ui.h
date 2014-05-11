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
#ifndef __SMART_SNMPD_UI_H_INCLUDED__
#define __SMART_SNMPD_UI_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/agent.h>

#include <smart-snmpd/log.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#ifndef _NO_LOGGING
# ifdef WITH_LIBLOG4CPLUS
using namespace log4cplus;
# endif
#endif

namespace SmartSnmpd
{

    class UI
    {
    protected:
        //! contains information about the child process to wait for
        struct ChildInfo
        {
            //! pid of the child
            pid_t child;
            //! human readable name (for status/error messages)
            string who;
            //! semaphore number
            int semnum;
            //! desired semaphore value
            int semval;
            //! true when exit of child process is required
            bool wantexit;
            //! returned, value from waitpid() or alike
            int procrc;
            //! returned, value from semctl(GETVAL)
            int semrc;
        };

    public:
        /**
         * constructor
         *
         * @param argc - argument count from command line
         * @param argv - argument value array from command line
         */
        UI(int argc, char **argv);

        //! destructor
        ~UI();

        /**
         * invokes the right method based on the command line requested action
         */
        int run();

    protected:
        //! file descriptor of pid file (when open)
        int mPidfileDesc;
        //! identifier/handle of the SystemV semaphore
        int mSemId;
#ifndef _NO_LOGGING
# ifdef WITH_LIBLOG4CPLUS
        //! auto-reconfigure thread of log4cplus
        ConfigureAndWatchThread *mConfigureAndWatchThread;
# else
        //! file stream for snmp++ builtin logging
        FILE *mLogfileHandle;
# endif
#endif
#ifdef HAVE_PTHREAD
        //! signal mask before own signal handler thread has been started (for recovering)
        sigset_t mOldSigMask;
        //! signal watcher thread routine
        //! @param arg - posix thread forced parameter, unused
        static void *sig_watcher(void *arg);
#else
        //! installs signal handlers to be called from kernel when no threads are available
        static void init_signals();
#endif
        //! routine to react on signals
        //! @param signo - signal number passed from the kernel
        static void sig(int signo);

#ifndef _NO_LOGGING
        /**
         * setup logging from configuration or command line
         *
         * @param reopen - when true, assume context switch, configuration
         *                 reload or finalized bootstrap and force reinitialize
         *                 all targets (eg. reopen files)
         */
        int setup_logging(bool reopen = false);
#endif
        //! creates the System V semaphore to control startup
        int create_sysv_sem();
        //! cleans up the System V semaphore after startup process ends
        int cleanup_sysv_sem();
        /**
         * wait for forked child to decouple daemon from starting process
         *
         * @param childInfo - information about the child process to wait for
         * @param sleep_seconds - seconds to sleep before recheck child status
         *
         * @return -1 on error, 0 when child is still running, child pid when child exited
         */
        int wait_for_child( ChildInfo &childInfo, int sleep_seconds = 1 );

        //! called to reload the configuration (eg. when SIGHUP is catched)
        int reload_config();
        //! fork the daemon process to decouple daemon from starting process
        int fork_daemon();
        /**
         * get the daemon process identifier
         *
         * This method retrieved the daemon identifier from the configured
         * pid file.
         *
         * @return pid_t - the retrieved pid or (pid_t)-1 on error
         */
        pid_t get_daemon_pid();
        //! writes the current pid into configured pid file
        int hold_daemon_pid();

        //! runs required actions to start the daemon
        int start_daemon();
        //! runs required actions to stop the daemon
        int stop_daemon();
        //! runs required actions to gracefully stop a running daemon
        int graceful_stop_daemon();
        //! runs the required actions to restart an already running daemon
        int restart_daemon();
        //! forces an already running daemon to reload it's configuration
        int reload_daemon();
        //! runs the required actions to gracefully restart an already running daemon
        int graceful_restart_daemon();
        //! takes the action to kill an already running daemon
        int kill_daemon();
    };

}

#endif /* __SMART_SNMPD_UI_H_INCLUDED__ */
