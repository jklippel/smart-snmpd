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

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/oids.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/cmndline.h>
#include <smart-snmpd/agent.h>

#include <snmp_pp/log.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.agent";

//! initialization routine for mib modules when library is loaded
typedef MibModule * (*getModuleInfoFunc)(void);

#ifdef WITH_LIBSTATGRAB
extern MibModule * getStatgrabModInfo(void);
#endif

#ifdef WITH_EXTERNAL_COMMANDS
extern MibModule * getExtCmdModInfo(void);
#endif

static const getModuleInfoFunc moduleInfoFuncs[] =
{
#ifdef WITH_LIBSTATGRAB
    getStatgrabModInfo,
#endif
#ifdef WITH_EXTERNAL_COMMANDS
    getExtCmdModInfo,
#endif
    0 // to avoid empty array
};

Agent::Agent()
    : mMib( 0 )
    , mReqList( 0 )
    , mSnmp( 0 )
    , mv3mp( 0 )
    , mSignal(0)
    , mRunning( false )
    , mMibModules()
{
    for( size_t i = 0; i < (lengthof(moduleInfoFuncs) - 1); ++i )
    {
        MibModule *modInfo = moduleInfoFuncs[i]();
        mMibModules.push_back( modInfo );

        if( !modInfo->InitModule() )
        {
            LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
            LOG("Agent::Init(): mibs module initialization failed at (index)");
            LOG(i);
            LOG_END;

            exit(1);
        }
    }

    int status;

    Snmp::socket_startup();  // Initialize socket subsystem
    if( Config::getInstance().getListenOn().empty() )
        mSnmp = new Snmpx(status, Config::getInstance().getPort());
    else
        mSnmp = new Snmpx(status, UdpAddress( Config::getInstance().getListenOn().c_str() ) );
    if( !mSnmp )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
        LOG("Agent::Init(): Error allocating Snmpx");
        LOG(status);
        LOG_END;

        exit(1);
    }

    if (status == SNMP_CLASS_SUCCESS)
    {
        LOG_BEGIN(loggerModuleName, EVENT_LOG | 1);
        LOG("Agent::Init(): SNMP listen port");
        LOG(Config::getInstance().getPort());
        LOG_END;
    }
    else
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
        LOG("Agent::Init(): SNMP port init failed");
        LOG(status);
        LOG_END;

        delete mSnmp;

        mSnmp = 0;

        exit(1);
    }

    unsigned int snmpEngineBoots = 0;
    OctetStr engineId(SnmpEngineID::create_engine_id(Config::getInstance().getPort()));

    LOG_BEGIN(loggerModuleName,  DEBUG_LOG | 1 );
    LOG("status file name");
    LOG(Config::getInstance().getStatusFileName().c_str());
    LOG_END;

    // you may use your own methods to load/store this counter
    status = getBootCounter(Config::getInstance().getStatusFileName().c_str(), engineId, snmpEngineBoots);
    if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR))
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
        LOG("Agent::Init(): Error loading snmpEngineBoots counter (status)");
        LOG(status);
        LOG_END;
        exit(1);
    }

    snmpEngineBoots++;
    status = saveBootCounter(Config::getInstance().getStatusFileName().c_str(), engineId, snmpEngineBoots);
    if (status != SNMPv3_OK)
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
        LOG("Agent::Init(): Error saving snmpEngineBoots counter (status)");
        LOG(status);
        LOG_END;
        exit(1);
    }

    int stat;
    mv3mp = new v3MP(engineId, snmpEngineBoots, stat);
    if( !mv3mp )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
        LOG("Agent::Init(): Error allocating v3MP");
        LOG(status);
        LOG_END;

        delete mSnmp;
        mSnmp = 0;

        exit(1);
    }

    mMib = new Mib();
    if( !mMib )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
        LOG("Agent::Init(): Error allocating Mib");
        LOG(status);
        LOG_END;

        delete mSnmp;
        mSnmp = 0;

        delete mv3mp;
        mv3mp = 0;

        exit(1);
    }

    mReqList = new RequestList();
    if( !mMib )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
        LOG("Agent::Init(): Error allocating RequestList");
        LOG(status);
        LOG_END;

        delete mMib;
        mMib = 0;

        delete mSnmp;
        mSnmp = 0;

        delete mv3mp;
        mv3mp = 0;

        exit(1);
    }

    // register v3MP
    mReqList->set_v3mp(mv3mp);

    // register requestList for outgoing requests
    mMib->set_request_list(mReqList);
}

