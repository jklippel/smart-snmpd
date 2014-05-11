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
#ifndef __SMART_SNMPD_DATASOURCE_USER_LOGIN_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_USER_LOGIN_H_INCLUDED__

#include <smart-snmpd/mibs/statgrab/datasourcestatgrab.h>

#include <agent_pp/mib.h>

namespace SmartSnmpd
{
    /**
     * data source for user login statistics
     */
    class DataSourceUserLogin
        : public DataSourceStatgrab
    {
    public:
        /**
         * destructor
         */
        virtual ~DataSourceUserLogin() {}

        /**
         * accessor to the controlled MibObject instance bound to SM_USER_LOGIN_STATUS
         *
         * This method returns the controlled MibObject instance containing
         * the user login related measuring values. If there is none, it creates a
         * MibObject instance, binds it to SM_USER_LOGIN_STATUS and fill it initially
         * with reasonable basic configuration.
         *
         * @return MibObject * - controlled MibObject
         */
        virtual MibObject * getMibObject();
        /**
         * updates the managed mib object
         *
         * This method fetches the current user login statistics via the
         * sg_get_user_stats function from the statgrab library and updates
         * the desired mib leafs.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool updateMibObj();

        /**
         * get the single instance of this data source for user login statistics
         *
         * @return DataSourceUserLogin & - reference to this instance
         */
        static DataSourceUserLogin & getInstance();
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
        DataSourceUserLogin()
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
         * get an agent++ MibTable instance configured for a smart-snmpd user login statistic table
         *
         * @return MibTable * - pointer to new MibTable instance configured to deliver user login statistics
         */
        NS_AGENT MibTable * getUserLoginTable() const;
#endif
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_USER_LOGIN_H_INCLUDED__ */
