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
#ifndef __SMART_SNMPD_DATASOURCE_DAEMONSTATUS_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_DAEMONSTATUS_H_INCLUDED__

#include <smart-snmpd/mibs/statgrab/datasourcestatgrab.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    /**
     * data source for daemon status
     */
    class DataSourceDaemonStatus
        : public DataSourceStatgrab
    {
    public:
        /**
         * contains information about memory consumption of the smart-snmpd
         *
         * This structure combines different views to the behavior of
         * memory consumption of the smart-snmpd. To understand the mean
         * value computing, refer either to
         * - http://en.wikipedia.org/wiki/Standard_deviation or
         * - Donald E. Knuth: The Art of Computer Programming. Volume 2: Seminumerical Algorithms. (p232)
         */
        struct MemoryInfo
        {
            MemoryInfo()
                : mInitialValue( 0 )
                , mCurrentValue( 0 )
                , mMeanValue( 0 )
                , mMaxValue( 0 )
                , mMaxViolations( 0 )
                , mMeanSqrValue( 0 )
                , mMeanErrDistance( 0 )
                , mMeanViolations( 0 )
            {}
            /**
             * initial consumed memory (usually measured on initMibObj)
             */
            unsigned long long mInitialValue;
            /**
             * currently consumed memory
             */
            unsigned long long mCurrentValue;
            /**
             * mean memory consumption
             */
            unsigned long long mMeanValue;
            /**
             * maximum measured memory consumption value
             */
            unsigned long long mMaxValue;
            /**
             * counter increased each time when the current measured memory 
             * consumption exceeds the saved maximum memory consumption
             */
            unsigned long mMaxViolations;
            /**
             * mean square value of measured memory consumption
             */
            unsigned long long mMeanSqrValue;
            /**
             * error distance of measured memory consumption compared to
             * mean value
             */
            unsigned long long mMeanErrDistance;
            /**
             * counter increased each time when current measured memory
             * consumption exceeds the error distance compared to the mean
             * memory consumption
             */
            unsigned long mMeanViolations;

            inline void init( unsigned long long curMeasuredValue )
            {
                mMeanValue = mInitialValue = mMaxValue = mCurrentValue = curMeasuredValue;
            }

            void updateMemoryInfo( unsigned long long curMeasuredValue, unsigned long measureCount );
        };

        /**
         * destructor
         */
        virtual ~DataSourceDaemonStatus() {}

        /**
         * accessor to the controlled MibObject instance bound to SM_DAEMON_STATUS
         *
         * This method returns the controlled MibObject instance containing
         * the smart-snmpd process statistic related measuring values. If
         * there is none, it creates a MibObject instance, binds it to
         * SM_DAEMON_STATUS and fill it initially with reasonable basic
         * configuration.
         *
         * @return MibObject * - controlled MibObject
         */
        virtual MibObject * getMibObject();
        /**
         * initialize controlled mib object
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool initMibObj();
        /**
         * updates the managed mib object
         *
         * This method fetches the current process statistics via the
         * sg_get_process_stats function from the statgrab library, searches
         * for and the entry of this daemon (comparing pid against getpid())
         * and updates the desired mib leafs.
         *
         * @return bool - true when successful, false otherwise
         */
        virtual bool updateMibObj();

        /**
         * get the single instance of this data source for daemon status (of this daemon)
         *
         * @return DataSourceDaemonStatus & - reference to this instance
         */
        static DataSourceDaemonStatus & getInstance();
        /**
         * destroys singleton instance
         */
        static void destroyInstance();

    protected:
        unsigned long long mMeasureCount;
        MemoryInfo mResidentMemoryInfo;
        MemoryInfo mVirtualMemoryInfo;

        /**
         * default constructor
         */
        DataSourceDaemonStatus()
            : DataSourceStatgrab()
            , mMeasureCount(0)
            , mResidentMemoryInfo()
            , mVirtualMemoryInfo()
        {}

        sg_process_stats * findCurrentProcessStats();
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_DAEMONSTATUS_H_INCLUDED__ */
