/****************************************************************************
** $Id: tabletstats.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <math.h>

#include "tabletstats.h"

MyOrientation::MyOrientation( QWidget *parent, const char *name )
    : QFrame( parent, name, WRepaintNoErase )
{
//    QSizePolicy mySize( QSizePolicy::Minimum, QSizePolicy::Expanding );
//    setSizePolicy( mySize );
    setFrameStyle( QFrame::Box | QFrame::Sunken );
}

MyOrientation::~MyOrientation()
{
}

void MyOrientation::newOrient( int tiltX, int tiltY )
{
    double PI = 3.14159265359;
    int realWidth,
        realHeight,
        hypot,	// a faux hypoteneus, to mess with calculations
        shaX,
	shaY;
    static int oldX = 0,
	       oldY = 0;
    realWidth = width() - 2 * frameWidth();
    realHeight = height() - 2 * frameWidth();



    QRect cr( 0 + frameWidth(), 0 + frameWidth(), realWidth, realHeight );
    QPixmap pix( cr.size() );
    pix.fill( this, cr.topLeft() );
    QPainter p( &pix );

    if ( realWidth > realHeight )
	hypot = realHeight / 2;
    else
	hypot = realWidth / 2;

    // create a shadow...
    shaX = int(hypot * sin( tiltX * (PI / 180) ));
    shaY = int(hypot * sin( tiltY * (PI / 180) ));

    p.translate( realWidth / 2, realHeight / 2 );
    p.setPen( backgroundColor() );
    p.drawLine( 0, 0, oldX, oldY );
    p.setPen( foregroundColor() );
    p.drawLine( 0, 0,shaX, shaY );
    oldX = shaX;
    oldY = shaY;
    p.end();

    QPainter p2( this );
    p2.drawPixmap( cr.topLeft(), pix );
    p2.end();
}


StatsCanvas::StatsCanvas( QWidget *parent, const char* name )
  : Canvas( parent, name, WRepaintNoErase )
{
    QSizePolicy mySize( QSizePolicy::Expanding, QSizePolicy::Minimum );
    setSizePolicy( mySize );
}

StatsCanvas::~StatsCanvas()
{
}

void StatsCanvas::tabletEvent( QTabletEvent *e )
{
    static QRect oldR( -1, -1, -1, -1);
    QPainter p;

    e->accept();
    switch( e->type() ) {
    case QEvent::TabletPress:
	qDebug( "Tablet Press" );
	mousePressed = TRUE;
	break;
    case QEvent::TabletRelease:
	qDebug( "Tablet Release" );
	mousePressed = FALSE;
	clearScreen();
	break;
    default:
	break;
    }

    r.setRect( e->x() - e->pressure() / 2,
	e->y() - e->pressure() / 2, e->pressure(), e->pressure() );
    QRect tmpR = r | oldR;
    oldR = r;

    update( tmpR );
    emit signalNewTilt( e->xTilt(), e->yTilt() );
    emit signalNewDev( e->device() );
    emit signalNewLoc( e->x(), e->y() );
    emit signalNewPressure( e->pressure() );
}

void StatsCanvas::mouseMoveEvent( QMouseEvent *e )
{
    qDebug( "Mouse Move" );
    // do nothing
    QWidget::mouseMoveEvent( e );
}


void StatsCanvas::mousePressEvent( QMouseEvent *e )
{
    qDebug( "Mouse Press" );
    QWidget::mousePressEvent( e );
}

void StatsCanvas::mouseReleaseEvent( QMouseEvent *e )
{
    qDebug( "Mouse Release" );
    QWidget::mouseReleaseEvent( e );
}

void StatsCanvas::paintEvent( QPaintEvent *e )
{
    QPainter p;
    p.begin( &buffer );
    p.fillRect( e->rect(), colorGroup().base() );

    // draw a circle if we have the tablet down
    if ( mousePressed ) {
	p.setBrush( red );
	p.drawEllipse( r );
    }
    bitBlt( this, e->rect().x(), e->rect().y(), &buffer, e->rect().x(),
	    e->rect().y(), e->rect().width(), e->rect().height() );
    p.end();
}

TabletStats::TabletStats( QWidget *parent, const char *name )
	: TabletStatsBase( parent, name )
{
    lblXPos->setMinimumSize( lblXPos->sizeHint() );
    lblYPos->setMinimumSize( lblYPos->sizeHint() );
    lblPressure->setMinimumSize( lblPressure->sizeHint() );
    lblDev->setMinimumSize( lblDev->sizeHint() );
    lblXTilt->setMinimumSize( lblXTilt->sizeHint() );
    lblYTilt->setMinimumSize( lblYTilt->sizeHint() );

    QObject::connect( statCan, SIGNAL(signalNewTilt(int, int)),
	              orient, SLOT(newOrient(int, int)) );
    QObject::connect( statCan, SIGNAL(signalNewTilt(int, int)),
	              this, SLOT(slotTiltChanged(int, int)) );
    QObject::connect( statCan, SIGNAL(signalNewDev(int)),
                      this, SLOT(slotDevChanged(int)) );
    QObject::connect( statCan, SIGNAL(signalNewLoc(int,int)),
                      this, SLOT( slotLocationChanged(int,int)) );
}

TabletStats::~TabletStats()
{
}

void TabletStats::slotDevChanged( int newDev )
{
    if ( newDev == QTabletEvent::Stylus )
	lblDev->setText( tr("Stylus") );
    else if ( newDev == QTabletEvent::Eraser )
	lblDev->setText( tr("Eraser") );
}

void TabletStats::slotLocationChanged( int newX, int newY )
{
    lblXPos->setNum( newX );
    lblYPos->setNum( newY );
}

void TabletStats::slotTiltChanged( int newTiltX, int newTiltY )
{
    lblXTilt->setNum( newTiltX );
    lblYTilt->setNum( newTiltY );
}
