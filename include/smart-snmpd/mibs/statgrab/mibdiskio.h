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
#ifndef __SMART_SNMPD_MIB_DISKIO_H_INCLUDED__
#define __SMART_SNMPD_MIB_DISKIO_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class DiskIoMib
    {
    public:
        virtual ~DiskIoMib() {}

        virtual DiskIoMib & setCount( unsigned long long nelem ) = 0;
        virtual DiskIoMib & addTotalRow( sg_disk_io_stats const &diskIo ) = 0;
        virtual DiskIoMib & setIntervalSpec( time_t fromSecsSinceEpoch, time_t untilSecsSinceEpoch ) = 0;
        virtual DiskIoMib & addIntervalRow( sg_disk_io_stats const &diskIo ) = 0;
        virtual DiskIoMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        DiskIoMib() {}
    };

    class SmartSnmpdDiskIoMib
        : public DiskIoMib
    {
    public:
        SmartSnmpdDiskIoMib( MibObject::ContentManagerType &aCntMgr )
            : DiskIoMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mRowCount( aCntMgr, SM_DISK_IO_COUNT_KEY )
            , mTotalIoStats( aCntMgr, SM_DISK_IO_TOTAL_TABLE_KEY, 4 )
            , mIntervalFrom( aCntMgr, SM_DISK_IO_INTERVAL_FROM_KEY )
            , mIntervalUntil( aCntMgr, SM_DISK_IO_INTERVAL_UNTIL_KEY )
            , mIntervalIoStats( aCntMgr, SM_DISK_IO_INTERVAL_TABLE_KEY, 4 )
        {}

        virtual ~SmartSnmpdDiskIoMib() {}

        virtual DiskIoMib & setCount( unsigned long long nelem )
        {
            mRowCount.set( nelem );
            return *this;
        }

        virtual DiskIoMib & addTotalRow( sg_disk_io_stats const &diskIo )
        {
            if( diskIo.disk_name )
            {
                mTotalIoStats.addRow().setCurrentColumn( Counter64( mTotalIoStats.getLastRowIndex() + 1 ) )
                                      .setCurrentColumn( OctetStr( diskIo.disk_name ) )
                                      .setCurrentColumn( Counter64( diskIo.read_bytes ) )
                                      .setCurrentColumn( Counter64( diskIo.write_bytes ) );
            }

            return *this;
        }

        virtual DiskIoMib & setIntervalSpec( time_t fromSecsSinceEpoch, time_t untilSecsSinceEpoch )
        {
            mIntervalFrom.set( fromSecsSinceEpoch );
            mIntervalUntil.set( untilSecsSinceEpoch );

            return *this;
        }

        virtual DiskIoMib & addIntervalRow( sg_disk_io_stats const &diskIo )
        {
            if( diskIo.disk_name )
            {
                mIntervalIoStats.addRow().setCurrentColumn( Counter64( mIntervalIoStats.getLastRowIndex() + 1 ) )
                                         .setCurrentColumn( OctetStr( diskIo.disk_name ) )
                                         .setCurrentColumn( Counter64( diskIo.read_bytes ) )
                                         .setCurrentColumn( Counter64( diskIo.write_bytes ) );
            }

            return *this;
        }

        virtual DiskIoMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
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
        SmartSnmpdDiskIoMib();
    };
}

#endif /* __SMART_SNMPD_MIB_DISKIO_H_INCLUDED__ */
