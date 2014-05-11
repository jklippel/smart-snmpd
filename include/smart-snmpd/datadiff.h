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
#ifndef __SMART_SNMPD_DATA_DIFF_H_INCLUDED__
#define __SMART_SNMPD_DATA_DIFF_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <queue>
#include <functional>

namespace SmartSnmpd
{
    /**
     * class encompassing the diff operator
     *
     * @param T - the type to calculate differences for
     */
    template <class T>
    class calc_diff
        : public binary_function<T, T, T>
    {
    public:
        /**
         * diff operator
         *
         * This operator is called to calculate the difference between a recent
         * value and an earlier measured comperator by subtracting the comperator
         * from the recent value.
         *
         * Implement specializations of this operator is valuable when the
         * subtraction operator can't be overloaded.
         *
         * @param comperator - operand to compare against the most recent value
         * @param recent - the most recent value
         *
         * @return calculated difference between given comperator and recent
         */
        inline T operator () ( T const &comperator, T const &recent ) const { return recent - comperator; }
    };

    /**
     * base class to calculate differences between measuring values
     *
     * @param T - the type of measuring values to manage and calculate
     *            differences for.
     * @param Differ - the difference calculating class
     *
     * @todo rewrite to use allocators for pointer to measuring values
     */
    template <class T, class Differ = calc_diff< T > >
    class DataDiff
    {
    public:
        /**
         * destructor
         *
         * This destructor calls freeItem for each managed measuring value
         * and the lastest calculated difference
         */
        virtual ~DataDiff()
        {
            if( mValidDiffResult )
            {
                mValidDiffResult = false;
                freeItem(mDiffResult);
            }

            while( !mHistory.empty() )
            {
                freeItem( mHistory.front() );
                mHistory.pop();
            }
        } 

    protected:
        /**
         * history of measuring values
         */
        std::queue <T> mHistory;
        /**
         * maximum size of history
         */
        size_t mHistoryMaxSize;
        /**
         * difference calculator
         */
        Differ mDiffer;
        /**
         * difference result
         */
        T mDiffResult;
        /**
         * true when mDiffResult contains some data
         */
        bool mValidDiffResult;

        // will be singletons ...
        /**
         * default constructor
         *
         * The default constructor is protected to allow deriving into singletons
         */
        inline DataDiff()
            : mHistory()
            , mHistoryMaxSize(1)
            , mDiffer()
            , mDiffResult()
            , mValidDiffResult(false)
        {}

        /**
         * simple allocator replacement to free no more measuring values
         *
         * This method should be specialized when there's anything to do
         * to free (deallocate) an item. It should call the destructor and
         * the deallocation routine to release the memory.
         *
         * @param t - the measuring value to be freed
         */
        inline void freeItem( T &t ) { (void)t; }

        /**
         * setup the history buffer
         *
         * This method configures the history buffer to fit enough elements
         * to collect one object per update interval and have room for
         * mrInterval / cacheTime measuring values to calcultate the difference
         * between new measuring values and the last one before the measuring
         * interval is exceeded.
         *
         * @param mibObj - the MibObject instance to get the configured parameters from
         */
        inline void setupHistory(MibObject const &mibObj)
        {
            time_t cacheTime = mibObj.getConfig().CacheTime;
            time_t mrInterval = mibObj.getConfig().MostRecentIntervalTime;
            mHistoryMaxSize = cacheTime ? mrInterval / cacheTime : 1;
            while( mHistory.size() > mHistoryMaxSize )
            {
                freeItem( mHistory.front() );
                mHistory.pop();
            }
        }

        /**
         * get the difference against recent measuring value
         *
         * This method returns the difference between the most recent and
         * the oldest available measuring value. If there is no measurng
         * value available to compare against, the given measuring value
         * is stored for later comparison with a later "recent measuring value".
         *
         * @param mostRecent - the most recent measuring value
         *
         * @return T const & - reference to immutable difference result
         */
        inline T const & diff(T const &mostRecent)
        {
            if( mValidDiffResult )
            {
                mValidDiffResult = false;
                freeItem( mDiffResult );
                memset( &mDiffResult, 0, sizeof(mDiffResult) );
            }

            if( mHistory.empty() )
            {
                mHistory.push( mostRecent );
                return mostRecent;
            }
            else
            {
                T const &comperator = mHistory.front();
                mDiffResult = mDiffer( comperator, mostRecent );
                mValidDiffResult = true;

                mHistory.push( mostRecent );
                while( mHistory.size() > mHistoryMaxSize )
                {
                    freeItem( mHistory.front() );
                    mHistory.pop();
                }

                return mDiffResult;
            }
        }
    };
}

#endif /* __SMART_SNMPD_DATA_DIFF_H_INCLUDED__ */
