/****************************************************************************
** $Id: qlcdnumber.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QLCDNumber class
**
** Created : 940518
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

#include "qlcdnumber.h"
#ifndef QT_NO_LCDNUMBER
#include "qbitarray.h"
#include "qpainter.h"


/*!
    \class QLCDNumber qlcdnumber.h

    \brief The QLCDNumber widget displays a number with LCD-like digits.

    \ingroup basic
    \mainclass

    It can display a number in just about any size. It can display
    decimal, hexadecimal, octal or binary numbers. It is easy to
    connect to data sources using the display() slot, which is
    overloaded to take any of five argument types.

    There are also slots to change the base with setMode() and the
    decimal point with setSmallDecimalPoint().

    QLCDNumber emits the overflow() signal when it is asked to display
    something beyond its range. The range is set by setNumDigits(),
    but setSmallDecimalPoint() also influences it. If the display is
    set to hexadecimal, octal or binary, the integer equivalent of the
    value is displayed.

    These digits and other symbols can be shown: 0/O, 1, 2, 3, 4, 5/S,
    6, 7, 8, 9/g, minus, decimal point, A, B, C, D, E, F, h, H, L, o,
    P, r, u, U, Y, colon, degree sign (which is specified as single
    quote in the string) and space. QLCDNumber substitutes spaces for
    illegal characters.

    It is not possible to retrieve the contents of a QLCDNumber
    object, although you can retrieve the numeric value with value().
    If you really need the text, we recommend that you connect the
    signals that feed the display() slot to another slot as well and
    store the value there.

    Incidentally, QLCDNumber is the very oldest part of Qt, tracing
    back to a BASIC program on the \link
    http://www.nvg.ntnu.no/sinclair/computers/zxspectrum/zxspectrum.htm
    Sinclair Spectrum\endlink.

    <img src=qlcdnum-m.png> <img src=qlcdnum-w.png>

    \sa QLabel, QFrame
*/

/*!
    \enum QLCDNumber::Mode

    This type determines how numbers are shown.

    \value Hex  Hexadecimal
    \value Dec  Decimal
    \value Oct  Octal
    \value Bin  Binary

    If the display is set to hexadecimal, octal or binary, the integer
    equivalent of the value is displayed.
*/

/*!
    \enum QLCDNumber::SegmentStyle

    This type determines the visual appearance of the QLCDNumber
    widget.

    \value Outline gives raised segments filled with the background brush.
    \value Filled gives raised segments filled with the foreground brush.
    \value Flat gives flat segments filled with the foreground brush.
*/



/*!
    \fn void QLCDNumber::overflow()

    This signal is emitted whenever the QLCDNumber is asked to display
    a too-large number or a too-long string.

    It is never emitted by setNumDigits().
*/


static QString int2string( int num, int base, int ndigits, bool *oflow )
{
    QString s;
    bool negative;
    if ( num < 0 ) {
        negative = TRUE;
        num      = -num;
    } else {
        negative = FALSE;
    }
    switch( base ) {
        case QLCDNumber::HEX:
            s.sprintf( "%*x", ndigits, num );
            break;
        case QLCDNumber::DEC:
            s.sprintf( "%*i", ndigits, num );
            break;
        case QLCDNumber::OCT:
            s.sprintf( "%*o", ndigits, num );
            break;
        case QLCDNumber::BIN:
            {
                char buf[42];
                char *p = &buf[41];
                uint n = num;
                int len = 0;
                *p = '\0';
                do {
                    *--p = (char)((n&1)+'0');
                    n >>= 1;
                    len++;
                } while ( n != 0 );
                len = ndigits - len;
                if ( len > 0 )
                s.fill( ' ', len );
                s += QString::fromLatin1(p);
            }
            break;
    }
    if ( negative ) {
        for ( int i=0; i<(int)s.length(); i++ ) {
            if ( s[i] != ' ' ) {
                if ( i != 0 ) {
                    s[i-1] = '-';
                } else {
                    s.insert( 0, '-' );
                }
                break;
            }
        }
    }
    if ( oflow )
        *oflow = (int)s.length() > ndigits;
    return s;
}


