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
#ifndef __SMART_SNMPD_DATASOURCE_PROCESS_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_PROCESS_H_INCLUDED__

#include <smart-snmpd/mibs/statgrab/datasourcestatgrab.h>
#include <smart-snmpd/pwent.h>

#include <agent_pp/mib.h>

namespace SmartSnmpd
{
    /**
     * data source for process statistics
     */
    class DataSourceProcess
        : public DataSourceStatgrab
    {
    public:
        /**
         * destructor
         */
        virtual ~DataSourceProcess() {}

        /**
         * accessor to the controlled MibObject instance bound to SM_PROCESS_STATUS
         *
         * This method returns the controlled MibObject instance containing
         * the process info related measuring values. If there is none, it creates a
         * MibObject instance, binds it to SM_PROCESS_STATUS and fill it initially
         * with reasonable basic configuration.
         *
         * @return MibObject * - controlled MibObject
         */
        virtual MibObject * getMibObject();
        /**
         * updates the managed mib object
         *
         * This method fetches the current process statistics via the
         * sg_get_process_stats function from the statgrab library and updates
         * the desired mib leafs.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool updateMibObj();

        /**
         * get the single instance of this data source for process statistics
         *
         * @return DataSourceProcess & - reference to this instance
         */
        static DataSourceProcess & getInstance();
        /**
         * destroys singleton instance
         */
        static void destroyInstance();

    protected:
#if 0
        /**
         * system user information accessor
         */
        SystemUserInfo mSysUserInfo;
#endif

        /**
         * default constructor
         */
        DataSourceProcess()
            : DataSourceStatgrab()
#if 0
            , mSysUserInfo()
#endif
        {}

#if 0
        /**
         * initialize controlled mib object
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool initMibObj();

        /**
         * get an agent++ MibTable instance configured for a smart-snmpd process statistic table
         *
         * @return MibTable * - pointer to new MibTable instance configured to deliver process statistics
         */
        NS_AGENT MibTable * getProcessTable() const;
#endif
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_PROCESS_H_INCLUDED__ */
