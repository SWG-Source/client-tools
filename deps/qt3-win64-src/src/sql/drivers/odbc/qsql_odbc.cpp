/****************************************************************************
**
** Implementation of ODBC driver classes
**
** Created : 001103
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
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

#include "qsql_odbc.h"
#include <qsqlrecord.h>

#if defined (Q_OS_WIN32)
#include <qt_windows.h>
#include <qapplication.h>
#endif
#include <qdatetime.h>
#include <private/qsqlextension_p.h>
#include <private/qinternal_p.h>
#include <stdlib.h>

// undefine this to prevent initial check of the ODBC driver
#define ODBC_CHECK_DRIVER

#if defined(Q_ODBC_VERSION_2)
//crude hack to get non-unicode capable driver managers to work
# undef UNICODE
# define SQLTCHAR SQLCHAR
# define SQL_C_WCHAR SQL_C_CHAR
#endif

// newer platform SDKs use SQLLEN instead of SQLINTEGER
#ifdef SQLLEN
# define QSQLLEN SQLLEN
#else
# define QSQLLEN SQLINTEGER
#endif

#ifdef SQLULEN
# define QSQLULEN SQLULEN
#else
# define QSQLULEN SQLUINTEGER
#endif


static const QSQLLEN COLNAMESIZE = 256;
//Map Qt parameter types to ODBC types
static const SQLSMALLINT qParamType[ 4 ] = { SQL_PARAM_INPUT, SQL_PARAM_INPUT, SQL_PARAM_OUTPUT, SQL_PARAM_INPUT_OUTPUT };

class QODBCPrivate
{
public:
    QODBCPrivate()
    : hEnv(0), hDbc(0), hStmt(0), useSchema(FALSE)
    {
	sql_char_type = sql_varchar_type = sql_longvarchar_type = QVariant::CString;
	unicode = FALSE;
    }

    SQLHANDLE hEnv;
    SQLHANDLE hDbc;
    SQLHANDLE hStmt;

    bool unicode;
    bool useSchema;
    QVariant::Type sql_char_type;
    QVariant::Type sql_varchar_type;
    QVariant::Type sql_longvarchar_type;

    QSqlRecordInfo rInf;

    bool checkDriver() const;
    void checkUnicode();
    void checkSchemaUsage();
    bool setConnectionOptions( const QString& connOpts );
    void splitTableQualifier(const QString &qualifier, QString &catalog,
			     QString &schema, QString &table);
};

class QODBCPreparedExtension : public QSqlExtension
{
public:
    QODBCPreparedExtension( QODBCResult * r )
	: result( r ) {}

    bool prepare( const QString& query )
    {
	return result->prepare( query );
    }

    bool exec()
    {
	return result->exec();
    }

    QODBCResult * result;
};

QPtrDict<QSqlOpenExtension> *qSqlOpenExtDict();

class QODBCOpenExtension : public QSqlOpenExtension
{
public:
    QODBCOpenExtension( QODBCDriver *dri )
	: QSqlOpenExtension(), driver(dri) {}
    ~QODBCOpenExtension() {}

    bool open( const QString& db,
	       const QString& user,
	       const QString& password,
	       const QString& host,
	       int port,
	       const QString& connOpts );
private:
    QODBCDriver *driver;
};

bool QODBCOpenExtension::open( const QString& db,
			       const QString& user,
			       const QString& password,
			       const QString& host,
			       int port,
			       const QString& connOpts )
{
    return driver->open( db, user, password, host, port, connOpts );
}

static QString qWarnODBCHandle(int handleType, SQLHANDLE handle)
{
    SQLINTEGER nativeCode_;
    SQLSMALLINT msgLen;
    SQLRETURN r = SQL_ERROR;
    SQLTCHAR state_[SQL_SQLSTATE_SIZE+1];
    SQLTCHAR description_[SQL_MAX_MESSAGE_LENGTH];
    r = SQLGetDiagRec( handleType,
			 handle,
			 1,
			 (SQLTCHAR*)state_,
			 &nativeCode_,
			 (SQLTCHAR*)description_,
			 SQL_MAX_MESSAGE_LENGTH-1, /* in bytes, not in characters */
			 &msgLen);
    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO )
#ifdef UNICODE
	return QString( (const QChar*)description_, (uint)msgLen );
#else
	return QString::fromLocal8Bit( (const char*)description_ );
#endif
    return QString::null;
}

static QString qODBCWarn( const QODBCPrivate* odbc)
{
    return ( qWarnODBCHandle( SQL_HANDLE_ENV, odbc->hEnv ) + " "
	     + qWarnODBCHandle( SQL_HANDLE_DBC, odbc->hDbc ) + " "
	     + qWarnODBCHandle( SQL_HANDLE_STMT, odbc->hStmt ) );
}

static void qSqlWarning( const QString& message, const QODBCPrivate* odbc )
{
#ifdef QT_CHECK_RANGE
    qWarning( "%s\tError: %s", message.local8Bit().data(), qODBCWarn( odbc ).local8Bit().data() );
#endif
}

static QSqlError qMakeError( const QString& err, int type, const QODBCPrivate* p )
{
    return QSqlError( "QODBC3: " + err, qODBCWarn(p), type );
}

static QVariant::Type qDecodeODBCType( SQLSMALLINT sqltype, const QODBCPrivate* p )
{
    QVariant::Type type = QVariant::Invalid;
    switch ( sqltype ) {
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
	type = QVariant::Double;
	break;
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIT:
    case SQL_TINYINT:
	type = QVariant::Int;
	break;
    case SQL_BIGINT:
	type = QVariant::LongLong;
	break;
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
	type = QVariant::ByteArray;
	break;
    case SQL_DATE:
    case SQL_TYPE_DATE:
	type = QVariant::Date;
	break;
    case SQL_TIME:
    case SQL_TYPE_TIME:
	type = QVariant::Time;
	break;
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIMESTAMP:
	type = QVariant::DateTime;
	break;
#ifndef Q_ODBC_VERSION_2
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
	type = QVariant::String;
	break;
#endif
    case SQL_CHAR:
	type = p->sql_char_type;
	break;
    case SQL_VARCHAR:
	type = p->sql_varchar_type;
	break;
    case SQL_LONGVARCHAR:
	type = p->sql_longvarchar_type;
	break;
    default:
	type = QVariant::CString;
	break;
    }
    return type;
}

static QString qGetStringData( SQLHANDLE hStmt, int column, int colSize, bool& isNull, bool unicode = FALSE )
{
    QString     fieldVal;
    SQLRETURN   r = SQL_ERROR;
    QSQLLEN  lengthIndicator = 0;

    if ( colSize <= 0 ) {
	colSize = 256;
    } else if ( colSize > 65536 ) { // limit buffer size to 64 KB
	colSize = 65536;
    } else {
	colSize++; // make sure there is room for more than the 0 termination
	if ( unicode ) {
	    colSize *= 2; // a tiny bit faster, since it saves a SQLGetData() call
	}
    }
    char* buf = new char[ colSize ];
    while ( TRUE ) {
	r = SQLGetData( hStmt,
			column+1,
			unicode ? SQL_C_WCHAR : SQL_C_CHAR,
			(SQLPOINTER)buf,
			(QSQLLEN)colSize,
			&lengthIndicator );
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	    if ( lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL ) {
		fieldVal = QString::null;
		isNull = TRUE;
		break;
	    }
	    // if SQL_SUCCESS_WITH_INFO is returned, indicating that
	    // more data can be fetched, the length indicator does NOT
	    // contain the number of bytes returned - it contains the
	    // total number of bytes that CAN be fetched
	    // colSize-1: remove 0 termination when there is more data to fetch
	    int rSize = (r == SQL_SUCCESS_WITH_INFO) ? (unicode ? colSize-2 : colSize-1) : lengthIndicator;
	    if ( unicode ) {
		fieldVal += QString( (QChar*) buf, rSize / 2 );
	    } else {
		buf[ rSize ] = 0;
		fieldVal += buf;
	    }
	    if ( lengthIndicator < colSize ) {
		// workaround for Drivermanagers that don't return SQL_NO_DATA
		break;
	    }
	} else if ( r == SQL_NO_DATA ) {
	    break;
	} else {
#ifdef QT_CHECK_RANGE
	    qWarning( "qGetStringData: Error while fetching data (%d)", r );
#endif
	    fieldVal = QString::null;
	    break;
	}
    }
    delete[] buf;
    return fieldVal;
}

