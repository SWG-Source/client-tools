/****************************************************************************
** $Id: qtextedit.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QTextEdit class
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

#include "qtextedit.h"

#ifndef QT_NO_TEXTEDIT

#include "../kernel/qrichtext_p.h"
#include "qpainter.h"
#include "qpen.h"
#include "qbrush.h"
#include "qpixmap.h"
#include "qfont.h"
#include "qcolor.h"
#include "qstyle.h"
#include "qsize.h"
#include "qevent.h"
#include "qtimer.h"
#include "qapplication.h"
#include "qlistbox.h"
#include "qvbox.h"
#include "qapplication.h"
#include "qclipboard.h"
#include "qcolordialog.h"
#include "qfontdialog.h"
#include "qstylesheet.h"
#include "qdragobject.h"
#include "qurl.h"
#include "qcursor.h"
#include "qregexp.h"
#include "qpopupmenu.h"
#include "qptrstack.h"
#include "qmetaobject.h"
#include "qtextbrowser.h"
#include <private/qucom_p.h>
#include "private/qsyntaxhighlighter_p.h"
#include <qguardedptr.h>

#ifndef QT_NO_ACCEL
#include <qkeysequence.h>
#define ACCEL_KEY(k) "\t" + QString(QKeySequence( Qt::CTRL | Qt::Key_ ## k ))
#else
#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)
#endif

#ifdef QT_TEXTEDIT_OPTIMIZATION
#define LOGOFFSET(i) d->logOffset + i
#endif

struct QUndoRedoInfoPrivate
{
    QTextString text;
};

class QTextEditPrivate
{
public:
    QTextEditPrivate()
	:preeditStart(-1),preeditLength(-1),ensureCursorVisibleInShowEvent(FALSE),
	 tabChangesFocus(FALSE),
#ifndef QT_NO_CLIPBOARD
	 clipboard_mode( QClipboard::Clipboard ),
#endif
#ifdef QT_TEXTEDIT_OPTIMIZATION
	 od(0), optimMode(FALSE),
	 maxLogLines(-1),
	 logOffset(0),
#endif
	 autoFormatting( (uint)QTextEdit::AutoAll )
    {
	for ( int i=0; i<7; i++ )
	    id[i] = 0;
    }
    int id[ 7 ];
    int preeditStart;
    int preeditLength;
    uint ensureCursorVisibleInShowEvent : 1;
    uint tabChangesFocus : 1;
    QString scrollToAnchor; // used to deferr scrollToAnchor() until the show event when we are resized
    QString pressedName;
    QString onName;
#ifndef QT_NO_CLIPBOARD
    QClipboard::Mode clipboard_mode;
#endif
    QTimer *trippleClickTimer;
    QPoint trippleClickPoint;
#ifdef QT_TEXTEDIT_OPTIMIZATION
    QTextEditOptimPrivate * od;
    bool optimMode : 1;
    int maxLogLines;
    int logOffset;
#endif
    uint autoFormatting;
};

#ifndef QT_NO_MIME
class QRichTextDrag : public QTextDrag
{
public:
    QRichTextDrag( QWidget *dragSource = 0, const char *name = 0 );

    void setPlainText( const QString &txt ) { setText( txt ); }
    void setRichText( const QString &txt ) { richTxt = txt; }

    virtual QByteArray encodedData( const char *mime ) const;
    virtual const char* format( int i ) const;

    static bool decode( QMimeSource *e, QString &str, const QCString &mimetype,
			const QCString &subtype );
    static bool canDecode( QMimeSource* e );

private:
    QString richTxt;

};

QRichTextDrag::QRichTextDrag( QWidget *dragSource, const char *name )
    : QTextDrag( dragSource, name )
{
}

QByteArray QRichTextDrag::encodedData( const char *mime ) const
{
    if ( qstrcmp( "application/x-qrichtext", mime ) == 0 ) {
	return richTxt.utf8(); // #### perhaps we should use USC2 instead?
    } else
	return QTextDrag::encodedData( mime );
}

bool QRichTextDrag::decode( QMimeSource *e, QString &str, const QCString &mimetype,
			    const QCString &subtype )
{
    if ( mimetype == "application/x-qrichtext" ) {
	// do richtext decode
	const char *mime;
	int i;
	for ( i = 0; ( mime = e->format( i ) ); ++i ) {
	    if ( qstrcmp( "application/x-qrichtext", mime ) != 0 )
		continue;
	    str = QString::fromUtf8( e->encodedData( mime ) );
	    return TRUE;
	}
	return FALSE;
    }

    // do a regular text decode
    QCString subt = subtype;
    return QTextDrag::decode( e, str, subt );
}

bool QRichTextDrag::canDecode( QMimeSource* e )
{
    if ( e->provides( "application/x-qrichtext" ) )
	return TRUE;
    return QTextDrag::canDecode( e );
}

const char* QRichTextDrag::format( int i ) const
{
    if ( QTextDrag::format( i ) )
	return QTextDrag::format( i );
    if ( QTextDrag::format( i-1 ) )
	return "application/x-qrichtext";
    return 0;
}

#endif

static bool block_set_alignment = FALSE;

/*!
    \class QTextEdit qtextedit.h
    \brief The QTextEdit widget provides a powerful single-page rich text editor.

    \ingroup basic
    \ingroup text
    \mainclass

    \tableofcontents

    \section1 Introduction and Concepts

    QTextEdit is an advanced WYSIWYG viewer/editor supporting rich
    text formatting using HTML-style tags. It is optimized to handle
    large documents and to respond quickly to user input.

    QTextEdit has four modes of operation:
    \table
    \header \i Mode \i Command \i Notes
    \row \i Plain Text Editor \i setTextFormat(PlainText)
	 \i Set text with setText(); text() returns plain text. Text
	 attributes (e.g. colors) can be set, but plain text is always
	 returned.
    \row \i Rich Text Editor \i setTextFormat(RichText)
	 \i Set text with setText(); text() returns rich text. Rich
	 text editing is fairly limited. You can't set margins or
	 insert images for example (although you can read and
	 correctly display files that have margins set and that
	 include images). This mode is mostly useful for editing small
	 amounts of rich text. <sup>1.</sup>
    \row \i Text Viewer \i setReadOnly(TRUE)
         \i Set text with setText() or append() (which has no undo
	 history so is faster and uses less memory); text() returns
	 plain or rich text depending on the textFormat(). This mode
	 can correctly display a large subset of HTML tags.
    \row \i Log Viewer \i setTextFormat(LogText)
	 \i Append text using append(). The widget is set to be read
	 only and rich text support is disabled although a few HTML
	 tags (for color, bold, italic and underline) may be used.
	 (See \link #logtextmode LogText mode\endlink for details.)
    \endtable

    <sup>1.</sup><small>A more complete API that supports setting
    margins, images, etc., is planned for a later Qt release.</small>

    QTextEdit can be used as a syntax highlighting editor when used in
    conjunction with QSyntaxHighlighter.

    We recommend that you always call setTextFormat() to set the mode
    you want to use. If you use \c AutoText then setText() and
    append() will try to determine whether the text they are given is
    plain text or rich text. If you use \c RichText then setText() and
    append() will assume that the text they are given is rich text.
    insert() simply inserts the text it is given.

    QTextEdit works on paragraphs and characters. A paragraph is a
    formatted string which is word-wrapped to fit into the width of
    the widget. By default when reading plain text, one newline
    signify a paragraph. A document consists of zero or more
    paragraphs, indexed from 0. Characters are indexed on a
    per-paragraph basis, also indexed from 0. The words in the
    paragraph are aligned in accordance with the paragraph's
    alignment(). Paragraphs are separated by hard line breaks. Each
    character within a paragraph has its own attributes, for example,
    font and color.

    The text edit documentation uses the following concepts:
    \list
    \i \e{current format} --
    this is the format at the current cursor position, \e and it
    is the format of the selected text if any.
    \i \e{current paragraph} -- the paragraph which contains the
    cursor.
    \endlist

    QTextEdit can display images (using QMimeSourceFactory), lists and
    tables. If the text is too large to view within the text edit's
    viewport, scrollbars will appear. The text edit can load both
    plain text and HTML files (a subset of HTML 3.2 and 4). The
    rendering style and the set of valid tags are defined by a
    styleSheet(). Custom tags can be created and placed in a custom
    style sheet. Change the style sheet with \l{setStyleSheet()}; see
    QStyleSheet for details. The images identified by image tags are
    displayed if they can be interpreted using the text edit's
    \l{QMimeSourceFactory}; see setMimeSourceFactory().

    If you want a text browser with more navigation use QTextBrowser.
    If you just need to display a small piece of rich text use QLabel
    or QSimpleRichText.

    If you create a new QTextEdit, and want to allow the user to edit
    rich text, call setTextFormat(Qt::RichText) to ensure that the
    text is treated as rich text. (Rich text uses HTML tags to set
    text formatting attributes. See QStyleSheet for information on the
    HTML tags that are supported.). If you don't call setTextFormat()
    explicitly the text edit will guess from the text itself whether
    it is rich text or plain text. This means that if the text looks
    like HTML or XML it will probably be interpreted as rich text, so
    you should call setTextFormat(Qt::PlainText) to preserve such
    text.

    Note that we do not intend to add a full-featured web browser
    widget to Qt (because that would easily double Qt's size and only
    a few applications would benefit from it). The rich
    text support in Qt is designed to provide a fast, portable and
    efficient way to add reasonable online help facilities to
    applications, and to provide a basis for rich text editors.

    \section1 Using QTextEdit as a Display Widget

    QTextEdit can display a large HTML subset, including tables and
    images.

    The text is set or replaced using setText() which deletes any
    existing text and replaces it with the text passed in the
    setText() call. If you call setText() with legacy HTML (with
    setTextFormat(RichText) in force), and then call text(), the text
    that is returned may have different markup, but will render the
    same. Text can be inserted with insert(), paste(), pasteSubType()
    and append(). Text that is appended does not go into the undo
    history; this makes append() faster and consumes less memory. Text
    can also be cut(). The entire text is deleted with clear() and the
    selected text is deleted with removeSelectedText(). Selected
    (marked) text can also be deleted with del() (which will delete
    the character to the right of the cursor if no text is selected).

    Loading and saving text is achieved using setText() and text(),
    for example:
    \code
    QFile file( fileName ); // Read the text from a file
    if ( file.open( IO_ReadOnly ) ) {
	QTextStream stream( &file );
	textEdit->setText( stream.read() );
    }

    QFile file( fileName ); // Write the text to a file
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream stream( &file );
	stream << textEdit->text();
	textEdit->setModified( FALSE );
    }
    \endcode

    By default the text edit wraps words at whitespace to fit within
    the text edit widget. The setWordWrap() function is used to
    specify the kind of word wrap you want, or \c NoWrap if you don't
    want any wrapping. Call setWordWrap() to set a fixed pixel width
    \c FixedPixelWidth, or character column (e.g. 80 column) \c
    FixedColumnWidth with the pixels or columns specified with
    setWrapColumnOrWidth(). If you use word wrap to the widget's width
    \c WidgetWidth, you can specify whether to break on whitespace or
    anywhere with setWrapPolicy().

    The background color is set differently than other widgets, using
    setPaper(). You specify a brush style which could be a plain color
    or a complex pixmap.

    Hypertext links are automatically underlined; this can be changed
    with setLinkUnderline(). The tab stop width is set with
    setTabStopWidth().

    The zoomIn() and zoomOut() functions can be used to resize the
    text by increasing (decreasing for zoomOut()) the point size used.
    Images are not affected by the zoom functions.

    The lines() function returns the number of lines in the text and
    paragraphs() returns the number of paragraphs. The number of lines
    within a particular paragraph is returned by linesOfParagraph().
    The length of the entire text in characters is returned by
    length().

    You can scroll to an anchor in the text, e.g.
    \c{<a name="anchor">} with scrollToAnchor(). The find() function
    can be used to find and select a given string within the text.

    A read-only QTextEdit provides the same functionality as the
    (obsolete) QTextView. (QTextView is still supplied for
    compatibility with old code.)

    \section2 Read-only key bindings

    When QTextEdit is used read-only the key-bindings are limited to
    navigation, and text may only be selected with the mouse:
    \table
    \header \i Keypresses \i Action
    \row \i UpArrow	\i Move one line up
    \row \i DownArrow	\i Move one line down
    \row \i LeftArrow	\i Move one character left
    \row \i RightArrow	\i Move one character right
    \row \i PageUp	\i Move one (viewport) page up
    \row \i PageDown	\i Move one (viewport) page down
    \row \i Home	\i Move to the beginning of the text
    \row \i End		\i Move to the end of the text
    \row \i Shift+Wheel
	 \i Scroll the page horizontally (the Wheel is the mouse wheel)
    \row \i Ctrl+Wheel	\i Zoom the text
    \endtable

    The text edit may be able to provide some meta-information. For
    example, the documentTitle() function will return the text from
    within HTML \c{<title>} tags.

    The text displayed in a text edit has a \e context. The context is
    a path which the text edit's QMimeSourceFactory uses to resolve
    the locations of files and images. It is passed to the
    mimeSourceFactory() when quering data. (See QTextEdit() and
    \l{context()}.)

    \target logtextmode
    \section2 Using QTextEdit in LogText Mode

    Setting the text format to \c LogText puts the widget in a special
    mode which is optimized for very large texts. Editing, word wrap,
    and rich text support are disabled in this mode (the widget is
    explicitly made read-only). This allows the text to be stored in a
    different, more memory efficient manner. However, a certain degree
    of text formatting is supported through the use of formatting tags.
    A tag is delimited by \c < and \c {>}. The characters \c {<}, \c >
    and \c & are escaped by using \c {&lt;}, \c {&gt;} and \c {&amp;}.
    A tag pair consists of a left and a right tag (or open/close tags).
    Left-tags mark the starting point for formatting, while right-tags
    mark the ending point. A right-tag always start with a \c / before
    the tag keyword. For example \c <b> and \c </b> are a tag pair.
    Tags can be nested, but they have to be closed in the same order as
    they are opened. For example, \c <b><u></u></b> is valid, while \c
    <b><u></b></u> will output an error message.

    By using tags it is possible to change the color, bold, italic and
    underline settings for a piece of text. A color can be specified
    by using the HTML font tag \c {<font color=colorname>}. The color
    name can be one of the color names from the X11 color database, or
    a RGB hex value (e.g \c {#00ff00}). Example of valid color tags:
    \c {<font color=red>}, \c {<font color="light blue">}, \c {<font
    color="#223344">}. Bold, italic and underline settings can be
    specified by the tags \c {<b>}, \c <i> and \c {<u>}. Note that a
    tag does not necessarily have to be closed. A valid example:
    \code
    This is <font color=red>red</font> while <b>this</b> is <font color=blue>blue</font>.
    <font color=green><font color=yellow>Yellow,</font> and <u>green</u>.
    \endcode

    Stylesheets can also be used in LogText mode. To create and use a
    custom tag, you could do the following:
    \code
    QTextEdit * log = new QTextEdit( this );
    log->setTextFormat( Qt::LogText );
    QStyleSheetItem * item = new QStyleSheetItem( log->styleSheet(), "mytag" );
    item->setColor( "red" );
    item->setFontWeight( QFont::Bold );
    item->setFontUnderline( TRUE );
    log->append( "This is a <mytag>custom tag</mytag>!" );
    \endcode
    Note that only the color, bold, underline and italic attributes of
    a QStyleSheetItem is used in LogText mode.

    Note that you can use setMaxLogLines() to limit the number of
    lines the widget can hold in LogText mode.

    There are a few things that you need to be aware of when the
    widget is in this mode:
    \list
    \i Functions that deal with rich text formatting and cursor
    movement will not work or return anything valid.
    \i Lines are equivalent to paragraphs.
    \endlist

    \section1 Using QTextEdit as an Editor

    All the information about using QTextEdit as a display widget also
    applies here.

    The current format's attributes are set with setItalic(),
    setBold(), setUnderline(), setFamily() (font family),
    setPointSize(), setColor() and setCurrentFont(). The current
    paragraph's alignment is set with setAlignment().

    Use setSelection() to select text. The setSelectionAttributes()
    function is used to indicate how selected text should be
    displayed. Use hasSelectedText() to find out if any text is
    selected. The currently selected text's position is available
    using getSelection() and the selected text itself is returned by
    selectedText(). The selection can be copied to the clipboard with
    copy(), or cut to the clipboard with cut(). It can be deleted with
    removeSelectedText(). The entire text can be selected (or
    unselected) using selectAll(). QTextEdit supports multiple
    selections. Most of the selection functions operate on the default
    selection, selection 0. If the user presses a non-selecting key,
    e.g. a cursor key without also holding down Shift, all selections
    are cleared.

    Set and get the position of the cursor with setCursorPosition()
    and getCursorPosition() respectively. When the cursor is moved,
    the signals currentFontChanged(), currentColorChanged() and
    currentAlignmentChanged() are emitted to reflect the font, color
    and alignment at the new cursor position.

    If the text changes, the textChanged() signal is emitted, and if
    the user inserts a new line by pressing Return or Enter,
    returnPressed() is emitted. The isModified() function will return
    TRUE if the text has been modified.

    QTextEdit provides command-based undo and redo. To set the depth
    of the command history use setUndoDepth() which defaults to 100
    steps. To undo or redo the last operation call undo() or redo().
    The signals undoAvailable() and redoAvailable() indicate whether
    the undo and redo operations can be executed.

    The indent() function is used to reindent a paragraph. It is
    useful for code editors, for example in \link designer-manual.book
    Qt Designer\endlink's code editor \e{Ctrl+I} invokes the indent()
    function.

    \section2 Editing key bindings

    The list of key-bindings which are implemented for editing:
    \table
    \header \i Keypresses \i Action
    \row \i Backspace \i Delete the character to the left of the cursor
    \row \i Delete \i Delete the character to the right of the cursor
    \row \i Ctrl+A \i Move the cursor to the beginning of the line
    \row \i Ctrl+B \i Move the cursor one character left
    \row \i Ctrl+C \i Copy the marked text to the clipboard (also
		      Ctrl+Insert under Windows)
    \row \i Ctrl+D \i Delete the character to the right of the cursor
    \row \i Ctrl+E \i Move the cursor to the end of the line
    \row \i Ctrl+F \i Move the cursor one character right
    \row \i Ctrl+H \i Delete the character to the left of the cursor
    \row \i Ctrl+K \i Delete to end of line
    \row \i Ctrl+N \i Move the cursor one line down
    \row \i Ctrl+P \i Move the cursor one line up
    \row \i Ctrl+V \i Paste the clipboard text into line edit
		      (also Shift+Insert under Windows)
    \row \i Ctrl+X \i Cut the marked text, copy to clipboard
		      (also Shift+Delete under Windows)
    \row \i Ctrl+Z \i Undo the last operation
    \row \i Ctrl+Y \i Redo the last operation
    \row \i LeftArrow	    \i Move the cursor one character left
    \row \i Ctrl+LeftArrow  \i Move the cursor one word left
    \row \i RightArrow	    \i Move the cursor one character right
    \row \i Ctrl+RightArrow \i Move the cursor one word right
    \row \i UpArrow	    \i Move the cursor one line up
    \row \i Ctrl+UpArrow    \i Move the cursor one word up
    \row \i DownArrow	    \i Move the cursor one line down
    \row \i Ctrl+Down Arrow \i Move the cursor one word down
    \row \i PageUp	    \i Move the cursor one page up
    \row \i PageDown	    \i Move the cursor one page down
    \row \i Home	    \i Move the cursor to the beginning of the line
    \row \i Ctrl+Home	    \i Move the cursor to the beginning of the text
    \row \i End		    \i Move the cursor to the end of the line
    \row \i Ctrl+End	    \i Move the cursor to the end of the text
    \row \i Shift+Wheel	    \i Scroll the page horizontally
			    (the Wheel is the mouse wheel)
    \row \i Ctrl+Wheel	    \i Zoom the text
    \endtable

    To select (mark) text hold down the Shift key whilst pressing one
    of the movement keystrokes, for example, \e{Shift+Right Arrow}
    will select the character to the right, and \e{Shift+Ctrl+Right
    Arrow} will select the word to the right, etc.

    By default the text edit widget operates in insert mode so all
    text that the user enters is inserted into the text edit and any
    text to the right of the cursor is moved out of the way. The mode
    can be changed to overwrite, where new text overwrites any text to
    the right of the cursor, using setOverwriteMode().
*/

/*!
    \enum QTextEdit::AutoFormatting

    \value AutoNone Do not perform any automatic formatting
    \value AutoBulletList Only automatically format bulletted lists
    \value AutoAll Apply all available autoformatting
*/


/*!
    \enum QTextEdit::KeyboardAction

    This enum is used by doKeyboardAction() to specify which action
    should be executed:

    \value ActionBackspace  Delete the character to the left of the
    cursor.

    \value ActionDelete  Delete the character to the right of the
    cursor.

    \value ActionReturn  Split the paragraph at the cursor position.

    \value ActionKill If the cursor is not at the end of the
    paragraph, delete the text from the cursor position until the end
    of the paragraph. If the cursor is at the end of the paragraph,
    delete the hard line break at the end of the paragraph: this will
    cause this paragraph to be joined with the following paragraph.

    \value ActionWordBackspace Delete the word to the left of the
    cursor position.

    \value ActionWordDelete Delete the word to the right of the
    cursor position

*/

/*!
    \enum QTextEdit::VerticalAlignment

    This enum is used to set the vertical alignment of the text.

    \value AlignNormal Normal alignment
    \value AlignSuperScript Superscript
    \value AlignSubScript Subscript
*/

/*!
    \enum QTextEdit::TextInsertionFlags

    \internal

    \value RedoIndentation
    \value CheckNewLines
    \value RemoveSelected
*/


/*!
    \fn void QTextEdit::copyAvailable(bool yes)

    This signal is emitted when text is selected or de-selected in the
    text edit.

    When text is selected this signal will be emitted with \a yes set
    to TRUE. If no text has been selected or if the selected text is
    de-selected this signal is emitted with \a yes set to FALSE.

    If \a yes is TRUE then copy() can be used to copy the selection to
    the clipboard. If \a yes is FALSE then copy() does nothing.

    \sa selectionChanged()
*/


/*!
    \fn void QTextEdit::textChanged()

    This signal is emitted whenever the text in the text edit changes.

    \sa setText() append()
*/

/*!
    \fn void QTextEdit::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa setSelection() copyAvailable()
*/

/*!  \fn QTextDocument *QTextEdit::document() const

    \internal

  This function returns the QTextDocument which is used by the text
  edit.
*/

/*!  \fn void QTextEdit::setDocument( QTextDocument *doc )

    \internal

  This function sets the QTextDocument which should be used by the text
  edit to \a doc. This can be used, for example, if you want to
  display a document using multiple views. You would create a
  QTextDocument and set it to the text edits which should display it.
  You would need to connect to the textChanged() and
  selectionChanged() signals of all the text edits and update them all
  accordingly (preferably with a slight delay for efficiency reasons).
*/

/*!
    \enum QTextEdit::CursorAction

    This enum is used by moveCursor() to specify in which direction
    the cursor should be moved:

    \value MoveBackward  Moves the cursor one character backward

    \value MoveWordBackward Moves the cursor one word backward

    \value MoveForward  Moves the cursor one character forward

    \value MoveWordForward Moves the cursor one word forward

    \value MoveUp  Moves the cursor up one line

    \value MoveDown  Moves the cursor down one line

    \value MoveLineStart  Moves the cursor to the beginning of the line

    \value MoveLineEnd Moves the cursor to the end of the line

    \value MoveHome  Moves the cursor to the beginning of the document

    \value MoveEnd Moves the cursor to the end of the document

    \value MovePgUp  Moves the cursor one viewport page up

    \value MovePgDown  Moves the cursor one viewport page down
*/

/*!
  \enum Qt::AnchorAttribute

  An anchor has one or more of the following attributes:

  \value AnchorName the name attribute of the anchor. This attribute is
  used when scrolling to an anchor in the document.

  \value AnchorHref the href attribute of the anchor. This attribute is
  used when a link is clicked to determine what content to load.
*/

/*!
    \property QTextEdit::overwriteMode
    \brief the text edit's overwrite mode

    If FALSE (the default) characters entered by the user are inserted
    with any characters to the right being moved out of the way. If
    TRUE, the editor is in overwrite mode, i.e. characters entered by
    the user overwrite any characters to the right of the cursor
    position.
*/

/*!
    \fn void QTextEdit::setCurrentFont( const QFont &f )

    Sets the font of the current format to \a f.

    If the widget is in \c LogText mode this function will do
    nothing. Use setFont() instead.

    \sa currentFont() setPointSize() setFamily()
*/

/*!
    \property QTextEdit::undoDepth
    \brief the depth of the undo history

    The maximum number of steps in the undo/redo history. The default
    is 100.

    \sa undo() redo()
*/

/*!
    \fn void QTextEdit::undoAvailable( bool yes )

    This signal is emitted when the availability of undo changes. If
    \a yes is TRUE, then undo() will work until undoAvailable( FALSE )
    is next emitted.

    \sa undo() undoDepth()
*/

/*!
    \fn void QTextEdit::modificationChanged( bool m )

    This signal is emitted when the modification status of the
    document has changed. If \a m is TRUE, the document was modified,
    otherwise the modification state has been reset to unmodified.

    \sa modified
*/

/*!
    \fn void QTextEdit::redoAvailable( bool yes )

    This signal is emitted when the availability of redo changes. If
    \a yes is TRUE, then redo() will work until redoAvailable( FALSE )
    is next emitted.

    \sa redo() undoDepth()
*/

/*!
    \fn void QTextEdit::currentFontChanged( const QFont &f )

    This signal is emitted if the font of the current format has
    changed.

    The new font is \a f.

    \sa setCurrentFont()
*/

/*!
    \fn void QTextEdit::currentColorChanged( const QColor &c )

    This signal is emitted if the color of the current format has
    changed.

    The new color is \a c.

    \sa setColor()
*/

/*!
    \fn void QTextEdit::currentVerticalAlignmentChanged( VerticalAlignment a )

    This signal is emitted if the vertical alignment of the current
    format has changed.

    The new vertical alignment is \a a.

    \sa setVerticalAlignment()
*/

/*!
    \fn void QTextEdit::currentAlignmentChanged( int a )

    This signal is emitted if the alignment of the current paragraph
    has changed.

    The new alignment is \a a.

    \sa setAlignment()
*/

/*!
    \fn void QTextEdit::cursorPositionChanged( QTextCursor *c )

    \internal
*/

/*!
    \fn void QTextEdit::cursorPositionChanged( int para, int pos )

    This signal is emitted if the position of the cursor has changed.
    \a para contains the paragraph index and \a pos contains the
    character position within the paragraph.

    \sa setCursorPosition()
*/

/*!
    \fn void QTextEdit::clicked( int para, int pos )

    This signal is emitted when the mouse is clicked on the paragraph
    \a para at character position \a pos.

    \sa doubleClicked()
*/

/*! \fn void QTextEdit::doubleClicked( int para, int pos )

  This signal is emitted when the mouse is double-clicked on the
  paragraph \a para at character position \a pos.

  \sa clicked()
*/


/*!
    \fn void QTextEdit::returnPressed()

    This signal is emitted if the user pressed the Return or the Enter
    key.
*/

/*!
    \fn QTextCursor *QTextEdit::textCursor() const

    Returns the text edit's text cursor.

    \warning QTextCursor is not in the public API, but in special
    circumstances you might wish to use it.
*/

/*!
    Constructs an empty QTextEdit called \a name, with parent \a
    parent.
*/

QTextEdit::QTextEdit( QWidget *parent, const char *name )
    : QScrollView( parent, name, WStaticContents | WNoAutoErase ),
      doc( new QTextDocument( 0 ) ), undoRedoInfo( doc )
{
    init();
}

/*!
    Constructs a QTextEdit called \a name, with parent \a parent. The
    text edit will display the text \a text using context \a context.

    The \a context is a path which the text edit's QMimeSourceFactory
    uses to resolve the locations of files and images. It is passed to
    the mimeSourceFactory() when quering data.

    For example if the text contains an image tag,
    \c{<img src="image.png">}, and the context is "path/to/look/in", the
    QMimeSourceFactory will try to load the image from
    "path/to/look/in/image.png". If the tag was
    \c{<img src="/image.png">}, the context will not be used (because
    QMimeSourceFactory recognizes that we have used an absolute path)
    and will try to load "/image.png". The context is applied in exactly
    the same way to \e hrefs, for example,
    \c{<a href="target.html">Target</a>}, would resolve to
    "path/to/look/in/target.html".
*/

