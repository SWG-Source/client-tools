/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "helpwindow.h"
#include <qapplication.h>
#include <qdir.h>
#include <stdlib.h>


int main( int argc, char ** argv )
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication a(argc, argv);

    QString home;
    if (argc > 1) {
        home = argv[1];
    } else {
	// Use a hard coded path. It is only an example.
	home = QDir( "../../doc/html/index.html" ).absPath();
    }

    HelpWindow *help = new HelpWindow(home, ".", 0, "help viewer");
    help->setCaption("Qt Example - Helpviewer");
    if ( QApplication::desktop()->width() > 400
	 && QApplication::desktop()->height() > 500 )
	help->show();
    else
	help->showMaximized();

    QObject::connect( &a, SIGNAL(lastWindowClosed()),
                      &a, SLOT(quit()) );

    return a.exec();
}
