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
#include <smart-snmpd/mibutils/mibcomposed.h>

namespace SmartSnmpd
{

static const char * const loggerModuleName = "smartsnmpd.mibutils.mibobjectbase";

template< class C >
void
deleteAndClear( C &c )
{
    for( typename C::iterator iter = c.begin(); iter != c.end(); ++iter )
    {
        if( *iter )
        {
            (*iter)->reset();

            delete *iter;
        }
    }

    c.clear();
}

MibComposed::MibComposed(MibComposed &aRef)
    : MibComplexEntry(aRef)
    , mLastUpdate( *this, &SmartSnmpd::MibComposed::readLastUpdate, &SmartSnmpd::MibComposed::writeLastUpdate )
    , mContent( mib_entry_less_skip( oid.len() ) )
    , mSearchHelper()
{
    for( set<MibEntry *>::iterator iter = aRef.mContent.begin(); iter != aRef.mContent.end(); ++iter )
    {
        if( *iter )
        {
	    mContent.insert( (*iter)->clone() );
        }
    }
}

bool
MibComposed::set_value(NS_AGENT Oidx const &anOid, const NS_SNMP SnmpSyntax &aSyntax)
{
    Oidx tmpoid( oid.is_root_of(anOid) ? anOid : Oidx(oid + anOid) );

    ThreadSynchronize guard( *this );
    mSearchHelper = MibEntry( tmpoid, NOACCESS );
    set<MibEntry *>::iterator i = mContent.find( &mSearchHelper );

    if( i == mContent.end() )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
        LOG("MibComposed::set_value: Invalid object at (me)(oid)");
        LOG(oid.get_printable());
        LOG(anOid.get_printable());
        LOG_END;

        return false;
    }

    bool rc = true;
    if( is_leaf_node(*i) )
    {
	((MibLeaf *)(*i))->set_value(aSyntax);
    }
    else
    {
        rc = ((MibComposed *)(*i))->set_value(anOid, aSyntax);
    }

    return rc;
}

bool
MibComposed::set_value(NS_AGENT MibEntry * const anEntry)
{
    bool rc = true;
    MibEntry *lazy_delete = 0;

    {
	ThreadSynchronize guard( *this );
	mSearchHelper = MibEntry( *(anEntry->key()), NOACCESS );
	set<MibEntry *>::iterator i = mContent.find( &mSearchHelper );

	if( i == mContent.end() )
	{
	    LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
	    LOG("MibComposed::set_value: Invalid object at (me)(oid)");
	    LOG(oid.get_printable());
	    LOG(anEntry->key()->get_printable());
	    LOG_END;

	    return false;
	}

	if( is_leaf_node(*i) )
	{
	    rc = ((MibLeaf *)(*i))->set_value(((MibLeaf *)anEntry)->get_value());
	}
	else
	{
	    lazy_delete = *i;
	    const_cast<MibEntry *&>(*i) = anEntry; // we're sure that the key isn't modified
	}
    }

    if( lazy_delete )
	delete lazy_delete;

    return rc;
}

MibEntry *
MibComposed::add(MibEntry *anEntry)
{
    ThreadSynchronize guard( *this );
    MibEntry *oldEntry = insert_or_update( anEntry );

    return oldEntry;
}

MibEntry *
MibComposed::remove(const Oidx &anOid)
{
    MibEntry *rc = 0;
    Oidx tmpoid;

    if( oid.is_root_of(anOid) )
	tmpoid = anOid;
    else
        tmpoid = oid + anOid;

    ThreadSynchronize guard( *this );
    mSearchHelper = MibEntry( tmpoid, NOACCESS );
    set<MibEntry *>::iterator i = mContent.find( &mSearchHelper );

    if( i != mContent.end() )
    {
	rc = *i;
	mContent.erase( i );
    }

    return rc;
}

MibEntry *
MibComposed::get(const Oidx &anOid, bool suffixOnly)
{
    Oidx tmpoid = suffixOnly || oid.is_root_of(anOid) ? Oidx(oid + anOid) : anOid;

    ThreadSynchronize guard( *this );
    mSearchHelper = MibEntry( tmpoid, NOACCESS );
    set<MibEntry *>::iterator i = mContent.find( &mSearchHelper );

    if( i == mContent.end() )
    {
        LOG_BEGIN(loggerModuleName, ERROR_LOG | 1);
        LOG("MibComposed::get(): full qualified OID below (me) required, but got (oid)");
        LOG(oid.get_printable());
        LOG(anOid.get_printable());
        LOG_END;

        return 0;
    }

    return *i;
}

void
MibComposed::clear()
{
    ThreadSynchronize guard( *this );
    deleteAndClear( mContent );
    /*
    for( set<MibEntry *>::iterator iter = mContent.begin(); iter != mContent.end(); ++iter )
    {
        if( *iter )
        {
            (*iter)->reset();

            delete *iter;
        }
    }

    mContent.clear();
    */
}

size_t
MibComposed::assign(set<MibEntry *, mib_entry_less_skip> & newContent)
{
    set<MibEntry *, mib_entry_less_skip> lazy_delete;

    {
	ThreadSynchronize guard( *this );
	if( !mContent.empty() )
	    lazy_delete.swap( mContent );
	mContent.swap( newContent );
    }

    deleteAndClear( lazy_delete );
    /*
    for( set<MibEntry *>::iterator iter = lazy_delete.begin(); iter != lazy_delete.end(); ++iter )
    {
        if( *iter )
        {
            (*iter)->reset();

            delete *iter;
            *iter = 0;
        }
    }

    lazy_delete.clear();
    */

    return mContent.size();
}

Oidx
MibComposed::find_succ(const Oidx &anOid, Request *aReq)
{
    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 6);
    LOG("MibComposed::find_succ: (oid)");
    LOG(anOid.get_printable());
    LOG_END;

    update(aReq);

    Oidx retval;

    ThreadSynchronize guard( *this );
    set<MibEntry *>::iterator i;
    if( anOid > oid )
    {
	mSearchHelper = MibEntry( anOid, NOACCESS );
	i = mContent.upper_bound( &mSearchHelper );

	set<MibEntry *>::iterator j = i;
	--j;
	MibEntry *baseEntry = *j;

	if( baseEntry->key()->is_root_of( anOid ) )
	{
	    --i;
	}
    }
    else
	i = mContent.begin();

    while( i != mContent.end() )
    {
	LOG_BEGIN(loggerModuleName, DEBUG_LOG | 6);
	LOG("MibComposed::find_succ: (oid) - checking (entry)");
	LOG(anOid.get_printable());
	LOG(((*i)->key())->get_printable());
	LOG_END;

	if( !is_leaf_node( *i ) )
	{
	    retval = (*i)->find_succ( anOid, aReq );
	    if( retval.len() != 0 )
		break;
	}
	else if( oid.is_root_of( *((*i)->key()) ) )
	{
	    retval = *((*i)->key());
	    break;
	}
	else
	{
	    retval = oid;
	    retval += *((*i)->key());
	    break;
	}

	++i;
    }

    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 6);
    LOG("MibComposed::find_succ: (oid)(result)");
    LOG(anOid.get_printable());
    LOG(retval.get_printable());
    LOG_END;

    return retval;
}

void
MibComposed::get_request(Request *aReq, int ind)
{
    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 6);
    LOG("MibComposed::get_request: (oid)");
    LOG(aReq->get_oid(ind).get_printable());
    LOG_END;

    if ( aReq->get_oid(ind) >= oid && !oid.is_root_of(aReq->get_oid(ind)))
    {
        Vbx vb(aReq->get_oid(ind));
        vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
        // error status (v1) will be set by RequestList
        aReq->finish(ind, vb); 
    }

    update(aReq);

    ThreadSynchronize guard( *this );
    mSearchHelper = MibEntry( aReq->get_oid(ind), NOACCESS );
    set<MibEntry *>::iterator i = mContent.lower_bound( &mSearchHelper );

    if( i != mContent.begin() )
    {
	set<MibEntry *>::iterator j = i;
	--j;
	MibEntry *baseEntry = *j;

	if( baseEntry->key()->is_root_of( aReq->get_oid(ind) ) )
	{
	    --i;
	}
    }

    if( i == mContent.end() )
    {
	Vbx vb(aReq->get_oid(ind));
	vb.set_syntax( sNMP_SYNTAX_NOSUCHOBJECT );
	// error status (v1) will be set by RequestList
	aReq->finish(ind, vb); 
    }
    else
    {
	(*i)->get_next_request(aReq, ind);
    }
}

void
MibComposed::get_next_request(Request *aReq, int ind)
{
    LOG_BEGIN(loggerModuleName, DEBUG_LOG | 6);
    LOG("MibComposed::get_next_request: (oid)");
    LOG(aReq->get_oid(ind).get_printable());
    LOG_END;

    if ( aReq->get_oid(ind) >= oid && !oid.is_root_of(aReq->get_oid(ind)))
    {
        Vbx vb(aReq->get_oid(ind));
        vb.set_syntax(sNMP_SYNTAX_NOSUCHOBJECT);
        // error status (v1) will be set by RequestList
        aReq->finish(ind, vb); 
    }

    ThreadSynchronize guard( *this );
    mSearchHelper = MibEntry( aReq->get_oid(ind), NOACCESS );
    set<MibEntry *>::iterator i = mContent.lower_bound( &mSearchHelper );

    if( i != mContent.begin() )
    {
	set<MibEntry *>::iterator j = i;
	--j;
	MibEntry *baseEntry = *j;

	if( baseEntry->key()->is_root_of( aReq->get_oid(ind) ) )
	{
	    --i;
	}
    }

    if( i == mContent.end() )
    {
	Vbx vb(aReq->get_oid(ind));
	vb.set_syntax( sNMP_SYNTAX_NOSUCHOBJECT );
	// error status (v1) will be set by RequestList
	aReq->finish(ind, vb); 
    }
    else
    {
	(*i)->get_next_request(aReq, ind);
    }
}

time_t
MibObject::readLastUpdate() const
{
    MibObject &self = const_cast<MibObject &>(*this);
    ThreadSynchronize guard( self );
    /*
     * internal agreement: last update is always on first mib below controlling object
     */
    self.mSearchHelper = MibEntry( Oidx("1") , NOACCESS );
    set<MibEntry *>::iterator i = self.mContent.find( &self.mSearchHelper );

    if( ( i != self.mContent.end() ) && is_leaf_node(*i) )
    {
        Counter64 cntr64;
        time_t val;
        ((MibLeaf const *)(*i))->get_value(cntr64);
        val = cntr64;
        return val;
    }

    return 0;
}

void
MibObject::writeLastUpdate(time_t const &aLastUpdate)
{
    ThreadSynchronize guard( *this );
    /*
     * internal agreement: last update is always on first mib below controlling object
     */
    mSearchHelper = MibEntry( Oidx("1") , NOACCESS );
    set<MibEntry *>::iterator i = mContent.find( &mSearchHelper );

    if( ( i != mContent.end() ) && is_leaf_node(*i) )
    {
        ((MibLeaf *)(*i))->set_value( Counter64(aLastUpdate) );
    }
}

}
