/****************************************************************************
** $Id: qbutton.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QButton widget class
**
** Created : 940206
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

#undef QT_NO_COMPAT
#include "qbutton.h"
#ifndef QT_NO_BUTTON
#include "qbuttongroup.h"
#include "qbitmap.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qaccel.h"
#include "qpixmapcache.h"
#include "qapplication.h"
#include "qpushbutton.h"
#include "qradiobutton.h"
#include "qguardedptr.h"
#include "../kernel/qinternal_p.h"

#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

#define AUTO_REPEAT_DELAY  300
#define AUTO_REPEAT_PERIOD 100

class QButtonData
{
public:
    QButtonData() {
#ifndef QT_NO_BUTTONGROUP
	group = 0;
#endif
#ifndef QT_NO_ACCEL
	a = 0;
#endif
    }
#ifndef QT_NO_BUTTONGROUP
    QButtonGroup *group;
#endif
    QTimer timer;
#ifndef QT_NO_ACCEL
    QAccel *a;
#endif
};


void QButton::ensureData()
{
    if ( !d ) {
	d = new QButtonData;
	Q_CHECK_PTR( d );
	connect(&d->timer, SIGNAL(timeout()), this, SLOT(autoRepeatTimeout()));
    }
}


/*!
    Returns the group that this button belongs to.

    If the button is not a member of any QButtonGroup, this function
    returns 0.

    \sa QButtonGroup
*/

QButtonGroup *QButton::group() const
{
#ifndef QT_NO_BUTTONGROUP
    return d ? d->group : 0;
#else
    return 0;
#endif
}


void QButton::setGroup( QButtonGroup* g )
{
#ifndef QT_NO_BUTTONGROUP
    ensureData();
    d->group = g;
#endif
}


QTimer *QButton::timer()
{
    ensureData();
    return &d->timer;
}


/*!
    \class QButton qbutton.h
    \brief The QButton class is the abstract base class of button
    widgets, providing functionality common to buttons.

    \ingroup abstractwidgets

    <b>If you want to create a button use QPushButton.</b>

    The QButton class implements an \e abstract button, and lets
    subclasses specify how to reply to user actions and how to draw
    the button.

    QButton provides both push and toggle buttons. The QRadioButton
    and QCheckBox classes provide only toggle buttons; QPushButton and
    QToolButton provide both toggle and push buttons.

    Any button can have either a text or pixmap label. setText() sets
    the button to be a text button and setPixmap() sets it to be a
    pixmap button. The text/pixmap is manipulated as necessary to
    create the "disabled" appearance when the button is disabled.

    QButton provides most of the states used for buttons:
    \list
    \i isDown() indicates whether the button is \e pressed down.
    \i isOn() indicates whether the button is \e on.
       Only toggle buttons can be switched on and off  (see below).
    \i isEnabled() indicates whether the button can be pressed by the
       user.
    \i setAutoRepeat() sets whether the button will auto-repeat
       if the user holds it down.
    \i setToggleButton() sets whether the button is a toggle
       button or not.
    \endlist

    The difference between isDown() and isOn() is as follows: When the
    user clicks a toggle button to toggle it on, the button is first
    \e pressed and then released into the \e on state. When the user
    clicks it again (to toggle it off), the button moves first to the
    \e pressed state, then to the \e off state (isOn() and isDown()
    are both FALSE).

    Default buttons (as used in many dialogs) are provided by
    QPushButton::setDefault() and QPushButton::setAutoDefault().

    QButton provides five signals:
    \list 1
    \i pressed() is emitted when the button is pressed. E.g. with the mouse
       or when animateClick() is called.
    \i released() is emitted when the button is released. E.g. when the mouse
       is released or the cursor is moved outside the widget.
    \i clicked() is emitted when the button is first pressed and then
       released when the accelerator key is typed, or when
       animateClick() is called.
    \i toggled(bool) is emitted when the state of a toggle button changes.
    \i stateChanged(int) is emitted when the state of a tristate
       toggle button changes.
    \endlist

    If the button is a text button with an ampersand (\&) in its text,
    QButton creates an automatic accelerator key. This code creates a
    push button labelled "Ro<u>c</u>k \& Roll" (where the c is
    underlined). The button gets an automatic accelerator key, Alt+C:

    \code
	QPushButton *p = new QPushButton( "Ro&ck && Roll", this );
    \endcode

    In this example, when the user presses Alt+C the button will call
    animateClick().

    You can also set a custom accelerator using the setAccel()
    function. This is useful mostly for pixmap buttons because they
    have no automatic accelerator.

    \code
	p->setPixmap( QPixmap("print.png") );
	p->setAccel( ALT+Key_F7 );
    \endcode

    All of the buttons provided by Qt (\l QPushButton, \l QToolButton,
    \l QCheckBox and \l QRadioButton) can display both text and
    pixmaps.

    To subclass QButton, you must reimplement at least drawButton()
    (to draw the button's outline) and drawButtonLabel() (to draw its
    text or pixmap). It is generally advisable to reimplement
    sizeHint() as well, and sometimes hitButton() (to determine
    whether a button press is within the button).

    To reduce flickering, QButton::paintEvent() sets up a pixmap that
    the drawButton() function draws in. You should not reimplement
    paintEvent() for a subclass of QButton unless you want to take
    over all drawing.

    \sa QButtonGroup
*/


