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
#include <smart-snmpd/mibs/statgrab/datasourcememory.h>
#include <smart-snmpd/mibs/statgrab/mibmemory.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.memory";

static DataSourceMemory *instance = NULL;

DataSourceMemory &
DataSourceMemory::getInstance()
{
    if( !instance )
        instance = new DataSourceMemory();

    return *instance;
}

void
DataSourceMemory::destroyInstance()
{
    DataSourceMemory *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

MibObject *
DataSourceMemory::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_MEMORY_USAGE, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

#if 0
bool
DataSourceMemory::initMibObj()
{
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_MEMORY_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_TOTAL_MEMORY_PHYSICAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_FREE_MEMORY_PHYSICAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_USED_MEMORY_PHYSICAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_CACHE_MEMORY_PHYSICAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_TOTAL_MEMORY_SWAP, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_FREE_MEMORY_SWAP, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_USED_MEMORY_SWAP, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_TOTAL_MEMORY_VIRTUAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_FREE_MEMORY_VIRTUAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_USED_MEMORY_VIRTUAL, READONLY, new Counter64(), VMODE_DEFAULT ) );

    return DataSourceStatgrab::initMibObj();
}
#endif

bool
DataSourceMemory::updateMibObj()
{
    sg_mem_stats *mem_stats = sg_get_mem_stats();
    if( !mem_stats )
    {
        report_sg_error( "DataSourceMemory::updateMibObj", "sg_get_mem_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceMemory::updateMibObj: sg_get_mem_stats()");
    LOG_END;

    sg_swap_stats *swap_stats = sg_get_swap_stats();
    if( !swap_stats )
    {
        report_sg_error( "updateMibObj", "sg_get_swap_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceMemory::updateMibObj: sg_get_swap_stats()");
    LOG_END;

    sg_swap_stats virt_stats;

    virt_stats.total = mem_stats->total + swap_stats->total;
    virt_stats.free = mem_stats->free + swap_stats->free;
    virt_stats.used = mem_stats->used + swap_stats->used;

    ThreadSynchronize guard(*this);
#if 0
    rc = mMibObj->set_value( SM_TOTAL_MEMORY_PHYSICAL, Counter64(mem_stats->total) );
    status &= rc;
    rc = mMibObj->set_value( SM_FREE_MEMORY_PHYSICAL, Counter64(mem_stats->free) );
    status &= rc;
    rc = mMibObj->set_value( SM_USED_MEMORY_PHYSICAL, Counter64(mem_stats->used) );
    status &= rc;
    rc = mMibObj->set_value( SM_CACHE_MEMORY_PHYSICAL, Counter64(mem_stats->cache) );
    status &= rc;

    rc = mMibObj->set_value( SM_TOTAL_MEMORY_SWAP, Counter64(swap_stats->total) );
    status &= rc;
    rc = mMibObj->set_value( SM_FREE_MEMORY_SWAP, Counter64(swap_stats->free) );
    status &= rc;
    rc = mMibObj->set_value( SM_USED_MEMORY_SWAP, Counter64(swap_stats->used) );
    status &= rc;

    rc = mMibObj->set_value( SM_TOTAL_MEMORY_VIRTUAL, Counter64(virt_stats.total) );
    status &= rc;
    rc = mMibObj->set_value( SM_FREE_MEMORY_VIRTUAL, Counter64(virt_stats.free) );
    status &= rc;
    rc = mMibObj->set_value( SM_USED_MEMORY_VIRTUAL, Counter64(virt_stats.used) );
    status &= rc;

    rc = mMibObj->set_value( SM_LAST_UPDATE_MEMORY_USAGE, Counter64( time(NULL) ) );
    status &= rc;
#else
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();
    SmartSnmpdMemoryMib smMemMib( cntMgr );
    smMemMib.setPhysMem( *mem_stats );
    smMemMib.setSwapMem( *swap_stats );
    smMemMib.setVirtMem( virt_stats );
    smMemMib.setUpdateTimestamp( mem_stats->systime );

    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
