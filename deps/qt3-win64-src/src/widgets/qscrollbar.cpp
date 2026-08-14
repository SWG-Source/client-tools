/****************************************************************************
** $Id: qscrollbar.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QScrollBar class
**
** Created : 940427
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

#include "qscrollbar.h"
#ifndef QT_NO_SCROLLBAR
#include "qpainter.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qtimer.h"
#include "qstyle.h"
#ifndef QT_NO_CURSOR
#include <qcursor.h>
#endif
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif
#include <limits.h>

/*!
    \class QScrollBar
    \brief The QScrollBar widget provides a vertical or horizontal scroll bar.

    \ingroup basic

    A scroll bar allows the user to control a value within a
    program-definable range and gives users a visible indication of
    the current value of a \link QRangeControl range control \endlink.

    Scroll bars include four separate controls:

    \list

    \i The \e line-up and \e line-down controls are little buttons
    which the user can use to move one "line" up or down. The meaning
    of line is configurable. In editors and list boxes it means one
    line of text; in an image viewer it might mean 20 pixels.

    \i The \e slider is the handle that indicates the current value of
    the scroll bar, which the user can drag to change the value. This
    part of the scroll bar is sometimes called the "thumb".

    \i The \e page-up/page-down control is the area on which the
    slider slides (the scroll bar's background). Clicking here moves
    the scroll bar towards the click. The meaning of "page" is also
    configurable: in editors and list boxes it means as many lines as
    there is space for in the widget.

    \endlist

    QScrollBar has very few of its own functions; it mostly relies on
    QRangeControl. The most useful functions are setValue() to set the
    scroll bar directly to some value; addPage(), addLine(),
    subtractPage(), and subtractLine() to simulate the effects of
    clicking (useful for accelerator keys); setSteps() to define the
    values of pageStep() and lineStep(); and setRange() to set the
    minValue() and maxValue() of the scroll bar. QScrollBar has a
    convenience constructor with which you can set most of these
    properties.

    Some GUI styles (for example, the Windows and Motif styles
    provided with Qt), also use the pageStep() value to calculate the
    size of the slider.

    In addition to the access functions from QRangeControl, QScrollBar
    provides a comprehensive set of signals:
    \table
    \header \i Signal \i Emitted when
    \row \i \l valueChanged()
	 \i the scroll bar's value has changed. The tracking()
	    determines whether this signal is emitted during user
	    interaction.
    \row \i \l sliderPressed()
	 \i the user starts to drag the slider.
    \row \i \l sliderMoved()
	 \i the user drags the slider.
    \row \i \l sliderReleased()
	 \i the user releases the slider.
    \row \i \l nextLine()
	 \i the scroll bar has moved one line down or right. Line is
	    defined in QRangeControl.
    \row \i \l prevLine()
	 \i the scroll bar has moved one line up or left.
    \row \i \l nextPage()
	 \i the scroll bar has moved one page down or right.
    \row \i \l prevPage()
	 \i the scroll bar has moved one page up or left.
    \endtable

    QScrollBar only provides integer ranges. Note that although
    QScrollBar handles very large numbers, scroll bars on current
    screens cannot usefully control ranges above about 100,000 pixels.
    Beyond that, it becomes difficult for the user to control the
    scroll bar using either the keyboard or the mouse.

    A scroll bar can be controlled by the keyboard, but it has a
    default focusPolicy() of \c NoFocus. Use setFocusPolicy() to
    enable keyboard focus. See keyPressEvent() for a list of key
    bindings.

    If you need to add scroll bars to an interface, consider using the
    QScrollView class, which encapsulates the common uses for scroll
    bars.

    <img src=qscrbar-m.png> <img src=qscrbar-w.png>

    \sa QSlider QSpinBox QScrollView
    \link guibooks.html#fowler GUI Design Handbook: Scroll Bar\endlink
*/


/*!
    \fn void QScrollBar::valueChanged( int value )

    This signal is emitted when the scroll bar value has changed, with
    the new scroll bar \a value as an argument.
*/

