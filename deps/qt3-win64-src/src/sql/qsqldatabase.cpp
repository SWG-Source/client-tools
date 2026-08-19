/****************************************************************************
**
** Implementation of QSqlDatabase class
**
** Created : 2000-11-03
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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

#include "qsqldatabase.h"

#ifndef QT_NO_SQL

#ifdef Q_OS_WIN32
// Conflicting declarations of LPCBYTE in sqlfront.h and winscard.h
#define _WINSCARD_H_
#endif

#ifdef QT_SQL_POSTGRES
#include "drivers/psql/qsql_psql.h"
#endif
#ifdef QT_SQL_MYSQL
#include "drivers/mysql/qsql_mysql.h"
#endif
#ifdef QT_SQL_ODBC
#include "drivers/odbc/qsql_odbc.h"
#endif
#ifdef QT_SQL_OCI
#include "drivers/oci/qsql_oci.h"
#endif
#ifdef QT_SQL_TDS
#include "drivers/tds/qsql_tds.h"
#endif
#ifdef QT_SQL_DB2
#include "drivers/db2/qsql_db2.h"
#endif
#ifdef QT_SQL_SQLITE
#include "drivers/sqlite/qsql_sqlite.h"
#endif
#ifdef QT_SQL_IBASE
#include "drivers/ibase/qsql_ibase.h"
#endif

#include "qapplication.h"
#include "qsqlresult.h"
#include "qsqldriver.h"
#include "qsqldriverinterface_p.h"
#include <private/qpluginmanager_p.h>
#include <private/qsqlextension_p.h>
#include "qobject.h"
#include "qguardedptr.h"
#include "qcleanuphandler.h"
#include "qdict.h"
#include <stdlib.h>

QT_STATIC_CONST_IMPL char * const QSqlDatabase::defaultConnection = "qt_sql_default_connection";

QPtrDict<QSqlDriverExtension> *qt_driver_extension_dict = 0;
QPtrDict<QSqlOpenExtension> *qt_open_extension_dict = 0;

static QSingleCleanupHandler< QPtrDict<QSqlDriverExtension> > qt_driver_ext_cleanup;
static QSingleCleanupHandler< QPtrDict<QSqlOpenExtension> > qt_open_ext_cleanup;

Q_EXPORT QPtrDict<QSqlDriverExtension> *qSqlDriverExtDict()
{
    if ( !qt_driver_extension_dict ) {
	qt_driver_extension_dict = new QPtrDict<QSqlDriverExtension>;
	qt_driver_ext_cleanup.set( &qt_driver_extension_dict );
    }
    return qt_driver_extension_dict;
}

Q_EXPORT QPtrDict<QSqlOpenExtension> *qSqlOpenExtDict()
{
    if ( !qt_open_extension_dict ) {
	qt_open_extension_dict = new QPtrDict<QSqlOpenExtension>;
	qt_open_ext_cleanup.set( &qt_open_extension_dict );
    }
    return qt_open_extension_dict;
}

class QNullResult : public QSqlResult
{
public:
    QNullResult(const QSqlDriver* d): QSqlResult(d){}
    ~QNullResult(){}
protected:
    QVariant    data( int ) { return QVariant(); }
    bool        reset ( const QString& sqlquery ) { QString s(sqlquery); return FALSE; }
    bool        fetch( int i ) { i = i; return FALSE; }
    bool        fetchFirst() { return FALSE; }
    bool        fetchLast() { return FALSE; }
    bool        isNull( int ) {return FALSE; }
    QSqlRecord 	record() {return QSqlRecord();}
    int         size()  {return 0;}
    int         numRowsAffected() {return 0;}
};

class QNullDriver : public QSqlDriver
{
public:
    QNullDriver(): QSqlDriver(){}
    ~QNullDriver(){}
    bool    hasFeature( DriverFeature /* f */ ) const { return FALSE; } ;
    bool    open( const QString & ,
		  const QString & ,
		  const QString & ,
		  const QString &,
		  int ) {
	return FALSE;
    }
    void    close() {}
    QSqlQuery createQuery() const { return QSqlQuery( new QNullResult(this) ); }
};

typedef QDict<QSqlDriverCreatorBase> QDriverDict;

