/****************************************************************************
** $Id: canvas.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "canvas.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qevent.h>
#include <qrect.h>

const bool no_writing = FALSE;

Canvas::Canvas( QWidget *parent, const char *name, WFlags fl )
    : QWidget( parent, name, WStaticContents | fl ),
      pen( Qt::red, 3 ), polyline(3),
      mousePressed( FALSE ), oldPressure( 0 ), saveColor( red ),
      buffer( width(), height() )
{

    if ((qApp->argc() > 0) && !buffer.load(qApp->argv()[1]))
	buffer.fill( colorGroup().base() );
    setBackgroundMode( QWidget::PaletteBase );
#ifndef QT_NO_CURSOR
    setCursor( Qt::crossCursor );
#endif
}

void Canvas::save( const QString &filename, const QString &format )
{
    if ( !no_writing )
	buffer.save( filename, format.upper() );
}

void Canvas::clearScreen()
{
    buffer.fill( colorGroup().base() );
    repaint( FALSE );
}

void Canvas::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    polyline[2] = polyline[1] = polyline[0] = e->pos();
}

void Canvas::mouseReleaseEvent( QMouseEvent * )
{
    mousePressed = FALSE;
}

void Canvas::mouseMoveEvent( QMouseEvent *e )
{
    if ( mousePressed ) {
	QPainter painter;
	painter.begin( &buffer );
	painter.setPen( pen );
	polyline[2] = polyline[1];
	polyline[1] = polyline[0];
	polyline[0] = e->pos();
	painter.drawPolyline( polyline );
	painter.end();

	QRect r = polyline.boundingRect();
	r = r.normalize();
	r.setLeft( r.left() - penWidth() );
	r.setTop( r.top() - penWidth() );
	r.setRight( r.right() + penWidth() );
	r.setBottom( r.bottom() + penWidth() );

	bitBlt( this, r.x(), r.y(), &buffer, r.x(), r.y(), r.width(), r.height() );
    }
}

void Canvas::tabletEvent( QTabletEvent *e )
{
    e->accept();
    // change the width based on range of pressure
    if ( e->device() == QTabletEvent::Stylus )	{
	if ( e->pressure() >= 0 && e->pressure() <= 32 )
	    pen.setColor( saveColor.light(175) );
	else if ( e->pressure() > 32 && e->pressure() <= 64 )
	    pen.setColor( saveColor.light(150) );
	else if ( e->pressure() > 64  && e->pressure() <= 96 )
	    pen.setColor( saveColor.light(125) );
	else if ( e->pressure() > 96 && e->pressure() <= 128 )
	    pen.setColor( saveColor );
	else if ( e->pressure() > 128 && e->pressure() <= 160 )
	    pen.setColor( saveColor.dark(150) );
	else if ( e->pressure() > 160 && e->pressure() <= 192 )
	    pen.setColor( saveColor.dark(200) );
	else if ( e->pressure() > 192 && e->pressure() <= 224 )
	    pen.setColor( saveColor.dark(250) );
	else // pressure > 224
	    pen.setColor( saveColor.dark(300) );
    } else if ( e->device() == QTabletEvent::Eraser
		&& pen.color() != backgroundColor() ) {
	pen.setColor( backgroundColor() );
    }

    int xt = e->xTilt();
    int yt = e->yTilt();
    if ( ( xt > -15 && xt < 15 ) && ( yt > -15 && yt < 15 ) )
	pen.setWidth( 3 );
    else if ( ((xt < -15 && xt > -30) || (xt > 15 && xt < 30)) &&
	      ((yt < -15 && yt > -30) || (yt > 15 && yt < 30 )) )
	pen.setWidth( 6 );
    else if ( ((xt < -30 && xt > -45) || (xt > 30 && xt < 45)) &&
	      ((yt < -30 && yt > -45) || (yt > 30 && yt < 45)) )
	pen.setWidth( 9 );
    else if (  (xt < -45 || xt > 45 ) && ( yt < -45 || yt > 45 ) )
	pen.setWidth( 12 );

    switch ( e->type() ) {
    case QEvent::TabletPress:
	mousePressed = TRUE;
	polyline[2] = polyline[1] = polyline[0] = e->pos();
	break;
    case QEvent::TabletRelease:
	mousePressed = FALSE;
	break;
    case QEvent::TabletMove:
	if ( mousePressed ) {
	    QPainter painter;
	    painter.begin( &buffer );
	    painter.setPen( pen );
	    polyline[2] = polyline[1];
	    polyline[1] = polyline[0];
	    polyline[0] = e->pos();
	    painter.drawPolyline( polyline );
	    painter.end();

	    QRect r = polyline.boundingRect();
	    r = r.normalize();
	    r.setLeft( r.left() - penWidth() );
	    r.setTop( r.top() - penWidth() );
	    r.setRight( r.right() + penWidth() );
	    r.setBottom( r.bottom() + penWidth() );

	    bitBlt( this, r.x(), r.y(), &buffer, r.x(), r.y(), r.width(),
		    r.height() );
	}
	break;
    default:
	break;
    }
}

void Canvas::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );

    int w = width() > buffer.width() ?
	    width() : buffer.width();
    int h = height() > buffer.height() ?
	    height() : buffer.height();

    QPixmap tmp( buffer );
    buffer.resize( w, h );
    buffer.fill( colorGroup().base() );
    bitBlt( &buffer, 0, 0, &tmp, 0, 0, tmp.width(), tmp.height() );
}

void Canvas::paintEvent( QPaintEvent *e )
{
    QWidget::paintEvent( e );

    QMemArray<QRect> rects = e->region().rects();
    for ( uint i = 0; i < rects.count(); i++ ) {
	QRect r = rects[(int)i];
	bitBlt( this, r.x(), r.y(), &buffer, r.x(), r.y(), r.width(), r.height() );
    }
}
