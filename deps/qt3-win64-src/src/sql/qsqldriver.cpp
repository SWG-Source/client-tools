/****************************************************************************
**
** Implementation of QSqlDriver class
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

#include "qsqldriver.h"

#ifndef QT_NO_SQL

#include "qdatetime.h"
#include "qsqlextension_p.h"

// database states
#define DBState_Open		0x0001
#define DBState_OpenError	0x0002

// ### This needs to go in 4.0!
QPtrDict<QSqlDriverExtension> *qSqlDriverExtDict();
QPtrDict<QSqlOpenExtension> *qSqlOpenExtDict();

/*!
    \class QSqlDriver qsqldriver.h
    \brief The QSqlDriver class is an abstract base class for accessing
    SQL databases.

    \ingroup database
    \module sql

    This class should not be used directly. Use QSqlDatabase instead.
*/

/*!
    Default constructor. Creates a new driver with parent \a parent,
    called \a name.

*/

QSqlDriver::QSqlDriver( QObject * parent, const char * name )
: QObject(parent, name),
  dbState(0),
  error()
{
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlDriver::~QSqlDriver()
{
}

/*!
    \fn bool QSqlDriver::open( const QString& db, const QString& user,
    const QString& password, const QString& host, int port )

    Derived classes must reimplement this abstract virtual function in
    order to open a database connection on database \a db, using user
    name \a user, password \a password, host \a host and port \a port.

    The function \e must return TRUE on success and FALSE on failure.

    \sa setOpen()

*/

/*!
    \fn bool QSqlDriver::close()

    Derived classes must reimplement this abstract virtual function in
    order to close the database connection. Return TRUE on success,
    FALSE on failure.

    \sa setOpen()

*/

/*!
    \fn QSqlQuery QSqlDriver::createQuery() const

    Creates an empty SQL result on the database. Derived classes must
    reimplement this function and return a QSqlQuery object
    appropriate for their database to the caller.

*/

//void QSqlDriver::destroyResult( QSqlResult* r ) const
//{
//    if ( r )
//	delete r;
//}

/*!
    Returns TRUE if the database connection is open; otherwise returns
    FALSE.
*/

bool QSqlDriver::isOpen() const
{
    if ( !qSqlDriverExtDict()->isEmpty() ) {
	QSqlDriverExtension *ext = qSqlDriverExtDict()->find( (QSqlDriver *) this );
	if ( ext )
	    return ext->isOpen();
    }

    return ((dbState & DBState_Open) == DBState_Open);
}

/*!
    Returns TRUE if the there was an error opening the database
    connection; otherwise returns FALSE.
*/

bool QSqlDriver::isOpenError() const
{
    return ((dbState & DBState_OpenError) == DBState_OpenError);
}

/*!
    \enum QSqlDriver::DriverFeature

    This enum contains a list of features a driver may support. Use
    hasFeature() to query whether a feature is supported or not.

    \value Transactions  whether the driver supports SQL transactions
    \value QuerySize  whether the database is capable of reporting the size
    of a query. Note that some databases do not support returning the size
    (i.e. number of rows returned) of a query, in which case
    QSqlQuery::size() will return -1
    \value BLOB  whether the driver supports Binary Large Object fields
    \value Unicode  whether the driver supports Unicode strings if the
    database server does
    \value PreparedQueries  whether the driver supports prepared query execution
    \value NamedPlaceholders  whether the driver supports usage of named placeholders
    \value PositionalPlaceholders  whether the driver supports usage of positional placeholders

    More information about supported features can be found in the
    \link sql-driver.html Qt SQL driver\endlink documentation.

    \sa hasFeature()
*/

/*!
    \fn bool QSqlDriver::hasFeature( DriverFeature f ) const

    Returns TRUE if the driver supports feature \a f; otherwise
    returns FALSE.

    Note that some databases need to be open() before this can be
    determined.

    \sa DriverFeature
*/

/*!
    Protected function which sets the open state of the database to \a
    o. Derived classes can use this function to report the status of
    open().

    \sa open(), setOpenError()
*/

void QSqlDriver::setOpen( bool o )
{
    if ( o )
	dbState |= DBState_Open;
    else
	dbState &= ~DBState_Open;
}

/*!
    Protected function which sets the open error state of the database
    to \a e. Derived classes can use this function to report the
    status of open(). Note that if \a e is TRUE the open state of the
    database is set to closed (i.e. isOpen() returns FALSE).

    \sa open(), setOpenError()
*/

void QSqlDriver::setOpenError( bool e )
{
    if ( e ) {
	dbState |= DBState_OpenError;
	dbState &= ~DBState_Open;
    }
    else
	dbState &= ~DBState_OpenError;
}

/*!
    Protected function which derived classes can reimplement to begin
    a transaction. If successful, return TRUE, otherwise return FALSE.
    The default implementation returns FALSE.

    \sa commitTransaction(), rollbackTransaction()
*/

bool QSqlDriver::beginTransaction()
{
    return FALSE;
}

/*!
    Protected function which derived classes can reimplement to commit
    a transaction. If successful, return TRUE, otherwise return FALSE.
    The default implementation returns FALSE.

    \sa beginTransaction(), rollbackTransaction()
*/

bool QSqlDriver::commitTransaction()
{
    return FALSE;
}

/*!
    Protected function which derived classes can reimplement to
    rollback a transaction. If successful, return TRUE, otherwise
    return FALSE. The default implementation returns FALSE.

    \sa beginTransaction(), commitTransaction()
*/

bool QSqlDriver::rollbackTransaction()
{
    return FALSE;
}

/*!
    Protected function which allows derived classes to set the value
    of the last error, \a e, that occurred on the database.

    \sa lastError()
*/

void QSqlDriver::setLastError( const QSqlError& e )
{
    error = e;
}

/*!
    Returns a QSqlError object which contains information about the
    last error that occurred on the database.
*/

QSqlError QSqlDriver::lastError() const
{
    return error;
}

/*!
    Returns a list of tables in the database. The default
    implementation returns an empty list.

    The \a tableType argument describes what types of tables
    should be returned. Due to binary compatibility, the string
    contains the value of the enum QSql::TableTypes as text.
    An empty string should be treated as QSql::Tables for
    downward compatibility.

    \sa QSql::TableType
*/

QStringList QSqlDriver::tables( const QString&	) const
{
    return QStringList();
}

/*!
    Returns the primary index for table \a tableName. Returns an empty
    QSqlIndex if the table doesn't have a primary index. The default
    implementation returns an empty index.
*/

QSqlIndex QSqlDriver::primaryIndex( const QString& ) const
{
    return QSqlIndex();
}


/*!
    Returns a QSqlRecord populated with the names of the fields in
    table \a tableName. If no such table exists, an empty record is
    returned. The default implementation returns an empty record.
*/

QSqlRecord QSqlDriver::record( const QString&  ) const
{
    return QSqlRecord();
}

/*!
    \overload

    Returns a QSqlRecord populated with the names of the fields in the
    SQL \a query. The default implementation returns an empty record.
*/

QSqlRecord QSqlDriver::record( const QSqlQuery& ) const
{
   return QSqlRecord();
}

/*!
    Returns a QSqlRecordInfo object with meta data about the table \a
    tablename.
*/
QSqlRecordInfo QSqlDriver::recordInfo( const QString& tablename ) const
{
    return QSqlRecordInfo( record( tablename ) );
}

/*!
    \overload

    Returns a QSqlRecordInfo object with meta data for the QSqlQuery
    \a query. Note that this overloaded function may return less
    information than the recordInfo() function which takes the name of
    a table as parameter.
*/
QSqlRecordInfo QSqlDriver::recordInfo( const QSqlQuery& query ) const
{
    return QSqlRecordInfo( record( query ) );
}


/*!
    Returns a string representation of the NULL value for the
    database. This is used, for example, when constructing INSERT and
    UPDATE statements. The default implementation returns the string
    "NULL".
*/

QString QSqlDriver::nullText() const
{
    return "NULL";
}

/*!
    Returns a string representation of the \a field value for the
    database. This is used, for example, when constructing INSERT and
    UPDATE statements.

    The default implementation returns the value formatted as a string
    according to the following rules:

    \list

    \i If \a field is NULL, nullText() is returned.

    \i If \a field is character data, the value is returned enclosed
    in single quotation marks, which is appropriate for many SQL
    databases. Any embedded single-quote characters are escaped
    (replaced with two single-quote characters). If \a trimStrings is
    TRUE (the default is FALSE), all trailing whitespace is trimmed
    from the field.

    \i If \a field is date/time data, the value is formatted in ISO
    format and enclosed in single quotation marks. If the date/time
    data is invalid, nullText() is returned.

    \i If \a field is bytearray data, and the driver can edit binary
    fields, the value is formatted as a hexadecimal string.

    \i For any other field type toString() will be called on its value
    and the result returned.

    \endlist

    \sa QVariant::toString().

*/
QString QSqlDriver::formatValue( const QSqlField* field, bool trimStrings ) const
{
    QString r;
    if ( field->isNull() )
	r = nullText();
    else {
	switch ( field->type() ) {
	case QVariant::Int:
	case QVariant::UInt:
	    if ( field->value().type() == QVariant::Bool )
		r = field->value().toBool() ? "1" : "0";
	    else
		r = field->value().toString();
	    break;
	case QVariant::Date:
	    if ( field->value().toDate().isValid() )
		r = "'" + field->value().toDate().toString( Qt::ISODate ) + "'";
	    else
		r = nullText();
	    break;
	case QVariant::Time:
	    if ( field->value().toTime().isValid() )
		r = "'" + field->value().toTime().toString( Qt::ISODate ) + "'";
	    else
		r = nullText();
	    break;
	case QVariant::DateTime:
	    if ( field->value().toDateTime().isValid() )
		r = "'" +
		    field->value().toDateTime().toString( Qt::ISODate ) + "'";
	    else
		r = nullText();
	    break;
	case QVariant::String:
	case QVariant::CString: {
	    QString result = field->value().toString();
	    if ( trimStrings ) {
		int end = result.length() - 1;
		while ( end && result[end].isSpace() ) /* skip white space from end */
		    end--;
		result.truncate( end );
	    }
	    /* escape the "'" character */
	    result.replace( QChar( '\'' ), "''" );
	    r = "'" + result + "'";
	    break;
	}
	case QVariant::Bool:
	    if ( field->value().toBool() )
		r = "1";
	    else
		r = "0";
	    break;
	case QVariant::ByteArray : {
	    if ( hasFeature( BLOB ) ) {
		QByteArray ba = field->value().toByteArray();
		QString res;
		static const char hexchars[] = "0123456789abcdef";
		for ( uint i = 0; i < ba.size(); ++i ) {
		    uchar s = (uchar) ba[(int)i];
		    res += hexchars[s >> 4];
		    res += hexchars[s & 0x0f];
		}
		r = "'" + res + "'";
		break;
	    }
	}
	default:
	    r = field->value().toString();
	    break;
	}
    }
    return r;
}

/*!
    \overload
    
    Open a database connection on database \a db, using user name \a
    user, password \a password, host \a host, port \a port and
    connection options \a connOpts.

    Returns TRUE on success and FALSE on failure.

    \sa setOpen()
*/
bool QSqlDriver::open( const QString& db,
		       const QString& user,
		       const QString& password,
		       const QString& host,
		       int port,
		       const QString& connOpts )
{
    if ( !qSqlOpenExtDict()->isEmpty() ) {
	QSqlOpenExtension *ext = qSqlOpenExtDict()->find( (QSqlDriver *) this );
	if ( ext )
	    return ext->open( db, user, password, host, port, connOpts );
    }
    return open( db, user, password, host, port );
}

#endif // QT_NO_SQL
