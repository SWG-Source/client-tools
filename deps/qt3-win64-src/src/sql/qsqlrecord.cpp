/****************************************************************************
**
** Implementation of QSqlRecord class
**
** Created : 2000-11-03
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsqlrecord.h"

#ifndef QT_NO_SQL

#include "qregexp.h"
#include "qvaluevector.h"
#include "qshared.h"
#include "qnamespace.h"

class QSqlRecordPrivate
{
public:
    class info {
    public:
	info() : nogen(FALSE){}
	~info() {}
	info( const info& other )
	    : field( other.field ), nogen( other.nogen )
	{
	}
	info& operator=(const info& other)
	{
	    field = other.field;
	    nogen = other.nogen;
	    return *this;
	}
	bool isValid() const
	{
	    return !field.name().isNull();
	}
	Q_DUMMY_COMPARISON_OPERATOR(info)
	QSqlField field;
	bool    nogen;
    };

    QSqlRecordPrivate(): cnt(0) 
    {
    }
    QSqlRecordPrivate( const QSqlRecordPrivate& other )
    {
	*this = other;
    }
    ~QSqlRecordPrivate() {};
    QSqlRecordPrivate& operator=( const QSqlRecordPrivate& other )
    {
	fi = other.fi;
	cnt = other.cnt;
	return *this;
    }
    void append( const QSqlField& field )
    {
	info i;
	i.field = field;
	fi.append( i );
	cnt++;
    }
    void insert( int pos, const QSqlField& field )
    {
	info i;
	i.field = field;
	if ( pos == (int)fi.size() )
	    append( field );
	if ( pos > (int)fi.size() ) {
	    fi.resize( pos + 1 );
	    cnt++;
	}
	fi[ pos ] = i;
    }
    void remove( int i )
    {
	info inf;
	if ( i >= (int)fi.count() )
	    return;
	if ( fi[ i ].isValid() )
	    cnt--;
	fi[ i ] = inf;
	// clean up some memory
	while ( fi.count() && !fi.back().isValid() )
	    fi.pop_back();
    }
    void clear()
    {
	fi.clear();
	cnt = 0;
    }
    bool isEmpty()
    {
	return cnt == 0;
    }
    info* fieldInfo( int i )
    {
	if ( i < (int)fi.count() )
	    return &fi[i];
	return 0;
    }
    uint count() const
    {
	return cnt;
    }
    bool contains( int i ) const
    {
	return i >= 0 && i < (int)fi.count() && fi[ i ].isValid();
    }
private:
    QValueVector< info > fi;
    uint		 cnt;
};

QSqlRecordShared::~QSqlRecordShared()
{
    if ( d )
	delete d;
}

/*!
    \class QSqlRecord qsqlfield.h
    \brief The QSqlRecord class encapsulates a database record, i.e. a
    set of database fields.

    \ingroup database
    \module sql

    The QSqlRecord class encapsulates the functionality and
    characteristics of a database record (usually a table or view within
    the database). QSqlRecords support adding and removing fields as
    well as setting and retrieving field values.

    QSqlRecord is implicitly shared. This means you can make copies of
    the record in time O(1). If multiple QSqlRecord instances share
    the same data and one is modifying the record's data then this
    modifying instance makes a copy and modifies its private copy -
    thus it does not affect other instances.

    \sa QSqlRecordInfo
*/


/*!
    Constructs an empty record.
*/

QSqlRecord::QSqlRecord()
{
    sh = new QSqlRecordShared( new QSqlRecordPrivate() );
}

/*!
    Constructs a copy of \a other.
*/

QSqlRecord::QSqlRecord( const QSqlRecord& other )
    : sh( other.sh )
{
    sh->ref();
}

/*!
    Sets the record equal to \a other.
*/

QSqlRecord& QSqlRecord::operator=( const QSqlRecord& other )
{
    other.sh->ref();
    deref();
    sh = other.sh;
    return *this;
}

/*! \internal
*/

void QSqlRecord::deref()
{
    if ( sh->deref() ) {
	delete sh;
	sh = 0;
    }
}

/*! \internal
*/

