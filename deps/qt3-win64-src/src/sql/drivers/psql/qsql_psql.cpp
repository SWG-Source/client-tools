/****************************************************************************
**
** Implementation of PostgreSQL driver classes
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

#include "qsql_psql.h"
#include <private/qsqlextension_p.h>

#include <math.h>

#include <qpointarray.h>
#include <qsqlrecord.h>
#include <qregexp.h>
#include <qdatetime.h>
// PostgreSQL header <utils/elog.h> included by <postgres.h> redefines DEBUG.
#if defined(DEBUG)
# undef DEBUG
#endif
#include <postgres.h>
#include <libpq/libpq-fs.h>
// PostgreSQL header <catalog/pg_type.h> redefines errno erroneously.
#if defined(errno)
# undef errno
#endif
#define errno qt_psql_errno
#include <catalog/pg_type.h>
#undef errno

QPtrDict<QSqlDriverExtension> *qSqlDriverExtDict();
QPtrDict<QSqlOpenExtension> *qSqlOpenExtDict();

class QPSQLPrivate
{
public:
  QPSQLPrivate():connection(0), result(0), isUtf8(FALSE) {}
    PGconn	*connection;
    PGresult	*result;
    bool        isUtf8;
};

class QPSQLDriverExtension : public QSqlDriverExtension
{
public:
    QPSQLDriverExtension( QPSQLDriver *dri )
	: QSqlDriverExtension(), driver(dri) { }
    ~QPSQLDriverExtension() {}

    bool isOpen() const;
private:
    QPSQLDriver *driver;
};

bool QPSQLDriverExtension::isOpen() const
{
    return PQstatus( driver->connection() ) == CONNECTION_OK;
}

class QPSQLOpenExtension : public QSqlOpenExtension
{
public:
    QPSQLOpenExtension( QPSQLDriver *dri )
	: QSqlOpenExtension(), driver(dri) { }
    ~QPSQLOpenExtension() {}

    bool open( const QString& db,
	       const QString& user,
	       const QString& password,
	       const QString& host,
	       int port,
	       const QString& connOpts );
private:
    QPSQLDriver *driver;
};

bool QPSQLOpenExtension::open( const QString& db,
			       const QString& user,
			       const QString& password,
			       const QString& host,
			       int port,
			       const QString& connOpts )
{
    return driver->open( db, user, password, host, port, connOpts );
}

static QSqlError qMakeError( const QString& err, int type, const QPSQLPrivate* p )
{
    const char *s = PQerrorMessage(p->connection);
    QString msg = p->isUtf8 ? QString::fromUtf8(s) : QString::fromLocal8Bit(s);
    return QSqlError("QPSQL: " + err, msg, type);
}

static QVariant::Type qDecodePSQLType( int t )
{
    QVariant::Type type = QVariant::Invalid;
    switch ( t ) {
    case BOOLOID	:
	type = QVariant::Bool;
	break;
    case INT8OID	:
	type = QVariant::LongLong;
	break;
    case INT2OID	:
	//    case INT2VECTOROID  : // 7.x
    case INT4OID        :
	type = QVariant::Int;
	break;
    case NUMERICOID     :
    case FLOAT4OID      :
    case FLOAT8OID      :
	type = QVariant::Double;
	break;
    case ABSTIMEOID     :
    case RELTIMEOID     :
    case DATEOID	:
	type = QVariant::Date;
	break;
    case TIMEOID	:
#ifdef TIMETZOID // 7.x
	case TIMETZOID  :
#endif
	type = QVariant::Time;
	break;
    case TIMESTAMPOID   :
#ifdef DATETIMEOID
    // Postgres 6.x datetime workaround.
    // DATETIMEOID == TIMESTAMPOID (only the names have changed)
    case DATETIMEOID    :
#endif
#ifdef TIMESTAMPTZOID
    // Postgres 7.2 workaround
    // TIMESTAMPTZOID == TIMESTAMPOID == DATETIMEOID
    case TIMESTAMPTZOID :
#endif
	type = QVariant::DateTime;
	break;
	//    case ZPBITOID	: // 7.x
	//    case VARBITOID	: // 7.x
    case OIDOID         :
    case BYTEAOID       :
	type = QVariant::ByteArray;
	break;
    case REGPROCOID     :
    case TIDOID         :
    case XIDOID         :
    case CIDOID         :
	//    case OIDVECTOROID   : // 7.x
    case UNKNOWNOID     :
	//    case TINTERVALOID   : // 7.x
	type = QVariant::Invalid;
	break;
    default:
    case CHAROID	:
    case BPCHAROID	:
	//    case LZTEXTOID	: // 7.x
    case VARCHAROID	:
    case TEXTOID	:
    case NAMEOID	:
    case CASHOID        :
    case INETOID        :
    case CIDROID        :
    case CIRCLEOID      :
	type = QVariant::String;
	break;
    }
    return type;
}

QPSQLResult::QPSQLResult( const QPSQLDriver* db, const QPSQLPrivate* p )
: QSqlResult( db ),
  currentSize( 0 )
{
    d =   new QPSQLPrivate();
    (*d) = (*p);
}

QPSQLResult::~QPSQLResult()
{
    cleanup();
    delete d;
}

PGresult* QPSQLResult::result()
{
    return d->result;
}

void QPSQLResult::cleanup()
{
    if ( d->result )
	PQclear( d->result );
    d->result = 0;
    setAt( -1 );
    currentSize = 0;
    setActive( FALSE );
}

bool QPSQLResult::fetch( int i )
{
    if ( !isActive() )
	return FALSE;
    if ( i < 0 )
	return FALSE;
    if ( i >= currentSize )
	return FALSE;
    if ( at() == i )
	return TRUE;
    setAt( i );
    return TRUE;
}

bool QPSQLResult::fetchFirst()
{
    return fetch( 0 );
}

bool QPSQLResult::fetchLast()
{
    return fetch( PQntuples( d->result ) - 1 );
}

// some Postgres conversions
static QPoint pointFromString( const QString& s)
{
    // format '(x,y)'
    int pivot = s.find( ',' );
    if ( pivot != -1 ) {
	int x = s.mid( 1, pivot-1 ).toInt();
	int y = s.mid( pivot+1, s.length()-pivot-2 ).toInt();
	return QPoint( x, y ) ;
    } else
	return QPoint();
}

QVariant QPSQLResult::data( int i )
{
    if ( i >= PQnfields( d->result ) ) {
	qWarning( "QPSQLResult::data: column %d out of range", i );
	return QVariant();
    }
    int ptype = PQftype( d->result, i );
    QVariant::Type type = qDecodePSQLType( ptype );
    const QString val = ( d->isUtf8 && ptype != BYTEAOID ) ?
			QString::fromUtf8( PQgetvalue( d->result, at(), i ) ) :
			QString::fromLocal8Bit( PQgetvalue( d->result, at(), i ) );
    if ( PQgetisnull( d->result, at(), i ) ) {
	QVariant v;
	v.cast( type );
	return v;
    }
    switch ( type ) {
    case QVariant::Bool:
	{
	    QVariant b ( (bool)(val == "t"), 0 );
	    return ( b );
	}
    case QVariant::String:
	return QVariant( val );
    case QVariant::LongLong:
	if ( val[0] == '-' )
	    return QVariant( val.toLongLong() );
	else
	    return QVariant( val.toULongLong() );
    case QVariant::Int:
	return QVariant( val.toInt() );
    case QVariant::Double:
	if ( ptype == NUMERICOID )
	    return QVariant( val );
	return QVariant( val.toDouble() );
    case QVariant::Date:
	if ( val.isEmpty() ) {
	    return QVariant( QDate() );
	} else {
	    return QVariant( QDate::fromString( val, Qt::ISODate ) );
	}
    case QVariant::Time:
	if ( val.isEmpty() )
	    return QVariant( QTime() );
	if ( val.at( val.length() - 3 ) == '+' )
	    // strip the timezone
	    return QVariant( QTime::fromString( val.left( val.length() - 3 ), Qt::ISODate ) );
	return QVariant( QTime::fromString( val, Qt::ISODate ) );
    case QVariant::DateTime: {
	if ( val.length() < 10 )
	    return QVariant( QDateTime() );
	// remove the timezone
	QString dtval = val;
	if ( dtval.at( dtval.length() - 3 ) == '+' )
	    dtval.truncate( dtval.length() - 3 );
	// milliseconds are sometimes returned with 2 digits only
	if ( dtval.at( dtval.length() - 3 ).isPunct() )
	    dtval += '0';
	if ( dtval.isEmpty() )
	    return QVariant( QDateTime() );
	else
	    return QVariant( QDateTime::fromString( dtval, Qt::ISODate ) );
    }
    case QVariant::Point:
	return QVariant( pointFromString( val ) );
    case QVariant::Rect: // format '(x,y),(x',y')'
	{
	    int pivot = val.find( "),(" );
	    if ( pivot != -1 )
		return QVariant( QRect( pointFromString( val.mid(pivot+2,val.length()) ), pointFromString( val.mid(0,pivot+1) ) ) );
	    return QVariant( QRect() );
	}
    case QVariant::PointArray: // format '((x,y),(x1,y1),...,(xn,yn))'
	{
	    QRegExp pointPattern("\\([0-9-]*,[0-9-]*\\)");
	    int points = val.contains( pointPattern );
	    QPointArray parray( points );
	    int idx = 1;
	    for ( int i = 0; i < points; i++ ){
		int start = val.find( pointPattern, idx );
		int end = -1;
		if ( start != -1 ) {
		    end = val.find( ')', start+1 );
		    if ( end != -1 ) {
			parray.setPoint( i, pointFromString( val.mid(idx, end-idx+1) ) );
		    }
		    else
			parray.setPoint( i, QPoint() );
		} else {
		    parray.setPoint( i, QPoint() );
		    break;
		}
		idx = end+2;
	    }
	    return QVariant( parray );
	}
    case QVariant::ByteArray: {
	if ( ptype == BYTEAOID ) {
	    uint i = 0;
	    int index = 0;
	    uint len = val.length();
	    static const QChar backslash( '\\' );
	    QByteArray ba( (int)len );
	    while ( i < len ) {
		if ( val.at( i ) == backslash ) {
		    if ( val.at( i + 1 ).isDigit() ) {
			ba[ index++ ] = (char)(val.mid( i + 1, 3 ).toInt( 0, 8 ));
			i += 4;
		    } else {
			ba[ index++ ] = val.at( i + 1 );
			i += 2;
		    }
		} else {
		    ba[ index++ ] = val.at( i++ ).unicode();
		}
	    }
	    ba.resize( index );
	    return QVariant( ba );
	}

	QByteArray ba;
	((QSqlDriver*)driver())->beginTransaction();
	Oid oid = val.toInt();
	int fd = lo_open( d->connection, oid, INV_READ );
#ifdef QT_CHECK_RANGE
	if ( fd < 0) {
	    qWarning( "QPSQLResult::data: unable to open large object for read" );
	    ((QSqlDriver*)driver())->commitTransaction();
	    return QVariant( ba );
	}
#endif
	int size = 0;
	int retval = lo_lseek( d->connection, fd, 0L, SEEK_END );
	if ( retval >= 0 ) {
	    size = lo_tell( d->connection, fd );
	    lo_lseek( d->connection, fd, 0L, SEEK_SET );
	}
	if ( size == 0 ) {
	    lo_close( d->connection, fd );
	    ((QSqlDriver*)driver())->commitTransaction();
	    return QVariant( ba );
	}
	char * buf = new char[ size ];

#ifdef Q_OS_WIN32
	// ### For some reason lo_read() fails if we try to read more than
	// ### 32760 bytes
	char * p = buf;
	int nread = 0;

	while( size < nread ){
		retval = lo_read( d->connection, fd, p, 32760 );
		nread += retval;
		p += retval;
	}
#else
	retval = lo_read( d->connection, fd, buf, size );
#endif

	if (retval < 0) {
	    qWarning( "QPSQLResult::data: unable to read large object" );
	} else {
	    ba.duplicate( buf, size );
	}
	delete [] buf;
	lo_close( d->connection, fd );
	((QSqlDriver*)driver())->commitTransaction();
	return QVariant( ba );
    }
    default:
    case QVariant::Invalid:
#ifdef QT_CHECK_RANGE
	qWarning("QPSQLResult::data: unknown data type");
#endif
	;
    }
    return QVariant();
}

bool QPSQLResult::isNull( int field )
{
    PQgetvalue( d->result, at(), field );
    return PQgetisnull( d->result, at(), field );
}

bool QPSQLResult::reset ( const QString& query )
{
    cleanup();
    if ( !driver() )
	return FALSE;
    if ( !driver()->isOpen() || driver()->isOpenError() )
	return FALSE;
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    if ( d->result )
	PQclear( d->result );
    if ( d->isUtf8 ) {
	d->result = PQexec( d->connection, query.utf8().data() );
    } else {
	d->result = PQexec( d->connection, query.local8Bit().data() );
    }
    int status =  PQresultStatus( d->result );
    if ( status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK ) {
	if ( status == PGRES_TUPLES_OK ) {
	    setSelect( TRUE );
	    currentSize = PQntuples( d->result );
	} else {
	    setSelect( FALSE );
	    currentSize = -1;
	}
	setActive( TRUE );
	return TRUE;
    }
    setLastError( qMakeError( "Unable to create query", QSqlError::Statement, d ) );
    return FALSE;
}

int QPSQLResult::size()
{
    return currentSize;
}

int QPSQLResult::numRowsAffected()
{
    return QString( PQcmdTuples( d->result ) ).toInt();
}

///////////////////////////////////////////////////////////////////

static bool setEncodingUtf8( PGconn* connection )
{
    PGresult* result = PQexec( connection, "SET CLIENT_ENCODING TO 'UNICODE'" );
    int status = PQresultStatus( result );
    PQclear( result );
    return status == PGRES_COMMAND_OK;
}

static void setDatestyle( PGconn* connection )
{
    PGresult* result = PQexec( connection, "SET DATESTYLE TO 'ISO'" );
#ifdef QT_CHECK_RANGE
    int status =  PQresultStatus( result );
    if ( status != PGRES_COMMAND_OK )
        qWarning( "%s", PQerrorMessage( connection ) );
#endif
    PQclear( result );
}

static QPSQLDriver::Protocol getPSQLVersion( PGconn* connection )
{
    PGresult* result = PQexec( connection, "select version()" );
    int status =  PQresultStatus( result );
    if ( status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK ) {
	QString val( PQgetvalue( result, 0, 0 ) );
	PQclear( result );
	QRegExp rx( "(\\d+)\\.(\\d+)" );
	rx.setMinimal ( TRUE ); // enforce non-greedy RegExp
        if ( rx.search( val ) != -1 ) {
	    int vMaj = rx.cap( 1 ).toInt();
	    int vMin = rx.cap( 2 ).toInt();
	    if ( vMaj < 6 ) {
#ifdef QT_CHECK_RANGE
		qWarning( "This version of PostgreSQL is not supported and may not work." );
#endif
		return QPSQLDriver::Version6;
	    }
	    if ( vMaj == 6 ) {
		return QPSQLDriver::Version6;
	    } else if ( vMaj == 7 ) {
		if ( vMin < 1 )
		    return QPSQLDriver::Version7;
		else if ( vMin < 3 )
		    return QPSQLDriver::Version71;
	    }
	    return QPSQLDriver::Version73;
	}
    } else {
#ifdef QT_CHECK_RANGE
	qWarning( "This version of PostgreSQL is not supported and may not work." );
#endif
    }

    return QPSQLDriver::Version6;
}

QPSQLDriver::QPSQLDriver( QObject * parent, const char * name )
    : QSqlDriver(parent,name ? name : "QPSQL"), pro( QPSQLDriver::Version6 )
{
    init();
}

QPSQLDriver::QPSQLDriver( PGconn * conn, QObject * parent, const char * name )
    : QSqlDriver(parent,name ? name : "QPSQL"), pro( QPSQLDriver::Version6 )
{
    init();
    d->connection = conn;
    if ( conn ) {
	pro = getPSQLVersion( d->connection );
	setOpen( TRUE );
	setOpenError( FALSE );
    }
}

void QPSQLDriver::init()
{
    qSqlDriverExtDict()->insert( this, new QPSQLDriverExtension(this) );
    qSqlOpenExtDict()->insert( this, new QPSQLOpenExtension(this) );

    d = new QPSQLPrivate();
}

QPSQLDriver::~QPSQLDriver()
{
    if ( d->connection )
	PQfinish( d->connection );
    delete d;
    if ( !qSqlDriverExtDict()->isEmpty() ) {
	QSqlDriverExtension *ext = qSqlDriverExtDict()->take( this );
	delete ext;
    }
    if ( !qSqlOpenExtDict()->isEmpty() ) {
	QSqlOpenExtension *ext = qSqlOpenExtDict()->take( this );
	delete ext;
    }
}

PGconn* QPSQLDriver::connection()
{
    return d->connection;
}


bool QPSQLDriver::hasFeature( DriverFeature f ) const
{
    switch ( f ) {
    case Transactions:
	return TRUE;
    case QuerySize:
	return TRUE;
    case BLOB:
	return pro >= QPSQLDriver::Version71;
    case Unicode:
	return d->isUtf8;
    default:
	return FALSE;
    }
}

bool QPSQLDriver::open( const QString&,
			const QString&,
			const QString&,
			const QString&,
			int )
{
    qWarning("QPSQLDriver::open(): This version of open() is no longer supported." );
    return FALSE;
}

bool QPSQLDriver::open( const QString & db,
			const QString & user,
			const QString & password,
			const QString & host,
			int port,
			const QString& connOpts )
{
    if ( isOpen() )
	close();
    QString connectString;
    if ( host.length() )
	connectString.append( "host=" ).append( host );
    if ( db.length() )
	connectString.append( " dbname=" ).append( db );
    if ( user.length() )
	connectString.append( " user=" ).append( user );
    if ( password.length() )
	connectString.append( " password=" ).append( password );
    if ( port > -1 )
	connectString.append( " port=" ).append( QString::number( port ) );

    // add any connect options - the server will handle error detection
    if ( !connOpts.isEmpty() )
	connectString += " " + QStringList::split( ';', connOpts ).join( " " );

    d->connection = PQconnectdb( connectString.local8Bit().data() );
    if ( PQstatus( d->connection ) == CONNECTION_BAD ) {
	setLastError( qMakeError("Unable to connect", QSqlError::Connection, d ) );
	setOpenError( TRUE );
	return FALSE;
    }

    pro = getPSQLVersion( d->connection );
    d->isUtf8 = setEncodingUtf8( d->connection );
    setDatestyle( d->connection );

    setOpen( TRUE );
    setOpenError( FALSE );
    return TRUE;
}

void QPSQLDriver::close()
{
    if ( isOpen() ) {
        if (d->connection)
	    PQfinish( d->connection );
	d->connection = 0;
	setOpen( FALSE );
	setOpenError( FALSE );
    }
}

QSqlQuery QPSQLDriver::createQuery() const
{
    return QSqlQuery( new QPSQLResult( this, d ) );
}

bool QPSQLDriver::beginTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QPSQLDriver::beginTransaction: Database not open" );
#endif
	return FALSE;
    }
    PGresult* res = PQexec( d->connection, "BEGIN" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	PQclear( res );
	setLastError( qMakeError( "Could not begin transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

bool QPSQLDriver::commitTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QPSQLDriver::commitTransaction: Database not open" );
#endif
	return FALSE;
    }
    PGresult* res = PQexec( d->connection, "COMMIT" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	PQclear( res );
	setLastError( qMakeError( "Could not commit transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

bool QPSQLDriver::rollbackTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QPSQLDriver::rollbackTransaction: Database not open" );
#endif
	return FALSE;
    }
    PGresult* res = PQexec( d->connection, "ROLLBACK" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	setLastError( qMakeError( "Could not rollback transaction", QSqlError::Transaction, d ) );
	PQclear( res );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

QStringList QPSQLDriver::tables( const QString& typeName ) const
{
    QStringList tl;
    if ( !isOpen() )
	return tl;
    int type = typeName.toInt();
    QSqlQuery t = createQuery();
    t.setForwardOnly( TRUE );

    if ( typeName.isEmpty() || ((type & (int)QSql::Tables) == (int)QSql::Tables) ) {

        QString query("select relname from pg_class where (relkind = 'r') "
                      "and (relname !~ '^Inv') "
                      "and (relname !~ '^pg_') ");
        if (pro >= QPSQLDriver::Version73)
            query.append("and (relnamespace not in "
                         "(select oid from pg_namespace where nspname = 'information_schema')) "
                         "and pg_table_is_visible(pg_class.oid) ");
        t.exec(query);
	while ( t.next() )
	    tl.append( t.value(0).toString() );
    }
    if ( (type & (int)QSql::Views) == (int)QSql::Views ) {
        QString query("select relname from pg_class where ( relkind = 'v' ) "
                "and ( relname !~ '^Inv' ) "
                "and ( relname !~ '^pg_' ) ");
        if (pro >= QPSQLDriver::Version73)
            query.append("and (relnamespace not in "
                         "(select oid from pg_namespace where nspname = 'information_schema')) "
                         "and pg_table_is_visible(pg_class.oid) ");
        t.exec(query);
	while ( t.next() )
	    tl.append( t.value(0).toString() );
    }
    if ( (type & (int)QSql::SystemTables) == (int)QSql::SystemTables ) {
        QString query( "select relname from pg_class where ( relkind = 'r' ) "
		"and ( relname like 'pg_%' ) " );
        if (pro >= QPSQLDriver::Version73)
		query.append( "and pg_table_is_visible(pg_class.oid) " );
        t.exec(query);
	while ( t.next() )
	    tl.append( t.value(0).toString() );
    }

    return tl;
}

QSqlIndex QPSQLDriver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex idx( tablename );
    if ( !isOpen() )
	return idx;
    QSqlQuery i = createQuery();
    QString stmt;

    switch( pro ) {
    case QPSQLDriver::Version6:
	stmt = "select pg_att1.attname, int(pg_att1.atttypid), pg_att2.attnum, pg_cl.relname "
		"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
		"where lower(pg_cl.relname) = '%1_pkey' ";
	break;
    case QPSQLDriver::Version7:
    case QPSQLDriver::Version71:
	stmt = "select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
		"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
		"where lower(pg_cl.relname) = '%1_pkey' ";
	break;
    case QPSQLDriver::Version73:
	stmt = "select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
		"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
		"where lower(pg_cl.relname) = '%1_pkey' "
		"and pg_table_is_visible(pg_cl.oid) "
		"and pg_att1.attisdropped = false ";
	break;
    }
    stmt += "and pg_cl.oid = pg_ind.indexrelid "
	    "and pg_att2.attrelid = pg_ind.indexrelid "
	    "and pg_att1.attrelid = pg_ind.indrelid "
	    "and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
	    "order by pg_att2.attnum";

    i.exec( stmt.arg( tablename.lower() ) );
    while ( i.isActive() && i.next() ) {
	QSqlField f( i.value(0).toString(), qDecodePSQLType( i.value(1).toInt() ) );
	idx.append( f );
	idx.setName( i.value(2).toString() );
    }
    return idx;
}

QSqlRecord QPSQLDriver::record( const QString& tablename ) const
{
    QSqlRecord fil;
    if ( !isOpen() )
	return fil;
    QString stmt;
    switch( pro ) {
    case QPSQLDriver::Version6:
	stmt = "select pg_attribute.attname, int(pg_attribute.atttypid) "
			"from pg_class, pg_attribute "
			"where lower(pg_class.relname) = '%1' "
			"and pg_attribute.attnum > 0 "
			"and pg_attribute.attrelid = pg_class.oid ";
	break;
    case QPSQLDriver::Version7:
    case QPSQLDriver::Version71:
	stmt = "select pg_attribute.attname, pg_attribute.atttypid::int "
			"from pg_class, pg_attribute "
			"where lower(pg_class.relname) = '%1' "
			"and pg_attribute.attnum > 0 "
			"and pg_attribute.attrelid = pg_class.oid ";
	break;
    case QPSQLDriver::Version73:
	stmt = "select pg_attribute.attname, pg_attribute.atttypid::int "
			"from pg_class, pg_attribute "
			"where lower(pg_class.relname) = '%1' "
			"and pg_table_is_visible(pg_class.oid) "
			"and pg_attribute.attnum > 0 "
			"and pg_attribute.attisdropped = false "
			"and pg_attribute.attrelid = pg_class.oid ";
	break;
    }

    QSqlQuery fi = createQuery();
    fi.exec( stmt.arg( tablename.lower() ) );
    while ( fi.next() ) {
	QSqlField f( fi.value(0).toString(), qDecodePSQLType( fi.value(1).toInt() ) );
	fil.append( f );
    }
    return fil;
}

QSqlRecord QPSQLDriver::record( const QSqlQuery& query ) const
{
    QSqlRecord fil;
    if ( !isOpen() )
	return fil;
    if ( query.isActive() && query.driver() == this ) {
	QPSQLResult* result = (QPSQLResult*)query.result();
	int count = PQnfields( result->d->result );
	for ( int i = 0; i < count; ++i ) {
	    QString name = PQfname( result->d->result, i );
	    QVariant::Type type = qDecodePSQLType( PQftype( result->d->result, i ) );
	    QSqlField rf( name, type );
	    fil.append( rf );
	}
    }
    return fil;
}

QSqlRecordInfo QPSQLDriver::recordInfo( const QString& tablename ) const
{
    QSqlRecordInfo info;
    if ( !isOpen() )
	return info;

    QString stmt;
    switch( pro ) {
    case QPSQLDriver::Version6:
	stmt = "select pg_attribute.attname, int(pg_attribute.atttypid), pg_attribute.attnotnull, "
		"pg_attribute.attlen, pg_attribute.atttypmod, int(pg_attribute.attrelid), pg_attribute.attnum "
		"from pg_class, pg_attribute "
		"where lower(pg_class.relname) = '%1' "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid ";
	break;
    case QPSQLDriver::Version7:
	stmt = "select pg_attribute.attname, pg_attribute.atttypid::int, pg_attribute.attnotnull, "
		"pg_attribute.attlen, pg_attribute.atttypmod, pg_attribute.attrelid::int, pg_attribute.attnum "
		"from pg_class, pg_attribute "
		"where lower(pg_class.relname) = '%1' "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid ";
	break;
    case QPSQLDriver::Version71:
	stmt = "select pg_attribute.attname, pg_attribute.atttypid::int, pg_attribute.attnotnull, "
		"pg_attribute.attlen, pg_attribute.atttypmod, pg_attrdef.adsrc "
		"from pg_class, pg_attribute "
		"left join pg_attrdef on (pg_attrdef.adrelid = pg_attribute.attrelid and pg_attrdef.adnum = pg_attribute.attnum) "
		"where lower(pg_class.relname) = '%1' "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid "
		"order by pg_attribute.attnum ";
	break;
    case QPSQLDriver::Version73:
	stmt = "select pg_attribute.attname, pg_attribute.atttypid::int, pg_attribute.attnotnull, "
		"pg_attribute.attlen, pg_attribute.atttypmod, pg_attrdef.adsrc "
		"from pg_class, pg_attribute "
		"left join pg_attrdef on (pg_attrdef.adrelid = pg_attribute.attrelid and pg_attrdef.adnum = pg_attribute.attnum) "
		"where lower(pg_class.relname) = '%1' "
		"and pg_table_is_visible(pg_class.oid) "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid "
		"and pg_attribute.attisdropped = false "
		"order by pg_attribute.attnum ";
	break;
    }

    QSqlQuery query = createQuery();
    query.exec( stmt.arg( tablename.lower() ) );
    if ( pro >= QPSQLDriver::Version71 ) {
	while ( query.next() ) {
	    int len = query.value( 3 ).toInt();
	    int precision = query.value( 4 ).toInt();
	    // swap length and precision if length == -1
	    if ( len == -1 && precision > -1 ) {
		len = precision - 4;
		precision = -1;
	    }
	    QString defVal = query.value( 5 ).toString();
	    if ( !defVal.isEmpty() && defVal.startsWith( "'" ) )
		defVal = defVal.mid( 1, defVal.length() - 2 );
	    info.append( QSqlFieldInfo( query.value( 0 ).toString(),
					qDecodePSQLType( query.value( 1 ).toInt() ),
					query.value( 2 ).toBool(),
					len,
					precision,
					defVal,
					query.value( 1 ).toInt() ) );
	}
    } else {
	// Postgres < 7.1 cannot handle outer joins
	while ( query.next() ) {
	    QString defVal;
	    QString stmt2 = "select pg_attrdef.adsrc from pg_attrdef where "
			    "pg_attrdef.adrelid = %1 and pg_attrdef.adnum = %2 ";
	    QSqlQuery query2 = createQuery();
	    query2.exec( stmt2.arg( query.value( 5 ).toInt() ).arg( query.value( 6 ).toInt() ) );
	    if ( query2.isActive() && query2.next() )
		defVal = query2.value( 0 ).toString();
	    if ( !defVal.isEmpty() && defVal.startsWith( "'" ) )
		defVal = defVal.mid( 1, defVal.length() - 2 );
	    int len = query.value( 3 ).toInt();
	    int precision = query.value( 4 ).toInt();
	    // swap length and precision if length == -1
	    if ( len == -1 && precision > -1 ) {
		len = precision - 4;
		precision = -1;
	    }
	    info.append( QSqlFieldInfo( query.value( 0 ).toString(),
					qDecodePSQLType( query.value( 1 ).toInt() ),
					query.value( 2 ).toBool(),
					len,
					precision,
					defVal,
					query.value( 1 ).toInt() ) );
	}
    }

    return info;
}

QSqlRecordInfo QPSQLDriver::recordInfo( const QSqlQuery& query ) const
{
    QSqlRecordInfo info;
    if ( !isOpen() )
	return info;
    if ( query.isActive() && query.driver() == this ) {
	QPSQLResult* result = (QPSQLResult*)query.result();
	int count = PQnfields( result->d->result );
	for ( int i = 0; i < count; ++i ) {
	    QString name = PQfname( result->d->result, i );
	    int len = PQfsize( result->d->result, i );
	    int precision = PQfmod( result->d->result, i );
	    // swap length and precision if length == -1
	    if ( len == -1 && precision > -1 ) {
		len = precision - 4;
		precision = -1;
	    }
	    info.append( QSqlFieldInfo( name,
					qDecodePSQLType( PQftype( result->d->result, i ) ),
					-1,
					len,
					precision,
					QVariant(),
					PQftype( result->d->result, i ) ) );
	}
    }
    return info;
}

QString QPSQLDriver::formatValue( const QSqlField* field,
				  bool ) const
{
    QString r;
    if ( field->isNull() ) {
	r = nullText();
    } else {
	switch ( field->type() ) {
	case QVariant::DateTime:
	    if ( field->value().toDateTime().isValid() ) {
		QDate dt = field->value().toDateTime().date();
		QTime tm = field->value().toDateTime().time();
		// msecs need to be right aligned otherwise psql
		// interpretes them wrong
		r = "'" + QString::number( dt.year() ) + "-" +
			  QString::number( dt.month() ) + "-" +
			  QString::number( dt.day() ) + " " +
			  tm.toString() + "." +
			  QString::number( tm.msec() ).rightJustify( 3, '0' ) + "'";
	    } else {
		r = nullText();
	    }
	    break;
	case QVariant::Time:
	    if ( field->value().toTime().isValid() ) {
		r = field->value().toTime().toString( Qt::ISODate );
	    } else {
		r = nullText();
	    }
	case QVariant::String:
	case QVariant::CString: {
	    switch ( field->value().type() ) {
		case QVariant::Rect: {
		    QRect rec = field->value().toRect();
		    // upper right corner then lower left according to psql docs
		    r = "'(" + QString::number( rec.right() ) +
			"," + QString::number( rec.bottom() ) +
			"),(" + QString::number( rec.left() ) +
			"," + QString::number( rec.top() ) + ")'";
		    break;
		}
		case QVariant::Point: {
		    QPoint p = field->value().toPoint();
		    r = "'(" + QString::number( p.x() ) +
			"," + QString::number( p.y() ) + ")'";
		    break;
		}
		case QVariant::PointArray: {
		    QPointArray pa = field->value().toPointArray();
		    r = "' ";
		    for ( int i = 0; i < (int)pa.size(); ++i ) {
			r += "(" + QString::number( pa[i].x() ) +
			     "," + QString::number( pa[i].y() ) + "),";
		    }
		    r.truncate( r.length() - 1 );
		    r += "'";
		    break;
		}
		default:
		    // Escape '\' characters
		    r = QSqlDriver::formatValue( field );
		    r.replace( "\\", "\\\\" );
		    break;
	    }
	    break;
	}
	case QVariant::Bool:
	    if ( field->value().toBool() )
		r = "TRUE";
	    else
		r = "FALSE";
	    break;
	case QVariant::ByteArray: {
	    QByteArray ba = field->value().asByteArray();
	    QString res;
	    r = "'";
	    unsigned char uc;
	    for ( int i = 0; i < (int)ba.size(); ++i ) {
		uc = (unsigned char) ba[ i ];
		if ( uc > 40 && uc < 92 ) {
		    r += uc;
		} else {
		    r += "\\\\";
		    r += QString::number( (unsigned char) ba[ i ], 8 ).rightJustify( 3, '0', TRUE );
		}
	    }
	    r += "'";
	    break;
	}
	default:
	    r = QSqlDriver::formatValue( field );
	    break;
	}
    }
    return r;
}
