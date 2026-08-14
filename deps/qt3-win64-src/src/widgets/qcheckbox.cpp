/****************************************************************************
** $Id: qcheckbox.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QCheckBox class
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

#include "qcheckbox.h"
#ifndef QT_NO_CHECKBOX
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qtextstream.h"
#include "qapplication.h"
#include "qstyle.h"

/*!
    \class QCheckBox qcheckbox.h
    \brief The QCheckBox widget provides a checkbox with a text label.

    \ingroup basic
    \mainclass

    QCheckBox and QRadioButton are both option buttons. That is, they
    can be switched on (checked) or off (unchecked). The classes
    differ in how the choices for the user are restricted. Radio
    buttons define a "one of many" choice, whereas checkboxes provide
    "many of many" choices.

    A QButtonGroup can be used to group check buttons visually.

    Whenever a checkbox is checked or cleared it emits the signal
    toggled(). Connect to this signal if you want to trigger an action
    each time the checkbox changes state. You can use isChecked() to
    query whether or not a checkbox is checked.

    \warning The toggled() signal can not be trusted for tristate
    checkboxes.

    In addition to the usual checked and unchecked states, QCheckBox
    optionally provides a third state to indicate "no change". This
    is useful whenever you need to give the user the option of neither
    checking nor unchecking a checkbox. If you need this third state,
    enable it with setTristate() and use state() to query the current
    toggle state. When a tristate checkbox changes state, it emits the
    stateChanged() signal.

    Just like QPushButton, a checkbox can display text or a pixmap.
    The text can be set in the constructor or with setText(); the
    pixmap is set with setPixmap().

    \important text(), setText(), text(), pixmap(), setPixmap(),
    accel(), setAccel(), isToggleButton(), setDown(), isDown(),
    isOn(), state(), autoRepeat(), isExclusiveToggle(), group(),
    setAutoRepeat(), toggle(), pressed(), released(), clicked(),
    toggled(), state() stateChanged()

    <img src=qchkbox-m.png> <img src=qchkbox-w.png>

    \sa QButton QRadioButton
    \link guibooks.html#fowler Fowler: Check Box \endlink
*/

/*!
    \property QCheckBox::checked
    \brief whether the checkbox is checked

    The default is unchecked, i.e. FALSE.
*/

/*!
    \property QCheckBox::autoMask
    \brief whether the checkbox is automatically masked

    \sa QWidget::setAutoMask()
*/

/*!
    \property QCheckBox::tristate
    \brief whether the checkbox is a tri-state checkbox

    The default is two-state, i.e. tri-state is FALSE.
*/

/*!
    Constructs a checkbox with no text.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.
*/

QCheckBox::QCheckBox( QWidget *parent, const char *name )
	: QButton( parent, name, WNoAutoErase | WMouseNoMask )
{
    setToggleButton( TRUE );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
}

/*!
    Constructs a checkbox with text \a text.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.
*/

QCheckBox::QCheckBox( const QString &text, QWidget *parent, const char *name )
	: QButton( parent, name, WNoAutoErase | WMouseNoMask )
{
    setText( text );
    setToggleButton( TRUE );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
}

/*!
    Sets the checkbox to the "no change" state.

    \sa setTristate()
*/
void QCheckBox::setNoChange()
{
    setTristate(TRUE);
    setState( NoChange );
}

void QCheckBox::setTristate(bool y)
{
    setToggleType( y ? Tristate : Toggle );
}

bool QCheckBox::isTristate() const
{
    return toggleType() == Tristate;
}


/*!\reimp
*/
QSize QCheckBox::sizeHint() const
{
    // NB: QRadioButton::sizeHint() is similar
    constPolish();

    QPainter p(this);
    QSize sz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, FALSE,
				pixmap(), text()).size();

    return (style().sizeFromContents(QStyle::CT_CheckBox, this, sz).
	    expandedTo(QApplication::globalStrut()));
}


/*!\reimp
*/

