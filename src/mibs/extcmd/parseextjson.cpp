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

#include <smart-snmpd/mibs/extcmd/parseextjson.h>
#include <typeinfo>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.parseextjson";

static const char* json_errors[] =
{
    "SUCCESS",
    "JSON_ERROR_NO_MEMORY",
    "JSON_ERROR_BAD_CHAR",
    "JSON_ERROR_POP_EMPTY",
    "JSON_ERROR_POP_UNEXPECTED_MODE",
    "JSON_ERROR_NESTING_LIMIT",
    "JSON_ERROR_DATA_LIMIT",
    "JSON_ERROR_COMMENT_NOT_ALLOWED",
    "JSON_ERROR_UNEXPECTED_CHAR",
    "JSON_ERROR_UNICODE_MISSING_LOW_SURROGATE",
    "JSON_ERROR_UNICODE_UNEXPECTED_LOW_SURROGATE",
    "JSON_ERROR_COMMA_OUT_OF_STRUCTURE",
    "JSON_ERROR_CALLBACK"
};

#define LOG_JSON_ERROR(prefix,rc) \
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 1); \
        LOG(prefix " (error)"); \
        if( ( rc > 0 ) && ( ((unsigned)rc) < lengthof(json_errors) ) ) \
            LOG( json_errors[rc] ); \
        else \
            LOG( rc ); \
        LOG_END

ParseExternalJson::ParseExternalJson()
    : mParserState(pejInitial)
    , mJsonParser()
    , mSyntax(0)
    , mData()
{
    memset( &mJsonParser, 0, sizeof(mJsonParser) );
    int rc = json_parser_init(&mJsonParser, NULL, parser_callback, this);
    if( rc )
    {
        LOG_JSON_ERROR("ParseExternalJson::ParseExternalJson(): Can't initialize parser",rc);
        mJsonParser.callback = NULL;
    }
}

ParseExternalJson::~ParseExternalJson()
{
    if( mJsonParser.callback )
    {
        json_parser_free(&mJsonParser);
        memset( &mJsonParser, 0, sizeof(mJsonParser) );
        mJsonParser.callback = NULL;
    }
}

int
ParseExternalJson::parse(string const &buf)
{
    if( mJsonParser.callback )
    {
        mData.clear();
        size_t length = buf.size();
        size_t processed = 0;
        while( length > 0 )
        {
            uint32_t p;
            int rc = json_parser_string( &mJsonParser, buf.c_str() + processed, length > UINT32_MAX ? UINT32_MAX : (uint32_t)length, &p );
            if( rc )
            {
                LOG_JSON_ERROR("Error parsing json output:",rc);
                return -1;
            }
            processed += p;
            length -= p;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

static const char *json_parser_types[] =
{
    "JSON_OBJECT_BEGIN",
    "JSON_ARRAY_BEGIN",
    "JSON_OBJECT_END",
    "JSON_ARRAY_END",
    "JSON_KEY",
    "JSON_STRING",
    "JSON_INT",
    "JSON_FLOAT",
    "JSON_NULL",
    "JSON_TRUE",
    "JSON_FALSE"
};

static const char *pej_states[] =
{
    "pejInitial",
    "pejListEntered",
    "pejTupleEntered",
    "pejOidParsed",
    "pejTypeParsed",
    "pejDatumParsed"
};

#define LOG_PARSE_EXT_JSON_ERROR(type) \
        LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 ); \
        LOG("ParseExternalJson::parse_item(): unexpected (json item) encountered in (state)"); \
        if( ( type >= 0 ) && ( ((unsigned)type) < lengthof(json_parser_types) ) ) \
            LOG( json_parser_types[type] ); \
        else \
            LOG( type ); \
        LOG( pej_states[mParserState] ); \
        LOG_END

#define LOG_PARSE_BAD_TYPE_ERROR(type) \
        LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 ); \
        LOG("ParseExternalJson::parse_item(): unexpected (json item) encountered for specified (syntax)"); \
        if( ( type >= 0 ) && ( ((unsigned)type) < lengthof(json_parser_types) ) ) \
            LOG( json_parser_types[type] ); \
        else \
            LOG( type ); \
        LOG( mSyntax ); \
        LOG_END

static int
parse_i32_from_json(const char *data, uint32_t length) throw()
{
    int i;
    char *buf = (char *)alloca( sizeof(char) * (length+1) );
    memcpy(buf, data, length);
    buf[length] = 0;
    if( 1 != sscanf( buf, "%d", &i ) )
    {
        LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 );
        LOG( "parse_i32_from_json(): invalid integer (data)" );
        LOG( buf );
        LOG_END;
        throw bad_cast();
    }
    return i;
}