static QString double2string( double num, int base, int ndigits, bool *oflow )
{
    QString s;
    if ( base != QLCDNumber::DEC ) {
        bool of = num >= 2147483648.0 || num < -2147483648.0;
        if ( of ) {                             // oops, integer overflow
            if ( oflow )
                *oflow = TRUE;
            return s;
        }
        s = int2string( (int)num, base, ndigits, 0 );
    } else {                                    // decimal base
        int nd = ndigits;
        do {
            s.sprintf( "%*.*g", ndigits, nd, num );
            int i = s.find('e');
            if ( i > 0 && s[i+1]=='+' ) {
                s[i] = ' ';
                s[i+1] = 'e';
            }
        } while (nd-- && (int)s.length() > ndigits);
    }
    if ( oflow )
        *oflow = (int)s.length() > ndigits;
    return s;
}


static const char *getSegments( char ch )               // gets list of segments for ch
{
    static const char segments[30][8] =
       { { 0, 1, 2, 4, 5, 6,99, 0},             // 0    0 / O
         { 2, 5,99, 0, 0, 0, 0, 0},             // 1    1
         { 0, 2, 3, 4, 6,99, 0, 0},             // 2    2
         { 0, 2, 3, 5, 6,99, 0, 0},             // 3    3
         { 1, 2, 3, 5,99, 0, 0, 0},             // 4    4
         { 0, 1, 3, 5, 6,99, 0, 0},             // 5    5 / S
         { 0, 1, 3, 4, 5, 6,99, 0},             // 6    6
         { 0, 2, 5,99, 0, 0, 0, 0},             // 7    7
         { 0, 1, 2, 3, 4, 5, 6,99},             // 8    8
         { 0, 1, 2, 3, 5, 6,99, 0},             // 9    9 / g
         { 3,99, 0, 0, 0, 0, 0, 0},             // 10   -
         { 7,99, 0, 0, 0, 0, 0, 0},             // 11   .
         { 0, 1, 2, 3, 4, 5,99, 0},             // 12   A
         { 1, 3, 4, 5, 6,99, 0, 0},             // 13   B
         { 0, 1, 4, 6,99, 0, 0, 0},             // 14   C
         { 2, 3, 4, 5, 6,99, 0, 0},             // 15   D
         { 0, 1, 3, 4, 6,99, 0, 0},             // 16   E
         { 0, 1, 3, 4,99, 0, 0, 0},             // 17   F
         { 1, 3, 4, 5,99, 0, 0, 0},             // 18   h
         { 1, 2, 3, 4, 5,99, 0, 0},             // 19   H
         { 1, 4, 6,99, 0, 0, 0, 0},             // 20   L
         { 3, 4, 5, 6,99, 0, 0, 0},             // 21   o
         { 0, 1, 2, 3, 4,99, 0, 0},             // 22   P
         { 3, 4,99, 0, 0, 0, 0, 0},             // 23   r
         { 4, 5, 6,99, 0, 0, 0, 0},             // 24   u
         { 1, 2, 4, 5, 6,99, 0, 0},             // 25   U
         { 1, 2, 3, 5, 6,99, 0, 0},             // 26   Y
         { 8, 9,99, 0, 0, 0, 0, 0},             // 27   :
         { 0, 1, 2, 3,99, 0, 0, 0},             // 28   '
         {99, 0, 0, 0, 0, 0, 0, 0} };           // 29   empty

    if (ch >= '0' && ch <= '9')
        return segments[ch - '0'];
    if (ch >= 'A' && ch <= 'F')
        return segments[ch - 'A' + 12];
    if (ch >= 'a' && ch <= 'f')
        return segments[ch - 'a' + 12];

    int n;
    switch ( ch ) {
        case '-':
            n = 10;  break;
        case 'O':
            n = 0;   break;
        case 'g':
            n = 9;   break;
        case '.':
            n = 11;  break;
        case 'h':
            n = 18;  break;
        case 'H':
            n = 19;  break;
        case 'l':
        case 'L':
            n = 20;  break;
        case 'o':
            n = 21;  break;
        case 'p':
        case 'P':
            n = 22;  break;
        case 'r':
        case 'R':
            n = 23;  break;
        case 's':
        case 'S':
            n = 5;   break;
        case 'u':
            n = 24;  break;
        case 'U':
            n = 25;  break;
        case 'y':
        case 'Y':
            n = 26;  break;
        case ':':
            n = 27;  break;
        case '\'':
            n = 28;  break;
        default:
            n = 29;  break;
    }
    return segments[n];
}


