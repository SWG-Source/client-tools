/****************************************************************************
** $Id: qsimplerichtext.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QSimpleRichText class
**
** Created : 990101
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qsimplerichtext.h"

#ifndef QT_NO_RICHTEXT
#include "qrichtext_p.h"
#include "qapplication.h"

class QSimpleRichTextData
{
public:
    QTextDocument *doc;
    QFont font;
    int cachedWidth;
    bool cachedWidthWithPainter;
    void adjustSize(QPainter *p = 0);
};

// Pull this private function in from qglobal.cpp
extern unsigned int qt_int_sqrt( unsigned int n );

void QSimpleRichTextData::adjustSize(QPainter *p) {
    QFontMetrics fm( font );
    int mw =  fm.width( 'x' ) * 80;
    int w = mw;
    doc->doLayout(p, w);
    if ( doc->widthUsed() != 0 ) {
	w = qt_int_sqrt( 5 * doc->height() * doc->widthUsed() / 3 );
	doc->doLayout(p, QMIN(w, mw));

	if ( w*3 < 5*doc->height() ) {
	    w = qt_int_sqrt( 2 * doc->height() * doc->widthUsed() );
	    doc->doLayout(p, QMIN(w, mw));
	}
    }
    cachedWidth = doc->width();
    cachedWidthWithPainter = FALSE;
}

/*!
    \class QSimpleRichText qsimplerichtext.h
    \brief The QSimpleRichText class provides a small displayable piece of rich text.

    \ingroup text
    \mainclass

    This class encapsulates simple rich text usage in which a string
    is interpreted as rich text and can be drawn. This is particularly
    useful if you want to display some rich text in a custom widget. A
    QStyleSheet is needed to interpret the tags and format the rich
    text. Qt provides a default HTML-like style sheet, but you may
    define custom style sheets.

    Once created, the rich text object can be queried for its width(),
    height(), and the actual width used (see widthUsed()). Most
    importantly, it can be drawn on any given QPainter with draw().
    QSimpleRichText can also be used to implement hypertext or active
    text facilities by using anchorAt(). A hit test through inText()
    makes it possible to use simple rich text for text objects in
    editable drawing canvases.

    Once constructed from a string the contents cannot be changed,
    only resized. If the contents change, just throw the rich text
    object away and make a new one with the new contents.

    For large documents use QTextEdit or QTextBrowser. For very small
    items of rich text you can use a QLabel.

    If you are using QSimpleRichText to print in high resolution you
    should call setWidth(QPainter, int) so that the content will be
    laid out properly on the page.
*/

/*!
    Constructs a QSimpleRichText from the rich text string \a text and
    the font \a fnt.

    The font is used as a basis for the text rendering. When using
    rich text rendering on a widget \e w, you would normally specify
    the widget's font, for example:

    \code
    QSimpleRichText myrichtext( contents, mywidget->font() );
    \endcode

    \a context is the optional context of the rich text object. This
    becomes important if \a text contains relative references, for
    example within image tags. QSimpleRichText always uses the default
    mime source factory (see \l{QMimeSourceFactory::defaultFactory()})
    to resolve those references. The context will then be used to
    calculate the absolute path. See
    QMimeSourceFactory::makeAbsolute() for details.

    The \a sheet is an optional style sheet. If it is 0, the default
    style sheet will be used (see \l{QStyleSheet::defaultSheet()}).
*/

QSimpleRichText::QSimpleRichText( const QString& text, const QFont& fnt,
				  const QString& context, const QStyleSheet* sheet )
{
    d = new QSimpleRichTextData;
    d->cachedWidth = -1;
    d->cachedWidthWithPainter = FALSE;
    d->font = fnt;
    d->doc = new QTextDocument( 0 );
    d->doc->setTextFormat( Qt::RichText );
    d->doc->setLeftMargin( 0 );
    d->doc->setRightMargin( 0 );
    d->doc->setFormatter( new QTextFormatterBreakWords );
    d->doc->setStyleSheet( (QStyleSheet*)sheet );
    d->doc->setDefaultFormat( fnt, QColor() );
    d->doc->setText( text, context );
}


