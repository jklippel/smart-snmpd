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
#ifndef __SMART_SNMPD_DATASOURCE_STATGRAB_H_INCLUDED__
#define __SMART_SNMPD_DATASOURCE_STATGRAB_H_INCLUDED__

#include <smart-snmpd/datasource.h>

namespace SmartSnmpd
{
    /**
     * data source for statistics from statgrab library
     */
    class DataSourceStatgrab
        : public DataSource
    {
        friend struct StatgrabMibModule;

    public:
        /**
         * destructor
         */
        virtual ~DataSourceStatgrab() {}

    protected:
        /**
         * default constructor
         */
        inline DataSourceStatgrab()
            : DataSource()
        {}

        /**
         * initializes statgrab library
         *
         * @return bool - true when initialization succeeds and false oetherwise (error is logged)
         */
        static bool InitStatgrab();
        /**
         * shuts down statgrab library
         *
         * @return bool - true when shutting down succeeds and false oetherwise (error is logged)
         */
        static bool ShutdownStatgrab();

        /**
         * grabs error information from statgrab library and reports it (into log target)
         *
         * @param who - who detects the error (eg. full qualified method name)
         * @param what - what has been failed (short action description, eg. which statgrab call failed)
         */
        static void report_sg_error(string const &who, string const &what);
    };
}

#endif /* __SMART_SNMPD_DATASOURCE_STATGRAB_H_INCLUDED__ */