class QSqlDatabaseManager : public QObject
{
public:
    QSqlDatabaseManager( QObject * parent = 0, const char * name = 0 );
    ~QSqlDatabaseManager();
    static QSqlDatabase* database( const QString& name, bool open );
    static QSqlDatabase* addDatabase( QSqlDatabase* db, const QString & name );
    static void          removeDatabase( const QString& name );
    static void          removeDatabase( QSqlDatabase* db );
    static bool          contains( const QString& name );
    static QDriverDict*  driverDict();

protected:
    static QSqlDatabaseManager* instance();
    QDict< QSqlDatabase > dbDict;
    QDriverDict* drDict;
};

/*!
    Constructs an SQL database manager.
*/

QSqlDatabaseManager::QSqlDatabaseManager( QObject * parent, const char * name )
    : QObject( parent, name ), dbDict( 1 ), drDict( 0 )
{
}

/*!
    Destroys the object and frees any allocated resources. All open
    database connections are closed. All database connections are
    deleted.
*/

QSqlDatabaseManager::~QSqlDatabaseManager()
{
    QDictIterator< QSqlDatabase > it( dbDict );
    while ( it.current() ) {
	it.current()->close();
	delete it.current();
	++it;
    }
    delete drDict;
}

/*!
  \internal
*/
QDriverDict* QSqlDatabaseManager::driverDict()
{
    QSqlDatabaseManager* sqlConnection = instance();
    if ( !sqlConnection->drDict ) {
	sqlConnection->drDict = new QDriverDict();
	sqlConnection->drDict->setAutoDelete( TRUE );
    }
    return sqlConnection->drDict;
}


/*!
  \internal
*/
QSqlDatabaseManager* QSqlDatabaseManager::instance()
{
    static QGuardedPtr<QSqlDatabaseManager> sqlConnection = 0;
    if ( !sqlConnection ) {
	if( qApp == 0 ){
	    qFatal( "QSqlDatabaseManager: A QApplication object has to be "
		    "instantiated in order to use the SQL module." );
	    return 0;
	}
	sqlConnection = new QSqlDatabaseManager( qApp, "database manager" );
    }
    return (QSqlDatabaseManager*)sqlConnection;
}

/*!
    Returns the database connection called \a name. If \a open is
    TRUE, the database connection is opened. If \a name does not exist
    in the list of managed databases, 0 is returned.
*/

QSqlDatabase* QSqlDatabaseManager::database( const QString& name, bool open )
{
    if ( !contains( name ) )
	return 0;

    QSqlDatabaseManager* sqlConnection = instance();
    QSqlDatabase* db = sqlConnection->dbDict.find( name );
    if ( db && !db->isOpen() && open ) {
	db->open();
#ifdef QT_CHECK_RANGE
	if ( !db->isOpen() )
	    qWarning("QSqlDatabaseManager::database: unable to open database: " + db->lastError().databaseText() + ": " + db->lastError().driverText() );
#endif
    }
    return db;
}

/*!
    Returns TRUE if the list of database connections contains \a name;
    otherwise returns FALSE.
*/

bool QSqlDatabaseManager::contains( const QString& name )
{
   QSqlDatabaseManager* sqlConnection = instance();
   QSqlDatabase* db = sqlConnection->dbDict.find( name );
   if ( db )
       return TRUE;
   return FALSE;
}


/*!
    Adds a database to the SQL connection manager. The database
    connection is referred to by \a name. The newly added database
    connection is returned. This function will only return 0 if it is
    called \e before a QApplication object has been instantiated. Use
    the output of drivers() to determine whether a particular driver
    is available or not.

    The returned QSqlDatabase object is owned by the framework and
    must not be deleted. If you want to explicitly remove the connection,
    use removeDatabase().

    \sa QSqlDatabase database()
*/

QSqlDatabase* QSqlDatabaseManager::addDatabase( QSqlDatabase* db, const QString & name )
{
    QSqlDatabaseManager* sqlConnection = instance();
    if( sqlConnection == 0 )
	return 0;
    if ( contains( name ) )
	sqlConnection->removeDatabase( name );
    sqlConnection->dbDict.insert( name, db );
    return db;
}

/*!
    Removes the database connection \a name from the SQL connection
    manager.

    \warning There should be no open queries on the database
    connection when this function is called, otherwise a resource leak
    will occur.
*/

void QSqlDatabaseManager::removeDatabase( const QString& name )
{
    QSqlDatabaseManager* sqlConnection = instance();
    sqlConnection->dbDict.setAutoDelete( TRUE );
    sqlConnection->dbDict.remove( name );
    sqlConnection->dbDict.setAutoDelete( FALSE );
}


/*!
    Removes the database connection \a db from the SQL connection
    manager. The QSqlDatabase object is destroyed when it is removed
    from the manager.

    \warning The \a db pointer is not valid after this function has
    been called.
*/

void QSqlDatabaseManager::removeDatabase( QSqlDatabase* db )
{
    QSqlDatabaseManager* sqlConnection = instance();
    if ( !sqlConnection )
	return;
    QDictIterator< QSqlDatabase > it( sqlConnection->dbDict );
    while ( it.current() ) {
	if ( it.current() == db ) {
	    sqlConnection->dbDict.remove( it.currentKey() );
	    db->close();
	    delete db;
	    break;
	}
	++it;
    }
}

class QSqlDatabasePrivate
{
public:
    QSqlDatabasePrivate():
	driver(0),
#ifndef QT_NO_COMPONENT
	plugIns(0),
#endif
	port(-1) {}
    ~QSqlDatabasePrivate()
    {
    }
    QSqlDriver* driver;
#ifndef QT_NO_COMPONENT
    QPluginManager<QSqlDriverFactoryInterface> *plugIns;
#endif
    QString dbname;
    QString uname;
    QString pword;
    QString hname;
    QString drvName;
    int port;
    QString connOptions;
};

/*!
    \class QSqlDatabase qsqldatabase.h
    \brief The QSqlDatabase class is used to create SQL database
    connections and to provide transaction handling.

    \ingroup database
    \mainclass
    \module sql

    Note that transaction handling is not supported by every SQL
    database. You can find out whether transactions are supported
    using QSqlDriver::hasFeature().

    The QSqlDatabase class provides an abstract interface for
    accessing many types of database backends. Database-specific
    drivers are used internally to actually access and manipulate
    data, (see QSqlDriver). Result set objects provide the interface
    for executing and manipulating SQL queries (see QSqlQuery).
*/

/*!
    Adds a database to the list of database connections using the
    driver \a type and the connection name \a connectionName.

    The database connection is referred to by \a connectionName. The
    newly added database connection is returned. This pointer is owned
    by QSqlDatabase and will be deleted on program exit or when
    removeDatabase() is called.

    If \a connectionName is not specified, the newly added database
    connection becomes the default database connection for the
    application, and subsequent calls to database() (without a
    database name parameter) will return a pointer to it. If \a
    connectionName is given, use \link QSqlDatabase::database()
    database(connectionName)\endlink to retrieve a pointer to the
    database connection.

    \warning If you add a database with the same name as an
    existing database, the new database will replace the old one.
    This will happen automatically if you call this function more
    than once without specifying \a connectionName.

    \sa database() removeDatabase()
*/
QSqlDatabase* QSqlDatabase::addDatabase( const QString& type, const QString& connectionName )
{
    return QSqlDatabaseManager::addDatabase( new QSqlDatabase( type, connectionName ), connectionName );
}

/*!
    Returns the database connection called \a connectionName. The
    database connection must have been previously added with
    addDatabase(). If \a open is TRUE (the default) and the database
    connection is not already open it is opened now. If no \a
    connectionName is specified the default connection is used. If \a
    connectionName does not exist in the list of databases, 0 is
    returned. The pointer returned is owned by QSqlDatabase and should
    \e not be deleted.

    \warning There are restrictions on the use of database connections
    in threaded applications. Please see the \link threads.html#threads-sql
    Thread Support in Qt\endlink document for more information about
    threading and SQL databases.
*/

QSqlDatabase* QSqlDatabase::database( const QString& connectionName, bool open )
{
    return QSqlDatabaseManager::database( connectionName, open );
}

/*!
    Removes the database connection \a connectionName from the list of
    database connections.

    \warning There should be no open queries on the database
    connection when this function is called, otherwise a resource leak
    will occur.
*/

void QSqlDatabase::removeDatabase( const QString& connectionName )
{
    QSqlDatabaseManager::removeDatabase( connectionName );
}

/*!
    \overload

    Removes the database connection \a db from the list of database
    connections. The QSqlDatabase object is destroyed when it is removed
    from the list.

    \warning The \a db pointer is not valid after this function has
    been called. There should be no open queries on the database
    connection when this function is called, otherwise a resource leak
    will occur.
*/

void QSqlDatabase::removeDatabase( QSqlDatabase* db )
{
    QSqlDatabaseManager::removeDatabase( db );
}

