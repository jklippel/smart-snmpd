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
#include <smart-snmpd/mibs/statgrab/datasourceuserlogin.h>
#include <smart-snmpd/mibs/statgrab/mibuserlogin.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.userlogin";

static DataSourceUserLogin *instance = NULL;

DataSourceUserLogin &
DataSourceUserLogin::getInstance()
{
    if( !instance )
        instance = new DataSourceUserLogin();

    return *instance;
}

void
DataSourceUserLogin::destroyInstance()
{
    DataSourceUserLogin *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

MibObject *
DataSourceUserLogin::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_USER_LOGIN_STATUS, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

#if 0
static const index_info indUserLoginTable[1] = {
    {sNMP_SYNTAX_INT, FALSE, 1, 1}
};

MibTable *
DataSourceUserLogin::getUserLoginTable() const
{
    MibTable *newLoginTable = new MibTable( SM_USER_LOGIN_ENTRY, indUserLoginTable, 1 ); // XXX what does it mean?

    newLoginTable->add_col( new MibLeaf( "1", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // INDEX
    newLoginTable->add_col( new MibLeaf( "2", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // USER NAME
    newLoginTable->add_col( new MibLeaf( "3", READONLY, new SnmpUInt32(), VMODE_DEFAULT ) ); // USER ID
    newLoginTable->add_col( new MibLeaf( "4", READONLY, new OctetStr(), VMODE_DEFAULT ) );  // TTY
    newLoginTable->add_col( new MibLeaf( "5", READONLY, new Counter64(), VMODE_DEFAULT ) ); // LOGIN TIME
    newLoginTable->add_col( new MibLeaf( "6", READONLY, new Counter64(), VMODE_DEFAULT ) ); // IDLE TIME
    newLoginTable->add_col( new MibLeaf( "7", READONLY, new Counter64(), VMODE_DEFAULT ) ); // JCPU
    newLoginTable->add_col( new MibLeaf( "8", READONLY, new Counter64(), VMODE_DEFAULT ) ); // PCPU
    newLoginTable->add_col( new MibLeaf( "9", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // WHAT
    newLoginTable->add_col( new MibLeaf( "10", READONLY, new OctetStr(), VMODE_DEFAULT ) ); // FROM (location)

    return newLoginTable;
}

bool
DataSourceUserLogin::initMibObj()
{
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_USER_LOGIN, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_USER_LOGIN_COUNT, READONLY, new SnmpUInt32(), VMODE_DEFAULT ) );
    mMibObj->add( getUserLoginTable() );

    return DataSourceStatgrab::initMibObj();
}
#endif

bool
DataSourceUserLogin::updateMibObj()
{
    size_t entries = 0;
    sg_user_stats *user_stats = sg_get_user_stats(&entries);
    if( !user_stats )
    {
        report_sg_error( "updateMibObj", "sg_get_user_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceUserLogin::updateMibObj: sg_get_user_stats()");
    LOG_END;

#if 0
    MibTable *loginTable = getUserLoginTable();
    for( size_t i = 0; i < entries; ++i )
    {
        MibTableRow *row = loginTable->add_row( loginTable->get_next_avail_index() );
        row->get_nth(0)->replace_value( new SnmpUInt32( i ) );
        if( NULL != user_stats[i].login_name )
            row->get_nth(1)->replace_value( new OctetStr( user_stats[i].login_name ) );
        if( NULL != user_stats[i].login_name )
            row->get_nth(2)->replace_value( new SnmpUInt32( mSysUserInfo.getuseridbyname(user_stats[i].login_name) ) );
        else
            row->get_nth(2)->replace_value( new SnmpUInt32( -1 ) );
        if( NULL != user_stats[i].device )
            row->get_nth(3)->replace_value( new OctetStr( user_stats[i].device ) );
        row->get_nth(4)->replace_value( new Counter64(user_stats[i].login_time) );
#if 0
        row->get_nth(5)->replace_value( new Counter64(user_stats[i].login_time) ); // IDLE TIME
        row->get_nth(6)->replace_value( new Counter64(user_stats[i].login_time) ); // JCPU
        row->get_nth(7)->replace_value( new Counter64(user_stats[i].login_time) ); // PCPU
        row->get_nth(8)->replace_value( new Counter64(user_stats[i].login_time) ); // WHAT
#endif
        if( NULL != user_stats[i].hostname )
            row->get_nth(9)->replace_value( new OctetStr( user_stats[i].hostname ) );
    }

    rc = mMibObj->set_value(loginTable);
    status &= rc;
    if( rc )
    {
        rc = mMibObj->set_value( SM_USER_LOGIN_COUNT, SnmpUInt32(entries) );
        status &= rc;
        if( entries > 0 )
            rc = mMibObj->set_value( SM_LAST_UPDATE_USER_LOGIN, Counter64(user_stats[0].systime) );
        status &= rc;
    }
#else
    time_t now = time(NULL);
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();

    SmartSnmpdUserLoginMib smUserLoginMib( cntMgr );

    for( size_t i = 0; i < entries; ++i )
    {
        smUserLoginMib.addRow( user_stats[i] );
        now = user_stats[i].systime;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceUserLogin::updateMibObj: added (entries) rows to total stats");
    LOG(entries);
    LOG_END;

    smUserLoginMib.setCount( entries );
    smUserLoginMib.setUpdateTimestamp( now );

    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