/*!
    \enum QButton::ToggleType

    This enum type defines what a button can do in response to a
    mouse/keyboard press:

    \value SingleShot  pressing the button causes an action, then the
    button returns to the unpressed state.

    \value Toggle  pressing the button toggles it between an \c On and
    an \c Off state.

    \value Tristate  pressing the button cycles between the three
    states \c On, \c Off and \c NoChange
*/

/*!
    \enum QButton::ToggleState

    This enum defines the state of a toggle button.

    \value Off  the button is in the "off" state
    \value NoChange  the button is in the default/unchanged state
    \value On  the button is in the "on" state
*/

/*!
    \property QButton::accel
    \brief the accelerator associated with the button

    This property is 0 if there is no accelerator set. If you set this
    property to 0 then any current accelerator is removed.
*/

/*!
    \property QButton::autoRepeat
    \brief whether autoRepeat is enabled

    If autoRepeat is enabled then the clicked() signal is emitted at
    regular intervals if the button is down. This property has no
    effect on toggle buttons. autoRepeat is off by default.
*/

/*! \property QButton::autoResize
    \brief whether autoResize is enabled
    \obsolete

  If autoResize is enabled then the button will resize itself
  whenever the contents are changed.
*/

/*!
    \property QButton::down
    \brief whether the button is pressed

    If this property is TRUE, the button is pressed down. The signals
    pressed() and clicked() are not emitted if you set this property
    to TRUE. The default is FALSE.
*/

/*!
    \property QButton::exclusiveToggle
    \brief whether the button is an exclusive toggle

    If this property is TRUE and the button is in a QButtonGroup, the
    button can only be toggled off by another one being toggled on.
    The default is FALSE.
*/

/*!
    \property QButton::on
    \brief whether the button is toggled

    This property should only be set for toggle buttons.
*/

/*!
    \fn void QButton::setOn( bool on )

    Sets the state of this button to On if \a on is TRUE; otherwise to
    Off.

    \sa toggleState
*/

/*!
    \property QButton::pixmap
    \brief the pixmap shown on the button

    If the pixmap is monochrome (i.e. it is a QBitmap or its \link
    QPixmap::depth() depth\endlink is 1) and it does not have a mask,
    this property will set the pixmap to be its own mask. The purpose
    of this is to draw transparent bitmaps which are important for
    toggle buttons, for example.

    pixmap() returns 0 if no pixmap was set.
*/

