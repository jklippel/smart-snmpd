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
#ifndef __SMART_SNMPD_DATASOURCE_MEMORY_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_MEMORY_H_INCLUDED__

#include <smart-snmpd/mibs/statgrab/datasourcestatgrab.h>

namespace SmartSnmpd
{
    /**
     * data source for memory (real/swap/virtual) statistics
     */
    class DataSourceMemory
        : public DataSourceStatgrab
    {
    public:
        /**
         * destructor
         */
        virtual ~DataSourceMemory() {}

        /**
         * accessor to the controlled MibObject instance bound to SM_MEMORY_USAGE
         *
         * This method returns the controlled MibObject instance containing
         * the memory related measuring values. If there is none, it creates a
         * MibObject instance, binds it to SM_MEMORY_USAGE and fill it initially
         * with reasonable basic configuration.
         *
         * @return MibObject * - controlled MibObject
         */
        virtual MibObject * getMibObject();
        /**
         * updates the managed mib object
         *
         * This method fetches the current memory statistics via the
         * sg_get_mem_stats and sg_get_swap_stats functions from the
         * statgrab library and updates the desired mib leafs.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool updateMibObj();

        /**
         * get the single instance of this data source for memory statistics
         *
         * @return DataSourceMemory & - reference to this instance
         */
        static DataSourceMemory & getInstance();
        /**
         * destroys singleton instance
         */
        static void destroyInstance();

    protected:
        /**
         * default constructor
         */
        DataSourceMemory()
            : DataSourceStatgrab()
        {}

#if 0
        /**
         * initialize controlled mib object
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool initMibObj();
#endif
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_MEMORY_H_INCLUDED__ */
