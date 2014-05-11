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
#include <smart-snmpd/mibs/statgrab/datasourcenetworkio.h>
#include <smart-snmpd/mibs/statgrab/mibnetworkio.h>

#include <statgrab.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.networkio";

sg_network_io_stats *
calc_diff<sg_network_io_stats *>::operator () (sg_network_io_stats * const &aComperator, sg_network_io_stats * const &aMostRecent) const
{
    sg_network_io_stats *result = sg_get_network_io_stats_diff_between( aMostRecent, aComperator, NULL );
    return result;
}

/**
 * specialization for sg_network_io_stats vectors
 *
 * @param t - pointer to the first element of the vector to be freed
 */
template<>
void
DataDiff< sg_network_io_stats * >::freeItem( sg_network_io_stats * &t )
{
    sg_free_network_io_stats( t );
}

static DataSourceNetworkIO *instance = NULL;

DataSourceNetworkIO &
DataSourceNetworkIO::getInstance()
{
    if( !instance )
        instance = new DataSourceNetworkIO();

    return *instance;
}

void
DataSourceNetworkIO::destroyInstance()
{
    DataSourceNetworkIO *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

#if 0
static const index_info indNetworkIOTable[1] = {
    {sNMP_SYNTAX_INT, FALSE, 1, 1}
};
#endif

MibObject *
DataSourceNetworkIO::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_NETWORK_IO_STATUS, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

#if 0
MibTable *
DataSourceNetworkIO::getNetworkIOTable(NS_AGENT Oidx const &baseOid) const
{
    MibTable *newNetworkIoTable = new MibTable( baseOid, indNetworkIOTable, 1 ); // XXX what does it mean?

    newNetworkIoTable->add_col( new MibLeaf( "1", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_INDEX
    newNetworkIoTable->add_col( new MibLeaf( "2", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_INTERFACE
    newNetworkIoTable->add_col( new MibLeaf( "3", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_TRANSMITTED
    newNetworkIoTable->add_col( new MibLeaf( "4", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_RECEIVED
    newNetworkIoTable->add_col( new MibLeaf( "5", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_INPKTS
    newNetworkIoTable->add_col( new MibLeaf( "6", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_OUTPKTS
    newNetworkIoTable->add_col( new MibLeaf( "7", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_INERRORS
    newNetworkIoTable->add_col( new MibLeaf( "8", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_OUTERRORS
    newNetworkIoTable->add_col( new MibLeaf( "9", READONLY, new Counter64(), VMODE_DEFAULT ) ); // SM_NETWORK_IO_COLLISIONS

    return newNetworkIoTable;
}
#endif

bool
DataSourceNetworkIO::initMibObj()
{
#if 0
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_NETWORK_IO_STATUS, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_NETWORK_IO_COUNT, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( getNetworkIOTable(SM_NETWORK_IO_ENTRY) );

    // provide interval mib only if have interval configured
    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        initMibObjInterval();
    }
#endif

    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        setupHistory(*mMibObj);
    }

    return DataSourceStatgrab::initMibObj();
}

#if 0
bool
DataSourceNetworkIO::initMibObjInterval()
{
    mMibObj->add( new MibLeaf( SM_NETWORK_IO_INTERVAL_FROM, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_NETWORK_IO_INTERVAL_UNTIL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( getNetworkIOTable( SM_NETWORK_IO_INTERVAL_ENTRY ) );

    return true;
}
#endif

bool
DataSourceNetworkIO::checkMibObjConfig( Mib &mainMibCtrl )
{
    bool rc = DataSourceStatgrab::checkMibObjConfig(mainMibCtrl); // includes mMibObj->updateConfig();

    if( mMibObj )
    {
        setupHistory(*mMibObj);
#if 0
        bool needInterval = mMibObj->getConfig().MostRecentIntervalTime != 0;

        if( needInterval && mMibObj->get(SM_NETWORK_IO_INTERVAL_TABLE) == NULL )
        {
            // previously without interval - now with
            initMibObjInterval();
        }
        else if( mMibObj->get(SM_NETWORK_IO_INTERVAL_TABLE) != NULL && !needInterval )
        {
            // previously with interval - now without
            delete mMibObj->remove( SM_NETWORK_IO_INTERVAL_FROM );
            delete mMibObj->remove( SM_NETWORK_IO_INTERVAL_UNTIL );
            delete mMibObj->remove( SM_NETWORK_IO_INTERVAL_TABLE );
        }
#endif
    }

    return rc;
}

bool
DataSourceNetworkIO::updateMibObj()
{
    size_t entries = 0;
    sg_network_io_stats *network_io_stats = mMibObj->getConfig().MostRecentIntervalTime ? sg_get_network_io_stats_r(&entries) : sg_get_network_io_stats(&entries);
    if( !network_io_stats )
    {
        report_sg_error( "DataSourceNetworkIO::updateMibObj", "sg_get_network_io_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceNetworkIO::updateMibObj: sg_get_network_io_stats()");
    LOG_END;

    ThreadSynchronize guard(*this);
    time_t now = time(NULL);

#if 0
    {
        MibTable *networkIoTable = getNetworkIOTable(SM_NETWORK_IO_ENTRY);
        for( size_t i = 0; i < entries; ++i )
        {
            MibTableRow *row = networkIoTable->add_row( networkIoTable->get_next_avail_index() );
            row->get_nth(0)->replace_value( new SnmpUInt32(i) );
            if( NULL != network_io_stats[i].interface_name )
                row->get_nth(1)->replace_value( new OctetStr(network_io_stats[i].interface_name) );
            row->get_nth(2)->replace_value( new Counter64(network_io_stats[i].tx) );
            row->get_nth(3)->replace_value( new Counter64(network_io_stats[i].rx) );
            row->get_nth(4)->replace_value( new Counter64(network_io_stats[i].ipackets) );
            row->get_nth(5)->replace_value( new Counter64(network_io_stats[i].opackets) );
            row->get_nth(6)->replace_value( new Counter64(network_io_stats[i].ierrors) );
            row->get_nth(7)->replace_value( new Counter64(network_io_stats[i].oerrors) );
            row->get_nth(8)->replace_value( new Counter64(network_io_stats[i].collisions) );
            now = network_io_stats[i].systime;
        }
        rc = mMibObj->set_value(networkIoTable);
        status &= rc;
    }

    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        MibTable *networkIoIntervalTable = getNetworkIOTable(SM_NETWORK_IO_INTERVAL_ENTRY);
        sg_network_io_stats * network_io_diff = diff( network_io_stats );
        size_t diff_entries = sg_get_nelements( network_io_diff );
        time_t from = 0;

        for( size_t i = 0; i < diff_entries; ++i )
        {
            MibTableRow *row = networkIoIntervalTable->add_row( networkIoIntervalTable->get_next_avail_index() );
            row->get_nth(0)->replace_value( new SnmpUInt32(i) );
            if( NULL != network_io_diff[i].interface_name )
                row->get_nth(1)->replace_value( new OctetStr(network_io_diff[i].interface_name) );
            row->get_nth(2)->replace_value( new Counter64(network_io_diff[i].tx) );
            row->get_nth(3)->replace_value( new Counter64(network_io_diff[i].rx) );
            row->get_nth(4)->replace_value( new Counter64(network_io_diff[i].ipackets) );
            row->get_nth(5)->replace_value( new Counter64(network_io_diff[i].opackets) );
            row->get_nth(6)->replace_value( new Counter64(network_io_diff[i].ierrors) );
            row->get_nth(7)->replace_value( new Counter64(network_io_diff[i].oerrors) );
            row->get_nth(8)->replace_value( new Counter64(network_io_diff[i].collisions) );

            from = now - network_io_diff[i].systime;
        }

        rc = mMibObj->set_value(networkIoIntervalTable);
        status &= rc;

        rc = mMibObj->set_value( SM_NETWORK_IO_INTERVAL_FROM, Counter64(from) );
        status &= rc;
        rc = mMibObj->set_value( SM_NETWORK_IO_INTERVAL_UNTIL, Counter64(now) );
        status &= rc;
    }

    if( rc )
    {
        rc = mMibObj->set_value( SM_NETWORK_IO_COUNT, Counter64(entries) );
        status &= rc;
        rc = mMibObj->set_value( SM_LAST_UPDATE_NETWORK_IO_STATUS, Counter64( now ) );
        status &= rc;
    }
#else
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();

    SmartSnmpdNetworkIoMib smNetworkIoMib( cntMgr );

    for( size_t i = 0; i < entries; ++i )
    {
        smNetworkIoMib.addTotalRow( network_io_stats[i] );
        now = network_io_stats[i].systime;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceNetworkIO::updateMibObj: added (entries) rows to total stats");
    LOG(entries);
    LOG_END;

    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        sg_network_io_stats * network_io_diff = diff( network_io_stats );
        size_t diff_entries = sg_get_nelements( network_io_diff );
        time_t from = 0;

        for( size_t i = 0; i < diff_entries; ++i )
        {
            smNetworkIoMib.addIntervalRow( network_io_diff[i] );
            from = now - network_io_diff[i].systime;
        }

        LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
        LOG("DataSourceNetworkIO::updateMibObj: added (entries) rows to interval stats");
        LOG(entries);
        LOG_END;

        smNetworkIoMib.setIntervalSpec( from, now );
    }

    smNetworkIoMib.setCount( entries );
    smNetworkIoMib.setUpdateTimestamp( now );

    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