/*!
    Returns a list of all the available database drivers.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = QSqlDatabase::drivers();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/

QStringList QSqlDatabase::drivers()
{
    QStringList l;

#ifndef QT_NO_COMPONENT
    QPluginManager<QSqlDriverFactoryInterface> *plugIns;
    plugIns = new QPluginManager<QSqlDriverFactoryInterface>( IID_QSqlDriverFactory, QApplication::libraryPaths(), "/sqldrivers" );

    l = plugIns->featureList();
    delete plugIns;
#endif

    QDictIterator<QSqlDriverCreatorBase> itd( *QSqlDatabaseManager::driverDict() );
    while ( itd.current() ) {
	if ( !l.contains( itd.currentKey() ) )
	    l << itd.currentKey();
	++itd;
    }

#ifdef QT_SQL_POSTGRES
    if ( !l.contains( "QPSQL7" ) )
	l << "QPSQL7";
#endif
#ifdef QT_SQL_MYSQL
    if ( !l.contains( "QMYSQL3" ) )
	l << "QMYSQL3";
#endif
#ifdef QT_SQL_ODBC
    if ( !l.contains( "QODBC3" ) )
	l << "QODBC3";
#endif
#ifdef QT_SQL_OCI
    if ( !l.contains( "QOCI8" ) )
	l << "QOCI8";
#endif
#ifdef QT_SQL_TDS
    if ( !l.contains( "QTDS7" ) )
	l << "QTDS7";
#endif
#ifdef QT_SQL_DB2
    if ( !l.contains( "QDB2" ) )
	l << "QDB2";
#endif
#ifdef QT_SQL_SQLITE
    if ( !l.contains( "QSQLITE" ) )
	l << "QSQLITE";
#endif
#ifdef QT_SQL_IBASE
    if ( !l.contains( "QIBASE" ) )
	l << "QIBASE";
#endif

    return l;
}

/*!
    This function registers a new SQL driver called \a name, within
    the SQL framework. This is useful if you have a custom SQL driver
    and don't want to compile it as a plugin.

    Example usage:

    \code
    QSqlDatabase::registerSqlDriver( "MYDRIVER", new QSqlDriverCreator<MyDatabaseDriver> );
    QSqlDatabase* db = QSqlDatabase::addDatabase( "MYDRIVER" );
    ...
    \endcode

    \warning The framework takes ownership of the \a creator pointer,
    so it should not be deleted.
*/
void QSqlDatabase::registerSqlDriver( const QString& name, const QSqlDriverCreatorBase* creator )
{
    QSqlDatabaseManager::driverDict()->remove( name );
    if ( creator )
	QSqlDatabaseManager::driverDict()->insert( name, creator );
}

/*!
    Returns TRUE if the list of database connections contains \a
    connectionName; otherwise returns FALSE.
*/

bool QSqlDatabase::contains( const QString& connectionName )
{
    return QSqlDatabaseManager::contains( connectionName );
}


/*!
    Creates a QSqlDatabase connection called \a name that uses the
    driver referred to by \a type, with the parent \a parent and the
    object name \a objname. If the \a type is not recognized, the
    database connection will have no functionality.

    The currently available drivers are:

    \table
    \header \i Driver Type \i Description
    \row \i QODBC3 \i ODBC Driver (includes Microsoft SQL Server)
    \row \i QOCI8 \i Oracle Call Interface Driver
    \row \i QPSQL7 \i PostgreSQL v6.x and v7.x Driver
    \row \i QTDS7 \i Sybase Adaptive Server
    \row \i QMYSQL3 \i MySQL Driver
    \row \i QDB2 \i IBM DB2, v7.1 and higher
    \row \i QSQLITE \i SQLite Driver
    \row \i QIBASE \i Borland Interbase Driver
    \endtable

    Additional third party drivers, including your own custom drivers,
    can be loaded dynamically.

    \sa registerSqlDriver()
*/

QSqlDatabase::QSqlDatabase( const QString& type, const QString& name, QObject * parent, const char * objname )
    : QObject( parent, objname )
{
    init( type, name );
}


/*!
    \overload

     Creates a database connection using the driver \a driver, with
     the parent \a parent and the object name \a objname.

     \warning The framework takes ownership of the \a driver pointer,
     so it should not be deleted.
*/

QSqlDatabase::QSqlDatabase( QSqlDriver* driver, QObject * parent, const char * objname )
    : QObject( parent, objname )
{
    d = new QSqlDatabasePrivate();
    d->driver = driver;
}

/*!
  \internal

  Create the actual driver instance \a type.
*/