/*!
    Constructs an LCD number, sets the number of digits to 5, the base
    to decimal, the decimal point mode to 'small' and the frame style
    to a raised box. The segmentStyle() is set to \c Outline.

    The \a parent and \a name arguments are passed to the QFrame
    constructor.

    \sa setNumDigits(), setSmallDecimalPoint()
*/

QLCDNumber::QLCDNumber( QWidget *parent, const char *name )
        : QFrame( parent, name )
{
    ndigits = 5;
    init();
}


/*!
    Constructs an LCD number, sets the number of digits to \a
    numDigits, the base to decimal, the decimal point mode to 'small'
    and the frame style to a raised box. The segmentStyle() is set to
    \c Outline.

    The \a parent and \a name arguments are passed to the QFrame
    constructor.

    \sa setNumDigits(), setSmallDecimalPoint()
*/

QLCDNumber::QLCDNumber( uint numDigits, QWidget *parent, const char *name )
        : QFrame( parent, name )
{
    ndigits = numDigits;
    init();
}

/*!
  \internal
*/

void QLCDNumber::init()
{
    setFrameStyle( QFrame::Box | QFrame::Raised );
    val        = 0;
    base       = DEC;
    smallPoint = FALSE;
    setNumDigits( ndigits );
    setSegmentStyle( Outline );
    d = 0;
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
}

/*!
    Destroys the LCD number.
*/

QLCDNumber::~QLCDNumber()
{
}


/*!
    \property QLCDNumber::numDigits
    \brief the current number of digits displayed

    Corresponds to the current number of digits. If \l
    QLCDNumber::smallDecimalPoint is FALSE, the decimal point occupies
    one digit position.

    \sa numDigits, smallDecimalPoint
*/

void QLCDNumber::setNumDigits( int numDigits )
{
    if ( numDigits > 99 ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QLCDNumber::setNumDigits: (%s) Max 99 digits allowed",
                 name( "unnamed" ) );
#endif
        numDigits = 99;
    }
    if (numDigits < 0 ) {
#if defined(QT_CHECK_RANGE)
        qWarning( "QLCDNumber::setNumDigits: (%s) Min 0 digits allowed",
                 name( "unnamed" ) );
#endif
        numDigits = 0;
    }
    if ( digitStr.isNull() ) {                  // from constructor
        ndigits = numDigits;
        digitStr.fill( ' ', ndigits );
        points.fill( 0, ndigits );
        digitStr[ndigits - 1] = '0';            // "0" is the default number
    } else {
        bool doDisplay = ndigits == 0;
        if ( numDigits == ndigits )             // no change
            return;
        register int i;
        int dif;
        if ( numDigits > ndigits ) {            // expand
            dif = numDigits - ndigits;
            QString buf;
            buf.fill( ' ', dif );
            digitStr.insert( 0, buf );
            points.resize( numDigits );
            for ( i=numDigits-1; i>=dif; i-- )
                points.setBit( i, points.testBit(i-dif) );
            for ( i=0; i<dif; i++ )
                points.clearBit( i );
        } else {                                        // shrink
            dif = ndigits - numDigits;
            digitStr = digitStr.right( numDigits );
            QBitArray tmpPoints = points.copy();
            points.resize( numDigits );
            for ( i=0; i<(int)numDigits; i++ )
                points.setBit( i, tmpPoints.testBit(i+dif) );
        }
        ndigits = numDigits;
        if ( doDisplay )
            display( value() );
        update();
    }
}


