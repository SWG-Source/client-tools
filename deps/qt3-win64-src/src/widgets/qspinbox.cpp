/****************************************************************************
** $Id: qspinbox.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QSpinBox widget class
**
** Created : 970101
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

#include "qspinbox.h"
#ifndef QT_NO_SPINBOX

#include "qcursor.h"
#include "qpushbutton.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qlineedit.h"
#include "qvalidator.h"
#include "qpixmapcache.h"
#include "qapplication.h"
#include "qstyle.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

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

class QSpinBoxPrivate
{
public:
    QSpinBoxPrivate() {}
    QSpinWidget* controls;
    uint selreq	: 1;
};

class QSpinBoxValidator : public QIntValidator
{
public:
    QSpinBoxValidator( QSpinBox *sb, const char *name )
	: QIntValidator( sb, name ), spinBox( sb ) { }

    virtual State validate( QString& str, int& pos ) const;

private:
    QSpinBox *spinBox;
};

QValidator::State QSpinBoxValidator::validate( QString& str, int& pos ) const
{
    QString pref = spinBox->prefix();
    QString suff = spinBox->suffix();
    QString suffStriped = suff.stripWhiteSpace();
    uint overhead = pref.length() + suff.length();
    State state = Invalid;

    ((QIntValidator *) this)->setRange( spinBox->minValue(),
					spinBox->maxValue() );
    if ( overhead == 0 ) {
	state = QIntValidator::validate( str, pos );
    } else {
	bool stripedVersion = FALSE;
	if ( str.length() >= overhead && str.startsWith(pref)
	     && (str.endsWith(suff)
		 || (stripedVersion = str.endsWith(suffStriped))) ) {
	    if ( stripedVersion )
		overhead = pref.length() + suffStriped.length();
	    QString core = str.mid( pref.length(), str.length() - overhead );
	    int corePos = pos - pref.length();
	    state = QIntValidator::validate( core, corePos );
	    pos = corePos + pref.length();
	    str.replace( pref.length(), str.length() - overhead, core );
	} else {
	    state = QIntValidator::validate( str, pos );
	    if ( state == Invalid ) {
		// stripWhiteSpace(), cf. QSpinBox::interpretText()
		QString special = spinBox->specialValueText().stripWhiteSpace();
		QString candidate = str.stripWhiteSpace();

		if ( special.startsWith(candidate) ) {
		    if ( candidate.length() == special.length() ) {
			state = Acceptable;
		    } else {
			state = Intermediate;
		    }
		}
	    }
	}
    }
    return state;
}

/*!
    \class QSpinBox
    \brief The QSpinBox class provides a spin box widget (spin button).

    \ingroup basic
    \mainclass

    QSpinBox allows the user to choose a value either by clicking the
    up/down buttons to increase/decrease the value currently displayed
    or by typing the value directly into the spin box. If the value is
    entered directly into the spin box, Enter (or Return) must be
    pressed to apply the new value. The value is usually an integer.

    Every time the value changes QSpinBox emits the valueChanged()
    signal. The current value can be fetched with value() and set
    with setValue().

    The spin box keeps the value within a numeric range, and to
    multiples of the lineStep() size (see QRangeControl for details).
    Clicking the up/down buttons or using the keyboard accelerator's
    up and down arrows will increase or decrease the current value in
    steps of size lineStep(). The minimum and maximum value and the
    step size can be set using one of the constructors, and can be
    changed later with setMinValue(), setMaxValue() and setLineStep().

    Most spin boxes are directional, but QSpinBox can also operate as
    a circular spin box, i.e. if the range is 0-99 and the current
    value is 99, clicking "up" will give 0. Use setWrapping() if you
    want circular behavior.

    The displayed value can be prepended and appended with arbitrary
    strings indicating, for example, currency or the unit of
    measurement. See setPrefix() and setSuffix(). The text in the spin
    box is retrieved with text() (which includes any prefix() and
    suffix()), or with cleanText() (which has no prefix(), no suffix()
    and no leading or trailing whitespace). currentValueText() returns
    the spin box's current value as text.

    Normally the spin box displays up and down arrows in the buttons.
    You can use setButtonSymbols() to change the display to show
    <b>+</b> and <b>-</b> symbols if you prefer. In either case the up
    and down arrow keys work as expected.

    It is often desirable to give the user a special (often default)
    choice in addition to the range of numeric values. See
    setSpecialValueText() for how to do this with QSpinBox.

    The default \l QWidget::focusPolicy() is StrongFocus.

    If using prefix(), suffix() and specialValueText() don't provide
    enough control, you can ignore them and subclass QSpinBox instead.

    QSpinBox can easily be subclassed to allow the user to input
    things other than an integer value as long as the allowed input
    can be mapped to a range of integers. This can be done by
    overriding the virtual functions mapValueToText() and
    mapTextToValue(), and setting another suitable validator using
    setValidator().

    For example, these functions could be changed so that the user
    provided values from 0.0 to 10.0, or -1 to signify 'Auto', while
    the range of integers used inside the program would be -1 to 100:

    \code
	class MySpinBox : public QSpinBox
	{
	    Q_OBJECT
	public:
	    ...

	    QString mapValueToText( int value )
	    {
		if ( value == -1 ) // special case
		    return QString( "Auto" );

		return QString( "%1.%2" ) // 0.0 to 10.0
		    .arg( value / 10 ).arg( value % 10 );
	    }

	    int mapTextToValue( bool *ok )
	    {
		if ( text() == "Auto" ) // special case
		    return -1;

		return (int) ( 10 * text().toFloat() ); // 0 to 100
	    }
	};
    \endcode

    <img src=qspinbox-m.png> <img src=qspinbox-w.png>

    \sa QScrollBar QSlider
    \link guibooks.html#fowler GUI Design Handbook: Spin Box \endlink
*/


