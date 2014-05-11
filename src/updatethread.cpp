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
#include <build-smart-snmpd.h>

#include <smart-snmpd/oids.h>
#include <smart-snmpd/updatethread.h>
#include <smart-snmpd/datasource.h>
#include <smart-snmpd/mibobject.h>

#include <agent_pp/snmp_textual_conventions.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.updatethread";

void
UpdateThread::run()
{
    time_t cacheTime = mMibObject.getConfig().CacheTime;
    do
    {
        time_t begin = time(NULL);
        mDataSource.updateMibObj();
        do
        {
            time_t end = time(NULL);
            time_t sleepTimeMalus = end - begin;
            if( sleepTimeMalus >= cacheTime )
                break;
            else
            {
                lock();
                if( !wait( ( cacheTime - sleepTimeMalus ) * 1000 ) )
                {
                    LOG_BEGIN( loggerModuleName, DEBUG_LOG | 1 );
                    LOG( "Received a notify() for update thread of (oid)" );
                    LOG( mDataSource.getMibObject()->key()->get_printable() );
                    LOG_END;
                }
                unlock();
            }
        } while( mRunning );
    } while( mRunning );
}

}
