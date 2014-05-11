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
#ifndef __SMART_SNMPD_MIB_SWAPIO_H_INCLUDED__
#define __SMART_SNMPD_MIB_SWAPIO_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class SwapIoMib
    {
    public:
        virtual ~SwapIoMib() {}

        virtual SwapIoMib &setTotalSwapIoStats( sg_page_stats const &pageStats ) = 0;
        virtual SwapIoMib &setIntervalSwapIoStats( unsigned long long fromSecsSinceEpoch, unsigned long long untilSecsSinceEpoch, sg_page_stats const &pageStats ) = 0;
        virtual SwapIoMib &setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        SwapIoMib() {}
    };

    class SmartSnmpdSwapIoMib
        : public SwapIoMib
    {
    public:
        SmartSnmpdSwapIoMib( MibObject::ContentManagerType &aCntMgr )
            : SwapIoMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mPagesInTotal( aCntMgr, SM_SWAP_PAGES_IN_TOTAL_KEY )
            , mPagesOutTotal( aCntMgr, SM_SWAP_PAGES_OUT_TOTAL_KEY )
            , mIntervalFrom( aCntMgr, SM_SWAP_IO_INTERVAL_FROM_KEY )
            , mIntervalUntil( aCntMgr, SM_SWAP_IO_INTERVAL_UNTIL_KEY )
            , mPagesInInterval( aCntMgr, SM_SWAP_PAGES_IN_INTERVAL_KEY )
            , mPagesOutInterval( aCntMgr, SM_SWAP_PAGES_OUT_INTERVAL_KEY )
        {}

        virtual ~SmartSnmpdSwapIoMib() {}

        virtual SwapIoMib &setTotalSwapIoStats( sg_page_stats const &pageStats )
        {
            mPagesInTotal.set( pageStats.pages_pagein );
            mPagesOutTotal.set( pageStats.pages_pageout );

            return *this;
        }

        virtual SwapIoMib &setIntervalSwapIoStats( unsigned long long fromSecsSinceEpoch, unsigned long long untilSecsSinceEpoch, sg_page_stats const &pageStats )
        {
            mIntervalFrom.set( fromSecsSinceEpoch );
            mIntervalUntil.set( untilSecsSinceEpoch );

            mPagesInInterval.set( pageStats.pages_pagein );
            mPagesOutInterval.set( pageStats.pages_pageout );

            return *this;
        }

        virtual SwapIoMib &setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );

            return *this;
        }

    protected:
        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mPagesInTotal;
        MibObject::ContentManagerType::LeafType mPagesOutTotal;
        MibObject::ContentManagerType::LeafType mIntervalFrom;
        MibObject::ContentManagerType::LeafType mIntervalUntil;
        MibObject::ContentManagerType::LeafType mPagesInInterval;
        MibObject::ContentManagerType::LeafType mPagesOutInterval;

    private:
        SmartSnmpdSwapIoMib();
    };
}

#endif /* __SMART_SNMPD_MIB_SWAPIO_H_INCLUDED__ */