QTextEdit::QTextEdit( const QString& text, const QString& context,
		      QWidget *parent, const char *name)
    : QScrollView( parent, name, WStaticContents | WNoAutoErase ),
      doc( new QTextDocument( 0 ) ), undoRedoInfo( doc )
{
    init();
    setText( text, context );
}

/*!
    \reimp
*/

QTextEdit::~QTextEdit()
{
    delete undoRedoInfo.d;
    undoRedoInfo.d = 0;
    delete cursor;
    delete doc;
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	delete d->od;
#endif
    delete d;
}

void QTextEdit::init()
{
    d = new QTextEditPrivate;
    doc->formatCollection()->setPaintDevice( this );
    undoEnabled = TRUE;
    readonly = TRUE;
    setReadOnly( FALSE );
    setFrameStyle( LineEditPanel | Sunken );
    connect( doc, SIGNAL( minimumWidthChanged(int) ),
	     this, SLOT( documentWidthChanged(int) ) );

    mousePressed = FALSE;
    inDoubleClick = FALSE;
    modified = FALSE;
    onLink = QString::null;
    d->onName = QString::null;
    overWrite = FALSE;
    wrapMode = WidgetWidth;
    wrapWidth = -1;
    wPolicy = AtWhiteSpace;
    inDnD = FALSE;
    doc->setFormatter( new QTextFormatterBreakWords );
    doc->formatCollection()->defaultFormat()->setFont( QScrollView::font() );
    doc->formatCollection()->defaultFormat()->setColor( colorGroup().color( QColorGroup::Text ) );
    currentFormat = doc->formatCollection()->defaultFormat();
    currentAlignment = Qt::AlignAuto;

    setBackgroundMode( PaletteBase );
    viewport()->setBackgroundMode( PaletteBase );
    viewport()->setAcceptDrops( TRUE );
    resizeContents( 0, doc->lastParagraph() ?
		    ( doc->lastParagraph()->paragId() + 1 ) * doc->formatCollection()->defaultFormat()->height() : 0 );

    setKeyCompression( TRUE );
    viewport()->setMouseTracking( TRUE );
#ifndef QT_NO_CURSOR
    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
    cursor = new QTextCursor( doc );

    formatTimer = new QTimer( this );
    connect( formatTimer, SIGNAL( timeout() ),
	     this, SLOT( formatMore() ) );
    lastFormatted = doc->firstParagraph();

    scrollTimer = new QTimer( this );
    connect( scrollTimer, SIGNAL( timeout() ),
	     this, SLOT( autoScrollTimerDone() ) );

    interval = 0;
    changeIntervalTimer = new QTimer( this );
    connect( changeIntervalTimer, SIGNAL( timeout() ),
	     this, SLOT( doChangeInterval() ) );

    cursorVisible = TRUE;
    blinkTimer = new QTimer( this );
    connect( blinkTimer, SIGNAL( timeout() ),
	     this, SLOT( blinkCursor() ) );

#ifndef QT_NO_DRAGANDDROP
    dragStartTimer = new QTimer( this );
    connect( dragStartTimer, SIGNAL( timeout() ),
	     this, SLOT( startDrag() ) );
#endif

    d->trippleClickTimer = new QTimer( this );

    formatMore();

    blinkCursorVisible = FALSE;

    viewport()->setFocusProxy( this );
    viewport()->setFocusPolicy( WheelFocus );
    setInputMethodEnabled( TRUE );
    viewport()->installEventFilter( this );
    connect( this, SIGNAL(horizontalSliderReleased()), this, SLOT(sliderReleased()) );
    connect( this, SIGNAL(verticalSliderReleased()), this, SLOT(sliderReleased()) );
    installEventFilter( this );
}

void QTextEdit::paintDocument( bool drawAll, QPainter *p, int cx, int cy, int cw, int ch )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    Q_ASSERT( !d->optimMode );
    if ( d->optimMode )
	return;
#endif

    bool drawCur = hasFocus() || viewport()->hasFocus();
    if (( hasSelectedText() && !style().styleHint( QStyle::SH_BlinkCursorWhenTextSelected ) ) ||
	isReadOnly() || !cursorVisible || doc->hasSelection( QTextDocument::IMSelectionText ))
	drawCur = FALSE;
    QColorGroup g = colorGroup();
    const QColorGroup::ColorRole backRole = QPalette::backgroundRoleFromMode(backgroundMode());
    if ( doc->paper() )
	g.setBrush( backRole, *doc->paper() );

    if ( contentsY() < doc->y() ) {
	p->fillRect( contentsX(), contentsY(), visibleWidth(), doc->y(),
		     g.brush( backRole ) );
    }
    if ( drawAll && doc->width() - contentsX() < cx + cw ) {
	p->fillRect( doc->width() - contentsX(), cy, cx + cw - doc->width() + contentsX(), ch,
		     g.brush( backRole ) );
    }

    p->setBrushOrigin( -contentsX(), -contentsY() );

    lastFormatted = doc->draw( p, cx, cy, cw, ch, g, !drawAll, drawCur, cursor );

    if ( lastFormatted == doc->lastParagraph() )
	resizeContents( contentsWidth(), doc->height() );

    if ( contentsHeight() < visibleHeight() && ( !doc->lastParagraph() || doc->lastParagraph()->isValid() ) && drawAll )
	p->fillRect( 0, contentsHeight(), visibleWidth(),
		     visibleHeight() - contentsHeight(), g.brush( backRole ) );
}

/*!
    \reimp
*/

void QTextEdit::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimDrawContents( p, cx, cy, cw, ch );
	return;
    }
#endif
    paintDocument( TRUE, p, cx, cy, cw, ch );
    int v;
    p->setPen( foregroundColor() );
    if ( document()->isPageBreakEnabled() &&  ( v = document()->flow()->pageSize() ) > 0 ) {
	int l = int(cy / v) * v;
	while ( l < cy + ch ) {
	    p->drawLine( cx, l, cx + cw - 1, l );
	    l += v;
	}
    }
}

/*!
    \reimp
*/

void QTextEdit::drawContents( QPainter *p )
{
    if ( horizontalScrollBar()->isVisible() &&
	 verticalScrollBar()->isVisible() ) {
	const QRect verticalRect = verticalScrollBar()->geometry();
	const QRect horizontalRect = horizontalScrollBar()->geometry();

	QRect cornerRect;
	cornerRect.setTop( verticalRect.bottom() );
	cornerRect.setBottom( horizontalRect.bottom() );
	cornerRect.setLeft( verticalRect.left() );
	cornerRect.setRight( verticalRect.right() );

	p->fillRect( cornerRect, colorGroup().background() );
    }
}

/*!
    \reimp
*/

bool QTextEdit::event( QEvent *e )
{
    if ( e->type() == QEvent::AccelOverride && !isReadOnly() ) {
	QKeyEvent* ke = (QKeyEvent*) e;
	switch(ke->state()) {
	case NoButton:
	case Keypad:
	case ShiftButton:
	    if ( ke->key() < Key_Escape ) {
		ke->accept();
	    } else {
		switch ( ke->key() ) {
		case Key_Return:
		case Key_Enter:
		case Key_Delete:
		case Key_Home:
		case Key_End:
		case Key_Backspace:
		case Key_Left:
		case Key_Right:
		    ke->accept();
		default:
		    break;
		}
	    }
	    break;

	case ControlButton:
	case ControlButton|ShiftButton:
	case ControlButton|Keypad:
	case ControlButton|ShiftButton|Keypad:
	    switch ( ke->key() ) {
	    case Key_Tab:
	    case Key_Backtab:
		ke->ignore();
		break;
// Those are too frequently used for application functionality
/*	    case Key_A:
	    case Key_B:
	    case Key_D:
	    case Key_E:
	    case Key_F:
	    case Key_H:
	    case Key_I:
	    case Key_K:
	    case Key_N:
	    case Key_P:
	    case Key_T:
*/
	    case Key_C:
	    case Key_V:
	    case Key_X:
	    case Key_Y:
	    case Key_Z:
	    case Key_Left:
	    case Key_Right:
	    case Key_Up:
	    case Key_Down:
	    case Key_Home:
	    case Key_End:
#if defined (Q_WS_WIN)
	    case Key_Insert:
	    case Key_Delete:
#endif
		ke->accept();
	    default:
		break;
	    }
	    break;

	default:
	    switch ( ke->key() ) {
#if defined (Q_WS_WIN)
	    case Key_Insert:
		ke->accept();
#endif
	    default:
		break;
	    }
	    break;
	}
    }

    if ( e->type() == QEvent::Show ) {
	if (
#ifdef QT_TEXTEDIT_OPTIMIZATION
	     !d->optimMode &&
#endif
	     d->ensureCursorVisibleInShowEvent  ) {
	    ensureCursorVisible();
	    d->ensureCursorVisibleInShowEvent = FALSE;
	}
	if ( !d->scrollToAnchor.isEmpty() ) {
	    scrollToAnchor( d->scrollToAnchor );
	    d->scrollToAnchor = QString::null;
	}
    }
    return QWidget::event( e );
}

/*!
    Processes the key event, \a e. By default key events are used to
    provide keyboard navigation and text editing.
*/

void QTextEdit::keyPressEvent( QKeyEvent *e )
{
    changeIntervalTimer->stop();
    interval = 10;
    bool unknownKey = FALSE;
    if ( isReadOnly() ) {
	if ( !handleReadOnlyKeyEvent( e ) )
	    QScrollView::keyPressEvent( e );
	changeIntervalTimer->start( 100, TRUE );
	return;
    }


    bool selChanged = FALSE;
    for ( int i = 1; i < doc->numSelections(); ++i ) // start with 1 as we don't want to remove the Standard-Selection
	selChanged = doc->removeSelection( i ) || selChanged;

    if ( selChanged ) {
	cursor->paragraph()->document()->nextDoubleBuffered = TRUE;
	repaintChanged();
    }

    bool clearUndoRedoInfo = TRUE;


    switch ( e->key() ) {
    case Key_Left:
    case Key_Right: {
	// a bit hacky, but can't change this without introducing new enum values for move and keeping the
	// correct semantics and movement for BiDi and non BiDi text.
	CursorAction a;
	if ( cursor->paragraph()->string()->isRightToLeft() == (e->key() == Key_Right) )
	    a = e->state() & ControlButton ? MoveWordBackward : MoveBackward;
	else
	    a = e->state() & ControlButton ? MoveWordForward : MoveForward;
	moveCursor( a, e->state() & ShiftButton );
	break;
    }
    case Key_Up:
	moveCursor( e->state() & ControlButton ? MovePgUp : MoveUp, e->state() & ShiftButton );
	break;
    case Key_Down:
	moveCursor( e->state() & ControlButton ? MovePgDown : MoveDown, e->state() & ShiftButton );
	break;
    case Key_Home:
	moveCursor( e->state() & ControlButton ? MoveHome : MoveLineStart, e->state() & ShiftButton );
	break;
    case Key_End:
	moveCursor( e->state() & ControlButton ? MoveEnd : MoveLineEnd, e->state() & ShiftButton );
	break;
    case Key_Prior:
	moveCursor( MovePgUp, e->state() & ShiftButton );
	break;
    case Key_Next:
	moveCursor( MovePgDown, e->state() & ShiftButton );
	break;
    case Key_Return: case Key_Enter:
	if ( doc->hasSelection( QTextDocument::Standard, FALSE ) )
	    removeSelectedText();
	if ( textFormat() == Qt::RichText && ( e->state() & ControlButton ) ) {
	    // Ctrl-Enter inserts a line break in rich text mode
	    insert( QString( QChar( 0x2028) ), TRUE, FALSE );
	} else {
#ifndef QT_NO_CURSOR
	    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
	    clearUndoRedoInfo = FALSE;
	    doKeyboardAction( ActionReturn );
	    emit returnPressed();
	}
	break;
    case Key_Delete:
#if defined (Q_WS_WIN)
	if ( e->state() & ShiftButton ) {
	    cut();
	    break;
	} else
#endif
        if ( doc->hasSelection( QTextDocument::Standard, TRUE ) ) {
	    removeSelectedText();
	    break;
	}
	doKeyboardAction( e->state() & ControlButton ? ActionWordDelete
			  : ActionDelete );
	clearUndoRedoInfo = FALSE;

	break;
    case Key_Insert:
	if ( e->state() & ShiftButton )
	    paste();
#if defined (Q_WS_WIN)
	else if ( e->state() & ControlButton )
	    copy();
#endif
	else
	    setOverwriteMode( !isOverwriteMode() );
	break;
    case Key_Backspace:
#if defined (Q_WS_WIN)
	if ( e->state() & AltButton ) {
	    if (e->state() & ControlButton ) {
		break;
	    } else if ( e->state() & ShiftButton ) {
		redo();
		break;
	    } else {
		undo();
		break;
	    }
	} else
#endif
	if ( doc->hasSelection( QTextDocument::Standard, TRUE ) ) {
	    removeSelectedText();
	    break;
	}

	doKeyboardAction( e->state() & ControlButton ? ActionWordBackspace
			  : ActionBackspace );
	clearUndoRedoInfo = FALSE;
	break;
    case Key_F16: // Copy key on Sun keyboards
	copy();
	break;
    case Key_F18:  // Paste key on Sun keyboards
	paste();
	break;
    case Key_F20:  // Cut key on Sun keyboards
	cut();
	break;
    case Key_Direction_L:
	if ( doc->textFormat() == Qt::PlainText ) {
	    // change the whole doc
	    QTextParagraph *p = doc->firstParagraph();
	    while ( p ) {
		p->setDirection( QChar::DirL );
		p->setAlignment( Qt::AlignLeft );
		p->invalidate( 0 );
		p = p->next();
	    }
	} else {
	    if ( !cursor->paragraph() || cursor->paragraph()->direction() == QChar::DirL )
		return;
	    cursor->paragraph()->setDirection( QChar::DirL );
	    if ( cursor->paragraph()->length() <= 1&&
		 ( (cursor->paragraph()->alignment() & (Qt::AlignLeft | Qt::AlignRight) ) != 0 ) )
		setAlignment( Qt::AlignLeft );
	}
	repaintChanged();
	break;
    case Key_Direction_R:
	if ( doc->textFormat() == Qt::PlainText ) {
	    // change the whole doc
	    QTextParagraph *p = doc->firstParagraph();
	    while ( p ) {
		p->setDirection( QChar::DirR );
		p->setAlignment( Qt::AlignRight );
		p->invalidate( 0 );
		p = p->next();
	    }
	} else {
	    if ( !cursor->paragraph() || cursor->paragraph()->direction() == QChar::DirR )
		return;
	    cursor->paragraph()->setDirection( QChar::DirR );
	    if ( cursor->paragraph()->length() <= 1&&
		 ( (cursor->paragraph()->alignment() & (Qt::AlignLeft | Qt::AlignRight) ) != 0 ) )
		setAlignment( Qt::AlignRight );
	}
	repaintChanged();
	break;
    default: {
	    if ( e->text().length() &&
		( !( e->state() & ControlButton ) &&
#ifndef Q_OS_MACX
		  !( e->state() & AltButton ) &&
#endif
		  !( e->state() & MetaButton ) ||
		 ( ( (e->state()&ControlButton) | AltButton ) == (ControlButton|AltButton) ) ) &&
		 ( !e->ascii() || e->ascii() >= 32 || e->text() == "\t" ) ) {
		clearUndoRedoInfo = FALSE;
		if ( e->key() == Key_Tab ) {
		    if ( d->tabChangesFocus ) {
			e->ignore();
			break;
		    }
		    if ( textFormat() == Qt::RichText && cursor->index() == 0
			 && ( cursor->paragraph()->isListItem() || cursor->paragraph()->listDepth() ) ) {
			clearUndoRedo();
			undoRedoInfo.type = UndoRedoInfo::Style;
			undoRedoInfo.id = cursor->paragraph()->paragId();
			undoRedoInfo.eid = undoRedoInfo.id;
			undoRedoInfo.styleInformation = QTextStyleCommand::readStyleInformation( doc, undoRedoInfo.id, undoRedoInfo.eid );
			cursor->paragraph()->setListDepth( cursor->paragraph()->listDepth() +1 );
			clearUndoRedo();
			drawCursor( FALSE );
			repaintChanged();
			drawCursor( TRUE );
			break;
		    }
		} else if ( e->key() == Key_BackTab ) {
		    if ( d->tabChangesFocus ) {
			e->ignore();
			break;
		    }
		}

		if ( ( autoFormatting() & AutoBulletList ) &&
		     textFormat() == Qt::RichText && cursor->index() == 0
		     && !cursor->paragraph()->isListItem()
		     && ( e->text()[0] == '-' || e->text()[0] == '*' ) ) {
			clearUndoRedo();
			undoRedoInfo.type = UndoRedoInfo::Style;
			undoRedoInfo.id = cursor->paragraph()->paragId();
			undoRedoInfo.eid = undoRedoInfo.id;
			undoRedoInfo.styleInformation = QTextStyleCommand::readStyleInformation( doc, undoRedoInfo.id, undoRedoInfo.eid );
			setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListDisc );
			clearUndoRedo();
			drawCursor( FALSE );
			repaintChanged();
			drawCursor( TRUE );
			break;
		}
		if (overWrite && !cursor->atParagEnd() && !doc->hasSelection(QTextDocument::Standard)) {
                    doKeyboardAction(ActionDelete);
                    clearUndoRedoInfo = FALSE;
                }
		QString t = e->text();
#ifdef Q_WS_X11
		extern bool qt_hebrew_keyboard_hack;
		if ( qt_hebrew_keyboard_hack ) {
		    // the X11 keyboard layout is broken and does not reverse
		    // braces correctly. This is a hack to get halfway correct
		    // behaviour
		    QTextParagraph *p = cursor->paragraph();
		    if ( p && p->string() && p->string()->isRightToLeft() ) {
			QChar *c = (QChar *)t.unicode();
			int l = t.length();
			while( l-- ) {
			    if ( c->mirrored() )
				*c = c->mirroredChar();
			    c++;
			}
		    }
		}
#endif
		insert( t, TRUE, FALSE );
		break;
	    } else if ( e->state() & ControlButton ) {
		switch ( e->key() ) {
		case Key_C: case Key_F16: // Copy key on Sun keyboards
		    copy();
		    break;
		case Key_V:
		    paste();
		    break;
		case Key_X:
		    cut();
		    break;
		case Key_I: case Key_T: case Key_Tab:
		    if ( !d->tabChangesFocus )
			indent();
		    break;
		case Key_A:
#if defined(Q_WS_X11)
		    moveCursor( MoveLineStart, e->state() & ShiftButton );
#else
		    selectAll( TRUE );
#endif
		    break;
		case Key_B:
		    moveCursor( MoveBackward, e->state() & ShiftButton );
		    break;
		case Key_F:
		    moveCursor( MoveForward, e->state() & ShiftButton );
		    break;
		case Key_D:
		    if ( doc->hasSelection( QTextDocument::Standard ) ) {
			removeSelectedText();
			break;
		    }
		    doKeyboardAction( ActionDelete );
		    clearUndoRedoInfo = FALSE;
		    break;
		case Key_H:
		    if ( doc->hasSelection( QTextDocument::Standard ) ) {
			removeSelectedText();
			break;
		    }
		    if ( !cursor->paragraph()->prev() &&
			 cursor->atParagStart() )
			break;

		    doKeyboardAction( ActionBackspace );
		    clearUndoRedoInfo = FALSE;
		    break;
		case Key_E:
		    moveCursor( MoveLineEnd, e->state() & ShiftButton );
		    break;
		case Key_N:
		    moveCursor( MoveDown, e->state() & ShiftButton );
		    break;
		case Key_P:
		    moveCursor( MoveUp, e->state() & ShiftButton );
		    break;
		case Key_Z:
		    if(e->state() & ShiftButton)
			redo();
		    else
			undo();
		    break;
		case Key_Y:
		    redo();
		    break;
		case Key_K:
		    doKeyboardAction( ActionKill );
		    break;
#if defined(Q_WS_WIN)
		case Key_Insert:
		    copy();
		    break;
		case Key_Delete:
		    del();
		    break;
#endif
		default:
		    unknownKey = FALSE;
		    break;
		}
	    } else {
		unknownKey = TRUE;
	    }
        }
    }

    emit cursorPositionChanged( cursor );
    emit cursorPositionChanged( cursor->paragraph()->paragId(), cursor->index() );
    if ( clearUndoRedoInfo )
	clearUndoRedo();
    changeIntervalTimer->start( 100, TRUE );
    if ( unknownKey )
	e->ignore();
}

/*!
    \reimp
*/
void QTextEdit::imStartEvent( QIMEvent *e )
{
    if ( isReadOnly() ) {
	e->ignore();
	return;
    }

    if ( hasSelectedText() )
	removeSelectedText();
    d->preeditStart = cursor->index();
    clearUndoRedo();
    undoRedoInfo.type = UndoRedoInfo::IME;
}

/*!
    \reimp
*/
void QTextEdit::imComposeEvent( QIMEvent *e )
{
    if ( isReadOnly() ) {
	e->ignore();
	return;
    }

    doc->removeSelection( QTextDocument::IMCompositionText );
    doc->removeSelection( QTextDocument::IMSelectionText );

    if ( d->preeditLength > 0 && cursor->paragraph() )
	cursor->paragraph()->remove( d->preeditStart, d->preeditLength );
    cursor->setIndex( d->preeditStart );
    d->preeditLength = e->text().length();
    insert( e->text() );
    // insert can trigger an imEnd event as it emits a textChanged signal, so better
    // be careful
    if(d->preeditStart != -1) {
	cursor->setIndex( d->preeditStart + d->preeditLength );
	QTextCursor c = *cursor;
	cursor->setIndex( d->preeditStart );
	doc->setSelectionStart( QTextDocument::IMCompositionText, *cursor );
	doc->setSelectionEnd( QTextDocument::IMCompositionText, c );

	cursor->setIndex( d->preeditStart + e->cursorPos() );

	int sellen = e->selectionLength();
	if ( sellen > 0 ) {
	    cursor->setIndex( d->preeditStart + e->cursorPos() + sellen );
	    c = *cursor;
	    cursor->setIndex( d->preeditStart + e->cursorPos() );
	    doc->setSelectionStart( QTextDocument::IMSelectionText, *cursor );
	    doc->setSelectionEnd( QTextDocument::IMSelectionText, c );
	    cursor->setIndex( d->preeditStart + d->preeditLength );
	}
    }

    updateMicroFocusHint();
    repaintChanged();
}

/*!
    \reimp
*/
void QTextEdit::imEndEvent( QIMEvent *e )
{
    if ( isReadOnly() ) {
	e->ignore();
	return;
    }

    doc->removeSelection( QTextDocument::IMCompositionText );
    doc->removeSelection( QTextDocument::IMSelectionText );

    if (undoRedoInfo.type == UndoRedoInfo::IME)
        undoRedoInfo.type = UndoRedoInfo::Invalid;

    if ( d->preeditLength > 0 && cursor->paragraph() )
	cursor->paragraph()->remove( d->preeditStart, d->preeditLength );
    if ( d->preeditStart >= 0 ) {
        cursor->setIndex( d->preeditStart );
        insert( e->text() );
    }
    d->preeditStart = d->preeditLength = -1;

    repaintChanged();
}


static bool qtextedit_ignore_readonly = FALSE;

/*!
    Executes keyboard action \a action. This is normally called by a
    key event handler.
*/

void QTextEdit::doKeyboardAction( KeyboardAction action )
{
    if ( isReadOnly() && !qtextedit_ignore_readonly )
	return;

    if ( cursor->nestedDepth() != 0 ) // #### for 3.0, disable editing of tables as this is not advanced enough
	return;

    lastFormatted = cursor->paragraph();
    drawCursor( FALSE );
    bool doUpdateCurrentFormat = TRUE;

    switch ( action ) {
    case ActionWordDelete:
    case ActionDelete:
	if ( action == ActionDelete && !cursor->atParagEnd() ) {
	    if ( undoEnabled ) {
		checkUndoRedoInfo( UndoRedoInfo::Delete );
		if ( !undoRedoInfo.valid() ) {
		    undoRedoInfo.id = cursor->paragraph()->paragId();
		    undoRedoInfo.index = cursor->index();
		    undoRedoInfo.d->text = QString::null;
		}
		int idx = cursor->index();
		do {
		    undoRedoInfo.d->text.insert( undoRedoInfo.d->text.length(), cursor->paragraph()->at( idx++ ), TRUE );
		} while ( !cursor->paragraph()->string()->validCursorPosition( idx ) );
	    }
	    cursor->remove();
	} else {
	    clearUndoRedo();
	    doc->setSelectionStart( QTextDocument::Temp, *cursor );
	    if ( action == ActionWordDelete && !cursor->atParagEnd() ) {
		cursor->gotoNextWord();
	    } else {
		cursor->gotoNextLetter();
	    }
	    doc->setSelectionEnd( QTextDocument::Temp, *cursor );
	    removeSelectedText( QTextDocument::Temp );
	}
	break;
    case ActionWordBackspace:
    case ActionBackspace:
	if ( textFormat() == Qt::RichText
	     && (cursor->paragraph()->isListItem()
		 || cursor->paragraph()->listDepth() )
	     && cursor->index() == 0 ) {
	    if ( undoEnabled ) {
		clearUndoRedo();
		undoRedoInfo.type = UndoRedoInfo::Style;
		undoRedoInfo.id = cursor->paragraph()->paragId();
		undoRedoInfo.eid = undoRedoInfo.id;
		undoRedoInfo.styleInformation = QTextStyleCommand::readStyleInformation( doc, undoRedoInfo.id, undoRedoInfo.eid );
	    }
	    int ldepth = cursor->paragraph()->listDepth();
	    if ( cursor->paragraph()->isListItem() && ldepth == 1 ) {
		cursor->paragraph()->setListItem( FALSE );
	    } else if ( QMAX( ldepth, 1 ) == 1 ) {
		cursor->paragraph()->setListItem( FALSE );
		cursor->paragraph()->setListDepth( 0 );
	    } else {
		cursor->paragraph()->setListDepth( ldepth - 1 );
	    }
	    clearUndoRedo();
	    lastFormatted = cursor->paragraph();
	    repaintChanged();
	    drawCursor( TRUE );
	    return;
	}

	if ( action == ActionBackspace && !cursor->atParagStart() ) {
	    if ( undoEnabled ) {
		checkUndoRedoInfo( UndoRedoInfo::Delete );
		if ( !undoRedoInfo.valid() ) {
		    undoRedoInfo.id = cursor->paragraph()->paragId();
		    undoRedoInfo.index = cursor->index();
		    undoRedoInfo.d->text = QString::null;
		}
		undoRedoInfo.d->text.insert( 0, cursor->paragraph()->at( cursor->index()-1 ), TRUE );
		undoRedoInfo.index = cursor->index()-1;
	    }
	    cursor->removePreviousChar();
	    lastFormatted = cursor->paragraph();
	} else if ( cursor->paragraph()->prev()
		    || (action == ActionWordBackspace
			&& !cursor->atParagStart()) ) {
	    clearUndoRedo();
	    doc->setSelectionStart( QTextDocument::Temp, *cursor );
	    if ( action == ActionWordBackspace && !cursor->atParagStart() ) {
		cursor->gotoPreviousWord();
	    } else {
		cursor->gotoPreviousLetter();
	    }
	    doc->setSelectionEnd( QTextDocument::Temp, *cursor );
	    removeSelectedText( QTextDocument::Temp );
	}
	break;
    case ActionReturn:
	if ( undoEnabled ) {
	    checkUndoRedoInfo( UndoRedoInfo::Return );
	    if ( !undoRedoInfo.valid() ) {
		undoRedoInfo.id = cursor->paragraph()->paragId();
		undoRedoInfo.index = cursor->index();
		undoRedoInfo.d->text = QString::null;
	    }
	    undoRedoInfo.d->text += "\n";
	}
	cursor->splitAndInsertEmptyParagraph();
	if ( cursor->paragraph()->prev() ) {
	    lastFormatted = cursor->paragraph()->prev();
	    lastFormatted->invalidate( 0 );
	}
	doUpdateCurrentFormat = FALSE;
	break;
    case ActionKill:
	clearUndoRedo();
	doc->setSelectionStart( QTextDocument::Temp, *cursor );
	if ( cursor->atParagEnd() )
	    cursor->gotoNextLetter();
	else
	    cursor->setIndex( cursor->paragraph()->length() - 1 );
	doc->setSelectionEnd( QTextDocument::Temp, *cursor );
	removeSelectedText( QTextDocument::Temp );
	break;
    }

    formatMore();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
    updateMicroFocusHint();
    if ( doUpdateCurrentFormat )
	updateCurrentFormat();
    setModified();
    emit textChanged();
}