/*!
    \fn void QScrollBar::sliderPressed()

    This signal is emitted when the user presses the slider with the
    mouse.
*/

/*!
    \fn void QScrollBar::sliderMoved( int value )

    This signal is emitted when the slider is dragged by the user, with
    the new scroll bar \a value as an argument.

    This signal is emitted even when tracking is turned off.

    \sa tracking() valueChanged() nextLine() prevLine() nextPage()
    prevPage()
*/

/*!
    \fn void QScrollBar::sliderReleased()

    This signal is emitted when the user releases the slider with the
    mouse.
*/

/*!
    \fn void QScrollBar::nextLine()

    This signal is emitted when the scroll bar scrolls one line down
    or right.
*/

/*!
    \fn void QScrollBar::prevLine()

    This signal is emitted when the scroll bar scrolls one line up or
    left.
*/

/*!
    \fn void QScrollBar::nextPage()

    This signal is emitted when the scroll bar scrolls one page down
    or right.
*/

/*!
    \fn void QScrollBar::prevPage()

    This signal is emitted when the scroll bar scrolls one page up or
    left.
*/



static const int thresholdTime = 500;
static const int repeatTime	= 50;

#define HORIZONTAL	(orientation() == Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	2
#define SLIDER_MIN	9


/*!
    Constructs a vertical scroll bar.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.

    The \c minValue defaults to 0, the \c maxValue to 99, with a \c
    lineStep size of 1 and a \c pageStep size of 10, and an initial
    \c value of 0.
*/

QScrollBar::QScrollBar( QWidget *parent, const char *name )
    : QWidget( parent, name ), orient( Vertical )
{
    init();
}

/*!
    Constructs a scroll bar.

    The \a orientation must be \c Qt::Vertical or \c Qt::Horizontal.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.

    The \c minValue defaults to 0, the \c maxValue to 99, with a \c
    lineStep size of 1 and a \c pageStep size of 10, and an initial
    \c value of 0.
*/

QScrollBar::QScrollBar( Orientation orientation, QWidget *parent,
			const char *name )
    : QWidget( parent, name ), orient( orientation )
{
    init();
}

/*!
    Constructs a scroll bar whose value can never be smaller than \a
    minValue or greater than \a maxValue, whose line step size is \a
    lineStep and page step size is \a pageStep and whose value is
    initially \a value (which is guaranteed to be in range using
    bound()).

    If \a orientation is \c Vertical the scroll bar is vertical and if
    it is \c Horizontal the scroll bar is horizontal.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.
*/

QScrollBar::QScrollBar( int minValue, int maxValue, int lineStep, int pageStep,
			int value,  Orientation orientation,
			QWidget *parent, const char *name )
    : QWidget( parent, name ),
      QRangeControl( minValue, maxValue, lineStep, pageStep, value ),
      orient( orientation )
{
    init();
}

/*!
    Destructor.
*/
QScrollBar::~QScrollBar()
{
}

void QScrollBar::init()
{
    track = TRUE;
    sliderPos = 0;
    pressedControl = QStyle::SC_None;
    clickedAt = FALSE;
    setFocusPolicy( NoFocus );

    repeater = 0;

    setBackgroundMode((Qt::BackgroundMode)
		      style().styleHint(QStyle::SH_ScrollBar_BackgroundMode));

    QSizePolicy sp( QSizePolicy::Minimum, QSizePolicy::Fixed );
    if ( orient == Vertical )
	sp.transpose();
    setSizePolicy( sp );
    clearWState( WState_OwnSizePolicy );
}


/*!
    \property QScrollBar::orientation
    \brief the orientation of the scroll bar

    The orientation must be \l Qt::Vertical (the default) or \l
    Qt::Horizontal.
*/

void QScrollBar::setOrientation( Orientation orientation )
{
    if ( orientation == orient )
	return;
    if ( !testWState( WState_OwnSizePolicy ) ) {
	QSizePolicy sp = sizePolicy();
	sp.transpose();
	setSizePolicy( sp );
	clearWState( WState_OwnSizePolicy );
    }

    orient = orientation;

    positionSliderFromValue();
    update();
    updateGeometry();
}

