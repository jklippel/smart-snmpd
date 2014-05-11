/*
 * Copyright 1997-2011 Jens Rehsack
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
#ifndef __SMART_SNMPD_PROPERTY_H_INCLUDED__
#define __SMART_SNMPD_PROPERTY_H_INCLUDED__

/* ChangeLog for h/basic/Properties.h, include/smart-snmpd/property.h
 *  No | User | Date     | Description (multilined)
 *   0   FRIE   97/07/20   Initial design
 *       REHS
 *   1   FRIE   ??/??/??   Design enhancements
 *       REHS              XStream-enhancement
 *   2   ????   98/11/25   Last modification of this file in X-Files
 *   3   REHS   99/07/12   Changed some X-Files settings to -Settings
 *   4   REHS   10/10/19   Import into smart-snmpd
 *   5   REHS   11/06/15   Split into ReadProperty and WriteProperty
 */
/*
 * $Id: Properties.h,v 1.3 2000/06/29 09:41:25 rehsack Exp $
 * $Log: Properties.h,v $
 * Revision 1.3  2000/06/29 09:41:25  rehsack
 * Some Include checks added, Double opened namespace fixed,
 * Watcom IDE Project fixed
 *
 * Revision 1.2  2000/06/29 08:51:54  yeti
 * LiWingObjects namespace added, some defines and functions redefined
 * and some old bugs removed.
 *
 */

#include <stdexcept>

using namespace std;

/* Property.h (c) 1997 by Software-Schmiede GbR Halle
 * written 20.7.1997 by Mike Friedrich and Jens Rehsack
 */

namespace SmartSnmpd
{
    /**
     * allows defining of attributes without write access and optional
     * restricted read access
     */
    template <class Caller, class T>
    class ReadProperty
    {
    protected:
        /**
         * type of getter method in the caller class
         *
         * Wrapped getter methods within the owning object can be used for
         * different kinds of access control:
         *   - restrict/filter access (eg. not the entire content of the real value is provided)
         *   - redirect access (eg. return values from another attribute like mList.size())
         */
        typedef T (Caller::*pGetFunction) () const;

        /**
         * reference to the real attribute for unrestricted read access
         */
        T const &mReadValue;
        /**
         * instance of owning class (a property shall always be a class
         * attribute)
         */
        Caller const &mOwner;
        /**
         * get method pointer for restricted, redirected or filtered read access
         */
        const pGetFunction pGetFunc;

    public:
        /**
         * constructor for unrestricted read access
         *
         * @param aReadValue - reference to the attribute for direct read access
         */
        inline ReadProperty( T &aReadValue )
            : mReadValue( aReadValue )
            , mOwner( *((Caller const *)0) )
            , pGetFunc( 0 )
        {}

        /**
         * constructor for restricted read access
         *
         * @param anOwner - reference to the owning object to call getter method
         * @param apGetFunc - getter method pointer (if it's a NULL pointer, an
         *                    invalid_argument exception is thrown)
         */
        inline ReadProperty( Caller const &anOwner, const pGetFunction apGetFunc )
            : mReadValue( *((T const *)0) )
            , mOwner( anOwner )
            , pGetFunc( apGetFunc )
        {
            if( apGetFunc == 0 )
                throw invalid_argument( "require access method to read value" );
        }

        /**
         * destructor
         */
        virtual ~ReadProperty() {}

        /**
         * read access via cast operator
         *
         * This cast operator provides read access on the type of the
         * encapsulated attribute.
         */
        inline operator T() const
        {
            if( pGetFunc )
                return (mOwner.*pGetFunc)();
            else
                return mReadValue;
        }

    private:
        //! forbidden default constructor
        inline ReadProperty();
    };

    /**
     * allows defining of attributes without read access and optional
     * restricted write access
     */
    template <class Caller, class T>
    class WriteProperty
    {
    protected:
        /**
         * type of setter method in the caller class
         *
         * Wrapped setter methods within the owning object can be used for
         * different kinds of access control:
         *   - restrict/filter access (eg. adapt set value to fit in a range)
         *   - redirect access (eg. set values in another attribute like mList.reserve())
         *   - act upon set a value
         */
        typedef void (Caller::*pSetFunction) ( T const & );

        /**
         * reference to the real attribute for unrestricted write access
         */
        T &mWriteValue;
        /**
         * instance of owning class (a property shall always be a class
         * attribute)
         */
        Caller &mOwner;
        /**
         * set method pointer for restricted, redirected or filtered write access
         */
        const pSetFunction pSetFunc;