void QTextEdit::readFormats( QTextCursor &c1, QTextCursor &c2, QTextString &text, bool fillStyles )
{
#ifndef QT_NO_DATASTREAM
    QDataStream styleStream( undoRedoInfo.styleInformation, IO_WriteOnly );
#endif
    c2.restoreState();
    c1.restoreState();
    int lastIndex = text.length();
    if ( c1.paragraph() == c2.paragraph() ) {
	for ( int i = c1.index(); i < c2.index(); ++i )
	    text.insert( lastIndex + i - c1.index(), c1.paragraph()->at( i ), TRUE );
#ifndef QT_NO_DATASTREAM
	if ( fillStyles ) {
	    styleStream << (int) 1;
	    c1.paragraph()->writeStyleInformation( styleStream );
	}
#endif
    } else {
	int i;
	for ( i = c1.index(); i < c1.paragraph()->length()-1; ++i )
	    text.insert( lastIndex++, c1.paragraph()->at( i ), TRUE );
	int num = 2; // start and end, being different
	text += "\n"; lastIndex++;

        if (c1.paragraph()->next() != c2.paragraph()) {
            num += text.appendParagraphs(c1.paragraph()->next(), c2.paragraph());
            lastIndex = text.length();
        }

	for ( i = 0; i < c2.index(); ++i )
	    text.insert( i + lastIndex, c2.paragraph()->at( i ), TRUE );
#ifndef QT_NO_DATASTREAM
	if ( fillStyles ) {
	    styleStream << num;
	    for ( QTextParagraph *p = c1.paragraph(); --num >= 0; p = p->next() )
		p->writeStyleInformation( styleStream );
	}
#endif
    }
}

/*!
    Removes the selection \a selNum (by default 0). This does not
    remove the selected text.

    \sa removeSelectedText()
*/

void QTextEdit::removeSelection( int selNum )
{
    doc->removeSelection( selNum );
    repaintChanged();
}

/*!
    Deletes the text of selection \a selNum (by default, the default
    selection, 0). If there is no selected text nothing happens.

    \sa selectedText removeSelection()
*/

void QTextEdit::removeSelectedText( int selNum )
{
    if(selNum != 0)
        resetInputContext();

    QTextCursor c1 = doc->selectionStartCursor( selNum );
    c1.restoreState();
    QTextCursor c2 = doc->selectionEndCursor( selNum );
    c2.restoreState();

    // ### no support for editing tables yet, plus security for broken selections
    if ( c1.nestedDepth() || c2.nestedDepth() )
	return;

    for ( int i = 0; i < (int)doc->numSelections(); ++i ) {
	if ( i == selNum )
	    continue;
	doc->removeSelection( i );
    }

    drawCursor( FALSE );
    if ( undoEnabled ) {
	checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
	if ( !undoRedoInfo.valid() ) {
	    doc->selectionStart( selNum, undoRedoInfo.id, undoRedoInfo.index );
	    undoRedoInfo.d->text = QString::null;
	}
	readFormats( c1, c2, undoRedoInfo.d->text, TRUE );
    }

    doc->removeSelectedText( selNum, cursor );
    if ( cursor->isValid() ) {
	lastFormatted = 0; // make sync a noop
	ensureCursorVisible();
	lastFormatted = cursor->paragraph();
	formatMore();
	repaintContents( FALSE );
	ensureCursorVisible();
	drawCursor( TRUE );
	clearUndoRedo();
#if defined(Q_WS_WIN)
	// there seems to be a problem with repainting or erasing the area
	// of the scrollview which is not the contents on windows
	if ( contentsHeight() < visibleHeight() )
	    viewport()->repaint( 0, contentsHeight(), visibleWidth(), visibleHeight() - contentsHeight(), TRUE );
#endif
#ifndef QT_NO_CURSOR
	viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
	updateMicroFocusHint();
    } else {
	delete cursor;
	cursor = new QTextCursor( doc );
	drawCursor( TRUE );
	repaintContents( TRUE );
    }
    setModified();
    emit textChanged();
    emit selectionChanged();
    emit copyAvailable( doc->hasSelection( QTextDocument::Standard ) );
}

/*!
    Moves the text cursor according to \a action. This is normally
    used by some key event handler. \a select specifies whether the
    text between the current cursor position and the new position
    should be selected.
*/

void QTextEdit::moveCursor( CursorAction action, bool select )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return;
#endif
#ifdef Q_WS_MACX
    QTextCursor c1 = *cursor;
    QTextCursor c2;
#endif
    drawCursor( FALSE );
    if ( select ) {
	if ( !doc->hasSelection( QTextDocument::Standard ) )
	    doc->setSelectionStart( QTextDocument::Standard, *cursor );
	moveCursor( action );
#ifdef Q_WS_MACX
	c2 = *cursor;
	if (c1 == c2)
	    if (action == MoveDown || action == MovePgDown)
		moveCursor( MoveEnd );
	    else if (action == MoveUp || action == MovePgUp)
		moveCursor( MoveHome );
#endif
	if ( doc->setSelectionEnd( QTextDocument::Standard, *cursor ) ) {
	    cursor->paragraph()->document()->nextDoubleBuffered = TRUE;
	    repaintChanged();
	} else {
	    drawCursor( TRUE );
	}
	ensureCursorVisible();
	emit selectionChanged();
	emit copyAvailable( doc->hasSelection( QTextDocument::Standard ) );
    } else {
#ifdef Q_WS_MACX
	QTextCursor cStart = doc->selectionStartCursor( QTextDocument::Standard );
	QTextCursor cEnd = doc->selectionEndCursor( QTextDocument::Standard );
	bool redraw = doc->removeSelection( QTextDocument::Standard );
	if (redraw && action == MoveDown)
	    *cursor = cEnd;
	else if (redraw && action == MoveUp)
	    *cursor = cStart;
	if (redraw && action == MoveForward)
	    *cursor = cEnd;
	else if (redraw && action == MoveBackward)
	    *cursor = cStart;
	else
	    moveCursor( action );
	c2 = *cursor;
	if (c1 == c2)
	    if (action == MoveDown)
		moveCursor( MoveEnd );
	    else if (action == MoveUp)
		moveCursor( MoveHome );
#else
	bool redraw = doc->removeSelection( QTextDocument::Standard );
	moveCursor( action );
#endif
	if ( !redraw ) {
	    ensureCursorVisible();
	    drawCursor( TRUE );
	} else {
	    cursor->paragraph()->document()->nextDoubleBuffered = TRUE;
	    repaintChanged();
	    ensureCursorVisible();
	    drawCursor( TRUE );
#ifndef QT_NO_CURSOR
	    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
	}
	if ( redraw ) {
	    emit copyAvailable( doc->hasSelection( QTextDocument::Standard ) );
	    emit selectionChanged();
	}
    }

    drawCursor( TRUE );
    updateCurrentFormat();
    updateMicroFocusHint();
}

/*!
    \overload
*/

void QTextEdit::moveCursor( CursorAction action )
{
    resetInputContext();
    switch ( action ) {
    case MoveBackward:
	cursor->gotoPreviousLetter();
	break;
    case MoveWordBackward:
	cursor->gotoPreviousWord();
	break;
    case MoveForward:
	cursor->gotoNextLetter();
	break;
    case MoveWordForward:
	cursor->gotoNextWord();
	break;
    case MoveUp:
	cursor->gotoUp();
	break;
    case MovePgUp:
	cursor->gotoPageUp( visibleHeight() );
	break;
    case MoveDown:
	cursor->gotoDown();
	break;
    case MovePgDown:
	cursor->gotoPageDown( visibleHeight() );
	break;
    case MoveLineStart:
	cursor->gotoLineStart();
	break;
    case MoveHome:
	cursor->gotoHome();
	break;
    case MoveLineEnd:
	cursor->gotoLineEnd();
	break;
    case MoveEnd:
	ensureFormatted( doc->lastParagraph() );
	cursor->gotoEnd();
	break;
    }
    updateMicroFocusHint();
    updateCurrentFormat();
}

/*!
    \reimp
*/

void QTextEdit::resizeEvent( QResizeEvent *e )
{
    QScrollView::resizeEvent( e );
    if ( doc->visibleWidth() == 0 )
	doResize();
}

/*!
    \reimp
*/

void QTextEdit::viewportResizeEvent( QResizeEvent *e )
{
    QScrollView::viewportResizeEvent( e );
    if ( e->oldSize().width() != e->size().width() ) {
	bool stayAtBottom = e->oldSize().height() != e->size().height() &&
	       contentsY() > 0 && contentsY() >= doc->height() - e->oldSize().height();
	doResize();
	if ( stayAtBottom )
	    scrollToBottom();
    }
}

/*!
    Ensures that the cursor is visible by scrolling the text edit if
    necessary.

    \sa setCursorPosition()
*/

void QTextEdit::ensureCursorVisible()
{
    // Not visible or the user is draging the window, so don't position to caret yet
    if ( !isUpdatesEnabled() || !isVisible() || isHorizontalSliderPressed() || isVerticalSliderPressed() ) {
	d->ensureCursorVisibleInShowEvent = TRUE;
	return;
    }
    sync();
    QTextStringChar *chr = cursor->paragraph()->at( cursor->index() );
    int h = cursor->paragraph()->lineHeightOfChar( cursor->index() );
    int x = cursor->paragraph()->rect().x() + chr->x + cursor->offsetX();
    int y = 0; int dummy;
    cursor->paragraph()->lineHeightOfChar( cursor->index(), &dummy, &y );
    y += cursor->paragraph()->rect().y() + cursor->offsetY();
    int w = 1;
    ensureVisible( x, y + h / 2, w, h / 2 + 2 );
}

/*!
    \internal
*/
void QTextEdit::sliderReleased()
{
    if ( d->ensureCursorVisibleInShowEvent && isVisible() ) {
	d->ensureCursorVisibleInShowEvent = FALSE;
	ensureCursorVisible();
    }
}

/*!
    \internal
*/
void QTextEdit::drawCursor( bool visible )
{
    if ( !isUpdatesEnabled() ||
	 !viewport()->isUpdatesEnabled() ||
	 !cursor->paragraph() ||
	 !cursor->paragraph()->isValid() ||
	 ( !style().styleHint( QStyle::SH_BlinkCursorWhenTextSelected ) &&
	   ( d->optimMode ? optimHasSelection() : doc->hasSelection( QTextDocument::Standard, TRUE ))) ||
	 ( visible && !hasFocus() && !viewport()->hasFocus() && !inDnD ) ||
         doc->hasSelection( QTextDocument::IMSelectionText ) ||
	 isReadOnly() )
	return;

    QPainter p( viewport() );
    QRect r( cursor->topParagraph()->rect() );
    cursor->paragraph()->setChanged( TRUE );
    p.translate( -contentsX() + cursor->totalOffsetX(), -contentsY() + cursor->totalOffsetY() );
    QPixmap *pix = 0;
    QColorGroup cg( colorGroup() );
    const QColorGroup::ColorRole backRole = QPalette::backgroundRoleFromMode(backgroundMode());
    if ( cursor->paragraph()->background() )
	cg.setBrush( backRole, *cursor->paragraph()->background() );
    else if ( doc->paper() )
	cg.setBrush( backRole, *doc->paper() );
    p.setBrushOrigin( -contentsX(), -contentsY() );
    cursor->paragraph()->document()->nextDoubleBuffered = TRUE;
    if ( !cursor->nestedDepth() ) {
	int h = cursor->paragraph()->lineHeightOfChar( cursor->index() );
	int dist = 5;
	if ( ( cursor->paragraph()->alignment() & Qt::AlignJustify ) == Qt::AlignJustify )
	    dist = 50;
	int x = r.x() - cursor->totalOffsetX() + cursor->x() - dist;
	x = QMAX( x, 0 );
	p.setClipRect( QRect( x - contentsX(),
			      r.y() - cursor->totalOffsetY() + cursor->y() - contentsY(), 2 * dist, h ) );
	doc->drawParagraph( &p, cursor->paragraph(), x,
			r.y() - cursor->totalOffsetY() + cursor->y(), 2 * dist, h, pix, cg, visible, cursor );
    } else {
	doc->drawParagraph( &p, cursor->paragraph(), r.x() - cursor->totalOffsetX(),
			r.y() - cursor->totalOffsetY(), r.width(), r.height(),
			pix, cg, visible, cursor );
    }
    cursorVisible = visible;
}

enum {
    IdUndo = 0,
    IdRedo = 1,
    IdCut = 2,
    IdCopy = 3,
    IdPaste = 4,
    IdClear = 5,
    IdSelectAll = 6
};

/*!
    \reimp
*/
#ifndef QT_NO_WHEELEVENT
void QTextEdit::contentsWheelEvent( QWheelEvent *e )
{
    if ( isReadOnly() ) {
	if ( e->state() & ControlButton ) {
	    if ( e->delta() > 0 )
		zoomOut();
	    else if ( e->delta() < 0 )
		zoomIn();
	    return;
	}
    }
    QScrollView::contentsWheelEvent( e );
}
#endif

/*!
    \reimp
*/

void QTextEdit::contentsMousePressEvent( QMouseEvent *e )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimMousePressEvent( e );
	return;
    }
#endif

    if ( d->trippleClickTimer->isActive() &&
	 ( e->globalPos() - d->trippleClickPoint ).manhattanLength() <
	 QApplication::startDragDistance() ) {
	QTextCursor c1 = *cursor;
	QTextCursor c2 = *cursor;
	c1.gotoLineStart();
	c2.gotoLineEnd();
	doc->setSelectionStart( QTextDocument::Standard, c1 );
	doc->setSelectionEnd( QTextDocument::Standard, c2 );
	*cursor = c2;
	repaintChanged();
	mousePressed = TRUE;
	return;
    }

    clearUndoRedo();
    QTextCursor oldCursor = *cursor;
    QTextCursor c = *cursor;
    mousePos = e->pos();
    mightStartDrag = FALSE;
    pressedLink = QString::null;
    d->pressedName = QString::null;

    if ( e->button() == LeftButton ) {
	mousePressed = TRUE;
	drawCursor( FALSE );
	placeCursor( e->pos() );
	ensureCursorVisible();

	if ( isReadOnly() && linksEnabled() ) {
	    QTextCursor c = *cursor;
	    placeCursor( e->pos(), &c, TRUE );
	    if ( c.paragraph() && c.paragraph()->at( c.index() ) &&
		 c.paragraph()->at( c.index() )->isAnchor() ) {
		pressedLink = c.paragraph()->at( c.index() )->anchorHref();
		d->pressedName = c.paragraph()->at( c.index() )->anchorName();
	    }
	}

#ifndef QT_NO_DRAGANDDROP
	if ( doc->inSelection( QTextDocument::Standard, e->pos() ) ) {
	    mightStartDrag = TRUE;
	    drawCursor( TRUE );
	    dragStartTimer->start( QApplication::startDragTime(), TRUE );
	    dragStartPos = e->pos();
	    return;
	}
#endif

	bool redraw = FALSE;
	if ( doc->hasSelection( QTextDocument::Standard ) ) {
	    if ( !( e->state() & ShiftButton ) ) {
		redraw = doc->removeSelection( QTextDocument::Standard );
		doc->setSelectionStart( QTextDocument::Standard, *cursor );
	    } else {
		redraw = doc->setSelectionEnd( QTextDocument::Standard, *cursor ) || redraw;
	    }
	} else {
	    if ( isReadOnly() || !( e->state() & ShiftButton ) ) {
		doc->setSelectionStart( QTextDocument::Standard, *cursor );
	    } else {
		doc->setSelectionStart( QTextDocument::Standard, c );
		redraw = doc->setSelectionEnd( QTextDocument::Standard, *cursor ) || redraw;
	    }
	}

	for ( int i = 1; i < doc->numSelections(); ++i ) // start with 1 as we don't want to remove the Standard-Selection
	    redraw = doc->removeSelection( i ) || redraw;

	if ( !redraw ) {
	    drawCursor( TRUE );
	} else {
	    repaintChanged();
#ifndef QT_NO_CURSOR
	    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
	}
    } else if ( e->button() == MidButton ) {
	bool redraw = doc->removeSelection( QTextDocument::Standard );
	if ( !redraw ) {
	    drawCursor( TRUE );
	} else {
	    repaintChanged();
#ifndef QT_NO_CURSOR
	    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
	}
    }

    if ( *cursor != oldCursor )
	updateCurrentFormat();
}

/*!
    \reimp
*/

void QTextEdit::contentsMouseMoveEvent( QMouseEvent *e )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimMouseMoveEvent( e );
	return;
    }
#endif
    if ( mousePressed ) {
#ifndef QT_NO_DRAGANDDROP
	if ( mightStartDrag ) {
	    dragStartTimer->stop();
	    if ( ( e->pos() - dragStartPos ).manhattanLength() > QApplication::startDragDistance() ) {
		QGuardedPtr<QTextEdit> guard( this );
		startDrag();
		if (guard.isNull()) // we got deleted during the dnd
                    return;
	    }
#ifndef QT_NO_CURSOR
	    if ( !isReadOnly() )
		viewport()->setCursor( ibeamCursor );
#endif
	    return;
	}
#endif
	mousePos = e->pos();
	handleMouseMove( mousePos );
	oldMousePos = mousePos;
    }

#ifndef QT_NO_CURSOR
    if ( !isReadOnly() && !mousePressed ) {
	if ( doc->hasSelection( QTextDocument::Standard ) && doc->inSelection( QTextDocument::Standard, e->pos() ) )
	    viewport()->setCursor( arrowCursor );
	else
	    viewport()->setCursor( ibeamCursor );
    }
#endif
    updateCursor( e->pos() );
}

void QTextEdit::copyToClipboard()
{
#ifndef QT_NO_CLIPBOARD
    if (QApplication::clipboard()->supportsSelection()) {
	d->clipboard_mode = QClipboard::Selection;

	// don't listen to selection changes
	disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);
	copy();
	// listen to selection changes
	connect( QApplication::clipboard(), SIGNAL(selectionChanged()),
		 this, SLOT(clipboardChanged()) );

	d->clipboard_mode = QClipboard::Clipboard;
    }
#endif
}

/*!
    \reimp
*/

void QTextEdit::contentsMouseReleaseEvent( QMouseEvent * e )
{
    if ( !inDoubleClick ) { // could be the release of a dblclick
	int para = 0;
	int index = charAt( e->pos(), &para );
	emit clicked( para, index );
    }
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimMouseReleaseEvent( e );
	return;
    }
#endif
    QTextCursor oldCursor = *cursor;
    if ( scrollTimer->isActive() )
	scrollTimer->stop();
#ifndef QT_NO_DRAGANDDROP
    if ( dragStartTimer->isActive() )
	dragStartTimer->stop();
    if ( mightStartDrag ) {
	selectAll( FALSE );
	mousePressed = FALSE;
    }
#endif
    bool mouseWasPressed = mousePressed;
    if ( mousePressed ) {
	mousePressed = FALSE;
	copyToClipboard();
    }
#ifndef QT_NO_CLIPBOARD
    else if ( e->button() == MidButton && !isReadOnly() ) {
        // only do middle-click pasting on systems that have selections (ie. X11)
        if (QApplication::clipboard()->supportsSelection()) {
            drawCursor( FALSE );
            placeCursor( e->pos() );
            ensureCursorVisible();
            doc->setSelectionStart( QTextDocument::Standard, oldCursor );
            bool redraw = FALSE;
            if ( doc->hasSelection( QTextDocument::Standard ) ) {
                redraw = doc->removeSelection( QTextDocument::Standard );
                doc->setSelectionStart( QTextDocument::Standard, *cursor );
            } else {
                doc->setSelectionStart( QTextDocument::Standard, *cursor );
            }
            // start with 1 as we don't want to remove the Standard-Selection
            for ( int i = 1; i < doc->numSelections(); ++i )
                redraw = doc->removeSelection( i ) || redraw;
            if ( !redraw ) {
                drawCursor( TRUE );
            } else {
                repaintChanged();
#ifndef QT_NO_CURSOR
                viewport()->setCursor( ibeamCursor );
#endif
            }
	    d->clipboard_mode = QClipboard::Selection;
            paste();
	    d->clipboard_mode = QClipboard::Clipboard;
        }
    }
#endif
    emit cursorPositionChanged( cursor );
    emit cursorPositionChanged( cursor->paragraph()->paragId(), cursor->index() );
    if ( oldCursor != *cursor )
	updateCurrentFormat();
    inDoubleClick = FALSE;

#ifndef QT_NO_NETWORKPROTOCOL
    if ( (   (!onLink.isEmpty() && onLink == pressedLink)
	  || (!d->onName.isEmpty() && d->onName == d->pressedName))
	 && linksEnabled() && mouseWasPressed ) {
	if (!onLink.isEmpty()) {
	    QUrl u( doc->context(), onLink, TRUE );
	    emitLinkClicked( u.toString( FALSE, FALSE ) );
	}
	if (::qt_cast<QTextBrowser*>(this)) { // change for 4.0
	    QConnectionList *clist = receivers(
			"anchorClicked(const QString&,const QString&)");
	    if (!signalsBlocked() && clist) {
		QUObject o[3];
		static_QUType_QString.set(o+1, d->onName);
		static_QUType_QString.set(o+2, onLink);
		activate_signal( clist, o);
	    }
	}

	// emitting linkClicked() may result in that the cursor winds
	// up hovering over a different valid link - check this and
	// set the appropriate cursor shape
	updateCursor( e->pos() );
    }
#endif
    drawCursor( TRUE );
    if ( !doc->hasSelection( QTextDocument::Standard, TRUE ) )
	doc->removeSelection( QTextDocument::Standard );

    emit copyAvailable( doc->hasSelection( QTextDocument::Standard ) );
    emit selectionChanged();
}

/*!
    \reimp
*/

void QTextEdit::contentsMouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() != Qt::LeftButton ) {
	e->ignore();
	return;
    }
    int para = 0;
    int index = charAt( e->pos(), &para );
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	QString str = d->od->lines[ LOGOFFSET(para) ];
	int startIdx = index, endIdx = index, i;
	if ( !str[ index ].isSpace() ) {
	    i = startIdx;
	    // find start of word
	    while ( i >= 0 && !str[ i ].isSpace() ) {
		startIdx = i--;
	    }
	    i = endIdx;
	    // find end of word..
	    while ( (uint) i < str.length() && !str[ i ].isSpace() ) {
		endIdx = ++i;
	    }
	    // ..and start of next
	    while ( (uint) i < str.length() && str[ i ].isSpace() ) {
		endIdx = ++i;
	    }
	    optimSetSelection( para, startIdx, para, endIdx );
	    repaintContents( FALSE );
	}
    } else
#endif
    {
	QTextCursor c1 = *cursor;
	QTextCursor c2 = *cursor;
#if defined(Q_OS_MAC)
	QTextParagraph *para = cursor->paragraph();
	if ( cursor->isValid() ) {
	    if ( para->at( cursor->index() )->c.isLetterOrNumber() ) {
		while ( c1.index() > 0 &&
			c1.paragraph()->at( c1.index()-1 )->c.isLetterOrNumber() )
		    c1.gotoPreviousLetter();
		while ( c2.paragraph()->at( c2.index() )->c.isLetterOrNumber() &&
			!c2.atParagEnd() )
		    c2.gotoNextLetter();
	    } else if ( para->at( cursor->index() )->c.isSpace() ) {
		while ( c1.index() > 0 &&
			c1.paragraph()->at( c1.index()-1 )->c.isSpace() )
		    c1.gotoPreviousLetter();
		while ( c2.paragraph()->at( c2.index() )->c.isSpace() &&
			!c2.atParagEnd() )
		    c2.gotoNextLetter();
	    } else if ( !c2.atParagEnd() ) {
		c2.gotoNextLetter();
	    }
	}
#else
	if ( cursor->index() > 0 && !cursor->paragraph()->at( cursor->index()-1 )->c.isSpace() )
	    c1.gotoPreviousWord();
	if ( !cursor->paragraph()->at( cursor->index() )->c.isSpace() && !cursor->atParagEnd() )
	    c2.gotoNextWord();
#endif
	doc->setSelectionStart( QTextDocument::Standard, c1 );
	doc->setSelectionEnd( QTextDocument::Standard, c2 );

	*cursor = c2;

	repaintChanged();

	d->trippleClickTimer->start( qApp->doubleClickInterval(), TRUE );
	d->trippleClickPoint = e->globalPos();
    }
    inDoubleClick = TRUE;
    mousePressed = TRUE;
    emit doubleClicked( para, index );
}

#ifndef QT_NO_DRAGANDDROP

/*!
    \reimp
*/

void QTextEdit::contentsDragEnterEvent( QDragEnterEvent *e )
{
    if ( isReadOnly() || !QTextDrag::canDecode( e ) ) {
	e->ignore();
	return;
    }
    e->acceptAction();
    inDnD = TRUE;
}

/*!
    \reimp
*/

void QTextEdit::contentsDragMoveEvent( QDragMoveEvent *e )
{
    if ( isReadOnly() || !QTextDrag::canDecode( e ) ) {
	e->ignore();
	return;
    }
    drawCursor( FALSE );
    placeCursor( e->pos(),  cursor );
    drawCursor( TRUE );
    e->acceptAction();
}

/*!
    \reimp
*/

void QTextEdit::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    drawCursor( FALSE );
    inDnD = FALSE;
}

/*!
    \reimp
*/

void QTextEdit::contentsDropEvent( QDropEvent *e )
{
    if ( isReadOnly() )
	return;
    inDnD = FALSE;
    e->acceptAction();
    bool intern = FALSE;
    if ( QRichTextDrag::canDecode( e ) ) {
	bool hasSel = doc->hasSelection( QTextDocument::Standard );
	bool internalDrag = e->source() == this || e->source() == viewport();
	int dropId, dropIndex;
	QTextCursor insertCursor = *cursor;
	dropId = cursor->paragraph()->paragId();
	dropIndex = cursor->index();
	if ( hasSel && internalDrag ) {
	    QTextCursor c1, c2;
	    int selStartId, selStartIndex;
	    int selEndId, selEndIndex;
	    c1 = doc->selectionStartCursor( QTextDocument::Standard );
	    c1.restoreState();
	    c2 = doc->selectionEndCursor( QTextDocument::Standard );
	    c2.restoreState();
	    selStartId = c1.paragraph()->paragId();
	    selStartIndex = c1.index();
	    selEndId = c2.paragraph()->paragId();
	    selEndIndex = c2.index();
	    if ( ( ( dropId > selStartId ) ||
		   ( dropId == selStartId && dropIndex > selStartIndex ) ) &&
		 ( ( dropId < selEndId ) ||
		   ( dropId == selEndId && dropIndex <= selEndIndex ) ) )
		insertCursor = c1;
	    if ( dropId == selEndId && dropIndex > selEndIndex ) {
		insertCursor = c1;
		if ( selStartId == selEndId ) {
		    insertCursor.setIndex( dropIndex -
					   ( selEndIndex - selStartIndex ) );
		} else {
		    insertCursor.setIndex( dropIndex - selEndIndex +
					   selStartIndex );
		}
	    }
	 }

	if ( internalDrag && e->action() == QDropEvent::Move ) {
	    removeSelectedText();
	    intern = TRUE;
	    doc->removeSelection( QTextDocument::Standard );
	} else {
	    doc->removeSelection( QTextDocument::Standard );
#ifndef QT_NO_CURSOR
	    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
	}
	drawCursor( FALSE );
	cursor->setParagraph( insertCursor.paragraph() );
	cursor->setIndex( insertCursor.index() );
	drawCursor( TRUE );
	if ( !cursor->nestedDepth() ) {
	    QString subType = "plain";
	    if ( textFormat() != PlainText ) {
		if ( e->provides( "application/x-qrichtext" ) )
		    subType = "x-qrichtext";
	    }
#ifndef QT_NO_CLIPBOARD
	    pasteSubType( subType.latin1(), e );
#endif
	    // emit appropriate signals.
	    emit selectionChanged();
	    emit cursorPositionChanged( cursor );
	    emit cursorPositionChanged( cursor->paragraph()->paragId(), cursor->index() );
	} else {
	    if ( intern )
		undo();
	    e->ignore();
	}
    }
}

