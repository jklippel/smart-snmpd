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

#include <smart-snmpd/oids.h>
#include <smart-snmpd/datasource.h>
#include <smart-snmpd/mibutils/mibcontainer.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.mibutils.mibcontainer";

const Oidx MibContainerTable::TableEntryKey = SM_TABLE_ENTRY_KEY;

Oidx MibContainer::find_succ(Oidx const &o, Request *)
{
    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 6);
    LOG("MibContainer::find_succ: (oid)(o)");
    LOG(mBaseOid.get_printable());
    LOG(o.get_printable());
    LOG_END;

    Oidx rc;
    set<Vbx>::const_iterator i;
    if( o <= mBaseOid )
    {
        i = mContent.begin();
    }
    else
    {
        if( mBaseOid.is_root_of( o ) )
        {
            Vbx tmpvb( o.cut_left( mBaseOid.len() ) );
            i = mContent.upper_bound( tmpvb );
        }
        else
        {
            i = mContent.upper_bound( Vbx( o ) );
        }
    }

    if( i != mContent.end() )
    {
        rc = mBaseOid + i->get_oid();
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 6);
    LOG("MibContainer::find_succ: (o)(result)");
    LOG(o.get_printable());
    LOG(rc.get_printable());
    LOG_END;

    return rc;
}

void MibContainer::get_request( Request *aReq, int idx )
{
    Oidx tmpoid;

    if( 0 == aReq )
        throw invalid_argument( "NULL pointer for req" );

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 6);
    LOG("MibContainer::get_request: (oid)");
    LOG(aReq->get_oid(idx).get_printable());
    LOG_END;

    if( mBaseOid.is_root_of( aReq->get_oid(idx) ) )
    {
        tmpoid = aReq->get_oid(idx).cut_left( mBaseOid.len() );
    }
    else
    {
        Vbx vb(aReq->get_oid(idx));
        vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
        // error status (v1) will be set by RequestList
        aReq->finish(idx, vb); 
        return;
    }

    set<Vbx>::const_iterator entry = findItem( tmpoid, true );
    if( entry == mContent.end() )
    {
        Vbx vb( aReq->get_oid(idx) );
        // TODO: This error status is just a guess, we cannot
        // determine exactly whether it is a noSuchInstance or
        // noSuchObject. May be a subclass could do a better 
        // job by knowing more details from the MIB structure?
        if( tmpoid.len() == 0 )
        {
            vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
        }
        else
        {
            vb.set_syntax(sNMP_SYNTAX_NOSUCHINSTANCE);
        }
        // error status (v1) will be set by RequestList
        aReq->finish(idx, vb); 
    }
    else
    {
#if 0
        Oidx id(mBaseOid);
        id += entry->get_oid();
        Vbx vb(*entry);
        vb.set_oid(id);
#else
        Vbx vb( Oidx( mBaseOid, entry->Vb::get_oid() ) );
        vb.set_value( *entry );
#endif
        aReq->finish(idx, vb);
    }
}

}
