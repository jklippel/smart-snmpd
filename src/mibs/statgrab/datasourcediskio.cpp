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
#include <smart-snmpd/mibs/statgrab/datasourcediskio.h>
#include <smart-snmpd/mibs/statgrab/mibdiskio.h>

#include <statgrab.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.diskio";

sg_disk_io_stats *
calc_diff<sg_disk_io_stats *>::operator () (sg_disk_io_stats * const &aComperator, sg_disk_io_stats * const &aMostRecent) const
{
    sg_disk_io_stats *result = sg_get_disk_io_stats_diff_between( aMostRecent, aComperator, NULL );
    return result;
}

/**
 * specialization for sg_disk_io_stats vectors
 *
 * @param t - pointer to the first element of the vector to be freed
 */
template<>
void
DataDiff< sg_disk_io_stats * >::freeItem( sg_disk_io_stats * &t )
{
    sg_free_disk_io_stats( t );
}

static DataSourceDiskIO *instance = NULL;

DataSourceDiskIO &
DataSourceDiskIO::getInstance()
{
    if( !instance )
        instance = new DataSourceDiskIO();

    return *instance;
}

void
DataSourceDiskIO::destroyInstance()
{
    DataSourceDiskIO *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

#if 0
static const index_info indDiskIOTable[1] = {
    {sNMP_SYNTAX_INT, FALSE, 1, 1}
};
#endif

MibObject *
DataSourceDiskIO::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_DISK_IO_STATUS, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

#if 0
MibTable *
DataSourceDiskIO::getDiskIOTable(NS_AGENT Oidx const &baseOid) const
{
    MibTable *newDiskIoTable = new MibTable( baseOid, indDiskIOTable, 1 ); // XXX what does it mean?

    newDiskIoTable->add_col( new MibLeaf( "1", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_DISK_IO_INDEX
    newDiskIoTable->add_col( new MibLeaf( "2", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_DISK_IO_DISKNAME
    newDiskIoTable->add_col( new MibLeaf( "3", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_DISK_IO_READ_BYTES
    newDiskIoTable->add_col( new MibLeaf( "4", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_DISK_IO_WRIT_BYTES

    return newDiskIoTable;
}
#endif

bool
DataSourceDiskIO::initMibObj()
{
#if 0
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_DISK_IO_STATUS, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_DISK_IO_COUNT, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( getDiskIOTable( SM_DISK_IO_ENTRY ) );
#endif

    // provide interval mib only if have interval configured
    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        setupHistory(*mMibObj);
#if 0
        initMibObjInterval();
#endif
    }

    return DataSourceStatgrab::initMibObj();
}

#if 0
bool
DataSourceDiskIO::initMibObjInterval()
{
    mMibObj->add( new MibLeaf( SM_DISK_IO_INTERVAL_FROM, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_DISK_IO_INTERVAL_UNTIL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( getDiskIOTable( SM_DISK_IO_INTERVAL_ENTRY ) );

    return true;
}
#endif

bool
DataSourceDiskIO::checkMibObjConfig( Mib &mainMibCtrl )
{
    bool rc = DataSourceStatgrab::checkMibObjConfig(mainMibCtrl); // includes mMibObj->updateConfig();

    if( mMibObj )
    {
        setupHistory(*mMibObj);
#if 0
        bool needInterval = mMibObj->getConfig().MostRecentIntervalTime != 0;

        if( needInterval && mMibObj->get(SM_DISK_IO_INTERVAL_TABLE) == NULL )
        {
            // previously without interval - now with
            initMibObjInterval();
        }
        else if( mMibObj->get(SM_DISK_IO_INTERVAL_TABLE) != NULL && !needInterval )
        {
            // previously with interval - now without
            delete mMibObj->remove( SM_DISK_IO_INTERVAL_FROM );
            delete mMibObj->remove( SM_DISK_IO_INTERVAL_UNTIL );
            delete mMibObj->remove( SM_DISK_IO_INTERVAL_TABLE );
        }
#endif
    }

    return rc;
}

bool
DataSourceDiskIO::updateMibObj()
{
    size_t entries = 0;
    sg_disk_io_stats *disk_io_stats = mMibObj->getConfig().MostRecentIntervalTime ? sg_get_disk_io_stats_r(&entries) : sg_get_disk_io_stats(&entries);
    if( !disk_io_stats )
    {
        report_sg_error( "DataSourceDiskIO::updateMibObj", "sg_get_disk_io_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceDiskIO::updateMibObj: sg_get_disk_io_stats()");
    LOG_END;

    ThreadSynchronize guard(*this);
    time_t now = time(NULL);
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();

    SmartSnmpdDiskIoMib smDiskIoMib( cntMgr );

    for( size_t i = 0; i < entries; ++i )
    {
        smDiskIoMib.addTotalRow( disk_io_stats[i] );
        now = disk_io_stats[i].systime;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceDiskIO::updateMibObj: added (entries) rows to total stats");
    LOG(entries);
    LOG_END;

    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        sg_disk_io_stats * disk_io_diff = diff( disk_io_stats );
        size_t diff_entries = sg_get_nelements( disk_io_diff );
        time_t from = 0;

        for( size_t i = 0; i < diff_entries; ++i )
        {
            smDiskIoMib.addIntervalRow( disk_io_diff[i] );
            from = now - disk_io_diff[i].systime;
        }

        LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
        LOG("DataSourceDiskIO::updateMibObj: added (entries) rows to interval stats");
        LOG(entries);
        LOG_END;

        smDiskIoMib.setIntervalSpec( from, now );
    }

    smDiskIoMib.setCount( entries );
    smDiskIoMib.setUpdateTimestamp( now );

    mMibObj->commitContentUpdate();

    return true;
}

}
