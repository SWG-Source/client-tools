/****************************************************************************
** $Id: qwindowsstyle.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of Windows-like style class
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

#include "qwindowsstyle.h"

#if !defined(QT_NO_STYLE_WINDOWS) || defined(QT_PLUGIN)

#include "qpopupmenu.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h" // for now
#include "qpixmap.h" // for now
#include "qwidget.h"
#include "qlabel.h"
#include "qimage.h"
#include "qpushbutton.h"
#include "qcombobox.h"
#include "qlistbox.h"
#include "qwidget.h"
#include "qrangecontrol.h"
#include "qscrollbar.h"
#include "qslider.h"
#include "qtabwidget.h"
#include "qtabbar.h"
#include "qlistview.h"
#include "qbitmap.h"
#include "qcleanuphandler.h"
#include "qdockwindow.h"
#include "qobjectlist.h"
#include "qmenubar.h"

#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif

#include <limits.h>


static const int windowsItemFrame		=  2; // menu item frame width
static const int windowsSepHeight		=  2; // separator item height
static const int windowsItemHMargin		=  3; // menu item hor text margin
static const int windowsItemVMargin		=  2; // menu item ver text margin
static const int windowsArrowHMargin		=  6; // arrow horizontal margin
static const int windowsTabSpacing		= 12; // space between text and tab
static const int windowsCheckMarkHMargin	=  2; // horiz. margins of check mark
static const int windowsRightBorder		= 12; // right border on windows
static const int windowsCheckMarkWidth		= 12; // checkmarks width on windows

static bool use2000style = TRUE;

enum QSliderDirection { SlUp, SlDown, SlLeft, SlRight };

// A friendly class providing access to QMenuData's protected member.
class FriendlyMenuData : public QMenuData
{
    friend class QWindowsStyle;
};

// Private class
class QWindowsStyle::Private : public QObject
{
public:
    Private(QWindowsStyle *parent);

    bool hasSeenAlt(const QWidget *widget) const;
    bool altDown() const { return alt_down; }

protected:
    bool eventFilter(QObject *o, QEvent *e);

private:
    QPtrList<QWidget> seenAlt;
    bool alt_down;
    int menuBarTimer;
};

QWindowsStyle::Private::Private(QWindowsStyle *parent)
: QObject(parent, "QWindowsStylePrivate"), alt_down(FALSE), menuBarTimer(0)
{
}

// Returns true if the toplevel parent of \a widget has seen the Alt-key
bool QWindowsStyle::Private::hasSeenAlt(const QWidget *widget) const
{
    widget = widget->topLevelWidget();
    return seenAlt.contains(widget);
}

// Records Alt- and Focus events
bool QWindowsStyle::Private::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
	return QObject::eventFilter(o, e);

    QWidget *widget = ::qt_cast<QWidget*>(o);

    switch(e->type()) {
    case QEvent::KeyPress:
	if (((QKeyEvent*)e)->key() == Key_Alt) {
	    widget = widget->topLevelWidget();

	    // Alt has been pressed - find all widgets that care
	    QObjectList *l = widget->queryList("QWidget");
	    QObjectListIt it( *l );
	    QWidget *w;
	    while ( (w = (QWidget*)it.current()) != 0 ) {
		++it;
		if (w->isTopLevel() || !w->isVisible() ||
		    w->style().styleHint(SH_UnderlineAccelerator, w))
		    l->removeRef(w);
	    }
	    // Update states before repainting
	    seenAlt.append(widget);
	    alt_down = TRUE;

	    // Repaint all relevant widgets
	    it.toFirst();
	    while ( (w = (QWidget*)it.current()) != 0 ) {
		++it;
		w->repaint(FALSE);
	    }
	    delete l;
	}
	break;
    case QEvent::KeyRelease:
	if (((QKeyEvent*)e)->key() == Key_Alt) {
	    widget = widget->topLevelWidget();

	    // Update state
	    alt_down = FALSE;
	    // Repaint only menubars
	    QObjectList *l = widget->queryList("QMenuBar");
	    QObjectListIt it( *l );
	    QMenuBar *menuBar;
	    while ( (menuBar = (QMenuBar*)it.current()) != 0) {
		++it;
		menuBar->repaint(FALSE);
	    }
	}
	break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
	{
	    // Menubars toggle based on focus
	    QMenuBar *menuBar = ::qt_cast<QMenuBar*>(o);
	    if (menuBar && !menuBarTimer) // delayed repaint to avoid flicker
		menuBarTimer = menuBar->startTimer(0);
	}
	break;
    case QEvent::Close:
	// Reset widget when closing
	seenAlt.removeRef(widget);
	seenAlt.removeRef(widget->topLevelWidget());
	break;
    case QEvent::Timer:
	{
	    QMenuBar *menuBar = ::qt_cast<QMenuBar*>(o);
	    QTimerEvent *te = (QTimerEvent*)e;
	    if (menuBar && te->timerId() == menuBarTimer) {
		menuBar->killTimer(te->timerId());
		menuBarTimer = 0;
		menuBar->repaint(FALSE);
		return TRUE;
	    }
	}
	break;
    default:
	break;
    }

    return QObject::eventFilter(o, e);
}

/*!
    \class QWindowsStyle qwindowsstyle.h
    \brief The QWindowsStyle class provides a Microsoft Windows-like look and feel.

    \ingroup appearance

    This style is Qt's default GUI style on Windows.
*/

/*!
    Constructs a QWindowsStyle
*/
QWindowsStyle::QWindowsStyle() : QCommonStyle(), d(0)
{
#if defined(Q_OS_WIN32)
    use2000style = qWinVersion() != Qt::WV_NT && qWinVersion() != Qt::WV_95;
#endif
}

/*! \reimp */
QWindowsStyle::~QWindowsStyle()
{
    delete d;
}

/*! \reimp */
void QWindowsStyle::polish(QApplication *app)
{
    // We only need the overhead when shortcuts are sometimes hidden
    if (!styleHint(SH_UnderlineAccelerator, 0)) {
	d = new Private(this);
	app->installEventFilter(d);
    }
}

/*! \reimp */
void QWindowsStyle::unPolish(QApplication *)
{
    delete d;
    d = 0;
}

/*! \reimp */
void QWindowsStyle::polish(QWidget *widget)
{
    QCommonStyle::polish(widget);
}

/*! \reimp */
void QWindowsStyle::unPolish(QWidget *widget)
{
    QCommonStyle::polish(widget);
}

/*! \reimp */
void QWindowsStyle::polish( QPalette &pal )
{
    QCommonStyle::polish(pal);
}

