#include <qapplication.h>
#include "chartform.h"


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    QString filename;
    if ( app.argc() > 1 ) {
	filename = app.argv()[1];
	if ( !filename.endsWith( ".cht" ) )
	    filename = QString::null;
    }

    ChartForm *cf = new ChartForm( filename );
    app.setMainWidget( cf );
    cf->show();

    return app.exec();
}
