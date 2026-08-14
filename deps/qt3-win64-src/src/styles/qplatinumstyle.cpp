/****************************************************************************
** $Id: qplatinumstyle.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of Platinum-like style class
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

#include "qplatinumstyle.h"

#if !defined(QT_NO_STYLE_PLATINUM) || defined(QT_PLUGIN)

#include "qapplication.h"
#include "qcombobox.h"
#include "qdrawutil.h"
#include "qpainter.h"
#include "qpalette.h"
#include "qpixmap.h"
#include "qpushbutton.h"
#include "qscrollbar.h"
#include "qslider.h"
#include <limits.h>

/*!
    \class QPlatinumStyle qplatinumstyle.h
    \brief The QPlatinumStyle class provides Mac/Platinum look and feel.

    \ingroup appearance

    This class implements the Platinum look and feel. It's an
    experimental class that tries to resemble a Macinosh-like GUI
    style with the QStyle system. The emulation is currently far from
    perfect.

    \sa QAquaStyle
*/


/*!
    Constructs a QPlatinumStyle
*/
QPlatinumStyle::QPlatinumStyle()
{
}

/*!\reimp
*/
QPlatinumStyle::~QPlatinumStyle()
{
}


/*!\reimp
 */
void QPlatinumStyle::drawPrimitive( PrimitiveElement pe,
				    QPainter *p,
				    const QRect &r,
				    const QColorGroup &cg,
				    SFlags flags,
				    const QStyleOption& opt ) const
{
    switch (pe) {
    case PE_HeaderSection:
	{
	    // adjust the sunken flag, otherwise headers are drawn
	    // sunken...
	    if ( flags & Style_Sunken )
		flags ^= Style_Sunken;
	    drawPrimitive( PE_ButtonBevel, p, r, cg, flags, opt );
	    break;
	}
    case PE_ButtonTool:
	{
	    // tool buttons don't change color when pushed in platinum,
	    // so we need to make the mid and button color the same
	    QColorGroup myCG = cg;
	    QBrush fill;

	    // quick trick to make sure toolbuttons drawn sunken
	    // when they are activated...
	    if ( flags & Style_On )
		flags |= Style_Sunken;

	    fill = myCG.brush( QColorGroup::Button );
	    myCG.setBrush( QColorGroup::Mid, fill );
	    drawPrimitive( PE_ButtonBevel, p, r, myCG, flags, opt );
	    break;
	}
    case PE_ButtonBevel:
	{
	    int x,
		y,
		w,
		h;
	    r.rect( &x, &y, &w, &h );

	    QPen oldPen = p->pen();
	    if ( w * h < 1600 ||
		 QABS(w - h) > 10 ) {
		// small buttons

		if ( !(flags & (Style_Sunken | Style_Down)) ) {
		    p->fillRect( x + 2, y + 2, w - 4, h - 4,
				 cg.brush(QColorGroup::Button) );
		    // the bright side
		    p->setPen( cg.dark() );
		    // the bright side
		    p->setPen( cg.dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.light() );
		    p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
		    p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );

		    // the dark side
		    p->setPen( cg.mid() );
		    p->drawLine( x + 2, y + h - 2, x + w - 2, y + h - 2 );
		    p->drawLine( x + w - 2, y + 2, x + w - 2, y + h - 3 );

		    p->setPen( cg.dark().dark() );
		    p->drawLine( x + 1, y + h - 1, x + w - 1,
				 y + h - 1 );
		    p->drawLine( x + w - 1, y + 1,
				 x + w - 1,
				 y + h - 2 );
		} else {
		    p->fillRect(x + 2, y + 2,
				w - 4, h - 4,
				cg.brush( QColorGroup::Mid ));

		    // the dark side
		    p->setPen( cg.dark().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.mid().dark());
		    p->drawLine( x + 1, y + 1,
				 x + w-2, y + 1);
		    p->drawLine( x + 1, y + 1,
				 x + 1, y + h - 2 );


		    // the bright side!

		    p->setPen(cg.button());
		    p->drawLine( x + 1, y + h - 2,
				 x + w - 2,
				 y + h - 2 );
		    p->drawLine( x + w - 2, y + 1,
				 x + w - 2,
				 y + h - 2 );
		    p->setPen(cg.dark());
		    p->drawLine(x, y + h - 1,
				x + w - 1,
				y + h - 1 );
		    p->drawLine(x + w - 1, y,
				x + w - 1,
				y + h - 1 );
		}
	    } else {
		// big ones
		if ( !(flags & (Style_Sunken | Style_Down)) ) {
		    p->fillRect( x + 3, y + 3, w - 6,
				 h - 6,
				 cg.brush(QColorGroup::Button) );

		    // the bright side
		    p->setPen( cg.button().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.button() );
		    p->drawLine( x + 1, y + 1,
				 x + w - 2, y + 1 );
		    p->drawLine( x + 1, y + 1,
				 x + 1, y + h - 2 );

		    p->setPen( cg.light() );
		    p->drawLine( x + 2, y + 2,
				 x + 2, y + h - 2 );
		    p->drawLine( x + 2, y + 2,
				 x + w - 2, y + 2 );
		    // the dark side!

		    p->setPen( cg.mid() );
		    p->drawLine( x + 3, y + h - 3,
				 x + w - 3,
				 y + h - 3 );
		    p->drawLine( x + w - 3, y + 3,
				 x + w - 3,
				 y + h - 3 );
		    p->setPen( cg.dark() );
		    p->drawLine( x + 2, y + h - 2,
				 x + w - 2,
				 y + h - 2 );
		    p->drawLine( x + w - 2, y + 2,
				 x + w - 2,
				 y + h - 2 );

		    p->setPen( cg.dark().dark() );
		    p->drawLine( x + 1, y + h - 1,
				 x + w - 1,
				 y + h - 1 );
		    p->drawLine( x + w - 1, y + 1,
				 x + w - 1,
				 y + h - 1 );
		} else {
		    p->fillRect( x + 3, y + 3, w - 6,
				 h - 6,
				 cg.brush( QColorGroup::Mid ) );

		    // the dark side
		    p->setPen( cg.dark().dark().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.dark().dark() );
		    p->drawLine( x + 1, y + 1,
				 x + w - 2, y + 1 );
		    p->drawLine( x + 1, y + 1,
				 x + 1, y + h - 2 );

		    p->setPen( cg.mid().dark() );
		    p->drawLine( x + 2, y + 2,
				 x + 2, y + w - 2 );
		    p->drawLine( x + 2, y + 2,
				 x + w - 2, y + 2 );


		    // the bright side!

		    p->setPen( cg.button() );
		    p->drawLine( x + 2, y + h - 3,
				 x + w - 3,
				 y + h - 3 );
		    p->drawLine( x + w - 3, y + 3,
				 x + w - 3,
				 y + h - 3 );

		    p->setPen( cg.midlight() );
		    p->drawLine( x + 1, y + h - 2,
				 x + w - 2,
				 y + h - 2 );
		    p->drawLine( x + w - 2, y + 1,
				 x + w - 2,
				 y + h - 2 );

		    p->setPen( cg.dark() );
		    p->drawLine( x, y + h - 1,
				 x + w - 1,
				 y + h - 1 );
		    p->drawLine( x + w - 1, y,
				 x + w - 1,
				 y + h - 1 );


		    // corners
		    p->setPen( mixedColor(cg.dark().dark().dark(),
					  cg.dark()) );
		    p->drawPoint( x, y + h - 1 );
		    p->drawPoint( x + w - 1, y );

		    p->setPen( mixedColor(cg.dark().dark(), cg.midlight()) );
		    p->drawPoint( x + 1, y + h - 2 );
		    p->drawPoint( x + w - 2, y + 1 );

		    p->setPen( mixedColor(cg.mid().dark(), cg.button() ) );
		    p->drawPoint( x + 2, y + h - 3 );
		    p->drawPoint( x + w - 3, y + 2 );
		}
	    }
	    p->setPen( oldPen );
	    break;
	}
    case PE_ButtonCommand:
	{
	    QPen oldPen = p->pen();
	    int x,
		y,
		w,
		h;
	    r.rect( &x, &y, &w, &h);

	    if ( !(flags & (Style_Down | Style_On)) ) {
		p->fillRect( x+3, y+3, w-6, h-6,
			     cg.brush( QColorGroup::Button ));
		// the bright side
		p->setPen( cg.shadow() );
		p->drawLine( x, y, x+w-1, y );
		p->drawLine( x, y, x, y + h - 1 );

		p->setPen( cg.button() );
		p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
		p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );

		p->setPen( cg.light() );
		p->drawLine( x + 2, y + 2, x + 2, y + h - 2 );
		p->drawLine( x + 2, y + 2, x + w - 2, y + 2 );


		// the dark side!

		p->setPen( cg.mid() );
		p->drawLine( x + 3, y + h - 3 ,x + w - 3, y + h - 3 );
		p->drawLine( x + w - 3, y + 3, x + w - 3, y + h - 3 );

		p->setPen( cg.dark() );
		p->drawLine( x + 2, y + h - 2, x + w - 2, y + h - 2 );
		p->drawLine( x + w - 2, y + 2, x + w - 2, y + h - 2 );

		p->setPen( cg.shadow() );
		p->drawLine( x + 1, y + h - 1, x + w - 1, y + h - 1 );
		p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );


		// top left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y );
		p->drawPoint( x + 1, y );
		p->drawPoint( x, y+1 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + 1, y + 1 );
		p->setPen( cg.button() );
		p->drawPoint( x + 2, y + 2 );
		p->setPen( white );
		p->drawPoint( x + 3, y + 3 );
		// bottom left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y + h - 1 );
		p->drawPoint( x + 1, y + h - 1 );
		p->drawPoint( x, y + h - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + 1, y + h - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( x + 2, y + h - 3 );
		// top right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w -1, y );
		p->drawPoint( x + w - 2, y );
		p->drawPoint( x + w - 1, y + 1 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + w - 2, y + 1 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 3, y + 2 );
		// bottom right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y + h - 1 );
		p->drawPoint( x + w - 2, y + h - 1 );
		p->drawPoint( x + w - 1, y + h - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + w - 2, y + h - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 3, y + h - 3 );
		p->setPen( cg.mid() );
		p->drawPoint( x + w - 4, y + h - 4 );

	    } else {
		p->fillRect( x + 2, y + 2, w - 4, h - 4,
			     cg.brush(QColorGroup::Dark) );

		// the dark side
		p->setPen( cg.shadow() );
		p->drawLine( x, y, x + w - 1, y );
		p->drawLine( x, y, x, y + h - 1 );

		p->setPen( cg.dark().dark() );
		p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
		p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );

		// the bright side!

		p->setPen( cg.button() );
		p->drawLine( x + 1, y + h - 2, x + w - 2, y + h - 2 );
		p->drawLine( x + w - 2, y + 1, x + w - 2, y + h - 2 );

		p->setPen( cg.dark() );
		p->drawLine( x, y + h - 1, x + w - 1, y + h - 1 );
		p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );

		// top left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y );
		p->drawPoint( x + 1, y );
		p->drawPoint( x, y + 1 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + 1, y + 1 );
		p->setPen( cg.dark().dark() );
		p->drawPoint( x + 3, y + 3 );
		// bottom left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y + h - 1 );
		p->drawPoint( x + 1, y + h - 1 );
		p->drawPoint( x, y + h - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + 1, y + h - 2 );
		// top right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y );
		p->drawPoint( x + w - 2, y );
		p->drawPoint( x + w - 1, y + 1 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + w - 2, y + 1 );
		// bottom right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y + h - 1 );
		p->drawPoint( x + w - 2, y + h - 1 );
		p->drawPoint( x + w - 1, y + h - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( x + w - 2, y + h - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 3, y + h - 3 );
		p->setPen( cg.mid() );
		p->drawPoint( x + w - 4, y + h - 4 );
	    }
	    p->setPen( oldPen );
	    break;
	}
    case PE_Indicator:
	{
	    drawPrimitive( PE_ButtonBevel, p, QRect(r.x(), r.y(),
						    r.width() - 2, r.height()),
			   cg, flags );
	    p->fillRect( r.x() + r.width() - 2, r.y(), 2, r.height(),
			 cg.brush( QColorGroup::Background ) );
	    p->setPen( cg.shadow() );
	    p->drawRect( r.x(), r.y(), r.width() - 2, r.height() );

	    static const QCOORD nochange_mark[] = { 3,5, 9,5,  3,6, 9,6 };
	    static const QCOORD check_mark[] = {
		3,5, 5,5,  4,6, 5,6,  5,7, 6,7,  5,8, 6,8,      6,9, 9,9,
		6,10, 8,10, 7,11, 8,11,  7,12, 7,12,  8,8, 9,8,  8,7, 10,7,
		9,6, 10,6, 9,5, 11,5,  10,4, 11,4,  10,3, 12,3,
		11,2, 12,2, 11,1, 13,1,  12,0, 13,0 };
	    if ( !(flags & Style_Off) ) {
		QPen oldPen = p->pen();
		int x1 = r.x();
		int y1 = r.y();
		if ( flags & Style_Down ) {
		    x1++;
		    y1++;
		}
		QPointArray amark;
		if ( flags & Style_On ) {
		    amark = QPointArray( sizeof(check_mark)/(sizeof(QCOORD)*2),
					 check_mark );
		    // ### KLUDGE!!
		    flags ^= Style_On;
		    flags ^= Style_Down;
		} else if ( flags & Style_NoChange ) {
		    amark = QPointArray( sizeof(nochange_mark)
					 / (sizeof(QCOORD) * 2),
					 nochange_mark );
		}

		amark.translate( x1 + 1, y1 + 1 );
		p->setPen( cg.dark() );
		p->drawLineSegments( amark );
		amark.translate( -1, -1 );
		p->setPen( cg.foreground() );
		p->drawLineSegments( amark );
		p->setPen( oldPen );
	    }
	    break;
	}
    case PE_IndicatorMask:
	{
	    int x,
		y,
		w,
		h;
	    r.rect( &x, &y, &w, &h );
	    p->fillRect( x, y, w - 2, h, color1);
	    if ( flags & Style_Off ) {
		QPen oldPen = p->pen();
		p->setPen ( QPen(color1, 2));
		p->drawLine( x + 2, y + h / 2 - 1,
			     x + w / 2 - 1, y + h - 4 );
		p->drawLine( x + w / 2 - 1, y + h - 4,
			     x + w, 0);
		p->setPen( oldPen );
	    }
	    break;
	}
    case PE_ExclusiveIndicator:
	{
#define QCOORDARRLEN(x) sizeof(x) / (sizeof(QCOORD) * 2 )
	    bool down = flags & Style_Down;
	    bool on = flags & Style_On;

	    static const QCOORD pts1[] = {		// normal circle
		5,0, 8,0, 9,1, 10,1, 11,2, 12,3, 12,4, 13,5,
		13,8, 12,9, 12,10, 11,11, 10,12, 9,12, 8,13,
		5,13, 4,12, 3,12, 2,11, 1,10, 1,9, 0,8, 0,5,
		1,4, 1,3, 2,2, 3,1, 4,1 };
	    static const QCOORD pts2[] = {		// top left shadow
		5,1, 8,1, 3,2, 7,2, 2,3, 5,3,  2,4, 4,4,
		1,5, 3,5, 1,6, 1,8, 2,6, 2,7 };
	    static const QCOORD pts3[] = {		// bottom right, dark
		5,12, 8,12, 7,11, 10,11, 8,10, 11,10,
		9,9, 11,9, 10,8, 12,8, 11,7, 11,7,
		12,5, 12,7 };
	    static const QCOORD pts4[] = {		// bottom right, light
		5,12, 8,12, 7,11, 10,11, 9,10, 11,10,
		10,9, 11,9, 11,7, 11,8, 12,5, 12,8 };
	    static const QCOORD pts5[] = {		// check mark
		6,4, 8,4, 10,6, 10,8, 8,10, 6,10, 4,8, 4,6 };
	    static const QCOORD pts6[] = {		// check mark extras
		4,5, 5,4, 9,4, 10,5, 10,9, 9,10, 5,10, 4,9 };
	    int x, y;
	    x = r.x();
	    y = r.y();
	    p->eraseRect( r );
	    p->setBrush( (down||on) ? cg.brush( QColorGroup::Dark )
			 : cg.brush( QColorGroup::Button) );
	    p->setPen( NoPen );
	    p->drawEllipse( x, y, 13, 13 );
	    p->setPen( cg.shadow() );
	    QPointArray a( QCOORDARRLEN(pts1), pts1 );
	    a.translate( x, y );
	    p->drawPolyline( a );	// draw normal circle
	    QColor tc, bc;
	    const QCOORD *bp;
	    int	bl;
	    if ( down || on ) {			// pressed down or on
		tc = cg.dark().dark();
		bc = cg.light();
		bp = pts4;
		bl = QCOORDARRLEN(pts4);
	    } else {					// released
		tc = cg.light();
		bc = cg.dark();
		bp = pts3;
		bl = QCOORDARRLEN(pts3);
	    }
	    p->setPen( tc );
	    a.setPoints( QCOORDARRLEN(pts2), pts2 );
	    a.translate( x, y );
	    p->drawLineSegments( a );		// draw top shadow
	    p->setPen( bc );
	    a.setPoints( bl, bp );
	    a.translate( x, y );
	    p->drawLineSegments( a );
	    if ( on ) {				// draw check mark
		int x1 = x,
		    y1 = y;
		p->setBrush( cg.foreground() );
		p->setPen( cg.foreground() );
		a.setPoints( QCOORDARRLEN(pts5), pts5 );
		a.translate( x1, y1 );
		p->drawPolygon( a );
		p->setBrush( NoBrush );
		p->setPen( cg.dark() );
		a.setPoints( QCOORDARRLEN(pts6), pts6 );
		a.translate( x1, y1 );
		p->drawLineSegments( a );
	    }
	    break;
	}

    case PE_ExclusiveIndicatorMask:
	{
	    static const QCOORD pts1[] = {		// normal circle
		5,0, 8,0, 9,1, 10,1, 11,2, 12,3, 12,4, 13,5,
		13,8, 12,9, 12,10, 11,11, 10,12, 9,12, 8,13,
		5,13, 4,12, 3,12, 2,11, 1,10, 1,9, 0,8, 0,5,
		1,4, 1,3, 2,2, 3,1, 4,1 };
	    QPointArray a(QCOORDARRLEN(pts1), pts1);
	    a.translate(r.x(), r.y());
	    p->setPen(color1);
	    p->setBrush(color1);
	    p->drawPolygon(a);
	    break;
	}
    case PE_ScrollBarAddLine:
	{
	    drawPrimitive( PE_ButtonBevel, p, r, cg,
			   (flags & Style_Enabled) | ((flags & Style_Down)
						      ? Style_Sunken
						      : Style_Raised) );
	    p->setPen( cg.shadow() );
	    p->drawRect( r );
	    drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowRight
			    : PE_ArrowDown), p, QRect(r.x() + 2,
						      r.y() + 2,
						      r.width() - 4,
						      r.height() - 4),
			   cg, flags );
	    break;
	}
    case PE_ScrollBarSubLine:
	{
	    drawPrimitive( PE_ButtonBevel, p, r, cg,
			   (flags & Style_Enabled) | ((flags & Style_Down)
						      ? Style_Sunken
						      : Style_Raised) );
	    p->setPen( cg.shadow() );
	    p->drawRect( r );
	    drawPrimitive( ((flags & Style_Horizontal) ? PE_ArrowLeft
			    : PE_ArrowUp ), p, QRect(r.x() + 2,
						     r.y() + 2,
						     r.width() - 4,
						     r.height() - 4),
			   cg, flags );
	    break;
	}
    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage:
	{
 	    QPen oldPen = p->pen();
	    if ( r.width() < 3 || r.height() < 3 ) {
		p->fillRect( r, cg.brush(QColorGroup::Mid) );
		p->setPen( cg.shadow() );
		p->drawRect( r );
		p->setPen( oldPen );
	    } else {
		int x,
		    y,
		    w,
		    h;
		r.rect( &x, &y, &w, &h );
		if ( flags & Style_Horizontal ) {
		    p->fillRect( x + 2, y + 2, w - 2,
				 h - 4,
				 cg.brush(QColorGroup::Mid) );
		    // the dark side
		    p->setPen( cg.dark().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->setPen( cg.shadow());
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.mid().dark());
		    p->drawLine( x + 1, y + 1, x + w - 1,
				 y + 1 );
		    p->drawLine( x + 1, y + 1, x + 1,
				 y + h - 2 );

		    // the bright side!

		    p->setPen( cg.button());
		    p->drawLine( x + 1, y + h - 2,
				 x + w - 1,
				 y + h - 2 );
		    p->setPen( cg.shadow());
		    p->drawLine( x, y + h - 1,
				 x + w - 1,
				 y + h - 1 );

		} else {
		    p->fillRect( x + 2, y + 2, w - 4,
				 h - 2,
				 cg.brush(QColorGroup::Mid) );

		    // the dark side
		    p->setPen( cg.dark().dark() );
		    p->drawLine( x, y, x + w - 1, y );
		    p->setPen( cg.shadow() );
		    p->drawLine( x, y, x, y + h - 1 );

		    p->setPen( cg.mid().dark() );
		    p->drawLine( x + 1, y + 1, x + w - 2,
				 y + 1 );
		    p->drawLine( x + 1, y + 1, x + 1,
				 y + h - 1 );

		    // the bright side!
		    p->setPen( cg.button() );
		    p->drawLine( x + w - 2, y + 1,
				 x + w - 2,
				 y + h - 1 );

		    p->setPen( cg.shadow() );
		    p->drawLine( x + w - 1, y,
				 x + w - 1,
				 y + h - 1 );

		}
	    }
	    p->setPen( oldPen );
	    break;
	}
    case PE_ScrollBarSlider:
	{
	    QPoint bo = p->brushOrigin();
	    p->setBrushOrigin( r.topLeft() );
	    drawPrimitive( PE_ButtonBevel, p, r, cg, Style_Raised );
	    p->setBrushOrigin( bo );
	    drawRiffles( p, r.x(), r.y(), r.width(), r.height(), cg,
			 flags & Style_Horizontal );
	    p->setPen( cg.shadow() );
	    p->drawRect( r );
	    if ( flags & Style_HasFocus ) {
		drawPrimitive( PE_FocusRect, p, QRect(r.x() + 2, r.y() + 2,
						      r.width() - 5,
						      r.height() - 5 ),
			       cg, flags );
	    }
	    break;
	}
    default:
	QWindowsStyle::drawPrimitive( pe, p, r, cg, flags, opt );
	break;
    }

}

