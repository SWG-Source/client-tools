/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include "../connection.h"

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv, FALSE );

    if ( createConnections() ) {
	QSqlDatabase *oracledb = QSqlDatabase::database( "ORACLE" );
	// Copy data from the oracle database to the ODBC (default)
	// database
	QSqlQuery target;
	QSqlQuery query( "SELECT id, name FROM people", oracledb );
	if ( query.isActive() ) {
	    while ( query.next() ) {
		target.exec( "INSERT INTO people ( id, name ) VALUES ( " + 
			      query.value(0).toString() + 
			      ", '" + query.value(1).toString() +  "' )" );
	    }
	}
    }

    return 0;
}
