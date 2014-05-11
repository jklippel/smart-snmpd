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
#ifndef __SMART_SNMPD_MIB_CPU_H_INCLUDED__
#define __SMART_SNMPD_MIB_CPU_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class CpuMib
    {
    public:
        virtual ~CpuMib() {}

        virtual CpuMib &setTotalCpuStats( sg_cpu_stats const &cpuStats ) = 0;
        virtual CpuMib &setIntervalCpuStats( unsigned long long fromSecsSinceEpoch, unsigned long long untilSecsSinceEpoch, sg_cpu_stats const &cpuStats ) = 0;
        virtual CpuMib &setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        CpuMib() {}
    };

    class SmartSnmpdCpuMib
        : public CpuMib
    {
    public:
        SmartSnmpdCpuMib( MibObject::ContentManagerType &aCntMgr )
            : CpuMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mTotalUserTime( aCntMgr, SM_CPU_USER_TIME_TOTAL_KEY )
            , mTotalKernelTime( aCntMgr, SM_CPU_KERNEL_TIME_TOTAL_KEY )
            , mTotalIdleTime( aCntMgr, SM_CPU_IDLE_TIME_TOTAL_KEY )
            , mTotalWaitTime( aCntMgr, SM_CPU_WAIT_TIME_TOTAL_KEY )
            , mTotalSwapTime( aCntMgr, SM_CPU_SWAP_TIME_TOTAL_KEY )
            , mTotalNiceTime( aCntMgr, SM_CPU_NICE_TIME_TOTAL_KEY )
            , mTotalTimeTotal( aCntMgr, SM_CPU_TOTAL_TIME_TOTAL_KEY )

            , mTotalCtxSw( aCntMgr, SM_CPU_CONTEXT_SWITCHES_TOTAL_KEY )
            , mTotalVolCtxSw( aCntMgr, SM_CPU_INVOL_CTX_SWITCHES_TOTAL_KEY )
            , mTotalIvCtxSw( aCntMgr, SM_CPU_VOLUNTARY_CTX_SWITCHES_TOTAL_KEY )
            , mTotalIntrs( aCntMgr, SM_CPU_INTERRUPTS_TOTAL_KEY )
            , mTotalSoftIntrs( aCntMgr, SM_CPU_SOFT_INTERRUPTS_TOTAL_KEY )

            , mIntervalFrom( aCntMgr, SM_CPU_INTERVAL_FROM_KEY )
            , mIntervalUntil( aCntMgr, SM_CPU_INTERVAL_UNTIL_KEY )

            , mIntervalUserTime( aCntMgr, SM_CPU_USER_TIME_INTERVAL_KEY )
            , mIntervalKernelTime( aCntMgr, SM_CPU_KERNEL_TIME_INTERVAL_KEY )
            , mIntervalIdleTime( aCntMgr, SM_CPU_IDLE_TIME_INTERVAL_KEY )
            , mIntervalWaitTime( aCntMgr, SM_CPU_WAIT_TIME_INTERVAL_KEY )
            , mIntervalSwapTime( aCntMgr, SM_CPU_SWAP_TIME_INTERVAL_KEY )
            , mIntervalNiceTime( aCntMgr, SM_CPU_NICE_TIME_INTERVAL_KEY )
            , mIntervalTimeTotal( aCntMgr, SM_CPU_TOTAL_TIME_INTERVAL_KEY )

            , mIntervalCtxSw( aCntMgr, SM_CPU_CONTEXT_SWITCHES_INTERVAL_KEY )
            , mIntervalVolCtxSw( aCntMgr, SM_CPU_INVOL_CTX_SWITCHES_INTERVAL_KEY )
            , mIntervalIvCtxSw( aCntMgr, SM_CPU_VOLUNTARY_CTX_SWITCHES_INTERVAL_KEY )
            , mIntervalIntrs( aCntMgr, SM_CPU_INTERRUPTS_INTERVAL_KEY )
            , mIntervalSoftIntrs( aCntMgr, SM_CPU_SOFT_INTERRUPTS_INTERVAL_KEY )
        {}

        virtual ~SmartSnmpdCpuMib() {}

        /**
         * sets cpu statistics aggregated over all cpu's
         *
         * @param cpuStats (struct sg_cpu_stats) aggregated cpu statistics over
         *        all cpu's grabbed from libstatgrab
         *        
         *
         * @return reference to myself
         */
        virtual CpuMib &setTotalCpuStats( sg_cpu_stats const &cpuStats )
        {
            // fills overall cpu time counters
            mTotalUserTime.set( cpuStats.user );
            mTotalKernelTime.set( cpuStats.kernel );
            mTotalIdleTime.set( cpuStats.idle );
            mTotalWaitTime.set( cpuStats.iowait );
            mTotalSwapTime.set( cpuStats.swap );
            mTotalNiceTime.set( cpuStats.nice );
            mTotalTimeTotal.set( cpuStats.total );

            // fills overall cpu context switches
            mTotalCtxSw.set( cpuStats.context_switches );
            mTotalVolCtxSw.set( cpuStats.voluntary_context_switches );
            mTotalIvCtxSw.set( cpuStats.involuntary_context_switches );

            // fills overall cpu interupt counts
            mTotalIntrs.set( cpuStats.interrupts );
            mTotalSoftIntrs.set( cpuStats.soft_interrupts );

            return *this;
        }

        virtual CpuMib &setIntervalCpuStats( unsigned long long fromSecsSinceEpoch, unsigned long long untilSecsSinceEpoch, sg_cpu_stats const &cpuStats )
        {
            mIntervalFrom.set( fromSecsSinceEpoch );
            mIntervalUntil.set( untilSecsSinceEpoch );

            mIntervalUserTime.set( cpuStats.user );
            mIntervalKernelTime.set( cpuStats.kernel );
            mIntervalIdleTime.set( cpuStats.idle );
            mIntervalWaitTime.set( cpuStats.iowait );
            mIntervalSwapTime.set( cpuStats.swap );
            mIntervalNiceTime.set( cpuStats.nice );
            mIntervalTimeTotal.set( cpuStats.total );

            mIntervalCtxSw.set( cpuStats.context_switches );
            mIntervalVolCtxSw.set( cpuStats.voluntary_context_switches );
            mIntervalIvCtxSw.set( cpuStats.involuntary_context_switches );
            mIntervalIntrs.set( cpuStats.interrupts );
            mIntervalSoftIntrs.set( cpuStats.soft_interrupts );

            return *this;
        }

        virtual CpuMib &setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );

            return *this;
        }

    protected:
        MibObject::ContentManagerType::LeafType mUpdateTimestamp;

        MibObject::ContentManagerType::LeafType mTotalUserTime;
        MibObject::ContentManagerType::LeafType mTotalKernelTime;
        MibObject::ContentManagerType::LeafType mTotalIdleTime;
        MibObject::ContentManagerType::LeafType mTotalWaitTime;
        MibObject::ContentManagerType::LeafType mTotalSwapTime;
        MibObject::ContentManagerType::LeafType mTotalNiceTime;
        MibObject::ContentManagerType::LeafType mTotalTimeTotal;

        MibObject::ContentManagerType::LeafType mTotalCtxSw;
        MibObject::ContentManagerType::LeafType mTotalVolCtxSw;
        MibObject::ContentManagerType::LeafType mTotalIvCtxSw;
        MibObject::ContentManagerType::LeafType mTotalIntrs;
        MibObject::ContentManagerType::LeafType mTotalSoftIntrs;

        MibObject::ContentManagerType::LeafType mIntervalFrom;
        MibObject::ContentManagerType::LeafType mIntervalUntil;

        MibObject::ContentManagerType::LeafType mIntervalUserTime;
        MibObject::ContentManagerType::LeafType mIntervalKernelTime;
        MibObject::ContentManagerType::LeafType mIntervalIdleTime;
        MibObject::ContentManagerType::LeafType mIntervalWaitTime;
        MibObject::ContentManagerType::LeafType mIntervalSwapTime;
        MibObject::ContentManagerType::LeafType mIntervalNiceTime;
        MibObject::ContentManagerType::LeafType mIntervalTimeTotal;

        MibObject::ContentManagerType::LeafType mIntervalCtxSw;
        MibObject::ContentManagerType::LeafType mIntervalVolCtxSw;
        MibObject::ContentManagerType::LeafType mIntervalIvCtxSw;
        MibObject::ContentManagerType::LeafType mIntervalIntrs;
        MibObject::ContentManagerType::LeafType mIntervalSoftIntrs;

    private:
        SmartSnmpdCpuMib();
    };
}

#endif /* __SMART_SNMPD_MIB_CPU_H_INCLUDED__ */
