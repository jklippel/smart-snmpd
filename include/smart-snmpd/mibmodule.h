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
#ifndef __SMART_SNMPD_MIBMODULE_H_INCLUDED__
#define __SMART_SNMPD_MIBMODULE_H_INCLUDED__

#include <smart-snmpd/config.h>
#include <agent_pp/agent++.h>

namespace SmartSnmpd
{
    struct MibModule
    {
        virtual bool InitModule(void) = 0;
        virtual bool ShutdownModule(void) = 0;

        virtual bool RefreshConfig(NS_AGENT Mib &mainMibCtrl) = 0;

        virtual bool RegisterMibs(NS_AGENT Mib &mainMibCtrl) = 0;
        virtual bool UnregisterMibs(NS_AGENT Mib &mainMibCtrl) = 0;
    };
}

#endif /* ?__SMART_SNMPD_MIBMODULE_H_INCLUDED__ */