void QCheckBox::drawButton( QPainter *paint )
{
    QPainter *p = paint;
    QRect irect = QStyle::visualRect( style().subRect(QStyle::SR_CheckBoxIndicator, this), this );
    const QColorGroup &cg = colorGroup();

#if !defined( QT_NO_TEXTSTREAM ) && !defined( Q_WS_MACX )
#   define  SAVE_CHECKBOX_PIXMAPS
#endif
#if defined(SAVE_CHECKBOX_PIXMAPS)
    QString pmkey;				// pixmap key
    int kf = 0;
    if ( isDown() )
	kf |= 1;
    if ( isEnabled() )
	kf |= 2;
    if ( hasFocus() )
	kf |= 4;				// active vs. normal colorgroup
    if( isActiveWindow() )
	kf |= 8;
    if ( hasMouse() )
	kf |= 16;

    kf |= state() << 5;
    QTextOStream os(&pmkey);
    os << "$qt_check_" << style().className() << "_"
       << palette().serialNumber() << "_" << irect.width() << "x" << irect.height() << "_" << kf;
    QPixmap *pm = QPixmapCache::find( pmkey );
    if ( pm ) {					// pixmap exists
	p->drawPixmap( irect.topLeft(), *pm );
	drawButtonLabel( p );
	return;
    }
    bool use_pm = TRUE;
    QPainter pmpaint;
    int wx = 0, wy = 0;
    if ( use_pm ) {
	pm = new QPixmap( irect.size() );	// create new pixmap
	Q_CHECK_PTR( pm );
	pm->fill( cg.background() );
	QPainter::redirect(this, pm);
	pmpaint.begin(this);
	p = &pmpaint;				// draw in pixmap
	wx = irect.x();				// save x,y coords
	wy = irect.y();
	irect.moveTopLeft(QPoint(0, 0));
	p->setBackgroundColor( cg.background() );
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
    else if ( state() == QButton::NoChange )
	flags |= QStyle::Style_NoChange;

    style().drawControl(QStyle::CE_CheckBox, p, this, irect, cg, flags);

#if defined(SAVE_CHECKBOX_PIXMAPS)
    if ( use_pm ) {
	pmpaint.end();
	QPainter::redirect( this, 0 );
	if ( backgroundPixmap() || backgroundMode() == X11ParentRelative ) {
	    QBitmap bm( pm->size() );
	    bm.fill( color0 );
	    pmpaint.begin( &bm );
	    style().drawControlMask(QStyle::CE_CheckBox, &pmpaint, this, irect);
	    pmpaint.end();
	    pm->setMask( bm );
	}
	p = paint;				// draw in default device
	p->drawPixmap( wx, wy, *pm );
	if (!QPixmapCache::insert(pmkey, pm) )	// save in cache
	    delete pm;
    }
#endif

    drawButtonLabel( paint );
}


/*!\reimp
*/
void QCheckBox::drawButtonLabel( QPainter *p )
{
    QRect r =
	QStyle::visualRect( style().subRect(QStyle::SR_CheckBoxContents, this), this );

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
    else if (state() == QButton::NoChange)
	flags |= QStyle::Style_NoChange;

    style().drawControl(QStyle::CE_CheckBoxLabel, p, this, r, colorGroup(), flags);
}

/*!
  \reimp
*/
void QCheckBox::resizeEvent( QResizeEvent *e )
{
    QButton::resizeEvent(e);
    if ( isVisible() ) {
	QPainter p(this);
	QSize isz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, FALSE,
				     pixmap(), text()).size();
	QSize wsz = (style().sizeFromContents(QStyle::CT_CheckBox, this, isz).
		     expandedTo(QApplication::globalStrut()));

	update(wsz.width(), isz.width(), 0, wsz.height());
    }
    if (autoMask())
	updateMask();
}

/*!
  \reimp
*/
void QCheckBox::updateMask()
{
    QRect irect =
	QStyle::visualRect( style().subRect(QStyle::SR_CheckBoxIndicator, this), this );

    QBitmap bm(width(), height());
    bm.fill(color0);

    QPainter p( &bm, this );
    style().drawControlMask(QStyle::CE_CheckBox, &p, this, irect);
    if ( ! text().isNull() || ( pixmap() && ! pixmap()->isNull() ) ) {
	QRect crect =
	    QStyle::visualRect( style().subRect( QStyle::SR_CheckBoxContents,
						 this ), this );
	QRect frect =
	    QStyle::visualRect( style().subRect( QStyle::SR_CheckBoxFocusRect,
						 this ), this );
	QRect label(crect.unite(frect));
	p.fillRect(label, color1);
    }
    p.end();

    setMask(bm);
}

/*!\reimp*/
bool QCheckBox::hitButton( const QPoint &pos ) const
{
    QRect r = QStyle::visualRect( style().subRect( QStyle::SR_CheckBoxFocusRect, this ), this );
    if ( qApp->reverseLayout() ) {
	r.setRight( width() );
    } else {
	r.setLeft( 0 );
    }
    return r.contains( pos );
}

#endif
