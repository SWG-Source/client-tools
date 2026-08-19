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
#include <qtable.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qstringlist.h>

// Qt logo: static const char *qtlogo_xpm[]
#include "qtlogo.xpm"

// Table size

const int numRows = 30;
const int numCols = 10;

// The program starts here.

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QTable table( numRows, numCols );

    QHeader *header = table.horizontalHeader();
    header->setLabel( 0, QObject::tr( "Tiny" ), 40 );
    header->setLabel( 1, QObject::tr( "Checkboxes" ) );
    header->setLabel( 5, QObject::tr( "Combos" ) );
    table.setColumnMovingEnabled(TRUE);

    QImage img( qtlogo_xpm );
    QPixmap pix = img.scaleHeight( table.rowHeight(3) );
    table.setPixmap( 3, 2, pix );
    table.setText( 3, 2, "A Pixmap" );

    QStringList comboEntries;
    comboEntries << "one" << "two" << "three" << "four";

    for ( int i = 0; i < numRows; ++i ){
	QComboTableItem * item = new QComboTableItem( &table, comboEntries, FALSE );
	item->setCurrentItem( i % 4 );
	table.setItem( i, 5, item );
    }
    for ( int j = 0; j < numRows; ++j )
	table.setItem( j, 1, new QCheckTableItem( &table, "Check me" ) );

    app.setMainWidget( &table );
    table.show();
    return app.exec();
}
