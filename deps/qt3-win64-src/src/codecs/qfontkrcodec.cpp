/****************************************************************************
** $Id: qfontkrcodec.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Korean Font utilities for X11
**
** Created : 20010130
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
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

#include "private/qfontcodecs_p.h"

#ifndef QT_NO_CODECS
#ifndef QT_NO_BIG_CODECS

extern unsigned int qt_UnicodeToKsc5601(unsigned int unicode);


int QFontKsc5601Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


QFontKsc5601Codec::QFontKsc5601Codec()
{
}


const char* QFontKsc5601Codec::name() const
{
    return "ksc5601.1987-0";
}


int QFontKsc5601Codec::mibEnum() const
{
    return 36;
}


QString QFontKsc5601Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString(); //###
}

unsigned short QFontKsc5601Codec::characterFromUnicode(const QString &str, int pos) const
{
    return qt_UnicodeToKsc5601((str.unicode() + pos)->unicode());
}

QCString QFontKsc5601Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	ch = qt_UnicodeToKsc5601(ch.unicode());

	if ( ! ch.isNull() ) {
	    *rdata++ = ch.row() & 0x7f ;
	    *rdata++ = ch.cell() & 0x7f;
	} else {
	    //white square
	    *rdata++ = 0x21;
	    *rdata++ = 0x60;
	}
    }

    lenInOut *= 2;

    return result;
}

void QFontKsc5601Codec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    while (length--) {
	*out++ = (qt_UnicodeToKsc5601(in->unicode()) & 0x7f7f);
	++in;
    }
}

bool QFontKsc5601Codec::canEncode( QChar ch ) const
{
    return (qt_UnicodeToKsc5601(ch.unicode()) != 0);
}

#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS
