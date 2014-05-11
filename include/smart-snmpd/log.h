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
#ifndef __SMART_SNMPD_LOG_H_INCLUDED__
#define __SMART_SNMPD_LOG_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/config.h>

#include <agent_pp/snmp_pp_ext.h>

# ifdef WITH_LIBLOG4CPLUS
#  include <smart-snmpd/log4cplus.h>
# else
#  include <snmp_pp/log.h>
# endif

#include <string>
#include <sstream>

using namespace std;

namespace SmartSnmpd
{
    /**
     * lexical cast function to convert anything - which can be put into a
     * stream - into a string
     *
     * @param v - the value to convert into a string representation
     *
     * @return string - converted representation
     */
    template<typename T>
    std::string
    to_string(T const &v)
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }

}

#endif /* __SMART_SNMPD_LOG_H_INCLUDED__ */