Agent::~Agent()
{
    Snmp::socket_cleanup();  // Shut down socket subsystem

    for( vector<MibModule *>::size_type i = 0; i < mMibModules.size(); ++i )
    {
        MibModule *modInfo = mMibModules[i];
        if( !modInfo->UnregisterMibs( *mMib ) )
        {
            LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
            LOG("Agent::~Agent(): mibs module unregistering failed at (index)");
            LOG(i);
            LOG_END;
        }
    }

    delete mMib; mMib = 0;
    delete mReqList; mReqList = 0;
    delete mSnmp; mSnmp = 0;
    delete mVacm; mVacm = 0;
    delete mv3mp; mv3mp = 0;

    for( vector<MibModule *>::size_type i = 0; i < mMibModules.size(); ++i )
    {
        MibModule *modInfo = mMibModules[i];
        if( !modInfo->ShutdownModule() )
        {
            LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
            LOG("Agent::~Agent(): mibs module shutdown failed at (index)");
            LOG(i);
            LOG_END;
        }
    }
}

int
Agent::RefreshMibConfig()
{
    for( vector<MibModule *>::size_type i = 0; i < mMibModules.size(); ++i )
    {
        MibModule *modInfo = mMibModules[i];
        if( !modInfo->RefreshConfig( *mMib ) )
        {
            LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
            LOG("Agent::~Agent(): mibs module config refresh failed at (index)");
            LOG(i);
            LOG_END;
        }
    }

    return 0;
}

void
Agent::UserInit()
{
    UsmUserTable *uut = new UsmUserTable();

    vector<UsmEntry> const &usmEntries = Config::getInstance().getUsmEntries();
    for( vector<UsmEntry>::const_iterator iter = usmEntries.begin(); iter != usmEntries.end(); ++iter )
    {
        uut->addNewRow( iter->Username.c_str(), iter->AuthProto, iter->PrivProto, iter->AuthKey.c_str(), iter->PrivKey.c_str() );
    }

    // add non persistent USM statistics
    mMib->add(new UsmStats());
    // add the USM MIB - usm_mib MibGroup is used to
    // make user added entries persistent
    mMib->add(new usm_mib(uut));
    // add non persistent SNMPv3 engine object
    mMib->add(new V3SnmpEngine());
    mMib->add(new MPDGroup());
}