/*!\reimp
 */
void QPlatinumStyle::drawControl( ControlElement element,
				  QPainter *p,
				  const QWidget *widget,
				  const QRect &r,
				  const QColorGroup &cg,
				  SFlags how,
				  const QStyleOption& opt ) const
{
    switch( element ) {
    case CE_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
	    QColorGroup myCg( cg );
	    const QPushButton *btn;
	    int x1, y1, x2, y2;
	    bool useBevelButton;
	    SFlags flags;
	    flags = Style_Default;
	    btn = (const QPushButton*)widget;
	    p->setBrushOrigin( -widget->backgroundOffset().x(),
			       -widget->backgroundOffset().y() );

	    // take care of the flags based on what we know...
	    if ( btn->isDown() )
		flags |= Style_Down;
	    if ( btn->isOn() )
		flags |= Style_On;
	    if ( btn->isEnabled() )
		flags |= Style_Enabled;
	    if ( btn->isDefault() )
		flags |= Style_Default;
 	    if (! btn->isFlat() && !(flags & Style_Down))
 		flags |= Style_Raised;

	    r.coords( &x1, &y1, &x2, &y2 );

	    p->setPen( cg.foreground() );
	    p->setBrush( QBrush(cg.button(), NoBrush) );

	    QBrush fill;
	    if ( btn->isDown() ) {
		fill = cg.brush( QColorGroup::Dark );
		// this could be done differently, but this
		// makes a down Bezel drawn correctly...
		myCg.setBrush( QColorGroup::Mid, fill );
	    } else if ( btn->isOn() ) {
		fill = QBrush( cg.mid(), Dense4Pattern );
		myCg.setBrush( QColorGroup::Mid, fill );
	    }
	    // to quote the old QPlatinumStlye drawPushButton...
	    // small or square image buttons as well as toggle buttons are
	    // bevel buttons (what a heuristic....)
	    if ( btn->isToggleButton()
		 || ( btn->pixmap() &&
		      (btn->width() * btn->height() < 1600 ||
		       QABS( btn->width() - btn->height()) < 10 )) )
		useBevelButton = TRUE;
	    else
		useBevelButton = FALSE;

	    int diw = pixelMetric( PM_ButtonDefaultIndicator, widget );
	    if ( btn->isDefault() ) {
		x1 += 1;
		y1 += 1;
		x2 -= 1;
		y2 -= 1;
		QColorGroup cg2( myCg );
		SFlags myFlags = flags;
		// don't draw the default button sunken, unless it's necessary.
		if ( myFlags & Style_Down )
		    myFlags ^= Style_Down;
		if ( myFlags & Style_Sunken )
		    myFlags ^= Style_Sunken;

		cg2.setColor( QColorGroup::Button, cg.mid() );
		if ( useBevelButton ) {
		    drawPrimitive( PE_ButtonBevel, p, QRect( x1, y1,
							     x2 - x1 + 1,
							     y2 - y1 + 1 ),
				   myCg, myFlags, opt );
		} else {
		    drawPrimitive( PE_ButtonCommand, p, QRect( x1, y1,
							       x2 - x1 + 1,
							       y2 - y1 + 1 ),
				   cg2, myFlags, opt );
		}
	    }

	    if ( btn->isDefault() || btn->autoDefault() ) {
		x1 += diw;
		y1 += diw;
		x2 -= diw;
		y2 -= diw;
	    }

	    if ( !btn->isFlat() || btn->isOn() || btn->isDown() ) {
		if ( useBevelButton ) {
		    // fix for toggle buttons...
		    if ( flags & (Style_Down | Style_On) )
			flags |= Style_Sunken;
		    drawPrimitive( PE_ButtonBevel, p, QRect( x1, y1,
							     x2 - x1 + 1,
							     y2 - y1 + 1 ),
				   myCg, flags, opt );
		} else {

		    drawPrimitive( PE_ButtonCommand, p, QRect( x1, y1,
							       x2 - x1 + 1,
							       y2 - y1 + 1 ),
				   myCg, flags, opt );
		}
	    }


	    if ( p->brush().style() != NoBrush )
		p->setBrush( NoBrush );
	    break;
#endif
	}
    case CE_PushButtonLabel:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *btn;
	    bool on;
	    int x, y, w, h;
	    SFlags flags;
	    flags = Style_Default;
	    btn = (const QPushButton*)widget;
	    on = btn->isDown() || btn->isOn();
	    r.rect( &x, &y, &w, &h );
	    if ( btn->isMenuButton() ) {
		int dx = pixelMetric( PM_MenuButtonIndicator, widget );

		QColorGroup g = cg;
		int xx = x + w - dx - 4;
		int yy = y - 3;
		int hh = h + 6;

		if ( !on ) {
		    p->setPen( g.mid() );
		    p->drawLine( xx, yy + 2, xx, yy + hh - 3 );
		    p->setPen( g.button() );
		    p->drawLine( xx + 1, yy + 1, xx + 1, yy + hh - 2 );
		    p->setPen( g.light() );
		    p->drawLine( xx + 2, yy + 2, xx + 2, yy + hh - 2 );
		}
		if ( btn->isEnabled() )
		    flags |= Style_Enabled;
		drawPrimitive( PE_ArrowDown, p, QRect(x + w - dx - 1, y + 2,
						      dx, h - 4),
			       g, flags, opt );
		w -= dx;
	    }
#ifndef QT_NO_ICONSET
	    if ( btn->iconSet() && !btn->iconSet()->isNull() ) {
		QIconSet::Mode mode = btn->isEnabled()
				      ? QIconSet::Normal : QIconSet::Disabled;
		if ( mode == QIconSet::Normal && btn->hasFocus() )
		    mode = QIconSet::Active;
		QIconSet::State state = QIconSet::Off;
		if ( btn->isToggleButton() && btn->isOn() )
		    state = QIconSet::On;
		QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small,
							 mode, state );
		int pixw = pixmap.width();
		int pixh = pixmap.height();
		p->drawPixmap( x + 2, y + h / 2 - pixh / 2, pixmap );
		x += pixw + 4;
		w -= pixw + 4;
	    }