#endif

/*!
    \reimp
*/
void QTextEdit::contentsContextMenuEvent( QContextMenuEvent *e )
{
    clearUndoRedo();
    mousePressed = FALSE;

    e->accept();
#ifndef QT_NO_POPUPMENU
    QGuardedPtr<QTextEdit> that = this;
    QGuardedPtr<QPopupMenu> popup = createPopupMenu( e->pos() );
    if ( !popup )
	popup = createPopupMenu();
    if ( !popup )
	return;

    int r = popup->exec( e->globalPos() );
    delete popup;
    if (!that)
        return;

    if ( r == d->id[ IdClear ] )
	clear();
    else if ( r == d->id[ IdSelectAll ] ) {
	selectAll();
#ifndef QT_NO_CLIPBOARD
        // if the clipboard support selections, put the newly selected text into
        // the clipboard
	if (QApplication::clipboard()->supportsSelection()) {
	    d->clipboard_mode = QClipboard::Selection;

            // don't listen to selection changes
            disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);
	    copy();
            // listen to selection changes
            connect( QApplication::clipboard(), SIGNAL(selectionChanged()),
                     this, SLOT(clipboardChanged()) );

	    d->clipboard_mode = QClipboard::Clipboard;
	}
#endif
    } else if ( r == d->id[ IdUndo ] )
	undo();
    else if ( r == d->id[ IdRedo ] )
	redo();
#ifndef QT_NO_CLIPBOARD
    else if ( r == d->id[ IdCut ] )
	cut();
    else if ( r == d->id[ IdCopy ] )
	copy();
    else if ( r == d->id[ IdPaste ] )
	paste();
#endif
#endif
}


void QTextEdit::autoScrollTimerDone()
{
    if ( mousePressed )
	handleMouseMove(  viewportToContents( viewport()->mapFromGlobal( QCursor::pos() )  ) );
}

void QTextEdit::handleMouseMove( const QPoint& pos )
{
    if ( !mousePressed )
	return;

    if ( !scrollTimer->isActive() && pos.y() < contentsY() || pos.y() > contentsY() + visibleHeight() )
	scrollTimer->start( 100, FALSE );
    else if ( scrollTimer->isActive() && pos.y() >= contentsY() && pos.y() <= contentsY() + visibleHeight() )
	scrollTimer->stop();

    drawCursor( FALSE );
    QTextCursor oldCursor = *cursor;

    placeCursor( pos );

    if ( inDoubleClick ) {
	QTextCursor cl = *cursor;
	cl.gotoPreviousWord();
	QTextCursor cr = *cursor;
	cr.gotoNextWord();

	int diff = QABS( oldCursor.paragraph()->at( oldCursor.index() )->x - mousePos.x() );
	int ldiff = QABS( cl.paragraph()->at( cl.index() )->x - mousePos.x() );
	int rdiff = QABS( cr.paragraph()->at( cr.index() )->x - mousePos.x() );


	if ( cursor->paragraph()->lineStartOfChar( cursor->index() ) !=
	     oldCursor.paragraph()->lineStartOfChar( oldCursor.index() ) )
	    diff = 0xFFFFFF;

	if ( rdiff < diff && rdiff < ldiff )
	    *cursor = cr;
	else if ( ldiff < diff && ldiff < rdiff )
	    *cursor = cl;
	else
	    *cursor = oldCursor;

    }
    ensureCursorVisible();

    bool redraw = FALSE;
    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	redraw = doc->setSelectionEnd( QTextDocument::Standard, *cursor ) || redraw;
    }

    if ( !redraw ) {
	drawCursor( TRUE );
    } else {
	repaintChanged();
	drawCursor( TRUE );
    }

    if ( currentFormat && currentFormat->key() != cursor->paragraph()->at( cursor->index() )->format()->key() ) {
	currentFormat->removeRef();
	currentFormat = doc->formatCollection()->format( cursor->paragraph()->at( cursor->index() )->format() );
	if ( currentFormat->isMisspelled() ) {
	    currentFormat->removeRef();
	    currentFormat = doc->formatCollection()->format( currentFormat->font(), currentFormat->color() );
	}
	emit currentFontChanged( currentFormat->font() );
	emit currentColorChanged( currentFormat->color() );
	emit currentVerticalAlignmentChanged( (VerticalAlignment)currentFormat->vAlign() );
    }

    if ( currentAlignment != cursor->paragraph()->alignment() ) {
	currentAlignment = cursor->paragraph()->alignment();
	block_set_alignment = TRUE;
	emit currentAlignmentChanged( currentAlignment );
	block_set_alignment = FALSE;
    }
}

/*! \internal */

void QTextEdit::placeCursor( const QPoint &pos, QTextCursor *c, bool link )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return;
#endif
    if ( !c )
	c = cursor;

    resetInputContext();
    c->restoreState();
    QTextParagraph *s = doc->firstParagraph();
    c->place( pos, s, link );
    updateMicroFocusHint();
}


void QTextEdit::updateMicroFocusHint()
{
    QTextCursor c( *cursor );
    if ( d->preeditStart != -1 ) {
	c.setIndex( d->preeditStart );
        if(doc->hasSelection(QTextDocument::IMSelectionText)) {
            int para, index;
            doc->selectionStart(QTextDocument::IMSelectionText, para, index);
            c.setIndex(index);
        }
    }

    if ( hasFocus() || viewport()->hasFocus() ) {
	int h = c.paragraph()->lineHeightOfChar( cursor->index() );
	if ( !readonly ) {
	    QFont f = c.paragraph()->at( c.index() )->format()->font();
	    setMicroFocusHint( c.x() - contentsX() + frameWidth(),
			       c.y() + cursor->paragraph()->rect().y() - contentsY() + frameWidth(), 0, h, TRUE, &f );
	}
    }
}



void QTextEdit::formatMore()
{
    if ( !lastFormatted )
	return;

    int bottom = contentsHeight();
    int lastTop = -1;
    int lastBottom = -1;
    int to = 20;
    bool firstVisible = FALSE;
    QRect cr( contentsX(), contentsY(), visibleWidth(), visibleHeight() );
    for ( int i = 0; lastFormatted &&
	  ( i < to || ( firstVisible && lastTop < contentsY()+height() ) );
	  i++ ) {
	lastFormatted->format();
	lastTop = lastFormatted->rect().top();
	lastBottom = lastFormatted->rect().bottom();
	if ( i == 0 )
	    firstVisible = lastBottom < cr.bottom();
	bottom = QMAX( bottom, lastBottom );
	lastFormatted = lastFormatted->next();
    }

    if ( bottom > contentsHeight() ) {
	resizeContents( contentsWidth(), QMAX( doc->height(), bottom ) );
    } else if ( !lastFormatted && lastBottom < contentsHeight() ) {
	resizeContents( contentsWidth(), QMAX( doc->height(), lastBottom ) );
	if ( contentsHeight() < visibleHeight() )
	    updateContents( 0, contentsHeight(), visibleWidth(),
			    visibleHeight() - contentsHeight() );
    }

    if ( lastFormatted )
	formatTimer->start( interval, TRUE );
    else
	interval = QMAX( 0, interval );
}

void QTextEdit::doResize()
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( !d->optimMode )
#endif
    {
	if ( wrapMode == FixedPixelWidth )
	    return;
	doc->setMinimumWidth( -1 );
	resizeContents( 0, 0 );
	doc->setWidth( visibleWidth() );
	doc->invalidate();
	lastFormatted = doc->firstParagraph();
	interval = 0;
	formatMore();
    }
    repaintContents( FALSE );
}

/*! \internal */

void QTextEdit::doChangeInterval()
{
    interval = 0;
}

/*!
    \reimp
*/

bool QTextEdit::eventFilter( QObject *o, QEvent *e )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( !d->optimMode && (o == this || o == viewport()) ) {
#else
    if ( o == this || o == viewport() ) {
#endif
	if ( e->type() == QEvent::FocusIn ) {
	    if ( QApplication::cursorFlashTime() > 0 )
		blinkTimer->start( QApplication::cursorFlashTime() / 2 );
	    drawCursor( TRUE );
	    updateMicroFocusHint();
	} else if ( e->type() == QEvent::FocusOut ) {
	    blinkTimer->stop();
	    drawCursor( FALSE );
	}
    }

    if ( o == this && e->type() == QEvent::PaletteChange ) {
	QColor old( viewport()->colorGroup().color( QColorGroup::Text ) );
	if ( old != colorGroup().color( QColorGroup::Text ) ) {
	    QColor c( colorGroup().color( QColorGroup::Text ) );
	    doc->setMinimumWidth( -1 );
	    doc->setDefaultFormat( doc->formatCollection()->defaultFormat()->font(), c );
	    lastFormatted = doc->firstParagraph();
	    formatMore();
	    repaintChanged();
	}
    }

    return QScrollView::eventFilter( o, e );
}

/*!
  \obsolete
 */
void QTextEdit::insert( const QString &text, bool indent,
			bool checkNewLine, bool removeSelected )
{
    uint f = 0;
    if ( indent )
	f |= RedoIndentation;
    if ( checkNewLine )
	f |= CheckNewLines;
    if ( removeSelected )
	f |= RemoveSelected;
    insert( text, f );
}

/*!
    Inserts \a text at the current cursor position.

    The \a insertionFlags define how the text is inserted. If \c
    RedoIndentation is set, the paragraph is re-indented. If \c
    CheckNewLines is set, newline characters in \a text result in hard
    line breaks (i.e. new paragraphs). If \c checkNewLine is not set,
    the behaviour of the editor is undefined if the \a text contains
    newlines. (It is not possible to change QTextEdit's newline handling
    behavior, but you can use QString::replace() to preprocess text
    before inserting it.) If \c RemoveSelected is set, any selected
    text (in selection 0) is removed before the text is inserted.

    The default flags are \c CheckNewLines | \c RemoveSelected.

    If the widget is in \c LogText mode this function will do nothing.

    \sa paste() pasteSubType()
*/


void QTextEdit::insert( const QString &text, uint insertionFlags )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return;
#endif

    if ( cursor->nestedDepth() != 0 ) // #### for 3.0, disable editing of tables as this is not advanced enough
	return;

    bool indent = insertionFlags & RedoIndentation;
    bool checkNewLine = insertionFlags & CheckNewLines;
    bool removeSelected = insertionFlags & RemoveSelected;
    QString txt( text );
    drawCursor( FALSE );
    if ( !isReadOnly() && doc->hasSelection( QTextDocument::Standard ) && removeSelected )
	removeSelectedText();
    QTextCursor c2 = *cursor;
    int oldLen = 0;

    if ( undoEnabled && !isReadOnly() && undoRedoInfo.type != UndoRedoInfo::IME ) {
	checkUndoRedoInfo( UndoRedoInfo::Insert );

        if (undoRedoInfo.valid() && undoRedoInfo.index + undoRedoInfo.d->text.length() != cursor->index()) {
            clearUndoRedo();
            undoRedoInfo.type = UndoRedoInfo::Insert;
        }

	if ( !undoRedoInfo.valid() ) {
	    undoRedoInfo.id = cursor->paragraph()->paragId();
	    undoRedoInfo.index = cursor->index();
	    undoRedoInfo.d->text = QString::null;
	}
	oldLen = undoRedoInfo.d->text.length();
    }

    lastFormatted = checkNewLine && cursor->paragraph()->prev() ?
		    cursor->paragraph()->prev() : cursor->paragraph();
    QTextCursor oldCursor = *cursor;
    cursor->insert( txt, checkNewLine );
    if ( doc->useFormatCollection() && !doc->preProcessor() ) {
	doc->setSelectionStart( QTextDocument::Temp, oldCursor );
	doc->setSelectionEnd( QTextDocument::Temp, *cursor );
	doc->setFormat( QTextDocument::Temp, currentFormat, QTextFormat::Format );
	doc->removeSelection( QTextDocument::Temp );
    }

    if ( indent && ( txt == "{" || txt == "}" || txt == ":" || txt == "#" ) )
	cursor->indent();
    formatMore();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );

    if ( undoEnabled && !isReadOnly() && undoRedoInfo.type != UndoRedoInfo::IME ) {
	undoRedoInfo.d->text += txt;
	if ( !doc->preProcessor() ) {
	    for ( int i = 0; i < (int)txt.length(); ++i ) {
		if ( txt[ i ] != '\n' && c2.paragraph()->at( c2.index() )->format() ) {
		    c2.paragraph()->at( c2.index() )->format()->addRef();
		    undoRedoInfo.d->text.
			setFormat( oldLen + i,
				   c2.paragraph()->at( c2.index() )->format(), TRUE );
		}
		c2.gotoNextLetter();
	    }
	}
    }

    if ( !removeSelected ) {
	doc->setSelectionStart( QTextDocument::Standard, oldCursor );
	doc->setSelectionEnd( QTextDocument::Standard, *cursor );
	repaintChanged();
    }
    updateMicroFocusHint();
    setModified();
    emit textChanged();
}

/*!
    Inserts \a text in the paragraph \a para at position \a index.
*/

void QTextEdit::insertAt( const QString &text, int para, int index )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimInsert( text, para, index );
	return;
    }
#endif
    resetInputContext();
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return;
    removeSelection( QTextDocument::Standard );
    QTextCursor tmp = *cursor;
    cursor->setParagraph( p );
    cursor->setIndex( index );
    insert( text, FALSE, TRUE, FALSE );
    *cursor = tmp;
    removeSelection( QTextDocument::Standard );
}

/*!
    Inserts \a text as a new paragraph at position \a para. If \a para
    is -1, the text is appended. Use append() if the append operation
    is performance critical.
*/

void QTextEdit::insertParagraph( const QString &text, int para )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimInsert( text + "\n", para, 0 );
	return;
    }
#endif
    resetInputContext();
    for ( int i = 0; i < (int)doc->numSelections(); ++i )
	doc->removeSelection( i );

    QTextParagraph *p = doc->paragAt( para );

    bool append = !p;
    if ( !p )
	p = doc->lastParagraph();

    QTextCursor old = *cursor;
    drawCursor( FALSE );

    cursor->setParagraph( p );
    cursor->setIndex( 0 );
    clearUndoRedo();
    qtextedit_ignore_readonly = TRUE;
    if ( append && cursor->paragraph()->length() > 1 ) {
	cursor->setIndex( cursor->paragraph()->length() - 1 );
	doKeyboardAction( ActionReturn );
    }
    insert( text, FALSE, TRUE, TRUE );
    doKeyboardAction( ActionReturn );
    qtextedit_ignore_readonly = FALSE;

    drawCursor( FALSE );
    *cursor = old;
    drawCursor( TRUE );

    repaintChanged();
}

/*!
    Removes the paragraph \a para.
*/

void QTextEdit::removeParagraph( int para )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return;
#endif
    resetInputContext();
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return;

    for ( int i = 0; i < doc->numSelections(); ++i )
	doc->removeSelection( i );

    QTextCursor start( doc );
    QTextCursor end( doc );
    start.setParagraph( p );
    start.setIndex( 0 );
    end.setParagraph( p );
    end.setIndex( p->length() - 1 );

    if ( !(p == doc->firstParagraph() && p == doc->lastParagraph()) ) {
	if ( p->next() ) {
	    end.setParagraph( p->next() );
	    end.setIndex( 0 );
	} else if ( p->prev() ) {
	    start.setParagraph( p->prev() );
	    start.setIndex( p->prev()->length() - 1 );
	}
    }

    doc->setSelectionStart( QTextDocument::Temp, start );
    doc->setSelectionEnd( QTextDocument::Temp, end );
    removeSelectedText( QTextDocument::Temp );
}

/*!
    Undoes the last operation.

    If there is no operation to undo, i.e. there is no undo step in
    the undo/redo history, nothing happens.

    \sa undoAvailable() redo() undoDepth()
*/

void QTextEdit::undo()
{
    clearUndoRedo();
    if ( isReadOnly() || !doc->commands()->isUndoAvailable() || !undoEnabled )
	return;

    resetInputContext();
    for ( int i = 0; i < (int)doc->numSelections(); ++i )
	doc->removeSelection( i );

#ifndef QT_NO_CURSOR
    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif

    clearUndoRedo();
    drawCursor( FALSE );
    QTextCursor *c = doc->undo( cursor );
    if ( !c ) {
	drawCursor( TRUE );
	return;
    }
    lastFormatted = 0;
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
    updateMicroFocusHint();
    setModified();
    // ### If we get back to a completely blank textedit, it
    // is possible that cursor is invalid and further actions
    // might not fix the problem, so reset the cursor here.
    // This is copied from removeSeletedText(), it might be
    // okay to just call that.
    if ( !cursor->isValid() ) {
	delete cursor;
	cursor = new QTextCursor( doc );
	drawCursor( TRUE );
	repaintContents( TRUE );
    }
    emit undoAvailable( isUndoAvailable() );
    emit redoAvailable( isRedoAvailable() );
    emit textChanged();
}

/*!
    Redoes the last operation.

    If there is no operation to redo, i.e. there is no redo step in
    the undo/redo history, nothing happens.

    \sa redoAvailable() undo() undoDepth()
*/

void QTextEdit::redo()
{
    if ( isReadOnly() || !doc->commands()->isRedoAvailable() || !undoEnabled )
	return;

    resetInputContext();
    for ( int i = 0; i < (int)doc->numSelections(); ++i )
	doc->removeSelection( i );

#ifndef QT_NO_CURSOR
    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif

    clearUndoRedo();
    drawCursor( FALSE );
    QTextCursor *c = doc->redo( cursor );
    if ( !c ) {
	drawCursor( TRUE );
	return;
    }
    lastFormatted = 0;
    ensureCursorVisible();
    repaintChanged();
    ensureCursorVisible();
    drawCursor( TRUE );
    updateMicroFocusHint();
    setModified();
    emit undoAvailable( isUndoAvailable() );
    emit redoAvailable( isRedoAvailable() );
    emit textChanged();
}

/*!
    Pastes the text from the clipboard into the text edit at the
    current cursor position. Only plain text is pasted.

    If there is no text in the clipboard nothing happens.

    \sa pasteSubType() cut() QTextEdit::copy()
*/

void QTextEdit::paste()
{
#ifndef QT_NO_MIMECLIPBOARD
    if ( isReadOnly() )
	return;
    QString subType = "plain";
    if ( textFormat() != PlainText ) {
	QMimeSource *m = QApplication::clipboard()->data( d->clipboard_mode );
	if ( !m )
	    return;
	if ( m->provides( "application/x-qrichtext" ) )
	    subType = "x-qrichtext";
    }

    pasteSubType( subType.latin1() );
    updateMicroFocusHint();
#endif
}

void QTextEdit::checkUndoRedoInfo( UndoRedoInfo::Type t )
{
    if ( undoRedoInfo.valid() && t != undoRedoInfo.type ) {
	clearUndoRedo();
    }
    undoRedoInfo.type = t;
}

/*!
    Repaints any paragraphs that have changed.

    Although used extensively internally you shouldn't need to call
    this yourself.
*/

void QTextEdit::repaintChanged()
{
    if ( !isUpdatesEnabled() || !viewport()->isUpdatesEnabled() )
	return;

    QPainter p( viewport() );
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimDrawContents( &p, contentsX(), contentsY(), visibleWidth(), visibleHeight() );
	return;
    }
#endif
    p.translate( -contentsX(), -contentsY() );
    paintDocument( FALSE, &p, contentsX(), contentsY(), visibleWidth(), visibleHeight() );
}

#ifndef QT_NO_MIME
QTextDrag *QTextEdit::dragObject( QWidget *parent ) const
{
    if ( !doc->hasSelection( QTextDocument::Standard ) ||
	 doc->selectedText( QTextDocument::Standard ).isEmpty() )
	return 0;
    if ( textFormat() != RichText )
	return new QTextDrag( doc->selectedText( QTextDocument::Standard ), parent );
    QRichTextDrag *drag = new QRichTextDrag( parent );
    drag->setPlainText( doc->selectedText( QTextDocument::Standard ) );
    drag->setRichText( doc->selectedText( QTextDocument::Standard, TRUE ) );
    return drag;
}
#endif

/*!
    Copies the selected text (from selection 0) to the clipboard and
    deletes it from the text edit.

    If there is no selected text (in selection 0) nothing happens.

    \sa QTextEdit::copy() paste() pasteSubType()
*/

void QTextEdit::cut()
{
    if ( isReadOnly() )
	return;
    resetInputContext();
    normalCopy();
    removeSelectedText();
    updateMicroFocusHint();
}

void QTextEdit::normalCopy()
{
#ifndef QT_NO_MIME
    QTextDrag *drag = dragObject();
    if ( !drag )
	return;
#ifndef QT_NO_MIMECLIPBOARD
    QApplication::clipboard()->setData( drag, d->clipboard_mode );
#endif // QT_NO_MIMECLIPBOARD
#endif // QT_NO_MIME
}

/*!
    Copies any selected text (from selection 0) to the clipboard.

    \sa hasSelectedText() copyAvailable()
*/

void QTextEdit::copy()
{
#ifndef QT_NO_CLIPBOARD
# ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode && optimHasSelection() )
	QApplication::clipboard()->setText( optimSelectedText(), d->clipboard_mode );
    else
	normalCopy();
# else
    normalCopy();
# endif
#endif
}

/*!
    \internal

    Re-indents the current paragraph.
*/

void QTextEdit::indent()
{
    if ( isReadOnly() )
	return;

    drawCursor( FALSE );
    if ( !doc->hasSelection( QTextDocument::Standard ) )
	cursor->indent();
    else
	doc->indentSelection( QTextDocument::Standard );
    repaintChanged();
    drawCursor( TRUE );
    setModified();
    emit textChanged();
}

/*!
    Reimplemented to allow tabbing through links. If \a n is TRUE the
    tab moves the focus to the next child; if \a n is FALSE the tab
    moves the focus to the previous child. Returns TRUE if the focus
    was moved; otherwise returns FALSE.
 */

bool QTextEdit::focusNextPrevChild( bool n )
{
    if ( !isReadOnly() || !linksEnabled() )
	return FALSE;
    bool b = doc->focusNextPrevChild( n );
    repaintChanged();
    if ( b ) {
        QTextParagraph *p = doc->focusIndicator.parag;
        int start = doc->focusIndicator.start;
        int len = doc->focusIndicator.len;

        int y = p->rect().y();
        while ( p
                && len == 0
                && p->at( start )->isCustom()
                && p->at( start )->customItem()->isNested() ) {

            QTextTable *t = (QTextTable*)p->at( start )->customItem();
            QPtrList<QTextTableCell> cells = t->tableCells();
            QTextTableCell *c;
            for ( c = cells.first(); c; c = cells.next() ) {
                QTextDocument *cellDoc = c->richText();
                if ( cellDoc->hasFocusParagraph() ) {
                    y += c->geometry().y() + c->verticalAlignmentOffset();

                    p = cellDoc->focusIndicator.parag;
                    start = cellDoc->focusIndicator.start;
                    len = cellDoc->focusIndicator.len;
                    if ( p )
                        y += p->rect().y();

                    break;
                }
            }
        }
        setContentsPos( contentsX(), QMIN( y, contentsHeight() - visibleHeight() ) );
    }
    return b;
}

/*!
    \internal

  This functions sets the current format to \a f. Only the fields of \a
  f which are specified by the \a flags are used.
*/

void QTextEdit::setFormat( QTextFormat *f, int flags )
{
    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	drawCursor( FALSE );
	QTextCursor c1 = doc->selectionStartCursor( QTextDocument::Standard );
	c1.restoreState();
	QTextCursor c2 = doc->selectionEndCursor( QTextDocument::Standard );
	c2.restoreState();
	if ( undoEnabled ) {
	    clearUndoRedo();
	    undoRedoInfo.type = UndoRedoInfo::Format;
	    undoRedoInfo.id = c1.paragraph()->paragId();
	    undoRedoInfo.index = c1.index();
	    undoRedoInfo.eid = c2.paragraph()->paragId();
	    undoRedoInfo.eindex = c2.index();
	    readFormats( c1, c2, undoRedoInfo.d->text );
	    undoRedoInfo.format = f;
	    undoRedoInfo.flags = flags;
	    clearUndoRedo();
	}
	doc->setFormat( QTextDocument::Standard, f, flags );
	repaintChanged();
	formatMore();
	drawCursor( TRUE );
	setModified();
	emit textChanged();
    }
    if ( currentFormat && currentFormat->key() != f->key() ) {
	currentFormat->removeRef();
	currentFormat = doc->formatCollection()->format( f );
	if ( currentFormat->isMisspelled() ) {
	    currentFormat->removeRef();
	    currentFormat = doc->formatCollection()->format( currentFormat->font(),
							     currentFormat->color() );
	}
	emit currentFontChanged( currentFormat->font() );
	emit currentColorChanged( currentFormat->color() );
	emit currentVerticalAlignmentChanged( (VerticalAlignment)currentFormat->vAlign() );
	if ( cursor->index() == cursor->paragraph()->length() - 1 ) {
	    currentFormat->addRef();
	    cursor->paragraph()->string()->setFormat( cursor->index(), currentFormat, TRUE );
	    if ( cursor->paragraph()->length() == 1 ) {
		cursor->paragraph()->invalidate( 0 );
		cursor->paragraph()->format();
		repaintChanged();
	    }
	}
    }
}

/*!
    \reimp
*/

void QTextEdit::setPalette( const QPalette &p )
{
    QScrollView::setPalette( p );
    if ( textFormat() == PlainText ) {
	QTextFormat *f = doc->formatCollection()->defaultFormat();
	f->setColor( colorGroup().text() );
	updateContents();
    }
}

/*! \internal
  \warning In Qt 3.1 we will provide a cleaer API for the
  functionality which is provided by this function and in Qt 4.0 this
  function will go away.

  Sets the paragraph style of the current paragraph
  to \a dm. If \a dm is QStyleSheetItem::DisplayListItem, the
  type of the list item is set to \a listStyle.

  \sa setAlignment()
*/

void QTextEdit::setParagType( QStyleSheetItem::DisplayMode dm,
			      QStyleSheetItem::ListStyle listStyle )
{
    if ( isReadOnly() )
	return;

    drawCursor( FALSE );
    QTextParagraph *start = cursor->paragraph();
    QTextParagraph *end = start;
    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	start = doc->selectionStartCursor( QTextDocument::Standard ).topParagraph();
	end = doc->selectionEndCursor( QTextDocument::Standard ).topParagraph();
	if ( end->paragId() < start->paragId() )
	    return; // do not trust our selections
    }

    clearUndoRedo();
    undoRedoInfo.type = UndoRedoInfo::Style;
    undoRedoInfo.id = start->paragId();
    undoRedoInfo.eid = end->paragId();
    undoRedoInfo.styleInformation = QTextStyleCommand::readStyleInformation( doc, undoRedoInfo.id, undoRedoInfo.eid );

    while ( start != end->next() ) {
	start->setListStyle( listStyle );
	if ( dm == QStyleSheetItem::DisplayListItem ) {
	    start->setListItem( TRUE );
	    if( start->listDepth() == 0 )
		start->setListDepth( 1 );
	} else if ( start->isListItem() ) {
	    start->setListItem( FALSE );
	    start->setListDepth( QMAX( start->listDepth()-1, 0 ) );
	}
	start = start->next();
    }

    clearUndoRedo();
    repaintChanged();
    formatMore();
    drawCursor( TRUE );
    setModified();
    emit textChanged();
}

/*!
    Sets the alignment of the current paragraph to \a a. Valid
    alignments are \c Qt::AlignLeft, \c Qt::AlignRight,
    \c Qt::AlignJustify and \c Qt::AlignCenter (which centers
    horizontally).
*/

