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
#include <smart-snmpd/mibs/statgrab/datasourcedaemonstatus.h>
#include <smart-snmpd/mibs/statgrab/mibdaemonstatus.h>
#include <smart-snmpd/mibobject.h>

#include <agent_pp/snmp_textual_conventions.h>
#include <agent_pp/snmp_counters.h>

#include <cmath>

namespace SmartSnmpd
{

#ifndef HAVE_SQRT_LONG_LONG
static long long sqrt(long long ll)
{
    double d = ll;
    return (long long)( std::sqrt( d ) );
}

static unsigned long long sqrt(unsigned long long ull)
{
    double d = ull;
    return (unsigned long long)( std::sqrt( d ) );
}
#endif

void
DataSourceDaemonStatus::MemoryInfo::updateMemoryInfo( unsigned long long curMeasuredValue, unsigned long measureCount )
{
    if( mMaxValue < curMeasuredValue )
    {
        mMaxValue = curMeasuredValue;
        if( mMaxViolations < ULONG_MAX )
            ++mMaxViolations;
    }

    // here begins mean computing (standard deviation)

    unsigned long long delta;
    delta = curMeasuredValue - mMeanValue;
    mMeanValue = mMeanValue + (unsigned long long)(delta/measureCount);
    mMeanSqrValue += delta * (curMeasuredValue - mMeanValue); // new mean!

    mMeanErrDistance = mMeanSqrValue/measureCount;
    mMeanErrDistance = (unsigned long long)( sqrt(mMeanErrDistance) );

    if( curMeasuredValue > (mMeanValue + mMeanErrDistance) )
    {
        if( mMeanViolations < ULONG_MAX )
            ++mMeanViolations;
    }
    else
    {
        if( mMeanViolations > 0 )
            --mMeanViolations;
    }
}

static const char * const loggerModuleName = "smartsnmpd.datasource.daemonstatus";

static DataSourceDaemonStatus *instance;

DataSourceDaemonStatus &
DataSourceDaemonStatus::getInstance()
{
    if( !instance )
        instance = new DataSourceDaemonStatus();

    return *instance;
}

void
DataSourceDaemonStatus::destroyInstance()
{
    DataSourceDaemonStatus *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

MibObject *
DataSourceDaemonStatus::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_DAEMON_STATUS, *this );
            initMibObj();
        }
    }

    return mMibObj;
}



sg_process_stats *
DataSourceDaemonStatus::findCurrentProcessStats()
{
    size_t entries = 0;
    sg_process_stats *process_stats = sg_get_process_stats(&entries);
    if( !process_stats )
    {
        report_sg_error( "DataSourceDaemonStatus::findCurrentProcessStats", "sg_get_process_stats() failed" );
        return NULL;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceProcess::findCurrentProcessStats: sg_get_process_stats()");
    LOG_END;

#ifdef WIN32
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif
    size_t i;
    for( i = 0; i < entries; ++i )
    {
        if( process_stats[i].pid != pid )
            continue;

        return &process_stats[i];
    }

    return NULL;
}

bool
DataSourceDaemonStatus::initMibObj()
{
#if 0
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_DAEMON_STATUS, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_INITIAL_VIRTUAL_MEMORY_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_INITIAL_RESIDENT_MEMORY_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_MEAN_VIRTUAL_MEMORY_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_MEAN_RESIDENT_MEMORY_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CURRENT_VIRTUAL_MEMORY_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CURRENT_RESIDENT_MEMORY_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CURRENT_VIRTUAL_MEMORY_INCREASES, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CURRENT_RESIDENT_MEMORY_INCREASES, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_HANDLED_REQUESTS, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_DAEMON_UPTIME, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_DAEMON_CPUTIME, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_MEAN_VIRTUAL_MEMORY_VIOLATIONS, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_MEAN_RESIDENT_MEMORY_VIOLATIONS, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_MEAN_VIRTUAL_MEMORY_ERR_OK, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_MEAN_RESIDENT_MEMORY_ERR_OK, READONLY, new Counter64(), VMODE_DEFAULT ) );
#endif
    sg_process_stats *curProcessStats = findCurrentProcessStats();
    if( !curProcessStats )
    {
        return false;
    }

    mVirtualMemoryInfo.init( curProcessStats->proc_size );
    mResidentMemoryInfo.init( curProcessStats->proc_resident );
    mMeasureCount = 1;

    return DataSourceStatgrab::initMibObj();
}

bool
DataSourceDaemonStatus::updateMibObj()
{
    sg_process_stats *curProcessStats = findCurrentProcessStats();
    if( !curProcessStats )
    {
        return false;
    }

    ThreadSynchronize guard(*this);

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceProcess::updateMibObj: sg_get_process_stats()");
    LOG_END;

    mVirtualMemoryInfo.updateMemoryInfo( curProcessStats->proc_size, mMeasureCount );
    mResidentMemoryInfo.updateMemoryInfo( curProcessStats->proc_resident, mMeasureCount );
    ++mMeasureCount;

    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();

    SmartSnmpdDaemonStatusMib smDaemonMib( cntMgr );
    smDaemonMib.setVirtualMemoryStatus( mVirtualMemoryInfo );
    smDaemonMib.setResidentMemoryStatus( mResidentMemoryInfo );
    smDaemonMib.setDaemonUptime( curProcessStats->systime - curProcessStats->start_time );
    smDaemonMib.setDaemonCpuTime( curProcessStats->time_spent );

    // should be equal (in code) to MibIIsnmpCounters::outGetResponses() - but will probably change in later version of agent++
    unsigned long long handled_requests  = MibIIsnmpCounters::inGetRequests();
                       handled_requests += MibIIsnmpCounters::inGetNexts();
                       handled_requests += MibIIsnmpCounters::inSetRequests();
    smDaemonMib.setHandledRequests( handled_requests );

    smDaemonMib.setUpdateTimestamp( curProcessStats->systime );

    mMibObj->commitContentUpdate();

    return true;
}

}
