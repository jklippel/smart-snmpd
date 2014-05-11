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
#ifndef __SMART_SNMPD_MIB_HOSTINFO_H_INCLUDED__
#define __SMART_SNMPD_MIB_HOSTINFO_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class HostInfoMib
    {
    public:
        virtual ~HostInfoMib() {}

        virtual HostInfoMib & setHostInfo( sg_host_info const &hostInfo ) = 0;
        virtual HostInfoMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        HostInfoMib() {}
    };

    class SmartSnmpdHostInfoMib
        : public HostInfoMib
    {
    public:
        SmartSnmpdHostInfoMib( MibObject::ContentManagerType &aCntMgr )
            : HostInfoMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mHostname( aCntMgr, SM_HOSTNAME_KEY )
            , mOsName( aCntMgr, SM_HOST_OS_NAME_KEY )
            , mOsRelease( aCntMgr, SM_HOST_OS_RELEASE_KEY )
            , mOsVersion( aCntMgr, SM_HOST_OS_VERSION_KEY )
            , mPlatform( aCntMgr, SM_HOST_PLATFORM_KEY )
            , mUptime( aCntMgr, SM_HOST_UPTIME_KEY )
            , mBitWidth( aCntMgr, SM_HOST_BITWIDTH_KEY )
            , mVirtualized( aCntMgr, SM_HOST_VIRTUALIZED_KEY )
            , mCpuCountMin( aCntMgr, SM_HOST_CPU_COUNT_MIN_KEY )
            , mCpuCountMax( aCntMgr, SM_HOST_CPU_COUNT_MAX_KEY )
            , mCpuCountCur( aCntMgr, SM_HOST_CPU_COUNT_CURRENT_KEY )
        {}

        virtual ~SmartSnmpdHostInfoMib() {}

        virtual HostInfoMib & setHostInfo( sg_host_info const &hostInfo )
        {
            mHostname.set( hostInfo.hostname ? hostInfo.hostname : "" );
            mOsName.set( hostInfo.os_name ? hostInfo.os_name : "" );
            mOsRelease.set( hostInfo.os_release ? hostInfo.os_release : "" );
            mOsVersion.set( hostInfo.os_version ? hostInfo.os_version : "" );
            mPlatform.set( hostInfo.platform ? hostInfo.platform : "" );
            mUptime.set( hostInfo.uptime );
            mBitWidth.set( hostInfo.bitwidth );
            mVirtualized.set( (unsigned long)(hostInfo.host_state) );
            mCpuCountMin.set( 1 ); // XXX not determined
            mCpuCountMax.set( hostInfo.maxcpus );
            mCpuCountCur.set( hostInfo.ncpus );

            return *this;
        }

        virtual HostInfoMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );
            return *this;
        }

    protected:
        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mHostname;
        MibObject::ContentManagerType::LeafType mOsName;
        MibObject::ContentManagerType::LeafType mOsRelease;
        MibObject::ContentManagerType::LeafType mOsVersion;
        MibObject::ContentManagerType::LeafType mPlatform;
        MibObject::ContentManagerType::LeafType mUptime;
        MibObject::ContentManagerType::LeafType mBitWidth;
        MibObject::ContentManagerType::LeafType mVirtualized;
        MibObject::ContentManagerType::LeafType mCpuCountMin;
        MibObject::ContentManagerType::LeafType mCpuCountMax;
        MibObject::ContentManagerType::LeafType mCpuCountCur;

    private:
        SmartSnmpdHostInfoMib();
    };
}

#endif /* __SMART_SNMPD_MIB_HOSTINFO_H_INCLUDED__ */
