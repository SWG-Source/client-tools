/****************************************************************************
** $Id: qrtlcodec.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QTextCodec class
**
** Created : 981015
**
** Copyright (C) 1998-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

#include "qrtlcodec.h"
#include <private/qtextengine_p.h>

#ifndef QT_NO_CODEC_HEBREW

// NOT REVISED

static const uchar unkn = '?'; // BLACK SQUARE (94) would be better

static const ushort heb_to_unicode[128] = {
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x00A0, 0xFFFD, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7,
    0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x203E,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7,
    0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x2017,
    0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7,
    0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
    0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7,
    0x05E8, 0x05E9, 0x05EA, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD
};

static const uchar unicode_to_heb_00[32] = {
    0xA0, unkn, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xD7, 0xAB, 0xAC, 0xAD, 0xAE, unkn,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xF7, 0xBB, 0xBC, 0xBD, 0xBE, unkn,
};

static const uchar unicode_to_heb_05[32] = {
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
    0xF8, 0xF9, 0xFA, unkn, unkn, unkn, unkn, unkn
};

static bool to8bit(const QChar ch, QCString *rstr)
{
    bool converted = FALSE;

    if( ch.isMark() ) return TRUE; // ignore marks for conversion

    if ( ch.row() ) {
	if ( ch.row() == 0x05 ) {
	    if ( ch.cell() > 0x91 )
		converted = TRUE;
	    // 0x0591 - 0x05cf: Hebrew punctuation... dropped
	    if ( ch.cell() >= 0xD0 )
		*rstr += (char)unicode_to_heb_05[ch.cell()- 0xD0];
	} else if ( ch.row() == 0x20 ) {
	    if ( ch.cell() == 0x3E ) {
		*rstr += (char)0xAF;
		converted = TRUE;
	    } else if ( ch.cell() == 0x17 ) {
		*rstr += (char)0xCF;
		converted = TRUE;
	    }
	} else {
	    converted = FALSE;
	}
    } else {
	if ( ch.cell() < 0x80 ) {
	    *rstr += (char)ch.cell();
	    converted = TRUE;
	} else if( ch.cell() < 0xA0 ) {
	    *rstr += (char)unicode_to_heb_00[ch.cell() - 0x80];
	    converted = TRUE;
	}
    }

    if(converted) return TRUE;

    // couldn't convert the char... lets try its decomposition
    QString d = ch.decomposition();
    if(d.isNull())
	return FALSE;

    int l = d.length();
    for (int i=0; i<l; i++) {
	const QChar ch = d[i];

	if(to8bit(ch, rstr))
	    converted = TRUE;
    }

    return converted;
}

#if 0
static QString run(const QString &input, unsigned int from, unsigned int to, QChar::Direction runDir)
{
    if ( to <= from )
	return QString::null;

    QString out;
    if ( runDir == QChar::DirR ) {
	const QChar *ch = input.unicode() + to - 1;
	int len = to - from;
	while (len--) {
	    out += *ch;
	    ch--;
	}
    } else {
	out = input.mid(from, to - from );
    }
    return out;
}

/*
  we might do better here, but I'm currently not sure if it's worth the effort. It will hopefully convert
  90% of the visually ordered Hebrew correctly.
*/
static QString reverseLine(const QString &str, unsigned int from, unsigned int to, QChar::Direction dir)
{
    QString out;

    if ( to <= from ) {
	out += str.at(from);
	return out;
    }

    // since we don't have embedding marks, we get around with bidi levels up to 2.

    // simple case: dir = RTL:
    // go through the line from right to left, and reverse all continuous Hebrew strings.
    if ( dir == QChar::DirR ) {
	unsigned int pos = to;
	to = from;
	from = pos;
	QChar::Direction runDir = QChar::DirON;

	while ( pos > to ) {
	    QChar::Direction d = str.at(pos).direction();
	    switch ( d ) {
		case QChar::DirL:
		case QChar::DirAN:
		case QChar::DirEN:
		    if ( runDir != QChar::DirL ) {
			out += run( str, pos, from, runDir );
			from = pos - 1;
		    }
		    runDir = QChar::DirL;
		    break;
		case QChar::DirON:
		    if ( runDir == QChar::DirON ) {
			runDir = QChar::DirR;
			break;
		    }
		    // fall through
		case QChar::DirR:
		    if ( runDir != QChar::DirR ) {
			out += run( str, pos, from, runDir );
			from = pos - 1;
		    }
		    runDir = QChar::DirR;
		default:
		    break;
	    }
	    pos--;
	}
	out += run( str, pos, from, runDir );
    } else {
	// basicDir == DirL. A bit more complicated, as we might need to reverse two times for numbers.
	unsigned int pos = from;
	QChar::Direction runDir = QChar::DirON;

	// first reversing. Ignore numbers
	while ( pos < to ) {
	    QChar::Direction d = str.at(pos).direction();
	    switch ( d ) {
		case QChar::DirL:
		    if ( runDir != QChar::DirL && runDir != QChar::DirON ) {
			out += run( str, from, pos, runDir );
			qDebug( "out = %s", out.latin1() );
			from = pos;
		    }
		    runDir = QChar::DirL;
		    break;
		case QChar::DirON:
		    if ( runDir == QChar::DirON ) {
			runDir = QChar::DirL;
			break;
		    }
		    // fall through
		case QChar::DirR:
		case QChar::DirAN:
		case QChar::DirEN:
		    if ( runDir != QChar::DirR && runDir != QChar::DirON ) {
			out += run( str, from, pos, runDir );
			qDebug( "out = %s", out.latin1() );
			from = pos;
		    }
		    runDir = QChar::DirR;
		default:
		    break;
	    }
	    pos++;
	}
	out += run( str, from, pos, runDir );
	qDebug( "out = %s", out.latin1() );
	// second reversing for numbers
	QString in = out;
	out = "";
	pos = 0;
	from = 0;
	to = in.length() - 1;
	runDir = QChar::DirON;
	while ( pos < to ) {
	    QChar::Direction d = str.at(pos).direction();
	    switch ( d ) {
		case QChar::DirL:
		case QChar::DirON:
		case QChar::DirR:
		    if ( runDir == QChar::DirEN && runDir != QChar::DirON ) {
			out += run( in, from, pos, QChar::DirR ); //DirR ensures reversing
			qDebug( "out = %s", out.latin1() );
			runDir = QChar::DirR;
			from = pos;
		    }
		    runDir = QChar::DirL;
		    break;
		case QChar::DirAN:
		case QChar::DirEN:
		    if ( runDir != QChar::DirEN && runDir != QChar::DirON ) {
			out += in.mid(from, pos-from+1);
			qDebug( "out = %s", out.latin1() );
			from = pos;
		    }
		    runDir = QChar::DirEN;
		default:
		    break;
	    }
	    pos++;
	}
	out += run( str, from, pos, runDir );

    }
    return out;
}
#endif