/*!
    \overload

    Returns TRUE if \a num is too big to be displayed in its entirety;
    otherwise returns FALSE.

    \sa display(), numDigits(), smallDecimalPoint()
*/

bool QLCDNumber::checkOverflow( int num ) const
{
    bool of;
    int2string( num, base, ndigits, &of );
    return of;
}


/*!
    Returns TRUE if \a num is too big to be displayed in its entirety;
    otherwise returns FALSE.

    \sa display(), numDigits(), smallDecimalPoint()
*/

bool QLCDNumber::checkOverflow( double num ) const
{
    bool of;
    double2string( num, base, ndigits, &of );
    return of;
}


/*!
    \property QLCDNumber::mode
    \brief the current display mode (number base)

    Corresponds to the current display mode, which is one of \c BIN,
    \c OCT, \c DEC (the default) and \c HEX. \c DEC mode can display
    floating point values, the other modes display the integer
    equivalent.

    \sa smallDecimalPoint(), setHexMode(), setDecMode(), setOctMode(), setBinMode()
*/

QLCDNumber::Mode QLCDNumber::mode() const
{
    return (QLCDNumber::Mode) base;
}

void QLCDNumber::setMode( Mode m )
{
    base = m;

    display( val );
}


/*!
    \property QLCDNumber::value
    \brief the displayed value

    This property corresponds to the current value displayed by the
    LCDNumber.

    If the displayed value is not a number, the property has a value
    of 0.
*/

double QLCDNumber::value() const
{
    return val;
}

/*!
    \overload

    Displays the number \a num.
*/
void QLCDNumber::display( double num )
{
    val = num;
    bool of;
    QString s = double2string( num, base, ndigits, &of );
    if ( of )
        emit overflow();
    else
        internalSetString( s );
}

/*!
    \property QLCDNumber::intValue
    \brief the displayed value rounded to the nearest integer

    This property corresponds to the nearest integer to the current
    value displayed by the LCDNumber. This is the value used for
    hexadecimal, octal and binary modes.

    If the displayed value is not a number, the property has a value
    of 0.
*/
int QLCDNumber::intValue() const
{
    return (int)(val < 0 ? val - 0.5 : val + 0.5);
}


/*!
    \overload

    Displays the number \a num.
*/
void QLCDNumber::display( int num )
{
    val = (double)num;
    bool of;
    QString s = int2string( num, base, ndigits, &of );
    if ( of )
        emit overflow();
    else
        internalSetString( s );
}


/*!
    Displays the number represented by the string \a s.

    This version of the function disregards mode() and
    smallDecimalPoint().

    These digits and other symbols can be shown: 0/O, 1, 2, 3, 4, 5/S,
    6, 7, 8, 9/g, minus, decimal point, A, B, C, D, E, F, h, H, L, o,
    P, r, u, U, Y, colon, degree sign (which is specified as single
    quote in the string) and space. QLCDNumber substitutes spaces for
    illegal characters.
*/

void QLCDNumber::display( const QString &s )
{
    val = 0;
    bool ok = FALSE;
    double v = s.toDouble( &ok );
    if ( ok )
        val = v;
    internalSetString( s );
}

/*!
    Calls setMode( HEX ). Provided for convenience (e.g. for
    connecting buttons to it).

    \sa setMode(), setDecMode(), setOctMode(), setBinMode(), mode()
*/

void QLCDNumber::setHexMode()
{
    setMode( HEX );
}


/*!
    Calls setMode( DEC ). Provided for convenience (e.g. for
    connecting buttons to it).

    \sa setMode(), setHexMode(), setOctMode(), setBinMode(), mode()
*/

