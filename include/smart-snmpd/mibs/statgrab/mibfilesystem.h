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
#ifndef __SMART_SNMPD_MIB_FILESYSTEM_H_INCLUDED__
#define __SMART_SNMPD_MIB_FILESYSTEM_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class FilesystemMib
    {
    public:
        virtual ~FilesystemMib() {}

        virtual FilesystemMib & setCount( unsigned long long nelem ) = 0;
        virtual FilesystemMib & addRow( sg_fs_stats const &fs ) = 0;
        virtual FilesystemMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        FilesystemMib() {}
    };

    class SmartSnmpdFilesystemMib
        : public FilesystemMib
    {
    public:
        SmartSnmpdFilesystemMib( MibObject::ContentManagerType &aCntMgr )
            : FilesystemMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mRowCount( aCntMgr, SM_FILE_SYSTEM_COUNT_KEY )
            , mFsStats( aCntMgr, SM_FILE_SYSTEM_TABLE_KEY, 20 )
        {}

        virtual ~SmartSnmpdFilesystemMib() {}

        virtual FilesystemMib & setCount( unsigned long long nelem )
        {
            mRowCount.set( nelem );
            return *this;
        }

        virtual FilesystemMib & addRow( sg_fs_stats const &fs )
        {
            mFsStats.addRow().setCurrentColumn( Counter64( mFsStats.getLastRowIndex() + 1 ) )
                             .setCurrentColumn( fs.mnt_point ? OctetStr( fs.mnt_point ) : OctetStr() )
                             .setCurrentColumn( fs.device_name ? OctetStr( fs.device_name ) : OctetStr() )
                             .setCurrentColumn() // mount options aren't determined at the moment
                             .setCurrentColumn( fs.fs_type ? OctetStr( fs.fs_type ) : OctetStr() )
                             .setCurrentColumn( SnmpUInt32( (unsigned)(fs.device_type) ) )
                             .setCurrentColumn( Counter64(fs.size) )
                             .setCurrentColumn( Counter64(fs.used) )
                             .setCurrentColumn( Counter64(fs.free) ) // XXX was: fs.size - fs.used
                             .setCurrentColumn( Counter64(fs.avail) )
                             .setCurrentColumn( Counter64(fs.total_inodes) )
                             .setCurrentColumn( Counter64(fs.used_inodes) )
                             .setCurrentColumn( Counter64(fs.free_inodes) )
                             .setCurrentColumn( Counter64(fs.avail_inodes) )
                             .setCurrentColumn( Counter64(fs.total_blocks) )
                             .setCurrentColumn( Counter64(fs.used_blocks) )
                             .setCurrentColumn( Counter64(fs.free_blocks) )
                             .setCurrentColumn( Counter64(fs.avail_blocks) )
                             .setCurrentColumn( Counter64(fs.block_size) )
                             .setCurrentColumn( Counter64(fs.io_size) );

            return *this;
        }

        virtual FilesystemMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );
            return *this;
        }

    protected:
        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mRowCount;
        MibObject::ContentManagerType::TableType mFsStats;

    private:
        SmartSnmpdFilesystemMib();
    };
}

#endif /* __SMART_SNMPD_MIB_FILESYSTEM_H_INCLUDED__ */
