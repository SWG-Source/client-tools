/****************************************************************************
** $Id: qmotifstyle.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of Motif-like style class
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

#include "qmotifstyle.h"

#if !defined(QT_NO_STYLE_MOTIF) || defined(QT_PLUGIN)

#include "qpopupmenu.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpalette.h"
#include "qwidget.h"
#include "qpushbutton.h"
#include "qscrollbar.h"
#include "qtabbar.h"
#include "qtabwidget.h"
#include "qlistview.h"
#include "qsplitter.h"
#include "qslider.h"
#include "qcombobox.h"
#include "qdockwindow.h"
#include "qdockarea.h"
#include "qprogressbar.h"
#include "qimage.h"
#include <limits.h>



// old constants that might still be useful...
static const int motifItemFrame         = 2;    // menu item frame width
static const int motifSepHeight         = 2;    // separator item height
static const int motifItemHMargin       = 3;    // menu item hor text margin
static const int motifItemVMargin       = 2;    // menu item ver text margin
static const int motifArrowHMargin      = 6;    // arrow horizontal margin
static const int motifTabSpacing        = 12;   // space between text and tab
static const int motifCheckMarkHMargin  = 2;    // horiz. margins of check mark
static const int motifCheckMarkSpace    = 12;


/*!
    \class QMotifStyle qmotifstyle.h
    \brief The QMotifStyle class provides Motif look and feel.

    \ingroup appearance

    This class implements the Motif look and feel. It closely
    resembles the original Motif look as defined by the Open Group,
    but with some minor improvements. The Motif style is Qt's default
    GUI style on UNIX platforms.
*/

/*!
    Constructs a QMotifStyle.

    If \a useHighlightCols is FALSE (the default), the style will
    polish the application's color palette to emulate the Motif way of
    highlighting, which is a simple inversion between the base and the
    text color.
*/
QMotifStyle::QMotifStyle( bool useHighlightCols ) : QCommonStyle()
{
    highlightCols = useHighlightCols;
}

/*!\reimp
*/
QMotifStyle::~QMotifStyle()
{
}

/*!
    If \a arg is FALSE, the style will polish the application's color
    palette to emulate the Motif way of highlighting, which is a
    simple inversion between the base and the text color.

    The effect will show up the next time an application palette is
    set via QApplication::setPalette(). The current color palette of
    the application remains unchanged.

    \sa QStyle::polish()
*/
void QMotifStyle::setUseHighlightColors( bool arg )
{
    highlightCols = arg;
}

/*!
    Returns TRUE if the style treats the highlight colors of the
    palette in a Motif-like manner, which is a simple inversion
    between the base and the text color; otherwise returns FALSE. The
    default is FALSE.
*/
bool QMotifStyle::useHighlightColors() const
{
    return highlightCols;
}

/*! \reimp */

void QMotifStyle::polish( QPalette& pal )
{
    if ( pal.active().light() == pal.active().base() ) {
        QColor nlight = pal.active().light().dark(108 );
        pal.setColor( QPalette::Active, QColorGroup::Light, nlight ) ;
        pal.setColor( QPalette::Disabled, QColorGroup::Light, nlight ) ;
        pal.setColor( QPalette::Inactive, QColorGroup::Light, nlight ) ;
    }

    if ( highlightCols )
        return;

    // force the ugly motif way of highlighting *sigh*
    QColorGroup disabled = pal.disabled();
    QColorGroup active = pal.active();

    pal.setColor( QPalette::Active, QColorGroup::Highlight,
                  active.text() );
    pal.setColor( QPalette::Active, QColorGroup::HighlightedText,
                  active.base());
    pal.setColor( QPalette::Disabled, QColorGroup::Highlight,
                  disabled.text() );
    pal.setColor( QPalette::Disabled, QColorGroup::HighlightedText,
                  disabled.base() );
    pal.setColor( QPalette::Inactive, QColorGroup::Highlight,
                  active.text() );
    pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText,
                  active.base() );
}

/*!
 \reimp
 \internal
 Keep QStyle::polish() visible.
*/
void QMotifStyle::polish( QWidget* w )
{
    QStyle::polish(w);
}

/*!
 \reimp
 \internal
 Keep QStyle::polish() visible.
*/
void QMotifStyle::polish( QApplication* a )
{
    QStyle::polish(a);
}

static void rot(QPointArray& a, int n)
{
    QPointArray r(a.size());
    for (int i = 0; i < (int)a.size(); i++) {
	switch (n) {
	    case 1: r.setPoint(i,-a[i].y(),a[i].x()); break;
	    case 2: r.setPoint(i,-a[i].x(),-a[i].y()); break;
	    case 3: r.setPoint(i,a[i].y(),-a[i].x()); break;
	}
    }
    a = r;
}


