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
	QSqlQuery query( "SELECT id, name FROM people ORDER BY name" );
	if ( ! query.isActive() ) return 1; // Query failed
	int i;
	i = query.size();		// In this example we have 9 records; i == 9.
	query.first();			// Moves to the first record. 
	i = query.at();			// i == 0
	query.last();			// Moves to the last record.  
	i = query.at();			// i == 8
	query.seek( query.size() / 2 ); // Moves to the middle record. 
	i = query.at();			// i == 4
    }

    return 0;
}
