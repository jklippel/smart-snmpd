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
#ifndef __SMART_SNMPD_DATASOURCE_NETWORK_IO_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_NETWORK_IO_H_INCLUDED__

#include <smart-snmpd/mibs/statgrab/datasourcestatgrab.h>
#include <smart-snmpd/datadiff.h>

#include <statgrab.h>

#include <agent_pp/mib.h>

namespace SmartSnmpd
{
    /**
     * specialization to calculate differences between two sg_network_io_stats vectors
     */
    template<>
    class calc_diff<sg_network_io_stats *>
    {
    public:
        /**
         * diff operator for network input/output statistics
         *
         * @param comperator - operand to compare against the most recent value
         * @param recent - the most recent value
         *
         * @return calculated difference between given comperator and recent
         */
        inline sg_network_io_stats * operator () ( sg_network_io_stats * const &comperator, sg_network_io_stats * const &recent ) const;
    };

    /**
     * specialization for sg_network_io_stats vectors
     *
     * @param t - pointer to the first element of the vector to be freed
     */
    template<>
    void
    DataDiff< sg_network_io_stats * >::freeItem( sg_network_io_stats * &t );

    /**
     * data source for Network I/O statistics
     */
    class DataSourceNetworkIO
        : public DataSourceStatgrab
        , public DataDiff< sg_network_io_stats * >
    {
    public:
        /**
         * destructor
         */
        virtual ~DataSourceNetworkIO() {}

        /**
         * accessor to the controlled MibObject instance bound to SM_NETWORK_IO_STATUS
         *
         * This method returns the controlled MibObject instance containing
         * the Network I/O related measuring values. If there is none, it creates a
         * MibObject instance, binds it to SM_NETWORK_IO_STATUS and fill it initially
         * with reasonable basic configuration.
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
         * This method fetches the current network i/o statistics via the
         * sg_get_network_io_stats function from the statgrab library and
         * updates the desired mib leafs.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool updateMibObj();

        /**
         * get the single instance of this data source for network i/o statistics
         *
         * @return DataSourceNetworkIO & - reference to this instance
         */
        static DataSourceNetworkIO & getInstance();
        /**
         * destroys singleton instance
         */
        static void destroyInstance();

    protected:
        /**
         * default constructor
         */
        DataSourceNetworkIO()
            : DataSourceStatgrab()
            , DataDiff< sg_network_io_stats * >()
        {}

        /**
         * initialize controlled mib object
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool initMibObj();
#if 0
        /**
         * initialize interval part of controlled mib object
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool initMibObjInterval();

        /**
         * get an agent++ MibTable instance configured for a smart-snmpd network I/O statistic table
         *
         * @param baseOid - either SM_NETWORK_IO_ENTRY for absolute network i/o
         *                  statistics or SM_NETWORK_IO_INTERVAL_ENTRY for
         *                  interval statistics
         *
         * @return MibTable * - pointer to new MibTable instance configured to deliver network I/O statistics
         */
        NS_AGENT MibTable * getNetworkIOTable(NS_AGENT Oidx const &baseOid) const;
#endif
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_NETWORK_IO_H_INCLUDED__ */
