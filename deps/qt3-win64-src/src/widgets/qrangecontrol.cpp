/****************************************************************************
** $Id: qrangecontrol.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QRangeControl class
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

#include "qrangecontrol.h"
#ifndef QT_NO_RANGECONTROL
#include "qglobal.h"
#include <limits.h>

static bool sumOutOfRange(int current, int add);


/*!
    \class QRangeControl qrangecontrol.h
    \brief The QRangeControl class provides an integer value within a range.

    \ingroup misc

    Although originally designed for the QScrollBar widget, the
    QRangeControl can also be used in conjunction with other widgets
    such as QSlider and QSpinBox. Here are the five main concepts in
    the class:

    \list 1

    \i \e{Current value} The bounded integer that
    QRangeControl maintains. value() returns it, and several
    functions, including setValue(), set it.

    \i \e{Minimum} The lowest value that value() can ever
    return. Returned by minValue() and set by setRange() or one of the
    constructors.

    \i \e{Maximum} The highest value that value() can ever
    return. Returned by maxValue() and set by setRange() or one of the
    constructors.

    \i \e{Line step} The smaller of two natural steps that
    QRangeControl provides and typically corresponds to the user
    pressing an arrow key. The line step is returned by lineStep()
    and set using setSteps(). The functions addLine() and
    subtractLine() respectively increment and decrement the current
    value by lineStep().

    \i \e{Page step} The larger of two natural steps that
    QRangeControl provides and typically corresponds to the user
    pressing PageUp or PageDown. The page step is returned by
    pageStep() and set using setSteps(). The functions addPage() and
    substractPage() respectively increment and decrement the current
    value by pageStep().

    \endlist

    Unity (1) may be viewed as a third step size. setValue() lets you
    set the current value to any integer in the allowed range, not
    just minValue() + \e n * lineStep() for integer values of \e n.
    Some widgets may allow the user to set any value at all; others
    may just provide multiples of lineStep() or pageStep().

    QRangeControl provides three virtual functions that are well
    suited for updating the on-screen representation of range controls
    and emitting signals: valueChange(), rangeChange() and
    stepChange().

    QRangeControl also provides a function called bound() which lets
    you force arbitrary integers to be within the allowed range of the
    range control.

    We recommend that all widgets that inherit QRangeControl provide
    at least a signal called valueChanged(); many widgets will want to
    provide addStep(), addPage(), substractStep() and substractPage()
    as slots.

    Note that you must use multiple inheritance if you plan to
    implement a widget using QRangeControl because QRangeControl is
    not derived from QWidget.
*/


/*!
    Constructs a range control with a minimum value of 0, maximum
    value of 99, line step of 1, page step of 10 and initial value 0.
*/

QRangeControl::QRangeControl()
{
    minVal  = 0;
    maxVal  = 99;
    line    = 1;
    page    = 10;
    val	    = 0;
    prevVal = -1;
    d	    = 0;
}

/*!
    Constructs a range control whose value can never be smaller than
    \a minValue or greater than \a maxValue, whose line step size is
    \a lineStep and page step size is \a pageStep and whose value is
    initially \a value (which is guaranteed to be in range using
    bound()).
*/

QRangeControl::QRangeControl( int minValue, int maxValue,
			      int lineStep, int pageStep,
			      int value )
{
    minVal  = minValue;
    maxVal  = maxValue;
    line    = QABS( lineStep );
    page    = QABS( pageStep );
    prevVal = minVal - 1;
    val	    = bound( value );
    d	    = 0;
}

/*!
    Destroys the range control
*/

QRangeControl::~QRangeControl()
{
}


/*!
    \fn int QRangeControl::value() const

    Returns the current range control value. This is guaranteed to be
    within the range [minValue(), maxValue()].

    \sa setValue() prevValue()
*/

/*!
    \fn int QRangeControl::prevValue() const

    Returns the previous value of the range control. "Previous value"
    means the value before the last change occurred. Setting a new
    range may affect the value, too, because the value is forced to be
    inside the specified range. When the range control is initially
    created, this is the same as value().

    prevValue() can be outside the current legal range if a call to
    setRange() causes the current value to change. For example, if the
    range was [0, 1000] and the current value is 500, setRange(0, 400)
    makes value() return 400 and prevValue() return 500.

    \sa value() setRange()
*/

