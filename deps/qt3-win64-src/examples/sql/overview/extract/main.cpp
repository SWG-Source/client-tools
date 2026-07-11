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
	QSqlCursor cur( "creditors" );

	QStringList orderFields = QStringList() << "surname" << "forename";
	QSqlIndex order = cur.index( orderFields );

	QStringList filterFields = QStringList() << "surname" << "city";
	QSqlIndex filter = cur.index( filterFields );
	cur.setValue( "surname", "Chirac" );
	cur.setValue( "city", "Paris" );

	cur.select( filter, order );

	while ( cur.next() ) {
	    int id = cur.value( "id" ).toInt();
	    QString name = cur.value( "forename" ).toString() + " " +
			   cur.value( "surname" ).toString();
	    qDebug( QString::number( id ) + ": " + name );
	}
    }

    return 0;
}
