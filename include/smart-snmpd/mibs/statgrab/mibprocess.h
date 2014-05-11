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
#ifndef __SMART_SNMPD_MIB_PROCESS_H_INCLUDED__
#define __SMART_SNMPD_MIB_PROCESS_H_INCLUDED__

#include <smart-snmpd/mibobject.h>
#include <smart-snmpd/pwent.h>

#include <statgrab.h>

#include <iomanip>
#include <sstream>

namespace SmartSnmpd
{
    class ProcessMib
    {
    public:
        virtual ~ProcessMib() {}

        virtual ProcessMib & setCounts( sg_process_count const &procCounts ) = 0;
        virtual ProcessMib & addRow( sg_process_stats const &proc ) = 0;
        virtual ProcessMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        ProcessMib() {}
    };

    class SmartSnmpdProcessMib
        : public ProcessMib
    {
    public:
        SmartSnmpdProcessMib( MibObject::ContentManagerType &aCntMgr )
            : ProcessMib()
            , mSysUserInfo()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mTotalProcCount( aCntMgr, SM_PROCESS_TOTAL_KEY )
            , mRunningProcCount( aCntMgr, SM_PROCESS_RUNNING_KEY )
            , mSleepingProcCount( aCntMgr, SM_PROCESS_SLEEPING_KEY )
            , mStoppedProcCount( aCntMgr, SM_PROCESS_STOPPED_KEY )
            , mZombieProcCount( aCntMgr, SM_PROCESS_ZOMBIE_KEY )
            , mProcList( aCntMgr, SM_PROCESS_TABLE_KEY, 19 )
        {}

        virtual ~SmartSnmpdProcessMib() {}

        virtual ProcessMib & setCounts( sg_process_count const &procCounts )
        {
            mTotalProcCount.set( procCounts.total );
            mRunningProcCount.set( procCounts.running );
            mSleepingProcCount.set( procCounts.sleeping );
            mStoppedProcCount.set( procCounts.stopped );
            mZombieProcCount.set( procCounts.zombie );

            return *this;
        }

        virtual ProcessMib & addRow( sg_process_stats const &proc )
        {
            mProcList.addRow().setCurrentColumn( SnmpUInt32(proc.pid) )
                              .setCurrentColumn( SnmpUInt32(proc.parent) )
                              .setCurrentColumn( proc.process_name ? OctetStr(proc.process_name) : OctetStr() )
                              .setCurrentColumn( proc.proctitle ? OctetStr(proc.proctitle) : OctetStr() )
                              .setCurrentColumn( SnmpUInt32(proc.state) )
                              .setCurrentColumn( Counter64( proc.proc_size ) )
                              .setCurrentColumn( Counter64( proc.proc_resident ) )
                              .setCurrentColumn( Counter64( proc.start_time ) )
                              .setCurrentColumn( Counter64( proc.time_spent ) )
                              .setCurrentColumn( SnmpUInt32( proc.uid ) )
                              .setCurrentColumn( mSysUserInfo.getusernamebyuid( proc.uid ) )
                              .setCurrentColumn( SnmpUInt32( proc.gid ) )
                              .setCurrentColumn( mSysUserInfo.getgroupnamebygid( proc.gid ) )
                              .setCurrentColumn( SnmpUInt32( proc.euid ) )
                              .setCurrentColumn( mSysUserInfo.getusernamebyuid( proc.euid ) )
                              .setCurrentColumn( SnmpUInt32( proc.egid ) )
                              .setCurrentColumn( mSysUserInfo.getgroupnamebygid( proc.egid ) )
                              .setCurrentColumn( SnmpInt32( proc.nice ) )
                              .setCurrentColumn( fmtPercent( proc.cpu_percent ) );

            return *this;
        }

        virtual ProcessMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );
            return *this;
        }

    protected:
        /**
         * system user information accessor
         */
        SystemUserInfo mSysUserInfo;

        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mTotalProcCount;
        MibObject::ContentManagerType::LeafType mRunningProcCount;
        MibObject::ContentManagerType::LeafType mSleepingProcCount;
        MibObject::ContentManagerType::LeafType mStoppedProcCount;
        MibObject::ContentManagerType::LeafType mZombieProcCount;
        MibObject::ContentManagerType::TableType mProcList;

        static std::string fmtPercent( double d, int precision = 2 )
        {
            ostringstream oss;

            if( precision )
                oss << fixed << setprecision(2);
            oss << d;

            return oss.str();
        }

    private:
        SmartSnmpdProcessMib();
    };
}

#endif /* __SMART_SNMPD_MIB_PROCESS_H_INCLUDED__ */
