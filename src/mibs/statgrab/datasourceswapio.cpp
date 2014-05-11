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
#include <smart-snmpd/mibs/statgrab/datasourceswapio.h>
#include <smart-snmpd/mibs/statgrab/mibswapio.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.swapio";

sg_page_stats
calc_diff<sg_page_stats>::operator () (sg_page_stats const &aComperator, sg_page_stats const &aMostRecent) const
{
    sg_page_stats result;

    result.pages_pagein = aMostRecent.pages_pagein - aComperator.pages_pagein;
    result.pages_pageout = aMostRecent.pages_pageout - aComperator.pages_pageout;

    result.systime = aMostRecent.systime - aComperator.systime;

    return result;
}

static DataSourceSwapIO *instance = NULL;

DataSourceSwapIO &
DataSourceSwapIO::getInstance()
{
    if( !instance )
        instance = new DataSourceSwapIO();

    return *instance;
}

void
DataSourceSwapIO::destroyInstance()
{
    DataSourceSwapIO *ptr = 0;
    if( instance )
    {
        ThreadSynchronize guard( *instance );
        ptr = instance;
        instance = 0;
    }

    delete ptr;
}

MibObject *
DataSourceSwapIO::getMibObject()
{
    if( !mMibObj )
    {
        ThreadSynchronize guard( *instance );
        if( !mMibObj )
        {
            mMibObj = new MibObject( SM_SWAP_IO_STATUS, *this );
            initMibObj();
        }
    }

    return mMibObj;
}

bool
DataSourceSwapIO::initMibObj()
{
#if 0
    mMibObj->add( new MibLeaf( SM_LAST_UPDATE_SWAP_IO_STATUS, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SWAP_PAGES_IN_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SWAP_PAGES_OUT_TOTAL, READONLY, new Counter64(), VMODE_DEFAULT ) );

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
DataSourceSwapIO::initMibObjInterval()
{
    mMibObj->add( new MibLeaf( SM_SWAP_IO_INTERVAL_FROM, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SWAP_IO_INTERVAL_UNTIL, READONLY, new Counter64(), VMODE_DEFAULT ) );

    mMibObj->add( new MibLeaf( SM_SWAP_PAGES_IN_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( SM_SWAP_PAGES_OUT_INTERVAL, READONLY, new Counter64(), VMODE_DEFAULT ) );

    return true;
}
#endif

bool
DataSourceSwapIO::checkMibObjConfig( Mib &mainMibCtrl )
{
    bool rc = DataSourceStatgrab::checkMibObjConfig(mainMibCtrl); // includes mMibObj->updateConfig();

    if( mMibObj )
    {
        setupHistory(*mMibObj);
#if 0
        bool needInterval = mMibObj->getConfig().MostRecentIntervalTime != 0;

        if( needInterval && mMibObj->get(SM_SWAP_IO_INTERVAL) == NULL )
        {
            // previously without interval - now with
            initMibObjInterval();
        }
        else if( mMibObj->get(SM_SWAP_IO_INTERVAL) != NULL && !needInterval )
        {
            // previously with interval - now without
            delete mMibObj->remove( SM_SWAP_IO_INTERVAL_FROM );
            delete mMibObj->remove( SM_SWAP_IO_INTERVAL_UNTIL );
            delete mMibObj->remove( SM_SWAP_IO_INTERVAL );
        }
#endif
    }

    return rc;
}

bool
DataSourceSwapIO::updateMibObj()
{
    sg_page_stats *page_stats = sg_get_page_stats();
    if( !page_stats )
    {
        report_sg_error( "updateMibObj", "sg_get_page_stats() failed" );

        return false;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1);
    LOG("DataSourceSwapIO::updateMibObj: sg_get_page_stats()");
    LOG_END;

    ThreadSynchronize guard(*this);
#if 0
    rc = mMibObj->set_value( SM_SWAP_PAGES_IN_TOTAL, Counter64(page_stats->pages_pagein) );
    status &= rc;
    rc = mMibObj->set_value( SM_SWAP_PAGES_OUT_TOTAL, Counter64(page_stats->pages_pageout) );
    status &= rc;

    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        sg_page_stats const &page_diff = diff( *page_stats );

        rc = mMibObj->set_value( SM_SWAP_IO_INTERVAL_FROM, Counter64(page_stats->systime - page_diff.systime) );
        status &= rc;
        rc = mMibObj->set_value( SM_SWAP_IO_INTERVAL_UNTIL, Counter64(page_stats->systime) );
        status &= rc;

        rc = mMibObj->set_value( SM_SWAP_PAGES_IN_INTERVAL, Counter64(page_diff.pages_pagein) );
        status &= rc;
        rc = mMibObj->set_value( SM_SWAP_PAGES_OUT_INTERVAL, Counter64(page_diff.pages_pageout) );
        status &= rc;
    }

    rc = mMibObj->set_value( SM_LAST_UPDATE_SWAP_IO_STATUS, Counter64(page_stats->systime) );
    status &= rc;
#else
    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();
    SmartSnmpdSwapIoMib smPageIoMib( cntMgr );
    smPageIoMib.setTotalSwapIoStats( *page_stats );

    if( mMibObj->getConfig().MostRecentIntervalTime )
    {
        sg_page_stats const &page_diff = diff( *page_stats );
        smPageIoMib.setIntervalSwapIoStats( page_stats->systime - page_diff.systime, page_stats->systime, page_diff );
    }

    smPageIoMib.setUpdateTimestamp( page_stats->systime );

    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