bool QSqlRecord::checkDetach()
{
    if ( sh->count > 1 ) {
	sh->deref();
	sh = new QSqlRecordShared( new QSqlRecordPrivate( *sh->d ) );
	return TRUE;
    }
    return FALSE;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlRecord::~QSqlRecord()
{
    deref();
}

/*!
    Returns the value of the field located at position \a i in the
    record. If field \a i does not exist the resultant behaviour is
    undefined.

    This function should be used with \l{QSqlQuery}s. When working
    with a QSqlCursor the \link QSqlCursor::value() value(const
    QString&)\endlink overload which uses field names is more
    appropriate.
*/

QVariant QSqlRecord::value( int i ) const
{
    const QSqlField * f = field(i);

    if( f )
	return f->value();
    return QVariant();
}

/*!
    \overload

    Returns the value of the field called \a name in the record. If
    field \a name does not exist the resultant behaviour is undefined.
*/

QVariant  QSqlRecord::value( const QString& name ) const
{
    const QSqlField * f = field( name );

    if( f )
	return f->value();
    return QVariant();
}

/*!
    Returns the name of the field at position \a i. If the field does
    not exist, QString::null is returned.
*/

QString QSqlRecord::fieldName( int i ) const
{
    const QSqlField* f = field( i );
    if ( f )
	return f->name();
    return QString::null;
}

/*!
    Returns the position of the field called \a name within the
    record, or -1 if it cannot be found. Field names are not
    case-sensitive. If more than one field matches, the first one is
    returned.
*/

int QSqlRecord::position( const QString& name ) const
{
    for ( uint i = 0; i < count(); ++i ) {
	if ( fieldName(i).upper() == name.upper() )
	    return i;
    }
#ifdef QT_CHECK_RANGE
    qWarning( "QSqlRecord::position: unable to find field " + name );
#endif
    return -1;
}

/*!
    Returns the field at position \a i within the record, or 0 if it
    cannot be found.
*/

QSqlField* QSqlRecord::field( int i )
{
    checkDetach();
    if ( !sh->d->contains( i ) ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QSqlRecord::field: index out of range: " + QString::number( i ) );
#endif
	return 0;
    }
    return &sh->d->fieldInfo( i )->field;
}

/*!
    \overload

    Returns the field called \a name within the record, or 0 if it
    cannot be found. Field names are not case-sensitive.
*/

QSqlField* QSqlRecord::field( const QString& name )
{
    checkDetach();
    if ( !sh->d->contains( position( name ) ) )
	return 0;
    return &sh->d->fieldInfo( position( name ) )->field;
}


/*!
    \overload
*/

const QSqlField* QSqlRecord::field( int i ) const
{
    if ( !sh->d->contains( i ) ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QSqlRecord::field: index out of range: " + QString::number( i ) );
#endif // QT_CHECK_RANGE
	return 0;
    }
    return &sh->d->fieldInfo( i )->field;
}

/*!
    \overload

    Returns the field called \a name within the record, or 0 if it
    cannot be found. Field names are not case-sensitive.
*/

const QSqlField* QSqlRecord::field( const QString& name ) const
{
    if( !sh->d->contains( position( name ) ) )
	return 0;
    return &sh->d->fieldInfo( position( name ) )->field;
}

/*!
    Append a copy of field \a field to the end of the record.
*/

void QSqlRecord::append( const QSqlField& field )
{
    checkDetach();
    sh->d->append( field );
}

/*!
    Insert a copy of \a field at position \a pos. If a field already
    exists at \a pos, it is removed.
*/

void QSqlRecord::insert( int pos, const QSqlField& field ) // ### 4.0: rename to ::replace
{
    checkDetach();
    sh->d->insert( pos, field );
}

/*!
    Removes the field at \a pos. If \a pos does not exist, nothing
    happens.
*/

void QSqlRecord::remove( int pos )
{
    checkDetach();
    sh->d->remove( pos );
}

/*!
    Removes all the record's fields.

    \sa clearValues()
*/

void QSqlRecord::clear()
{
    checkDetach();
    sh->d->clear();
}

/*!
    Returns TRUE if there are no fields in the record; otherwise
    returns FALSE.
*/

bool QSqlRecord::isEmpty() const
{
    return sh->d->isEmpty();
}


/*!
    Returns TRUE if there is a field in the record called \a name;
    otherwise returns FALSE.
*/

bool QSqlRecord::contains( const QString& name ) const
{
    for ( uint i = 0; i < count(); ++i ) {
	if ( fieldName(i).upper() == name.upper() )
	    return TRUE;
    }
    return FALSE;
}

/*!
    Clears the value of all fields in the record. If \a nullify is
    TRUE, (the default is FALSE), each field is set to NULL.
*/

void QSqlRecord::clearValues( bool nullify )
{
    checkDetach();
    int cnt = (int)count();
    int i;
    for ( i = 0; i < cnt; ++i ) {
	field( i )->clear( nullify );
    }
}

/*!
    Sets the generated flag for the field called \a name to \a
    generated. If the field does not exist, nothing happens. Only
    fields that have \a generated set to TRUE are included in the SQL
    that is generated, e.g. by QSqlCursor.

    \sa isGenerated()
*/

void QSqlRecord::setGenerated( const QString& name, bool generated )
{
    setGenerated( position( name ), generated );
}

/*!
    \overload

    Sets the generated flag for the field \a i to \a generated.

    \sa isGenerated()
*/

void QSqlRecord::setGenerated( int i, bool generated )
{
    checkDetach();
    if ( !field( i ) )
	return;
    sh->d->fieldInfo( i )->nogen = !generated;
}

/*!
    \internal
    ### Remove in 4.0
*/
bool QSqlRecord::isNull( int i )
{
    checkDetach();
    QSqlField* f = field( i );
    if ( f ) {
	return f->isNull();
    }
    return TRUE;
}

/*! 
    \internal
    ### Remove in 4.0
*/
bool QSqlRecord::isNull( const QString& name )
{
    return isNull( position( name ) );
}

/*!
    \overload

    Returns TRUE if the field \a i is NULL or if there is no field at
    position \a i; otherwise returns FALSE.

    \sa fieldName()
*/
bool QSqlRecord::isNull( int i ) const
{
    const QSqlField* f = field( i );
    if ( f ) {
	return f->isNull();
    }
    return TRUE;
}

/*!
    Returns TRUE if the field called \a name is NULL or if there is no
    field called \a name; otherwise returns FALSE.

    \sa position()
*/
bool QSqlRecord::isNull( const QString& name ) const
{
    return isNull( position( name ) );
}

/*!
    Sets the value of field \a i to NULL. If the field does not exist,
    nothing happens.
*/
void QSqlRecord::setNull( int i )
{
    checkDetach();
    QSqlField* f = field( i );
    if ( f ) {
	f->setNull();
    }
}

/*!
    \overload

    Sets the value of the field called \a name to NULL. If the field
    does not exist, nothing happens.
*/
void QSqlRecord::setNull( const QString& name )
{
    setNull( position( name ) );
}


/*!
    Returns TRUE if the record has a field called \a name and this
    field is to be generated (the default); otherwise returns FALSE.

    \sa setGenerated()
*/
bool QSqlRecord::isGenerated( const QString& name ) const
{
    return isGenerated( position( name ) );
}

/*!
    \overload

    Returns TRUE if the record has a field at position \a i and this
    field is to be generated (the default); otherwise returns FALSE.

    \sa setGenerated()
*/
bool QSqlRecord::isGenerated( int i ) const
{
    if ( !field( i ) )
	return FALSE;
    return !sh->d->fieldInfo( i )->nogen;
}


/*!
    Returns a list of all the record's field names as a string
    separated by \a sep.

    Note that fields which are not generated are \e not included (see
    \l{isGenerated()}). The returned string is suitable, for example, for
    generating SQL SELECT statements. If a \a prefix is specified,
    e.g. a table name, all fields are prefixed in the form:

    "\a{prefix}.\<fieldname\>"
*/

QString QSqlRecord::toString( const QString& prefix, const QString& sep ) const
{
    QString pflist;
    bool comma = FALSE;
    for ( uint i = 0; i < count(); ++i ){
	if ( isGenerated( field(i)->name() ) ) {
	    if( comma )
		pflist += sep + " ";
	    pflist += createField( i, prefix );
	    comma = TRUE;
	}
    }
    return pflist;
}

/*!
    Returns a list of all the record's field names, each having the
    prefix \a prefix.

    Note that fields which have generated set to FALSE are \e not
    included. (See \l{isGenerated()}). If \a prefix is supplied, e.g.
    a table name, all fields are prefixed in the form:

    "\a{prefix}.\<fieldname\>"
*/

QStringList QSqlRecord::toStringList( const QString& prefix ) const
{
    QStringList s;
    for ( uint i = 0; i < count(); ++i ) {
	if ( isGenerated( field(i)->name() ) )
	    s += createField( i, prefix );
    }
    return s;
}

/*! \internal
*/

QString QSqlRecord::createField( int i, const QString& prefix ) const
{
    QString f;
    if ( !prefix.isEmpty() )
	f = prefix + ".";
    f += field( i )->name();
    return f;
}

/*!
    Returns the number of fields in the record.
*/

uint QSqlRecord::count() const
{
    return sh->d->count();
}

/*!
    Sets the value of the field at position \a i to \a val. If the
    field does not exist, nothing happens.
*/

void QSqlRecord::setValue( int i, const QVariant& val )
{
    checkDetach();
    QSqlField* f = field( i );
    if ( f ) {
	f->setValue( val );
    }
}


/*!
    \overload

    Sets the value of the field called \a name to \a val. If the field
    does not exist, nothing happens.
*/

void QSqlRecord::setValue( const QString& name, const QVariant& val )
{
    setValue( position( name ), val );
}


/******************************************/
/*******     QSqlRecordInfo Impl     ******/
/******************************************/

/*!
    \class QSqlRecordInfo qsqlrecord.h
    \brief The QSqlRecordInfo class encapsulates a set of database field meta data.

    \ingroup database
    \module sql

    This class is a QValueList that holds a set of database field meta
    data. Use contains() to see if a given field name exists in the
    record, and use find() to get a QSqlFieldInfo record for a named
    field.

    \sa QValueList, QSqlFieldInfo
*/


/*!
    Constructs a QSqlRecordInfo object based on the fields in the
    QSqlRecord \a other.
*/
QSqlRecordInfo::QSqlRecordInfo( const QSqlRecord& other )
{
    for ( uint i = 0; i < other.count(); ++i ) {
	push_back( QSqlFieldInfo( *(other.field( i )), other.isGenerated( i ) ) );
    }
}

/*!
    Returns the number of times a field called \a fieldName occurs in
    the record. Returns 0 if no field by that name could be found.
*/
QSqlRecordInfo::size_type QSqlRecordInfo::contains( const QString& fieldName ) const
{
   size_type i = 0;
   QString fName = fieldName.upper();
   for( const_iterator it = begin(); it != end(); ++it ) {
	if ( (*it).name().upper() == fName ) {
	    ++i;
	}
    }
    return i;
}

/*!
    Returns a QSqlFieldInfo object for the first field in the record
    which has the field name \a fieldName. If no matching field is
    found then an empty QSqlFieldInfo object is returned.
*/
QSqlFieldInfo QSqlRecordInfo::find( const QString& fieldName ) const
{
   QString fName = fieldName.upper();
   for( const_iterator it = begin(); it != end(); ++it ) {
	if ( (*it).name().upper() == fName ) {
	    return *it;
	}
    }
    return QSqlFieldInfo();
}

/*!
    Returns an empty QSqlRecord based on the field information
    in this QSqlRecordInfo.
*/
QSqlRecord QSqlRecordInfo::toRecord() const
{
   QSqlRecord buf;
   for( const_iterator it = begin(); it != end(); ++it ) {
	buf.append( (*it).toField() );
   }
   return buf;
}

/*!
    \fn QSqlRecordInfo::QSqlRecordInfo()

    Constructs an empty record info object
*/

/*!
    \fn QSqlRecordInfo::QSqlRecordInfo( const QSqlFieldInfoList& other )

    Constructs a copy of \a other.
*/

#endif
