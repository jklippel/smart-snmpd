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
#include <smart-snmpd/mibs/statgrab/datasourcefilesystem.h>
#include <smart-snmpd/mibs/statgrab/mibfilesystem.h>

#include <statgrab.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.filesystem";

static DataSourceFileSystem *instance = NULL;

DataSourceFileSystem &
DataSourceFileSystem::getInstance()
{
    if( !instance )
        instance = new DataSourceFileSystem();

    return *instance;
}

void
DataSourceFileSystem::destroyInstance()
{
    DataSourceFileSystem *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

#if 0
static const index_info indFileSystemTable[1] = {
    {sNMP_SYNTAX_INT, FALSE, 1, 1}
};
#endif

MibObject *
DataSourceFileSystem::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_FILE_SYSTEM_USAGE, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

#if 0
MibTable *
DataSourceFileSystem::getFileSystemTable() const
{
    MibTable *newFsTbl = new MibTable( SM_FILE_SYSTEM_ENTRY, indFileSystemTable, 1 ); // XXX what does it mean?

    newFsTbl->add_col( new MibLeaf( "1", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "2", READONLY, new OctetStr(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "3", READONLY, new OctetStr(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "4", READONLY, new OctetStr(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "5", READONLY, new OctetStr(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "6", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "7", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "8", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "9", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "10", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "11", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "12", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "13", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "14", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "15", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "16", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "17", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "18", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "19", READONLY, new Counter64(), VMODE_DEFAULT ) );
    newFsTbl->add_col( new MibLeaf( "20", READONLY, new Counter64(), VMODE_DEFAULT ) );

    return newFsTbl;
}

bool
DataSourceFileSystem::initMibObj()
{
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_FILE_SYSTEM_USAGE, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_FILE_SYSTEM_COUNT, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( getFileSystemTable() );

    return DataSourceStatgrab::initMibObj();
}
#endif

bool
DataSourceFileSystem::updateMibObj()
{
    size_t entries = 0;
    sg_fs_stats *fs_stats = sg_get_fs_stats(&entries);
    if( !fs_stats )
    {
        report_sg_error( "DataSourceFileSystem::updateMibObj", "sg_get_fs_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceFileSystem::updateMibObj: sg_get_fs_stats()");
    LOG_END;

    ThreadSynchronize guard(*this);

#if 0
    MibTable *fsTable = getFileSystemTable();
    for( size_t i = 0; i < entries; ++i )
    {
        MibTableRow *row = fsTable->add_row( fsTable->get_next_avail_index() );
        row->get_nth(0)->replace_value( new Counter64(i) );
        if( NULL != fs_stats[i].mnt_point )
            row->get_nth(1)->replace_value( new OctetStr( fs_stats[i].mnt_point ) );
        if( NULL != fs_stats[i].device_name )
            row->get_nth(2)->replace_value( new OctetStr( fs_stats[i].device_name ) );
#if 0
        if( NULL != fs_stats[i].options )
            row->get_nth(3)->replace_value( new OctetStr( fs_stats[i].options ) );
#endif
        if( NULL != fs_stats[i].fs_type )
            row->get_nth(4)->replace_value( new OctetStr( fs_stats[i].fs_type ) );
        row->get_nth(5)->replace_value( new SnmpUInt32( (unsigned)(fs_stats[i].device_type) ) );

        row->get_nth(6)->replace_value( new Counter64(fs_stats[i].size) );
        row->get_nth(7)->replace_value( new Counter64(fs_stats[i].used) );
        row->get_nth(8)->replace_value( new Counter64(fs_stats[i].size - fs_stats[i].used) );
        row->get_nth(9)->replace_value( new Counter64(fs_stats[i].avail) );

        row->get_nth(10)->replace_value( new Counter64(fs_stats[i].total_inodes) );
        row->get_nth(11)->replace_value( new Counter64(fs_stats[i].used_inodes) );
        row->get_nth(12)->replace_value( new Counter64(fs_stats[i].free_inodes) );
        row->get_nth(13)->replace_value( new Counter64(fs_stats[i].avail_inodes) );

        row->get_nth(14)->replace_value( new Counter64(fs_stats[i].total_blocks) );
        row->get_nth(15)->replace_value( new Counter64(fs_stats[i].free_blocks) );
        row->get_nth(16)->replace_value( new Counter64(fs_stats[i].used_blocks) );
        row->get_nth(17)->replace_value( new Counter64(fs_stats[i].avail_blocks) );

        row->get_nth(18)->replace_value( new Counter64(fs_stats[i].block_size) );
        row->get_nth(19)->replace_value( new Counter64(fs_stats[i].io_size) );
    }

    rc = mMibObj->set_value(fsTable);
    status &= rc;
    if( rc )
    {
        rc = mMibObj->set_value( SM_FILE_SYSTEM_COUNT, Counter64(entries) );
        status &= rc;
        rc = mMibObj->set_value( SM_LAST_UPDATE_FILE_SYSTEM_USAGE, Counter64( time(NULL) ) );
        status &= rc;
    }
#else
    time_t lastUpdated = time(NULL); /* in case of no entries ... */
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();

    SmartSnmpdFilesystemMib smFsMib( cntMgr );

    for( size_t i = 0; i < entries; ++i )
    {
        smFsMib.addRow( fs_stats[i] );
        lastUpdated = fs_stats[i].systime;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceFileSystem::updateMibObj: added (entries) rows to total stats()");
    LOG(entries);
    LOG_END;

    smFsMib.setCount( entries );
    smFsMib.setUpdateTimestamp( lastUpdated );

    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