static QByteArray qGetBinaryData( SQLHANDLE hStmt, int column, QSQLLEN& lengthIndicator, bool& isNull )
{
    QByteArray fieldVal;
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    QSQLULEN colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;

    SQLTCHAR colName[COLNAMESIZE];
    r = SQLDescribeCol( hStmt,
			column+1,
			colName,
			COLNAMESIZE,
			&colNameLen,
			&colType,
			&colSize,
			&colScale,
			&nullable );
#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qWarning( "qGetBinaryData: Unable to describe column %d", column );
#endif
    // SQLDescribeCol may return 0 if size cannot be determined
    if (!colSize) {
	colSize = 256;
    }
    if ( colSize > 65536 ) { // read the field in 64 KB chunks
	colSize = 65536;
    }
    char * buf = new char[ colSize ];
    while ( TRUE ) {
	r = SQLGetData( hStmt,
			column+1,
			SQL_C_BINARY,
			(SQLPOINTER) buf,
			(QSQLLEN)colSize,
			&lengthIndicator );
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	    if ( lengthIndicator == SQL_NULL_DATA ) {
 		isNull = TRUE;
		break;
	    } else {
		int rSize;
		r == SQL_SUCCESS ? rSize = lengthIndicator : rSize = colSize;
		if ( lengthIndicator == SQL_NO_TOTAL ) { // size cannot be determined
		    rSize = colSize;
		}
		// NB! This is not a memleak - the mem will be deleted by QByteArray when
		// no longer ref'd
		char * tmp = (char *) malloc( rSize + fieldVal.size() );
		if ( fieldVal.size() ) {
		    memcpy( tmp, fieldVal.data(), fieldVal.size() );
		}
		memcpy( tmp + fieldVal.size(), buf, rSize );
		fieldVal = fieldVal.assign( tmp, fieldVal.size() + rSize );

		if ( r == SQL_SUCCESS ) { // the whole field was read in one chunk
		    break;
		}
	    }
	} else {
	    break;
	}
    }
    delete [] buf;
    return fieldVal;
}

static int qGetIntData( SQLHANDLE hStmt, int column, bool& isNull  )
{
    QSQLLEN intbuf = 0;
    isNull = FALSE;
    QSQLLEN lengthIndicator = 0;
    SQLRETURN r = SQLGetData( hStmt,
			      column+1,
			      SQL_C_SLONG,
			      (SQLPOINTER)&intbuf,
			      (QSQLLEN)0,
			      &lengthIndicator );
    if ( ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) || lengthIndicator == SQL_NULL_DATA ) {
	isNull = TRUE;
	return 0;
    }
    return (int)intbuf;
}

static double qGetDoubleData( SQLHANDLE hStmt, int column, bool& isNull )
{
    SQLDOUBLE dblbuf;
    QSQLLEN lengthIndicator = 0;
    isNull = FALSE;
    SQLRETURN r = SQLGetData( hStmt,
			      column+1,
			      SQL_C_DOUBLE,
			      (SQLPOINTER)&dblbuf,
			      (QSQLLEN)0,
			      &lengthIndicator );
    if ( ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) || lengthIndicator == SQL_NULL_DATA ) {
	isNull = TRUE;
	return 0.0;
    }

    return (double) dblbuf;
}

static SQLBIGINT qGetBigIntData( SQLHANDLE hStmt, int column, bool& isNull )
{
    SQLBIGINT lngbuf = Q_INT64_C( 0 );
    isNull = FALSE;
    QSQLLEN lengthIndicator = 0;
    SQLRETURN r = SQLGetData( hStmt,
			      column+1,
			      SQL_C_SBIGINT,
			      (SQLPOINTER) &lngbuf,
			      (QSQLLEN)0,
			      &lengthIndicator );
    if ( ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) || lengthIndicator == SQL_NULL_DATA )
	isNull = TRUE;

    return lngbuf;
}

// creates a QSqlFieldInfo from a valid hStmt generated
// by SQLColumns. The hStmt has to point to a valid position.
static QSqlFieldInfo qMakeFieldInfo( const SQLHANDLE hStmt, const QODBCPrivate* p )
{
    bool isNull;
    QString fname = qGetStringData( hStmt, 3, -1, isNull, p->unicode );
    int type = qGetIntData( hStmt, 4, isNull ); // column type
    int required = qGetIntData( hStmt, 10, isNull ); // nullable-flag
    // required can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    if ( required == SQL_NO_NULLS ) {
	required = 1;
    } else if ( required == SQL_NULLABLE ) {
	required = 0;
    } else {
	required = -1;
    }
    int size = qGetIntData( hStmt, 6, isNull ); // column size
    int prec = qGetIntData( hStmt, 8, isNull ); // precision
    return QSqlFieldInfo( fname, qDecodeODBCType( type, p ), required, size, prec, QVariant(), type );
}

static QSqlFieldInfo qMakeFieldInfo( const QODBCPrivate* p, int i  )
{
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    QSQLULEN colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    SQLTCHAR colName[ COLNAMESIZE ];
    r = SQLDescribeCol( p->hStmt,
			i+1,
			colName,
			(QSQLULEN)COLNAMESIZE,
			&colNameLen,
			&colType,
			&colSize,
			&colScale,
			&nullable);

    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( QString("qMakeField: Unable to describe column %1").arg(i), p );
#endif
	return QSqlFieldInfo();
    }
#ifdef UNICODE
    QString qColName( (const QChar*)colName, (uint)colNameLen );
#else
    QString qColName = QString::fromLocal8Bit( (const char*)colName );
#endif
    // nullable can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    int required = -1;
    if ( nullable == SQL_NO_NULLS ) {
	required = 1;
    } else if ( nullable == SQL_NULLABLE ) {
	required = 0;
    }
    QVariant::Type type = qDecodeODBCType( colType, p );
    return QSqlFieldInfo( qColName,
    			  type,
    			  required,
    			  (int)colSize == 0 ? -1 : (int)colSize,
    			  (int)colScale == 0 ? -1 : (int)colScale,
    			  QVariant(),
    			  (int)colType );
}

