/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "hello.h"
#include <qapplication.h>


/*
  The program starts here. It parses the command line and builds a message
  string to be displayed by the Hello widget.
*/

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    QString s;
    for ( int i=1; i<argc; i++ ) {
	s += argv[i];
	if ( i<argc-1 )
	    s += " ";
    }
    if ( s.isEmpty() )
	s = "Hello, World";
    Hello h( s );
#ifndef QT_NO_WIDGET_TOPEXTRA	// for Qt/Embedded minimal build
    h.setCaption( "Qt says hello" );
#endif
    QObject::connect( &h, SIGNAL(clicked()), &a, SLOT(quit()) );
    h.setFont( QFont("times",32,QFont::Bold) );		// default font
    h.setBackgroundColor( Qt::white );			// default bg color
    a.setMainWidget( &h );
    h.show();
    return a.exec();
}
