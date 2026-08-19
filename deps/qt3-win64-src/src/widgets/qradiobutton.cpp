/****************************************************************************
** $Id: qradiobutton.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QRadioButton class
**
** Created : 940222
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

#include "qradiobutton.h"
#ifndef QT_NO_RADIOBUTTON
#include "qbuttongroup.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qtextstream.h"
#include "qapplication.h"
#include "qstyle.h"

/*!
    \class QRadioButton qradiobutton.h
    \brief The QRadioButton widget provides a radio button with a text or pixmap label.

    \ingroup basic
    \mainclass

    QRadioButton and QCheckBox are both option buttons. That is, they
    can be switched on (checked) or off (unchecked). The classes
    differ in how the choices for the user are restricted. Check boxes
    define "many of many" choices, whereas radio buttons provide a
    "one of many" choice. In a group of radio buttons only one radio
    button at a time can be checked; if the user selects another
    button, the previously selected button is switched off.

    The easiest way to implement a "one of many" choice is simply to
    put the radio buttons into QButtonGroup.

    Whenever a button is switched on or off it emits the signal
    toggled(). Connect to this signal if you want to trigger an action
    each time the button changes state. Otherwise, use isChecked() to
    see if a particular button is selected.

    Just like QPushButton, a radio button can display text or a
    pixmap. The text can be set in the constructor or with setText();
    the pixmap is set with setPixmap().

    <img src=qradiobt-m.png> <img src=qradiobt-w.png>

    \important text, setText, text, pixmap, setPixmap, accel, setAccel, isToggleButton, setDown, isDown, isOn, state, autoRepeat, isExclusiveToggle, group, setAutoRepeat, toggle, pressed, released, clicked, toggled, state stateChanged

    \sa QPushButton QToolButton
    \link guibooks.html#fowler GUI Design Handbook: Radio Button\endlink
*/

/*!
    \property QRadioButton::checked \brief Whether the radio button is
    checked

    This property will not effect any other radio buttons unless they
    have been placed in the same QButtonGroup. The default value is
    FALSE (unchecked).
*/

/*!
    \property QRadioButton::autoMask \brief whether the radio button
    is automatically masked

    \sa QWidget::setAutoMask()
*/

/*!
    Constructs a radio button with no text.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.
*/

QRadioButton::QRadioButton( QWidget *parent, const char *name )
	: QButton( parent, name, WNoAutoErase | WMouseNoMask )
{
    init();
}

/*!
    Constructs a radio button with the text \a text.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.
*/

QRadioButton::QRadioButton( const QString &text, QWidget *parent,
			    const char *name )
	: QButton( parent, name, WNoAutoErase | WMouseNoMask )
{
    init();
    setText( text );
}


/*
    Initializes the radio button.
*/

void QRadioButton::init()
{
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    setToggleButton( TRUE );
#ifndef QT_NO_BUTTONGROUP
    QButtonGroup *bgrp = ::qt_cast<QButtonGroup*>(parentWidget());
    if ( bgrp )
	bgrp->setRadioButtonExclusive( TRUE );
#endif
}

void QRadioButton::setChecked( bool check )
{
    setOn( check );
}




/*!
    \reimp
*/
QSize QRadioButton::sizeHint() const
{
    // Any more complex, and we will use style().itemRect()
    // NB: QCheckBox::sizeHint() is similar
    constPolish();

    QPainter p(this);
    QSize sz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, FALSE,
				pixmap(), text()).size();

    return (style().sizeFromContents(QStyle::CT_RadioButton, this, sz).
	    expandedTo(QApplication::globalStrut()));
}


/*!
    \reimp
*/
bool QRadioButton::hitButton( const QPoint &pos ) const
{
    QRect r =
	QStyle::visualRect( style().subRect( QStyle::SR_RadioButtonFocusRect,
					     this ), this );
    if ( qApp->reverseLayout() ) {
	r.setRight( width() );
    } else {
	r.setLeft( 0 );
    }
    return r.contains( pos );
}


