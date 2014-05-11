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
#ifndef __SMART_SNMPD_CONFIG_H_INCLUDED__
#define __SMART_SNMPD_CONFIG_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/cmndline.h>
#include <smart-snmpd/resourcelimits.h>

#include <confuse.h>

#include <string>
#include <vector>
#include <map>

using namespace std;

namespace SmartSnmpd
{
    struct ExternalCommandConfig
    {
        string Executable;
        vector<string> Arguments;
        string User;
        class ResourceLimits ResourceLimits;
    };

    /**
     * basic configuration - common for all configurable mib objects
     */
    struct MibObjectConfig
    {
        inline MibObjectConfig()
            : MibEnabled(true)
            , AsyncUpdate(false)
            , CacheTime(30)
            , MostRecentIntervalTime(0)
            , ExternalCommand()
            , SubOid(0)
        {}

        inline MibObjectConfig(const MibObjectConfig &ref)
            : MibEnabled(ref.MibEnabled)
            , AsyncUpdate(ref.AsyncUpdate)
            , CacheTime(ref.CacheTime)
            , MostRecentIntervalTime(ref.MostRecentIntervalTime)
            // , ExternalCommand(ref.ExternalCommand)
            // , CommandArguments(ref.CommandArguments)
            // , User(ref.User)
            , ExternalCommand(ref.ExternalCommand)
            , SubOid(ref.SubOid)
        {}

        bool MibEnabled;
        bool AsyncUpdate;
        time_t CacheTime;
        // special addional settings for interval mibs (cpu cycles per minute etc.)
        time_t MostRecentIntervalTime;
        // special additional settings for mibs filled from external programs
        struct ExternalCommandConfig ExternalCommand;
        unsigned SubOid; // for unknown mibs
    };

    struct StatgrabSettings
    {
        bool RemoveFilesystems;
        vector<string> ValidFilesystems;
    };

    /**
     * USM (User-based Security Model) User Table Configuration
     */
    struct UsmEntry
    {
        string Username;
        int AuthProto;
        string AuthKey;
        int PrivProto;
        string PrivKey;
    };

    /**
     * VACM Group Entry
     */
    struct VacmGroupEntry
    {
        int SecurityModel;
        vector<string> SecurityNames;
        string GroupName;
        int StorageType;
    };

    /**
     * VACM Access Entry
     */
    struct VacmAccessEntry
    {
        string GroupName;
        string Context;
        int SecurityModel;
        int SecurityLevel;
        int Match;
        string ReadView;
        string WriteView;
        string NotifyView;
        int StorageType;
    };

    /**
     * VACM Context Entry
     */
    struct VacmViewEntry
    {
        string ViewName;
        string SubTree; // dotted n (1.3.6.1.2.1...)
        string Mask;
        int ViewType;
        int StorageType;
    };

#define v1v2_community_no_access 0
#define v1v2_community_read_access (1 << 0)
#define v1v2_community_write_access (1 << 1)
#define v1v2_community_notify_access (1 << 2)
#define v1v2_community_read_write_access (v1v2_community_read_access | v1v2_community_write_access)
#define v1v2_community_full_access (v1v2_community_read_access | v1v2_community_write_access | v1v2_community_notify_access)

    /**
     * V1/V2 compat shortcuts
     */
    struct VacmV1V2ShortCutEntry
    {
        string CommunityName;
        bool V1;
        int Access;
    };

    class Config
    {
        friend class UI;
        friend class DataSource; // to correct settings when not running

    protected:
        inline Config()
            : mCfg(0)
            // global agent config
            , mDaemonize(true)
            , mPort(161) // port to listen on, default nnnn
            , mListenOn()
            , mStatusFile()
            , mPidFile()
#ifdef WITH_SU_CMD
            , mSuCmd()
            , mSuArgs()
#endif

#ifndef _NO_LOGGING
#ifdef WITH_LOG_PROFILES
            , mLogProfile()
#endif
#ifdef WITH_LIBLOG4CPLUS
            , mLogPropertyFile()
#else
            , mLogFile()
#endif
            , mLogConfig()
#endif
#ifdef AGENTPP_USE_THREAD_POOL
            , mNumberOfJobThreads(16)
#endif
            // resource limits of the daemon
            , mDaemonResourceLimits()
            , mOnFatalError(onfKill)
            // managed mib-objects
            , mMibObjectConfigs()

            // statgrab settings
            , mStatgrabSettings()

            // v3 permissions
            , mUsmEntries()
            , mVacmGroupEntries()
            , mVacmAccessEntries()
            , mVacmViewEntries()
            , mVacmContextEntries()
            , mVacmV1V2ShortCutEntries()
        {}

        // create instance (probably only compiler helper)
        static void createInstance();

        int Read();

        // singleton
        static Config & getInstance(int)
        {
            if( 0 == mInstance )
                createInstance();
            return *mInstance;
        }

        MibObjectConfig & getMibObjectConfig(string const &mibOid);

    private:
        // unusables ...
        Config(Config const &);
        Config & operator = (Config const &);

    public:
        enum OnFatalError
        {
            onfIgnore,
            onfRaise,
            onfKill,
            onfExit,
            onf_Exit,
            onfAbort
        };

        virtual ~Config();

        // singleton
        static Config const & getInstance()
        {
            if( 0 == mInstance )
                createInstance();
            return *mInstance;
        }

        inline static bool getDaemonize()
        {
            if( !Options::getInstance().getDaemonizeDefault() )
                return Options::getInstance().getDaemonize();

            return getInstance().mDaemonize;
        }

