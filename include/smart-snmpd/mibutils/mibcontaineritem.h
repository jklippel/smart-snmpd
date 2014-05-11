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
#ifndef __SMART_SNMPD_MIB_CONTAINER_ITEM_H_INCLUDED__
#define __SMART_SNMPD_MIB_CONTAINER_ITEM_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <smart-snmpd/config.h>
#include <smart-snmpd/functional.h>
#include <smart-snmpd/mibutils/mibcontainer.h>

#include <string>
#include <set>
#include <stdexcept>

namespace SmartSnmpd
{
    class MibContainerItem
    {
    public:
        MibContainerItem( MibContainer &aContainer )
            : mContainer( aContainer )
        {}

        MibContainerItem( MibContainerItem const &ref )
            : mContainer( ref.mContainer )
        {}

        virtual ~MibContainerItem() {}

    protected:
        MibContainer &mContainer;

    private:
        MibContainerItem();
    };
}

#endif /* __SMART_SNMPD_MIB_CONTAINER_ITEM_H_INCLUDED__ */
