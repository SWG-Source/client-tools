#include <qapplication.h>
#include "mainform.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainForm *w = new MainForm;
    w->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), w, SLOT( quit() ) );
    return a.exec();
}
