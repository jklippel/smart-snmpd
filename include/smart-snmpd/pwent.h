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
#ifndef __SMART_SNMPD_PWENT_H_INCLUDED__
#define __SMART_SNMPD_PWENT_H_INCLUDED__

#include <smart-snmpd/sysbuf.h>

#include <string>
#include <map>

#ifndef WIN32
#include <sys/types.h>
#endif

namespace SmartSnmpd
{
    /**
     * provides access to information of system user accounts
     */
    class SystemUserInfo
    {
    public:
#ifdef WIN32
        //! Win32 compatible user id type
        typedef DWORD uid_t;
        //! Win32 compatible group id type
        typedef DWORD gid_t;
#else
        //! XPG compatible user id type
        typedef ::uid_t uid_t;
        //! XPG compatible group id type
        typedef ::gid_t gid_t;
#endif

        /**
         * default constructor
         */
        inline SystemUserInfo()
            : mPwNameCache()
            , mPwUidCache()
            , mGrNameCache()
            , mGrGidCache()
#ifndef WIN32
            , mSysBuf( guess_pwent_bufsize() )
#endif
        {}

        /**
         * destructor
         */
        ~SystemUserInfo() {}

        /**
         * delivers the user name from the user id
         *
         * This method tries to resolve the user name from given user id.
         * The local used buffer objects are not reentrant. Avoid using
         * the same object from different threads.
         *
         * @param id - user id
         * @return string const & - reference to a string containing the
         *  user name, the string is empty if no name could resolved
         */
        string const & getusernamebyuid(uid_t id);
        /**
         * delivers the group name from the group id
         *
         * This method tries to resolve the group name from given group id.
         * The local used buffer objects are not reentrant. Avoid using
         * the same object from different threads.
         *
         * @param id - group id
         * @return string const & - reference to a string containing the
         *  group name, the string is empty if no name could resolved
         */
        string const & getgroupnamebygid(gid_t id);
        /**
         * delivers the user id from the user name
         *
         * This method tries to resolve the user id from given user name.
         * The local used buffer objects are not reentrant. Avoid using
         * the same object from different threads.
         *
         * @param name - string containing the user name to retrieve the id
         * @return uid_t - user id, (uid_t)-1 if no id could resolved
         */
        uid_t getuseridbyname(string const &name);
        /**
         * delivers the group id from the group name
         *
         * This method tries to resolve the group id from given group name.
         * The local used buffer objects are not reentrant. Avoid using
         * the same object from different threads.
         *
         * @param name - string containing the group name to retrieve the id
         * @return gid_t - group id, (gid_t)-1 if no id could resolved
         */
        gid_t getgroupidbyname(string const &name);

    protected:
        std::map<string, uid_t> mPwNameCache;
        std::map<uid_t, string> mPwUidCache;
        std::map<string, gid_t> mGrNameCache;
        std::map<gid_t, string> mGrGidCache;
#ifndef WIN32
        /**
         * buffer object to allow reentrant system calls to store data
         */
        SystemBuffer<char> mSysBuf;
#endif

        static size_t guess_pwent_bufsize();
    };
}

#endif /* __SMART_SNMPD_PWENT_H_INCLUDED__ */
