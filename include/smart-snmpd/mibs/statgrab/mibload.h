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
#ifndef __SMART_SNMPD_MIB_LOAD_H_INCLUDED__
#define __SMART_SNMPD_MIB_LOAD_H_INCLUDED__

#include <smart-snmpd/mibobject.h>

#include <statgrab.h>

#include <iomanip>
#include <sstream>

namespace SmartSnmpd
{
    class LoadMib
    {
    public:
        virtual ~LoadMib() {}

        virtual LoadMib & setRealLoad( sg_load_stats const &load ) = 0;
        virtual LoadMib & setNormalizedLoad( sg_load_stats const &load ) = 0;

        virtual LoadMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        LoadMib() {}
    };

    class SmartSnmpdLoadMib
        : public LoadMib
    {
    public:
        SmartSnmpdLoadMib( MibObject::ContentManagerType &aCntMgr )
            : LoadMib()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mRealLoad1Float( aCntMgr, SM_SYSTEM_LOAD1_REAL_FLOAT_KEY )
            , mRealLoad5Float( aCntMgr, SM_SYSTEM_LOAD5_REAL_FLOAT_KEY )
            , mRealLoad15Float( aCntMgr, SM_SYSTEM_LOAD15_REAL_FLOAT_KEY )
            , mRealLoad1Int( aCntMgr, SM_SYSTEM_LOAD1_REAL_INTEGER_KEY )
            , mRealLoad5Int( aCntMgr, SM_SYSTEM_LOAD5_REAL_INTEGER_KEY )
            , mRealLoad15Int( aCntMgr, SM_SYSTEM_LOAD15_REAL_INTEGER_KEY )
            , mNormalizedLoad1Float( aCntMgr, SM_SYSTEM_LOAD1_NORM_FLOAT_KEY )
            , mNormalizedLoad5Float( aCntMgr, SM_SYSTEM_LOAD5_NORM_FLOAT_KEY )
            , mNormalizedLoad15Float( aCntMgr, SM_SYSTEM_LOAD15_NORM_FLOAT_KEY )
            , mNormalizedLoad1Int( aCntMgr, SM_SYSTEM_LOAD1_NORM_INTEGER_KEY )
            , mNormalizedLoad5Int( aCntMgr, SM_SYSTEM_LOAD5_NORM_INTEGER_KEY )
            , mNormalizedLoad15Int( aCntMgr, SM_SYSTEM_LOAD15_NORM_INTEGER_KEY )
        {}

        virtual ~SmartSnmpdLoadMib() {}

        virtual LoadMib & setRealLoad( sg_load_stats const &load )
        {
            mRealLoad1Float.set( fmtLoad(load.min1) );
            mRealLoad5Float.set( fmtLoad(load.min5) );
            mRealLoad15Float.set( fmtLoad(load.min15) );
            mRealLoad1Int.set( (unsigned long)(load.min1 * 100) );
            mRealLoad5Int.set( (unsigned long)(load.min5 * 100) );
            mRealLoad15Int.set( (unsigned long)(load.min15 * 100) );

            return *this;
        }

        virtual LoadMib & setNormalizedLoad( sg_load_stats const &load )
        {
            mNormalizedLoad1Float.set( fmtLoad(load.min1) );
            mNormalizedLoad5Float.set( fmtLoad(load.min5) );
            mNormalizedLoad15Float.set( fmtLoad(load.min15) );
            mNormalizedLoad1Int.set( (unsigned long)(load.min1 * 100) );
            mNormalizedLoad5Int.set( (unsigned long)(load.min5 * 100) );
            mNormalizedLoad15Int.set( (unsigned long)(load.min15 * 100) );

            return *this;
        }

        virtual LoadMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );
            return *this;
        }

    protected:
        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mRealLoad1Float;
        MibObject::ContentManagerType::LeafType mRealLoad5Float;
        MibObject::ContentManagerType::LeafType mRealLoad15Float;
        MibObject::ContentManagerType::LeafType mRealLoad1Int;
        MibObject::ContentManagerType::LeafType mRealLoad5Int;
        MibObject::ContentManagerType::LeafType mRealLoad15Int;
        MibObject::ContentManagerType::LeafType mNormalizedLoad1Float;
        MibObject::ContentManagerType::LeafType mNormalizedLoad5Float;
        MibObject::ContentManagerType::LeafType mNormalizedLoad15Float;
        MibObject::ContentManagerType::LeafType mNormalizedLoad1Int;
        MibObject::ContentManagerType::LeafType mNormalizedLoad5Int;
        MibObject::ContentManagerType::LeafType mNormalizedLoad15Int;

        static std::string fmtLoad( double d, int precision = 2 )
        {
            ostringstream oss;

            if( precision )
                oss << fixed << setprecision(2);
            oss << d;

            return oss.str();
        }

    private:
        SmartSnmpdLoadMib();
    };
}

#endif /* __SMART_SNMPD_MIB_LOAD_H_INCLUDED__ */
