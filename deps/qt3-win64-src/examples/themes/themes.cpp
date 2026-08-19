/****************************************************************************
** $Id: themes.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "themes.h"
#include "wood.h"
#include "metal.h"

#include "../buttongroups/buttongroups.h"
#include "../lineedits/lineedits.h"
#include "../listboxcombo/listboxcombo.h"
#include "../checklists/checklists.h"
#include "../progressbar/progressbar.h"
#include "../rangecontrols/rangecontrols.h"
#include "../richtext/richtext.h"

#include <qtabwidget.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qfont.h>
#include <qstylefactory.h>
#include <qaction.h>
#include <qsignalmapper.h>
#include <qdict.h>

Themes::Themes( QWidget *parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f )
{
    appFont = QApplication::font();
    tabwidget = new QTabWidget( this );

    tabwidget->addTab( new ButtonsGroups( tabwidget ), "Buttons/Groups" );
    QHBox *hbox = new QHBox( tabwidget );
    hbox->setMargin( 5 );
    (void)new LineEdits( hbox );
    (void)new ProgressBar( hbox );
    tabwidget->addTab( hbox, "Lineedits/Progressbar" );
    tabwidget->addTab( new ListBoxCombo( tabwidget ), "Listboxes/Comboboxes" );
    tabwidget->addTab( new CheckLists( tabwidget ), "Listviews" );
    tabwidget->addTab( new RangeControls( tabwidget ), "Rangecontrols" );
    tabwidget->addTab( new MyRichText( tabwidget ), "Fortune" );

    setCentralWidget( tabwidget );

    QPopupMenu *style = new QPopupMenu( this );
    style->setCheckable( TRUE );
    menuBar()->insertItem( "&Style" , style );

    style->setCheckable( TRUE );
    QActionGroup *ag = new QActionGroup( this, 0 );
    ag->setExclusive( TRUE );
    QSignalMapper *styleMapper = new QSignalMapper( this );
    connect( styleMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( makeStyle( const QString& ) ) );
    QStringList list = QStyleFactory::keys();
    list.sort();
#ifndef QT_NO_STYLE_WINDOWS
    list.insert(list.begin(), "Norwegian Wood");
    list.insert(list.begin(), "Metal");
#endif
    QDict<int> stylesDict( 17, FALSE );
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
	QString styleStr = *it;
	QString styleAccel = styleStr;
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
	QAction *a = new QAction( styleStr, QIconSet(), styleAccel, 0, ag, 0, ag->isExclusive() );
	connect( a, SIGNAL( activated() ), styleMapper, SLOT(map()) );
	styleMapper->setMapping( a, a->text() );
    }
    ag->addTo(style);
    style->insertSeparator();
    style->insertItem("&Quit", qApp, SLOT( quit() ), CTRL | Key_Q );

    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( "&Help", help );
    help->insertItem( "&About", this, SLOT(about()), Key_F1);
    help->insertItem( "About &Qt", this, SLOT(aboutQt()));

#ifndef QT_NO_STYLE_WINDOWS
    qApp->setStyle( new NorwegianWoodStyle );
#endif
}

void Themes::makeStyle(const QString &style)
{
    if(style == "Norwegian Wood") {
#ifndef QT_NO_STYLE_WINDOWS
	qApp->setStyle( new NorwegianWoodStyle );
#endif
    } else if( style == "Metal" ) {
#ifndef QT_NO_STYLE_WINDOWS
	qApp->setStyle( new MetalStyle );
#endif
    } else {
	qApp->setStyle(style);
	if(style == "Platinum") {
	    QPalette p( QColor( 239, 239, 239 ) );
	    qApp->setPalette( p, TRUE );
	    qApp->setFont( appFont, TRUE );
	} else if(style == "Windows") {
	    qApp->setFont( appFont, TRUE );
	} else if(style == "CDE") {
	    QPalette p( QColor( 75, 123, 130 ) );
	    p.setColor( QPalette::Active, QColorGroup::Base, QColor( 55, 77, 78 ) );
	    p.setColor( QPalette::Inactive, QColorGroup::Base, QColor( 55, 77, 78 ) );
	    p.setColor( QPalette::Disabled, QColorGroup::Base, QColor( 55, 77, 78 ) );
	    p.setColor( QPalette::Active, QColorGroup::Highlight, Qt::white );
	    p.setColor( QPalette::Active, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	    p.setColor( QPalette::Inactive, QColorGroup::Highlight, Qt::white );
	    p.setColor( QPalette::Inactive, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	    p.setColor( QPalette::Disabled, QColorGroup::Highlight, Qt::white );
	    p.setColor( QPalette::Disabled, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	    p.setColor( QPalette::Active, QColorGroup::Foreground, Qt::white );
	    p.setColor( QPalette::Active, QColorGroup::Text, Qt::white );
	    p.setColor( QPalette::Active, QColorGroup::ButtonText, Qt::white );
	    p.setColor( QPalette::Inactive, QColorGroup::Foreground, Qt::white );
	    p.setColor( QPalette::Inactive, QColorGroup::Text, Qt::white );
	    p.setColor( QPalette::Inactive, QColorGroup::ButtonText, Qt::white );
	    p.setColor( QPalette::Disabled, QColorGroup::Foreground, Qt::lightGray );
	    p.setColor( QPalette::Disabled, QColorGroup::Text, Qt::lightGray );
	    p.setColor( QPalette::Disabled, QColorGroup::ButtonText, Qt::lightGray );
	    qApp->setPalette( p, TRUE );
	    qApp->setFont( QFont( "times", appFont.pointSize() ), TRUE );
	} else if(style == "Motif" || style == "MotifPlus") {
	    QPalette p( QColor( 192, 192, 192 ) );
	    qApp->setPalette( p, TRUE );
	    qApp->setFont( appFont, TRUE );
	}
    }
}

void Themes::about()
{
    QMessageBox::about( this, "Qt Themes Example",
			"<p>This example demonstrates the concept of "
			"<b>generalized GUI styles </b> first introduced "
			" with the 2.0 release of Qt.</p>" );
}


void Themes::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Themes Example" );
}


