/****************************************************************************
** $Id: frame.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "frame.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qaccel.h>
#include <qtoolbox.h>
#include <qpainter.h>
#include <qwidgetstack.h>
#include <qstylefactory.h>
#include <qaction.h>
#include <qsignalmapper.h>
#include <qdict.h>
#include <qdir.h>
#include <qtextcodec.h>
#include <stdlib.h>
#include <qbuttongroup.h>
#include <qtoolbutton.h>

static QTranslator *translator = 0;
static QTranslator *qt_translator = 0;

Frame::Frame( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    QMenuBar *mainMenu = menuBar();
    QPopupMenu *fileMenu = new QPopupMenu( this, "file" );
    fileMenu->insertItem( tr( "&Exit" ), this, SLOT( close() ),
			  QAccel::stringToKey( tr( "Ctrl+Q" ) ) );

    QPopupMenu *styleMenu = new QPopupMenu( this, "style" );
    styleMenu->setCheckable( TRUE );
    QActionGroup *ag = new QActionGroup( this, 0 );
    ag->setExclusive( TRUE );
    QSignalMapper *styleMapper = new QSignalMapper( this );
    connect( styleMapper, SIGNAL( mapped( const QString& ) ),
	     this, SLOT( setStyle( const QString& ) ) );

    QStringList list = QStyleFactory::keys();
    list.sort();
    QDict<int> stylesDict( 17, FALSE );
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
	QString style = *it;
	QString styleAccel = style;
	if ( stylesDict[styleAccel.left(1)] ) {
	    for ( uint i = 0; i < styleAccel.length(); i++ ) {
		if ( !stylesDict[styleAccel.mid( i, 1 )] ) {
		    stylesDict.insert(styleAccel.mid( i, 1 ), (const int *)1);
		    styleAccel = styleAccel.insert( i, '&' );
		    break;
		}
	    }
	} else {
	    stylesDict.insert(styleAccel.left(1), (const int *)1);
	    styleAccel = "&"+styleAccel;
	}
	QAction *a = new QAction( style, QIconSet(),
				  styleAccel, 0, ag, 0, ag->isExclusive() );
	connect( a, SIGNAL( activated() ), styleMapper, SLOT(map()) );
	styleMapper->setMapping( a, a->text() );
    }
    ag->addTo( styleMenu );

    mainMenu->insertItem( tr( "&File" ), fileMenu );
    mainMenu->insertItem( tr( "St&yle" ), styleMenu );

    stack = new QWidgetStack( this );

    setCentralWidget( stack );
}

void Frame::setCategories( const QPtrList<CategoryInterface> &l )
{
    categories = l;
    QDockWindow *dw = new QDockWindow( QDockWindow::InDock, this );
    dw->setResizeEnabled( TRUE );
    dw->setVerticalStretchable( TRUE );
    addDockWindow( dw, DockLeft );
    setDockEnabled( dw, DockTop, FALSE );
    setDockEnabled( dw, DockBottom, FALSE );
    dw->setCloseMode( QDockWindow::Always );

    toolBox = new QToolBox( dw );
    dw->setWidget( toolBox );

    dw->setCaption( tr( "Demo Categories" ) );

    for ( int i = 0; i < categories.count(); ++i )
	toolBox->addItem( createCategoryPage( categories.at(i) ),
			  categories.at(i)->icon(),
			  categories.at(i)->name() );

    categories.first()->setCurrentCategory( 0 );
}

QWidget *Frame::createCategoryPage( CategoryInterface *c )
{
    QButtonGroup *g = new QButtonGroup( 1, Horizontal, toolBox );
    g->setFrameStyle( QFrame::NoFrame );
    g->setEraseColor(green);
    g->setBackgroundMode(PaletteBase);
    for ( int i = 0; i < c->numCategories(); ++i ) {
	QToolButton *b = new QToolButton( g );
	b->setBackgroundMode(PaletteBase);
	b->setTextLabel( c->categoryName( i ) );
	b->setIconSet( c->categoryIcon( i ) );
	b->setAutoRaise( TRUE );
	b->setTextPosition( QToolButton::Right );
	b->setUsesTextLabel( TRUE );
	g->insert( b, i + c->categoryOffset() );
	connect( g, SIGNAL( clicked( int ) ), c, SLOT( setCurrentCategory( int ) ) );
    }
    return g;
}

void Frame::setStyle( const QString& style )
{
    QStyle *s = QStyleFactory::create( style );
    if ( s )
	QApplication::setStyle( s );
}

void Frame::updateTranslators()
{
    if ( !qt_translator ) {
	qt_translator = new QTranslator( qApp );
	translator = new QTranslator( qApp );
	qApp->installTranslator( qt_translator );
	qApp->installTranslator( translator );
    }

    QString base = QDir("../../translations").absPath();
    qt_translator->load( QString( "qt_%1" ).arg( QTextCodec::locale() ), base );
    translator->load( QString( "translations/demo_%1" ).arg( QTextCodec::locale() ) );
}

bool Frame::event( QEvent *e )
{
    if ( e->type() == QEvent::LocaleChange )
	updateTranslators();

    return QMainWindow::event( e );
}
