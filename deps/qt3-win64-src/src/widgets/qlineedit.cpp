/**********************************************************************
** $Id: qlineedit.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QLineEdit widget class
**
** Created : 941011
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

#include "qlineedit.h"
#ifndef QT_NO_LINEEDIT
#include "qpainter.h"
#include "qdrawutil.h"
#include "qfontmetrics.h"
#include "qpixmap.h"
#include "qclipboard.h"
#include "qapplication.h"
#include "qvalidator.h"
#include "qdragobject.h"
#include "qtimer.h"
#include "qpopupmenu.h"
#include "qstringlist.h"
#include "qguardedptr.h"
#include "qstyle.h"
#include "qwhatsthis.h"
#include "../kernel/qinternal_p.h"
#include "private/qtextlayout_p.h"
#include "qvaluevector.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

#ifndef QT_NO_ACCEL
#include "qkeysequence.h"
#define ACCEL_KEY(k) "\t" + QString(QKeySequence( Qt::CTRL | Qt::Key_ ## k ))
#else
#define ACCEL_KEY(k) "\t" + QString("Ctrl+" #k)
#endif

#define innerMargin 1

struct QLineEditPrivate : public Qt
{
    QLineEditPrivate( QLineEdit *q )
	: q(q), cursor(0), cursorTimer(0), tripleClickTimer(0), frame(1),
	  cursorVisible(0), separator(0), readOnly(0), modified(0),
	  direction(QChar::DirON), dragEnabled(1), alignment(0),
	  echoMode(0), textDirty(0), selDirty(0), validInput(1),
	  ascent(0), maxLength(32767), menuId(0),
	  hscroll(0), validator(0), maskData(0),
	  undoState(0), selstart(0), selend(0),
	  imstart(0), imend(0), imselstart(0), imselend(0)
#ifndef QT_NO_DRAGANDDROP
	,dndTimer(0)
#endif
	{}
    void init( const QString&);

    QLineEdit *q;
    QString text;
    int cursor;
    int cursorTimer;
    QPoint tripleClick;
    int tripleClickTimer;
    uint frame : 1;
    uint cursorVisible : 1;
    uint separator : 1;
    uint readOnly : 1;
    uint modified : 1;
    uint direction : 5;
    uint dragEnabled : 1;
    uint alignment : 3;
    uint echoMode : 2;
    uint textDirty : 1;
    uint selDirty : 1;
    uint validInput : 1;
    int ascent;
    int maxLength;
    int menuId;
    int hscroll;
    QChar passwordChar; // obsolete

    void finishChange( int validateFromState = -1, bool setModified = TRUE );

    const QValidator* validator;
    struct MaskInputData {
	enum Casemode { NoCaseMode, Upper, Lower };
	QChar maskChar; // either the separator char or the inputmask
	bool separator;
	Casemode caseMode;
    };
    QString inputMask;
    QChar blank;
    MaskInputData *maskData;
    inline int nextMaskBlank( int pos ) {
	int c = findInMask( pos, TRUE, FALSE );
	separator |= ( c != pos );
	return ( c != -1 ?  c : maxLength );
    }
    inline int prevMaskBlank( int pos ) {
	int c = findInMask( pos, FALSE, FALSE );
	separator |= ( c != pos );
	return ( c != -1 ? c : 0 );
    }

    void setCursorVisible( bool visible );


    // undo/redo handling
    enum CommandType { Separator, Insert, Remove, Delete, RemoveSelection, DeleteSelection };
    struct Command {
	inline Command(){}
	inline Command( CommandType type, int pos, QChar c )
	    :type(type),c(c),pos(pos){}
	uint type : 4;
	QChar c;
	int pos;
    };
    int undoState;
    QValueVector<Command> history;
    void addCommand( const Command& cmd );
    void insert( const QString& s );
    void del( bool wasBackspace = FALSE );
    void remove( int pos );

    inline void separate() { separator = TRUE; }
    inline void undo( int until = -1 ) {
	if ( !isUndoAvailable() )
	    return;
	deselect();
	while ( undoState && undoState > until ) {
	    Command& cmd = history[--undoState];
	    switch ( cmd.type ) {
	    case Insert:
		text.remove( cmd.pos, 1);
		cursor = cmd.pos;
		break;
	    case Remove:
	    case RemoveSelection:
		text.insert( cmd.pos, cmd.c );
		cursor = cmd.pos + 1;
		break;
	    case Delete:
	    case DeleteSelection:
		text.insert( cmd.pos, cmd.c );
		cursor = cmd.pos;
		break;
	    case Separator:
		continue;
	    }
	    if ( until < 0 && undoState ) {
		Command& next = history[undoState-1];
		if ( next.type != cmd.type && next.type < RemoveSelection
		     && !( cmd.type >= RemoveSelection && next.type != Separator ) )
		    break;
	    }
	}
	modified = ( undoState != 0 );
	textDirty = TRUE;
    }
    inline void redo() {
	if ( !isRedoAvailable() )
	    return;
	deselect();
	while ( undoState < (int)history.size() ) {
	    Command& cmd = history[undoState++];
	    switch ( cmd.type ) {
	    case Insert:
		text.insert( cmd.pos, cmd.c );
		cursor = cmd.pos + 1;
		break;
	    case Remove:
	    case Delete:
	    case RemoveSelection:
	    case DeleteSelection:
		text.remove( cmd.pos, 1 );
		cursor = cmd.pos;
		break;
	    case Separator:
		continue;
	    }
	    if ( undoState < (int)history.size() ) {
		Command& next = history[undoState];
		if ( next.type != cmd.type && cmd.type < RemoveSelection
		     && !( next.type >= RemoveSelection && cmd.type != Separator ) )
		    break;
	    }
	}
	textDirty = TRUE;
    }
    inline bool isUndoAvailable() const { return !readOnly && undoState; }
    inline bool isRedoAvailable() const { return !readOnly && undoState < (int)history.size(); }

    // bidi
    inline bool isRightToLeft() const { return direction==QChar::DirON?text.isRightToLeft():(direction==QChar::DirR); }

    // selection
    int selstart, selend;
    inline bool allSelected() const { return !text.isEmpty() && selstart == 0 && selend == (int)text.length(); }
    inline bool hasSelectedText() const { return !text.isEmpty() && selend > selstart; }
    inline void deselect() { selDirty |= (selend > selstart); selstart = selend = 0; }
    void removeSelectedText();
#ifndef QT_NO_CLIPBOARD
    void copy( bool clipboard = TRUE ) const;
#endif
    inline bool inSelection( int x ) const
    { if ( selstart >= selend ) return FALSE;
    int pos = xToPos( x, QTextItem::OnCharacters );  return pos >= selstart && pos < selend; }

    // masking
    void parseInputMask( const QString &maskFields );
    bool isValidInput( QChar key, QChar mask ) const;
    QString maskString( uint pos, const QString &str, bool clear = FALSE ) const;
    QString clearString( uint pos, uint len ) const;
    QString stripString( const QString &str ) const;
    int findInMask( int pos, bool forward, bool findSeparator, QChar searchChar = QChar() ) const;

    // input methods
    int imstart, imend, imselstart, imselend;

    // complex text layout
    QTextLayout textLayout;
    void updateTextLayout();
    void moveCursor( int pos, bool mark = FALSE );
    void setText( const QString& txt );
    int xToPos( int x, QTextItem::CursorPosition = QTextItem::BetweenCharacters ) const;
    inline int visualAlignment() const { return alignment ? alignment : int( isRightToLeft() ? AlignRight : AlignLeft ); }
    QRect cursorRect() const;
    void updateMicroFocusHint();

#ifndef QT_NO_DRAGANDDROP
    // drag and drop
    QPoint dndPos;
    int dndTimer;
    bool drag();
#endif
};


/*!
    \class QLineEdit
    \brief The QLineEdit widget is a one-line text editor.

    \ingroup basic
    \mainclass

    A line edit allows the user to enter and edit a single line of
    plain text with a useful collection of editing functions,
    including undo and redo, cut and paste, and drag and drop.

    By changing the echoMode() of a line edit, it can also be used as
    a "write-only" field, for inputs such as passwords.

    The length of the text can be constrained to maxLength(). The text
    can be arbitrarily constrained using a validator() or an
    inputMask(), or both.

    A related class is QTextEdit which allows multi-line, rich-text
    editing.

    You can change the text with setText() or insert(). The text is
    retrieved with text(); the displayed text (which may be different,
    see \l{EchoMode}) is retrieved with displayText(). Text can be
    selected with setSelection() or selectAll(), and the selection can
    be cut(), copy()ied and paste()d. The text can be aligned with
    setAlignment().

    When the text changes the textChanged() signal is emitted; when
    the Return or Enter key is pressed the returnPressed() signal is
    emitted. Note that if there is a validator set on the line edit,
    the returnPressed() signal will only be emitted if the validator
    returns \c Acceptable.

    By default, QLineEdits have a frame as specified by the Windows
    and Motif style guides; you can turn it off by calling
    setFrame(FALSE).

    The default key bindings are described below. The line edit also
    provides a context menu (usually invoked by a right mouse click)
    that presents some of these editing options.
    \target desc
    \table
    \header \i Keypress \i Action
    \row \i Left Arrow \i Moves the cursor one character to the left.
    \row \i Shift+Left Arrow \i Moves and selects text one character to the left.
    \row \i Right Arrow \i Moves the cursor one character to the right.
    \row \i Shift+Right Arrow \i Moves and selects text one character to the right.
    \row \i Home \i Moves the cursor to the beginning of the line.
    \row \i End \i Moves the cursor to the end of the line.
    \row \i Backspace \i Deletes the character to the left of the cursor.
    \row \i Ctrl+Backspace \i Deletes the word to the left of the cursor.
    \row \i Delete \i Deletes the character to the right of the cursor.
    \row \i Ctrl+Delete \i Deletes the word to the right of the cursor.
    \row \i Ctrl+A \i Moves the cursor to the beginning of the line.
    \row \i Ctrl+B \i Moves the cursor one character to the left.
    \row \i Ctrl+C \i Copies the selected text to the clipboard.
		      (Windows also supports Ctrl+Insert for this operation.)
    \row \i Ctrl+D \i Deletes the character to the right of the cursor.
    \row \i Ctrl+E \i Moves the cursor to the end of the line.
    \row \i Ctrl+F \i Moves the cursor one character to the right.
    \row \i Ctrl+H \i Deletes the character to the left of the cursor.
    \row \i Ctrl+K \i Deletes to the end of the line.
    \row \i Ctrl+V \i Pastes the clipboard text into line edit.
		      (Windows also supports Shift+Insert for this operation.)
    \row \i Ctrl+X \i Deletes the selected text and copies it to the clipboard.
		      (Windows also supports Shift+Delete for this operation.)
    \row \i Ctrl+Z \i Undoes the last operation.
    \row \i Ctrl+Y \i Redoes the last undone operation.
    \endtable

    Any other key sequence that represents a valid character, will
    cause the character to be inserted into the line edit.

    <img src=qlined-m.png> <img src=qlined-w.png>

    \sa QTextEdit QLabel QComboBox
	\link guibooks.html#fowler GUI Design Handbook: Field, Entry\endlink
*/


