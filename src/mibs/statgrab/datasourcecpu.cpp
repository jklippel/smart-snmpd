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
#include <smart-snmpd/mibs/statgrab/datasourcecpu.h>
#include <smart-snmpd/mibs/statgrab/mibcpu.h>
#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.cpu";

sg_cpu_stats
calc_diff<sg_cpu_stats>::operator () (sg_cpu_stats const &aComperator, sg_cpu_stats const &aMostRecent) const
{
    sg_cpu_stats result;

    result.user = aMostRecent.user - aComperator.user;
    result.kernel = aMostRecent.kernel - aComperator.kernel;
    result.idle = aMostRecent.idle - aComperator.idle;
    result.iowait = aMostRecent.iowait - aComperator.iowait;
    result.swap = aMostRecent.swap - aComperator.swap;
    result.nice = aMostRecent.nice - aComperator.nice;
    result.total = aMostRecent.total - aComperator.total;

    result.context_switches  = aMostRecent.context_switches - aComperator.context_switches;
    result.voluntary_context_switches  = aMostRecent.voluntary_context_switches - aComperator.voluntary_context_switches;
    result.involuntary_context_switches  = aMostRecent.involuntary_context_switches - aComperator.involuntary_context_switches;

    result.interrupts  = aMostRecent.interrupts - aComperator.interrupts;
    result.soft_interrupts  = aMostRecent.soft_interrupts - aComperator.soft_interrupts;

    result.systime = aMostRecent.systime - aComperator.systime;

    return result;
}

static DataSourceCPU *instance = NULL;

DataSourceCPU &
DataSourceCPU::getInstance()
{
    if( !instance )
        instance = new DataSourceCPU();

    return *instance;
}

void
DataSourceCPU::destroyInstance()
{
    DataSourceCPU *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

MibObject *
DataSourceCPU::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_CPU_USAGE, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

bool
DataSourceCPU::initMibObj()
{
    setupHistory(*mMibObj);

#if 0
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_CPU_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );

    // SM_CPU_TOTAL
    mMibObj->add( new MibLeaf( SM_CPU_USER_TIME_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_KERNEL_TIME_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_IDLE_TIME_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_WAIT_TIME_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_SWAP_TIME_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_NICE_TIME_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_TOTAL_TIME_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_CONTEXT_SWITCHES_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_INVOL_CTX_SWITCHES_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_VOLUNTARY_CTX_SWITCHES_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_INTERRUPTS_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_SOFT_INTERRUPTS_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );

    // provide interval mib only if have interval configured
    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        initMibObjInterval();
    }
#endif

    return DataSourceStatgrab::initMibObj();
}