/*!
    Sets the range control's value to \a value and forces it to be
    within the legal range.

    Calls the virtual valueChange() function if the new value is
    different from the previous value. The old value can still be
    retrieved using prevValue().

    \sa value()
*/

void QRangeControl::setValue( int value )
{
    directSetValue( value );
    if ( prevVal != val )
	valueChange();
}

/*!
    Sets the range control \a value directly without calling
    valueChange().

    Forces the new \a value to be within the legal range.

    You will rarely have to call this function. However, if you want
    to change the range control's value inside the overloaded method
    valueChange(), setValue() would call the function valueChange()
    again. To avoid this recursion you must use directSetValue()
    instead.

    \sa setValue()
*/

void QRangeControl::directSetValue(int value)
{
    prevVal = val;
    val = bound( value );
}

/*!
    Equivalent to \c{setValue( value() + pageStep() )}.

    If the value is changed, then valueChange() is called.

    \sa subtractPage() addLine() setValue()
*/

void QRangeControl::addPage()
{
    if (!sumOutOfRange(value(), pageStep()))
        setValue(value() + pageStep());
}

/*!
    Equivalent to \c{setValue( value() - pageStep() )}.

    If the value is changed, then valueChange() is called.

    \sa addPage() subtractLine() setValue()
*/

void QRangeControl::subtractPage()
{
    if (!sumOutOfRange(value(), -pageStep()))
        setValue(value() - pageStep());
}

/*!
    Equivalent to \c{setValue( value() + lineStep() )}.

    If the value is changed, then valueChange() is called.

    \sa subtractLine() addPage() setValue()
*/

void QRangeControl::addLine()
{
    if (!sumOutOfRange(value(), lineStep()))
        setValue(value() + lineStep());
}

/*!
    Equivalent to \c{setValue( value() - lineStep() )}.

    If the value is changed, then valueChange() is called.

    \sa addLine() subtractPage() setValue()
*/

void QRangeControl::subtractLine()
{
    if (!sumOutOfRange(value(), -lineStep()))
        setValue(value() - lineStep());
}


/*!
    \fn int QRangeControl::minValue() const

    Returns the minimum value of the range.

    \sa setMinValue() setRange() maxValue()
*/

/*!
    \fn int QRangeControl::maxValue() const

    Returns the maximum value of the range.

    \sa setMaxValue() setRange() minValue()
*/

/*!
    Sets the minimum value of the range to \a minVal.

    If necessary, the maxValue() is adjusted so that the range remains
    valid.

    \sa minValue() setMaxValue()
*/
void QRangeControl::setMinValue( int minVal )
{
    int maxVal = maxValue();
    if ( maxVal < minVal )
	maxVal = minVal;
    setRange( minVal, maxVal );
}

/*!
    Sets the minimum value of the range to \a maxVal.

    If necessary, the minValue() is adjusted so that the range remains
    valid.

    \sa maxValue() setMinValue()
*/
void QRangeControl::setMaxValue( int maxVal )
{
    int minVal = minValue();
    if ( minVal > maxVal )
	minVal = maxVal;
    setRange( minVal, maxVal );
}

/*!
    Sets the range control's minimum value to \a minValue and its
    maximum value to \a maxValue.

    Calls the virtual rangeChange() function if one or both of the new
    minimum and maximum values are different from the previous
    setting. Calls the virtual valueChange() function if the current
    value is adjusted because it was outside the new range.

    If \a maxValue is smaller than \a minValue, \a minValue becomes
    the only legal value.

    \sa minValue() maxValue()
*/

void QRangeControl::setRange( int minValue, int maxValue )
{
    if ( minValue > maxValue ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QRangeControl::setRange: minValue %d > maxValue %d",
		  minValue, maxValue );
#endif
	maxValue = minValue;
    }
    if ( minValue == minVal && maxValue == maxVal )
	return;
    minVal = minValue;
    maxVal = maxValue;
    int tmp = bound( val );
    rangeChange();
    if ( tmp != val ) {
	prevVal = val;
	val = tmp;
	valueChange();
    }
}