/*! \reimp */
void QWindowsStyle::drawPrimitive( PrimitiveElement pe,
				   QPainter *p,
				   const QRect &r,
				   const QColorGroup &cg,
				   SFlags flags,
				   const QStyleOption& opt ) const
{
    QRect rr( r );
    switch (pe) {
    case PE_ButtonCommand:
	{
	    QBrush fill;

	    if (! (flags & Style_Down) && (flags & Style_On))
		fill = QBrush(cg.light(), Dense4Pattern);
	    else
		fill = cg.brush(QColorGroup::Button);

	    if (flags & Style_ButtonDefault && flags & Style_Down) {
		p->setPen(cg.dark());
		p->setBrush(fill);
		p->drawRect(r);
	    } else if (flags & (Style_Raised | Style_Down | Style_On | Style_Sunken))
		qDrawWinButton(p, r, cg, flags & (Style_Sunken | Style_Down |
						  Style_On), &fill);
	    else
		p->fillRect(r, fill);
	    break;
	}

    case PE_ButtonBevel:
    case PE_HeaderSection:
	{
	    QBrush fill;

	    if (! (flags & Style_Down) && (flags & Style_On))
		fill = QBrush(cg.light(), Dense4Pattern);
	    else
		fill = cg.brush(QColorGroup::Button);

	    if (flags & (Style_Raised | Style_Down | Style_On | Style_Sunken))
		qDrawWinButton(p, r, cg, flags & (Style_Down | Style_On), &fill);
	    else
		p->fillRect(r, fill);
	    break;
	}
#if defined(Q_WS_WIN)
    case PE_HeaderArrow:
	p->save();
	if ( flags & Style_Up ) { // invert logic to follow Windows style guide
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
#endif

    case PE_ButtonDefault:
	p->setPen(cg.shadow());
	p->drawRect(r);
	break;

    case PE_ButtonTool:
	{
	    QBrush fill;
	    bool stippled = FALSE;

	    if (! (flags & (Style_Down | Style_MouseOver)) &&
		(flags & Style_On) &&
		use2000style) {
		fill = QBrush(cg.light(), Dense4Pattern);
		stippled = TRUE;
	    } else
		fill = cg.brush(QColorGroup::Button);

	    if (flags & (Style_Raised | Style_Down | Style_On)) {
		if (flags & Style_AutoRaise) {
		    qDrawShadePanel(p, r, cg, flags & (Style_Down | Style_On),
				    1, &fill);

		    if (stippled) {
			p->setPen(cg.button());
			p->drawRect(r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2);
		    }
		} else
		    qDrawWinButton(p, r, cg, flags & (Style_Down | Style_On),
				   &fill);
	    } else
		p->fillRect(r, fill);

	    break;
	}

    case PE_FocusRect:
	if (opt.isDefault())
	    p->drawWinFocusRect(r);
	else
	    p->drawWinFocusRect(r, opt.color());
	break;

    case PE_Indicator:
	{
	    QBrush fill;
	    if (flags & Style_NoChange) {
		QBrush b = p->brush();
		QColor c = p->backgroundColor();
		p->setBackgroundMode( TransparentMode );
		p->setBackgroundColor( green );
		fill = QBrush(cg.base(), Dense4Pattern);
		p->setBackgroundColor( c );
		p->setBrush( b );
	    } else if (flags & Style_Down)
		fill = cg.brush( QColorGroup::Button );
	    else if (flags & Style_Enabled)
		fill = cg.brush( QColorGroup::Base );
	    else
		fill = cg.brush( QColorGroup::Background );

	    qDrawWinPanel( p, r, cg, TRUE, &fill );

	    if (flags & Style_NoChange )
		p->setPen( cg.dark() );
	    else
		p->setPen( cg.text() );
	} // FALLTHROUGH
    case PE_CheckListIndicator:
	if ( pe == PE_CheckListIndicator ) { //since we fall through from PE_Indicator
	    if ( flags & Style_Enabled )
		p->setPen( QPen( cg.text(), 1 ) );
	    else
		p->setPen( QPen( cg.dark(), 1 ) );
	    if ( flags & Style_NoChange )
		p->setBrush( cg.brush( QColorGroup::Button ) );
	    p->drawRect( r.x()+1, r.y()+1, 11, 11 );
	}
	if (! (flags & Style_Off)) {
	    QPointArray a( 7*2 );
	    int i, xx, yy;
	    xx = rr.x() + 3;
	    yy = rr.y() + 5;

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
	break;

    case PE_ExclusiveIndicator:
	{
#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)
	    static const QCOORD pts1[] = {              // dark lines
		1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
	    static const QCOORD pts2[] = {              // black lines
		2,8, 1,7, 1,4, 2,3, 2,2, 3,2, 4,1, 7,1, 8,2, 9,2 };
	    static const QCOORD pts3[] = {              // background lines
		2,9, 3,9, 4,10, 7,10, 8,9, 9,9, 9,8, 10,7, 10,4, 9,3 };
	    static const QCOORD pts4[] = {              // white lines
		2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
		11,4, 10,3, 10,2 };
	    static const QCOORD pts5[] = {              // inner fill
		4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };

	    // make sure the indicator is square
	    QRect ir = r;

	    if (r.width() < r.height()) {
		ir.setTop(r.top() + (r.height() - r.width()) / 2);
		ir.setHeight(r.width());
	    } else if (r.height() < r.width()) {
		ir.setLeft(r.left() + (r.width() - r.height()) / 2);
		ir.setWidth(r.height());
	    }

	    p->eraseRect(ir);
	    bool down = flags & Style_Down;
	    bool enabled = flags & Style_Enabled;
	    bool on = flags & Style_On;
	    QPointArray a;
	    a.setPoints( QCOORDARRLEN(pts1), pts1 );
	    a.translate( ir.x(), ir.y() );
	    p->setPen( cg.dark() );
	    p->drawPolyline( a );
	    a.setPoints( QCOORDARRLEN(pts2), pts2 );
	    a.translate( ir.x(), ir.y() );
	    p->setPen( cg.shadow() );
	    p->drawPolyline( a );
	    a.setPoints( QCOORDARRLEN(pts3), pts3 );
	    a.translate( ir.x(), ir.y() );
	    p->setPen( cg.midlight() );
	    p->drawPolyline( a );
	    a.setPoints( QCOORDARRLEN(pts4), pts4 );
	    a.translate( ir.x(), ir.y() );
	    p->setPen( cg.light() );
	    p->drawPolyline( a );
	    a.setPoints( QCOORDARRLEN(pts5), pts5 );
	    a.translate( ir.x(), ir.y() );
	    QColor fillColor = ( down || !enabled ) ? cg.button() : cg.base();
	    p->setPen( fillColor );
	    p->setBrush( fillColor  ) ;
	    p->drawPolygon( a );
	    if ( on ) {
		p->setPen( NoPen );
		p->setBrush( cg.text() );
		p->drawRect( ir.x() + 5, ir.y() + 4, 2, 4 );
		p->drawRect( ir.x() + 4, ir.y() + 5, 4, 2 );
	    }
	    break;
	}

    case PE_Panel:
    case PE_PanelPopup:
	{
	    int lw = opt.isDefault() ? pixelMetric(PM_DefaultFrameWidth)
			: opt.lineWidth();

	    if (lw == 2) {
		QColorGroup popupCG = cg;
		if ( pe == PE_PanelPopup ) {
		    popupCG.setColor( QColorGroup::Light, cg.background() );
		    popupCG.setColor( QColorGroup::Midlight, cg.light() );
		}
		qDrawWinPanel(p, r, popupCG, flags & Style_Sunken);
	    } else {
		QCommonStyle::drawPrimitive(pe, p, r, cg, flags, opt);
	    }
	    break;
	}

    case PE_Splitter:
	{
	    QPen oldPen = p->pen();
	    p->setPen( cg.light() );
	    if ( flags & Style_Horizontal ) {
		p->drawLine( r.x() + 1, r.y(), r.x() + 1, r.height() );
		p->setPen( cg.dark() );
		p->drawLine( r.x(), r.y(), r.x(), r.height() );
		p->drawLine( r.right()-1, r.y(), r.right()-1, r.height() );
		p->setPen( cg.shadow() );
		p->drawLine( r.right(), r.y(), r.right(), r.height() );
	    } else {
		p->drawLine( r.x(), r.y() + 1, r.width(), r.y() + 1 );
		p->setPen( cg.dark() );
		p->drawLine( r.x(), r.bottom() - 1, r.width(), r.bottom() - 1 );
		p->setPen( cg.shadow() );
		p->drawLine( r.x(), r.bottom(), r.width(), r.bottom() );
	    }
	    p->setPen( oldPen );
	    break;
	}
    case PE_DockWindowResizeHandle:
	{
	    QPen oldPen = p->pen();
	    p->setPen( cg.light() );
	    if ( flags & Style_Horizontal ) {
		p->drawLine( r.x(), r.y(), r.width(), r.y() );
		p->setPen( cg.dark() );
		p->drawLine( r.x(), r.bottom() - 1, r.width(), r.bottom() - 1 );
		p->setPen( cg.shadow() );
		p->drawLine( r.x(), r.bottom(), r.width(), r.bottom() );
	    } else {
		p->drawLine( r.x(), r.y(), r.x(), r.height() );
		p->setPen( cg.dark() );
		p->drawLine( r.right()-1, r.y(), r.right()-1, r.height() );
		p->setPen( cg.shadow() );
		p->drawLine( r.right(), r.y(), r.right(), r.height() );
	    }
	    p->setPen( oldPen );
	    break;
	}

    case PE_ScrollBarSubLine:
	if (use2000style) {
	    if (flags & Style_Down) {
		p->setPen( cg.dark() );
		p->setBrush( cg.brush( QColorGroup::Button ) );
		p->drawRect( r );
	    } else
		drawPrimitive(PE_ButtonBevel, p, r, cg, flags | Style_Raised);
	} else
	    drawPrimitive(PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
			  ((flags & Style_Down) ? Style_Down : Style_Raised));

	drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowLeft : PE_ArrowUp),
		      p, r, cg, flags);
	break;

    case PE_ScrollBarAddLine:
	if (use2000style) {
	    if (flags & Style_Down) {
		p->setPen( cg.dark() );
		p->setBrush( cg.brush( QColorGroup::Button ) );
		p->drawRect( r );
	    } else
		drawPrimitive(PE_ButtonBevel, p, r, cg, flags | Style_Raised);
	} else
	    drawPrimitive(PE_ButtonBevel, p, r, cg, (flags & Style_Enabled) |
			  ((flags & Style_Down) ? Style_Down : Style_Raised));

	drawPrimitive(((flags & Style_Horizontal) ? PE_ArrowRight : PE_ArrowDown),
		      p, r, cg, flags);
	break;

    case PE_ScrollBarAddPage:
    case PE_ScrollBarSubPage:
	{
	    QBrush br;
	    QColor c = p->backgroundColor();

	    p->setPen(NoPen);
	    p->setBackgroundMode(OpaqueMode);

	    if (flags & Style_Down) {
		br = QBrush(cg.shadow(), Dense4Pattern);
		p->setBackgroundColor( cg.dark() );
		p->setBrush( QBrush(cg.shadow(), Dense4Pattern) );
	    } else {
		br = (cg.brush(QColorGroup::Light).pixmap() ?
		      cg.brush(QColorGroup::Light) :
		      QBrush(cg.light(), Dense4Pattern));
		p->setBrush(br);
	    }

	    p->drawRect(r);
	    p->setBackgroundColor(c);
	    break;
	}

    case PE_ScrollBarSlider:
	if (! (flags & Style_Enabled)) {
	    QBrush br = (cg.brush(QColorGroup::Light).pixmap() ?
			 cg.brush(QColorGroup::Light) :
			 QBrush(cg.light(), Dense4Pattern));
	    p->setPen(NoPen);
	    p->setBrush(br);
	    p->setBackgroundMode(OpaqueMode);
	    p->drawRect(r);
	} else
	    drawPrimitive(PE_ButtonBevel, p, r, cg, Style_Enabled | Style_Raised);
	break;

    case PE_WindowFrame:
	{
	    QColorGroup popupCG = cg;
	    popupCG.setColor( QColorGroup::Light, cg.background() );
	    popupCG.setColor( QColorGroup::Midlight, cg.light() );
	    qDrawWinPanel(p, r, popupCG, flags & Style_Sunken);
	}
	break;

    default:
	if (pe >= PE_ArrowUp && pe <= PE_ArrowLeft) {
	    QPointArray a;

	    switch ( pe ) {
	    case PE_ArrowUp:
		a.setPoints( 7, -4,1, 2,1, -3,0, 1,0, -2,-1, 0,-1, -1,-2 );
		break;

	    case PE_ArrowDown:
		a.setPoints( 7, -4,-2, 2,-2, -3,-1, 1,-1, -2,0, 0,0, -1,1 );
		break;

	    case PE_ArrowRight:
		a.setPoints( 7, -2,-3, -2,3, -1,-2, -1,2, 0,-1, 0,1, 1,0 );
		break;

	    case PE_ArrowLeft:
		a.setPoints( 7, 0,-3, 0,3, -1,-2, -1,2, -2,-1, -2,1, -3,0 );
		break;

	    default:
		break;
	    }

	    if (a.isNull())
		return;

	    p->save();
	    if ( flags & Style_Down )
		p->translate( pixelMetric( PM_ButtonShiftHorizontal ),
			      pixelMetric( PM_ButtonShiftVertical ) );

	    if ( flags & Style_Enabled ) {
		a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
		p->setPen( cg.buttonText() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    } else {
		a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
		p->setPen( cg.light() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
		a.translate( -1, -1 );
		p->setPen( cg.mid() );
		p->drawLineSegments( a, 0, 3 );         // draw arrow
		p->drawPoint( a[6] );
	    }
	    p->restore();
	} else
	    QCommonStyle::drawPrimitive(pe, p, r, cg, flags, opt);
    }
}


/*!
  \reimp
*/
void QWindowsStyle::drawControl( ControlElement element,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QColorGroup &cg,
				 SFlags flags,
				 const QStyleOption& opt ) const
{
    switch (element) {
#ifndef QT_NO_TABBAR
    case CE_TabBarTab:
	{
	    if ( !widget || !widget->parentWidget() || !opt.tab() )
		break;

	    const QTabBar * tb = (const QTabBar *) widget;
	    const QTab * t = opt.tab();
	    bool selected = flags & Style_Selected;
	    bool lastTab = (tb->indexOf( t->identifier() ) == tb->count()-1) ?
			   TRUE : FALSE;
	    QRect r2( r );
	    if ( tb->shape()  == QTabBar::RoundedAbove ) {
		p->setPen( cg.midlight() );

		p->drawLine( r2.left(), r2.bottom(), r2.right(), r2.bottom() );
		p->setPen( cg.light() );
		p->drawLine( r2.left(), r2.bottom()-1, r2.right(), r2.bottom()-1 );
		if ( r2.left() == 0 )
		    p->drawPoint( tb->rect().bottomLeft() );

		if ( selected ) {
		    p->fillRect( QRect( r2.left()+1, r2.bottom()-1, r2.width()-3, 2),
				 cg.brush( QColorGroup::Background ));
		    p->setPen( cg.background() );
		    p->drawLine( r2.left()+1, r2.bottom(), r2.left()+1, r2.top()+2 );
		    p->setPen( cg.light() );
		} else {
		    p->setPen( cg.light() );
		    r2.setRect( r2.left() + 2, r2.top() + 2,
			       r2.width() - 4, r2.height() - 2 );
		}

		int x1, x2;
		x1 = r2.left();
		x2 = r2.right() - 2;
		p->drawLine( x1, r2.bottom()-1, x1, r2.top() + 2 );
		x1++;
		p->drawPoint( x1, r2.top() + 1 );
		x1++;
		p->drawLine( x1, r2.top(), x2, r2.top() );
		if ( r2.left() > 0 ) {
		    p->setPen( cg.midlight() );
		}
		x1 = r2.left();
		p->drawPoint( x1, r2.bottom());

		p->setPen( cg.midlight() );
		x1++;
		p->drawLine( x1, r2.bottom(), x1, r2.top() + 2 );
		x1++;
		p->drawLine( x1, r2.top()+1, x2, r2.top()+1 );

		p->setPen( cg.dark() );
		x2 = r2.right() - 1;
		p->drawLine( x2, r2.top() + 2, x2, r2.bottom() - 1 +
			     (selected ? 1:-1) );
		p->setPen( cg.shadow() );
		p->drawPoint( x2, r2.top() + 1 );
		p->drawPoint( x2, r2.top() + 1 );
		x2++;
		p->drawLine( x2, r2.top() + 2, x2, r2.bottom() -
			     (selected ? (lastTab ? 0:1) :2));
	    } else if ( tb->shape() == QTabBar::RoundedBelow ) {
		bool rightAligned = styleHint( SH_TabBar_Alignment, tb ) == AlignRight;
		bool firstTab = tb->indexOf( t->identifier() ) == 0;
		if ( selected ) {
		    p->fillRect( QRect( r2.left()+1, r2.top(), r2.width()-3, 1),
				 cg.brush( QColorGroup::Background ));
		    p->setPen( cg.background() );
		    p->drawLine( r2.left()+1, r2.top(), r2.left()+1, r2.bottom()-2 );
		    p->setPen( cg.dark() );
		} else {
		    p->setPen( cg.shadow() );
		    p->drawLine( r2.left() +
				 (rightAligned && firstTab ? 0 : 1),
				 r2.top() + 1,
				 r2.right() - (lastTab ? 0 : 2),
				 r2.top() + 1 );

		    if ( rightAligned && lastTab )
			p->drawPoint( r2.right(), r2.top() );
		    p->setPen( cg.dark() );
		    p->drawLine( r2.left(), r2.top(), r2.right() - 1,
				 r2.top() );
		    r2.setRect( r2.left() + 2, r2.top(),
				r2.width() - 4, r2.height() - 2 );
		}

		p->drawLine( r2.right() - 1, r2.top() + (selected ? 0: 2),
			     r2.right() - 1, r2.bottom() - 2 );
		p->drawPoint( r2.right() - 2, r2.bottom() - 2 );
		p->drawLine( r2.right() - 2, r2.bottom() - 1,
			     r2.left() + 1, r2.bottom() - 1 );

		p->setPen( cg.midlight() );
		p->drawLine( r2.left() + 1, r2.bottom() - 2,
			     r2.left() + 1, r2.top() + (selected ? 0 : 2) );

		p->setPen( cg.shadow() );
		p->drawLine( r2.right(),
			     r2.top() + (lastTab && rightAligned &&
					 selected) ? 0 : 1,
			     r2.right(), r2.bottom() - 1 );
		p->drawPoint( r2.right() - 1, r2.bottom() - 1 );
		p->drawLine( r2.right() - 1, r2.bottom(),
			     r2.left() + 2, r2.bottom() );

		p->setPen( cg.light() );
		p->drawLine( r2.left(), r2.top() + (selected ? 0 : 2),
			     r2.left(), r2.bottom() - 2 );
	    } else {
		QCommonStyle::drawControl(element, p, widget, r, cg, flags, opt);
	    }
	    break;
	}
#endif // QT_NO_TABBAR
    case CE_ToolBoxTab:
	{
	    qDrawShadePanel( p, r, cg, flags & (Style_Sunken | Style_Down | Style_On) , 1,
			     &cg.brush(QColorGroup::Button));
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
	    bool dis = !(flags&Style_Enabled);
	    bool checkable = popupmenu->isCheckable();
	    bool act = flags & Style_Active;
	    int x, y, w, h;

	    r.rect(&x, &y, &w, &h);

	    if ( checkable ) {
		// space for the checkmarks
		if (use2000style)
		    maxpmw = QMAX( maxpmw, 20 );
		else
		    maxpmw = QMAX( maxpmw, 12 );
	    }

	    int checkcol = maxpmw;

	    if ( mi && mi->isSeparator() ) {                    // draw separator
		p->setPen( cg.dark() );
		p->drawLine( x, y, x+w, y );
		p->setPen( cg.light() );
		p->drawLine( x, y+1, x+w, y+1 );
		return;
	    }

	    QBrush fill = (act ?
			   cg.brush( QColorGroup::Highlight ) :
			   cg.brush( QColorGroup::Button ));
	    p->fillRect( x, y, w, h, fill);

	    if ( !mi )
		return;

	    int xpos = x;
	    QRect vrect = visualRect( QRect( xpos, y, checkcol, h ), r );
	    int xvis = vrect.x();
	    if ( mi->isChecked() ) {
		if ( act && !dis )
		    qDrawShadePanel( p, xvis, y, checkcol, h,
				     cg, TRUE, 1, &cg.brush( QColorGroup::Button ) );
		else {
		    QBrush fill( cg.light(), Dense4Pattern );
		    // set the brush origin for the hash pattern to the x/y coordinate
		    // of the menu item's checkmark... this way, the check marks have
		    // a consistent look
		    QPoint origin = p->brushOrigin();
		    p->setBrushOrigin( xvis, y );
		    qDrawShadePanel( p, xvis, y, checkcol, h, cg, TRUE, 1,
				     &fill );
		    // restore the previous brush origin
		    p->setBrushOrigin( origin );
		}
	    } else if (! act)
		p->fillRect(xvis, y, checkcol , h, cg.brush( QColorGroup::Button ));

	    if ( mi->iconSet() ) {              // draw iconset
		QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
		if (act && !dis )
		    mode = QIconSet::Active;
		QPixmap pixmap;
		if ( checkable && mi->isChecked() )
		    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
		else
		    pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
		int pixw = pixmap.width();
		int pixh = pixmap.height();
		if ( act && !dis && !mi->isChecked() )
		    qDrawShadePanel( p, xvis, y, checkcol, h, cg, FALSE, 1,
				     &cg.brush( QColorGroup::Button ) );
		QRect pmr( 0, 0, pixw, pixh );
		pmr.moveCenter( vrect.center() );
		p->setPen( cg.text() );
		p->drawPixmap( pmr.topLeft(), pixmap );

		fill = (act ?
			cg.brush( QColorGroup::Highlight ) :
			cg.brush( QColorGroup::Button ));
		int xp = xpos + checkcol + 1;
		p->fillRect( visualRect( QRect( xp, y, w - checkcol - 1, h ), r ), fill);
	    } else  if ( checkable ) {  // just "checking"...
		if ( mi->isChecked() ) {
		    int xp = xpos + windowsItemFrame;

		    SFlags cflags = Style_Default;
		    if (! dis)
			cflags |= Style_Enabled;
		    if (act)
			cflags |= Style_On;

		    drawPrimitive(PE_CheckMark, p,
				  visualRect( QRect(xp, y + windowsItemFrame,
					checkcol - 2*windowsItemFrame,
					h - 2*windowsItemFrame), r ), cg, cflags);
		}
	    }

	    p->setPen( act ? cg.highlightedText() : cg.buttonText() );

	    QColor discol;
	    if ( dis ) {
		discol = cg.text();
		p->setPen( discol );
	    }

	    int xm = windowsItemFrame + checkcol + windowsItemHMargin;
	    xpos += xm;

	    vrect = visualRect( QRect( xpos, y+windowsItemVMargin, w-xm-tab+1, h-2*windowsItemVMargin ), r );
	    xvis = vrect.x();
	    if ( mi->custom() ) {
		p->save();
		if ( dis && !act ) {
		    p->setPen( cg.light() );
		    mi->custom()->paint( p, cg, act, !dis,
					 xvis+1, y+windowsItemVMargin+1, w-xm-tab+1, h-2*windowsItemVMargin );
		    p->setPen( discol );
		}
		mi->custom()->paint( p, cg, act, !dis,
				     xvis, y+windowsItemVMargin, w-xm-tab+1, h-2*windowsItemVMargin );
		p->restore();
	    }
	    QString s = mi->text();
	    if ( !s.isNull() ) {                        // draw text
		int t = s.find( '\t' );
		int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
		if (!styleHint(SH_UnderlineAccelerator, widget))
		    text_flags |= NoAccel;
		text_flags |= (QApplication::reverseLayout() ? AlignRight : AlignLeft );
		if ( t >= 0 ) {                         // draw tab text
		    int xp = x + w - tab - windowsItemHMargin - windowsItemFrame + 1;
		    if ( use2000style )
			xp -= 20;
		    else
			xp -= windowsRightBorder;
		    int xoff = visualRect( QRect( xp, y+windowsItemVMargin, tab, h-2*windowsItemVMargin ), r ).x();
		    if ( dis && !act ) {
			p->setPen( cg.light() );
			p->drawText( xoff+1, y+windowsItemVMargin+1, tab, h-2*windowsItemVMargin, text_flags, s.mid( t+1 ));
			p->setPen( discol );
		    }
		    p->drawText( xoff, y+windowsItemVMargin, tab, h-2*windowsItemVMargin, text_flags, s.mid( t+1 ) );
		    s = s.left( t );
		}
		if ( dis && !act ) {
		    p->setPen( cg.light() );
		    p->drawText( xvis+1, y+windowsItemVMargin+1, w-xm-tab+1, h-2*windowsItemVMargin, text_flags, s, t );
		    p->setPen( discol );
		}
		p->drawText( xvis, y+windowsItemVMargin, w-xm-tab+1, h-2*windowsItemVMargin, text_flags, s, t );
	    } else if ( mi->pixmap() ) {                        // draw pixmap
		QPixmap *pixmap = mi->pixmap();
		if ( pixmap->depth() == 1 )
		    p->setBackgroundMode( OpaqueMode );
		p->drawPixmap( xvis, y+windowsItemFrame, *pixmap );
		if ( pixmap->depth() == 1 )
		    p->setBackgroundMode( TransparentMode );
	    }
	    if ( mi->popup() ) {                        // draw sub menu arrow
		int dim = (h-2*windowsItemFrame) / 2;
		PrimitiveElement arrow;
		arrow = ( QApplication::reverseLayout() ? PE_ArrowLeft : PE_ArrowRight );
		xpos = x+w - windowsArrowHMargin - windowsItemFrame - dim;
		vrect = visualRect( QRect(xpos, y + h / 2 - dim / 2, dim, dim), r );
		if ( act ) {
		    QColorGroup g2 = cg;
		    g2.setColor( QColorGroup::ButtonText, g2.highlightedText() );
		    drawPrimitive(arrow, p, vrect,
				  g2, dis ? Style_Default : Style_Enabled);
		} else {
		    drawPrimitive(arrow, p, vrect,
				  cg, dis ? Style_Default : Style_Enabled );
		}
	    }

	    break;
	}
#endif

    case CE_MenuBarItem:
	{
	    bool active = flags & Style_Active;
	    bool hasFocus = flags & Style_HasFocus;
	    bool down = flags & Style_Down;
	    QRect pr = r;

	    p->fillRect( r, cg.brush( QColorGroup::Button ) );
	    if ( active || hasFocus ) {
		QBrush b = cg.brush( QColorGroup::Button );
		if ( active && down )
		    p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
		if ( active && hasFocus )
		    qDrawShadeRect( p, r.x(), r.y(), r.width(), r.height(),
				    cg, active && down, 1, 0, &b );
		if ( active && down ) {
		    pr.moveBy( pixelMetric(PM_ButtonShiftHorizontal, widget),
			       pixelMetric(PM_ButtonShiftVertical, widget) );
		    p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
		}
	    }
	    QCommonStyle::drawControl(element, p, widget, pr, cg, flags, opt);
	    break;
	}

    default:
	QCommonStyle::drawControl(element, p, widget, r, cg, flags, opt);
    }
}


/*!
  \reimp
*/
int QWindowsStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    int ret;

    switch (metric) {
    case PM_ButtonDefaultIndicator:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 1;
	break;

    case PM_MaximumDragDistance:
	ret = 60;
	break;

#ifndef QT_NO_SLIDER
    case PM_SliderLength:
	ret = 11;
	break;

	// Returns the number of pixels to use for the business part of the
	// slider (i.e., the non-tickmark portion). The remaining space is shared
	// equally between the tickmark regions.
    case PM_SliderControlThickness:
	{
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
	    if ( ticks != QSlider::Both && ticks != QSlider::NoMarks )
		thick += pixelMetric( PM_SliderLength, sl ) / 4;

	    space -= thick;
	    //### the two sides may be unequal in size
	    if ( space > 0 )
		thick += (space * 2) / (n + 2);
	    ret = thick;
	    break;
	}
#endif // QT_NO_SLIDER

    case PM_MenuBarFrameWidth:
	ret = 0;
	break;

#if defined(Q_WS_WIN)
    case PM_TitleBarHeight:
	if ( widget && ( widget->testWFlags( WStyle_Tool ) || ::qt_cast<QDockWindow*>(widget) ) ) {
	    // MS always use one less than they say
#if defined(Q_OS_TEMP)
	    ret = GetSystemMetrics( SM_CYCAPTION ) - 1;
#else
	    ret = GetSystemMetrics( SM_CYSMCAPTION ) - 1;
#endif
	} else {
	    ret = GetSystemMetrics( SM_CYCAPTION ) - 1;
	}
	break;

    case PM_ScrollBarExtent:
	{
#ifndef Q_OS_TEMP
	    NONCLIENTMETRICS ncm;
	    ncm.cbSize = sizeof(NONCLIENTMETRICS);
	    if ( SystemParametersInfo( SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0 ) )
		ret = QMAX( ncm.iScrollHeight, ncm.iScrollWidth );
	    else
#endif
		ret = QCommonStyle::pixelMetric( metric, widget );
	}
	break;
#endif

    case PM_SplitterWidth:
	ret = QMAX( 6, QApplication::globalStrut().width() );
	break;

#if defined(Q_WS_WIN)
    case PM_MDIFrameWidth:
	ret = GetSystemMetrics(SM_CYFRAME);
	break;
#endif

    default:
	ret = QCommonStyle::pixelMetric(metric, widget);
	break;
    }

    return ret;
}


/*!
  \reimp
*/
QSize QWindowsStyle::sizeFromContents( ContentsType contents,
				       const QWidget *widget,
				       const QSize &contentsSize,
				       const QStyleOption& opt ) const
{
    QSize sz(contentsSize);

    switch (contents) {
    case CT_PushButton:
	{
#ifndef QT_NO_PUSHBUTTON
	    const QPushButton *button = (const QPushButton *) widget;
	    sz = QCommonStyle::sizeFromContents(contents, widget, contentsSize, opt);
	    int w = sz.width(), h = sz.height();

	    int defwidth = 0;
	    if (button->isDefault() || button->autoDefault())
		defwidth = 2*pixelMetric( PM_ButtonDefaultIndicator, widget );

	    if (w < 80+defwidth && !button->pixmap())
		w = 80+defwidth;
	    if (h < 23+defwidth)
		h = 23+defwidth;

	    sz = QSize(w, h);
#endif
	    break;
	}

    case CT_PopupMenuItem:
	{
#ifndef QT_NO_POPUPMENU
	    if (! widget || opt.isDefault())
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
		    h += 2*windowsItemVMargin + 2*windowsItemFrame;
	    } else if ( mi->widget() ) {
	    } else if (mi->isSeparator()) {
		w = 10; // arbitrary
		h = windowsSepHeight;
	    } else {
		if (mi->pixmap())
		    h = QMAX(h, mi->pixmap()->height() + 2*windowsItemFrame);
		else if (! mi->text().isNull())
		    h = QMAX(h, popup->fontMetrics().height() + 2*windowsItemVMargin +
			     2*windowsItemFrame);

		if (mi->iconSet() != 0)
		    h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						      QIconSet::Normal).height() +
			     2*windowsItemFrame);
	    }

	    if (! mi->text().isNull() && mi->text().find('\t') >= 0) {
		if ( use2000style )
		    w += 20;
		else
		    w += windowsTabSpacing;
	    } else if (mi->popup()) {
		w += 2*windowsArrowHMargin;
	    }

	    if (use2000style) {
		if (checkable && maxpmw < 20)
		    w += 20 - maxpmw;
	    } else {
		if (checkable && maxpmw < windowsCheckMarkWidth)
		    w += windowsCheckMarkWidth - maxpmw;
	    }
	    if (checkable || maxpmw > 0)
		w += windowsCheckMarkHMargin;
	    if (use2000style)
		w += 20;
	    else
		w += windowsRightBorder;

	    sz = QSize(w, h);
#endif
	    break;
	}

    default:
	sz = QCommonStyle::sizeFromContents(contents, widget, sz, opt);
	break;
    }

    return sz;
}

