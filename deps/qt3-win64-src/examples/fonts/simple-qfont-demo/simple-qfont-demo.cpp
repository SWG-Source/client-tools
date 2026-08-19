/* $Id: simple-qfont-demo.cpp 1439 2006-05-05 18:17:01Z chehrlic $ */

#include "viewer.h"

#include <qapplication.h>
 
int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    Viewer * textViewer = new Viewer();
    textViewer->setCaption( "Qt Example - Simple QFont Demo" );
    app.setMainWidget( textViewer );
    textViewer->show();
    return app.exec();
}                  
