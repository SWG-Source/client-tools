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
#include "qwerty.h"


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    
    bool isSmall =  qApp->desktop()->size().width() < 450 
		  || qApp->desktop()->size().height() < 450;
    
    int i;
    for ( i= argc <= 1 ? 0 : 1; i<argc; i++ ) {
	Editor *e = new Editor;
	e->setCaption("Qt Example - QWERTY");
	if ( i > 0 )
	    e->load( argv[i] );
	if ( isSmall ) {
	    e->showMaximized();
	} else {
	    e->resize( 400, 400 );
	    e->show();
	}
    }
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
