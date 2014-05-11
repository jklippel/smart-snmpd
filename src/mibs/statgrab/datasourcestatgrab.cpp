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

#include <smart-snmpd/mibs/statgrab/datasourcestatgrab.h>
#include <smart-snmpd/config.h>

#include <statgrab.h>

#include <set>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.datasource.statgrab";

bool
DataSourceStatgrab::InitStatgrab()
{
    if( SG_ERROR_NONE != sg_init(1) )
    {
        report_sg_error("InitDataSourceStatgrab", "sg_init() failed");

        return false;
    }

    const StatgrabSettings & sgs = Config::getInstance().getStatgrabSettings();
    if( !sgs.ValidFilesystems.empty() )
    {
        vector<const char *> newValidFs;
        set<string> buildValidFs;

        if( sgs.RemoveFilesystems )
        {
            const char **old_valid_fs = sg_get_valid_filesystems(0);
            if( 0 == old_valid_fs )
            {
                LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
                LOG("InitDataSourceStatgrab(): sg_get_valid_filesystems() failed");
                LOG_END;

                return false;
            }

            while( *old_valid_fs )
            {
                buildValidFs.insert(*old_valid_fs);
                ++old_valid_fs;
            }

            for( vector<string>::const_iterator iter = sgs.ValidFilesystems.begin();
                 iter != sgs.ValidFilesystems.end();
                 ++iter )
            {
                set<string>::iterator jter = buildValidFs.find( *iter );
                if( jter != buildValidFs.end() )
                    buildValidFs.erase(jter);
            }
        }
        else
        {
            for( vector<string>::const_iterator iter = sgs.ValidFilesystems.begin();
                 iter != sgs.ValidFilesystems.end();
                 ++iter )
            {
                buildValidFs.insert(*iter);
            }
        }

        newValidFs.reserve( buildValidFs.size() + 1 );
        for( set<string>::const_iterator iter = buildValidFs.begin();
             iter != buildValidFs.end();
             ++iter )
        {
            newValidFs.push_back( iter->c_str() );
        }
        newValidFs.push_back( 0 );

        if( SG_ERROR_NONE != sg_set_valid_filesystems( &newValidFs[0] ) )
        {
            report_sg_error("InitDataSourceStatgrab", "sg_set_valid_filesystems() failed");

            return false;
        }
    }

    return true;
}

bool
DataSourceStatgrab::ShutdownStatgrab()
{
    if( SG_ERROR_NONE != sg_shutdown() )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 0);
        LOG("ShutdownDataSourceStatgrab(): sg_shutdown() failed");
        LOG(sg_str_error(sg_get_error()));
        LOG(sg_get_error_arg());
        LOG_END;

        return false;
    }

    return true;
}

void
DataSourceStatgrab::report_sg_error(string const &who, string const &what)
{
    sg_error_details err_det;
    char *errmsg = NULL;
    sg_error errc;

    if( SG_ERROR_NONE != ( errc = sg_get_error_details(&err_det) ) )
    {
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
        LOG( string( string("report_sg_error(") + who + ", " + what + "): can't get error details - " + sg_str_error(errc) ).c_str() );
        LOG_END;
        return;
    }

    if( NULL == sg_strperror(&errmsg, &err_det) )
    {
        errc = sg_get_error();
        LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
        LOG( string( string("report_sg_error(") + who + ", " + what + "): can't prepare error message - " + sg_str_error(errc) ).c_str() );
        LOG_END;
        return;
    }

    LOG_BEGIN( loggerModuleName, ERROR_LOG | 1 );
    if( errmsg )
    {
        LOG( string( who + ": " + what + " - " + errmsg ).c_str() );
    }
    else
    {
        LOG( string( who + ": " + what + " - " + sg_str_error( sg_get_error() ) + " - unknown details" ).c_str() );
    }
    LOG_END;

    free( errmsg );

    return;
}

}