/* this function assuems the QString is still visually ordered.
 * Finding the basic direction of the text is not easy in this case, since
 * a string like "my friend MOLAHS" could (in logical order) mean aswell
 * "SHALOM my friend" or "my friend SHALOM", depending on the basic direction
 * one assumes for the text.
 *
 * So this function uses some heuristics to find the right answer...
 */
static QChar::Direction findBasicDirection(QString str)
{
    unsigned int pos;
    unsigned int len = str.length();
    QChar::Direction dir1 = QChar::DirON;
    QChar::Direction dir2 = QChar::DirON;

    unsigned int startLine = 0;
    // If the visual representation of the first line starts and ends with the same
    // directionality, we know the answer.
    pos = 0;
    while (pos < len) {
	if ( str.at(pos) == '\n' )
	    startLine = pos;
	if (str.at(pos).direction() < 2) { // DirR or DirL
	    dir1 = str.at(pos).direction();
	    break;
	}
	pos++;
    }

    if( pos == len ) // no directional chars, assume QChar::DirL
	return QChar::DirL;

    // move to end of line
    while( pos < len && str.at(pos) != '\n' )
	pos++;

    while (pos > startLine) {
	if (str.at(pos).direction() < 2) { // DirR or DirL
	    dir2 = str.at(pos).direction();
	    break;
	}
	pos--;
    }

    // both are the same, so we have the direction!
    if ( dir1 == dir2 ) return dir1;

    // guess with the help of punktuation marks...
    // if the sentence ends with a punktuation, we should have a mark
    // at one side of the text...

    pos = 0;
    while (pos < len-1 ) {
	if(str.at(pos).category() == QChar::Punctuation_Other) {
	    if( str.at(pos) != (char)0xbf && str.at(pos) != (char)0xa1 ) // spanish inverted question and exclamation mark
		if( str.at(pos+1).direction() < 2 ) return QChar::DirR;
	}
	pos++;
    }

    pos = len;
    while (pos < 1 && str.at(pos).direction() < 2 ) {
	if(str.at(pos).category() == QChar::Punctuation_Other) {
	    if( str.at(pos-1).direction() < 2 ) return QChar::DirL;
	}
	pos--;
    }

    // don't know try DirR...
    return QChar::DirR;
}