/*!\reimp
*/
void QMotifStyle::drawPrimitive( PrimitiveElement pe,
				 QPainter *p,
				 const QRect &r,
				 const QColorGroup &cg,
				 SFlags flags,
				 const QStyleOption& opt ) const
{
    switch( pe ) {
#ifndef QT_NO_LISTVIEW
    case PE_CheckListExclusiveIndicator: {
	QCheckListItem *item = opt.checkListItem();
	QListView *lv = item->listView();
	if(!item)
	    return;

	if ( item->isEnabled() )
	    p->setPen( QPen( cg.text() ) );
	else
	    p->setPen( QPen( lv->palette().color( QPalette::Disabled, QColorGroup::Text ) ) );
	QPointArray a;

	int cx = r.width()/2 - 1;
	int cy = r.height()/2;
	int e = r.width()/2 - 1;
	for ( int i = 0; i < 3; i++ ) { //penWidth 2 doesn't quite work
	    a.setPoints( 4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e );
	    p->drawPolygon( a );
	    e--;
	}
	if ( item->isOn() ) {
	    if ( item->isEnabled() )
		p->setPen( QPen( cg.text()) );
	    else
		p->setPen( QPen( item->listView()->palette().color( QPalette::Disabled,
								    QColorGroup::Text ) ) );
	    QBrush saveBrush = p->brush();
	    p->setBrush( cg.text() );
	    e = e - 2;
	    a.setPoints( 4, cx-e, cy, cx, cy-e,  cx+e, cy,  cx, cy+e );
	    p->drawPolygon( a );
	    p->setBrush( saveBrush );
	}
	break; }
#endif
    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
    case PE_HeaderSection:
	qDrawShadePanel( p, r, cg, bool(flags & (Style_Down | Style_On )),
			 pixelMetric(PM_DefaultFrameWidth),
			 &cg.brush(QColorGroup::Button) );
	break;

    case PE_Indicator: {
#ifndef QT_NO_BUTTON
	bool on = flags & Style_On;
	bool down = flags & Style_Down;
	bool showUp = !( down ^ on );
	QBrush fill = showUp || flags & Style_NoChange ? cg.brush( QColorGroup::Button ) : cg.brush(QColorGroup::Mid );
	if ( flags & Style_NoChange ) {
	    qDrawPlainRect( p, r, cg.text(),
			    1, &fill );
	    p->drawLine( r.x() + r.width() - 1, r.y(),
			 r.x(), r.y() + r.height() - 1);
	} else
	    qDrawShadePanel( p, r, cg, !showUp,
			     pixelMetric(PM_DefaultFrameWidth), &fill );
#endif
	break;
    }

    case PE_ExclusiveIndicator:
	{
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
	    QCOORD inner_pts[] = { // used for filling diamond
		2,r.height()/2,
		r.width()/2,2,
		r.width()-3,r.height()/2,
		r.width()/2,r.height()-3
	    };
	    QCOORD top_pts[] = { // top (^) of diamond
		0,r.height()/2,
		r.width()/2,0,
		r.width()-2,r.height()/2-1,
		r.width()-3,r.height()/2-1,
		r.width()/2,1,
		1,r.height()/2,
		2,r.height()/2,
		r.width()/2,2,
		r.width()-4,r.height()/2-1
	    };
	    QCOORD bottom_pts[] = { // bottom (v) of diamond
		1,r.height()/2+1,
		r.width()/2,r.height()-1,
		r.width()-1,r.height()/2,
		r.width()-2,r.height()/2,
		r.width()/2,r.height()-2,
		2,r.height()/2+1,
		3,r.height()/2+1,
		r.width()/2,r.height()-3,
		r.width()-3,r.height()/2
	    };
	    bool on = flags & Style_On;
	    bool down = flags & Style_Down;
	    bool showUp = !(down ^ on );
	    QPointArray a( QCOORDARRLEN(inner_pts), inner_pts );
	    p->eraseRect( r );
	    p->setPen( NoPen );
	    p->setBrush( showUp ? cg.brush( QColorGroup::Button ) :
			 cg.brush( QColorGroup::Mid ) );
	    a.translate( r.x(), r.y() );
	    p->drawPolygon( a );
	    p->setPen( showUp ? cg.light() : cg.dark() );
	    p->setBrush( NoBrush );
	    a.setPoints( QCOORDARRLEN(top_pts), top_pts );
	    a.translate( r.x(), r.y() );
	    p->drawPolyline( a );
	    p->setPen( showUp ? cg.dark() : cg.light() );
	    a.setPoints( QCOORDARRLEN(bottom_pts), bottom_pts );
	    a.translate( r.x(), r.y() );
	    p->drawPolyline( a );

	    break;
	}

    case PE_ExclusiveIndicatorMask:
	{
	    static QCOORD inner_pts[] = { // used for filling diamond
		0,r.height()/2,
		r.width()/2,0,
		r.width()-1,r.height()/2,
		r.width()/2,r.height()-1
	    };
	    QPointArray a(QCOORDARRLEN(inner_pts), inner_pts);
	    p->setPen(color1);
	    p->setBrush(color1);
	    a.translate(r.x(), r.y());
	    p->drawPolygon(a);
	    break;
	}

    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft:
	{
	    QRect rect = r;
	    QPointArray bFill;
	    QPointArray bTop;
	    QPointArray bBot;
	    QPointArray bLeft;
	    bool vertical = pe == PE_ArrowUp || pe == PE_ArrowDown;
	    bool horizontal = !vertical;
	    int dim = rect.width() < rect.height() ? rect.width() : rect.height();
	    int colspec = 0x0000;

	    if ( dim < 2 )
		break;

	    // adjust size and center (to fix rotation below)
	    if ( rect.width() > dim ) {
		rect.setX( rect.x() + ((rect.width() - dim ) / 2) );
		rect.setWidth( dim );
	    }
	    if ( rect.height() > dim ) {
		rect.setY( rect.y() + ((rect.height() - dim ) / 2 ));
		rect.setHeight( dim );
	    }

	    if ( dim > 3 ) {
		if ( dim > 6 )
		    bFill.resize( dim & 1 ? 3 : 4 );
		bTop.resize( (dim/2)*2 );
		bBot.resize( dim & 1 ? dim + 1 : dim );
		bLeft.resize( dim > 4 ? 4 : 2 );
		bLeft.putPoints( 0, 2, 0,0, 0,dim-1 );
		if ( dim > 4 )
		    bLeft.putPoints( 2, 2, 1,2, 1,dim-3 );
		bTop.putPoints( 0, 4, 1,0, 1,1, 2,1, 3,1 );
		bBot.putPoints( 0, 4, 1,dim-1, 1,dim-2, 2,dim-2, 3,dim-2 );

		for( int i=0; i<dim/2-2 ; i++ ) {
		    bTop.putPoints( i*2+4, 2, 2+i*2,2+i, 5+i*2, 2+i );
		    bBot.putPoints( i*2+4, 2, 2+i*2,dim-3-i, 5+i*2,dim-3-i );
		}
		if ( dim & 1 )                          // odd number size: extra line
		    bBot.putPoints( dim-1, 2, dim-3,dim/2, dim-1,dim/2 );
		if ( dim > 6 ) {                        // dim>6: must fill interior
		    bFill.putPoints( 0, 2, 1,dim-3, 1,2 );
		    if ( dim & 1 )                      // if size is an odd number
			bFill.setPoint( 2, dim - 3, dim / 2 );
		    else
			bFill.putPoints( 2, 2, dim-4,dim/2-1, dim-4,dim/2 );
		}
	    }
	    else {
		if ( dim == 3 ) {                       // 3x3 arrow pattern
		    bLeft.setPoints( 4, 0,0, 0,2, 1,1, 1,1 );
		    bTop .setPoints( 2, 1,0, 1,0 );
		    bBot .setPoints( 2, 1,2, 2,1 );
		}
		else {                                  // 2x2 arrow pattern
		    bLeft.setPoints( 2, 0,0, 0,1 );
		    bTop .setPoints( 2, 1,0, 1,0 );
		    bBot .setPoints( 2, 1,1, 1,1 );
		}
	    }

	    // We use rot() and translate() as it is more efficient that
	    // matrix transformations on the painter, and because it still
	    // works with QT_NO_TRANSFORMATIONS defined.

	    if ( pe == PE_ArrowUp || pe == PE_ArrowLeft ) {
		if ( vertical ) {
		    rot(bFill,3);
		    rot(bLeft,3);
		    rot(bTop,3);
		    rot(bBot,3);
		    bFill.translate( 0, rect.height() - 1 );
		    bLeft.translate( 0, rect.height() - 1 );
		    bTop.translate( 0, rect.height() - 1 );
		    bBot.translate( 0, rect.height() - 1 );
		} else {
		    rot(bFill,2);
		    rot(bLeft,2);
		    rot(bTop,2);
		    rot(bBot,2);
		    bFill.translate( rect.width() - 1, rect.height() - 1 );
		    bLeft.translate( rect.width() - 1, rect.height() - 1 );
		    bTop.translate( rect.width() - 1, rect.height() - 1 );
		    bBot.translate( rect.width() - 1, rect.height() - 1 );
		}
		if ( flags & Style_Down )
		    colspec = horizontal ? 0x2334 : 0x2343;
		else
		    colspec = horizontal ? 0x1443 : 0x1434;
	    } else {
		if ( vertical ) {
		    rot(bFill,1);
		    rot(bLeft,1);
		    rot(bTop,1);
		    rot(bBot,1);
		    bFill.translate( rect.width() - 1, 0 );
		    bLeft.translate( rect.width() - 1, 0 );
		    bTop.translate( rect.width() - 1, 0 );
		    bBot.translate( rect.width() - 1, 0 );
		}
		if ( flags & Style_Down )
		    colspec = horizontal ? 0x2443 : 0x2434;
		else
		    colspec = horizontal ? 0x1334 : 0x1343;
	    }
	    bFill.translate( rect.x(), rect.y() );
	    bLeft.translate( rect.x(), rect.y() );
	    bTop.translate( rect.x(), rect.y() );
	    bBot.translate( rect.x(), rect.y() );

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

#define CMID *cols[ (colspec>>12) & 0xf ]
#define CLEFT *cols[ (colspec>>8) & 0xf ]
#define CTOP *cols[ (colspec>>4) & 0xf ]
#define CBOT *cols[ colspec & 0xf ]

	    QPen savePen = p->pen();
	    QBrush saveBrush = p->brush();
	    QPen pen( NoPen );
	    QBrush brush = cg.brush( flags & Style_Enabled ? QColorGroup::Button :
				     QColorGroup::Mid );
	    p->setPen( pen );
	    p->setBrush( brush );
	    p->drawPolygon( bFill );
	    p->setBrush( NoBrush );

	    p->setPen( CLEFT );
	    p->drawLineSegments( bLeft );
	    p->setPen( CTOP );
	    p->drawLineSegments( bTop );
	    p->setPen( CBOT );
	    p->drawLineSegments( bBot );

	    p->setBrush( saveBrush );
	    p->setPen( savePen );
#undef CMID
#undef CLEFT
#undef CTOP
#undef CBOT
	    break;
	}

    case PE_SpinWidgetPlus:
    case PE_SpinWidgetMinus:
	{
	    p->save();
	    int fw = pixelMetric( PM_DefaultFrameWidth );
	    QRect br;
	    br.setRect( r.x() + fw, r.y() + fw, r.width() - fw*2,
			r.height() - fw*2 );

	    if ( flags & Style_Sunken )
		p->fillRect( r, cg.brush( QColorGroup::Dark ) );
	    else
		p->fillRect( r, cg.brush( QColorGroup::Button ) );

	    p->setPen( cg.buttonText() );
	    p->setBrush( cg.buttonText() );

	    int length;
	    int x = r.x(), y = r.y(), w = r.width(), h = r.height();
	    if ( w <= 8 || h <= 6 )
		length = QMIN( w-2, h-2 );
	    else
		length = QMIN( 2*w / 3, 2*h / 3 );

	    if ( !(length & 1) )
		length -=1;
	    int xmarg = ( w - length ) / 2;
	    int ymarg = ( h - length ) / 2;

	    p->drawLine( x + xmarg, ( y + h / 2 - 1 ),
			 x + xmarg + length - 1, ( y + h / 2 - 1 ) );
	    if ( pe == PE_SpinWidgetPlus )
		p->drawLine( ( x+w / 2 ) - 1, y + ymarg,
			     ( x+w / 2 ) - 1, y + ymarg + length - 1 );
	    p->restore();
	    break;
	}

    case PE_SpinWidgetUp:
    case PE_SpinWidgetDown:
	{
	    p->save();
	    int fw = pixelMetric( PM_DefaultFrameWidth );
	    QRect br;
	    br.setRect( r.x() + fw, r.y() + fw, r.width() - fw*2,
			r.height() - fw*2 );
	    if ( flags & Style_Sunken )
		p->fillRect( br, cg.brush( QColorGroup::Mid ) );
	    else
		p->fillRect( br, cg.brush( QColorGroup::Button ) );

	    int x = r.x(), y = r.y(), w = r.width(), h = r.height();
	    int sw = w-4;
	    if ( sw < 3 )
		return;
	    else if ( !(sw & 1) )
		sw--;
	    sw -= ( sw / 7 ) * 2;	// Empty border
	    int sh = sw/2 + 2;      // Must have empty row at foot of arrow

	    int sx = x + w / 2 - sw / 2 - 1;
	    int sy = y + h / 2 - sh / 2 - 1;

	    QPointArray a;
	    if ( pe == PE_SpinWidgetDown )
		a.setPoints( 3,  0, 1,  sw-1, 1,  sh-2, sh-1 );
	    else
		a.setPoints( 3,  0, sh-1,  sw-1, sh-1,  sh-2, 1 );
	    int bsx = 0;
	    int bsy = 0;
	    if ( flags & Style_Sunken ) {
		bsx = pixelMetric(PM_ButtonShiftHorizontal);
		bsy = pixelMetric(PM_ButtonShiftVertical);
	    }
	    p->translate( sx + bsx, sy + bsy );
	    p->setPen( cg.buttonText() );
	    p->setBrush( cg.buttonText() );
	    p->drawPolygon( a );
	    p->restore();
	    break;
	}

    case PE_DockWindowHandle:
	{
	    p->save();
	    p->translate( r.x(), r.y() );

	    QColor dark( cg.dark() );
	    QColor light( cg.light() );
	    unsigned int i;
	    if ( flags & Style_Horizontal ) {
		int h = r.height();
		if ( h > 6 ) {
		    if ( flags & Style_On )
			p->fillRect( 1, 1, 8, h - 2, cg.highlight() );
		    QPointArray a( 2 * ((h-6)/3) );
		    int y = 3 + (h%3)/2;
		    p->setPen( dark );
		    p->drawLine( 8, 1, 8, h-2 );
		    for( i=0; 2*i < a.size(); i ++ ) {
			a.setPoint( 2*i, 5, y+1+3*i );
			a.setPoint( 2*i+1, 2, y+2+3*i );
		    }
		    p->drawPoints( a );
		    p->setPen( light );
		    p->drawLine( 9, 1, 9, h-2 );
		    for( i=0; 2*i < a.size(); i++ ) {
			a.setPoint( 2*i, 4, y+3*i );
			a.setPoint( 2*i+1, 1, y+1+3*i );
		    }
		    p->drawPoints( a );
		    // if ( drawBorder ) {
		    // p->setPen( QPen( Qt::darkGray ) );
		    // p->drawLine( 0, r.height() - 1,
		    // tbExtent, r.height() - 1 );
		    // }
		}
	    } else {
		int w = r.width();
		if ( w > 6 ) {
		    if ( flags & Style_On )
			p->fillRect( 1, 1, w - 2, 9, cg.highlight() );
		    QPointArray a( 2 * ((w-6)/3) );

		    int x = 3 + (w%3)/2;
		    p->setPen( dark );
		    p->drawLine( 1, 8, w-2, 8 );
		    for( i=0; 2*i < a.size(); i ++ ) {
			a.setPoint( 2*i, x+1+3*i, 6 );
			a.setPoint( 2*i+1, x+2+3*i, 3 );
		    }
		    p->drawPoints( a );
		    p->setPen( light );
		    p->drawLine( 1, 9, w-2, 9 );
		    for( i=0; 2*i < a.size(); i++ ) {
			a.setPoint( 2*i, x+3*i, 5 );
			a.setPoint( 2*i+1, x+1+3*i, 2 );
		    }
		    p->drawPoints( a );
		    // if ( drawBorder ) {
		    // p->setPen( QPen( Qt::darkGray ) );
		    // p->drawLine( r.width() - 1, 0,
		    // r.width() - 1, tbExtent );
		    // }
		}
	    }
	    p->restore();
	    break;
	}

    case PE_Splitter:
	if (flags & Style_Horizontal)
	    flags &= ~Style_Horizontal;
	else
	    flags |= Style_Horizontal;
	// fall through intended

    case PE_DockWindowResizeHandle:
	{
	    const int motifOffset = 10;
 	    int sw = pixelMetric( PM_SplitterWidth );
	    if ( flags & Style_Horizontal ) {
 		QCOORD yPos = r.y() + r.height() / 2;
 		QCOORD kPos = r.width() - motifOffset - sw;
 		QCOORD kSize = sw - 2;

 		qDrawShadeLine( p, 0, yPos, kPos, yPos, cg );
 		qDrawShadePanel( p, kPos, yPos - sw / 2 + 1, kSize, kSize,
 				 cg, FALSE, 1, &cg.brush( QColorGroup::Button ) );
 		qDrawShadeLine( p, kPos + kSize - 1, yPos, r.width(), yPos, cg );
 	    } else {
 		QCOORD xPos = r.x() + r.width() / 2;
 		QCOORD kPos = motifOffset;
 		QCOORD kSize = sw - 2;

 		qDrawShadeLine( p, xPos, kPos + kSize - 1, xPos, r.height(), cg );
 		qDrawShadePanel( p, xPos - sw / 2 + 1, kPos, kSize, kSize, cg,
 				 FALSE, 1, &cg.brush( QColorGroup::Button ) );
 		qDrawShadeLine( p, xPos, 0, xPos, kPos, cg );
 	    }
 	    break;
	}

    case PE_CheckMark:
	{
	    const int markW = 6;
	    const int markH = 6;
	    int posX = r.x() + ( r.width()  - markW ) / 2 - 1;
	    int posY = r.y() + ( r.height() - markH ) / 2;
	    int dfw = pixelMetric(PM_DefaultFrameWidth);

	    if (dfw < 2) {
		// Could do with some optimizing/caching...
		QPointArray a( 7*2 );
		int i, xx, yy;
		xx = posX;
		yy = 3 + posY;
		for ( i=0; i<3; i++ ) {
		    a.setPoint( 2*i,   xx, yy );
		    a.setPoint( 2*i+1, xx, yy+2 );
		    xx++; yy++;
		}
		yy -= 2;
		for ( i=3; i<7; i++ ) {
		    a.setPoint( 2*i,   xx, yy );
		    a.setPoint( 2*i+1, xx, yy+2 );
		    xx++; yy--;
		}
		if ( ! (flags & Style_Enabled) && ! (flags & Style_On) ) {
		    int pnt;
		    p->setPen( cg.highlightedText() );
		    QPoint offset(1,1);
		    for ( pnt = 0; pnt < (int)a.size(); pnt++ )
			a[pnt] += offset;
		    p->drawLineSegments( a );
		    for ( pnt = 0; pnt < (int)a.size(); pnt++ )
			a[pnt] -= offset;
		}
		p->setPen( cg.text() );
		p->drawLineSegments( a );

		qDrawShadePanel( p, posX-2, posY-2, markW+4, markH+6, cg, TRUE, dfw);
	    } else
		qDrawShadePanel( p, posX, posY, markW, markH, cg, TRUE, dfw,
				 &cg.brush( QColorGroup::Mid ) );

	    break;
	}

    case PE_ScrollBarSubLine:
	drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp),
		      p, r, cg, Style_Enabled | flags);
	break;

    case PE_ScrollBarAddLine:
	drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown),
		      p, r, cg, Style_Enabled | flags);
	break;

    case PE_ScrollBarSubPage:
    case PE_ScrollBarAddPage:
	p->fillRect(r, cg.brush(QColorGroup::Mid));
	break;

    case PE_ScrollBarSlider:
	drawPrimitive(PE_ButtonBevel, p, r, cg,
		      (flags | Style_Raised) & ~Style_Down);
	break;

    case PE_ProgressBarChunk:
	p->fillRect( r.x(), r.y() + 2, r.width() - 2,
		     r.height() - 4, cg.brush(QColorGroup::Highlight));
	break;

    default:
	QCommonStyle::drawPrimitive( pe, p, r, cg, flags, opt );
	break;
    }
}


