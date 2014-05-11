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
#ifndef __SMART_SNMPD_MIB_CONTAINER_H_INCLUDED__
#define __SMART_SNMPD_MIB_CONTAINER_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/oids.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/functional.h>
#include <smart-snmpd/property.h>

#include <agent_pp/mib_complex_entry.h>

#include <string>
#include <set>
#include <stdexcept>

namespace SmartSnmpd
{
    class MibContainerItem;
    class MibContainerTable;
    class MibContainerLeaf;

    /**
     * base class for smart-snmpd mib-trees
     */
    class MibContainer
        : public NS_AGENT ThreadManager
    {
    public:
        typedef MibContainerItem ItemType;
        typedef MibContainerLeaf LeafType;
        typedef MibContainerTable TableType;
        typedef std::set<NS_AGENT Vbx> ContainerType;

        /**
         * last update timestamp
         */
        Property<MibContainer, time_t> mLastUpdate;

        explicit MibContainer(NS_AGENT Oidx const &oid)
            : ThreadManager()
            , mLastUpdate( *this, &SmartSnmpd::MibContainer::readLastUpdate, &SmartSnmpd::MibContainer::writeLastUpdate )
            , mBaseOid( oid )
            , mContent()
        {}

        MibContainer(MibContainer const &other)
            : ThreadManager(other)
            , mLastUpdate( *this, &SmartSnmpd::MibContainer::readLastUpdate, &SmartSnmpd::MibContainer::writeLastUpdate )
            , mBaseOid( other.mBaseOid )
            , mContent( other.mContent )
        {}

        /**
         * Destructor
         */
        virtual ~MibContainer() {}
#if 0
        /**
         * Return a clone of the receiver.
         *
         * @return
         *    a pointer to a clone of the MibContainer object.  
         */
        virtual NS_AGENT MibEntry *clone() { return new MibContainer(*this); }
#endif
        /**
         * Add an instance to the table. If such an instance already
         * exists, it will be removed. (SYNCHRONIZED)
         *
         * @param instance
         *    a MibStaticEntry instance.
         */
        void add(NS_AGENT Vbx const &vb)
        {
            insert_or_update( vb );
        }

        /**
         * Add an instance to the table. If such an instance already
         * exists, it will be removed. (SYNCHRONIZED)
         *
         * @param instance
         *    a MibStaticEntry instance.
         */
        void add( NS_AGENT Oidx const &o, NS_SNMP SnmpSyntax const &syn )
        {
            insert_or_update( o, syn );
        }

        /**
         * Remove an instance from the table. (SYNCHRONIZED)
         *
         * @param o
         *    the object ID of the entry to be removed.
         */
        void remove(NS_AGENT Oidx const &o, bool suffixOnly = true)
        {
            set<NS_AGENT Vbx>::iterator i = findItem(o, suffixOnly);

            if( i != mContent.end() )
                mContent.erase( i );
        }

        /**
         * Get the entry instance with the given OID. If suffixOnly 
         * is false (the default), the specified OID must be the full 
         * OID of the entry, including the OID prefix from the 
         * MibContainer. (NOT SYNCHRONIZED)
         *
         * @param o
         *    the OID (or OID suffix) of the requested entry.
         * @param suffixOnly
         *    determines whether the given OID should be interpreted
         *    as suffix appended to the table's OID or whether the 
         *    given OID fully specifies the requested entry.
         * @return
         *    the entry with the given OID or an empty object if such
         *    an object does not exist.
         */
        ContainerType::const_iterator get(NS_AGENT Oidx const &o, bool suffixOnly = false) const
        {
            set<NS_AGENT Vbx>::const_iterator i = findItem(o, suffixOnly);
            return i;
        }

        /**
         * Get the entry instance with the given OID. If suffixOnly 
         * is false (the default), the specified OID must be the full 
         * OID of the entry, including the OID prefix from the 
         * MibContainer. (NOT SYNCHRONIZED)
         *
         * @param o
         *    the OID (or OID suffix) of the requested entry.
         * @param suffixOnly
         *    determines whether the given OID should be interpreted
         *    as suffix appended to the table's OID or whether the 
         *    given OID fully specifies the requested entry.
         * @return
         *    the entry with the given OID or an empty object if such
         *    an object does not exist.
         */
        ContainerType::iterator get(NS_AGENT Oidx const &o, bool suffixOnly = false)
        {
            set<NS_AGENT Vbx>::iterator i = findItem(o, suffixOnly);
            return i;
        }

        /**
         * Get method for last update timestamp
         *
         * @return time_t - timestamp of last object update
         */
        time_t GetLastUpdate() const { return readLastUpdate(); }

        /**
         * Clear the table.
         */
        void clear() { mContent.clear(); }

        /**
         * Resets the content of the container to its state right after
         * construction. By default this method calls MibComposed::clear()
         * to remove all rows.
         */
        virtual void reset() { clear(); }

        /**
         * swaps current content against given other (SYNCHRONIZED)
         *
         * This methods swaps the currently hold content with the content of
         * the specified content manager. It's expected that exclusive
         * read/write access to the other content manager is guaranteed.
         *
         * return reference to this instance
         */
        MibContainer & swap( MibContainer &other )
        {
            mContent.swap( other.mContent );
            return *this;
        }

        /**
         * Return the successor of a given object identifier within the 
         * receiver's scope and the context of a given Request.
         *
         * @param o
         *    an object identifier
         * @param request
         *    a pointer to a Request instance.
         * @return 
         *    an object identifier if a successor could be found,
         *    otherwise (if no successor exists or is out of scope) 
         *    a zero length oid is returned
         */
        virtual Oidx find_succ( NS_AGENT Oidx const &o, NS_AGENT Request *aReq = 0 );

        /**
         * Let the receiver process a SNMP GET subrequest
         * 
         * @param req - A pointer to the whole SNMP GET request.
         * @param idx - The index of the subrequest to be processed.
         */
        virtual void get_request( NS_AGENT Request *aReq, int idx );

        /**
         * Let the receiver process a SNMP GETNEXT subrequest
         * 
         * @param req - A pointer to the whole SNMP GETNEXT request.
         * @param idx - The index of the subrequest to be processed.
         */
        virtual void get_next_request( NS_AGENT Request *aReq, int idx )
        {
            return get_request( aReq, idx );
        }
        
     protected:
        NS_AGENT Oidx const mBaseOid;
        std::set<NS_AGENT Vbx> mContent;

        inline set<NS_AGENT Vbx>::iterator insert_or_update( NS_AGENT Vbx const &ref )
        {
            std::set<NS_AGENT Vbx>::iterator i = mContent.lower_bound(ref);
            if( ( i == mContent.end() ) || ( mContent.key_comp()(ref, *i ) ) )
            {
                i = mContent.insert( i, ref );
            }
            else
            {
                NS_AGENT Vbx &vb = const_cast<NS_AGENT Vbx &>(*i);
                vb.set_value(ref);
            }
            return i;
        }

        inline set<NS_AGENT Vbx>::iterator insert_or_update( NS_AGENT Oidx const &o, NS_SNMP SnmpSyntax const &syn )
        {
            std::set<NS_AGENT Vbx>::iterator i = mContent.lower_bound( Vbx(o) );
            if( ( i == mContent.end() ) || ( mContent.key_comp()( Vbx(o), *i ) ) )
            {
                Vbx tmpvb( o );
                tmpvb.set_value( syn );
                i = mContent.insert( i, tmpvb );
            }
            else
            {
                NS_AGENT Vbx &vb = const_cast<NS_AGENT Vbx &>(*i);
                vb.set_value(syn);
            }
            return i;
        }

        inline std::set<NS_AGENT Vbx>::iterator findItem( Oidx const &o, bool suffixOnly )
        {
            std::set<NS_AGENT Vbx>::iterator i = mContent.end();
            if( suffixOnly || !mBaseOid.is_root_of( o ) )
            {
                i = mContent.find( Vbx( o ) );
            }
            else
            {
                Vbx tmpvb;
                Oidx tmpoid( o.cut_left( mBaseOid.len() ) );
                tmpvb.set_oid( tmpoid );
                i = mContent.find( tmpvb );
            }

            return i;
        }

        inline std::set<NS_AGENT Vbx>::const_iterator findItem( Oidx const &o, bool suffixOnly ) const
        {
            std::set<NS_AGENT Vbx>::const_iterator i = mContent.end();
            if( suffixOnly || !mBaseOid.is_root_of( o ) )
            {
                i = mContent.find( Vbx( o ) );
            }
            else
            {
                Vbx tmpvb;
                Oidx tmpoid( o.cut_left( mBaseOid.len() ) );
                tmpvb.set_oid( tmpoid );
                i = mContent.find( tmpvb );
            }

            return i;
        }

        /**
         * reads the last update timestamp from the managed mib objects
         * (always the one with the number "1" below our root)
         *
         * @return last update timestamp in seconds since epoch or 0, if
         *   never updated or MibEntry doesn't exists (in fact, that's the same)
         */
        time_t readLastUpdate() const
        { 
            std::set<NS_AGENT Vbx>::iterator i = mContent.find( Vbx( SM_LAST_UPDATE_MIB_KEY ) );
            if( i != mContent.end() )
            {
                Counter64 cntr64;
                i->get_value(cntr64);
                return ((time_t)cntr64);
            }

            return 0; // January 1st 1970 0:00:00
        }

        /**
         * sets the last update timestamp to given timestamp
         *
         * @param aLastUpdate - timestamp of last update
         */
        void writeLastUpdate(time_t const &aLastUpdate)
        {
            insert_or_update( SM_LAST_UPDATE_MIB_KEY, Counter64(aLastUpdate) );
        }

    private:
        MibContainer();
    };
}

#include <smart-snmpd/mibutils/mibcontaineritem.h>
#include <smart-snmpd/mibutils/mibcontainerleaf.h>
#include <smart-snmpd/mibutils/mibcontainertable.h>

#endif /* __SMART_SNMPD_MIB_CONTAINER_H_INCLUDED__ */
