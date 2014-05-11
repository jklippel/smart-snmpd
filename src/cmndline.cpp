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
#include <smart-snmpd/cmndline.h>
#include <smart-snmpd/smart-snmpd.h>
#include <map>

using namespace std;

namespace SmartSnmpd
{

class ActionMap
    : public map<string, Options::Action>
{
public:
    ActionMap()
        : map<string, Options::Action>()
    {
        (*this)["start"] = Options::saStart;
        (*this)["stop"] = Options::saStop;
        (*this)["restart"] = Options::saRestart;
        (*this)["graceful"] = Options::saGraceful;
        (*this)["reload"] = Options::saReload;
        (*this)["graceful-stop"] = Options::saGracefulStop;
        (*this)["kill"] = Options::saKill;
        (*this)["check"] = Options::saCheck;
    }
};

const ActionMap actionMap;

Options *Options::mInstance = 0;

void
Options::createInstance()
{
    static Options instance;
    mInstance = &instance;
}

#ifndef _NO_LOGGING
#  ifdef WITH_LOG_PROFILES
#    define LOG_OPTIONS "h?dDf:p:L:k:l:s:"
#  else
#    define LOG_OPTIONS "h?dDf:p:l:k:s:"
#  endif
#else
#  define LOG_OPTIONS "h?dDf:p:k:s:"
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

void
Options::setArguments(int argc, char **argv)
{
    int ch, rc;
    char buf[PATH_MAX + 1];

    if( getcwd( buf, sizeof(buf) ) != 0 )
    {
        mStartDirectory = buf;
    }
    else
    {
        (void)fprintf( stderr, "smart-snmpd: getcwd(): %s\n", strerror(errno) );
        exit(1);
    }

    while ((ch = getopt(argc, argv, LOG_OPTIONS)) != -1)
    {
        switch (ch) {
        case 'f':
            if ((rc = access(optarg, R_OK)) < 0)
            {
                (void)fprintf(stderr, "smart-snmpd: %s: %s\n", optarg, strerror(errno));
                exit(1);
            }
            if( 0 == realpath( optarg, buf ) )
                mConfigFile = optarg;
            else
                mConfigFile = buf;
            break;

        case 'p':
            if( 0 == realpath( optarg, buf ) )
                mPidFile = optarg;
            else
                mPidFile = buf;
            break;

#ifndef _NO_LOGGING
        case 'l':
#ifdef WITH_LIBLOG4CPLUS
            if ((rc = access(optarg, R_OK)) < 0)
            {
                (void)fprintf(stderr, "smart-snmpd: %s: %s\n", optarg, strerror(errno));
                exit(1);
            }
            if( 0 == realpath( optarg, buf ) )
                mLogPropertyFile = optarg;
            else
                mLogPropertyFile = buf;
#else
            if ((rc = open(optarg, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
            {
                (void)fprintf(stderr, "smart-snmpd: %s: %s\n", optarg, strerror(errno));
                exit(1);
            }
            close(rc);
            if( 0 == realpath( optarg, buf ) )
                mLogFile = optarg;
            else
                mLogFile = buf;
#endif
            break;

#ifdef WITH_LOG_PROFILES
        case 'L':
            // currently we swallow every input here ... - we need to select better
            mLogProfile = optarg;
            break;
#endif
#endif

        case 'k':
            {
                map<string, Options::Action>::const_iterator item = actionMap.find(optarg);
                if( item == actionMap.end() )
                {
                    (void)fprintf(stderr, "smart-snmpd: %s: unknown action\n", optarg);
                    exit(1);
                }
                mAction = item->second;
            }
            break;

        case 's':
            if ((rc = open(optarg, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) < 0)
            {
                (void)fprintf(stderr, "smart-snmpd: %s: %s\n", optarg, strerror(errno));
                exit(1);
            }
            close(rc);
            if( 0 == realpath( optarg, buf ) )
                mStatusFile = optarg;
            else
                mStatusFile = buf;
            break;

        case 'd':
            mDaemonize = true;
            mDaemonizeDefault = false;
            break;

        case 'D':
            mDaemonize = false;
            mDaemonizeDefault = false;
            break;

        case 'h':
            help();

        case '?':
        default:
            usage();
        }
    }

    argc -= optind;
    argv += optind;

    if( mConfigFile.empty() )
        mConfigFile = DEFAULT_CONFIG_FILE;
}

void
Options::usage()
{
    cout << "Usage:" << endl
         << "smart-snmpd [OPTIONS]" << endl;
    exit(0);
}

void
Options::help()
{
    cout << "smart-snmpd [OPTIONS]" << endl
         << "Options:" << endl
         << "\t-f config\tloads config from specified file" << endl
         << "\t\t\t(default: " << DEFAULT_CONFIG_FILE << ")" << endl
         << "\t-p pid-file\tuses specified pid-file" << endl
         << "\t\t\t(default: " << DEFAULT_PID_FILE << ")" << endl
         << "\t-k start|restart|graceful|stop|check|reload|kill" << endl
         << "\t\t\tdo specified action (default: start)" << endl
         << "\t-s status-file\tuse specified status file" << endl
         << "\t\t\t(default: " << DEFAULT_STATUS_FILE << ")" << endl
#ifndef _NO_LOGGING
#ifdef WITH_LIBLOG4CPLUS
         << "\t-l log-property-file\tconfigures log4cplus using specified property file" << endl
         << "\t\t\t(default: " << DEFAULT_LOG_PROPERTY_FILE << ")" << endl
#else
         << "\t-l log-file\tlogs into specified file" << endl
         << "\t\t\t(default: " << DEFAULT_LOG_FILE << ")" << endl
#endif
#ifdef WITH_LOG_PROFILES
         << "\t-L log-profile\tuses specified log-profile (see config example)" << endl
         << "\t\t\t(default: quiet)" << endl
#endif
#endif
         << "\t-d - daemonizes smart-snmpd (default)" << endl
         << "\t-D - debug smart-snmpd in foreground (implies -L debug)" << endl
         << endl;
    exit(0);
}

}