/*! \reimp
*/
void QWindowsStyle::polishPopupMenu( QPopupMenu* p)
{
#ifndef QT_NO_POPUPMENU
    if ( !p->testWState( WState_Polished ) )
        p->setCheckable( TRUE );
#endif
}

#ifndef QT_NO_IMAGEIO_XPM
static const char * const qt_close_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"..##....##..",
"...##..##...",
"....####....",
".....##.....",
"....####....",
"...##..##...",
"..##....##..",
"............",
"............",
"............"};

static const char * const qt_maximize_xpm[]={
"12 12 2 1",
"# c #000000",
". c None",
"............",
".#########..",
".#########..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#########..",
"............",
"............"};


static const char * const qt_minimize_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"..######....",
"..######....",
"............",
"............"};

static const char * const qt_normalizeup_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"....######..",
"....######..",
"....#....#..",
"..######.#..",
"..######.#..",
"..#....###..",
"..#....#....",
"..#....#....",
"..######....",
"............",
"............"};


static const char * const qt_shade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
".....#......",
"....###.....",
"...#####....",
"..#######...",
"............",
"............",
"............"};

static const char * const qt_unshade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"..#######...",
"...#####....",
"....###.....",
".....#......",
"............",
"............",
"............",
"............"};

static const char * dock_window_close_xpm[] = {
"8 8 2 1",
"# c #000000",
". c None",
"........",
".##..##.",
"..####..",
"...##...",
"..####..",
".##..##.",
"........",
"........"};

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
#endif //QT_NO_IMAGEIO_XPM