/*!
    \class QHebrewCodec qrtlcodec.h
    \reentrant
    \ingroup i18n

    \brief The QHebrewCodec class provides conversion to and from
    visually ordered Hebrew.

    Hebrew as a semitic language is written from right to left.
    Because older computer systems couldn't handle reordering a string
    so that the first letter appears on the right, many older
    documents were encoded in visual order, so that the first letter
    of a line is the rightmost one in the string.

    In contrast to this, Unicode defines characters to be in logical
    order (the order you would read the string). This codec tries to
    convert visually ordered Hebrew (8859-8) to Unicode. This might
    not always work perfectly, because reversing the \e bidi
    (bi-directional) algorithm that transforms from logical to visual
    order is non-trivial.

    Transformation from Unicode to visual Hebrew (8859-8) is done
    using the bidi algorithm in Qt, and will produce correct results,
    so long as the codec is given the text a whole paragraph at a
    time. Places where newlines are supposed to go can be indicated by
    a newline character ('\n'). Note that these newline characters
    change the reordering behaviour of the algorithm, since the bidi
    reordering only takes place within one line of text, whereas
    line breaks are determined in visual order.

    Visually ordered Hebrew is still used quite often in some places,
    mainly in email communication (since most email programs still
    don't understand logically ordered Hebrew) and on web pages. The
    use on web pages is rapidly decreasing, due to the availability of
    browsers that correctly support logically ordered Hebrew.

    This codec has the name "iso8859-8". If you don't want any bidi
    reordering to happen during conversion, use the "iso8859-8-i"
    codec, which assumes logical order for the 8-bit string.
*/

/*! \reimp */
int QHebrewCodec::mibEnum() const
{
    return 11;
}

/*! \reimp */
const char* QHebrewCodec::name() const
{
    return "ISO 8859-8";
}

/*!
    Returns the codec's mime name.
*/
const char* QHebrewCodec::mimeName() const
{
    return "ISO-8859-8";
}

static QString visualOrder(QString logical, QChar::Direction basicDir)
{
    logical.replace(QChar('\n'), QChar(0x2028));

    QTextEngine e(logical, 0);
    e.direction = basicDir;
    e.itemize();
    Q_UINT8 l[256];
    Q_UINT8 *levels = l;
    int vo[256];
    int *visualOrder = vo;
    int nitems = e.items.size();
    if (nitems > 255) {
	levels = new Q_UINT8[nitems];
	visualOrder = new int[nitems];
    }
    int i;
    for (i = 0; i < nitems; ++i) {
	//qDebug("item %d bidiLevel=%d", i,  e.items[i].analysis.bidiLevel);
	levels[i] = e.items[i].analysis.bidiLevel;
    }
    e.bidiReorder(nitems, levels, visualOrder);

    QString visual;
    for (i = 0; i < nitems; ++i) {
	QScriptItem &si = e.items[visualOrder[i]];
	QString sub = logical.mid(si.position, e.length(visualOrder[i]));
	if (si.analysis.bidiLevel % 2) {
	    // reverse sub
	    QChar *a = (QChar *)sub.unicode();
	    QChar *b = a + sub.length() - 1;
	    while (a < b) {
		QChar tmp = *a;
		*a = *b;
		*b = tmp;
		++a;
		--b;
	    }
	    a = (QChar *)sub.unicode();
	    b = a + sub.length();
	    while (a<b) {
		*a = a->mirroredChar();
		++a;
	    }
	}
	visual += sub;
    }
    // replace Unicode newline back with  \n to compare.
    visual.replace(QChar(0x2028), QChar('\n'));
    if (l != levels) {
	delete [] levels;
	delete [] visualOrder;
    }
    return visual;
}

