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
#ifndef __SMART_SNMPD_DATASOURCE_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_H_INCLUDED__

#include <agent_pp/agent++.h>
#include <agent_pp/threads.h>

namespace SmartSnmpd
{
    class MibObject;
    class UpdateThread;

    /**
     * base class for data sources
     *
     * DataSource objects are responsible to fill MibObject instances
     * with data. While MibObject is designed to be a generic container,
     * the DataSource knows how to setup and fill them.
     */
    class DataSource
        : public NS_AGENT ThreadManager
    {
        friend class Agent;

    public:
        /**
         * destructor
         */
        virtual ~DataSource();

        /**
         * accessor to the controlled MibObject instance
         *
         * This method returns the controlled MibObject instance. If there
         * is none, it shall create a suitable one and fill it initially
         * with reasonable basic configuration.
         *
         * @return MibObject * - controlled MibObject
         */
        virtual MibObject * getMibObject() = 0;
        /**
         * check whether current state needs to be adjusted based on
         * configration of managed MibObject
         *
         * This method performes all actions required to adjust the state of
         * the controlled mib object and belonging helpers to match the
         * current configuration. This includes starting or stopping the
         * background updater, allocate memory etc.
         *
         * As base class method it checks whether the background update
         * thread needs to be started (new configuration enables updating
         * in background, but there is no background thread running for
         * this data source) or (elsewise) stopped.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool checkMibObjConfig( NS_AGENT Mib &mainMibCtrl );
        /**
         * called to update the managed mib object
         *
         * This method is called by either the update scheduler (in case of
         * asynchronous updates) or the get_request or get_next_request
         * methods of the managed mib object (in case of synchronous updates).
         * This method cares to fetch the required data to update the mib
         * object and when all data could be successfully fetched, the mib
         * object is updated and the LastUpdate timestamp is set.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool updateMibObj() = 0;

        /**
         * register controlled mibs
         *
         * @param mainMibCtrl - the Agent_PP::Mib instance where to add
         *    the controlled mibs to.
         *
         * @return true when successful, false on error
         */
        virtual bool registerMibs( NS_AGENT Mib &mainMibCtrl );

        /**
         * unregister controlled mibs
         *
         * @param mainMibCtrl - the Agent_PP::Mib instance where to remove
         *    the controlled mibs from.
         *
         * @return true when successful, false on error
         */
        virtual bool unregisterMibs( NS_AGENT Mib &mainMibCtrl );

    protected:
        /**
         * Instance of asynchronous update thread, if once configured.
         * The update thread doesn't always run, but isn't deleted when
         * configuration change from asynchronous to synchronous data
         * providing.
         */
        UpdateThread *mUpdateThread;
        /**
         * Instance of controlled mib object.
         */
        MibObject *mMibObj;

        /**
         * default constructor
         *
         * Because of some of the derived classes are singletons, the default
         * constructor isn't public accessible.
         */
        inline DataSource()
            : ThreadManager()
            , mUpdateThread(0)
            , mMibObj(0)
        {}

        /**
         * initialize controlled mib object
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool initMibObj();
        /**
         * start the asynchronous update thread for the controlled mib
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool startMibObjUpdater();
        /**
         * stop the asynchronous update thread for the controlled mib
         *
         * @return bool - true when successful, false otherwise
         */
        virtual void stopMibObjUpdater();
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_H_INCLUDED__ */
