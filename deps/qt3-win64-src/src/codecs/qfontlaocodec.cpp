/****************************************************************************
** $Id: qfontlaocodec.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Font utilities for X11
**
** Created : 20001101
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

static unsigned char const unicode_to_mulelao[256] =
    {
	// U+0E80
	0x00, 0xa1, 0xa2, 0x00, 0xa4, 0x00, 0x00, 0xa7,
	0xa8, 0x00, 0xaa, 0x00, 0x00, 0xad, 0x00, 0x00,
	// U+0E90
	0x00, 0x00, 0x00, 0x00, 0xb4, 0xb5, 0xb6, 0xb7,
	0x00, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	// U+0EA0
	0x00, 0xc1, 0xc2, 0xc3, 0x00, 0xc5, 0x00, 0xc7,
	0x00, 0x00, 0xca, 0xcb, 0x00, 0xcd, 0xce, 0xcf,
	// U+0EB0
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0x00, 0xdb, 0xdc, 0xdd, 0x00, 0x00,
	// U+0EC0
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0x00, 0xe6, 0x00,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0x00, 0x00,
	// U+0ED0
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0x00, 0x00, 0xfc, 0xfd, 0x00, 0x00,
	// U+0EE0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// U+0EF0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };


QFontLaoCodec::QFontLaoCodec()
{
}

const char *QFontLaoCodec::name() const
{
    return "mulelao-1";
}

int QFontLaoCodec::mibEnum() const
{
    return -4242;
}

unsigned short QFontLaoCodec::characterFromUnicode(const QString &str, int pos) const
{
    const QChar * const ch = str.unicode() + pos;
    if (ch->unicode() < 0x80)
        return ch->unicode();
    if ( ch->unicode() >= 0x0e80 && ch->unicode() <= 0x0eff )
        return unicode_to_mulelao[ch->unicode() - 0x0e80];
    return 0;
}

QCString QFontLaoCodec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString rstring( lenInOut+1 );
    uchar *rdata = (uchar *) rstring.data();
    const QChar *sdata = uc.unicode();
    int i = 0;
    for ( ; i < lenInOut; ++i, ++sdata, ++rdata ) {
	if ( sdata->unicode() < 0x80 ) {
	    *rdata = (uchar) sdata->unicode();
	} else if ( sdata->unicode() >= 0x0e80 && sdata->unicode() <= 0x0eff ) {
	    uchar lao = unicode_to_mulelao[sdata->unicode() - 0x0e80];
	    if ( lao )
		*rdata = lao;
	    else
		*rdata = '?';
	} else {
	    *rdata = '?';
	}
    }
    *rdata = 0u;
    return rstring;
}

void QFontLaoCodec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    while (length--) {
	if ( in->unicode() < 0x80 ) {
	    *out = (uchar) in->unicode();
	} else if ( in->unicode() >= 0x0e80 && in->unicode() <= 0x0eff ) {
	    *out = unicode_to_mulelao[in->unicode() - 0x0e80];
	} else {
	    *out = 0;
	}

	++in;
	++out;
    }
}

int QFontLaoCodec::heuristicContentMatch(const char *, int) const
{
    return -1;
}

bool QFontLaoCodec::canEncode( QChar ch ) const
{
    return ( ch.unicode() < 0x80 ||
	     unicode_to_mulelao[ch.unicode() - 0x0e80] != 0x00 );
}


#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS

