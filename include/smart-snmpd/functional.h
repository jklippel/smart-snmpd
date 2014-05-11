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
#ifndef __SMART_SNMPD_FUNCTIONAL_H_INCLUDED__
#define __SMART_SNMPD_FUNCTIONAL_H_INCLUDED__

#include <functional>
#include <agent_pp/snmp_pp_ext.h>
#include <agent_pp/mib_entry.h>

namespace std
{

#ifndef AGENTPP_USE_STL
//! specialized implementation of STL's less for Agent++ Oidx class
template<>
struct less<Oidx>
{
    //! return true when Oidx instance x is less than y
    inline bool operator () (const Oidx &x, const Oidx &y) const { return x < y; }
};
#endif

//! specialized implementation of STL's less for Agent++ MibEntry class
template<>
struct less<MibEntry>
{
    //! return true when MibEntry instance x is less than y
    inline bool operator () (const MibEntry &x, const MibEntry &y) const { return x < y; }
};

//! specialized implementation of STL's less for pointer to Agent++ MibEntry class
template<>
struct less<MibEntry *>
{
    //! return true when MibEntry instance x is less than y
    inline bool operator () (const MibEntry *x, const MibEntry *y) const
    {
	if( x == y )
	    return false;
	else if( x == 0 && y != 0 )
	    return true;
	else if( x != 0 && y == 0 )
	    return false;
	else
	    return *x < *y;
    }
};

//! specialized implementation of STL's less for pointer to Agent++ MibEntry class
struct mib_entry_less_skip
{
public:
    mib_entry_less_skip( unsigned long skip = 0 )
        : mSkip( skip )
    {}

    //! return true when MibEntry instance x is less than y
    inline bool operator () (const MibEntry *x, const MibEntry *y) const
    {
	if( x == y )
	    return false;
	else if( x == 0 && y != 0 )
	    return true;
	else if( x != 0 && y == 0 )
	    return false;
	else
	    return x->key()->mnCompare( *(y->key()), mSkip, x->key()->len() ) < 0;
    }

protected:
    long mSkip;
};
} // end of namespace std

#endif //__SMART_SNMPD_FUNCTIONAL_H_INCLUDED__