/*!
    \property QButton::text
    \brief the text shown on the button

    This property will return a QString::null if the button has no
    text. If the text has an ampersand (\&) in it, then an
    accelerator is automatically created for it using the character
    that follows the '\&' as the accelerator key. Any previous
    accelerator will be overwritten, or cleared if no accelerator is
    defined by the text.

    There is no default text.
*/

/*!
    \property QButton::toggleButton
    \brief whether the button is a toggle button

    The default value is FALSE.
*/

/*!
    \fn QButton::setToggleButton( bool b )

    If \a b is TRUE, this button becomes a toggle button; if \a b is
    FALSE, this button becomes a command button.

    \sa toggleButton
*/

/*!
    \property QButton::toggleState
    \brief the state of the toggle button

    If this property is changed then it does not cause the button
    to be repainted.
*/

/*!
    \property QButton::toggleType
    \brief the type of toggle on the button

    The default toggle type is \c SingleShot.

    \sa QButton::ToggleType
*/

/*!
    Constructs a standard button called \a name with parent \a parent,
    using the widget flags \a f.

    If \a parent is a QButtonGroup, this constructor calls
    QButtonGroup::insert().
*/

QButton::QButton( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    bpixmap    = 0;
    toggleTyp  = SingleShot;			// button is simple
    buttonDown = FALSE;				// button is up
    stat       = Off;				// button is off
    mlbDown    = FALSE;				// mouse left button up
    autoresize = FALSE;				// not auto resizing
    animation  = FALSE;				// no pending animateClick
    repeat     = FALSE;				// not in autorepeat mode
    d	       = 0;
#ifndef QT_NO_BUTTONGROUP
    if ( ::qt_cast<QButtonGroup*>(parent) ) {
	setGroup((QButtonGroup*)parent);
	group()->insert( this );		// insert into button group
    }
#endif
    setFocusPolicy( TabFocus );
}

/*!
    Destroys the button.
 */
QButton::~QButton()
{
#ifndef QT_NO_BUTTONGROUP
    if ( group() )
	group()->remove( this );
#endif
    delete bpixmap;
    delete d;
}


/*!
    \fn void QButton::pressed()

    This signal is emitted when the button is pressed down.

    \sa released(), clicked()
*/

/*!
    \fn void QButton::released()

    This signal is emitted when the button is released.

    \sa pressed(), clicked(), toggled()
*/

/*!
    \fn void QButton::clicked()

    This signal is emitted when the button is activated (i.e. first
    pressed down and then released when the mouse cursor is inside the
    button), when the accelerator key is typed or when animateClick()
    is called. This signal is \e not emitted if you call setDown().

    The QButtonGroup::clicked() signal does the same job, if you want
    to connect several buttons to the same slot.

    \warning Don't launch a model dialog in response to this signal
    for a button that has \c autoRepeat turned on.

    \sa pressed(), released(), toggled() autoRepeat down
*/

/*!
    \fn void QButton::toggled( bool on )

    This signal is emitted whenever a toggle button changes status. \a
    on is TRUE if the button is on, or FALSE if the button is off.

    This may be the result of a user action, toggle() slot activation,
    or because setOn() was called.

    \sa clicked()
*/

/*!
    \fn void QButton::stateChanged( int state )

    This signal is emitted whenever a toggle button changes state. \a
    state is \c On if the button is on, \c NoChange if it is in the
    \link QCheckBox::setTristate() "no change" state\endlink or \c Off
    if the button is off.

    This may be the result of a user action, toggle() slot activation,
    setState(), or because setOn() was called.

    \sa clicked() QButton::ToggleState
*/

void QButton::setText( const QString &text )
{
    if ( btext == text )
	return;
    btext = text;
#ifndef QT_NO_ACCEL
    setAccel( QAccel::shortcutKey( text ) );
#endif

    if ( bpixmap ) {
	delete bpixmap;
	bpixmap = 0;
    }

    if ( autoresize )
	adjustSize();

    update();
    updateGeometry();

#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::NameChanged );
#endif
}