void
Agent::VacmInit()
{
    // register VACM
    mVacm = new Vacm(*mMib);
    mReqList->set_vacm(mVacm);

    // initialize security information for v1/v2 community compat
    if( !Config::getInstance().getVacmV1V2ShortCutEntries().empty() )
    {
        mVacm->addNewView( "v1v2ReadView", 
			   "1.3",       
			   "",             // Mask "" is same as 0xFFFFFFFFFF...
			   view_included,  // alternatively: view_excluded
			   storageType_nonVolatile );
        mVacm->addNewView( "v1v2WriteView", 
			   "1.3",       
			   "",             // Mask "" is same as 0xFFFFFFFFFF...
			   view_included,  // alternatively: view_excluded
			   storageType_nonVolatile );
        mVacm->addNewView( "v1v2NotifyView", 
			   "1.3",       
			   "",             // Mask "" is same as 0xFFFFFFFFFF...
			   view_included,  // alternatively: view_excluded
			   storageType_nonVolatile );

        mVacm->addNewView( "v1v2NoReadView", 
			   "1.3",       
			   "",             // Mask "" is same as 0xFFFFFFFFFF...
			   view_excluded,  // alternatively: view_included
			   storageType_nonVolatile );
        mVacm->addNewView( "v1v2NoWriteView", 
			   "1.3",       
			   "",             // Mask "" is same as 0xFFFFFFFFFF...
			   view_excluded,  // alternatively: view_included
			   storageType_nonVolatile );
        mVacm->addNewView( "v1v2NoNotifyView", 
			   "1.3",       
			   "",             // Mask "" is same as 0xFFFFFFFFFF...
			   view_excluded,  // alternatively: view_included
			   storageType_nonVolatile );

        mVacm->addNewContext("");
    }

    vector<VacmV1V2ShortCutEntry> const &vacmV1V2ShortCutEntries = Config::getInstance().getVacmV1V2ShortCutEntries();
    for( vector<VacmV1V2ShortCutEntry>::const_iterator iter = vacmV1V2ShortCutEntries.begin();
         iter != vacmV1V2ShortCutEntries.end();
         ++iter )
    {
        const char *viewsForPermission[3];

        mVacm->addNewGroup( iter->V1 ? SNMP_SECURITY_MODEL_V1 : SNMP_SECURITY_MODEL_V2,
                           iter->CommunityName.c_str(), 
                           iter->V1 ? "v1group" : "v2group",
                           storageType_volatile);

        viewsForPermission[0] = iter->Access & v1v2_community_read_access ? "v1v2ReadView" : "v1v2NoReadView";
        viewsForPermission[1] = iter->Access & v1v2_community_write_access ? "v1v2WriteView" : "v1v2NoWriteView";
        viewsForPermission[2] = iter->Access & v1v2_community_notify_access ? "v1v2NotifyView" : "v1v2NoNotifyView";

        mVacm->addNewAccessEntry( iter->V1 ? "v1group" : "v2group", "", 
				  iter->V1 ? SNMP_SECURITY_MODEL_V1 : SNMP_SECURITY_MODEL_V2,
                                  SecurityLevel_noAuthNoPriv, match_exact,
				  viewsForPermission[0], viewsForPermission[1],
                                  viewsForPermission[2], storageType_nonVolatile );

    }

    // initialize security information
    map<string, bool> const &vacmContextEntries = Config::getInstance().getVacmContextEntries();
    for( map<string, bool>::const_iterator iter = vacmContextEntries.begin();
         iter != vacmContextEntries.end();
         ++iter )
    {
        mVacm->addNewContext( iter->first.c_str() );
    }

    // Add new entries to the SecurityToGroupTable.
    // Used to determine the group a given SecurityName belongs to. 
    // User "new" of the USM belongs to newGroup
    vector<VacmGroupEntry> const &vacmGroupEntries = Config::getInstance().getVacmGroupEntries();
    for( vector<VacmGroupEntry>::const_iterator iter = vacmGroupEntries.begin();
         iter != vacmGroupEntries.end();
         ++iter )
    {
        for( vector<string>::const_iterator jter = iter->SecurityNames.begin();
             jter != iter->SecurityNames.end();
             ++jter )
        {
            mVacm->addNewGroup( iter->SecurityModel, jter->c_str(),
                                iter->GroupName.c_str(), iter->StorageType );
        }
    }

    // Set access rights of groups.
    // The group "newGroup" (when using the USM with a security
    // level >= noAuthNoPriv within context "") would have full access  
    // (read, write, notify) to all objects in view "newView". 
    vector<VacmAccessEntry> const &vacmAccessEntries = Config::getInstance().getVacmAccessEntries();
    for( vector<VacmAccessEntry>::const_iterator iter = vacmAccessEntries.begin();
         iter != vacmAccessEntries.end();
         ++iter )
    {
        mVacm->addNewAccessEntry( iter->GroupName.c_str(), iter->Context.c_str(),
                                  iter->SecurityModel, iter->SecurityLevel,
                                  iter->Match, iter->ReadView.c_str(),
                                  iter->WriteView.c_str(), iter->NotifyView.c_str(),
                                  iter->StorageType );
    }

    // Defining Views
    // View "v1ReadView" includes all objects starting with "1.3".
    // If the ith bit of the mask is not set (0), then also all objects
    // which have a different subid at position i are included in the 
    // view.
    // For example: Oid "6.5.4.3.2.1", Mask(binary) 110111 
    //              Then all objects with Oid with "6.5.<?>.3.2.1" 
    //              are included in the view, whereas <?> may be any
    //              natural number.
    vector<VacmViewEntry> const &vacmViewEntries = Config::getInstance().getVacmViewEntries();
    for( vector<VacmViewEntry>::const_iterator iter = vacmViewEntries.begin();
         iter != vacmViewEntries.end();
         ++iter )
    {
        mVacm->addNewView( iter->ViewName.c_str(), iter->SubTree.c_str(),
                           iter->Mask.c_str(), iter->ViewType, iter->StorageType );
    }
}

