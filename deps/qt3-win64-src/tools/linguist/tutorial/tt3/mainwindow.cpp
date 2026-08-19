/****************************************************************
**
** Implementation of MainWindow class, translation tutorial 3
**
****************************************************************/

#include "mainwindow.h"
#include "printpanel.h"

#include <qaccel.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>

MainWindow::MainWindow( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    setCaption( tr("Troll Print 1.0") );

    PrintPanel *pp = new PrintPanel( this );
    setCentralWidget( pp );

    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( tr("E&xit"), qApp, SLOT(quit()),
		      tr("Ctrl+Q", "Quit") );
    QPopupMenu *help = new QPopupMenu( this );
    help->insertItem( tr("&About"), this, SLOT(about()), Key_F1 );
    help->insertItem( tr("About &Qt"), this, SLOT(aboutQt()) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertSeparator();
    menuBar()->insertItem( tr("&Help"), help );
    menuBar()->setSeparator( QMenuBar::InWindowsStyle );
}

void MainWindow::about()
{
    QMessageBox::information( this, tr("About Troll Print 1.0"),
		   tr("Troll Print 1.0.\n\n"
		      "Copyright 1999 Macroshaft, Inc.") );
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt( this );
}
