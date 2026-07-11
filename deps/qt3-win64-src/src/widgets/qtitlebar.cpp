/****************************************************************************
** $Id: qtitlebar.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of some Qt private functions.
**
** Created : 001101
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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

#include "qplatformdefs.h"

#include "qtitlebar_p.h"

#ifndef QT_NO_TITLEBAR

#include <qcursor.h>
#include "qapplication.h"
#include "qstyle.h"
#include "qdatetime.h"
#include "private/qapplication_p.h"
#include "qtooltip.h"
#include "qimage.h"
#include "qtimer.h"
#include "qpainter.h"
#include "qstyle.h"
#include "private/qinternal_p.h"
#ifndef QT_NO_WORKSPACE
#include "qworkspace.h"
#endif
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif

#ifndef QT_NO_TOOLTIP
class QTitleBarTip : public QToolTip
{
public:
    QTitleBarTip( QWidget * parent ) : QToolTip( parent ) { }

    void maybeTip( const QPoint &pos )
    {
	if ( !::qt_cast<QTitleBar*>(parentWidget()) )
	    return;
	QTitleBar *t = (QTitleBar *)parentWidget();

	QString tipstring;
	QStyle::SubControl ctrl = t->style().querySubControl(QStyle::CC_TitleBar, t, pos);
	QSize controlSize = t->style().querySubControlMetrics(QStyle::CC_TitleBar, t, ctrl).size();

	QWidget *window = t->window();
	if ( window ) {
	    switch(ctrl) {
	    case QStyle::SC_TitleBarSysMenu:
		if ( t->testWFlags( WStyle_SysMenu ) )
		    tipstring = QTitleBar::tr( "System Menu" );
		break;

	    case QStyle::SC_TitleBarShadeButton:
		if ( t->testWFlags( WStyle_Tool ) && t->testWFlags( WStyle_MinMax ) )
		    tipstring = QTitleBar::tr( "Shade" );
		break;

	    case QStyle::SC_TitleBarUnshadeButton:
		if ( t->testWFlags( WStyle_Tool ) && t->testWFlags( WStyle_MinMax ) )
		    tipstring = QTitleBar::tr( "Unshade" );
		break;

	    case QStyle::SC_TitleBarNormalButton:
	    case QStyle::SC_TitleBarMinButton:
		if ( !t->testWFlags( WStyle_Tool ) && t->testWFlags( WStyle_Minimize ) ) {
		    if( window->isMinimized() )
			tipstring = QTitleBar::tr( "Normalize" );
		    else
			tipstring = QTitleBar::tr( "Minimize" );
		}
		break;

	    case QStyle::SC_TitleBarMaxButton:
		if ( !t->testWFlags( WStyle_Tool ) && t->testWFlags( WStyle_Maximize ) )
		    tipstring = QTitleBar::tr( "Maximize" );
		break;

	    case QStyle::SC_TitleBarCloseButton:
		if ( t->testWFlags( WStyle_SysMenu ) )
		    tipstring = QTitleBar::tr( "Close" );
		break;

	    default:
		break;
	    }
	}
#ifndef QT_NO_WIDGET_TOPEXTRA
	if ( tipstring.isEmpty() ) {
	    if ( t->visibleText() != t->caption() )
		tipstring = t->caption();
	}
#endif
	if(!tipstring.isEmpty())
	    tip( QRect(pos, controlSize), tipstring );
    }
};
#endif

class QTitleBarPrivate
{
public:
    QTitleBarPrivate()
	: toolTip( 0 ), act( 0 ), window( 0 ), movable( 1 ), pressed( 0 ), autoraise(0)
    {
    }

    QStyle::SCFlags buttonDown;
    QPoint moveOffset;
    QToolTip *toolTip;
    bool act		    :1;
    QWidget* window;
    bool movable            :1;
    bool pressed            :1;
    bool autoraise          :1;
    QString cuttext;
#ifdef QT_NO_WIDGET_TOPEXTRA
    QString cap;
#endif
};

QTitleBar::QTitleBar(QWidget* w, QWidget* parent, const char* name)
    : QWidget( parent, name, WStyle_Customize | WStyle_NoBorder | WNoAutoErase )
{
    d = new QTitleBarPrivate();

#ifndef QT_NO_TOOLTIP
    d->toolTip = new QTitleBarTip( this );
#endif
    d->window = w;
    d->buttonDown = QStyle::SC_None;
    d->act = 0;
    if ( w ) {
	setWFlags( ((QTitleBar*)w)->getWFlags() | WNoAutoErase );
	if ( w->minimumSize() == w->maximumSize() )
	    clearWFlags( WStyle_Maximize );
#ifndef QT_NO_WIDGET_TOPEXTRA
    	setCaption( w->caption() );
#endif
    } else {
	setWFlags( WStyle_Customize | WNoAutoErase );
    }

    readColors();
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    setMouseTracking(TRUE);
}

QTitleBar::~QTitleBar()
{
#ifndef QT_NO_TOOLTIP
    delete d->toolTip;
#endif

    delete d;
    d = 0;
}

#ifdef Q_WS_WIN
extern QRgb qt_colorref2qrgb(COLORREF col);
#endif

void QTitleBar::readColors()
{
    QPalette pal = palette();

    bool colorsInitialized = FALSE;

#ifdef Q_WS_WIN // ask system properties on windows
#ifndef SPI_GETGRADIENTCAPTIONS
#define SPI_GETGRADIENTCAPTIONS 0x1008
#endif
#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION 27
#endif
#ifndef COLOR_GRADIENTINACTIVECAPTION
#define COLOR_GRADIENTINACTIVECAPTION 28
#endif
    if ( QApplication::desktopSettingsAware() ) {
	pal.setColor( QPalette::Active, QColorGroup::Highlight, qt_colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION)) );
	pal.setColor( QPalette::Inactive, QColorGroup::Highlight, qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION)) );
	pal.setColor( QPalette::Active, QColorGroup::HighlightedText, qt_colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT)) );
	pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT)) );
	if ( qt_winver != Qt::WV_95 && qt_winver != WV_NT ) {
	    colorsInitialized = TRUE;
	    BOOL gradient;
	    QT_WA( {
		SystemParametersInfo( SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0 );
	    } , {
		SystemParametersInfoA( SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0 );
	    } );
	    if ( gradient ) {
		pal.setColor( QPalette::Active, QColorGroup::Base, qt_colorref2qrgb(GetSysColor(COLOR_GRADIENTACTIVECAPTION)) );
		pal.setColor( QPalette::Inactive, QColorGroup::Base, qt_colorref2qrgb(GetSysColor(COLOR_GRADIENTINACTIVECAPTION)) );
	    } else {
		pal.setColor( QPalette::Active, QColorGroup::Base, palette().active().highlight() );
		pal.setColor( QPalette::Inactive, QColorGroup::Base, palette().inactive().highlight() );
	    }
	}
    }
#endif // Q_WS_WIN
    if ( !colorsInitialized ) {
	pal.setColor( QPalette::Active, QColorGroup::Highlight, palette().active().highlight() );
	pal.setColor( QPalette::Active, QColorGroup::Base, palette().active().highlight() );
	pal.setColor( QPalette::Inactive, QColorGroup::Highlight, palette().inactive().dark() );
	pal.setColor( QPalette::Inactive, QColorGroup::Base, palette().inactive().dark() );
	pal.setColor( QPalette::Inactive, QColorGroup::HighlightedText, palette().inactive().background() );
    }

    setPalette( pal );
    setActive( d->act );
}

void QTitleBar::mousePressEvent( QMouseEvent * e)
{
    if ( !d->act )
	emit doActivate();
    if ( e->button() == LeftButton ) {
	d->pressed = TRUE;
	QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
	switch (ctrl) {
	case QStyle::SC_TitleBarSysMenu:
	    if ( testWFlags( WStyle_SysMenu ) && !testWFlags( WStyle_Tool ) ) {
		d->buttonDown = QStyle::SC_None;
		static QTime* t = 0;
		static QTitleBar* tc = 0;
		if ( !t )
		    t = new QTime;
		if ( tc != this || t->elapsed() > QApplication::doubleClickInterval() ) {
		    emit showOperationMenu();
		    t->start();
		    tc = this;
		} else {
		    tc = 0;
		    emit doClose();
		    return;
		}
	    }
	    break;

	case QStyle::SC_TitleBarShadeButton:
	case QStyle::SC_TitleBarUnshadeButton:
	    if ( testWFlags( WStyle_MinMax ) && testWFlags( WStyle_Tool ) )
		d->buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarNormalButton:
	    if( testWFlags( WStyle_Minimize ) && !testWFlags( WStyle_Tool ) )
		d->buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarMinButton:
	    if( testWFlags( WStyle_Minimize ) && !testWFlags( WStyle_Tool ) )
		d->buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarMaxButton:
	    if ( testWFlags( WStyle_Maximize ) && !testWFlags( WStyle_Tool ) )
		d->buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarCloseButton:
	    if ( testWFlags( WStyle_SysMenu ) )
		d->buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarLabel:
	    d->buttonDown = ctrl;
	    d->moveOffset = mapToParent( e->pos() );
	    break;

	default:
	    break;
	}
	repaint( FALSE );
    } else {
	d->pressed = FALSE;
    }
}

void QTitleBar::contextMenuEvent( QContextMenuEvent *e )
{
    QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
    if( ctrl == QStyle::SC_TitleBarLabel || ctrl == QStyle::SC_TitleBarSysMenu )
	emit popupOperationMenu(e->globalPos());
    else
	e->ignore();
}

void QTitleBar::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton && d->pressed) {
	QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());

	if (ctrl == d->buttonDown) {
	    switch(ctrl) {
	    case QStyle::SC_TitleBarShadeButton:
	    case QStyle::SC_TitleBarUnshadeButton:
		if( testWFlags( WStyle_MinMax ) && testWFlags( WStyle_Tool ) )
		    emit doShade();
		break;

	    case QStyle::SC_TitleBarNormalButton:
		if( testWFlags( WStyle_MinMax ) && !testWFlags( WStyle_Tool ) )
		    emit doNormal();
		break;

	    case QStyle::SC_TitleBarMinButton:
		if( testWFlags( WStyle_Minimize ) && !testWFlags( WStyle_Tool ) )
		    emit doMinimize();
		break;

	    case QStyle::SC_TitleBarMaxButton:
		if( d->window && testWFlags( WStyle_Maximize ) && !testWFlags( WStyle_Tool ) ) {
		    if(d->window->isMaximized())
			emit doNormal();
		    else
			emit doMaximize();
		}
		break;

	    case QStyle::SC_TitleBarCloseButton:
		if( testWFlags( WStyle_SysMenu ) ) {
		    d->buttonDown = QStyle::SC_None;
		    repaint(FALSE);
		    emit doClose();
		    return;
		}
		break;

	    default:
		break;
	    }
	}
	d->buttonDown = QStyle::SC_None;
	repaint(FALSE);
	d->pressed = FALSE;
    }
}

void QTitleBar::mouseMoveEvent( QMouseEvent * e)
{
    switch (d->buttonDown) {
    case QStyle::SC_None:
	if(autoRaise())
	    repaint( FALSE );
	break;
    case QStyle::SC_TitleBarSysMenu:
	break;
    case QStyle::SC_TitleBarShadeButton:
    case QStyle::SC_TitleBarUnshadeButton:
    case QStyle::SC_TitleBarNormalButton:
    case QStyle::SC_TitleBarMinButton:
    case QStyle::SC_TitleBarMaxButton:
    case QStyle::SC_TitleBarCloseButton:
	{
	    QStyle::SCFlags last_ctrl = d->buttonDown;
	    d->buttonDown = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
	    if( d->buttonDown != last_ctrl)
		d->buttonDown = QStyle::SC_None;
	    repaint(FALSE);
	    d->buttonDown = last_ctrl;
	}
	break;

    case QStyle::SC_TitleBarLabel:
	if ( d->buttonDown == QStyle::SC_TitleBarLabel && d->movable && d->pressed ) {
	    if ( (d->moveOffset - mapToParent( e->pos() ) ).manhattanLength() >= 4 ) {
		QPoint p = mapFromGlobal(e->globalPos());
#ifndef QT_NO_WORKSPACE
		if(d->window && d->window->parentWidget()->inherits("QWorkspaceChild")) {
		    QWorkspace *workspace = ::qt_cast<QWorkspace*>(d->window->parentWidget()->parentWidget());
		    if(workspace) {
			p = workspace->mapFromGlobal( e->globalPos() );
			if ( !workspace->rect().contains(p) ) {
			    if ( p.x() < 0 )
				p.rx() = 0;
			    if ( p.y() < 0 )
				p.ry() = 0;
			    if ( p.x() > workspace->width() )
				p.rx() = workspace->width();
			    if ( p.y() > workspace->height() )
				p.ry() = workspace->height();
			}
		    }
		}
#endif
		QPoint pp = p - d->moveOffset;
		if (!parentWidget()->isMaximized())
		    parentWidget()->move( pp );
	    }
	} else {
	    QStyle::SCFlags last_ctrl = d->buttonDown;
	    d->buttonDown = QStyle::SC_None;
	    if( d->buttonDown != last_ctrl)
		repaint(FALSE);
	}
	break;
    }
}

void QTitleBar::resizeEvent( QResizeEvent *r)
{
    QWidget::resizeEvent(r);
    cutText();
}

void QTitleBar::paintEvent(QPaintEvent *)
{
    QStyle::SCFlags ctrls = QStyle::SC_TitleBarLabel;
    if ( testWFlags( WStyle_SysMenu) ) {
	if ( testWFlags( WStyle_Tool ) ) {
	    ctrls |= QStyle::SC_TitleBarCloseButton;
	    if ( d->window && testWFlags( WStyle_MinMax ) ) {
		if ( d->window->isMinimized() )
		    ctrls |= QStyle::SC_TitleBarUnshadeButton;
		else
		    ctrls |= QStyle::SC_TitleBarShadeButton;
	    }
	} else {
	    ctrls |= QStyle::SC_TitleBarSysMenu | QStyle::SC_TitleBarCloseButton;
	    if ( d->window && testWFlags( WStyle_Minimize ) ) {
		if( d->window && d->window->isMinimized() )
		    ctrls |= QStyle::SC_TitleBarNormalButton;
		else
		    ctrls |= QStyle::SC_TitleBarMinButton;
	    }
	    if ( d->window && testWFlags( WStyle_Maximize ) && !d->window->isMaximized() )
		ctrls |= QStyle::SC_TitleBarMaxButton;
	}
    }

    QStyle::SCFlags under_mouse = QStyle::SC_None;
    if( autoRaise() && hasMouse() ) {
	QPoint p(mapFromGlobal(QCursor::pos()));
	under_mouse = style().querySubControl(QStyle::CC_TitleBar, this, p);
	ctrls ^= under_mouse;
    }

    QSharedDoubleBuffer buffer( this, rect() );
    style().drawComplexControl(QStyle::CC_TitleBar, buffer.painter(), this, rect(),
			       colorGroup(),
			       isEnabled() ? QStyle::Style_Enabled :
			       QStyle::Style_Default, ctrls, d->buttonDown);
    if(under_mouse != QStyle::SC_None)
	style().drawComplexControl(QStyle::CC_TitleBar, buffer.painter(), this, rect(),
				   colorGroup(),
				   QStyle::Style_MouseOver |
				   (isEnabled() ? QStyle::Style_Enabled : 0),
				   under_mouse, d->buttonDown);
}

void QTitleBar::mouseDoubleClickEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;

    switch(style().querySubControl(QStyle::CC_TitleBar, this, e->pos())) {
    case QStyle::SC_TitleBarLabel:
	emit doubleClicked();
	break;

    case QStyle::SC_TitleBarSysMenu:
	if ( testWFlags( WStyle_SysMenu ) )
	    emit doClose();
	break;

    default:
	break;
    }
}

#ifdef QT_NO_WIDGET_TOPEXTRA
// We provide one, since titlebar is useless otherwise.
QString QTitleBar::caption() const
{
    return d->cap;
}
#endif

void QTitleBar::cutText()
{
    QFontMetrics fm( font() );

    int maxw = style().querySubControlMetrics(QStyle::CC_TitleBar, this,
					      QStyle::SC_TitleBarLabel).width();
    if ( !d->window )
	maxw = width() - 20;
    const QString txt = caption();
    d->cuttext = txt;
    if ( fm.width( txt + "m" ) > maxw ) {
	int i = txt.length();
	int dotlength = fm.width( "..." );
	while ( i>0 && fm.width(txt.left( i )) + dotlength > maxw )
	    i--;
	if(i != (int)txt.length())
	    d->cuttext = txt.left( i ) + "...";
    }
}

void QTitleBar::setCaption( const QString& title )
{
    if( caption() == title)
	return;
#ifndef QT_NO_WIDGET_TOPEXTRA
    QWidget::setCaption( title );
#else
    d->cap = title;
#endif
    cutText();

    update();
}


void QTitleBar::setIcon( const QPixmap& icon )
{
#ifndef QT_NO_WIDGET_TOPEXTRA
#ifndef QT_NO_IMAGE_SMOOTHSCALE
    QRect menur = style().querySubControlMetrics(QStyle::CC_TitleBar, this,
						  QStyle::SC_TitleBarSysMenu);

    QPixmap theIcon;
    if (icon.width() > menur.width()) {
	// try to keep something close to the same aspect
	int aspect = (icon.height() * 100) / icon.width();
	int newh = (aspect * menur.width()) / 100;
	theIcon.convertFromImage( icon.convertToImage().smoothScale(menur.width(),
								   newh) );
    } else if (icon.height() > menur.height()) {
	// try to keep something close to the same aspect
	int aspect = (icon.width() * 100) / icon.height();
	int neww = (aspect * menur.height()) / 100;
	theIcon.convertFromImage( icon.convertToImage().smoothScale(neww,
								   menur.height()) );
    } else
	theIcon = icon;

    QWidget::setIcon( theIcon );
#else
    QWidget::setIcon( icon );
#endif

    update();
#endif
}

void QTitleBar::leaveEvent( QEvent * )
{
    if(autoRaise() && !d->pressed)
	repaint( FALSE );
}

void QTitleBar::enterEvent( QEvent * )
{
    if(autoRaise() && !d->pressed)
	repaint( FALSE );
    QEvent e( QEvent::Leave );
    QApplication::sendEvent( parentWidget(), &e );
}

void QTitleBar::setActive( bool active )
{
    if ( d->act == active )
	return ;

    d->act = active;
    update();
}

bool QTitleBar::isActive() const
{
    return d->act;
}

bool QTitleBar::usesActiveColor() const
{
    return ( isActive() && isActiveWindow() ) ||
	   ( !window() && topLevelWidget()->isActiveWindow() );
}

QString QTitleBar::visibleText() const
{
    return d->cuttext;
}

QWidget *QTitleBar::window() const
{
    return d->window;
}

bool QTitleBar::event( QEvent* e )
{
    if ( e->type() == QEvent::ApplicationPaletteChange ) {
	readColors();
	return TRUE;
    } else if ( e->type() == QEvent::WindowActivate ) {
	setActive( d->act );
    } else if ( e->type() == QEvent::WindowDeactivate ) {
	bool wasActive = d->act;
	setActive( FALSE );
	d->act = wasActive;
    }

    return QWidget::event( e );
}

void QTitleBar::setMovable(bool b)
{
    d->movable = b;
}

bool QTitleBar::isMovable() const
{
    return d->movable;
}

void QTitleBar::setAutoRaise(bool b)
{
    d->autoraise = b;
}

bool QTitleBar::autoRaise() const
{
    return d->autoraise;
}

QSize QTitleBar::sizeHint() const
{
    constPolish();
    QRect menur = style().querySubControlMetrics(QStyle::CC_TitleBar, this,
						 QStyle::SC_TitleBarSysMenu);
    return QSize( menur.width(), style().pixelMetric( QStyle::PM_TitleBarHeight, this ) );
}

#endif //QT_NO_TITLEBAR
