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
#include <qsqlcursor.h>
#include "../connection.h"

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv, FALSE );

    if ( createConnections() ) {
    	int count = 0;
	QSqlCursor cur( "prices" ); 
	QStringList names = QStringList() <<
	    "Screwdriver" << "Hammer" << "Wrench" << "Saw";
	int id = 20;
	for ( QStringList::Iterator name = names.begin();
	      name != names.end(); ++name ) {
	    QSqlRecord *buffer = cur.primeInsert();
	    buffer->setValue( "id", id );
	    buffer->setValue( "name", *name );
	    buffer->setValue( "price", 100.0 + (double)id );
	    count += cur.insert();
	    id++;
	}
    }

    return 0;
}
