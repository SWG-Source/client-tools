/****************************************************************************
** $Id: qcommonstyle.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QCommonStyle class
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

#include "qcommonstyle.h"

#ifndef QT_NO_STYLE

#include "qmenubar.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpushbutton.h"
#include "qtabbar.h"
#include "qscrollbar.h"
#include "qtoolbutton.h"
#include "qtoolbar.h"
#include "qdockarea.h"
#include "qheader.h"
#include "qspinbox.h"
#include "qrangecontrol.h"
#include "qgroupbox.h"
#include "qslider.h"
#include "qlistview.h"
#include "qcheckbox.h"
#include "qradiobutton.h"
#include "qbitmap.h"
#include "qprogressbar.h"
#include "private/qdialogbuttons_p.h"
#include <limits.h>
#include <qpixmap.h>
#include "../widgets/qtitlebar_p.h"
#include <qtoolbox.h>

/*!
    \class QCommonStyle qcommonstyle.h
    \brief The QCommonStyle class encapsulates the common Look and Feel of a GUI.

    \ingroup appearance

    This abstract class implements some of the widget's look and feel
    that is common to all GUI styles provided and shipped as part of
    Qt.

    All the functions are documented in \l QStyle.
*/

/*!
    \enum Qt::ArrowType

    \value UpArrow
    \value DownArrow
    \value LeftArrow
    \value RightArrow

*/

// the active painter, if any... this is used as an optimzation to
// avoid creating a painter if we have an active one (since
// QStyle::itemRect() needs a painter to operate correctly
static QPainter *activePainter = 0;

/*!
    Constructs a QCommonStyle.
*/
QCommonStyle::QCommonStyle() : QStyle()
{
    activePainter = 0;
}

/*! \reimp */
QCommonStyle::~QCommonStyle()
{
    activePainter = 0;
}


static const char * const check_list_controller_xpm[] = {
"16 16 4 1",
"	c None",
".	c #000000000000",
"X	c #FFFFFFFF0000",
"o	c #C71BC30BC71B",
"                ",
"                ",
" ..........     ",
" .XXXXXXXX.     ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" .XXXXXXXX.oo   ",
" ..........oo   ",
"   oooooooooo   ",
"   oooooooooo   ",
"                ",
"                "};

/*! \reimp */
void QCommonStyle::drawPrimitive( PrimitiveElement pe,
				  QPainter *p,
				  const QRect &r,
				  const QColorGroup &cg,
				  SFlags flags,
				  const QStyleOption& opt ) const
{
    activePainter = p;

    switch (pe) {
#ifndef QT_NO_LISTVIEW
    case PE_CheckListController: {
	p->drawPixmap(r, QPixmap((const char **)check_list_controller_xpm));
	break; }
    case PE_CheckListExclusiveIndicator: {
	QCheckListItem *item = opt.checkListItem();
	QListView *lv = item->listView();
	if(!item)
	    return;
	int x = r.x(), y = r.y();
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
	static const QCOORD pts1[] = {		// dark lines
	    1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
	static const QCOORD pts2[] = {		// black lines
	    2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
	static const QCOORD pts3[] = {		// background lines
	    2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
	static const QCOORD pts4[] = {		// white lines
	    2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
	    11,4, 10,3, 10,2 };
	// static const QCOORD pts5[] = {		// inner fill
	//    4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };
	//QPointArray a;
	//	p->eraseRect( x, y, w, h );

	if ( flags & Style_Enabled )
	    p->setPen( cg.text() );
	else
	    p->setPen( QPen( lv->palette().color( QPalette::Disabled, QColorGroup::Text ) ) );
	QPointArray a( QCOORDARRLEN(pts1), pts1 );
	a.translate( x, y );
	//p->setPen( cg.dark() );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts2), pts2 );
	a.translate( x, y );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts3), pts3 );
	a.translate( x, y );
	//		p->setPen( black );
	p->drawPolyline( a );
	a.setPoints( QCOORDARRLEN(pts4), pts4 );
	a.translate( x, y );
	//			p->setPen( blue );
	p->drawPolyline( a );
	//		a.setPoints( QCOORDARRLEN(pts5), pts5 );
	//		a.translate( x, y );
	//	QColor fillColor = isDown() ? g.background() : g.base();
	//	p->setPen( fillColor );
	//	p->setBrush( fillColor );
	//	p->drawPolygon( a );
	if ( flags & Style_On ) {
	    p->setPen( NoPen );
	    p->setBrush( cg.text() );
	    p->drawRect( x+5, y+4, 2, 4 );
	    p->drawRect( x+4, y+5, 4, 2 );
	}
	break; }
    case PE_CheckListIndicator: {
	QCheckListItem *item = opt.checkListItem();
	QListView *lv = item->listView();
	if(!item)
	    return;
	int x = r.x(), y = r.y(), w = r.width(), h = r.width(), marg = lv->itemMargin();

	if ( flags & Style_Enabled )
	    p->setPen( QPen( cg.text(), 2 ) );
	else
	    p->setPen( QPen( lv->palette().color( QPalette::Disabled, QColorGroup::Text ),
			     2 ) );
	bool parentControl = FALSE;
	if ( item->parent() && item->parent()->rtti() == 1  &&
	     ((QCheckListItem*) item->parent())->type() == QCheckListItem::Controller )
	    parentControl = TRUE;
	if ( flags & Style_Selected && !lv->rootIsDecorated() && !parentControl ) {
	    p->fillRect( 0, 0, x + marg + w + 4, item->height(),
			 cg.brush( QColorGroup::Highlight ) );
	    if ( item->isEnabled() )
		p->setPen( QPen( cg.highlightedText(), 2 ) );
	}

	if ( flags & Style_NoChange )
	    p->setBrush( cg.brush( QColorGroup::Button ) );
	p->drawRect( x+marg, y+2, w-4, h-4 );
	/////////////////////
	x++;
	y++;
	if ( ( flags & Style_On) || ( flags & Style_NoChange ) ) {
	    QPointArray a( 7*2 );
	    int i, xx = x+1+marg, yy=y+5;
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
	    p->drawLineSegments( a );
	}
	break; }
#endif
    case PE_HeaderArrow:
	p->save();
	if ( flags & Style_Down ) {
	    QPointArray pa( 3 );
	    p->setPen( cg.light() );
	    p->drawLine( r.x() + r.width(), r.y(), r.x() + r.width() / 2, r.height() );
	    p->setPen( cg.dark() );
	    pa.setPoint( 0, r.x() + r.width() / 2, r.height() );
	    pa.setPoint( 1, r.x(), r.y() );
	    pa.setPoint( 2, r.x() + r.width(), r.y() );
	    p->drawPolyline( pa );
	} else {
	    QPointArray pa( 3 );
	    p->setPen( cg.light() );
	    pa.setPoint( 0, r.x(), r.height() );
	    pa.setPoint( 1, r.x() + r.width(), r.height() );
	    pa.setPoint( 2, r.x() + r.width() / 2, r.y() );
	    p->drawPolyline( pa );
	    p->setPen( cg.dark() );
	    p->drawLine( r.x(), r.height(), r.x() + r.width() / 2, r.y() );
	}
	p->restore();
	break;

    case PE_StatusBarSection:
	qDrawShadeRect( p, r, cg, TRUE, 1, 0, 0 );
	break;

    case PE_ButtonCommand:
    case PE_ButtonBevel:
    case PE_ButtonTool:
    case PE_ButtonDropDown:
    case PE_HeaderSection:
	qDrawShadePanel(p, r, cg, flags & (Style_Sunken | Style_Down | Style_On) , 1,
			&cg.brush(QColorGroup::Button));
	break;

    case PE_Separator:
	qDrawShadeLine( p, r.left(), r.top(), r.right(), r.bottom(), cg,
			flags & Style_Sunken, 1, 0);
	break;

    case PE_FocusRect: {
	const QColor *bg = 0;

	if (!opt.isDefault())
	    bg = &opt.color();

	QPen oldPen = p->pen();

	if (bg) {
	    int h, s, v;
	    bg->hsv(&h, &s, &v);
	    if (v >= 128)
		p->setPen(Qt::black);
	    else
		p->setPen(Qt::white);
	} else
	    p->setPen(cg.foreground());

	if (flags & Style_FocusAtBorder)
	    p->drawRect(QRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2));
	else
	    p->drawRect(r);

	p->setPen(oldPen);
	break; }

    case PE_SpinWidgetPlus:
    case PE_SpinWidgetMinus: {
	p->save();
	int fw = pixelMetric( PM_DefaultFrameWidth, 0 );
	QRect br;
	br.setRect( r.x() + fw, r.y() + fw, r.width() - fw*2,
		    r.height() - fw*2 );

	p->fillRect( br, cg.brush( QColorGroup::Button ) );
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
	break; }

    case PE_SpinWidgetUp:
    case PE_SpinWidgetDown: {
	int fw = pixelMetric( PM_DefaultFrameWidth, 0 );
	QRect br;
	br.setRect( r.x() + fw, r.y() + fw, r.width() - fw*2,
		    r.height() - fw*2 );
	p->fillRect( br, cg.brush( QColorGroup::Button ) );
	int x = r.x(), y = r.y(), w = r.width(), h = r.height();
	int sw = w-4;
	if ( sw < 3 )
	    break;
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
	p->save();
	p->translate( sx + bsx, sy + bsy );
	p->setPen( cg.buttonText() );
	p->setBrush( cg.buttonText() );
	p->drawPolygon( a );
	p->restore();
	break; }

    case PE_Indicator: {
	if (flags & Style_NoChange) {
	    p->setPen(cg.foreground());
	    p->fillRect(r, cg.brush(QColorGroup::Button));
	    p->drawRect(r);
	    p->drawLine(r.topLeft(), r.bottomRight());
	} else
	    qDrawShadePanel(p, r.x(), r.y(), r.width(), r.height(),
			    cg, flags & (Style_Sunken | Style_On), 1,
			    &cg.brush(QColorGroup::Button));
	break; }

    case PE_IndicatorMask: {
	p->fillRect(r, color1);
	break; }

    case PE_ExclusiveIndicator: {
	QRect ir = r;
	p->setPen(cg.dark());
	p->drawArc(r, 0, 5760);

	if (flags & (Style_Sunken | Style_On)) {
	    ir.addCoords(2, 2, -2, -2);
	    p->setBrush(cg.foreground());
	    p->drawEllipse(ir);
	}

	break; }

    case PE_ExclusiveIndicatorMask: {
	p->setPen(color1);
	p->setBrush(color1);
	p->drawEllipse(r);
	break; }

    case PE_DockWindowHandle: {
	bool highlight = flags & Style_On;

	p->save();
	p->translate( r.x(), r.y() );
	if ( flags & Style_Horizontal ) {
	    int x = r.width() / 3;
	    if ( r.height() > 4 ) {
		qDrawShadePanel( p, x, 2, 3, r.height() - 4,
				 cg, highlight, 1, 0 );
		qDrawShadePanel( p, x+3, 2, 3, r.height() - 4,
				 cg, highlight, 1, 0 );
	    }
	} else {
	    if ( r.width() > 4 ) {
		int y = r.height() / 3;
		qDrawShadePanel( p, 2, y, r.width() - 4, 3,
				 cg, highlight, 1, 0 );
		qDrawShadePanel( p, 2, y+3, r.width() - 4, 3,
				 cg, highlight, 1, 0 );
	    }
	}
	p->restore();
	break;
    }

    case PE_DockWindowSeparator: {
	QPoint p1, p2;
	if ( flags & Style_Horizontal ) {
	    p1 = QPoint( r.width()/2, 0 );
	    p2 = QPoint( p1.x(), r.height() );
	} else {
	    p1 = QPoint( 0, r.height()/2 );
	    p2 = QPoint( r.width(), p1.y() );
	}
	qDrawShadeLine( p, p1, p2, cg, 1, 1, 0 );
	break; }

    case PE_Panel:
    case PE_PanelPopup: {
	int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth)
		    : opt.lineWidth();

	qDrawShadePanel(p, r, cg, (flags & Style_Sunken), lw);
	break; }

    case PE_PanelDockWindow: {
	int lw = opt.isDefault() ? pixelMetric(PM_DockWindowFrameWidth)
		    : opt.lineWidth();

	qDrawShadePanel(p, r, cg, FALSE, lw);
	break; }

    case PE_PanelMenuBar: {
	int lw = opt.isDefault() ? pixelMetric(PM_MenuBarFrameWidth)
		    : opt.lineWidth();

	qDrawShadePanel(p, r, cg, FALSE, lw, &cg.brush(QColorGroup::Button));
	break; }

    case PE_SizeGrip: {
	p->save();

	int x, y, w, h;
	r.rect(&x, &y, &w, &h);

	int sw = QMIN( h,w );
	if ( h > w )
	    p->translate( 0, h - w );
	else
	    p->translate( w - h, 0 );

	int sx = x;
	int sy = y;
	int s = sw / 3;

	if ( QApplication::reverseLayout() ) {
	    sx = x + sw;
	    for ( int i = 0; i < 4; ++i ) {
		p->setPen( QPen( cg.light(), 1 ) );
		p->drawLine(  x, sy - 1 , sx + 1,  sw );
		p->setPen( QPen( cg.dark(), 1 ) );
		p->drawLine(  x, sy, sx,  sw );
		p->setPen( QPen( cg.dark(), 1 ) );
		p->drawLine(  x, sy + 1, sx - 1,  sw );
		sx -= s;
		sy += s;
	    }
	} else {
	    for ( int i = 0; i < 4; ++i ) {
		p->setPen( QPen( cg.light(), 1 ) );
		p->drawLine(  sx-1, sw, sw,  sy-1 );
		p->setPen( QPen( cg.dark(), 1 ) );
		p->drawLine(  sx, sw, sw,  sy );
		p->setPen( QPen( cg.dark(), 1 ) );
		p->drawLine(  sx+1, sw, sw,  sy+1 );
		sx += s;
		sy += s;
	    }
	}

	p->restore();
	break; }

    case PE_CheckMark: {
	const int markW = r.width() > 7 ? 7 : r.width();
	const int markH = markW;
	int posX = r.x() + ( r.width() - markW )/2 + 1;
	int posY = r.y() + ( r.height() - markH )/2;

	// Could do with some optimizing/caching...
	QPointArray a( markH*2 );
	int i, xx, yy;
	xx = posX;
	yy = 3 + posY;
	for ( i=0; i<markW/2; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy++;
	}
	yy -= 2;
	for ( ; i<markH; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy--;
	}
	if ( !(flags & Style_Enabled) && !(flags & Style_On)) {
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
	break; }

    case PE_PanelGroupBox: //We really do not need PE_GroupBoxFrame anymore, nasty holdover ###
	drawPrimitive( PE_GroupBoxFrame, p, r, cg, flags, opt );
	break;
    case PE_GroupBoxFrame: {
#ifndef QT_NO_FRAME
	if ( opt.isDefault() )
	    break;
	int lwidth = opt.lineWidth(), mlwidth = opt.midLineWidth();
	if ( flags & (Style_Sunken|Style_Raised))
	    qDrawShadeRect( p, r.x(), r.y(), r.width(), r.height(), cg, flags & Style_Sunken, lwidth, mlwidth );
	else
	    qDrawPlainRect( p, r.x(), r.y(), r.width(), r.height(), cg.foreground(), lwidth );
#endif
	break; }

    case PE_ProgressBarChunk:
	p->fillRect( r.x(), r.y() + 3, r.width() -2, r.height() - 6,
	    cg.brush(QColorGroup::Highlight));
	break;

    case PE_PanelLineEdit:
    case PE_PanelTabWidget:
    case PE_WindowFrame:
	drawPrimitive( PE_Panel, p, r, cg, flags, opt );
	break;

    case PE_RubberBand:
	drawPrimitive(PE_FocusRect, p, r, cg, flags, opt);
	break;
    default:
	break;
    }

    activePainter = 0;
}