/*!
    \fn void QLineEdit::textChanged( const QString& )

    This signal is emitted whenever the text changes. The argument is
    the new text.
*/

/*!
    \fn void QLineEdit::selectionChanged()

    This signal is emitted whenever the selection changes.

    \sa hasSelectedText(), selectedText()
*/

/*!
    \fn void QLineEdit::lostFocus()

    This signal is emitted when the line edit has lost focus.

    \sa hasFocus(), QWidget::focusInEvent(), QWidget::focusOutEvent()
*/



/*!
    Constructs a line edit with no text.

    The maximum text length is set to 32767 characters.

    The \a parent and \a name arguments are sent to the QWidget constructor.

    \sa setText(), setMaxLength()
*/

QLineEdit::QLineEdit( QWidget* parent, const char* name )
    : QFrame( parent, name, WNoAutoErase ), d(new QLineEditPrivate( this ))
{
    d->init( QString::null );
}

/*!
    Constructs a line edit containing the text \a contents.

    The cursor position is set to the end of the line and the maximum
    text length to 32767 characters.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.

    \sa text(), setMaxLength()
*/

QLineEdit::QLineEdit( const QString& contents, QWidget* parent, const char* name )
    : QFrame( parent, name, WNoAutoErase ), d(new QLineEditPrivate( this ))
{
    d->init( contents );
}

/*!
    Constructs a line edit with an input \a inputMask and the text \a
    contents.

    The cursor position is set to the end of the line and the maximum
    text length is set to the length of the mask (the number of mask
    characters and separators).

    The \a parent and \a name arguments are sent to the QWidget
    constructor.

    \sa setMask() text()
*/
QLineEdit::QLineEdit( const QString& contents, const QString &inputMask, QWidget* parent, const char* name )
    : QFrame( parent, name, WNoAutoErase ), d(new QLineEditPrivate( this ))
{
    d->parseInputMask( inputMask );
    if ( d->maskData ) {
	QString ms = d->maskString( 0, contents );
	d->init( ms + d->clearString( ms.length(), d->maxLength - ms.length() ) );
	d->cursor = d->nextMaskBlank( ms.length() );
    } else {
	d->init( contents );
    }
}

/*!
    Destroys the line edit.
*/

QLineEdit::~QLineEdit()
{
    delete [] d->maskData;
    delete d;
}


/*!
    \property QLineEdit::text
    \brief the line edit's text

    Note that setting this property clears the selection, clears the
    undo/redo history, moves the cursor to the end of the line and
    resets the \c modified property to FALSE. The text is not
    validated when inserted with setText().

    The text is truncated to maxLength() length.

    \sa insert()
*/
QString QLineEdit::text() const
{
    QString res = d->text;
    if ( d->maskData )
	res = d->stripString( d->text );
    return ( res.isNull() ? QString::fromLatin1("") : res );
}

void QLineEdit::setText( const QString& text)
{
    resetInputContext();
    d->setText( text );
    d->modified = FALSE;
    d->finishChange( -1, FALSE );
}


/*!
    \property QLineEdit::displayText
    \brief the displayed text

    If \c EchoMode is \c Normal this returns the same as text(); if
    \c EchoMode is \c Password it returns a string of asterisks
    text().length() characters long, e.g. "******"; if \c EchoMode is
    \c NoEcho returns an empty string, "".

    \sa setEchoMode() text() EchoMode
*/

QString QLineEdit::displayText() const
{
    if ( d->echoMode == NoEcho )
	return QString::fromLatin1("");
    QString res = d->text;
    if ( d->echoMode == Password )
	res.fill( passwordChar() );
    return ( res.isNull() ? QString::fromLatin1("") : res );
}


/*!
    \property QLineEdit::maxLength
    \brief the maximum permitted length of the text

    If the text is too long, it is truncated at the limit.

    If truncation occurs any selected text will be unselected, the
    cursor position is set to 0 and the first part of the string is
    shown.

    If the line edit has an input mask, the mask defines the maximum
    string length.

    \sa inputMask
*/

int QLineEdit::maxLength() const
{
    return d->maxLength;
}

void QLineEdit::setMaxLength( int maxLength )
{
    if ( d->maskData )
	return;
    d->maxLength = maxLength;
    setText( d->text );
}



/*!
    \property QLineEdit::frame
    \brief whether the line edit draws itself with a frame

    If enabled (the default) the line edit draws itself inside a
    two-pixel frame, otherwise the line edit draws itself without any
    frame.
*/
bool QLineEdit::frame() const
{
    return frameShape() != NoFrame;
}


void QLineEdit::setFrame( bool enable )
{
    setFrameStyle( enable ? ( LineEditPanel | Sunken ) : NoFrame  );
}


/*!
    \enum QLineEdit::EchoMode

    This enum type describes how a line edit should display its
    contents.

    \value Normal   Display characters as they are entered. This is the
		    default.
    \value NoEcho   Do not display anything. This may be appropriate
		    for passwords where even the length of the
		    password should be kept secret.
    \value Password  Display asterisks instead of the characters
		    actually entered.

    \sa setEchoMode() echoMode()
*/


/*!
    \property QLineEdit::echoMode
    \brief the line edit's echo mode

    The initial setting is \c Normal, but QLineEdit also supports \c
    NoEcho and \c Password modes.

    The widget's display and the ability to copy or drag the text is
    affected by this setting.

    \sa EchoMode displayText()
*/

QLineEdit::EchoMode QLineEdit::echoMode() const
{
    return (EchoMode) d->echoMode;
}

void QLineEdit::setEchoMode( EchoMode mode )
{
    if (mode == (EchoMode)d->echoMode)
	return;
    d->echoMode = mode;
    d->updateTextLayout();
    update();
}



/*!
    Returns a pointer to the current input validator, or 0 if no
    validator has been set.

    \sa setValidator()
*/

const QValidator * QLineEdit::validator() const
{
    return d->validator;
}

/*!
    Sets this line edit to only accept input that the validator, \a v,
    will accept. This allows you to place any arbitrary constraints on
    the text which may be entered.

    If \a v == 0, setValidator() removes the current input validator.
    The initial setting is to have no input validator (i.e. any input
    is accepted up to maxLength()).

    \sa validator() QIntValidator QDoubleValidator QRegExpValidator
*/

void QLineEdit::setValidator( const QValidator *v )
{
    if ( d->validator )
	disconnect( (QObject*)d->validator, SIGNAL( destroyed() ),
		    this, SLOT( clearValidator() ) );
    d->validator = v;
    if ( d->validator )
	connect( (QObject*)d->validator, SIGNAL( destroyed() ),
	         this, SLOT( clearValidator() ) );
}



/*!
    Returns a recommended size for the widget.

    The width returned, in pixels, is usually enough for about 15 to
    20 characters.
*/

QSize QLineEdit::sizeHint() const
{
    constPolish();
    QFontMetrics fm( font() );
    int h = QMAX(fm.lineSpacing(), 14) + 2*innerMargin;
    int w = fm.width( 'x' ) * 17; // "some"
    int m = frameWidth() * 2;
    return (style().sizeFromContents(QStyle::CT_LineEdit, this,
				     QSize( w + m, h + m ).
				     expandedTo(QApplication::globalStrut())));
}


/*!
    Returns a minimum size for the line edit.

    The width returned is enough for at least one character.
*/

QSize QLineEdit::minimumSizeHint() const
{
    constPolish();
    QFontMetrics fm = fontMetrics();
    int h = fm.height() + QMAX( 2*innerMargin, fm.leading() );
    int w = fm.maxWidth();
    int m = frameWidth() * 2;
    return QSize( w + m, h + m );
}


/*!
    \property QLineEdit::cursorPosition
    \brief the current cursor position for this line edit

    Setting the cursor position causes a repaint when appropriate.
*/

int QLineEdit::cursorPosition() const
{
    return d->cursor;
}


void QLineEdit::setCursorPosition( int pos )
{
    if (pos < 0)
        pos = 0;

    if ( pos <= (int) d->text.length() )
	d->moveCursor( pos );
}


/*! \obsolete Use setText(), setCursorPosition() and setSelection() instead.
*/

bool QLineEdit::validateAndSet( const QString &newText, int newPos,
				 int newMarkAnchor, int newMarkDrag )
{
    int priorState = d->undoState;
    d->selstart = 0;
    d->selend = d->text.length();
    d->removeSelectedText();
    d->insert( newText );
    d->finishChange( priorState );
    if ( d->undoState > priorState ) {
	d->cursor = newPos;
	d->selstart = QMIN( newMarkAnchor, newMarkDrag );
	d->selend = QMAX( newMarkAnchor, newMarkDrag );
	d->updateMicroFocusHint();
	update();
	return TRUE;
    }
    return FALSE;
}


/*!
    \property QLineEdit::alignment
    \brief the alignment of the line edit

    Possible Values are \c Qt::AlignAuto, \c Qt::AlignLeft, \c
    Qt::AlignRight and \c Qt::AlignHCenter.

    Attempting to set the alignment to an illegal flag combination
    does nothing.

    \sa Qt::AlignmentFlags
*/

int QLineEdit::alignment() const
{
    return d->alignment;
}

void QLineEdit::setAlignment( int flag )
{
    d->alignment = flag & 0x7;
    update();
}


/*!
  \obsolete
  \fn void QLineEdit::cursorRight( bool, int )

  Use cursorForward() instead.

  \sa cursorForward()
*/