/*!
    \property QScrollBar::tracking
    \brief whether scroll bar tracking is enabled

    If tracking is enabled (the default), the scroll bar emits the
    valueChanged() signal while the slider is being dragged. If
    tracking is disabled, the scroll bar emits the valueChanged()
    signal only when the user releases the mouse button after moving
    the slider.
*/


/*!
    \property QScrollBar::draggingSlider
    \brief whether the user has clicked the mouse on the slider and is currently dragging it
*/

bool QScrollBar::draggingSlider() const
{
    return pressedControl == QStyle::SC_ScrollBarSlider;
}


/*!
    Reimplements the virtual function QWidget::setPalette().

    Sets the background color to the mid color for Motif style scroll
    bars using palette \a p.
*/

void QScrollBar::setPalette( const QPalette &p )
{
    QWidget::setPalette( p );
    setBackgroundMode((Qt::BackgroundMode)
		      style().styleHint(QStyle::SH_ScrollBar_BackgroundMode));
}


/*! \reimp */
QSize QScrollBar::sizeHint() const
{
    constPolish();
    int sbextent = style().pixelMetric(QStyle::PM_ScrollBarExtent, this);

    if ( orient == Horizontal ) {
	return QSize( 30, sbextent );
    } else {
	return QSize( sbextent, 30 );
    }
}

/*! \fn void QScrollBar::setSizePolicy( QSizePolicy::SizeType, QSizePolicy::SizeType, bool )
    \reimp
*/

