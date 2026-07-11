/****************************************************************************
** $Id: qcdestyle.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of CDE-like style class
**
** Created : 981231
**
** Copyright (C) 1998-2007 Trolltech ASA.  All rights reserved.
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

#include "qcdestyle.h"

#if !defined(QT_NO_STYLE_CDE) || defined(QT_PLUGIN)

#include "qpainter.h"
#include "qdrawutil.h"
#include "qbutton.h"
#include <limits.h>

/*!
    \class QCDEStyle qcdestyle.h
    \brief The QCDEStyle class provides a CDE look and feel.

    \ingroup appearance

    This style provides a slightly improved Motif look similar to some
    versions of the Common Desktop Environment (CDE). The main
    differences are thinner frames and more modern radio buttons and
    checkboxes. Together with a dark background and a bright
    text/foreground color, the style looks quite attractive (at least
    for Motif fans).

    Note that the functions provided by QCDEStyle are
    reimplementations of QStyle functions; see QStyle for their
    documentation.
*/

/*!
    Constructs a QCDEStyle.

    If \a useHighlightCols is FALSE (the default), then the style will
    polish the application's color palette to emulate the Motif way of
    highlighting, which is a simple inversion between the base and the
    text color.
*/
QCDEStyle::QCDEStyle( bool useHighlightCols ) : QMotifStyle( useHighlightCols )
{
}

/*!
    Destroys the style.
*/
QCDEStyle::~QCDEStyle()
{
}


/*!\reimp
*/
int QCDEStyle::pixelMetric( PixelMetric metric, const QWidget *widget ) const
{
    int ret;

    switch( metric ) {
    case PM_DefaultFrameWidth:
	ret = 1;
	break	;
    case PM_MenuBarFrameWidth:
	ret = 1;
	break;
    case PM_ScrollBarExtent:
	ret = 13;
	break;
    default:
	ret = QMotifStyle::pixelMetric( metric, widget );
	break;
    }
    return ret;
}

/*! \reimp
*/
void QCDEStyle::drawControl( ControlElement element,
			     QPainter *p,
			     const QWidget *widget,
			     const QRect &r,
			     const QColorGroup &cg,
			     SFlags how,
			     const QStyleOption& opt ) const
{

    switch( element ) {
    case CE_MenuBarItem:
	{
	    if ( how & Style_Active )  // active item
		qDrawShadePanel( p, r, cg, TRUE, 1,
				 &cg.brush( QColorGroup::Button ) );
	    else  // other item
		p->fillRect( r, cg.brush( QColorGroup::Button ) );
	    QCommonStyle::drawControl( element, p, widget, r, cg, how, opt );
	    break;
	}
    default:
	QMotifStyle::drawControl( element, p, widget, r, cg, how, opt );
    break;
    }


}

