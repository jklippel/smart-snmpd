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

#include <smart-snmpd/resourcelimits.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.resourcelimits";

static const ResourceLimits InitialSoftLimits( ResourceSoft );
static const ResourceLimits InitialHardLimits( ResourceHard );

ResourceLimit &
ResourceLimit::fetch()
{
    if( !empty() && !fix() )
    {
        // this is a bit tricky: during initialization phase each limit is added
        set<ResourceLimit>::const_iterator ci = mFlags & ResourceSoft ? InitialSoftLimits.find(mResource) : InitialHardLimits.find(mResource);
        if( ci == ( mFlags & ResourceSoft ? InitialSoftLimits.end() : InitialHardLimits.end() ) )
        {
            struct rlimit cur_limit;
            if( -1 == getrlimit( mResource, &cur_limit ) )
            {
                LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
                LOG( "getrlimit(rlimit): (error)" );
                LOG( mResource );
                LOG( strerror( errno ) );
                LOG_END;
            }
            mValue = mFlags & ResourceSoft ? cur_limit.rlim_cur : cur_limit.rlim_max;
        }
        else
        {
            mValue = ci->mValue;
        }
    }

    return *this;
}

ResourceLimit const &
ResourceLimit::commit() const
{
    if( filled() )
    {
        struct rlimit cur_limit;
        if( -1 == getrlimit( mResource, &cur_limit ) )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG( "getrlimit(rlimit): (error)" );
            LOG( mResource );
            LOG( strerror( errno ) );
            LOG_END;
        }

        if( hard() )
        {
            cur_limit.rlim_max = mValue;
        }

        if( soft() )
        {
            cur_limit.rlim_cur = mValue;
        }

        if( -1 == setrlimit( mResource, &cur_limit ) )
        {
            LOG_BEGIN( loggerModuleName, ERROR_LOG | 0 );
            LOG( "setrlimit(rlimit)(soft)(hard): (error)" );
            LOG( mResource );
            LOG( cur_limit.rlim_cur );
            LOG( cur_limit.rlim_max );
            LOG( strerror( errno ) );
            LOG_END;
        }
    }

    return *this;
}

}