/*! \reimp */
void QCommonStyle::drawControl( ControlElement element,
				QPainter *p,
				const QWidget *widget,
				const QRect &r,
				const QColorGroup &cg,
				SFlags flags,
				const QStyleOption& opt ) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::drawControl: widget parameter cannot be zero!");
	return;
    }
#endif

    activePainter = p;

    switch (element) {
    case CE_MenuBarEmptyArea: {
	QRegion reg;
	if(p->hasClipping()) //ick
	    reg = p->clipRegion();
	else
	    reg = r;
	((QWidget *)widget)->erase( reg );
	break; }
    case CE_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    QRect br = r;
	    int dbi = pixelMetric(PM_ButtonDefaultIndicator, widget);

	    if (button->isDefault() || button->autoDefault()) {
		if ( button->isDefault())
		    drawPrimitive(PE_ButtonDefault, p, br, cg, flags);

		br.setCoords(br.left()   + dbi,
			     br.top()    + dbi,
			     br.right()  - dbi,
			     br.bottom() - dbi);
	    }

	    p->save();
	    p->setBrushOrigin( -widget->backgroundOffset().x(),
			       -widget->backgroundOffset().y() );
	    drawPrimitive(PE_ButtonCommand, p, br, cg, flags);
	    p->restore();
#endif
	    break;
	}

    case CE_PushButtonLabel:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    QRect ir = r;

	    if (button->isDown() || button->isOn()) {
		flags |= Style_Sunken;
		ir.moveBy(pixelMetric(PM_ButtonShiftHorizontal, widget),
			  pixelMetric(PM_ButtonShiftVertical, widget));
	    }

	    if (button->isMenuButton()) {
		int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
		QRect ar(ir.right() - mbi, ir.y() + 2, mbi - 4, ir.height() - 4);
		drawPrimitive(PE_ArrowDown, p, ar, cg, flags, opt);
		ir.setWidth(ir.width() - mbi);
	    }

	    int tf=AlignVCenter | ShowPrefix;
	    if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
		tf |= NoAccel;

#ifndef QT_NO_ICONSET
	    if ( button->iconSet() && ! button->iconSet()->isNull() ) {
		QIconSet::Mode mode =
		    button->isEnabled() ? QIconSet::Normal : QIconSet::Disabled;
		if ( mode == QIconSet::Normal && button->hasFocus() )
		    mode = QIconSet::Active;

		QIconSet::State state = QIconSet::Off;
		if ( button->isToggleButton() && button->isOn() )
		    state = QIconSet::On;

		QPixmap pixmap = button->iconSet()->pixmap( QIconSet::Small, mode, state );
		int pixw = pixmap.width();
		int pixh = pixmap.height();

		//Center the icon if there is neither text nor pixmap
		if ( button->text().isEmpty() && !button->pixmap() )
		    p->drawPixmap( ir.x() + ir.width() / 2 - pixw / 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap );
		else
		    p->drawPixmap( ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap );

		ir.moveBy(pixw + 4, 0);
		ir.setWidth(ir.width() - (pixw + 4));
		// left-align text if there is
		if (!button->text().isEmpty())
		    tf |= AlignLeft;
		else if (button->pixmap())
		    tf |= AlignHCenter;
	    } else
#endif //QT_NO_ICONSET
		tf |= AlignHCenter;
	    drawItem(p, ir, tf, cg,
		     flags & Style_Enabled, button->pixmap(), button->text(),
		     button->text().length(), &(cg.buttonText()) );

	    if (flags & Style_HasFocus)
		drawPrimitive(PE_FocusRect, p, subRect(SR_PushButtonFocusRect, widget),
			      cg, flags);
#endif
	    break;
	}

    case CE_CheckBox:
	drawPrimitive(PE_Indicator, p, r, cg, flags, opt);
	break;

    case CE_CheckBoxLabel:
	{
#ifndef QT_NO_CHECKBOX
	    const QCheckBox *checkbox = (const QCheckBox *) widget;

	    int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
	    if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
		alignment |= NoAccel;

	    drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
		     flags & Style_Enabled, checkbox->pixmap(), checkbox->text());

	    if (flags & Style_HasFocus) {
		QRect fr = visualRect(subRect(SR_CheckBoxFocusRect, widget), widget);
		drawPrimitive(PE_FocusRect, p, fr, cg, flags);
	    }
#endif
	    break;
	}

    case CE_RadioButton:
	drawPrimitive(PE_ExclusiveIndicator, p, r, cg, flags, opt);
	break;

    case CE_RadioButtonLabel:
	{
#ifndef QT_NO_RADIOBUTTON
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;

	    int alignment = QApplication::reverseLayout() ? AlignRight : AlignLeft;
	    if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
		alignment |= NoAccel;
	    drawItem(p, r, alignment | AlignVCenter | ShowPrefix, cg,
		     flags & Style_Enabled, radiobutton->pixmap(), radiobutton->text());

	    if (flags & Style_HasFocus) {
		QRect fr = visualRect(subRect(SR_RadioButtonFocusRect, widget), widget);
		drawPrimitive(PE_FocusRect, p, fr, cg, flags);
	    }
#endif
	    break;
	}