/*! \reimp */
void QScrollBar::setSizePolicy( QSizePolicy sp )
{
    //## remove 4.0
    QWidget::setSizePolicy( sp );
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::valueChange()
{
    int tmp = sliderPos;
    positionSliderFromValue();
    if ( tmp != sliderPos && isVisible() )
	drawControls(QStyle::SC_ScrollBarAddPage |
		     QStyle::SC_ScrollBarSubPage |
		     QStyle::SC_ScrollBarSlider,
		     pressedControl );
    emit valueChanged(value());
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::stepChange()
{
    rangeChange();
}

/*!
  \internal
  Implements the virtual QRangeControl function.
*/

void QScrollBar::rangeChange()
{
    positionSliderFromValue();

    if ( isVisible() )
	drawControls(QStyle::SC_ScrollBarAddLine |
		     QStyle::SC_ScrollBarSubLine |
		     QStyle::SC_ScrollBarAddPage |
		     QStyle::SC_ScrollBarSubPage |
		     QStyle::SC_ScrollBarFirst   |
		     QStyle::SC_ScrollBarLast    |
		     QStyle::SC_ScrollBarSlider,
		     pressedControl );
}


/*!
    Handles timer events for the scroll bar.
*/

void QScrollBar::doAutoRepeat()
{
    bool sendRepeat = clickedAt;
#if !defined( QT_NO_CURSOR ) && !defined( QT_NO_STYLE )
    if(sendRepeat && (pressedControl == QStyle::SC_ScrollBarAddPage ||
                      pressedControl == QStyle::SC_ScrollBarSubPage) &&
       style().styleHint(QStyle::SH_ScrollBar_StopMouseOverSlider, this) &&
       style().querySubControl(QStyle::CC_ScrollBar, this,
                               mapFromGlobal(QCursor::pos()) ) == QStyle::SC_ScrollBarSlider)
        sendRepeat = FALSE;
#endif
    if ( sendRepeat ){
        if ( repeater )
            repeater->changeInterval( repeatTime );
        action( (QStyle::SubControl) pressedControl );
        QApplication::syncX();
    } else {
        stopAutoRepeat();
    }
}


/*!
    Starts the auto-repeat logic. Some time after this function is
    called, the auto-repeat starts taking effect and from then on
    repeats until stopAutoRepeat() is called.
*/

void QScrollBar::startAutoRepeat()
{
    if ( !repeater ) {
	repeater = new QTimer( this, "auto-repeat timer" );
	connect( repeater, SIGNAL(timeout()),
		 this, SLOT(doAutoRepeat()) );
    }
    repeater->start( thresholdTime, FALSE );
}


/*!
    Stops the auto-repeat logic.
*/

void QScrollBar::stopAutoRepeat()
{
    delete repeater;
    repeater = 0;
}


/*!
    \reimp
*/
#ifndef QT_NO_WHEELEVENT
void QScrollBar::wheelEvent( QWheelEvent *e )
{
    static float offset = 0;
    static QScrollBar* offset_owner = 0;
    if (offset_owner != this){
	offset_owner = this;
	offset = 0;
    }
    if ( e->orientation() != orient && !rect().contains(e->pos()) )
	return;
    e->accept();
    int step = QMIN( QApplication::wheelScrollLines()*lineStep(),
		     pageStep() );
    if ( ( e->state() & ControlButton ) || ( e->state() & ShiftButton ) )
	step = pageStep();
    offset += -e->delta()*step/120;
    if (QABS(offset)<1)
	return;
    setValue( value() + int(offset) );
    offset -= int(offset);
}
#endif

/*!
    \reimp
*/
void QScrollBar::keyPressEvent( QKeyEvent *e )
{
    //   \list
    //     \i Left/Right move a horizontal scrollbar by one line.
    //     \i Up/Down move a vertical scrollbar by one line.
    //     \i PageUp moves up one page.
    //     \i PageDown moves down one page.
    //     \i Home moves to the start (minValue()).
    //     \i End moves to the end (maxValue()).
    //   \endlist

    //   Note that unless you call setFocusPolicy(), the default NoFocus
    //   will apply and the user will not be able to use the keyboard to
    //   interact with the scrollbar.
    switch ( e->key() ) {
    case Key_Left:
	if ( orient == Horizontal )
	    subtractLine();
	break;
    case Key_Right:
	if ( orient == Horizontal )
	    addLine();
	break;
    case Key_Up:
	if ( orient == Vertical )
	    subtractLine();
	break;
    case Key_Down:
	if ( orient == Vertical )
	    addLine();
	break;
    case Key_PageUp:
	subtractPage();
	break;
    case Key_PageDown:
	addPage();
	break;
    case Key_Home:
	setValue( minValue() );
	break;
    case Key_End:
	setValue( maxValue() );
	break;
    default:
	e->ignore();
	break;
    }
}


/*!
    \reimp
*/
void QScrollBar::resizeEvent( QResizeEvent * )
{
    positionSliderFromValue();
}


/*!
    \reimp
*/
void QScrollBar::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    drawControls(QStyle::SC_ScrollBarAddLine |
		 QStyle::SC_ScrollBarSubLine |
		 QStyle::SC_ScrollBarAddPage |
		 QStyle::SC_ScrollBarSubPage |
		 QStyle::SC_ScrollBarFirst   |
		 QStyle::SC_ScrollBarLast    |
		 QStyle::SC_ScrollBarSlider,
		 pressedControl, &p );
}

static QCOORD sliderStartPos = 0;

/*!
    \reimp
 */
void QScrollBar::contextMenuEvent( QContextMenuEvent *e )
{
    if(clickedAt)
	e->consume();
    else
	e->ignore();
}

/*!
    \reimp
*/
void QScrollBar::mousePressEvent( QMouseEvent *e )
{
    bool midButtonAbsPos =
	style().styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition,
			  this);

    if ( !(e->button() == LeftButton ||
	   (midButtonAbsPos && e->button() == MidButton) ) )
	return;

    if ( maxValue() == minValue() ) // nothing to be done
	return;

    if ( e->state() & MouseButtonMask ) // another button was already pressed
	return;

    clickedAt	   = TRUE;
    pressedControl = style().querySubControl(QStyle::CC_ScrollBar, this, e->pos() );

    if ( (pressedControl == QStyle::SC_ScrollBarAddPage ||
	  pressedControl == QStyle::SC_ScrollBarSubPage ||
	  pressedControl == QStyle::SC_ScrollBarSlider ) &&
	 ((midButtonAbsPos && e->button() == MidButton) ||
	  style().styleHint(QStyle::SH_ScrollBar_LeftClickAbsolutePosition) && e->button() == LeftButton)) {

	QRect sr = style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
						  QStyle::SC_ScrollBarSlider ),
	      gr = style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
						  QStyle::SC_ScrollBarGroove );
	int sliderMin, sliderMax, sliderLength;
	sliderMin = sliderMax = sliderLength = 0;
	if (HORIZONTAL) {
	    sliderMin = gr.x();
	    sliderMax = sliderMin + gr.width();
	    sliderLength = sr.width();
	} else {
	    sliderMin = gr.y();
	    sliderMax = sliderMin + gr.height();
	    sliderLength = sr.height();
	}

	int newSliderPos = (HORIZONTAL ? e->pos().x() : e->pos().y())
			   - sliderLength/2;
	newSliderPos = QMIN( newSliderPos, sliderMax - sliderLength );
	newSliderPos = QMAX( newSliderPos, sliderMin );
	setValue( sliderPosToRangeValue(newSliderPos) );
	sliderPos = newSliderPos;
	pressedControl = QStyle::SC_ScrollBarSlider;
    }

    if ( pressedControl == QStyle::SC_ScrollBarSlider ) {
	clickOffset = (QCOORD)( (HORIZONTAL ? e->pos().x() : e->pos().y())
				- sliderPos );
	slidePrevVal   = value();
	sliderStartPos = sliderPos;
	drawControls( pressedControl, pressedControl );
	emit sliderPressed();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::ScrollingStart );
