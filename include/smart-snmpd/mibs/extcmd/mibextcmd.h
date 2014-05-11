/*
 * Copyright 2010 Matthias Haag, Jens Rehsack
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
#ifndef __SMART_SNMPD_MIB_EXTCMD_H_INCLUDED__
#define __SMART_SNMPD_MIB_EXTCMD_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <smart-snmpd/mibs/extcmd/parseextjson.h>

namespace SmartSnmpd
{
    class ExtCmdMib
    {
    public:
        ExtCmdMib( MibObject::ContentManagerType &aCntMgr )
            : mContentMgr( aCntMgr )
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mLastStartedTimestamp( aCntMgr, SM_LAST_STARTED_EXTERNAL_COMMAND )
            , mLastFinishedTimestamp( aCntMgr, SM_LAST_FINISHED_EXTERNAL_COMMAND )
            , mCommandPath( aCntMgr, SM_EXTERNAL_COMMAND_COMMAND_PATH )
            , mCommandLine( aCntMgr, SM_EXTERNAL_COMMAND_COMMAND_LINE )
            , mCommandUser( aCntMgr, SM_EXTERNAL_COMMAND_USER )
            , mLastExitCode( aCntMgr, SM_EXTERNAL_COMMAND_LAST_EXIT_CODE )
            , mLastExitSignal( aCntMgr, SM_EXTERNAL_COMMAND_LAST_EXIT_SIGNAL )
            , mLastErrorCode( aCntMgr, SM_EXTERNAL_COMMAND_ERROR_CODE )
            , mLastErrorMessage( aCntMgr, SM_EXTERNAL_COMMAND_ERROR_MESSAGE )
            , mExternalDataOid( SM_EXTERNAL_COMMAND_DATA )
        {}

        virtual ~ExtCmdMib() {}

        virtual ExtCmdMib & setCommandConfig( std::string const &path, std::string const &cmndline, std::string const &user )
        {
            mCommandPath.set( path );
            mCommandLine.set( cmndline );
            mCommandUser.set( user );

            return *this;
        }

        virtual ExtCmdMib & setLastStartedTimestamp( time_t secsSinceEpoch )
        {
            mLastStartedTimestamp.set( Counter64( secsSinceEpoch ) );

            return *this;
        }

        virtual ExtCmdMib & setLastExecutionErrorCode( int rc )
        {
            // negative exit/signal codes means: n/a (both are valid in ranges from 0..255)
            mLastExitCode.set( -1 );
            mLastExitSignal.set( -1 );
            mLastErrorCode.set( rc );

            return *this;
        }

        virtual ExtCmdMib & setLastExecutionState( int exitCode, int exitSignal, std::string const &errBuf, time_t finishTimestamp )
        {
            mLastExitCode.set( exitCode );
            mLastExitSignal.set( exitSignal );
            mLastErrorMessage.set( errBuf );
            mLastFinishedTimestamp.set( finishTimestamp );

            return *this;
        }

        // json parse error - leave exit code/signal untouched
        virtual ExtCmdMib & setLastErrorCode( int rc )
        {
            mLastErrorCode.set( rc );

            return *this;
        }

        virtual ExtCmdMib &setCommandResult( vector<ExternalDataTuple> const &data )
        {
            for( vector<ExternalDataTuple>::const_iterator iter = data.begin();
                 iter != data.end();
                 ++iter )
            {
                const ExternalDataTuple &edt = *iter;
                mContentMgr.add( mExternalDataOid + edt.Oid, *edt.Datum );
            }

            return *this;
        }

        virtual ExtCmdMib &setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );

            return *this;
        }

    protected:
        MibObject::ContentManagerType &mContentMgr;

        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mLastStartedTimestamp;
        MibObject::ContentManagerType::LeafType mLastFinishedTimestamp;
        MibObject::ContentManagerType::LeafType mCommandPath;
        MibObject::ContentManagerType::LeafType mCommandLine;
        MibObject::ContentManagerType::LeafType mCommandUser;
        MibObject::ContentManagerType::LeafType mLastExitCode;
        MibObject::ContentManagerType::LeafType mLastExitSignal;
        MibObject::ContentManagerType::LeafType mLastErrorCode;
        MibObject::ContentManagerType::LeafType mLastErrorMessage;

        NS_AGENT Oidx const mExternalDataOid;

    };
}

#endif /* ?__SMART_SNMPD_MIB_EXTCMD_H_INCLUDED__ */