void QButton::setPixmap( const QPixmap &pixmap )
{
    if ( bpixmap && bpixmap->serialNumber() == pixmap.serialNumber() )
	return;

    bool newSize;
    if ( bpixmap ) {
	newSize = pixmap.width() != bpixmap->width() ||
		  pixmap.height() != bpixmap->height();
	*bpixmap = pixmap;
    } else {
	newSize = TRUE;
	bpixmap = new QPixmap( pixmap );
	Q_CHECK_PTR( bpixmap );
    }
    if ( bpixmap->depth() == 1 && !bpixmap->mask() )
	bpixmap->setMask( *((QBitmap *)bpixmap) );
    if ( !btext.isNull() ) {
	btext = QString::null;
#ifndef QT_NO_ACCEL
	setAccel( QKeySequence() );
#endif
    }
    if ( autoresize && newSize )
	adjustSize();
    if ( autoMask() )
	updateMask();
    update();
    if ( newSize )
	updateGeometry();
}


#ifndef QT_NO_ACCEL
QKeySequence QButton::accel() const
{
    if ( d && d->a )
	return d->a->key( 0 );
    return QKeySequence();
}

void QButton::setAccel( const QKeySequence& key )
{
    if ( d && d->a )
	d->a->clear();
    if ( key.isEmpty() )
	return;
    ensureData();
    if ( !d->a ) {
	d->a = new QAccel( this, "buttonAccel" );
	connect( d->a, SIGNAL( activated(int) ), this, SLOT( animateClick() ) );
	connect( d->a, SIGNAL( activatedAmbiguously(int) ), this, SLOT( setFocus() ) );
    }
    d->a->insertItem( key, 0 );
}
#endif

#ifndef QT_NO_COMPAT

void QButton::setAutoResize( bool enable )
{
    if ( (bool)autoresize != enable ) {
	autoresize = enable;
	if ( autoresize )
	    adjustSize();			// calls resize which repaints
    }
}

#endif

void QButton::setAutoRepeat( bool enable )
{
    repeat = (uint)enable;
    if ( repeat && mlbDown )
	timer()->start( AUTO_REPEAT_DELAY, TRUE );
}

/*!
    Performs an animated click: the button is pressed and released a
    short while later.

    The pressed(), released(), clicked(), toggled(), and
    stateChanged() signals are emitted as appropriate.

    This function does nothing if the button is \link setEnabled()
    disabled. \endlink

    \sa setAccel()
*/

void QButton::animateClick()
{
    if ( !isEnabled() || animation )
	return;
    animation = TRUE;
    buttonDown = TRUE;
    repaint( FALSE );
    emit pressed();
    QTimer::singleShot( 100, this, SLOT(animateTimeout()) );
}

void QButton::emulateClick()
{
    if ( !isEnabled() || animation )
	return;
    animation = TRUE;
    buttonDown = TRUE;
    emit pressed();
    animateTimeout();
}

void QButton::setDown( bool enable )
{
    if ( d )
	timer()->stop();
    mlbDown = FALSE;				// the safe setting
    if ( (bool)buttonDown != enable ) {
	buttonDown = enable;
	repaint( FALSE );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
    }
}

/*!
  Sets the toggle state of the button to \a s. \a s can be \c Off, \c
  NoChange or \c On.
*/

void QButton::setState( ToggleState s )
{
    if ( !toggleTyp ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QButton::setState() / setOn: (%s) Only toggle buttons "
		 "may be switched", name( "unnamed" ) );
#endif
	return;
    }

    if ( (ToggleState)stat != s ) {		// changed state
	bool was = stat != Off;
	stat = s;
	if ( autoMask() )
	    updateMask();
	repaint( FALSE );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
	// ### toggled for tristate makes no sense. Don't emit the signal in 4.0
	if ( was != (stat != Off) )
	    emit toggled( stat != Off );
	emit stateChanged( s );
    }
}