/*! \reimp
*/
void QCDEStyle::drawPrimitive( PrimitiveElement pe,
			       QPainter *p,
			       const QRect &r,
			       const QColorGroup &cg,
			       SFlags flags,
			       const QStyleOption& opt ) const
{
    switch( pe ) {
    case PE_Indicator: {
#ifndef QT_NO_BUTTON
	bool down = flags & Style_Down;
	bool on = flags & Style_On;
	bool showUp = !( down ^ on );
	QBrush fill = showUp || flags & Style_NoChange ? cg.brush( QColorGroup::Button ) : cg.brush( QColorGroup::Mid );
	qDrawShadePanel( p, r, cg, !showUp, pixelMetric( PM_DefaultFrameWidth ), &cg.brush( QColorGroup::Button ) );

	if ( !( flags & Style_Off ) ) {
	    QPointArray a( 7 * 2 );
	    int i, xx, yy;
	    xx = r.x() + 3;
	    yy = r.y() + 5;
	    for ( i = 0; i < 3; i++ ) {
		a.setPoint( 2 * i,   xx, yy );
		a.setPoint( 2 * i + 1, xx, yy + 2 );
		xx++; yy++;
	    }
	    yy -= 2;
	    for ( i = 3; i < 7; i++ ) {
		a.setPoint( 2 * i, xx, yy );
		a.setPoint( 2 * i + 1, xx, yy + 2 );
		xx++; yy--;
	    }
	    if ( flags & Style_NoChange )
		p->setPen( cg.dark() );
	    else
		p->setPen( cg.foreground() );
	    p->drawLineSegments( a );
	}
#endif
    }
	break;
    case PE_ExclusiveIndicator:
	{
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
	    static const QCOORD pts1[] = {              // up left  lines
		1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
	    static const QCOORD pts4[] = {              // bottom right  lines
		2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
		11,4, 10,3, 10,2 };
	    static const QCOORD pts5[] = {              // inner fill
		4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
	    bool down = flags & Style_Down;
	    bool on = flags & Style_On;
	    p->eraseRect( r );
	    QPointArray a( QCOORDARRLEN(pts1), pts1 );
	    a.translate( r.x(), r.y() );
	    p->setPen( ( down || on ) ? cg.dark() : cg.light() );
	    p->drawPolyline( a );
	    a.setPoints( QCOORDARRLEN(pts4), pts4 );
	    a.translate( r.x(), r.y() );
	    p->setPen( ( down || on ) ? cg.light() : cg.dark() );
	    p->drawPolyline( a );
	    a.setPoints( QCOORDARRLEN(pts5), pts5 );
	    a.translate( r.x(), r.y() );
	    QColor fillColor = on ? cg.dark() : cg.background();
	    p->setPen( fillColor );
	    p->setBrush( on ? cg.brush( QColorGroup::Dark ) :
			 cg.brush( QColorGroup::Background ) );
	    p->drawPolygon( a );
	    break;
	}

    case PE_ExclusiveIndicatorMask:
	{
	    static const QCOORD pts1[] = {
		// up left  lines
		1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1,
		// bottom right  lines
		10,2, 10,3, 11,4, 11,7, 10,8, 10,9, 9,10, 8,10, 7,11, 4,11, 3,10, 2,10
	    };
	    QPointArray a(QCOORDARRLEN(pts1), pts1);
	    a.translate(r.x(), r.y());
	    p->setPen(color1);
	    p->setBrush(color1);
	    p->drawPolygon(a);
	    break;
	}
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
	QRect rect = r;
	QPointArray bFill;                          // fill polygon
	QPointArray bTop;                           // top shadow.
	QPointArray bBot;                           // bottom shadow.
	QPointArray bLeft;                          // left shadow.
	QWMatrix    matrix;                         // xform matrix
	bool vertical = pe == PE_ArrowUp || pe == PE_ArrowDown;
	bool horizontal = !vertical;
	int  dim = rect.width() < rect.height() ? rect.width() : rect.height();
	int  colspec = 0x0000;                      // color specification array

	if ( dim < 2 )                              // too small arrow
	    return;

	// adjust size and center (to fix rotation below)
	if ( rect.width() >  dim ) {
	    rect.setX( rect.x() + ( ( rect.width() - dim ) / 2 ) );
	    rect.setWidth( dim );
	}
	if ( rect.height() > dim ) {
	    rect.setY( rect.y() + ( ( rect.height() - dim ) / 2 ) );
	    rect.setHeight( dim );
	}

	if ( dim > 3 ) {
	    bFill.resize( dim & 1 ? 3 : 4 );
	    bTop.resize( 2 );
	    bBot.resize( 2 );
	    bLeft.resize( 2 );
	    bLeft.putPoints( 0, 2, 0, 0, 0, dim-1 );
	    bTop.putPoints( 0, 2, 1, 0, dim-1, dim/2 );
	    bBot.putPoints( 0, 2, 1, dim-1, dim-1, dim/2 );

	    if ( dim > 6 ) {                        // dim>6: must fill interior
		bFill.putPoints( 0, 2, 1, dim-1, 1, 1 );
		if ( dim & 1 )                      // if size is an odd number
		    bFill.setPoint( 2, dim - 2, dim / 2 );
		else
		    bFill.putPoints( 2, 2, dim-2, dim/2-1, dim-2, dim/2 );
	    }
	} else {
	    if ( dim == 3 ) {                       // 3x3 arrow pattern
		bLeft.setPoints( 4, 0,0, 0,2, 1,1, 1,1 );
		bTop .setPoints( 2, 1,0, 1,0 );
		bBot .setPoints( 2, 1,2, 2,1 );
	    } else {                                  // 2x2 arrow pattern
		bLeft.setPoints( 2, 0,0, 0,1 );
		bTop .setPoints( 2, 1,0, 1,0 );
		bBot .setPoints( 2, 1,1, 1,1 );
	    }
	}

	if ( pe == PE_ArrowUp || pe == PE_ArrowLeft ) {
	    matrix.translate( rect.x(), rect.y() );
	    if ( vertical ) {
		matrix.translate( 0, rect.height() - 1 );
		matrix.rotate( -90 );
	    } else {
		matrix.translate( rect.width() - 1, rect.height() - 1 );
		matrix.rotate( 180 );
	    }
	    if ( flags & Style_Down )
		colspec = horizontal ? 0x2334 : 0x2343;
	    else
		colspec = horizontal ? 0x1443 : 0x1434;
	} else if ( pe == PE_ArrowDown || pe == PE_ArrowRight ) {
	    matrix.translate( rect.x(), rect.y() );
	    if ( vertical ) {
		matrix.translate( rect.width()-1, 0 );
		matrix.rotate( 90 );
	    }
	    if ( flags & Style_Down )
		colspec = horizontal ? 0x2443 : 0x2434;
	    else
		colspec = horizontal ? 0x1334 : 0x1343;
	}

	QColor *cols[5];
	if ( flags & Style_Enabled ) {
	    cols[0] = 0;
	    cols[1] = (QColor *)&cg.button();
	    cols[2] = (QColor *)&cg.mid();
	    cols[3] = (QColor *)&cg.light();
	    cols[4] = (QColor *)&cg.dark();
	} else {
	    cols[0] = 0;
	    cols[1] = (QColor *)&cg.button();
	    cols[2] = (QColor *)&cg.button();
	    cols[3] = (QColor *)&cg.button();
	    cols[4] = (QColor *)&cg.button();
	}

#define CMID    *cols[ (colspec>>12) & 0xf ]
#define CLEFT   *cols[ (colspec>>8) & 0xf ]
#define CTOP    *cols[ (colspec>>4) & 0xf ]
#define CBOT    *cols[ colspec & 0xf ]

	QPen     savePen   = p->pen();              // save current pen
	QBrush   saveBrush = p->brush();            // save current brush
	QWMatrix wxm = p->worldMatrix();
	QPen     pen( NoPen );
	QBrush brush = cg.brush( flags & Style_Enabled ? QColorGroup::Button :
				 QColorGroup::Mid );

	p->setPen( pen );
	p->setBrush( brush );
	p->setWorldMatrix( matrix, TRUE );          // set transformation matrix
	p->drawPolygon( bFill );                    // fill arrow
	p->setBrush( NoBrush );                     // don't fill

	p->setPen( CLEFT );
	p->drawLineSegments( bLeft );
	p->setPen( CBOT );
	p->drawLineSegments( bBot );
	p->setPen( CTOP );
	p->drawLineSegments( bTop );

	p->setWorldMatrix( wxm );
	p->setBrush( saveBrush );                   // restore brush
	p->setPen( savePen );                       // restore pen

#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT

    }
	break;
    default:
	QMotifStyle::drawPrimitive( pe, p, r, cg, flags, opt );
    }
}

#endif