/*!\reimp
*/
void QMotifStyle::drawControl( ControlElement element,
			       QPainter *p,
			       const QWidget *widget,
			       const QRect &r,
			       const QColorGroup &cg,
			       SFlags flags,
			       const QStyleOption& opt ) const
{
    switch( element ) {
    case CE_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
 	    int diw, x1, y1, x2, y2;
 	    const QPushButton *btn;
	    QColorGroup newCg = cg;
 	    btn = ( const QPushButton * )widget;
 	    p->setPen( cg.foreground() );
 	    p->setBrush( QBrush( cg.button(), NoBrush ) );
 	    diw = pixelMetric( PM_ButtonDefaultIndicator );
 	    r.coords( &x1, &y1, &x2, &y2 );
 	    if ( btn->isDefault() || btn->autoDefault() ) {
 		x1 += diw;
 		y1 += diw;
 		x2 -= diw;
 		y2 -= diw;
 	    }
 	    QBrush fill;
 	    if ( btn->isDown() )
 		fill = newCg.brush( QColorGroup::Mid );
 	    else if ( btn->isOn() )
 		fill = QBrush( newCg.mid(), Dense4Pattern );
 	    else
 		fill = newCg.brush( QColorGroup::Button );

	    newCg.setBrush( QColorGroup::Button, fill );
 	    if ( btn->isDefault() ) {
 		if ( diw == 0 ) {
		    QPointArray a;
		    a.setPoints( 9,
				 x1, y1, x2, y1, x2, y2, x1, y2, x1, y1+1,
				 x2-1, y1+1, x2-1, y2-1, x1+1, y2-1, x1+1, y1+1 );
 		    p->setPen( newCg.shadow() );
 		    p->drawPolygon( a );
 		    x1 += 2;
 		    y1 += 2;
 		    x2 -= 2;
 		    y2 -= 2;
 		} else {
 		    qDrawShadePanel( p, r, newCg, TRUE );
 		}
 	    }
 	    if ( !btn->isFlat() || btn->isOn() || btn->isDown() ) {
		QRect tmp( x1, y1, x2 - x1 + 1, y2 - y1 + 1 );
		SFlags flags = Style_Default;
		if ( btn->isOn())
		    flags |= Style_On;
		if (btn->isDown())
		    flags |= Style_Down;
		p->save();
		p->setBrushOrigin( -widget->backgroundOffset().x(),
				   -widget->backgroundOffset().y() );
		drawPrimitive( PE_ButtonCommand, p,
			       tmp, newCg,
 			       flags );
		p->restore();
	    }
 	    if ( p->brush().style() != NoBrush )
 		p->setBrush( NoBrush );
#endif
	    break;
	}

    case CE_TabBarTab:
	{
#ifndef QT_NO_TABBAR
	    if ( !widget || !widget->parentWidget() || !opt.tab() )
		break;

	    const QTabBar * tb = (const QTabBar *) widget;
	    const QTab * t = opt.tab();

	    int dfw = pixelMetric( PM_DefaultFrameWidth, tb );
	    bool selected = flags & Style_Selected;
	    int o =  dfw > 1 ? 1 : 0;
	    bool lastTab = FALSE;

	    QRect r2( r );
	    if ( tb->shape() == QTabBar::RoundedAbove ) {
		if ( styleHint( SH_TabBar_Alignment, tb ) == AlignRight &&
		     tb->indexOf( t->identifier() ) == tb->count()-1 )
		    lastTab = TRUE;

		if ( o ) {
		    p->setPen( tb->colorGroup().light() );
		    p->drawLine( r2.left(), r2.bottom(), r2.right(), r2.bottom() );
		    p->setPen( tb->colorGroup().light() );
		    p->drawLine( r2.left(), r2.bottom()-1, r2.right(), r2.bottom()-1 );
		    if ( r2.left() == 0 )
			p->drawPoint( tb->rect().bottomLeft() );
		}
		else {
		    p->setPen( tb->colorGroup().light() );
		    p->drawLine( r2.left(), r2.bottom(), r2.right(), r2.bottom() );
		}

		if ( selected ) {
		    p->fillRect( QRect( r2.left()+1, r2.bottom()-o, r2.width()-3, 2),
				 tb->palette().active().brush( QColorGroup::Background ));
		    p->setPen( tb->colorGroup().background() );
		    // p->drawLine( r2.left()+1, r2.bottom(), r2.right()-2, r2.bottom() );
		    // if (o)
		    // p->drawLine( r2.left()+1, r2.bottom()-1, r2.right()-2, r2.bottom()-1 );
		    p->drawLine( r2.left()+1, r2.bottom(), r2.left()+1, r2.top()+2 );
		    p->setPen( tb->colorGroup().light() );
		} else {
		    p->setPen( tb->colorGroup().light() );
		    r2.setRect( r2.left() + 2, r2.top() + 2,
				r2.width() - 4, r2.height() - 2 );
		}

		p->drawLine( r2.left(), r2.bottom()-1, r2.left(), r2.top() + 2 );
		p->drawPoint( r2.left()+1, r2.top() + 1 );
		p->drawLine( r2.left()+2, r2.top(),
			     r2.right() - 2, r2.top() );
		p->drawPoint( r2.left(), r2.bottom());

		if ( o ) {
		    p->drawLine( r2.left()+1, r2.bottom(), r2.left()+1, r2.top() + 2 );
		    p->drawLine( r2.left()+2, r2.top()+1,
				 r2.right() - 2, r2.top()+1 );
		}

		p->setPen( tb->colorGroup().dark() );
		p->drawLine( r2.right() - 1, r2.top() + 2,
			     r2.right() - 1, r2.bottom() - 1 + (selected ? o : -o));
		if ( o ) {
		    p->drawPoint( r2.right() - 1, r2.top() + 1 );
		    p->drawLine( r2.right(), r2.top() + 2, r2.right(),
				 r2.bottom() -
				 (selected ? (lastTab ? 0:1):1+o));
		    p->drawPoint( r2.right() - 1, r2.top() + 1 );
		}
	    } else if ( tb->shape()  == QTabBar::RoundedBelow ) {
		if ( styleHint( SH_TabBar_Alignment, tb ) == AlignLeft &&
		     tb->indexOf( t->identifier() ) == tb->count()-1 )
		    lastTab = TRUE;
		if ( selected ) {
		    p->fillRect( QRect( r2.left()+1, r2.top(), r2.width()-3, 1),
				 tb->palette().active().brush( QColorGroup::Background ));
		    p->setPen( tb->colorGroup().background() );
		    // p->drawLine( r2.left()+1, r2.top(), r2.right()-2, r2.top() );
		    p->drawLine( r2.left()+1, r2.top(), r2.left()+1, r2.bottom()-2 );
		    p->setPen( tb->colorGroup().dark() );
		} else {
		    p->setPen( tb->colorGroup().dark() );
		    p->drawLine( r2.left(), r2.top(), r2.right(), r2.top() );
		    p->drawLine( r2.left() + 1, r2.top() + 1,
				 r2.right() - (lastTab ? 0 : 2),
				 r2.top() + 1 );
		    r2.setRect( r2.left() + 2, r2.top(),
				r2.width() - 4, r2.height() - 2 );
		}

		p->drawLine( r2.right() - 1, r2.top(),
			     r2.right() - 1, r2.bottom() - 2 );
		p->drawPoint( r2.right() - 2, r2.bottom() - 2 );
		p->drawLine( r2.right() - 2, r2.bottom() - 1,
			     r2.left() + 1, r2.bottom() - 1 );
		p->drawPoint( r2.left() + 1, r2.bottom() - 2 );

		if (dfw > 1) {
		    p->drawLine( r2.right(), r2.top(),
				 r2.right(), r2.bottom() - 1 );
		    p->drawPoint( r2.right() - 1, r2.bottom() - 1 );
		    p->drawLine( r2.right() - 1, r2.bottom(),
				 r2.left() + 2, r2.bottom() );
		}

		p->setPen( tb->colorGroup().light() );
		p->drawLine( r2.left(), r2.top() + (selected ? 0 : 2),
			     r2.left(), r2.bottom() - 2 );
		p->drawLine( r2.left() + 1, r2.top() + (selected ? 0 : 2),
			     r2.left() + 1, r2.bottom() - 3 );

	    } else {
		QCommonStyle::drawControl( element, p, widget, r, cg, flags, opt );
	    }
#endif
	    break;
	}

    case CE_ProgressBarGroove:
	qDrawShadePanel(p, r, cg, TRUE, 2);
	break;

    case CE_ProgressBarLabel:
	{
#ifndef QT_NO_PROGRESSBAR
	    const QProgressBar * pb = (const QProgressBar *) widget;
	    const int unit_width = pixelMetric( PM_ProgressBarChunkWidth, pb );
	    int u = r.width() / unit_width;
	    int p_v = pb->progress();
	    int t_s = pb->totalSteps();
	    if ( u > 0 && pb->progress() >= INT_MAX / u && t_s >= u ) {
		// scale down to something usable.
		p_v /= u;
		t_s /= u;
	    }
	    if ( pb->percentageVisible() && pb->totalSteps() ) {
		int nu = ( u * p_v + t_s/2 ) / t_s;
		int x = unit_width * nu;
		if (pb->indicatorFollowsStyle() || pb->centerIndicator()) {
		    p->setPen( cg.highlightedText() );
		    p->setClipRect( r.x(), r.y(), x, r.height() );
		    p->drawText( r, AlignCenter | SingleLine, pb->progressString() );

		    if ( pb->progress() != pb->totalSteps() ) {
			p->setClipRect( r.x() + x, r.y(), r.width() - x, r.height() );
			p->setPen( cg.highlight() );
			p->drawText( r, AlignCenter | SingleLine, pb->progressString() );
		    }
		} else {
		    p->setPen( cg.text() );
		    p->drawText( r, AlignCenter | SingleLine, pb->progressString() );
		}
	    }
#endif
	    break;
	}

#ifndef QT_NO_POPUPMENU
    case CE_PopupMenuItem:
	{
	    if (! widget || opt.isDefault())
		break;

	    const QPopupMenu *popupmenu = (const QPopupMenu *) widget;
	    QMenuItem *mi = opt.menuItem();
	    if ( !mi )
		break;

	    int tab = opt.tabWidth();
	    int maxpmw = opt.maxIconWidth();
	    bool dis = ! (flags & Style_Enabled);
	    bool checkable = popupmenu->isCheckable();
	    bool act = flags & Style_Active;
	    int x, y, w, h;

	    r.rect(&x, &y, &w, &h);

	    if ( checkable )
		maxpmw = QMAX( maxpmw, motifCheckMarkSpace );

	    int checkcol = maxpmw;

	    if ( mi && mi->isSeparator() ) {                    // draw separator
		p->setPen( cg.dark() );
		p->drawLine( x, y, x+w, y );
		p->setPen( cg.light() );
		p->drawLine( x, y+1, x+w, y+1 );
		return;
	    }

	    int pw = motifItemFrame;

	    if ( act && !dis ) {                        // active item frame
		if (pixelMetric( PM_DefaultFrameWidth ) > 1)
		    qDrawShadePanel( p, x, y, w, h, cg, FALSE, pw,
				     &cg.brush( QColorGroup::Button ) );
		else
		    qDrawShadePanel( p, x+1, y+1, w-2, h-2, cg, TRUE, 1,
				     &cg.brush( QColorGroup::Button ) );
	    }
	    else                                // incognito frame
		p->fillRect(x, y, w, h, cg.brush( QColorGroup::Button ));

	    if ( !mi )
		return;

	    QRect vrect = visualRect( QRect( x+motifItemFrame, y+motifItemFrame, checkcol, h-2*motifItemFrame ), r );
	    int xvis = vrect.x();
	    if ( mi->isChecked() ) {
		if ( mi->iconSet() ) {
		    qDrawShadePanel( p, xvis, y+motifItemFrame, checkcol, h-2*motifItemFrame,
				     cg, TRUE, 1, &cg.brush( QColorGroup::Midlight ) );
		}
	    } else if ( !act ) {
		p->fillRect(xvis, y+motifItemFrame, checkcol, h-2*motifItemFrame,
			    cg.brush( QColorGroup::Button ));
	    }

	    if ( mi->iconSet() ) {              // draw iconset
		QIconSet::Mode mode = QIconSet::Normal; // no disabled icons in Motif
		if (act && !dis )
		    mode = QIconSet::Active;
		QPixmap pixmap;
		if ( checkable && mi->isChecked() )
		    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
		else
		    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );

		int pixw = pixmap.width();
		int pixh = pixmap.height();
		QRect pmr( 0, 0, pixw, pixh );
		pmr.moveCenter( vrect.center() );
		p->setPen( cg.text() );
		p->drawPixmap( pmr.topLeft(), pixmap );

	    } else  if ( checkable ) {  // just "checking"...
		int mw = checkcol;
		int mh = h - 2*motifItemFrame;
		if ( mi->isChecked() ) {
		    SFlags cflags = Style_Default;
		    if (! dis)
			cflags |= Style_Enabled;
		    if (act)
			cflags |= Style_On;

		    drawPrimitive(PE_CheckMark, p,
				  QRect(xvis, y+motifItemFrame, mw, mh),
				  cg, cflags);
		}
	    }


	    p->setPen( cg.buttonText() );

	    QColor discol;
	    if ( dis ) {
		discol = cg.text();
		p->setPen( discol );
	    }

	    int xm = motifItemFrame + checkcol + motifItemHMargin;

	    vrect = visualRect( QRect( x+xm, y+motifItemVMargin, w-xm-tab, h-2*motifItemVMargin ), r );
	    xvis = vrect.x();
	    if ( mi->custom() ) {
		int m = motifItemVMargin;
		p->save();
		mi->custom()->paint( p, cg, act, !dis,
				     xvis, y+m, w-xm-tab+1, h-2*m );
		p->restore();
	    }
	    QString s = mi->text();
	    if ( !s.isNull() ) {                        // draw text
		int t = s.find( '\t' );
		int m = motifItemVMargin;
		int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
		text_flags |= (QApplication::reverseLayout() ? AlignRight : AlignLeft );
		if ( t >= 0 ) {                         // draw tab text
		    QRect vr = visualRect( QRect( x+w-tab-motifItemHMargin-motifItemFrame,
						  y+motifItemVMargin, tab, h-2*motifItemVMargin ), r );
		    int xv = vr.x();
		    p->drawText( xv, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
		    s = s.left( t );
		}
		p->drawText( xvis, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );
	    } else if ( mi->pixmap() ) {                        // draw pixmap
		QPixmap *pixmap = mi->pixmap();
		if ( pixmap->depth() == 1 )
		    p->setBackgroundMode( OpaqueMode );
		p->drawPixmap( xvis, y+motifItemFrame, *pixmap );
		if ( pixmap->depth() == 1 )
		    p->setBackgroundMode( TransparentMode );
	    }
	    if ( mi->popup() ) {                        // draw sub menu arrow
		int dim = (h-2*motifItemFrame) / 2;
		QStyle::PrimitiveElement arrow = (QApplication::reverseLayout() ? PE_ArrowLeft : PE_ArrowRight);
		QRect vr = visualRect( QRect(x+w - motifArrowHMargin - motifItemFrame - dim,
					y+h/2-dim/2, dim, dim), r );
		if ( act )
		    drawPrimitive(arrow, p, vr, cg,
				  (Style_Down |
				  (dis ? Style_Default : Style_Enabled)) );
		else
		    drawPrimitive(arrow, p, vr, cg,
				  (dis ? Style_Default : Style_Enabled));
	    }

	    break;
	}
#endif // QT_NO_POPUPMENU

    case CE_MenuBarItem:
	{
	    if ( flags & Style_Active )  // active item
		qDrawShadePanel( p, r, cg, FALSE, motifItemFrame,
				 &cg.brush(QColorGroup::Button) );
	    else  // other item
		p->fillRect( r, cg.brush(QColorGroup::Button) );
	    QCommonStyle::drawControl( element, p, widget, r, cg, flags, opt );
	    break;
	}

    default:
	QCommonStyle::drawControl( element, p, widget, r, cg, flags, opt );
	break;
    }
}

