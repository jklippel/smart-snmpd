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
#ifndef __SMART_SNMPD_PARSEEXTJSON_H_INCLUDED__
#define __SMART_SNMPD_PARSEEXTJSON_H_INCLUDED__

#include <smart-snmpd/smart-snmpd.h>
#include <agent_pp/snmp_pp_ext.h>

#include <string>
#include <vector>

#ifdef WITH_BUNDLED_LIBJSON
#include <smart-snmpd/mibs/extcmd/json.h>
#else
#include <json.h>
#endif

namespace SmartSnmpd
{
    /**
     * VarBind replacement for reusing on parsing
     */
    struct ExternalDataTuple
    {
        /**
         * Oid part of the tuple
         */
        NS_AGENT Oidx Oid;
        /**
         * value part of the tuple - always owned by this instance
         */
        NS_SNMP SnmpSyntax *Datum;

        /**
         * default constructor
         */
        inline ExternalDataTuple()
            : Oid()
            , Datum(0)
        {}

        /**
         * copy constructor - clones the Datum attribute of the source, if any
         *
         * @param ref - copy source
         */
        inline ExternalDataTuple(const ExternalDataTuple &ref)
            : Oid( ref.Oid )
            , Datum( ref.Datum ? ref.Datum->clone() : 0 )
        {}

        /**
         * destructor
         *
         * owned Datum attribute is freed when the destructor is called
         */
        inline virtual ~ExternalDataTuple() { delete Datum; Datum = 0; }

        /**
         * assignes the attribute values of the referenced object to this object
         *
         * If the source object contains a Datum instance, this object
         * will use a cloned copy of this instance.
         *
         * @param ref - reference to the instance to copy from
         */
        inline ExternalDataTuple & operator = (const ExternalDataTuple &ref)
        {
            if( &ref == this )
                return *this;

            Oid = ref.Oid;
            delete Datum;
            Datum = ref.Datum ? ref.Datum->clone() : 0;

            return *this;
        }

        /**
         * clears the content of this object
         */
        inline void clear()
        {
            Oid.clear();
            delete Datum;
            Datum = 0;
        }
    };

    /**
     * parses a json serialized list
     *
     * This class helps parsing JSON encoded serialized snmp items.
     * The serialized document must represent a list of three-piece-tuples
     * with the elements (oid, type, value). The oid must be passed through
     * in string representation, the type is the ASN1 type and the value
     * can be one of ASN_INTEGER, SMI_OPAQUE, ASN_OCTET_STR, SMI_IPADDRESS,
     * SMI_COUNTER, SMI_GAUGE, SMI_TIMETICKS, SMI_COUNTER64 or SMI_UINTEGER:
     * <code>[ [ ".1", 4, "foo" ], [ ".2", 2, 42 ], ... ]</code>
     *
     * ParseExternalJson are not MT-safe.
     */
    class ParseExternalJson
    {
    public:
        /**
         * default constructor
         */
        ParseExternalJson();
        /**
         * destructor
         */
        virtual ~ParseExternalJson();

        /**
         * parses content of specified string
         *
         * @param buf - string buffer containing json serialized content
         *
         * @return int - 0 on success, -1 on error
         */
        int parse(string const &buf);

        /**
         * access the parsed data
         *
         * @return vector<ExternalDataTuple> const & - reference to vector containing the parsed data
         */
        inline vector<ExternalDataTuple> const & getData() const { return mData; }

        /**
         * allows to reserve room to store expected data without reallocations during parsing
         *
         * @param n - amount of expected elements
         *
         * @return ParseExternalJson & - reference to this instance
         */
        inline ParseExternalJson & reserve(vector<ExternalDataTuple>::size_type n) { mData.reserve(n); return *this; }

    protected:
        //! parser state
        enum {
            pejInitial,
            pejListEntered,
            pejTupleEntered,
            pejOidParsed,
            pejTypeParsed,
            pejDatumParsed
        } mParserState;
        //! parse instance
        struct json_parser mJsonParser;
        //! syntax format of next value to parse
        SmiUINT32 mSyntax;
        //! parsed data
        vector<ExternalDataTuple> mData;

        /**
         * parse routine called from json parser
         *
         * @param type - json type
         * @param data - pointer to the begin of the json data
         * @param length - length of submitted data
         *
         * @return 0 on success, JSON_ERROR_CALLBACK on error parsing data
         */
        virtual int parse_item(int type, const char *data, uint32_t length);

    private:
        /**
         * wrapper to be called from json parser
         *
         * @param userdata - pointer to the calling ParseExternalJson object
         * @param type - json type
         * @param data - pointer to the begin of the json data
         * @param length - length of submitted data
         *
         * @return 0 on success, JSON_ERROR_CALLBACK on error parsing data
         */
        static int parser_callback(void *userdata, int type, const char *data, uint32_t length);
    };
}

#endif /* __SMART_SNMPD_PARSEEXTJSON_H_INCLUDED__ */