/*!
    \reimp

    Since Hebrew (and Arabic) is written from left to right, but
    iso8859-8 assumes visual ordering (as opposed to the logical
    ordering of Unicode), we must reverse the order of the input
    string (the first \a len characters of \a chars) to put it into
    logical order.

    One problem is that the basic text direction is unknown. So this
    function uses some heuristics to guess it, and if it can't guess
    the right one, it assumes, the basic text direction is right to
    left.

    This behaviour can be overridden, by putting a control character
    at the beginning of the text to indicate which basic text
    direction to use. If the basic text direction is left-to-right,
    the control character should be (uchar) 0xFE. For right-to-left it
    should be 0xFF. Both characters are undefined in the iso 8859-8
    charset.

    Example: A visually ordered string "english WERBEH american" would
    be recognized as having a basic left to right direction. So the
    logically ordered QString would be "english HEBREW american".

    By prepending a (uchar)0xFF at the start of the string,
    QHebrewCodec::toUnicode() would use a basic text direction of
    right to left, and the string would thus become "american HEBREW
    english".
*/
QString QHebrewCodec::toUnicode(const char* chars, int len ) const
{
    QString r;
    const unsigned char * c = (const unsigned char *)chars;
    QChar::Direction basicDir = QChar::DirON; // neutral, we don't know

    if( len == 0 ) return QString::null;

    // Test, if the user gives us a directionality.
    // We use 0xFE and 0xFF in ISO8859-8 for that.
    // These chars are undefined in the charset, and are mapped to
    // RTL overwrite
    if( c[0] == 0xfe ) {
	basicDir = QChar::DirL;
	c++; // skip directionality hint
    }
    if( c[0] == 0xff ) {
	basicDir = QChar::DirR;
	c++; // skip directionality hint
    }

    for( int i=0; i<len; i++ ) {
	if ( c[i] > 127 )
	    r[i] = heb_to_unicode[c[i]-128];
	else
	    r[i] = c[i];
    }

    // do transformation from visual byte ordering to logical byte
    // ordering
    if( basicDir == QChar::DirON )
	basicDir = findBasicDirection(r);

    return visualOrder(r, basicDir);
}

/*!
    Transforms the logically ordered QString, \a uc, into a visually
    ordered string in the 8859-8 encoding. Qt's bidi algorithm is used
    to perform this task. Note that newline characters affect the
    reordering, since reordering is done on a line by line basis.

    The algorithm is designed to work on whole paragraphs of text, so
    processing a line at a time may produce incorrect results. This
    approach is taken because the reordering of the contents of a
    particular line in a paragraph may depend on the previous line in
    the same paragraph.

    Some encodings (for example Japanese or UTF-8) are multibyte (so
    one input character is mapped to two output characters). The \a
    lenInOut argument specifies the number of QChars that should be
    converted and is set to the number of characters returned.
*/
QCString QHebrewCodec::fromUnicode(const QString& uc, int& lenInOut) const
{
    // process only len chars...
    int l;
    if( lenInOut > 0 )
	l = QMIN((int)uc.length(),lenInOut);
    else
	l = (int)uc.length();

    QCString rstr;
    if( l == 1 ) {
	if( !to8bit( uc[0], &rstr ) )
	    rstr += (char)unkn;
    } else {
	QString tmp = uc;
	tmp.truncate(l);
	QString vis = visualOrder(tmp, QChar::DirON);

	for (int i=0; i<l; i++) {
	    const QChar ch = vis[i];

	    if( !to8bit( ch, &rstr ) )
		rstr += (char)unkn;
	}
	// lenInOut = cursor - result;
    }
    if( l > 0 && !rstr.length() )
	rstr += (char)unkn;

    return rstr;
}

/*! \reimp
 */
int QHebrewCodec::heuristicContentMatch(const char* chars, int len) const
{
    const unsigned char * c = (const unsigned char *)chars;

    int score = 0;
    for (int i=0; i<len; i++) {
	if(c[i] > 0x80 ) {
	    if ( heb_to_unicode[c[i] - 0x80] != 0xFFFD)
		score++;
	    else
		return -1;
	}
    }
    return score;
}

#endif