/*!
    Constructs a spin box with the default QRangeControl range and
    step values. It is called \a name and has parent \a parent.

    \sa minValue(), maxValue(), setRange(), lineStep(), setSteps()
*/

QSpinBox::QSpinBox( QWidget * parent , const char *name )
    : QWidget( parent, name, WNoAutoErase ),
      QRangeControl()
{
    initSpinBox();
}


/*!
    Constructs a spin box that allows values from \a minValue to \a
    maxValue inclusive, with step amount \a step. The value is
    initially set to \a minValue.

    The spin box is called \a name and has parent \a parent.

    \sa minValue(), maxValue(), setRange(), lineStep(), setSteps()
*/

QSpinBox::QSpinBox( int minValue, int maxValue, int step, QWidget* parent,
		    const char* name )
    : QWidget( parent, name, WNoAutoErase ),
      QRangeControl( minValue, maxValue, step, step, minValue )
{
    initSpinBox();
}

/*
  \internal Initialization.
*/

void QSpinBox::initSpinBox()
{
    d = new QSpinBoxPrivate;

    d->controls = new QSpinWidget( this, "controls" );
    connect( d->controls, SIGNAL( stepUpPressed() ), SLOT( stepUp() ) );
    connect( d->controls, SIGNAL( stepDownPressed() ), SLOT( stepDown() ) );

    wrap = FALSE;
    edited = FALSE;
    d->selreq = FALSE;

    validate = new QSpinBoxValidator( this, "validator" );
    vi = new QLineEdit( this, "qt_spinbox_edit" );
    d->controls->setEditWidget( vi );
    vi->setValidator( validate );
    vi->installEventFilter( this );
    vi->setFrame( FALSE );
    setFocusProxy( vi );

    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed ) );
    setBackgroundMode( PaletteBackground, PaletteBase );

    updateDisplay();

    connect( vi, SIGNAL(textChanged(const QString&)), SLOT(textChanged()) );
}

/*!
    Destroys the spin box, freeing all memory and other resources.
*/

QSpinBox::~QSpinBox()
{
    delete d;
}


/*!
    \property QSpinBox::text
    \brief the spin box's text, including any prefix() and suffix()

    There is no default text.

    \sa value()
*/

