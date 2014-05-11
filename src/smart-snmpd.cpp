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
#include <smart-snmpd/config.h>
#include <smart-snmpd/cmndline.h>
#include <smart-snmpd/agent.h>
#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/ui.h>
//#include <smart-snmpd/>

#ifndef _SNMPv3
#error smart-snmpd requires SNMPv3 support
#endif

using namespace SmartSnmpd;

int
main (int argc, char **argv)
{
    UI ui(argc, argv);
    int rc = ui.run();
    for( int i = 0; i < 10; ++i )
        sched_yield();
    return rc;
}