/*!
    Constructs a QSimpleRichText from the rich text string \a text and
    the font \a fnt.

    This is a slightly more complex constructor for QSimpleRichText
    that takes an additional mime source factory \a factory, a page
    break parameter \a pageBreak and a bool \a linkUnderline. \a
    linkColor is only provided for compatibility, but has no effect,
    as QColorGroup's QColorGroup::link() color is used now.

    \a context is the optional context of the rich text object. This
    becomes important if \a text contains relative references, for
    example within image tags. QSimpleRichText always uses the default
    mime source factory (see \l{QMimeSourceFactory::defaultFactory()})
    to resolve those references. The context will then be used to
    calculate the absolute path. See
    QMimeSourceFactory::makeAbsolute() for details.

    The \a sheet is an optional style sheet. If it is 0, the default
    style sheet will be used (see \l{QStyleSheet::defaultSheet()}).

    This constructor is useful for creating a QSimpleRichText object
    suitable for printing. Set \a pageBreak to be the height of the
    contents area of the pages.
*/

QSimpleRichText::QSimpleRichText( const QString& text, const QFont& fnt,
				  const QString& context,  const QStyleSheet* sheet,
				  const QMimeSourceFactory* factory, int pageBreak,
				  const QColor& /*linkColor*/, bool linkUnderline )
{
    d = new QSimpleRichTextData;
    d->cachedWidth = -1;
    d->cachedWidthWithPainter = FALSE;
    d->font = fnt;
    d->doc = new QTextDocument( 0 );
    d->doc->setTextFormat( Qt::RichText );
    d->doc->setFormatter( new QTextFormatterBreakWords );
    d->doc->setStyleSheet( (QStyleSheet*)sheet );
    d->doc->setDefaultFormat( fnt, QColor() );
    d->doc->flow()->setPageSize( pageBreak );
    d->doc->setPageBreakEnabled( TRUE );
#ifndef QT_NO_MIME
    d->doc->setMimeSourceFactory( (QMimeSourceFactory*)factory );
#endif
    d->doc->setUnderlineLinks( linkUnderline );
    d->doc->setText( text, context );
}

/*!
    Destroys the rich text object, freeing memory.
*/

QSimpleRichText::~QSimpleRichText()
{
    delete d->doc;
    delete d;
}

/*!
    \overload

    Sets the width of the rich text object to \a w pixels.

    \sa height(), adjustSize()
*/

void QSimpleRichText::setWidth( int w )
{
    if ( w == d->cachedWidth && !d->cachedWidthWithPainter )
	return;
    d->doc->formatter()->setAllowBreakInWords( d->doc->isPageBreakEnabled() );
    d->cachedWidth = w;
    d->cachedWidthWithPainter = FALSE;
    d->doc->doLayout( 0, w );
}

/*!
    Sets the width of the rich text object to \a w pixels,
    recalculating the layout as if it were to be drawn with painter \a
    p.

    Passing a painter is useful when you intend drawing on devices
    other than the screen, for example a QPrinter.

    \sa height(), adjustSize()
*/

void QSimpleRichText::setWidth( QPainter *p, int w )
{
    if ( w == d->cachedWidth  && d->cachedWidthWithPainter )
	return;
    d->doc->formatter()->setAllowBreakInWords( d->doc->isPageBreakEnabled() ||
		       p && p->device() &&
		     p->device()->devType() == QInternal::Printer );
    p->save();
    d->cachedWidth = w;
    d->cachedWidthWithPainter = TRUE;
    d->doc->doLayout( p, w );
    p->restore();
}

/*!
    Returns the set width of the rich text object in pixels.

    \sa widthUsed()
*/

int QSimpleRichText::width() const
{
    if ( d->cachedWidth < 0 )
	d->adjustSize();
    return d->doc->width();
}

/*!
    Returns the width in pixels that is actually used by the rich text
    object. This can be smaller or wider than the set width.

    It may be wider, for example, if the text contains images or
    non-breakable words that are already wider than the available
    space. It's smaller when the object only consists of lines that do
    not fill the width completely.

    \sa width()
*/

