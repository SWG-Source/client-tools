#include <qapplication.h>
#include "metric.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    ConversionForm *w = new ConversionForm;
    w->show();
    a.setMainWidget( w );
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