QString QSpinBox::text() const
{
    return vi->text();
}



/*!
    \property QSpinBox::cleanText
    \brief the spin box's text with no prefix(), suffix() or leading
    or trailing whitespace.

    \sa text, prefix, suffix
*/

QString QSpinBox::cleanText() const
{
    QString s = QString(text()).stripWhiteSpace();
    if ( !prefix().isEmpty() ) {
	QString px = QString(prefix()).stripWhiteSpace();
	int len = px.length();
	if ( len && s.left(len) == px )  // Remove _only_ if it is the prefix
	    s.remove( (uint)0, len );
    }
    if ( !suffix().isEmpty() ) {
	QString sx = QString(suffix()).stripWhiteSpace();
	int len = sx.length();
	if ( len && s.right(len) == sx )  // Remove _only_ if it is the suffix
	    s.truncate( s.length() - len );
    }
    return s.stripWhiteSpace();
}


/*!
    \property QSpinBox::specialValueText
    \brief the special-value text

    If set, the spin box will display this text instead of a numeric
    value whenever the current value is equal to minVal(). Typical use
    is to indicate that this choice has a special (default) meaning.

    For example, if your spin box allows the user to choose the margin
    width in a print dialog and your application is able to
    automatically choose a good margin width, you can set up the spin
    box like this:
    \code
	QSpinBox marginBox( -1, 20, 1, parent, "marginBox" );
	marginBox->setSuffix( " mm" );
	marginBox->setSpecialValueText( "Auto" );
    \endcode
    The user will then be able to choose a margin width from 0-20
    millimeters or select "Auto" to leave it to the application to
    choose. Your code must then interpret the spin box value of -1 as
    the user requesting automatic margin width.

    All values are displayed with the prefix() and suffix() (if set),
    \e except for the special value, which only shows the special
    value text.

    To turn off the special-value text display, call this function
    with an empty string. The default is no special-value text, i.e.
    the numeric value is shown as usual.

    If no special-value text is set, specialValueText() returns
    QString::null.
*/

void QSpinBox::setSpecialValueText( const QString &text )
{
    specText = text;
    updateDisplay();
}


QString QSpinBox::specialValueText() const
{
    if ( specText.isEmpty() )
	return QString::null;
    else
	return specText;
}


/*!
    \property QSpinBox::prefix
    \brief the spin box's prefix

    The prefix is prepended to the start of the displayed value.
    Typical use is to display a unit of measurement or a currency
    symbol. For example:

    \code
	sb->setPrefix( "$" );
    \endcode

    To turn off the prefix display, set this property to an empty
    string. The default is no prefix. The prefix is not displayed for
    the minValue() if specialValueText() is not empty.

    If no prefix is set, prefix() returns QString::null.

    \sa suffix()
*/

void QSpinBox::setPrefix( const QString &text )
{
    pfix = text;
    updateDisplay();
}


QString QSpinBox::prefix() const
{
    if ( pfix.isEmpty() )
	return QString::null;
    else
	return pfix;
}


/*!
    \property QSpinBox::suffix
    \brief the suffix of the spin box

    The suffix is appended to the end of the displayed value. Typical
    use is to display a unit of measurement or a currency symbol. For
    example:

    \code
	sb->setSuffix( " km" );
    \endcode

    To turn off the suffix display, set this property to an empty
    string. The default is no suffix. The suffix is not displayed for
    the minValue() if specialValueText() is not empty.

    If no suffix is set, suffix() returns a QString::null.

    \sa prefix()
*/

void QSpinBox::setSuffix( const QString &text )
{
    sfix = text;
    updateDisplay();
}

QString QSpinBox::suffix() const
{
    if ( sfix.isEmpty() )
	return QString::null;
    else
	return sfix;
}


/*!
    \property QSpinBox::wrapping
    \brief whether it is possible to step the value from the highest
    value to the lowest value and vice versa

    By default, wrapping is turned off.

    If you have a range of 0..100 and wrapping is off when the user
    reaches 100 and presses the Up Arrow nothing will happen; but if
    wrapping is on the value will change from 100 to 0, then to 1,
    etc. When wrapping is on, navigating past the highest value takes
    you to the lowest and vice versa.

    \sa minValue, maxValue, setRange()
*/

