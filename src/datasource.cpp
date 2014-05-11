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
#include <smart-snmpd/datasource.h>
#include <smart-snmpd/mibobject.h>
#include <smart-snmpd/updatethread.h>

#include <agent_pp/snmp_textual_conventions.h>
#include <snmp_pp/log.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource";

DataSource::~DataSource()
{
    ThreadSynchronize guard(*this);

    delete mUpdateThread;
    mUpdateThread = 0;
    mMibObj = 0;
}

bool
DataSource::startMibObjUpdater()
{
    if( !mMibObj )
        return false;

    ThreadSynchronize guard(*this);

    if( mUpdateThread && mUpdateThread->is_alive() )
    {
        return true;
    }

    if( !mUpdateThread )
        mUpdateThread = new UpdateThread( *this, *mMibObj );
    if( !mUpdateThread )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
        LOG("DataSource::initMibObj: failed to create update thread for (oid)");
        LOG(mMibObj->key()->get_printable());
        LOG_END;

        return false;
    }

    mUpdateThread->start();
    if( !mUpdateThread->is_alive() )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
        LOG("DataSource::initMibObj: failed to start update thread for (oid)");
        LOG(mMibObj->key()->get_printable());
        LOG_END;

        return false;
    }

    return true;
}

void
DataSource::stopMibObjUpdater()
{
    ThreadSynchronize guard(*this);

    if( mUpdateThread && mUpdateThread->is_alive() )
    {
        mUpdateThread->stop();

        delete mUpdateThread;
        mUpdateThread = 0;
    }
}

bool
DataSource::checkMibObjConfig( Mib &mainMibCtrl )
{
    bool asynchUpdate = false;
    bool mibEnabled = false;
    bool mibRegistered = false;

    ThreadSynchronize guard(*this);

    if( mMibObj )
    {
        mMibObj->updateConfig();
        asynchUpdate = mMibObj->getConfig().AsyncUpdate;
        mibEnabled = mMibObj->getConfig().MibEnabled;
        mibRegistered = NULL != mainMibCtrl.get( *( mMibObj->key() ) );
    }

    if( (mUpdateThread && mUpdateThread->is_alive() ) && !asynchUpdate )
    {
        LOG_BEGIN(loggerModuleName, EVENT_LOG | 3);
        LOG("DataSource::checkMibObjConfig: asynchronous update disabled for (oid) - stopping thread");
        LOG(mMibObj ? mMibObj->key()->get_printable() : "<deleted>");
        LOG_END;

        stopMibObjUpdater();
    }
    else if( asynchUpdate && !(mUpdateThread && mUpdateThread->is_alive() ) )
    {
        LOG_BEGIN(loggerModuleName, DEBUG_LOG | 5);
        LOG("DataSource::checkMibObjConfig: asynchronous update enabled for (oid) - starting thread");
        LOG(mMibObj->key()->get_printable());
        LOG_END;

        if( !startMibObjUpdater() )
        {
            MibObjectConfig &modCfg = Config::getInstance(0).getMibObjectConfig(mMibObj->key()->get_printable());
            modCfg.AsyncUpdate = false;

            LOG_BEGIN(loggerModuleName, ERROR_LOG | 5);
            LOG("DataSource::checkMibObjConfig: set asynchronous update to false for (oid)");
            LOG(mMibObj->key()->get_printable());
            LOG_END;
        }
    }

    if( mibRegistered && !mibEnabled )
    {
        LOG_BEGIN(loggerModuleName, EVENT_LOG | 3);
        LOG("DataSource::checkMibObjConfig: ");
        LOG(mMibObj ? mMibObj->key()->get_printable() : "<deleted>");
        LOG_END;

        unregisterMibs( mainMibCtrl );
    }
    else if( mibEnabled && !mibRegistered )
    {
        LOG_BEGIN(loggerModuleName, DEBUG_LOG | 5);
        LOG("DataSource::checkMibObjConfig: ");
        LOG(mMibObj->key()->get_printable());
        LOG_END;

        registerMibs( mainMibCtrl );
    }

    return true;
}

bool
DataSource::initMibObj()
{
    if( mMibObj->getConfig().AsyncUpdate )
    {
        LOG_BEGIN(loggerModuleName, DEBUG_LOG | 5);
        LOG("DataSource::initMibObj: going to create update thread for (oid)");
        LOG(mMibObj->key()->get_printable());
        LOG_END;

        if( !startMibObjUpdater() )
        {
            MibObjectConfig &modCfg = Config::getInstance(0).getMibObjectConfig(mMibObj->key()->get_printable());
            modCfg.AsyncUpdate = false;

            LOG_BEGIN(loggerModuleName, ERROR_LOG | 5);
            LOG("DataSource::initMibObj: set asynchronous update to false for (oid)");
            LOG(mMibObj->key()->get_printable());
            LOG_END;
        }
    }

    return true;
}

bool
DataSource::registerMibs( NS_AGENT Mib &mainMibCtrl )
{
    ThreadSynchronize guard(*this);

    if( NULL == mMibObj )
        getMibObject();

    if( NULL == mMibObj )
        return false;

    if( mMibObj->getConfig().MibEnabled )
    {
        LOG_BEGIN( loggerModuleName, EVENT_LOG | 1 );
        LOG( "DataSource::registerMibs(): registering (oid)" );
        LOG( mMibObj->key()->get_printable() );
        LOG_END;

        return mainMibCtrl.add( mMibObj ); // might return false on error
    }

    return true; // not registered when disabled means success
}

bool
DataSource::unregisterMibs( NS_AGENT Mib &mainMibCtrl )
{
    ThreadSynchronize guard(*this);

    if( NULL == mMibObj )
        return true; // not unregistering not existing object means success

    stopMibObjUpdater(); // no update thread should access non-existing mib ...

    LOG_BEGIN( loggerModuleName, EVENT_LOG | 1 );
    LOG( "DataSource::unregisterMibs(): unregistering (oid)" );
    LOG( mMibObj->key()->get_printable() );
    LOG_END;

    bool rc = mainMibCtrl.remove( *(mMibObj->key()) ); // might return false on error
    if( rc )
    {
        mMibObj = 0; // has been deleted by mib object
    }

    return rc;
}

}