/*!
  \obsolete
  \fn void QLineEdit::cursorLeft( bool, int )
  For compatibilty with older applications only. Use cursorBackward()
  instead.
  \sa cursorBackward()
*/

/*!
    Moves the cursor forward \a steps characters. If \a mark is TRUE
    each character moved over is added to the selection; if \a mark is
    FALSE the selection is cleared.

    \sa cursorBackward()
*/

void QLineEdit::cursorForward( bool mark, int steps )
{
    int cursor = d->cursor;
    if ( steps > 0 ) {
	while( steps-- )
	    cursor = d->textLayout.nextCursorPosition( cursor );
    } else if ( steps < 0 ) {
	while ( steps++ )
	    cursor = d->textLayout.previousCursorPosition( cursor );
    }
    d->moveCursor( cursor, mark );
}


/*!
    Moves the cursor back \a steps characters. If \a mark is TRUE each
    character moved over is added to the selection; if \a mark is
    FALSE the selection is cleared.

    \sa cursorForward()
*/
void QLineEdit::cursorBackward( bool mark, int steps )
{
    cursorForward( mark, -steps );
}

/*!
    Moves the cursor one word forward. If \a mark is TRUE, the word is
    also selected.

    \sa cursorWordBackward()
*/
void QLineEdit::cursorWordForward( bool mark )
{
    d->moveCursor( d->textLayout.nextCursorPosition(d->cursor, QTextLayout::SkipWords), mark );
}

/*!
    Moves the cursor one word backward. If \a mark is TRUE, the word
    is also selected.

    \sa cursorWordForward()
*/

void QLineEdit::cursorWordBackward( bool mark )
{
    d->moveCursor( d->textLayout.previousCursorPosition(d->cursor, QTextLayout::SkipWords), mark );
}


/*!
    If no text is selected, deletes the character to the left of the
    text cursor and moves the cursor one position to the left. If any
    text is selected, the cursor is moved to the beginning of the
    selected text and the selected text is deleted.

    \sa del()
*/
void QLineEdit::backspace()
{
    int priorState = d->undoState;
    if ( d->hasSelectedText() ) {
	d->removeSelectedText();
    } else if ( d->cursor ) {
	    --d->cursor;
	    if ( d->maskData )
		d->cursor = d->prevMaskBlank( d->cursor );
	    d->del( TRUE );
    }
    d->finishChange( priorState );
}

/*!
    If no text is selected, deletes the character to the right of the
    text cursor. If any text is selected, the cursor is moved to the
    beginning of the selected text and the selected text is deleted.

    \sa backspace()
*/

void QLineEdit::del()
{
    int priorState = d->undoState;
    if ( d->hasSelectedText() ) {
	d->removeSelectedText();
    } else {
	int n = d->textLayout.nextCursorPosition( d->cursor ) - d->cursor;
	while ( n-- )
	    d->del();
    }
    d->finishChange( priorState );
}

/*!
    Moves the text cursor to the beginning of the line unless it is
    already there. If \a mark is TRUE, text is selected towards the
    first position; otherwise, any selected text is unselected if the
    cursor is moved.

    \sa end()
*/

void QLineEdit::home( bool mark )
{
    d->moveCursor( 0, mark );
}

/*!
    Moves the text cursor to the end of the line unless it is already
    there. If \a mark is TRUE, text is selected towards the last
    position; otherwise, any selected text is unselected if the cursor
    is moved.

    \sa home()
*/

void QLineEdit::end( bool mark )
{
    d->moveCursor( d->text.length(), mark );
}


/*!
    \property QLineEdit::modified
    \brief whether the line edit's contents has been modified by the user

    The modified flag is never read by QLineEdit; it has a default value
    of FALSE and is changed to TRUE whenever the user changes the line
    edit's contents.

    This is useful for things that need to provide a default value but
    do not start out knowing what the default should be (perhaps it
    depends on other fields on the form). Start the line edit without
    the best default, and when the default is known, if modified()
    returns FALSE (the user hasn't entered any text), insert the
    default value.

    Calling clearModified() or setText() resets the modified flag to
    FALSE.
*/

bool QLineEdit::isModified() const
{
    return d->modified;
}

/*!
    Resets the modified flag to FALSE.

    \sa isModified()
*/
void QLineEdit::clearModified()
{
    d->modified = FALSE;
    d->history.clear();
    d->undoState = 0;
}

/*!
  \obsolete
  \property QLineEdit::edited
  \brief whether the line edit has been edited. Use modified instead.
*/
bool QLineEdit::edited() const { return d->modified; }
void QLineEdit::setEdited( bool on ) { d->modified = on; }

/*!
    \obsolete
    \property QLineEdit::hasMarkedText
    \brief whether part of the text has been selected by the user. Use hasSelectedText instead.
*/

/*!
    \property QLineEdit::hasSelectedText
    \brief whether there is any text selected

    hasSelectedText() returns TRUE if some or all of the text has been
    selected by the user; otherwise returns FALSE.

    \sa selectedText()
*/


bool QLineEdit::hasSelectedText() const
{
    return d->hasSelectedText();
}

/*!
  \obsolete
  \property QLineEdit::markedText
  \brief the text selected by the user. Use selectedText instead.
*/

/*!
    \property QLineEdit::selectedText
    \brief the selected text

    If there is no selected text this property's value is
    QString::null.

    \sa hasSelectedText()
*/

QString QLineEdit::selectedText() const
{
    if ( d->hasSelectedText() )
	return d->text.mid( d->selstart, d->selend - d->selstart );
    return QString::null;
}

/*!
    selectionStart() returns the index of the first selected character in the
    line edit or -1 if no text is selected.

    \sa selectedText()
*/

int QLineEdit::selectionStart() const
{
    return d->hasSelectedText() ? d->selstart : -1;
}

/*! \obsolete use selectedText(), selectionStart() */
bool QLineEdit::getSelection( int *start, int *end )
{
    if ( d->hasSelectedText() && start && end ) {
	*start = d->selstart;
	*end = d->selend;
	return TRUE;
    }
    return FALSE;
}


/*!
    Selects text from position \a start and for \a length characters.

    Note that this function sets the cursor's position to the end of
    the selection regardless of its current position.

    \sa deselect() selectAll() getSelection() cursorForward() cursorBackward()
*/

void QLineEdit::setSelection( int start, int length )
{
    if ( start < 0 || start > (int)d->text.length() || length < 0 ) {
	d->selstart = d->selend = 0;
    } else {
	d->selstart = start;
	d->selend = QMIN( start + length, (int)d->text.length() );
	d->cursor = d->selend;
    }
    update();
}


/*!
    \property QLineEdit::undoAvailable
    \brief whether undo is available
*/

bool QLineEdit::isUndoAvailable() const
{
    return d->isUndoAvailable();
}

/*!
    \property QLineEdit::redoAvailable
    \brief whether redo is available
*/

bool QLineEdit::isRedoAvailable() const
{
    return d->isRedoAvailable();
}

/*!
    \property QLineEdit::dragEnabled
    \brief whether the lineedit starts a drag if the user presses and
    moves the mouse on some selected text
*/

bool QLineEdit::dragEnabled() const
{
    return d->dragEnabled;
}

void QLineEdit::setDragEnabled( bool b )
{
    d->dragEnabled = b;
}

/*!
    \property QLineEdit::acceptableInput
    \brief whether the input satisfies the inputMask and the
    validator.

    \sa setInputMask(), setValidator()
*/
bool QLineEdit::hasAcceptableInput() const
{
#ifndef QT_NO_VALIDATOR
    QString text = d->text;
    int cursor = d->cursor;
    if ( d->validator && d->validator->validate( text, cursor ) != QValidator::Acceptable )
	return FALSE;
#endif

    if ( !d->maskData )
	return TRUE;

    if ( d->text.length() != (uint)d->maxLength )
 	return FALSE;

    for ( uint i=0; i < (uint)d->maxLength; i++) {
	if ( d->maskData[i].separator ) {
	    if ( d->text[(int)i] != d->maskData[i].maskChar )
		return FALSE;
	} else {
	    if ( !d->isValidInput( d->text[(int)i], d->maskData[i].maskChar ) )
		return FALSE;
	}
    }
    return TRUE;
}


/*!
    \property QLineEdit::inputMask
    \brief The validation input mask

    If no mask is set, inputMask() returns QString::null.

    Sets the QLineEdit's validation mask. Validators can be used
    instead of, or in conjunction with masks; see setValidator().

    Unset the mask and return to normal QLineEdit operation by passing
    an empty string ("") or just calling setInputMask() with no
    arguments.

    The mask format understands these mask characters:
    \table
    \header \i Character \i Meaning
    \row \i \c A \i ASCII alphabetic character required. A-Z, a-z.
    \row \i \c a \i ASCII alphabetic character permitted but not required.
    \row \i \c N \i ASCII alphanumeric character required. A-Z, a-z, 0-9.
    \row \i \c n \i ASCII alphanumeric character permitted but not required.
    \row \i \c X \i Any character required.
    \row \i \c x \i Any character permitted but not required.
    \row \i \c 9 \i ASCII digit required. 0-9.
    \row \i \c 0 \i ASCII digit permitted but not required.
    \row \i \c D \i ASCII digit required. 1-9.
    \row \i \c d \i ASCII digit permitted but not required (1-9).
    \row \i \c # \i ASCII digit or plus/minus sign permitted but not required.
    \row \i \c > \i All following alphabetic characters are uppercased.
    \row \i \c < \i All following alphabetic characters are lowercased.
    \row \i \c ! \i Switch off case conversion.
    \row \i <tt>\\</tt> \i Use <tt>\\</tt> to escape the special
			   characters listed above to use them as
			   separators.
    \endtable

    The mask consists of a string of mask characters and separators,
    optionally followed by a semi-colon and the character used for
    blanks: the blank characters are always removed from the text
    after editing. The default blank character is space.

    Examples:
    \table
    \header \i Mask \i Notes
    \row \i \c 000.000.000.000;_ \i IP address; blanks are \c{_}.
    \row \i \c 0000-00-00 \i ISO Date; blanks are \c space
    \row \i \c >AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;# \i License number;
    blanks are \c - and all (alphabetic) characters are converted to
    uppercase.
    \endtable

    To get range control (e.g. for an IP address) use masks together
    with \link setValidator() validators\endlink.

    \sa maxLength
*/
QString QLineEdit::inputMask() const
{
    return ( d->maskData ? d->inputMask + ';' + d->blank : QString::null );
}

void QLineEdit::setInputMask( const QString &inputMask )
{
    d->parseInputMask( inputMask );
    if ( d->maskData )
	d->moveCursor( d->nextMaskBlank( 0 ) );
}

/*!
    Selects all the text (i.e. highlights it) and moves the cursor to
    the end. This is useful when a default value has been inserted
    because if the user types before clicking on the widget, the
    selected text will be deleted.

    \sa setSelection() deselect()
*/

void QLineEdit::selectAll()
{
    d->selstart = d->selend = d->cursor = 0;
    d->moveCursor( d->text.length(), TRUE );
}

/*!
    Deselects any selected text.

    \sa setSelection() selectAll()
*/

void QLineEdit::deselect()
{
    d->deselect();
    d->finishChange();
}


/*!
    This slot is equivalent to setValidator(0).
*/

void QLineEdit::clearValidator()
{
    setValidator( 0 );
}

/*!
    Deletes any selected text, inserts \a newText, and validates the
    result. If it is valid, it sets it as the new contents of the line
    edit.
*/
void QLineEdit::insert( const QString &newText )
{
//     q->resetInputContext(); //#### FIX ME IN QT
    int priorState = d->undoState;
    d->removeSelectedText();
    d->insert( newText );
    d->finishChange( priorState );
}

/*!
    Clears the contents of the line edit.
*/
void QLineEdit::clear()
{
    int priorState = d->undoState;
    resetInputContext();
    d->selstart = 0;
    d->selend = d->text.length();
    d->removeSelectedText();
    d->separate();
    d->finishChange( priorState );
}

/*!
    Undoes the last operation if undo is \link
    QLineEdit::undoAvailable available\endlink. Deselects any current
    selection, and updates the selection start to the current cursor
    position.
*/
void QLineEdit::undo()
{
    resetInputContext();
    d->undo();
    d->finishChange( -1, FALSE );
}

/*!
    Redoes the last operation if redo is \link
    QLineEdit::redoAvailable available\endlink.
*/
void QLineEdit::redo()
{
    resetInputContext();
    d->redo();
    d->finishChange();
}


/*!
    \property QLineEdit::readOnly
    \brief whether the line edit is read only.

    In read-only mode, the user can still copy the text to the
    clipboard or drag-and-drop the text (if echoMode() is \c Normal),
    but cannot edit it.

    QLineEdit does not show a cursor in read-only mode.

    \sa setEnabled()
*/

bool QLineEdit::isReadOnly() const
{
    return d->readOnly;
}

void QLineEdit::setReadOnly( bool enable )
{
    d->readOnly = enable;
#ifndef QT_NO_CURSOR
    setCursor( enable ? arrowCursor : ibeamCursor );
#endif
    update();
}


#ifndef QT_NO_CLIPBOARD
/*!
    Copies the selected text to the clipboard and deletes it, if there
    is any, and if echoMode() is \c Normal.

    If the current validator disallows deleting the selected text,
    cut() will copy without deleting.

    \sa copy() paste() setValidator()
*/

void QLineEdit::cut()
{
    if ( hasSelectedText() ) {
	copy();
	del();
    }
}


/*!
    Copies the selected text to the clipboard, if there is any, and if
    echoMode() is \c Normal.

    \sa cut() paste()
*/

void QLineEdit::copy() const
{
    d->copy();
}

/*!
    Inserts the clipboard's text at the cursor position, deleting any
    selected text, providing the line edit is not \link
    QLineEdit::readOnly read-only\endlink.

    If the end result would not be acceptable to the current
    \link setValidator() validator\endlink, nothing happens.

    \sa copy() cut()
*/

void QLineEdit::paste()
{
    insert( QApplication::clipboard()->text( QClipboard::Clipboard ) );
}

void QLineEditPrivate::copy( bool clipboard ) const
{
    QString t = q->selectedText();
    if ( !t.isEmpty() && echoMode == QLineEdit::Normal ) {
	q->disconnect( QApplication::clipboard(), SIGNAL(selectionChanged()), q, 0);
	QApplication::clipboard()->setText( t, clipboard ? QClipboard::Clipboard : QClipboard::Selection );
	q->connect( QApplication::clipboard(), SIGNAL(selectionChanged()),
		 q, SLOT(clipboardChanged()) );
    }
}

#endif // !QT_NO_CLIPBOARD

/*!\reimp
*/

void QLineEdit::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
}

/*! \reimp
*/
bool QLineEdit::event( QEvent * e )
{
    if ( e->type() == QEvent::AccelOverride && !d->readOnly ) {
	QKeyEvent* ke = (QKeyEvent*) e;
	if ( ke->state() == NoButton || ke->state() == ShiftButton
	     || ke->state() == Keypad ) {
	    if ( ke->key() < Key_Escape ) {
		ke->accept();
	    } else {
		switch ( ke->key() ) {
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
	} else if ( ke->state() & ControlButton ) {
	    switch ( ke->key() ) {
// Those are too frequently used for application functionality
/*	    case Key_A:
	    case Key_B:
	    case Key_D:
	    case Key_E:
	    case Key_F:
	    case Key_H:
	    case Key_K:
*/
	    case Key_C:
	    case Key_V:
	    case Key_X:
	    case Key_Y:
	    case Key_Z:
	    case Key_Left:
	    case Key_Right:
#if defined (Q_WS_WIN)
	    case Key_Insert:
	    case Key_Delete:
#endif
		ke->accept();
	    default:
		break;
	    }
	}
    } else if ( e->type() == QEvent::Timer ) {
	// should be timerEvent, is here for binary compatibility
	int timerId = ((QTimerEvent*)e)->timerId();
	if ( timerId == d->cursorTimer ) {
	    if(!hasSelectedText() || style().styleHint( QStyle::SH_BlinkCursorWhenTextSelected ))
		d->setCursorVisible( !d->cursorVisible );
#ifndef QT_NO_DRAGANDDROP
	} else if ( timerId == d->dndTimer ) {
	    if( !d->drag() )
                return TRUE;
#endif
	} else if ( timerId == d->tripleClickTimer ) {
	    killTimer( d->tripleClickTimer );
	    d->tripleClickTimer = 0;
	}
    }
    return QWidget::event( e );
}

/*! \reimp
*/
void QLineEdit::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == RightButton )
	return;
    if ( d->tripleClickTimer && ( e->pos() - d->tripleClick ).manhattanLength() <
	 QApplication::startDragDistance() ) {
	selectAll();
	return;
    }
    bool mark = e->state() & ShiftButton;
    int cursor = d->xToPos( e->pos().x() );
#ifndef QT_NO_DRAGANDDROP
    if ( !mark && d->dragEnabled && d->echoMode == Normal &&
	 e->button() == LeftButton && d->inSelection( e->pos().x() ) ) {
	d->cursor = cursor;
	d->updateMicroFocusHint();
	update();
	d->dndPos = e->pos();
	if ( !d->dndTimer )
	    d->dndTimer = startTimer( QApplication::startDragTime() );
    } else
#endif
    {
	d->moveCursor( cursor, mark );
    }
}

/*! \reimp
*/
void QLineEdit::mouseMoveEvent( QMouseEvent * e )
{

#ifndef QT_NO_CURSOR
    if ( ( e->state() & MouseButtonMask ) == 0 ) {
	if ( !d->readOnly && d->dragEnabled
#ifndef QT_NO_WHATSTHIS
	     && !QWhatsThis::inWhatsThisMode()
#endif
	    )
	    setCursor( ( d->inSelection( e->pos().x() ) ? arrowCursor : ibeamCursor ) );
    }
#endif

    if ( e->state() & LeftButton ) {
#ifndef QT_NO_DRAGANDDROP
	if ( d->dndTimer ) {
	    if ( ( d->dndPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() )
		d->drag();
	} else
#endif
	{
	    d->moveCursor( d->xToPos( e->pos().x() ), TRUE );
	}
    }
}

/*! \reimp
*/
void QLineEdit::mouseReleaseEvent( QMouseEvent* e )
{
#ifndef QT_NO_DRAGANDDROP
    if ( e->button() == LeftButton ) {
	if ( d->dndTimer ) {
	    killTimer( d->dndTimer );
	    d->dndTimer = 0;
	    deselect();
	    return;
	}
    }
#endif
#ifndef QT_NO_CLIPBOARD
    if (QApplication::clipboard()->supportsSelection() ) {
	if ( e->button() == LeftButton ) {
	    d->copy( FALSE );
	} else if ( !d->readOnly && e->button() == MidButton ) {
	    d->deselect();
	    insert( QApplication::clipboard()->text( QClipboard::Selection ) );
	}
    }
#endif
}

/*! \reimp
*/
void QLineEdit::mouseDoubleClickEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton ) {
	deselect();
	d->cursor = d->xToPos( e->pos().x() );
	d->cursor = d->textLayout.previousCursorPosition( d->cursor, QTextLayout::SkipWords );
	// ## text layout should support end of words.
	int end = d->textLayout.nextCursorPosition( d->cursor, QTextLayout::SkipWords );
	while ( end > d->cursor && d->text[end-1].isSpace() )
	    --end;
	d->moveCursor( end, TRUE );
	d->tripleClickTimer = startTimer( QApplication::doubleClickInterval() );
	d->tripleClick = e->pos();
    }
}

/*!
    \fn void  QLineEdit::returnPressed()

    This signal is emitted when the Return or Enter key is pressed.
    Note that if there is a validator() or inputMask() set on the line
    edit, the returnPressed() signal will only be emitted if the input
    follows the inputMask() and the validator() returns \c Acceptable.
*/

/*!
    Converts key press event \a e into a line edit action.

    If Return or Enter is pressed and the current text is valid (or
    can be \link QValidator::fixup() made valid\endlink by the
    validator), the signal returnPressed() is emitted.

    The default key bindings are listed in the \link #desc detailed
    description.\endlink
*/

