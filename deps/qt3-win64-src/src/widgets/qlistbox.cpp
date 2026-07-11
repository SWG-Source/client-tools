/**********************************************************************
** $Id: qlistbox.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QListBox widget class
**
** Created : 941121
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

#include "qglobal.h"
#if defined(Q_CC_BOR)
// needed for qsort() because of a std namespace problem on Borland
#include "qplatformdefs.h"
#endif

#include "qlistbox.h"
#ifndef QT_NO_LISTBOX
#include "qmemarray.h"
#include "qfontmetrics.h"
#include "qpainter.h"
#include "qstrlist.h"
#include "qpixmap.h"
#include "qapplication.h"
#include "qptrdict.h"
#include "qtimer.h"
#include "qstringlist.h"
#include "qstyle.h"
#include "qpopupmenu.h"
#include "qguardedptr.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif

#include <stdlib.h>

class QListBoxPrivate
{
public:
    QListBoxPrivate( QListBox *lb ):
	head( 0 ), last( 0 ), cache( 0 ), cacheIndex( -1 ), current( 0 ),
	highlighted( 0 ), tmpCurrent( 0 ), columnPos( 1 ), rowPos( 1 ), rowPosCache( 0 ),
	columnPosOne( 0 ), rowMode( QListBox::FixedNumber ),
	columnMode( QListBox::FixedNumber ), numRows( 1 ), numColumns( 1 ),
	currentRow( 0 ), currentColumn( 0 ),
	mousePressRow( -1 ), mousePressColumn( -1 ),
	mouseMoveRow( -1 ), mouseMoveColumn( -1 ), mouseInternalPress( FALSE ),
	scrollTimer( 0 ), updateTimer( 0 ), visibleTimer( 0 ),
	selectionMode( QListBox::Single ),
	count( 0 ),
	listBox( lb ), currInputString( QString::null ),
	rowModeWins( FALSE ),
	ignoreMoves( FALSE ),
	layoutDirty( TRUE ),
	mustPaintAll( TRUE ),
	dragging( FALSE ),
	dirtyDrag ( FALSE ),
	variableHeight( TRUE /* !!! ### FALSE */ ),
	variableWidth( FALSE ),
	inMenuMode( FALSE )
    {}
    int findItemByName( int item, const QString &text );
    ~QListBoxPrivate();

    QListBoxItem * head, *last, *cache;
    int cacheIndex;
    QListBoxItem * current, *highlighted, *tmpCurrent;

    QMemArray<int> columnPos;
    QMemArray<int> rowPos;
    int rowPosCache;
    int columnPosOne;

    QListBox::LayoutMode rowMode;
    QListBox::LayoutMode columnMode;
    int numRows;
    int numColumns;

    int currentRow;
    int currentColumn;
    int mousePressRow;
    int mousePressColumn;
    int mouseMoveRow;
    int mouseMoveColumn;
    bool mouseInternalPress;

    QTimer * scrollTimer;
    QTimer * updateTimer;
    QTimer * visibleTimer;
    QTimer * resizeTimer;

    QPoint scrollPos;

    QListBox::SelectionMode selectionMode;

    int count;


    QListBox *listBox;
    QString currInputString;
    QTimer *inputTimer;

    QListBoxItem *pressedItem, *selectAnchor;

    uint select :1;
    uint pressedSelected :1;
    uint rowModeWins :1;
    uint ignoreMoves :1;
    uint clearing :1;
    uint layoutDirty :1;
    uint mustPaintAll :1;
    uint dragging :1;
    uint dirtyDrag :1;
    uint variableHeight :1;
    uint variableWidth :1;
    uint inMenuMode :1;

    QRect *rubber;
    QPtrDict<bool> selectable;

    struct SortableItem {
	QListBoxItem *item;
    };
};


QListBoxPrivate::~QListBoxPrivate()
{
    Q_ASSERT( !head );
}


/*!
    \class QListBoxItem qlistbox.h
    \brief The QListBoxItem class is the base class of all list box items.

    This class is an abstract base class used for all list box items.
    If you need to insert customized items into a QListBox you must
    inherit this class and reimplement paint(), height() and width().

    \sa QListBox
*/

/*!
    \fn bool QListBox::dragSelect() const
    \internal
*/

/*!
    \fn void QListBox::setDragSelect( bool )
    \internal
*/

/*!
    \fn bool QListBox::autoScroll() const
    \internal
*/

/*!
    \fn void QListBox::setAutoScroll( bool )
    \internal
*/

/*!
    \fn bool QListBox::autoScrollBar() const

    \obsolete

    Returns TRUE if vScrollBarMode() is \c Auto; otherwise returns
    FALSE.
*/

/*!
    \fn void QListBox::setAutoScrollBar( bool enable )

    \obsolete

    If \a enable is TRUE sets setVScrollBarMode() to \c AlwaysOn;
    otherwise sets setVScrollBarMode() to \c AlwaysOff.
*/

/*!
    \fn bool QListBox::scrollBar() const

    \obsolete

    Returns FALSE if vScrollBarMode() is \c AlwaysOff; otherwise
    returns TRUE.
*/

/*!
    \fn void QListBox::setScrollBar( bool enable )

    \obsolete

    If \a enable is TRUE sets setVScrollBarMode() to \c AlwaysOn;
    otherwise sets setVScrollBarMode() to \c AlwaysOff.
*/

/*!
    \fn bool QListBox::autoBottomScrollBar() const

    \obsolete

    Returns TRUE if hScrollBarMode() is \c Auto; otherwise returns
    FALSE.
*/

/*!
    \fn void QListBox::setAutoBottomScrollBar( bool enable )

    \obsolete

    If \a enable is TRUE sets setHScrollBarMode() to \c AlwaysOn;
    otherwise sets setHScrollBarMode() to \c AlwaysOff.
*/

/*!
    \fn bool QListBox::bottomScrollBar() const

    \obsolete

    Returns FALSE if vScrollBarMode() is \c AlwaysOff; otherwise
    returns TRUE.
*/

/*!
    \fn void QListBox::setBottomScrollBar( bool enable )

    \obsolete

    If \a enable is TRUE sets setHScrollBarMode() to \c AlwaysOn;
    otherwise sets setHScrollBarMode() to \c AlwaysOff.
*/

/*!
    \fn bool QListBox::smoothScrolling() const
    \internal
*/

/*!
    \fn void QListBox::setSmoothScrolling( bool )
    \internal
*/

/*!
    \fn bool QListBox::autoUpdate() const
    \internal
*/

/*!
    \fn void QListBox::setAutoUpdate( bool )
    \internal
*/

/*!
    \fn void QListBox::setFixedVisibleLines( int lines )
    \internal
*/

/*!
    \obsolete
    \fn int QListBox::cellHeight( int i ) const
    Returns the item height of item \a i.
    \sa itemHeight()
*/

/*!
    \obsolete
    \overload int QListBox::cellHeight() const
    Returns the item height of the first item, item 0.
    \sa itemHeight()
*/

/*!
    \obsolete
    \fn int QListBox::cellWidth() const
    Returns the maximum item width.
    \sa maxItemWidth()
*/

/*!
    \overload int QListBox::cellWidth(int i) const
    \internal
*/

/*!
    \obsolete
    \fn int QListBox::numCols() const
    Returns the number of columns.
    \sa numColumns()
*/

/*!
    \fn void QListBox::updateCellWidth()
    \internal
*/

/*!
    \obsolete
    \fn int QListBox::totalWidth() const
    Returns contentsWidth().
*/

/*!
    \obsolete
    \fn int QListBox::totalHeight() const
    Returns contentsHeight().
*/

/*!
    \obsolete
    \fn int QListBox::findItem( int yPos ) const
    Returns the index of the item a point (0, \a yPos).
    \sa index() itemAt()
*/


/*!
    Constructs an empty list box item in the list box \a listbox.
*/

QListBoxItem::QListBoxItem( QListBox* listbox )
{
    lbox = listbox;
    s = FALSE;
    dirty = TRUE;
    custom_highlight = FALSE;
    p = n = 0;

    // just something that'll look noticeable in the debugger
    x = y = 42;

    if (listbox)
	listbox->insertItem( this );
}

/*!
    Constructs an empty list box item in the list box \a listbox and
    inserts it after the item \a after or at the beginning if \a after
    is 0.
*/

QListBoxItem::QListBoxItem( QListBox* listbox, QListBoxItem *after )
{
    lbox = listbox;
    s = FALSE;
    dirty = TRUE;
    custom_highlight = FALSE;
    p = n = 0;

    // just something that'll be noticeable in the debugger
    x = y = 42;

    if (listbox)
	listbox->insertItem( this, after );
}


/*!
    Destroys the list box item.
*/

QListBoxItem::~QListBoxItem()
{
    if ( lbox )
	lbox->takeItem( this );
}


/*!
    Defines whether the list box item is responsible for drawing
    itself in a highlighted state when being selected.

    If \a b is FALSE (the default), the list box will draw some
    default highlight indicator before calling paint().

    \sa selected(), paint()
*/
void QListBoxItem::setCustomHighlighting( bool b )
{
    custom_highlight = b;
}

/*!
    \fn void QListBoxItem::paint( QPainter *p )

    Implement this function to draw your item. The painter, \a p, is
    already open for painting.

    \sa height(), width()
*/

/*!
    \fn int QListBoxItem::width( const QListBox* lb ) const

    Reimplement this function to return the width of your item. The \a
    lb parameter is the same as listBox() and is provided for
    convenience and compatibility.

    The default implementation returns
    \l{QApplication::globalStrut()}'s width.

    \sa paint(), height()
*/
int QListBoxItem::width(const QListBox*)  const
{
    return QApplication::globalStrut().width();
}

/*!
    \fn int QListBoxItem::height( const QListBox* lb ) const

    Implement this function to return the height of your item. The \a
    lb parameter is the same as listBox() and is provided for
    convenience and compatibility.

    The default implementation returns
    \l{QApplication::globalStrut()}'s height.

    \sa paint(), width()
*/
int QListBoxItem::height(const QListBox*)  const
{
    return QApplication::globalStrut().height();
}


/*!
    Returns the text of the item. This text is also used for sorting.

    \sa setText()
*/
QString QListBoxItem::text() const
{
    return txt;
}

/*!
    Returns the pixmap associated with the item, or 0 if there isn't
    one.

    The default implementation returns 0.
*/
const QPixmap *QListBoxItem::pixmap() const
{
    return 0;
}

/*!
    If \a b is TRUE (the default) then this item can be selected by
    the user; otherwise this item cannot be selected by the user.

    \sa isSelectable()
*/

void QListBoxItem::setSelectable( bool b )
{
    if ( !listBox() )
	return;
    bool *sel = listBox()->d->selectable.find( this );
    if ( !sel )
	listBox()->d->selectable.insert( this, new bool( b ) );
    else
	listBox()->d->selectable.replace( this, new bool( b ) );
}

/*!
    Returns TRUE if this item is selectable (the default); otherwise
    returns FALSE.

    \sa setSelectable()
*/

bool QListBoxItem::isSelectable() const
{
    if ( !listBox() )
	return TRUE;
    bool *sel = listBox()->d->selectable.find( ( (QListBoxItem*)this ) );
    if ( !sel )
	return TRUE;
    else
	return *sel;
}

/*!
    \fn void QListBoxItem::setText( const QString &text )

    Sets the text of the QListBoxItem to \a text. This \a text is also
    used for sorting. The text is not shown unless explicitly drawn in
    paint().

    \sa text()
*/


/*!
    \class QListBoxText qlistbox.h
    \brief The QListBoxText class provides list box items that display text.

    \ingroup advanced

    The text is drawn in the widget's current font. If you need
    several different fonts, you must implement your own subclass of
    QListBoxItem.

    \sa QListBox, QListBoxItem
*/


/*!
    Constructs a list box item in list box \a listbox showing the text
    \a text.
*/
QListBoxText::QListBoxText( QListBox *listbox, const QString &text )
    :QListBoxItem( listbox )
{
    setText( text );
}

/*!
    Constructs a list box item showing the text \a text.
*/

QListBoxText::QListBoxText( const QString &text )
    :QListBoxItem()
{
    setText( text );
}

/*!
    Constructs a list box item in list box \a listbox showing the text
    \a text. The item is inserted after the item \a after, or at the
    beginning if \a after is 0.
*/

QListBoxText::QListBoxText( QListBox* listbox, const QString &text, QListBoxItem *after )
    : QListBoxItem( listbox, after )
{
    setText( text );
}

/*!
    Destroys the item.
*/

QListBoxText::~QListBoxText()
{
}

/*!
    Draws the text using \a painter.
*/

void QListBoxText::paint( QPainter *painter )
{
    int itemHeight = height( listBox() );
    QFontMetrics fm = painter->fontMetrics();
    int yPos = ( ( itemHeight - fm.height() ) / 2 ) + fm.ascent();
    painter->drawText( 3, yPos, text() );
}

/*!
    Returns the height of a line of text in list box \a lb.

    \sa paint(), width()
*/

int QListBoxText::height( const QListBox* lb ) const
{
    int h = lb ? lb->fontMetrics().lineSpacing() + 2 : 0;
    return QMAX( h, QApplication::globalStrut().height() );
}

/*!
    Returns the width of this line in list box \a lb.

    \sa paint(), height()
*/

int QListBoxText::width( const QListBox* lb ) const
{
    int w = lb ? lb->fontMetrics().width( text() ) + 6 : 0;
    return QMAX( w, QApplication::globalStrut().width() );
}

int QListBoxText::RTTI = 1;

/*!
    \fn int QListBoxText::rtti() const

    \reimp

    Returns 1.

    Make your derived classes return their own values for rtti(), and
    you can distinguish between listbox items. You should use values
    greater than 1000 preferably a large random number, to allow for
    extensions to this class.
*/

int QListBoxText::rtti() const
{
    return RTTI;
}

/*!
    \class QListBoxPixmap qlistbox.h
    \brief The QListBoxPixmap class provides list box items with a
    pixmap and optional text.

    \ingroup advanced

    Items of this class are drawn with the pixmap on the left with the
    optional text to the right of the pixmap.

    \sa QListBox, QListBoxItem
*/


/*!
    Constructs a new list box item in list box \a listbox showing the
    pixmap \a pixmap.
*/

QListBoxPixmap::QListBoxPixmap( QListBox* listbox, const QPixmap &pixmap )
    : QListBoxItem( listbox )
{
    pm = pixmap;
}

/*!
    Constructs a new list box item showing the pixmap \a pixmap.
*/

QListBoxPixmap::QListBoxPixmap( const QPixmap &pixmap )
    : QListBoxItem()
{
    pm = pixmap;
}

/*!
    Constructs a new list box item in list box \a listbox showing the
    pixmap \a pixmap. The item gets inserted after the item \a after,
    or at the beginning if \a after is 0.
*/

QListBoxPixmap::QListBoxPixmap( QListBox* listbox, const QPixmap &pixmap, QListBoxItem *after )
    : QListBoxItem( listbox, after )
{
    pm = pixmap;
}


/*!
    Destroys the item.
*/

QListBoxPixmap::~QListBoxPixmap()
{
}


/*!
    Constructs a new list box item in list box \a listbox showing the
    pixmap \a pix and the text \a text.
*/
QListBoxPixmap::QListBoxPixmap( QListBox* listbox, const QPixmap &pix, const QString& text)
    : QListBoxItem( listbox )
{
    pm = pix;
    setText( text );
}

