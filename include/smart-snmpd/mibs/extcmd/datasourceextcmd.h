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
#ifndef __SMART_SNMPD_DATASOURCE_EXTCMD_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_EXTCMD_H_INCLUDED__

#include <smart-snmpd/datasource.h>
#include <agent_pp/snmp_pp_ext.h>

#include <string>

using namespace std;

namespace SmartSnmpd
{
    /**
     * data source for external delivered statistics
     */
    class DataSourceExternalCommand
        : public DataSource
    {
    public:
        /**
         * constructor
         *
         * @param rootOid - the root oid (by trend below 1.3.6.1.4.1.36539.20)
         *                  where the meta data and the reported values for
         *                  the external commands should be available
         */
        DataSourceExternalCommand( Oidx const &rootOid )
            : DataSource()
            , mRootOid(rootOid)
            , mCommandLine()
            , mExpectedEntryCount( 1000 )
        {}

        /**
         * destructor
         */
        virtual ~DataSourceExternalCommand() {}

        /**
         * accessor to the controlled MibObject instance bound to the oid
         * given when constructing the object
         *
         * This method returns the controlled MibObject instance containing
         * extern provided statistics below the oid given on construction. If
         * there is none, it creates a MibObject instance, binds it to named
         * oid and fill it initially with reasonable basic configuration.
         *
         * @return MibObject * - controlled MibObject
         */
        virtual MibObject * getMibObject();
        /**
         * check whether current state needs to be adjusted based on
         * configration of managed MibObject
         *
         * This method updates the objects delivers by the mib, specifically
         * the interval data depending if the most recent interval is
         * configured or not. Changes to the current mib are made only, when
         * the configuration doesn't match the mib state.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool checkMibObjConfig( NS_AGENT Mib &mainMibCtrl );
        /**
         * updates the managed mib object
         *
         * This method fetches the current content by executing the configured
         * external command and parses the resulting json output. The resulting
         * data is put at the oid ".100" below the configured base oid. The
         * meta data is updated upon the status of the command execution.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool updateMibObj();

    protected:
        /**
         * oid of the root MibObject
         */
        const Oidx mRootOid;
        /**
         * the command line to execute (for logging output and SM_EXTERNAL_COMMAND_COMMAND_LINE)
         */
        string mCommandLine;
        /**
         * contains the estimated number of json entries (to avoid reallocs during parsing)
         */
        size_t mExpectedEntryCount;

        /**
         * builds the command line to be executed (to be reported via logging and mib fetching)
         */
        void buildCommandLine();

        /**
         * initialize controlled mib object
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool initMibObj();

    private:
        /**
         * forbidden default constructor
         */
        DataSourceExternalCommand();
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_EXTCMD_H_INCLUDED__ */
