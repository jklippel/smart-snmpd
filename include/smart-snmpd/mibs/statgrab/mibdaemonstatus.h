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
#ifndef __SMART_SNMPD_MIB_DAEMONSTATUS_H_INCLUDED__
#define __SMART_SNMPD_MIB_DAEMONSTATUS_H_INCLUDED__

#include <smart-snmpd/mibs/statgrab/datasourcedaemonstatus.h>
#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class DaemonStatusMib
    {
    public:
        virtual ~DaemonStatusMib() {}

        virtual DaemonStatusMib & setVirtualMemoryStatus( DataSourceDaemonStatus::MemoryInfo &memInfo ) = 0;
        virtual DaemonStatusMib & setResidentMemoryStatus( DataSourceDaemonStatus::MemoryInfo &memInfo ) = 0;

        virtual DaemonStatusMib & setHandledRequests( unsigned long long reqs ) = 0;
        virtual DaemonStatusMib & setDaemonUptime( unsigned long long secs ) = 0;
        virtual DaemonStatusMib & setDaemonCpuTime( unsigned long long secs ) = 0;

        virtual DaemonStatusMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        DaemonStatusMib() {}
    };

    class SmartSnmpdDaemonStatusMib
        : public DaemonStatusMib
    {
    public:
        SmartSnmpdDaemonStatusMib( MibObject::ContentManagerType &aCntMgr )
            : DaemonStatusMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mInitialVirtualMemoryUsage( aCntMgr, SM_INITIAL_VIRTUAL_MEMORY_USAGE_KEY )
            , mInitialResidentMemoryUsage( aCntMgr, SM_INITIAL_RESIDENT_MEMORY_USAGE_KEY )
            , mCurrentVirtualMemoryUsage( aCntMgr, SM_CURRENT_VIRTUAL_MEMORY_USAGE_KEY )
            , mCurrentResidentMemoryUsage( aCntMgr, SM_CURRENT_RESIDENT_MEMORY_USAGE_KEY )
            , mMeanVirtualMemoryUsage( aCntMgr, SM_MEAN_VIRTUAL_MEMORY_USAGE_KEY )
            , mMeanResidentMemoryUsage( aCntMgr, SM_MEAN_RESIDENT_MEMORY_USAGE_KEY )
            , mCurrentVirtualMemoryIncreases( aCntMgr, SM_CURRENT_VIRTUAL_MEMORY_INCREASES_KEY )
            , mCurrentResidentMemoryIncreases( aCntMgr, SM_CURRENT_RESIDENT_MEMORY_INCREASES_KEY )
            , mCurrentVirtualMemoryViolations( aCntMgr, SM_MEAN_VIRTUAL_MEMORY_VIOLATIONS_KEY )
            , mCurrentResidentMemoryViolations( aCntMgr, SM_MEAN_RESIDENT_MEMORY_VIOLATIONS_KEY )
            , mMeanVirtualMemoryErrDistance( aCntMgr, SM_MEAN_VIRTUAL_MEMORY_ERR_OK_KEY )
            , mMeanResidentMemoryErrDistance( aCntMgr, SM_MEAN_RESIDENT_MEMORY_ERR_OK_KEY )
            , mHandledRequests( aCntMgr, SM_HANDLED_REQUESTS_KEY )
            , mDaemonUptime( aCntMgr, SM_DAEMON_UPTIME_KEY )
            , mDaemonCpuTime( aCntMgr, SM_DAEMON_CPUTIME_KEY )
        {}

        virtual ~SmartSnmpdDaemonStatusMib() {}

        virtual DaemonStatusMib & setVirtualMemoryStatus( DataSourceDaemonStatus::MemoryInfo &memInfo )
        {
            mInitialVirtualMemoryUsage.set( memInfo.mInitialValue );
            mCurrentVirtualMemoryUsage.set( memInfo.mCurrentValue );
            mMeanVirtualMemoryUsage.set( memInfo.mMeanValue );
            mCurrentVirtualMemoryIncreases.set( memInfo.mMaxViolations );
            mCurrentVirtualMemoryViolations.set( memInfo.mMeanViolations );
            mMeanVirtualMemoryErrDistance.set( memInfo.mMeanErrDistance );

            return *this;
        }

        virtual DaemonStatusMib & setResidentMemoryStatus( DataSourceDaemonStatus::MemoryInfo &memInfo )
        {
            mInitialResidentMemoryUsage.set( memInfo.mInitialValue );
            mCurrentResidentMemoryUsage.set( memInfo.mCurrentValue );
            mMeanResidentMemoryUsage.set( memInfo.mMeanValue );
            mCurrentResidentMemoryIncreases.set( memInfo.mMaxViolations );
            mCurrentResidentMemoryViolations.set( memInfo.mMeanViolations );
            mMeanResidentMemoryErrDistance.set( memInfo.mMeanErrDistance );

            return *this;
        }

        virtual DaemonStatusMib & setHandledRequests( unsigned long long reqs )
        {
            mHandledRequests.set( reqs );

            return *this;
        }

        virtual DaemonStatusMib & setDaemonUptime( unsigned long long secs )
        {
            mDaemonUptime.set( secs );

            return *this;
        }

        virtual DaemonStatusMib & setDaemonCpuTime( unsigned long long secs )
        {
            mDaemonCpuTime.set( secs );

            return *this;
        }

        virtual DaemonStatusMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );

            return *this;
        }

    protected:
        MibObject::ContentManagerType::LeafType mUpdateTimestamp;

        MibObject::ContentManagerType::LeafType mInitialVirtualMemoryUsage;
        MibObject::ContentManagerType::LeafType mInitialResidentMemoryUsage;
        MibObject::ContentManagerType::LeafType mCurrentVirtualMemoryUsage;
        MibObject::ContentManagerType::LeafType mCurrentResidentMemoryUsage;
        MibObject::ContentManagerType::LeafType mMeanVirtualMemoryUsage;
        MibObject::ContentManagerType::LeafType mMeanResidentMemoryUsage;

        MibObject::ContentManagerType::LeafType mCurrentVirtualMemoryIncreases;
        MibObject::ContentManagerType::LeafType mCurrentResidentMemoryIncreases;
        MibObject::ContentManagerType::LeafType mCurrentVirtualMemoryViolations;
        MibObject::ContentManagerType::LeafType mCurrentResidentMemoryViolations;
        MibObject::ContentManagerType::LeafType mMeanVirtualMemoryErrDistance;
        MibObject::ContentManagerType::LeafType mMeanResidentMemoryErrDistance;

        MibObject::ContentManagerType::LeafType mHandledRequests;
        MibObject::ContentManagerType::LeafType mDaemonUptime;
        MibObject::ContentManagerType::LeafType mDaemonCpuTime;
    };
}

#endif /* __SMART_SNMPD_MIB_DAEMONSTATUS_H_INCLUDED__ */