/*!
    Returns TRUE if \a pos is inside the clickable button rectangle;
    otherwise returns FALSE.

    By default, the clickable area is the entire widget. Subclasses
    may reimplement it, though.
*/
bool QButton::hitButton( const QPoint &pos ) const
{
    return rect().contains( pos );
}

/*!
    Draws the button. The default implementation does nothing.

    This virtual function is reimplemented by subclasses to draw real
    buttons. At some point, these reimplementations should call
    drawButtonLabel().

    \sa drawButtonLabel(), paintEvent()
*/
#if (QT_VERSION-0 >= 0x040000)
#error "QButton. Make pure virtual"
#endif
void QButton::drawButton( QPainter * )
{
    return;
}

/*!
    Draws the button text or pixmap.

    This virtual function is reimplemented by subclasses to draw real
    buttons. It is invoked by drawButton().

    \sa drawButton(), paintEvent()
*/

void QButton::drawButtonLabel( QPainter * )
{
    return;
}

/*! \reimp */
void QButton::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Enter:
    case Key_Return:
	{
#ifndef QT_NO_PUSHBUTTON
	    QPushButton *pb = (QPushButton*)qt_cast( "QPushButton" );
	    if ( pb && ( pb->autoDefault() || pb->isDefault() ) )
		emit clicked();
	    else
#endif
		e->ignore();
	}
	break;
    case Key_Space:
	if ( !e->isAutoRepeat() ) {
	    setDown( TRUE );
#ifndef QT_NO_PUSHBUTTON
	    if ( ::qt_cast<QPushButton*>(this) )
		emit pressed();
	    else
#endif
		e->ignore();
	}
	break;
    case Key_Up:
    case Key_Left:
#ifndef QT_NO_BUTTONGROUP
	if ( group() ) {
	    group()->moveFocus( e->key() );
	} else
#endif
	{
	    QFocusEvent::setReason(QFocusEvent::Backtab);
	    focusNextPrevChild( FALSE );
	    QFocusEvent::resetReason();
	}
	break;
    case Key_Right:
    case Key_Down:
#ifndef QT_NO_BUTTONGROUP
	if ( group() ) {
	    group()->moveFocus( e->key() );
	} else
#endif
	{
	    QFocusEvent::setReason(QFocusEvent::Tab);
	    focusNextPrevChild( TRUE );
	    QFocusEvent::resetReason();
	}
	break;
    case Key_Escape:
	if ( buttonDown ) {
	    buttonDown = FALSE;
	    update();
	    break;
	}
	// fall through
    default:
	e->ignore();
    }
}

/*! \reimp */
void QButton::keyReleaseEvent( QKeyEvent * e)
{
    switch ( e->key() ) {
    case Key_Space:
	if ( buttonDown && !e->isAutoRepeat() ) {
	    buttonDown = FALSE;
	    nextState();
	    emit released();
	    emit clicked();
	}
	break;
    default:
	e->ignore();
    }
}

/*! \reimp */
void QButton::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton ) {
	e->ignore();
	return;
    }
    bool hit = hitButton( e->pos() );
    if ( hit ) {				// mouse press on button
	mlbDown = TRUE;				// left mouse button down
	buttonDown = TRUE;
	if ( autoMask() )
	    updateMask();

	repaint( FALSE );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
	QGuardedPtr<QTimer> t = timer();
	emit pressed();
	if ( t && repeat )
	    t->start( AUTO_REPEAT_DELAY, TRUE );
    }
}

/*! \reimp */
void QButton::mouseReleaseEvent( QMouseEvent *e)
{
    if ( e->button() != LeftButton ) {

	// clean up apperance if left button has been pressed
	if (mlbDown || buttonDown) {
	    mlbDown = FALSE;
	    buttonDown = FALSE;

	    if ( autoMask() )
		updateMask();
	    repaint( FALSE );
	}

	e->ignore();
	return;
    }
    if ( !mlbDown )
	return;
    if ( d )
	timer()->stop();

    const bool oldButtonDown = buttonDown;
    mlbDown = FALSE;				// left mouse button up
    buttonDown = FALSE;
    if ( hitButton( e->pos() ) ) {		// mouse release on button
	nextState();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
	emit released();
	emit clicked();
    } else {
	repaint( FALSE );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
        if (oldButtonDown)
            emit released();
    }
}

