/****************************************************************************
**
** Implementation of QSpinWidget class
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

#include "qrangecontrol.h"

#ifndef QT_NO_SPINWIDGET

#include "qrect.h"
#include "qtimer.h"
#include "qstyle.h"
#include "qpainter.h"

class QSpinWidgetPrivate
{
public:
    QSpinWidgetPrivate()
	: upEnabled( TRUE ),
	  downEnabled( TRUE ),
	  theButton( 0 ),
	  buttonDown( 0 ),
	  timerUp( 0 ),
	  bsyms( QSpinWidget::UpDownArrows ),
	  ed ( 0 ) {}
    uint upEnabled :1;
    uint downEnabled :1;
    uint theButton :2;
    uint buttonDown :2;
    uint timerUp : 1;
    QRect up;
    QRect down;
    QTimer auRepTimer;
    QSpinWidget::ButtonSymbols bsyms;
    QWidget *ed;
    void startTimer( int msec ) { auRepTimer.start( msec, TRUE ); }
    void startTimer( bool up, int msec ) { timerUp = up; startTimer( msec ); }
    void stopTimer() { auRepTimer.stop(); }
};

/*!

    \class QSpinWidget qspinwidget.h
    \brief The QSpinWidget class is an internal range control related class.

    \internal

    Constructs an empty range control widget with parent \a parent
    called \a name.

*/

QSpinWidget::QSpinWidget( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    d = new QSpinWidgetPrivate();
    connect( &d->auRepTimer, SIGNAL( timeout() ), this, SLOT( timerDone() ) );
    setFocusPolicy( StrongFocus );

    arrange();
    updateDisplay();
}


/*! Destroys the object and frees any allocated resources.

*/

QSpinWidget::~QSpinWidget()
{
    delete d;
}

/*! */
QWidget * QSpinWidget::editWidget()
{
    return d->ed;
}

/*!
    Sets the editing widget to \a w.
*/
void QSpinWidget::setEditWidget( QWidget * w )
{
    if ( w ) {
        if (w->parentWidget() != this)
	    w->reparent( this, QPoint( 0, 0 ) );
	setFocusProxy( w );
    }
    d->ed = w;
    arrange();
    updateDisplay();
}

/*! \reimp

*/

void QSpinWidget::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton ) {
	d->stopTimer();
	d->buttonDown = 0;
	d->theButton = 0;
	repaint( d->down.unite( d->up ), FALSE );
	return;
    }

    uint oldButtonDown = d->buttonDown;

    if ( d->down.contains( e->pos() ) && d->downEnabled )
	d->buttonDown = 1;
    else if ( d->up.contains( e->pos() ) && d->upEnabled )
	d->buttonDown = 2;
    else
	d->buttonDown = 0;

    d->theButton = d->buttonDown;
    if ( oldButtonDown != d->buttonDown ) {
	if ( !d->buttonDown ) {
	    repaint( d->down.unite( d->up ), FALSE );
	} else if ( d->buttonDown & 1 ) {
	    repaint( d->down, FALSE );
	    stepDown();
	    d->startTimer( FALSE, 300 );
	} else if ( d->buttonDown & 2 ) {
	    repaint( d->up, FALSE );
	    stepUp();
	    d->startTimer( TRUE, 300 );
	}
    }
}

/*!

*/

void QSpinWidget::arrange()
{
    d->up = QStyle::visualRect( style().querySubControlMetrics( QStyle::CC_SpinWidget, this,
								QStyle::SC_SpinWidgetUp ), this );
    d->down = QStyle::visualRect( style().querySubControlMetrics( QStyle::CC_SpinWidget, this,
								  QStyle::SC_SpinWidgetDown ), this );
    if ( d->ed ) {
    	QRect r = QStyle::visualRect( style().querySubControlMetrics( QStyle::CC_SpinWidget, this,
								  QStyle::SC_SpinWidgetEditField ), this );
	d->ed->setGeometry( r );
    }
}

/*!

*/

void QSpinWidget::stepUp()
{
    emit stepUpPressed();
}

void QSpinWidget::resizeEvent( QResizeEvent* )
{
    arrange();
}

/*!

*/

void QSpinWidget::stepDown()
{
    emit stepDownPressed();
}


void QSpinWidget::timerDone()
{
    // we use a double timer to make it possible for users to do
    // something with 0-timer on valueChanged.
    QTimer::singleShot( 1, this, SLOT( timerDoneEx() ) );
}

void QSpinWidget::timerDoneEx()
{
    if ( !d->buttonDown )
	return;
    if ( d->timerUp )
	stepUp();
    else
	stepDown();
    d->startTimer( 100 );
}


void QSpinWidget::windowActivationChange( bool oldActive )
{
    //was active, but lost focus
    if ( oldActive && d->buttonDown ) {
	d->stopTimer();
	d->buttonDown = 0;
	d->theButton = 0;
    }
    QWidget::windowActivationChange( oldActive );
}



/*!
    The event is passed in \a e.
*/