static unsigned int
parse_u32_from_json(const char *data, uint32_t length) throw()
{
    unsigned int u;
    char *buf = (char *)alloca( sizeof(char) * (length+1) );
    memcpy(buf, data, length);
    buf[length] = 0;
    if( 1 != sscanf( buf, "%u", &u ) )
    {
        LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 );
        LOG( "parse_u32_from_json(): invalid unsigned integer (data)" );
        LOG( buf );
        LOG_END;
        throw bad_cast();
    }

    return u;
}

static uint64_t
parse_u64_from_json(const char *data, uint32_t length) throw()
{
#define U64T_SCANF_FMT (sizeof(uint64_t) == sizeof(unsigned long) ? "%lu" : "%llu" )
    uint64_t u64;
    char *buf = (char *)alloca( sizeof(char) * (length+1) );
    memcpy(buf, data, length);
    buf[length] = 0;
    if( 1 != sscanf( buf, U64T_SCANF_FMT, &u64 ) )
    {
        LOG_BEGIN(loggerModuleName,  ERROR_LOG | 1 );
        LOG( "parse_u64_from_json(): invalid uint64 (data)" );
        LOG( buf );
        LOG_END;
        throw bad_cast();
    }

    return u64;
#undef U64T_SCANF_FMT
}

int
ParseExternalJson::parse_item(int type, const char *data, uint32_t length)
{
    switch(mParserState)
    {
    case pejInitial:
        switch (type)
        {
        case JSON_ARRAY_BEGIN:
            mParserState = pejListEntered;
            mData.clear();
            break;

        default:
            LOG_PARSE_EXT_JSON_ERROR(type);
            return JSON_ERROR_CALLBACK;
        }
        break;

    case pejListEntered:
        switch (type)
        {
        case JSON_ARRAY_BEGIN:
            mParserState = pejTupleEntered;
            mData.resize( mData.size() + 1 );
            break;

        case JSON_ARRAY_END:
            mParserState = pejInitial;
            break;

        default:
            LOG_PARSE_EXT_JSON_ERROR(type);
            return JSON_ERROR_CALLBACK;
        }
        break;

    case pejTupleEntered:
        switch (type)
        {
        case JSON_STRING:
            { // g++ throws "crosses initialization of ‘std::string s’" otherwise
                string s(data, length);
                mData.back().Oid = s.c_str(); // XXX missing Oid::Oid(const char *s, size_t len, bool is_dotted = true );
            }
            mParserState = pejOidParsed;
            break;

        default:
            LOG_PARSE_EXT_JSON_ERROR(type);
            return JSON_ERROR_CALLBACK;
        }
        break;

    case pejOidParsed:
        switch (type)
        {
        case JSON_INT:
            try
            {
                mSyntax = parse_i32_from_json(data, type);
                mParserState = pejTypeParsed;
            }
            catch( bad_cast )
            {
                return JSON_ERROR_CALLBACK;
            }
            break;

        default:
            LOG_PARSE_EXT_JSON_ERROR(type);
            return JSON_ERROR_CALLBACK;
        }
        break;

    case pejTypeParsed:
        switch( mSyntax )
        {
            case ASN_INTEGER:
                if( JSON_INT == type )
                {
                    try
                    {
                        mData.back().Datum = new SnmpInt32( parse_i32_from_json( data, length ) );
                        mParserState = pejDatumParsed;
                    }
                    catch( bad_cast )
                    {
                        return JSON_ERROR_CALLBACK;
                    }
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            case SMI_OPAQUE:
                if( JSON_STRING == type )
                {
                    mData.back().Datum = new OpaqueStr( (const unsigned char *)data, length );
                    mParserState = pejDatumParsed;
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            case ASN_OCTET_STR:
                if( ( JSON_STRING == type ) || ( JSON_INT == type ) || ( JSON_FLOAT == type ) )
                {
                    mData.back().Datum = new OctetStr( (const unsigned char *)data, length );
                    mParserState = pejDatumParsed;
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            case SMI_IPADDRESS:
                if( JSON_STRING == type )
                {
                    string s( data, length );
                    mData.back().Datum = new IpAddress( s.c_str() );
                    if( mData.back().Datum->valid() )
                        mParserState = pejDatumParsed;
                    else
                        return JSON_ERROR_CALLBACK;
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            case SMI_COUNTER:
                if( JSON_INT == type )
                {
                    try
                    {
                        mData.back().Datum = new Counter32( parse_u32_from_json( data, length ) );
                        mParserState = pejDatumParsed;
                    }
                    catch( bad_cast )
                    {
                        return JSON_ERROR_CALLBACK;
                    }
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            case SMI_GAUGE:
                if( JSON_INT == type )
                {
                    try
                    {
                        mData.back().Datum = new Gauge32( parse_u32_from_json( data, length ) );
                        mParserState = pejDatumParsed;
                    }
                    catch( bad_cast )
                    {
                        return JSON_ERROR_CALLBACK;
                    }
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            case SMI_TIMETICKS:
                if( JSON_INT == type )
                {
                    try
                    {
                        mData.back().Datum = new TimeTicks( parse_u32_from_json( data, length ) );
                        mParserState = pejDatumParsed;
                    }
                    catch( bad_cast )
                    {
                        return JSON_ERROR_CALLBACK;
                    }
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            case SMI_COUNTER64:
                if( JSON_INT == type )
                {
                    try
                    {
                        mData.back().Datum = new Counter64( parse_u64_from_json( data, length ) );
                        mParserState = pejDatumParsed;
                    }
                    catch( bad_cast )
                    {
                        return JSON_ERROR_CALLBACK;
                    }
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            case SMI_UINTEGER:
                if( JSON_INT == type )
                {
                    try
                    {
                        mData.back().Datum = new SnmpUInt32( parse_u32_from_json( data, length ) );
                        mParserState = pejDatumParsed;
                    }
                    catch( bad_cast )
                    {
                        return JSON_ERROR_CALLBACK;
                    }
                }
                else
                {
                    LOG_PARSE_BAD_TYPE_ERROR(type);
                    return JSON_ERROR_CALLBACK;
                }

                break;

            default:
                LOG_PARSE_BAD_TYPE_ERROR(type);
                return JSON_ERROR_CALLBACK;
        }

        mParserState = pejDatumParsed;
        break;

    case pejDatumParsed:
        switch (type)
        {
        case JSON_ARRAY_END:
            mParserState = pejListEntered;
            break;

        default:
            LOG_PARSE_EXT_JSON_ERROR(type);
            return JSON_ERROR_CALLBACK;
        }
        break;
    }

    /*
    switch (type)
    {
        case JSON_OBJECT_BEGIN:
        case JSON_ARRAY_BEGIN:
                fprintf( stderr, "entering %s\n", (type == JSON_ARRAY_BEGIN) ? "array" : "object");
                break;
        case JSON_OBJECT_END:
        case JSON_ARRAY_END:
                fprintf( stderr, "leaving %s\n", (type == JSON_ARRAY_END) ? "array" : "object");
                break;
        case JSON_KEY:
        case JSON_STRING:
        case JSON_INT:
        case JSON_FLOAT:
                fprintf( stderr, "value %*s\n", length, data);
                break;
        case JSON_NULL:
                fprintf( stderr, "constant null\n"); break;
        case JSON_TRUE:
                fprintf( stderr, "constant true\n"); break;
        case JSON_FALSE:
                fprintf( stderr, "constant false\n"); break;
    }
    */

    return 0;
}

int
ParseExternalJson::parser_callback(void *userdata, int type, const char *data, uint32_t length)
{
    ParseExternalJson *pej = (ParseExternalJson *)userdata;
    return pej->parse_item( type, data, length );
}

}
