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
#ifndef __SMART_SNMPD_MIB_MEMORY_H_INCLUDED__
#define __SMART_SNMPD_MIB_MEMORY_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class MemoryMib
    {
    public:
        virtual ~MemoryMib() {}

        virtual MemoryMib & setPhysMem( sg_mem_stats const &physMem ) = 0;
        virtual MemoryMib & setSwapMem( sg_swap_stats const &swapMem ) = 0;
        virtual MemoryMib & setVirtMem( sg_swap_stats const &virtMem ) = 0;

        virtual MemoryMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        MemoryMib() {}
    };

    class SmartSnmpdMemoryMib
        : public MemoryMib
    {
    public:
        SmartSnmpdMemoryMib( MibObject::ContentManagerType &aCntMgr )
            : MemoryMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mPhysMemTotal( aCntMgr, SM_TOTAL_MEMORY_PHYSICAL_KEY )
            , mPhysMemFree( aCntMgr, SM_FREE_MEMORY_PHYSICAL_KEY )
            , mPhysMemUsed( aCntMgr, SM_USED_MEMORY_PHYSICAL_KEY )
            , mPhysMemCache( aCntMgr, SM_CACHE_MEMORY_PHYSICAL_KEY )
            , mSwapMemTotal( aCntMgr, SM_TOTAL_MEMORY_SWAP_KEY )
            , mSwapMemFree( aCntMgr, SM_FREE_MEMORY_SWAP_KEY )
            , mSwapMemUsed( aCntMgr, SM_USED_MEMORY_SWAP_KEY )
            , mVirtMemTotal( aCntMgr, SM_TOTAL_MEMORY_VIRTUAL_KEY )
            , mVirtMemFree( aCntMgr, SM_FREE_MEMORY_VIRTUAL_KEY )
            , mVirtMemUsed( aCntMgr, SM_USED_MEMORY_VIRTUAL_KEY )
        {}

        virtual ~SmartSnmpdMemoryMib() {}

        virtual MemoryMib & setPhysMem( sg_mem_stats const &physMem )
        {
            mPhysMemTotal.set( physMem.total );
            mPhysMemFree.set( physMem.free );
            mPhysMemUsed.set( physMem.used );
            mPhysMemCache.set( physMem.cache );

            return *this;
        }

        virtual MemoryMib & setSwapMem( sg_swap_stats const &swapMem )
        {
            mSwapMemTotal.set( swapMem.total );
            mSwapMemFree.set( swapMem.free );
            mSwapMemUsed.set( swapMem.used );

            return *this;
        }

        virtual MemoryMib & setVirtMem( sg_swap_stats const &virtMem )
        {
            mVirtMemTotal.set( virtMem.total );
            mVirtMemFree.set( virtMem.free );
            mVirtMemUsed.set( virtMem.used );

            return *this;
        }

        virtual MemoryMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );
            return *this;
        }

    protected:
        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mPhysMemTotal;
        MibObject::ContentManagerType::LeafType mPhysMemFree;
        MibObject::ContentManagerType::LeafType mPhysMemUsed;
        MibObject::ContentManagerType::LeafType mPhysMemCache;
        MibObject::ContentManagerType::LeafType mSwapMemTotal;
        MibObject::ContentManagerType::LeafType mSwapMemFree;
        MibObject::ContentManagerType::LeafType mSwapMemUsed;
        MibObject::ContentManagerType::LeafType mVirtMemTotal;
        MibObject::ContentManagerType::LeafType mVirtMemFree;
        MibObject::ContentManagerType::LeafType mVirtMemUsed;

    private:
        SmartSnmpdMemoryMib();
    };
}

#endif /* __SMART_SNMPD_MIB_MEMORY_H_INCLUDED__ */