static int get_combo_extra_width( int h, int w, int *return_awh=0 )
{
    int awh,
	tmp;
    if ( h < 8 ) {
        awh = 6;
    } else if ( h < 14 ) {
        awh = h - 2;
    } else {
        awh = h/2;
    }
    tmp = (awh * 3) / 2;
    if ( tmp > w / 2 ) {
	awh = w / 2 - 3;
	tmp = w / 2 + 3;
    }

    if ( return_awh )
        *return_awh = awh;

    return tmp;
}

static void get_combo_parameters( const QRect &r,
                                  int &ew, int &awh, int &ax,
                                  int &ay, int &sh, int &dh,
                                  int &sy )
{
    ew = get_combo_extra_width( r.height(), r.width(), &awh );

    sh = (awh+3)/4;
    if ( sh < 3 )
        sh = 3;
    dh = sh/2 + 1;

    ay = r.y() + (r.height()-awh-sh-dh)/2;
    if ( ay < 0 ) {
        //panic mode
        ay = 0;
        sy = r.height();
    } else {
        sy = ay+awh+dh;
    }
    ax = r.x() + r.width() - ew;
    ax  += (ew-awh)/2;
}

/*!\reimp
*/
void QMotifStyle::drawComplexControl( ComplexControl control,
				     QPainter *p,
				     const QWidget *widget,
				     const QRect &r,
				     const QColorGroup &cg,
				     SFlags flags,
				     SCFlags sub,
				     SCFlags subActive,
				     const QStyleOption& opt ) const
{
    switch ( control ) {
    case CC_SpinWidget: {
	SCFlags drawSub = SC_None;
	if ( sub & SC_SpinWidgetFrame )
	    qDrawShadePanel( p, r, cg, TRUE,
			     pixelMetric( PM_DefaultFrameWidth) );

	if ( sub & SC_SpinWidgetUp || sub & SC_SpinWidgetDown ) {
	    if ( sub & SC_SpinWidgetUp )
		drawSub |= SC_SpinWidgetUp;
	    if ( sub & SC_SpinWidgetDown )
		drawSub |= SC_SpinWidgetDown;

	    QCommonStyle::drawComplexControl( control, p, widget, r, cg, flags,
					      drawSub, subActive, opt );
	}
	break; }

    case CC_Slider:
	{
#ifndef QT_NO_SLIDER
	    const QSlider * slider = (const QSlider *) widget;

	    QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
						  opt),
		  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
						  opt);

	    if ((sub & SC_SliderGroove) && groove.isValid()) {
		qDrawShadePanel( p, groove, cg, TRUE, 2,
				 &cg.brush( QColorGroup::Mid ) );


		if ( flags & Style_HasFocus ) {
		    QRect fr = subRect( SR_SliderFocusRect, widget );
		    drawPrimitive( PE_FocusRect, p, fr, cg );
		}
	    }

	    if (( sub & SC_SliderHandle ) && handle.isValid()) {
		drawPrimitive( PE_ButtonBevel, p, handle, cg );

		if ( slider->orientation() == Horizontal ) {
		    QCOORD mid = handle.x() + handle.width() / 2;
		    qDrawShadeLine( p, mid, handle.y(), mid,
				    handle.y() + handle.height() - 2,
				    cg, TRUE, 1);
		} else {
		    QCOORD mid = handle.y() + handle.height() / 2;
		    qDrawShadeLine( p, handle.x(), mid,
				    handle.x() + handle.width() - 2, mid,
				    cg, TRUE, 1);
		}
	    }

	    if ( sub & SC_SliderTickmarks )
		QCommonStyle::drawComplexControl( control, p, widget, r, cg, flags,
						  SC_SliderTickmarks, subActive,
						  opt );
#endif
	    break;
	}

    case CC_ComboBox:
