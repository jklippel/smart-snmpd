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

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/log4cplus.h>

namespace log4cplus
{

static log4cplus::tstring const EVENT_STRING(LOG4CPLUS_TEXT("EVENT"));
static log4cplus::tstring const empty_str;

static
tstring const &
eventToStringMethod(LogLevel ll)
{
    if(ll == EVENT_LOG_LEVEL) {
        return EVENT_STRING;
    }
    else {
        return empty_str;
    }
}

static
LogLevel
eventFromStringMethod(const tstring& s) 
{
    if(s == EVENT_STRING)
        return EVENT_LOG_LEVEL;

    return NOT_SET_LOG_LEVEL;
}

class EventLogLevelInitializer {
public:
    EventLogLevelInitializer() {
        getLogLevelManager().pushToStringMethod(eventToStringMethod);
        getLogLevelManager().pushFromStringMethod(eventFromStringMethod);
    }
};

EventLogLevelInitializer eventLogLevelInitializer_;

}

namespace SmartSnmpd
{
/*------------------------ class AgentLog4CPlus -------------------------*/

AgentLog &
AgentLog4CPlus::operator += (const LogEntry *log)
{
    LogLevel ll = LogEntry4CPlus::getLog4CPlusLogLevel( log->get_type() );
    Logger logger = Logger::getInstance( log->get_name() );
    logger.log( ll, static_cast<LogEntry4CPlus const *>(log)->get_buf().str() );

    // check if critical error
    if( FATAL_LOG_LEVEL == ll )
    {
	logger.log( INFO_LOG_LEVEL, "Exiting now" );
        switch( Config::getInstance().getOnFatalError() )
        {
        case Config::onfIgnore:
            break;

        case Config::onfRaise:
            raise( SIGTERM );
            break;

        case Config::onfKill:
            kill( getpid(), SIGTERM ); // otherwise seems it's send to the local thread
            break;

        case Config::onfExit:
            exit(255);
            break;

        case Config::onf_Exit:
            _exit(255);
            break;

        case Config::onfAbort:
            abort();
            break;
        }
    }

    return *this;
}

}
