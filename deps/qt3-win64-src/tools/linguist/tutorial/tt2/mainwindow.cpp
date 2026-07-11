/****************************************************************
**
** Implementation of MainWindow class, translation tutorial 2
**
****************************************************************/

#include "arrowpad.h"
#include "mainwindow.h"

#include <qaccel.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>

MainWindow::MainWindow( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    ArrowPad *ap = new ArrowPad( this, "arrow pad" );
    setCentralWidget( ap );

    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( tr("E&xit"), qApp, SLOT(quit()),
		      tr("Ctrl+Q", "Quit") );
    menuBar()->insertItem( tr("&File"), file );
    menuBar()->setSeparator( QMenuBar::InWindowsStyle );
}
