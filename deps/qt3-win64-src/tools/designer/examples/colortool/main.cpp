#include <qapplication.h>
#include "mainform.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MainForm *w = new MainForm;
    w->show();
    return a.exec();
}
