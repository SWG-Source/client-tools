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
#include "textedit.h"

int main( int argc, char ** argv ) 
{
    QApplication a( argc, argv );
    TextEdit * mw = new TextEdit();
    mw->setCaption( "Richtext Editor" );
    mw->resize( 640, 800 );
    mw->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