#endif
	    drawItem( p, QRect( x, y, w, h ),
		      AlignCenter | ShowPrefix,
		      btn->colorGroup(), btn->isEnabled(),
		      btn->pixmap(), btn->text(), -1,
		      on ? &btn->colorGroup().brightText()
		      : &btn->colorGroup().buttonText() );
	    if ( btn->hasFocus() )
		drawPrimitive( PE_FocusRect, p,
			       subRect(SR_PushButtonFocusRect, widget),
			       cg, flags );
	    break;
#endif
	}
    default:
	QWindowsStyle::drawControl( element, p, widget, r, cg, how, opt );
	break;
    }
}

/*!\reimp
 */
void QPlatinumStyle::drawComplexControl( ComplexControl control,
					 QPainter *p,
					 const QWidget *widget,
					 const QRect &r,
					 const QColorGroup &cg,
					 SFlags how,
					 SCFlags sub,
					 SCFlags subActive,
					 const QStyleOption& opt ) const
{
    switch ( control ) {
    case CC_ComboBox:
	{
	    int x,
		y,
		w,
		h;
	    r.rect( &x, &y, &w, &h );
	    p->fillRect( x + 2,  y + 2, w - 4,
			 h - 4, cg.brush(QColorGroup::Button) );
	    // the bright side
	    p->setPen(cg.shadow());
	    p->drawLine( x, y, x + w - 1, y );
	    p->drawLine( x, y, x, y + h - 1 );

	    p->setPen( cg.light() );
	    p->drawLine( x + 1, y + 1,
			 x + w - 2, y + 1 );
	    p->drawLine( x + 1, y + 1,
			 x + 1, y + h - 2 );

	    // the dark side!


	    p->setPen( cg.mid() );
	    p->drawLine( x + 2, y + h - 2,
			 x + w - 2, y + h - 2 );
	    p->drawLine( x + w - 2, y + 2,
			 x + w - 2, y + h - 2 );

	    p->setPen (cg.shadow() );
	    p->drawLine( x + 1, y + h - 1,
			 x + w - 1, y + h - 1 );
	    p->drawLine( x + w - 1, y,
			 x + w - 1, y + h - 1 );

	    // top left corner:
	    p->setPen( cg.background() );
	    p->drawPoint( x, y );
	    p->drawPoint( x + 1, y );
	    p->drawPoint( x, y + 1 );
	    p->setPen( cg.shadow() );
	    p->drawPoint( x + 1, y + 1 );
	    p->setPen( white );
	    p->drawPoint( x + 3, y + 3 );
	    // bottom left corner:
	    p->setPen( cg.background() );
	    p->drawPoint( x, y + h - 1 );
	    p->drawPoint( x + 1, y + h - 1 );
	    p->drawPoint( x, y + h - 2 );
	    p->setPen( cg.shadow() );
	    p->drawPoint( x + 1, y + h - 2 );
	    // top right corner:
	    p->setPen( cg.background() );
	    p->drawPoint( x + w - 1, y );
	    p->drawPoint( x + w - 2, y );
	    p->drawPoint( x + w - 1, y + 1 );
	    p->setPen( cg.shadow() );
	    p->drawPoint( x + w - 2, y + 1 );
	    // bottom right corner:
	    p->setPen( cg.background() );
	    p->drawPoint( x + w - 1, y + h - 1 );
	    p->drawPoint( x + w - 2, y + h - 1 );
	    p->drawPoint( x + w - 1, y + h - 2 );
	    p->setPen( cg.shadow() );
	    p->drawPoint( x + w - 2, y + h - 2 );
	    p->setPen( cg.dark() );
	    p->drawPoint( x + w - 3, y + h - 3 );

	    if ( sub & SC_ComboBoxArrow ) {
		QRect rTmp = querySubControlMetrics( CC_ComboBox, widget,
						     SC_ComboBoxArrow, opt );
		int xx = rTmp.x(),
		    yy = rTmp.y(),
		    ww = rTmp.width(),
		    hh = rTmp.height();
		// the bright side

		p->setPen( cg.mid() );
		p->drawLine( xx, yy+2, xx, yy+hh-3 );

		p->setPen( cg.button() );
		p->drawLine( xx+1, yy+1, xx+ww-2, yy+1 );
		p->drawLine( xx+1, yy+1, xx+1, yy+hh-2 );

		p->setPen( cg.light() );
		p->drawLine( xx+2, yy+2, xx+2, yy+hh-2 );
		p->drawLine( xx+2, yy+2, xx+ww-2, yy+2 );


		// the dark side!

		p->setPen( cg.mid() );
		p->drawLine( xx+3, yy+hh-3 ,xx+ww-3, yy+hh-3 );
		p->drawLine( xx+ww-3, yy+3, xx+ww-3, yy+hh-3 );

		p->setPen( cg.dark() );
		p->drawLine( xx+2, yy+hh-2 ,xx+ww-2, yy+hh-2 );
		p->drawLine( xx+ww-2, yy+2, xx+ww-2, yy+hh-2 );

		p->setPen( cg.shadow() );
		p->drawLine( xx+1, yy+hh-1,xx+ww-1, yy+hh-1 );
		p->drawLine( xx+ww-1, yy, xx+ww-1, yy+hh-1 );

		// top right corner:
		p->setPen( cg.background() );
		p->drawPoint( xx + ww - 1, yy );
		p->drawPoint( xx + ww - 2, yy );
		p->drawPoint( xx + ww - 1, yy + 1 );
		p->setPen( cg.shadow() );
		p->drawPoint( xx + ww - 2, yy + 1 );
		// bottom right corner:
		p->setPen( cg.background() );
		p->drawPoint( xx + ww - 1, yy + hh - 1 );
		p->drawPoint( xx + ww - 2, yy + hh - 1 );
		p->drawPoint( xx + ww - 1, yy + hh - 2 );
		p->setPen( cg.shadow() );
		p->drawPoint( xx + ww - 2, yy + hh - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( xx + ww - 3, yy + hh - 3 );
		p->setPen( cg.mid() );
		p->drawPoint( xx + ww - 4, yy + hh - 4 );

		// and the arrows
		p->setPen( cg.foreground() );
		QPointArray a ;
		a.setPoints(  7, -3,1, 3,1, -2,0, 2,0, -1,-1, 1,-1, 0,-2  );
		a.translate( xx + ww / 2, yy + hh / 2 - 3 );
		p->drawLineSegments( a, 0, 3 );		// draw arrow
		p->drawPoint( a[6] );
		a.setPoints( 7, -3,-1, 3,-1, -2,0, 2,0, -1,1, 1,1, 0,2 );
		a.translate( xx + ww / 2, yy + hh / 2 + 2 );
		p->drawLineSegments( a, 0, 3 );		// draw arrow
		p->drawPoint( a[6] );

	    }
#ifndef QT_NO_COMBOBOX
	    if ( sub & SC_ComboBoxEditField ) {
		const QComboBox *cmb;
		cmb = (const QComboBox*)widget;
		// sadly this is pretty much the windows code, except
		// for the first fillRect call...
		QRect re =
		    QStyle::visualRect( querySubControlMetrics( CC_ComboBox,
								widget,
								SC_ComboBoxEditField ),
					widget );
		if ( cmb->hasFocus() && !cmb->editable() )
		    p->fillRect( re.x() + 1, re.y() + 1,
				 re.width() - 2, re.height() - 2,
				 cg.brush( QColorGroup::Highlight ) );

		if ( cmb->hasFocus() ) {
		    p->setPen( cg.highlightedText() );
		    p->setBackgroundColor( cg.highlight() );
		} else {
		    p->setPen( cg.text() );
		    p->setBackgroundColor( cg.background() );
		}

		if ( cmb->hasFocus() && !cmb->editable() ) {
		    QRect re =
			QStyle::visualRect( subRect( SR_ComboBoxFocusRect,
						     cmb ),
					    widget );
		    drawPrimitive( PE_FocusRect, p, re, cg,
				   Style_FocusAtBorder,
				   QStyleOption(cg.highlight()));
		}
		if ( cmb->editable() ) {
		    // need this for the moment...
		    // was the code in comboButton rect
		    QRect ir( x + 3, y + 3,
			      w - 6 - 16, h - 6 );
		    if ( QApplication::reverseLayout() )
			ir.moveBy( 16, 0 );
		    // end comboButtonRect...
		    ir.setRect( ir.left() - 1, ir.top() - 1, ir.width() + 2,
				ir.height() + 2 );
		    qDrawShadePanel( p, ir, cg, TRUE, 2, 0 );
		}
	    }
#endif
	    break;
	}
    case CC_Slider:
	{
#ifndef QT_NO_SLIDER
	    const QSlider *slider = (const QSlider *) widget;
	    int thickness = pixelMetric( PM_SliderControlThickness, widget );
	    int len = pixelMetric( PM_SliderLength, widget );
	    int ticks = slider->tickmarks();

	    QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
						  opt),
		  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
						  opt);

	    if ((sub & SC_SliderGroove) && groove.isValid()) {
		p->fillRect( groove, cg.brush(QColorGroup::Background) );

		int x, y, w, h;
		int mid = thickness / 2;

		if ( ticks & QSlider::Above )
		    mid += len / 8;
		if ( ticks & QSlider::Below )
		    mid -= len / 8;

		if ( slider->orientation() == Horizontal ) {
		    x = 0;
		    y = groove.y() + mid - 3;
		    w = slider->width();
		    h = 7;
		} else {
		    x = groove.x() + mid - 3;
		    y = 0;
		    w = 7;
		    h = slider->height();
		}

		p->fillRect( x, y, w, h, cg.brush( QColorGroup::Dark ));
		// the dark side
		p->setPen( cg.dark() );
		p->drawLine( x, y, x + w - 1, y );
		p->drawLine( x, y, x, y + h - 1);
		p->setPen( cg.shadow() );
		p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
		p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );
		// the bright side!
		p->setPen(cg.shadow());
		p->drawLine( x + 1,  y + h - 2, x + w - 2,  y + h - 2 );
		p->drawLine( x + w - 2, y + 1, x + w - 2, y + h - 2 );
		p->setPen( cg.light() );
		p->drawLine( x, y + h - 1, x + w - 1, y + h - 1 );
		p->drawLine( x + w - 1, y, x + w - 1, y + h - 1 );
		// top left corner:
		p->setPen(cg.background());
		p->drawPoint( x, y );
		p->drawPoint( x + 1, y );
		p->drawPoint( x, y + 1 );
		p->setPen(cg.shadow());
		p->drawPoint( x + 1, y + 1 );
		// bottom left corner:
		p->setPen( cg.background() );
		p->drawPoint( x, y + h - 1 );
		p->drawPoint( x + 1, y + h - 1 );
		p->drawPoint( x, y + h - 2 );
		p->setPen( cg.light() );
		p->drawPoint( x + 1, y + h - 2 );
		// top right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y );
		p->drawPoint( x + w - 2, y );
		p->drawPoint( x + w - 1, y + 1 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 2, y + 1 );
		// bottom right corner:
		p->setPen( cg.background() );
		p->drawPoint( x + w - 1, y + h - 1 );
		p->drawPoint( x + w - 2, y + h - 1 );
		p->drawPoint( x + w - 1, y + h - 2 );
		p->setPen( cg.light() );
		p->drawPoint( x + w - 2, y + h - 2 );
		p->setPen( cg.dark() );
		p->drawPoint( x + w - 3, y + h - 3 );
		// ### end slider groove

		if ( how & Style_HasFocus )
		    drawPrimitive( PE_FocusRect, p, groove, cg );
	    }

	    if ((sub & SC_SliderHandle) && handle.isValid()) {
		const QColor c0 = cg.shadow();
		const QColor c1 = cg.dark();
		const QColor c3 = cg.light();

		int x1 = handle.x();
		int x2 = handle.x() + handle.width() - 1;
		int y1 = handle.y();
		int y2 = handle.y() + handle.height() - 1;
		int mx = handle.width() / 2;
		int my = handle.height() / 2;

		if ( slider->orientation() == Vertical ) {
		    // Background
		    QBrush oldBrush = p->brush();
		    p->setBrush( cg.brush( QColorGroup::Button ) );
		    p->setPen( NoPen );
		    QPointArray a(6);
		    a.setPoint( 0, x1 + 1, y1 + 1 );
		    a.setPoint( 1, x2 - my + 2, y1 + 1 );
		    a.setPoint( 2, x2 - 1, y1 + my - 1 );
		    a.setPoint( 3, x2 - 1, y2 - my + 1 );
		    a.setPoint( 4, x2 - my + 2, y2 - 1 );
		    a.setPoint( 5, x1 + 1, y2 - 1 );
		    p->drawPolygon( a );
		    p->setBrush( oldBrush );

		    // shadow border
		    p->setPen( c0 );
		    p->drawLine( x1, y1 + 1, x1,y2 - 1 );
		    p->drawLine( x2 - my + 2, y1, x2, y1 + my - 2 );
		    p->drawLine( x2 - my + 2, y2, x2, y1 + my + 2 );
		    p->drawLine( x2, y1 + my - 2, x2, y1 + my + 2 );
		    p->drawLine( x1 + 1, y1, x2 - my + 2, y1 );
		    p->drawLine( x1 + 1, y2, x2 - my + 2, y2 );

		    // light shadow
		    p->setPen( c3 );
		    p->drawLine( x1 + 1, y1 + 2, x1 + 1, y2 - 2 );
		    p->drawLine( x1 + 1, y1 + 1, x2 - my + 2, y1 + 1 );
		    p->drawLine( x2 - my + 2, y1 + 1, x2 - 1, y1 + my - 2 );

		    // dark shadow
		    p->setPen(c1);
		    p->drawLine( x2 - 1, y1 + my - 2, x2 - 1, y1 + my + 2 );
		    p->drawLine( x2 - my + 2, y2 - 1, x2 - 1, y1 + my + 2 );
		    p->drawLine( x1 + 1, y2 - 1, x2 -my + 2, y2 - 1 );

		    drawRiffles( p, handle.x(), handle.y() + 2, handle.width() - 3,
				 handle.height() - 4, cg, TRUE );
		} else {  // Horizontal
		    QBrush oldBrush = p->brush();
		    p->setBrush( cg.brush( QColorGroup::Button ) );
		    p->setPen( NoPen );
		    QPointArray a(6);
		    a.setPoint( 0, x2 - 1, y1 + 1 );
		    a.setPoint( 1, x2 - 1, y2 - mx + 2 );
		    a.setPoint( 2, x2 - mx + 1, y2 - 1 );
		    a.setPoint( 3, x1 + mx - 1, y2 - 1 );
		    a.setPoint( 4, x1 + 1, y2 - mx + 2 );
		    a.setPoint( 5, x1 + 1, y1 + 1 );
		    p->drawPolygon( a );
		    p->setBrush( oldBrush );

		    // shadow border
		    p->setPen( c0 );
		    p->drawLine( x1 + 1, y1, x2 - 1, y1 );
		    p->drawLine( x1, y2 - mx + 2, x1 + mx - 2, y2 );
		    p->drawLine( x2, y2 - mx + 2, x1 + mx + 2, y2 );
		    p->drawLine( x1 + mx - 2, y2, x1 + mx + 2, y2 );
		    p->drawLine( x1, y1 + 1, x1, y2 - mx + 2 );
		    p->drawLine( x2, y1 + 1, x2, y2 - mx + 2 );

		    // light shadow
		    p->setPen(c3);
		    p->drawLine( x1 + 1, y1 + 1, x2 - 1, y1 + 1 );
		    p->drawLine( x1 + 1, y1 + 1, x1 + 1, y2 - mx + 2 );

		    // dark shadow
		    p->setPen(c1);
		    p->drawLine( x2 - 1, y1 + 1, x2 - 1, y2 - mx + 2 );
		    p->drawLine( x1 + 1, y2 - mx + 2, x1 + mx - 2, y2 - 1 );
		    p->drawLine( x2 - 1, y2 - mx + 2, x1 + mx + 2, y2 - 1 );
		    p->drawLine( x1 + mx - 2, y2 - 1, x1 + mx + 2, y2 - 1 );

		    drawRiffles( p, handle.x() + 2, handle.y(), handle.width() - 4,
				 handle.height() - 5, cg, FALSE );
		}
	    }

	    if ( sub & SC_SliderTickmarks )
		QCommonStyle::drawComplexControl( control, p, widget, r,
						  cg, how, SC_SliderTickmarks,
						  subActive, opt );
#endif
	    break;
	}
    default:
	QWindowsStyle::drawComplexControl( control, p, widget, r, cg,
					   how, sub, subActive, opt );
	break;
    }
}



