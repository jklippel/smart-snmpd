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
#ifndef __SMART_SNMPD_MIB_USERLOGIN_H_INCLUDED__
#define __SMART_SNMPD_MIB_USERLOGIN_H_INCLUDED__

#include <smart-snmpd/mibobject.h>
#include <smart-snmpd/pwent.h>

#include <statgrab.h>

namespace SmartSnmpd
{
    class UserLoginMib
    {
    public:
        virtual ~UserLoginMib() {}

        virtual UserLoginMib & setCount( unsigned long long nelem ) = 0;
        virtual UserLoginMib & addRow( sg_user_stats const &login ) = 0;
        virtual UserLoginMib & setUpdateTimestamp( unsigned long long secsSinceEpoch ) = 0;

    protected:
        UserLoginMib() {}
    };

    class SmartSnmpdUserLoginMib
        : public UserLoginMib
    {
    public:
        SmartSnmpdUserLoginMib( MibObject::ContentManagerType &aCntMgr )
            : UserLoginMib()
            , mSysUserInfo()
            , mUpdateTimestamp( aCntMgr, SM_LAST_UPDATE_MIB_KEY )
            , mRowCount( aCntMgr, SM_USER_LOGIN_COUNT_KEY )
            , mUserLogins( aCntMgr, SM_USER_LOGIN_TABLE_KEY, 10 )
        {}

        virtual ~SmartSnmpdUserLoginMib() {}

        virtual UserLoginMib & setCount( unsigned long long nelem )
        {
            mRowCount.set( nelem );
            return *this;
        }

        virtual UserLoginMib & addRow( sg_user_stats const &login )
        {
            mUserLogins.addRow().setCurrentColumn( mUserLogins.getLastRowIndex() + 1 )
                                .setCurrentColumn( login.login_name ? OctetStr( login.login_name ) : OctetStr() )
                                .setCurrentColumn( login.login_name ? SnmpUInt32( mSysUserInfo.getuseridbyname( login.login_name ) ) : SnmpUInt32((unsigned long)(-1)) )
                                .setCurrentColumn( login.device ? OctetStr( login.device ) : OctetStr()  )
                                .setCurrentColumn( Counter64(login.login_time) )
                                .setCurrentColumn() // SM_USER_LOGIN_IDLE_TIME_KEY
                                .setCurrentColumn() // SM_USER_LOGIN_JCPU_TIME_KEY
                                .setCurrentColumn() // SM_USER_LOGIN_PCPU_TIME_KEY
                                .setCurrentColumn() // SM_USER_LOGIN_WHAT_KEY
                                .setCurrentColumn( login.hostname ? OctetStr( login.hostname ) : OctetStr() );

            return *this;
        }

        virtual UserLoginMib & setUpdateTimestamp( unsigned long long secsSinceEpoch )
        {
            mUpdateTimestamp.set( secsSinceEpoch );
            return *this;
        }

    protected:
        /**
         * system user information accessor
         */
        SystemUserInfo mSysUserInfo;

        MibObject::ContentManagerType::LeafType mUpdateTimestamp;
        MibObject::ContentManagerType::LeafType mRowCount;
        MibObject::ContentManagerType::TableType mUserLogins;

    private:
        SmartSnmpdUserLoginMib();
    };
}

#endif /* __SMART_SNMPD_MIB_USERLOGIN_H_INCLUDED__ */
