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
#ifndef __SMART_SNMPD_RESOURCE_LIMITS_H_INCLUDED__
#define __SMART_SNMPD_RESOURCE_LIMITS_H_INCLUDED__

#include <smart-snmpd/datasource.h>
#include <agent_pp/snmp_pp_ext.h>

#include <cassert>
#include <set>
#include <stdexcept>

using namespace std;

namespace SmartSnmpd
{
    //! flag combination for unset resource values
    static const int ResourceUnset =    0;
    //! flag for fixed resource values
    static const int ResourceFixValue = 1 << 0;
    //! flag for soft limit resource values
    static const int ResourceSoft =     1 << 1;
    //! flag for hard limit resource values
    static const int ResourceHard =     1 << 2;
    //! flag combination for fixed resource values for the soft limit
    static const int ResourceFixSoft =  ResourceFixValue | ResourceSoft;
    //! flag combination for fixed resource values for the hard limit
    static const int ResourceFixHard =  ResourceFixValue | ResourceHard;

    class ResourceLimits;

    /**
     * class to wrap a specific resource limit
     */
    class ResourceLimit
    {
        friend class ResourceLimits;
    public:
        /**
         * default constructor
         */
        inline ResourceLimit()
            : mResource(0)
            , mFlags(ResourceUnset) // unset
            , mValue(0)
        {}

        /**
         * full constructor
         *
         * Constructing a resource limit with flags like ResourceSoft without
         * ResourceFixValue leads this constructor to fetch the value from
         * the process start. If both flags are specified, ResourceSoft
         * overrides ResourceHard.
         *
         * @param resource - resource identifier, eg. RLIMIT_CORE
         * @param flags - flags for the resource, eg. ResourceFixSoft
         * @param value - value for the resource, eg. RLIM_INFINITY or 512
         */
        inline ResourceLimit(int resource, int flags = ResourceUnset, rlim_t value = 0 )
            : mResource(resource)
            , mFlags(flags)
            , mValue(value)
        {
            if( mFlags && !fix() )
            {
                fetch();
            }
        }

        /**
         * copy constructor
         *
         * @param r - ResourceLimit instance to copy from
         */
        inline ResourceLimit(ResourceLimit const &r)
            : mResource(r.mResource)
            , mFlags(r.mFlags)
            , mValue(r.mValue)
        {}

        /**
         * check whether this resource value is empty or not
         *
         * @return true when this limit is unset in every manner
         */
        bool empty() const { return !filled(); }
        /**
         * check whether this resource value is set with any kind of (indirect) value or not
         *
         * @return true when this limit has any kind of value set
         */
        bool filled() const { return 0 != ( mFlags & (ResourceFixValue|ResourceSoft|ResourceHard) ); }

        /**
         * check whether this resource describes a soft limit
         *
         * @return true when this limit is a soft limit
         */
        bool soft() const { return mFlags & ResourceSoft; }
        /**
         * check whether this resource describes a hard limit
         *
         * @return true when this limit is a hard limit
         */
        bool hard() const { return mFlags & ResourceHard; }
        /**
         * check whether this resource has a fixed value (read: not derived from anywhere)
         *
         * @return true when this resource has a fixed value to be set
         */
        bool fix() const { return mFlags & ResourceFixValue; }

        /**
         * modify resource to let it describe a soft limit
         *
         * This method changes the ResourceSoft flag of this instance. When it's
         * turned on, the process start soft limit is loaded into the hold value.
         * When both flags, ResourceSoft and ResourceHard, are set on the same
         * instance, the latter set one affects the content of the hold resource
         * value.
         *
         * @param flag - true when it shall describe a soft limit, false otherwise
         *
         * @return this resource limit instance
         */
        inline ResourceLimit & set_soft(bool flag)
        {
            if( flag )
            {
                mFlags |= ResourceSoft;
                if( !fix() )
                    fetch();
            }
            else
            {
                mFlags &= ~ResourceSoft;
            }

            return *this;
        }

        /**
         * modify resource to let it describe a hard limit
         *
         * This method changes the ResourceHard flag of this instance. When it's
         * turned on, the process start hard limit is loaded into the hold value.
         * When both flags, ResourceSoft and ResourceHard, are set on the same
         * instance, the latter set one affects the content of the hold resource
         * value.
         *
         * @param flag - true when it shall describe a hard limit, false otherwise
         *
         * @return this resource limit instance
         */
        inline ResourceLimit & set_hard(bool flag)
        {
            if( flag )
            {
                mFlags |= ResourceHard;
                if( !fix() )
                    fetch();
            }
            else
            {
                mFlags &= ~ResourceHard;
            }

            return *this;
        }

