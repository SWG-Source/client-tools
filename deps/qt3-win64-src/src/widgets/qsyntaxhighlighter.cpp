/****************************************************************************
** $Id: qsyntaxhighlighter.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QSyntaxHighlighter class
**
** Created : 990101
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

#include "qsyntaxhighlighter.h"
#include "private/qsyntaxhighlighter_p.h"

#ifndef QT_NO_SYNTAXHIGHLIGHTER
#include "../kernel/qrichtext_p.h"
#include "qtextedit.h"
#include "qtimer.h"

/*!
    \class QSyntaxHighlighter qsyntaxhighlighter.h
    \brief The QSyntaxHighlighter class is a base class for
    implementing QTextEdit syntax highlighters.

    \ingroup basic
    \ingroup text

    A syntax highligher automatically highlights parts of the text in
    a QTextEdit. Syntax highlighters are often used when the user is
    entering text in a specific format (for example, source code) and
    help the user to read the text and identify syntax errors.

    To provide your own syntax highlighting for QTextEdit, you must
    subclass QSyntaxHighlighter and reimplement highlightParagraph().

    When you create an instance of your QSyntaxHighlighter subclass,
    pass it the QTextEdit that you want the syntax highlighting to be
    applied to. After this your highlightParagraph() function will be
    called automatically whenever necessary. Use your
    highlightParagraph() function to apply formatting (e.g. setting
    the font and color) to the text that is passed to it.
*/

/*!
    Constructs the QSyntaxHighlighter and installs it on \a textEdit.

    It is the caller's responsibility to delete the
    QSyntaxHighlighter when it is no longer needed.
*/

QSyntaxHighlighter::QSyntaxHighlighter( QTextEdit *textEdit )
    : para( 0 ), edit( textEdit ), d( new QSyntaxHighlighterPrivate )
{
    textEdit->document()->setPreProcessor( new QSyntaxHighlighterInternal( this ) );
    textEdit->document()->invalidate();
    QTimer::singleShot( 0, textEdit->viewport(), SLOT( update() ) );
}

/*!
    Destructor. Uninstalls this syntax highlighter from the textEdit()
*/

QSyntaxHighlighter::~QSyntaxHighlighter()
{
    delete d;
    textEdit()->document()->setPreProcessor( 0 );
}

/*!
    \fn int QSyntaxHighlighter::highlightParagraph( const QString &text, int endStateOfLastPara )

    This function is called when necessary by the rich text engine,
    i.e. on paragraphs which have changed.

    In your reimplementation you should parse the paragraph's \a text
    and call setFormat() as often as necessary to apply any font and
    color changes that you require. Your function must return a value
    which indicates the paragraph's end state: see below.

    Some syntaxes can have constructs that span paragraphs. For
    example, a C++ syntax highlighter should be able to cope with
    \c{/}\c{*...*}\c{/} comments that span paragraphs. To deal
    with these cases it is necessary to know the end state of the
    previous paragraph (e.g. "in comment").

    If your syntax does not have paragraph spanning constructs, simply
    ignore the \a endStateOfLastPara parameter and always return 0.

    Whenever highlightParagraph() is called it is passed a value for
    \a endStateOfLastPara. For the very first paragraph this value is
    always -2. For any other paragraph the value is the value returned
    by the most recent highlightParagraph() call that applied to the
    preceding paragraph.

    The value you return is up to you. We recommend only returning 0
    (to signify that this paragraph's syntax highlighting does not
    affect the following paragraph), or a positive integer (to signify
    that this paragraph has ended in the middle of a paragraph
    spanning construct).

    To find out which paragraph is highlighted, call
    currentParagraph().

    For example, if you're writing a simple C++ syntax highlighter,
    you might designate 1 to signify "in comment". For a paragraph
    that ended in the middle of a comment you'd return 1, and for
    other paragraphs you'd return 0. In your parsing code if \a
    endStateOfLastPara was 1, you would highlight the text as a C++
    comment until you reached the closing \c{*}\c{/}.
*/

/*!
    This function is applied to the syntax highlighter's current
    paragraph (the text of which is passed to the highlightParagraph()
    function).

    The specified \a font and \a color are applied to the text from
    position \a start for \a count characters. (If \a count is 0,
    nothing is done.)
*/

void QSyntaxHighlighter::setFormat( int start, int count, const QFont &font, const QColor &color )
{
    if ( !para || count <= 0 )
	return;
    QTextFormat *f = 0;
    f = para->document()->formatCollection()->format( font, color );
    para->setFormat( start, count, f );
    f->removeRef();
}

/*! \overload */

void QSyntaxHighlighter::setFormat( int start, int count, const QColor &color )
{
    if ( !para || count <= 0 )
	return;
    QTextFormat *f = 0;
    QFont fnt = textEdit()->QWidget::font();
    f = para->document()->formatCollection()->format( fnt, color );
    para->setFormat( start, count, f );
    f->removeRef();
}

/*! \overload */

void QSyntaxHighlighter::setFormat( int start, int count, const QFont &font )
{
    if ( !para || count <= 0 )
	return;
    QTextFormat *f = 0;
    QColor c = textEdit()->viewport()->paletteForegroundColor();
    f = para->document()->formatCollection()->format( font, c );
    para->setFormat( start, count, f );
    f->removeRef();
}

/*!
    \fn QTextEdit *QSyntaxHighlighter::textEdit() const

    Returns the QTextEdit on which this syntax highlighter is
    installed
*/

/*!  Redoes the highlighting of the whole document.
*/

void QSyntaxHighlighter::rehighlight()
{
    QTextParagraph *s = edit->document()->firstParagraph();
    while ( s ) {
	s->invalidate( 0 );
	s->state = -1;
	s->needPreProcess = TRUE;
	s = s->next();
    }
    edit->repaintContents( FALSE );
}

/*!
    Returns the id of the paragraph which is highlighted, or -1 of no
    paragraph is currently highlighted.

    Usually this function is called from within highlightParagraph().
*/

int QSyntaxHighlighter::currentParagraph() const
{
    return d->currentParagraph;
}

#endif