void QLCDNumber::setDecMode()
{
    setMode( DEC );
}


/*!
    Calls setMode( OCT ). Provided for convenience (e.g. for
    connecting buttons to it).

    \sa setMode(), setHexMode(), setDecMode(), setBinMode(), mode()
*/

void QLCDNumber::setOctMode()
{
    setMode( OCT );
}


/*!
    Calls setMode( BIN ). Provided for convenience (e.g. for
    connecting buttons to it).

    \sa setMode(), setHexMode(), setDecMode(), setOctMode(), mode()
*/

void QLCDNumber::setBinMode()
{
    setMode( BIN );
}


/*!
    \property QLCDNumber::smallDecimalPoint
    \brief the style of the decimal point

    If TRUE the decimal point is drawn between two digit positions.
    Otherwise it occupies a digit position of its own, i.e. is drawn
    in a digit position. The default is FALSE.

    The inter-digit space is made slightly wider when the decimal
    point is drawn between the digits.

    \sa mode
*/

void QLCDNumber::setSmallDecimalPoint( bool b )
{
    smallPoint = b;
}


/*!
    Draws the LCD number using painter \a p. This function is called
    from QFrame::paintEvent().
*/


void QLCDNumber::drawContents( QPainter *p )
{
    if ( smallPoint )
        drawString( digitStr, *p, &points, FALSE );
    else
        drawString( digitStr, *p, 0, FALSE );
}


/*!
  \internal
*/

void QLCDNumber::internalDisplay( const QString & )
{
    // Not used anymore
}

void QLCDNumber::internalSetString( const QString& s )
{
    QString buffer;
    int i;
    int len = s.length();
    QBitArray newPoints(ndigits);

    if ( !smallPoint ) {
        if ( len == ndigits )
            buffer = s;
        else
            buffer = s.right( ndigits ).rightJustify( ndigits, ' ' );
    } else {
        int  index = -1;
        bool lastWasPoint = TRUE;
        newPoints.clearBit(0);
        for ( i=0; i<len; i++ ) {
            if ( s[i] == '.' ) {
                if ( lastWasPoint ) {           // point already set for digit?
                    if ( index == ndigits - 1 ) // no more digits
                        break;
                    index++;
                    buffer[index] = ' ';        // 2 points in a row, add space
                }
                newPoints.setBit(index);        // set decimal point
                lastWasPoint = TRUE;
            } else {
                if ( index == ndigits - 1 )
                    break;
                index++;
                buffer[index] = s[i];
                newPoints.clearBit(index);      // decimal point default off
                lastWasPoint = FALSE;
            }
        }
        if ( index < ((int) ndigits) - 1 ) {
            for( i=index; i>=0; i-- ) {
                buffer[ndigits - 1 - index + i] = buffer[i];
                newPoints.setBit( ndigits - 1 - index + i,
                                   newPoints.testBit(i) );
            }
            for( i=0; i<ndigits-index-1; i++ ) {
                buffer[i] = ' ';
                newPoints.clearBit(i);
            }
        }
    }

    if ( buffer == digitStr )
        return;

    if ( backgroundMode() == FixedPixmap
         || colorGroup().brush( QColorGroup::Background ).pixmap() ) {
        digitStr = buffer;
        if ( smallPoint )
            points = newPoints;
        repaint( contentsRect() );
    } else {
        QPainter p( this );
        if ( !smallPoint )
            drawString( buffer, p );
        else
            drawString( buffer, p, &newPoints );
    }
}

/*!
  \internal
*/