bool QODBCPrivate::setConnectionOptions( const QString& connOpts )
{
    // Set any connection attributes
    QStringList raw = QStringList::split( ';', connOpts );
    QStringList opts;
    SQLRETURN r = SQL_SUCCESS;
    QMap<QString, QString> connMap;
    for ( QStringList::ConstIterator it = raw.begin(); it != raw.end(); ++it ) {
	QString tmp( *it );
	int idx;
	if ( (idx = tmp.find( '=' )) != -1 )
	    connMap[ tmp.left( idx ) ] = tmp.mid( idx + 1 ).simplifyWhiteSpace();
	else
	    qWarning( "QODBCDriver::open: Illegal connect option value '%s'", tmp.latin1() );
    }
    if ( connMap.count() ) {
	QMap<QString, QString>::ConstIterator it;
	QString opt, val;
	SQLUINTEGER v = 0;
	for ( it = connMap.begin(); it != connMap.end(); ++it ) {
	    opt = it.key().upper();
	    val = it.data().upper();
	    r = SQL_SUCCESS;
	    if ( opt == "SQL_ATTR_ACCESS_MODE" ) {
		if ( val == "SQL_MODE_READ_ONLY" ) {
		    v = SQL_MODE_READ_ONLY;
		} else if ( val == "SQL_MODE_READ_WRITE" ) {
		    v = SQL_MODE_READ_WRITE;
		} else {
		    qWarning( QString( "QODBCDriver::open: Unknown option value '%1'" ).arg( *it ) );
		    break;
		}
		r = SQLSetConnectAttr( hDbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER) v, 0 );
	    } else if ( opt == "SQL_ATTR_CONNECTION_TIMEOUT" ) {
		v = val.toUInt();
		r = SQLSetConnectAttr( hDbc, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER) v, 0 );
	    } else if ( opt == "SQL_ATTR_LOGIN_TIMEOUT" ) {
		v = val.toUInt();
		r = SQLSetConnectAttr( hDbc, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) v, 0 );
	    } else if ( opt == "SQL_ATTR_CURRENT_CATALOG" ) {
		val.ucs2(); // 0 terminate
		r = SQLSetConnectAttr( hDbc, SQL_ATTR_CURRENT_CATALOG,
#ifdef UNICODE
				       (SQLWCHAR*) val.unicode(),
#else
				       (SQLCHAR*) val.latin1(),
#endif
				       SQL_NTS );
	    } else if ( opt == "SQL_ATTR_METADATA_ID" ) {
		if ( val == "SQL_TRUE" ) {
		    v = SQL_TRUE;
		} else if ( val == "SQL_FALSE" ) {
		    v = SQL_FALSE;
		} else {
		    qWarning( QString( "QODBCDriver::open: Unknown option value '%1'" ).arg( *it ) );
		    break;
		}
		r = SQLSetConnectAttr( hDbc, SQL_ATTR_METADATA_ID, (SQLPOINTER) v, 0 );
	    } else if ( opt == "SQL_ATTR_PACKET_SIZE" ) {
		v = val.toUInt();
		r = SQLSetConnectAttr( hDbc, SQL_ATTR_PACKET_SIZE, (SQLPOINTER) v, 0 );
	    } else if ( opt == "SQL_ATTR_TRACEFILE" ) {
		val.ucs2(); // 0 terminate
		r = SQLSetConnectAttr( hDbc, SQL_ATTR_TRACEFILE,
#ifdef UNICODE
				       (SQLWCHAR*) val.unicode(),
#else
				       (SQLCHAR*) val.latin1(),
#endif
				       SQL_NTS );
	    } else if ( opt == "SQL_ATTR_TRACE" ) {
		if ( val == "SQL_OPT_TRACE_OFF" ) {
		    v = SQL_OPT_TRACE_OFF;
		} else if ( val == "SQL_OPT_TRACE_ON" ) {
		    v = SQL_OPT_TRACE_ON;
		} else {
		    qWarning( QString( "QODBCDriver::open: Unknown option value '%1'" ).arg( *it ) );
		    break;
		}
		r = SQLSetConnectAttr( hDbc, SQL_ATTR_TRACE, (SQLPOINTER) v, 0 );
	    }
#ifdef QT_CHECK_RANGE
              else {
		  qWarning( QString("QODBCDriver::open: Unknown connection attribute '%1'").arg( opt ) );
	    }
#endif
	    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
#ifdef QT_CHECK_RANGE
		qSqlWarning( QString("QODBCDriver::open: Unable to set connection attribute '%1'").arg( opt ), this );
#endif
		return FALSE;
	    }
	}
    }
    return TRUE;
}

void QODBCPrivate::splitTableQualifier(const QString & qualifier, QString &catalog,
				       QString &schema, QString &table)
{
    if (!useSchema) {
	table = qualifier;
	return;
    }
    QStringList l = QStringList::split( ".", qualifier, TRUE );
    if ( l.count() > 3 )
	return; // can't possibly be a valid table qualifier
    int i = 0, n = l.count();
    if ( n == 1 ) {
	table = qualifier;
    } else {
	for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
	    if ( n == 3 ) {
		if ( i == 0 ) {
		    catalog = *it;
		} else if ( i == 1 ) {
		    schema = *it;
		} else if ( i == 2 ) {
		    table = *it;
		}
	    } else if ( n == 2 ) {
		if ( i == 0 ) {
		    schema = *it;
		} else if ( i == 1 ) {
		    table = *it;
		}
	    }
	    i++;
	}
    }
}

////////////////////////////////////////////////////////////////////////////

QODBCResult::QODBCResult( const QODBCDriver * db, QODBCPrivate* p )
: QSqlResult(db)
{
    d = new QODBCPrivate();
    (*d) = (*p);
    setExtension( new QODBCPreparedExtension( this ) );
}

QODBCResult::~QODBCResult()
{
    if ( d->hStmt && driver()->isOpen() ) {
	SQLRETURN r = SQLFreeHandle( SQL_HANDLE_STMT, d->hStmt );
#ifdef QT_CHECK_RANGE
	if ( r != SQL_SUCCESS )
	    qSqlWarning( "QODBCDriver: Unable to free statement handle " + QString::number(r), d );
#endif
    }

    delete d;
}

bool QODBCResult::reset ( const QString& query )
{
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    SQLRETURN r;

    d->rInf.clear();
    // Always reallocate the statement handle - the statement attributes
    // are not reset if SQLFreeStmt() is called which causes some problems.
    if ( d->hStmt ) {
	r = SQLFreeHandle( SQL_HANDLE_STMT, d->hStmt );
	if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	    qSqlWarning( "QODBCResult::reset: Unable to free statement handle", d );
#endif
	    return FALSE;
	}
    }
    r  = SQLAllocHandle( SQL_HANDLE_STMT,
			 d->hDbc,
			 &d->hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCResult::reset: Unable to allocate statement handle", d );
#endif
	return FALSE;
    }

    if ( isForwardOnly() ) {
	r = SQLSetStmtAttr( d->hStmt,
			    SQL_ATTR_CURSOR_TYPE,
			    (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			    SQL_IS_UINTEGER );
    } else {
	r = SQLSetStmtAttr( d->hStmt,
			    SQL_ATTR_CURSOR_TYPE,
			    (SQLPOINTER)SQL_CURSOR_STATIC,
			    SQL_IS_UINTEGER );
    }
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCResult::reset: Unable to set 'SQL_CURSOR_STATIC' as statement attribute. Please check your ODBC driver configuration", d );
#endif
	return FALSE;
    }

#ifdef UNICODE
    r = SQLExecDirect( d->hStmt,
		       (SQLWCHAR*) query.unicode(),
		       (SQLINTEGER) query.length() );
#else
    QCString query8 = query.local8Bit();
    r = SQLExecDirect( d->hStmt,
                       (SQLCHAR*) query8.data(),
                       (SQLINTEGER) query8.length() );
#endif
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	return FALSE;
    }
    SQLSMALLINT count;
    r = SQLNumResultCols( d->hStmt, &count );
    if ( count ) {
	setSelect( TRUE );
	for ( int i = 0; i < count; ++i ) {
	    d->rInf.append( qMakeFieldInfo( d, i ) );
	}
    } else {
	setSelect( FALSE );
    }
    setActive( TRUE );
    return TRUE;
}