int QSimpleRichText::widthUsed() const
{
    if ( d->cachedWidth < 0 )
	d->adjustSize();
    return d->doc->widthUsed();
}

/*!
    Returns the height of the rich text object in pixels.

    \sa setWidth()
*/

int QSimpleRichText::height() const
{
    if ( d->cachedWidth < 0 )
	d->adjustSize();
    return d->doc->height();
}

/*!
    Adjusts the richt text object to a reasonable size.

    \sa setWidth()
*/

void QSimpleRichText::adjustSize()
{
    d->adjustSize();
}

/*!
    Draws the formatted text with painter \a p, at position (\a x, \a
    y), clipped to \a clipRect. The clipping rectangle is given in the
    rich text object's coordinates translated by (\a x, \a y). Passing
    an null rectangle results in no clipping. Colors from the color
    group \a cg are used as needed, and if not 0, \a *paper is used as
    the background brush.

    Note that the display code is highly optimized to reduce flicker,
    so passing a brush for \a paper is preferable to simply clearing
    the area to be painted and then calling this without a brush.
*/

void QSimpleRichText::draw( QPainter *p,  int x, int y, const QRect& clipRect,
			    const QColorGroup& cg, const QBrush* paper ) const
{
    p->save();
    if ( d->cachedWidth < 0 )
	d->adjustSize(p);

    QRect r = clipRect;
    if ( !r.isNull() )
	r.moveBy( -x, -y );

    if ( paper )
	d->doc->setPaper( new QBrush( *paper ) );
    QColorGroup g = cg;
    if ( d->doc->paper() )
	g.setBrush( QColorGroup::Base, *d->doc->paper() );

    if ( !clipRect.isNull() )
	p->setClipRect( clipRect, QPainter::CoordPainter );
    p->translate( x, y );
    d->doc->draw( p, r, g, paper );
    p->translate( -x, -y );
    p->restore();
}


/*! \fn void QSimpleRichText::draw( QPainter *p,  int x, int y, const QRegion& clipRegion,
  const QColorGroup& cg, const QBrush* paper ) const

  \obsolete

  Use the version with clipRect instead. The region version has
  problems with larger documents on some platforms (on X11 regions
  internally are represented with 16bit coordinates).
*/



/*!
    Returns the context of the rich text object. If no context has
    been specified in the constructor, a null string is returned. The
    context is the path to use to look up relative links, such as
    image tags and anchor references.
*/

QString QSimpleRichText::context() const
{
    return d->doc->context();
}

/*!
    Returns the anchor at the requested position, \a pos. An empty
    string is returned if no anchor is specified for this position.
*/

QString QSimpleRichText::anchorAt( const QPoint& pos ) const
{
    if ( d->cachedWidth < 0 )
	d->adjustSize();
    QTextCursor c( d->doc );
    c.place( pos, d->doc->firstParagraph(), TRUE );
    return c.paragraph()->at( c.index() )->anchorHref();
}

/*!
    Returns TRUE if \a pos is within a text line of the rich text
    object; otherwise returns FALSE.
*/

bool QSimpleRichText::inText( const QPoint& pos ) const
{
    if ( d->cachedWidth < 0 )
	d->adjustSize();
    if ( pos.y()  > d->doc->height() )
	return FALSE;
    QTextCursor c( d->doc );
    c.place( pos, d->doc->firstParagraph() );
    return c.totalOffsetX() + c.paragraph()->at( c.index() )->x +
	c.paragraph()->at( c.index() )->format()->width( c.paragraph()->at( c.index() )->c ) > pos.x();
}

/*!
    Sets the default font for the rich text object to \a f
*/

void QSimpleRichText::setDefaultFont( const QFont &f )
{
    if ( d->font == f )
	return;
    d->font = f;
    d->cachedWidth = -1;
    d->cachedWidthWithPainter = FALSE;
    d->doc->setDefaultFormat( f, QColor() );
    d->doc->setText( d->doc->originalText(), d->doc->context() );
}

#endif //QT_NO_RICHTEXT
