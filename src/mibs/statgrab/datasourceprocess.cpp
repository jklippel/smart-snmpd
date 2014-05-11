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
#include <smart-snmpd/mibs/statgrab/datasourceprocess.h>
#include <smart-snmpd/mibs/statgrab/mibprocess.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.process";

static DataSourceProcess *instance = NULL;

DataSourceProcess &
DataSourceProcess::getInstance()
{
    if( !instance )
        instance = new DataSourceProcess();

    return *instance;
}

void
DataSourceProcess::destroyInstance()
{
    DataSourceProcess *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

#if 0
static const index_info indProcessTable[1] = {
    {sNMP_SYNTAX_INT, FALSE, 1, 1}
};
#endif

MibObject *
DataSourceProcess::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_PROCESS_STATUS, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

#if 0
MibTable *
DataSourceProcess::getProcessTable() const
{
    MibTable *newProcTable = new MibTable( SM_PROCESS_ENTRY, indProcessTable, 1 ); // XXX what does it mean?

    newProcTable->add_col( new MibLeaf( "1", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_PROCESS_PID
    newProcTable->add_col( new MibLeaf( "2", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_PROCESS_PPID
    newProcTable->add_col( new MibLeaf( "3", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_PROCESS_EXE
    newProcTable->add_col( new MibLeaf( "4", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_PROCESS_ARGS
    newProcTable->add_col( new MibLeaf( "5", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_PROCESS_STATE
    newProcTable->add_col( new MibLeaf( "6", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_PROCESS_VSIZE
    newProcTable->add_col( new MibLeaf( "7", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_PROCESS_RSIZE
    newProcTable->add_col( new MibLeaf( "8", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_PROCESS_START_TIME
    newProcTable->add_col( new MibLeaf( "9", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_PROCESS_CPU_TIME
    newProcTable->add_col( new MibLeaf( "10", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_PROCESS_USERID
    newProcTable->add_col( new MibLeaf( "11", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_PROCESS_USERNAME
    newProcTable->add_col( new MibLeaf( "12", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_PROCESS_GROUPID
    newProcTable->add_col( new MibLeaf( "13", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_PROCESS_GROUPNAME
    newProcTable->add_col( new MibLeaf( "14", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_PROCESS_EFFECTIVE_USERID
    newProcTable->add_col( new MibLeaf( "15", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_PROCESS_EFFECTIVE_USERNAME
    newProcTable->add_col( new MibLeaf( "16", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_PROCESS_EFFECTIVE_GROUPID
    newProcTable->add_col( new MibLeaf( "17", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_PROCESS_EFFECTIVE_GROUPNAME
    newProcTable->add_col( new MibLeaf( "18", READONLY, new SnmpInt32(), VMODE_DEFAULT ) ); // SM_PROCESS_NICE
    newProcTable->add_col( new MibLeaf( "19", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_PROCESS_CPU_PERCENT

    return newProcTable;
}

bool
DataSourceProcess::initMibObj()
{
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_PROCESS_STATUS, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_PROCESS_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_PROCESS_RUNNING, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_PROCESS_SLEEPING, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_PROCESS_STOPPED, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_PROCESS_ZOMBIE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( getProcessTable() );

    return DataSourceStatgrab::initMibObj();
}
#endif

bool
DataSourceProcess::updateMibObj()
{
    size_t entries = 0;
    sg_process_stats *process_stats = sg_get_process_stats_r(&entries);
    if( !process_stats )
    {
        report_sg_error( "DataSourceProcess::updateMibObj", "sg_get_process_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceProcess::updateMibObj: sg_get_process_stats()");
    LOG_END;

    sg_process_count proc_counts;
    memset( &proc_counts, 0, sizeof(proc_counts) );

    ThreadSynchronize guard(*this);
#if 0
    MibTable *procTable = getProcessTable();
    for( size_t i = 0; i < entries; ++i )
    {
        MibTableRow *row = procTable->add_row( procTable->get_next_avail_index() );
        row->get_nth(0)->replace_value( new SnmpUInt32( process_stats[i].pid ) );
        row->get_nth(1)->replace_value( new SnmpUInt32( process_stats[i].parent ) );
        if( NULL != process_stats[i].process_name )
            row->get_nth(2)->replace_value( new OctetStr( process_stats[i].process_name ) );
        if( NULL != process_stats[i].proctitle )
            row->get_nth(3)->replace_value( new OctetStr( process_stats[i].proctitle ) );
        row->get_nth(4)->replace_value( new SnmpUInt32( (int)(process_stats[i].state) ) );

        row->get_nth(5)->replace_value( new Counter64(process_stats[i].proc_size) );
        row->get_nth(6)->replace_value( new Counter64(process_stats[i].proc_resident) );

        row->get_nth(7)->replace_value( new Counter64(process_stats[i].start_time) );
        row->get_nth(8)->replace_value( new Counter64(process_stats[i].time_spent) );

        row->get_nth(9)->replace_value( new SnmpUInt32( process_stats[i].uid ) );
        row->get_nth(10)->replace_value( new OctetStr( mSysUserInfo.getusernamebyuid(process_stats[i].uid).c_str() ) );
        row->get_nth(11)->replace_value( new SnmpUInt32( process_stats[i].gid ) );
        row->get_nth(12)->replace_value( new OctetStr( mSysUserInfo.getgroupnamebygid(process_stats[i].gid).c_str() ) );
        row->get_nth(13)->replace_value( new SnmpUInt32( process_stats[i].euid ) );
        row->get_nth(14)->replace_value( new OctetStr( mSysUserInfo.getusernamebyuid(process_stats[i].euid).c_str() ) );
        row->get_nth(15)->replace_value( new SnmpUInt32( process_stats[i].egid ) );
        row->get_nth(16)->replace_value( new OctetStr( mSysUserInfo.getgroupnamebygid(process_stats[i].egid).c_str() ) );

        row->get_nth(17)->replace_value( new SnmpInt32( process_stats[i].nice ) );

        snprintf( buf, sizeof(buf), "%0.02f", process_stats[i].cpu_percent );
        row->get_nth(18)->replace_value( new OctetStr( buf ) );

        switch( process_stats[i].state )
        {
        case SG_PROCESS_STATE_RUNNING:
            ++procCounts.running;
            break;
        case SG_PROCESS_STATE_SLEEPING:
            ++procCounts.sleeping;
            break;
        case SG_PROCESS_STATE_STOPPED:
            ++procCounts.stopped;
            break;
        case SG_PROCESS_STATE_ZOMBIE:
            ++procCounts.zombie;
            break;
        default:
            /* unhandled :) */
            break;
        }
    }

    rc = mMibObj->set_value(procTable);
    status &= rc;
    if( rc )
    {
        rc = mMibObj->set_value( SM_PROCESS_TOTAL, Counter64(entries) );
        status &= rc;
        rc = mMibObj->set_value( SM_PROCESS_RUNNING, Counter64(procCounts.running) );
        status &= rc;
        rc = mMibObj->set_value( SM_PROCESS_SLEEPING, Counter64(procCounts.sleeping) );
        status &= rc;
        rc = mMibObj->set_value( SM_PROCESS_STOPPED, Counter64(procCounts.stopped) );
        status &= rc;
        rc = mMibObj->set_value( SM_PROCESS_ZOMBIE, Counter64(procCounts.zombie) );
        status &= rc;
        rc = mMibObj->set_value( SM_LAST_UPDATE_PROCESS_STATUS, Counter64( time(NULL) ) );
        status &= rc;
    }
#else
    time_t now = time(NULL);
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();

    SmartSnmpdProcessMib smProcessMib( cntMgr );

    for( size_t i = 0; i < entries; ++i )
    {
        smProcessMib.addRow( process_stats[i] );
        now = process_stats[i].systime;

        ++proc_counts.total;

        switch( process_stats[i].state )
        {
        case SG_PROCESS_STATE_RUNNING:
            ++proc_counts.running;
            break;
        case SG_PROCESS_STATE_SLEEPING:
            ++proc_counts.sleeping;
            break;
        case SG_PROCESS_STATE_STOPPED:
            ++proc_counts.stopped;
            break;
        case SG_PROCESS_STATE_ZOMBIE:
            ++proc_counts.zombie;
            break;
        default:
            ++proc_counts.unknown;
            break;
        }
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceProcess::updateMibObj: added (total) (running) (sleeping) (stopped) (zombie) (unknown) rows to total stats");
    LOG(proc_counts.running);
    LOG(proc_counts.sleeping);
    LOG(proc_counts.stopped);
    LOG(proc_counts.zombie);
    LOG(proc_counts.unknown);
    LOG_END;

    smProcessMib.setCounts( proc_counts );
    smProcessMib.setUpdateTimestamp( now );

    mMibObj->commitContentUpdate();
#endif

    sg_free_process_stats( process_stats );

    return true;
}

}