/*!
    Constructs a new list box item showing the pixmap \a pix and the
    text to \a text.
*/
QListBoxPixmap::QListBoxPixmap( const QPixmap & pix, const QString& text)
    : QListBoxItem()
{
    pm = pix;
    setText( text );
}

/*!
    Constructs a new list box item in list box \a listbox showing the
    pixmap \a pix and the string \a text. The item gets inserted after
    the item \a after, or at the beginning if \a after is 0.
*/
QListBoxPixmap::QListBoxPixmap( QListBox* listbox, const QPixmap & pix, const QString& text,
				QListBoxItem *after )
    : QListBoxItem( listbox, after )
{
    pm = pix;
    setText( text );
}

/*!
    \fn const QPixmap *QListBoxPixmap::pixmap() const

    Returns the pixmap associated with the item.
*/


/*!
    Draws the pixmap using \a painter.
*/

void QListBoxPixmap::paint( QPainter *painter )
{
    int itemHeight = height( listBox() );
    int yPos;

    const QPixmap *pm = pixmap();
    if ( pm && ! pm->isNull() ) {
	yPos = ( itemHeight - pm->height() ) / 2;
	painter->drawPixmap( 3, yPos, *pm);
    }

    if ( !text().isEmpty() ) {
	QFontMetrics fm = painter->fontMetrics();
	yPos = ( ( itemHeight - fm.height() ) / 2 ) + fm.ascent();
	painter->drawText( pm->width() + 5, yPos, text() );
    }
}

/*!
    Returns the height of the pixmap in list box \a lb.

    \sa paint(), width()
*/

int QListBoxPixmap::height( const QListBox* lb ) const
{
    int h;
    if ( text().isEmpty() )
	h = pm.height();
    else
	h = QMAX( pm.height(), lb->fontMetrics().lineSpacing() + 2 );
    return QMAX( h, QApplication::globalStrut().height() );
}

/*!
    Returns the width of the pixmap plus some margin in list box \a lb.

    \sa paint(), height()
*/

int QListBoxPixmap::width( const QListBox* lb ) const
{
    if ( text().isEmpty() )
	return QMAX( pm.width() + 6, QApplication::globalStrut().width() );
    return QMAX( pm.width() + lb->fontMetrics().width( text() ) + 6,
	    QApplication::globalStrut().width() );
}

int QListBoxPixmap::RTTI = 2;

/*!
    \fn int QListBoxPixmap::rtti() const

    \reimp

    Returns 2.

    Make your derived classes return their own values for rtti(), and
    you can distinguish between listbox items. You should use values
    greater than 1000 preferably a large random number, to allow for
    extensions to this class.
*/

int QListBoxPixmap::rtti() const
{
    return RTTI;
}

/*!
    \class QListBox qlistbox.h
    \brief The QListBox widget provides a list of selectable, read-only items.

    \ingroup advanced
    \mainclass

    This is typically a single-column list in which either no item or
    one item is selected, but it can also be used in many other ways.

    QListBox will add scroll bars as necessary, but it isn't intended
    for \e really big lists. If you want more than a few thousand
    items, it's probably better to use a different widget mainly
    because the scroll bars won't provide very good navigation, but
    also because QListBox may become slow with huge lists. (See
    QListView and QTable for possible alternatives.)

    There are a variety of selection modes described in the
    QListBox::SelectionMode documentation. The default is \c Single
    selection mode, but you can change it using setSelectionMode().
    (setMultiSelection() is still provided for compatibility with Qt
    1.x. We recommend using setSelectionMode() in all code.)

    Because QListBox offers multiple selection it must display
    keyboard focus and selection state separately. Therefore there are
    functions both to set the selection state of an item, i.e.
    setSelected(), and to set which item displays keyboard focus, i.e.
    setCurrentItem().

    The list box normally arranges its items in a single column and
    adds a vertical scroll bar if required. It is possible to have a
    different fixed number of columns (setColumnMode()), or as many
    columns as will fit in the list box's assigned screen space
    (setColumnMode(FitToWidth)), or to have a fixed number of rows
    (setRowMode()) or as many rows as will fit in the list box's
    assigned screen space (setRowMode(FitToHeight)). In all these
    cases QListBox will add scroll bars, as appropriate, in at least
    one direction.

    If multiple rows are used, each row can be as high as necessary
    (the normal setting), or you can request that all items will have
    the same height by calling setVariableHeight(FALSE). The same
    applies to a column's width, see setVariableWidth().

    The QListBox's items are QListBoxItem objects. QListBox provides
    methods to insert new items as strings, as pixmaps, and as
    QListBoxItem * (insertItem() with various arguments), and to
    replace an existing item with a new string, pixmap or QListBoxItem
    (changeItem() with various arguments). You can also remove items
    singly with removeItem() or clear() the entire list box. Note that
    if you create a QListBoxItem yourself and insert it, QListBox
    takes ownership of the item.

    You can also create a QListBoxItem, such as QListBoxText or
    QListBoxPixmap, with the list box as first parameter. The item
    will then append itself. When you delete an item it is
    automatically removed from the list box.

    The list of items can be arbitrarily large; QListBox will add
    scroll bars if necessary. QListBox can display a single-column
    (the common case) or multiple-columns, and offers both single and
    multiple selection. QListBox does not support multiple-column
    items (but QListView and QTable do), or tree hierarchies (but
    QListView does).

    The list box items can be accessed both as QListBoxItem objects
    (recommended) and using integer indexes (the original QListBox
    implementation used an array of strings internally, and the API
    still supports this mode of operation). Everything can be done
    using the new objects, and most things can be done using indexes.

    Each item in a QListBox contains a QListBoxItem. One of the items
    can be the current item. The currentChanged() signal and the
    highlighted() signal are emitted when a new item becomes current,
    e.g. because the user clicks on it or QListBox::setCurrentItem()
    is called. The selected() signal is emitted when the user
    double-clicks on an item or presses Enter on the current item.

    If the user does not select anything, no signals are emitted and
    currentItem() returns -1.

    A list box has \c WheelFocus as a default focusPolicy(), i.e. it
    can get keyboard focus by tabbing, clicking and through the use of
    the mouse wheel.

    New items can be inserted using insertItem(), insertStrList() or
    insertStringList(). inSort() is obsolete because this method is
    quite inefficient. It's preferable to insert the items normally
    and call sort() afterwards, or to insert a sorted QStringList().

    By default, vertical and horizontal scroll bars are added and
    removed as necessary. setHScrollBarMode() and setVScrollBarMode()
    can be used to change this policy.

    If you need to insert types other than strings and pixmaps, you
    must define new classes which inherit QListBoxItem.

    \warning The list box assumes ownership of all list box items and
    will delete them when it does not need them any more.

    <img src=qlistbox-m.png> <img src=qlistbox-w.png>

    \sa QListView QComboBox QButtonGroup
    \link guibooks.html#fowler GUI Design Handbook: List Box (two
    sections)\endlink
*/

/*!
    \enum QListBox::SelectionMode

    This enumerated type is used by QListBox to indicate how it reacts
    to selection by the user.

    \value Single  When the user selects an item, any already-selected
    item becomes unselected and the user cannot unselect the selected
    item. This means that the user can never clear the selection, even
    though the selection may be cleared by the application programmer
    using QListBox::clearSelection().

    \value Multi  When the user selects an item the selection status
    of that item is toggled and the other items are left alone. Also,
    multiple items can be selected by dragging the mouse while the
    left mouse button is kept pressed.

    \value Extended When the user selects an item the selection is
    cleared and the new item selected. However, if the user presses
    the Ctrl key when clicking on an item, the clicked item gets
    toggled and all other items are left untouched. And if the user
    presses the Shift key while clicking on an item, all items between
    the current item and the clicked item get selected or unselected,
    depending on the state of the clicked item. Also, multiple items
    can be selected by dragging the mouse while the left mouse button
    is kept pressed.

    \value NoSelection  Items cannot be selected.

    In other words, \c Single is a real single-selection list box, \c
    Multi is a real multi-selection list box, \c Extended is a list
    box in which users can select multiple items but usually want to
    select either just one or a range of contiguous items, and \c
    NoSelection is for a list box where the user can look but not
    touch.
*/


/*!
    \enum QListBox::LayoutMode

    This enum type is used to specify how QListBox lays out its rows
    and columns.

    \value FixedNumber  There is a fixed number of rows (or columns).

    \value FitToWidth   There are as many columns as will fit
    on-screen.

    \value FitToHeight  There are as many rows as will fit on-screen.

    \value Variable  There are as many rows as are required by the
    column mode. (Or as many columns as required by the row mode.)

    Example: When you call setRowMode( FitToHeight ), columnMode()
    automatically becomes \c Variable to accommodate the row mode
    you've set.
*/

/*!
    \fn void  QListBox::onItem( QListBoxItem *i )

    This signal is emitted when the user moves the mouse cursor onto
    an item, similar to the QWidget::enterEvent() function. \a i is
    the QListBoxItem that the mouse has moved on.
*/

// ### bug here too? enter/leave event may noit considered. move the
// mouse out of the window and back in, to the same item - does it
// work?

/*!
    \fn void  QListBox::onViewport()

    This signal is emitted when the user moves the mouse cursor from
    an item to an empty part of the list box.
*/


/*!
    Constructs a new empty list box called \a name and with parent \a
    parent.

    Performance is boosted by modifying the widget flags \a f so that
    only part of the QListBoxItem children is redrawn. This may be
    unsuitable for custom QListBoxItem classes, in which case \c
    WStaticContents and \c WNoAutoErase should be cleared
    immediately after construction.

    \sa QWidget::clearWFlags() Qt::WidgetFlags
*/

QListBox::QListBox( QWidget *parent, const char *name, WFlags f )
    : QScrollView( parent, name, f | WStaticContents | WNoAutoErase )
{
    d = new QListBoxPrivate( this );
    d->updateTimer = new QTimer( this, "listbox update timer" );
    d->visibleTimer = new QTimer( this, "listbox visible timer" );
    d->inputTimer = new QTimer( this, "listbox input timer" );
    d->resizeTimer = new QTimer( this, "listbox resize timer" );
    d->clearing = FALSE;
    d->pressedItem = 0;
    d->selectAnchor = 0;
    d->select = FALSE;
    d->rubber = 0;
    d->selectable.setAutoDelete( TRUE );

    setMouseTracking( TRUE );
    viewport()->setMouseTracking( TRUE );

    connect( d->updateTimer, SIGNAL(timeout()),
	     this, SLOT(refreshSlot()) );
    connect( d->visibleTimer, SIGNAL(timeout()),
	     this, SLOT(ensureCurrentVisible()) );
    connect( d->resizeTimer, SIGNAL( timeout() ),
	     this, SLOT( adjustItems() ) );
    viewport()->setBackgroundMode( PaletteBase );
    setBackgroundMode( PaletteBackground, PaletteBase );
    viewport()->setFocusProxy( this );
    viewport()->setFocusPolicy( WheelFocus );
}


QListBox * QListBox::changedListBox = 0;

/*!
    Destroys the list box. Deletes all list box items.
*/

QListBox::~QListBox()
{
    if ( changedListBox == this )
	changedListBox = 0;
    clear();
    delete d;
    d = 0;
}

/*!
    \fn void QListBox::pressed( QListBoxItem *item )

    This signal is emitted when the user presses any mouse button. If
    \a item is not 0, the cursor is on \a item. If \a item is 0, the
    mouse cursor isn't on any item.

    Note that you must not delete any QListBoxItem objects in slots
    connected to this signal.
*/

/*!
    \overload void QListBox::pressed( QListBoxItem *item, const QPoint &pnt )

    This signal is emitted when the user presses any mouse button. If
    \a item is not 0, the cursor is on \a item. If \a item is 0, the
    mouse cursor isn't on any item.

    \a pnt is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()).

    Note that you must not delete any QListBoxItem objects in slots
    connected to this signal.

    \sa mouseButtonPressed() rightButtonPressed() clicked()
*/

/*!
    \fn void QListBox::clicked( QListBoxItem *item )

    This signal is emitted when the user clicks any mouse button. If
    \a item is not 0, the cursor is on \a item. If \a item is 0, the
    mouse cursor isn't on any item.

    Note that you must not delete any QListBoxItem objects in slots
    connected to this signal.
*/

/*!
    \overload void QListBox::clicked( QListBoxItem *item, const QPoint &pnt )

    This signal is emitted when the user clicks any mouse button. If
    \a item is not 0, the cursor is on \a item. If \a item is 0, the
    mouse cursor isn't on any item.

    \a pnt is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()). (If the click's
    press and release differs by a pixel or two, \a pnt is the
    position at release time.)

    Note that you must not delete any QListBoxItem objects in slots
    connected to this signal.
*/

/*!
    \fn void QListBox::mouseButtonClicked (int button, QListBoxItem * item, const QPoint & pos)

    This signal is emitted when the user clicks mouse button \a
    button. If \a item is not 0, the cursor is on \a item. If \a item
    is 0, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()). (If the click's
    press and release differs by a pixel or two, \a pos is the
    position at release time.)

    Note that you must not delete any QListBoxItem objects in slots
    connected to this signal.
*/

/*!
    \fn void QListBox::mouseButtonPressed (int button, QListBoxItem * item, const QPoint & pos)

    This signal is emitted when the user presses mouse button \a
    button. If \a item is not 0, the cursor is on \a item. If \a item
    is 0, the mouse cursor isn't on any item.

    \a pos is the position of the mouse cursor in the global
    coordinate system (QMouseEvent::globalPos()).

    Note that you must not delete any QListBoxItem objects in slots
    connected to this signal.
*/

/*!
    \fn void QListBox::doubleClicked( QListBoxItem *item )

    This signal is emitted whenever an item is double-clicked. It's
    emitted on the second button press, not the second button release.
    If \a item is not 0, the cursor is on \a item. If \a item is 0,
    the mouse cursor isn't on any item.
*/


/*!
    \fn void QListBox::returnPressed( QListBoxItem * )

    This signal is emitted when Enter or Return is pressed. The
    argument is currentItem().
*/

/*!
    \fn void QListBox::rightButtonClicked( QListBoxItem *, const QPoint& )

    This signal is emitted when the right button is clicked (i.e. when
    it's released at the same point where it was pressed). The
    arguments are the relevant QListBoxItem (may be 0) and the point
    in global coordinates.
*/


/*!
    \fn void QListBox::rightButtonPressed (QListBoxItem *, const QPoint & )

    This signal is emitted when the right button is pressed. The
    arguments are the relevant QListBoxItem (may be 0) and the point
    in global coordinates.
*/

/*!
    \fn void QListBox::contextMenuRequested( QListBoxItem *item, const QPoint & pos )

    This signal is emitted when the user invokes a context menu with
    the right mouse button or with special system keys, with \a item
    being the item under the mouse cursor or the current item,
    respectively.

    \a pos is the position for the context menu in the global
    coordinate system.
*/

/*!
    \fn void QListBox::selectionChanged()

    This signal is emitted when the selection set of a list box
    changes. This signal is emitted in each selection mode. If the
    user selects five items by drag-selecting, QListBox tries to emit
    just one selectionChanged() signal so the signal can be connected
    to computationally expensive slots.

    \sa selected() currentItem()
*/

