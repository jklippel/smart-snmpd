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
#include <smart-snmpd/mibs/statgrab/datasourcehostinfo.h>
#include <smart-snmpd/mibs/statgrab/mibhostinfo.h>

#include <statgrab.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.hostinfo";

static DataSourceHostInfo *instance = NULL;

DataSourceHostInfo &
DataSourceHostInfo::getInstance()
{
    if( !instance )
        instance = new DataSourceHostInfo();

    return *instance;
}

void
DataSourceHostInfo::destroyInstance()
{
    DataSourceHostInfo *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

MibObject *
DataSourceHostInfo::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_HOST_INFO, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

#if 0
bool
DataSourceHostInfo::initMibObj()
{
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_HOST_INFO, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_HOSTNAME, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_OS_NAME, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_OS_RELEASE, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_OS_VERSION, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_PLATFORM, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_HOST_UPTIME, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_BITWIDTH, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_VIRTUALIZED, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_COUNT_MIN, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_COUNT_MAX, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_COUNT_CURRENT, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );

    return DataSourceStatgrab::initMibObj();
}
#endif

bool
DataSourceHostInfo::updateMibObj()
{
    sg_host_info *host_info = sg_get_host_info();
    if( !host_info )
    {
        report_sg_error( "DataSourceHostInfo::updateMibObj", "sg_get_host_info() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceHostInfo::updateMibObj: sg_get_host_info()");
    LOG_END;

    ThreadSynchronize guard(*this);
#if 0
    rc = mMibObj->set_value( SM_HOSTNAME, OctetStr(host_info->os_name) );
    status &= rc;
    rc = mMibObj->set_value( SM_OS_NAME, OctetStr(host_info->os_release) );
    status &= rc;
    rc = mMibObj->set_value( SM_OS_RELEASE, OctetStr(host_info->os_version) );
    status &= rc;
    rc = mMibObj->set_value( SM_OS_VERSION, OctetStr(host_info->platform) );
    status &= rc;
    rc = mMibObj->set_value( SM_PLATFORM, OctetStr(host_info->hostname) );
    status &= rc;
    rc = mMibObj->set_value( SM_LAST_UPDATE_HOST_INFO, Counter64(host_info->uptime) );
    status &= rc;

    rc = mMibObj->set_value( SM_LAST_UPDATE_HOST_INFO, Counter64( time(NULL) ) );
    status &= rc;
#else
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();
    SmartSnmpdHostInfoMib smHostInfoMib( cntMgr );
    smHostInfoMib.setHostInfo( *host_info );
    smHostInfoMib.setUpdateTimestamp( host_info->systime );

    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
