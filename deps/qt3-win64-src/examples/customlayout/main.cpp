/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Main for custom layout example
**
** Copyright (C) 1996-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "flow.h"
#include "border.h"
#include "card.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qcolor.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget *f = new QWidget;
    QBoxLayout *gm = new QVBoxLayout( f, 5 );

    SimpleFlow *b1 = new SimpleFlow( gm );

    b1->add( new QPushButton( "Short", f ) );
    b1->add( new QPushButton( "Longer", f ) );
    b1->add( new QPushButton( "Different text", f ) );
    b1->add( new QPushButton( "More text", f ) );
    b1->add( new QPushButton( "Even longer button text", f ) );
    QPushButton* qb = new QPushButton( "Quit", f );
    a.connect( qb, SIGNAL( clicked() ), SLOT( quit() ) );
    b1->add( qb );

    QWidget *wid = new QWidget( f );

    BorderLayout *large = new BorderLayout( wid );
    large->setSpacing( 5 );
    large->addWidget( new QPushButton( "North", wid ), BorderLayout::North );
    large->addWidget( new QPushButton( "West", wid ), BorderLayout::West );
    QMultiLineEdit* m = new QMultiLineEdit( wid );
    m->setText( "Central\nWidget" );
    large->addWidget( m, BorderLayout::Center );
    QWidget *east1 = new QPushButton( "East", wid );
    large->addWidget( east1, BorderLayout::East );
    QWidget *east2 = new QPushButton( "East 2", wid );
    large->addWidget( east2 , BorderLayout::East );
    large->addWidget( new QPushButton( "South", wid ), BorderLayout::South );
    //Left-to-right tab order looks better:
    QWidget::setTabOrder( east2, east1 );
    gm->addWidget( wid );


    wid = new QWidget( f );
    CardLayout *card = new CardLayout( wid, 10 );

    QWidget *crd = new QWidget( wid );
    crd->setBackgroundColor( Qt::red );
    card->add( crd );
    crd = new QWidget( wid );
    crd->setBackgroundColor( Qt::green );
    card->add( crd );
    crd = new QWidget( wid );
    crd->setBackgroundColor( Qt::blue );
    card->add( crd );
    crd = new QWidget( wid );
    crd->setBackgroundColor( Qt::white );
    card->add( crd );
    crd = new QWidget( wid );
    crd->setBackgroundColor( Qt::black );
    card->add( crd );
    crd = new QWidget( wid );
    crd->setBackgroundColor( Qt::yellow );
    card->add( crd );

    gm->addWidget( wid );

    QLabel* s = new QLabel( f );
    s->setText( "outermost box" );
    s->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    gm->addWidget( s );
    a.setMainWidget( f );
    f->setCaption("Qt Example - Custom Layout");
    f->show();

    int result = a.exec();
    delete f;
    return result;
}