void QTextEdit::setAlignment( int a )
{
    if ( isReadOnly() || block_set_alignment )
	return;

    drawCursor( FALSE );
    QTextParagraph *start = cursor->paragraph();
    QTextParagraph *end = start;
    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	start = doc->selectionStartCursor( QTextDocument::Standard ).topParagraph();
	end = doc->selectionEndCursor( QTextDocument::Standard ).topParagraph();
	if ( end->paragId() < start->paragId() )
	    return; // do not trust our selections
    }

    clearUndoRedo();
    undoRedoInfo.type = UndoRedoInfo::Style;
    undoRedoInfo.id = start->paragId();
    undoRedoInfo.eid = end->paragId();
    undoRedoInfo.styleInformation = QTextStyleCommand::readStyleInformation( doc, undoRedoInfo.id, undoRedoInfo.eid );

    while ( start != end->next() ) {
	start->setAlignment( a );
	start = start->next();
    }

    clearUndoRedo();
    repaintChanged();
    formatMore();
    drawCursor( TRUE );
    if ( currentAlignment != a ) {
	currentAlignment = a;
	emit currentAlignmentChanged( currentAlignment );
    }
    setModified();
    emit textChanged();
}

void QTextEdit::updateCurrentFormat()
{
    int i = cursor->index();
    if ( i > 0 )
	--i;
    if ( doc->useFormatCollection() &&
	 ( !currentFormat || currentFormat->key() != cursor->paragraph()->at( i )->format()->key() ) ) {
	if ( currentFormat )
	    currentFormat->removeRef();
	currentFormat = doc->formatCollection()->format( cursor->paragraph()->at( i )->format() );
	if ( currentFormat->isMisspelled() ) {
	    currentFormat->removeRef();
	    currentFormat = doc->formatCollection()->format( currentFormat->font(), currentFormat->color() );
	}
	emit currentFontChanged( currentFormat->font() );
	emit currentColorChanged( currentFormat->color() );
	emit currentVerticalAlignmentChanged( (VerticalAlignment)currentFormat->vAlign() );
    }

    if ( currentAlignment != cursor->paragraph()->alignment() ) {
	currentAlignment = cursor->paragraph()->alignment();
	block_set_alignment = TRUE;
	emit currentAlignmentChanged( currentAlignment );
	block_set_alignment = FALSE;
    }
}

/*!
    If \a b is TRUE sets the current format to italic; otherwise sets
    the current format to non-italic.

    \sa italic()
*/

void QTextEdit::setItalic( bool b )
{
    QTextFormat f( *currentFormat );
    f.setItalic( b );
    QTextFormat *f2 = doc->formatCollection()->format( &f );
    setFormat(f2, QTextFormat::Italic );
}

/*!
    If \a b is TRUE sets the current format to bold; otherwise sets
    the current format to non-bold.

    \sa bold()
*/

void QTextEdit::setBold( bool b )
{
    QTextFormat f( *currentFormat );
    f.setBold( b );
    QTextFormat *f2 = doc->formatCollection()->format( &f );
    setFormat( f2, QTextFormat::Bold );
}

/*!
    If \a b is TRUE sets the current format to underline; otherwise
    sets the current format to non-underline.

    \sa underline()
*/

void QTextEdit::setUnderline( bool b )
{
    QTextFormat f( *currentFormat );
    f.setUnderline( b );
    QTextFormat *f2 = doc->formatCollection()->format( &f );
    setFormat( f2, QTextFormat::Underline );
}

/*!
    Sets the font family of the current format to \a fontFamily.

    \sa family() setCurrentFont()
*/

void QTextEdit::setFamily( const QString &fontFamily )
{
    QTextFormat f( *currentFormat );
    f.setFamily( fontFamily );
    QTextFormat *f2 = doc->formatCollection()->format( &f );
    setFormat( f2, QTextFormat::Family );
}

/*!
    Sets the point size of the current format to \a s.

    Note that if \a s is zero or negative, the behaviour of this
    function is not defined.

    \sa pointSize() setCurrentFont() setFamily()
*/

void QTextEdit::setPointSize( int s )
{
    QTextFormat f( *currentFormat );
    f.setPointSize( s );
    QTextFormat *f2 = doc->formatCollection()->format( &f );
    setFormat( f2, QTextFormat::Size );
}

/*!
    Sets the color of the current format, i.e. of the text, to \a c.

    \sa color() setPaper()
*/

void QTextEdit::setColor( const QColor &c )
{
    QTextFormat f( *currentFormat );
    f.setColor( c );
    QTextFormat *f2 = doc->formatCollection()->format( &f );
    setFormat( f2, QTextFormat::Color );
}

/*!
    Sets the vertical alignment of the current format, i.e. of the
    text, to \a a.

    \sa color() setPaper()
*/

void QTextEdit::setVerticalAlignment( VerticalAlignment a )
{
    QTextFormat f( *currentFormat );
    f.setVAlign( (QTextFormat::VerticalAlignment)a );
    QTextFormat *f2 = doc->formatCollection()->format( &f );
    setFormat( f2, QTextFormat::VAlign );
}

void QTextEdit::setFontInternal( const QFont &f_ )
{
    QTextFormat f( *currentFormat );
    f.setFont( f_ );
    QTextFormat *f2 = doc->formatCollection()->format( &f );
    setFormat( f2, QTextFormat::Font );
}


QString QTextEdit::text() const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return optimText();
#endif

    QTextParagraph *p = doc->firstParagraph();
    if ( !p || (!p->next() && p->length() <= 1) )
	return QString::fromLatin1("");

    if ( isReadOnly() )
	return doc->originalText();
    return doc->text();
}

/*!
    \overload

    Returns the text of paragraph \a para.

    If textFormat() is \c RichText the text will contain HTML
    formatting tags.
*/

QString QTextEdit::text( int para ) const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode && (d->od->numLines >= para) ) {
	QString paraStr = d->od->lines[ LOGOFFSET(para) ];
	if ( paraStr.isEmpty() )
	    paraStr = "\n";
	return paraStr;
    } else
#endif
    return doc->text( para );
}

/*!
    \overload

    Changes the text of the text edit to the string \a text and the
    context to \a context. Any previous text is removed.

    \a text may be interpreted either as plain text or as rich text,
    depending on the textFormat(). The default setting is \c AutoText,
    i.e. the text edit auto-detects the format from \a text.

    For rich text the rendering style and available tags are defined
    by a styleSheet(); see QStyleSheet for details.

    The optional \a context is a path which the text edit's
    QMimeSourceFactory uses to resolve the locations of files and
    images. (See \l{QTextEdit::QTextEdit()}.) It is passed to the text
    edit's QMimeSourceFactory when quering data.

    Note that the undo/redo history is cleared by this function.

    \sa text(), setTextFormat()
*/

void QTextEdit::setText( const QString &text, const QString &context )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimSetText( text );
	return;
    }
#endif
    resetInputContext();
    if ( !isModified() && isReadOnly() &&
	 this->context() == context && this->text() == text )
	return;

    emit undoAvailable( FALSE );
    emit redoAvailable( FALSE );
    undoRedoInfo.clear();
    doc->commands()->clear();

    lastFormatted = 0;
    int oldCursorPos = cursor->index();
    int oldCursorPar = cursor->paragraph()->paragId();
    cursor->restoreState();
    delete cursor;
    doc->setText( text, context );

    if ( wrapMode == FixedPixelWidth ) {
	resizeContents( wrapWidth, 0 );
	doc->setWidth( wrapWidth );
	doc->setMinimumWidth( wrapWidth );
    } else {
	doc->setMinimumWidth( -1 );
	resizeContents( 0, 0 );
    }

    lastFormatted = doc->firstParagraph();
    cursor = new QTextCursor( doc );
    updateContents();

    if ( isModified() )
	setModified( FALSE );
    emit textChanged();
    if ( cursor->index() != oldCursorPos || cursor->paragraph()->paragId() != oldCursorPar ) {
	emit cursorPositionChanged( cursor );
	emit cursorPositionChanged( cursor->paragraph()->paragId(), cursor->index() );
    }
    formatMore();
    updateCurrentFormat();
    d->scrollToAnchor = QString::null;
}

/*!
    \property QTextEdit::text
    \brief the text edit's text

    There is no default text.

    On setting, any previous text is deleted.

    The text may be interpreted either as plain text or as rich text,
    depending on the textFormat(). The default setting is \c AutoText,
    i.e. the text edit auto-detects the format of the text.

    For richtext, calling text() on an editable QTextEdit will cause
    the text to be regenerated from the textedit. This may mean that
    the QString returned may not be exactly the same as the one that
    was set.

    \sa textFormat
*/


/*!
    \property QTextEdit::readOnly
    \brief whether the text edit is read-only

    In a read-only text edit the user can only navigate through the
    text and select text; modifying the text is not possible.

    This property's default is FALSE.
*/

/*!
    Finds the next occurrence of the string, \a expr. Returns TRUE if
    \a expr was found; otherwise returns FALSE.

    If \a para and \a index are both 0 the search begins from the
    current cursor position. If \a para and \a index are both not 0,
    the search begins from the \a *index character position in the
    \a *para paragraph.

    If \a cs is TRUE the search is case sensitive, otherwise it is
    case insensitive. If \a wo is TRUE the search looks for whole word
    matches only; otherwise it searches for any matching text. If \a
    forward is TRUE (the default) the search works forward from the
    starting position to the end of the text, otherwise it works
    backwards to the beginning of the text.

    If \a expr is found the function returns TRUE. If \a index and \a
    para are not 0, the number of the paragraph in which the first
    character of the match was found is put into \a *para, and the
    index position of that character within the paragraph is put into
    \a *index.

    If \a expr is not found the function returns FALSE. If \a index
    and \a para are not 0 and \a expr is not found, \a *index
    and \a *para are undefined.

    Please note that this function will make the next occurrence of
    the string (if found) the current selection, and will thus
    modify the cursor position.

    Using the \a para and \a index parameters will not work correctly
    in case the document contains tables.
*/

bool QTextEdit::find( const QString &expr, bool cs, bool wo, bool forward,
		      int *para, int *index )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return optimFind( expr, cs, wo, forward, para, index );
#endif
    drawCursor( FALSE );
#ifndef QT_NO_CURSOR
    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
    QTextCursor findcur = *cursor;
    if ( para && index ) {
	if ( doc->paragAt( *para ) )
	    findcur.gotoPosition( doc->paragAt(*para), *index );
	else
	    findcur.gotoEnd();
    } else if ( doc->hasSelection( QTextDocument::Standard ) ){
	// maks sure we do not find the same selection again
	if ( forward )
	    findcur.gotoNextLetter();
	else
	    findcur.gotoPreviousLetter();
    } else if (!forward && findcur.index() == 0 && findcur.paragraph() == findcur.topParagraph()) {
	findcur.gotoEnd();
    }
    removeSelection( QTextDocument::Standard );
    bool found = doc->find( findcur, expr, cs, wo, forward );
    if ( found ) {
	if ( para )
	    *para = findcur.paragraph()->paragId();
	if ( index )
	    *index = findcur.index();
	*cursor = findcur;
	repaintChanged();
	ensureCursorVisible();
    }
    drawCursor( TRUE );
    if (found) {
	emit cursorPositionChanged( cursor );
	emit cursorPositionChanged( cursor->paragraph()->paragId(), cursor->index() );
    }
    return found;
}

void QTextEdit::blinkCursor()
{
    if ( !cursorVisible )
	return;
    bool cv = cursorVisible;
    blinkCursorVisible = !blinkCursorVisible;
    drawCursor( blinkCursorVisible );
    cursorVisible = cv;
}

/*!
    Sets the cursor to position \a index in paragraph \a para.

    \sa getCursorPosition()
*/

void QTextEdit::setCursorPosition( int para, int index )
{
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return;

    resetInputContext();
    if ( index > p->length() - 1 )
	index = p->length() - 1;

    drawCursor( FALSE );
    cursor->setParagraph( p );
    cursor->setIndex( index );
    ensureCursorVisible();
    drawCursor( TRUE );
    updateCurrentFormat();
    emit cursorPositionChanged( cursor );
    emit cursorPositionChanged( cursor->paragraph()->paragId(), cursor->index() );
}

/*!
    This function sets the \a *para and \a *index parameters to the
    current cursor position. \a para and \a index must not be 0.

    \sa setCursorPosition()
*/

void QTextEdit::getCursorPosition( int *para, int *index ) const
{
    if ( !para || !index )
	return;
    *para = cursor->paragraph()->paragId();
    *index = cursor->index();
}

/*!
    Sets a selection which starts at position \a indexFrom in
    paragraph \a paraFrom and ends at position \a indexTo in paragraph
    \a paraTo.

    Any existing selections which have a different id (\a selNum) are
    left alone, but if an existing selection has the same id as \a
    selNum it is removed and replaced by this selection.

    Uses the selection settings of selection \a selNum. If \a selNum
    is 0, this is the default selection.

    The cursor is moved to the end of the selection if \a selNum is 0,
    otherwise the cursor position remains unchanged.

    \sa getSelection() selectedText
*/

void QTextEdit::setSelection( int paraFrom, int indexFrom,
			      int paraTo, int indexTo, int selNum )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if (d->optimMode) {
	optimSetSelection(paraFrom, indexFrom, paraTo, indexTo);
	repaintContents(FALSE);
	return;
    }
#endif
    resetInputContext();
    if ( doc->hasSelection( selNum ) ) {
	doc->removeSelection( selNum );
	repaintChanged();
    }
    if ( selNum > doc->numSelections() - 1 )
	doc->addSelection( selNum );
    QTextParagraph *p1 = doc->paragAt( paraFrom );
    if ( !p1 )
	return;
    QTextParagraph *p2 = doc->paragAt( paraTo );
    if ( !p2 )
	return;

    if ( indexFrom > p1->length() - 1 )
	indexFrom = p1->length() - 1;
    if ( indexTo > p2->length() - 1 )
	indexTo = p2->length() - 1;

    drawCursor( FALSE );
    QTextCursor c = *cursor;
    QTextCursor oldCursor = *cursor;
    c.setParagraph( p1 );
    c.setIndex( indexFrom );
    cursor->setParagraph( p2 );
    cursor->setIndex( indexTo );
    doc->setSelectionStart( selNum, c );
    doc->setSelectionEnd( selNum, *cursor );
    repaintChanged();
    ensureCursorVisible();
    if ( selNum != QTextDocument::Standard )
	*cursor = oldCursor;
    drawCursor( TRUE );
}

/*!
    If there is a selection, \a *paraFrom is set to the number of the
    paragraph in which the selection begins and \a *paraTo is set to
    the number of the paragraph in which the selection ends. (They
    could be the same.) \a *indexFrom is set to the index at which the
    selection begins within \a *paraFrom, and \a *indexTo is set to
    the index at which the selection ends within \a *paraTo.

    If there is no selection, \a *paraFrom, \a *indexFrom, \a *paraTo
    and \a *indexTo are all set to -1.

    If \a paraFrom, \a indexFrom, \a paraTo or \a indexTo is 0 this
    function does nothing.

    The \a selNum is the number of the selection (multiple selections
    are supported). It defaults to 0 (the default selection).

    \sa setSelection() selectedText
*/

void QTextEdit::getSelection( int *paraFrom, int *indexFrom,
			      int *paraTo, int *indexTo, int selNum ) const
{
    if ( !paraFrom || !paraTo || !indexFrom || !indexTo )
	return;
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if (d->optimMode) {
	*paraFrom = d->od->selStart.line;
	*paraTo = d->od->selEnd.line;
	*indexFrom = d->od->selStart.index;
	*indexTo = d->od->selEnd.index;
	return;
    }
#endif
    if ( !doc->hasSelection( selNum ) ) {
	*paraFrom = -1;
	*indexFrom = -1;
	*paraTo = -1;
	*indexTo = -1;
	return;
    }

    doc->selectionStart( selNum, *paraFrom, *indexFrom );
    doc->selectionEnd( selNum, *paraTo, *indexTo );
}

/*!
    \property QTextEdit::textFormat
    \brief the text format: rich text, plain text, log text or auto text.

    The text format is one of the following:
    \list
    \i PlainText - all characters, except newlines, are displayed
    verbatim, including spaces. Whenever a newline appears in the text
    the text edit inserts a hard line break and begins a new
    paragraph.
    \i RichText - rich text rendering. The available styles are
    defined in the default stylesheet QStyleSheet::defaultSheet().
    \i LogText -  optimized mode for very large texts. Supports a very
    limited set of formatting tags (color, bold, underline and italic
    settings).
    \i AutoText - this is the default. The text edit autodetects which
    rendering style is best, \c PlainText or \c RichText. This is done
    by using the QStyleSheet::mightBeRichText() function.
    \endlist
*/

void QTextEdit::setTextFormat( TextFormat format )
{
    doc->setTextFormat( format );
#ifdef QT_TEXTEDIT_OPTIMIZATION
    checkOptimMode();
#endif
}

Qt::TextFormat QTextEdit::textFormat() const
{
    return doc->textFormat();
}

/*!
    Returns the number of paragraphs in the text; an empty textedit is always
    considered to have one paragraph, so 1 is returned in this case.
*/

int QTextEdit::paragraphs() const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	return d->od->numLines;
    }
#endif
    return doc->lastParagraph()->paragId() + 1;
}

/*!
    Returns the number of lines in paragraph \a para, or -1 if there
    is no paragraph with index \a para.
*/

int QTextEdit::linesOfParagraph( int para ) const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	if ( d->od->numLines >= para )
	    return 1;
	else
	    return -1;
    }
#endif
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return -1;
    return p->lines();
}

/*!
    Returns the length of the paragraph \a para (i.e. the number of
    characters), or -1 if there is no paragraph with index \a para.

    This function ignores newlines.
*/

int QTextEdit::paragraphLength( int para ) const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	if ( d->od->numLines >= para ) {
	    if ( d->od->lines[ LOGOFFSET(para) ].isEmpty() ) // CR
		return 1;
	    else
		return d->od->lines[ LOGOFFSET(para) ].length();
	}
	return -1;
    }
#endif
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return -1;
    return p->length() - 1;
}

/*!
    Returns the number of lines in the text edit; this could be 0.

    \warning This function may be slow. Lines change all the time
    during word wrapping, so this function has to iterate over all the
    paragraphs and get the number of lines from each one individually.
*/

int QTextEdit::lines() const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	return d->od->numLines;
    }
#endif
    QTextParagraph *p = doc->firstParagraph();
    int l = 0;
    while ( p ) {
	l += p->lines();
	p = p->next();
    }

    return l;
}

/*!
    Returns the line number of the line in paragraph \a para in which
    the character at position \a index appears. The \a index position is
    relative to the beginning of the paragraph. If there is no such
    paragraph or no such character at the \a index position (e.g. the
    index is out of range) -1 is returned.
*/

int QTextEdit::lineOfChar( int para, int index )
{
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return -1;

    int idx, line;
    QTextStringChar *c = p->lineStartOfChar( index, &idx, &line );
    if ( !c )
	return -1;

    return line;
}

void QTextEdit::setModified( bool m )
{
    bool oldModified = modified;
    modified = m;
    if ( modified && doc->oTextValid )
	doc->invalidateOriginalText();
    if ( oldModified != modified )
	emit modificationChanged( modified );
}

/*!
    \property QTextEdit::modified
    \brief whether the document has been modified by the user
*/

bool QTextEdit::isModified() const
{
    return modified;
}

void QTextEdit::setModified()
{
    if ( !isModified() )
	setModified( TRUE );
}

/*!
    Returns TRUE if the current format is italic; otherwise returns FALSE.

    \sa setItalic()
*/

bool QTextEdit::italic() const
{
    return currentFormat->font().italic();
}

/*!
    Returns TRUE if the current format is bold; otherwise returns FALSE.

    \sa setBold()
*/

bool QTextEdit::bold() const
{
    return currentFormat->font().bold();
}

/*!
    Returns TRUE if the current format is underlined; otherwise returns
    FALSE.

    \sa setUnderline()
*/

bool QTextEdit::underline() const
{
    return currentFormat->font().underline();
}

/*!
    Returns the font family of the current format.

    \sa setFamily() setCurrentFont() setPointSize()
*/

QString QTextEdit::family() const
{
    return currentFormat->font().family();
}

/*!
    Returns the point size of the font of the current format.

    \sa setFamily() setCurrentFont() setPointSize()
*/

int QTextEdit::pointSize() const
{
    return currentFormat->font().pointSize();
}

/*!
    Returns the color of the current format.

    \sa setColor() setPaper()
*/

QColor QTextEdit::color() const
{
    return currentFormat->color();
}

/*!
    \obsolete

    Returns QScrollView::font()

    \warning In previous versions this function returned the font of
    the current format. This lead to confusion. Please use
    currentFont() instead.
*/

QFont QTextEdit::font() const
{
    return QScrollView::font();
}

/*!
    Returns the font of the current format.

    \sa setCurrentFont() setFamily() setPointSize()
*/

QFont QTextEdit::currentFont() const
{
    return currentFormat->font();
}


/*!
    Returns the alignment of the current paragraph.

    \sa setAlignment()
*/

int QTextEdit::alignment() const
{
    return currentAlignment;
}

void QTextEdit::startDrag()
{
#ifndef QT_NO_DRAGANDDROP
    mousePressed = FALSE;
    inDoubleClick = FALSE;
    QDragObject *drag = dragObject( viewport() );
    if ( !drag )
	return;
    if ( isReadOnly() ) {
	drag->dragCopy();
    } else {
	if ( drag->drag() && QDragObject::target() != this && QDragObject::target() != viewport() )
	    removeSelectedText();
    }
#endif
}

/*!
    If \a select is TRUE (the default), all the text is selected as
    selection 0. If \a select is FALSE any selected text is
    unselected, i.e. the default selection (selection 0) is cleared.

    \sa selectedText
*/

void QTextEdit::selectAll( bool select )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	if ( select )
	    optimSelectAll();
	else
	    optimRemoveSelection();
	return;
    }
#endif
    if ( !select )
	doc->removeSelection( QTextDocument::Standard );
    else
	doc->selectAll( QTextDocument::Standard );
    repaintChanged();
    emit copyAvailable( doc->hasSelection( QTextDocument::Standard ) );
    emit selectionChanged();
#ifndef QT_NO_CURSOR
    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
}

void QTextEdit::UndoRedoInfo::clear()
{
    if ( valid() ) {
	if ( type == Insert || type == Return )
	    doc->addCommand( new QTextInsertCommand( doc, id, index, d->text.rawData(), styleInformation ) );
	else if ( type == Format )
	    doc->addCommand( new QTextFormatCommand( doc, id, index, eid, eindex, d->text.rawData(), format, flags ) );
	else if ( type == Style )
	    doc->addCommand( new QTextStyleCommand( doc, id, eid, styleInformation ) );
	else if ( type != Invalid ) {
	    doc->addCommand( new QTextDeleteCommand( doc, id, index, d->text.rawData(), styleInformation ) );
	}
    }
    type = Invalid;
    d->text = QString::null;
    id = -1;
    index = -1;
    styleInformation = QByteArray();
}


/*!
    If there is some selected text (in selection 0) it is deleted. If
    there is no selected text (in selection 0) the character to the
    right of the text cursor is deleted.

    \sa removeSelectedText() cut()
*/

void QTextEdit::del()
{
    if ( doc->hasSelection( QTextDocument::Standard ) ) {
	removeSelectedText();
	return;
    }

    doKeyboardAction( ActionDelete );
}


QTextEdit::UndoRedoInfo::UndoRedoInfo( QTextDocument *dc )
    : type( Invalid ), doc( dc )
{
    d = new QUndoRedoInfoPrivate;
    d->text = QString::null;
    id = -1;
    index = -1;
}

QTextEdit::UndoRedoInfo::~UndoRedoInfo()
{
    delete d;
}

bool QTextEdit::UndoRedoInfo::valid() const
{
    return id >= 0 &&  type != Invalid;
}

/*!
    \internal

  Resets the current format to the default format.
*/

void QTextEdit::resetFormat()
{
    setAlignment( Qt::AlignAuto );
    setParagType( QStyleSheetItem::DisplayBlock, QStyleSheetItem::ListDisc );
    setFormat( doc->formatCollection()->defaultFormat(), QTextFormat::Format );
}

/*!
    Returns the QStyleSheet which is being used by this text edit.

    \sa setStyleSheet()
*/

QStyleSheet* QTextEdit::styleSheet() const
{
    return doc->styleSheet();
}

/*!
    Sets the stylesheet to use with this text edit to \a styleSheet.
    Changes will only take effect for new text added with setText() or
    append().

    \sa styleSheet()
*/

void QTextEdit::setStyleSheet( QStyleSheet* styleSheet )
{
    doc->setStyleSheet( styleSheet );
}

/*!
    \property QTextEdit::paper
    \brief the background (paper) brush.

    The brush that is currently used to draw the background of the
    text edit. The initial setting is an empty brush.
*/

void QTextEdit::setPaper( const QBrush& pap )
{
    doc->setPaper( new QBrush( pap ) );

    if ( pap.pixmap() ) {
        viewport()->setBackgroundPixmap( *pap.pixmap() );
    } else {
        setPaletteBackgroundColor( pap.color() );
        viewport()->setPaletteBackgroundColor( pap.color() );
    }

#ifdef QT_TEXTEDIT_OPTIMIZATION
    // force a repaint of the entire viewport - using updateContents()
    // would clip the coords to the content size
    if (d->optimMode)
	repaintContents(contentsX(), contentsY(), viewport()->width(), viewport()->height());
    else
#endif
	updateContents();
}

QBrush QTextEdit::paper() const
{
    if ( doc->paper() )
	return *doc->paper();
    return QBrush( colorGroup().base() );
}

/*!
    \property QTextEdit::linkUnderline
    \brief whether hypertext links will be underlined

    If TRUE (the default) hypertext links will be displayed
    underlined. If FALSE links will not be displayed underlined.
*/

void QTextEdit::setLinkUnderline( bool b )
{
    if ( doc->underlineLinks() == b )
	return;
    doc->setUnderlineLinks( b );
    repaintChanged();
}

bool QTextEdit::linkUnderline() const
{
    return doc->underlineLinks();
}

/*!
    Sets the text edit's mimesource factory to \a factory. See
    QMimeSourceFactory for further details.

    \sa mimeSourceFactory()
 */

#ifndef QT_NO_MIME
void QTextEdit::setMimeSourceFactory( QMimeSourceFactory* factory )
{
    doc->setMimeSourceFactory( factory );
}

/*!
    Returns the QMimeSourceFactory which is being used by this text
    edit.

    \sa setMimeSourceFactory()
*/

QMimeSourceFactory* QTextEdit::mimeSourceFactory() const
{
    return doc->mimeSourceFactory();
}
#endif

/*!
    Returns how many pixels high the text edit needs to be to display
    all the text if the text edit is \a w pixels wide.
*/

int QTextEdit::heightForWidth( int w ) const
{
    int oldw = doc->width();
    doc->doLayout( 0, w );
    int h = doc->height();
    doc->setWidth( oldw );
    doc->invalidate();
    ( (QTextEdit*)this )->formatMore();
    return h;
}

/*!
    Appends a new paragraph with \a text to the end of the text edit. Note that
    the undo/redo history is cleared by this function, and no undo
    history is kept for appends which makes them faster than
    insert()s. If you want to append text which is added to the
    undo/redo history as well, use insertParagraph().
*/

void QTextEdit::append( const QString &text )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimAppend( text );
	return;
    }
#endif
    // flush and clear the undo/redo stack if necessary
    undoRedoInfo.clear();
    doc->commands()->clear();

    doc->removeSelection( QTextDocument::Standard );
    TextFormat f = doc->textFormat();
    if ( f == AutoText ) {
	if ( QStyleSheet::mightBeRichText( text ) )
	    f = RichText;
	else
	    f = PlainText;
    }

    drawCursor( FALSE );
    QTextCursor oldc( *cursor );
    ensureFormatted( doc->lastParagraph() );
    bool atBottom = contentsY() >= contentsHeight() - visibleHeight();
    cursor->gotoEnd();
    if ( cursor->index() > 0 )
	cursor->splitAndInsertEmptyParagraph();
    QTextCursor oldCursor2 = *cursor;

    if ( f == Qt::PlainText ) {
	cursor->insert( text, TRUE );
	if ( doc->useFormatCollection() && !doc->preProcessor() &&
	     currentFormat != cursor->paragraph()->at( cursor->index() )->format() ) {
	    doc->setSelectionStart( QTextDocument::Temp, oldCursor2 );
	    doc->setSelectionEnd( QTextDocument::Temp, *cursor );
	    doc->setFormat( QTextDocument::Temp, currentFormat, QTextFormat::Format );
	    doc->removeSelection( QTextDocument::Temp );
	}
    } else {
	cursor->paragraph()->setListItem( FALSE );
	cursor->paragraph()->setListDepth( 0 );
	if ( cursor->paragraph()->prev() )
	    cursor->paragraph()->prev()->invalidate(0); // vertical margins might have to change
	doc->setRichTextInternal( text );
    }
    formatMore();
    repaintChanged();
    if ( atBottom )
        scrollToBottom();
    *cursor = oldc;
    if ( !isReadOnly() )
	cursorVisible = TRUE;
    setModified();
    emit textChanged();
}