#ifndef QT_NO_TABBAR
    case CE_TabBarTab:
	{
	    const QTabBar * tb = (const QTabBar *) widget;

	    if ( tb->shape() == QTabBar::TriangularAbove ||
		 tb->shape() == QTabBar::TriangularBelow ) {
		// triangular, above or below
		int y;
		int x;
		QPointArray a( 10 );
		a.setPoint( 0, 0, -1 );
		a.setPoint( 1, 0, 0 );
		y = r.height()-2;
		x = y/3;
		a.setPoint( 2, x++, y-1 );
		a.setPoint( 3, x++, y );
		a.setPoint( 3, x++, y++ );
		a.setPoint( 4, x, y );

		int i;
		int right = r.width() - 1;
		for ( i = 0; i < 5; i++ )
		    a.setPoint( 9-i, right - a.point( i ).x(), a.point( i ).y() );

		if ( tb->shape() == QTabBar::TriangularAbove )
		    for ( i = 0; i < 10; i++ )
			a.setPoint( i, a.point(i).x(),
				    r.height() - 1 - a.point( i ).y() );

		a.translate( r.left(), r.top() );

		if ( flags & Style_Selected )
		    p->setBrush( cg.base() );
		else
		    p->setBrush( cg.background() );
		p->setPen( cg.foreground() );
		p->drawPolygon( a );
		p->setBrush( NoBrush );
	    }
	    break;
	}

    case CE_TabBarLabel:
	{
	    if ( opt.isDefault() )
		break;

	    const QTabBar * tb = (const QTabBar *) widget;
	    QTab * t = opt.tab();

	    QRect tr = r;
	    if ( t->identifier() == tb->currentTab() )
		tr.setBottom( tr.bottom() -
			      pixelMetric( QStyle::PM_DefaultFrameWidth, tb ) );

	    int alignment = AlignCenter | ShowPrefix;
	    if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
		alignment |= NoAccel;
	    drawItem( p, tr, alignment, cg,
		      flags & Style_Enabled, 0, t->text() );

	    if ( (flags & Style_HasFocus) && !t->text().isEmpty() )
		drawPrimitive( PE_FocusRect, p, r, cg );
	    break;
	}
#endif // QT_NO_TABBAR
#ifndef QT_NO_TOOLBOX
    case CE_ToolBoxTab:
	{
	    int d = 20 + r.height() - 3;
	    QPointArray a( 7 );
	    a.setPoint( 0, -1, r.height() + 1 );
	    a.setPoint( 1, -1, 1 );
	    a.setPoint( 2, r.width() - d, 1 );
	    a.setPoint( 3, r.width() - 20, r.height() - 2 );
	    a.setPoint( 4, r.width() - 1, r.height() - 2 );
	    a.setPoint( 5, r.width() - 1, r.height() + 1 );
	    a.setPoint( 6, -1, r.height() + 1 );

	    const QToolBox *tb = (const QToolBox*)widget;

	    if ( flags & Style_Selected && tb->currentItem() )
		p->setBrush( tb->currentItem()->paletteBackgroundColor() );
	    else
		p->setBrush( cg.brush(QColorGroup::Background) );

	    p->setPen( cg.mid().dark( 150 ) );
	    p->drawPolygon( a );
	    p->setPen( cg.light() );
	    p->drawLine( 0, 2, r.width() - d, 2 );
	    p->drawLine( r.width() - d - 1, 2, r.width() - 21, r.height() - 1 );
	    p->drawLine( r.width() - 20, r.height() - 1, r.width(), r.height() - 1 );
	    p->setBrush( NoBrush );
	    break;
	}
#endif // QT_NO_TOOLBOX
    case CE_ProgressBarGroove:
	qDrawShadePanel(p, r, cg, TRUE, 1, &cg.brush(QColorGroup::Background));
	break;

#ifndef QT_NO_PROGRESSBAR
    case CE_ProgressBarContents:
	{
	    const QProgressBar *progressbar = (const QProgressBar *) widget;
	    // Correct the highlight color if same as background,
	    // or else we cannot see the progress...
	    QColorGroup cgh = cg;
	    if ( cgh.highlight() == cgh.background() )
		cgh.setColor( QColorGroup::Highlight, progressbar->palette().active().highlight() );
	    bool reverse = QApplication::reverseLayout();
	    int fw = 2;
	    int w = r.width() - 2*fw;
	    if ( !progressbar->totalSteps() ) {
		// draw busy indicator
		int x = progressbar->progress() % (w * 2);
		if (x > w)
		    x = 2 * w - x;
		x = reverse ? r.right() - x : x + r.x();
		p->setPen( QPen(cgh.highlight(), 4) );
		p->drawLine(x, r.y() + 1, x, r.height() - fw);
	    } else {
		const int unit_width = pixelMetric(PM_ProgressBarChunkWidth, widget);
		int u;
		if ( unit_width > 1 )
		    u = (r.width()+unit_width/3) / unit_width;
		else
		    u = w / unit_width;
		int p_v = progressbar->progress();
		int t_s = progressbar->totalSteps() ? progressbar->totalSteps() : 1;

		if ( u > 0 && p_v >= INT_MAX / u && t_s >= u ) {
		    // scale down to something usable.
		    p_v /= u;
		    t_s /= u;
		}

		// nu < tnu, if last chunk is only a partial chunk
		int tnu, nu;
		tnu = nu = p_v * u / t_s;

		if (nu * unit_width > w)
		    nu--;

		// Draw nu units out of a possible u of unit_width
		// width, each a rectangle bordered by background
		// color, all in a sunken panel with a percentage text
		// display at the end.
		int x = 0;
		int x0 = reverse ? r.right() - ((unit_width > 1) ?
						unit_width : fw) : r.x() + fw;
		for (int i=0; i<nu; i++) {
		    drawPrimitive( PE_ProgressBarChunk, p,
				   QRect( x0+x, r.y(), unit_width, r.height() ),
				   cgh, Style_Default, opt );
		    x += reverse ? -unit_width: unit_width;
		}

		// Draw the last partial chunk to fill up the
		// progressbar entirely
		if (nu < tnu) {
		    int pixels_left = w - (nu*unit_width);
		    int offset = reverse ? x0+x+unit_width-pixels_left : x0+x;
		    drawPrimitive( PE_ProgressBarChunk, p,
				   QRect( offset, r.y(), pixels_left,
					  r.height() ), cgh, Style_Default,
				   opt );
		}
	    }
	}
	break;

    case CE_ProgressBarLabel:
	{
	    const QProgressBar *progressbar = (const QProgressBar *) widget;
	    QColor penColor = cg.highlightedText();
	    QColor *pcolor = 0;
	    if ( progressbar->centerIndicator() && !progressbar->indicatorFollowsStyle() &&
		 progressbar->progress()*2 >= progressbar->totalSteps() )
		pcolor = &penColor;
	    drawItem(p, r, AlignCenter | SingleLine, cg, flags & Style_Enabled, 0,
		     progressbar->progressString(), -1, pcolor );
	}
	break;
#endif // QT_NO_PROGRESSBAR

    case CE_MenuBarItem:
	{
#ifndef QT_NO_MENUDATA
	    if (opt.isDefault())
		break;

	    QMenuItem *mi = opt.menuItem();
	    int alignment = AlignCenter|ShowPrefix|DontClip|SingleLine;
	    if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
		alignment |= NoAccel;
	    drawItem( p, r, alignment, cg,
		      flags & Style_Enabled, mi->pixmap(), mi->text(), -1,
		      &cg.buttonText() );
#endif
	    break;
	}

#ifndef QT_NO_TOOLBUTTON
    case CE_ToolButtonLabel:
	{
	    const QToolButton *toolbutton = (const QToolButton *) widget;
	    QRect rect = r;
	    Qt::ArrowType arrowType = opt.isDefault()
			? Qt::DownArrow : opt.arrowType();

	    int shiftX = 0;
	    int shiftY = 0;
	    if (flags & (Style_Down | Style_On)) {
		shiftX = pixelMetric(PM_ButtonShiftHorizontal, widget);
		shiftY = pixelMetric(PM_ButtonShiftVertical, widget);
	    }

	    if (!opt.isDefault()) {
		PrimitiveElement pe;
		switch (arrowType) {
		case Qt::LeftArrow:  pe = PE_ArrowLeft;  break;
		case Qt::RightArrow: pe = PE_ArrowRight; break;
		case Qt::UpArrow:    pe = PE_ArrowUp;    break;
		default:
		case Qt::DownArrow:  pe = PE_ArrowDown;  break;
		}

		rect.moveBy(shiftX, shiftY);
		drawPrimitive(pe, p, rect, cg, flags, opt);
	    } else {
		QColor btext = toolbutton->paletteForegroundColor();

		if (toolbutton->iconSet().isNull() &&
		    ! toolbutton->text().isNull() &&
		    ! toolbutton->usesTextLabel()) {
		    int alignment = AlignCenter | ShowPrefix;
		    if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
			alignment |= NoAccel;

		    rect.moveBy(shiftX, shiftY);
		    drawItem(p, rect, alignment, cg,
			     flags & Style_Enabled, 0, toolbutton->text(),
			     toolbutton->text().length(), &btext);
		} else {
		    QPixmap pm;
		    QIconSet::Size size =
			toolbutton->usesBigPixmap() ? QIconSet::Large : QIconSet::Small;
		    QIconSet::State state =
			toolbutton->isOn() ? QIconSet::On : QIconSet::Off;
		    QIconSet::Mode mode;
		    if (! toolbutton->isEnabled())
			mode = QIconSet::Disabled;
		    else if (flags & (Style_Down | Style_On) ||
			     (flags & Style_Raised) && (flags & Style_AutoRaise))
			mode = QIconSet::Active;
		    else
			mode = QIconSet::Normal;
		    pm = toolbutton->iconSet().pixmap( size, mode, state );

		    if ( toolbutton->usesTextLabel() ) {
			p->setFont( toolbutton->font() );
			QRect pr = rect, tr = rect;
			int alignment = ShowPrefix;
			if (!styleHint(SH_UnderlineAccelerator, widget, QStyleOption::Default, 0))
			    alignment |= NoAccel;

			if ( toolbutton->textPosition() == QToolButton::Under ) {
			    int fh = p->fontMetrics().height();
			    pr.addCoords( 0, 1, 0, -fh-3 );
			    tr.addCoords( 0, pr.bottom(), 0, -3 );
			    pr.moveBy(shiftX, shiftY);
			    drawItem( p, pr, AlignCenter, cg, TRUE, &pm, QString::null );
			    alignment |= AlignCenter;
			} else {
			    pr.setWidth( pm.width() + 8 );
			    tr.addCoords( pr.right(), 0, 0, 0 );
			    pr.moveBy(shiftX, shiftY);

			    drawItem( p, pr, AlignCenter, cg, TRUE, &pm, QString::null );
			    alignment |= AlignLeft | AlignVCenter;
			}

			tr.moveBy(shiftX, shiftY);
			drawItem( p, tr, alignment, cg,
				  flags & Style_Enabled, 0, toolbutton->textLabel(),
				  toolbutton->textLabel().length(), &btext);
		    } else {
			rect.moveBy(shiftX, shiftY);
			drawItem( p, rect, AlignCenter, cg, TRUE, &pm, QString::null );
		    }
		}
	    }

	    break;
	}
