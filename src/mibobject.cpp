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

#include <smart-snmpd/oids.h>
#include <smart-snmpd/datasource.h>
#include <smart-snmpd/mibobject.h>

#include <map>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.mibobject";

class NameOidMap
    : public map<string, Oidx>
{
public:
    NameOidMap()
        : map<string, Oidx>()
    {
        insert( make_pair( "DaemonStatus", SM_DAEMON_STATUS ) );
        insert( make_pair( "HostInfo", SM_HOST_INFO ) );
        insert( make_pair( "CpuUsage", SM_CPU_USAGE ) );
        insert( make_pair( "MemoryUsage", SM_MEMORY_USAGE ) );
        insert( make_pair( "SystemLoad", SM_SYSTEM_LOAD ) );
        insert( make_pair( "UserLogins", SM_USER_LOGIN_STATUS ) );
        insert( make_pair( "ProcessStatus", SM_PROCESS_STATUS ) );
        insert( make_pair( "FileSystemUsage", SM_FILE_SYSTEM_USAGE ) );
        insert( make_pair( "DiskIO", SM_DISK_IO_STATUS ) );
        insert( make_pair( "NetworkIO", SM_NETWORK_IO_STATUS ) );
        insert( make_pair( "SwapIO", SM_SWAP_IO_STATUS ) );
        insert( make_pair( "ExternalCommands", SM_EXTERNAL_COMMANDS ) );
        insert( make_pair( "AppMonitoring", SM_APP_MONITORING ) );
    }
};

static NameOidMap MapByName;

static const Oidx emptyOid;

map<string, Oidx> const &
getMapByName()
{
    return MapByName;
}

Oidx const &
FindOidForMibName(string const &aMibName)
{
    map<string, Oidx>::const_iterator iter = MapByName.find(aMibName);

    if( iter == MapByName.end() )
        return emptyOid;

    return iter->second;
}

MibObject::MibObject(const Oidx &anOid, DataSource &aDataSource)
    : MibComplexEntry( anOid, READONLY )
    , mConfig( Config::getInstance().getMibObjectConfig(anOid.get_printable() ) )
    , mDataSource( aDataSource )
    , mContentMgr( anOid )
    , mShadowContentMgr( anOid )
{
    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("MibObject::MibObject fresh initialized (Oid)");
    LOG(anOid.get_printable());
    LOG_END;
}

MibObject::MibObject(MibObject const &aRef)
    : MibComplexEntry(aRef)
    , mConfig( aRef.mConfig )
    , mDataSource( aRef.mDataSource )
    , mContentMgr( aRef.mContentMgr )
    , mShadowContentMgr( oid )
{
    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("MibObject::MibObject copy constructor (Oid)");
    LOG(aRef.key()->get_printable());
    LOG_END;
}

void
MibObject::updateConfig()
{
    ThreadSynchronize guard(*this);
    mConfig = Config::getInstance().getMibObjectConfig( oid.get_printable() );
}

void
MibObject::update(Request *)
{
    if( !mConfig.AsyncUpdate )
    {
        time_t now = time(NULL);

        LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
        LOG("MibObject::update (Oid)(CacheTime)(LastUpdate)(now)");
        LOG(key()->get_printable());
        LOG(mConfig.CacheTime);
        LOG(mContentMgr.mLastUpdate);
        LOG(now);
        LOG_END;

        if( ( ( mContentMgr.mLastUpdate + mConfig.CacheTime ) < now ) )
        {
            if( !mDataSource.updateMibObj() )
            {
                // report error
            }
        }
    }

    return;
}

}