#ifndef QT_NO_COMBOBOX
	if ( sub & SC_ComboBoxArrow ) {
	    const QComboBox * cb = (const QComboBox *) widget;
	    int awh, ax, ay, sh, sy, dh, ew;
	    int fw = pixelMetric( PM_DefaultFrameWidth, cb);

	    drawPrimitive( PE_ButtonCommand, p, r, cg, flags );
	    QRect ar = QStyle::visualRect( querySubControlMetrics( CC_ComboBox, cb, SC_ComboBoxArrow,
								   opt ), cb );
	    drawPrimitive( PE_ArrowDown, p, ar, cg, flags | Style_Enabled );

	    QRect tr = r;
	    tr.addCoords( fw, fw, -fw, -fw );
	    get_combo_parameters( tr, ew, awh, ax, ay, sh, dh, sy );

	    // draws the shaded line under the arrow
	    p->setPen( cg.light() );
	    p->drawLine( ar.x(), sy, ar.x()+awh-1, sy );
	    p->drawLine( ar.x(), sy, ar.x(), sy+sh-1 );
	    p->setPen( cg.dark() );
	    p->drawLine( ar.x()+1, sy+sh-1, ar.x()+awh-1, sy+sh-1 );
	    p->drawLine( ar.x()+awh-1, sy+1, ar.x()+awh-1, sy+sh-1 );

	    if ( cb->hasFocus() ) {
		QRect re = QStyle::visualRect( subRect( SR_ComboBoxFocusRect, cb ), cb );
		drawPrimitive( PE_FocusRect, p, re, cg );
	    }
	}

	if ( sub & SC_ComboBoxEditField ) {
	    QComboBox * cb = (QComboBox *) widget;
	    if ( cb->editable() ) {
		QRect er = QStyle::visualRect( querySubControlMetrics( CC_ComboBox, cb,
								       SC_ComboBoxEditField ), cb );
		er.addCoords( -1, -1, 1, 1);
		qDrawShadePanel( p, er, cg, TRUE, 1,
				 &cg.brush( QColorGroup::Button ));
	    }
	}
#endif
	p->setPen(cg.buttonText());
	break;

    case CC_ScrollBar:
	{
	    if (sub == (SC_ScrollBarAddLine | SC_ScrollBarSubLine | SC_ScrollBarAddPage |
			SC_ScrollBarSubPage | SC_ScrollBarFirst | SC_ScrollBarLast |
			SC_ScrollBarSlider))
		qDrawShadePanel(p, widget->rect(), cg, TRUE,
				pixelMetric(PM_DefaultFrameWidth, widget),
				&cg.brush(QColorGroup::Mid));
	    QCommonStyle::drawComplexControl(control, p, widget, r, cg, flags, sub,
					     subActive, opt);
	    break;
	}

