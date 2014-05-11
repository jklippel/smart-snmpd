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
#ifndef __SMART_SNMPD_EXTCMD_H_INCLUDED__
#define __SMART_SNMPD_EXTCMD_H_INCLUDED__

#include <smart-snmpd/config.h>
#include <smart-snmpd/resourcelimits.h>

#include <string>
#include <vector>

namespace SmartSnmpd
{
    /**
     * class to encapsulate external command starting and output capturing
     */
    class ExternalCommand
    {
    public:
        /**
         * constructor
         *
         * Creates execution plan from external command configuration
         *
         * @param cmdcfg - specification how to run external command
         */
        ExternalCommand(ExternalCommandConfig const &cmdcfg)
            : mExecutable(cmdcfg.Executable)
            , mArguments(cmdcfg.Arguments)
            , mResourceLimits(cmdcfg.ResourceLimits)
            , mBuffers(2)
            , mExitCode(-1)
            , mExitSignal(-1)
            , mChildPid(-1)
        {
            mFileno[0] = mFileno[1] = -1;
        }

        /**
         * destructor
         *
         * If invoked while the command is still running, a SIGTERM is send
         * and the child process is reaped via finish().
         */
        virtual ~ExternalCommand();

        /**
         * start configured external command
         *
         * @return int - 0 on success, errno code on error
         */
        virtual int start();
        /**
         * polls external command status
         *
         * @param seconds - amount of seconds to wait for data or process exit
         *
         * @return bool - true if still running, false if can reaped
         */
        virtual bool poll(unsigned seconds);
        /**
         * sends a signal to the process started via external command
         *
         * @param signal - signal number to send (eg. SIGABRT)
         *
         * @return int - 0 on success, errno code on error (eg. ESRCH on "no process (anymore)")
         */
        virtual int kill(int signal);
        /**
         * waits (blocking) until started process has been ended and all
         * data is collected
         */
        virtual void finish() { while( poll(1) ) (void)0; }

        /**
         * grants access to the captured STDOUT content
         *
         * @return string const & - string buffer containing the captured
         *   output without any modification
         */
        inline string const & getOutBuf() const { return mBuffers[0]; }
        /**
         * grants access to the captured STDERR content
         *
         * @return string const & - string buffer containing the captured
         *   error output without any modification
         */
        inline string const & getErrBuf() const { return mBuffers[1]; }
        /**
         * returns the exit code of the process
         *
         * @return int - exit code from finished external command or -1, if
         *   ended by signal or never been started etc.
         */
        inline int getExitCode() const { return mExitCode; }
        /**
         * returns the exit signal of the process
         *
         * @return int - exit code from finished external command or -1, if
         *   ended by exit code or never been started etc.
         */
        inline int getExitSignal() const { return mExitSignal; }
        /**
         * return pid of started child, while running
         *
         * @return pid_t - child's pid or (pid_t)-1 if not running
         */
        inline int getChildPid() const { return mChildPid; }

    protected:
        /**
         * path to executable command
         */
        string mExecutable;
        /**
         * list of arguments pass to the command when executing
         */
        vector<string> mArguments;
        /**
         * resource limits to adjust before executing
         */
        class ResourceLimits mResourceLimits;
        /**
         * output capture buffers
         */
        vector<string> mBuffers;
        /**
         * exit code "cache"
         */
        int mExitCode;
        /**
         * exit signal "cache"
         */
        int mExitSignal;

        /**
         * process id of running child
         */
        pid_t mChildPid;
        /**
         * file handles of capturing pipes
         */
        int mFileno[2];

    private:
        //! forbidden default constructor
        ExternalCommand();
        //! forbidden copy constructor
        ExternalCommand(ExternalCommand const &);
        //! forbidden assignment operator
        ExternalCommand & operator = (ExternalCommand const &);
    };

    /**
     * class to encapsulate external command starting as specific user and output capturing
     */
    class ExternalCommandAsUser
        : public ExternalCommand
    {
    public:
        /**
         * constructor
         *
         * Creates execution plan from external command configuration
         *
         * @param cmdcfg - specification how to run external command
         */
        ExternalCommandAsUser(ExternalCommandConfig const &cmdcfg);

    protected:
        /**
         * user name to run as
         */
        string mUser;
    };
}

#endif /* __SMART_SNMPD_EXTCMD_H_INCLUDED__ */