/*!\reimp
 */
QRect QPlatinumStyle::querySubControlMetrics( ComplexControl control,
					      const QWidget *widget,
					      SubControl sc,
					      const QStyleOption& opt ) const
{
    switch( control ) {
#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
	const QComboBox *cb;
	cb = (const QComboBox *)widget;
	switch( sc ) {
	case SC_ComboBoxArrow: {
	    QRect ir = cb->rect();
	    int xx;
	    if( QApplication::reverseLayout() )
		xx = ir.x();
	    else
		xx = ir.x() + ir.width() - 20;
	    return QRect( xx, ir.y(), 20, ir.height()); }
	default:
	    break;
	}
	break;
#endif
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	const QScrollBar *sb;
	sb = (const QScrollBar *)widget;
	int sliderStart = sb->sliderStart();
	int sbextent = pixelMetric( PM_ScrollBarExtent, widget );
	int maxlen = ((sb->orientation() == Qt::Horizontal) ?
		      sb->width() : sb->height()) - ( sbextent * 2 );

	int sliderlen;

	// calculate length
	if ( sb->maxValue() != sb->minValue() ) {
	    uint range = sb->maxValue() - sb->minValue();
	    sliderlen = ( sb->pageStep() * maxlen ) /
			( range + sb->pageStep() );

	    int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
	    if ( sliderlen < slidermin || range > INT_MAX / 2 )
		sliderlen = slidermin;
	    if ( sliderlen > maxlen )
		sliderlen = maxlen;
	} else {
	    sliderlen = maxlen;
	}

	switch ( sc ) {
	case SC_ScrollBarSubLine:
	    if ( sb->orientation() == Qt::Horizontal ) {
		int buttonw = QMIN( sb->width() / 2, sbextent );
		return QRect( sb->width() - 2 * buttonw, 0, buttonw, sbextent );
	    } else {
		int buttonh = QMIN( sb->height() / 2, sbextent );
		return QRect( 0, sb->height() - 2 * buttonh, sbextent, buttonh );
	    }
	case SC_ScrollBarAddLine:
	    if ( sb->orientation() == Qt::Horizontal ) {
		int buttonw = QMIN( sb->width() / 2, sbextent );
		return QRect( sb->width() - buttonw, 0, sbextent, buttonw );
	    } else {
		int buttonh = QMIN( sb->height() / 2, sbextent );
		return QRect(0, sb->height() - buttonh, sbextent, buttonh );
	    }
	case SC_ScrollBarSubPage:
	    if ( sb->orientation() == Qt::Horizontal )
		return QRect( 1, 0, sliderStart, sbextent );
	    return QRect( 0, 1, sbextent, sliderStart );
	case SC_ScrollBarAddPage:
	    if ( sb->orientation() == Qt::Horizontal )
		return QRect( sliderStart + sliderlen, 0, maxlen - sliderStart - sliderlen, sbextent );
	    return QRect( 0, sliderStart + sliderlen, sbextent, maxlen - sliderStart - sliderlen );
	case SC_ScrollBarGroove:
	    if ( sb->orientation() == Qt::Horizontal )
		return QRect( 1, 0, sb->width() - sbextent * 2, sb->height() );
	    return QRect( 0, 1, sb->width(), sb->height() - sbextent * 2 );
	default:
	    break;
	}
	break; }
#endif
#ifndef QT_NO_SLIDER
    case CC_Slider: {

	const QSlider *slider = (const QSlider *) widget;
	int tickOffset = pixelMetric( PM_SliderTickmarkOffset, widget);
	int thickness = pixelMetric( PM_SliderControlThickness, widget);
	int mid = thickness / 2;
	int ticks = slider->tickmarks();
	int len = pixelMetric( PM_SliderLength, widget );

	switch ( sc ) {
	case SC_SliderGroove:
	    if ( ticks & QSlider::Above )
		mid += len / 8;
	    if ( ticks & QSlider::Below )
		mid -= len / 8;
	    if ( slider->orientation() == QSlider::Horizontal )
		return QRect( 0, tickOffset, slider->width(), thickness );
	    return QRect( tickOffset, 0, thickness, slider->height() );
	default:
	    break;
	}
	break; }
#endif
    default:
	break;
    }
    return QWindowsStyle::querySubControlMetrics( control, widget, sc, opt );
}


/*!\reimp
 */
int QPlatinumStyle::pixelMetric( PixelMetric metric,
				 const QWidget *widget ) const
{
    int ret;
    switch( metric ) {
    case PM_ButtonDefaultIndicator:
	ret = 3;
	break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;
    case PM_IndicatorWidth:
	ret = 15;
	break;
    case PM_IndicatorHeight:
	ret = 13;
	break;
    case PM_ExclusiveIndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
	ret = 15;
	break;
    case PM_SliderLength:
	ret = 17;
	break;
    case PM_MaximumDragDistance:
	ret = -1;
	break;
    default:
	ret = QWindowsStyle::pixelMetric( metric, widget );
	break;
    }
    return ret;
}

/*!\reimp
 */
QRect QPlatinumStyle::subRect( SubRect r, const QWidget *widget ) const
{
    QRect rect;
    switch ( r ) {
    case SR_ComboBoxFocusRect:
	{
	    QRect tmpR = widget->rect();
	    rect = QRect( tmpR.x() + 4, tmpR.y() + 4, tmpR.width() - 8 - 16,
			  tmpR.height() - 8 );
	    break;
	}
    default:
	rect = QWindowsStyle::subRect( r, widget );
	break;
    }
    return rect;
}

/*!
    Mixes two colors \a c1 and \a c2 to a new color.
*/
QColor QPlatinumStyle::mixedColor(const QColor &c1, const QColor &c2) const
{
    int h1,s1,v1,h2,s2,v2;
    c1.hsv(&h1,&s1,&v1);
    c2.hsv(&h2,&s2,&v2);
    return QColor( (h1+h2)/2, (s1+s2)/2, (v1+v2)/2, QColor::Hsv );
}

/*!
    Draws the nifty Macintosh decoration used on sliders using painter
    \a p and colorgroup \a g. \a x, \a y, \a w, \a h and \a horizontal
    specify the geometry and orientation of the riffles.
*/
void QPlatinumStyle::drawRiffles( QPainter* p,  int x, int y, int w, int h,
				  const QColorGroup &g, bool horizontal ) const
{
    if (!horizontal) {
	if (h > 20) {
	    y += (h-20)/2 ;
	    h = 20;
	}
	if (h > 8) {
	    int n = h / 4;
	    int my = y+h/2-n;
	    int i ;
	    p->setPen(g.light());
	    for (i=0; i<n; i++) {
		p->drawLine(x+3, my+2*i, x+w-5, my+2*i);
	    }
	    p->setPen(g.dark());
	    my++;
	    for (i=0; i<n; i++) {
		p->drawLine(x+4, my+2*i, x+w-4, my+2*i);
	    }
	}
    }
    else {
	if (w > 20) {
	    x += (w-20)/2 ;
	    w = 20;
	}
	if (w > 8) {
	    int n = w / 4;
	    int mx = x+w/2-n;
	    int i ;
	    p->setPen(g.light());
	    for (i=0; i<n; i++) {
		p->drawLine(mx+2*i, y+3, mx + 2*i, y+h-5);
	    }
	    p->setPen(g.dark());
	    mx++;
	    for (i=0; i<n; i++) {
		p->drawLine(mx+2*i, y+4, mx + 2*i, y+h-4);
	    }
	}
    }
}


#endif
