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
#ifndef __SMART_SNMPD_MIB_COMPOSED_H_INCLUDED__
#define __SMART_SNMPD_MIB_COMPOSED_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/functional.h>

#include <agent_pp/mib_complex_entry.h>

#include <set>

namespace SmartSnmpd
{
    /**
     * base class for smart-snmpd mib-trees
     */
    class MibComposed
        : public NS_AGENT MibComplexEntry
    {
    public:
        /**
         * last update timestamp
         */
        Property<MibComposed, time_t> mLastUpdate;

        /**
         * constructor to instantiate a MibComposed at given OID
         *
         * @param anOid - base oid where the mib object starts delivering
         */
        explicit MibComposed(const Oidx &anOid)
            : MibComplexEntry( anOid, READONLY )
            , mLastUpdate( *this, &SmartSnmpd::MibComposed::readLastUpdate, &SmartSnmpd::MibComposed::writeLastUpdate )
            , mContent( mib_entry_less_skip( anOid.len() ) )
	    , mSearchHelper()
        {}

        /**
         * copy constructor
         *
         * @param ref - source to copy from
         */
        MibComposed(MibComposed &ref);

        /**
         * destructor
         */
        virtual ~MibComposed() { clear(); }

        /**
         * virtual constructor to clone this instance into a new object
         *
         * @return new instance of MibComposed based on this instance
         */
        virtual MibEntry *clone() { return new MibComposed(*this); }

        /**
         * called to update this instance data
         */
        virtual void update(NS_AGENT Request *) {}

        /**
         * empty check
         *
         * @return true when no items are managed by this instance
         */
        virtual bool is_empty() const { return mContent.empty(); }

        /**
         * assign content to the managed object identified by an oid
         * (SYNCHRONIZED)
         *
         * This method assigns the content of the given SnmpSyntax object to
         * the managed object identified by the Oidx object anOid. If no
         * such object exists, an error is reported.
         *
         * @param anOid - oid to identify the object for setting new value,
         *   either full qualified or with the exact length of 1.
         * @param aSyntax - Snmp content for the object
         *
         * @return true on success or false if an error occured
         */
        virtual bool set_value(NS_AGENT Oidx const &anOid, const NS_SNMP SnmpSyntax &aSyntax);
        /**
         * assign an object to it's place in list of managed objects
         * (SYNCHRONIZED)
         *
         * This method puts the given MibEntry instance at it's place in
         * the internal list of managed objects. The current managed MibEntry
         * object will be deleted after replacing. If no such entry exists,
         * an error is reported.
         *
         * @param anEntry - a regular MibEntry instance to replace the
         *   one currently managed.
         *
         * @return true on success or false if an error occured
         */
        virtual bool set_value(NS_AGENT MibEntry * const anEntry);

        /**
         * Add a mib entry to the table. If such an instance already
         * exists, it will be removed. (SYNCHRONIZED)
         *
         * @param anEntry - MibEntry instance to be added at the address
         *   defined by it's oid.
         * @return MibEntry * - 0 if no previous added object has been
         *   replace, it's instance otherwise
         */
        virtual NS_AGENT MibEntry * add(NS_AGENT MibEntry *anEntry);

        /**
         * Remove an instance from the table. (SYNCHRONIZED)
         *
         * @param anOid - the object ID of the entry to be removed.
         *
         * @return MibEntry * - address of the remove object
         */
        virtual NS_AGENT MibEntry * remove(const NS_AGENT Oidx &anOid);

        /**
         * Get the entry instance with the given OID. If suffixOnly 
         * is FALSE (the default), the specified OID must be the full 
         * OID of the entry, including the OID prefix from the 
         * MibStaticTable. (NOT SYBCHRONIZED)
         *
         * @param anOid - the OID (or OID suffix) of the requested entry
         * @param suffixOnly - determines whether the given OID should be
         *   interpreted as suffix appended to the table's OID or whether
         *   the given OID fully specifies the requested entry.
         * @return MibEntry * - the found MibEntry object or 0 on error
         */
        virtual NS_AGENT MibEntry * get(const NS_AGENT Oidx &anOid, bool suffixOnly = FALSE);

        /**
         * Clear the table. (SYNCHRONIZED)
         */
        virtual void clear();

	/**
	 * Resets the content of the container to its state right after
	 * construction. By default this method calls MibComposed::clear()
	 * to remove all rows.
	 */
	virtual void reset() { clear(); }

        /**
         * Assign new content the table. (SYNCHRONIZED)
         *
         * @param newContent - the list of the objects to assign to the table
         *   (replaces the old ones)
         * @return size_t - count of objects processed
         */
        virtual size_t assign(set<NS_AGENT MibEntry *, mib_entry_less_skip> & newContent);

        /**
         * Return the successor of a given object identifier within the 
         * receiver's scope and the context of a given Request.
         *
         * @param anOid - an object identifier
         * @param aReq - a pointer to a Request instance.
         * @return Oidx - an object identifier if a successor could be found,
         *    otherwise (if no successor exists or is out of scope) 
         *    a zero length oid is returned
         */
        virtual NS_AGENT Oidx find_succ(const NS_AGENT Oidx &anOid, Request *aReq = 0);

        /**
         * Let the receiver process a SNMP GET subrequest
         * 
         * @param aReq - A pointer to the whole SNMP GET request.
         * @param ind - The index of the subrequest to be processed.
         */
        virtual void get_request(NS_AGENT Request *aReq, int ind);

        /**
         * Let the receiver process a SNMP GETNEXT subrequest
         * 
         * @param aReq - A pointer to the whole SNMP GETNEXT request.
         * @param ind - The index of the subrequest to be processed.
         */
        virtual void get_next_request(NS_AGENT Request *aReq, int ind);
        
        /**
         * Check whether a node (a MIB object) is a table.
         *
         * @param e - a pointer to a node (MIB object).
         *
         * @return bool - true if it's a table node, false otherwise
         */
        inline static bool is_table_node(const NS_AGENT MibEntry *e) { return (e->type() == AGENTPP_TABLE); }

        /**
         * Check whether a node (a MIB object) is a leaf node.
         *
         * @param e - a pointer to a node (MIB object).
         *
         * @return bool - true if it's a leaf node, false otherwise
         */
        inline static bool is_leaf_node(const NS_AGENT MibEntry *e) { return (e->type() == AGENTPP_LEAF); }

        /**
         * Check whether a node (a MIB object) is complex, i.e.
         * whether the node manages more than one leaf object.
         *
         * @param e - a pointer to the node (MIB object) to prove.
         *
         * @return bool - true if it's a complex node, false otherwise
         */
        inline static bool is_complex_node(const NS_AGENT MibEntry *e)
        {
            return ((e->type() == AGENTPP_TABLE) ||
                    (e->type() == AGENTPP_PROXY) ||
                    (e->type() == AGENTX_NODE) ||
                    (e->type() == AGENTPP_COMPLEX));
        }

    protected:
        /**
         * vector of managed objects, identified by their oid value below the
         * base oid of this instance
         */
        set<MibEntry *, mib_entry_less_skip> mContent;
	MibEntry mSearchHelper;

        inline MibEntry * insert_or_update( MibEntry *ref )
        {
            set<MibEntry *>::iterator i = mContent.lower_bound(ref);
            if( ( i == mContent.end() ) || ( mContent.key_comp()(ref, *i ) ) )
            {
                i = mContent.insert( i, ref );
		return 0;
            }
            else
            {
                //const_cast<MibStaticEntry &>(*i) = ref; // we're sure that the key isn't modified
		MibEntry *entry = *i;
                const_cast<MibEntry *&>(*i) = ref; // we're sure that the key isn't modified
		return entry;
            }
        }

        /**
         * reads the last update timestamp from the managed mib objects
         * (always the one with the number "1" below our root)
         *
         * @return last update timestamp in seconds since epoch or 0, if
         *   never updated or MibEntry doesn't exists (in fact, that's the same)
         */
        virtual time_t readLastUpdate() const;
        /**
         * sets the last update timestamp to given timestamp
         *
         * @param aLastUpdate - timestamp of last update
         */
        virtual void writeLastUpdate(time_t const &aLastUpdate);

    private:
        //! forbidden default constructor
        MibComposed();
    };
}

#endif /* __SMART_SNMPD_MIB_COMPOSED_H_INCLUDED__ */
