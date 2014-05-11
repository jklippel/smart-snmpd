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
#ifndef __SMART_SNMPD_MIB_CONTAINER_TABLE_H_INCLUDED__
#define __SMART_SNMPD_MIB_CONTAINER_TABLE_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/oids.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/functional.h>
#include <smart-snmpd/mibutils/mibcontainer.h>
#include <smart-snmpd/mibutils/mibcontaineritem.h>

#include <string>
#include <set>
#include <stdexcept>

namespace SmartSnmpd
{
    class MibContainerTableRow;

    class MibContainerTable
        : public MibContainerItem
    {
        friend class MibContainerTableRow;

    public:
        typedef MibContainerTableRow RowType;

        MibContainerTable( MibContainer &aContainer, NS_AGENT Oidx const &aTableBaseOid,
                           unsigned long aColumnCount, bool scanForRows = false )
            : MibContainerItem( aContainer )
            , mTableEntryOid( aTableBaseOid + TableEntryKey )
            , mColumnCount( aColumnCount )
            , mLastRow( 0 )
            // , mRows()
        {
            if( scanForRows )
                scanContainerForRows();
        }

        MibContainerTable( MibContainerTable const &ref )
            : MibContainerItem( ref )
            , mTableEntryOid( ref.mTableEntryOid )
            , mColumnCount( ref.mColumnCount )
            , mLastRow( ref.mLastRow )
            // , mRows( ref.mRows )
        {}

        virtual ~MibContainerTable() {}

        NS_AGENT Oidx const & getTableEntryOid() const { return mTableEntryOid; }

        /**
         * returns the row with the specified index (UNIMPLEMENTED)
         *
         * TODO should return some kind of iterator
         */
        MibContainerTableRow const & getRow(unsigned long aRow) const;
        /**
         * returns the row with the specified index (UNIMPLEMENTED)
         *
         * TODO should return some kind of iterator
         */
        MibContainerTableRow & getRow(unsigned long aRow);

        inline MibContainerTableRow addRow();

        unsigned long getLastRowIndex() const { return mLastRow; }

    protected:
        static const NS_AGENT Oidx TableEntryKey;

        const NS_AGENT Oidx mTableEntryOid;
        const unsigned long mColumnCount;
        unsigned long mLastRow;
        // vector<unsigned long> mRows;

        void scanContainerForRows() { throw std::runtime_error( "MibContainerTable::scanContainerForRows is not implemented" ); }

    private:
        MibContainerTable();
    };
}

#include <smart-snmpd/mibutils/mibcontainertablerow.h>


namespace SmartSnmpd
{
    inline MibContainerTableRow
    MibContainerTable::addRow()
    {
        return MibContainerTableRow( *this, mColumnCount, ++mLastRow );
    }
}


#endif /* __SMART_SNMPD_MIB_CONTAINER_TABLE_H_INCLUDED__ */