void QSpinWidget::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;

    uint oldButtonDown = d->theButton;
    d->theButton = 0;
    if ( oldButtonDown != d->theButton ) {
	if ( oldButtonDown & 1 )
	    repaint( d->down, FALSE );
	else if ( oldButtonDown & 2 )
	    repaint( d->up, FALSE );
    }
    d->stopTimer();
    d->buttonDown = 0;
}


/*!
    The event is passed in \a e.
*/

void QSpinWidget::mouseMoveEvent( QMouseEvent *e )
{
    if ( !(e->state() & LeftButton ) )
	return;

    uint oldButtonDown = d->theButton;
    if ( oldButtonDown & 1 && !d->down.contains( e->pos() ) ) {
	d->stopTimer();
	d->theButton = 0;
	repaint( d->down, FALSE );
    } else if ( oldButtonDown & 2 && !d->up.contains( e->pos() ) ) {
	d->stopTimer();
	d->theButton = 0;
	repaint( d->up, FALSE );
    } else if ( !oldButtonDown && d->up.contains( e->pos() ) && d->buttonDown & 2 ) {
	d->startTimer( 500 );
	d->theButton = 2;
	repaint( d->up, FALSE );
    } else if ( !oldButtonDown && d->down.contains( e->pos() ) && d->buttonDown & 1 ) {
	d->startTimer( 500 );
	d->theButton = 1;
	repaint( d->down, FALSE );
    }
}


/*!
    The event is passed in \a e.
*/
#ifndef QT_NO_WHEELEVENT
void QSpinWidget::wheelEvent( QWheelEvent *e )
{
    e->accept();
    static float offset = 0;
    static QSpinWidget* offset_owner = 0;
    if ( offset_owner != this ) {
	offset_owner = this;
	offset = 0;
    }
    offset += -e->delta()/120;
    if ( QABS( offset ) < 1 )
	return;
    int ioff = int(offset);
    int i;
    for( i=0; i < QABS( ioff ); i++ )
	offset > 0 ? stepDown() : stepUp();
    offset -= ioff;
}
#endif

/*!

*/
void QSpinWidget::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (hasFocus() || focusProxy() && focusProxy()->hasFocus())
	flags |= QStyle::Style_HasFocus;

    QStyle::SCFlags active;
    if ( d->theButton & 1 )
	active = QStyle::SC_SpinWidgetDown;
    else if ( d->theButton & 2 )
	active = QStyle::SC_SpinWidgetUp;
    else
	active = QStyle::SC_None;

    QRect fr = QStyle::visualRect(
	style().querySubControlMetrics( QStyle::CC_SpinWidget, this,
					QStyle::SC_SpinWidgetFrame ), this );
    style().drawComplexControl( QStyle::CC_SpinWidget, &p, this,
				fr, colorGroup(),
				flags,
				(uint)QStyle::SC_All,
				active );
}


/*!
    The previous style is passed in \a old.
*/

void QSpinWidget::styleChange( QStyle& old )
{
    arrange();
    QWidget::styleChange( old );
}

/*!
*/

QRect QSpinWidget::upRect() const
{
    return d->up;
}

/*!
*/

QRect QSpinWidget::downRect() const
{
    return d->down;
}

/*!
*/

void QSpinWidget::updateDisplay()
{
    if ( !isEnabled() ) {
	d->upEnabled = FALSE;
	d->downEnabled = FALSE;
    }
    if ( d->theButton & 1 && ( d->downEnabled ) == 0 ) {
	d->theButton &= ~1;
	d->buttonDown &= ~1;
    }

    if ( d->theButton & 2 && ( d->upEnabled ) == 0 ) {
	d->theButton &= ~2;
	d->buttonDown &= ~2;
    }
    repaint( FALSE );
}


/*!
    The previous enabled state is passed in \a old.
*/

void QSpinWidget::enableChanged( bool )
{
    d->upEnabled = isEnabled();
    d->downEnabled = isEnabled();
    updateDisplay();
}


/*!
    Sets up-enabled to \a on.
*/

void QSpinWidget::setUpEnabled( bool on )
{
    if ( (bool)d->upEnabled != on ) {
	d->upEnabled = on;
	updateDisplay();
    }
}

/*!
*/

bool QSpinWidget::isUpEnabled() const
{
    return d->upEnabled;
}

/*!
    Sets down-enabled to \a on.
*/

void QSpinWidget::setDownEnabled( bool on )
{
    if ( (bool)d->downEnabled != on ) {
	d->downEnabled = on;
	updateDisplay();
    }
}

/*!
*/

bool QSpinWidget::isDownEnabled() const
{
    return d->downEnabled;
}

/*!
    Sets the button symbol to \a bs.
*/

void QSpinWidget::setButtonSymbols( ButtonSymbols bs )
{
    d->bsyms = bs;
}

/*!
*/

QSpinWidget::ButtonSymbols QSpinWidget::buttonSymbols() const
{
    return d->bsyms;
}

#endif