/*!
    \overload void QListBox::selectionChanged( QListBoxItem *item )

    This signal is emitted when the selection in a \c Single selection
    list box changes. \a item is the newly selected list box item.

    \sa selected() currentItem()
*/

/*!
    \fn void QListBox::currentChanged( QListBoxItem *item )

    This signal is emitted when the user makes a new item the current
    item. \a item is the new current list box item.

    \sa setCurrentItem() currentItem()
*/

/*!
    \fn void QListBox::highlighted( int index )

    This signal is emitted when the user makes a new item the current
    item. \a index is the index of the new current item.

    \sa currentChanged() selected() currentItem() selectionChanged()
*/

/*!
    \overload void QListBox::highlighted( QListBoxItem * )

    This signal is emitted when the user makes a new item the current
    item. The argument is a pointer to the new current item.

    \sa currentChanged() selected() currentItem() selectionChanged()
*/

/*!
    \overload void QListBox::highlighted( const QString & )

    This signal is emitted when the user makes a new item the current
    item and the item is (or has) a string. The argument is the text
    of the new current item.

    \sa currentChanged() selected() currentItem() selectionChanged()
*/

/*!
    \fn void QListBox::selected( int index )

    This signal is emitted when the user double-clicks on an item or
    presses Enter on the current item. \a index is the index of the
    selected item.

    \sa currentChanged() highlighted() selectionChanged()
*/

/*!
    \overload void QListBox::selected( QListBoxItem * )

    This signal is emitted when the user double-clicks on an item or
    presses Enter on the current item. The argument is a pointer to
    the new selected item.

    \sa currentChanged() highlighted() selectionChanged()
*/

/*!
    \overload void QListBox::selected( const QString &)

    This signal is emitted when the user double-clicks on an item or
    presses Enter on the current item, and the item is (or has) a
    string. The argument is the text of the selected item.

    \sa currentChanged() highlighted() selectionChanged()
*/

/*! \reimp */

void QListBox::setFont( const QFont &font )
{
    QScrollView::setFont( font );
    triggerUpdate( TRUE );
}


/*!
    \property QListBox::count
    \brief the number of items in the list box
*/

uint QListBox::count() const
{
    return d->count;
}


/*!
    Inserts the string list \a list into the list at position \a
    index.

    If \a index is negative, \a list is inserted at the end of the
    list. If \a index is too large, the operation is ignored.

    \warning This function uses \c{const char *} rather than QString,
    so we recommend against using it. It is provided so that legacy
    code will continue to work, and so that programs that certainly
    will not need to handle code outside a single 8-bit locale can use
    it. See insertStringList() which uses real QStrings.

    \warning This function is never significantly faster than a loop
    around insertItem().

    \sa insertItem(), insertStringList()
*/

void QListBox::insertStrList( const QStrList *list, int index )
{
    if ( !list ) {
#if defined(QT_CHECK_NULL)
	Q_ASSERT( list != 0 );
#endif
	return;
    }
    insertStrList( *list, index );
}



/*!
    Inserts the string list \a list into the list at position \a
    index.

    If \a index is negative, \a list is inserted at the end of the
    list. If \a index is too large, the operation is ignored.

    \warning This function is never significantly faster than a loop
    around insertItem().

    \sa insertItem(), insertStrList()
*/

void QListBox::insertStringList( const QStringList & list, int index )
{
    if ( index < 0 )
	index = count();
    for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
	insertItem( new QListBoxText(*it), index++ );
}



/*!
    \overload

    Inserts the string list \a list into the list at position \a
    index.

    If \a index is negative, \a list is inserted at the end of the
    list. If \a index is too large, the operation is ignored.

    \warning This function uses \c{const char *} rather than QString,
    so we recommend against using it. It is provided so that legacy
    code will continue to work, and so that programs that certainly
    will not need to handle code outside a single 8-bit locale can use
    it. See insertStringList() which uses real QStrings.

    \warning This function is never significantly faster than a loop
    around insertItem().

    \sa insertItem(), insertStringList()
*/

void QListBox::insertStrList( const QStrList & list, int index )
{
    QStrListIterator it( list );
    const char* txt;
    if ( index < 0 )
	index = count();
    while ( (txt=it.current()) ) {
	++it;
	insertItem( new QListBoxText(QString::fromLatin1(txt)),
		    index++ );
    }
    if ( hasFocus() && !d->current )
	setCurrentItem( d->head );
}


/*!
    \overload

    Inserts the \a numStrings strings of the array \a strings into the
    list at position \a index.

    If \a index is negative, insertStrList() inserts \a strings at the
    end of the list. If \a index is too large, the operation is
    ignored.

    \warning This function uses \c{const char *} rather than QString,
    so we recommend against using it. It is provided so that legacy
    code will continue to work, and so that programs that certainly
    will not need to handle code outside a single 8-bit locale can use
    it. See insertStringList() which uses real QStrings.

    \warning This function is never significantly faster than a loop
    around insertItem().

    \sa insertItem(), insertStringList()
*/

void QListBox::insertStrList( const char **strings, int numStrings, int index )
{
    if ( !strings ) {
#if defined(QT_CHECK_NULL)
	Q_ASSERT( strings != 0 );
#endif
	return;
    }
    if ( index < 0 )
	index = count();
    int i = 0;
    while ( (numStrings<0 && strings[i]!=0) || i<numStrings ) {
	insertItem( new QListBoxText( QString::fromLatin1(strings[i])),
		    index + i );
	i++;
    }
    if ( hasFocus() && !d->current )
	setCurrentItem( d->head );
}

/*!
    Inserts the item \a lbi into the list at position \a index.

    If \a index is negative or larger than the number of items in the
    list box, \a lbi is inserted at the end of the list.

    \sa insertStrList()
*/

void QListBox::insertItem( const QListBoxItem *lbi, int index )
{
#if defined ( QT_CHECK_NULL )
    Q_ASSERT( lbi != 0 );
#else
    if ( !lbi )
	return;
#endif

    if ( index < 0 )
	index = d->count;

    if ( index >= d->count ) {
	insertItem( lbi, d->last );
	return;
    }

    QListBoxItem * item = (QListBoxItem *)lbi;
    d->count++;
    d->cache = 0;

    item->lbox = this;
    if ( !d->head || index == 0 ) {
	item->n = d->head;
	item->p = 0;
	d->head = item;
	item->dirty = TRUE;
	if ( item->n )
	    item->n->p = item;
    } else {
	QListBoxItem * i = d->head;
	while ( i->n && index > 1 ) {
	    i = i->n;
	    index--;
	}
	if ( i->n ) {
	    item->n = i->n;
	    item->p = i;
	    item->n->p = item;
	    item->p->n = item;
	} else {
	    i->n = item;
	    item->p = i;
	    item->n = 0;
	}
    }

    if ( hasFocus() && !d->current ) {
	d->current = d->head;
	updateItem( d->current );
	emit highlighted( d->current );
	emit highlighted( d->current->text() );
	emit highlighted( index );
    }

    triggerUpdate( TRUE );
}

/*!
    \overload

    Inserts the item \a lbi into the list after the item \a after, or
    at the beginning if \a after is 0.

    \sa insertStrList()
*/

void QListBox::insertItem( const QListBoxItem *lbi, const QListBoxItem *after )
{
#if defined ( QT_CHECK_NULL )
    Q_ASSERT( lbi != 0 );
#else
    if ( !lbi )
	return;
#endif

    QListBoxItem * item = (QListBoxItem*)lbi;
    d->count++;
    d->cache = 0;

    item->lbox = this;
    if ( !d->head || !after ) {
	item->n = d->head;
	item->p = 0;
	d->head = item;
	item->dirty = TRUE;
	if ( item->n )
	    item->n->p = item;
    } else {
	QListBoxItem * i = (QListBoxItem*) after;
	if ( i ) {
	    item->n = i->n;
	    item->p = i;
	    if ( item->n )
		item->n->p = item;
	    if ( item->p )
		item->p->n = item;
	}
    }

    if ( after == d->last )
	d->last = (QListBoxItem*) lbi;

    if ( hasFocus() && !d->current ) {
	d->current = d->head;
	updateItem( d->current );
	emit highlighted( d->current );
	emit highlighted( d->current->text() );
	emit highlighted( index( d->current ) );
    }

    triggerUpdate( TRUE );
}

/*!
    \overload

    Inserts a new list box text item with the text \a text into the
    list at position \a index.

    If \a index is negative, \a text is inserted at the end of the
    list.

    \sa insertStrList()
*/

void QListBox::insertItem( const QString &text, int index )
{
    insertItem( new QListBoxText(text), index );
}

/*!
    \overload

    Inserts a new list box pixmap item with the pixmap \a pixmap into
    the list at position \a index.

    If \a index is negative, \a pixmap is inserted at the end of the
    list.

    \sa insertStrList()
*/

void QListBox::insertItem( const QPixmap &pixmap, int index )
{
    insertItem( new QListBoxPixmap(pixmap), index );
}

/*!
    \overload

    Inserts a new list box pixmap item with the pixmap \a pixmap and
    the text \a text into the list at position \a index.

    If \a index is negative, \a pixmap is inserted at the end of the
    list.

    \sa insertStrList()
*/

void QListBox::insertItem( const QPixmap &pixmap, const QString &text, int index )
{
    insertItem( new QListBoxPixmap(pixmap, text), index );
}

/*!
    Removes and deletes the item at position \a index. If \a index is
    equal to currentItem(), a new item becomes current and the
    currentChanged() and highlighted() signals are emitted.

    \sa insertItem(), clear()
*/

void QListBox::removeItem( int index )
{
    bool wasVisible = itemVisible( currentItem() );
    delete item( index );
    triggerUpdate( TRUE );
    if ( wasVisible )
	ensureCurrentVisible();
}


/*!
    Deletes all the items in the list.

    \sa removeItem()
*/

void QListBox::clear()
{
    setContentsPos( 0, 0 );
    bool blocked = signalsBlocked();
    blockSignals( TRUE );
    d->clearing = TRUE;
    d->current = 0;
    d->tmpCurrent = 0;
    QListBoxItem * i = d->head;
    d->head = 0;
    while ( i ) {
	QListBoxItem * n = i->n;
	i->n = i->p = 0;
	delete i;
	i = n;
    }
    d->count = 0;
    d->numRows = 1;
    d->numColumns = 1;
    d->currentRow = 0;
    d->currentColumn = 0;
    d->mousePressRow = -1;
    d->mousePressColumn = -1;
    d->mouseMoveRow = -1;
    d->mouseMoveColumn = -1;
    d->selectable.clear();
    clearSelection();
    blockSignals( blocked );
    triggerUpdate( TRUE );
    d->last = 0;
    d->clearing = FALSE;
}


/*!
    Returns the text at position \a index, or QString::null if there
    is no text at that position.

    \sa pixmap()
*/

QString QListBox::text( int index ) const
{
    QListBoxItem * i = item( index );
    if ( i )
	return i->text();
    return QString::null;
}


/*!
    Returns a pointer to the pixmap at position \a index, or 0 if
    there is no pixmap there.

    \sa text()
*/

const QPixmap *QListBox::pixmap( int index ) const
{
    QListBoxItem * i = item( index );
    if ( i )
	return i->pixmap();
    return 0;
}

/*!
    \overload

    Replaces the item at position \a index with a new list box text
    item with text \a text.

    The operation is ignored if \a index is out of range.

    \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QString &text, int index )
{
    if( index >= 0 && index < (int)count() )
	changeItem( new QListBoxText(text), index );
}

/*!
    \overload

    Replaces the item at position \a index with a new list box pixmap
    item with pixmap \a pixmap.

    The operation is ignored if \a index is out of range.

    \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QPixmap &pixmap, int index )
{
    if( index >= 0 && index < (int)count() )
	changeItem( new QListBoxPixmap(pixmap), index );
}

/*!
    \overload

    Replaces the item at position \a index with a new list box pixmap
    item with pixmap \a pixmap and text \a text.

    The operation is ignored if \a index is out of range.

    \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QPixmap &pixmap, const QString &text, int index )
{
    if( index >= 0 && index < (int)count() )
	changeItem( new QListBoxPixmap(pixmap, text), index );
}



/*!
    Replaces the item at position \a index with \a lbi.	If \a index is
    negative or too large, changeItem() does nothing.

    The item that has been changed will become selected.

    \sa insertItem(), removeItem()
*/

void QListBox::changeItem( const QListBoxItem *lbi, int index )
{
    if ( !lbi || index < 0 || index >= (int)count() )
	return;

    removeItem( index );
    insertItem( lbi, index );
    setCurrentItem( index );
}


/*!
    \property QListBox::numItemsVisible
    \brief the number of visible items.

    Both partially and entirely visible items are counted.
*/

int QListBox::numItemsVisible() const
{
    doLayout();

    int columns = 0;

    int x = contentsX();
    int i=0;
    while ( i < (int)d->columnPos.size()-1 &&
	   d->columnPos[i] < x )
	i++;
    if ( i < (int)d->columnPos.size()-1 &&
	 d->columnPos[i] > x )
	columns++;
    x += visibleWidth();
    while ( i < (int)d->columnPos.size()-1 &&
	   d->columnPos[i] < x ) {
	i++;
	columns++;
    }

    int y = contentsY();
    int rows = 0;
    while ( i < (int)d->rowPos.size()-1 &&
	   d->rowPos[i] < y )
	i++;
    if ( i < (int)d->rowPos.size()-1 &&
	 d->rowPos[i] > y )
	rows++;
    y += visibleHeight();
    while ( i < (int)d->rowPos.size()-1 &&
	   d->rowPos[i] < y ) {
	i++;
	rows++;
    }

    return rows*columns;
}

int QListBox::currentItem() const
{
    if ( !d->current || !d->head )
	return -1;

    return index( d->current );
}


/*!
    \property QListBox::currentText
    \brief the text of the current item.

    This is equivalent to text(currentItem()).
*/


/*!
    \property QListBox::currentItem
    \brief the current highlighted item

    When setting this property, the highlighting is moved to the item
    and the list box scrolled as necessary.

    If no item is current, currentItem() returns -1.
*/

void QListBox::setCurrentItem( int index )
{
    setCurrentItem( item( index ) );
}


/*!
    \overload

    Sets the current item to the QListBoxItem \a i.
*/
void QListBox::setCurrentItem( QListBoxItem * i )
{
    if ( !i || d->current == i || numRows() == 0 )
	return;

    QRect mfrect = itemRect( i );
    if ( mfrect.isValid() )
	setMicroFocusHint( mfrect.x(), mfrect.y(), mfrect.width(), mfrect.height(), FALSE );

    QListBoxItem * o = d->current;
    d->current = i;
    int ind = index( i );

    if ( i && selectionMode() == Single ) {
	bool changed = FALSE;
	if ( o && o->s ) {
	    changed = TRUE;
	    o->s = FALSE;
	}
	if ( i && !i->s && d->selectionMode != NoSelection && i->isSelectable() ) {
	    i->s = TRUE;
	    changed = TRUE;
	    emit selectionChanged( i );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    QAccessible::updateAccessibility( viewport(), ind+1, QAccessible::StateChanged );
#endif
	}
	if ( changed ) {
	    emit selectionChanged();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    QAccessible::updateAccessibility( viewport(), 0, QAccessible::Selection );
#endif
	}
    }

    d->currentColumn = ind / numRows();
    d->currentRow = ind % numRows();
    if ( o )
	updateItem( o );
    if ( i )
	updateItem( i );
    // scroll after the items are redrawn
    d->visibleTimer->start( 1, TRUE );

    QString tmp;
    if ( i )
	tmp = i->text();
    emit highlighted( i );
    if ( !tmp.isNull() )
	emit highlighted( tmp );
    emit highlighted( ind );
    emit currentChanged( i );

#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( viewport(), ind+1, QAccessible::Focus );
#endif
}


