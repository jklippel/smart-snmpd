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
#include <smart-snmpd/mibs/statgrab/datasourceload.h>
#include <smart-snmpd/mibs/statgrab/mibload.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.load";

static DataSourceLoad *instance = NULL;

DataSourceLoad &
DataSourceLoad::getInstance()
{
    if( !instance )
        instance = new DataSourceLoad();

    return *instance;
}

void
DataSourceLoad::destroyInstance()
{
    DataSourceLoad *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

MibObject *
DataSourceLoad::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_SYSTEM_LOAD, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

#if 0
bool
DataSourceLoad::initMibObj()
{
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_SYSTEM_LOAD, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD1_REAL_FLOAT, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD5_REAL_FLOAT, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD15_REAL_FLOAT, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD1_REAL_INTEGER, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD5_REAL_INTEGER, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD15_REAL_INTEGER, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD1_NORM_FLOAT, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD5_NORM_FLOAT, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD15_NORM_FLOAT, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD1_NORM_INTEGER, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD5_NORM_INTEGER, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SYSTEM_LOAD15_NORM_INTEGER, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );

    return DataSourceStatgrab::initMibObj();
}
#endif

bool
DataSourceLoad::updateMibObj()
{
    sg_load_stats *load_stats = sg_get_load_stats();
    if( !load_stats )
    {
        report_sg_error( "DataSourceLoad::updateMibObj", "sg_get_load_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceLoad::updateMibObj: sg_get_load_stats()");
    LOG_END;

    ThreadSynchronize guard(*this);

#if 0
    rc = mMibObj->set_value( SM_SYSTEM_LOAD1_REAL_INTEGER, SnmpUInt32( (unsigned long)(load_stats->min1 * 100) ) );
    status &= rc;
    rc = mMibObj->set_value( SM_SYSTEM_LOAD5_REAL_INTEGER, SnmpUInt32( (unsigned long)(load_stats->min5 * 100) ) );
    status &= rc;
    rc = mMibObj->set_value( SM_SYSTEM_LOAD15_REAL_INTEGER, SnmpUInt32( (unsigned long)(load_stats->min15 * 100) ) );
    status &= rc;

    char buf[16];
    snprintf( buf, sizeof(buf), "%0.02f", load_stats->min1 );
    rc = mMibObj->set_value( SM_SYSTEM_LOAD1_REAL_FLOAT, OctetStr(buf) );
    status &= rc;
    snprintf( buf, sizeof(buf), "%0.02f", load_stats->min5 );
    rc = mMibObj->set_value( SM_SYSTEM_LOAD5_REAL_FLOAT, OctetStr(buf) );
    status &= rc;
    snprintf( buf, sizeof(buf), "%0.02f", load_stats->min15 );
    rc = mMibObj->set_value( SM_SYSTEM_LOAD15_REAL_FLOAT, OctetStr(buf) );
    status &= rc;

    rc = mMibObj->set_value( SM_LAST_UPDATE_SYSTEM_LOAD, Counter64( time(NULL) ) );
    status &= rc;
#else
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();
    SmartSnmpdLoadMib smLoadMib( cntMgr );
    smLoadMib.setRealLoad( *load_stats );
    // XXX normalized load must be calculated separately,
    //     by collecting load statistics really often and
    //     either normalize min1 locally and recompute min5/min15 based on a normalized load1
    //     or collect load per cpu
    // OTOH it migth be more reasonable to provide a statistic about the
    //      highest number of cpu's of the last 15 minutes
    smLoadMib.setUpdateTimestamp( load_stats->systime );

    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