#ifndef QT_NO_LISTVIEW
    case CC_ListView:
	{
	    if ( sub & SC_ListView ) {
		QCommonStyle::drawComplexControl( control, p, widget, r, cg, flags, sub, subActive, opt );
	    }
	    if ( sub & ( SC_ListViewBranch | SC_ListViewExpand ) ) {
		if (opt.isDefault())
		    break;

		QListViewItem *item = opt.listViewItem();
		QListViewItem *child = item->firstChild();

		int y = r.y();
		int c;
		QPointArray dotlines;
		if ( subActive == (uint)SC_All && sub == SC_ListViewExpand ) {
		    c = 2;
		    dotlines.resize(2);
		    dotlines[0] = QPoint( r.right(), r.top() );
		    dotlines[1] = QPoint( r.right(), r.bottom() );
		} else {
		    int linetop = 0, linebot = 0;
		    // each branch needs at most two lines, ie. four end points
		    dotlines.resize( item->childCount() * 4 );
		    c = 0;

		    // skip the stuff above the exposed rectangle
		    while ( child && y + child->height() <= 0 ) {
			y += child->totalHeight();
			child = child->nextSibling();
		    }

		    int bx = r.width() / 2;

		    // paint stuff in the magical area
		    QListView* v = item->listView();
		    while ( child && y < r.height() ) {
			if (child->isVisible()) {
			    int lh;
			    if ( !item->multiLinesEnabled() )
				lh = child->height();
			    else
				lh = p->fontMetrics().height() + 2 * v->itemMargin();
			    lh = QMAX( lh, QApplication::globalStrut().height() );
			    if ( lh % 2 > 0 )
				lh++;
			    linebot = y + lh/2;
			    if ( (child->isExpandable() || child->childCount()) &&
				 (child->height() > 0) ) {
				// needs a box
				p->setPen( cg.text() );
				p->drawRect( bx-4, linebot-4, 9, 9 );
				QPointArray a;
				if ( child->isOpen() )
				    a.setPoints( 3, bx-2, linebot-2,
						 bx, linebot+2,
						 bx+2, linebot-2 ); //RightArrow
				else
				    a.setPoints( 3, bx-2, linebot-2,
						 bx+2, linebot,
						 bx-2, linebot+2 ); //DownArrow
				p->setBrush( cg.text() );
				p->drawPolygon( a );
				p->setBrush( NoBrush );
				// dotlinery
				dotlines[c++] = QPoint( bx, linetop );
				dotlines[c++] = QPoint( bx, linebot - 5 );
				dotlines[c++] = QPoint( bx + 5, linebot );
				dotlines[c++] = QPoint( r.width(), linebot );
				linetop = linebot + 5;
			    } else {
				// just dotlinery
				dotlines[c++] = QPoint( bx+1, linebot );
				dotlines[c++] = QPoint( r.width(), linebot );
			    }
			    y += child->totalHeight();
			}
			child = child->nextSibling();
		    }

		    // Expand line height to edge of rectangle if there's any
		    // visible child below
		    while ( child && child->height() <= 0)
			child = child->nextSibling();
		    if ( child )
			linebot = r.height();

		    if ( linetop < linebot ) {
			dotlines[c++] = QPoint( bx, linetop );
			dotlines[c++] = QPoint( bx, linebot );
		    }
		}

		int line; // index into dotlines
		p->setPen( cg.text() );
		if ( sub & SC_ListViewBranch ) for( line = 0; line < c; line += 2 ) {
		    p->drawLine( dotlines[line].x(), dotlines[line].y(),
				 dotlines[line+1].x(), dotlines[line+1].y() );
		}
	    }

	    break;
	}
#endif // QT_NO_LISTVIEW

    default:
	QCommonStyle::drawComplexControl( control, p, widget, r, cg, flags,
					  sub, subActive, opt );
    }
}


/*! \reimp */
int QMotifStyle::pixelMetric( PixelMetric metric, const QWidget *widget ) const
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

    case PM_SplitterWidth:
	ret = QMAX( 10, QApplication::globalStrut().width() );
	break;

    case PM_SliderLength:
	ret = 30;
	break;

    case PM_SliderThickness:
	ret = 24;
	break;

    case PM_SliderControlThickness:
	{
#ifndef QT_NO_SLIDER
	    const QSlider * sl = (const QSlider *) widget;
	    int space = (sl->orientation() == Horizontal) ? sl->height()
			: sl->width();
	    int ticks = sl->tickmarks();
	    int n = 0;
	    if ( ticks & QSlider::Above ) n++;
	    if ( ticks & QSlider::Below ) n++;
	    if ( !n ) {
		ret = space;
		break;
	    }

	    int thick = 6;	// Magic constant to get 5 + 16 + 5

	    space -= thick;
	    //### the two sides may be unequal in size
	    if ( space > 0 )
		thick += (space * 2) / (n + 2);
	    ret = thick;
#endif
	    break;
	}

    case PM_SliderSpaceAvailable:
	{
#ifndef QT_NO_SLIDER
	    const QSlider * sl = (const QSlider *) widget;
	    if ( sl->orientation() == Horizontal )
		ret = sl->width() - pixelMetric( PM_SliderLength, sl ) - 6;
	    else
		ret = sl->height() - pixelMetric( PM_SliderLength, sl ) - 6;
#endif
	    break;
	}

    case PM_DockWindowHandleExtent:
	ret = 9;
	break;

    case PM_ProgressBarChunkWidth:
	ret = 1;
	break;

    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
	ret = 13;
	break;

    default:
	ret =  QCommonStyle::pixelMetric( metric, widget );
	break;
    }
    return ret;
}


/*!\reimp
*/
QRect QMotifStyle::querySubControlMetrics( ComplexControl control,
					   const QWidget *widget,
					   SubControl sc,
					   const QStyleOption& opt ) const
{
    switch ( control ) {
    case CC_SpinWidget: {
	if ( !widget )
	    return QRect();
	int fw = pixelMetric( PM_SpinBoxFrameWidth, 0 );
	QSize bs;
	bs.setHeight( widget->height()/2 );
	if ( bs.height() < 8 )
	    bs.setHeight( 8 );
	bs.setWidth( QMIN( bs.height() * 8 / 5, widget->width() / 4 ) ); // 1.6 -approximate golden mean
	bs = bs.expandedTo( QApplication::globalStrut() );
	int y = 0;
	int x, lx, rx;
	x = widget->width() - y - bs.width();
	lx = fw;
	rx = x - fw * 2;
	switch ( sc ) {
	case SC_SpinWidgetUp:
	    return QRect(x, y, bs.width(), bs.height());
	case SC_SpinWidgetDown:
	    return QRect(x, y + bs.height(), bs.width(), bs.height());
	case SC_SpinWidgetButtonField:
	    return QRect(x, y, bs.width(), widget->height() - 2*fw);
	case SC_SpinWidgetEditField:
	    return QRect(lx, fw, rx, widget->height() - 2*fw);
	case SC_SpinWidgetFrame:
	    return QRect( 0, 0,
			  widget->width() - bs.width(), widget->height() );
	default:
	    break;
	}
	break; }

#ifndef QT_NO_SLIDER
    case CC_Slider: {
	if (sc == SC_SliderHandle) {
	    const QSlider * sl = (const QSlider *) widget;
	    int tickOffset  = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int thickness   = pixelMetric( PM_SliderControlThickness, sl );
	    int sliderPos   = sl->sliderStart();
	    int len         = pixelMetric( PM_SliderLength, sl );
	    int motifBorder = 3;

	    if ( sl->orientation() == Horizontal )
		return QRect( sliderPos + motifBorder, tickOffset + motifBorder, len,
			      thickness - 2*motifBorder );
	    return QRect( tickOffset + motifBorder, sliderPos + motifBorder,
			  thickness - 2*motifBorder, len );
	}
	break; }
#endif

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	if (! widget)
	    return QRect();

	const QScrollBar *scrollbar = (const QScrollBar *) widget;
	int sliderstart = scrollbar->sliderStart();
	int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
	int fw = pixelMetric(PM_DefaultFrameWidth, widget);
	int buttonw = sbextent - (fw * 2);
	int buttonh = sbextent - (fw * 2);
	int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
		      scrollbar->width() : scrollbar->height()) -
		     (buttonw * 2) - (fw * 2);
	int sliderlen;

	// calculate slider length
	if (scrollbar->maxValue() != scrollbar->minValue()) {
	    uint range = scrollbar->maxValue() - scrollbar->minValue();
	    sliderlen = (scrollbar->pageStep() * maxlen) /
			(range + scrollbar->pageStep());

	    if ( sliderlen < 9 || range > INT_MAX/2 )
		sliderlen = 9;
	    if ( sliderlen > maxlen )
		sliderlen = maxlen;
	} else
	    sliderlen = maxlen;

	switch (sc) {
	case SC_ScrollBarSubLine:
	    // top/left button
	    if (scrollbar->orientation() == Qt::Horizontal) {
		if ( scrollbar->width()/2 < sbextent )
		    buttonw = scrollbar->width()/2 - (fw*2);
		return QRect(fw, fw, buttonw, buttonh);
	    } else {
		if ( scrollbar->height()/2 < sbextent )
		    buttonh = scrollbar->height()/2 - (fw*2);
		return QRect(fw, fw, buttonw, buttonh);
	    }
	case SC_ScrollBarAddLine:
	    // bottom/right button
	    if (scrollbar->orientation() == Qt::Horizontal) {
		if ( scrollbar->width()/2 < sbextent )
		    buttonw = scrollbar->width()/2 - (fw*2);
		return QRect(scrollbar->width() - buttonw - fw, fw,
			     buttonw, buttonh);
	    } else {
		if ( scrollbar->height()/2 < sbextent )
		    buttonh = scrollbar->height()/2 - (fw*2);
		return QRect(fw, scrollbar->height() - buttonh - fw,
			     buttonw, buttonh);
	    }
	case SC_ScrollBarSubPage:
	    if (scrollbar->orientation() == Qt::Horizontal)
		return QRect(buttonw + fw, fw, sliderstart - buttonw - fw, buttonw);
	    return QRect(fw, buttonw + fw, buttonw, sliderstart - buttonw - fw);

	case SC_ScrollBarAddPage:
	    if (scrollbar->orientation() == Qt::Horizontal)
		return QRect(sliderstart + sliderlen, fw,
			     maxlen - sliderstart - sliderlen + buttonw + fw, buttonw);
	    return QRect(fw, sliderstart + sliderlen, buttonw,
			 maxlen - sliderstart - sliderlen + buttonw + fw);

	case SC_ScrollBarGroove:
	    if (scrollbar->orientation() == Qt::Horizontal)
		return QRect(buttonw + fw, fw, maxlen, buttonw);
	    return QRect(fw, buttonw + fw, buttonw, maxlen);

	case SC_ScrollBarSlider:
	    if (scrollbar->orientation() == Qt::Horizontal)
		return QRect(sliderstart, fw, sliderlen, buttonw);
	    return QRect(fw, sliderstart, buttonw, sliderlen);

	default:
	    break;
	}
	break; }
