#include <qapplication.h>
#include "vcr.h"

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
    Vcr *vcr = new Vcr;
    vcr->show();
    return app.exec();
}