bool QODBCResult::fetch(int i)
{
    if ( isForwardOnly() && i < at() )
	return FALSE;
    if ( i == at() )
	return TRUE;
    fieldCache.clear();
    nullCache.clear();
    int actualIdx = i + 1;
    if ( actualIdx <= 0 ) {
	setAt( QSql::BeforeFirst );
	return FALSE;
    }
    SQLRETURN r;
    if ( isForwardOnly() ) {
	bool ok = TRUE;
	while ( ok && i > at() )
	    ok = fetchNext();
	return ok;
    } else {
	r = SQLFetchScroll( d->hStmt,
			    SQL_FETCH_ABSOLUTE,
			    actualIdx );
    }
    if ( r != SQL_SUCCESS ){
 	return FALSE;
    }
    setAt( i );
    return TRUE;
}

bool QODBCResult::fetchNext()
{
    SQLRETURN r;
    fieldCache.clear();
    nullCache.clear();
    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_NEXT,
		       0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( at() + 1 );
    return TRUE;
}

bool QODBCResult::fetchFirst()
{
    if ( isForwardOnly() && at() != QSql::BeforeFirst )
	return FALSE;
    SQLRETURN r;
    fieldCache.clear();
    nullCache.clear();
    if ( isForwardOnly() ) {
	return fetchNext();
    }
    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_FIRST,
		       0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( 0 );
    return TRUE;
}

bool QODBCResult::fetchPrior()
{
    if ( isForwardOnly() )
	return FALSE;
    SQLRETURN r;
    fieldCache.clear();
    nullCache.clear();
    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_PRIOR,
		       0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( at() - 1 );
    return TRUE;
}

bool QODBCResult::fetchLast()
{
    SQLRETURN r;
    fieldCache.clear();
    nullCache.clear();

    if ( isForwardOnly() ) {
	// cannot seek to last row in forwardOnly mode, so we have to use brute force
	int i = at();
	if ( i == QSql::AfterLast )
	    return FALSE;
	if ( i == QSql::BeforeFirst )
	    i = 0;
	while ( fetchNext() )
	    ++i;
	setAt( i );
	return TRUE;
    }

    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_LAST,
		       0 );
    if ( r != SQL_SUCCESS ) {
	return FALSE;
    }
    SQLINTEGER currRow;
    r = SQLGetStmtAttr( d->hStmt,
			SQL_ROW_NUMBER,
			&currRow,
			SQL_IS_INTEGER,
			0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( currRow-1 );
    return TRUE;
}

QVariant QODBCResult::data( int field )
{
    if ( field >= (int) d->rInf.count() ) {
	qWarning( "QODBCResult::data: column %d out of range", field );
	return QVariant();
    }
    if ( fieldCache.contains( field ) )
	return fieldCache[ field ];
    SQLRETURN r(0);
    QSQLLEN lengthIndicator = 0;
    bool isNull = FALSE;
    int current = fieldCache.count();
    for ( ; current < (field + 1); ++current ) {
	const QSqlFieldInfo info = d->rInf[ current ];
	switch ( info.type() ) {
	case QVariant::LongLong:
	    fieldCache[ current ] = QVariant( (Q_LLONG) qGetBigIntData( d->hStmt, current, isNull ) );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::Int:
	    fieldCache[ current ] = QVariant( qGetIntData( d->hStmt, current, isNull ) );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::Date:
	    DATE_STRUCT dbuf;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_DATE,
			    (SQLPOINTER)&dbuf,
			    (QSQLLEN)0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		fieldCache[ current ] = QVariant( QDate( dbuf.year, dbuf.month, dbuf.day ) );
		nullCache[ current ] = FALSE;
	    } else {
		fieldCache[ current ] = QVariant( QDate() );
		nullCache[ current ] = TRUE;
	    }
	    break;
	case QVariant::Time:
	    TIME_STRUCT tbuf;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_TIME,
			    (SQLPOINTER)&tbuf,
			    (QSQLLEN)0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		fieldCache[ current ] = QVariant( QTime( tbuf.hour, tbuf.minute, tbuf.second ) );
		nullCache[ current ] = FALSE;
	    } else {
		fieldCache[ current ] = QVariant( QTime() );
		nullCache[ current ] = TRUE;
	    }
	    break;
	case QVariant::DateTime:
	    TIMESTAMP_STRUCT dtbuf;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_TIMESTAMP,
			    (SQLPOINTER)&dtbuf,
			    (QSQLLEN)0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		fieldCache[ current ] = QVariant( QDateTime( QDate( dtbuf.year, dtbuf.month, dtbuf.day ), QTime( dtbuf.hour, dtbuf.minute, dtbuf.second, dtbuf.fraction / 1000000 ) ) );
		nullCache[ current ] = FALSE;
	    } else {
		fieldCache[ current ] = QVariant( QDateTime() );
		nullCache[ current ] = TRUE;
	    }
	    break;
        case QVariant::ByteArray: {
	    isNull = FALSE;
	    QByteArray val = qGetBinaryData( d->hStmt, current, lengthIndicator, isNull );
	    fieldCache[ current ] = QVariant( val );
	    nullCache[ current ] = isNull;
	    break; }
	case QVariant::String:
	    isNull = FALSE;
	    fieldCache[ current ] = QVariant( qGetStringData( d->hStmt, current,
					      info.length(), isNull, TRUE ) );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::Double:
	    if ( info.typeID() == SQL_DECIMAL || info.typeID() == SQL_NUMERIC )
		// bind Double values as string to prevent loss of precision
		fieldCache[ current ] = QVariant( qGetStringData( d->hStmt, current,
						  info.length() + 1, isNull, FALSE ) ); // length + 1 for the comma
	    else
		fieldCache[ current ] = QVariant( qGetDoubleData( d->hStmt, current, isNull ) );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::CString:
	default:
	    isNull = FALSE;
	    fieldCache[ current ] = QVariant( qGetStringData( d->hStmt, current,
					      info.length(), isNull, FALSE ) );
	    nullCache[ current ] = isNull;
	    break;
	}
    }
    return fieldCache[ --current ];
}

bool QODBCResult::isNull( int field )
{
    if ( !fieldCache.contains( field ) ) {
	// since there is no good way to find out whether the value is NULL
	// without fetching the field we'll fetch it here.
	// (data() also sets the NULL flag)
	data( field );
    }
    return nullCache[ field ];
}

int QODBCResult::size()
{
    return -1;
}

int QODBCResult::numRowsAffected()
{
    QSQLLEN affectedRowCount(0);
    SQLRETURN r = SQLRowCount( d->hStmt, &affectedRowCount );
    if ( r == SQL_SUCCESS )
	return affectedRowCount;
#ifdef QT_CHECK_RANGE
    else
	qSqlWarning( "QODBCResult::numRowsAffected: Unable to count affected rows", d );
#endif
    return -1;
}

bool QODBCResult::prepare( const QString& query )
{
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    SQLRETURN r;

    d->rInf.clear();
    if ( d->hStmt ) {
	r = SQLFreeHandle( SQL_HANDLE_STMT, d->hStmt );
	if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	    qSqlWarning( "QODBCResult::prepare: Unable to close statement", d );
#endif
	    return FALSE;
	}
    }
    r  = SQLAllocHandle( SQL_HANDLE_STMT,
	    		 d->hDbc,
			 &d->hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCResult::prepare: Unable to allocate statement handle", d );
#endif
	return FALSE;
    }

    if ( isForwardOnly() ) {
	r = SQLSetStmtAttr( d->hStmt,
	    		    SQL_ATTR_CURSOR_TYPE,
			    (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			    SQL_IS_UINTEGER );
    } else {
	r = SQLSetStmtAttr( d->hStmt,
			    SQL_ATTR_CURSOR_TYPE,
			    (SQLPOINTER)SQL_CURSOR_STATIC,
			    SQL_IS_UINTEGER );
    }
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCResult::prepare: Unable to set 'SQL_CURSOR_STATIC' as statement attribute. Please check your ODBC driver configuration", d );
#endif
	return FALSE;
    }

