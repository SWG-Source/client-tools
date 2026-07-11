#include <qapplication.h>
#include "addressbook.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    AddressBook *w = new AddressBook;
    w->show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