    public:
        /**
         * constructor for unrestricted write access
         *
         * @param aSetValue - reference to the attribute for direct write access
         */
        inline WriteProperty( T &aSetValue )
            : mWriteValue( aSetValue )
            , mOwner( *((Caller *)0) )
            , pSetFunc( 0 )
        {}

        /**
         * constructor for restricted write access
         *
         * @param anOwner - reference to the owning object to call getter method
         * @param apSetFunc - setter method pointer (if it's a NULL pointer, an
         *                    invalid_argument exception is thrown)
         */
        inline WriteProperty( Caller &anOwner, const pSetFunction apSetFunc )
            : mWriteValue( *((T *)0) )
            , mOwner( anOwner )
            , pSetFunc( apSetFunc )
        {
            if( apSetFunc == 0 )
                throw invalid_argument( "require access method to set value" );
        }


        /**
         * destructor
         */
        virtual ~WriteProperty() {}

        /**
         * assignment operator from a value
         *
         * This operator allows the property value being set via normal
         * attribut modifying syntax.
         */
        inline WriteProperty<Caller, T> & operator = ( T const & aValue )
        {
            if( pSetFunc )
                (mOwner.*pSetFunc)( aValue );
            else
                mWriteValue = aValue;

            return *this;
        }

    private:
        //! forbidden default constructor
        inline WriteProperty();
    };

    /**
     * This class gives you the chance to declare get- and set functions
     * for typed variables.
     *
     * The idea is modeling the programming feeling like in Borland Delphi,
     * or Perl5 (using tie). The one and (in my opinion only) handicap is
     * that member pointers are used and that's why only get and set
     * methods of the controlling class can be defined.
     *
     * This class defines two operators to access the embedded value: an
     * assign operator and a convert operator.
     */
    template <class Caller, class T>
    class Property
        : public ReadProperty<Caller, T>, public WriteProperty<Caller, T>
    {
    protected:
        typedef typename ReadProperty<Caller, T>::pGetFunction pGetFunction;
        typedef typename WriteProperty<Caller, T>::pSetFunction pSetFunction;

    public:
        /**
         * Constructor to allow unfiltered read access and optional filtered write access
         *
         * @param anOwner - reference to the class instance to call the setting member function
         * @param aGetValue - reference to the (class) attribute containing the value to read from
         * @param apSetFunc - pointer to a member function of caller to set new value
         */
        inline Property( Caller &anOwner, T &aGetValue, const pSetFunction apSetFunc )
            : ReadProperty<Caller, T>( aGetValue )
            , WriteProperty<Caller, T>( anOwner, apSetFunc )
        {}

        /**
         * Constructor to allow filtered read and write access
         *
         * At least one kind of access method (read or write access) must
         * be provided, otherwise an invalid_argument exception is thrown.
         *
         * @param anOwner - reference to the class instance to call the setting member function
         * @param apGetFunc - pointer to a member function of caller to get current value (NULL to disable setting)
         * @param apSetFunc - pointer to a member function of caller to set new value (NULL to disable setting)
         */
        inline Property( Caller &anOwner, const pGetFunction apGetFunc, const pSetFunction apSetFunc = (pSetFunction)NULL )
            : ReadProperty<Caller, T>( anOwner, apGetFunc )
            , WriteProperty<Caller, T>( anOwner, apSetFunc )
        {
            if( apGetFunc == NULL && apSetFunc == NULL )
                throw invalid_argument( "At least one accessor must be provided" );
        }

        /**
         * Constructor to allow filtered read and unfiltered write access
         *
         * @param anOwner - reference to the class instance to call the setting member function
         * @param apGetFunc - pointer to a member function of caller to get current value (NULL to disable setting)
         * @param aSetValue - reference to the (class) attribute containing the value to write into
         */
        inline Property( Caller &anOwner, const pGetFunction apGetFunc, T &aSetValue )
            : ReadProperty<Caller, T>( anOwner, apGetFunc )
            , WriteProperty<Caller, T>( aSetValue )
        {}

        /**
         * destructor
         */
        virtual ~Property() {}

    private:
        //! forbidden default constructor
        inline Property();
    };

    template <class Caller, class T>
    inline std::istream &
    operator >> ( std::ostream &is, WriteProperty<Caller,T> &P )
    {
        T value;
        is >> value;
        P = value;
        return is;
    }

    template <class Caller, class T>
    inline std::ostream &
    operator << ( std::ostream &os, const ReadProperty<Caller,T> &P )
    {
        T Value = P;
        os << Value;
        return os;
    }

} //namespace SmartSnmp

#endif //__SMART_SNMPD_PROPERTY_H_INCLUDED__
