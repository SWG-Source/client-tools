/****************************************************************************
** $Id: infodata.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "infodata.h"


// we hard code all nodes and data in InfoData class
InfoData::InfoData() :
    nodes( 17, TRUE ), data( 17, TRUE )
{
    nodes.setAutoDelete(TRUE);
    data.setAutoDelete(TRUE);
    QStringList *item;

    nodes.insert( "/", item = new QStringList( ) );
    (*item) << "D network";
    nodes.insert( "/network/", item = new QStringList() );
    (*item) << "D workstations" << "D printers" << "D fax";
    nodes.insert( "/network/workstations/", item = new QStringList() );
    (*item) << "D nibble" << "D douglas";
    nodes.insert( "/network/workstations/nibble/", item = new QStringList() );
    (*item) << "F os" << "F cpu" << "F memory";
    nodes.insert( "/network/workstations/douglas/", item = new QStringList() );
    (*item) << "F os" << "F cpu" << "F memory";
    nodes.insert( "/network/printers/", item = new QStringList() );
    (*item) << "D overbitt" << "D kroksleiven";
    nodes.insert( "/network/printers/overbitt/", item = new QStringList() );
    (*item) << "D jobs" << "F type";
    nodes.insert( "/network/printers/overbitt/jobs/", item = new QStringList() );
    (*item) << "F job1" << "F job2";
    nodes.insert( "/network/printers/kroksleiven/", item = new QStringList() );
    (*item) << "D jobs" << "F type";
    nodes.insert( "/network/printers/kroksleiven/jobs/", item = new QStringList() );
    nodes.insert( "/network/fax/", item = new QStringList() );
    (*item) << "F last_number";

    data.insert( "/network/workstations/nibble/os", new QString( "Linux" ) );
    data.insert( "/network/workstations/nibble/cpu", new QString( "AMD Athlon 1000" ) );
    data.insert( "/network/workstations/nibble/memory", new QString( "256 MB" ) );
    data.insert( "/network/workstations/douglas/os", new QString( "Windows 2000" ) );
    data.insert( "/network/workstations/douglas/cpu", new QString( "2 x Intel Pentium III 800" ) );
    data.insert( "/network/workstations/douglas/memory", new QString( "256 MB" ) );
    data.insert( "/network/printers/overbitt/type", new QString( "Lexmark Optra S 1255 PS" ) );
    data.insert( "/network/printers/overbitt/jobs/job1", 
                 new QString( "Qt manual\n" "A4 size\n" "3000 pages" ) );
    data.insert( "/network/printers/overbitt/jobs/job2", 
		 new QString( "Monthly report\n" "Letter size\n" "24 pages\n" "8 copies" ) );
    data.insert( "/network/printers/kroksleiven/type", new QString( "HP C LaserJet 4500-PS" ) );
    data.insert( "/network/fax/last_number", new QString( "22 22 22 22" ) );
}

QStringList InfoData::list( QString path, bool *found ) const
{
    if ( !path.endsWith( "/" ) )
	path += "/";
    if ( !path.startsWith( "/" ) )
	path = "/" + path;
    QStringList *list = nodes[ path ];
    if ( list ) {
	*found = TRUE;
	return *list;
    } else {
	*found = FALSE;
	QStringList empty;
	return empty;
    }
}

QString InfoData::get( QString path, bool *found ) const
{
    if ( !path.startsWith( "/" ) )
	path = "/" + path;
    QString *file = data[ path ];
    if ( file ) {
	*found = TRUE;
	return *file;
    } else {
	*found = FALSE;
	QString empty;
	return empty;
    }
}