void
Agent::Init()
{
    mMib->add(new sysGroup( "Smart-SNMPd, Agent++ based snmpd version " SMART_SNMP_VERSION_STRING, 
                            SM_SMART_SNMPD_MIB "." SMART_SNMP_VERSION_STRING, 10 ) ); 
    mMib->add(new snmpGroup());
    mMib->add(new snmp_target_mib());
    mMib->add(new snmp_notification_mib());

    for( vector<MibModule *>::size_type i = 0; i < mMibModules.size(); ++i )
    {
        MibModule *modInfo = mMibModules[i];
        if( !modInfo->RegisterMibs( *mMib ) )
        {
            LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
            LOG("Agent::~Agent(): mibs module registering failed at (index)");
            LOG(i);
            LOG_END;
        }
    }

    UserInit();

#ifdef AGENTPP_USE_THREAD_POOL
    int numberOfJobThreads = Config::getInstance().getNumberOfJobThreads();
    if( numberOfJobThreads > 0 )
    {
        QueuedThreadPool *tp = new QueuedThreadPool(numberOfJobThreads);
        mMib->set_thread_pool(tp);
        tp->start();
    }
#endif
    mMib->init();
#ifdef AGENTPP_USE_THREAD_POOL
    if( numberOfJobThreads <= 0 )
    {
        mMib->delete_thread_pool();
    }
#endif
    mReqList->set_snmp(mSnmp);

    VacmInit();
}

void
Agent::Run()
{
    LOG_BEGIN(loggerModuleName, EVENT_LOG | 1);
    LOG("Agent::Run(): starting run loop ...");
    LOG_END;

#if 0
    Vbx* vbs = 0;
    coldStartOid coldOid;
    NotificationOriginator no;
    UdpAddress dest("127.0.0.1/162");
    no.add_v1_trap_destination(dest, "defaultV1Trap", "v1trap", "public");
    no.generate(vbs, 0, coldOid, "", "");
#endif

    mRunning = true;
    time_t timeout_start = 0;

    while( mRunning )
    {
        if( timeout_start )
        {
            if( !mReqList->is_empty() && ( ( time(NULL) - timeout_start ) < 2 ) )
            {
                sched_yield(); // give (pool-)threads up to 2 seconds to answer requests
            }
            else
            {
                mRunning = false;
            }
        }
        else
        {
            Request *req = mReqList->receive(2);

            if( req )
            {
                LOG_BEGIN( loggerModuleName, DEBUG_LOG | 14 );
                LOG( "Got request with (id)" );
                LOG( req->get_pdu()->get_request_id() );
                LOG_END;

                mMib->process_request( req );
            }
            else
            {
                LOG_BEGIN( loggerModuleName, DEBUG_LOG | 14 );
                LOG( "Got no request from request list, calling mMib->cleanup()" );
                LOG_END;
                mMib->cleanup();
            }

            if( ( mSignal == SIGTERM ) || ( mSignal == SIGINT ) )
                timeout_start = time(0);
        }

        if( mSignal == SIGQUIT )
            mRunning = false;
    }
}

}