/*!
    \fn int QRangeControl::lineStep() const

    Returns the line step.

    \sa setSteps() pageStep()
*/

/*!
    \fn int QRangeControl::pageStep() const

    Returns the page step.

    \sa setSteps() lineStep()
*/

/*!
    Sets the range's line step to \a lineStep and page step to \a
    pageStep.

    Calls the virtual stepChange() function if the new line step
    or page step are different from the previous settings.

    \sa lineStep() pageStep() setRange()
*/

void QRangeControl::setSteps( int lineStep, int pageStep )
{
    if ( lineStep != line || pageStep != page ) {
	line = QABS( lineStep );
	page = QABS( pageStep );
	stepChange();
    }
}


/*!
    This virtual function is called whenever the range control value
    changes. You can reimplement it if you want to be notified when
    the value changes. The default implementation does nothing.

    Note that this method is called after the value has changed. The
    previous value can be retrieved using prevValue().

    \sa setValue(), addPage(), subtractPage(), addLine(),
    subtractLine() rangeChange(), stepChange()
*/

void QRangeControl::valueChange()
{
}


/*!
    This virtual function is called whenever the range control's range
    changes. You can reimplement it if you want to be notified when
    the range changes. The default implementation does nothing.

    Note that this method is called after the range has changed.

    \sa setRange(), valueChange(), stepChange()
*/

void QRangeControl::rangeChange()
{
}


/*!
    This virtual function is called whenever the range control's
    line or page step settings change. You can reimplement it if you
    want to be notified when the step changes. The default
    implementation does nothing.

    Note that this method is called after a step setting has changed.

    \sa setSteps(), rangeChange(), valueChange()
*/

void QRangeControl::stepChange()
{
}


/*!
    Forces the value \a v to be within the range from minValue() to
    maxValue() inclusive, and returns the result.

    This function is provided so that you can easily force other
    numbers than value() into the allowed range. You do not need to
    call it in order to use QRangeControl itself.

    \sa setValue() value() minValue() maxValue()
*/

int QRangeControl::bound( int v ) const
{
    if ( v < minVal )
	return minVal;
    if ( v > maxVal )
	return maxVal;
    return v;
}


/*!
    Converts \a logical_val to a pixel position. minValue() maps to 0,
    maxValue() maps to \a span and other values are distributed evenly
    in-between.

    This function can handle the entire integer range without
    overflow, providing \a span is \<= 4096.

    Calling this method is useful when actually drawing a range
    control such as a QScrollBar on-screen.

    \sa valueFromPosition()
*/

int QRangeControl::positionFromValue( int logical_val, int span ) const
{
    if ( span <= 0 || logical_val < minValue() || maxValue() <= minValue() )
	return 0;
    if ( logical_val > maxValue() )
	return span;

    uint range = maxValue() - minValue();
    uint p = logical_val - minValue();

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


/*!
    Converts the pixel position \a pos to a value. 0 maps to
    minValue(), \a span maps to maxValue() and other values are
    distributed evenly in-between.

    This function can handle the entire integer range without
    overflow.

    Calling this method is useful if you actually implemented a range
    control widget such as QScrollBar and want to handle mouse press
    events. This function then maps screen coordinates to the logical
    values.

    \sa positionFromValue()
*/

int QRangeControl::valueFromPosition( int pos, int span ) const
{
    if ( span <= 0 || pos <= 0 )
	return minValue();
    if ( pos >= span )
	return maxValue();

    uint range = maxValue() - minValue();

    if ( (uint)span > range )
	return  minValue() + (2*pos*range + span) / (2*span);
    else {
	uint div = range / span;
	uint mod = range % span;
	return  minValue() + pos*div + (2*pos*mod + span) / (2*span);
    }
    // equiv. to minValue() + (pos*range)/span + 0.5
    // no overflow because of this implicit assumption:
    // pos <= span < sqrt(INT_MAX+0.0625)+0.25 ~ sqrt(INT_MAX)
}

static bool sumOutOfRange(int current, int add)
{
    if (add > 0 && INT_MAX - add < current) {
        return true;
    }
    if (add < 0 && INT_MIN - add > current) {
        return true;
    }
    return false;
}

#endif