void QSqlDatabase::init( const QString& type, const QString& )
{
    d = new QSqlDatabasePrivate();
    d->drvName = type;

    if ( !d->driver ) {

#ifdef QT_SQL_POSTGRES
	if ( type == "QPSQL7" )
	    d->driver = new QPSQLDriver();
#endif

#ifdef QT_SQL_MYSQL
	if ( type == "QMYSQL3" )
	    d->driver = new QMYSQLDriver();
#endif

#ifdef QT_SQL_ODBC
	if ( type == "QODBC3" )
	    d->driver = new QODBCDriver();
#endif

#ifdef QT_SQL_OCI
	if ( type == "QOCI8" )
	    d->driver = new QOCIDriver();
#endif

#ifdef QT_SQL_TDS
	if ( type == "QTDS7" )
	    d->driver = new QTDSDriver();
#endif

#ifdef QT_SQL_DB2
	if ( type == "QDB2" )
	    d->driver = new QDB2Driver();
#endif

#ifdef QT_SQL_SQLITE
	if ( type == "QSQLITE" )
	    d->driver = new QSQLiteDriver();
#endif

#ifdef QT_SQL_IBASE
	if ( type == "QIBASE" )
	    d->driver = new QIBaseDriver();
#endif

    }

    if ( !d->driver ) {
	QDictIterator<QSqlDriverCreatorBase> it( *QSqlDatabaseManager::driverDict() );
	while ( it.current() && !d->driver ) {
	    if ( type == it.currentKey() ) {
		d->driver = it.current()->createObject();
	    }
	    ++it;
	}
    }

#ifndef QT_NO_COMPONENT
    if ( !d->driver ) {
	d->plugIns =
	    new QPluginManager<QSqlDriverFactoryInterface>( IID_QSqlDriverFactory, QApplication::libraryPaths(), "/sqldrivers" );

	QInterfacePtr<QSqlDriverFactoryInterface> iface = 0;
	d->plugIns->queryInterface( type, &iface );
	if( iface )
	    d->driver = iface->create( type );
    }
#endif

    if ( !d->driver ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QSqlDatabase: %s driver not loaded", type.latin1() );
	qWarning( "QSqlDatabase: available drivers: " + drivers().join(" ") );
#endif
	d->driver = new QNullDriver();
	d->driver->setLastError( QSqlError( "Driver not loaded", "Driver not loaded" ) );
    }
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlDatabase::~QSqlDatabase()
{
    delete d->driver;
#ifndef QT_NO_COMPONENT
    delete d->plugIns;
#endif
    delete d;
}

/*!
    Executes a SQL statement (e.g. an \c INSERT, \c UPDATE or \c
    DELETE statement) on the database, and returns a QSqlQuery object.
    Use lastError() to retrieve error information. If \a query is
    QString::null, an empty, invalid query is returned and lastError()
    is not affected.

    \sa QSqlQuery lastError()
*/

QSqlQuery QSqlDatabase::exec( const QString & query ) const
{
    QSqlQuery r = d->driver->createQuery();
    if ( !query.isNull() ) {
	r.exec( query );
	d->driver->setLastError( r.lastError() );
    }
    return r;
}

/*!
    Opens the database connection using the current connection values.
    Returns TRUE on success; otherwise returns FALSE. Error
    information can be retrieved using the lastError() function.

    \sa lastError()
*/

bool QSqlDatabase::open()
{
    return d->driver->open( d->dbname, d->uname, d->pword, d->hname,
			    d->port, d->connOptions );
}

/*!
    \overload

    Opens the database connection using the given \a user name and \a
    password. Returns TRUE on success; otherwise returns FALSE. Error
    information can be retrieved using the lastError() function.

    This function does not store the password it is given. Instead,
    the password is passed directly to the driver for opening a
    connection and is then discarded.

    \sa lastError()
*/

bool QSqlDatabase::open( const QString& user, const QString& password )
{
    setUserName( user );
    return d->driver->open( d->dbname, user, password, d->hname,
			    d->port, d->connOptions );
}

/*!
    Closes the database connection, freeing any resources acquired.

    \sa removeDatabase()
*/

void QSqlDatabase::close()
{
    d->driver->close();
}

/*!
    Returns TRUE if the database connection is currently open;
    otherwise returns FALSE.
*/

bool QSqlDatabase::isOpen() const
{
    return d->driver->isOpen();
}

/*!
    Returns TRUE if there was an error opening the database
    connection; otherwise returns FALSE. Error information can be
    retrieved using the lastError() function.
*/

bool QSqlDatabase::isOpenError() const
{
    return d->driver->isOpenError();
}

/*!
    Begins a transaction on the database if the driver supports
    transactions. Returns TRUE if the operation succeeded; otherwise
    returns FALSE.

    \sa QSqlDriver::hasFeature() commit() rollback()
*/

bool QSqlDatabase::transaction()
{
    if ( !d->driver->hasFeature( QSqlDriver::Transactions ) )
	return FALSE;
    return d->driver->beginTransaction();
}

/*!
    Commits a transaction to the database if the driver supports
    transactions. Returns TRUE if the operation succeeded; otherwise
    returns FALSE.

    \sa QSqlDriver::hasFeature() rollback()
*/

bool QSqlDatabase::commit()
{
    if ( !d->driver->hasFeature( QSqlDriver::Transactions ) )
	return FALSE;
    return d->driver->commitTransaction();
}

/*!
    Rolls a transaction back on the database if the driver supports
    transactions. Returns TRUE if the operation succeeded; otherwise
    returns FALSE.

    \sa QSqlDriver::hasFeature() commit() transaction()
*/

bool QSqlDatabase::rollback()
{
    if ( !d->driver->hasFeature( QSqlDriver::Transactions ) )
	return FALSE;
    return d->driver->rollbackTransaction();
}

/*!
    \property QSqlDatabase::databaseName
    \brief the name of the database

    Note that the database name is the TNS Service Name for the QOCI8
    (Oracle) driver.

    For the QODBC3 driver it can either be a DSN, a DSN filename (the
    file must have a \c .dsn extension), or a connection string. MS
    Access users can for example use the following connection string
    to open a \c .mdb file directly, instead of having to create a DSN
    entry in the ODBC manager:

    \code
    ...
    db = QSqlDatabase::addDatabase( "QODBC3" );
    db->setDatabaseName( "DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=myaccessfile.mdb" );
    if ( db->open() ) {
        // success!
    }
    ...
    \endcode
    ("FIL" is the required spelling in Microsoft's API.)

    There is no default value.
*/

void QSqlDatabase::setDatabaseName( const QString& name )
{
    d->dbname = name;
}

/*!
    \property QSqlDatabase::userName
    \brief the user name connected to the database

    There is no default value.
*/

void QSqlDatabase::setUserName( const QString& name )
{
    d->uname = name;
}

/*!
    \property QSqlDatabase::password
    \brief the password used to connect to the database

    There is no default value.

    \warning This function stores the password in plain text within
    Qt. Use the open() call that takes a password as parameter to
    avoid this behaviour.

    \sa open()
*/

void QSqlDatabase::setPassword( const QString& password )
{
    d->pword = password;
}

/*!
    \property QSqlDatabase::hostName
    \brief the host name where the database resides

    There is no default value.
*/

void QSqlDatabase::setHostName( const QString& host )
{
    d->hname = host;
}

/*!
    \property QSqlDatabase::port
    \brief the port used to connect to the database

    There is no default value.
*/

void QSqlDatabase::setPort( int p )
{
    d->port = p;
}

QString QSqlDatabase::databaseName() const
{
    return d->dbname;
}

QString QSqlDatabase::userName() const
{
    return d->uname;
}

QString QSqlDatabase::password() const
{
    return d->pword;
}

QString QSqlDatabase::hostName() const
{
    return d->hname;
}

/*!
    Returns the name of the driver used by the database connection.
*/
QString QSqlDatabase::driverName() const
{
    return d->drvName;
}

int QSqlDatabase::port() const
{
    return d->port;
}

/*!
    Returns the database driver used to access the database
    connection.
*/

QSqlDriver* QSqlDatabase::driver() const
{
    return d->driver;
}

/*!
    Returns information about the last error that occurred on the
    database. See QSqlError for more information.
*/

QSqlError QSqlDatabase::lastError() const
{
    return d->driver->lastError();
}


/*!
    \overload

    Returns a list of the database's tables that are visible to the
    user. To include views or system tables, use the version of this
    function that takes a table \c type parameter.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myDatabase.tables();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode
*/

QStringList QSqlDatabase::tables() const
{
    return tables( QSql::Tables );
}

/*!
    Returns a list of the database's tables, system tables and views,
    as specified by the parameter \a type.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myDatabase.tables( QSql::Tables | QSql::Views );
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
        myProcessing( *it );
        ++it;
    }
    \endcode
*/

QStringList QSqlDatabase::tables( QSql::TableType type ) const
{
    return d->driver->tables( QString::number( (int)type ) );
}

/*!
    Returns the primary index for table \a tablename. If no primary
    index exists an empty QSqlIndex will be returned.
*/

QSqlIndex QSqlDatabase::primaryIndex( const QString& tablename ) const
{
    return d->driver->primaryIndex( tablename );
}


/*!
    Returns a QSqlRecord populated with the names of all the fields in
    the table (or view) called \a tablename. The order in which the
    fields appear in the record is undefined. If no such table (or
    view) exists, an empty record is returned.

    \sa recordInfo()
*/

QSqlRecord QSqlDatabase::record( const QString& tablename ) const
{
    return d->driver->record( tablename );
}


/*!
    \overload

    Returns a QSqlRecord populated with the names of all the fields
    used in the SQL \a query. If the query is a "SELECT *" the order
    in which fields appear in the record is undefined.

    \sa recordInfo()
*/

QSqlRecord QSqlDatabase::record( const QSqlQuery& query ) const
{
    return d->driver->record( query );
}

/*!
    Returns a QSqlRecordInfo populated with meta data about the table
    or view \a tablename. If no such table (or view) exists, an empty
    record is returned.

    \sa QSqlRecordInfo, QSqlFieldInfo, record()
*/
QSqlRecordInfo QSqlDatabase::recordInfo( const QString& tablename ) const
{
    return d->driver->recordInfo( tablename );
}

/*!
    \overload

    Returns a QSqlRecordInfo object with meta data for the QSqlQuery
    \a query. Note that this overloaded function may return less
    information than the recordInfo() function which takes the name of
    a table as parameter.

    \sa QSqlRecordInfo, QSqlFieldInfo, record()
*/
QSqlRecordInfo QSqlDatabase::recordInfo( const QSqlQuery& query ) const
{
    return d->driver->recordInfo( query );
}

/*!
    \property QSqlDatabase::connectOptions
    \brief the database connect options

    The format of the options string is a semi-colon separated list of
    option names or option = value pairs. The options depend on the
    database client used:

    \table
    \header \i ODBC \i MySQL \i PostgreSQL
    \row

    \i
    \list
    \i SQL_ATTR_ACCESS_MODE
    \i SQL_ATTR_LOGIN_TIMEOUT
    \i SQL_ATTR_CONNECTION_TIMEOUT
    \i SQL_ATTR_CURRENT_CATALOG
    \i SQL_ATTR_METADATA_ID
    \i SQL_ATTR_PACKET_SIZE
    \i SQL_ATTR_TRACEFILE
    \i SQL_ATTR_TRACE
    \endlist

    \i
    \list
    \i CLIENT_COMPRESS
    \i CLIENT_FOUND_ROWS
    \i CLIENT_IGNORE_SPACE
    \i CLIENT_SSL
    \i CLIENT_ODBC
    \i CLIENT_NO_SCHEMA
    \i CLIENT_INTERACTIVE
    \endlist

    \i
    \list
    \i connect_timeout
    \i options
    \i tty
    \i requiressl
    \i service
    \endlist

    \header \i DB2 \i OCI \i TDS
    \row

    \i
    \list
    \i SQL_ATTR_ACCESS_MODE
    \i SQL_ATTR_LOGIN_TIMEOUT
    \endlist

    \i
    \e none

    \i
    \e none

    \endtable

    Example of usage:
    \code
    ...
    // MySQL connection
    db->setConnectOptions( "CLIENT_SSL;CLIENT_IGNORE_SPACE" ); // use an SSL connection to the server
    if ( !db->open() ) {
        db->setConnectOptions(); // clears the connect option string
	...
    }
    ...
    // PostgreSQL connection
    db->setConnectOptions( "requiressl=1" ); // enable PostgreSQL SSL connections
    if ( !db->open() ) {
        db->setConnectOptions(); // clear options
	...
    }
    ...
    // ODBC connection
    db->setConnectOptions( "SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_ONLY;SQL_ATTR_TRACE=SQL_OPT_TRACE_ON" ); // set ODBC options
    if ( !db->open() ) {
        db->setConnectOptions(); // don't try to set this option
	...
    }
    \endcode

    Please refer to the client library documentation for more
    information about the different options. The options will be set
    prior to opening the database connection. Setting new options
    without re-opening the connection does nothing.

    \sa connectOptions()
*/

void QSqlDatabase::setConnectOptions( const QString& options )
{
    d->connOptions = options;
}

QString QSqlDatabase::connectOptions() const
{
    return d->connOptions;
}

/*!
    Returns TRUE if a driver called \a name is available; otherwise
    returns FALSE.

    \sa drivers()
*/

bool QSqlDatabase::isDriverAvailable( const QString& name )
{
    QStringList l = drivers();
    QStringList::ConstIterator it = l.begin();
    for ( ;it != l.end(); ++it ) {
	if ( *it == name )
	    return TRUE;
    }
    return FALSE;
}

/*! \overload

    This function is useful if you need to set up the database
    connection and instantiate the driver yourself. If you do this, it
    is recommended that you include the driver code in your own
    application. For example, setting up a custom PostgreSQL
    connection and instantiating the QPSQL7 driver can be done the
    following way:

    \code
    #include "qtdir/src/sql/drivers/psql/qsql_psql.cpp"
    \endcode
    (We assume that \c qtdir is the directory where Qt is installed.)
    This will pull in the code that is needed to use the PostgreSQL
    client library and to instantiate a QPSQLDriver object, assuming
    that you have the PostgreSQL headers somewhere in your include
    search path.

    \code
    PGconn* con = PQconnectdb( "host=server user=bart password=simpson dbname=springfield" );
    QPSQLDriver* drv =  new QPSQLDriver( con );
    QSqlDatabase* db = QSqlDatabase::addDatabase( drv ); // becomes the new default connection
    QSqlQuery q;
    q.exec( "SELECT * FROM people" );
    ...
    \endcode

    The above code sets up a PostgreSQL connection and instantiates a
    QPSQLDriver object. Next, addDatabase() is called to add the
    connection to the known connections so that it can be used by the
    Qt SQL classes. When a driver is instantiated with a connection
    handle (or set of handles), Qt assumes that you have already
    opened the database connection.

    Remember that you must link your application against the database
    client library as well. The simplest way to do this is to add
    lines like those below to your \c .pro file:

    \code
    unix:LIBS += -lpq
    win32:LIBS += libpqdll.lib
    \endcode

    You will need to have the client library in your linker's search
    path.

    The method described above will work for all the drivers, the only
    difference is the arguments the driver constructors take. Below is
    an overview of the drivers and their constructor arguments.

    \table
    \header \i Driver \i Class name \i Constructor arguments \i File to include
    \row
    \i QPSQL7
    \i QPSQLDriver
    \i PGconn* connection
    \i \c qsql_psql.cpp
    \row
    \i QMYSQL3
    \i QMYSQLDriver
    \i MYSQL* connection
    \i \c qsql_mysql.cpp
    \row
    \i QOCI8
    \i QOCIDriver
    \i OCIEnv* environment, OCIError* error, OCISvcCtx* serviceContext
    \i \c qsql_oci.cpp
    \row
    \i QODBC3
    \i QODBCDriver
    \i SQLHANDLE environment, SQLHANDLE connection
    \i \c qsql_odbc.cpp
    \row
    \i QDB2
    \i QDB2
    \i SQLHANDLE environment, SQLHANDLE connection
    \i \c qsql_db2.cpp
    \row
    \i QTDS7
    \i QTDSDriver
    \i LOGINREC* loginRecord, DBPROCESS* dbProcess, const QString& hostName
    \i \c qsql_tds.cpp
    \row
    \i QSQLITE
    \i QSQLiteDriver
    \i sqlite* connection
    \i \c qsql_sqlite.cpp
    \row
    \i QIBASE
    \i QIBaseDriver
    \i isc_db_handle connection
    \i \c qsql_ibase.cpp
    \endtable

    Note: The host name (or service name) is needed when constructing
    the QTDSDriver for creating new connections for internal
    queries. This is to prevent the simultaneous usage of several
    QSqlQuery/\l{QSqlCursor} objects from blocking each other.

    \warning The SQL framework takes ownership of the \a driver pointer,
    and it should not be deleted. The returned QSqlDatabase object is
    owned by the framework and must not be deleted. If you want to
    explicitly remove the connection, use removeDatabase()

    \sa drivers()
*/

QSqlDatabase* QSqlDatabase::addDatabase( QSqlDriver* driver, const QString& connectionName )
{
    return QSqlDatabaseManager::addDatabase( new QSqlDatabase( driver ), connectionName );
}
#endif // QT_NO_SQL