#endif // QT_NO_TOOLBUTTON
#ifndef QT_NO_HEADER
        case CE_HeaderLabel:
	{
	    QRect rect = r;
	    const QHeader* header = (const QHeader *) widget;
	    int section = opt.headerSection();

	    QIconSet* icon = header->iconSet( section );
	    if ( icon ) {
		QPixmap pixmap = icon->pixmap( QIconSet::Small,
					       flags & Style_Enabled ?
					       QIconSet::Normal : QIconSet::Disabled );
		int pixw = pixmap.width();
		int pixh = pixmap.height();
		// "pixh - 1" because of tricky integer division

		QRect pixRect = rect;
		pixRect.setY( rect.center().y() - (pixh - 1) / 2 );
		drawItem ( p, pixRect, AlignVCenter, cg, flags & Style_Enabled,
			   &pixmap, QString::null );
                if (QApplication::reverseLayout())
                    rect.setRight(rect.right() - pixw - 2);
                else
                    rect.setLeft(rect.left() + pixw + 2);
	    }

            if (rect.isValid())
                drawItem ( p, rect, AlignVCenter, cg, flags & Style_Enabled,
                           0, header->label( section ), -1, &(cg.buttonText()) );
	}
#endif // QT_NO_HEADER
    default:
	break;
    }

    activePainter = 0;
}

/*! \reimp */
void QCommonStyle::drawControlMask( ControlElement control,
				    QPainter *p,
				    const QWidget *widget,
				    const QRect &r,
				    const QStyleOption& opt ) const
{
    Q_UNUSED(widget);

    activePainter = p;

    QColorGroup cg(color1,color1,color1,color1,color1,color1,color1,color1,color0);

    switch (control) {
    case CE_PushButton:
	drawPrimitive(PE_ButtonCommand, p, r, cg, Style_Default, opt);
	break;

    case CE_CheckBox:
	drawPrimitive(PE_IndicatorMask, p, r, cg, Style_Default, opt);
	break;

    case CE_RadioButton:
	drawPrimitive(PE_ExclusiveIndicatorMask, p, r, cg, Style_Default, opt);
	break;

    default:
	p->fillRect(r, color1);
	break;
    }

    activePainter = 0;
}

/*! \reimp */
QRect QCommonStyle::subRect(SubRect r, const QWidget *widget) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::subRect: widget parameter cannot be zero!");
	return QRect();
    }
#endif

    QRect rect, wrect(widget->rect());

    switch (r) {
#ifndef QT_NO_DIALOGBUTTONS
    case SR_DialogButtonAbort:
    case SR_DialogButtonRetry:
    case SR_DialogButtonIgnore:
    case SR_DialogButtonAccept:
    case SR_DialogButtonReject:
    case SR_DialogButtonApply:
    case SR_DialogButtonHelp:
    case SR_DialogButtonAll:
    case SR_DialogButtonCustom: {
	QDialogButtons::Button srch = QDialogButtons::None;
	if(r == SR_DialogButtonAccept)
	    srch = QDialogButtons::Accept;
	else if(r == SR_DialogButtonReject)
	    srch = QDialogButtons::Reject;
	else if(r == SR_DialogButtonAll)
	    srch = QDialogButtons::All;
	else if(r == SR_DialogButtonApply)
	    srch = QDialogButtons::Apply;
	else if(r == SR_DialogButtonHelp)
	    srch = QDialogButtons::Help;
	else if(r == SR_DialogButtonRetry)
	    srch = QDialogButtons::Retry;
	else if(r == SR_DialogButtonIgnore)
	    srch = QDialogButtons::Ignore;
	else if(r == SR_DialogButtonAbort)
	    srch = QDialogButtons::Abort;

	const int bwidth = pixelMetric(PM_DialogButtonsButtonWidth, widget),
		 bheight = pixelMetric(PM_DialogButtonsButtonHeight, widget),
		  bspace = pixelMetric(PM_DialogButtonsSeparator, widget),
		      fw = pixelMetric(PM_DefaultFrameWidth, widget);
	const QDialogButtons *dbtns = (const QDialogButtons *) widget;
	int start = fw;
	if(dbtns->orientation() == Horizontal)
	    start = wrect.right() - fw;
	QDialogButtons::Button btns[] = { QDialogButtons::All, QDialogButtons::Reject, QDialogButtons::Accept, //reverse order (right to left)
					  QDialogButtons::Apply, QDialogButtons::Retry, QDialogButtons::Ignore, QDialogButtons::Abort,
					  QDialogButtons::Help };
	for(unsigned int i = 0, cnt = 0; i < (sizeof(btns)/sizeof(btns[0])); i++) {
	    if(dbtns->isButtonVisible(btns[i])) {
		QSize szH = dbtns->sizeHint(btns[i]);
		int mwidth = QMAX(bwidth, szH.width()), mheight = QMAX(bheight, szH.height());
		if(dbtns->orientation() == Horizontal) {
		    start -= mwidth;
		    if(cnt)
			start -= bspace;
		} else if(cnt) {
		    start += mheight;
		    start += bspace;
		}
		cnt++;
		if(btns[i] == srch) {
		    if(dbtns->orientation() == Horizontal)
			return QRect(start, wrect.bottom() - fw - mheight, mwidth, mheight);
		    else
			return QRect(fw, start, mwidth, mheight);
		}
	    }
	}
	if(r == SR_DialogButtonCustom) {
	    if(dbtns->orientation() == Horizontal)
		return QRect(fw, fw, start - fw - bspace, wrect.height() - (fw*2));
	    else
		return QRect(fw, start, wrect.width() - (fw*2), wrect.height() - start - (fw*2));
	}
	return QRect(); }
#endif //QT_NO_DIALOGBUTTONS
    case SR_PushButtonContents:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    int dx1, dx2;

	    dx1 = pixelMetric(PM_DefaultFrameWidth, widget);
	    if (button->isDefault() || button->autoDefault())
		dx1 += pixelMetric(PM_ButtonDefaultIndicator, widget);
	    dx2 = dx1 * 2;

	    rect.setRect(wrect.x()      + dx1,
			 wrect.y()      + dx1,
			 wrect.width()  - dx2,
			 wrect.height() - dx2);
#endif
	    break;
	}

    case SR_PushButtonFocusRect:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    int dbw1 = 0, dbw2 = 0;
	    if (button->isDefault() || button->autoDefault()) {
		dbw1 = pixelMetric(PM_ButtonDefaultIndicator, widget);
		dbw2 = dbw1 * 2;
	    }

	    int dfw1 = pixelMetric(PM_DefaultFrameWidth, widget) * 2,
		dfw2 = dfw1 * 2;

	    rect.setRect(wrect.x()      + dfw1 + dbw1,
			 wrect.y()      + dfw1 + dbw1,
			 wrect.width()  - dfw2 - dbw2,
			 wrect.height() - dfw2 - dbw2);
#endif
	    break;
	}

    case SR_CheckBoxIndicator:
	{
	    int h = pixelMetric( PM_IndicatorHeight, widget );
	    rect.setRect(0, ( wrect.height() - h ) / 2,
			 pixelMetric( PM_IndicatorWidth, widget ), h );
	    break;
	}

    case SR_CheckBoxContents:
	{
#ifndef QT_NO_CHECKBOX
	    QRect ir = subRect(SR_CheckBoxIndicator, widget);
	    rect.setRect(ir.right() + 6, wrect.y(),
			 wrect.width() - ir.width() - 6, wrect.height());
#endif
	    break;
	}

    case SR_CheckBoxFocusRect:
	{
#ifndef QT_NO_CHECKBOX
	    const QCheckBox *checkbox = (const QCheckBox *) widget;
	    if ( !checkbox->pixmap() && checkbox->text().isEmpty() ) {
		rect = subRect( SR_CheckBoxIndicator, widget );
		rect.addCoords( 1, 1, -1, -1 );
		break;
	    }
	    QRect cr = subRect(SR_CheckBoxContents, widget);

	    // don't create a painter if we have an active one
	    QPainter *p = 0;
	    if (! activePainter)
		p = new QPainter(checkbox);
	    rect = itemRect((activePainter ? activePainter : p),
			    cr, AlignLeft | AlignVCenter | ShowPrefix,
			    checkbox->isEnabled(),
			    checkbox->pixmap(),
			    checkbox->text());

	    delete p;

	    rect.addCoords( -3, -2, 3, 2 );
	    rect = rect.intersect(wrect);
#endif
	    break;
	}

    case SR_RadioButtonIndicator:
	{
	    int h = pixelMetric( PM_ExclusiveIndicatorHeight, widget );
	    rect.setRect(0, ( wrect.height() - h ) / 2,
			 pixelMetric( PM_ExclusiveIndicatorWidth, widget ), h );
	    break;
	}

    case SR_RadioButtonContents:
	{
	    QRect ir = subRect(SR_RadioButtonIndicator, widget);
	    rect.setRect(ir.right() + 6, wrect.y(),
			 wrect.width() - ir.width() - 6, wrect.height());
	    break;
	}

    case SR_RadioButtonFocusRect:
	{
#ifndef QT_NO_RADIOBUTTON
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;
	    if ( !radiobutton->pixmap() && radiobutton->text().isEmpty() ) {
		rect = subRect( SR_RadioButtonIndicator, widget );
		rect.addCoords( 1, 1, -1, -1 );
		break;
	    }
	    QRect cr = subRect(SR_RadioButtonContents, widget);

	    // don't create a painter if we have an active one
	    QPainter *p = 0;
	    if (! activePainter)
		p = new QPainter(radiobutton);
	    rect = itemRect((activePainter ? activePainter : p),
			    cr, AlignLeft | AlignVCenter | ShowPrefix,
			    radiobutton->isEnabled(),
			    radiobutton->pixmap(),
			    radiobutton->text());
	    delete p;

	    rect.addCoords( -3, -2, 3, 2 );
	    rect = rect.intersect(wrect);
#endif
	    break;
	}

    case SR_ComboBoxFocusRect:
	rect.setRect(3, 3, widget->width()-6-16, widget->height()-6);
	break;

