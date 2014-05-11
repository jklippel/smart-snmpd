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
#ifndef __SMART_SNMPD_MIB_CONTAINER_LEAF_H_INCLUDED__
#define __SMART_SNMPD_MIB_CONTAINER_LEAF_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/functional.h>
#include <smart-snmpd/mibutils/mibcontainer.h>
#include <smart-snmpd/mibutils/mibcontaineritem.h>

#include <agent_pp/snmp_pp_ext.h>

#include <string>
#include <set>
#include <stdexcept>

namespace SmartSnmpd
{
    class MibContainerLeaf
        : public MibContainerItem
    {
    public:
        MibContainerLeaf( MibContainer &aContainer, NS_AGENT Oidx const &aLeafOid )
            : MibContainerItem( aContainer )
            , mLeafOid( aLeafOid )
        {}

        MibContainerLeaf( MibContainerLeaf const &ref )
            : MibContainerItem( ref )
            , mLeafOid( ref.mLeafOid )
        {}

        virtual ~MibContainerLeaf() {}

        MibContainerLeaf & set( NS_SNMP SnmpSyntax const &syn ) { mContainer.add( mLeafOid, syn ); return *this; }
        MibContainerLeaf & set( int i ) { mContainer.add( mLeafOid, SnmpInt32(i) ); return *this; }
        MibContainerLeaf & set( long l ) { mContainer.add( mLeafOid, SnmpInt32(l) ); return *this; }
        MibContainerLeaf & set( unsigned int u ) { mContainer.add( mLeafOid, SnmpUInt32(u) ); return *this; }
        MibContainerLeaf & set( unsigned long lu ) { mContainer.add( mLeafOid, SnmpUInt32(lu) ); return *this; }
        MibContainerLeaf & set( unsigned long long llu ) { mContainer.add( mLeafOid, Counter64(llu) ); return *this; }
        MibContainerLeaf & set( std::string const &str ) { mContainer.add( mLeafOid, OctetStr( str.c_str() ) ); return *this; }

    protected:
        const NS_AGENT Oidx mLeafOid;

    private:
        MibContainerLeaf();
    };
}

#endif /* __SMART_SNMPD_MIB_CONTAINER_LEAF_H_INCLUDED__ */
