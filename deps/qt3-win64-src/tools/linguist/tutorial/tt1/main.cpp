/****************************************************************
**
** Translation tutorial 1
**
****************************************************************/

#include <qapplication.h>
#include <qpushbutton.h>
#include <qtranslator.h>


int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QTranslator translator( 0 );
    translator.load( "tt1_la", "." );
    app.installTranslator( &translator );

    QPushButton hello( QPushButton::tr("Hello world!"), 0 );

    app.setMainWidget( &hello );
    hello.show();
    return app.exec();
}
