/****************************************************************
**
** Translation tutorial 2
**
****************************************************************/

#include "mainwindow.h"

#include <qapplication.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QTranslator translator( 0 );
    translator.load( QString("tt2_") + QTextCodec::locale(), "." );
    app.installTranslator( &translator );

    MainWindow *mw = new MainWindow;
    app.setMainWidget( mw );
    mw->show();
    return app.exec();
}
