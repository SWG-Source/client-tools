#include <qapplication.h>
#include "sizeaware.h"

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
    SizeAware *sizeaware = new SizeAware( 0, 0, TRUE );
    sizeaware->exec();
    sizeaware->destroy();
    return 0; 
}
