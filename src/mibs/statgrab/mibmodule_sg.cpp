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

#include <smart-snmpd/mibs/statgrab/datasourcestatgrab.h>

#include <smart-snmpd/mibs/statgrab/datasourcecpu.h>
#include <smart-snmpd/mibs/statgrab/datasourcedaemonstatus.h>
#include <smart-snmpd/mibs/statgrab/datasourcediskio.h>
#include <smart-snmpd/mibs/statgrab/datasourcefilesystem.h>
#include <smart-snmpd/mibs/statgrab/datasourcehostinfo.h>
#include <smart-snmpd/mibs/statgrab/datasourceload.h>
#include <smart-snmpd/mibs/statgrab/datasourcememory.h>
#include <smart-snmpd/mibs/statgrab/datasourcenetworkio.h>
#include <smart-snmpd/mibs/statgrab/datasourceprocess.h>
#include <smart-snmpd/mibs/statgrab/datasourceswapio.h>
#include <smart-snmpd/mibs/statgrab/datasourceuserlogin.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.mibs.statgrab.mibmodule";

struct StatgrabMibModule
    : public MibModule
{
    vector<DataSource *> mDataSources;

    StatgrabMibModule();
    virtual ~StatgrabMibModule();

    virtual bool InitModule(void);
    virtual bool ShutdownModule(void);

    virtual bool RefreshConfig(NS_AGENT Mib &mainMibCtrl);

    virtual bool RegisterMibs(NS_AGENT Mib &mainMibCtrl);
    virtual bool UnregisterMibs(NS_AGENT Mib &mainMibCtrl);
};

StatgrabMibModule::StatgrabMibModule()
    : MibModule()
    , mDataSources()
{}

StatgrabMibModule::~StatgrabMibModule() {}

bool
StatgrabMibModule::InitModule(void)
{
    mDataSources.clear();

    mDataSources.push_back( &DataSourceDaemonStatus::getInstance() );
    mDataSources.push_back( &DataSourceHostInfo::getInstance() );
    mDataSources.push_back( &DataSourceCPU::getInstance() );
    mDataSources.push_back( &DataSourceMemory::getInstance() );
    mDataSources.push_back( &DataSourceLoad::getInstance() );
    mDataSources.push_back( &DataSourceUserLogin::getInstance() );
    mDataSources.push_back( &DataSourceProcess::getInstance() );
    mDataSources.push_back( &DataSourceFileSystem::getInstance() );
    mDataSources.push_back( &DataSourceDiskIO::getInstance() );
    mDataSources.push_back( &DataSourceNetworkIO::getInstance() );
    mDataSources.push_back( &DataSourceSwapIO::getInstance() );

    return DataSourceStatgrab::InitStatgrab();
}

bool
StatgrabMibModule::ShutdownModule(void)
{
    DataSourceDaemonStatus::destroyInstance();
    DataSourceHostInfo::destroyInstance();
    DataSourceCPU::destroyInstance();
    DataSourceMemory::destroyInstance();
    DataSourceLoad::destroyInstance();
    DataSourceUserLogin::destroyInstance();
    DataSourceProcess::destroyInstance();
    DataSourceFileSystem::destroyInstance();
    DataSourceDiskIO::destroyInstance();
    DataSourceNetworkIO::destroyInstance();
    DataSourceSwapIO::destroyInstance();

    mDataSources.clear();

    delete this;

    return DataSourceStatgrab::ShutdownStatgrab();
}

bool
StatgrabMibModule::RefreshConfig(NS_AGENT Mib &mainMibCtrl)
{
    for( vector<DataSource *>::iterator i = mDataSources.begin(); i != mDataSources.end(); ++i )
    {
        (*i)->checkMibObjConfig(mainMibCtrl);
    }

    return true;
}

bool
StatgrabMibModule::RegisterMibs(NS_AGENT Mib &mainMibCtrl)
{
    for( vector<DataSource *>::iterator i = mDataSources.begin(); i != mDataSources.end(); ++i )
    {
        (*i)->registerMibs(mainMibCtrl);
    }

    return true;
}

bool
StatgrabMibModule::UnregisterMibs(NS_AGENT Mib &mainMibCtrl)
{
    for( vector<DataSource *>::iterator i = mDataSources.begin(); i != mDataSources.end(); ++i )
    {
        (*i)->unregisterMibs(mainMibCtrl);
    }

    return true;
}

MibModule *
getStatgrabModInfo(void)
{
    return new StatgrabMibModule();
}

}
