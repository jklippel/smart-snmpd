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
#ifndef __SMART_SNMPD_SYSBUF_H_INCLUDED__
#define __SMART_SNMPD_SYSBUF_H_INCLUDED__

#include <new>

#define FALLBACK_BUF_SIZE 16384

namespace SmartSnmpd
{
    /**
     * buffer class for calling syscalls with undefined space requirements
     * @todo add bounds checker for heap overflow detection
     */
    template< class Ch >
    class SystemBuffer
    {
    public:
        /**
         * default constructor
         * @param buf_size - optional size of buffer to reserve, default 16kb
         */
        SystemBuffer( size_t buf_size = 0 )
            : mBufSize( buf_size ? buf_size : FALLBACK_BUF_SIZE )
            , mBuffer( new Ch[mBufSize] )
        {}

        //! destructor
        virtual ~SystemBuffer() { delete [] mBuffer; }

        //! get buffer size in bytes
        size_t getBufSize() const { return mBufSize * sizeof(Ch); }
        //! get pointer to buffer
        Ch * getBuffer() { return mBuffer; }

    protected:
        //! number of reserved buffer items
        const size_t mBufSize;
        //! pointer to reserved buffer
        Ch * mBuffer;

    private:
        //! forbidden copy constructor
        SystemBuffer( SystemBuffer const & r );
        //! forbidden assignment operator
        SystemBuffer & operator = ( SystemBuffer const & r );
    };
}

#endif /* ?__SMART_SNMPD_SYSBUF_H_INCLUDED__ */
