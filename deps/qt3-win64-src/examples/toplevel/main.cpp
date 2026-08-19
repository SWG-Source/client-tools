#include <qapplication.h>
#include "options.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    OptionsDialog dlg;
    return dlg.exec();
}
