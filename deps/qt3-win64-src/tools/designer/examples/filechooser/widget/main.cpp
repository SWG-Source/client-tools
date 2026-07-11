#include <qapplication.h>
#include "filechooser.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    FileChooser *fc = new FileChooser;
    fc->show();
    return a.exec();
}