#ifdef UNICODE
    r = SQLPrepare( d->hStmt,
		    (SQLWCHAR*) query.unicode(),
		    (SQLINTEGER) query.length() );
#else
    QCString query8 = query.local8Bit();
    r = SQLPrepare( d->hStmt,
		    (SQLCHAR*) query8.data(),
		    (SQLINTEGER) query8.length() );
#endif

    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCResult::prepare: Unable to prepare statement", d );
#endif
	return FALSE;
    }
    return TRUE;
}

bool QODBCResult::exec()
{
    SQLRETURN r;
    QPtrList<QVirtualDestructor> tmpStorage; // holds temporary ptrs. which will be deleted on fu exit
    tmpStorage.setAutoDelete( TRUE );

    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    d->rInf.clear();

    if ( !d->hStmt ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCResult::exec: No statement handle available", d );
#endif
	return FALSE;
    } else {
	r = SQLFreeStmt( d->hStmt, SQL_CLOSE );
	if ( r != SQL_SUCCESS ) {
	    qSqlWarning( "QODBCResult::exec: Unable to close statement handle", d );
	    return FALSE;
	}
    }

    // bind parameters - only positional binding allowed
    if ( extension()->index.count() > 0 ) {
	QMap<int, QString>::Iterator it;
	int para = 1;
	QVariant val;
	for ( it = extension()->index.begin(); it != extension()->index.end(); ++it ) {
	    val = extension()->values[ it.data() ].value;
	    QSQLLEN *ind = new QSQLLEN( SQL_NTS );
	    tmpStorage.append( qAutoDeleter(ind) );
	    if ( val.isNull() ) {
		*ind = SQL_NULL_DATA;
	    }
	    switch ( val.type() ) {
		case QVariant::Date: {
		    DATE_STRUCT * dt = new DATE_STRUCT;
		    tmpStorage.append( qAutoDeleter(dt) );
		    QDate qdt = val.toDate();
		    dt->year = qdt.year();
		    dt->month = qdt.month();
		    dt->day = qdt.day();
		    r = SQLBindParameter( d->hStmt,
					  para,
					  qParamType[ (int)extension()->values[ it.data() ].typ ],
					  SQL_C_DATE,
					  SQL_DATE,
					  0,
					  0,
					  (void *) dt,
					  (QSQLLEN)0,
					  *ind == SQL_NULL_DATA ? ind : NULL );
		    break; }
		case QVariant::Time: {
		    TIME_STRUCT * dt = new TIME_STRUCT;
		    tmpStorage.append( qAutoDeleter(dt) );
		    QTime qdt = val.toTime();
		    dt->hour = qdt.hour();
		    dt->minute = qdt.minute();
		    dt->second = qdt.second();
		    r = SQLBindParameter( d->hStmt,
					  para,
					  qParamType[ (int)extension()->values[ it.data() ].typ ],
					  SQL_C_TIME,
					  SQL_TIME,
					  0,
					  0,
					  (void *) dt,
					  (QSQLLEN)0,
	        			  *ind == SQL_NULL_DATA ? ind : NULL );
		    break; }
		case QVariant::DateTime: {
		    TIMESTAMP_STRUCT * dt = new TIMESTAMP_STRUCT;
		    tmpStorage.append( qAutoDeleter(dt) );
		    QDateTime qdt = val.toDateTime();
		    dt->year = qdt.date().year();
		    dt->month = qdt.date().month();
		    dt->day = qdt.date().day();
		    dt->hour = qdt.time().hour();
		    dt->minute = qdt.time().minute();
		    dt->second = qdt.time().second();
		    dt->fraction = 0;
		    r = SQLBindParameter( d->hStmt,
					  para,
					  qParamType[ (int)extension()->values[ it.data() ].typ ],
					  SQL_C_TIMESTAMP,
					  SQL_TIMESTAMP,
					  0,
					  0,
					  (void *) dt,
					  (QSQLLEN)0,
					  *ind == SQL_NULL_DATA ? ind : NULL );
		    break; }
	        case QVariant::Int: {
		    int * v = new int( val.toInt() );
		    tmpStorage.append( qAutoDeleter(v) );
		    r = SQLBindParameter( d->hStmt,
					  para,
					  qParamType[ (int)extension()->values[ it.data() ].typ ],
					  SQL_C_SLONG,
					  SQL_INTEGER,
					  0,
					  0,
					  (void *) v,
					  (QSQLLEN)0,
					  *ind == SQL_NULL_DATA ? ind : NULL );
		    break; }
	        case QVariant::Double: {
		    double * v = new double( val.toDouble() );
		    tmpStorage.append( qAutoDeleter(v) );
		    r = SQLBindParameter( d->hStmt,
					  para,
					  qParamType[ (int)extension()->values[ it.data() ].typ ],
					  SQL_C_DOUBLE,
					  SQL_DOUBLE,
					  0,
					  0,
					  (void *) v,
					  (QSQLLEN)0,
					  *ind == SQL_NULL_DATA ? ind : NULL );
		    break; }
	        case QVariant::ByteArray: {
		    if ( *ind != SQL_NULL_DATA ) {
			*ind = val.asByteArray().size();
		    }
		    r = SQLBindParameter( d->hStmt,
					  para,
					  qParamType[ (int)extension()->values[ it.data() ].typ ],
					  SQL_C_BINARY,
					  SQL_LONGVARBINARY,
					  val.asByteArray().size(),
					  0,
					  (void *) val.asByteArray().data(),
					  (QSQLLEN)val.asByteArray().size(),
					  ind );
		    break; }
#ifndef Q_ODBC_VERSION_2
	        case QVariant::String:
		    if ( d->unicode ) {
			QString * str = new QString( val.asString() );
			str->ucs2();
			int len = str->length()*2;
			tmpStorage.append( qAutoDeleter(str) );
			r = SQLBindParameter( d->hStmt,
					      para,
					      qParamType[ (int)extension()->values[ it.data() ].typ ],
					      SQL_C_WCHAR,
					      len > 8000 ? SQL_WLONGVARCHAR : SQL_WVARCHAR,
					      len > 8000 ? len : 0,
					      0,
					      (void *) str->unicode(),
					      (QSQLLEN) len,
					      ind );
			break;
		    }
#endif
		    // fall through
	        default: {
		    QCString * str = new QCString( val.asString().local8Bit() );
		    tmpStorage.append( qAutoDeleter(str) );
		    r = SQLBindParameter( d->hStmt,
					  para,
					  qParamType[ (int)extension()->values[ it.data() ].typ ],
					  SQL_C_CHAR,
					  str->length() > 4000 ? SQL_LONGVARCHAR : SQL_VARCHAR,
					  str->length() + 1,
					  0,
					  (void *) str->data(),
					  (QSQLLEN)(str->length() + 1),
					  ind );
		    break; }
	    }
	    para++;
	    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
		qWarning( "QODBCResult::exec: unable to bind variable: %s", qODBCWarn( d ).local8Bit().data() );
#endif
		setLastError( qMakeError( "Unable to bind variable", QSqlError::Statement, d ) );
		return FALSE;
	    }
	}
    }
    r = SQLExecute( d->hStmt );
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QODBCResult::exec: Unable to execute statement: %s", qODBCWarn( d ).local8Bit().data() );
#endif
	setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	return FALSE;
    }
    SQLSMALLINT count;
    r = SQLNumResultCols( d->hStmt, &count );
    if ( count ) {
	setSelect( TRUE );
	for ( int i = 0; i < count; ++i ) {
	    d->rInf.append( qMakeFieldInfo( d, i ) );
	}
    } else {
	setSelect( FALSE );
    }
    setActive( TRUE );

    //get out parameters
    if ( extension()->index.count() > 0 ) {
	QMap<int, QString>::Iterator it;
	for ( it = extension()->index.begin(); it != extension()->index.end(); ++it ) {

	    SQLINTEGER* indPtr = qAutoDeleterData( (QAutoDeleter<SQLINTEGER>*)tmpStorage.getFirst() );
	    if ( !indPtr )
		return FALSE;
	    bool isNull = (*indPtr == SQL_NULL_DATA);
	    tmpStorage.removeFirst();

	    QVariant::Type type = extension()->values[ it.data() ].value.type();
	    if ( isNull ) {
		QVariant v;
		v.cast(type);
		extension()->values[ it.data() ].value = v;
		if (type != QVariant::ByteArray)
		    tmpStorage.removeFirst();
		continue;
	    }

	    switch (type) {
		case QVariant::Date: {
		    DATE_STRUCT * ds = qAutoDeleterData( (QAutoDeleter<DATE_STRUCT>*)tmpStorage.getFirst() );
		    extension()->values[ it.data() ].value = QVariant( QDate( ds->year, ds->month, ds->day ) );
		    break; }
		case QVariant::Time: {
		    TIME_STRUCT * dt = qAutoDeleterData( (QAutoDeleter<TIME_STRUCT>*)tmpStorage.getFirst() );
		    extension()->values[ it.data() ].value = QVariant( QTime( dt->hour, dt->minute, dt->second ) );
		    break; }
		case QVariant::DateTime: {
		    TIMESTAMP_STRUCT * dt = qAutoDeleterData( (QAutoDeleter<TIMESTAMP_STRUCT>*)tmpStorage.getFirst() );
		    extension()->values[ it.data() ].value = QVariant( QDateTime( QDate( dt->year, dt->month, dt->day ),
										  QTime( dt->hour, dt->minute, dt->second ) ) );
		    break; }
	        case QVariant::Int: {
		    int * v = qAutoDeleterData( (QAutoDeleter<int>*)tmpStorage.getFirst() );
		    extension()->values[ it.data() ].value = QVariant( *v );
		    break; }
	        case QVariant::Double: {
		    double * v = qAutoDeleterData( (QAutoDeleter<double>*)tmpStorage.getFirst() );
		    extension()->values[ it.data() ].value = QVariant( *v );
		    break; }
	        case QVariant::ByteArray:
		    break;
	        case QVariant::String:
		    if ( d->unicode ) {
			QString * str = qAutoDeleterData( (QAutoDeleter<QString>*)tmpStorage.getFirst() );
			extension()->values[ it.data() ].value = QVariant( *str );
			break;
		    }
		    // fall through
	        default: {
		    QCString * str = qAutoDeleterData( (QAutoDeleter<QCString>*)tmpStorage.getFirst() );
		    extension()->values[ it.data() ].value = QVariant( *str );
		    break; }
	    }
	    if (type != QVariant::ByteArray)
		tmpStorage.removeFirst();
	}
    }

    return TRUE;
}

