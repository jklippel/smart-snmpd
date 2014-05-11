/*
 * Copyright 2010 Matthias Haag, Jens Rehsack
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
#ifndef __SMART_SNMPD_CMNDLINE_H_INCLUDED__
#define __SMART_SNMPD_CMNDLINE_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>

#include <string>

using namespace std;

namespace SmartSnmpd
{
    class Options
    {
        friend class UI;

    protected:
        // first hide default constructor
        inline Options()
            : mConfigFile()
            , mPidFile()
            , mAction()
#ifndef _NO_LOGGING
# ifdef WITH_LOG_PROFILES
            , mLogProfile()
# endif
# ifdef WITH_LIBLOG4CPLUS
	    , mLogPropertyFile()
# else
            , mLogFile()
# endif
#endif
            , mStatusFile()
            , mDaemonize(true)
            , mDaemonizeDefault(true)
        {}

        // create instance (probably only compiler helper)
        static void createInstance();

        // set arguments and parse
        virtual void setArguments(int argc, char **argv);

        static Options & getInstance(int argc, char **argv)
        {
            if( 0 == mInstance )
                createInstance(); // will be done initially and need not to be locked
            mInstance->setArguments(argc, argv);
            return *mInstance;
        }

    private:
        // unusables ...
        Options(Options const &);
        Options & operator = (Options const &);

    public:
        enum Action {
            saStart = 0,
            saStop,
            saGracefulStop, // currently unimplemented
            saRestart,
            saReload,
            saGraceful, // graceful restart
            saKill, // SIGKILL
            saCheck
        };

        // destruct
        virtual ~Options() {}

        // singleton
        static Options const & getInstance()
        {
            if( 0 == mInstance )
                createInstance();
            return *mInstance;
        }

        // typical accesses
        virtual const string & getStartDirectory() const { return mStartDirectory; }
        virtual const string & getConfigFile() const { return mConfigFile; }
        virtual const string & getPidFile() const { return mPidFile; }
        virtual Action getAction() const { return mAction; }
#ifndef _NO_LOGGING
# ifdef WITH_LOG_PROFILES
        virtual const string & getLogProfile() const { return mLogProfile; }
# endif
# ifdef WITH_LIBLOG4CPLUS
        virtual const string & getLogPropertyFile() const { return mLogPropertyFile; }
# else
        virtual const string & getLogFile() const { return mLogFile; }
# endif
#endif
        virtual const string & getStatusFile() const { return mStatusFile; }
        virtual bool getDaemonize() const { return mDaemonize; }
        virtual bool getDaemonizeDefault() const { return mDaemonizeDefault; }

    protected:
        // override when override setArguments
        virtual void usage();
        virtual void help();

    protected:
        static Options *mInstance;

        string mStartDirectory;
        string mConfigFile;
        string mPidFile;
        Action mAction;
#ifndef _NO_LOGGING
# ifdef WITH_LOG_PROFILES
        string mLogProfile;
# endif
# ifdef WITH_LIBLOG4CPLUS
	string mLogPropertyFile;
# else
        string mLogFile;
# endif
#endif
        string mStatusFile;
        bool mDaemonize;
        bool mDaemonizeDefault;
    };
}

#endif /* __SMART_SNMPD_CMNDLINE_H_INCLUDED__ */
