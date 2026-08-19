#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qaction.h>

#include "labelonoff.xpm"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    QMainWindow * window = new QMainWindow;
    window->setCaption("Qt Example - Toggleaction");
    QToolBar * toolbar = new QToolBar( window );

    QAction * labelonoffaction = new QAction( window, "labelonoff" );
    labelonoffaction->setToggleAction( TRUE );

    labelonoffaction->setText( "labels on/off" );
    labelonoffaction->setAccel( Qt::ALT+Qt::Key_L );
    labelonoffaction->setIconSet( (QPixmap) labelonoff_xpm );

    QObject::connect( labelonoffaction, SIGNAL( toggled( bool ) ),
                      window, SLOT( setUsesTextLabel( bool ) ) );

    labelonoffaction->addTo( toolbar );

    app.setMainWidget( window );
    window->show();
    return app.exec();
}