void QLineEdit::keyPressEvent( QKeyEvent * e )
{
    d->setCursorVisible( TRUE );
    if ( e->key() == Key_Enter || e->key() == Key_Return ) {
	const QValidator * v = d->validator;
	if ( hasAcceptableInput() ) {
	    emit returnPressed();
	}
#ifndef QT_NO_VALIDATOR
	else if ( v && v->validate( d->text, d->cursor ) != QValidator::Acceptable ) {
	    QString vstr = d->text;
	    v->fixup( vstr );
	    if ( vstr != d->text ) {
		setText( vstr );
		if ( hasAcceptableInput() )
		    emit returnPressed();
	    }
	}
#endif
	e->ignore();
	return;
    }
    if ( !d->readOnly ) {
	QString t = e->text();
	if ( !t.isEmpty() && (!e->ascii() || e->ascii()>=32) &&
	     e->key() != Key_Delete &&
	     e->key() != Key_Backspace ) {
#ifdef Q_WS_X11
	    extern bool qt_hebrew_keyboard_hack;
	    if ( qt_hebrew_keyboard_hack ) {
		// the X11 keyboard layout is broken and does not reverse
		// braces correctly. This is a hack to get halfway correct
		// behaviour
		if ( d->isRightToLeft() ) {
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
	    insert( t );
	    return;
	}
    }
    bool unknown = FALSE;
    if ( e->state() & ControlButton ) {
	switch ( e->key() ) {
	case Key_A:
#if defined(Q_WS_X11)
	    home( e->state() & ShiftButton );
#else
	    selectAll();
#endif
	    break;
	case Key_B:
	    cursorForward( e->state() & ShiftButton, -1 );
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_C:
	    copy();
	    break;
#endif
	case Key_D:
	    if ( !d->readOnly ) {
		del();
	    }
	    break;
	case Key_E:
	    end( e->state() & ShiftButton );
	    break;
	case Key_F:
	    cursorForward( e->state() & ShiftButton, 1 );
	    break;
	case Key_H:
	    if ( !d->readOnly ) {
		backspace();
	    }
	    break;
	case Key_K:
	    if ( !d->readOnly ) {
		int priorState = d->undoState;
		d->deselect();
		while ( d->cursor < (int) d->text.length() )
		    d->del();
		d->finishChange( priorState );
	    }
	    break;
#if defined(Q_WS_X11)
        case Key_U:
	    if ( !d->readOnly )
		clear();
	    break;
#endif
#ifndef QT_NO_CLIPBOARD
	case Key_V:
	    if ( !d->readOnly )
		paste();
	    break;
	case Key_X:
	    if ( !d->readOnly && d->hasSelectedText() && echoMode() == Normal ) {
		copy();
		del();
	    }
	    break;
#if defined (Q_WS_WIN)
	case Key_Insert:
	    copy();
	    break;
#endif
#endif
	case Key_Delete:
	    if ( !d->readOnly ) {
		cursorWordForward( TRUE );
		del();
	    }
	    break;
	case Key_Backspace:
	    if ( !d->readOnly ) {
		cursorWordBackward( TRUE );
		del();
	    }
	    break;
	case Key_Right:
	case Key_Left:
	    if ( d->isRightToLeft() == (e->key() == Key_Right) ) {
	        if ( echoMode() == Normal )
		    cursorWordBackward( e->state() & ShiftButton );
		else
		    home( e->state() & ShiftButton );
	    } else {
		if ( echoMode() == Normal )
		    cursorWordForward( e->state() & ShiftButton );
		else
		    end( e->state() & ShiftButton );
	    }
	    break;
	case Key_Z:
	    if ( !d->readOnly ) {
		if(e->state() & ShiftButton)
		    redo();
		else
		    undo();
	    }
	    break;
	case Key_Y:
	    if ( !d->readOnly )
		redo();
	    break;
	default:
	    unknown = TRUE;
	}
    } else { // ### check for *no* modifier
	switch ( e->key() ) {
	case Key_Shift:
	    // ### TODO
	    break;
	case Key_Left:
	case Key_Right: {
	    int step =  (d->isRightToLeft() == (e->key() == Key_Right)) ? -1 : 1;
	    cursorForward( e->state() & ShiftButton, step );
	}
	break;
	case Key_Backspace:
	    if ( !d->readOnly ) {
		backspace();
	    }
	    break;
	case Key_Home:
#ifdef Q_WS_MACX
	case Key_Up:
#endif
	    home( e->state() & ShiftButton );
	    break;
	case Key_End:
#ifdef Q_WS_MACX
	case Key_Down:
#endif
	    end( e->state() & ShiftButton );
	    break;
	case Key_Delete:
	    if ( !d->readOnly ) {
#if defined (Q_WS_WIN)
		if ( e->state() & ShiftButton ) {
		    cut();
		    break;
		}
#endif
		del();
	    }
	    break;
#if defined (Q_WS_WIN)
	case Key_Insert:
	    if ( !d->readOnly && e->state() & ShiftButton )
		paste();
	    else
		unknown = TRUE;
	    break;
#endif
	case Key_F14: // Undo key on Sun keyboards
	    if ( !d->readOnly )
		undo();
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_F16: // Copy key on Sun keyboards
	    copy();
	    break;
	case Key_F18: // Paste key on Sun keyboards
	    if ( !d->readOnly )
		paste();
	    break;
	case Key_F20: // Cut key on Sun keyboards
	    if ( !d->readOnly && hasSelectedText() && echoMode() == Normal ) {
		copy();
		del();
	    }
	    break;
#endif
	default:
	    unknown = TRUE;
	}
    }
    if ( e->key() == Key_Direction_L || e->key() == Key_Direction_R ) {
	d->direction = (e->key() == Key_Direction_L) ? QChar::DirL : QChar::DirR;
        d->updateTextLayout();
	update();
    }

    if ( unknown )
	e->ignore();
}

/*! \reimp
 */
void QLineEdit::imStartEvent( QIMEvent *e )
{
    if ( d->readOnly ) {
	e->ignore();
	return;
    }
    d->removeSelectedText();
    d->updateMicroFocusHint();
    d->imstart = d->imend = d->imselstart = d->imselend = d->cursor;
}

/*! \reimp
 */
void QLineEdit::imComposeEvent( QIMEvent *e )
{
    if ( d->readOnly ) {
	e->ignore();
	return;
    }
    d->text.replace( d->imstart, d->imend - d->imstart, e->text() );
    d->imend = d->imstart + e->text().length();
    d->imselstart = d->imstart + e->cursorPos();
    d->imselend = d->imselstart + e->selectionLength();
    d->cursor = d->imstart + e->cursorPos();
    d->updateTextLayout();
    d->updateMicroFocusHint();
    update();
}

/*! \reimp
 */
void QLineEdit::imEndEvent( QIMEvent *e )
{
    if ( d->readOnly) {
	e->ignore();
	return;
    }
    d->text.remove( d->imstart, d->imend - d->imstart );
    d->cursor = d->imselstart = d->imselend = d->imend = d->imstart;
    d->textDirty = TRUE;
    insert( e->text() );
}

/*!\reimp
*/

void QLineEdit::focusInEvent( QFocusEvent* )
{
    if ( QFocusEvent::reason() == QFocusEvent::Tab ||
	 QFocusEvent::reason() == QFocusEvent::Backtab  ||
	 QFocusEvent::reason() == QFocusEvent::Shortcut )
	d->maskData ? d->moveCursor( d->nextMaskBlank( 0 ) ) : selectAll();
    if ( !d->cursorTimer ) {
	int cft = QApplication::cursorFlashTime();
	d->cursorTimer = cft ? startTimer( cft/2 ) : -1;
    }
    if( !hasSelectedText() || style().styleHint( QStyle::SH_BlinkCursorWhenTextSelected ) )
	d->setCursorVisible( TRUE );
    d->updateMicroFocusHint();
}

/*!\reimp
*/

void QLineEdit::focusOutEvent( QFocusEvent* )
{
    if ( QFocusEvent::reason() != QFocusEvent::ActiveWindow &&
	 QFocusEvent::reason() != QFocusEvent::Popup )
	deselect();
    d->setCursorVisible( FALSE );
    if ( d->cursorTimer > 0 )
	killTimer( d->cursorTimer );
    d->cursorTimer = 0;
    if (QFocusEvent::reason() != QFocusEvent::Popup)
        emit lostFocus();
}

/*!\reimp
*/
void QLineEdit::drawContents( QPainter *p )
{
    const QColorGroup& cg = colorGroup();
    QRect cr = contentsRect();
    QFontMetrics fm = fontMetrics();
    QRect lineRect( cr.x() + innerMargin, cr.y() + (cr.height() - fm.height() + 1) / 2,
		    cr.width() - 2*innerMargin, fm.height() );
    QBrush bg = QBrush( paletteBackgroundColor() );
    if ( paletteBackgroundPixmap() )
	bg = QBrush( cg.background(), *paletteBackgroundPixmap() );
    else if ( !isEnabled() )
	bg = cg.brush( QColorGroup::Background );
    QPoint brushOrigin = p->brushOrigin();
    p->save();
    p->setClipRegion( QRegion(cr) - lineRect );
    p->setBrushOrigin(brushOrigin - backgroundOffset());
    p->fillRect( cr, bg );
    p->restore();
    QSharedDoubleBuffer buffer( p, lineRect.x(), lineRect.y(),
 				lineRect.width(), lineRect.height(),
 				hasFocus() ? QSharedDoubleBuffer::Force : 0 );
    p = buffer.painter();
    brushOrigin = p->brushOrigin();
    p->setBrushOrigin(brushOrigin - backgroundOffset());
    p->fillRect( lineRect, bg );
    p->setBrushOrigin(brushOrigin);

    // locate cursor position
    int cix = 0;
    QTextItem ci = d->textLayout.findItem( d->cursor );
    if ( ci.isValid() ) {
	if ( d->cursor != (int)d->text.length() && d->cursor == ci.from() + ci.length()
	     && ci.isRightToLeft() != d->isRightToLeft() )
	    ci = d->textLayout.findItem( d->cursor + 1 );
	cix = ci.x() + ci.cursorToX( d->cursor - ci.from() );
    }

    // horizontal scrolling
    int minLB = QMAX( 0, -fm.minLeftBearing() );
    int minRB = QMAX( 0, -fm.minRightBearing() );
    int widthUsed = d->textLayout.widthUsed() + 1 + minRB;
    if ( (minLB + widthUsed) <=  lineRect.width() ) {
	switch ( d->visualAlignment() ) {
	case AlignRight:
	    d->hscroll = widthUsed - lineRect.width() + 1;
	    break;
	case AlignHCenter:
	    d->hscroll = ( widthUsed - lineRect.width() ) / 2;
	    break;
	default:
	    d->hscroll = 0;
	    break;
	}
	d->hscroll -= minLB;
    } else if ( cix - d->hscroll >= lineRect.width() ) {
	d->hscroll = cix - lineRect.width() + 1;
    } else if ( cix - d->hscroll < 0 ) {
	d->hscroll = cix;
    } else if ( widthUsed - d->hscroll < lineRect.width() ) {
	d->hscroll = widthUsed - lineRect.width() + 1;
    } else if (d->hscroll < 0) {
        d->hscroll = 0;
    }
    // the y offset is there to keep the baseline constant in case we have script changes in the text.
    QPoint topLeft = lineRect.topLeft() - QPoint(d->hscroll, d->ascent-fm.ascent());

    // draw text, selections and cursors
    p->setPen( cg.text() );
    bool supressCursor = d->readOnly, hasRightToLeft = d->isRightToLeft();
    int textflags = 0;
    if ( font().underline() )
	textflags |= Qt::Underline;
    if ( font().strikeOut() )
	textflags |= Qt::StrikeOut;
    if ( font().overline() )
	textflags |= Qt::Overline;

    for ( int i = 0; i < d->textLayout.numItems(); i++ ) {
	QTextItem ti = d->textLayout.itemAt( i );
	hasRightToLeft |= ti.isRightToLeft();
	int tix = topLeft.x() + ti.x();
	int first = ti.from();
	int last = ti.from() + ti.length() - 1;

	// text and selection
	if ( d->selstart < d->selend && (last >= d->selstart && first < d->selend ) ) {
	    QRect highlight = QRect( QPoint( tix + ti.cursorToX( QMAX( d->selstart - first, 0 ) ),
					     lineRect.top() ),
				     QPoint( tix + ti.cursorToX( QMIN( d->selend - first, last - first + 1 ) ) - 1,
					     lineRect.bottom() ) ).normalize();
	    p->save();
  	    p->setClipRegion( QRegion( lineRect ) - highlight, QPainter::CoordPainter );
 	    p->drawTextItem( topLeft, ti, textflags );
 	    p->setClipRect( lineRect & highlight, QPainter::CoordPainter );
	    p->fillRect( highlight, cg.highlight() );
 	    p->setPen( cg.highlightedText() );
	    p->drawTextItem( topLeft, ti, textflags );
	    p->restore();
	} else {
	    p->drawTextItem( topLeft, ti, textflags );
	}

	// input method edit area
	if ( d->imstart < d->imend && (last >= d->imstart && first < d->imend ) ) {
	    QRect highlight = QRect( QPoint( tix + ti.cursorToX( QMAX( d->imstart - first, 0 ) ), lineRect.top() ),
			      QPoint( tix + ti.cursorToX( QMIN( d->imend - first, last - first + 1 ) )-1, lineRect.bottom() ) ).normalize();
	    p->save();
 	    p->setClipRect( lineRect & highlight, QPainter::CoordPainter );

	    int h1, s1, v1, h2, s2, v2;
	    cg.color( QColorGroup::Base ).hsv( &h1, &s1, &v1 );
	    cg.color( QColorGroup::Background ).hsv( &h2, &s2, &v2 );
	    QColor imCol;
	    imCol.setHsv( h1, s1, ( v1 + v2 ) / 2 );
	    p->fillRect( highlight, imCol );
	    p->drawTextItem( topLeft, ti, textflags );
	    p->restore();
	}

	// input method selection
	if ( d->imselstart < d->imselend && (last >= d->imselstart && first < d->imselend ) ) {
	    QRect highlight = QRect( QPoint( tix + ti.cursorToX( QMAX( d->imselstart - first, 0 ) ), lineRect.top() ),
			      QPoint( tix + ti.cursorToX( QMIN( d->imselend - first, last - first + 1 ) )-1, lineRect.bottom() ) ).normalize();
	    p->save();
	    p->setClipRect( lineRect & highlight, QPainter::CoordPainter );
	    p->fillRect( highlight, cg.text() );
	    p->setPen( paletteBackgroundColor() );
	    p->drawTextItem( topLeft, ti, textflags );
	    p->restore();
            supressCursor = TRUE;
	}

	// overwrite cursor
	if ( d->cursorVisible && d->maskData &&
	     d->selend <= d->selstart && (last >= d->cursor && first <= d->cursor ) ) {
	    QRect highlight = QRect( QPoint( tix + ti.cursorToX( QMAX( d->cursor - first, 0 ) ), lineRect.top() ),
				     QPoint( tix + ti.cursorToX( QMIN( d->cursor + 1 - first, last - first + 1 ) )-1, lineRect.bottom() ) ).normalize();
	    p->save();
	    p->setClipRect( lineRect & highlight, QPainter::CoordPainter );
	    p->fillRect( highlight, cg.text() );
	    p->setPen( paletteBackgroundColor() );
	    p->drawTextItem( topLeft, ti, textflags );
	    p->restore();
	    supressCursor = TRUE;
	}
    }

    // draw cursor
    if ( d->cursorVisible && !supressCursor ) {
	QPoint from( topLeft.x() + cix, lineRect.top() );
	QPoint to = from + QPoint( 0, lineRect.height() );
	p->drawLine( from, to );
	if ( hasRightToLeft ) {
	    bool rtl = ci.isValid() ? ci.isRightToLeft() : TRUE;
	    to = from + QPoint( (rtl ? -2 : 2), 2 );
	    p->drawLine( from, to );
	    from.ry() += 4;
	    p->drawLine( from, to );
	}
    }
    buffer.end();
}


#ifndef QT_NO_DRAGANDDROP
/*!\reimp
*/
void QLineEdit::dragMoveEvent( QDragMoveEvent *e )
{
    if ( !d->readOnly && QTextDrag::canDecode(e) ) {
	e->acceptAction();
	d->cursor = d->xToPos( e->pos().x() );
	d->cursorVisible = TRUE;
	update();
    }
}

/*!\reimp */
void QLineEdit::dragEnterEvent( QDragEnterEvent * e )
{
    QLineEdit::dragMoveEvent( e );
}

/*!\reimp */
void QLineEdit::dragLeaveEvent( QDragLeaveEvent *)
{
    if ( d->cursorVisible ) {
	d->cursorVisible = FALSE;
	update();
    }
}

/*!\reimp */
void QLineEdit::dropEvent( QDropEvent* e )
{
    QString str;
    // try text/plain
    QCString plain = "plain";
    bool decoded = QTextDrag::decode(e, str, plain);
    // otherwise we'll accept any kind of text (like text/uri-list)
    if (! decoded)
	decoded = QTextDrag::decode(e, str);

    if ( decoded && !d->readOnly ) {
	if ( e->source() == this && e->action() == QDropEvent::Copy )
 	    deselect();
	d->cursor =d->xToPos( e->pos().x() );
	int selStart = d->cursor;
	int oldSelStart = d->selstart;
	int oldSelEnd = d->selend;
	d->cursorVisible = FALSE;
	e->acceptAction();
	insert( str );
	if ( e->source() == this ) {
	    if ( e->action() == QDropEvent::Move ) {
		if ( selStart > oldSelStart && selStart <= oldSelEnd )
		    setSelection( oldSelStart, str.length() );
		else if ( selStart > oldSelEnd )
		    setSelection( selStart - str.length(), str.length() );
		else
		    setSelection( selStart, str.length() );
	    } else {
		setSelection( selStart, str.length() );
	    }
	}
    } else {
	e->ignore();
	update();
    }
}

bool QLineEditPrivate::drag()
{
    q->killTimer( dndTimer );
    dndTimer = 0;
    QTextDrag *tdo = new QTextDrag( q->selectedText(), q );

    QGuardedPtr<QLineEdit> gptr = q;
    bool r = tdo->drag();
    if ( !gptr )
        return FALSE;

    // ### fix the check QDragObject::target() != q in Qt4 (should not be needed)
    if ( r && !readOnly && QDragObject::target() != q ) {
	int priorState = undoState;
	removeSelectedText();
	finishChange( priorState );
    }
#ifndef QT_NO_CURSOR
    q->setCursor( readOnly ? arrowCursor : ibeamCursor );
#endif
    return TRUE;
}

#endif // QT_NO_DRAGANDDROP

enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

/*!\reimp
*/
void QLineEdit::contextMenuEvent( QContextMenuEvent * e )
{
#ifndef QT_NO_POPUPMENU
    d->separate();
    QPopupMenu *menu = createPopupMenu();
    if (!menu)
        return;
    QGuardedPtr<QPopupMenu> popup = menu;
    QGuardedPtr<QLineEdit> that = this;
    QPoint pos = e->reason() == QContextMenuEvent::Mouse ? e->globalPos() :
		 mapToGlobal( QPoint(e->pos().x(), 0) ) + QPoint( width() / 2, height() / 2 );
    int r = popup->exec( pos );
    delete (QPopupMenu*)popup;
    if ( that && d->menuId ) {
	switch ( d->menuId - r ) {
	case IdClear: clear(); break;
	case IdSelectAll: selectAll(); break;
	case IdUndo: undo(); break;
	case IdRedo: redo(); break;
#ifndef QT_NO_CLIPBOARD
	case IdCut: cut(); break;
	case IdCopy: copy(); break;
	case IdPaste: paste(); break;
#endif
	default:
	    ; // nothing selected or lineedit destroyed. Be careful.
	}
    }
#endif //QT_NO_POPUPMENU
}

/*!
    This function is called to create the popup menu which is shown
    when the user clicks on the line edit with the right mouse button.
    If you want to create a custom popup menu, reimplement this
    function and return the popup menu you create. The popup menu's
    ownership is transferred to the caller.
*/

QPopupMenu *QLineEdit::createPopupMenu()
{
#ifndef QT_NO_POPUPMENU
    QPopupMenu *popup = new QPopupMenu( this, "qt_edit_menu" );
    int id = d->menuId = popup->insertItem( tr( "&Undo" ) + ACCEL_KEY( Z ) );
    popup->insertItem( tr( "&Redo" ) + ACCEL_KEY( Y ) );
    popup->insertSeparator();
    popup->insertItem( tr( "Cu&t" ) + ACCEL_KEY( X ) );
    popup->insertItem( tr( "&Copy" ) + ACCEL_KEY( C ) );
    popup->insertItem( tr( "&Paste" ) + ACCEL_KEY( V ) );
    popup->insertItem( tr( "Clear" ) );
    popup->insertSeparator();
    popup->insertItem( tr( "Select All" )
#ifndef Q_WS_X11
    + ACCEL_KEY( A )
#endif
	);
    popup->setItemEnabled( id - IdUndo, d->isUndoAvailable() );
    popup->setItemEnabled( id - IdRedo, d->isRedoAvailable() );
#ifndef QT_NO_CLIPBOARD
    popup->setItemEnabled( id - IdCut, !d->readOnly && d->hasSelectedText() );
    popup->setItemEnabled( id - IdCopy, d->hasSelectedText() );
    popup->setItemEnabled( id - IdPaste, !d->readOnly && !QApplication::clipboard()->text().isEmpty() );
#else
    popup->setItemVisible( id - IdCut, FALSE );
    popup->setItemVisible( id - IdCopy, FALSE );
    popup->setItemVisible( id - IdPaste, FALSE );
#endif
    popup->setItemEnabled( id - IdClear, !d->readOnly && !d->text.isEmpty() );
    popup->setItemEnabled( id - IdSelectAll, !d->text.isEmpty() && !d->allSelected() );
    return popup;
#else
    return 0;
#endif
}

/*! \reimp */
void QLineEdit::windowActivationChange( bool b )
{
    //### remove me with WHighlightSelection attribute
    if ( palette().active() != palette().inactive() )
	update();
    QWidget::windowActivationChange( b );
}

/*! \reimp */

void QLineEdit::setPalette( const QPalette & p )
{
    //### remove me with WHighlightSelection attribute
    QWidget::setPalette( p );
    update();
}

/*!
  \obsolete
  \fn void QLineEdit::repaintArea( int from, int to )
  Repaints all characters from \a from to \a to. If cursorPos is
  between from and to, ensures that cursorPos is visible.
*/

/*! \reimp
 */
void QLineEdit::setFont( const QFont & f )
{
    QWidget::setFont( f );
    d->updateTextLayout();
}

/*! \obsolete
*/
int QLineEdit::characterAt( int xpos, QChar *chr ) const
{
    int pos = d->xToPos( xpos + contentsRect().x() - d->hscroll + innerMargin );
    if ( chr && pos < (int) d->text.length() )
	*chr = d->text.at( pos );
    return pos;
}

/*!
    \internal

    Sets the password character to \a c.

    \sa passwordChar()
*/

void QLineEdit::setPasswordChar( QChar c )
{
    d->passwordChar = c;
}

/*!
    \internal

    Returns the password character.

    \sa setPasswordChar()
*/
QChar QLineEdit::passwordChar() const
{
    return ( d->passwordChar.isNull() ? QChar( style().styleHint( QStyle::SH_LineEdit_PasswordCharacter, this ) ) : d->passwordChar );
}

void QLineEdit::clipboardChanged()
{
}

void QLineEditPrivate::init( const QString& txt )
{
#ifndef QT_NO_CURSOR
    q->setCursor( readOnly ? arrowCursor : ibeamCursor );
#endif
    q->setFocusPolicy( QWidget::StrongFocus );
    q->setInputMethodEnabled( TRUE );
    //   Specifies that this widget can use more, but is able to survive on
    //   less, horizontal space; and is fixed vertically.
    q->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    q->setBackgroundMode( PaletteBase );
    q->setKeyCompression( TRUE );
    q->setMouseTracking( TRUE );
    q->setAcceptDrops( TRUE );
    q->setFrame( TRUE );
    text = txt;
    updateTextLayout();
    cursor = text.length();
}

void QLineEditPrivate::updateTextLayout()
{
    // replace all non-printable characters with spaces (to avoid
    // drawing boxes when using fonts that don't have glyphs for such
    // characters)
    const QString &displayText = q->displayText();
    QString str(displayText.unicode(), displayText.length());
    QChar* uc = (QChar*)str.unicode();
    for (int i = 0; i < (int)str.length(); ++i) {
	if (! uc[i].isPrint())
	    uc[i] = QChar(0x0020);
    }
    textLayout.setText( str, q->font() );
    textLayout.setDirection((QChar::Direction)direction);
    textLayout.beginLayout(QTextLayout::SingleLine);
    textLayout.beginLine( INT_MAX );
    while ( !textLayout.atEnd() )
	textLayout.addCurrentItem();
    ascent = 0;
    textLayout.endLine(0, 0, Qt::AlignLeft|Qt::SingleLine, &ascent);
}

int QLineEditPrivate::xToPos( int x, QTextItem::CursorPosition betweenOrOn ) const
{
    x-= q->contentsRect().x() - hscroll + innerMargin;
    for ( int i = 0; i < textLayout.numItems(); ++i ) {
	QTextItem ti = textLayout.itemAt( i );
	QRect tir = ti.rect();
	if ( x >= tir.left() && x <= tir.right() )
	    return ti.xToCursor( x - tir.x(), betweenOrOn ) + ti.from();
    }
    return x < 0 ? 0 : text.length();
}


QRect QLineEditPrivate::cursorRect() const
{
    QRect cr = q->contentsRect();
    int cix = cr.x() - hscroll + innerMargin;
    QTextItem ci = textLayout.findItem( cursor );
    if ( ci.isValid() ) {
	if ( cursor != (int)text.length() && cursor == ci.from() + ci.length()
	     && ci.isRightToLeft() != isRightToLeft() )
	    ci = textLayout.findItem( cursor + 1 );
	cix += ci.x() + ci.cursorToX( cursor - ci.from() );
    }
    int ch = q->fontMetrics().height();
    return QRect( cix-4, cr.y() + ( cr.height() -  ch + 1) / 2, 8, ch + 1 );
}

void QLineEditPrivate::updateMicroFocusHint()
{
    if ( q->hasFocus() ) {
	QRect r = cursorRect();
	q->setMicroFocusHint( r.x(), r.y(), r.width(), r.height() );
    }
}

void QLineEditPrivate::moveCursor( int pos, bool mark )
{
    if ( pos != cursor )
	separate();
    if ( maskData && pos > cursor )
	pos = nextMaskBlank( pos );
    else if ( maskData && pos < cursor )
	pos = prevMaskBlank( pos );
    bool fullUpdate = mark || hasSelectedText();
    if ( mark ) {
	int anchor;
	if ( selend > selstart && cursor == selstart )
	    anchor = selend;
	else if ( selend > selstart && cursor == selend )
	    anchor = selstart;
	else
	    anchor = cursor;
	selstart = QMIN( anchor, pos );
	selend = QMAX( anchor, pos );
    } else {
	deselect();
    }
    if ( fullUpdate ) {
	cursor = pos;
	q->update();
    } else {
	setCursorVisible( FALSE );
	cursor = pos;
	setCursorVisible( TRUE );
    }
    updateMicroFocusHint();
    if ( mark && !q->style().styleHint( QStyle::SH_BlinkCursorWhenTextSelected ) )
	setCursorVisible( FALSE );
    if ( mark || selDirty ) {
	selDirty = FALSE;
	emit q->selectionChanged();
    }
}

void QLineEditPrivate::finishChange( int validateFromState, bool setModified )
{
    bool lineDirty = selDirty;
    if ( textDirty ) {
	// do validation
	bool wasValidInput = validInput;
	validInput = TRUE;
#ifndef QT_NO_VALIDATOR
	if ( validator && validateFromState >= 0 ) {
	    QString textCopy = text;
	    int cursorCopy = cursor;
	    validInput = ( validator->validate( textCopy, cursorCopy ) != QValidator::Invalid );
	    if ( validInput ) {
		if ( text != textCopy ) {
		    q->setText( textCopy );
		    cursor = cursorCopy;
		    return;
		}
		cursor = cursorCopy;
	    }
	}
#endif
	if ( validateFromState >= 0 && wasValidInput && !validInput ) {
	    undo( validateFromState );
	    history.resize( undoState );
	    validInput = TRUE;
	    textDirty = setModified = FALSE;
	}
	updateTextLayout();
	updateMicroFocusHint();
	lineDirty |= textDirty;
	if ( setModified )
	    modified = TRUE;
	if ( textDirty ) {
	    textDirty = FALSE;
	    emit q->textChanged( maskData ? stripString(text) : text );
	}
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( q, 0, QAccessible::ValueChanged );
#endif
    }
    if ( selDirty ) {
	selDirty = FALSE;
	emit q->selectionChanged();
    }
    if ( lineDirty || !setModified )
	q->update();
}

void QLineEditPrivate::setText( const QString& txt )
{
    deselect();
    QString oldText = text;
    if ( maskData ) {
	text = maskString( 0, txt, TRUE );
	text += clearString( text.length(), maxLength - text.length() );
    } else {
 	text = txt.isEmpty() ? txt : txt.left( maxLength );
    }
    history.clear();
    undoState = 0;
    cursor = text.length();
    textDirty = ( oldText != text );
}


void QLineEditPrivate::setCursorVisible( bool visible )
{
    if ( (bool)cursorVisible == visible )
	return;
    if ( cursorTimer )
	cursorVisible = visible;
    QRect r = cursorRect();
    if ( maskData || !q->contentsRect().contains( r ) )
	q->update();
    else
	q->update( r );
}

void QLineEditPrivate::addCommand( const Command& cmd )
{
    if ( separator && undoState && history[undoState-1].type != Separator ) {
	history.resize( undoState + 2 );
	history[undoState++] = Command( Separator, 0, 0 );
    } else {
	history.resize( undoState + 1);
    }
    separator = FALSE;
    history[ undoState++ ] = cmd;
}

void QLineEditPrivate::insert( const QString& s )
{
    if ( maskData ) {
	QString ms = maskString( cursor, s );
	for ( int i = 0; i < (int) ms.length(); ++i ) {
	    addCommand ( Command( DeleteSelection, cursor+i, text.at(cursor+i) ) );
	    addCommand( Command( Insert, cursor+i, ms.at(i) ) );
	}
	text.replace( cursor, ms.length(), ms );
	cursor += ms.length();
	cursor = nextMaskBlank( cursor );
    } else {
	int remaining = maxLength - text.length();
	text.insert( cursor, s.left(remaining) );
	for ( int i = 0; i < (int) s.left(remaining).length(); ++i )
	    addCommand( Command( Insert, cursor++, s.at(i) ) );
    }
    textDirty = TRUE;
}

void QLineEditPrivate::del( bool wasBackspace )
{
    if ( cursor < (int) text.length() ) {
	addCommand ( Command( (CommandType)((maskData?2:0)+(wasBackspace?Remove:Delete)), cursor, text.at(cursor) ) );
	if ( maskData ) {
	    text.replace( cursor, 1, clearString( cursor, 1 ) );
	    addCommand( Command( Insert, cursor, text.at( cursor ) ) );
	} else {
	    text.remove( cursor, 1 );
	}
	textDirty = TRUE;
    }
}

void QLineEditPrivate::removeSelectedText()
{
    if ( selstart < selend && selend <= (int) text.length() ) {
	separate();
	int i ;
	if ( selstart <= cursor && cursor < selend ) {
	    // cursor is within the selection. Split up the commands
	    // to be able to restore the correct cursor position
	    for ( i = cursor; i >= selstart; --i )
		addCommand ( Command( DeleteSelection, i, text.at(i) ) );
	    for ( i = selend - 1; i > cursor; --i )
		addCommand ( Command( DeleteSelection, i - cursor + selstart - 1, text.at(i) ) );
	} else {
	    for ( i = selend-1; i >= selstart; --i )
		addCommand ( Command( RemoveSelection, i, text.at(i) ) );
	}
	if ( maskData ) {
	    text.replace( selstart, selend - selstart,  clearString( selstart, selend - selstart ) );
	    for ( int i = 0; i < selend - selstart; ++i )
		addCommand( Command( Insert, selstart + i, text.at( selstart + i ) ) );
	} else {
	    text.remove( selstart, selend - selstart );
	}
	if ( cursor > selstart )
	    cursor -= QMIN( cursor, selend ) - selstart;
	deselect();
	textDirty = TRUE;
    }
}

void QLineEditPrivate::parseInputMask( const QString &maskFields )
{
    if ( maskFields.isEmpty() || maskFields.section( ';', 0, 0 ).isEmpty() ) {
	if ( maskData ) {
	    delete [] maskData;
	    maskData = 0;
	    maxLength = 32767;
	    q->setText( QString::null );
	}
	return;
    }

    inputMask =  maskFields.section( ';', 0, 0 );
    blank = maskFields.section( ';', 1, 1 ).at(0);
    if ( blank.isNull() )
	blank = ' ';

    // calculate maxLength / maskData length
    maxLength = 0;
    QChar c = 0;
    uint i;
    for ( i=0; i<inputMask.length(); i++ ) {
	c = inputMask.at(i);
	if ( i > 0 && inputMask.at( i-1 ) == '\\' ) {
	    maxLength++;
	    continue;
	}
	if ( c != '\\' && c != '!' &&
	     c != '<' && c != '>' &&
	     c != '{' && c != '}' &&
	     c != '[' && c != ']' )
	    maxLength++;
    }

    delete [] maskData;
    maskData = new MaskInputData[ maxLength ];

    MaskInputData::Casemode m = MaskInputData::NoCaseMode;
    c = 0;
    bool s;
    bool escape = FALSE;
    int index = 0;
    for ( i = 0; i < inputMask.length(); i++ ) {
	c = inputMask.at(i);
	if ( escape ) {
	    s = TRUE;
	    maskData[ index ].maskChar = c;
	    maskData[ index ].separator = s;
	    maskData[ index ].caseMode = m;
	    index++;
	    escape = FALSE;
	} else if ( c == '<' || c == '>' || c == '!') {
	    switch ( c ) {
	    case '<':
		m = MaskInputData::Lower;
		break;
	    case '>':
		m = MaskInputData::Upper;
		break;
	    case '!':
		m = MaskInputData::NoCaseMode;
		break;
	    }
	} else if ( c != '{' && c != '}' && c != '[' && c != ']' ) {
	    switch ( c ) {
	    case 'A':
	    case 'a':
	    case 'N':
	    case 'n':
	    case 'X':
	    case 'x':
	    case '9':
	    case '0':
	    case 'D':
	    case 'd':
	    case '#':
		s = FALSE;
		break;
	    case '\\':
		escape = TRUE;
	    default:
		s = TRUE;
		break;
	    }

	    if ( !escape ) {
		maskData[ index ].maskChar = c;
		maskData[ index ].separator = s;
		maskData[ index ].caseMode = m;
		index++;
	    }
	}
    }
    q->setText( QString::null );
}


/* checks if the key is valid compared to the inputMask */
bool QLineEditPrivate::isValidInput( QChar key, QChar mask ) const
{
    switch ( mask ) {
    case 'A':
	if ( key.isLetter() && key != blank )
	    return TRUE;
	break;
    case 'a':
	if ( key.isLetter() || key == blank )
	    return TRUE;
	break;
    case 'N':
	if ( key.isLetterOrNumber() && key != blank )
	    return TRUE;
	break;
    case 'n':
	if ( key.isLetterOrNumber() || key == blank )
	    return TRUE;
	break;
    case 'X':
	if ( key.isPrint() && key != blank )
	    return TRUE;
	break;
    case 'x':
	if ( key.isPrint() || key == blank )
	    return TRUE;
	break;
    case '9':
	if ( key.isNumber() && key != blank )
	    return TRUE;
	break;
    case '0':
	if ( key.isNumber() || key == blank )
	    return TRUE;
	break;
    case 'D':
	if ( key.isNumber() && key.digitValue() > 0 && key != blank )
	    return TRUE;
	break;
    case 'd':
	if ( (key.isNumber() && key.digitValue() > 0) || key == blank )
	    return TRUE;
	break;
    case '#':
	if ( key.isNumber() || key == '+' || key == '-' || key == blank )
	    return TRUE;
	break;
    default:
	break;
    }
    return FALSE;
}

/*
  Applies the inputMask on \a str starting from position \a pos in the mask. \a clear
  specifies from where characters should be gotten when a separator is met in \a str - TRUE means
  that blanks will be used, FALSE that previous input is used.
  Calling this when no inputMask is set is undefined.
*/
QString QLineEditPrivate::maskString( uint pos, const QString &str, bool clear) const
{
    if ( pos >= (uint)maxLength )
	return QString::fromLatin1("");

    QString fill;
    fill = clear ? clearString( 0, maxLength ) : text;

    uint strIndex = 0;
    QString s = QString::fromLatin1("");
    int i = pos;
    while ( i < maxLength ) {
	if ( strIndex < str.length() ) {
	    if ( maskData[ i ].separator ) {
		s += maskData[ i ].maskChar;
		if ( str[(int)strIndex] == maskData[ i ].maskChar )
		    strIndex++;
		++i;
	    } else {
		if ( isValidInput( str[(int)strIndex], maskData[ i ].maskChar ) ) {
		    switch ( maskData[ i ].caseMode ) {
		    case MaskInputData::Upper:
			s += str[(int)strIndex].upper();
			break;
		    case MaskInputData::Lower:
			s += str[(int)strIndex].lower();
			break;
		    default:
			s += str[(int)strIndex];
		    }
		    ++i;
		} else {
		    // search for separator first
		    int n = findInMask( i, TRUE, TRUE, str[(int)strIndex] );
		    if ( n != -1 ) {
			if ( str.length() != 1 || i == 0 || (i > 0 && (!maskData[i-1].separator || maskData[i-1].maskChar != str[(int)strIndex])) ) {
			    s += fill.mid( i, n-i+1 );
			    i = n + 1; // update i to find + 1
			}
		    } else {
			// search for valid blank if not
			n = findInMask( i, TRUE, FALSE, str[(int)strIndex] );
			if ( n != -1 ) {
			    s += fill.mid( i, n-i );
			    switch ( maskData[ n ].caseMode ) {
			    case MaskInputData::Upper:
				s += str[(int)strIndex].upper();
				break;
			    case MaskInputData::Lower:
				s += str[(int)strIndex].lower();
				break;
			    default:
				s += str[(int)strIndex];
			    }
			    i = n + 1; // updates i to find + 1
			}
		    }
		}
		strIndex++;
	    }
	} else
	    break;
    }

    return s;
}



/*
  Returns a "cleared" string with only separators and blank chars.
  Calling this when no inputMask is set is undefined.
*/
QString QLineEditPrivate::clearString( uint pos, uint len ) const
{
    if ( pos >= (uint)maxLength )
	return QString::null;

    QString s;
    int end = QMIN( (uint)maxLength, pos + len );
    for ( int i=pos; i<end; i++ )
	if ( maskData[ i ].separator )
	    s += maskData[ i ].maskChar;
	else
	    s += blank;

    return s;
}

/*
  Strips blank parts of the input in a QLineEdit when an inputMask is set,
  separators are still included. Typically "127.0__.0__.1__" becomes "127.0.0.1".
*/
QString QLineEditPrivate::stripString( const QString &str ) const
{
    if ( !maskData )
	return str;

    QString s;
    int end = QMIN( maxLength, (int)str.length() );
    for (int i=0; i < end; i++ )
	if ( maskData[ i ].separator )
	    s += maskData[ i ].maskChar;
	else
	    if ( str[i] != blank )
		s += str[i];

    return s;
}

/* searches forward/backward in maskData for either a separator or a blank */
int QLineEditPrivate::findInMask( int pos, bool forward, bool findSeparator, QChar searchChar ) const
{
    if ( pos >= maxLength || pos < 0 )
	return -1;

    int end = forward ? maxLength : -1;
    int step = forward ? 1 : -1;
    int i = pos;

    while ( i != end ) {
	if ( findSeparator ) {
	    if ( maskData[ i ].separator && maskData[ i ].maskChar == searchChar )
		return i;
	} else {
	    if ( !maskData[ i ].separator ) {
 		if ( searchChar.isNull() )
		    return i;
 		else if ( isValidInput( searchChar, maskData[ i ].maskChar ) )
 		    return i;
	    }
	}
	i += step;
    }
    return -1;
}


#endif // QT_NO_LINEEDIT