        inline int getPort() const { return mPort; }
        inline string const & getListenOn() const { return mListenOn; }

#ifdef WITH_SU_CMD
        inline string const & getSuCmd() const { return mSuCmd; }
        inline vector<string> const & getSuArgs() const { return mSuArgs; }
#endif

        inline string const & getStatusFileName() const
        {
            if( !Options::getInstance().getStatusFile().empty() )
                return Options::getInstance().getStatusFile();
            if( mCfg && cfg_getstr( mCfg, "status-file" ) )
                return mStatusFile;
            return mDefaultStatusFile;
        }
        inline string const & getPidFileName(bool need = getDaemonize()) const
        {
            if( !Options::getInstance().getPidFile().empty() )
                return Options::getInstance().getPidFile();
            if( mCfg && cfg_getstr( mCfg, "pid-file" ) )
                return mPidFile;
            return need ? mDefaultPidFile : mEmptyString;
        }

#ifndef _NO_LOGGING
#ifdef WITH_LOG_PROFILES
        inline string const & getLogProfile() const
        {
            if( mCfg )
            {
                if( cfg_size( mCfg, "log-profile" ) > 0 )
                    return mLogProfile;
                if( cfg_size( mCfg, "log-class" ) > 0 )
                    return mIndividualLogClass;
            }

            return mLogProfile;
        }
#endif
#ifdef WITH_LIBLOG4CPLUS
        inline string const & getLogPropertyFile() const
        {
            if( !Options::getInstance().getLogPropertyFile().empty() )
                return Options::getInstance().getLogPropertyFile();
            if( mCfg && cfg_getstr( mCfg, "log4cplus-property-file" ) )
                return mLogPropertyFile;
            return mDefaultLogPropertyFile;
        }
#else
        inline string const & getLogFile() const
        {
            if( !Options::getInstance().getLogFile().empty() )
                return Options::getInstance().getLogFile();
            if( mCfg && cfg_getstr( mCfg, "log-file" ) )
                return mLogFile;
            return mDefaultLogFile;
        }
#endif
        inline int getLogLevel(string const &aLogClass) const
        {
            map<string,int>::const_iterator iter = mLogConfig.find(aLogClass);
            if( iter != mLogConfig.end() )
                return iter->second;

            return -2;
        }
#endif

#ifdef AGENTPP_USE_THREAD_POOL
        inline int getNumberOfJobThreads() const { return mNumberOfJobThreads; }
#endif

        inline ResourceLimits const & getDaemonResourceLimits() const { return mDaemonResourceLimits; }
        inline OnFatalError getOnFatalError() const { return mOnFatalError; }

        inline const map<string, MibObjectConfig> & getMibObjectConfigs() const { return mMibObjectConfigs; }
        MibObjectConfig const & getMibObjectConfig(string const &mibOid) const;

        inline StatgrabSettings const & getStatgrabSettings() const { return mStatgrabSettings; }

        inline vector<UsmEntry> const & getUsmEntries() const { return mUsmEntries; }
        inline vector<VacmGroupEntry> const & getVacmGroupEntries() const { return mVacmGroupEntries; }
        inline vector<VacmAccessEntry> const & getVacmAccessEntries() const { return mVacmAccessEntries; }
        inline vector<VacmViewEntry> const & getVacmViewEntries() const { return mVacmViewEntries; }
        inline map<string, bool> const & getVacmContextEntries() const { return mVacmContextEntries; }
        inline vector<VacmV1V2ShortCutEntry> const & getVacmV1V2ShortCutEntries() const { return mVacmV1V2ShortCutEntries; }

    protected:
        static const string mEmptyString;
        static const string mDefaultStatusFile;
        static const string mDefaultPidFile;
#ifndef _NO_LOGGING
#ifdef WITH_LIBLOG4CPLUS
	static const string mDefaultLogPropertyFile;
#else
        static const string mDefaultLogFile;
#endif
#ifdef WITH_LOG_PROFILES
        static const string mIndividualLogClass;
#endif
#endif

        static Config *mInstance;

        cfg_t *mCfg;
        // global agent config
        bool mDaemonize;
        int mPort; // port to listen on, default nnnn
        string mListenOn;
        string mStatusFile;
        string mPidFile;
#ifdef WITH_SU_CMD
        string mSuCmd;
        vector<string> mSuArgs;
#endif

#ifndef _NO_LOGGING
#ifdef WITH_LOG_PROFILES
        string mLogProfile;
#endif
#ifdef WITH_LIBLOG4CPLUS
        string mLogPropertyFile;
#else
        string mLogFile;
#endif
        map<string, int> mLogConfig;
#endif
#ifdef AGENTPP_USE_THREAD_POOL
        int mNumberOfJobThreads;
#endif
        ResourceLimits mDaemonResourceLimits;
        OnFatalError mOnFatalError;
        // managed mib-objects
        map<string, MibObjectConfig> mMibObjectConfigs;

        // statgrab settings
        StatgrabSettings mStatgrabSettings;

        // v3 permissions
        vector<UsmEntry> mUsmEntries;
        vector<VacmGroupEntry> mVacmGroupEntries;
        vector<VacmAccessEntry> mVacmAccessEntries;
        vector<VacmViewEntry> mVacmViewEntries;
        map<string, bool> mVacmContextEntries;
        vector<VacmV1V2ShortCutEntry> mVacmV1V2ShortCutEntries;
    };
}

#endif /* __SMART_SNMPD_CONFIG_H_INCLUDED__ */
