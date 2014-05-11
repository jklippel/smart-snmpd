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

#include <smart-snmpd/mibs/extcmd/extcmd.h>
#include <smart-snmpd/mibs/extcmd/parseextjson.h>
#include <smart-snmpd/mibs/extcmd/datasourceextcmd.h>
#include <smart-snmpd/mibs/extcmd/mibextcmd.h>

#include <agent_pp/snmp_textual_conventions.h>

#include <cctype>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.extcmd";

MibObject *
DataSourceExternalCommand::getMibObject()
{
    if( !mMibObj )
    {
        mMibObj = new MibObject( mRootOid, *this );
        initMibObj();
    }

    return mMibObj;
}

void
DataSourceExternalCommand::buildCommandLine()
{
    MibObjectConfig const &mibCfg = mMibObj->getConfig();

    mCommandLine.clear();
    vector<string> tmpcmdln;

    tmpcmdln.resize( mibCfg.ExternalCommand.Arguments.size() + 1 );

    tmpcmdln[0] = mibCfg.ExternalCommand.Executable;
    for( size_t i = 0; i < mibCfg.ExternalCommand.Arguments.size(); ++i )
        tmpcmdln[i + 1] = mibCfg.ExternalCommand.Arguments[i];

    for( vector<string>::const_iterator i = tmpcmdln.begin(); i != tmpcmdln.end(); ++i )
    {
        for( string::const_iterator j = i->begin(); j != i->end(); ++j )
        {
            if( (!isprint(*j)) || isspace(*j) || iscntrl(*j) || ('\\' == *j) || ('"' == *j) )
            {
                mCommandLine += string( "\\" ) + string( j, j + 1 );
            }
            else
            {
                mCommandLine += string( j, j + 1 );
            }
        }

        if( ( i + 1 ) != tmpcmdln.end() )
            mCommandLine += string( " " );
    }
}

bool
DataSourceExternalCommand::initMibObj()
{
    buildCommandLine();

#if 0
    mMibObj->add( new MibLeaf( mLastUpdateTimestampOid, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mLastStartedTimestampOid, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mLastFinishedTimestampOid, READONLY, new Counter64(), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mCommandPathOid, READONLY, new OctetStr(mibCfg.ExternalCommand.Executable.c_str()), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mCommandLineOid, READONLY, new OctetStr(mCommandLine.c_str()), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mCommandUserOid, READONLY, new OctetStr(mibCfg.ExternalCommand.User.c_str()), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mLastExitCodeOid, READONLY, new SnmpInt32(-1), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mLastSignalCodeOid, READONLY, new SnmpInt32(-1), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mLastErrorCodeOid, READONLY, new SnmpInt32(0), VMODE_DEFAULT ) );
    mMibObj->add( new MibLeaf( mLastErrorMessageOid, READONLY, new OctetStr(), VMODE_DEFAULT ) );
    mMibObj->add( new MibObjectBase( mExternalDataOid ) );
#endif

    return DataSource::initMibObj();
}

bool
DataSourceExternalCommand::checkMibObjConfig( Mib &mainMibCtrl )
{
    bool rc = DataSource::checkMibObjConfig(mainMibCtrl); // includes mMibObj->updateConfig();
    if( rc )
    {
        ThreadSynchronize guard(*this);

        buildCommandLine();
    }

    return rc;
}