/*!
    \property QTextEdit::hasSelectedText
    \brief whether some text is selected in selection 0
*/

bool QTextEdit::hasSelectedText() const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return optimHasSelection();
    else
#endif
	return doc->hasSelection( QTextDocument::Standard );
}

/*!
    \property QTextEdit::selectedText
    \brief The selected text (from selection 0) or an empty string if
    there is no currently selected text (in selection 0).

    The text is always returned as \c PlainText if the textFormat() is
    \c PlainText or \c AutoText, otherwise it is returned as HTML.

    \sa hasSelectedText
*/

QString QTextEdit::selectedText() const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return optimSelectedText();
    else
#endif
	return doc->selectedText( QTextDocument::Standard, textFormat() == RichText );
}

bool QTextEdit::handleReadOnlyKeyEvent( QKeyEvent *e )
{
    switch( e->key() ) {
    case Key_Down:
	setContentsPos( contentsX(), contentsY() + 10 );
	break;
    case Key_Up:
	setContentsPos( contentsX(), contentsY() - 10 );
	break;
    case Key_Left:
	setContentsPos( contentsX() - 10, contentsY() );
	break;
    case Key_Right:
	setContentsPos( contentsX() + 10, contentsY() );
	break;
    case Key_PageUp:
	setContentsPos( contentsX(), contentsY() - visibleHeight() );
	break;
    case Key_PageDown:
	setContentsPos( contentsX(), contentsY() + visibleHeight() );
	break;
    case Key_Home:
	setContentsPos( contentsX(), 0 );
	break;
    case Key_End:
	setContentsPos( contentsX(), contentsHeight() - visibleHeight() );
	break;
    case Key_F16: // Copy key on Sun keyboards
	copy();
	break;
#ifndef QT_NO_NETWORKPROTOCOL
    case Key_Return:
    case Key_Enter:
    case Key_Space: {
	if (!doc->focusIndicator.href.isEmpty()
		|| !doc->focusIndicator.name.isEmpty()) {
	    if (!doc->focusIndicator.href.isEmpty()) {
		QUrl u( doc->context(), doc->focusIndicator.href, TRUE );
		emitLinkClicked( u.toString( FALSE, FALSE ) );
	    }
	    if (!doc->focusIndicator.name.isEmpty()) {
		if (::qt_cast<QTextBrowser*>(this)) { // change for 4.0
		    QConnectionList *clist = receivers(
			    "anchorClicked(const QString&,const QString&)");
		    if (!signalsBlocked() && clist) {
			QUObject o[3];
			static_QUType_QString.set(o+1,
				doc->focusIndicator.name);
			static_QUType_QString.set(o+2,
				doc->focusIndicator.href);
			activate_signal( clist, o);
		    }
		}
	    }
#ifndef QT_NO_CURSOR
	    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
	}
    } break;
#endif
    default:
	if ( e->state() & ControlButton ) {
	    switch ( e->key() ) {
	    case Key_C: case Key_F16: // Copy key on Sun keyboards
		copy();
		break;
#ifdef Q_WS_WIN
	    case Key_Insert:
		copy();
		break;
	    case Key_A:
		selectAll();
		break;
#endif
	    }

	}
	return FALSE;
    }
    return TRUE;
}

/*!
    Returns the context of the text edit. The context is a path which
    the text edit's QMimeSourceFactory uses to resolve the locations
    of files and images.

    \sa text
*/

QString QTextEdit::context() const
{
    return doc->context();
}

/*!
    \property QTextEdit::documentTitle
    \brief the title of the document parsed from the text.

    For \c PlainText the title will be an empty string. For \c
    RichText the title will be the text between the \c{<title>} tags,
    if present, otherwise an empty string.
*/

QString QTextEdit::documentTitle() const
{
    return doc->attributes()[ "title" ];
}

void QTextEdit::makeParagVisible( QTextParagraph *p )
{
    setContentsPos( contentsX(), QMIN( p->rect().y(), contentsHeight() - visibleHeight() ) );
}

/*!
    Scrolls the text edit to make the text at the anchor called \a
    name visible, if it can be found in the document. If the anchor
    isn't found no scrolling will occur. An anchor is defined using
    the HTML anchor tag, e.g. \c{<a name="target">}.
*/

void QTextEdit::scrollToAnchor( const QString& name )
{
    if ( !isVisible() ) {
	d->scrollToAnchor = name;
	return;
    }
    if ( name.isEmpty() )
	return;
    sync();
    QTextCursor cursor( doc );
    QTextParagraph* last = doc->lastParagraph();
    for (;;) {
	QTextStringChar* c = cursor.paragraph()->at( cursor.index() );
	if( c->isAnchor() ) {
	    QString a = c->anchorName();
	    if ( a == name ||
		 (a.contains( '#' ) && QStringList::split( '#', a ).contains( name ) ) ) {
		setContentsPos( contentsX(), QMIN( cursor.paragraph()->rect().top() + cursor.totalOffsetY(), contentsHeight() - visibleHeight() ) );
		break;
	    }
	}
	if ( cursor.paragraph() == last && cursor.atParagEnd()  )
	    break;
	cursor.gotoNextLetter();
    }
}

#if (QT_VERSION-0 >= 0x040000)
#error "function anchorAt(const QPoint& pos) should be merged into function anchorAt(const QPoint& pos, AnchorAttribute attr)"
#endif

/*!
    \overload

    If there is an anchor at position \a pos (in contents
    coordinates), its \c href is returned, otherwise QString::null is
    returned.
*/

QString QTextEdit::anchorAt( const QPoint& pos )
{
    return anchorAt(pos, AnchorHref);
}

/*!
    If there is an anchor at position \a pos (in contents
    coordinates), the text for attribute \a attr is returned,
    otherwise QString::null is returned.
*/

QString QTextEdit::anchorAt( const QPoint& pos, AnchorAttribute attr )
{
    QTextCursor c( doc );
    placeCursor( pos, &c );
    switch(attr) {
	case AnchorName:
	    return c.paragraph()->at( c.index() )->anchorName();
	case AnchorHref:
	    return c.paragraph()->at( c.index() )->anchorHref();
    }
    // incase the compiler is really dumb about determining if a function
    // returns something :)
    return QString::null;
}

void QTextEdit::documentWidthChanged( int w )
{
    resizeContents( QMAX( visibleWidth(), w), contentsHeight() );
}

/*! \internal

  This function does nothing
*/

void QTextEdit::updateStyles()
{
}

void QTextEdit::setDocument( QTextDocument *dc )
{
    if ( dc == 0 ) {
        qWarning( "Q3TextEdit::setDocument() called with null Q3TextDocument pointer" );
        return;
    }
    if ( dc == doc )
	return;
    resetInputContext();
    doc = dc;
    delete cursor;
    cursor = new QTextCursor( doc );
    clearUndoRedo();
    undoRedoInfo.doc = doc;
    lastFormatted = 0;
}

#ifndef QT_NO_CLIPBOARD

/*!
    Pastes the text with format \a subtype from the clipboard into the
    text edit at the current cursor position. The \a subtype can be
    "plain" or "html".

    If there is no text with format \a subtype in the clipboard
    nothing happens.

    \sa paste() cut() QTextEdit::copy()
*/

void QTextEdit::pasteSubType( const QCString &subtype )
{
#ifndef QT_NO_MIMECLIPBOARD
    QMimeSource *m = QApplication::clipboard()->data( d->clipboard_mode );
    pasteSubType( subtype, m );
#endif
}

/*! \internal */

void QTextEdit::pasteSubType( const QCString& subtype, QMimeSource *m )
{
#ifndef QT_NO_MIME
    QCString st = subtype;
    if ( subtype != "x-qrichtext" )
	st.prepend( "text/" );
    else
	st.prepend( "application/" );
    if ( !m )
	return;
    if ( doc->hasSelection( QTextDocument::Standard ) )
	removeSelectedText();
    if ( !QRichTextDrag::canDecode( m ) )
	return;
    QString t;
    if ( !QRichTextDrag::decode( m, t, st.data(), subtype ) )
	return;
    if ( st == "application/x-qrichtext" ) {
	int start;
	if ( (start = t.find( "<!--StartFragment-->" )) != -1 ) {
	    start += 20;
	    int end = t.find( "<!--EndFragment-->" );
	    QTextCursor oldC = *cursor;

	    // during the setRichTextInternal() call the cursors
	    // paragraph might get joined with the provious one, so
	    // the cursors one would get deleted and oldC.paragraph()
	    // would be a dnagling pointer. To avoid that try to go
	    // one letter back and later go one forward again.
	    oldC.gotoPreviousLetter();
	    bool couldGoBack = oldC != *cursor;
	    // first para might get deleted, so remember to reset it
	    bool wasAtFirst = oldC.paragraph() == doc->firstParagraph();

	    if ( start < end )
		t = t.mid( start, end - start );
	    else
		t = t.mid( start );
	    lastFormatted = cursor->paragraph();
	    if ( lastFormatted->prev() )
		lastFormatted = lastFormatted->prev();
	    doc->setRichTextInternal( t, cursor );

	    // the first para might have been deleted in
	    // setRichTextInternal(). To be sure, reset it if
	    // necessary.
	    if ( wasAtFirst ) {
		int index = oldC.index();
		oldC.setParagraph( doc->firstParagraph() );
		oldC.setIndex( index );
	    }

	    // if we went back one letter before (see last comment),
	    // go one forward to point to the right position
	    if ( couldGoBack )
		oldC.gotoNextLetter();

	    if ( undoEnabled && !isReadOnly() ) {
		doc->setSelectionStart( QTextDocument::Temp, oldC );
		doc->setSelectionEnd( QTextDocument::Temp, *cursor );

		checkUndoRedoInfo( UndoRedoInfo::Insert );
		if ( !undoRedoInfo.valid() ) {
		    undoRedoInfo.id = oldC.paragraph()->paragId();
		    undoRedoInfo.index = oldC.index();
		    undoRedoInfo.d->text = QString::null;
		}
		int oldLen = undoRedoInfo.d->text.length();
		if ( !doc->preProcessor() ) {
		    QString txt = doc->selectedText( QTextDocument::Temp );
		    undoRedoInfo.d->text += txt;
		    for ( int i = 0; i < (int)txt.length(); ++i ) {
			if ( txt[ i ] != '\n' && oldC.paragraph()->at( oldC.index() )->format() ) {
			    oldC.paragraph()->at( oldC.index() )->format()->addRef();
			    undoRedoInfo.d->text.
				setFormat( oldLen + i, oldC.paragraph()->at( oldC.index() )->format(), TRUE );
			}
			oldC.gotoNextLetter();
		    }
		}
		undoRedoInfo.clear();
		removeSelection( QTextDocument::Temp );
	    }

	    formatMore();
	    setModified();
	    emit textChanged();
	    repaintChanged();
	    ensureCursorVisible();
	    return;
	}
    } else {
#if defined(Q_OS_WIN32)
	// Need to convert CRLF to LF
	t.replace( "\r\n", "\n" );
#elif defined(Q_OS_MAC)
	//need to convert CR to LF
	t.replace( '\r', '\n' );
#endif
	QChar *uc = (QChar *)t.unicode();
	for ( int i=0; (uint) i<t.length(); i++ ) {
	    if ( uc[ i ] < ' ' && uc[ i ] != '\n' && uc[ i ] != '\t' )
		uc[ i ] = ' ';
	}
	if ( !t.isEmpty() )
	    insert( t, FALSE, TRUE );
    }
#endif //QT_NO_MIME
}

#ifndef QT_NO_MIMECLIPBOARD
/*!
    Prompts the user to choose a type from a list of text types
    available, then copies text from the clipboard (if there is any)
    into the text edit at the current text cursor position. Any
    selected text (in selection 0) is first deleted.
*/
void QTextEdit::pasteSpecial( const QPoint& pt )
{
    QCString st = pickSpecial( QApplication::clipboard()->data( d->clipboard_mode ),
			       TRUE, pt );
    if ( !st.isEmpty() )
	pasteSubType( st );
}
#endif
#ifndef QT_NO_MIME
QCString QTextEdit::pickSpecial( QMimeSource* ms, bool always_ask, const QPoint& pt )
{
    if ( ms )  {
#ifndef QT_NO_POPUPMENU
	QPopupMenu popup( this, "qt_pickspecial_menu" );
	QString fmt;
	int n = 0;
	QDict<void> done;
	for (int i = 0; !( fmt = ms->format( i ) ).isNull(); i++) {
	    int semi = fmt.find( ";" );
	    if ( semi >= 0 )
		fmt = fmt.left( semi );
	    if ( fmt.left( 5 ) == "text/" ) {
		fmt = fmt.mid( 5 );
		if ( !done.find( fmt ) ) {
		    done.insert( fmt,(void*)1 );
		    popup.insertItem( fmt, i );
		    n++;
		}
	    }
	}
	if ( n ) {
	    int i = n ==1 && !always_ask ? popup.idAt( 0 ) : popup.exec( pt );
	    if ( i >= 0 )
		return popup.text(i).latin1();
	}
#else
	QString fmt;
	for (int i = 0; !( fmt = ms->format( i ) ).isNull(); i++) {
	    int semi = fmt.find( ";" );
	    if ( semi >= 0 )
		fmt = fmt.left( semi );
	    if ( fmt.left( 5 ) == "text/" ) {
		fmt = fmt.mid( 5 );
		return fmt.latin1();
	    }
	}
#endif
    }
    return QCString();
}
#endif // QT_NO_MIME
#endif // QT_NO_CLIPBOARD

/*!
    \enum QTextEdit::WordWrap

    This enum defines the QTextEdit's word wrap modes.

    \value NoWrap Do not wrap the text.

    \value WidgetWidth Wrap the text at the current width of the
    widget (this is the default). Wrapping is at whitespace by
    default; this can be changed with setWrapPolicy().

    \value FixedPixelWidth Wrap the text at a fixed number of pixels
    from the widget's left side. The number of pixels is set with
    wrapColumnOrWidth().

    \value FixedColumnWidth Wrap the text at a fixed number of
    character columns from the widget's left side. The number of
    characters is set with wrapColumnOrWidth(). This is useful if you
    need formatted text that can also be displayed gracefully on
    devices with monospaced fonts, for example a standard VT100
    terminal, where you might set wrapColumnOrWidth() to 80.

    \sa setWordWrap() wordWrap()
*/

/*!
    \property QTextEdit::wordWrap
    \brief the word wrap mode

    The default mode is \c WidgetWidth which causes words to be
    wrapped at the right edge of the text edit. Wrapping occurs at
    whitespace, keeping whole words intact. If you want wrapping to
    occur within words use setWrapPolicy(). If you set a wrap mode of
    \c FixedPixelWidth or \c FixedColumnWidth you should also call
    setWrapColumnOrWidth() with the width you want.

    \sa WordWrap, wrapColumnOrWidth, wrapPolicy,
*/

void QTextEdit::setWordWrap( WordWrap mode )
{
    if ( wrapMode == mode )
	return;
    wrapMode = mode;
    switch ( mode ) {
    case NoWrap:
	document()->formatter()->setWrapEnabled( FALSE );
	document()->formatter()->setWrapAtColumn( -1 );
	doc->setWidth( visibleWidth() );
	doc->setMinimumWidth( -1 );
	doc->invalidate();
	updateContents();
	lastFormatted = doc->firstParagraph();
	interval = 0;
	formatMore();
	break;
    case WidgetWidth:
	document()->formatter()->setWrapEnabled( TRUE );
	document()->formatter()->setWrapAtColumn( -1 );
	doResize();
	break;
    case FixedPixelWidth:
	document()->formatter()->setWrapEnabled( TRUE );
	document()->formatter()->setWrapAtColumn( -1 );
	if ( wrapWidth < 0 )
	    wrapWidth = 200;
	setWrapColumnOrWidth( wrapWidth );
	break;
    case FixedColumnWidth:
	if ( wrapWidth < 0 )
	    wrapWidth = 80;
	document()->formatter()->setWrapEnabled( TRUE );
	document()->formatter()->setWrapAtColumn( wrapWidth );
	setWrapColumnOrWidth( wrapWidth );
	break;
    }
#ifdef QT_TEXTEDIT_OPTIMIZATION
    checkOptimMode();
#endif
}

QTextEdit::WordWrap QTextEdit::wordWrap() const
{
    return wrapMode;
}

/*!
    \property QTextEdit::wrapColumnOrWidth
    \brief the position (in pixels or columns depending on the wrap mode) where text will be wrapped

    If the wrap mode is \c FixedPixelWidth, the value is the number of
    pixels from the left edge of the text edit at which text should be
    wrapped. If the wrap mode is \c FixedColumnWidth, the value is the
    column number (in character columns) from the left edge of the
    text edit at which text should be wrapped.

    \sa wordWrap
*/
void QTextEdit::setWrapColumnOrWidth( int value )
{
    wrapWidth = value;
    if ( wrapMode == FixedColumnWidth ) {
	document()->formatter()->setWrapAtColumn( wrapWidth );
	resizeContents( 0, 0 );
	doc->setWidth( visibleWidth() );
	doc->setMinimumWidth( -1 );
    } else if (wrapMode == FixedPixelWidth ) {
	document()->formatter()->setWrapAtColumn( -1 );
	resizeContents( wrapWidth, 0 );
	doc->setWidth( wrapWidth );
	doc->setMinimumWidth( wrapWidth );
    } else {
	return;
    }
    doc->invalidate();
    updateContents();
    lastFormatted = doc->firstParagraph();
    interval = 0;
    formatMore();
}

int QTextEdit::wrapColumnOrWidth() const
{
    if ( wrapMode == WidgetWidth )
	return visibleWidth();
    return wrapWidth;
}


/*!
    \enum QTextEdit::WrapPolicy

    This enum defines where text can be wrapped in word wrap mode.

    \value AtWhiteSpace Don't use this deprecated value (it is a
    synonym for \c AtWordBoundary which you should use instead).
    \value Anywhere  Break anywhere, including within words.
    \value AtWordBoundary Break lines at word boundaries, e.g. spaces or
    newlines
    \value AtWordOrDocumentBoundary Break lines at whitespace, e.g.
    spaces or newlines if possible. Break it anywhere otherwise.

    \sa setWrapPolicy()
*/

/*!
    \property QTextEdit::wrapPolicy
    \brief the word wrap policy, at whitespace or anywhere

    Defines where text can be wrapped when word wrap mode is not \c
    NoWrap. The choices are \c AtWordBoundary (the default), \c
    Anywhere and \c AtWordOrDocumentBoundary

    \sa wordWrap
*/

void QTextEdit::setWrapPolicy( WrapPolicy policy )
{
    if ( wPolicy == policy )
	return;
    wPolicy = policy;
    QTextFormatter *formatter;
    if ( policy == AtWordBoundary || policy == AtWordOrDocumentBoundary ) {
	formatter = new QTextFormatterBreakWords;
	formatter->setAllowBreakInWords( policy == AtWordOrDocumentBoundary );
    } else {
	formatter = new QTextFormatterBreakInWords;
    }
    formatter->setWrapAtColumn( document()->formatter()->wrapAtColumn() );
    formatter->setWrapEnabled( document()->formatter()->isWrapEnabled( 0 ) );
    document()->setFormatter( formatter );
    doc->invalidate();
    updateContents();
    lastFormatted = doc->firstParagraph();
    interval = 0;
    formatMore();
}

QTextEdit::WrapPolicy QTextEdit::wrapPolicy() const
{
    return wPolicy;
}

/*!
    Deletes all the text in the text edit.

    \sa cut() removeSelectedText() setText()
*/

void QTextEdit::clear()
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	optimSetText("");
    } else
#endif
    {
	// make clear undoable
	doc->selectAll( QTextDocument::Temp );
	removeSelectedText( QTextDocument::Temp );
	setContentsPos( 0, 0 );
	if ( cursor->isValid() )
	    cursor->restoreState();
	doc->clear( TRUE );
	delete cursor;
	cursor = new QTextCursor( doc );
	lastFormatted = 0;
    }
    updateContents();

    emit cursorPositionChanged( cursor );
    emit cursorPositionChanged( cursor->paragraph()->paragId(), cursor->index() );
}

int QTextEdit::undoDepth() const
{
    return document()->undoDepth();
}

/*!
    \property QTextEdit::length
    \brief the number of characters in the text
*/

int QTextEdit::length() const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode )
	return d->od->len;
    else
#endif
	return document()->length();
}

/*!
    \property QTextEdit::tabStopWidth
    \brief the tab stop width in pixels
*/

int QTextEdit::tabStopWidth() const
{
    return document()->tabStopWidth();
}

void QTextEdit::setUndoDepth( int d )
{
    document()->setUndoDepth( d );
}

void QTextEdit::setTabStopWidth( int ts )
{
    document()->setTabStops( ts );
    doc->invalidate();
    lastFormatted = doc->firstParagraph();
    interval = 0;
    formatMore();
    updateContents();
}

/*!
    \reimp
*/

QSize QTextEdit::sizeHint() const
{
    // cf. QScrollView::sizeHint()
    constPolish();
    int f = 2 * frameWidth();
    int h = fontMetrics().height();
    QSize sz( f, f );
    return sz.expandedTo( QSize(12 * h, 8 * h) );
}

void QTextEdit::clearUndoRedo()
{
    if ( !undoEnabled )
	return;
    undoRedoInfo.clear();
    emit undoAvailable( doc->commands()->isUndoAvailable() );
    emit redoAvailable( doc->commands()->isRedoAvailable() );
}

/*!  \internal
  \warning In Qt 3.1 we will provide a cleaer API for the
  functionality which is provided by this function and in Qt 4.0 this
  function will go away.

  This function gets the format of the character at position \a
  index in paragraph \a para. Sets \a font to the character's font, \a
  color to the character's color and \a verticalAlignment to the
  character's vertical alignment.

  Returns FALSE if \a para or \a index is out of range otherwise
  returns TRUE.
*/

bool QTextEdit::getFormat( int para, int index, QFont *font, QColor *color, VerticalAlignment *verticalAlignment )
{
    if ( !font || !color )
	return FALSE;
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return FALSE;
    if ( index < 0 || index >= p->length() )
	return FALSE;
    *font = p->at( index )->format()->font();
    *color = p->at( index )->format()->color();
    *verticalAlignment = (VerticalAlignment)p->at( index )->format()->vAlign();
    return TRUE;
}

/*!  \internal
  \warning In Qt 3.1 we will provide a cleaer API for the
  functionality which is provided by this function and in Qt 4.0 this
  function will go away.

  This function gets the format of the paragraph \a para. Sets \a
  font to the paragraphs's font, \a color to the paragraph's color, \a
  verticalAlignment to the paragraph's vertical alignment, \a
  alignment to the paragraph's alignment, \a displayMode to the
  paragraph's display mode, \a listStyle to the paragraph's list style
  (if the display mode is QStyleSheetItem::DisplayListItem) and \a
  listDepth to the depth of the list (if the display mode is
  QStyleSheetItem::DisplayListItem).

  Returns FALSE if \a para is out of range otherwise returns TRUE.
*/

bool QTextEdit::getParagraphFormat( int para, QFont *font, QColor *color,
				    VerticalAlignment *verticalAlignment, int *alignment,
				    QStyleSheetItem::DisplayMode *displayMode,
				    QStyleSheetItem::ListStyle *listStyle,
				    int *listDepth )
{
    if ( !font || !color || !alignment || !displayMode || !listStyle )
	return FALSE;
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return FALSE;
    *font = p->at(0)->format()->font();
    *color = p->at(0)->format()->color();
    *verticalAlignment = (VerticalAlignment)p->at(0)->format()->vAlign();
    *alignment = p->alignment();
    *displayMode = p->isListItem() ? QStyleSheetItem::DisplayListItem : QStyleSheetItem::DisplayBlock;
    *listStyle = p->listStyle();
    *listDepth = p->listDepth();
    return TRUE;
}



/*!
    This function is called to create a right mouse button popup menu
    at the document position \a pos. If you want to create a custom
    popup menu, reimplement this function and return the created popup
    menu. Ownership of the popup menu is transferred to the caller.

    \warning The QPopupMenu ID values 0-7 are reserved, and they map to the
    standard operations. When inserting items into your custom popup menu, be
    sure to specify ID values larger than 7.
*/

QPopupMenu *QTextEdit::createPopupMenu( const QPoint& pos )
{
    Q_UNUSED( pos )
#ifndef QT_NO_POPUPMENU
    QPopupMenu *popup = new QPopupMenu( this, "qt_edit_menu" );
    if ( !isReadOnly() ) {
	d->id[ IdUndo ] = popup->insertItem( tr( "&Undo" ) + ACCEL_KEY( Z ) );
	d->id[ IdRedo ] = popup->insertItem( tr( "&Redo" ) + ACCEL_KEY( Y ) );
	popup->insertSeparator();
    }
#ifndef QT_NO_CLIPBOARD
    if ( !isReadOnly() )
	d->id[ IdCut ] = popup->insertItem( tr( "Cu&t" ) + ACCEL_KEY( X ) );
    d->id[ IdCopy ] = popup->insertItem( tr( "&Copy" ) + ACCEL_KEY( C ) );
    if ( !isReadOnly() )
	d->id[ IdPaste ] = popup->insertItem( tr( "&Paste" ) + ACCEL_KEY( V ) );
#endif
    if ( !isReadOnly() ) {
	d->id[ IdClear ] = popup->insertItem( tr( "Clear" ) );
	popup->insertSeparator();
    }
#if defined(Q_WS_X11)
    d->id[ IdSelectAll ] = popup->insertItem( tr( "Select All" ) );
#else
    d->id[ IdSelectAll ] = popup->insertItem( tr( "Select All" ) + ACCEL_KEY( A ) );
#endif
    popup->setItemEnabled( d->id[ IdUndo ], !isReadOnly() && doc->commands()->isUndoAvailable() );
    popup->setItemEnabled( d->id[ IdRedo ], !isReadOnly() && doc->commands()->isRedoAvailable() );
#ifndef QT_NO_CLIPBOARD
    popup->setItemEnabled( d->id[ IdCut ], !isReadOnly() && doc->hasSelection( QTextDocument::Standard, TRUE ) );
#ifdef QT_TEXTEDIT_OPTIMIZATION
    popup->setItemEnabled( d->id[ IdCopy ], d->optimMode ? optimHasSelection() : doc->hasSelection( QTextDocument::Standard, TRUE ) );
#else
    popup->setItemEnabled( d->id[ IdCopy ], doc->hasSelection( QTextDocument::Standard, TRUE ) );
#endif
    popup->setItemEnabled( d->id[ IdPaste ], !isReadOnly() && !QApplication::clipboard()->text( d->clipboard_mode ).isEmpty() );
#endif
    const bool isEmptyDocument = (length() == 0);
    popup->setItemEnabled( d->id[ IdClear ], !isReadOnly() && !isEmptyDocument );
    popup->setItemEnabled( d->id[ IdSelectAll ], !isEmptyDocument );
    return popup;
#else
    return 0;
#endif
}

/*! \overload
    \obsolete
    This function is called to create a right mouse button popup menu.
    If you want to create a custom popup menu, reimplement this function
    and return the created popup menu. Ownership of the popup menu is
    transferred to the caller.

    This function is only called if createPopupMenu( const QPoint & )
    returns 0.
*/

QPopupMenu *QTextEdit::createPopupMenu()
{
    return 0;
}

/*!
    \reimp
*/

void QTextEdit::setFont( const QFont &f )
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	QScrollView::setFont( f );
	doc->setDefaultFormat( f, doc->formatCollection()->defaultFormat()->color() );
	// recalculate the max string width
	QFontMetrics fm(f);
	int i, sw;
	d->od->maxLineWidth = 0;
	for ( i = 0; i < d->od->numLines; i++ ) {
	    sw = fm.width(d->od->lines[LOGOFFSET(i)]);
	    if (d->od->maxLineWidth < sw)
		d->od->maxLineWidth = sw;
	}
	resizeContents(d->od->maxLineWidth + 4, d->od->numLines * fm.lineSpacing() + 1);
	return;
    }