/*!
    \reimp
*/
void QRadioButton::drawButton( QPainter *paint )
{
    QPainter *p = paint;
    QRect irect = QStyle::visualRect( style().subRect(QStyle::SR_RadioButtonIndicator, this), this );
    const QColorGroup &cg = colorGroup();

#if !defined( QT_NO_TEXTSTREAM ) && !defined( Q_WS_MACX )
#   define  SAVE_RADIOBUTTON_PIXMAPS
#endif
#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isOn() )
	kf |= 2;
    if ( isEnabled() )
	kf |= 4;
    if( isActiveWindow() )
	kf |= 8;
    if ( hasMouse() )
	kf |= 16;
    if ( hasFocus() )
	kf |= 32;

    QTextOStream os(&pmkey);
    os << "$qt_radio_" << style().className() << "_"
       << palette().serialNumber() << "_" << irect.width() << "x" << irect.height() << "_" << kf;
    QPixmap *pm = QPixmapCache::find( pmkey );
    if ( pm ) {					// pixmap exists
	drawButtonLabel( p );
	p->drawPixmap( irect.topLeft(), *pm );
	return;
    }
    bool use_pm = TRUE;
    QPainter pmpaint;
    int wx, wy;
    if ( use_pm ) {
	pm = new QPixmap( irect.size() );	// create new pixmap
	Q_CHECK_PTR( pm );
	pm->fill(paletteBackgroundColor());
	QPainter::redirect(this, pm);
	pmpaint.begin(this);
	p = &pmpaint;				// draw in pixmap
	wx = irect.x();				// save x,y coords
	wy = irect.y();
	irect.moveTopLeft(QPoint(0, 0));
	p->setBackgroundColor(paletteBackgroundColor());
    }
#endif

    QStyle::SFlags flags = QStyle::Style_Default;
    if ( isEnabled() )
	flags |= QStyle::Style_Enabled;
    if ( hasFocus() )
	flags |= QStyle::Style_HasFocus;
    if ( isDown() )
	flags |= QStyle::Style_Down;
    if ( hasMouse() )
	flags |= QStyle::Style_MouseOver;
    if ( state() == QButton::On )
	flags |= QStyle::Style_On;
    else if ( state() == QButton::Off )
	flags |= QStyle::Style_Off;

    style().drawControl(QStyle::CE_RadioButton, p, this, irect, cg, flags);

#if defined(SAVE_RADIOBUTTON_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	QPainter::redirect(this, NULL);
	if ( backgroundPixmap() || backgroundMode() == X11ParentRelative ) {
	    QBitmap bm( pm->size() );
	    bm.fill( color0 );
	    pmpaint.begin( &bm );
	    style().drawControlMask(QStyle::CE_RadioButton, &pmpaint, this, irect);
	    pmpaint.end();
	    pm->setMask( bm );
	}
	p = paint;				// draw in default device
	p->drawPixmap( wx, wy, *pm );
	if (!QPixmapCache::insert(pmkey, pm) )	// save in cache
	    delete pm;
    }
#endif

    drawButtonLabel( p );
}



/*!
    \reimp
*/
void QRadioButton::drawButtonLabel( QPainter *p )
{
    QRect r =
	QStyle::visualRect( style().subRect(QStyle::SR_RadioButtonContents,
					    this), this );

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (hasFocus())
	flags |= QStyle::Style_HasFocus;
    if (isDown())
	flags |= QStyle::Style_Down;
    if (state() == QButton::On)
	flags |= QStyle::Style_On;
    else if (state() == QButton::Off)
	flags |= QStyle::Style_Off;

    style().drawControl(QStyle::CE_RadioButtonLabel, p, this, r, colorGroup(), flags);
}


/*!
    \reimp
*/
void QRadioButton::resizeEvent( QResizeEvent* e )
{
    QButton::resizeEvent(e);
    if ( isVisible() ) {
	QPainter p(this);
	QSize isz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, FALSE,
				     pixmap(), text()).size();
	QSize wsz = (style().sizeFromContents(QStyle::CT_RadioButton, this, isz).
		     expandedTo(QApplication::globalStrut()));

	update(wsz.width(), isz.width(), 0, wsz.height());
    }
    if (autoMask())
	updateMask();
}

/*!
    \reimp
*/
void QRadioButton::updateMask()
{
    QRect irect =
	QStyle::visualRect( style().subRect( QStyle::SR_RadioButtonIndicator,
					     this ), this );

    QBitmap bm(width(), height());
    bm.fill(color0);

    QPainter p( &bm, this );
    style().drawControlMask(QStyle::CE_RadioButton, &p, this, irect);
    if ( ! text().isNull() || ( pixmap() && ! pixmap()->isNull() ) ) {
	QRect crect =
	    QStyle::visualRect( style().subRect( QStyle::SR_RadioButtonContents,
						 this ), this );
	QRect frect =
	    QStyle::visualRect( style().subRect( QStyle::SR_RadioButtonFocusRect,
						 this ), this );
	QRect label(crect.unite(frect));
	p.fillRect(label, color1);
    }
    p.end();

    setMask(bm);
}

#endif