#endif
    } else if ( pressedControl != QStyle::SC_None ) {
	drawControls( pressedControl, pressedControl );
	action( (QStyle::SubControl) pressedControl );
	startAutoRepeat();
    }
}


/*!
    \reimp
*/
void QScrollBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( !clickedAt )
	return;

    if ( e->stateAfter() & MouseButtonMask ) // some other button is still pressed
	return;

    QStyle::SubControl tmp = (QStyle::SubControl) pressedControl;
    clickedAt = FALSE;
    stopAutoRepeat();
    mouseMoveEvent( e );  // Might have moved since last mouse move event.
    pressedControl = QStyle::SC_None;

    if (tmp == QStyle::SC_ScrollBarSlider) {
	directSetValue( calculateValueFromSlider() );
	emit sliderReleased();
	if ( value() != prevValue() ) {
	    emit valueChanged( value() );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
	}
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::ScrollingEnd );
#endif
    }
    drawControls( tmp, pressedControl );
    if ( e->button() == MidButton )
	repaint( FALSE );
}


/*!
    \reimp
*/
void QScrollBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !isVisible() ) {
	clickedAt = FALSE;
	return;
    }

    bool mcab = style().styleHint(QStyle::SH_ScrollBar_MiddleClickAbsolutePosition,
				  this);
    if ( ! clickedAt || ! (e->state() & LeftButton ||
			   ((e->state() & MidButton) && mcab)))
	return;

    int newSliderPos;
    if ( pressedControl == QStyle::SC_ScrollBarSlider ) {
	QRect gr = style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
						  QStyle::SC_ScrollBarGroove ),
	      sr = style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
						  QStyle::SC_ScrollBarSlider );
	int sliderMin, sliderMax, sliderLength;

	if (HORIZONTAL) {
	    sliderLength = sr.width();
	    sliderMin = gr.x();
	    sliderMax = gr.right() - sliderLength + 1;
	} else {
	    sliderLength = sr.height();
	    sliderMin = gr.y();
	    sliderMax = gr.bottom() - sliderLength + 1;
	}

	QRect r = rect();
	int m = style().pixelMetric(QStyle::PM_MaximumDragDistance, this);
	if ( m >= 0 ) {
	    if ( orientation() == Horizontal )
		r.setRect( r.x() - m, r.y() - 2*m, r.width() + 2*m, r.height() + 4*m );
	    else
		r.setRect( r.x() - 2*m, r.y() - m, r.width() + 4*m, r.height() + 2*m );
	    if (! r.contains( e->pos()))
		newSliderPos = sliderStartPos;
	    else
		newSliderPos = (HORIZONTAL ? e->pos().x() :
				e->pos().y()) -clickOffset;
	} else
	    newSliderPos = (HORIZONTAL ? e->pos().x() :
			    e->pos().y()) -clickOffset;

	if ( newSliderPos < sliderMin )
	    newSliderPos = sliderMin;
	else if ( newSliderPos > sliderMax )
	    newSliderPos = sliderMax;
	int newVal = sliderPosToRangeValue(newSliderPos);
	if ( newVal != slidePrevVal )
	    emit sliderMoved( newVal );
	if ( track && newVal != value() ) {
	    directSetValue( newVal ); // Set directly, painting done below
	    emit valueChanged( value() );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
	}
	slidePrevVal = newVal;
	sliderPos = (QCOORD)newSliderPos;
	drawControls( QStyle::SC_ScrollBarAddPage |
		      QStyle::SC_ScrollBarSlider  |
		      QStyle::SC_ScrollBarSubPage,
		      pressedControl );
    } else if (! style().styleHint(QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl)) {
	// stop scrolling when the mouse pointer leaves a control
	// similar to push buttons
	if ( pressedControl != (uint)style().querySubControl(QStyle::CC_ScrollBar, this, e->pos() ) ) {
	    drawControls( pressedControl, QStyle::SC_None );
	    stopAutoRepeat();
	} else if ( !repeater ) {
	    drawControls( pressedControl, pressedControl );
	    action( (QStyle::SubControl) pressedControl );
	    startAutoRepeat();
	}
    }
}