#ifndef QT_NO_SLIDER
    case SR_SliderFocusRect:
	{
	    const QSlider * sl = (const QSlider *) widget;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int thickness  = pixelMetric( PM_SliderControlThickness, sl );

	    if ( sl->orientation() == Horizontal )
		rect.setRect( 0, tickOffset-1, sl->width(), thickness+2 );
	    else
		rect.setRect( tickOffset-1, 0, thickness+2, sl->height() );
	    rect = rect.intersect( sl->rect() ); // ## is this really necessary?
	    break;
	}
#endif // QT_NO_SLIDER

#ifndef QT_NO_MAINWINDOW
    case SR_DockWindowHandleRect:
	{
	    if (! widget->parentWidget())
		break;

	    const QDockWindow * dw = (const QDockWindow *) widget->parentWidget();

	    if ( !dw->area() || !dw->isCloseEnabled() )
		rect.setRect( 0, 0, widget->width(), widget->height() );
	    else {
		if ( dw->area()->orientation() == Horizontal )
		    rect.setRect(0, 15, widget->width(), widget->height() - 15);
		else
		    rect.setRect(0, 1, widget->width() - 15, widget->height() - 1);
	    }
	    break;
	}
#endif // QT_NO_MAINWINDOW

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
		! progressbar->centerIndicator())
		rect.setCoords(wrect.left(), wrect.top(),
			       wrect.right() - textw, wrect.bottom());
	    else
		rect = wrect;
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
		! progressbar->centerIndicator())
		rect.setCoords(wrect.right() - textw, wrect.top(),
			       wrect.right(), wrect.bottom());
	    else
		rect = wrect;
#endif
	    break;
	}

    case SR_ToolButtonContents:
	rect = querySubControlMetrics(CC_ToolButton, widget, SC_ToolButton);
	break;

    case SR_ToolBoxTabContents:
	rect = wrect;
	rect.addCoords( 0, 0, -30, 0 );
	break;

    default:
	rect = wrect;
	break;
    }

    return rect;
}

#ifndef QT_NO_RANGECONTROL
/*
  I really need this and I don't want to expose it in QRangeControl..
*/
static int qPositionFromValue( const QRangeControl * rc, int logical_val,
			       int span )
{
    if ( span <= 0 || logical_val < rc->minValue() ||
	 rc->maxValue() <= rc->minValue() )
	return 0;
    if ( logical_val > rc->maxValue() )
	return span;

    uint range = rc->maxValue() - rc->minValue();
    uint p = logical_val - rc->minValue();

    if ( range > (uint)INT_MAX/4096 ) {
	const int scale = 4096*2;
	return ( (p/scale) * span ) / (range/scale);
	// ### the above line is probably not 100% correct
	// ### but fixing it isn't worth the extreme pain...
    } else if ( range > (uint)span ) {
	return (2*p*span + range) / (2*range);
    } else {
	uint div = span / range;
	uint mod = span % range;
	return p*div + (2*p*mod + range) / (2*range);
    }
    //equiv. to (p*span)/range + 0.5
    // no overflow because of this implicit assumption:
    // span <= 4096
}
#endif // QT_NO_RANGECONTROL

/*! \reimp */
void QCommonStyle::drawComplexControl( ComplexControl control,
				       QPainter *p,
				       const QWidget *widget,
				       const QRect &r,
				       const QColorGroup &cg,
				       SFlags flags,
				       SCFlags controls,
				       SCFlags active,
				       const QStyleOption& opt ) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::drawComplexControl: widget parameter cannot be zero!");
	return;
    }
#endif

    activePainter = p;

    switch (control) {
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
	{
	    const QScrollBar *scrollbar = (const QScrollBar *) widget;
	    QRect addline, subline, addpage, subpage, slider, first, last;
	    bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	    subline = querySubControlMetrics(control, widget, SC_ScrollBarSubLine, opt);
	    addline = querySubControlMetrics(control, widget, SC_ScrollBarAddLine, opt);
	    subpage = querySubControlMetrics(control, widget, SC_ScrollBarSubPage, opt);
	    addpage = querySubControlMetrics(control, widget, SC_ScrollBarAddPage, opt);
	    slider  = querySubControlMetrics(control, widget, SC_ScrollBarSlider,  opt);
	    first   = querySubControlMetrics(control, widget, SC_ScrollBarFirst,   opt);
	    last    = querySubControlMetrics(control, widget, SC_ScrollBarLast,    opt);

       	    if ((controls & SC_ScrollBarSubLine) && subline.isValid())
		drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSubLine) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarAddLine) && addline.isValid())
		drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddLine) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSubPage) && subpage.isValid())
		drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSubPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarAddPage) && addpage.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
       	    if ((controls & SC_ScrollBarFirst) && first.isValid())
		drawPrimitive(PE_ScrollBarFirst, p, first, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarFirst) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarLast) && last.isValid())
		drawPrimitive(PE_ScrollBarLast, p, last, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarLast) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    if ((controls & SC_ScrollBarSlider) && slider.isValid()) {
		drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((active == SC_ScrollBarSlider) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));

		// ### perhaps this should not be able to accept focus if maxedOut?
		if (scrollbar->hasFocus()) {
		    QRect fr(slider.x() + 2, slider.y() + 2,
			     slider.width() - 5, slider.height() - 5);
		    drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
		}
	    }

	    break;
	}
#endif // QT_NO_SCROLLBAR

#ifndef QT_NO_TOOLBUTTON
    case CC_ToolButton:
	{
	    const QToolButton *toolbutton = (const QToolButton *) widget;

	    QColorGroup c = cg;
	    if ( toolbutton->backgroundMode() != PaletteButton )
		c.setBrush( QColorGroup::Button,
			    toolbutton->paletteBackgroundColor() );
	    QRect button, menuarea;
	    button   = visualRect( querySubControlMetrics(control, widget, SC_ToolButton, opt), widget );
	    menuarea = visualRect( querySubControlMetrics(control, widget, SC_ToolButtonMenu, opt), widget );

	    SFlags bflags = flags,
		   mflags = flags;

	    if (active & SC_ToolButton)
		bflags |= Style_Down;
	    if (active & SC_ToolButtonMenu)
		mflags |= Style_Down;

	    if (controls & SC_ToolButton) {
		if (bflags & (Style_Down | Style_On | Style_Raised)) {
		    drawPrimitive(PE_ButtonTool, p, button, c, bflags, opt);
		} else if ( toolbutton->parentWidget() &&
			  toolbutton->parentWidget()->backgroundPixmap() &&
			  ! toolbutton->parentWidget()->backgroundPixmap()->isNull() ) {
		    QPixmap pixmap =
			*(toolbutton->parentWidget()->backgroundPixmap());

		    p->drawTiledPixmap( r, pixmap, toolbutton->pos() );
		}
	    }

	    if (controls & SC_ToolButtonMenu) {
		if (mflags & (Style_Down | Style_On | Style_Raised))
		    drawPrimitive(PE_ButtonDropDown, p, menuarea, c, mflags, opt);
		drawPrimitive(PE_ArrowDown, p, menuarea, c, mflags, opt);
	    }

	    if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
		QRect fr = toolbutton->rect();
		fr.addCoords(3, 3, -3, -3);
		drawPrimitive(PE_FocusRect, p, fr, c);
	    }

	    break;
	}
#endif // QT_NO_TOOLBUTTON

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar:
	{
	    const QTitleBar *titlebar = (const QTitleBar *) widget;
	    if ( controls & SC_TitleBarLabel ) {
		QColorGroup cgroup = titlebar->usesActiveColor() ?
		    titlebar->palette().active() : titlebar->palette().inactive();

		QColor left = cgroup.highlight();
		QColor right = cgroup.base();

		if ( left != right ) {
		    double rS = left.red();
		    double gS = left.green();
		    double bS = left.blue();

		    const double rD = double(right.red() - rS) / titlebar->width();
		    const double gD = double(right.green() - gS) / titlebar->width();
		    const double bD = double(right.blue() - bS) / titlebar->width();

		    const int w = titlebar->width();
		    for ( int sx = 0; sx < w; sx++ ) {
			rS+=rD;
			gS+=gD;
			bS+=bD;
			p->setPen( QColor( (int)rS, (int)gS, (int)bS ) );
			p->drawLine( sx, 0, sx, titlebar->height() );
		    }
		} else {
		    p->fillRect( titlebar->rect(), left );
		}

		QRect ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarLabel ), widget );

		p->setPen( cgroup.highlightedText() );
		p->drawText(ir.x()+2, ir.y(), ir.width()-2, ir.height(),
			    AlignAuto | AlignVCenter | SingleLine, titlebar->visibleText() );
	    }

	    QRect ir;
	    bool down = FALSE;
	    QPixmap pm;

	    if ( controls & SC_TitleBarCloseButton ) {
		ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarCloseButton ), widget );
		down = active & SC_TitleBarCloseButton;
		if ( widget->testWFlags( WStyle_Tool )
#ifndef QT_NO_MAINWINDOW
		     || ::qt_cast<QDockWindow*>(widget)
#endif
		    )
		    pm = stylePixmap(SP_DockWindowCloseButton, widget);
		else
		    pm = stylePixmap(SP_TitleBarCloseButton, widget);
    		drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
			      down ? Style_Down : Style_Raised);

		p->save();
		if( down )
		    p->translate( pixelMetric(PM_ButtonShiftHorizontal, widget),
				  pixelMetric(PM_ButtonShiftVertical, widget) );
		drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		p->restore();
	    }

	    if ( titlebar->window() ) {
		if ( controls & SC_TitleBarMaxButton ) {
		    ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarMaxButton ), widget );

		    down = active & SC_TitleBarMaxButton;
		    pm = QPixmap(stylePixmap(SP_TitleBarMaxButton, widget));
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);

		    p->save();
		    if( down )
			p->translate( pixelMetric(PM_ButtonShiftHorizontal, widget),
				      pixelMetric(PM_ButtonShiftVertical, widget) );
    		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		    p->restore();
		}

		if ( controls & SC_TitleBarNormalButton || controls & SC_TitleBarMinButton ) {
		    ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarMinButton ), widget );
		    QStyle::SubControl ctrl = (controls & SC_TitleBarNormalButton ?
					       SC_TitleBarNormalButton :
					       SC_TitleBarMinButton);
		    QStyle::StylePixmap spixmap = (controls & SC_TitleBarNormalButton ?
						   SP_TitleBarNormalButton :
						   SP_TitleBarMinButton);
		    down = active & ctrl;
		    pm = QPixmap(stylePixmap(spixmap, widget));
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);

		    p->save();
		    if( down )
			p->translate( pixelMetric(PM_ButtonShiftHorizontal, widget),
				      pixelMetric(PM_ButtonShiftVertical, widget) );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		    p->restore();
		}

		if ( controls & SC_TitleBarShadeButton ) {
		    ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarShadeButton ), widget );

		    down = active & SC_TitleBarShadeButton;
		    pm = QPixmap(stylePixmap(SP_TitleBarShadeButton, widget));
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);
		    p->save();
		    if( down )
			p->translate( pixelMetric(PM_ButtonShiftHorizontal, widget),
				      pixelMetric(PM_ButtonShiftVertical, widget) );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		    p->restore();
		}

		if ( controls & SC_TitleBarUnshadeButton ) {
		    ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarUnshadeButton ), widget );

		    down = active & SC_TitleBarUnshadeButton;
		    pm = QPixmap(stylePixmap(SP_TitleBarUnshadeButton, widget));
		    drawPrimitive(PE_ButtonTool, p, ir, titlebar->colorGroup(),
				  down ? Style_Down : Style_Raised);
		    p->save();
		    if( down )
			p->translate( pixelMetric(PM_ButtonShiftHorizontal, widget),
				      pixelMetric(PM_ButtonShiftVertical, widget) );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, &pm, QString::null );
		    p->restore();
		}
	    }
