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
#ifndef __SMART_SNMPD_DATASOURCE_CPU_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_CPU_H_INCLUDED__

#include <smart-snmpd/mibs/statgrab/datasourcestatgrab.h>
#include <smart-snmpd/datadiff.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    /**
     * specialization to calculate differences between two sg_cpu_stats instances
     */
    template<>
    class calc_diff<sg_cpu_stats>
    {
    public:
        /**
         * diff operator for cpu statistics
         *
         * @param comperator - operand to compare against the most recent value
         * @param recent - the most recent value
         *
         * @return calculated difference between given comperator and recent
         */
        inline sg_cpu_stats operator () ( sg_cpu_stats const &comperator, sg_cpu_stats const &recent ) const;
    };

    /**
     * data source for CPU statistics
     */
    class DataSourceCPU
        : public DataSourceStatgrab
        , public DataDiff<sg_cpu_stats>
    {
    public:
        /**
         * destructor
         */
        virtual ~DataSourceCPU() {}

        /**
         * accessor to the controlled MibObject instance bound to SM_CPU_USAGE
         *
         * This method returns the controlled MibObject instance containing
         * the CPU related measuring values. If there is none, it creates a
         * MibObject instance, binds it to SM_CPU_USAGE and fill it initially
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
         * This method fetches the current cpu statistics via the
         * sg_get_cpu_stats function from the statgrab library and updates
         * the desired mib leafs.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool updateMibObj();

        /**
         * get the single instance of this data source for cpu statistics
         *
         * @return DataSourceCPU & - reference to this instance
         */
        static DataSourceCPU & getInstance();
        /**
         * destroys singleton instance
         */
        static void destroyInstance();

    protected:
        /**
         * default constructor
         */
        DataSourceCPU()
            : DataSourceStatgrab()
            , DataDiff<sg_cpu_stats>()
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
#endif
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_CPU_H_INCLUDED__ */