        /**
         * @return the resource id of this ResourceLimit instance (eg. RLIMIT_NOFILE)
         */
        inline int resource_id() const { return mResource; }

        /**
         * assignment operator for fixed limit value
         *
         * @return this instance
         */
        inline ResourceLimit & operator = (rlim_t value) { mFlags |= ResourceFixValue; mValue = value; return *this; }

        /**
         * cast operator to the limit value type
         *
         * @return the value contained in this ResourceLimit instance
         */
        inline operator rlim_t () const { return mValue; }

        /**
         * fetches the currently set value from the OS
         *
         * @return this instance
         */
        ResourceLimit & fetch();
        /**
         * commit the values from this instance into the kernel
         *
         * @return this instance
         */
        ResourceLimit const & commit() const; // doesn't modify class but kernel settings for this process *gg*

    protected:
        //! resource identifier (eg. RLIMIT_CORE)
        int mResource;
        //! flags for the resource value (eg. ResourceFixSoft)
        unsigned mFlags;
        //! hold resource value
        rlim_t mValue;

        /**
         * assignment operator for flags and limit value
         * 
         * This assignment operator assignes the flags and the resource value
         * of the given instance to this instance. It does not copy the
         * resource id but asserts their equality!
         *
         * @param ref - reference instance to copy from
         *
         * @return this instance
         */
        inline ResourceLimit & operator = (ResourceLimit const &ref)
        {
            /* mResource = ref.mResource; */
            assert( ref.mResource == mResource );
            mFlags = ref.mFlags;
            mValue = ref.mValue; return *this;
        }
    };

    //! @return true when both ResourceLimit instances having the same resource identifier, false otherwise
    inline bool operator == (ResourceLimit const &x, ResourceLimit const &y) { return x.resource_id() == y.resource_id(); }
    //! @return true when both ResourceLimit instances having different resource identifier, false otherwise
    inline bool operator != (ResourceLimit const &x, ResourceLimit const &y) { return x.resource_id() != y.resource_id(); }
    //! @return true when ResourceLimit instance x has lower resource id than ResourceLimit instance y
    inline bool operator <  (ResourceLimit const &x, ResourceLimit const &y) { return x.resource_id() <  y.resource_id(); }
    //! @return true when ResourceLimit instance x has lower or equal resource id compared with ResourceLimit instance y
    inline bool operator <= (ResourceLimit const &x, ResourceLimit const &y) { return x.resource_id() <= y.resource_id(); }
    //! @return true when ResourceLimit instance x has greater resource id than ResourceLimit instance y
    inline bool operator >  (ResourceLimit const &x, ResourceLimit const &y) { return x.resource_id() >  y.resource_id(); }
    //! @return true when ResourceLimit instance x has greater or equal resource id compared with ResourceLimit instance y
    inline bool operator >= (ResourceLimit const &x, ResourceLimit const &y) { return x.resource_id() >= y.resource_id(); }

