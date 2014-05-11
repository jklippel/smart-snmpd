/*
 * Copyright 2011 Jens Rehsack
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

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/oids.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/mibmodule.h>

#include <smart-snmpd/mibs/extcmd/datasourceextcmd.h>

#include <agent_pp/snmp_textual_conventions.h>

#include <map>

using namespace std;

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.mibs.extcmd.mibmodule";

struct ExtCmdMibModule
    : public MibModule
{
    map<Oidx, DataSourceExternalCommand *> mDataSources;
    const Oidx mExtCmdRootOid;

    ExtCmdMibModule();
    virtual ~ExtCmdMibModule();

    virtual bool InitModule(void);
    virtual bool ShutdownModule(void);

    virtual bool RefreshConfig(NS_AGENT Mib &mainMibCtrl);

    virtual bool RegisterMibs(NS_AGENT Mib &mainMibCtrl);
    virtual bool UnregisterMibs(NS_AGENT Mib &mainMibCtrl);
};

ExtCmdMibModule::ExtCmdMibModule()
    : MibModule()
    , mDataSources()
    , mExtCmdRootOid( SM_EXTERNAL_COMMANDS )
{}

ExtCmdMibModule::~ExtCmdMibModule()
{
    for( map<Oidx, DataSourceExternalCommand *>::iterator i = mDataSources.begin();
         i != mDataSources.end();
         ++i )
    {
        delete i->second;
        i->second = 0;
    }

    mDataSources.clear();
}

bool
ExtCmdMibModule::InitModule(void)
{
    mDataSources.clear();
    return true;
}

bool
ExtCmdMibModule::ShutdownModule(void)
{
    for( map<Oidx, DataSourceExternalCommand *>::iterator i = mDataSources.begin(); i != mDataSources.end(); ++i )
    {
        delete i->second;
        i->second = 0;
    }

    mDataSources.clear();

    delete this;

    return true;
}

bool
ExtCmdMibModule::RefreshConfig(NS_AGENT Mib &mainMibCtrl)
{
    // XXX first rebuild added/deleted extra mibs ...
    map<string, MibObjectConfig> mibConfigs = Config::getInstance().getMibObjectConfigs();
    map<Oidx, DataSourceExternalCommand *> toDelete;

    for( map<Oidx, DataSourceExternalCommand *>::iterator iter = mDataSources.begin();
         iter != mDataSources.end();
         ++iter )
    {
        Oidx oid = iter->first;

        // do not try destroying singletons
        if( mExtCmdRootOid.is_root_of( oid ) )
        {
            string oid_str = oid.get_printable();
            map<string, MibObjectConfig>::const_iterator where = mibConfigs.find(oid_str);
            if( ( where == mibConfigs.end() ) )
            {
                toDelete.insert( *iter );
            }
        }
    }

    for( map<Oidx, DataSourceExternalCommand *>::iterator iter = toDelete.begin(); iter != toDelete.end(); ++iter )
    {
        mDataSources.erase( iter->first );
        iter->second->unregisterMibs( mainMibCtrl );
        delete iter->second;
        iter->second = 0;
    }
    toDelete.clear();

    // refresh existing and insert newly added ...
    for( map<string, MibObjectConfig>::const_iterator iter = mibConfigs.begin();
         iter != mibConfigs.end();
         ++iter )
    {
        if( !iter->second.MibEnabled )
            continue;

        Oidx oid = iter->first.c_str();
        if( mExtCmdRootOid.is_root_of( oid ) )
        {
            map<Oidx, DataSourceExternalCommand *>::const_iterator where = mDataSources.find(oid);
            if( where == mDataSources.end() )
            {
                DataSourceExternalCommand *ds = new DataSourceExternalCommand(oid);
                mDataSources.insert( make_pair( oid, ds ) );
                ds->registerMibs( mainMibCtrl );
            }
            else
            {
                where->second->checkMibObjConfig( mainMibCtrl );
            }
        }
    }

    return true;
}

bool
ExtCmdMibModule::RegisterMibs(NS_AGENT Mib &mainMibCtrl)
{
    map<string, MibObjectConfig> mibConfigs = Config::getInstance().getMibObjectConfigs();
    for( map<string, MibObjectConfig>::const_iterator iter = mibConfigs.begin();
         iter != mibConfigs.end();
         ++iter )
    {
        Oidx oid = iter->first.c_str();
        if( !mExtCmdRootOid.is_root_of( oid ) )
            continue;
        if( !iter->second.MibEnabled )
            continue;

        DataSourceExternalCommand *ds = new DataSourceExternalCommand(oid);
        mDataSources.insert( make_pair( oid, ds ) );
        ds->registerMibs( mainMibCtrl );
    }

    return true;
}

bool
ExtCmdMibModule::UnregisterMibs(NS_AGENT Mib &mainMibCtrl)
{
    for( map<Oidx, DataSourceExternalCommand *>::iterator i = mDataSources.begin(); i != mDataSources.end(); ++i )
    {
        i->second->unregisterMibs( mainMibCtrl );
    }

    return true;
}

MibModule *
getExtCmdModInfo(void)
{
    return new ExtCmdMibModule();
}

}