/*!
    \fn int QScrollBar::sliderStart() const

    Returns the pixel position where the scroll bar slider starts.

    This is equivalent to sliderRect().y() for vertical scroll bars or
    sliderRect().x() for horizontal scroll bars.
*/

/*!
    Returns the scroll bar slider rectangle.

    \sa sliderStart()
*/

QRect QScrollBar::sliderRect() const
{
    return style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
					  QStyle::SC_ScrollBarSlider );
}

void QScrollBar::positionSliderFromValue()
{
    sliderPos = (QCOORD)rangeValueToSliderPos( value() );
}

int QScrollBar::calculateValueFromSlider() const
{
    return sliderPosToRangeValue( sliderPos );
}

int QScrollBar::rangeValueToSliderPos( int v ) const
{
    QRect gr = style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
					      QStyle::SC_ScrollBarGroove );
    QRect sr = style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
					      QStyle::SC_ScrollBarSlider );
    int sliderMin, sliderMax, sliderLength;

    if (HORIZONTAL) {
	sliderLength = sr.width();
	sliderMin = gr.x();
	sliderMax = gr.right() - sliderLength + 1;
    } else {
	sliderLength = sr.height();
	sliderMin = gr.y();
	sliderMax = gr.bottom() - sliderLength + 1;
    }

    return positionFromValue( v, sliderMax-sliderMin ) + sliderMin;
}

int QScrollBar::sliderPosToRangeValue( int pos ) const
{
    QRect gr = style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
					      QStyle::SC_ScrollBarGroove );
    QRect sr = style().querySubControlMetrics(QStyle::CC_ScrollBar, this,
					      QStyle::SC_ScrollBarSlider );
    int sliderMin, sliderMax, sliderLength;

    if (HORIZONTAL) {
	sliderLength = sr.width();
	sliderMin = gr.x();
	sliderMax = gr.right() - sliderLength + 1;
    } else {
	sliderLength = sr.height();
	sliderMin = gr.y();
	sliderMax = gr.bottom() - sliderLength + 1;
    }

    return  valueFromPosition( pos - sliderMin, sliderMax - sliderMin );
}