#ifndef QT_NO_WIDGET_TOPEXTRA
	    if ( controls & SC_TitleBarSysMenu ) {
		if ( titlebar->icon() ) {
		    ir = visualRect( querySubControlMetrics( CC_TitleBar, widget, SC_TitleBarSysMenu ), widget );
		    drawItem( p, ir, AlignCenter, titlebar->colorGroup(), TRUE, titlebar->icon(), QString::null );
		}
	    }
#endif
	    break;
	}
#endif //QT_NO_TITLEBAR

    case CC_SpinWidget: {
#ifndef QT_NO_SPINWIDGET
	const QSpinWidget * sw = (const QSpinWidget *) widget;
	SFlags flags;
	PrimitiveElement pe;

	if ( controls & SC_SpinWidgetFrame )
	    qDrawWinPanel( p, r, cg, TRUE ); //cstyle == Sunken );

	if ( controls & SC_SpinWidgetUp ) {
	    flags = Style_Default | Style_Enabled;
	    if (active == SC_SpinWidgetUp ) {
		flags |= Style_On;
		flags |= Style_Sunken;
	    } else
		flags |= Style_Raised;
	    if ( sw->buttonSymbols() == QSpinWidget::PlusMinus )
		pe = PE_SpinWidgetPlus;
	    else
		pe = PE_SpinWidgetUp;

	    QRect re = sw->upRect();
	    QColorGroup ucg = sw->isUpEnabled() ? cg : sw->palette().disabled();
	    drawPrimitive(PE_ButtonBevel, p, re, ucg, flags);
	    drawPrimitive(pe, p, re, ucg, flags);
	}

	if ( controls & SC_SpinWidgetDown ) {
	    flags = Style_Default | Style_Enabled;
	    if (active == SC_SpinWidgetDown ) {
		flags |= Style_On;
		flags |= Style_Sunken;
	    } else
		flags |= Style_Raised;
	    if ( sw->buttonSymbols() == QSpinWidget::PlusMinus )
		pe = PE_SpinWidgetMinus;
	    else
		pe = PE_SpinWidgetDown;

	    QRect re = sw->downRect();
	    QColorGroup dcg = sw->isDownEnabled() ? cg : sw->palette().disabled();
	    drawPrimitive(PE_ButtonBevel, p, re, dcg, flags);
	    drawPrimitive(pe, p, re, dcg, flags);
	}
#endif
	break; }

#ifndef QT_NO_SLIDER
    case CC_Slider:
	switch ( controls ) {
	case SC_SliderTickmarks: {
	    const QSlider * sl = (const QSlider *) widget;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int ticks = sl->tickmarks();
	    int thickness = pixelMetric( PM_SliderControlThickness, sl );
	    int len = pixelMetric( PM_SliderLength, sl );
	    int available = pixelMetric( PM_SliderSpaceAvailable, sl );
	    int interval = sl->tickInterval();

	    if ( interval <= 0 ) {
		interval = sl->lineStep();
		if ( qPositionFromValue( sl, interval, available ) -
		     qPositionFromValue( sl, 0, available ) < 3 )
		    interval = sl->pageStep();
	    }

	    int fudge = len / 2;
	    int pos;

	    if ( ticks & QSlider::Above ) {
		p->setPen( cg.foreground() );
		int v = sl->minValue();
		if ( !interval )
		    interval = 1;
		while ( v <= sl->maxValue() + 1 ) {
		    pos = qPositionFromValue( sl, v, available ) + fudge;
		    if ( sl->orientation() == Horizontal )
			p->drawLine( pos, 0, pos, tickOffset-2 );
		    else
			p->drawLine( 0, pos, tickOffset-2, pos );
		    v += interval;
		}
	    }

	    if ( ticks & QSlider::Below ) {
		p->setPen( cg.foreground() );
		int v = sl->minValue();
		if ( !interval )
		    interval = 1;
		while ( v <= sl->maxValue() + 1 ) {
		    pos = qPositionFromValue( sl, v, available ) + fudge;
		    if ( sl->orientation() == Horizontal )
			p->drawLine( pos, tickOffset+thickness+1, pos,
				     tickOffset+thickness+1 + available-2 );
		    else
			p->drawLine( tickOffset+thickness+1, pos,
				     tickOffset+thickness+1 + available-2,
				     pos );
		    v += interval;
		}

	    }

	    break; }
	}
	break;
#endif // QT_NO_SLIDER
#ifndef QT_NO_LISTVIEW
    case CC_ListView:
	if ( controls & SC_ListView ) {
	    QListView *listview = (QListView*)widget;
	    p->fillRect( r, listview->viewport()->backgroundBrush() );
	}
	break;
#endif //QT_NO_LISTVIEW
    default:
	break;
    }

    activePainter = 0;
}


/*! \reimp */
void QCommonStyle::drawComplexControlMask( ComplexControl control,
					   QPainter *p,
					   const QWidget *widget,
					   const QRect &r,
					   const QStyleOption& opt ) const
{
    Q_UNUSED(control);
    Q_UNUSED(widget);
    Q_UNUSED(opt);

    p->fillRect(r, color1);
}


/*! \reimp */
QRect QCommonStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *widget,
					    SubControl sc,
					    const QStyleOption &opt ) const
{
#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::querySubControlMetrics: widget parameter cannot be zero!");
	return QRect();
    }
#endif

    switch ( control ) {
    case CC_SpinWidget: {
	int fw = pixelMetric( PM_SpinBoxFrameWidth, widget);
	QSize bs;
	bs.setHeight( widget->height()/2 - fw );
	if ( bs.height() < 8 )
	    bs.setHeight( 8 );
	bs.setWidth( QMIN( bs.height() * 8 / 5, widget->width() / 4 ) ); // 1.6 -approximate golden mean
	bs = bs.expandedTo( QApplication::globalStrut() );
	int y = fw;
	int x, lx, rx;
	x = widget->width() - y - bs.width();
	lx = fw;
	rx = x - fw;
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
	    return widget->rect();
	default:
	    break;
	}
	break; }

    case CC_ComboBox: {
	int x = 0, y = 0, wi = widget->width(), he = widget->height();
	int xpos = x;
	xpos += wi - 2 - 16;

	switch ( sc ) {
	case SC_ComboBoxFrame:
	    return widget->rect();
	case SC_ComboBoxArrow:
	    return QRect(xpos, y+2, 16, he-4);
	case SC_ComboBoxEditField:
	    return QRect(x+3, y+3, wi-6-16, he-6);
	case SC_ComboBoxListBoxPopup:
	    return opt.rect();
	default:
	    break;
	}
	break; }

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	const QScrollBar *scrollbar = (const QScrollBar *) widget;
	int sliderstart = 0;
	int sbextent = pixelMetric(PM_ScrollBarExtent, widget);
	int maxlen = ((scrollbar->orientation() == Qt::Horizontal) ?
		      scrollbar->width() : scrollbar->height()) - (sbextent * 2);
	int sliderlen;

	sliderstart = scrollbar->sliderStart();

	// calculate slider length
	if (scrollbar->maxValue() != scrollbar->minValue()) {
	    uint range = scrollbar->maxValue() - scrollbar->minValue();
	    sliderlen = (Q_LLONG(scrollbar->pageStep()) * maxlen) / (range + scrollbar->pageStep());

	    int slidermin = pixelMetric( PM_ScrollBarSliderMin, widget );
	    if ( sliderlen < slidermin || range > INT_MAX / 2 )
		sliderlen = slidermin;
	    if ( sliderlen > maxlen )
		sliderlen = maxlen;
	} else
	    sliderlen = maxlen;

	switch (sc) {
	case SC_ScrollBarSubLine:	    // top/left button
	    if (scrollbar->orientation() == Qt::Horizontal) {
		int buttonWidth = QMIN(scrollbar->width()/2, sbextent);
		return QRect( 0, 0, buttonWidth, sbextent );
	    } else {
		int buttonHeight = QMIN(scrollbar->height()/2, sbextent);
		return QRect( 0, 0, sbextent, buttonHeight );
	    }

	case SC_ScrollBarAddLine:	    // bottom/right button
	    if (scrollbar->orientation() == Qt::Horizontal) {
		int buttonWidth = QMIN(scrollbar->width()/2, sbextent);
		return QRect( scrollbar->width() - buttonWidth, 0, buttonWidth, sbextent );
	    } else {
		int buttonHeight = QMIN(scrollbar->height()/2, sbextent);
		return QRect( 0, scrollbar->height() - buttonHeight, sbextent, buttonHeight );
	    }

	case SC_ScrollBarSubPage:	    // between top/left button and slider
	    if (scrollbar->orientation() == Qt::Horizontal)
		return QRect(sbextent, 0, sliderstart - sbextent, sbextent);
	    return QRect(0, sbextent, sbextent, sliderstart - sbextent);

	case SC_ScrollBarAddPage:	    // between bottom/right button and slider
	    if (scrollbar->orientation() == Qt::Horizontal)
		return QRect(sliderstart + sliderlen, 0,
			     maxlen - sliderstart - sliderlen + sbextent, sbextent);
	    return QRect(0, sliderstart + sliderlen,
			 sbextent, maxlen - sliderstart - sliderlen + sbextent);

	case SC_ScrollBarGroove:
	    if (scrollbar->orientation() == Qt::Horizontal)
		return QRect(sbextent, 0, scrollbar->width() - sbextent * 2,
			     scrollbar->height());
	    return QRect(0, sbextent, scrollbar->width(),
			 scrollbar->height() - sbextent * 2);

	case SC_ScrollBarSlider:
	    if (scrollbar->orientation() == Qt::Horizontal)
		return QRect(sliderstart, 0, sliderlen, sbextent);
	    return QRect(0, sliderstart, sbextent, sliderlen);

	default: break;
	}

	break; }