#endif
    QScrollView::setFont( f );
    doc->setMinimumWidth( -1 );
    doc->setDefaultFormat( f, doc->formatCollection()->defaultFormat()->color() );
    lastFormatted = doc->firstParagraph();
    formatMore();
    repaintChanged();
}

/*!
    \fn QTextEdit::zoomIn()

    \overload

    Zooms in on the text by making the base font size one point
    larger and recalculating all font sizes to be the new size. This
    does not change the size of any images.

    \sa zoomOut()
*/

/*!
    \fn QTextEdit::zoomOut()

    \overload

    Zooms out on the text by making the base font size one point
    smaller and recalculating all font sizes to be the new size. This
    does not change the size of any images.

    \sa zoomIn()
*/


/*!
    Zooms in on the text by making the base font size \a range
    points larger and recalculating all font sizes to be the new size.
    This does not change the size of any images.

    \sa zoomOut()
*/

void QTextEdit::zoomIn( int range )
{
    QFont f( QScrollView::font() );
    f.setPointSize( QFontInfo(f).pointSize() + range );
    setFont( f );
}

/*!
    Zooms out on the text by making the base font size \a range points
    smaller and recalculating all font sizes to be the new size. This
    does not change the size of any images.

    \sa zoomIn()
*/

void QTextEdit::zoomOut( int range )
{
    QFont f( QScrollView::font() );
    f.setPointSize( QMAX( 1, QFontInfo(f).pointSize() - range ) );
    setFont( f );
}

/*!
    Zooms the text by making the base font size \a size points and
    recalculating all font sizes to be the new size. This does not
    change the size of any images.
*/

void QTextEdit::zoomTo( int size )
{
    QFont f( QScrollView::font() );
    f.setPointSize( size );
    setFont( f );
}

/*!
   QTextEdit is optimized for large amounts text. One of its
   optimizations is to format only the visible text, formatting the rest
   on demand, e.g. as the user scrolls, so you don't usually need to
   call this function.

    In some situations you may want to force the whole text
    to be formatted. For example, if after calling setText(), you wanted
    to know the height of the document (using contentsHeight()), you
    would call this function first.
*/

void QTextEdit::sync()
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	QFontMetrics fm( QScrollView::font() );
	resizeContents( d->od->maxLineWidth + 4, d->od->numLines * fm.lineSpacing() + 1 );
    } else
#endif
    {
	while ( lastFormatted ) {
	    lastFormatted->format();
	    lastFormatted = lastFormatted->next();
	}
	resizeContents( contentsWidth(), doc->height() );
    }
    updateScrollBars();
}

/*!
    \reimp
*/

void QTextEdit::setEnabled( bool b )
{
    QScrollView::setEnabled( b );
    if ( textFormat() == PlainText ) {
	QTextFormat *f = doc->formatCollection()->defaultFormat();
	f->setColor( colorGroup().text() );
	updateContents();
    }
}

/*!
    Sets the background color of selection number \a selNum to \a back
    and specifies whether the text of this selection should be
    inverted with \a invertText.

    This only works for \a selNum > 0. The default selection (\a
    selNum == 0) gets its attributes from the text edit's
    colorGroup().
*/

void QTextEdit::setSelectionAttributes( int selNum, const QColor &back, bool invertText )
{
    if ( selNum < 1 )
	return;
    if ( selNum > doc->numSelections() )
	doc->addSelection( selNum );
    doc->setSelectionColor( selNum, back );
    doc->setInvertSelectionText( selNum, invertText );
}

/*!
    \reimp
*/
void QTextEdit::windowActivationChange( bool oldActive )
{
    if ( oldActive && scrollTimer )
	scrollTimer->stop();
    if ( palette().active() != palette().inactive() )
	updateContents();
    QScrollView::windowActivationChange( oldActive );
}

void QTextEdit::setReadOnly( bool b )
{
    if ( (bool) readonly == b )
	return;
    readonly = b;
#ifndef QT_NO_CURSOR
    if ( readonly )
	viewport()->setCursor( arrowCursor );
    else
	viewport()->setCursor( ibeamCursor );
    setInputMethodEnabled( !readonly );
#endif
#ifdef QT_TEXTEDIT_OPTIMIZATION
    checkOptimMode();
#endif
}

/*!
    Scrolls to the bottom of the document and does formatting if
    required.
*/

void QTextEdit::scrollToBottom()
{
    sync();
    setContentsPos( contentsX(), contentsHeight() - visibleHeight() );
}

/*!
    Returns the rectangle of the paragraph \a para in contents
    coordinates, or an invalid rectangle if \a para is out of range.
*/

QRect QTextEdit::paragraphRect( int para ) const
{
    QTextEdit *that = (QTextEdit *)this;
    that->sync();
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return QRect( -1, -1, -1, -1 );
    return p->rect();
}

/*!
    Returns the paragraph which is at position \a pos (in contents
    coordinates).
*/

int QTextEdit::paragraphAt( const QPoint &pos ) const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	QFontMetrics fm( QScrollView::font() );
	int parag = pos.y() / fm.lineSpacing();
	if ( parag <= d->od->numLines )
	    return parag;
	else
	    return 0;
    }
#endif
    QTextCursor c( doc );
    c.place( pos, doc->firstParagraph() );
    if ( c.paragraph() )
	return c.paragraph()->paragId();
    return -1; // should never happen..
}

/*!
    Returns the index of the character (relative to its paragraph) at
    position \a pos (in contents coordinates). If \a para is not 0,
    \a *para is set to the character's paragraph.
*/

int QTextEdit::charAt( const QPoint &pos, int *para ) const
{
#ifdef QT_TEXTEDIT_OPTIMIZATION
    if ( d->optimMode ) {
	int par = paragraphAt( pos );
	if ( para )
	    *para = par;
	return optimCharIndex( d->od->lines[ LOGOFFSET(par) ], pos.x() );
    }
#endif
    QTextCursor c( doc );
    c.place( pos, doc->firstParagraph() );
    if ( c.paragraph() ) {
	if ( para )
	    *para = c.paragraph()->paragId();
	return c.index();
    }
    return -1; // should never happen..
}

/*!
    Sets the background color of the paragraph \a para to \a bg.
*/

void QTextEdit::setParagraphBackgroundColor( int para, const QColor &bg )
{
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return;
    p->setBackgroundColor( bg );
    repaintChanged();
}

/*!
    Clears the background color of the paragraph \a para, so that the
    default color is used again.
*/

void QTextEdit::clearParagraphBackground( int para )
{
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return;
    p->clearBackgroundColor();
    repaintChanged();
}

/*!
    Returns the background color of the paragraph \a para or an
    invalid color if \a para is out of range or the paragraph has no
    background set
*/

QColor QTextEdit::paragraphBackgroundColor( int para ) const
{
    QTextParagraph *p = doc->paragAt( para );
    if ( !p )
	return QColor();
    QColor *c = p->backgroundColor();
    if ( c )
	return *c;
    return QColor();
}

/*!
    \property QTextEdit::undoRedoEnabled
    \brief whether undo/redo is enabled

    When changing this property, the undo/redo history is cleared.

    The default is TRUE.
*/

void QTextEdit::setUndoRedoEnabled( bool b )
{
    undoRedoInfo.clear();
    doc->commands()->clear();

    undoEnabled = b;
}

bool QTextEdit::isUndoRedoEnabled() const
{
    return undoEnabled;
}

/*!
    Returns TRUE if undo is available; otherwise returns FALSE.
*/

bool QTextEdit::isUndoAvailable() const
{
    return undoEnabled && (doc->commands()->isUndoAvailable() || undoRedoInfo.valid());
}

/*!
    Returns TRUE if redo is available; otherwise returns FALSE.
*/

bool QTextEdit::isRedoAvailable() const
{
    return undoEnabled && doc->commands()->isRedoAvailable();
}

void QTextEdit::ensureFormatted( QTextParagraph *p )
{
    while ( !p->isValid() ) {
	if ( !lastFormatted )
	    return;
	formatMore();
    }
}

/*! \internal */
void QTextEdit::updateCursor( const QPoint & pos )
{
    if ( isReadOnly() && linksEnabled() ) {
	QTextCursor c = *cursor;
	placeCursor( pos, &c, TRUE );

#ifndef QT_NO_NETWORKPROTOCOL
	bool insideParagRect = TRUE;
	if (c.paragraph() == doc->lastParagraph()
	    && c.paragraph()->rect().y() + c.paragraph()->rect().height() < pos.y())
	    insideParagRect = FALSE;
	if (insideParagRect && c.paragraph() && c.paragraph()->at( c.index() ) &&
	    c.paragraph()->at( c.index() )->isAnchor()) {
	    if (!c.paragraph()->at( c.index() )->anchorHref().isEmpty()
		    && c.index() < c.paragraph()->length() - 1 )
		onLink = c.paragraph()->at( c.index() )->anchorHref();
	    else
		onLink = QString::null;

	    if (!c.paragraph()->at( c.index() )->anchorName().isEmpty()
		    && c.index() < c.paragraph()->length() - 1 )
		d->onName = c.paragraph()->at( c.index() )->anchorName();
	    else
		d->onName = QString::null;

	    if (!c.paragraph()->at( c.index() )->anchorHref().isEmpty() ) {
#ifndef QT_NO_CURSOR
		viewport()->setCursor( onLink.isEmpty() ? arrowCursor : pointingHandCursor );
#endif
		QUrl u( doc->context(), onLink, TRUE );
		emitHighlighted( u.toString( FALSE, FALSE ) );
	    }
	} else {
#ifndef QT_NO_CURSOR
	    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
#endif
	    onLink = QString::null;
	    emitHighlighted( QString::null );
	}
#endif
    }
}

/*!
  Places the cursor \a c at the character which is closest to position
  \a pos (in contents coordinates). If \a c is 0, the default text
  cursor is used.

  \sa setCursorPosition()
*/
void QTextEdit::placeCursor( const QPoint &pos, QTextCursor *c )
{
    placeCursor( pos, c, FALSE );
}

/*! \internal */
void QTextEdit::clipboardChanged()
{
#ifndef QT_NO_CLIPBOARD
    // don't listen to selection changes
    disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), this, 0);
#endif
    selectAll(FALSE);
}

/*! \property QTextEdit::tabChangesFocus
  \brief whether TAB changes focus or is accepted as input

  In some occasions text edits should not allow the user to input
  tabulators or change indentation using the TAB key, as this breaks
  the focus chain. The default is FALSE.

*/

void QTextEdit::setTabChangesFocus( bool b )
{
    d->tabChangesFocus = b;
}

bool QTextEdit::tabChangesFocus() const
{
    return d->tabChangesFocus;
}

#ifdef QT_TEXTEDIT_OPTIMIZATION
/* Implementation of optimized LogText mode follows */