////////////////////////////////////////


QODBCDriver::QODBCDriver( QObject * parent, const char * name )
    : QSqlDriver(parent,name ? name : "QODBC")
{
    init();
}

QODBCDriver::QODBCDriver( SQLHANDLE env, SQLHANDLE con, QObject * parent, const char * name )
    : QSqlDriver(parent,name ? name : "QODBC")
{
    init();
    d->hEnv = env;
    d->hDbc = con;
    if ( env && con ) {
	setOpen( TRUE );
	setOpenError( FALSE );
    }
}

void QODBCDriver::init()
{
    qSqlOpenExtDict()->insert( this, new QODBCOpenExtension(this) );
    d = new QODBCPrivate();
}

QODBCDriver::~QODBCDriver()
{
    cleanup();
    delete d;
    if ( !qSqlOpenExtDict()->isEmpty() ) {
	QSqlOpenExtension *ext = qSqlOpenExtDict()->take( this );
	delete ext;
    }
}

bool QODBCDriver::hasFeature( DriverFeature f ) const
{
    switch ( f ) {
    case Transactions: {
	if ( !d->hDbc )
	    return FALSE;
	SQLUSMALLINT txn;
	SQLSMALLINT t;
	int r = SQLGetInfo( d->hDbc,
			(SQLUSMALLINT)SQL_TXN_CAPABLE,
			&txn,
			sizeof(txn),
			&t);
	if ( r != SQL_SUCCESS || txn == SQL_TC_NONE )
	    return FALSE;
	else
	    return TRUE;
    }
    case QuerySize:
	return FALSE;
    case BLOB:
	return TRUE;
    case Unicode:
	return d->unicode;
    case PreparedQueries:
	return TRUE;
    case PositionalPlaceholders:
	return TRUE;
    default:
	return FALSE;
    }
}

bool QODBCDriver::open( const QString&,
			const QString&,
			const QString&,
			const QString&,
			int )
{
    qWarning("QODBCDriver::open(): This version of open() is no longer supported." );
    return FALSE;
}

bool QODBCDriver::open( const QString & db,
			const QString & user,
			const QString & password,
			const QString &,
			int,
			const QString& connOpts )
{
    if ( isOpen() )
      close();
    SQLRETURN r;
    r = SQLAllocHandle( SQL_HANDLE_ENV,
			SQL_NULL_HANDLE,
			&d->hEnv);
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::open: Unable to allocate environment", d );
#endif
	setOpenError( TRUE );
	return FALSE;
    }
    r = SQLSetEnvAttr( d->hEnv,
		       SQL_ATTR_ODBC_VERSION,
		       (SQLPOINTER)SQL_OV_ODBC2,
		       SQL_IS_UINTEGER );
    r = SQLAllocHandle( SQL_HANDLE_DBC,
		        d->hEnv,
		        &d->hDbc);
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::open: Unable to allocate connection", d );
#endif
	setOpenError( TRUE );
	return FALSE;
    }

    if ( !d->setConnectionOptions( connOpts ) )
	return FALSE;

    // Create the connection string
    QString connQStr;
    // support the "DRIVER={SQL SERVER};SERVER=blah" syntax
    if ( db.contains(".dsn") )
	connQStr = "FILEDSN=" + db;
    else if ( db.contains( "DRIVER" ) || db.contains( "SERVER" ) )
	connQStr = db;
    else
	connQStr = "DSN=" + db;
    connQStr += ";UID=" + user + ";PWD=" + password;
    SQLSMALLINT cb;
    SQLTCHAR connOut[1024];
    r = SQLDriverConnect( d->hDbc,
			  NULL,
#ifdef UNICODE
			  (SQLWCHAR*)connQStr.unicode(),
#else
			  (SQLCHAR*)connQStr.latin1(),
#endif
			  (SQLSMALLINT)connQStr.length(),
			  connOut,
			  1024,
			  &cb,
			  SQL_DRIVER_NOPROMPT );
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	setLastError( qMakeError( "Unable to connect", QSqlError::Connection, d ) );
	setOpenError( TRUE );
	return FALSE;
    }

    if ( !d->checkDriver() ) {
	setLastError( qMakeError( "Unable to connect - Driver doesn't support all needed functionality", QSqlError::Connection, d ) );
	setOpenError( TRUE );
	return FALSE;
    }

    d->checkUnicode();
    d->checkSchemaUsage();

    setOpen( TRUE );
    setOpenError( FALSE );
    return TRUE;
}