/*!
    Returns a pointer to the item at position \a index, or 0 if \a
    index is out of bounds.

    \sa index()
*/

QListBoxItem *QListBox::item( int index ) const
{
    if ( index < 0 || index > d->count -1 )
	return 0;

    QListBoxItem * i = d->head;

    if ( d->cache && index > 0 ) {
	i = d->cache;
	int idx = d->cacheIndex;
	while ( i && idx < index ) {
	    idx++;
	    i = i->n;
	}
	while ( i && idx > index ) {
	    idx--;
	    i = i->p;
	}
    } else {
	int idx = index;
	while ( i && idx > 0 ) {
	    idx--;
	    i = i->n;
	}
    }

    if ( index > 0 ) {
	d->cache = i;
	d->cacheIndex = index;
    }

    return i;
}


/*!
    Returns the index of \a lbi, or -1 if the item is not in this list
    box or \a lbi is 0.

    \sa item()
*/

int QListBox::index( const QListBoxItem * lbi ) const
{
    if ( !lbi )
	return -1;
    QListBoxItem * i_n = d->head;
    int c_n = 0;
    if ( d->cache ) {
	i_n = d->cache;
	c_n = d->cacheIndex;
    }
    QListBoxItem* i_p = i_n;
    int c_p = c_n;
    while ( ( i_n != 0 || i_p != 0 ) && i_n != lbi && i_p != lbi ) {
	if ( i_n ) {
	    c_n++;
	    i_n = i_n->n;
	}
	if ( i_p ) {
	    c_p--;
	    i_p = i_p->p;
	}
    }
    if ( i_p == lbi )
	return c_p;
    if ( i_n == lbi )
	return c_n;
    return -1;
}



/*!
    Returns TRUE if the item at position \a index is at least partly
    visible; otherwise returns FALSE.
*/

bool QListBox::itemVisible( int index )
{
    QListBoxItem * i = item( index );
    return i ? itemVisible( i ) : FALSE;
}


/*!
    \overload

    Returns TRUE if \a item is at least partly visible; otherwise
    returns FALSE.
*/

bool QListBox::itemVisible( const QListBoxItem * item )
{
    if ( d->layoutDirty )
	doLayout();

    int i = index( item );
    int col = i / numRows();
    int row = i % numRows();
    return ( d->columnPos[col] < contentsX()+visibleWidth() &&
	     d->rowPos[row] < contentsY()+visibleHeight() &&
	     d->columnPos[col+1] > contentsX() &&
	     d->rowPos[row+1] > contentsY() );
}


/*! \reimp */

void QListBox::mousePressEvent( QMouseEvent *e )
{
    mousePressEventEx( e );
}

void QListBox::mousePressEventEx( QMouseEvent *e )
{
    d->mouseInternalPress = TRUE;
    QListBoxItem * i = itemAt( e->pos() );

    if ( !i && !d->current && d->head ) {
	d->current = d->head;
	updateItem( d->head );
    }

    if ( !i && ( d->selectionMode != Single || e->button() == RightButton )
	 && !( e->state() & ControlButton ) )
	clearSelection();

    d->select = d->selectionMode == Multi ? ( i ? !i->isSelected() : FALSE ) : TRUE;
    d->pressedSelected = i && i->s;

    if ( i )
	d->selectAnchor = i;
    if ( i ) {
	switch( selectionMode() ) {
	default:
	case Single:
	    if ( !i->s || i != d->current ) {
		if ( i->isSelectable() )
		    setSelected( i, TRUE );
		else
		    setCurrentItem( i );
	    }
	    break;
	case Extended:
	    if ( i ) {
		if ( !(e->state() & QMouseEvent::ShiftButton) &&
		     !(e->state() & QMouseEvent::ControlButton) ) {
		    if ( !i->isSelected() ) {
			bool b = signalsBlocked();
			blockSignals( TRUE );
			clearSelection();
			blockSignals( b );
		    }
		    setSelected( i, TRUE );
		    d->dragging = TRUE; // always assume dragging
		} else if ( e->state() & ShiftButton ) {
		    d->pressedSelected = FALSE;
		    QListBoxItem *oldCurrent = item( currentItem() );
		    bool down = index( oldCurrent ) < index( i );

		    QListBoxItem *lit = down ? oldCurrent : i;
		    bool select = d->select;
		    bool blocked = signalsBlocked();
		    blockSignals( TRUE );
		    for ( ;; lit = lit->n ) {
			if ( !lit ) {
			    triggerUpdate( FALSE );
			    break;
			}
			if ( down && lit == i ) {
			    setSelected( i, select );
			    triggerUpdate( FALSE );
			    break;
			}
			if ( !down && lit == oldCurrent ) {
			    setSelected( oldCurrent, select );
			    triggerUpdate( FALSE );
			    break;
			}
			setSelected( lit, select );
		    }
		    blockSignals( blocked );
		    emit selectionChanged();
		} else if ( e->state() & ControlButton ) {
		    setSelected( i, !i->isSelected() );
		    d->pressedSelected = FALSE;
		}
		setCurrentItem( i );
	    }
	    break;
	case Multi:
	    //d->current = i;
	    setSelected( i, !i->s );
	    setCurrentItem( i );
	    break;
	case NoSelection:
	    setCurrentItem( i );
	    break;
	}
    } else {
	bool unselect = TRUE;
	if ( e->button() == LeftButton ) {
	    if ( d->selectionMode == Multi ||
		 d->selectionMode == Extended ) {
		d->tmpCurrent = d->current;
		d->current = 0;
		updateItem( d->tmpCurrent );
		if ( d->rubber )
		    delete d->rubber;
		d->rubber = 0;
		d->rubber = new QRect( e->x(), e->y(), 0, 0 );

		if ( d->selectionMode == Extended && !( e->state() & ControlButton ) )
		    selectAll( FALSE );
		unselect = FALSE;
	    }
	    if ( unselect && ( e->button() == RightButton ||
			       ( selectionMode() == Multi || selectionMode() == Extended ) ) )
		clearSelection();
	}
    }

    // for sanity, in case people are event-filtering or whatnot
    delete d->scrollTimer;
    d->scrollTimer = 0;
    if ( i ) {
	d->mousePressColumn = d->currentColumn;
	d->mousePressRow = d->currentRow;
    } else {
	d->mousePressColumn = -1;
	d->mousePressRow = -1;
    }
    d->ignoreMoves = FALSE;

    d->pressedItem = i;

    emit pressed( i );
    emit pressed( i, e->globalPos() );
    emit mouseButtonPressed( e->button(), i, e->globalPos() );
    if ( e->button() == RightButton )
	emit rightButtonPressed( i, e->globalPos() );
}


/*! \reimp */

void QListBox::mouseReleaseEvent( QMouseEvent *e )
{
    if ( d->selectionMode == Extended &&
	d->dragging ) {
	d->dragging = FALSE;
	if (d->current != d->pressedItem) {
	    updateSelection(); // when we drag, we get an update after we release
	}
    }

    if ( d->rubber ) {
	drawRubber();
	delete d->rubber;
	d->rubber = 0;
	d->current = d->tmpCurrent;
	updateItem( d->current );
    }
    if ( d->scrollTimer )
	mouseMoveEvent( e );
    delete d->scrollTimer;
    d->scrollTimer = 0;
    d->ignoreMoves = FALSE;

    if ( d->selectionMode == Extended &&
	 d->current == d->pressedItem &&
	 d->pressedSelected && d->current ) {
	bool block = signalsBlocked();
	blockSignals( TRUE );
	clearSelection();
	blockSignals( block );
	d->current->s = TRUE;
	emit selectionChanged();
    }

    QListBoxItem * i = itemAt( e->pos() );
    bool emitClicked = d->mousePressColumn != -1 && d->mousePressRow != -1 || !d->pressedItem;
    emitClicked = emitClicked && d->pressedItem == i;
    d->pressedItem = 0;
    d->mousePressRow = -1;
    d->mousePressColumn = -1;
    d->mouseInternalPress = FALSE;
    if ( emitClicked ) {
	emit clicked( i );
	emit clicked( i, e->globalPos() );
	emit mouseButtonClicked( e->button(), i, e->globalPos() );
	if ( e->button() == RightButton )
	    emit rightButtonClicked( i, e->globalPos() );
    }
}


/*! \reimp */

void QListBox::mouseDoubleClickEvent( QMouseEvent *e )
{
    bool ok = TRUE;
    QListBoxItem *i = itemAt( e->pos() );
    if ( !i || selectionMode() == NoSelection )
	ok = FALSE;

    d->ignoreMoves = TRUE;

    if ( d->current && ok ) {
	QListBoxItem * i = d->current;
	QString tmp = d->current->text();
	emit selected( currentItem() );
	emit selected( i );
	if ( !tmp.isNull() )
	    emit selected( tmp );
	emit doubleClicked( i );
    }
}


/*! \reimp */

void QListBox::mouseMoveEvent( QMouseEvent *e )
{
    QListBoxItem * i = itemAt( e->pos() );
    if ( i != d->highlighted ) {
	if ( i ) {
	    emit onItem( i );
	} else {
	    emit onViewport();
	}
	d->highlighted = i;
    }

    if ( d->rubber ) {
	QRect r = d->rubber->normalize();
	drawRubber();
	d->rubber->setCoords( d->rubber->x(), d->rubber->y(), e->x(), e->y() );
	doRubberSelection( r, d->rubber->normalize() );
	drawRubber();
	return;
    }

    if ( ( (e->state() & ( RightButton | LeftButton | MidButton ) ) == 0 ) ||
	 d->ignoreMoves )
	return;

    // hack to keep the combo (and what else?) working: if we get a
    // move outside the listbox without having seen a press, discard
    // it.
    if ( !QRect( 0, 0, visibleWidth(), visibleHeight() ).contains( e->pos() ) &&
	 ( d->mousePressColumn < 0 && d->mousePressRow < 0 ||
	   (e->state() == NoButton && !d->pressedItem) ) )
	return;

    // figure out in what direction to drag-select and perhaps scroll
    int dx = 0;
    int x = e->x();
    if ( x >= visibleWidth() ) {
	x = visibleWidth()-1;
	dx = 1;
    } else if ( x < 0 ) {
	x = 0;
	dx = -1;
    }
    d->mouseMoveColumn = columnAt( x + contentsX() );

    // sanitize mousePressColumn, if we got here without a mouse press event
    if ( d->mousePressColumn < 0 && d->mouseMoveColumn >= 0 )
	d->mousePressColumn = d->mouseMoveColumn;
    if ( d->mousePressColumn < 0 && d->currentColumn >= 0 )
	d->mousePressColumn = d->currentColumn;

    // if it's beyond the last column, use the last one
    if ( d->mouseMoveColumn < 0 )
	d->mouseMoveColumn = dx >= 0 ? numColumns()-1 : 0;

    // repeat for y
    int dy = 0;
    int y = e->y();
    if ( y >= visibleHeight() ) {
	y = visibleHeight()-1;
	dy = 1;
    } else if ( y < 0 ) {
	y = 0;
	dy = -1;
    }
    d->mouseMoveRow = rowAt( y + contentsY() );

    if ( d->mousePressRow < 0 && d->mouseMoveRow >= 0 )
	d->mousePressRow = d->mouseMoveRow;
    if ( d->mousePressRow < 0 && d->currentRow >= 0 )
	d->mousePressRow = d->currentRow;

    if ( d->mousePressRow < 0 )
	d->mousePressRow = rowAt( x + contentsX() );

    d->scrollPos = QPoint( dx, dy );

    if ( ( dx || dy ) && !d->scrollTimer && e->state() == LeftButton && e->button() != LeftButton ) {
	// start autoscrolling if necessary
	d->scrollTimer = new QTimer( this );
	connect( d->scrollTimer, SIGNAL(timeout()),
		 this, SLOT(doAutoScroll()) );
	d->scrollTimer->start( 100, FALSE );
	doAutoScroll();
    } else if ( !d->scrollTimer ) {
	// or just select the required bits
	updateSelection();
    }
}



void QListBox::updateSelection()
{
    if ( d->mouseMoveColumn >= 0 && d->mouseMoveRow >= 0 &&
	 d->mousePressColumn >= 0 && d->mousePressRow >= 0 ) {
	QListBoxItem * i = item( d->mouseMoveColumn * numRows() +
				 d->mouseMoveRow );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	int ind = index(i);
#endif
	if ( selectionMode() == Single || selectionMode() == NoSelection ) {
	    if ( i && ( d->mouseInternalPress || testWFlags(WType_Popup) ) )
		setCurrentItem( i );
	} else {
	    if ( d->selectionMode == Extended && (
		 ( d->current == d->pressedItem && d->pressedSelected ) ||
		(d->dirtyDrag && !d->dragging) ) ) {
		if (d->dirtyDrag && !d->dragging) // emit after dragging stops
		    d->dirtyDrag = FALSE;
		else
		    clearSelection(); // dont reset drag-selected items
		d->pressedItem = 0;
		if ( i && i->isSelectable() ) {
		    bool block = signalsBlocked();
		    blockSignals( TRUE );
		    i->s = TRUE;
		    blockSignals( block );
		    emit selectionChanged();
#if defined(QT_ACCESSIBILITY_SUPPORT)
		    QAccessible::updateAccessibility( viewport(), ind+1, QAccessible::StateChanged );
		    QAccessible::updateAccessibility( viewport(), 0, QAccessible::Selection );
		    QAccessible::updateAccessibility( viewport(), ind+1, QAccessible::SelectionAdd );
#endif
		}
		triggerUpdate( FALSE );
	    } else {
		int c = QMIN( d->mouseMoveColumn, d->mousePressColumn );
		int r = QMIN( d->mouseMoveRow, d->mousePressRow );
		int c2 = QMAX( d->mouseMoveColumn, d->mousePressColumn );
		int r2 = QMAX( d->mouseMoveRow, d->mousePressRow );
		bool changed = FALSE;
		while( c <= c2 ) {
		    QListBoxItem * i = item( c*numRows()+r );
		    int rtmp = r;
		    while( i && rtmp <= r2 ) {
			if ( (bool)i->s != (bool)d->select && i->isSelectable() ) {
			    i->s = d->select;
#if defined(QT_ACCESSIBILITY_SUPPORT)
			    QAccessible::updateAccessibility( viewport(), ind+1, QAccessible::StateChanged );
			    QAccessible::updateAccessibility( viewport(), ind+1, d->select ? QAccessible::SelectionAdd : QAccessible::SelectionRemove );
#endif
			    i->dirty = TRUE;
			    d->dirtyDrag = changed = TRUE;
			}
			i = i->n;
			rtmp++;
		    }
		    c++;
		}
		if ( changed ) {
		    if (!d->dragging) // emit after dragging stops instead
			emit selectionChanged();
#if defined(QT_ACCESSIBILITY_SUPPORT)
		    QAccessible::updateAccessibility( viewport(), 0, QAccessible::Selection );
#endif
		    triggerUpdate( FALSE );
		}
	    }
	    if ( i )
		setCurrentItem( i );
	}
    }
}