/*!
 \reimp
 */
QPixmap QWindowsStyle::stylePixmap(StylePixmap stylepixmap,
				   const QWidget *widget,
				   const QStyleOption& opt) const
{
#ifndef QT_NO_IMAGEIO_XPM
    switch (stylepixmap) {
    case SP_TitleBarShadeButton:
	return QPixmap( (const char **)qt_shade_xpm );
    case SP_TitleBarUnshadeButton:
	return QPixmap( (const char **)qt_unshade_xpm );
    case SP_TitleBarNormalButton:
	return QPixmap( (const char **)qt_normalizeup_xpm );
    case SP_TitleBarMinButton:
	return QPixmap( (const char **)qt_minimize_xpm );
    case SP_TitleBarMaxButton:
	return QPixmap( (const char **)qt_maximize_xpm );
    case SP_TitleBarCloseButton:
	return QPixmap( (const char **)qt_close_xpm );
    case SP_DockWindowCloseButton:
	return QPixmap( (const char **)dock_window_close_xpm );
    case SP_MessageBoxInformation:
	return QPixmap( (const char **)information_xpm);
    case SP_MessageBoxWarning:
	return QPixmap( (const char **)warning_xpm );
    case SP_MessageBoxCritical:
	return QPixmap( (const char **)critical_xpm );
    case SP_MessageBoxQuestion:
	return QPixmap( (const char **)question_xpm );
    default:
	break;
    }
#endif //QT_NO_IMAGEIO_XPM
    return QCommonStyle::stylePixmap(stylepixmap, widget, opt);
}