#endif // QT_NO_SCROLLBAR

#ifndef QT_NO_SLIDER
    case CC_Slider: {
	    const QSlider * sl = (const QSlider *) widget;
	    int tickOffset = pixelMetric( PM_SliderTickmarkOffset, sl );
	    int thickness = pixelMetric( PM_SliderControlThickness, sl );

	    switch ( sc ) {
	    case SC_SliderHandle: {
		    int sliderPos = 0;
		    int len   = pixelMetric( PM_SliderLength, sl );

		    sliderPos = sl->sliderStart();

		    if ( sl->orientation() == Horizontal )
			return QRect( sliderPos, tickOffset, len, thickness );
		    return QRect( tickOffset, sliderPos, thickness, len ); }
	    case SC_SliderGroove: {
		if ( sl->orientation() == Horizontal )
		    return QRect( 0, tickOffset, sl->width(), thickness );
		return QRect( tickOffset, 0, thickness, sl->height() );	}

	    default:
		break;
	    }
	    break; }
#endif // QT_NO_SLIDER

#if !defined(QT_NO_TOOLBUTTON) && !defined(QT_NO_POPUPMENU)
    case CC_ToolButton: {
	    const QToolButton *toolbutton = (const QToolButton *) widget;
	    int mbi = pixelMetric(PM_MenuButtonIndicator, widget);

	    QRect rect = toolbutton->rect();
	    switch (sc) {
	    case SC_ToolButton:
		if (toolbutton->popup() && ! toolbutton->popupDelay())
		    rect.addCoords(0, 0, -mbi, 0);
		return rect;

	    case SC_ToolButtonMenu:
		if (toolbutton->popup() && ! toolbutton->popupDelay())
		    rect.addCoords(rect.width() - mbi, 0, 0, 0);
		return rect;

	    default: break;
	    }
	    break;
	}
#endif // QT_NO_TOOLBUTTON && QT_NO_POPUPMENU

#ifndef QT_NO_TITLEBAR
    case CC_TitleBar: {
	    const QTitleBar *titlebar = (const QTitleBar *) widget;
	    const int controlTop = 2;
	    const int controlHeight = widget->height() - controlTop * 2;

	    switch (sc) {
	    case SC_TitleBarLabel: {
		const QTitleBar *titlebar = (QTitleBar*)widget;
		QRect ir( 0, 0, titlebar->width(), titlebar->height() );
		if ( titlebar->testWFlags( WStyle_Tool ) ) {
		    if ( titlebar->testWFlags( WStyle_SysMenu ) )
			ir.addCoords( 0, 0, -controlHeight-3, 0 );
		    if ( titlebar->testWFlags( WStyle_MinMax ) )
			ir.addCoords( 0, 0, -controlHeight-2, 0 );
		} else {
		    if ( titlebar->testWFlags( WStyle_SysMenu ) )
			ir.addCoords( controlHeight+3, 0, -controlHeight-3, 0 );
		    if ( titlebar->testWFlags( WStyle_Minimize ) )
			ir.addCoords( 0, 0, -controlHeight-2, 0 );
		    if ( titlebar->testWFlags( WStyle_Maximize ) )
			ir.addCoords( 0, 0, -controlHeight-2, 0 );
		}
		return ir; }

	    case SC_TitleBarCloseButton:
		return QRect( titlebar->width() - ( controlHeight + controlTop ),
			      controlTop, controlHeight, controlHeight );

	    case SC_TitleBarMaxButton:
	    case SC_TitleBarShadeButton:
	    case SC_TitleBarUnshadeButton:
		return QRect( titlebar->width() - ( ( controlHeight + controlTop ) * 2 ),
			      controlTop, controlHeight, controlHeight );

	    case SC_TitleBarMinButton:
	    case SC_TitleBarNormalButton: {
		int offset = controlHeight + controlTop;
		if ( !titlebar->testWFlags( WStyle_Maximize ) )
		    offset *= 2;
		else
		    offset *= 3;
		return QRect( titlebar->width() - offset, controlTop, controlHeight, controlHeight );
	    }

	    case SC_TitleBarSysMenu:
		return QRect( 3, controlTop, controlHeight, controlHeight);

	    default: break;
	    }
	    break; }
#endif //QT_NO_TITLEBAR

    default:
	break;
    }
    return QRect();
}


/*! \reimp */
QStyle::SubControl QCommonStyle::querySubControl(ComplexControl control,
						 const QWidget *widget,
						 const QPoint &pos,
						 const QStyleOption& opt ) const
{
    SubControl ret = SC_None;

    switch (control) {
#ifndef QT_NO_LISTVIEW
    case CC_ListView:
	{
	    if(pos.x() >= 0 && pos.x() <
	       opt.listViewItem()->listView()->treeStepSize())
		ret = SC_ListViewExpand;
	    break;
	}
#endif
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar:
	{
	    QRect r;
	    uint ctrl = SC_ScrollBarAddLine;

	    // we can do this because subcontrols were designed to be masks as well...
	    while (ret == SC_None && ctrl <= SC_ScrollBarGroove) {
		r = querySubControlMetrics(control, widget,
					   (QStyle::SubControl) ctrl, opt);
		if (r.isValid() && r.contains(pos))
		    ret = (QStyle::SubControl) ctrl;

		ctrl <<= 1;
	    }

	    break;
	}
#endif
    case CC_TitleBar:
	{
#ifndef QT_NO_TITLEBAR
	    const QTitleBar *titlebar = (QTitleBar*)widget;
	    QRect r;
	    uint ctrl = SC_TitleBarLabel;

	    // we can do this because subcontrols were designed to be masks as well...
	    while (ret == SC_None && ctrl <= SC_TitleBarUnshadeButton) {
		r = visualRect( querySubControlMetrics( control, widget, (QStyle::SubControl) ctrl, opt ), widget );
		if (r.isValid() && r.contains(pos))
		    ret = (QStyle::SubControl) ctrl;

		ctrl <<= 1;
	    }
	    if ( titlebar->window() ) {
		if (titlebar->testWFlags( WStyle_Tool )) {
		    if ( ret == SC_TitleBarMinButton || ret == SC_TitleBarMaxButton ) {
			if ( titlebar->window()->isMinimized() )
			    ret = SC_TitleBarUnshadeButton;
			else
			    ret = SC_TitleBarShadeButton;
		    }
		} else if ( ret == SC_TitleBarMinButton && titlebar->window()->isMinimized() ) {
			ret = QStyle::SC_TitleBarNormalButton;
		}
	    }
#endif
	    break;
	}

    default:
	break;
    }

    return ret;
}