void QListBox::repaintSelection()
{
    if ( d->numColumns == 1 ) {
	for ( uint i = topItem(); itemVisible( i ) && i < count(); ++i ) {
	    QListBoxItem *it = item(i);
	    if ( !it )
		break;
	    if ( it->isSelected() )
		updateItem( it );
	}
    } else {
	for ( uint i = 0; i < count(); ++i ) {
	    QListBoxItem *it = item(i);
	    if ( !it )
		break;
	    if ( it->isSelected() )
		updateItem( it );
	}
    }
}

/*! \reimp
*/

void QListBox::contentsContextMenuEvent( QContextMenuEvent *e )
{
    if ( !receivers( SIGNAL(contextMenuRequested(QListBoxItem*,const QPoint&)) ) ) {
	e->ignore();
	return;
    }
    if ( e->reason() == QContextMenuEvent::Keyboard ) {
	QListBoxItem *i = item( currentItem() );
	if ( i ) {
	    QRect r = itemRect( i );
	    emit contextMenuRequested( i, mapToGlobal( r.topLeft() + QPoint( width() / 2, r.height() / 2 ) ) );
	}
    } else {
	QListBoxItem * i = itemAt( contentsToViewport( e->pos() ) );
	emit contextMenuRequested( i, e->globalPos() );
    }
}