void QLCDNumber::drawString( const QString &s, QPainter &p,
                             QBitArray *newPoints, bool newString )
{
    QPoint  pos;

    int digitSpace = smallPoint ? 2 : 1;
    int xSegLen    = width()*5/(ndigits*(5 + digitSpace) + digitSpace);
    int ySegLen    = height()*5/12;
    int segLen     = ySegLen > xSegLen ? xSegLen : ySegLen;
    int xAdvance   = segLen*( 5 + digitSpace )/5;
    int xOffset    = ( width() - ndigits*xAdvance + segLen/5 )/2;
    int yOffset    = ( height() - segLen*2 )/2;

    for ( int i=0;  i<ndigits; i++ ) {
        pos = QPoint( xOffset + xAdvance*i, yOffset );
        if ( newString )
            drawDigit( pos, p, segLen, s[i], digitStr[i].latin1() );
        else
            drawDigit( pos, p, segLen, s[i]);
        if ( newPoints ) {
            char newPoint = newPoints->testBit(i) ? '.' : ' ';
            if ( newString ) {
                char oldPoint = points.testBit(i) ? '.' : ' ';
                drawDigit( pos, p, segLen, newPoint, oldPoint );
            } else {
                drawDigit( pos, p, segLen, newPoint );
            }
        }
    }
    if ( newString ) {
        digitStr = s;
        if ( (int)digitStr.length() > ndigits )
            digitStr.truncate( ndigits );
        if ( newPoints )
            points = *newPoints;
    }
}


/*!
  \internal
*/

void QLCDNumber::drawDigit( const QPoint &pos, QPainter &p, int segLen,
                            char newCh, char oldCh )
{
// Draws and/or erases segments to change display of a single digit
// from oldCh to newCh

    char updates[18][2];        // can hold 2 times number of segments, only
                                // first 9 used if segment table is correct
    int  nErases;
    int  nUpdates;
    const char *segs;
    int  i,j;

    const char erase      = 0;
    const char draw       = 1;
    const char leaveAlone = 2;

    segs = getSegments(oldCh);
    for ( nErases=0; segs[nErases] != 99; nErases++ ) {
        updates[nErases][0] = erase;            // get segments to erase to
        updates[nErases][1] = segs[nErases];    // remove old char
    }
    nUpdates = nErases;
    segs = getSegments(newCh);
    for(i = 0 ; segs[i] != 99 ; i++) {
        for ( j=0;  j<nErases; j++ )
            if ( segs[i] == updates[j][1] ) {   // same segment ?
                updates[j][0] = leaveAlone;     // yes, already on screen
                break;
            }
        if ( j == nErases ) {                   // if not already on screen
            updates[nUpdates][0] = draw;
            updates[nUpdates][1] = segs[i];
            nUpdates++;
        }
    }
    for ( i=0; i<nUpdates; i++ ) {
        if ( updates[i][0] == draw )
            drawSegment( pos, updates[i][1], p, segLen );
        if (updates[i][0] == erase)
            drawSegment( pos, updates[i][1], p, segLen, TRUE );
    }
}


static void addPoint( QPointArray &a, const QPoint &p )
{
    uint n = a.size();
    a.resize( n + 1 );
    a.setPoint( n, p );
}

/*!
  \internal
*/