    /**
     * predefined set of resource limits
     *
     * contains POSIX defined resource limits
     *
     * @todo add controlled access to BSD, Linux etc. extensions
     */
    class ResourceLimits
        : public set<ResourceLimit>
    {
    public:
        /**
         * default constructor
         *
         * This constructor creates a set of the resource limits defined
         * by the XPG2 standard without any special settings.
         *
         * @param flags - the flags pass through embedded ResourceLimit object construction, usually ResourceSoft or ResourceHard
         */
        ResourceLimits( int flags = ResourceUnset )
            : set<ResourceLimit>()
        {
            insert( ResourceLimit( RLIMIT_CORE, flags ) );
            insert( ResourceLimit( RLIMIT_CPU, flags ) );
            insert( ResourceLimit( RLIMIT_DATA, flags ) );
            insert( ResourceLimit( RLIMIT_FSIZE, flags ) );
            insert( ResourceLimit( RLIMIT_NOFILE, flags ) );
            insert( ResourceLimit( RLIMIT_STACK, flags ) );
            insert( ResourceLimit( RLIMIT_AS, flags ) );
        }

        /**
         * delivers the hold core limit value
         * @return ResourceLimit instance for the RLIMIT_CORE resource id
         */
        inline ResourceLimit const & get_core_limit() const { return get_limit( RLIMIT_CORE ); }
        /**
         * sets the flags and the new limit for the RLIMIT_CORE resource id
         * @param flags - new flags for the entry (eg. from process reference to fixed value)
         * @param limit - new limit
         * @returns this reference
         */
        inline ResourceLimits & set_core_limit(int flags, rlim_t limit) { return set_limit( RLIMIT_CORE, flags, limit ); }
        /**
         * sets the flags and the new limit for the RLIMIT_CORE resource id
         * @param ref - reference to copy from
         * @returns this reference
         */
        inline ResourceLimits & set_core_limit(ResourceLimit const &ref) { assert( RLIMIT_CORE == ref.resource_id() ); return insert_or_update( ref ); }

        /**
         * delivers the hold cpu limit value
         * @return ResourceLimit instance for the RLIMIT_CPU resource id
         */
        inline ResourceLimit const & get_cpu_limit() const { return get_limit( RLIMIT_CPU ); }
        /**
         * sets the flags and the new limit for the RLIMIT_CPU resource id
         * @param flags - new flags for the entry (eg. from process reference to fixed value)
         * @param limit - new limit
         * @returns this reference
         */
        inline ResourceLimits & set_cpu_limit(int flags, rlim_t limit) { return set_limit( RLIMIT_CPU, flags, limit ); }
        /**
         * sets the flags and the new limit for the RLIMIT_CPU resource id
         * @param ref - reference to copy from
         * @returns this reference
         */
        inline ResourceLimits & set_cpu_limit(ResourceLimit const &ref) { assert( RLIMIT_CPU == ref.resource_id() ); return insert_or_update( ref ); }

        /**
         * delivers the hold data limit value
         * @return ResourceLimit instance for the RLIMIT_DATA resource id
         */
        inline ResourceLimit const & get_data_limit() const { return get_limit( RLIMIT_DATA ); }
        /**
         * sets the flags and the new limit for the RLIMIT_DATA resource id
         * @param flags - new flags for the entry (eg. from process reference to fixed value)
         * @param limit - new limit
         * @returns this reference
         */
        inline ResourceLimits & set_data_limit(int flags, rlim_t limit) { return set_limit( RLIMIT_DATA, flags, limit ); }
        /**
         * sets the flags and the new limit for the RLIMIT_DATA resource id
         * @param ref - reference to copy from
         * @returns this reference
         */
        inline ResourceLimits & set_data_limit(ResourceLimit const &ref) { assert( RLIMIT_DATA == ref.resource_id() ); return insert_or_update( ref ); }

        /**
         * delivers the hold file size limit value
         * @return ResourceLimit instance for the RLIMIT_FSIZE resource id
         */
        inline ResourceLimit const & get_fsize_limit() const { return get_limit( RLIMIT_FSIZE ); }
        /**
         * sets the flags and the new limit for the RLIMIT_FSIZE resource id
         * @param flags - new flags for the entry (eg. from process reference to fixed value)
         * @param limit - new limit
         * @returns this reference
         */
        inline ResourceLimits & set_fsize_limit(int flags, rlim_t limit) { return set_limit( RLIMIT_FSIZE, flags, limit ); }
        /**
         * sets the flags and the new limit for the RLIMIT_FSIZE resource id
         * @param ref - reference to copy from
         * @returns this reference
         */
        inline ResourceLimits & set_fsize_limit(ResourceLimit const &ref) { assert( RLIMIT_FSIZE == ref.resource_id() ); return insert_or_update( ref ); }

        /**
         * delivers the hold open file handles limit value
         * @return ResourceLimit instance for the RLIMIT_NOFILE resource id
         */
        inline ResourceLimit const & get_nofile_limit() const { return get_limit( RLIMIT_NOFILE ); }
        /**
         * sets the flags and the new limit for the RLIMIT_NOFILE resource id
         * @param flags - new flags for the entry (eg. from process reference to fixed value)
         * @param limit - new limit
         * @returns this reference
         */
        inline ResourceLimits & set_nofile_limit(int flags, rlim_t limit) { return set_limit( RLIMIT_NOFILE, flags, limit ); }
        /**
         * sets the flags and the new limit for the RLIMIT_NOFILE resource id
         * @param ref - reference to copy from
         * @returns this reference
         */
        inline ResourceLimits & set_nofile_limit(ResourceLimit const &ref) { assert( RLIMIT_NOFILE == ref.resource_id() ); return insert_or_update( ref ); }

        /**
         * delivers the hold stack size limit value
         * @return ResourceLimit instance for the RLIMIT_STACK resource id
         */
        inline ResourceLimit const & get_stack_limit() const { return get_limit( RLIMIT_STACK ); }
        /**
         * sets the flags and the new limit for the RLIMIT_STACK resource id
         * @param flags - new flags for the entry (eg. from process reference to fixed value)
         * @param limit - new limit
         * @returns this reference
         */
        inline ResourceLimits & set_stack_limit(int flags, rlim_t limit) { return set_limit( RLIMIT_STACK, flags, limit ); }
        /**
         * sets the flags and the new limit for the RLIMIT_STACK resource id
         * @param ref - reference to copy from
         * @returns this reference
         */
        inline ResourceLimits & set_stack_limit(ResourceLimit const &ref) { assert( RLIMIT_STACK == ref.resource_id() ); return insert_or_update( ref ); }

        /**
         * delivers the hold memory limit value
         * @return ResourceLimit instance for the RLIMIT_AS resource id
         */
        inline ResourceLimit const & get_as_limit() const { return get_limit( RLIMIT_AS ); }
        /**
         * sets the flags and the new limit for the RLIMIT_AS resource id
         * @param flags - new flags for the entry (eg. from process reference to fixed value)
         * @param limit - new limit
         * @returns this reference
         */
        inline ResourceLimits & set_as_limit(int flags, rlim_t limit) { return set_limit( RLIMIT_AS, flags, limit ); }
        /**
         * sets the flags and the new limit for the RLIMIT_AS resource id
         * @param ref - reference to copy from
         * @returns this reference
         */
        inline ResourceLimits & set_as_limit(ResourceLimit const &ref) { assert( RLIMIT_AS == ref.resource_id() ); return insert_or_update( ref ); }

    protected:
        /**
         * delivers the resource limit for the given resource id
         * @return ResourceLimit instance for the specified resource id
         */
        inline ResourceLimit const & get_limit( int resource ) const
        {
            set<ResourceLimit>::const_iterator ci = find(ResourceLimit(resource));
            if( ci == end() )
                throw invalid_argument( "Unknown resource value" );
            return *ci;
        }

        /**
         * sets the flags and the new limit for the given resource id
         *
         * This method modifies the controlled object identified by the
         * specified resource id. If there is no such object, a new
         * ResourceLimit instance with the specified resource id is added
         * with the given values.
         *
         * @param resource - the resource id entry to modify
         * @param flags - new flags for the entry (eg. from process reference to fixed value)
         * @param limit - new limit
         * @returns this reference
         */
        inline ResourceLimits & set_limit( int resource, int flags, rlim_t limit )
        {
            set<ResourceLimit>::iterator i = lower_bound(ResourceLimit(resource));
            if( ( i == end() ) || ( key_comp()(resource, *i ) ) )
            {
                i = insert( i, ResourceLimit( resource, flags, limit ) );
            }
            else
            {
                // XXX Why is that necessary?
                const_cast<ResourceLimit &>(*i) = ResourceLimit( resource, flags, limit );
            }
            return *this;
        }

        /**
         * sets the flags and the new limit from the specified reference object
         *
         * This method modifies the controlled object identified by the
         * resource id of the referenced ResourceLimit instance.  If there
         * is no such object, a new ResourceLimit instance created from the
         * given reference is added.
         *
         * @param ref - reference to copy from
         *
         * @returns this reference
         */
        inline ResourceLimits & insert_or_update( ResourceLimit const &ref )
        {
            set<ResourceLimit>::iterator i = lower_bound(ref);
            if( ( i == end() ) || ( key_comp()(ref, *i ) ) )
            {
                i = insert( i, ref );
            }
            else
            {
                const_cast<ResourceLimit &>(*i) = ref; // we're sure that the key isn't modified
            }
            return *this;
        }
    };

#if 0
    struct ResourceLimits
    {
        ResourceLimits();

        rlim_t rlimit_core; // RLIMIT_CORE
        rlim_t rlimit_cpu; // RLIMIT_CPU
        rlim_t rlimit_data; // RLIMIT_DATA
        rlim_t rlimit_fsize; // RLIMIT_FSIZE
        rlim_t rlimit_files; // RLIMIT_NOFILE
        rlim_t rlimit_stack; // RLIMIT_STACK
        rlim_t rlimit_mem; // RLIMIT_AS
#if 0
        rlim_t rlimit_memlock; // RLIMIT_MEMLOCK
        rlim_t rlimit_rss; // RLIMIT_RSS
        rlim_t rlimit_sbsize; // RLIMIT_SBSIZE
        rlim_t rlimit_threads; // RLIMIT_THREADS
        rlim_t rlimit_vmem; // RLIMIT_VMEM
#endif

    protected:
        static int GetCurrentResourceLimits();
    };
#endif
}

#endif //?__SMART_SNMPD_RESOURCE_LIMITS_H_INCLUDED__