void QSpinBox::setWrapping( bool on )
{
    wrap = on;
    updateDisplay();
}

bool QSpinBox::wrapping() const
{
    return wrap;
}

/*!
    \reimp
*/
QSize QSpinBox::sizeHint() const
{
    constPolish();
    QSize sz = vi->sizeHint();
    int h = sz.height();
    QFontMetrics fm( font() );
    int w = 35;
    int wx = fm.width( ' ' )*2;
    QString s;
    s = prefix() + ( (QSpinBox*)this )->mapValueToText( minValue() ) + suffix();
    w = QMAX( w, fm.width( s ) + wx);
    s = prefix() + ( (QSpinBox*)this )->mapValueToText( maxValue() ) + suffix();
    w = QMAX(w, fm.width( s ) + wx );
    if ( !specialValueText().isEmpty() ) {
	s = specialValueText();
	w = QMAX( w, fm.width( s ) + wx );
    }
    return style().sizeFromContents(QStyle::CT_SpinBox, this,
				    QSize( w + d->controls->downRect().width(),
					   h + style().pixelMetric( QStyle::PM_DefaultFrameWidth ) * 2).
				    expandedTo( QApplication::globalStrut() ));
}


/*!
    \reimp
*/
QSize QSpinBox::minimumSizeHint() const
{
    int w = vi->minimumSizeHint().width() + d->controls->downRect().width();
    int h = QMAX( vi->minimumSizeHint().height(), d->controls->minimumSizeHint().height() );
    return QSize( w, h );
}

// Does the layout of the lineedit and the buttons

void QSpinBox::arrangeWidgets()
{
    d->controls->arrange();
}

/*!
    \property QSpinBox::value
    \brief the value of the spin box

    \sa QRangeControl::setValue()
*/

void QSpinBox::setValue( int value )
{
    edited = FALSE; // we ignore anything entered and not yet interpreted
    QRangeControl::setValue( value );
    updateDisplay();
}

int QSpinBox::value() const
{
    QSpinBox * that = (QSpinBox *) this;
    if ( edited ) {
	that->edited = FALSE;  // avoid recursion
	that->interpretText();
    }
    return QRangeControl::value();
}


/*!
    Increases the spin box's value by one lineStep(), wrapping as
    necessary if wrapping() is TRUE. This is the same as clicking on
    the pointing-up button and can be used for keyboard accelerators,
    for example.

    \sa stepDown(), addLine(), lineStep(), setSteps(), setValue(), value()
*/

void QSpinBox::stepUp()
{
    if ( edited )
	interpretText();
    if ( wrapping() && ( value()+lineStep() > maxValue() || sumOutOfRange(value(), lineStep() ) ) ) {
	setValue( minValue() );
    } else {
	addLine();
    }
}


/*!
    Decreases the spin box's value one lineStep(), wrapping as
    necessary if wrapping() is TRUE. This is the same as clicking on
    the pointing-down button and can be used for keyboard
    accelerators, for example.

    \sa stepUp(), subtractLine(), lineStep(), setSteps(), setValue(), value()
*/

void QSpinBox::stepDown()
{
    if ( edited )
	interpretText();
    if ( wrapping() && ( value()-lineStep() < minValue() || sumOutOfRange(value(), -lineStep() ) ) ) {
	setValue( maxValue() );
    } else {
	subtractLine();
    }
}


/*!
    \fn void QSpinBox::valueChanged( int value )

    This signal is emitted every time the value of the spin box
    changes; the new value is passed in \a value. This signal will be
    emitted as a result of a call to setValue(), or because the user
    changed the value by using a keyboard accelerator or mouse click,
    etc.

    Note that the valueChanged() signal is emitted \e every time, not
    just for the "last" step; i.e. if the user clicks "up" three
    times, this signal is emitted three times.

    \sa value()
*/


