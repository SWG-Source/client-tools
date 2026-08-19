/****************************************************************************
**
** Definition of MySQL driver classes
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

#ifndef QSQL_MYSQL_H
#define QSQL_MYSQL_H

#include <qsqldriver.h>
#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqlindex.h>

#if defined (Q_OS_WIN32)
#include <qt_windows.h>
#endif

#include <mysql.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_MYSQL
#else
#define Q_EXPORT_SQLDRIVER_MYSQL Q_EXPORT
#endif

class QMYSQLDriverPrivate;
class QMYSQLResultPrivate;
class QMYSQLDriver;
class QSqlRecordInfo;

class QMYSQLResult : public QSqlResult
{
    friend class QMYSQLDriver;
public:
    QMYSQLResult( const QMYSQLDriver* db );
    ~QMYSQLResult();

    MYSQL_RES* result();
protected:
    void		cleanup();
    bool		fetch( int i );
    bool		fetchNext();
    bool		fetchLast();
    bool		fetchFirst();
    QVariant		data( int field );
    bool		isNull( int field );
    bool		reset ( const QString& query );
    int			size();
    int			numRowsAffected();
private:
    QMYSQLResultPrivate* d;
};

class Q_EXPORT_SQLDRIVER_MYSQL QMYSQLDriver : public QSqlDriver
{
    friend class QMYSQLResult;
public:
    QMYSQLDriver( QObject * parent=0, const char * name=0 );
    QMYSQLDriver( MYSQL * con, QObject * parent=0, const char * name=0 );
    ~QMYSQLDriver();
    bool		hasFeature( DriverFeature f ) const;
    bool		open( const QString & db,
			      const QString & user = QString::null,
			      const QString & password = QString::null,
			      const QString & host = QString::null,
			      int port = -1 );
    void		close();
    QSqlQuery		createQuery() const;
    QStringList		tables( const QString& user ) const;
    QSqlIndex		primaryIndex( const QString& tablename ) const;
    QSqlRecord		record( const QString& tablename ) const;
    QSqlRecord		record( const QSqlQuery& query ) const;
    QSqlRecordInfo	recordInfo( const QString& tablename ) const;
    QSqlRecordInfo	recordInfo( const QSqlQuery& query ) const;
    QString		formatValue( const QSqlField* field,
				     bool trimStrings ) const;
    MYSQL*		mysql();
    // ### remove me for 4.0
    bool open( const QString& db,
	       const QString& user,
	       const QString& password,
	       const QString& host,
	       int port,
	       const QString& connOpts );
    
protected:
    bool		beginTransaction();
    bool		commitTransaction();
    bool		rollbackTransaction();
private:
    void		init();
    QMYSQLDriverPrivate* d;
};


#endif