void QScrollBar::action( int control )
{
    switch( control ) {
    case QStyle::SC_ScrollBarAddLine:
	addLine();
	emit nextLine();
	break;
    case QStyle::SC_ScrollBarSubLine:
	subtractLine();
	emit prevLine();
	break;
    case QStyle::SC_ScrollBarAddPage:
	addPage();
	emit nextPage();
	break;
    case QStyle::SC_ScrollBarSubPage:
	subtractPage();
	emit prevPage();
	break;
    case QStyle::SC_ScrollBarFirst:
	setValue( minValue() );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
	emit valueChanged( minValue() );
	break;
    case QStyle::SC_ScrollBarLast:
	setValue( maxValue() );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
	emit valueChanged( maxValue() );
	break;
    default:
	break;
    }
}


void QScrollBar::drawControls( uint controls, uint activeControl ) const
{
    QPainter p ( this );
    drawControls( controls, activeControl, &p );
}


void QScrollBar::drawControls( uint controls, uint activeControl,
			       QPainter *p ) const
{
    if ( !isUpdatesEnabled() )
	return;

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
	flags |= QStyle::Style_Enabled;
    if (hasFocus())
	flags |= QStyle::Style_HasFocus;
    if ( orientation() == Horizontal )
	flags |= QStyle::Style_Horizontal;

    style().drawComplexControl(QStyle::CC_ScrollBar, p, this, rect(), colorGroup(),
			       flags, (QStyle::SubControl) controls,
			       (QStyle::SubControl) activeControl );
}

/*!
    \reimp
*/
void QScrollBar::styleChange( QStyle& old )
{
    positionSliderFromValue();
    setBackgroundMode((Qt::BackgroundMode)
		      style().styleHint(QStyle::SH_ScrollBar_BackgroundMode));
    QWidget::styleChange( old );
}

/*!
    \property QScrollBar::minValue
    \brief the scroll bar's minimum value

    When setting this property, the \l QScrollBar::maxValue is
    adjusted if necessary to ensure that the range remains valid.

    \sa setRange()
*/
int QScrollBar::minValue() const
{
    return QRangeControl::minValue();
}

void QScrollBar::setMinValue( int minVal )
{
    QRangeControl::setMinValue( minVal );
}

/*!
    \property QScrollBar::maxValue
    \brief the scroll bar's maximum value

    When setting this property, the \l QScrollBar::minValue is
    adjusted if necessary to ensure that the range remains valid.

    \sa setRange()
*/
int QScrollBar::maxValue() const
{
    return QRangeControl::maxValue();
}

void QScrollBar::setMaxValue( int maxVal )
{
    QRangeControl::setMaxValue( maxVal );
}

/*!
    \property QScrollBar::lineStep
    \brief the line step

    When setting lineStep, the virtual stepChange() function will be
    called if the new line step is different from the previous
    setting.

    \sa setSteps() QRangeControl::pageStep() setRange()
*/

int QScrollBar::lineStep() const
{
    return QRangeControl::lineStep();
}

/*!
    \property QScrollBar::pageStep
    \brief the page step

    When setting pageStep, the virtual stepChange() function will be
    called if the new page step is different from the previous
    setting.

    \sa QRangeControl::setSteps() setLineStep() setRange()
*/

int QScrollBar::pageStep() const
{
    return QRangeControl::pageStep();
}

void QScrollBar::setLineStep( int i )
{
    setSteps( i, pageStep() );
}

void QScrollBar::setPageStep( int i )
{
    setSteps( lineStep(), i );
}

/*!
    \property QScrollBar::value
    \brief the scroll bar's value

    \sa QRangeControl::value() prevValue()
*/

int QScrollBar::value() const
{
    return QRangeControl::value();
}

void QScrollBar::setValue( int i )
{
    QRangeControl::setValue( i );
}


/*!
    This function is called when the scrollbar is hidden.
*/
void QScrollBar::hideEvent( QHideEvent* )
{
    pressedControl = QStyle::SC_None;
    clickedAt = FALSE;
}


#undef ADD_LINE_ACTIVE
#undef SUB_LINE_ACTIVE
#endif