/*!
    \fn void QSpinBox::valueChanged( const QString& valueText )

    \overload

    This signal is emitted whenever the valueChanged( int ) signal is
    emitted, i.e. every time the value of the spin box changes
    (whatever the cause, e.g. by setValue(), by a keyboard
    accelerator, by mouse clicks, etc.).

    The \a valueText parameter is the same string that is displayed in
    the edit field of the spin box.

    \sa value() prefix() suffix() specialValueText()
*/



/*!
    Intercepts and handles the events coming to the embedded QLineEdit
    that have special meaning for the QSpinBox. The object is passed
    as \a o and the event is passed as \a ev.
*/

bool QSpinBox::eventFilter( QObject* o, QEvent* ev )
{
    if (o != vi)
	return QWidget::eventFilter(o,ev);

    if ( ev->type() == QEvent::KeyPress ) {
	QKeyEvent* k = (QKeyEvent*)ev;

	bool retval = FALSE; // workaround for MSVC++ optimization bug
	if( (k->key() == Key_Tab) || (k->key() == Key_BackTab) ){
	    if ( k->state() & Qt::ControlButton )
		return FALSE;
	    if ( edited )
		interpretText();
	    qApp->sendEvent( this, ev );
	    retval = TRUE;
	} if ( k->key() == Key_Up ) {
	    stepUp();
	    retval = TRUE;
	} else if ( k->key() == Key_Down ) {
	    stepDown();
	    retval = TRUE;
	} else if ( k->key() == Key_Enter || k->key() == Key_Return ) {
	    interpretText();
	    return FALSE;
	}
	if ( retval )
	    return retval;
    } else if ( ev->type() == QEvent::FocusOut || ev->type() == QEvent::Hide ) {
	if ( edited ) {
	    interpretText();
	}
	return FALSE;
    }
    return FALSE;
}

/*!
    \reimp
 */
void QSpinBox::setEnabled( bool enabled )
{
    QWidget::setEnabled( enabled );
    updateDisplay();
}

/*!
    \reimp
*/
void QSpinBox::leaveEvent( QEvent* )
{
}


/*!
    \reimp
*/
void QSpinBox::resizeEvent( QResizeEvent* )
{
    d->controls->resize( width(), height() );
}

/*!
    \reimp
*/
#ifndef QT_NO_WHEELEVENT
void QSpinBox::wheelEvent( QWheelEvent * e )
{
    e->accept();
    static float offset = 0;
    static QSpinBox* offset_owner = 0;
    if (offset_owner != this) {
	offset_owner = this;
	offset = 0;
    }
    offset += -e->delta()/120;
    if (QABS(offset) < 1)
	return;
    int ioff = int(offset);
    int i;
    for (i=0; i<QABS(ioff); i++)
	offset > 0 ? stepDown() : stepUp();
    offset -= ioff;
}
#endif

/*!
    This virtual function is called by QRangeControl whenever the
    value has changed. The QSpinBox reimplementation updates the
    display and emits the valueChanged() signals; if you need
    additional processing, either reimplement this or connect to one
    of the valueChanged() signals.
*/

void QSpinBox::valueChange()
{
    d->selreq = hasFocus();
    updateDisplay();
    d->selreq = FALSE;
    emit valueChanged( value() );
    emit valueChanged( currentValueText() );
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::ValueChanged );
#endif
}


/*!
    This virtual function is called by QRangeControl whenever the
    range has changed. It adjusts the default validator and updates
    the display; if you need additional processing, you can
    reimplement this function.
*/

void QSpinBox::rangeChange()
{
    updateDisplay();
}


/*!
    Sets the validator to \a v. The validator controls what keyboard
    input is accepted when the user is editing in the value field. The
    default is to use a suitable QIntValidator.

    Use setValidator(0) to turn off input validation (entered input
    will still be kept within the spin box's range).
*/

void QSpinBox::setValidator( const QValidator* v )
{
    if ( vi )
	vi->setValidator( v );
}


/*!
    Returns the validator that constrains editing for this spin box if
    there is any; otherwise returns 0.

    \sa setValidator() QValidator
*/

const QValidator * QSpinBox::validator() const
{
    return vi ? vi->validator() : 0;
}

/*!
    Updates the contents of the embedded QLineEdit to reflect the
    current value using mapValueToText(). Also enables/disables the
    up/down push buttons accordingly.

    \sa mapValueToText()
*/
void QSpinBox::updateDisplay()
{
    vi->setUpdatesEnabled( FALSE );
    vi->setText( currentValueText() );
    if ( d->selreq && isVisible() && ( hasFocus() || vi->hasFocus() ) ) {
	selectAll();
    } else {
	if ( !suffix().isEmpty() && vi->text().endsWith(suffix()) )
	     vi->setCursorPosition( vi->text().length() - suffix().length() );
    }
    vi->setUpdatesEnabled( TRUE );
    vi->repaint( FALSE ); // immediate repaint needed for some reason
    edited = FALSE;

    bool upEnabled = isEnabled() && ( wrapping() || value() < maxValue() );
    bool downEnabled = isEnabled() && ( wrapping() || value() > minValue() );

    d->controls->setUpEnabled( upEnabled );
    d->controls->setDownEnabled( downEnabled );
    vi->setEnabled( isEnabled() );
    repaint( FALSE );
}


/*!
    QSpinBox calls this after the user has manually edited the
    contents of the spin box (i.e. by typing in the embedded
    QLineEdit, rather than using the up/down buttons/keys).

    The default implementation of this function interprets the new
    text using mapTextToValue(). If mapTextToValue() is successful, it
    changes the spin box's value; if not, the value is left unchanged.

    \sa editor()
*/

void QSpinBox::interpretText()
{
    bool ok = TRUE;
    bool done = FALSE;
    int newVal = 0;
    if ( !specialValueText().isEmpty() ) {
	QString s = text().stripWhiteSpace();
	QString t = specialValueText().stripWhiteSpace();
	if ( s == t ) {
	    newVal = minValue();
	    done = TRUE;
	}
    }
    if ( !done )
	newVal = mapTextToValue( &ok );
    if ( ok )
	setValue( newVal );
    updateDisplay(); // sometimes redundant
}


/*!
    Returns the geometry of the "up" button.
*/

QRect QSpinBox::upRect() const
{
    return d->controls->upRect();
}


/*!
    Returns the geometry of the "down" button.
*/

QRect QSpinBox::downRect() const
{
    return d->controls->downRect();
}


/*!
    Returns a pointer to the embedded QLineEdit.
*/

QLineEdit* QSpinBox::editor() const
{
    return vi;
}


/*!
    This slot is called whenever the user edits the spin box's text.
*/

void QSpinBox::textChanged()
{
    edited = TRUE; // this flag is cleared in updateDisplay()
}


/*!
    This virtual function is used by the spin box whenever it needs to
    display value \a v. The default implementation returns a string
    containing \a v printed in the standard way. Reimplementations may
    return anything. (See the example in the detailed description.)

    Note that Qt does not call this function for specialValueText()
    and that neither prefix() nor suffix() are included in the return
    value.

    If you reimplement this, you may also need to reimplement
    mapTextToValue().

    \sa updateDisplay(), mapTextToValue()
*/

QString QSpinBox::mapValueToText( int v )
{
    QString s;
    s.setNum( v );
    return s;
}


/*!
    This virtual function is used by the spin box whenever it needs to
    interpret text entered by the user as a value. The text is
    available as text() and as cleanText(), and this function must
    parse it if possible. If \a ok is not 0: if it parses the text
    successfully, \a *ok is set to TRUE; otherwise \a *ok is set to
    FALSE.

    Subclasses that need to display spin box values in a non-numeric
    way need to reimplement this function.

    Note that Qt handles specialValueText() separately; this function
    is only concerned with the other values.

    The default implementation tries to interpret the text() as an
    integer in the standard way and returns the integer value.

    \sa interpretText(), mapValueToText()
*/