void QODBCDriver::close()
{
    cleanup();
    setOpen( FALSE );
    setOpenError( FALSE );
}

void QODBCDriver::cleanup()
{
    SQLRETURN r;
    if ( !d )
	return;

    if( d->hDbc ) {
	// Open statements/descriptors handles are automatically cleaned up by SQLDisconnect
	if ( isOpen() ) {
	    r = SQLDisconnect( d->hDbc );
#ifdef QT_CHECK_RANGE
	    if ( r != SQL_SUCCESS )
		qSqlWarning( "QODBCDriver::disconnect: Unable to disconnect datasource", d );
#endif
	}

	r = SQLFreeHandle( SQL_HANDLE_DBC, d->hDbc );
#ifdef QT_CHECK_RANGE
	if ( r != SQL_SUCCESS )
	    qSqlWarning( "QODBCDriver::cleanup: Unable to free connection handle", d );
#endif
	d->hDbc = 0;
    }

    if ( d->hEnv ) {
	r = SQLFreeHandle( SQL_HANDLE_ENV, d->hEnv );
#ifdef QT_CHECK_RANGE
	if ( r != SQL_SUCCESS )
	    qSqlWarning( "QODBCDriver::cleanup: Unable to free environment handle", d );
#endif
	d->hEnv = 0;
    }
}

// checks whether the server can return char, varchar and longvarchar
// as two byte unicode characters
void QODBCPrivate::checkUnicode()
{
#if defined(Q_WS_WIN)
    if ( !qt_winunicode ) {
	unicode = FALSE;
	return;
    }
#endif
    SQLRETURN   r;
    SQLUINTEGER fFunc;

    unicode = FALSE;
    r = SQLGetInfo( hDbc,
		    SQL_CONVERT_CHAR,
		    (SQLPOINTER)&fFunc,
		    sizeof(fFunc),
		    NULL );
    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( fFunc & SQL_CVT_WCHAR ) ) {
	sql_char_type = QVariant::String;
	unicode = TRUE;
    }

    r = SQLGetInfo( hDbc,
		    SQL_CONVERT_VARCHAR,
		    (SQLPOINTER)&fFunc,
		    sizeof(fFunc),
		    NULL );
    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( fFunc & SQL_CVT_WVARCHAR ) ) {
	sql_varchar_type = QVariant::String;
	unicode = TRUE;
    }

    r = SQLGetInfo( hDbc,
		    SQL_CONVERT_LONGVARCHAR,
		    (SQLPOINTER)&fFunc,
		    sizeof(fFunc),
		    NULL );
    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( fFunc & SQL_CVT_WLONGVARCHAR ) ) {
	sql_longvarchar_type = QVariant::String;
	unicode = TRUE;
    }
}

bool QODBCPrivate::checkDriver() const
{
#ifdef ODBC_CHECK_DRIVER
    // do not query for SQL_API_SQLFETCHSCROLL because it can't be used at this time
    static const SQLUSMALLINT reqFunc[] = {
		SQL_API_SQLDESCRIBECOL, SQL_API_SQLGETDATA, SQL_API_SQLCOLUMNS,
		SQL_API_SQLGETSTMTATTR, SQL_API_SQLGETDIAGREC, SQL_API_SQLEXECDIRECT,
		SQL_API_SQLGETINFO, SQL_API_SQLTABLES, 0
    };

    // these functions are optional
    static const SQLUSMALLINT optFunc[] = {
	SQL_API_SQLNUMRESULTCOLS, SQL_API_SQLROWCOUNT, 0
    };

    SQLRETURN r;
    SQLUSMALLINT sup;


    int i;
    // check the required functions
    for ( i = 0; reqFunc[ i ] != 0; ++i ) {

	r = SQLGetFunctions( hDbc, reqFunc[ i ], &sup );

#ifdef QT_CHECK_RANGE
        if ( r != SQL_SUCCESS ) {
	    qSqlWarning( "QODBCDriver::checkDriver: Cannot get list of supported functions", this );
	    return FALSE;
	}
#endif
	if ( sup == SQL_FALSE ) {
#ifdef QT_CHECK_RANGE
	    qWarning ( "QODBCDriver::open: Warning - Driver doesn't support all needed functionality (%d). "
		       "Please look at the Qt SQL Module Driver documentation for more information.", reqFunc[ i ] );
#endif
	    return FALSE;
	}
    }

    // these functions are optional and just generate a warning
    for ( i = 0; optFunc[ i ] != 0; ++i ) {

	r = SQLGetFunctions( hDbc, optFunc[ i ], &sup );

#ifdef QT_CHECK_RANGE
        if ( r != SQL_SUCCESS ) {
	    qSqlWarning( "QODBCDriver::checkDriver: Cannot get list of supported functions", this );
	    return FALSE;
	}
#endif
	if ( sup == SQL_FALSE ) {
#ifdef QT_CHECK_RANGE
	    qWarning( "QODBCDriver::checkDriver: Warning - Driver doesn't support some non-critical functions (%d)", optFunc[ i ] );
#endif
	    return TRUE;
	}
    }
#endif //ODBC_CHECK_DRIVER

    return TRUE;
}

void QODBCPrivate::checkSchemaUsage()
{
    SQLRETURN   r;
    SQLUINTEGER val;

    r = SQLGetInfo(hDbc,
		   SQL_SCHEMA_USAGE,
		   (SQLPOINTER) &val,
		   sizeof(val),
		   NULL);
    if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO)
	useSchema = (val != 0);
}

QSqlQuery QODBCDriver::createQuery() const
{
    return QSqlQuery( new QODBCResult( this, d ) );
}

bool QODBCDriver::beginTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning(" QODBCDriver::beginTransaction: Database not open" );
#endif
	return FALSE;
    }
    SQLUINTEGER ac(SQL_AUTOCOMMIT_OFF);
    SQLRETURN r  = SQLSetConnectAttr( d->hDbc,
				      SQL_ATTR_AUTOCOMMIT,
				      (SQLPOINTER)ac,
				      sizeof(ac) );
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to disable autocommit", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return TRUE;
}

bool QODBCDriver::commitTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning(" QODBCDriver::commitTransaction: Database not open" );
#endif
	return FALSE;
    }
    SQLRETURN r = SQLEndTran( SQL_HANDLE_DBC,
			      d->hDbc,
			      SQL_COMMIT );
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError("Unable to commit transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return endTrans();
}

bool QODBCDriver::rollbackTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning(" QODBCDriver::rollbackTransaction: Database not open" );
#endif
	return FALSE;
    }
    SQLRETURN r = SQLEndTran( SQL_HANDLE_DBC,
			      d->hDbc,
			      SQL_ROLLBACK );
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to rollback transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return endTrans();
}

bool QODBCDriver::endTrans()
{
    SQLUINTEGER ac(SQL_AUTOCOMMIT_ON);
    SQLRETURN r  = SQLSetConnectAttr( d->hDbc,
				      SQL_ATTR_AUTOCOMMIT,
				      (SQLPOINTER)ac,
				      sizeof(ac));
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to enable autocommit", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return TRUE;
}

