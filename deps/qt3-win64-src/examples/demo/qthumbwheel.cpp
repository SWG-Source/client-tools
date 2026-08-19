/****************************************************************************
** $Id: qthumbwheel.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of QThumbWheel class
**
** Created : 010205
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qthumbwheel.h"

#ifndef QT_NO_THUMBWHEEL
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <math.h>

static const double m_pi = 3.14159265358979323846;
static const double rad_factor = 180.0 / m_pi;

QThumbWheel::QThumbWheel( QWidget *parent, const char *name )
    : QFrame( parent, name )
{
    orient = Vertical;
    init();
}

/*!
  Destructs the wheel.
*/

QThumbWheel::~QThumbWheel()
{
}

/*!
  \internal
 */

void QThumbWheel::init()
{
    track = TRUE;
    mousePressed = FALSE;
    pressedAt = -1;
    rat = 1.0;
    setFrameStyle( WinPanel | Sunken );
    setMargin( 2 );
    setFocusPolicy( WheelFocus );
}

void QThumbWheel::setOrientation( Orientation orientation )
{
    orient = orientation;
    update();
}

void QThumbWheel::setTracking( bool enable )
{
    track = enable;
}

void QThumbWheel::setTransmissionRatio( double r )
{
    rat = r;
}

/*!
  Makes QRangeControl::setValue() available as a slot.
*/

void QThumbWheel::setValue( int value )
{
    QRangeControl::setValue( value );
}

void QThumbWheel::valueChange()
{
    repaint( FALSE );
    emit valueChanged(value());
}

void QThumbWheel::rangeChange()
{
}

void QThumbWheel::stepChange()
{
}

/*!
  \reimp
*/

void QThumbWheel::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Left:
	if ( orient == Horizontal )
	    subtractLine();
	break;
    case Key_Right:
	if ( orient == Horizontal )
	    addLine();
	break;
    case Key_Up:
	if ( orient == Vertical )
	    subtractLine();
	break;
    case Key_Down:
	if ( orient == Vertical )
	    addLine();
	break;
    case Key_PageUp:
	subtractPage();
	break;
    case Key_PageDown:
	addPage();
	break;
    case Key_Home:
	setValue( minValue() );
	break;
    case Key_End:
	setValue( maxValue() );
	break;
    default:
	e->ignore();
	return;
    };
}

/*!
  \reimp
*/

void QThumbWheel::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton ) {
	mousePressed = TRUE;
	pressedAt = valueFromPosition( e->pos() );
    }
}

/*!
  \reimp
*/

void QThumbWheel::mouseReleaseEvent( QMouseEvent *e )
{
    int movedTo = valueFromPosition( e->pos() );
    setValue( value() + movedTo - pressedAt );
    pressedAt = movedTo;
}

/*!
  \reimp
*/

void QThumbWheel::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    if ( track ) {
	int movedTo = valueFromPosition( e->pos() );
	setValue( value() + movedTo - pressedAt );
	pressedAt = movedTo;
    }
}

/*!
  \reimp
*/

void QThumbWheel::wheelEvent( QWheelEvent *e )
{
    int step = ( e->state() & ControlButton ) ? lineStep() : pageStep();
    setValue( value() - e->delta()*step/120 );
    e->accept();
}

/*!\reimp
*/

void QThumbWheel::focusInEvent( QFocusEvent *e )
{
    QWidget::focusInEvent( e );
}

/*!\reimp
*/

void QThumbWheel::focusOutEvent( QFocusEvent *e )
{
    QWidget::focusOutEvent( e );
}

void QThumbWheel::drawContents( QPainter *p )
{
    QRect cr = contentsRect();
    // double buffer
    QPixmap pix( width(), height() );
    QPainter pt( &pix );
    QBrush brush = backgroundPixmap() ?
		QBrush( backgroundColor(), *backgroundPixmap() ) : QBrush( backgroundColor() );
    pt.fillRect( cr, brush );

    const int n = 17;
    const double delta = m_pi / double(n);
    // ### use positionFromValue() with rad*16 or similar
    double alpha = 2*m_pi*double(value()-minValue())/
		   double(maxValue()-minValue())*transmissionRatio();
    alpha = fmod(alpha, delta);
    QPen pen0( colorGroup().midlight() );
    QPen pen1( colorGroup().dark() );

    if ( orient == Horizontal ) {
	double r = 0.5*cr.width();
	int y0 = cr.y()+1;
	int y1 = cr.bottom()-1;
	for ( int i = 0; i < n; i++ ) {
	    int x = cr.x() + int((1-cos(delta*double(i)+alpha))*r);
	    pt.setPen( pen0 );
	    pt.drawLine( x, y0, x, y1 );
	    pt.setPen( pen1 );
	    pt.drawLine( x+1, y0, x+1, y1 );
	}
    } else {
	// vertical orientation
	double r = 0.5*cr.height();
	int x0 = cr.x()+1;
	int x1 = cr.right()-1;
	for ( int i = 0; i < n; i++ ) {
	    int y = cr.y() + int((1-cos(delta*double(i)+alpha))*r);
	    pt.setPen( pen0 );
	    pt.drawLine( x0, y, x1, y );
	    pt.setPen( pen1 );
	    pt.drawLine( x0, y+1, x1, y+1 );
	}
    }
    qDrawShadePanel( &pt, cr, colorGroup());

    pt.end();
    p->drawPixmap( 0, 0, pix );
}

int QThumbWheel::valueFromPosition( const QPoint &p )
{
    QRect wrec = contentsRect();
    int pos, min, max;
    if ( orient == Horizontal ) {
	pos = p.x();
	min = wrec.left();
	max = wrec.right();
    } else {
	pos = p.y();
	min = wrec.top();
	max = wrec.bottom();
    }
    double alpha;
    if ( pos < min )
	alpha = 0;
    else if ( pos > max )
	alpha = m_pi;
    else
	alpha = acos( 1.0 - 2.0*double(pos-min)/double(max-min) );// ### taylor
    double deg = alpha*rad_factor/transmissionRatio();
    // ### use valueFromPosition()
    return minValue() + int((maxValue()-minValue())*deg/360.0);
}

#endif