/*!\reimp
*/
void QListBox::keyPressEvent( QKeyEvent *e )
{
    if ( ( e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab )
	 && e->state() & Qt::ControlButton )
	e->ignore();

    if ( count() == 0 ) {
	e->ignore();
	return;
    }

    QGuardedPtr<QListBox> selfCheck = this;

    QListBoxItem *old = d->current;
    if ( !old ) {
	setCurrentItem( d->head );
	if ( d->selectionMode == Single )
	    setSelected( d->head, TRUE );
	e->ignore();
	return;
    }

    bool selectCurrent = FALSE;
    switch ( e->key() ) {
	case Key_Up:
	    {
		d->currInputString = QString::null;
		if ( currentItem() > 0 ) {
		    setCurrentItem( currentItem() - 1 );
		    handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		}
		if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_Down:
	    {
		d->currInputString = QString::null;
		if ( currentItem() < (int)count() - 1 ) {
		    setCurrentItem( currentItem() + 1 );
		    handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		}
		if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_Left:
	    {
		d->currInputString = QString::null;
		if ( currentColumn() > 0 ) {
		    setCurrentItem( currentItem() - numRows() );
		    handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		} else if ( numColumns() > 1 && currentItem() > 0 ) {
		    int row = currentRow();
		    setCurrentItem( currentRow() - 1 + ( numColumns() - 1 ) * numRows() );

		    if ( currentItem() == -1 )
			setCurrentItem( row - 1 + ( numColumns() - 2 ) * numRows() );

		    handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		} else {
		    QApplication::sendEvent( horizontalScrollBar(), e );
		}
		if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_Right:
	    {
		d->currInputString = QString::null;
		if ( currentColumn() < numColumns()-1 ) {
		    int row = currentRow();
		    int i = currentItem();
		    QListBoxItem *it = item( i + numRows() );
		    if ( !it )
			it = item( count()-1 );
		    setCurrentItem( it );

		    if ( currentItem() == -1 ) {
			if ( row < numRows() - 1 )
			    setCurrentItem( row + 1 );
			else
			    setCurrentItem( i );
		    }

		    handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		} else if ( numColumns() > 1 && currentRow() < numRows() ) {
		    if ( currentRow() + 1 < numRows() ) {
			setCurrentItem( currentRow() + 1 );
			handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		    }
		} else {
		    QApplication::sendEvent( horizontalScrollBar(), e );
		}
		if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_Next:
	    {
		d->currInputString = QString::null;
		int i = 0;
		if ( numColumns() == 1 ) {
		    i = currentItem() + numItemsVisible();
		    i = i > (int)count() - 1 ? (int)count() - 1 : i;
		    setCurrentItem( i );
		    setBottomItem( i );
		} else {
		    // I'm not sure about this behavior...
		    if ( currentRow() == numRows() - 1 )
			i = currentItem() + numRows();
		    else
			i = currentItem() + numRows() - currentRow() - 1;
		    i = i > (int)count() - 1 ? (int)count() - 1 : i;
		    setCurrentItem( i );
		}
		handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_Prior:
	    {
		selectCurrent = TRUE;
		d->currInputString = QString::null;
		int i;
		if ( numColumns() == 1 ) {
		    i = currentItem() - numItemsVisible();
		    i = i < 0 ? 0 : i;
		    setCurrentItem( i );
		    setTopItem( i );
		} else {
		    // I'm not sure about this behavior...
		    if ( currentRow() == 0 )
			i = currentItem() - numRows();
		    else
			i = currentItem() - currentRow();
		    i = i < 0 ? 0 : i;
		    setCurrentItem( i );
		}
		handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_Space:
	    {
		selectCurrent = TRUE;
		d->currInputString = QString::null;
		toggleCurrentItem();
		if ( selectionMode() == Extended && d->current->isSelected() )
		    emit highlighted( currentItem() );
		if (selfCheck && (!( e->state() & ShiftButton ) || !d->selectAnchor))
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_Return:
	case Key_Enter:
	    {
		selectCurrent = TRUE;
		d->currInputString = QString::null;
		if ( currentItem() >= 0 && selectionMode() != NoSelection ) {
		    QString tmp = item( currentItem() )->text();
		    emit selected( currentItem());
		    emit selected( item( currentItem() ) );
		    if ( !tmp.isEmpty() )
			emit selected( tmp );
		    emit returnPressed( item( currentItem() ) );
		}
		if (selfCheck && (!( e->state() & ShiftButton ) || !d->selectAnchor))
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_Home:
	    {
		selectCurrent = TRUE;
		d->currInputString = QString::null;
		setCurrentItem( 0 );
		handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
		    d->selectAnchor = d->current;
	    }
	    break;
	case Key_End:
	    {
		selectCurrent = TRUE;
		d->currInputString = QString::null;
		int i = (int)count() - 1;
		setCurrentItem( i );
		handleItemChange( old, e->state() & ShiftButton, e->state() & ControlButton );
		if ( !( e->state() & ShiftButton ) || !d->selectAnchor )
		    d->selectAnchor = d->current;
	    }
	    break;
	default:
	    {
		if ( !e->text().isEmpty() && e->text()[ 0 ].isPrint() && count() ) {
		    int curItem = currentItem();
		    if ( curItem == -1 )
			curItem = 0;
		    if ( !d->inputTimer->isActive() ) {
			d->currInputString = e->text();
			curItem = d->findItemByName( ++curItem, d->currInputString );
		    } else {
			d->inputTimer->stop();
			d->currInputString += e->text();
			int oldCurItem = curItem;
			curItem = d->findItemByName( curItem, d->currInputString );
			if ( curItem < 0 ) {
			    curItem = d->findItemByName( ++oldCurItem, e->text() );
			    d->currInputString = e->text();
			}
		    }
		    if ( curItem >= 0 )
			setCurrentItem( curItem );
		    if ( curItem >= 0 && selectionMode() == QListBox::Extended ) {
			bool changed = FALSE;
			bool block = signalsBlocked();
			blockSignals( TRUE );
			selectAll( FALSE );
			blockSignals( block );
			QListBoxItem *i = item( curItem );
			if ( !i->s && i->isSelectable() ) {
			    changed = TRUE;
			    i->s = TRUE;
			    updateItem( i );
			}
			if ( changed )
			    emit selectionChanged();
		    }
		    d->inputTimer->start( 400, TRUE );
		} else {
		    d->currInputString = QString::null;
		    if ( e->state() & ControlButton ) {
			switch ( e->key() ) {
			    case Key_A:
				selectAll( TRUE );
				break;
			}
		    } else {
			e->ignore();
		    }
		}
	    }
    }

    if (selfCheck && selectCurrent && selectionMode() == Single &&
	d->current && !d->current->s ) {
	    updateItem( d->current );
	    setSelected( d->current, TRUE );
	}
}


/*!\reimp
*/
void QListBox::focusInEvent( QFocusEvent* )
{
    d->mousePressRow = -1;
    d->mousePressColumn = -1;
    d->inMenuMode = FALSE;
    if ( QFocusEvent::reason() != QFocusEvent::Mouse && !d->current && d->head ) {
	d->current = d->head;
	QListBoxItem *i = d->current;
	QString tmp;
	if ( i )
	    tmp = i->text();
	int tmp2 = index( i );
	emit highlighted( i );
	if ( !tmp.isNull() )
	    emit highlighted( tmp );
	emit highlighted( tmp2 );
	emit currentChanged( i );
    }
    if ( style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus, this ) )
	repaintSelection();

    if ( d->current ) {
	updateItem( currentItem() );
	QRect mfrect = itemRect( d->current );
	if ( mfrect.isValid() )
	    setMicroFocusHint( mfrect.x(), mfrect.y(), mfrect.width(), mfrect.height(), FALSE );
    }
}


/*!\reimp
*/
void QListBox::focusOutEvent( QFocusEvent* )
{
    if (style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus, this )) {
	d->inMenuMode =
	    QFocusEvent::reason() == QFocusEvent::Popup ||
 	    (qApp->focusWidget() && qApp->focusWidget()->inherits("QMenuBar"));
 	if ( !d->inMenuMode )
	    repaintSelection();
    }

    if ( d->current )
	updateItem( currentItem() );
}

/*!\reimp
*/
bool QListBox::eventFilter( QObject *o, QEvent *e )
{
    //### remove in 4.0
    return QScrollView::eventFilter( o, e );
}

/*!
    Repaints the item at position \a index in the list.
*/

void QListBox::updateItem( int index )
{
    if ( index >= 0 )
	updateItem( item( index ) );
}


/*!
    \overload

    Repaints the QListBoxItem \a i.
*/

void QListBox::updateItem( QListBoxItem * i )
{
    if ( !i )
	return;
    i->dirty = TRUE;
    d->updateTimer->start( 0, TRUE );
}


/*!
    \property QListBox::selectionMode
    \brief the selection mode of the list box

    Sets the list box's selection mode, which may be one of \c Single
    (the default), \c Extended, \c Multi or \c NoSelection.

    \sa SelectionMode
*/

void QListBox::setSelectionMode( SelectionMode mode )
{
    if ( d->selectionMode == mode )
	return;

    if ( ( selectionMode() == Multi || selectionMode() == Extended )
	 && ( mode == QListBox::Single || mode == QListBox::NoSelection ) ){
	clearSelection();
	if ( ( mode == QListBox::Single ) && currentItem() )
	    setSelected( currentItem(), TRUE );
    }

    d->selectionMode = mode;
    triggerUpdate( TRUE );
}


QListBox::SelectionMode QListBox::selectionMode() const
{
    return d->selectionMode;
}


/*!
  \obsolete
  \property QListBox::multiSelection
  \brief whether or not the list box is in Multi selection mode

  Consider using the \l QListBox::selectionMode property instead of
  this property.

  When setting this property, Multi selection mode is used if set to TRUE and
  to Single selection mode if set to FALSE.

  When getting this property, TRUE is returned if the list box is in
  Multi selection mode or Extended selection mode, and FALSE if it is
  in Single selection mode or NoSelection mode.

  \sa selectionMode
*/

bool QListBox::isMultiSelection() const
{
    return selectionMode() == Multi || selectionMode() == Extended;
}

void QListBox::setMultiSelection( bool enable )
{
    setSelectionMode( enable ? Multi : Single );
}


/*!
    Toggles the selection status of currentItem() and repaints if the
    list box is a \c Multi selection list box.

    \sa setMultiSelection()
*/

void QListBox::toggleCurrentItem()
{
    if ( selectionMode() == Single ||
	 selectionMode() == NoSelection ||
	 !d->current )
	return;

    if ( d->current->s || d->current->isSelectable() ) {
	d->current->s = !d->current->s;
	emit selectionChanged();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	int ind = index( d->current );
	QAccessible::updateAccessibility( viewport(), 0, QAccessible::Selection );
	QAccessible::updateAccessibility( viewport(), ind+1, QAccessible::StateChanged );
	QAccessible::updateAccessibility( viewport(), ind+1, d->current->s ? QAccessible::SelectionAdd : QAccessible::SelectionRemove );
#endif
    }
    updateItem( d->current );
}


/*!
    \overload

    If \a select is TRUE the item at position \a index is selected;
    otherwise the item is deselected.
*/

void QListBox::setSelected( int index, bool select )
{
    setSelected( item( index ), select );
}


/*!
    Selects \a item if \a select is TRUE or unselects it if \a select
    is FALSE, and repaints the item appropriately.

    If the list box is a \c Single selection list box and \a select is
    TRUE, setSelected() calls setCurrentItem().

    If the list box is a \c Single selection list box, \a select is
    FALSE, setSelected() calls clearSelection().

    \sa setMultiSelection(), setCurrentItem(), clearSelection(), currentItem()
*/

void QListBox::setSelected( QListBoxItem * item, bool select )
{
    if ( !item || !item->isSelectable() ||
	(bool)item->s == select || d->selectionMode == NoSelection )
	return;

    int ind = index( item );
    bool emitHighlighted = (d->current != item) ||
			   ( item->s != (uint) select && select );
    if ( selectionMode() == Single ) {
	if ( d->current != item ) {
	    QListBoxItem *o = d->current;
	    if ( d->current && d->current->s )
		d->current->s = FALSE;
	    d->current = item;
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    QAccessible::updateAccessibility( viewport(), ind+1, QAccessible::Focus );
#endif
	    d->currentColumn = ind / numRows();
	    d->currentRow = ind % numRows();

	    if ( o )
		updateItem( o );
	}
    }

    item->s = (uint)select;
    updateItem( item );

    if ( d->selectionMode == Single && select ) {
	emit selectionChanged( item );
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( viewport(), ind+1, QAccessible::StateChanged );
#endif
    }
    emit selectionChanged();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( viewport(), 0, QAccessible::Selection );
    if ( d->selectionMode != Single )
	QAccessible::updateAccessibility( viewport(), ind+1, select ? QAccessible::SelectionAdd : QAccessible::SelectionRemove );
#endif

    if ( emitHighlighted ) {
	QString tmp;
	if ( d->current )
	    tmp = d->current->text();
	int tmp2 = index( d->current );
	emit highlighted( d->current );
	if ( !tmp.isNull() )
	    emit highlighted( tmp );
	emit highlighted( tmp2 );
	emit currentChanged( d->current );
    }
}


/*!
    Returns TRUE if item \a i is selected; otherwise returns FALSE.
*/

bool QListBox::isSelected( int i ) const
{
    if ( selectionMode() == Single && i != currentItem() )
	return FALSE;

    QListBoxItem * lbi = item( i );
    if ( !lbi )
	return FALSE; // should not happen
    return lbi->s;
}


/*!
    \overload

    Returns TRUE if item \a i is selected; otherwise returns FALSE.
*/

bool QListBox::isSelected( const QListBoxItem * i ) const
{
    if ( !i )
	return FALSE;

    return i->s;
}

/*!  Returns the selected item if the list box is in
single-selection mode and an item is selected.

If no items are selected or the list box is in another selection mode
this function returns 0.

\sa setSelected() setMultiSelection()
*/

QListBoxItem* QListBox::selectedItem() const
{
    if ( d->selectionMode != Single )
	return 0;
    if ( isSelected( currentItem() ) )
	return  d->current;
    return 0;
}


/*!
    Deselects all items, if possible.

    Note that a \c Single selection list box will automatically select
    an item if it has keyboard focus.
*/

void QListBox::clearSelection()
{
    selectAll( FALSE );
}

/*!
    In \c Multi and \c Extended modes, this function sets all items to
    be selected if \a select is TRUE, and to be unselected if \a
    select is FALSE.

    In \c Single and \c NoSelection modes, this function only changes
    the selection status of currentItem().
*/

void QListBox::selectAll( bool select )
{
    if ( selectionMode() == Multi || selectionMode() == Extended ) {
	bool b = signalsBlocked();
	blockSignals( TRUE );
	for ( int i = 0; i < (int)count(); i++ )
	    setSelected( i, select );
	blockSignals( b );
	emit selectionChanged();
    } else if ( d->current ) {
	QListBoxItem * i = d->current;
	setSelected( i, select );
    }
}

/*!
    Inverts the selection. Only works in \c Multi and \c Extended
    selection mode.
*/

void QListBox::invertSelection()
{
    if ( d->selectionMode == Single ||
	 d->selectionMode == NoSelection )
	return;

    bool b = signalsBlocked();
    blockSignals( TRUE );
    for ( int i = 0; i < (int)count(); i++ )
	setSelected( i, !item( i )->isSelected() );
    blockSignals( b );
    emit selectionChanged();
}


/*!
  \obsolete
  Not used anymore; provided for binary compatibility
*/

void QListBox::emitChangedSignal( bool )
{
}


/*! \reimp */

QSize QListBox::sizeHint() const
{
    if ( cachedSizeHint().isValid() )
	return cachedSizeHint();

    constPolish();
    doLayout();

    int i=0;
    while( i < 10 &&
	   i < (int)d->columnPos.size()-1 &&
	   d->columnPos[i] < 200 )
	i++;
    int x;
    x = QMIN( 200, d->columnPos[i] +
	      2 * style().pixelMetric( QStyle::PM_DefaultFrameWidth ) );
    x = QMAX( 40, x );

    i = 0;
    while( i < 10 &&
	   i < (int)d->rowPos.size()-1 &&
	   d->rowPos[i] < 200 )
	i++;
    int y;
    y = QMIN( 200, d->rowPos[i] +
	      2 * style().pixelMetric( QStyle::PM_DefaultFrameWidth ) );
    y = QMAX( 40, y );

    QSize s( x, y );
    setCachedSizeHint( s );
    return s;
}

/*!
  \reimp
*/

QSize QListBox::minimumSizeHint() const
{
    return QScrollView::minimumSizeHint();
}


/*!
    Ensures that a single paint event will occur at the end of the
    current event loop iteration. If \a doLayout is TRUE, the layout
    is also redone.
*/

void QListBox::triggerUpdate( bool doLayout )
{
    if ( doLayout )
	d->layoutDirty = d->mustPaintAll = TRUE;
    d->updateTimer->start( 0, TRUE );
}


void QListBox::setColumnMode( LayoutMode mode )
{
    if ( mode == Variable )
	return;
    d->rowModeWins = FALSE;
    d->columnMode = mode;
    triggerUpdate( TRUE );
}


void QListBox::setColumnMode( int columns )
{
    if ( columns < 1 )
	columns = 1;
    d->columnMode = FixedNumber;
    d->numColumns = columns;
    d->rowModeWins = FALSE;
    triggerUpdate( TRUE );
}

void QListBox::setRowMode( LayoutMode mode )
{
    if ( mode == Variable )
	return;
    d->rowModeWins = TRUE;
    d->rowMode = mode;
    triggerUpdate( TRUE );
}


void QListBox::setRowMode( int rows )
{
    if ( rows < 1 )
	rows = 1;
    d->rowMode = FixedNumber;
    d->numRows = rows;
    d->rowModeWins = TRUE;
    triggerUpdate( TRUE );
}

/*!
    \property QListBox::columnMode
    \brief the column layout mode for this list box.

    setColumnMode() sets the layout mode and adjusts the number of
    displayed columns. The row layout mode automatically becomes \c
    Variable, unless the column mode is \c Variable.

    \sa setRowMode() columnMode() rowMode numColumns
*/


QListBox::LayoutMode QListBox::columnMode() const
{
    if ( d->rowModeWins )
	return Variable;
    else
	return d->columnMode;
}


/*!
    \property QListBox::rowMode
    \brief the row layout mode for this list box

    This property is normally \c Variable.

    setRowMode() sets the layout mode and adjusts the number of
    displayed rows. The column layout mode automatically becomes \c
    Variable, unless the row mode is \c Variable.

    \sa columnMode rowMode
*/


QListBox::LayoutMode QListBox::rowMode() const
{
    if ( d->rowModeWins )
	return d->rowMode;
    else
	return Variable;
}


/*!
    \property QListBox::numColumns
    \brief the number of columns in the list box

    This is normally 1, but can be different if \l
    QListBox::columnMode or \l QListBox::rowMode has been set.

    \sa columnMode rowMode numRows
*/

int QListBox::numColumns() const
{
    if ( count() == 0 )
	return 0;
    if ( !d->rowModeWins && d->columnMode == FixedNumber )
	return d->numColumns;
    doLayout();
    return d->columnPos.size()-1;
}


/*!
    \property QListBox::numRows
    \brief the number of rows in the list box.

    This is equal to the number of items in the default single-column
    layout, but can be different.

    \sa columnMode rowMode numColumns
*/

int QListBox::numRows() const
{
    if ( count() == 0 )
	return 0;
    if ( d->rowModeWins && d->rowMode == FixedNumber )
	return d->numRows;
    doLayout();
    return d->rowPos.size()-1;
}


/*!
    This function does the hard layout work. You should never need to
    call it.
*/

void QListBox::doLayout() const
{
    if ( !d->layoutDirty || d->resizeTimer->isActive() )
	return;
    constPolish();
    int c = count();
    switch( rowMode() ) {
    case FixedNumber:
	// columnMode() is known to be Variable
	tryGeometry( d->numRows, (c+d->numRows-1)/d->numRows );
	break;
    case FitToHeight:
	// columnMode() is known to be Variable
	if ( d->head ) {
	    // this is basically the FitToWidth code, but edited to use rows.
	    int maxh = 0;
	    QListBoxItem * i = d->head;
	    while ( i ) {
		int h = i->height(this);
		if ( maxh < h )
		    maxh = h;
		i = i->n;
	    }
	    int vh = viewportSize( 1, 1 ).height();
	    do {
		int rows = vh / maxh;
		if ( rows > c )
		    rows = c;
		if ( rows < 1 )
		    rows = 1;
		if ( variableHeight() && rows < c ) {
		    do {
			++rows;
			tryGeometry( rows, (c+rows-1)/rows );
		    } while ( rows <= c &&
			      d->rowPos[(int)d->rowPos.size()-1] <= vh );
		    --rows;
		}
		tryGeometry( rows, (c+rows-1)/rows );
		int nvh = viewportSize( d->columnPos[(int)d->columnPos.size()-1],
					d->rowPos[(int)d->rowPos.size()-1] ).height();
		if ( nvh < vh )
		    vh = nvh;
	    } while ( d->rowPos.size() > 2 &&
		      vh < d->rowPos[(int)d->rowPos.size()-1] );
	} else {
	    tryGeometry( 1, 1 );
	}
	break;
    case Variable:
	if ( columnMode() == FixedNumber ) {
	    tryGeometry( (count()+d->numColumns-1)/d->numColumns,
			 d->numColumns );
	} else if ( d->head ) { // FitToWidth, at least one item
	    int maxw = 0;
	    QListBoxItem * i = d->head;
	    while ( i ) {
		int w = i->width(this);
		if ( maxw < w )
		    maxw = w;
		i = i->n;
	    }
	    int vw = viewportSize( 1, 1 ).width();
	    do {
		int cols = vw / maxw;
		if ( cols > c )
		    cols = c;
		if ( cols < 1 )
		    cols = 1;
		if ( variableWidth() && cols < c ) {
		    do {
			++cols;
			tryGeometry( (c+cols-1)/cols, cols );
		    } while ( cols <= c &&
			      d->columnPos[(int)d->columnPos.size()-1] <= vw );
		    --cols;
		}
		tryGeometry( (c+cols-1)/cols, cols );
		int nvw = viewportSize( d->columnPos[(int)d->columnPos.size()-1],
					d->rowPos[(int)d->rowPos.size()-1] ).width();
		if ( nvw < vw )
		    vw = nvw;
	    } while ( d->columnPos.size() > 2 &&
		      vw < d->columnPos[(int)d->columnPos.size()-1] );
	} else {
	    tryGeometry( 1, 1 );
	}
	break;
    }

    d->layoutDirty = FALSE;
    int w = d->columnPos[(int)d->columnPos.size()-1];
    int h = d->rowPos[(int)d->rowPos.size()-1];
    QSize s( viewportSize( w, h ) );
    w = QMAX( w, s.width() );

    d->columnPosOne = d->columnPos[1];
    // extend the column for simple single-column listboxes
    if ( columnMode() == FixedNumber && d->numColumns == 1 &&
	 d->columnPos[1] < w )
	d->columnPos[1] = w;
    ((QListBox *)this)->resizeContents( w, h );
}


/*!
    Lay the items out in a \a columns by \a rows array. The array may
    be too big: doLayout() is expected to call this with the right
    values.
*/

void QListBox::tryGeometry( int rows, int columns ) const
{
    if ( columns < 1 )
	columns = 1;
    d->columnPos.resize( columns+1 );

    if ( rows < 1 )
	rows = 1;
    d->rowPos.resize( rows+1 );

    // funky hack I: dump the height/width of each column/row in
    // {column,row}Pos for later conversion to positions.
    int c;
    for( c=0; c<=columns; c++ )
	d->columnPos[c] = 0;
    int r;
    for( r=0; r<=rows; r++ )
	d->rowPos[r] = 0;
    r = c = 0;
    QListBoxItem * i = d->head;
    while ( i && c < columns ) {
	if ( i == d->current ) {
	    d->currentRow = r;
	    d->currentColumn = c;
	}

	int w = i->width(this);
	if ( d->columnPos[c] < w )
	    d->columnPos[c] = w;
	int h = i->height(this);
	if ( d->rowPos[r] < h )
	    d->rowPos[r] = h;
	i = i->n;
	r++;
	if ( r == rows ) {
	    r = 0;
	    c++;
	}
    }
    // funky hack II: if not variable {width,height}, unvariablify it.
    if ( !variableWidth() ) {
	int w = 0;
	for( c=0; c<columns; c++ )
	    if ( w < d->columnPos[c] )
		w = d->columnPos[c];
	for( c=0; c<columns; c++ )
	    d->columnPos[c] = w;
    }
    if ( !variableHeight() ) {
	int h = 0;
	for( r=0; r<rows; r++ )
	    if ( h < d->rowPos[r] )
		h = d->rowPos[r];
	for( r=0; r<rows; r++ )
	    d->rowPos[r] = h;
    }
    // repair the hacking.
    int x = 0;
    for( c=0; c<=columns; c++ ) {
	int w = d->columnPos[c];
	d->columnPos[c] = x;
	x += w;
    }
    int y = 0;
    for( r=0; r<=rows; r++ ) {
	int h = d->rowPos[r];
	d->rowPos[r] = y;
	y += h;
    }
}


/*!
    Returns the row index of the current item, or -1 if no item is the
    current item.
*/

int QListBox::currentRow() const
{
    if ( !d->current )
	return -1;
    if ( d->currentRow < 0 )
	d->layoutDirty = TRUE;
    if ( d->layoutDirty )
	doLayout();
    return d->currentRow;
}


/*!
    Returns the column index of the current item, or -1 if no item is
    the current item.
*/

int QListBox::currentColumn() const
{
    if ( !d->current )
	return -1;
    if ( d->currentColumn < 0 )
	d->layoutDirty = TRUE;
    if ( d->layoutDirty )
	doLayout();
    return d->currentColumn;
}


void QListBox::setTopItem( int index )
{
    if ( index >= (int)count() || count() == 0 )
	return;
    int col = index / numRows();
    int y = d->rowPos[index-col*numRows()];
    if ( d->columnPos[col] >= contentsX() &&
	 d->columnPos[col+1] <= contentsX() + visibleWidth() )
	setContentsPos( contentsX(), y );
    else
	setContentsPos( d->columnPos[col], y );
}

/*!
    Scrolls the list box so the item at position \a index in the list
    is displayed in the bottom row of the list box.

    \sa setTopItem()
*/

void QListBox::setBottomItem( int index )
{
    if ( index >= (int)count() || count() == 0 )
	return;
    int col = index / numRows();
    int y = d->rowPos[1+index-col*numRows()] - visibleHeight();
    if ( y < 0 )
	y = 0;
    if ( d->columnPos[col] >= contentsX() &&
	 d->columnPos[col+1] <= contentsX() + visibleWidth() )
	setContentsPos( contentsX(), y );
    else
	setContentsPos( d->columnPos[col], y );
}


/*!
    Returns the item at point \a p, specified in viewport coordinates,
    or a 0 if there is no item at \a p.

    Use contentsToViewport() to convert between widget coordinates and
    viewport coordinates.
*/

QListBoxItem * QListBox::itemAt( const QPoint& p ) const
{
    if ( d->layoutDirty )
	doLayout();
    QPoint np = p;

    // take into acount frame margin to get to viewport
    np -= viewport()->pos();
    if (!viewport()->rect().contains(np))
	return 0;

    // take into account contents position
    np = viewportToContents( np );

    int x = np.x();
    int y = np.y();

    // return 0 when y is below the last row
    if ( y > d->rowPos[ numRows() ] )
	return 0;

    int col = columnAt( x );
    int row = rowAt( y );

    QListBoxItem *i = item( col * numRows()  + row );
    if ( i && numColumns() > 1 ) {
	if ( d->columnPos[ col ] + i->width( this ) >= x )
	    return i;
    } else {
	if ( d->columnPos[ col + 1 ] >= x )
	    return i;
    }
    return 0;
}


/*!
    Ensures that the current item is visible.
*/

void QListBox::ensureCurrentVisible()
{
    if ( !d->current )
	return;

    doLayout();

    int row = currentRow();
    int column = currentColumn();
    int w = ( d->columnPos[column+1] - d->columnPos[column] ) / 2;
    int h = ( d->rowPos[row+1] - d->rowPos[row] ) / 2;
    // next four lines are Bad.  they mean that for pure left-to-right
    // languages, textual list box items are displayed better than
    // before when there is little space.  for non-textual items, or
    // other languages, it means... that you really should have enough
    // space in the first place :)
    if ( numColumns() == 1 )
	w = 0;
    if ( w*2 > viewport()->width() )
	w = viewport()->width()/2;

    ensureVisible( d->columnPos[column] + w, d->rowPos[row] + h, w, h);
}


/*! \internal */

void QListBox::doAutoScroll()
{
    if ( d->scrollPos.x() < 0 ) {
	// scroll left
	int x = contentsX() - horizontalScrollBar()->lineStep();
	if ( x < 0 )
	    x = 0;
	if ( x != contentsX() ) {
	    d->mouseMoveColumn = columnAt( x );
	    updateSelection();
	    if ( x < contentsX() )
		setContentsPos( x, contentsY() );
	}
    } else if ( d->scrollPos.x() > 0 ) {
	// scroll right
	int x = contentsX() + horizontalScrollBar()->lineStep();
	if ( x + visibleWidth() > contentsWidth() )
	    x = contentsWidth() - visibleWidth();
	if ( x != contentsX() ) {
	    d->mouseMoveColumn = columnAt( x + visibleWidth() - 1 );
	    updateSelection();
	    if ( x > contentsX() )
		setContentsPos( x, contentsY() );
	}
    }

    if ( d->scrollPos.y() < 0 ) {
	// scroll up
	int y = contentsY() - verticalScrollBar()->lineStep();
	if ( y < 0 )
	    y = 0;
	if ( y != contentsY() ) {
	    y = contentsY() - verticalScrollBar()->lineStep();
	    d->mouseMoveRow = rowAt( y );
	    updateSelection();
	}
    } else if ( d->scrollPos.y() > 0 ) {
	// scroll down
	int y = contentsY() + verticalScrollBar()->lineStep();
	if ( y + visibleHeight() > contentsHeight() )
	    y = contentsHeight() - visibleHeight();
	if ( y != contentsY() ) {
	    y = contentsY() + verticalScrollBar()->lineStep();
	    d->mouseMoveRow = rowAt(y + visibleHeight() - 1 );
	    updateSelection();
	}
    }

    if ( d->scrollPos == QPoint( 0, 0 ) ) {
	delete d->scrollTimer;
	d->scrollTimer = 0;
    }
}


/*!
    \property QListBox::topItem
    \brief the index of an item at the top of the screen.

    When getting this property and the listbox has multiple columns,
    an arbitrary item is selected and returned.

    When setting this property, the list box is scrolled so the item
    at position \e index in the list is displayed in the top row of
    the list box.
*/

int QListBox::topItem() const
{
    doLayout();

    // move rightwards to the best column
    int col = columnAt( contentsX() );
    int row = rowAt( contentsY() );
    return col * numRows() + row;
}


/*!
    \property QListBox::variableHeight
    \brief whether this list box has variable-height rows

    When the list box has variable-height rows (the default), each row
    is as high as the highest item in that row. When it has same-sized
    rows, all rows are as high as the highest item in the list box.

    \sa variableWidth
*/

bool QListBox::variableHeight() const
{
    return d->variableHeight;
}


void QListBox::setVariableHeight( bool enable )
{
    if ( (bool)d->variableHeight == enable )
	return;

    d->variableHeight = enable;
    triggerUpdate( TRUE );
}


/*!
    \property QListBox::variableWidth
    \brief whether this list box has variable-width columns

    When the list box has variable-width columns, each column is as
    wide as the widest item in that column. When it has same-sized
    columns (the default), all columns are as wide as the widest item
    in the list box.

    \sa variableHeight
*/

bool QListBox::variableWidth() const
{
    return d->variableWidth;
}


void QListBox::setVariableWidth( bool enable )
{
    if ( (bool)d->variableWidth == enable )
	return;

    d->variableWidth = enable;
    triggerUpdate( TRUE );
}


/*!
    Repaints only what really needs to be repainted.
*/
void QListBox::refreshSlot()
{
    if ( d->mustPaintAll ||
	 d->layoutDirty ) {
	d->mustPaintAll = FALSE;
	bool currentItemVisible = itemVisible( currentItem() );
	doLayout();
	if ( hasFocus() &&
	     currentItemVisible &&
	     d->currentColumn >= 0 &&
	     d->currentRow >= 0 &&
	     ( d->columnPos[d->currentColumn] < contentsX() ||
	       d->columnPos[d->currentColumn+1]>contentsX()+visibleWidth() ||
	       d->rowPos[d->currentRow] < contentsY() ||
	       d->rowPos[d->currentRow+1] > contentsY()+visibleHeight() ) )
	    ensureCurrentVisible();
	viewport()->repaint( FALSE );
	return;
    }

    QRegion r;
    int x = contentsX();
    int y = contentsY();
    int col = columnAt( x );
    int row = rowAt( y );
    int top = row;
    while( col < (int)d->columnPos.size()-1 && d->columnPos[col+1] < x )
	col++;
    while( top < (int)d->rowPos.size()-1 && d->rowPos[top+1] < y )
	top++;
    QListBoxItem * i = item( col * numRows() + row );

    while ( i && (int)col < numColumns() &&
	    d->columnPos[col] < x + visibleWidth()  ) {
	int cw = d->columnPos[col+1] - d->columnPos[col];
	while ( i && row < numRows() && d->rowPos[row] <
		y + visibleHeight() ) {
	    if ( i->dirty )
		r = r.unite( QRect( d->columnPos[col] - x, d->rowPos[row] - y,
				    cw, d->rowPos[row+1] - d->rowPos[row] ) );
	    row++;
	    i = i->n;
	}
	col++;
	if ( numColumns() > 1 ) {
	    row = top;
	    i = item( col *  numRows() + row );
	}
    }

    if ( r.isEmpty() )
	viewport()->repaint( FALSE );
    else
	viewport()->repaint( r, FALSE );
}


/*! \reimp */

void QListBox::viewportPaintEvent( QPaintEvent * e )
{
    doLayout();
    QWidget* vp = viewport();
    QPainter p( vp );
    QRegion r = e->region();

#if 0
    {
	// this stuff has been useful enough times that from now I'm
	//  leaving it in the source.
	uint i = 0;
	qDebug( "%s/%s: %i rects", className(), name(), r.rects().size() );
	while( i < r.rects().size() ) {
	    qDebug( "rect %d: %d, %d, %d, %d", i,
		   r.rects()[i].left(), r.rects()[i].top(),
		   r.rects()[i].width(), r.rects()[i].height() );
	    i++;
	}
	qDebug( "" );
    }
#endif

    int x = contentsX();
    int y = contentsY();
    int w = vp->width();
    int h = vp->height();

    int col = columnAt( x );
    int top = rowAt( y );
    int row = top;

    QListBoxItem * i = item( col*numRows() + row );

    const QColorGroup & g = colorGroup();
    p.setPen( g.text() );
    p.setBackgroundColor( backgroundBrush().color() );
    while ( i && (int)col < numColumns() && d->columnPos[col] < x + w ) {
	int cw = d->columnPos[col+1] - d->columnPos[col];
	while ( i && (int)row < numRows() && d->rowPos[row] < y + h ) {
	    int ch = d->rowPos[row+1] - d->rowPos[row];
	    QRect itemRect( d->columnPos[col]-x,  d->rowPos[row]-y, cw, ch );
	    QRegion tempRegion( itemRect );
	    QRegion itemPaintRegion( tempRegion.intersect( r  ) );
	    if ( !itemPaintRegion.isEmpty() ) {
		p.save();
		p.setClipRegion( itemPaintRegion );
		p.translate( d->columnPos[col]-x, d->rowPos[row]-y );
		paintCell( &p, row, col );
		p.restore();
		r = r.subtract( itemPaintRegion );
	    }
	    row++;
	    if ( i->dirty ) {
		// reset dirty flag only if the entire item was painted
		if ( itemPaintRegion == QRegion( itemRect ) )
		    i->dirty = FALSE;
	    }
	    i = i->n;
	}
	col++;
	if ( numColumns() > 1 ) {
	    row = top;
	    i = item( col *  numRows() + row );
	}
    }

    if ( r.isEmpty() )
	return;
    p.setClipRegion( r );
    p.fillRect( 0, 0, w, h, viewport()->backgroundBrush() );
}


/*!
    Returns the height in pixels of the item with index \a index. \a
    index defaults to 0.

    If \a index is too large, this function returns 0.
*/

int QListBox::itemHeight( int index ) const
{
    if ( index >= (int)count() || index < 0 )
	return 0;
    int r = index % numRows();
    return d->rowPos[r+1] - d->rowPos[r];
}


/*!
    Returns the index of the column at \a x, which is in the listbox's
    coordinates, not in on-screen coordinates.

    If there is no column that spans \a x, columnAt() returns -1.
*/

int QListBox::columnAt( int x ) const
{
    if ( x < 0 )
	return -1;
    if ( !d->columnPos.size() )
	return -1;
    if ( x >= d->columnPos[(int)d->columnPos.size()-1 ] )
	return numColumns() - 1;

    int col = 0;
    while( col < (int)d->columnPos.size()-1 && d->columnPos[col+1] < x )
	col++;
    return col;
}


/*!
    Returns the index of the row at \a y, which is in the listbox's
    coordinates, not in on-screen coordinates.

    If there is no row that spans \a y, rowAt() returns -1.
*/

int QListBox::rowAt( int y ) const
{
    if ( y < 0 )
	return -1;

    // find the top item, use bsearch for speed
    int l = 0;
    int r = d->rowPos.size() - 2;
    if ( r < 0 )
	return -1;
    if ( l <= d->rowPosCache && d->rowPosCache <= r ) {
	if ( d->rowPos[ QMAX( l, d->rowPosCache - 10 ) ] <= y
	     && y <= d->rowPos[ QMIN( r, d->rowPosCache + 10 ) ] ) {
	    l = QMAX( l, d->rowPosCache - 10 );
	    r = QMIN( r, d->rowPosCache + 10 );
	}
    }
    int i = ( (l+r+1) / 2 );
    while ( r - l ) {
	if ( d->rowPos[i] > y )
	    r = i -1;
	else
	    l = i;
	i = ( (l+r+1) / 2 );
    }
    d->rowPosCache = i;
    if ( d->rowPos[i] <= y && y <= d->rowPos[i+1]  )
	return  i;

    return d->count - 1;
}


/*!
    Returns the rectangle on the screen that \a item occupies in
    viewport()'s coordinates, or an invalid rectangle if \a item is 0
    or is not currently visible.
*/

QRect QListBox::itemRect( QListBoxItem *item ) const
{
    if ( d->resizeTimer->isActive() )
	return QRect( 0, 0, -1, -1 );
    if ( !item )
	return QRect( 0, 0, -1, -1 );

    int i = index( item );
    int col = i / numRows();
    int row = i % numRows();

    int x = d->columnPos[ col ] - contentsX();
    int y = d->rowPos[ row ] - contentsY();

    QRect r( x, y, d->columnPos[ col + 1 ] - d->columnPos[ col ],
		  d->rowPos[ row + 1 ] - d->rowPos[ row ] );
    if ( r.intersects( QRect( 0, 0, visibleWidth(), visibleHeight() ) ) )
	return r;
    return QRect( 0, 0, -1, -1 );
}


#ifndef QT_NO_COMPAT

/*!
  \obsolete

  Using this method is quite inefficient. We suggest to use insertItem()
  for inserting and sort() afterwards.

  Inserts \a lbi at its sorted position in the list box and returns the
  position.

  All items must be inserted with inSort() to maintain the sorting
  order. inSort() treats any pixmap (or user-defined type) as
  lexicographically less than any string.

  \sa insertItem(), sort()
*/
int QListBox::inSort( const QListBoxItem * lbi )
{
    qObsolete( "QListBox", "inSort", "insertItem" );
    if ( !lbi )
	return -1;

    QListBoxItem * i = d->head;
    int c = 0;

    while( i && i->text() < lbi->text() ) {
	i = i->n;
	c++;
    }
    insertItem( lbi, c );
    return c;
}

/*!
  \obsolete
  \overload
  Using this method is quite inefficient. We suggest to use insertItem()
  for inserting and sort() afterwards.

  Inserts a new item of \a text at its sorted position in the list box and
  returns the position.

  All items must be inserted with inSort() to maintain the sorting
  order. inSort() treats any pixmap (or user-defined type) as
  lexicographically less than any string.

  \sa insertItem(), sort()
*/
int QListBox::inSort( const QString& text )
{
    qObsolete( "QListBox", "inSort", "insertItem" );
    return inSort( new QListBoxText(text) );
}

#endif


/*! \reimp */

void QListBox::resizeEvent( QResizeEvent *e )
{
    d->layoutDirty = ( d->layoutDirty ||
		       rowMode() == FitToHeight ||
		       columnMode() == FitToWidth );

    if ( !d->layoutDirty && columnMode() == FixedNumber &&
	 d->numColumns == 1) {
	int w = d->columnPosOne;
	QSize s( viewportSize( w, contentsHeight() ) );
	w = QMAX( w, s.width() );
	d->columnPos[1] = QMAX( w, d->columnPosOne );
	resizeContents( d->columnPos[1], contentsHeight() );
    }

    if ( d->resizeTimer->isActive() )
	d->resizeTimer->stop();
    if ( d->rowMode == FixedNumber && d->columnMode == FixedNumber ) {
	bool currentItemVisible = itemVisible( currentItem() );
	doLayout();
	QScrollView::resizeEvent( e );
	if ( currentItemVisible )
	    ensureCurrentVisible();
	if ( d->current )
	    viewport()->repaint( itemRect( d->current ), FALSE );
    } else if ( ( d->columnMode == FitToWidth || d->rowMode == FitToHeight ) && !(isVisible()) ) {
	QScrollView::resizeEvent( e );
    } else if ( d->layoutDirty ) {
	d->resizeTimer->start( 100, TRUE );
	resizeContents( contentsWidth() - ( e->oldSize().width() - e->size().width() ),
			contentsHeight() - ( e->oldSize().height() - e->size().height() ) );
	QScrollView::resizeEvent( e );
    } else {
	QScrollView::resizeEvent( e );
    }
}

/*!
  \internal
*/

void QListBox::adjustItems()
{
    triggerUpdate( TRUE );
    ensureCurrentVisible();
}


/*!
    Provided for compatibility with the old QListBox. We recommend
    using QListBoxItem::paint() instead.

    Repaints the cell at \a row, \a col using painter \a p.
*/

void QListBox::paintCell( QPainter * p, int row, int col )
{
    bool drawActiveSelection = hasFocus() || d->inMenuMode ||
	!style().styleHint( QStyle::SH_ItemView_ChangeHighlightOnFocus, this );
    const QColorGroup &g = ( drawActiveSelection ? colorGroup() : palette().inactive() );

    int cw = d->columnPos[col+1] - d->columnPos[col];
    int ch = d->rowPos[row+1] - d->rowPos[row];
    QListBoxItem * i = item( col*numRows()+row );
    p->save();
    if ( i->s ) {
	if ( i->custom_highlight ) {
	    p->fillRect( 0, 0, cw, ch,
			 g.brush( QPalette::backgroundRoleFromMode( viewport()->backgroundMode() ) ) );
	    p->setPen( g.highlightedText() );
	    p->setBackgroundColor( g.highlight() );
	}
	else if ( numColumns()  == 1 ) {
	    p->fillRect( 0, 0, cw, ch, g.brush( QColorGroup::Highlight ) );
	    p->setPen( g.highlightedText() );
	    p->setBackgroundColor( g.highlight() );
	} else {
	    int iw = i->width( this );
	    p->fillRect( 0, 0, iw, ch, g.brush( QColorGroup::Highlight ) );
	    p->fillRect( iw, 0, cw - iw + 1, ch,
			 g.brush( QPalette::backgroundRoleFromMode( viewport()->backgroundMode() ) ) );
	    p->setPen( g.highlightedText() );
	    p->setBackgroundColor( g.highlight() );
	}
    } else {
	p->fillRect( 0, 0, cw, ch,
		     g.brush( QPalette::backgroundRoleFromMode( viewport()->backgroundMode() ) ) );
    }

    i->paint( p );

    if ( d->current == i && hasFocus() && !i->custom_highlight ) {
	if ( numColumns() > 1 )
	    cw = i->width( this );

	style().drawPrimitive( QStyle::PE_FocusRect, p, QRect( 0, 0, cw, ch ), g,
			       QStyle::Style_FocusAtBorder,
			       QStyleOption(i->isSelected() ? g.highlight() : g.base() ) );
    }

    p->restore();
}

/*!
    Returns the width of the widest item in the list box.
*/

long QListBox::maxItemWidth() const
{
    if ( d->layoutDirty )
	doLayout();
    long m = 0;
    int i = d->columnPos.size();
    while( i-- )
	if ( m < d->columnPos[i] )
	    m = d->columnPos[i];
    return m;
}


/*! \reimp */

void QListBox::showEvent( QShowEvent * )
{
    d->ignoreMoves = FALSE;
    d->mousePressRow = -1;
    d->mousePressColumn = -1;
    d->mustPaintAll = FALSE;
    ensureCurrentVisible();
}

#ifndef QT_NO_COMPAT

/*!
  \obsolete

  Returns the vertical pixel-coordinate in \a *yPos, of the list box
  item at position \a index in the list. Returns FALSE if the item is
  outside the visible area.
*/
bool QListBox::itemYPos( int index, int *yPos ) const
{
    qObsolete( "QListBox", "itemYPos" );
    QListBoxItem* i = item(index);
    if ( !i )
	return FALSE;
    if ( yPos )
	*yPos = i->y;
    return TRUE;
}

#endif

/*!
    \fn bool QListBoxItem::isSelected() const

    Returns TRUE if the item is selected; otherwise returns FALSE.

    \sa QListBox::isSelected(), isCurrent()
*/

/*!
  \fn bool QListBoxItem::selected() const
  \obsolete
*/

/*!
    Returns TRUE if the item is the current item; otherwise returns
    FALSE.

    \sa QListBox::currentItem(), QListBox::item(), isSelected()
*/
bool QListBoxItem::isCurrent() const
{
    return listBox() && listBox()->hasFocus() &&
	listBox()->item( listBox()->currentItem() ) == this;
}
/*!
  \fn bool QListBoxItem::current() const
  \obsolete
*/

/*!
    \fn void QListBox::centerCurrentItem()
    \obsolete

    This function does exactly the same as ensureCurrentVisible()

    \sa QListBox::ensureCurrentVisible()
*/

/*!
    Returns a pointer to the list box containing this item.
*/

QListBox * QListBoxItem::listBox() const
{
    return lbox;
}


/*!
    Removes \a item from the list box and causes an update of the
    screen display. The item is not deleted. You should normally not
    need to call this function because QListBoxItem::~QListBoxItem()
    calls it. The normal way to delete an item is with \c delete.

    \sa QListBox::insertItem()
*/
void QListBox::takeItem( const QListBoxItem * item )
{
    if ( !item || d->clearing )
	return;
    d->cache = 0;
    d->count--;
    if ( item == d->last )
	d->last = d->last->p;
    if ( item->p && item->p->n == item )
	item->p->n = item->n;
    if ( item->n && item->n->p == item )
	item->n->p = item->p;
    if ( d->head == item ) {
	d->head = item->n;
	d->currentColumn = d->currentRow = -1;
    }

    if ( d->current == item ) {
	d->current = item->n ? item->n : item->p;
	QListBoxItem *i = d->current;
	QString tmp;
	if ( i )
	    tmp = i->text();
	int tmp2 = index( i );
	emit highlighted( i );
	if ( !tmp.isNull() )
	    emit highlighted( tmp );
	emit highlighted( tmp2 );
	emit currentChanged( i );
    }
    if ( d->tmpCurrent == item )
        d->tmpCurrent = d->current;
    if ( d->selectAnchor == item )
	d->selectAnchor = d->current;

    if ( item->s )
	emit selectionChanged();
    ((QListBoxItem *)item)->lbox = 0;
    triggerUpdate( TRUE );
}

/*!
  \internal
  Finds the next item after start beginning with \a text.
*/

int QListBoxPrivate::findItemByName( int start, const QString &text )
{
    if ( start < 0 || (uint)start >= listBox->count() )
	start = 0;
    QString match = text.lower();
    if ( match.length() < 1 )
	return start;
    QString curText;
    int item = start;
    do {
	curText = listBox->text( item ).lower();
	if ( curText.startsWith( match ) )
	    return item;
	item++;
	if ( (uint)item == listBox->count() )
	    item = 0;
    } while ( item != start );
    return -1;
}

/*!
  \internal --- obsolete!
*/

void QListBox::clearInputString()
{
    d->currInputString = QString::null;
}

/*!
    Finds the first list box item that has the text \a text and
    returns it, or returns 0 of no such item could be found.
    The search starts from the current item if the current item exists,
    otherwise it starts from the first list box item.
    If \c ComparisonFlags are specified in \a compare then these flags
    are used, otherwise the default is a case-insensitive, "begins
    with" search.

    \sa Qt::StringComparisonMode
*/

QListBoxItem *QListBox::findItem( const QString &text, ComparisonFlags compare ) const
{
    if ( text.isEmpty() )
	return 0;

    if ( compare == CaseSensitive || compare == 0 )
	compare |= ExactMatch;

    QString itmtxt;
    QString comtxt = text;
    if ( ! (compare & CaseSensitive ) )
	comtxt = text.lower();

    QListBoxItem *item;
    if ( d->current )
	item = d->current;
    else
	item = d->head;

    QListBoxItem *beginsWithItem = 0;
    QListBoxItem *endsWithItem = 0;
    QListBoxItem *containsItem = 0;

    if ( item ) {
	for ( ; item; item = item->n ) {
	    if ( ! (compare & CaseSensitive) )
		itmtxt = item->text().lower();
	    else
		itmtxt = item->text();

	    if ( compare & ExactMatch && itmtxt == comtxt )
		return item;
	    if ( compare & BeginsWith && !beginsWithItem && itmtxt.startsWith( comtxt ) )
		beginsWithItem = containsItem = item;
	    if ( compare & EndsWith && !endsWithItem && itmtxt.endsWith( comtxt ) )
		endsWithItem = containsItem = item;
	    if ( compare & Contains && !containsItem && itmtxt.contains( comtxt ) )
		containsItem = item;
	}

	if ( d->current && d->head ) {
	    item = d->head;
	    for ( ; item && item != d->current; item = item->n ) {
		if ( ! (compare & CaseSensitive) )
		    itmtxt = item->text().lower();
		else
		    itmtxt = item->text();

		if ( compare & ExactMatch && itmtxt == comtxt )
		    return item;
		if ( compare & BeginsWith && !beginsWithItem && itmtxt.startsWith( comtxt ) )
		    beginsWithItem = containsItem = item;
		if ( compare & EndsWith && !endsWithItem && itmtxt.endsWith( comtxt ) )
		    endsWithItem = containsItem = item;
		if ( compare & Contains && !containsItem && itmtxt.contains( comtxt ) )
		    containsItem = item;
	    }
	}
    }

    // Obey the priorities
    if ( beginsWithItem )
	return beginsWithItem;
    else if ( endsWithItem )
	return endsWithItem;
    else if ( containsItem )
	return containsItem;
    return 0;
}

/*!
  \internal
*/

void QListBox::drawRubber()
{
    if ( !d->rubber )
	return;
    if ( !d->rubber->width() && !d->rubber->height() )
	return;
    QPainter p( viewport() );
    p.setRasterOp( NotROP );
    style().drawPrimitive( QStyle::PE_RubberBand, &p, d->rubber->normalize(),
			   colorGroup() );
    p.end();
}

/*!
  \internal
*/

void QListBox::doRubberSelection( const QRect &old, const QRect &rubber )
{
    QListBoxItem *i = d->head;
    QRect ir, pr;
    bool changed = FALSE;
    for ( ; i; i = i->n ) {
	ir = itemRect( i );
	if ( ir == QRect( 0, 0, -1, -1 ) )
	    continue;
	if ( i->isSelected() && !ir.intersects( rubber ) && ir.intersects( old ) ) {
	    i->s = FALSE;
	    pr = pr.unite( ir );
	    changed = TRUE;
	} else if ( !i->isSelected() && ir.intersects( rubber ) ) {
	    if ( i->isSelectable() ) {
		i->s = TRUE;
		pr = pr.unite( ir );
		changed = TRUE;
	    }
	}
    }
    if ( changed ) {
	emit selectionChanged();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( viewport(), 0, QAccessible::Selection );
#endif
    }
    viewport()->repaint( pr, TRUE );
}


/*!
    Returns TRUE if the user is selecting items using a rubber band
    rectangle; otherwise returns FALSE.
*/

bool QListBox::isRubberSelecting() const
{
    return d->rubber != 0;
}


/*!
    Returns the item that comes after this in the list box. If this is
    the last item, 0 is returned.

    \sa prev()
*/

QListBoxItem *QListBoxItem::next() const
{
    return n;
}

/*!
    Returns the item which comes before this in the list box. If this
    is the first item, 0 is returned.

    \sa next()
*/

QListBoxItem *QListBoxItem::prev() const
{
    return p;
}

/*!
    Returns the first item in this list box. If the list box is empty,
    returns 0.
*/

QListBoxItem *QListBox::firstItem() const
{
    return d->head;
}

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

#ifdef Q_OS_TEMP
static int _cdecl cmpListBoxItems( const void *n1, const void *n2 )
#else
static int cmpListBoxItems( const void *n1, const void *n2 )
#endif
{
    if ( !n1 || !n2 )
	return 0;

    QListBoxPrivate::SortableItem *i1 = (QListBoxPrivate::SortableItem *)n1;
    QListBoxPrivate::SortableItem *i2 = (QListBoxPrivate::SortableItem *)n2;

    return i1->item->text().localeAwareCompare( i2->item->text() );
}

#if defined(Q_C_CALLBACKS)
}
#endif

/*!
    If \a ascending is TRUE sorts the items in ascending order;
    otherwise sorts in descending order.

    To compare the items, the text (QListBoxItem::text()) of the items
    is used.
*/

void QListBox::sort( bool ascending )
{
    if ( count() == 0 )
	return;

    d->cache = 0;

    QListBoxPrivate::SortableItem *items = new QListBoxPrivate::SortableItem[ count() ];

    QListBoxItem *item = d->head;
    int i = 0;
    for ( ; item; item = item->n )
	items[ i++ ].item = item;

    qsort( items, count(), sizeof( QListBoxPrivate::SortableItem ), cmpListBoxItems );

    QListBoxItem *prev = 0;
    item = 0;
    if ( ascending ) {
	for ( i = 0; i < (int)count(); ++i ) {
	    item = items[ i ].item;
	    if ( item ) {
		item->p = prev;
		item->dirty = TRUE;
		if ( item->p )
		    item->p->n = item;
		item->n = 0;
	    }
	    if ( i == 0 )
		d->head = item;
	    prev = item;
	}
    } else {
	for ( i = (int)count() - 1; i >= 0 ; --i ) {
	    item = items[ i ].item;
	    if ( item ) {
		item->p = prev;
		item->dirty = TRUE;
		if ( item->p )
		    item->p->n = item;
		item->n = 0;
	    }
	    if ( i == (int)count() - 1 )
		d->head = item;
	    prev = item;
	}
    }
    d->last = item;

    delete [] items;

    // We have to update explicitly in case the current "vieport" overlaps the
    // new viewport we set (starting at (0,0)).
    bool haveToUpdate = contentsX() < visibleWidth() || contentsY() < visibleHeight();
    setContentsPos( 0, 0 );
    if ( haveToUpdate )
	updateContents( 0, 0, visibleWidth(), visibleHeight() );
}

void QListBox::handleItemChange( QListBoxItem *old, bool shift, bool control )
{
    if ( d->selectionMode == Single ) {
	// nothing
    } else if ( d->selectionMode == Extended ) {
	if ( shift ) {
	    selectRange( d->selectAnchor ? d->selectAnchor : old,
			 d->current, FALSE, TRUE, (d->selectAnchor && !control) ? TRUE : FALSE );
	} else if ( !control ) {
	    bool block = signalsBlocked();
	    blockSignals( TRUE );
	    selectAll( FALSE );
	    blockSignals( block );
	    setSelected( d->current, TRUE );
	}
    } else if ( d->selectionMode == Multi ) {
	if ( shift )
	    selectRange( old, d->current, TRUE, FALSE );
    }
}

void QListBox::selectRange( QListBoxItem *from, QListBoxItem *to, bool invert, bool includeFirst, bool clearSel )
{
    if ( !from || !to )
	return;
    if ( from == to && !includeFirst )
	return;
    QListBoxItem *i = 0;
    int index =0;
    int f_idx = -1, t_idx = -1;
    for ( i = d->head; i; i = i->n, index++ ) {
	if ( i == from )
	    f_idx = index;
	if ( i == to )
	    t_idx = index;
	if ( f_idx != -1 && t_idx != -1 )
	    break;
    }
    if ( f_idx > t_idx ) {
	i = from;
	from = to;
	to = i;
	if ( !includeFirst )
	    to = to->prev();
    } else {
	if ( !includeFirst )
	    from = from->next();
    }

    bool changed = FALSE;
    if ( clearSel ) {
	for ( i = d->head; i && i != from; i = i->n ) {
	    if ( i->s ) {
		i->s = FALSE;
		changed = TRUE;
		updateItem( i );
	    }
	}
	for ( i = to->n; i; i = i->n ) {
	    if ( i->s ) {
		i->s = FALSE;
		changed = TRUE;
		updateItem( i );
	    }
	}
    }

    for ( i = from; i; i = i->next() ) {
	if ( !invert ) {
	    if ( !i->s && i->isSelectable() ) {
		i->s = TRUE;
		changed = TRUE;
		updateItem( i );
	    }
	} else {
	    bool sel = !i->s;
	    if ( (bool)i->s != sel && sel && i->isSelectable() || !sel ) {
		i->s = sel;
		changed = TRUE;
		updateItem( i );
	    }
	}
	if ( i == to )
	    break;
    }
    if ( changed ) {
	emit selectionChanged();
#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( viewport(), 0, QAccessible::Selection );
#endif
    }
}

/*! \reimp */
void QListBox::windowActivationChange( bool oldActive )
{
    if ( oldActive && d->scrollTimer )
	d->scrollTimer->stop();
    if ( palette().active() != palette().inactive() )
	viewport()->update();
    QScrollView::windowActivationChange( oldActive );
}

int QListBoxItem::RTTI = 0;

/*!
    Returns 0.

    Make your derived classes return their own values for rtti(), and
    you can distinguish between listbox items. You should use values
    greater than 1000 preferably a large random number, to allow for
    extensions to this class.
*/

int QListBoxItem::rtti() const
{
    return RTTI;
}

#endif // QT_NO_LISTBOX
