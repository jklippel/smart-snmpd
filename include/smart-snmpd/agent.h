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
#ifndef __SMART_SNMPD_AGENT_H_INCLUDED__
#define __SMART_SNMPD_AGENT_H_INCLUDED__

#include <agent_pp/agent++.h>
#include <agent_pp/snmp_group.h>
#include <agent_pp/system_group.h>
#include <agent_pp/snmp_target_mib.h>
#include <agent_pp/snmp_notification_mib.h>
#include <agent_pp/notification_originator.h>
#include <agent_pp/mib_complex_entry.h>
#include <agent_pp/v3_mib.h>
#include <agent_pp/vacm.h>

#include <snmp_pp/oid_def.h>
#include <snmp_pp/mp_v3.h>

#include <smart-snmpd/datasource.h>
#include <smart-snmpd/functional.h>
#include <smart-snmpd/mibmodule.h>

#include <vector>

using namespace std;

namespace SmartSnmpd
{
    class Agent
    {
    public:
        //! daefault constructor
        Agent();
        //! destructor
        virtual ~Agent();

        /**
         * initializes the snmp agent
         *
         * This method initializes the configured data sources and
         * configures all global agent parameters. Additionally UserInit()
         * and VacmInit() are called to configure access control.
         */
        virtual void Init();
        //! initializes the user mibs
        virtual void UserInit();
        //! initializes VACM
        virtual void VacmInit();

        //! snmp agent main loop - handles received requests and controls graceful shutdowns
        virtual void Run();
        //! set appropriate flags to go down
        //! @param signo - catched signal
        virtual void Stop(int signo) { mSignal = signo; } // external interrupt, e.g. signals
        //! delivers run state
        virtual inline bool isRunning() const { return mRunning; }
        //! refreshes the configuration of controlled mib objects
        virtual int RefreshMibConfig();

    protected:
        //! agent mib object (container for all mibs propagated by the smart-snmpd)
        NS_AGENT Mib *mMib;
        //! SNMP request list
        NS_AGENT RequestList *mReqList;
        //! SNMP (message) handler
        NS_AGENT Snmpx *mSnmp;
        //! SNMPv3 Message Processing Entity
        v3MP *mv3mp;
        //! Vacm instance (access control)
        Vacm *mVacm;
        //! signal to flag shutdown
        int mSignal;
        //! flag for business as usual
        bool mRunning;
        //! loaded modules providing mibs
        vector<MibModule *> mMibModules;

#if 0
        //! controlled data sources initializer
        virtual void InitDataSources();
#endif
        
    private:
    };
}

#endif /* __SMART_SNMPD_AGENT_H_INCLUDED__ */
