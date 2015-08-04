/****************************************************************************
** $Id: qt/qtextlayout_p.h   3.3.4   edited Aug 25 16:11 $
**
** ???
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QTEXTLAYOUT_P_H
#define QTEXTLAYOUT_P_H

#ifndef QT_H
#include "qstring.h"
#include "qnamespace.h"
#include "qrect.h"
#endif // QT_H

class QTextEngine;
class QFont;

class Q_EXPORT QTextItem
{
public:
    inline QTextItem() : item(0), engine(0) {}
    inline bool isValid() const { return (bool)engine; }

    QRect rect() const;
    int x() const;
    int y() const;
    int width() const;
    int ascent() const;
    int descent() const;

    enum Edge {
	Leading,
	Trailing
    };
    enum CursorPosition {
	BetweenCharacters,
	OnCharacters
    };

    /* cPos gets set to the valid position */
    int cursorToX( int *cPos, Edge edge = Leading ) const;
    inline int cursorToX( int cPos, Edge edge = Leading ) const { return cursorToX( &cPos, edge ); }
    int xToCursor( int x, CursorPosition = BetweenCharacters ) const;

    bool isRightToLeft() const;
    bool isObject() const;
    bool isSpace() const;
    bool isTab() const;

    void setWidth( int w );
    void setAscent( int a );
    void setDescent( int d );

    int from() const;
    int length() const;

private:
    friend class QTextLayout;
    friend class QPainter;
    friend class QPSPrinter;
    QTextItem( int i, QTextEngine *e ) : item( i ), engine( e ) {}
    int item;
    QTextEngine *engine;
};


class QPainter;

class Q_EXPORT QTextLayout
{
public:
    // does itemization
    QTextLayout();
    QTextLayout( const QString& string, QPainter * = 0 );
    QTextLayout( const QString& string, const QFont& fnt );
    ~QTextLayout();

    void setText( const QString& string, const QFont& fnt );

    enum LineBreakStrategy {
	AtWordBoundaries,
	AtCharBoundaries
    };

    /* add an additional item boundary eg. for style change */
    void setBoundary( int strPos );

    int numItems() const;
    QTextItem itemAt( int i ) const;
    QTextItem findItem( int strPos ) const;

    enum LayoutMode {
	NoBidi,
	SingleLine,
	MultiLine
    };
    void beginLayout( LayoutMode m = MultiLine );
    void beginLine( int width );

    bool atEnd() const;
    QTextItem nextItem();
    QTextItem currentItem();
    /* ## maybe also currentItem() */
    void setLineWidth( int newWidth );
    int lineWidth() const;
    int widthUsed() const;
    int availableWidth() const;

    enum Result {
	Ok,
	LineFull,
	LineEmpty,
	Error
    };
    /* returns true if completely added */
    Result addCurrentItem();

    /* Note: if ascent and descent are used they must be initialized to the minimum ascent/descent
       acceptable for the line. QFontMetrics::ascent/descent() is usually the right choice */
    Result endLine( int x = 0, int y = 0, int alignment = Qt::AlignLeft,
		    int *ascent = 0, int *descent = 0, int *left = 0, int *right = 0 );
    void endLayout();

    enum CursorMode {
	SkipCharacters,
	SkipWords
    };
    bool validCursorPosition( int pos ) const;
    int nextCursorPosition( int oldPos, CursorMode mode = SkipCharacters ) const;
    int previousCursorPosition( int oldPos, CursorMode mode = SkipCharacters ) const;

    void setDirection(QChar::Direction);
private:
    QTextLayout( QTextEngine *e ) : d( e ) {}
    /* disable copy and assignment */
    QTextLayout( const QTextLayout & ) {}
    void operator = ( const QTextLayout & ) {}

    friend class QTextItem;
    friend class QPainter;
    friend class QPSPrinter;
    QTextEngine *d;
};


/*
  class QPainter {
      .....
      void drawTextItem( int x, int y, QTextItem *item );
  };
*/

#endif