#endif

#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:

	switch ( sc ) {
	case SC_ComboBoxArrow: {
	    const QComboBox * cb = (const QComboBox *) widget;
	    int ew, awh, sh, dh, ax, ay, sy;
	    int fw = pixelMetric( PM_DefaultFrameWidth, cb );
	    QRect cr = cb->rect();
	    cr.addCoords( fw, fw, -fw, -fw );
	    get_combo_parameters( cr, ew, awh, ax, ay, sh, dh, sy );
	    return QRect( ax, ay, awh, awh ); }

	case SC_ComboBoxEditField: {
	    const QComboBox * cb = (const QComboBox *) widget;
	    int fw = pixelMetric( PM_DefaultFrameWidth, cb );
	    QRect rect = cb->rect();
	    rect.addCoords( fw, fw, -fw, -fw );
	    int ew = get_combo_extra_width( rect.height(), rect.width() );
	    rect.addCoords( 1, 1, -1-ew, -1 );
	    return rect; }

	default:
	    break;
	}
	break;
#endif
    default: break;
    }
    return QCommonStyle::querySubControlMetrics( control, widget, sc, opt );
}

/*!\reimp
*/
QSize QMotifStyle::sizeFromContents( ContentsType contents,
				     const QWidget *widget,
				     const QSize &contentsSize,
				     const QStyleOption& opt ) const
{
    QSize sz(contentsSize);

    switch(contents) {
    case CT_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    sz = QCommonStyle::sizeFromContents(contents, widget, contentsSize, opt);
	    if ((button->isDefault() || button->autoDefault()) &&
		sz.width() < 80 && ! button->pixmap())
		sz.setWidth(80);
#endif
	    break;
	}

    case CT_PopupMenuItem:
	{
#ifndef QT_NO_POPUPMENU
	    if (! widget || opt.isDefault())
		break;

	    const QPopupMenu *popup = (QPopupMenu *) widget;
	    bool checkable = popup->isCheckable();
	    QMenuItem *mi = opt.menuItem();
	    int maxpmw = opt.maxIconWidth();
	    int w = sz.width(), h = sz.height();

	    if (mi->custom()) {
		w = mi->custom()->sizeHint().width();
		h = mi->custom()->sizeHint().height();
		if (! mi->custom()->fullSpan())
		    h += 2*motifItemVMargin + 2*motifItemFrame;
	    } else if ( mi->widget() ) {
	    } else if ( mi->isSeparator() ) {
		w = 10;
		h = motifSepHeight;
	    } else if (mi->pixmap() || ! mi->text().isNull())
		h += 2*motifItemVMargin + 2*motifItemFrame;

	    // a little bit of border can never harm
	    w += 2*motifItemHMargin + 2*motifItemFrame;

	    if ( !mi->text().isNull() && mi->text().find('\t') >= 0 )
		// string contains tab
		w += motifTabSpacing;
	    else if (mi->popup())
		// submenu indicator needs some room if we don't have a tab column
		w += motifArrowHMargin + 4*motifItemFrame;

	    if ( checkable && maxpmw <= 0)
		// if we are checkable and have no iconsets, add space for a checkmark
		w += motifCheckMarkSpace;
	    else if (checkable && maxpmw < motifCheckMarkSpace)
		// make sure the check-column is wide enough if we have iconsets
		w += (motifCheckMarkSpace - maxpmw);

	    // if we have a check-column ( iconsets of checkmarks), add space
	    // to separate the columns
	    if ( maxpmw > 0 || checkable )
		w += motifCheckMarkHMargin;

	    sz = QSize(w, h);
#endif
	    break;
	}

    default:
	sz = QCommonStyle::sizeFromContents( contents, widget, contentsSize, opt );
	break;
    }

    return sz;
}

/*!\reimp
*/
QRect QMotifStyle::subRect( SubRect r, const QWidget *widget ) const
{
    QRect rect;
    QRect wrect = widget->rect();

    switch ( r ) {
    case SR_SliderFocusRect:
	rect = QCommonStyle::subRect( r, widget );
	rect.addCoords( 2, 2, -2, -2 );
	break;

    case SR_ComboBoxFocusRect:
	{
	    int awh, ax, ay, sh, sy, dh, ew;
	    int fw = pixelMetric( PM_DefaultFrameWidth, widget );
	    QRect tr = wrect;

	    tr.addCoords( fw, fw, -fw, -fw );
	    get_combo_parameters( tr, ew, awh, ax, ay, sh, dh, sy );
	    rect.setRect(ax-2, ay-2, awh+4, awh+sh+dh+4);
	    break;
	}

    case SR_DockWindowHandleRect:
	{
#ifndef QT_NO_MAINWINDOW
	    if ( !widget || !widget->parent() )
		break;

	    const QDockWindow * dw = (const QDockWindow *) widget->parent();
	    if ( !dw->area() || !dw->isCloseEnabled() )
		rect.setRect( 0, 0, widget->width(), widget->height() );
	    else {
		if ( dw->area()->orientation() == Horizontal )
		    rect.setRect(2, 15, widget->width()-2, widget->height() - 15);
		else
		    rect.setRect(0, 2, widget->width() - 15, widget->height() - 2);
	    }
#endif
	    break;
	}

    case SR_ProgressBarGroove:
    case SR_ProgressBarContents:
	{
#ifndef QT_NO_PROGRESSBAR
	    QFontMetrics fm( ( widget ? widget->fontMetrics() :
			       QApplication::fontMetrics() ) );
	    const QProgressBar *progressbar = (const QProgressBar *) widget;
	    int textw = 0;
	    if (progressbar->percentageVisible())
		textw = fm.width("100%") + 6;

	    if (progressbar->indicatorFollowsStyle() ||
		progressbar->centerIndicator())
		rect = wrect;
	    else
		rect.setCoords(wrect.left(), wrect.top(),
			       wrect.right() - textw, wrect.bottom());
#endif
	    break;
	}

    case SR_ProgressBarLabel:
	{
#ifndef QT_NO_PROGRESSBAR
	    QFontMetrics fm( ( widget ? widget->fontMetrics() :
			       QApplication::fontMetrics() ) );
	    const QProgressBar *progressbar = (const QProgressBar *) widget;
	    int textw = 0;
	    if (progressbar->percentageVisible())
		textw = fm.width("100%") + 6;

	    if (progressbar->indicatorFollowsStyle() ||
		progressbar->centerIndicator())
		rect = wrect;
	    else
		rect.setCoords(wrect.right() - textw, wrect.top(),
			       wrect.right(), wrect.bottom());
#endif
	    break;
	}

    case SR_CheckBoxContents:
	{
#ifndef QT_NO_CHECKBOX
	    QRect ir = subRect(SR_CheckBoxIndicator, widget);
	    rect.setRect(ir.right() + 10, wrect.y(),
			 wrect.width() - ir.width() - 10, wrect.height());
#endif
	    break;
	}

    case SR_RadioButtonContents:
	{
	    QRect ir = subRect(SR_RadioButtonIndicator, widget);
	    rect.setRect(ir.right() + 10, wrect.y(),
			 wrect.width() - ir.width() - 10, wrect.height());
	    break;
	}

    default:
	rect = QCommonStyle::subRect( r, widget );
    }

    return rect;
}

/*! \reimp
*/
void QMotifStyle::polishPopupMenu( QPopupMenu* p)
{
#ifndef QT_NO_POPUPMENU
    if ( !p->testWState( WState_Polished ) )
        p->setCheckable( FALSE );
#endif
}


#ifndef QT_NO_IMAGEIO_XPM
static const char * const qt_close_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"   .    .   ",
"  ...  ...  ",
"   ......   ",
"    ....    ",
"    ....    ",
"   ......   ",
"  ...  ...  ",
"   .    .   ",
"            ",
"            "};

static const char * const qt_maximize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"     .      ",
"    ...     ",
"   .....    ",
"  .......   ",
" .........  ",
"            ",
"            ",
"            ",
"            "};

static const char * const qt_minimize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"            ",
" .........  ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            ",
"            ",
"            "};

#if 0 // ### not used???
static const char * const qt_normalize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"  .         ",
"  ..        ",
"  ...       ",
"  ....      ",
"  .....     ",
"  ......    ",
"  .......   ",
"            ",
"            ",
"            "};
#endif

static const char * const qt_normalizeup_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"  .......   ",
"   ......   ",
"    .....   ",
"     ....   ",
"      ...   ",
"       ..   ",
"        .   ",
"            ",
"            "};

static const char * const qt_shade_xpm[] = {
"12 12 2 1", "# c #000000",
". c None",
"............",
"............",
".#########..",
".#########..",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"............"};


static const char * const qt_unshade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
".#########..",
".#########..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#########..",
"............",
"............"};


static const char * dock_window_close_xpm[] = {
"8 8 2 1",
"# c #000000",
". c None",
"##....##",
".##..##.",
"..####..",
"...##...",
"..####..",
".##..##.",
"##....##",
"........"};

// Message box icons, from page 210 of the Windows style guide.