/*! \reimp */
int QCommonStyle::pixelMetric(PixelMetric m, const QWidget *widget) const
{
    int ret;

    switch (m) {
    case PM_DialogButtonsSeparator:
	ret = 5;
	break;
    case PM_DialogButtonsButtonWidth:
	ret = 70;
	break;
    case PM_DialogButtonsButtonHeight:
	ret = 30;
	break;
    case PM_CheckListControllerSize:
    case PM_CheckListButtonSize:
	ret = 16;
	break;
    case PM_TitleBarHeight: {
	if ( widget ) {
	    if ( widget->testWFlags( WStyle_Tool ) ) {
		ret = QMAX( widget->fontMetrics().lineSpacing(), 16 );
#ifndef QT_NO_MAINWINDOW
	    } else if ( ::qt_cast<QDockWindow*>(widget) ) {
		ret = QMAX( widget->fontMetrics().lineSpacing(), 13 );
#endif
	    } else {
		ret = QMAX( widget->fontMetrics().lineSpacing(), 18 );
	    }
	} else {
	    ret = 0;
	}

	break; }
    case PM_ScrollBarSliderMin:
	ret = 9;
	break;

    case PM_ButtonMargin:
	ret = 6;
	break;

    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;

    case PM_MenuButtonIndicator:
	if (! widget)
	    ret = 12;
	else
	    ret = QMAX(12, (widget->height() - 4) / 3);
	break;

    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;

    case PM_SpinBoxFrameWidth:
    case PM_DefaultFrameWidth:
	ret = 2;
	break;

    case PM_MDIFrameWidth:
	ret = 2;
	break;

    case PM_MDIMinimizedWidth:
	ret = 196;
	break;

#ifndef QT_NO_SCROLLBAR
    case PM_ScrollBarExtent:
	if ( !widget ) {
	    ret = 16;
	} else {
	    const QScrollBar *bar = (const QScrollBar*)widget;
	    int s = bar->orientation() == Qt::Horizontal ?
		    QApplication::globalStrut().height()
		    : QApplication::globalStrut().width();
	    ret = QMAX( 16, s );
	}
	break;
#endif
    case PM_MaximumDragDistance:
	ret = -1;
	break;

#ifndef QT_NO_SLIDER
    case PM_SliderThickness:
	ret = 16;
	break;

    case PM_SliderTickmarkOffset:
	{
	    if (! widget) {
		ret = 0;
		break;
	    }

	    const QSlider * sl = (const QSlider *) widget;
	    int space = (sl->orientation() == Horizontal) ? sl->height() :
			sl->width();
	    int thickness = pixelMetric( PM_SliderControlThickness, sl );
	    int ticks = sl->tickmarks();

	    if ( ticks == QSlider::Both )
		ret = (space - thickness) / 2;
	    else if ( ticks == QSlider::Above )
		ret = space - thickness;
	    else
		ret = 0;
	    break;
	}

    case PM_SliderSpaceAvailable:
	{
	    const QSlider * sl = (const QSlider *) widget;
	    if ( sl->orientation() == Horizontal )
		ret = sl->width() - pixelMetric( PM_SliderLength, sl );
	    else
		ret = sl->height() - pixelMetric( PM_SliderLength, sl );
	    break;
	}
#endif // QT_NO_SLIDER

    case PM_DockWindowSeparatorExtent:
	ret = 6;
	break;

    case PM_DockWindowHandleExtent:
	ret = 8;
	break;

    case PM_DockWindowFrameWidth:
	ret = 1;
	break;

    case PM_MenuBarFrameWidth:
	ret = 2;
	break;

    case PM_MenuBarItemSpacing:
    case PM_ToolBarItemSpacing:
	ret = 0;
	break;

    case PM_TabBarTabOverlap:
	ret = 3;
	break;

    case PM_TabBarBaseHeight:
	ret = 0;
	break;

    case PM_TabBarBaseOverlap:
	ret = 0;
	break;

    case PM_TabBarTabHSpace:
	ret = 24;
	break;

    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
	ret = 2;
	break;

#ifndef QT_NO_TABBAR
    case PM_TabBarTabVSpace:
	{
	    const QTabBar * tb = (const QTabBar *) widget;
	    if ( tb && ( tb->shape() == QTabBar::RoundedAbove ||
			 tb->shape() == QTabBar::RoundedBelow ) )
		ret = 10;
	    else
		ret = 0;
	    break;
	}
#endif

    case PM_ProgressBarChunkWidth:
	ret = 9;
	break;

    case PM_IndicatorWidth:
	ret = 13;
	break;

    case PM_IndicatorHeight:
	ret = 13;
	break;

    case PM_ExclusiveIndicatorWidth:
	ret = 12;
	break;

    case PM_ExclusiveIndicatorHeight:
	ret = 12;
	break;

    case PM_PopupMenuFrameHorizontalExtra:
    case PM_PopupMenuFrameVerticalExtra:
	ret = 0;
	break;

    case PM_HeaderMargin:
	ret = 4;
	break;
    case PM_HeaderMarkSize:
	ret = 32;
	break;
    case PM_HeaderGripMargin:
	ret = 4;
	break;
    case PM_TabBarScrollButtonWidth:
	ret = 16;
	break;
    default:
	ret = 0;
	break;
    }

    return ret;
}


/*! \reimp */
QSize QCommonStyle::sizeFromContents(ContentsType contents,
				     const QWidget *widget,
				     const QSize &contentsSize,
				     const QStyleOption& opt ) const
{
    QSize sz(contentsSize);

#if defined(QT_CHECK_STATE)
    if (! widget) {
	qWarning("QCommonStyle::sizeFromContents: widget parameter cannot be zero!");
	return sz;
    }
#endif

    switch (contents) {
#ifndef QT_NO_DIALOGBUTTONS
    case CT_DialogButtons: {
	const QDialogButtons *dbtns = (const QDialogButtons *)widget;
	int w = contentsSize.width(), h = contentsSize.height();
	const int bwidth = pixelMetric(PM_DialogButtonsButtonWidth, widget),
		  bspace = pixelMetric(PM_DialogButtonsSeparator, widget),
		 bheight = pixelMetric(PM_DialogButtonsButtonHeight, widget);
	if(dbtns->orientation() == Horizontal) {
	    if(!w)
		w = bwidth;
	} else {
	    if(!h)
		h = bheight;
	}
	QDialogButtons::Button btns[] = { QDialogButtons::All, QDialogButtons::Reject, QDialogButtons::Accept, //reverse order (right to left)
					  QDialogButtons::Apply, QDialogButtons::Retry, QDialogButtons::Ignore, QDialogButtons::Abort,
					  QDialogButtons::Help };
	for(unsigned int i = 0, cnt = 0; i < (sizeof(btns)/sizeof(btns[0])); i++) {
	    if(dbtns->isButtonVisible(btns[i])) {
		QSize szH = dbtns->sizeHint(btns[i]);
		int mwidth = QMAX(bwidth, szH.width()), mheight = QMAX(bheight, szH.height());
		if(dbtns->orientation() == Horizontal)
		    h = QMAX(h, mheight);
		else
		    w = QMAX(w, mwidth);

		if(cnt)
		    w += bspace;
		cnt++;
		if(dbtns->orientation() == Horizontal)
		    w += mwidth;
		else
		    h += mheight;
	    }
	}
	const int fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
	sz = QSize(w + fw, h + fw);
	break; }
#endif //QT_NO_DIALOGBUTTONS
    case CT_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    int w = contentsSize.width(),
		h = contentsSize.height(),
	       bm = pixelMetric(PM_ButtonMargin, widget),
	       fw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;

	    w += bm + fw;
	    h += bm + fw;

	    if (button->isDefault() || button->autoDefault()) {
		int dbw = pixelMetric(PM_ButtonDefaultIndicator, widget) * 2;
		w += dbw;
		h += dbw;
	    }

	    sz = QSize(w, h);
#endif
	    break;
	}

    case CT_CheckBox:
	{
#ifndef QT_NO_CHECKBOX
	    const QCheckBox *checkbox = (const QCheckBox *) widget;
	    QRect irect = subRect(SR_CheckBoxIndicator, widget);
	    int h = pixelMetric( PM_IndicatorHeight, widget );
	    int margins = (!checkbox->pixmap() && checkbox->text().isEmpty()) ? 0 : 10;
	    sz += QSize(irect.right() + margins, 4 );
	    sz.setHeight( QMAX( sz.height(), h ) );
#endif
	    break;
	}

    case CT_RadioButton:
	{
#ifndef QT_NO_RADIOBUTTON
	    const QRadioButton *radiobutton = (const QRadioButton *) widget;
	    QRect irect = subRect(SR_RadioButtonIndicator, widget);
	    int h = pixelMetric( PM_ExclusiveIndicatorHeight, widget );
	    int margins = (!radiobutton->pixmap() && radiobutton->text().isEmpty()) ? 0 : 10;
	    sz += QSize(irect.right() + margins, 4 );
	    sz.setHeight( QMAX( sz.height(), h ) );
#endif
	    break;
	}

    case CT_ToolButton:
	{
	    sz = QSize(sz.width() + 6, sz.height() + 5);
	    break;
	}

    case CT_ComboBox:
	{
	    int dfw = pixelMetric(PM_DefaultFrameWidth, widget) * 2;
	    sz = QSize(sz.width() + dfw + 21, sz.height() + dfw );
	    break;
	}

    case CT_PopupMenuItem:
	{
#ifndef QT_NO_POPUPMENU
	    if (opt.isDefault())
		break;

	    const QPopupMenu *popup = (const QPopupMenu *) widget;
	    bool checkable = popup->isCheckable();
	    QMenuItem *mi = opt.menuItem();
	    int maxpmw = opt.maxIconWidth();
	    int w = sz.width(), h = sz.height();

	    if (mi->custom()) {
		w = mi->custom()->sizeHint().width();
		h = mi->custom()->sizeHint().height();
		if (! mi->custom()->fullSpan())
		    h += 8;
	    } else if ( mi->widget() ) {
	    } else if (mi->isSeparator()) {
		w = 10;
		h = 2;
	    } else {
		if (mi->pixmap())
		    h = QMAX(h, mi->pixmap()->height() + 4);
		else
		    h = QMAX(h, popup->fontMetrics().height() + 8);

		if (mi->iconSet() != 0)
		    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						      QIconSet::Normal).height() + 4);
	    }

	    if (! mi->text().isNull()) {
		if (mi->text().find('\t') >= 0)
		    w += 12;
	    }

	    if (maxpmw)
		w += maxpmw + 6;
	    if (checkable && maxpmw < 20)
		w += 20 - maxpmw;
	    if (checkable || maxpmw > 0)
		w += 2;
	    w += 12;

	    sz = QSize(w, h);
#endif
	    break;
	}

    case CT_LineEdit:
    case CT_Header:
    case CT_Slider:
    case CT_ProgressBar:
	// just return the contentsSize for now
	// fall through intended

    default:
	break;
    }

    return sz;
}


/*! \reimp */
int QCommonStyle::styleHint(StyleHint sh, const QWidget * w, const QStyleOption &, QStyleHintReturn *) const
{
    int ret;

    switch (sh) {
#ifndef QT_NO_DIALOGBUTTONS
    case SH_DialogButtons_DefaultButton:
	ret = QDialogButtons::Accept;
	break;
#endif
    case SH_GroupBox_TextLabelVerticalAlignment:
	ret = Qt::AlignVCenter;
	break;

    case SH_GroupBox_TextLabelColor:
	ret = (int) ( w ? w->paletteForegroundColor().rgb() : 0 );
	break;

    case SH_ListViewExpand_SelectMouseType:
    case SH_TabBar_SelectMouseType:
	ret = QEvent::MouseButtonPress;
	break;

    case SH_GUIStyle:
	ret = WindowsStyle;
	break;

    case SH_ScrollBar_BackgroundMode:
	ret = QWidget::PaletteBackground;
	break;

    case SH_TabBar_Alignment:
    case SH_Header_ArrowAlignment:
	ret = Qt::AlignLeft;
	break;

    case SH_PopupMenu_SubMenuPopupDelay:
	ret = 256;
	break;

    case SH_ProgressDialog_TextLabelAlignment:
	ret = Qt::AlignCenter;
	break;

    case SH_BlinkCursorWhenTextSelected:
	ret = 1;
	break;

    case SH_Table_GridLineColor:
	ret = -1;
	break;

    case SH_LineEdit_PasswordCharacter:
	ret = '*';
	break;

    case SH_ToolBox_SelectedPageTitleBold:
	ret = 1;
	break;

    case SH_UnderlineAccelerator:
	ret = 1;
	break;

    case SH_ToolButton_Uses3D:
	ret = 1;
	break;

    default:
	ret = 0;
	break;
    }

    return ret;
}

/*! \reimp */
QPixmap QCommonStyle::stylePixmap(StylePixmap, const QWidget *, const QStyleOption&) const
{
    return QPixmap();
}


#endif // QT_NO_STYLE