static void qSwap( int * a, int * b )
{
    if ( !a || !b )
	return;
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

/*! \internal */
bool QTextEdit::checkOptimMode()
{
    bool oldMode = d->optimMode;
    if ( textFormat() == LogText ) {
	setReadOnly( TRUE );
	d->optimMode = TRUE;
    } else {
	d->optimMode = FALSE;
    }

    // when changing mode - try to keep selections and text
    if ( oldMode != d->optimMode ) {
	if ( d->optimMode ) {
	    d->od = new QTextEditOptimPrivate;
	    connect( scrollTimer, SIGNAL( timeout() ), this, SLOT( optimDoAutoScroll() ) );
	    disconnect( doc, SIGNAL( minimumWidthChanged(int) ), this, SLOT( documentWidthChanged(int) ) );
	    disconnect( scrollTimer, SIGNAL( timeout() ), this, SLOT( autoScrollTimerDone() ) );
	    disconnect( formatTimer, SIGNAL( timeout() ), this, SLOT( formatMore() ) );
	    optimSetText( doc->originalText() );
	    doc->clear(TRUE);
	    delete cursor;
	    cursor = new QTextCursor( doc );
	} else {
	    disconnect( scrollTimer, SIGNAL( timeout() ), this, SLOT( optimDoAutoScroll() ) );
	    connect( doc, SIGNAL( minimumWidthChanged(int) ), this, SLOT( documentWidthChanged(int) ) );
	    connect( scrollTimer, SIGNAL( timeout() ), this, SLOT( autoScrollTimerDone() ) );
	    connect( formatTimer, SIGNAL( timeout() ), this, SLOT( formatMore() ) );
	    setText( optimText() );
	    delete d->od;
	    d->od = 0;
	}
    }
    return d->optimMode;
}

/*! \internal */
QString QTextEdit::optimText() const
{
    QString str, tmp;

    if ( d->od->len == 0 )
	return str;

    // concatenate all strings
    int i;
    int offset;
    QMapConstIterator<int,QTextEditOptimPrivate::Tag *> it;
    QTextEditOptimPrivate::Tag * ftag = 0;
    for ( i = 0; i < d->od->numLines; i++ ) {
	if ( d->od->lines[ LOGOFFSET(i) ].isEmpty() ) { // CR lines are empty
	    str += "\n";
	} else {
	    tmp = d->od->lines[ LOGOFFSET(i) ] + "\n";
	    // inject the tags for this line
	    if ( (it = d->od->tagIndex.find( LOGOFFSET(i) )) != d->od->tagIndex.end() )
		ftag = it.data();
	    offset = 0;
	    while ( ftag && ftag->line == i ) {
		tmp.insert( ftag->index + offset, "<" + ftag->tag + ">" );
		offset += ftag->tag.length() + 2; // 2 -> the '<' and '>' chars
		ftag = ftag->next;
	    }
	    str += tmp;
	}
    }
    return str;
}

/*! \internal */
void QTextEdit::optimSetText( const QString &str )
{
    optimRemoveSelection();
// this is just too slow - but may have to go in due to compatibility reasons
//     if ( str == optimText() )
//	return;
    d->od->numLines = 0;
    d->od->lines.clear();
    d->od->maxLineWidth = 0;
    d->od->len = 0;
    d->od->clearTags();
    QFontMetrics fm( QScrollView::font() );
    if ( !(str.isEmpty() || str.isNull() || d->maxLogLines == 0) ) {
	QStringList strl = QStringList::split( '\n', str, TRUE );
	int lWidth = 0;
	for ( QStringList::Iterator it = strl.begin(); it != strl.end(); ++it ) {
	    optimParseTags( &*it );
	    optimCheckLimit( *it );
	    lWidth = fm.width( *it );
	    if ( lWidth > d->od->maxLineWidth )
		d->od->maxLineWidth = lWidth;
	}
    }
    resizeContents( d->od->maxLineWidth + 4, d->od->numLines * fm.lineSpacing() + 1 );
    repaintContents();
    emit textChanged();
}

/*! \internal

  Append \a tag to the tag list.
*/
QTextEditOptimPrivate::Tag * QTextEdit::optimAppendTag( int index, const QString & tag )
{
    QTextEditOptimPrivate::Tag * t = new QTextEditOptimPrivate::Tag, * tmp;

    if ( d->od->tags == 0 )
	d->od->tags = t;
    t->bold = t->italic = t->underline = FALSE;
    t->line  = d->od->numLines;
    t->index = index;
    t->tag   = tag;
    t->leftTag = 0;
    t->parent  = 0;
    t->prev = d->od->lastTag;
    if ( d->od->lastTag )
	d->od->lastTag->next = t;
    t->next = 0;
    d->od->lastTag = t;
    tmp = d->od->tagIndex[ LOGOFFSET(t->line) ];
    if ( !tmp || (tmp && tmp->index > t->index) ) {
	d->od->tagIndex.replace( LOGOFFSET(t->line), t );
    }
    return t;
}

 /*! \internal

   Insert \a tag in the tag - according to line and index numbers
*/

QTextEditOptimPrivate::Tag *QTextEdit::optimInsertTag(int line, int index, const QString &tag)
{
    QTextEditOptimPrivate::Tag *t = new QTextEditOptimPrivate::Tag, *tmp;

    if (d->od->tags == 0)
	d->od->tags = t;
    t->bold = t->italic = t->underline = FALSE;
    t->line  = line;
    t->index = index;
    t->tag   = tag;
    t->leftTag = 0;
    t->parent  = 0;
    t->next = 0;
    t->prev = 0;

    // find insertion pt. in tag struct.
    QMap<int,QTextEditOptimPrivate::Tag *>::ConstIterator it;
    if ((it = d->od->tagIndex.find(LOGOFFSET(line))) != d->od->tagIndex.end()) {
	tmp = *it;
	if (tmp->index >= index) { // the exisiting tag may be placed AFTER the one we want to insert
	    tmp = tmp->prev;
	} else {
	    while (tmp && tmp->next && tmp->next->line == line && tmp->next->index <= index)
		tmp = tmp->next;
	}
    } else {
	tmp = d->od->tags;
	while (tmp && tmp->next && tmp->next->line < line)
	    tmp = tmp->next;
	if (tmp == d->od->tags)
	    tmp = 0;
    }

    t->prev = tmp;
    t->next = tmp ? tmp->next : 0;
    if (t->next)
	t->next->prev = t;
    if (tmp)
	tmp->next = t;

    tmp = d->od->tagIndex[LOGOFFSET(t->line)];
    if (!tmp || (tmp && tmp->index >= t->index)) {
	d->od->tagIndex.replace(LOGOFFSET(t->line), t);
    }
    return t;
}


/*! \internal

  Find tags in \a line, remove them from \a line and put them in a
  structure.

  A tag is delimited by '<' and '>'. The characters '<', '>' and '&'
  are escaped by using '&lt;', '&gt;' and '&amp;'. Left-tags marks
  the starting point for formatting, while right-tags mark the ending
  point. A right-tag is the same as a left-tag, but with a '/'
  appearing before the tag keyword.  E.g a valid left-tag: <b>, and
  a valid right-tag: </b>.  Tags can be nested, but they have to be
  closed in the same order as they are opened. E.g:
  <font color=red><font color=blue>blue</font>red</font> - is valid, while:
  <font color=red><b>bold red</font> just bold</b> - is invalid since the font tag is
  closed before the bold tag. Note that a tag does not have to be
  closed: '<font color=blue>Lots of text - and then some..'  is perfectly valid for
  setting all text appearing after the tag to blue.  A tag can be used
  to change the color of a piece of text, or set one of the following
  formatting attributes: bold, italic and underline.  These attributes
  are set using the <b>, <i> and <u> tags.  Example of valid tags:
  <font color=red>, </font>, <b>, <u>, <i>, </i>.
  Example of valid text:
  This is some <font color=red>red text</font>, while this is some <font color=green>green
  text</font>. <font color=blue><font color=yellow>This is yellow</font>, while this is
  blue.</font>

  Note that only the color attribute of the HTML font tag is supported.

  Limitations:
  1. A tag cannot span several lines.
  2. Very limited error checking - mismatching left/right-tags is the
  only thing that is detected.

*/
void QTextEdit::optimParseTags( QString * line, int lineNo, int indexOffset )
{
    int len = line->length();
    int i, startIndex = -1, endIndex = -1, escIndex = -1;
    int state = 0; // 0 = outside tag, 1 = inside tag
    bool tagOpen, tagClose;
    int bold = 0, italic = 0, underline = 0;
    QString tagStr;
    QPtrStack<QTextEditOptimPrivate::Tag> tagStack;

    for ( i = 0; i < len; i++ ) {
	tagOpen = (*line)[i] == '<';
	tagClose = (*line)[i] == '>';

	// handle '&lt;' and '&gt;' and '&amp;'
	if ( (*line)[i] == '&' ) {
	    escIndex = i;
	    continue;
	} else if ( escIndex != -1 && (*line)[i] == ';' ) {
	    QString esc = line->mid( escIndex, i - escIndex + 1 );
	    QString c;
	    if ( esc == "&lt;" )
		c = '<';
	    else if ( esc == "&gt;" )
		c = '>';
	    else if ( esc == "&amp;" )
		c = '&';
	    line->replace( escIndex, i - escIndex + 1, c );
	    len = line->length();
	    i -= i-escIndex;
	    escIndex = -1;
	    continue;
	}

	if ( state == 0 && tagOpen ) {
	    state = 1;
	    startIndex = i;
	    continue;
	}
	if ( state == 1 && tagClose ) {
	    state = 0;
	    endIndex = i;
	    if ( !tagStr.isEmpty() ) {
		QTextEditOptimPrivate::Tag * tag, * cur, * tmp;
		bool format = TRUE;

		if ( tagStr == "b" )
		    bold++;
		else if ( tagStr == "/b" )
		    bold--;
		else if ( tagStr == "i" )
		    italic++;
		else if ( tagStr == "/i" )
		    italic--;
		else if ( tagStr == "u" )
		    underline++;
		else if ( tagStr == "/u" )
		    underline--;
		else
		    format = FALSE;
		if ( lineNo > -1 )
		    tag = optimInsertTag( lineNo, startIndex + indexOffset, tagStr );
		else
		    tag = optimAppendTag( startIndex, tagStr );
		// everything that is not a b, u or i tag is considered
		// to be a color tag.
		tag->type = format ? QTextEditOptimPrivate::Format
			    : QTextEditOptimPrivate::Color;
		if ( tagStr[0] == '/' ) {
		    // this is a right-tag - search for the left-tag
		    // and possible parent tag
		    cur = tag->prev;
		    if ( !cur ) {
#ifdef QT_CHECK_RANGE
			qWarning( "QTextEdit::optimParseTags: no left-tag for '<" + tag->tag + ">' in line %d.", tag->line + 1 );
#endif
			return; // something is wrong - give up
		    }
		    while ( cur ) {
			if ( cur->leftTag ) { // push right-tags encountered
			    tagStack.push( cur );
			} else {
			    tmp = tagStack.pop();
			    if ( !tmp ) {
				if ( (("/" + cur->tag) == tag->tag) ||
				     (tag->tag == "/font" && cur->tag.left(4) == "font") ) {
				    // set up the left and parent of this tag
				    tag->leftTag = cur;
				    tmp = cur->prev;
				    if ( tmp && tmp->parent ) {
					tag->parent = tmp->parent;
				    } else if ( tmp && !tmp->leftTag ) {
					tag->parent = tmp;
				    }
				    break;
				} else if ( !cur->leftTag ) {
#ifdef QT_CHECK_RANGE
				    qWarning( "QTextEdit::optimParseTags: mismatching %s-tag for '<" + cur->tag + ">' in line %d.", cur->tag[0] == '/' ? "left" : "right", cur->line + 1 );
#endif
				    return; // something is amiss - give up
				}
			    }
			}
			cur = cur->prev;
		    }
		} else {
		    tag->bold = bold > 0;
		    tag->italic = italic > 0;
		    tag->underline = underline > 0;
		    tmp = tag->prev;
		    while ( tmp && tmp->leftTag ) {
			tmp = tmp->leftTag->parent;
		    }
		    if ( tmp ) {
			tag->bold |= tmp->bold;
			tag->italic |= tmp->italic;
			tag->underline |= tmp->underline;
		    }
		}
	    }
	    if ( startIndex != -1 ) {
		int l = (endIndex == -1) ?
			line->length() - startIndex : endIndex - startIndex;
		line->remove( startIndex, l+1 );
		len = line->length();
		i -= l+1;
	    }
	    tagStr = "";
	    continue;
	}

	if ( state == 1 ) {
	    tagStr += (*line)[i];
	}
    }
}

// calculate the width of a string in pixels inc. tabs
static int qStrWidth(const QString& str, int tabWidth, const QFontMetrics& fm)
{
    int tabs = str.contains('\t');

    if (!tabs)
	return fm.width(str);

    int newIdx = 0;
    int lastIdx = 0;
    int strWidth = 0;
    int tn;
    for (tn = 1; tn <= tabs; ++tn) {
	newIdx = str.find('\t', newIdx);
	strWidth += fm.width(str.mid(lastIdx, newIdx - lastIdx));
	if (strWidth >= tn * tabWidth) {
	    int u = tn;
	    while (strWidth >= u * tabWidth)
		++u;
	    strWidth = u * tabWidth;
	} else {
	    strWidth = tn * tabWidth;
	}
	lastIdx = ++newIdx;
    }
    if ((int)str.length() > newIdx)
	strWidth += fm.width(str.mid(newIdx));
    return strWidth;
}

bool QTextEdit::optimHasBoldMetrics(int line)
{
    QTextEditOptimPrivate::Tag *t;
    QMapConstIterator<int,QTextEditOptimPrivate::Tag *> it;
    if ((it = d->od->tagIndex.find(line)) != d->od->tagIndex.end()) {
	t = *it;
	while (t && t->line == line) {
	    if (t->bold)
		return TRUE;
	    t = t->next;
	}
    } else if ((t = optimPreviousLeftTag(line)) && t->bold) {
	return TRUE;
    }
    return FALSE;
}

/*! \internal

  Append \a str to the current text buffer. Parses each line to find
  formatting tags.
*/
void QTextEdit::optimAppend( const QString &str )
{
    if ( str.isEmpty() || str.isNull() || d->maxLogLines == 0 )
	return;

    QStringList strl = QStringList::split( '\n', str, TRUE );
    QStringList::Iterator it = strl.begin();

    QFontMetrics fm(QScrollView::font());
    int lWidth = 0;

    for ( ; it != strl.end(); ++it ) {
	optimParseTags( &*it );
	optimCheckLimit( *it );
	if (optimHasBoldMetrics(d->od->numLines-1)) {
	    QFont fnt = QScrollView::font();
	    fnt.setBold(TRUE);
	    fm = QFontMetrics(fnt);
	}
	lWidth = qStrWidth(*it, tabStopWidth(), fm) + 4;
	if ( lWidth > d->od->maxLineWidth )
	    d->od->maxLineWidth = lWidth;
    }
    bool scrollToEnd = contentsY() >= contentsHeight() - visibleHeight();
    resizeContents( d->od->maxLineWidth + 4, d->od->numLines * fm.lineSpacing() + 1 );
    if ( scrollToEnd ) {
	updateScrollBars();
	ensureVisible( contentsX(), contentsHeight(), 0, 0 );
    }
    // when a max log size is set, the text may not be redrawn because
    // the size of the viewport may not have changed
    if ( d->maxLogLines > -1 )
	viewport()->update();
    emit textChanged();
}


static void qStripTags(QString *line)
{
    int len = line->length();
    int i, startIndex = -1, endIndex = -1, escIndex = -1;
    int state = 0; // 0 = outside tag, 1 = inside tag
    bool tagOpen, tagClose;

    for ( i = 0; i < len; i++ ) {
	tagOpen = (*line)[i] == '<';
	tagClose = (*line)[i] == '>';

	// handle '&lt;' and '&gt;' and '&amp;'
	if ( (*line)[i] == '&' ) {
	    escIndex = i;
	    continue;
	} else if ( escIndex != -1 && (*line)[i] == ';' ) {
	    QString esc = line->mid( escIndex, i - escIndex + 1 );
	    QString c;
	    if ( esc == "&lt;" )
		c = '<';
	    else if ( esc == "&gt;" )
		c = '>';
	    else if ( esc == "&amp;" )
		c = '&';
	    line->replace( escIndex, i - escIndex + 1, c );
	    len = line->length();
	    i -= i-escIndex;
	    escIndex = -1;
	    continue;
	}

	if ( state == 0 && tagOpen ) {
	    state = 1;
	    startIndex = i;
	    continue;
	}
	if ( state == 1 && tagClose ) {
	    state = 0;
	    endIndex = i;
	    if ( startIndex != -1 ) {
		int l = (endIndex == -1) ?
			line->length() - startIndex : endIndex - startIndex;
		line->remove( startIndex, l+1 );
		len = line->length();
		i -= l+1;
	    }
	    continue;
	}
    }
}

/*! \internal

    Inserts the text into \a line at index \a index.
*/

void QTextEdit::optimInsert(const QString& text, int line, int index)
{
    if (text.isEmpty() || d->maxLogLines == 0)
	return;
    if (line < 0)
	line = 0;
    if (line > d->od->numLines-1)
	line = d->od->numLines-1;
    if (index < 0)
	index = 0;
    if (index > (int) d->od->lines[line].length())
	index = d->od->lines[line].length();

    QStringList strl = QStringList::split('\n', text, TRUE);
    int numNewLines = (int)strl.count() - 1;
    QTextEditOptimPrivate::Tag *tag = 0;
    QMap<int,QTextEditOptimPrivate::Tag *>::ConstIterator ii;
    int x;

    if (numNewLines == 0) {
	// Case 1. Fast single line case - just inject it!
	QString stripped = text;
	qStripTags( &stripped );
	d->od->lines[LOGOFFSET(line)].insert(index, stripped);
	// move the tag indices following the insertion pt.
	if ((ii = d->od->tagIndex.find(LOGOFFSET(line))) != d->od->tagIndex.end()) {
	    tag = *ii;
	    while (tag && (LOGOFFSET(tag->line) == line && tag->index < index))
		tag = tag->next;
	    while (tag && (LOGOFFSET(tag->line) == line)) {
		tag->index += stripped.length();
		tag = tag->next;
	    }
	}
	stripped = text;
	optimParseTags(&stripped, line, index);
    } else if (numNewLines > 0) {
        // Case 2. We have at least 1 newline char - split at
        // insertion pt. and make room for new lines - complex and slow!
	QString left = d->od->lines[LOGOFFSET(line)].left(index);
	QString right = d->od->lines[LOGOFFSET(line)].mid(index);

	// rearrange lines for insertion
	for (x = d->od->numLines - 1; x > line; x--)
	    d->od->lines[x + numNewLines] = d->od->lines[x];
	d->od->numLines += numNewLines;

	// fix the tag index and the tag line/index numbers - this
	// might take a while..
	for (x = line; x < d->od->numLines; x++) {
	    if ((ii = d->od->tagIndex.find(LOGOFFSET(line))) != d->od->tagIndex.end()) {
		tag = ii.data();
		if (LOGOFFSET(tag->line) == line)
		    while (tag && (LOGOFFSET(tag->line) == line && tag->index < index))
			tag = tag->next;
	    }
	}

	// relabel affected tags with new line numbers and new index
	// positions
	while (tag) {
	    if (LOGOFFSET(tag->line) == line)
		tag->index -= index;
	    tag->line += numNewLines;
	    tag = tag->next;
	}

	// generate a new tag index
	d->od->tagIndex.clear();
	tag = d->od->tags;
	while (tag) {
	    if (!((ii = d->od->tagIndex.find(LOGOFFSET(tag->line))) != d->od->tagIndex.end()))
		d->od->tagIndex[LOGOFFSET(tag->line)] = tag;
	    tag = tag->next;
	}

	// update the tag indices on the spliced line - needs to be done before new tags are added
	QString stripped = strl[strl.count() - 1];
	qStripTags(&stripped);
	if ((ii = d->od->tagIndex.find(LOGOFFSET(line + numNewLines))) != d->od->tagIndex.end()) {
	    tag = *ii;
	    while (tag && (LOGOFFSET(tag->line) == line + numNewLines)) {
		tag->index += stripped.length();
		tag = tag->next;
	    }
	}

	// inject the new lines
	QStringList::Iterator it = strl.begin();
	x = line;
	int idx;
	for (; it != strl.end(); ++it) {
	    stripped = *it;
	    qStripTags(&stripped);
	    if (x == line) {
		stripped = left + stripped;
		idx = index;
	    } else {
		idx = 0;
	    }
	    d->od->lines[LOGOFFSET(x)] = stripped;
	    optimParseTags(&*it, x++, idx);
	}
	d->od->lines[LOGOFFSET(x - 1)] += right;
    }
    // recalculate the pixel width of the longest injected line -
    QFontMetrics fm(QScrollView::font());
    int lWidth = 0;

    for (x = line; x < line + numNewLines; x++) {
	if (optimHasBoldMetrics(x)) {
	    QFont fnt = QScrollView::font();
	    fnt.setBold(TRUE);
	    fm = QFontMetrics(fnt);
	}
	lWidth = fm.width(d->od->lines[x]) + 4;
	if (lWidth > d->od->maxLineWidth)
	    d->od->maxLineWidth = lWidth;
    }
    resizeContents(d->od->maxLineWidth + 4, d->od->numLines * fm.lineSpacing() + 1);
    repaintContents();
    emit textChanged();
}



/*! \internal

  Returns the first open left-tag appearing before line \a line.
 */
QTextEditOptimPrivate::Tag * QTextEdit::optimPreviousLeftTag( int line )
{
    QTextEditOptimPrivate::Tag * ftag = 0;
    QMapConstIterator<int,QTextEditOptimPrivate::Tag *> it;
    if ( (it = d->od->tagIndex.find( LOGOFFSET(line) )) != d->od->tagIndex.end() )
	ftag = it.data();
    if ( !ftag ) {
	// start searching for an open tag
	ftag = d->od->tags;
	while ( ftag ) {
	    if ( ftag->line > line || ftag->next == 0 ) {
		if ( ftag->line > line )
		    ftag = ftag->prev;
		break;
	    }
	    ftag = ftag->next;
	}
    } else {
	ftag = ftag->prev;
    }

    if ( ftag ) {
	if ( ftag && ftag->parent ) // use the open parent tag
	    ftag = ftag->parent;
	else if ( ftag && ftag->leftTag ) // this is a right-tag with no parent
	    ftag = 0;
    }
    return ftag;
}

/*! \internal

  Set the format for the string starting at index \a start and ending
  at \a end according to \a tag. If \a tag is a Format tag, find the
  first open color tag appearing before \a tag and use that tag to
  color the string.
*/
void QTextEdit::optimSetTextFormat( QTextDocument * td, QTextCursor * cur,
				    QTextFormat * f, int start, int end,
				    QTextEditOptimPrivate::Tag * tag )
{
    int formatFlags = QTextFormat::Bold | QTextFormat::Italic |
		      QTextFormat::Underline;
    cur->setIndex( start );
    td->setSelectionStart( 0, *cur );
    cur->setIndex( end );
    td->setSelectionEnd( 0, *cur );
    QStyleSheetItem * ssItem = styleSheet()->item( tag->tag );
    if ( !ssItem || tag->type == QTextEditOptimPrivate::Format ) {
	f->setBold( tag->bold );
	f->setItalic( tag->italic );
	f->setUnderline( tag->underline );
	if ( tag->type == QTextEditOptimPrivate::Format ) {
	    // check to see if there are any open color tags prior to
	    // this format tag
	    tag = tag->prev;
	    while ( tag && (tag->type == QTextEditOptimPrivate::Format ||
			    tag->leftTag) ) {
		tag = tag->leftTag ? tag->parent : tag->prev;
	    }
	}
	if ( tag ) {
	    QString col = tag->tag.simplifyWhiteSpace();
	    if ( col.left( 10 ) == "font color" ) {
		int i = col.find( '=', 10 );
		col = col.mid( i + 1 ).simplifyWhiteSpace();
		if ( col[0] == '\"' )
		    col = col.mid( 1, col.length() - 2 );
	    }
	    QColor color = QColor( col );
	    if ( color.isValid() ) {
		formatFlags |= QTextFormat::Color;
		f->setColor( color );
	    }
	}
    } else { // use the stylesheet tag definition
	if ( ssItem->color().isValid() ) {
	    formatFlags |= QTextFormat::Color;
	    f->setColor( ssItem->color() );
	}
	f->setBold( ssItem->fontWeight() == QFont::Bold );
	f->setItalic( ssItem->fontItalic() );
	f->setUnderline( ssItem->fontUnderline() );
    }
    td->setFormat( 0, f, formatFlags );
    td->removeSelection( 0 );
}

/*! \internal */
void QTextEdit::optimDrawContents( QPainter * p, int clipx, int clipy,
				   int clipw, int cliph )
{
    QFontMetrics fm( QScrollView::font() );
    int startLine = clipy / fm.lineSpacing();

    // we always have to fetch at least two lines for drawing because the
    // painter may be translated so that parts of two lines cover the area
    // of a single line
    int nLines = (cliph / fm.lineSpacing()) + 2;
    int endLine = startLine + nLines;

    if ( startLine >= d->od->numLines )
	return;
    if ( (startLine + nLines) > d->od->numLines )
	nLines = d->od->numLines - startLine;

    int i = 0;
    QString str;
    for ( i = startLine; i < (startLine + nLines); i++ )
	str.append( d->od->lines[ LOGOFFSET(i) ] + "\n" );

    QTextDocument * td = new QTextDocument( 0 );
    td->setDefaultFormat( QScrollView::font(), QColor() );
    td->setPlainText( str );
    td->setFormatter( new QTextFormatterBreakWords ); // deleted by QTextDoc
    td->formatter()->setWrapEnabled( FALSE );
    td->setTabStops(doc->tabStopWidth());

    // get the current text color from the current format
    td->selectAll( QTextDocument::Standard );
    QTextFormat f;
    f.setColor( colorGroup().text() );
    f.setFont( QScrollView::font() );
    td->setFormat( QTextDocument::Standard, &f,
		   QTextFormat::Color | QTextFormat::Font );
    td->removeSelection( QTextDocument::Standard );

    // add tag formatting
    if ( d->od->tags ) {
	int i = startLine;
	QMapConstIterator<int,QTextEditOptimPrivate::Tag *> it;
	QTextEditOptimPrivate::Tag * tag = 0, * tmp = 0;
	QTextCursor cur( td );
	// Step 1 - find previous left-tag
	tmp = optimPreviousLeftTag( i );
	for ( ; i < startLine + nLines; i++ ) {
	    if ( (it = d->od->tagIndex.find( LOGOFFSET(i) )) != d->od->tagIndex.end() )
		tag = it.data();
	    // Step 2 - iterate over tags on the current line
	    int lastIndex = 0;
	    while ( tag && tag->line == i ) {
		tmp = 0;
		if ( tag->prev && !tag->prev->leftTag ) {
		    tmp = tag->prev;
		} else if ( tag->prev && tag->prev->parent ) {
		    tmp = tag->prev->parent;
		}
		if ( (tag->index - lastIndex) > 0 && tmp ) {
		    optimSetTextFormat( td, &cur, &f, lastIndex, tag->index, tmp );
		}
		lastIndex = tag->index;
		tmp = tag;
		tag = tag->next;
	    }
	    // Step 3 - color last part of the line - if necessary
	    if ( tmp && tmp->parent )
		tmp = tmp->parent;
	    if ( (cur.paragraph()->length()-1 - lastIndex) > 0 && tmp && !tmp->leftTag ) {
		optimSetTextFormat( td, &cur, &f, lastIndex,
				    cur.paragraph()->length() - 1, tmp );
	    }
	    cur.setParagraph( cur.paragraph()->next() );
	}
        // useful debug info
	//
//	tag = d->od->tags;
//	qWarning("###");
//	while ( tag ) {
//	    qWarning( "Tag: %p, parent: %09p, leftTag: %09p, Name: %-15s, ParentName: %s, %d%d%d", tag,
//		       tag->parent, tag->leftTag, tag->tag.latin1(), tag->parent ? tag->parent->tag.latin1():"<none>",
//		      tag->bold, tag->italic, tag->underline );
//	    tag = tag->next;
//	}
    }

    // if there is a selection, make sure that the selection in the
    // part we need to redraw is set correctly
    if ( optimHasSelection() ) {
	QTextCursor c1( td );
	QTextCursor c2( td );
	int selStart = d->od->selStart.line;
	int idxStart = d->od->selStart.index;
	int selEnd = d->od->selEnd.line;
	int idxEnd = d->od->selEnd.index;
	if ( selEnd < selStart ) {
	    qSwap( &selStart, &selEnd );
	    qSwap( &idxStart, &idxEnd );
	}
	if ( selEnd > d->od->numLines-1 ) {
	    selEnd = d->od->numLines-1;
	}
	if ( startLine <= selStart && endLine >= selEnd ) {
	    // case 1: area to paint covers entire selection
	    int paragS = selStart - startLine;
	    int paragE = paragS + (selEnd - selStart);
	    QTextParagraph * parag = td->paragAt( paragS );
	    if ( parag ) {
		c1.setParagraph( parag );
		if ( td->text( paragS ).length() >= (uint) idxStart )
		    c1.setIndex( idxStart );
	    }
	    parag = td->paragAt( paragE );
	    if ( parag ) {
		c2.setParagraph( parag );
		if ( td->text( paragE ).length() >= (uint) idxEnd )
		    c2.setIndex( idxEnd );
	    }
	} else if ( startLine > selStart && endLine < selEnd ) {
	    // case 2: area to paint is all part of the selection
	    td->selectAll( QTextDocument::Standard );
	} else if ( startLine > selStart && endLine >= selEnd &&
		    startLine <= selEnd ) {
	    // case 3: area to paint starts inside a selection, ends past it
	    c1.setParagraph( td->firstParagraph() );
	    c1.setIndex( 0 );
	    int paragE = selEnd - startLine;
	    QTextParagraph * parag = td->paragAt( paragE );
	    if ( parag ) {
		c2.setParagraph( parag );
		if ( td->text( paragE ).length() >= (uint) idxEnd )
		    c2.setIndex( idxEnd );
	    }
	} else if ( startLine <= selStart && endLine < selEnd &&
		    endLine > selStart ) {
	    // case 4: area to paint starts before a selection, ends inside it
	    int paragS = selStart - startLine;
	    QTextParagraph * parag = td->paragAt( paragS );
	    if ( parag ) {
		c1.setParagraph( parag );
		c1.setIndex( idxStart );
	    }
	    c2.setParagraph( td->lastParagraph() );
	    c2.setIndex( td->lastParagraph()->string()->toString().length() - 1 );

	}
	// previously selected?
	if ( !td->hasSelection( QTextDocument::Standard ) ) {
	    td->setSelectionStart( QTextDocument::Standard, c1 );
	    td->setSelectionEnd( QTextDocument::Standard, c2 );
	}
    }
    td->doLayout( p, contentsWidth() );

    // have to align the painter so that partly visible lines are
    // drawn at the correct position within the area that needs to be
    // painted
    int offset = clipy % fm.lineSpacing() + 2;
    QRect r( clipx, 0, clipw, cliph + offset );
    p->translate( 0, clipy - offset );
    td->draw( p, r.x(), r.y(), r.width(), r.height(), colorGroup() );
    p->translate( 0, -(clipy - offset) );
    delete td;
}

/*! \internal */
void QTextEdit::optimMousePressEvent( QMouseEvent * e )
{
    if ( e->button() != LeftButton )
	return;

    QFontMetrics fm( QScrollView::font() );
    mousePressed = TRUE;
    mousePos = e->pos();
    d->od->selStart.line = e->y() / fm.lineSpacing();
    if ( d->od->selStart.line > d->od->numLines-1 ) {
	d->od->selStart.line = d->od->numLines-1;
	d->od->selStart.index = d->od->lines[ LOGOFFSET(d->od->numLines-1) ].length();
    } else {
	QString str = d->od->lines[ LOGOFFSET(d->od->selStart.line) ];
	d->od->selStart.index = optimCharIndex( str, mousePos.x() );
    }
    d->od->selEnd.line = d->od->selStart.line;
    d->od->selEnd.index = d->od->selStart.index;
    oldMousePos = e->pos();
    repaintContents( FALSE );
}

/*! \internal */
void QTextEdit::optimMouseReleaseEvent( QMouseEvent * e )
{
    if ( e->button() != LeftButton )
	return;

    if ( scrollTimer->isActive() )
	scrollTimer->stop();
    if ( !inDoubleClick ) {
	QFontMetrics fm( QScrollView::font() );
	d->od->selEnd.line = e->y() / fm.lineSpacing();
	if ( d->od->selEnd.line > d->od->numLines-1 ) {
	    d->od->selEnd.line = d->od->numLines-1;
	}
	QString str = d->od->lines[ LOGOFFSET(d->od->selEnd.line) ];
	mousePos = e->pos();
	d->od->selEnd.index = optimCharIndex( str, mousePos.x() );
	if ( d->od->selEnd.line < d->od->selStart.line ) {
	    qSwap( &d->od->selStart.line, &d->od->selEnd.line );
	    qSwap( &d->od->selStart.index, &d->od->selEnd.index );
	} else if ( d->od->selStart.line == d->od->selEnd.line &&
		    d->od->selStart.index > d->od->selEnd.index ) {
	    qSwap( &d->od->selStart.index, &d->od->selEnd.index );
	}
	oldMousePos = e->pos();
	repaintContents( FALSE );
    }
    if ( mousePressed ) {
	mousePressed = FALSE;
	copyToClipboard();
    }

    inDoubleClick = FALSE;
    emit copyAvailable( optimHasSelection() );
    emit selectionChanged();
}

/*! \internal */
void QTextEdit::optimMouseMoveEvent( QMouseEvent * e )
{
    mousePos = e->pos();
    optimDoAutoScroll();
    oldMousePos = mousePos;
}

/*! \internal */
void QTextEdit::optimDoAutoScroll()
{
    if ( !mousePressed )
	return;

    QFontMetrics fm( QScrollView::font() );
    QPoint pos( mapFromGlobal( QCursor::pos() ) );
    bool doScroll = FALSE;
    int xx = contentsX() + pos.x();
    int yy = contentsY() + pos.y();

    // find out how much we have to scroll in either dir.
    if ( pos.x() < 0 || pos.x() > viewport()->width() ||
	 pos.y() < 0 || pos.y() > viewport()->height() ) {
	int my = yy;
	if ( pos.x() < 0 )
	    xx = contentsX() - fm.width( 'w');
	else if ( pos.x() > viewport()->width() )
	    xx = contentsX() + viewport()->width() + fm.width('w');

	if ( pos.y() < 0 ) {
	    my = contentsY() - 1;
	    yy = (my / fm.lineSpacing()) * fm.lineSpacing() + 1;
	} else if ( pos.y() > viewport()->height() ) {
	    my = contentsY() + viewport()->height() + 1;
	    yy = (my / fm.lineSpacing() + 1) * fm.lineSpacing() - 1;
	}
	d->od->selEnd.line = my / fm.lineSpacing();
	mousePos.setX( xx );
	mousePos.setY( my );
	doScroll = TRUE;
    } else {
	d->od->selEnd.line = mousePos.y() / fm.lineSpacing();
    }

    if ( d->od->selEnd.line < 0 ) {
	d->od->selEnd.line = 0;
    } else if ( d->od->selEnd.line > d->od->numLines-1 ) {
	d->od->selEnd.line = d->od->numLines-1;
    }

    QString str = d->od->lines[ LOGOFFSET(d->od->selEnd.line) ];
    d->od->selEnd.index = optimCharIndex( str, mousePos.x() );

    // have to have a valid index before generating a paint event
    if ( doScroll )
	ensureVisible( xx, yy, 1, 1 );

    // if the text document is smaller than the heigth of the viewport
    // - redraw the whole thing otherwise calculate the rect that
    // needs drawing.
    if ( d->od->numLines * fm.lineSpacing() < viewport()->height() ) {
	repaintContents( contentsX(), contentsY(), width(), height(), FALSE );
    } else {
	int h = QABS(mousePos.y() - oldMousePos.y()) + fm.lineSpacing() * 2;
	int y;
	if ( oldMousePos.y() < mousePos.y() ) {
	    y = oldMousePos.y() - fm.lineSpacing();
	} else {
	    // expand paint area for a fully selected line
	    h += fm.lineSpacing();
	    y = mousePos.y() - fm.lineSpacing()*2;
	}
	if ( y < 0 )
	    y = 0;
	repaintContents( contentsX(), y, width(), h, FALSE );
    }

    if ( !scrollTimer->isActive() && pos.y() < 0 || pos.y() > height() )
	scrollTimer->start( 100, FALSE );
    else if ( scrollTimer->isActive() && pos.y() >= 0 && pos.y() <= height() )
	scrollTimer->stop();
}

/*! \internal

  Returns the index of the character in the string \a str that is
  currently under the mouse pointer.
*/
int QTextEdit::optimCharIndex( const QString &str, int mx ) const
{
    QFontMetrics fm(QScrollView::font());
    uint i = 0;
    int dd, dist = 10000000;
    int curpos = 0;
    int strWidth;
    mx = mx - 4; // ### get the real margin from somewhere

    if (!str.contains('\t') && mx > fm.width(str))
	return str.length();

    while (i < str.length()) {
	strWidth = qStrWidth(str.left(i), tabStopWidth(), fm);
	dd = strWidth - mx;
	if (QABS(dd) <= dist) {
	    dist = QABS(dd);
	    if (mx >= strWidth)
		curpos = i;
	}
	++i;
    }
    return curpos;
}

/*! \internal */
void QTextEdit::optimSelectAll()
{
    d->od->selStart.line = d->od->selStart.index = 0;
    d->od->selEnd.line = d->od->numLines - 1;
    d->od->selEnd.index = d->od->lines[ LOGOFFSET(d->od->selEnd.line) ].length();

    repaintContents( FALSE );
    emit copyAvailable( optimHasSelection() );
    emit selectionChanged();
}

/*! \internal */
void QTextEdit::optimRemoveSelection()
{
    d->od->selStart.line = d->od->selEnd.line = -1;
    d->od->selStart.index = d->od->selEnd.index = -1;
    repaintContents( FALSE );
}

/*! \internal */
void QTextEdit::optimSetSelection( int startLine, int startIdx,
				       int endLine, int endIdx )
{
    d->od->selStart.line = startLine;
    d->od->selEnd.line = endLine;
    d->od->selStart.index = startIdx;
    d->od->selEnd.index = endIdx;
}

/*! \internal */
bool QTextEdit::optimHasSelection() const
{
    if ( d->od->selStart.line != d->od->selEnd.line ||
	 d->od->selStart.index != d->od->selEnd.index )
	return TRUE;
    return FALSE;
}

/*! \internal */
QString QTextEdit::optimSelectedText() const
{
    QString str;

    if ( !optimHasSelection() )
	return str;

    // concatenate all strings
    if ( d->od->selStart.line == d->od->selEnd.line ) {
	str = d->od->lines[ LOGOFFSET(d->od->selEnd.line) ].mid( d->od->selStart.index,
			   d->od->selEnd.index - d->od->selStart.index );
    } else {
	int i = d->od->selStart.line;
	str = d->od->lines[ LOGOFFSET(i) ].right( d->od->lines[ LOGOFFSET(i) ].length() -
				  d->od->selStart.index ) + "\n";
	i++;
	for ( ; i < d->od->selEnd.line; i++ ) {
	    if ( d->od->lines[ LOGOFFSET(i) ].isEmpty() ) // CR lines are empty
		str += "\n";
	    else
		str += d->od->lines[ LOGOFFSET(i) ] + "\n";
	}
	str += d->od->lines[ LOGOFFSET(d->od->selEnd.line) ].left( d->od->selEnd.index );
    }
    return str;
}

/*! \internal */
bool QTextEdit::optimFind( const QString & expr, bool cs, bool /*wo*/,
			       bool fw, int * para, int * index )
{
    bool found = FALSE;
    int parag = para ? *para : d->od->search.line,
	  idx = index ? *index : d->od->search.index, i;

    if ( d->od->len == 0 )
	return FALSE;

    for ( i = parag; fw ? i < d->od->numLines : i >= 0; fw ? i++ : i-- ) {
	idx = fw ? d->od->lines[ LOGOFFSET(i) ].find( expr, idx, cs ) :
	      d->od->lines[ LOGOFFSET(i) ].findRev( expr, idx, cs );
	if ( idx != -1 ) {
	    found = TRUE;
	    break;
	} else if ( fw )
	    idx = 0;
    }

    if ( found ) {
	if ( index )
	    *index = idx;
	if ( para )
	    *para = i;
	d->od->search.index = idx + 1;
	d->od->search.line = i;
	optimSetSelection( i, idx, i, idx + expr.length() );
	QFontMetrics fm( QScrollView::font() );
	int h = fm.lineSpacing();
	int x = fm.width( d->od->lines[ LOGOFFSET(i) ].left( idx + expr.length()) ) + 4;
	ensureVisible( x, i * h + h / 2, 1, h / 2 + 2 );
	repaintContents(); // could possibly be optimized
    }
    return found;
}

/*! \reimp */
void QTextEdit::polish()
{
    // this will ensure that the last line is visible if text have
    // been added to the widget before it is shown
    if ( d->optimMode )
	scrollToBottom();
    QWidget::polish();
}

/*!
    Sets the maximum number of lines a QTextEdit can hold in \c
    LogText mode to \a limit. If \a limit is -1 (the default), this
    signifies an unlimited number of lines.

    \warning Never use formatting tags that span more than one line
    when the maximum log lines is set. When lines are removed from the
    top of the buffer it could result in an unbalanced tag pair, i.e.
    the left formatting tag is removed before the right one.
 */
void QTextEdit::setMaxLogLines( int limit )
{
    d->maxLogLines = limit;
    if ( d->maxLogLines < -1 )
	d->maxLogLines = -1;
    if ( d->maxLogLines == -1 )
	d->logOffset = 0;
}

/*!
    Returns the maximum number of lines QTextEdit can hold in \c
    LogText mode. By default the number of lines is unlimited, which
    is signified by a value of -1.
 */
int QTextEdit::maxLogLines()
{
    return d->maxLogLines;
}

/*!
    Check if the number of lines in the buffer is limited, and uphold
    that limit when appending new lines.
 */
void QTextEdit::optimCheckLimit( const QString& str )
{
    if ( d->maxLogLines > -1 && d->maxLogLines <= d->od->numLines ) {
	// NB! Removing the top line in the buffer will potentially
	// destroy the structure holding the formatting tags - if line
	// spanning tags are used.
	QTextEditOptimPrivate::Tag *t = d->od->tags, *tmp, *itr;
	QPtrList<QTextEditOptimPrivate::Tag> lst;
	while ( t ) {
	    t->line -= 1;
	    // unhook the ptr from the tag structure
	    if ( ((uint) LOGOFFSET(t->line) < (uint) d->logOffset &&
		  (uint) LOGOFFSET(t->line) < (uint) LOGOFFSET(d->od->numLines) &&
		  (uint) LOGOFFSET(d->od->numLines) > (uint) d->logOffset) )
	    {
		if ( t->prev )
		    t->prev->next = t->next;
		if ( t->next )
		    t->next->prev = t->prev;
		if ( d->od->tags == t )
		    d->od->tags = t->next;
		if ( d->od->lastTag == t ) {
		    if ( t->prev )
			d->od->lastTag = t->prev;
		    else
			d->od->lastTag = d->od->tags;
		}
		tmp = t;
		t = t->next;
		lst.append( tmp );
		delete tmp;
	    } else {
		t = t->next;
	    }
	}
	// Remove all references to the ptrs we just deleted
	itr = d->od->tags;
	while ( itr ){
	    for ( tmp = lst.first(); tmp; tmp = lst.next() ) {
		if ( itr->parent == tmp )
		    itr->parent = 0;
		if ( itr->leftTag == tmp )
		    itr->leftTag = 0;
	    }
	    itr = itr->next;
	}
	// ...in the tag index as well
	QMapIterator<int, QTextEditOptimPrivate::Tag *> idx;
	if ( (idx = d->od->tagIndex.find( d->logOffset )) != d->od->tagIndex.end() )
	    d->od->tagIndex.remove( idx );

	QMapIterator<int,QString> it;
	if ( (it = d->od->lines.find( d->logOffset )) != d->od->lines.end() ) {
	    d->od->len -= (*it).length();
	    d->od->lines.remove( it );
	    d->od->numLines--;
	    d->logOffset = LOGOFFSET(1);
	}
    }
    d->od->len += str.length();
    d->od->lines[ LOGOFFSET(d->od->numLines++) ] = str;
}

#endif // QT_TEXTEDIT_OPTIMIZATION

/*!
    \property QTextEdit::autoFormatting
    \brief the enabled set of auto formatting features

    The value can be any combination of the values in the \c
    AutoFormatting enum.  The default is \c AutoAll. Choose \c AutoNone
    to disable all automatic formatting.

    Currently, the only automatic formatting feature provided is \c
    AutoBulletList; future versions of Qt may offer more.
*/

void QTextEdit::setAutoFormatting( uint features )
{
    d->autoFormatting = features;
}

uint QTextEdit::autoFormatting() const
{
    return d->autoFormatting;
}

/*!
    Returns the QSyntaxHighlighter set on this QTextEdit. 0 is
    returned if no syntax highlighter is set.
 */
QSyntaxHighlighter * QTextEdit::syntaxHighlighter() const
{
    if (document()->preProcessor())
	return ((QSyntaxHighlighterInternal *) document()->preProcessor())->highlighter;
    else
	return 0;
}

#endif //QT_NO_TEXTEDIT
