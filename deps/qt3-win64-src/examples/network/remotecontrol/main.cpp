#include <qapplication.h>

#include "startup.h"

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    StartUp s;
    QObject::connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