// Hand-drawn to resemble Microsoft's icons, but in the Mac/Netscape palette.
// Thanks to TrueColor displays, it is slightly more efficient to have
// them duplicated.
/* XPM */
static const char * const information_xpm[]={
"32 32 5 1",
". c None",
"c c #000000",
"* c #999999",
"a c #ffffff",
"b c #0000ff",
"...........********.............",
"........***aaaaaaaa***..........",
"......**aaaaaaaaaaaaaa**........",
".....*aaaaaaaaaaaaaaaaaa*.......",
"....*aaaaaaaabbbbaaaaaaaac......",
"...*aaaaaaaabbbbbbaaaaaaaac.....",
"..*aaaaaaaaabbbbbbaaaaaaaaac....",
".*aaaaaaaaaaabbbbaaaaaaaaaaac...",
".*aaaaaaaaaaaaaaaaaaaaaaaaaac*..",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaaac*.",
"*aaaaaaaaaabbbbbbbaaaaaaaaaaac*.",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaabbbbbaaaaaaaaaaac**",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
".*aaaaaaaaaaabbbbbaaaaaaaaaac***",
"..*aaaaaaaaaabbbbbaaaaaaaaac***.",
"...caaaaaaabbbbbbbbbaaaaaac****.",
"....caaaaaaaaaaaaaaaaaaaac****..",
".....caaaaaaaaaaaaaaaaaac****...",
"......ccaaaaaaaaaaaaaacc****....",
".......*cccaaaaaaaaccc*****.....",
"........***cccaaaac*******......",
"..........****caaac*****........",
".............*caaac**...........",
"...............caac**...........",
"................cac**...........",
".................cc**...........",
"..................***...........",
"...................**..........."};
/* XPM */
static const char* const warning_xpm[]={
"32 32 4 1",
". c None",
"a c #ffff00",
"* c #000000",
"b c #999999",
".............***................",
"............*aaa*...............",
"...........*aaaaa*b.............",
"...........*aaaaa*bb............",
"..........*aaaaaaa*bb...........",
"..........*aaaaaaa*bb...........",
".........*aaaaaaaaa*bb..........",
".........*aaaaaaaaa*bb..........",
"........*aaaaaaaaaaa*bb.........",
"........*aaaa***aaaa*bb.........",
".......*aaaa*****aaaa*bb........",
".......*aaaa*****aaaa*bb........",
"......*aaaaa*****aaaaa*bb.......",
"......*aaaaa*****aaaaa*bb.......",
".....*aaaaaa*****aaaaaa*bb......",
".....*aaaaaa*****aaaaaa*bb......",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"....*aaaaaaaa***aaaaaaaa*bb.....",
"...*aaaaaaaaa***aaaaaaaaa*bb....",
"...*aaaaaaaaaa*aaaaaaaaaa*bb....",
"..*aaaaaaaaaaa*aaaaaaaaaaa*bb...",
"..*aaaaaaaaaaaaaaaaaaaaaaa*bb...",
".*aaaaaaaaaaaa**aaaaaaaaaaa*bb..",
".*aaaaaaaaaaa****aaaaaaaaaa*bb..",
"*aaaaaaaaaaaa****aaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaa**aaaaaaaaaaaa*bb.",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaa*bbb",
".*aaaaaaaaaaaaaaaaaaaaaaaaa*bbbb",
"..*************************bbbbb",
"....bbbbbbbbbbbbbbbbbbbbbbbbbbb.",
".....bbbbbbbbbbbbbbbbbbbbbbbbb.."};
/* XPM */
static const char* const critical_xpm[]={
"32 32 4 1",
". c None",
"a c #999999",
"* c #ff0000",
"b c #ffffff",
"...........********.............",
".........************...........",
".......****************.........",
"......******************........",
".....********************a......",
"....**********************a.....",
"...************************a....",
"..*******b**********b*******a...",
"..******bbb********bbb******a...",
".******bbbbb******bbbbb******a..",
".*******bbbbb****bbbbb*******a..",
"*********bbbbb**bbbbb*********a.",
"**********bbbbbbbbbb**********a.",
"***********bbbbbbbb***********aa",
"************bbbbbb************aa",
"************bbbbbb************aa",
"***********bbbbbbbb***********aa",
"**********bbbbbbbbbb**********aa",
"*********bbbbb**bbbbb*********aa",
".*******bbbbb****bbbbb*******aa.",
".******bbbbb******bbbbb******aa.",
"..******bbb********bbb******aaa.",
"..*******b**********b*******aa..",
"...************************aaa..",
"....**********************aaa...",
"....a********************aaa....",
".....a******************aaa.....",
"......a****************aaa......",
".......aa************aaaa.......",
".........aa********aaaaa........",
"...........aaaaaaaaaaa..........",
".............aaaaaaa............"};
/* XPM */
static const char *const question_xpm[] = {
"32 32 5 1",
". c None",
"c c #000000",
"* c #999999",
"a c #ffffff",
"b c #0000ff",
"...........********.............",
"........***aaaaaaaa***..........",
"......**aaaaaaaaaaaaaa**........",
".....*aaaaaaaaaaaaaaaaaa*.......",
"....*aaaaaaaaaaaaaaaaaaaac......",
"...*aaaaaaaabbbbbbaaaaaaaac.....",
"..*aaaaaaaabaaabbbbaaaaaaaac....",
".*aaaaaaaabbaaaabbbbaaaaaaaac...",
".*aaaaaaaabbbbaabbbbaaaaaaaac*..",
"*aaaaaaaaabbbbaabbbbaaaaaaaaac*.",
"*aaaaaaaaaabbaabbbbaaaaaaaaaac*.",
"*aaaaaaaaaaaaabbbbaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbbaaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbaaaaaaaaaaaaac**",
"*aaaaaaaaaaaaabbaaaaaaaaaaaaac**",
"*aaaaaaaaaaaaaaaaaaaaaaaaaaaac**",
".*aaaaaaaaaaaabbaaaaaaaaaaaac***",
".*aaaaaaaaaaabbbbaaaaaaaaaaac***",
"..*aaaaaaaaaabbbbaaaaaaaaaac***.",
"...caaaaaaaaaabbaaaaaaaaaac****.",
"....caaaaaaaaaaaaaaaaaaaac****..",
".....caaaaaaaaaaaaaaaaaac****...",
"......ccaaaaaaaaaaaaaacc****....",
".......*cccaaaaaaaaccc*****.....",
"........***cccaaaac*******......",
"..........****caaac*****........",
".............*caaac**...........",
"...............caac**...........",
"................cac**...........",
".................cc**...........",
"..................***...........",
"...................**...........",
};
#endif

/*!
 \reimp
 */
QPixmap QMotifStyle::stylePixmap(StylePixmap sp,
				 const QWidget *widget,
				 const QStyleOption& opt) const
{
#ifndef QT_NO_IMAGEIO_XPM
    switch (sp) {
    case SP_TitleBarShadeButton:
	return QPixmap((const char **)qt_shade_xpm);
    case SP_TitleBarUnshadeButton:
	return QPixmap((const char **)qt_unshade_xpm);
    case SP_TitleBarNormalButton:
	return QPixmap((const char **)qt_normalizeup_xpm);
    case SP_TitleBarMinButton:
	return QPixmap((const char **)qt_minimize_xpm);
    case SP_TitleBarMaxButton:
	return QPixmap((const char **)qt_maximize_xpm);
    case SP_TitleBarCloseButton:
	return QPixmap((const char **)qt_close_xpm);
    case SP_DockWindowCloseButton:
	return QPixmap((const char **)dock_window_close_xpm );

    case SP_MessageBoxInformation:
    case SP_MessageBoxWarning:
    case SP_MessageBoxCritical:
    case SP_MessageBoxQuestion:
	{
	    const char * const * xpm_data;
	    switch ( sp ) {
	    case SP_MessageBoxInformation:
		xpm_data = information_xpm;
		break;
	    case SP_MessageBoxWarning:
		xpm_data = warning_xpm;
		break;
	    case SP_MessageBoxCritical:
		xpm_data = critical_xpm;
		break;
	    case SP_MessageBoxQuestion:
		xpm_data = question_xpm;
		break;
	    default:
		xpm_data = 0;
		break;
	    }
	    QPixmap pm;
	    if ( xpm_data ) {
		QImage image( (const char **) xpm_data);
		// All that color looks ugly in Motif
		QColorGroup g = QApplication::palette().active();
		switch ( sp ) {
		case SP_MessageBoxInformation:
		case SP_MessageBoxQuestion:
		    image.setColor( 2, 0xff000000 | g.dark().rgb() );
		    image.setColor( 3, 0xff000000 | g.base().rgb() );
		    image.setColor( 4, 0xff000000 | g.text().rgb() );
		    break;
		case SP_MessageBoxWarning:
		    image.setColor( 1, 0xff000000 | g.base().rgb() );
		    image.setColor( 2, 0xff000000 | g.text().rgb() );
		    image.setColor( 3, 0xff000000 | g.dark().rgb() );
		    break;
		case SP_MessageBoxCritical:
		    image.setColor( 1, 0xff000000 | g.dark().rgb() );
		    image.setColor( 2, 0xff000000 | g.text().rgb() );
		    image.setColor( 3, 0xff000000 | g.base().rgb() );
		    break;
		default:
		    break;
		}
		pm.convertFromImage(image);
	    }
	    return pm;
	}

    default:
	break;
    }
#endif

    return QCommonStyle::stylePixmap(sp, widget, opt);
}


/*! \reimp */
int QMotifStyle::styleHint(StyleHint hint,
			   const QWidget *widget,
			   const QStyleOption &opt,
			   QStyleHintReturn *returnData) const
{
    int ret;

    switch (hint) {
    case SH_GUIStyle:
	ret = MotifStyle;
	break;

    case SH_ScrollBar_BackgroundMode:
	ret = QWidget::PaletteMid;
	break;

    case SH_ScrollBar_MiddleClickAbsolutePosition:
    case SH_Slider_SloppyKeyEvents:
    case SH_ProgressDialog_CenterCancelButton:
    case SH_PopupMenu_SpaceActivatesItem:
    case SH_ScrollView_FrameOnlyAroundContents:
	ret = 1;
	break;

    case SH_PopupMenu_SubMenuPopupDelay:
	ret = 96;
	break;

    case SH_ProgressDialog_TextLabelAlignment:
	ret = AlignAuto | AlignVCenter;
	break;

    case SH_ItemView_ChangeHighlightOnFocus:
	ret = 0;
	break;

    default:
	ret = QCommonStyle::styleHint(hint, widget, opt, returnData);
	break;
    }

    return ret;
}


#endif
