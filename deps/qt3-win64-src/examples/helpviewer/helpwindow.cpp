/****************************************************************************
** $Id: helpwindow.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "helpwindow.h"
#include <qstatusbar.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include <ctype.h>

HelpWindow::HelpWindow( const QString& home_, const QString& _path,
			QWidget* parent, const char *name )
    : QMainWindow( parent, name, WDestructiveClose ),
      pathCombo( 0 )
{
    readHistory();
    readBookmarks();

    browser = new QTextBrowser( this );

    browser->mimeSourceFactory()->setFilePath( _path );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( sourceChanged(const QString& ) ),
	     this, SLOT( sourceChanged( const QString&) ) );

    setCentralWidget( browser );

    if ( !home_.isEmpty() )
	browser->setSource( home_ );

    connect( browser, SIGNAL( highlighted( const QString&) ),
	     statusBar(), SLOT( message( const QString&)) );

    resize( 640,700 );

    QPopupMenu* file = new QPopupMenu( this );
    file->insertItem( tr("&New Window"), this, SLOT( newWindow() ), CTRL+Key_N );
    file->insertItem( tr("&Open File"), this, SLOT( openFile() ), CTRL+Key_O );
    file->insertItem( tr("&Print"), this, SLOT( print() ), CTRL+Key_P );
    file->insertSeparator();
    file->insertItem( tr("&Close"), this, SLOT( close() ), CTRL+Key_Q );
    file->insertItem( tr("E&xit"), qApp, SLOT( closeAllWindows() ), CTRL+Key_X );

    // The same three icons are used twice each.
    QIconSet icon_back( QPixmap("back.xpm") );
    QIconSet icon_forward( QPixmap("forward.xpm") );
    QIconSet icon_home( QPixmap("home.xpm") );

    QPopupMenu* go = new QPopupMenu( this );
    backwardId = go->insertItem( icon_back,
				 tr("&Backward"), browser, SLOT( backward() ),
				 CTRL+Key_Left );
    forwardId = go->insertItem( icon_forward,
				tr("&Forward"), browser, SLOT( forward() ),
				CTRL+Key_Right );
    go->insertItem( icon_home, tr("&Home"), browser, SLOT( home() ) );

    QPopupMenu* help = new QPopupMenu( this );
    help->insertItem( tr("&About"), this, SLOT( about() ) );
    help->insertItem( tr("About &Qt"), this, SLOT( aboutQt() ) );

    hist = new QPopupMenu( this );
    QStringList::Iterator it = history.begin();
    for ( ; it != history.end(); ++it )
	mHistory[ hist->insertItem( *it ) ] = *it;
    connect( hist, SIGNAL( activated( int ) ),
	     this, SLOT( histChosen( int ) ) );

    bookm = new QPopupMenu( this );
    bookm->insertItem( tr( "Add Bookmark" ), this, SLOT( addBookmark() ) );
    bookm->insertSeparator();

    QStringList::Iterator it2 = bookmarks.begin();
    for ( ; it2 != bookmarks.end(); ++it2 )
	mBookmarks[ bookm->insertItem( *it2 ) ] = *it2;
    connect( bookm, SIGNAL( activated( int ) ),
	     this, SLOT( bookmChosen( int ) ) );

    menuBar()->insertItem( tr("&File"), file );
    menuBar()->insertItem( tr("&Go"), go );
    menuBar()->insertItem( tr( "History" ), hist );
    menuBar()->insertItem( tr( "Bookmarks" ), bookm );
    menuBar()->insertSeparator();
    menuBar()->insertItem( tr("&Help"), help );

    menuBar()->setItemEnabled( forwardId, FALSE);
    menuBar()->setItemEnabled( backwardId, FALSE);
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     this, SLOT( setBackwardAvailable( bool ) ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     this, SLOT( setForwardAvailable( bool ) ) );


    QToolBar* toolbar = new QToolBar( this );
    addToolBar( toolbar, "Toolbar");
    QToolButton* button;

    button = new QToolButton( icon_back, tr("Backward"), "", browser, SLOT(backward()), toolbar );
    connect( browser, SIGNAL( backwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( icon_forward, tr("Forward"), "", browser, SLOT(forward()), toolbar );
    connect( browser, SIGNAL( forwardAvailable(bool) ), button, SLOT( setEnabled(bool) ) );
    button->setEnabled( FALSE );
    button = new QToolButton( icon_home, tr("Home"), "", browser, SLOT(home()), toolbar );

    toolbar->addSeparator();

    pathCombo = new QComboBox( TRUE, toolbar );
    connect( pathCombo, SIGNAL( activated( const QString & ) ),
	     this, SLOT( pathSelected( const QString & ) ) );
    toolbar->setStretchableWidget( pathCombo );
    setRightJustification( TRUE );
    setDockEnabled( DockLeft, FALSE );
    setDockEnabled( DockRight, FALSE );

    pathCombo->insertItem( home_ );
    browser->setFocus();

}


void HelpWindow::setBackwardAvailable( bool b)
{
    menuBar()->setItemEnabled( backwardId, b);
}

void HelpWindow::setForwardAvailable( bool b)
{
    menuBar()->setItemEnabled( forwardId, b);
}


void HelpWindow::sourceChanged( const QString& url )
{
    if ( browser->documentTitle().isNull() )
	setCaption( "Qt Example - Helpviewer - " + url );
    else
	setCaption( "Qt Example - Helpviewer - " + browser->documentTitle() ) ;

    if ( !url.isEmpty() && pathCombo ) {
	bool exists = FALSE;
	int i;
	for ( i = 0; i < pathCombo->count(); ++i ) {
	    if ( pathCombo->text( i ) == url ) {
		exists = TRUE;
		break;
	    }
	}
	if ( !exists ) {
	    pathCombo->insertItem( url, 0 );
	    pathCombo->setCurrentItem( 0 );
	    mHistory[ hist->insertItem( url ) ] = url;
	} else
	    pathCombo->setCurrentItem( i );
    }
}

HelpWindow::~HelpWindow()
{
    history =  mHistory.values();

    QFile f( QDir::currentDirPath() + "/.history" );
    f.open( IO_WriteOnly );
    QDataStream s( &f );
    s << history;
    f.close();

    bookmarks = mBookmarks.values();

    QFile f2( QDir::currentDirPath() + "/.bookmarks" );
    f2.open( IO_WriteOnly );
    QDataStream s2( &f2 );
    s2 << bookmarks;
    f2.close();
}

void HelpWindow::about()
{
    QMessageBox::about( this, "HelpViewer Example",
			"<p>This example implements a simple HTML help viewer "
			"using Qt's rich text capabilities</p>"
			"<p>It's just about 400 lines of C++ code, so don't expect too much :-)</p>"
			);
}


void HelpWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "QBrowser" );
}

void HelpWindow::openFile()
{
#ifndef QT_NO_FILEDIALOG
    QString fn = QFileDialog::getOpenFileName( QString::null, QString::null, this );
    if ( !fn.isEmpty() )
	browser->setSource( fn );
#endif
}

void HelpWindow::newWindow()
{
    ( new HelpWindow(browser->source(), "qbrowser") )->show();
}

void HelpWindow::print()
{
#ifndef QT_NO_PRINTER
    QPrinter printer( QPrinter::HighResolution );
    printer.setFullPage(TRUE);
    if ( printer.setup( this ) ) {
	QPainter p( &printer );
	if( !p.isActive() ) // starting printing failed
	    return;
	QPaintDeviceMetrics metrics(p.device());
	int dpiy = metrics.logicalDpiY();
	int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
	QRect view( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
	QSimpleRichText richText( browser->text(),
				  QFont(),
				  browser->context(),
				  browser->styleSheet(),
				  browser->mimeSourceFactory(),
				  view.height() );
	richText.setWidth( &p, view.width() );
	int page = 1;
	do {
	    richText.draw( &p, margin, margin, view, colorGroup() );
	    view.moveBy( 0, view.height() );
	    p.translate( 0 , -view.height() );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
			view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
	    if ( view.top() - margin >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	} while (TRUE);
    }
#endif
}

void HelpWindow::pathSelected( const QString &_path )
{
    browser->setSource( _path );
    if ( mHistory.values().contains(_path) )
	mHistory[ hist->insertItem( _path ) ] = _path;
}

void HelpWindow::readHistory()
{
    if ( QFile::exists( QDir::currentDirPath() + "/.history" ) ) {
	QFile f( QDir::currentDirPath() + "/.history" );
	f.open( IO_ReadOnly );
	QDataStream s( &f );
	s >> history;
	f.close();
	while ( history.count() > 20 )
	    history.remove( history.begin() );
    }
}

void HelpWindow::readBookmarks()
{
    if ( QFile::exists( QDir::currentDirPath() + "/.bookmarks" ) ) {
	QFile f( QDir::currentDirPath() + "/.bookmarks" );
	f.open( IO_ReadOnly );
	QDataStream s( &f );
	s >> bookmarks;
	f.close();
    }
}

void HelpWindow::histChosen( int i )
{
    if ( mHistory.contains( i ) )
	browser->setSource( mHistory[ i ] );
}

void HelpWindow::bookmChosen( int i )
{
    if ( mBookmarks.contains( i ) )
	browser->setSource( mBookmarks[ i ] );
}

void HelpWindow::addBookmark()
{
    mBookmarks[ bookm->insertItem( caption() ) ] = browser->context();
}
