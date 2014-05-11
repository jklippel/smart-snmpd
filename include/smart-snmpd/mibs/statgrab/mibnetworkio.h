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
#ifndef __SMART_SNMPD_MIB_NETWORKIO_H_INCLUDED__
#define __SMART_SNMPD_MIB_NETWORKIO_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class NetworkIoMib
    {
    public:
        virtual ~NetworkIoMib() {}

        virtual NetworkIoMib & setCount( unsigned long long nelem ) = 0;
        virtual NetworkIoMib & addTotalRow( sg_network_io_stats const &networkIo ) = 0;
        virtual NetworkIoMib & setIntervalSpec( unsigned long long fromSecsSinceEpoch, unsigned long long untilSecsSinceEpoch ) = 0;
        virtual NetworkIoMib & addIntervalRow( sg_network_io_stats const &networkIo ) = 0;
        virtual NetworkIoMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        NetworkIoMib() {}
    };

    class SmartSnmpdNetworkIoMib
        : public NetworkIoMib
    {
    public:
        SmartSnmpdNetworkIoMib( MibObject::ContentManagerType &aCntMgr )
            : NetworkIoMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mRowCount( aCntMgr, SM_NETWORK_IO_COUNT_KEY )
            , mTotalIoStats( aCntMgr, SM_NETWORK_IO_TABLE_KEY, 9 )
            , mIntervalFrom( aCntMgr, SM_NETWORK_IO_INTERVAL_FROM_KEY )
            , mIntervalUntil( aCntMgr, SM_NETWORK_IO_INTERVAL_UNTIL_KEY )
            , mIntervalIoStats( aCntMgr, SM_NETWORK_IO_INTERVAL_TABLE_KEY, 9 )
        {}

        virtual ~SmartSnmpdNetworkIoMib() {}

        virtual NetworkIoMib & setCount( unsigned long long nelem )
        {
            mRowCount.set( nelem );
            return *this;
        }

        virtual NetworkIoMib & addTotalRow( sg_network_io_stats const &networkIo )
        {
            if( networkIo.interface_name )
            {
                mTotalIoStats.addRow().setCurrentColumn( Counter64( mTotalIoStats.getLastRowIndex() + 1 ) )
                                      .setCurrentColumn( OctetStr( networkIo.interface_name ) )
                                      .setCurrentColumn( Counter64( networkIo.tx ) )
                                      .setCurrentColumn( Counter64( networkIo.rx ) )
                                      .setCurrentColumn( Counter64( networkIo.ipackets ) )
                                      .setCurrentColumn( Counter64( networkIo.opackets ) )
                                      .setCurrentColumn( Counter64( networkIo.ierrors ) )
                                      .setCurrentColumn( Counter64( networkIo.oerrors ) )
                                      .setCurrentColumn( Counter64( networkIo.collisions ) );
            }

            return *this;
        }

        virtual NetworkIoMib & setIntervalSpec( unsigned long long fromSecsSinceEpoch, unsigned long long untilSecsSinceEpoch )
        {
            mIntervalFrom.set( fromSecsSinceEpoch );
            mIntervalUntil.set( untilSecsSinceEpoch );

            return *this;
        }

        virtual NetworkIoMib & addIntervalRow( sg_network_io_stats const &networkIo )
        {
            if( networkIo.interface_name )
            {
                mIntervalIoStats.addRow().setCurrentColumn( Counter64( mIntervalIoStats.getLastRowIndex() + 1 ) )
                                         .setCurrentColumn( OctetStr( networkIo.interface_name ) )
                                         .setCurrentColumn( Counter64( networkIo.tx ) )
                                         .setCurrentColumn( Counter64( networkIo.rx ) )
                                         .setCurrentColumn( Counter64( networkIo.ipackets ) )
                                         .setCurrentColumn( Counter64( networkIo.opackets ) )
                                         .setCurrentColumn( Counter64( networkIo.ierrors ) )
                                         .setCurrentColumn( Counter64( networkIo.oerrors ) )
                                         .setCurrentColumn( Counter64( networkIo.collisions ) );
            }

            return *this;
        }

        virtual NetworkIoMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );
            return *this;
        }

    protected:
        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mRowCount;

        MibObject::ContentManagerType::TableType mTotalIoStats;

        MibObject::ContentManagerType::LeafType mIntervalFrom;
        MibObject::ContentManagerType::LeafType mIntervalUntil;
        MibObject::ContentManagerType::TableType mIntervalIoStats;

    private:
        SmartSnmpdNetworkIoMib();
    };
}

#endif /* __SMART_SNMPD_MIB_NETWORKIO_H_INCLUDED__ */
