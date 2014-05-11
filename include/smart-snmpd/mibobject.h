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
#ifndef __SMART_SNMPD_MIB_OBJECT_H_INCLUDED__
#define __SMART_SNMPD_MIB_OBJECT_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/config.h>
#if 0
#include <smart-snmpd/mibutils/mibcomposed.h>
#endif
#include <smart-snmpd/mibutils/mibcontainer.h>

#include <vector>

namespace SmartSnmpd
{
    extern NS_AGENT Oidx const & FindOidForMibName(std::string const &aMibName);

    class DataSource;

    class MibObject
        : public NS_AGENT MibComplexEntry
    {
    public:
        typedef MibContainer ContentManagerType;

        /**
         * constructor to instantiate a MibObject at given OID
         *
         * @param anOid - base oid where the mib object starts delivering
         * @param aDataSource - data source which delivers the managed data
         */
        MibObject(const Oidx &anOid, DataSource &aDataSource);

        /**
         * copy constructor
         *
         * @param ref - source to copy from
         */
        MibObject(MibObject const &ref);

        /**
         * destructor
         */
        virtual ~MibObject() {}

        /**
         * virtual constructor to clone this instance into a new object
         *
         * @return new instance of MibObject based on this instance
         */
        virtual MibEntry *clone() { NS_AGENT ThreadSynchronize guard(*this); return new MibObject(*this); }

        /**
         * Get method for last update timestamp
         *
         * @return time_t - timestamp of last object update
         */
        time_t GetLastUpdate() const { return mContentMgr.GetLastUpdate(); }

        /**
         * provide access to the configuration of this mib
         *
         * Even if the method returns a reference to an immutable
         * configuration object, be aware that it might change during a
         * backgrounded configuration reload. Create a copy when working
         * in unguarded execution paths.
         *
         * @return MibObjectConfig const & - immutable reference to the
         *   currently hold configuration
         */
        virtual MibObjectConfig const &getConfig() const { return mConfig; }
        /**
         * apply configuration update changes (SYNCHRONIZED)
         *
         * This method creates at least a private copy of the configuration
         * settings affecting this mib object. All modifications to this
         * instance must be guarded.
         */
        virtual void updateConfig();

        /**
         * update the data managed by this mib object (NOT SYNCHRONIZED)
         *
         * This methods trigers an update of the managed data when not
         * done asynchronously by a background updating thread. This is
         * thread safe anyway, because each modification of a value is
         * guarded.
         */
        virtual void update(NS_AGENT Request* aReq);

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
        virtual Oidx find_succ( NS_AGENT Oidx const &o, NS_AGENT Request *aReq = 0 )
        {
            NS_AGENT ThreadSynchronize guard(*this);
            return mContentMgr.find_succ( o, aReq );
        }

        /**
         * Let the receiver process a SNMP GET subrequest (SYNCHRONIZED by caller)
         * 
         * @param req - A pointer to the whole SNMP GET request.
         * @param idx - The index of the subrequest to be processed.
         */
        virtual void get_request( NS_AGENT Request *aReq, int idx )
        {
            mContentMgr.get_request( aReq, idx );
        }

        /**
         * Let the receiver process a SNMP GETNEXT subrequest (SYNCHRONIZED by caller)
         * 
         * @param req - A pointer to the whole SNMP GETNEXT request.
         * @param idx - The index of the subrequest to be processed.
         */
        virtual void get_next_request( NS_AGENT Request *aReq, int idx )
        {
            mContentMgr.get_next_request( aReq, idx );
        }

        /**
         * getter method to retrieve the instance of the content manager
         *
         * This method locks the container which contains the updated content
         * and holds that lock until commitContentUpdate() or
         * abortContentUpdate() is called.
         *
         * @return reference to the used content manager
         */
        ContentManagerType & beginContentUpdate()
        {
            NS_AGENT ThreadSynchronize guard(*this);
            mShadowContentMgr.start_synch();
            return mShadowContentMgr;
        }
        /**
         * getter method to retrieve the instance of the content manager
         *
         * @return reference to the (immutable) used content manager
         */
        ContentManagerType const & getShadowContentManager() const { return mShadowContentMgr; }
        /**
         * commit requested content update (SYNCHRONIZED)
         *
         * @return reference to this instance
         */
        MibObject & commitContentUpdate()
        {
            NS_AGENT ThreadSynchronize guard(*this);
            mShadowContentMgr.start_synch(); // (ccu1)
            mContentMgr.swap( mShadowContentMgr );
            mShadowContentMgr.end_synch(); // end sync from (ccu1)

            mShadowContentMgr.clear();
            mShadowContentMgr.end_synch(); // end sync from beginContentUpdate()

            return *this;
        }
        /**
         * aborts the requested content update
         *
         * @return reference to this instance
         */
        MibObject & abortContentUpdate()
        {
            mShadowContentMgr.clear();
            mShadowContentMgr.end_synch(); // end sync from beginContentUpdate();

            return *this;
        }

        /**
         * Start synchronized execution.
         */
        virtual void start_synch() { MibComplexEntry::start_synch(); mContentMgr.start_synch(); }
        /**
         * End synchronized execution.
         */
        virtual void end_synch() { mContentMgr.end_synch(); MibComplexEntry::end_synch(); }

    protected:
        /**
         * private copy of configuration settings of this mib object
         */
        MibObjectConfig mConfig;
        /**
         * reference to the data source instance responsible to deliver
         * content to this object
         */
        DataSource &mDataSource;
        /**
         * content manager
         */
        ContentManagerType mContentMgr;
        /**
         * shadow content manager to perform atomic updates
         */
        ContentManagerType mShadowContentMgr;
    };
}

#endif /* __SMART_SNMPD_MIB_OBJECT_H_INCLUDED__ */
