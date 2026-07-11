/****************************************************************************
** $Id: qfontcncodec.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Chinese Font utilities for X11
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


extern int qt_UnicodeToGbk(uint unicode, uchar *gbchar);


int QFontGb2312Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


QFontGb2312Codec::QFontGb2312Codec()
{
    //qDebug("QFontGb2312Codec::QFontGb2312Codec()");
}


const char* QFontGb2312Codec::name() const
{
    //qDebug("QFontGb2312Codec::name() = \"gb2312.1980-0\"");
    return "gb2312.1980-0";
}


int QFontGb2312Codec::mibEnum() const
{
    //qDebug("QFontGb2312Codec::mibEnum() = 57");
    return 57;
}


QString QFontGb2312Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString::null;
}

unsigned short QFontGb2312Codec::characterFromUnicode(const QString &str, int pos) const
{
    uchar buf[4];
    int len = qt_UnicodeToGbk((str.unicode() + pos)->unicode(), buf);
    if (len == 2 && buf[0] > 0xa0 && buf[1] > 0xa0)
        return ((buf[0] & 0x7f) << 8) + (buf[1] & 0x7f);
    return 0;
}

QCString QFontGb2312Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    //qDebug("QFontGb2312Codec::fromUnicode(const QString& uc, int& lenInOut = %d)", lenInOut);
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	uchar buf[8];

	int len = qt_UnicodeToGbk( ch.unicode(), buf );

	if ( len == 2 && buf[0] > 0xa0 && buf[1] > 0xa0 ) {
	    *rdata++ = buf[0] & 0x7f;
	    *rdata++ = buf[1] & 0x7f;
	} else {
	    //white square
	    *rdata++ = 0x21;
	    *rdata++ = 0x75;
	}
    }

    lenInOut *= 2;

    return result;
}

void QFontGb2312Codec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    int len;
    uchar buf[8];
    while (length--) {
	len = qt_UnicodeToGbk(in->unicode(), buf);
	if ( len == 2 && buf[0] > 0xa0 && buf[1] > 0xa0 ) {
	    *out = (((buf[0] << 8) | buf[1]) & 0x7f7f);
	} else {
	    *out = 0;
	}

	++in;
	++out;
    }
}


bool QFontGb2312Codec::canEncode( QChar ch ) const
{
    uchar buf[4];
    int len = qt_UnicodeToGbk( ch.unicode(), buf );
    //qDebug("QFontGb2312Codec::canEncode( QChar ch = %02X%02X )", ch.row(), ch.cell());
    return ( len == 2 && buf[0] > 0xa0 && buf[1] > 0xa0 );
}

//==========================================================================


int QFontGbkCodec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


int QFontGbkCodec::heuristicNameMatch(const char* hint) const
{
    //qDebug("QFontGbkCodec::heuristicNameMatch(const char* hint = \"%s\")", hint);
    return ( qstricmp(hint, "gbk-0") == 0 ||
	     qstricmp(hint, "gb18030.2000-0") == 0 )
        ? 13 : 0;
}


QFontGbkCodec::QFontGbkCodec()
{
    //qDebug("QFontGbkCodec::QFontGbkCodec()");
}


const char* QFontGbkCodec::name() const
{
    //qDebug("QFontGbkCodec::name() = \"gbk-0\"");
    return "gbk-0";
}


int QFontGbkCodec::mibEnum() const
{
    //qDebug("QFontGbkCodec::mibEnum() = -113");
    return -113;
}


QString QFontGbkCodec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString::null;
}

unsigned short QFontGbkCodec::characterFromUnicode(const QString &str, int pos) const
{
    uchar buf[4];
    int len = qt_UnicodeToGbk((str.unicode() + pos)->unicode(), buf);
    if (len == 2)
        return (buf[0] << 8) + buf[1];
    return 0;
}

QCString QFontGbkCodec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    //qDebug("QFontGbkCodec::fromUnicode(const QString& uc, int& lenInOut = %d)", lenInOut);
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	uchar buf[8];

	int len = qt_UnicodeToGbk( ch.unicode(), buf );

	if ( len == 2 ) {
	    *rdata++ = buf[0];
	    *rdata++ = buf[1];
	} else {
	    //white square
	    *rdata++ = 0xa1;
	    *rdata++ = 0xf5;
	}
    }

    lenInOut *= 2;

    return result;
}

void QFontGbkCodec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    uchar buf[8];
    while (length--) {
	*out++ = (qt_UnicodeToGbk(in->unicode(), buf) == 2) ? (buf[0] << 8) | buf[1] : 0;
	++in;
    }
}

bool QFontGbkCodec::canEncode( QChar ch ) const
{
    if (ch.unicode() >= 0x4e00 && ch.unicode() <= 0x9fa5)
        return TRUE;
    uchar buf[4];
    int len = qt_UnicodeToGbk( ch.unicode(), buf );
    //qDebug("QFontGbkCodec::canEncode( QChar ch = %02X%02X )", ch.row(), ch.cell());
    return ( len == 2 );
}

//==========================================================================

int QFontGb18030_0Codec::heuristicContentMatch(const char *, int) const
{
    return 0;
}


QFontGb18030_0Codec::QFontGb18030_0Codec()
{
    //qDebug("QFontGb18030_0Codec::QFontGb18030_0Codec()");
}


const char* QFontGb18030_0Codec::name() const
{
    //qDebug("QFontGb18030_0Codec::name() = \"gb18030-0\"");
    return "gb18030-0";
}


int QFontGb18030_0Codec::mibEnum() const
{
    //qDebug("QFontGb18030_0Codec::mibEnum() = -114");
    return -114;
}


QString QFontGb18030_0Codec::toUnicode(const char* /*chars*/, int /*len*/) const
{
    return QString::null;
}

unsigned short
QFontGb18030_0Codec::characterFromUnicode(const QString &str, int pos) const
{
    const QChar * const ch = str.unicode() + pos;
    if (ch->row () > 0 && !(ch->row () >= 0xd8 && ch->row () < 0xe0))
        return ch->unicode();
    return 0;
}

QCString QFontGb18030_0Codec::fromUnicode(const QString& uc, int& lenInOut ) const
{
    QCString result(lenInOut * 2 + 1);
    uchar *rdata = (uchar *) result.data();
    const QChar *ucp = uc.unicode();

    //qDebug("QFontGb18030_0Codec::fromUnicode(const QString& uc, int& lenInOut = %d)", lenInOut);
    for ( int i = 0; i < lenInOut; i++ ) {
	QChar ch(*ucp++);
	if (ch.row () > 0 && !(ch.row () >= 0xd8 && ch.row () < 0xe0)) {
	    *rdata++ = ch.row();
	    *rdata++ = ch.cell();
	} else {
	    *rdata++ = 0xff;
	    *rdata++ = 0xfd;
	}
    }

    lenInOut *= 2;

    return result;
}

void QFontGb18030_0Codec::fromUnicode(const QChar *in, unsigned short *out, int length) const
{
    while (length--) {
	*out = ((in->row () > 0 && !(in->row () >= 0xd8 && in->row () < 0xe0))
		? in->unicode() : 0);
	++in;
	++out;
    }
}

bool QFontGb18030_0Codec::canEncode( QChar ch ) const
{
    //qDebug("QFontGb18030_0Codec::canEncode( QChar ch = %02X%02X )", ch.row(), ch.cell());
    return (ch.row () > 0 && !(ch.row () >= 0xd8 && ch.row () < 0xe0));
}

//==========================================================================


#endif // QT_NO_BIG_CODECS
#endif // QT_NO_CODECS
