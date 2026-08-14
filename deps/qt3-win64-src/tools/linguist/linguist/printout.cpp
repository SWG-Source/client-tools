/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Linguist.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "printout.h"

#include <qprinter.h>
#include <qfontmetrics.h>

PrintOut::PrintOut( QPrinter *printer )
    : pr( printer ), pdmetrics( printer ), nextRule( NoRule ), page( 0 )
{
    p.begin( pr );
    QFont f( "Arial" );
    f8 = f;
    f8.setPointSize( 8 );
    f10 = f;
    f10.setPointSize( 10 );
    p.setFont( f10 );
    fmetrics = new QFontMetrics( p.fontMetrics() );
    hmargin = 5 * pdmetrics.width() / pdmetrics.widthMM(); // 5 mm
    vmargin = 5 * pdmetrics.height() / pdmetrics.heightMM(); // 5 mm
    hsize = pdmetrics.width() - 2 * hmargin;
    vsize = pdmetrics.height() - vmargin;
    dateTime = QDateTime::currentDateTime();
    breakPage();
    vsize -= voffset;
    cp = Paragraph( QPoint(hmargin, voffset) );
}

PrintOut::~PrintOut()
{
    flushLine();
    delete fmetrics;
    p.end();
}

void PrintOut::setRule( Rule rule )
{
    if ( (int) nextRule < (int) rule )
	nextRule = rule;
}

void PrintOut::setGuide( const QString& guide )
{
    g = guide;
}

void PrintOut::vskip()
{
    if ( !firstParagraph )
	voffset += 14;
}

void PrintOut::flushLine( bool /* mayBreak */ )
{
    if ( voffset + cp.rect.height() > vsize )
	breakPage();
    else if ( !firstParagraph )
	drawRule( nextRule );

    for ( int i = 0; i < (int) cp.boxes.count(); i++ ) {
	Box b = cp.boxes[i];
	b.rect.moveBy( 0, voffset );
	QRect r = b.rect;
	p.setFont( b.font );
	p.drawText( r, b.align, b.text );
    }
    voffset += cp.rect.height();

    nextRule = NoRule;
    cp = Paragraph( QPoint(hmargin, voffset) );
    firstParagraph = FALSE;
}

void PrintOut::addBox( int percent, const QString& text, Style style,
		       int halign )
{
    int align = halign | Qt::AlignTop;
    QFont f = f10;
    if ( style == Strong )
	f.setBold( TRUE );
    else if ( style == Emphasis )
	f.setItalic( TRUE );
    int wd = hsize * percent / 100;
    QRect r( cp.rect.x() + cp.rect.width(), 0, wd, vsize );
    int ht = p.boundingRect( r, align, text ).height();

    Box b( r, text, f, align );
    cp.boxes.append( b );
    cp.rect.setSize( QSize(cp.rect.width() + wd, QMAX(cp.rect.height(), ht)) );
}

void PrintOut::breakPage()
{
    static const int LeftAlign = Qt::AlignLeft | Qt::AlignTop;
    static const int RightAlign = Qt::AlignRight | Qt::AlignTop;
    QRect r1, r2;
    int h1 = 0;
    int h2 = 0;

    if ( page++ > 0 )
	pr->newPage();
    voffset = 0;

    p.setFont( f10 );
    r1 = QRect( hmargin, voffset, 3 * hsize / 4, vsize );
    r2 = QRect( r1.x() + r1.width(), voffset, hsize - r1.width(), vsize );
    h1 = p.boundingRect( r1, LeftAlign, pr->docName() ).height();
    p.drawText( r1, LeftAlign, pr->docName() );
    h2 = p.boundingRect( r2, RightAlign, QString::number(page) ).height();
    p.drawText( r2, RightAlign, QString::number(page) );
    voffset += QMAX( h1, h2 );

    r1 = QRect( hmargin, voffset, hsize / 2, LeftAlign );
    p.setFont( f8 );
    h1 = p.boundingRect( r1, LeftAlign, dateTime.toString() ).height();
    p.drawText( r1, LeftAlign, dateTime.toString() );
    p.setFont( f10 );
    voffset += QMAX( h1, h2 );

    voffset += 4;
    p.drawLine( QPoint(hmargin, voffset), QPoint(hmargin + hsize, voffset) );
    voffset += 14;
    firstParagraph = TRUE;
}

void PrintOut::drawRule( Rule rule )
{
    QPen pen;

    switch ( rule ) {
    case NoRule:
	voffset += 5;
	break;
    case ThinRule:
	pen.setColor( QColor(192, 192, 192) );
	pen.setStyle( QPen::DotLine );
	pen.setWidth( 0 );
	p.setPen( pen );
	voffset += 5;
	p.drawLine( QPoint(hmargin, voffset),
		    QPoint(hmargin + hsize, voffset) );
	p.setPen( QPen() );
	voffset += 2;
	break;
    case ThickRule:
	voffset += 7;
	p.drawLine( QPoint(hmargin, voffset),
		    QPoint(hmargin + hsize, voffset) );
	voffset += 4;
    }
}