int QSpinBox::mapTextToValue( bool* ok )
{
    QString s = text();
    int newVal = s.toInt( ok );
    if ( !(*ok) && !( !prefix() && !suffix() ) ) {// Try removing any pre/suffix
	s = cleanText();
	newVal = s.toInt( ok );
    }
    return newVal;
}


/*!
    Returns the full text calculated from the current value, including
    any prefix and suffix. If there is special value text and the
    value is minValue() the specialValueText() is returned.
*/

QString QSpinBox::currentValueText()
{
    QString s;
    if ( (value() == minValue()) && !specialValueText().isEmpty() ) {
	s = specialValueText();
    } else {
	s = prefix();
	s.append( mapValueToText( value() ) );
	s.append( suffix() );
    }
    return s;
}

/*!
    \reimp
*/

void QSpinBox::styleChange( QStyle& old )
{
    arrangeWidgets();
    QWidget::styleChange( old );
}


/*!
    \enum QSpinBox::ButtonSymbols

    This enum type determines what the buttons in a spin box show.

    \value UpDownArrows the buttons show little arrows in the classic
    style.

    \value PlusMinus the buttons show <b>+</b> and <b>-</b> symbols.

    \sa QSpinBox::buttonSymbols
*/

/*!
    \property QSpinBox::buttonSymbols

    \brief the current button symbol mode

    The possible values can be either \c UpDownArrows or \c PlusMinus.
    The default is \c UpDownArrows.

    \sa ButtonSymbols
*/

void QSpinBox::setButtonSymbols( ButtonSymbols newSymbols )
{
    if ( buttonSymbols() == newSymbols )
	return;

    switch ( newSymbols ) {
    case UpDownArrows:
	d->controls->setButtonSymbols( QSpinWidget::UpDownArrows );
	break;
    case PlusMinus:
	d->controls->setButtonSymbols( QSpinWidget::PlusMinus );
	break;
    }
    //    repaint( FALSE );
}

QSpinBox::ButtonSymbols QSpinBox::buttonSymbols() const
{
    switch( d->controls->buttonSymbols() ) {
    case QSpinWidget::UpDownArrows:
	return UpDownArrows;
    case QSpinWidget::PlusMinus:
	return PlusMinus;
    }
    return UpDownArrows;
}

/*!
    \property QSpinBox::minValue

    \brief the minimum value of the spin box

    When setting this property, \l QSpinBox::maxValue is adjusted, if
    necessary, to ensure that the range remains valid.

    \sa setRange() setSpecialValueText()
*/

int QSpinBox::minValue() const
{
    return QRangeControl::minValue();
}

void QSpinBox::setMinValue( int minVal )
{
    QRangeControl::setMinValue( minVal );
}

/*!
    \property QSpinBox::maxValue
    \brief the maximum value of the spin box

    When setting this property, \l QSpinBox::minValue is adjusted, if
    necessary, to ensure that the range remains valid.

    \sa setRange() setSpecialValueText()
*/

int QSpinBox::maxValue() const
{
    return QRangeControl::maxValue();
}

void QSpinBox::setMaxValue( int maxVal )
{
    QRangeControl::setMaxValue( maxVal );
}

/*!
    \property QSpinBox::lineStep
    \brief the line step

    When the user uses the arrows to change the spin box's value the
    value will be incremented/decremented by the amount of the line
    step.

    The setLineStep() function calls the virtual stepChange() function
    if the new line step is different from the previous setting.

    \sa QRangeControl::setSteps() setRange()
*/

int QSpinBox::lineStep() const
{
    return QRangeControl::lineStep();
}

void QSpinBox::setLineStep( int i )
{
    setSteps( i, pageStep() );
}

/*!
    Selects all the text in the spin box's editor.
*/

void QSpinBox::selectAll()
{
    int overhead = prefix().length() + suffix().length();
    if ( !overhead || currentValueText() == specialValueText() ) {
	vi->selectAll();
    } else {
	vi->setSelection( prefix().length(), vi->text().length() - overhead );
    }
}

#endif