/*!\reimp
*/
void QWindowsStyle::drawComplexControl( ComplexControl ctrl, QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags,
					SCFlags sub,
					SCFlags subActive,
					const QStyleOption& opt ) const
{
    switch (ctrl) {
#ifndef QT_NO_LISTVIEW
    case CC_ListView:
	{
	    if ( sub & SC_ListView ) {
		QCommonStyle::drawComplexControl( ctrl, p, widget, r, cg, flags, sub, subActive, opt );
	    }
	    if ( sub & ( SC_ListViewBranch | SC_ListViewExpand ) ) {
		if (opt.isDefault())
		    break;

		QListViewItem *item = opt.listViewItem(),
			     *child = item->firstChild();

		int y = r.y();
		int c;
		int dotoffset = 0;
		QPointArray dotlines;
		if ( subActive == (uint)SC_All && sub == SC_ListViewExpand ) {
		    c = 2;
		    dotlines.resize(2);
		    dotlines[0] = QPoint( r.right(), r.top() );
		    dotlines[1] = QPoint( r.right(), r.bottom() );
		} else {
		    int linetop = 0, linebot = 0;
		    // each branch needs at most two lines, ie. four end points
		    dotoffset = (item->itemPos() + item->height() - y) %2;
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
				p->setPen( cg.mid() );
				p->drawRect( bx-4, linebot-4, 9, 9 );
				// plus or minus
				p->setPen( cg.text() );
				p->drawLine( bx - 2, linebot, bx + 2, linebot );
				if ( !child->isOpen() )
				    p->drawLine( bx, linebot - 2, bx, linebot + 2 );
				// dotlinery
				p->setPen( cg.mid() );
				dotlines[c++] = QPoint( bx, linetop );
				dotlines[c++] = QPoint( bx, linebot - 4 );
				dotlines[c++] = QPoint( bx + 5, linebot );
				dotlines[c++] = QPoint( r.width(), linebot );
				linetop = linebot + 5;
			    } else {
				// just dotlinery
				dotlines[c++] = QPoint( bx+1, linebot -1);
				dotlines[c++] = QPoint( r.width(), linebot -1);
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
		p->setPen( cg.text() );

		static QBitmap *verticalLine = 0, *horizontalLine = 0;
		static QCleanupHandler<QBitmap> qlv_cleanup_bitmap;
		if ( !verticalLine ) {
		    // make 128*1 and 1*128 bitmaps that can be used for
		    // drawing the right sort of lines.
		    verticalLine = new QBitmap( 1, 129, TRUE );
		    horizontalLine = new QBitmap( 128, 1, TRUE );
		    QPointArray a( 64 );
		    QPainter p;
		    p.begin( verticalLine );
		    int i;
		    for( i=0; i<64; i++ )
			a.setPoint( i, 0, i*2+1 );
		    p.setPen( color1 );
		    p.drawPoints( a );
		    p.end();
		    QApplication::flushX();
		    verticalLine->setMask( *verticalLine );
		    p.begin( horizontalLine );
		    for( i=0; i<64; i++ )
			a.setPoint( i, i*2+1, 0 );
		    p.setPen( color1 );
		    p.drawPoints( a );
		    p.end();
		    QApplication::flushX();
		    horizontalLine->setMask( *horizontalLine );
		    qlv_cleanup_bitmap.add( &verticalLine );
		    qlv_cleanup_bitmap.add( &horizontalLine );
		}

		int line; // index into dotlines
		if ( sub & SC_ListViewBranch ) for( line = 0; line < c; line += 2 ) {
		    // assumptions here: lines are horizontal or vertical.
		    // lines always start with the numerically lowest
		    // coordinate.

		    // point ... relevant coordinate of current point
		    // end ..... same coordinate of the end of the current line
		    // other ... the other coordinate of the current point/line
		    if ( dotlines[line].y() == dotlines[line+1].y() ) {
			int end = dotlines[line+1].x();
			int point = dotlines[line].x();
			int other = dotlines[line].y();
			while( point < end ) {
			    int i = 128;
			    if ( i+point > end )
				i = end-point;
			    p->drawPixmap( point, other, *horizontalLine,
					   0, 0, i, 1 );
			    point += i;
			}
		    } else {
			int end = dotlines[line+1].y();
			int point = dotlines[line].y();
			int other = dotlines[line].x();
			int pixmapoffset = ((point & 1) != dotoffset ) ? 1 : 0;
			while( point < end ) {
			    int i = 128;
			    if ( i+point > end )
				i = end-point;
			    p->drawPixmap( other, point, *verticalLine,
					   0, pixmapoffset, 1, i );
			    point += i;
			}
		    }
		}
	    }
	}
	break;
#endif //QT_NO_LISTVIEW

#ifndef QT_NO_COMBOBOX
    case CC_ComboBox:
	if ( sub & SC_ComboBoxArrow ) {
	    SFlags flags = Style_Default;

	    qDrawWinPanel( p, r, cg, TRUE, widget->isEnabled() ?
			   &cg.brush( QColorGroup::Base ):
			   &cg.brush( QColorGroup::Background ) );

	    QRect ar =
		QStyle::visualRect( querySubControlMetrics( CC_ComboBox, widget,
							    SC_ComboBoxArrow ), widget );
	    if ( subActive == SC_ComboBoxArrow ) {
		p->setPen( cg.dark() );
		p->setBrush( cg.brush( QColorGroup::Button ) );
		p->drawRect( ar );
	    } else
		qDrawWinPanel( p, ar, cg, FALSE,
			       &cg.brush( QColorGroup::Button ) );

	    ar.addCoords( 2, 2, -2, -2 );
	    if ( widget->isEnabled() )
		flags |= Style_Enabled;

	    if ( subActive == SC_ComboBoxArrow ) {
		flags |= Style_Sunken;
	    }
	    drawPrimitive( PE_ArrowDown, p, ar, cg, flags );
	}

	if ( sub & SC_ComboBoxEditField ) {
	    const QComboBox * cb = (const QComboBox *) widget;
	    QRect re =
		QStyle::visualRect( querySubControlMetrics( CC_ComboBox, widget,
							    SC_ComboBoxEditField ), widget );
	    if ( cb->hasFocus() && !cb->editable() )
		p->fillRect( re.x(), re.y(), re.width(), re.height(),
			     cg.brush( QColorGroup::Highlight ) );

	    if ( cb->hasFocus() ) {
		p->setPen( cg.highlightedText() );
		p->setBackgroundColor( cg.highlight() );

	    } else {
		p->setPen( cg.text() );
		p->setBackgroundColor( cg.background() );
	    }

	    if ( cb->hasFocus() && !cb->editable() ) {
		QRect re =
		    QStyle::visualRect( subRect( SR_ComboBoxFocusRect, cb ), widget );
		drawPrimitive( PE_FocusRect, p, re, cg, Style_FocusAtBorder, QStyleOption(cg.highlight()));
	    }
	}

	break;
#endif	// QT_NO_COMBOBOX

#ifndef QT_NO_SLIDER
    case CC_Slider:
	{
	    const QSlider *sl = (const QSlider *) widget;
	    int thickness  = pixelMetric( PM_SliderControlThickness, widget );
	    int len        = pixelMetric( PM_SliderLength, widget );
	    int ticks = sl->tickmarks();

	    QRect groove = querySubControlMetrics(CC_Slider, widget, SC_SliderGroove,
						  opt),
		  handle = querySubControlMetrics(CC_Slider, widget, SC_SliderHandle,
						  opt);

	    if ((sub & SC_SliderGroove) && groove.isValid()) {
		int mid = thickness / 2;

		if ( ticks & QSlider::Above )
		    mid += len / 8;
		if ( ticks & QSlider::Below )
		    mid -= len / 8;

		p->setPen( cg.shadow() );
		if ( sl->orientation() == Horizontal ) {
		    qDrawWinPanel( p, groove.x(), groove.y() + mid - 2,
				   groove.width(), 4, cg, TRUE );
		    p->drawLine( groove.x() + 1, groove.y() + mid - 1,
				 groove.x() + groove.width() - 3, groove.y() + mid - 1 );
		} else {
		    qDrawWinPanel( p, groove.x() + mid - 2, groove.y(),
				   4, groove.height(), cg, TRUE );
		    p->drawLine( groove.x() + mid - 1, groove.y() + 1,
				 groove.x() + mid - 1,
				 groove.y() + groove.height() - 3 );
		}
	    }

	    if (sub & SC_SliderTickmarks)
		QCommonStyle::drawComplexControl(ctrl, p, widget, r, cg, flags,
						 SC_SliderTickmarks, subActive,
						 opt );

	    if ( sub & SC_SliderHandle ) {
		// 4444440
		// 4333310
		// 4322210
		// 4322210
		// 4322210
		// 4322210
		// *43210*
		// **410**
		// ***0***
		const QColor c0 = cg.shadow();
		const QColor c1 = cg.dark();
		// const QColor c2 = g.button();
		const QColor c3 = cg.midlight();
		const QColor c4 = cg.light();

		int x = handle.x(), y = handle.y(),
		   wi = handle.width(), he = handle.height();

		int x1 = x;
		int x2 = x+wi-1;
		int y1 = y;
		int y2 = y+he-1;

		Orientation orient = sl->orientation();
		bool tickAbove = sl->tickmarks() == QSlider::Above;
		bool tickBelow = sl->tickmarks() == QSlider::Below;

		p->fillRect( x, y, wi, he, cg.brush( QColorGroup::Background ) );

		if ( flags & Style_HasFocus ) {
		    QRect re = subRect( SR_SliderFocusRect, sl );
		    drawPrimitive( PE_FocusRect, p, re, cg );
		}

		if ( (tickAbove && tickBelow) || (!tickAbove && !tickBelow) ) {
		    qDrawWinButton( p, QRect(x,y,wi,he), cg, FALSE,
				    &cg.brush( QColorGroup::Button ) );
		    return;
		}

		QSliderDirection dir;

		if ( orient == Horizontal )
		    if ( tickAbove )
			dir = SlUp;
		    else
			dir = SlDown;
		else
		    if ( tickAbove )
			dir = SlLeft;
		    else
			dir = SlRight;

		QPointArray a;

		int d = 0;
		switch ( dir ) {
		case SlUp:
		    y1 = y1 + wi/2;
		    d =  (wi + 1) / 2 - 1;
		    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2,y1, x1+d,y1-d );
		    break;
		case SlDown:
		    y2 = y2 - wi/2;
		    d =  (wi + 1) / 2 - 1;
		    a.setPoints(5, x1,y1, x1,y2, x1+d,y2+d, x2,y2, x2,y1 );
		    break;
		case SlLeft:
		    d =  (he + 1) / 2 - 1;
		    x1 = x1 + he/2;
		    a.setPoints(5, x1,y1, x1-d,y1+d, x1,y2, x2,y2, x2,y1);
		    break;
		case SlRight:
		    d =  (he + 1) / 2 - 1;
		    x2 = x2 - he/2;
		    a.setPoints(5, x1,y1, x1,y2, x2,y2, x2+d,y1+d, x2,y1 );
		    break;
		}

		QBrush oldBrush = p->brush();
		p->setBrush( cg.brush( QColorGroup::Button ) );
		p->setPen( NoPen );
		p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
		p->drawPolygon( a );
		p->setBrush( oldBrush );

		if ( dir != SlUp ) {
		    p->setPen( c4 );
		    p->drawLine( x1, y1, x2, y1 );
		    p->setPen( c3 );
		    p->drawLine( x1, y1+1, x2, y1+1 );
		}
		if ( dir != SlLeft ) {
		    p->setPen( c3 );
		    p->drawLine( x1+1, y1+1, x1+1, y2 );
		    p->setPen( c4 );
		    p->drawLine( x1, y1, x1, y2 );
		}
		if ( dir != SlRight ) {
		    p->setPen( c0 );
		    p->drawLine( x2, y1, x2, y2 );
		    p->setPen( c1 );
		    p->drawLine( x2-1, y1+1, x2-1, y2-1 );
		}
		if ( dir != SlDown ) {
		    p->setPen( c0 );
		    p->drawLine( x1, y2, x2, y2 );
		    p->setPen( c1 );
		    p->drawLine( x1+1, y2-1, x2-1, y2-1 );
		}

		switch ( dir ) {
		case SlUp:
		    p->setPen( c4 );
		    p->drawLine( x1, y1, x1+d, y1-d);
		    p->setPen( c0 );
		    d = wi - d - 1;
		    p->drawLine( x2, y1, x2-d, y1-d);
		    d--;
		    p->setPen( c3 );
		    p->drawLine( x1+1, y1, x1+1+d, y1-d );
		    p->setPen( c1 );
		    p->drawLine( x2-1, y1, x2-1-d, y1-d);
		    break;
		case SlDown:
		    p->setPen( c4 );
		    p->drawLine( x1, y2, x1+d, y2+d);
		    p->setPen( c0 );
		    d = wi - d - 1;
		    p->drawLine( x2, y2, x2-d, y2+d);
		    d--;
		    p->setPen( c3 );
		    p->drawLine( x1+1, y2, x1+1+d, y2+d );
		    p->setPen( c1 );
		    p->drawLine( x2-1, y2, x2-1-d, y2+d);
		    break;
		case SlLeft:
		    p->setPen( c4 );
		    p->drawLine( x1, y1, x1-d, y1+d);
		    p->setPen( c0 );
		    d = he - d - 1;
		    p->drawLine( x1, y2, x1-d, y2-d);
		    d--;
		    p->setPen( c3 );
		    p->drawLine( x1, y1+1, x1-d, y1+1+d );
		    p->setPen( c1 );
		    p->drawLine( x1, y2-1, x1-d, y2-1-d);
		    break;
		case SlRight:
		    p->setPen( c4 );
		    p->drawLine( x2, y1, x2+d, y1+d);
		    p->setPen( c0 );
		    d = he - d - 1;
		    p->drawLine( x2, y2, x2+d, y2-d);
		    d--;
		    p->setPen( c3 );
		    p->drawLine(  x2, y1+1, x2+d, y1+1+d );
		    p->setPen( c1 );
		    p->drawLine( x2, y2-1, x2+d, y2-1-d);
		    break;
		}
	    }

	    break;
	}
#endif // QT_NO_SLIDER

    default:
	QCommonStyle::drawComplexControl( ctrl, p, widget, r, cg, flags, sub,
					  subActive, opt );
	break;
    }
}


