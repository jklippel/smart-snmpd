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
#ifndef __SMART_SNMPD_MIB_CONTAINER_TABLE_ROW_H_INCLUDED__
#define __SMART_SNMPD_MIB_CONTAINER_TABLE_ROW_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/functional.h>
#include <smart-snmpd/mibutils/mibcontainer.h>
#include <smart-snmpd/mibutils/mibcontainertable.h>
#include <smart-snmpd/log.h>

#include <string>
#include <set>
#include <stdexcept>

namespace SmartSnmpd
{
    class InvalidColumnIndex
        : public std::out_of_range
    {
    public:
        InvalidColumnIndex( unsigned long long colIdx, unsigned long long colCnt )
            : out_of_range( string( string("Invalid column index ") + to_string(colIdx) +
                                    " accessing table with " + to_string(colCnt) +
                                    " columns" ) )
        {}

        InvalidColumnIndex( unsigned long long colIdx, unsigned long long colCnt, std::string const & tblName )
            : out_of_range( string( string("Invalid column index ") + to_string(colIdx) +
                                    " accessing table '" + tblName + "' with " + to_string(colCnt) +
                                    " columns" ) )
        {}

        InvalidColumnIndex( unsigned long long colIdx, unsigned long long colCnt, NS_AGENT Oidx const & tblEntryOid )
            : out_of_range( string( string("Invalid column index ") + to_string(colIdx) +
                                    " accessing table at " + tblEntryOid.cut_right(1).get_printable() +
                                    " with " + to_string(colCnt) +
                                    " columns" ) )
        {}
    };

    class MibContainerTableRow
    {
    public:
        MibContainerTableRow( MibContainerTable &aTable, unsigned long aColumnCount, unsigned long long aRowIdx )
            : mTable( aTable )
            , mColumnCount( aColumnCount )
            , mRowIdx( aRowIdx )
            , mColIdx( 1 )
        {}

        MibContainerTableRow( MibContainerTableRow const &ref )
            : mTable( ref.mTable )
            , mColumnCount( ref.mColumnCount )
            , mRowIdx( ref.mRowIdx )
            , mColIdx( ref.mColIdx )
        {}

        virtual ~MibContainerTableRow() {}

        unsigned long long getCurrentRowIndex() const { return mRowIdx; }

        MibContainerTableRow & operator () (NS_SNMP SnmpSyntax const &syn) { return setCurrentColumn(syn); }
        MibContainerTableRow & operator () (int i) { return setCurrentColumn(i); }
        MibContainerTableRow & operator () (long l) { return setCurrentColumn(l); }
        MibContainerTableRow & operator () (unsigned int u) { return setCurrentColumn(u); }
        MibContainerTableRow & operator () (unsigned long lu) { return setCurrentColumn(lu); }
        MibContainerTableRow & operator () (unsigned long long llu) { return setCurrentColumn(llu); }
        MibContainerTableRow & operator () ( std::string const &s ) { return setCurrentColumn(s); }

        MibContainerTableRow & setCurrentColumn(NS_SNMP SnmpSyntax const &syn)
        {
            // XXX assert mColIdx < mColumnCount
            if( mColIdx > mColumnCount )
                throw InvalidColumnIndex( mColIdx, mColumnCount, mTable.getTableEntryOid() );

#if 0
            NS_AGENT Oidx curColOid( mTable.getTableEntryOid() );
            curColOid += mColIdx;
            curColOid += mRowIdx;

            mTable.mContainer.add( curColOid, syn );
#endif
            mTable.mContainer.add( Oidx( mTable.getTableEntryOid(), Oidx( &mColIdx, 1 ), Oidx( &mRowIdx, 1 ) ), syn );

            ++mColIdx;

            return *this;
        }

        MibContainerTableRow & setCurrentColumn(int i) { return setCurrentColumn( SnmpInt32(i) ); }
        MibContainerTableRow & setCurrentColumn(long l) { return setCurrentColumn( SnmpInt32(l) ); }
        MibContainerTableRow & setCurrentColumn(unsigned int u) { return setCurrentColumn( SnmpUInt32(u) ); }
        MibContainerTableRow & setCurrentColumn(unsigned long lu) { return setCurrentColumn( SnmpUInt32(lu) ); }
        MibContainerTableRow & setCurrentColumn(unsigned long long llu) { return setCurrentColumn( Counter64(llu) ); }
        MibContainerTableRow & setCurrentColumn( std::string const &s ) { return setCurrentColumn( OctetStr( s.c_str() ) ); }

        MibContainerTableRow & operator () () { return setCurrentColumn(); }

        MibContainerTableRow & setCurrentColumn()
        {
            // XXX assert mColIdx < mColumnCount
            if( mColIdx > mColumnCount )
                throw InvalidColumnIndex( mColIdx, mColumnCount, mTable.getTableEntryOid() );

#if 0
            NS_AGENT Oidx curColOid( mTable.getTableEntryOid() );
            curColOid += mColIdx;
            curColOid += mRowIdx;

            mTable.mContainer.add( NS_AGENT Vbx( curColOid ) );
#endif
            mTable.mContainer.add( NS_AGENT Vbx( Oidx( mTable.getTableEntryOid(), Oidx( &mColIdx, 1 ), Oidx( &mRowIdx, 1 ) ) ) );
            ++mColIdx;

            return *this;
        }

        MibContainer::ContainerType::const_iterator getNth(unsigned long aCol) const;
        MibContainer::ContainerType::const_iterator operator [] (unsigned long aCol) const { return getNth( aCol ); }

        MibContainer::ContainerType::iterator getNth(unsigned long aCol);
        MibContainer::ContainerType::iterator operator [] (unsigned long aCol) { return getNth( aCol ); }

    protected:
        MibContainerTable &mTable;
        const unsigned long mColumnCount;
        const unsigned long mRowIdx;
        unsigned long mColIdx;

    private:
        MibContainerTableRow();
    };
}

#endif /* __SMART_SNMPD_MIB_CONTAINER_TABLE_ROW_H_INCLUDED__ */