void QLCDNumber::drawSegment( const QPoint &pos, char segmentNo, QPainter &p,
                              int segLen, bool erase )
{
    QPoint pt = pos;
    int width = segLen/5;

    const QColorGroup & g = colorGroup();
    QColor lightColor,darkColor,fgColor;
    if ( erase ){
        lightColor = backgroundColor();
        darkColor  = lightColor;
        fgColor    = lightColor;
    } else {
        lightColor = g.light();
        darkColor  = g.dark();
        fgColor    = g.foreground();
    }

#define LINETO(X,Y) addPoint( a, QPoint(pt.x() + (X),pt.y() + (Y)))
#define LIGHT
#define DARK

    if ( fill ) {
        QPointArray a(0);

        //The following is an exact copy of the switch below.
        //don't make any changes here
        switch ( segmentNo ) {
        case 0 :
            p.moveTo(pt);
            LIGHT;
            LINETO(segLen - 1,0);
            DARK;
            LINETO(segLen - width - 1,width);
            LINETO(width,width);
            LINETO(0,0);
            break;
        case 1 :
            pt += QPoint(0 , 1);
            p.moveTo(pt);
            LIGHT;
            LINETO(width,width);
            DARK;
            LINETO(width,segLen - width/2 - 2);
            LINETO(0,segLen - 2);
            LIGHT;
            LINETO(0,0);
            break;
        case 2 :
            pt += QPoint(segLen - 1 , 1);
            p.moveTo(pt);
            DARK;
            LINETO(0,segLen - 2);
            LINETO(-width,segLen - width/2 - 2);
            LIGHT;
            LINETO(-width,width);
            LINETO(0,0);
            break;
        case 3 :
            pt += QPoint(0 , segLen);
            p.moveTo(pt);
            LIGHT;
            LINETO(width,-width/2);
            LINETO(segLen - width - 1,-width/2);
            LINETO(segLen - 1,0);
            DARK;
            if (width & 1) {            // adjust for integer division error
                LINETO(segLen - width - 3,width/2 + 1);
                LINETO(width + 2,width/2 + 1);
            } else {
                LINETO(segLen - width - 1,width/2);
                LINETO(width,width/2);
            }
            LINETO(0,0);
            break;
        case 4 :
            pt += QPoint(0 , segLen + 1);
            p.moveTo(pt);
            LIGHT;
            LINETO(width,width/2);
            DARK;
            LINETO(width,segLen - width - 2);
            LINETO(0,segLen - 2);
            LIGHT;
            LINETO(0,0);
            break;
        case 5 :
            pt += QPoint(segLen - 1 , segLen + 1);
            p.moveTo(pt);
            DARK;
            LINETO(0,segLen - 2);
            LINETO(-width,segLen - width - 2);
            LIGHT;
            LINETO(-width,width/2);
            LINETO(0,0);
            break;
        case 6 :
            pt += QPoint(0 , segLen*2);
            p.moveTo(pt);
            LIGHT;
            LINETO(width,-width);
            LINETO(segLen - width - 1,-width);
            LINETO(segLen - 1,0);
            DARK;
            LINETO(0,0);
            break;
        case 7 :
            if ( smallPoint )   // if smallpoint place'.' between other digits
                pt += QPoint(segLen + width/2 , segLen*2);
            else
                pt += QPoint(segLen/2 , segLen*2);
            p.moveTo(pt);
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
        case 8 :
            pt += QPoint(segLen/2 - width/2 + 1 , segLen/2 + width);
            p.moveTo(pt);
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
        case 9 :
            pt += QPoint(segLen/2 - width/2 + 1 , 3*segLen/2 + width);
            p.moveTo(pt);
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
#if defined(QT_CHECK_RANGE)
        default :
            qWarning( "QLCDNumber::drawSegment: (%s) Internal error."
                     "  Illegal segment id: %d\n",
                     name( "unnamed" ), segmentNo );
#endif
        }
        // End exact copy
        p.setPen( fgColor );
        p.setBrush( fgColor );
        p.drawPolygon( a );
        p.setBrush( NoBrush );

        pt = pos;
    }
#undef LINETO
#undef LIGHT
#undef DARK

#define LINETO(X,Y) p.lineTo(QPoint(pt.x() + (X),pt.y() + (Y)))
#define LIGHT p.setPen(lightColor)
#define DARK  p.setPen(darkColor)
    if ( shadow )
        switch ( segmentNo ) {
        case 0 :
            p.moveTo(pt);
            LIGHT;
            LINETO(segLen - 1,0);
            DARK;
            LINETO(segLen - width - 1,width);
            LINETO(width,width);
            LINETO(0,0);
            break;
        case 1 :
            pt += QPoint(0,1);
            p.moveTo(pt);
            LIGHT;
            LINETO(width,width);
            DARK;
            LINETO(width,segLen - width/2 - 2);
            LINETO(0,segLen - 2);
            LIGHT;
            LINETO(0,0);
            break;
        case 2 :
            pt += QPoint(segLen - 1 , 1);
            p.moveTo(pt);
            DARK;
            LINETO(0,segLen - 2);
            LINETO(-width,segLen - width/2 - 2);
            LIGHT;
            LINETO(-width,width);
            LINETO(0,0);
            break;
        case 3 :
            pt += QPoint(0 , segLen);
            p.moveTo(pt);
            LIGHT;
            LINETO(width,-width/2);
            LINETO(segLen - width - 1,-width/2);
            LINETO(segLen - 1,0);
            DARK;
            if (width & 1) {            // adjust for integer division error
                LINETO(segLen - width - 3,width/2 + 1);
                LINETO(width + 2,width/2 + 1);
            } else {
                LINETO(segLen - width - 1,width/2);
                LINETO(width,width/2);
            }
            LINETO(0,0);
            break;
        case 4 :
            pt += QPoint(0 , segLen + 1);
            p.moveTo(pt);
            LIGHT;
            LINETO(width,width/2);
            DARK;
            LINETO(width,segLen - width - 2);
            LINETO(0,segLen - 2);
            LIGHT;
            LINETO(0,0);
            break;
        case 5 :
            pt += QPoint(segLen - 1 , segLen + 1);
            p.moveTo(pt);
            DARK;
            LINETO(0,segLen - 2);
            LINETO(-width,segLen - width - 2);
            LIGHT;
            LINETO(-width,width/2);
            LINETO(0,0);
            break;
        case 6 :
            pt += QPoint(0 , segLen*2);
            p.moveTo(pt);
            LIGHT;
            LINETO(width,-width);
            LINETO(segLen - width - 1,-width);
            LINETO(segLen - 1,0);
            DARK;
            LINETO(0,0);
            break;
        case 7 :
            if ( smallPoint )   // if smallpoint place'.' between other digits
                pt += QPoint(segLen + width/2 , segLen*2);
            else
                pt += QPoint(segLen/2 , segLen*2);
            p.moveTo(pt);
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
        case 8 :
            pt += QPoint(segLen/2 - width/2 + 1 , segLen/2 + width);
            p.moveTo(pt);
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
        case 9 :
            pt += QPoint(segLen/2 - width/2 + 1 , 3*segLen/2 + width);
            p.moveTo(pt);
            DARK;
            LINETO(width,0);
            LINETO(width,-width);
            LIGHT;
            LINETO(0,-width);
            LINETO(0,0);
            break;
#if defined(QT_CHECK_RANGE)
        default :
            qWarning( "QLCDNumber::drawSegment: (%s) Internal error."
                     "  Illegal segment id: %d\n",
                     name( "unnamed" ), segmentNo );
#endif
        }

#undef LINETO
#undef LIGHT
#undef DARK
}



/*!
    \property QLCDNumber::segmentStyle
    \brief the style of the LCDNumber

    \table
    \header \i Style \i Result
    \row \i \c Outline
	 \i Produces raised segments filled with the background color
	    (this is the default).
    \row \i \c Filled
	 \i Produces raised segments filled with the foreground color.
    \row \i \c Flat
	 \i Produces flat segments filled with the foreground color.
    \endtable

    \c Outline and \c Filled will additionally use
    QColorGroup::light() and QColorGroup::dark() for shadow effects.
*/
void QLCDNumber::setSegmentStyle( SegmentStyle s )
{
    fill = ( s == Flat || s == Filled );
    shadow = ( s == Outline || s == Filled );
    update();
}

QLCDNumber::SegmentStyle QLCDNumber::segmentStyle() const
{
    Q_ASSERT( fill || shadow );
    if ( !fill && shadow )
        return Outline;
    if ( fill && shadow )
        return Filled;
    return Flat;
}


/*!\reimp
*/
QSize QLCDNumber::sizeHint() const
{
    return QSize( 10 + 9 * (numDigits() + (smallDecimalPoint() ? 0 : 1)), 23 );
}

#endif // QT_NO_LCDNUMBER
