/****************************************************************************
** $Id: connection.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/
#include <qsqldatabase.h>
#include "connection.h"

bool createConnections()
{
    // create the default database connection
    QSqlDatabase *defaultDB = QSqlDatabase::addDatabase( DB_BOOKS_DRIVER );
    defaultDB->setDatabaseName( DB_BOOKS );
    defaultDB->setUserName( DB_BOOKS_USER );
    defaultDB->setPassword( DB_BOOKS_PASSWD );
    defaultDB->setHostName( DB_BOOKS_HOST );
    if ( ! defaultDB->open() ) { 
	qWarning( "Failed to open books database: " + 
		  defaultDB->lastError().driverText() );
	qWarning( defaultDB->lastError().databaseText() );
	return FALSE;
    }

    QSqlQuery q(QString::null, defaultDB);
    q.exec("CREATE TABLE author ( id integer primary key, "
	   "forename varchar(40), surname varchar(40) )");
    q.exec("CREATE TABLE book ( id integer primary key, "
	   "title varchar(40), price numeric(10,2), authorid integer, notes varchar(255) )");

    q.exec("CREATE TABLE sequence ( tablename varchar(10), sequence numeric)");

    q.exec("INSERT INTO author VALUES ( 0, 'Philip K', 'Dick' )");
    q.exec("INSERT INTO author VALUES ( 1, 'Robert', 'Heinlein' )");
    q.exec("INSERT INTO author VALUES ( 2, 'Sarah', 'Paretsky' )");

    q.exec("INSERT INTO book VALUES (0, 'The Man Who Japed', 6.99, 0, 'A good book' )");
    q.exec("INSERT INTO book VALUES (1, 'The Man in the High Castle', 9.99, 0, 'Worth reading' )");
    q.exec("INSERT INTO book VALUES ( 2, 'The Number of the Beast', 8.99, 1, 'Get this!' )");
    q.exec("INSERT INTO book VALUES ( 3, 'Indemnity Only', 9.99, 2, 'Cool' )");
    q.exec("INSERT INTO book VALUES ( 4, 'Burn Marks', 9.99, 2, 'Need to make notes' )");
    q.exec("INSERT INTO book VALUES ( 5, 'Deadlock', 9.99, 2, 'Hmmm..' )");

    q.exec("INSERT INTO sequence VALUES ( 'author', 2 )");
    q.exec("INSERT INTO sequence VALUES ( 'book', 5 )");

    return TRUE;
}


