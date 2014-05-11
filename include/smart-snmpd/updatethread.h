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
#ifndef __SMART_SNMPD_UPDATE_THREAD_H_INCLUDED__
#define __SMART_SNMPD_UPDATE_THREAD_H_INCLUDED__

#include <agent_pp/threads.h>

namespace SmartSnmpd
{
    class MibObject;
    class DataSource;

    class UpdateThread
        : public NS_AGENT Thread
    {
    public:
        /**
         * constructor
         *
         * Constructs new UpdateThread instance
         *
         * @param aDataSource - the data source to fill the mib object
         * @param aMibObject - the mib object to be updated on a regular basis
         */
        inline UpdateThread( DataSource &aDataSource, MibObject &aMibObject )
            : NS_AGENT Thread()
            , mRunning( false )
            , mDataSource( aDataSource )
            , mMibObject( aMibObject )
        {}

        //! destructor - set appropriate flags before inherited destructor joins the thread
        virtual ~UpdateThread() { mRunning = false; notify(); }

        //! thread main method
        virtual void run();
        /**
         * starts an existing thread
         * @todo adapt Agent++ Thread API to recognize errors when starting a thread
         */
        virtual void start() { NS_AGENT Thread::start(); mRunning = is_alive(); }
        //! stops a running thread
        virtual void stop() { mRunning = false; notify(); join(); }

    protected:
        //! flag whether the thread is set running or not (do not confound this with Thread::status)
        volatile bool mRunning;
        //! data source used to update mMibObject
        DataSource &mDataSource;
        //! mib object to be updated from mDataSource
        MibObject &mMibObject;

    private:
        UpdateThread();
        UpdateThread( UpdateThread const & );
        UpdateThread & operator = ( UpdateThread const & );
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_H_INCLUDED__ */