bool
DataSourceExternalCommand::updateMibObj()
{
    ExternalCommand *cmd;
    MibObjectConfig const mibCfg = mMibObj->getConfig();

    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG("DataSourceExternalCommand::updateMibObj(): (commandline)");
    LOG( mCommandLine.c_str() );
    LOG_END;

    ThreadSynchronize guard(*this);

    if( !mibCfg.ExternalCommand.User.empty() )
        cmd = new ExternalCommandAsUser(mibCfg.ExternalCommand);
    else
        cmd = new ExternalCommand(mibCfg.ExternalCommand);

    MibObject::ContentManagerType &cntMgr = mMibObj->beginContentUpdate();
    cntMgr.clear();
    ExtCmdMib smExtCmdMib( cntMgr );

    smExtCmdMib.setLastStartedTimestamp( time(NULL) );
    smExtCmdMib.setCommandConfig( mibCfg.ExternalCommand.Executable, mCommandLine, mibCfg.ExternalCommand.User );

    int comp_rc;
    if( ( comp_rc = cmd->start() ) != 0 )
    {
#if 0
        mMibObj->set_value( mLastExitCodeOid, SnmpInt32(-1) );
        mMibObj->set_value( mLastSignalCodeOid, SnmpInt32(-1) );
        mMibObj->set_value( mLastErrorCodeOid, SnmpInt32(comp_rc) );
#else
        smExtCmdMib.setLastExecutionErrorCode( comp_rc );
#endif

        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
        LOG( "DataSourceExternalCommand::updateMibObj(): (command line) failed to start with (error)" );
        LOG( mCommandLine.c_str() );
        LOG( comp_rc );
        LOG_END;

        delete cmd;

        // copy last updated timestamp because it's lost otherwise after commit
        smExtCmdMib.setUpdateTimestamp( mMibObj->GetLastUpdate() );
        mMibObj->commitContentUpdate();

        return false;
    }

    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 5 );
    LOG( "DataSourceExternalCommand::updateMibObj(): (command line) started with (pid)" );
    LOG( mCommandLine.c_str() );
    LOG( cmd->getChildPid() );
    LOG_END;

    while( cmd->poll(5) )
        (void)0; /* nop() */

    cmd->finish();

#if 0
    rc = mMibObj->set_value( mLastFinishedTimestampOid, Counter64( time(NULL) ) );
    status &= rc;
    rc = mMibObj->set_value( mLastExitCodeOid, SnmpInt32( cmd->getExitCode() ) );
    status &= rc;
    rc = mMibObj->set_value( mLastSignalCodeOid, SnmpInt32( cmd->getExitSignal() ) );
    status &= rc;
    rc = mMibObj->set_value( mLastErrorMessageOid, OctetStr( cmd->getErrBuf().c_str() ) );
    status &= rc;
#else
    smExtCmdMib.setLastExecutionState( cmd->getExitCode(), cmd->getExitSignal(), cmd->getErrBuf(), time(NULL) );
#endif

    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
    LOG( "DataSourceExternalCommand::updateMibObj(): (command line) finished with (exit code) (exit signal)" );
    LOG( mCommandLine.c_str() );
    LOG( cmd->getExitCode() );
    LOG( cmd->getExitSignal() );
    LOG_END;

    if( cmd->getExitCode() != 0 )
    {
        delete cmd;

        // copy last updated timestamp because it's lost otherwise after commit
        smExtCmdMib.setUpdateTimestamp( mMibObj->GetLastUpdate() );
        mMibObj->commitContentUpdate();

        return false;
    }

    ParseExternalJson pej;
    pej.reserve(mExpectedEntryCount);

    if( 0 == ( comp_rc = pej.parse( cmd->getOutBuf() ) ) )
    {
        LOG_BEGIN(loggerModuleName, DEBUG_LOG | 1 );
        LOG( "DataSourceExternalCommand::updateMibObj(): json output successful parsed" );
        LOG_END;
        smExtCmdMib.setLastErrorCode( 0 );
    }
    else
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 3 );
        LOG( "DataSourceExternalCommand::updateMibObj(): error parsing json output" );
        LOG_END;

        smExtCmdMib.setLastErrorCode( comp_rc );
        // copy last updated timestamp because it's lost otherwise after commit
        smExtCmdMib.setUpdateTimestamp( mMibObj->GetLastUpdate() );
        mMibObj->commitContentUpdate();

        delete cmd;
        return false;
    }

    delete cmd;

    vector<ExternalDataTuple> const &data = pej.getData();
    mExpectedEntryCount = data.size();
#if 0
    MibStaticTable *extContent = new MibStaticTable( mExternalDataOid );

    for( vector<ExternalDataTuple>::const_iterator iter = data.begin();
         iter != data.end();
         ++iter )
    {
        const ExternalDataTuple &edt = *iter;
	extContent->add( MibStaticEntry( Vbx( edt.Oid, *edt.Datum ) ) );
    }

    rc = mMibObj->set_value( extContent );
    status &= rc;

    rc = mMibObj->set_value( mLastUpdateTimestampOid, Counter64( time(NULL) ) );
    status &= rc;
#else
    smExtCmdMib.setCommandResult( data );
    smExtCmdMib.setUpdateTimestamp( time(NULL) );
    mMibObj->commitContentUpdate();
#endif

    return true;
}

}