/*! \reimp */
void QButton::mouseMoveEvent( QMouseEvent *e )
{
    if ( !((e->state() & LeftButton) && mlbDown) ) {
	e->ignore();
	return;					// left mouse button is up
    }
    if ( hitButton(e->pos()) ) {		// mouse move in button
	if ( !buttonDown ) {
	    buttonDown = TRUE;
	    repaint( FALSE );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
	    emit pressed();
	}
    } else {					// mouse move outside button
	if ( buttonDown ) {
	    buttonDown = FALSE;
	    repaint( FALSE );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
	    emit released();
	}
    }
}


/*!
    Handles paint events for buttons. Small and typically complex
    buttons are painted double-buffered to reduce flicker. The
    actually drawing is done in the virtual functions drawButton() and
    drawButtonLabel().

    \sa drawButton(), drawButtonLabel()
*/
void QButton::paintEvent( QPaintEvent *)
{
    QSharedDoubleBuffer buffer( this );
    drawButton( buffer.painter() );
}

/*! \reimp */
void QButton::focusInEvent( QFocusEvent * e)
{
    QWidget::focusInEvent( e );
}

/*! \reimp */
void QButton::focusOutEvent( QFocusEvent * e )
{
    buttonDown = FALSE;
    QWidget::focusOutEvent( e );
}

/*!
    Internal slot used for auto repeat.
*/
void QButton::autoRepeatTimeout()
{
    if ( mlbDown && isEnabled() && autoRepeat() ) {
	QGuardedPtr<QTimer> t = timer();
	if ( buttonDown ) {
	    emit released();
	    emit clicked();
	    emit pressed();
	}
	if ( t )
	    t->start( AUTO_REPEAT_PERIOD, TRUE );
    }
}

/*!
    Internal slot used for the second stage of animateClick().
*/
void QButton::animateTimeout()
{
    if ( !animation )
	return;
    animation  = FALSE;
    buttonDown = FALSE;
    nextState();
    emit released();
    emit clicked();
}


void QButton::nextState()
{
    bool t = isToggleButton() && !( isOn() && isExclusiveToggle() );
    bool was = stat != Off;
    if ( t ) {
	if ( toggleTyp == Tristate )
	    stat = ( stat + 1 ) % 3;
	else
	    stat = stat ? Off : On;
    }
    if ( autoMask() )
        updateMask();
    repaint( FALSE );
    if ( t ) {
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
	if ( was != (stat != Off) )
	    emit toggled( stat != Off );
	emit stateChanged( stat );
    }
}

/*! \reimp */
void QButton::enabledChange( bool e )
{
    if ( !isEnabled() )
	setDown( FALSE );
    QWidget::enabledChange( e );
}


/*!
    Toggles the state of a toggle button.

    \sa isOn(), setOn(), toggled(), isToggleButton()
*/
void QButton::toggle()
{
    if ( isToggleButton() )
	 setOn( !isOn() );
}

/*!
    Sets the toggle type of the button to \a type.

    \a type can be set to \c SingleShot, \c Toggle and \c Tristate.
*/
void QButton::setToggleType( ToggleType type )
{
    toggleTyp = type;
    if ( type != Tristate && stat == NoChange )
	setState( On );
#if defined(QT_ACCESSIBILITY_SUPPORT)
    else
	QAccessible::updateAccessibility( this, 0, QAccessible::StateChanged );
#endif
}

bool QButton::isExclusiveToggle() const
{
#ifndef QT_NO_BUTTONGROUP
    return group() && ( group()->isExclusive() ||
			group()->isRadioButtonExclusive() &&
			::qt_cast<QRadioButton*>(this) );
#else
    return FALSE;
#endif
}

#endif