QStringList QODBCDriver::tables( const QString& typeName ) const
{
    QStringList tl;
    if ( !isOpen() )
	return tl;
    int type = typeName.toInt();
    SQLHANDLE hStmt;

    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::tables: Unable to allocate handle", d );
#endif
	return tl;
    }
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );
    QString tableType;
    if ( typeName.isEmpty() || ((type & (int)QSql::Tables) == (int)QSql::Tables) )
	tableType += "TABLE,";
    if ( (type & (int)QSql::Views) == (int)QSql::Views )
	tableType += "VIEW,";
    if ( (type & (int)QSql::SystemTables) == (int)QSql::SystemTables )
	tableType += "SYSTEM TABLE,";
    if ( tableType.isEmpty() )
	return tl;
    tableType.truncate( tableType.length() - 1 );

    r = SQLTables( hStmt,
		   NULL,
		   0,
		   NULL,
		   0,
		   NULL,
		   0,
#ifdef UNICODE
		   (SQLWCHAR*)tableType.unicode(),
#else
		   (SQLCHAR*)tableType.latin1(),
#endif
		   tableType.length() /* characters, not bytes */ );

#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::tables Unable to execute table list", d );
#endif
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0);
    while ( r == SQL_SUCCESS ) {
	bool isNull;
	QString fieldVal = qGetStringData( hStmt, 2, -1, isNull, d->unicode );
	tl.append( fieldVal );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );
    return tl;
}

QSqlIndex QODBCDriver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex index( tablename );
    if ( !isOpen() )
	return index;
    bool usingSpecialColumns = FALSE;
    QSqlRecord rec = record( tablename );

    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::primaryIndex: Unable to list primary key", d );
#endif
	return index;
    }
    QString catalog, schema, table;
    d->splitTableQualifier( tablename, catalog, schema, table );
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );
    r = SQLPrimaryKeys( hStmt,
#ifdef UNICODE
			catalog.length() == 0 ? NULL : (SQLWCHAR*)catalog.unicode(),
#else
			catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.latin1(),
#endif
			catalog.length(),
#ifdef UNICODE
			schema.length() == 0 ? NULL : (SQLWCHAR*)schema.unicode(),
#else
			schema.length() == 0 ? NULL : (SQLCHAR*)schema.latin1(),
#endif
			schema.length(),
#ifdef UNICODE
			(SQLWCHAR*)table.unicode(),
#else
			(SQLCHAR*)table.latin1(),
#endif
			table.length() /* in characters, not in bytes */);

    // if the SQLPrimaryKeys() call does not succeed (e.g the driver
    // does not support it) - try an alternative method to get hold of
    // the primary index (e.g MS Access and FoxPro)
    if ( r != SQL_SUCCESS ) {
	    r = SQLSpecialColumns( hStmt,
				   SQL_BEST_ROWID,
#ifdef UNICODE
				   catalog.length() == 0 ? NULL : (SQLWCHAR*)catalog.unicode(),
#else
				   catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.latin1(),
#endif
				   catalog.length(),
#ifdef UNICODE
				   schema.length() == 0 ? NULL : (SQLWCHAR*)schema.unicode(),
#else
				   schema.length() == 0 ? NULL : (SQLCHAR*)schema.latin1(),
#endif
				   schema.length(),
#ifdef UNICODE
				   (SQLWCHAR*)table.unicode(),
#else
				   (SQLCHAR*)table.latin1(),
#endif

				   table.length(),
				   SQL_SCOPE_CURROW,
				   SQL_NULLABLE );

	    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
		qSqlWarning( "QODBCDriver::primaryIndex: Unable to execute primary key list", d );
#endif
	    } else {
		usingSpecialColumns = TRUE;
	    }
    }
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0 );
    bool isNull;
    int fakeId = 0;
    QString cName, idxName;
    // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
    while ( r == SQL_SUCCESS ) {
	if ( usingSpecialColumns ) {
	    cName = qGetStringData( hStmt, 1, -1, isNull, d->unicode ); // column name
	    idxName = QString::number( fakeId++ ); // invent a fake index name
	} else {
	    cName = qGetStringData( hStmt, 3, -1, isNull, d->unicode ); // column name
	    idxName = qGetStringData( hStmt, 5, -1, isNull, d->unicode ); // pk index name
	}
        QSqlField *fld = rec.field(cName);
        if (fld)
	    index.append(*fld);
	index.setName( idxName );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0 );
    }
    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );
    return index;
}

QSqlRecord QODBCDriver::record( const QString& tablename ) const
{
    return recordInfo( tablename ).toRecord();
}

QSqlRecord QODBCDriver::record( const QSqlQuery& query ) const
{
    return recordInfo( query ).toRecord();
}

QSqlRecordInfo QODBCDriver::recordInfo( const QString& tablename ) const
{
    QSqlRecordInfo fil;
    if ( !isOpen() )
	return fil;

    SQLHANDLE hStmt;
    QString catalog, schema, table;
    d->splitTableQualifier( tablename, catalog, schema, table );
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::record: Unable to allocate handle", d );
#endif
	return fil;
    }
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );
    r =  SQLColumns( hStmt,
#ifdef UNICODE
		     catalog.length() == 0 ? NULL : (SQLWCHAR*)catalog.unicode(),
#else
		     catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.latin1(),
#endif
		     catalog.length(),
#ifdef UNICODE
		     schema.length() == 0 ? NULL : (SQLWCHAR*)schema.unicode(),
#else
		     schema.length() == 0 ? NULL : (SQLCHAR*)schema.latin1(),
#endif
		     schema.length(),
#ifdef UNICODE
		     (SQLWCHAR*)table.unicode(),
#else
		     (SQLCHAR*)table.latin1(),
#endif
		     table.length(),
		     NULL,
		     0 );
#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::record: Unable to execute column list", d );
#endif
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0);
    // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
    while ( r == SQL_SUCCESS ) {

	fil.append( qMakeFieldInfo( hStmt, d ) );

	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle " + QString::number(r), d );

    return fil;
}

QSqlRecordInfo QODBCDriver::recordInfo( const QSqlQuery& query ) const
{
    QSqlRecordInfo fil;
    if ( !isOpen() )
	return fil;
    if ( query.isActive() && query.driver() == this ) {
	QODBCResult* result = (QODBCResult*)query.result();
	fil = result->d->rInf;
    }
    return fil;
}

SQLHANDLE QODBCDriver::environment()
{
    return d->hEnv;
}

SQLHANDLE QODBCDriver::connection()
{
    return d->hDbc;
}

QString QODBCDriver::formatValue( const QSqlField* field,
				  bool trimStrings ) const
{
    QString r;
    if ( field->isNull() ) {
	r = nullText();
    } else if ( field->type() == QVariant::DateTime ) {
	// Use an escape sequence for the datetime fields
	if ( field->value().toDateTime().isValid() ){
	    QDate dt = field->value().toDateTime().date();
	    QTime tm = field->value().toDateTime().time();
	    // Dateformat has to be "yyyy-MM-dd hh:mm:ss", with leading zeroes if month or day < 10
	    r = "{ ts '" +
		QString::number(dt.year()) + "-" +
		QString::number(dt.month()).rightJustify( 2, '0', TRUE ) + "-" +
		QString::number(dt.day()).rightJustify( 2, '0', TRUE ) + " " +
		tm.toString() +
		"' }";
	} else
	    r = nullText();
    } else if ( field->type() == QVariant::ByteArray ) {
	QByteArray ba = field->value().toByteArray();
	QString res;
	static const char hexchars[] = "0123456789abcdef";
	for ( uint i = 0; i < ba.size(); ++i ) {
	    uchar s = (uchar) ba[(int)i];
	    res += hexchars[s >> 4];
	    res += hexchars[s & 0x0f];
	}
	r = "0x" + res;
    } else {
	r = QSqlDriver::formatValue( field, trimStrings );
    }
    return r;
}