/*! \reimp */
int QWindowsStyle::styleHint( StyleHint hint,
			      const QWidget *widget,
			      const QStyleOption &opt,
			      QStyleHintReturn *returnData ) const
{
    int ret;

    switch (hint) {
    case SH_EtchDisabledText:
    case SH_Slider_SnapToValue:
    case SH_PrintDialog_RightAlignButtons:
    case SH_MainWindow_SpaceBelowMenuBar:
    case SH_FontDialog_SelectAssociatedText:
    case SH_PopupMenu_AllowActiveAndDisabled:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_PopupMenu_MouseTracking:
    case SH_ComboBox_ListMouseTracking:
    case SH_ScrollBar_StopMouseOverSlider:
	ret = 1;
	break;

    case SH_ItemView_ChangeHighlightOnFocus:
#if defined(Q_WS_WIN)
	if ( qWinVersion() != WV_95 && qWinVersion() != WV_NT )
	    ret = 1;
	else
#endif
	    ret = 0;
	break;

    case SH_ToolBox_SelectedPageTitleBold:
	ret = 0;
	break;

#if defined(Q_WS_WIN)
    case SH_UnderlineAccelerator:
	ret = 1;
	if ( qWinVersion() != WV_95 && qWinVersion() != WV_98 && qWinVersion() != WV_NT ) {
	    BOOL cues;
	    SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &cues, 0);
	    ret = cues ? 1 : 0;
	    // Do nothing if we always paint underlines
	    if (!ret && widget && d) {
		QMenuBar *menuBar = ::qt_cast<QMenuBar*>(widget);
		QPopupMenu *popupMenu = 0;
		if (!menuBar)
		    popupMenu = ::qt_cast<QPopupMenu*>(widget);

		// If we paint a menubar draw underlines if it has focus, or if alt is down,
		// or if a popup menu belonging to the menubar is active and paints underlines
		if (menuBar) {
		    if (menuBar->hasFocus()) {
			ret = 1;
		    } else if (d->altDown()) {
			ret = 1;
		    } else if (qApp->focusWidget() && qApp->focusWidget()->isPopup()) {
			popupMenu = ::qt_cast<QPopupMenu*>(qApp->focusWidget());
			QMenuData *pm = popupMenu ? (QMenuData*)popupMenu->qt_cast("QMenuData") : 0;
			if (pm && ((FriendlyMenuData*)pm)->parentMenu == menuBar) {
			    if (d->hasSeenAlt(menuBar))
				ret = 1;
			}
		    }
		// If we paint a popup menu draw underlines if the respective menubar does
		} else if (popupMenu) {
		    QMenuData *pm = (QMenuData*)popupMenu->qt_cast("QMenuData");
		    while (pm) {
			if (((FriendlyMenuData*)pm)->isMenuBar) {
			    menuBar = (QMenuBar*)pm;
			    if (d->hasSeenAlt(menuBar))
				ret = 1;
			    break;
			}
			pm = ((FriendlyMenuData*)pm)->parentMenu;
		    }
		// Otherwise draw underlines if the toplevel widget has seen an alt-press
		} else if (d->hasSeenAlt(widget)) {
		    ret = 1;
		}
	    }

	}
	break;
#endif

    default:
	ret = QCommonStyle::styleHint(hint, widget, opt, returnData);
	break;
    }

    return ret;
}

/*! \reimp */
QRect QWindowsStyle::subRect(SubRect r, const QWidget *widget) const
{
    QRect rect;

    switch (r) {
#ifndef QT_NO_SLIDER
    case SR_SliderFocusRect:
	{
	    rect = widget->rect();
	    break;
	}
#endif // QT_NO_SLIDER
    case SR_ToolBoxTabContents:
	rect = widget->rect();
	break;
    default:
	rect = QCommonStyle::subRect( r, widget );
	break;
    }

    return rect;
}

#endif