#if 0
bool
DataSourceCPU::initMibObjInterval()
{
    mMibObj->add( new MibLeaf( SM_CPU_INTERVAL_FROM, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_INTERVAL_UNTIL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    // SM_CPU_INTERVAL
    mMibObj->add( new MibLeaf( SM_CPU_USER_TIME_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_KERNEL_TIME_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_IDLE_TIME_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_WAIT_TIME_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_SWAP_TIME_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_NICE_TIME_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_TOTAL_TIME_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_CONTEXT_SWITCHES_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_INVOL_CTX_SWITCHES_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_VOLUNTARY_CTX_SWITCHES_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_INTERRUPTS_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CPU_SOFT_INTERRUPTS_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );

    return true;
}
#endif

bool
DataSourceCPU::checkMibObjConfig( Mib &mainMibCtrl )
{
    bool rc = DataSourceStatgrab::checkMibObjConfig(mainMibCtrl); // includes mMibObj->updateConfig();

    if( mMibObj )
    {
        setupHistory(*mMibObj);
#if 0
        bool needInterval = mMibObj->getConfig().MostRecentIntervalTime != 0;

        if( needInterval && mMibObj->get(SM_CPU_INTERVAL) == NULL )
        {
            // previously without interval - now with
            initMibObjInterval();
        }
        else if( mMibObj->get(SM_CPU_INTERVAL) != NULL && !needInterval )
        {
            // previously with interval - now without
            delete mMibObj->remove( SM_CPU_INTERVAL_FROM );
            delete mMibObj->remove( SM_CPU_INTERVAL_UNTIL );
            delete mMibObj->remove( SM_CPU_INTERVAL );
        }
#endif
    }

    return rc;
}

bool
DataSourceCPU::updateMibObj()
{
    sg_cpu_stats *cpu_stats = sg_get_cpu_stats();
    if( !cpu_stats )
    {
        report_sg_error( "DataSourceCPU::updateMibObj", "sg_get_cpu_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceCPU::updateMibObj: sg_get_cpu_stats()");
    LOG_END;

    ThreadSynchronize guard(*this);
#if 0
    bool rc;

    rc = mMibObj->set_value( SM_CPU_USER_TIME_TOTAL, Counter64(cpu_stats->user) );
    status &= rc;
    rc = mMibObj->set_value( SM_CPU_KERNEL_TIME_TOTAL, Counter64(cpu_stats->kernel) );
    status &= rc;
    rc = mMibObj->set_value( SM_CPU_IDLE_TIME_TOTAL, Counter64(cpu_stats->idle) );
    status &= rc;
    rc = mMibObj->set_value( SM_CPU_WAIT_TIME_TOTAL, Counter64(cpu_stats->iowait) );
    status &= rc;
    rc = mMibObj->set_value( SM_CPU_SWAP_TIME_TOTAL, Counter64(cpu_stats->swap) );
    status &= rc;
    rc = mMibObj->set_value( SM_CPU_NICE_TIME_TOTAL, Counter64(cpu_stats->nice) );
    status &= rc;
    rc = mMibObj->set_value( SM_CPU_TOTAL_TIME_TOTAL, Counter64(cpu_stats->total) );
    status &= rc;

    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        sg_cpu_stats const &cpu_diff = diff( *cpu_stats );

        rc = mMibObj->set_value( SM_CPU_INTERVAL_FROM, Counter64(cpu_stats->systime - cpu_diff.systime) );
        status &= rc;
        rc = mMibObj->set_value( SM_CPU_INTERVAL_UNTIL, Counter64(cpu_stats->systime) );
        status &= rc;

        rc = mMibObj->set_value( SM_CPU_USER_TIME_INTERVAL, Counter64(cpu_diff.user) );
        status &= rc;
        rc = mMibObj->set_value( SM_CPU_KERNEL_TIME_INTERVAL, Counter64(cpu_diff.kernel) );
        status &= rc;
        rc = mMibObj->set_value( SM_CPU_IDLE_TIME_INTERVAL, Counter64(cpu_diff.idle) );
        status &= rc;
        rc = mMibObj->set_value( SM_CPU_WAIT_TIME_INTERVAL, Counter64(cpu_diff.iowait) );
        status &= rc;
        rc = mMibObj->set_value( SM_CPU_SWAP_TIME_INTERVAL, Counter64(cpu_diff.swap) );
        status &= rc;
        rc = mMibObj->set_value( SM_CPU_NICE_TIME_INTERVAL, Counter64(cpu_diff.nice) );
        status &= rc;
        rc = mMibObj->set_value( SM_CPU_TOTAL_TIME_INTERVAL, Counter64(cpu_diff.total) );
        status &= rc;
    }

    rc = mMibObj->set_value( SM_LAST_UPDATE_CPU_USAGE, Counter64(cpu_stats->systime) );
    status &= rc;
#else
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();
    SmartSnmpdCpuMib smCpuMib( cntMgr );
    smCpuMib.setTotalCpuStats( *cpu_stats );

    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        sg_cpu_stats const &cpu_diff = diff( *cpu_stats );
        smCpuMib.setIntervalCpuStats( cpu_stats->systime - cpu_diff.systime, cpu_stats->systime, cpu_diff );
    }

    smCpuMib.setUpdateTimestamp( cpu_stats->systime );

    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
