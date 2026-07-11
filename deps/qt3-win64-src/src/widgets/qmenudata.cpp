/****************************************************************************
** $Id: qmenudata.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QMenuData class
**
** Created : 941128
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

#include "qmenudata.h"
#ifndef QT_NO_MENUDATA
#include "qpopupmenu.h"
#include "qmenubar.h"
#include "qapplication.h"
#include "qguardedptr.h"

class QMenuItemData {
public:
    QCustomMenuItem    *custom_item;	// custom menu item
};

class QMenuDataData {
    // attention: also defined in qmenubar.cpp and qpopupmenu.cpp
public:
    QMenuDataData();
    QGuardedPtr<QWidget> aWidget;
    int aInt;
};
QMenuDataData::QMenuDataData()
    : aInt(-1)
{}

/*!
    \class QMenuData qmenudata.h
    \brief The QMenuData class is a base class for QMenuBar and QPopupMenu.

    \ingroup misc

    QMenuData has an internal list of menu items. A menu item can have
    a text(), an \link accel() accelerator\endlink, a pixmap(), an
    iconSet(), a whatsThis() text and a popup menu (unless it is a
    separator). Menu items may optionally be \link setItemChecked()
    checked\endlink (except for separators).

    The menu item sends out an \link QMenuBar::activated()
    activated()\endlink signal when it is chosen and a \link
    QMenuBar::highlighted() highlighted()\endlink signal when it
    receives the user input focus.

    \keyword menu identifier

    Menu items are assigned the menu identifier \e id that is passed
    in insertItem() or an automatically generated identifier if \e id
    is < 0 (the default). The generated identifiers (negative
    integers) are guaranteed to be unique within the entire
    application. The identifier is used to access the menu item in
    other functions.

    Menu items can be removed with removeItem() and removeItemAt(), or
    changed with changeItem(). All menu items can be removed with
    clear(). Accelerators can be changed or set with setAccel().
    Checkable items can be checked or unchecked with setItemChecked().
    Items can be enabled or disabled using setItemEnabled() and
    connected and disconnected with connectItem() and disconnectItem()
    respectively. By default, newly created menu items are visible.
    They can be hidden (and shown again) with setItemVisible().

    Menu items are stored in a list. Use findItem() to find an item by
    its list position or by its menu identifier. (See also indexOf()
    and idAt().)

    \sa QAccel QPopupMenu QAction
*/


/*****************************************************************************
  QMenuItem member functions
 *****************************************************************************/

QMenuItem::QMenuItem()
    :ident( -1 ), iconset_data( 0 ), pixmap_data( 0 ), popup_menu( 0 ),
     widget_item( 0 ), signal_data( 0 ), is_separator( FALSE ), is_enabled( TRUE ),
     is_checked( FALSE ), is_dirty( TRUE ), is_visible( TRUE ), d( 0)
{}

QMenuItem::~QMenuItem()
{
    delete iconset_data;
    delete pixmap_data;
    delete signal_data;
    delete widget_item;
    if ( d )
	delete d->custom_item;
    delete d;
}


/*****************************************************************************
  QMenuData member functions
 *****************************************************************************/

QMenuItemData* QMenuItem::extra()
{
    if ( !d ) d = new QMenuItemData;
    return d;
}

QCustomMenuItem *QMenuItem::custom() const
{
    if ( !d ) return 0;
    return d->custom_item;
}


static int get_seq_id()
{
    static int seq_no = -2;
    return seq_no--;
}


/*!
    Constructs an empty menu data list.
*/

QMenuData::QMenuData()
{
    actItem = -1;				// no active menu item
    mitems = new QMenuItemList;			// create list of menu items
    Q_CHECK_PTR( mitems );
    mitems->setAutoDelete( TRUE );
    parentMenu = 0;				// assume top level
    isPopupMenu = FALSE;
    isMenuBar = FALSE;
    mouseBtDn = FALSE;
    badSize = TRUE;
    avoid_circularity = 0;
    actItemDown = FALSE;
    d = new QMenuDataData;
}

/*!
    Removes all menu items and disconnects any signals that have been
    connected.
*/

QMenuData::~QMenuData()
{
    delete mitems;				// delete menu item list
    delete d;
}


/*!
    Virtual function; notifies subclasses about an item with \a id
    that has been changed.
*/

void QMenuData::updateItem( int /* id */ )	// reimplemented in subclass
{
}

/*!
    Virtual function; notifies subclasses that one or more items have
    been inserted or removed.
*/

void QMenuData::menuContentsChanged()		// reimplemented in subclass
{
}

/*!
    Virtual function; notifies subclasses that one or more items have
    changed state (enabled/disabled or checked/unchecked).
*/

void QMenuData::menuStateChanged()		// reimplemented in subclass
{
}

/*!
    Virtual function; notifies subclasses that a popup menu item has
    been inserted.
*/

void QMenuData::menuInsPopup( QPopupMenu * )	// reimplemented in subclass
{
}

/*!
    Virtual function; notifies subclasses that a popup menu item has
    been removed.
*/

void QMenuData::menuDelPopup( QPopupMenu * )	// reimplemented in subclass
{
}


/*!
    Returns the number of items in the menu.
*/

uint QMenuData::count() const
{
    return mitems->count();
}



/*!
  \internal

  Internal function that insert a menu item. Called by all insert()
  functions.
*/

int QMenuData::insertAny( const QString *text, const QPixmap *pixmap,
			  QPopupMenu *popup, const QIconSet* iconset, int id, int index,
			  QWidget* widget, QCustomMenuItem* custom )
{
    if ( index < 0 ) {	// append, but not if the rightmost item is an mdi separator in the menubar
	index = mitems->count();
	if ( isMenuBar && mitems->last() && mitems->last()->widget() && mitems->last()->isSeparator() )
	    index--;
    } else if ( index > (int) mitems->count() ) { // append
	index = mitems->count();
    }
    if ( id < 0 )				// -2, -3 etc.
	id = get_seq_id();

    register QMenuItem *mi = new QMenuItem;
    Q_CHECK_PTR( mi );
    mi->ident = id;
    if ( widget != 0 ) {
	mi->widget_item = widget;
	mi->is_separator = !widget->isFocusEnabled();
    } else if ( custom != 0 ) {
	mi->extra()->custom_item = custom;
	mi->is_separator = custom->isSeparator();
	if ( iconset && !iconset->isNull() )
	    mi->iconset_data = new QIconSet( *iconset );
    } else if ( text == 0 && pixmap == 0 && popup == 0 ) {
	mi->is_separator = TRUE;		// separator
    } else {
#ifndef Q_OS_TEMP
	mi->text_data = text?*text:QString();
#else
	QString newText( *text );
	newText.truncate( newText.findRev( '\t' ) );
	mi->text_data = newText.isEmpty()?QString():newText;
#endif
#ifndef QT_NO_ACCEL
	mi->accel_key = Qt::Key_unknown;
#endif
	if ( pixmap && !pixmap->isNull() )
	    mi->pixmap_data = new QPixmap( *pixmap );
	if ( (mi->popup_menu = popup) )
	    menuInsPopup( popup );
	if ( iconset && !iconset->isNull() )
	    mi->iconset_data = new QIconSet( *iconset );
    }

    mitems->insert( index, mi );
    QPopupMenu* p = ::qt_cast<QPopupMenu*>(QMenuData::d->aWidget);
    if (p && p->isVisible() && p->mitems) {
	p->mitems->clear();
	for ( QMenuItemListIt it( *mitems ); it.current(); ++it ) {
	    if ( it.current()->id() != QMenuData::d->aInt && !it.current()->widget() )
		p->mitems->append( it.current() );
	}
    }
    menuContentsChanged();			// menu data changed
    return mi->ident;
}

/*!
    \internal

  Internal function that finds the menu item where \a popup is located,
  storing its index at \a index if \a index is not NULL.
*/
QMenuItem *QMenuData::findPopup( QPopupMenu *popup, int *index )
{
    int i = 0;
    QMenuItem *mi = mitems->first();
    while ( mi ) {
	if ( mi->popup_menu == popup )		// found popup
	    break;
	i++;
	mi = mitems->next();
    }
    if ( index && mi )
	*index = i;
    return mi;
}

void QMenuData::removePopup( QPopupMenu *popup )
{
    int index = 0;
    QMenuItem *mi = findPopup( popup, &index );
    if ( mi ) {
	mi->popup_menu = 0;
	removeItemAt( index );
    }
}


/*!
    The family of insertItem() functions inserts menu items into a
    popup menu or a menu bar.

    A menu item is usually either a text string or a pixmap, both with
    an optional icon or keyboard accelerator. For special cases it is
    also possible to insert custom items (see \l{QCustomMenuItem}) or
    even widgets into popup menus.

    Some insertItem() members take a popup menu as an additional
    argument. Use this to insert submenus into existing menus or
    pulldown menus into a menu bar.

    The number of insert functions may look confusing, but they are
    actually quite simple to use.

    This default version inserts a menu item with the text \a text,
    the accelerator key \a accel, an id and an optional index and
    connects it to the slot \a member in the object \a receiver.

    Example:
    \code
	QMenuBar   *mainMenu = new QMenuBar;
	QPopupMenu *fileMenu = new QPopupMenu;
	fileMenu->insertItem( "New",  myView, SLOT(newFile()), CTRL+Key_N );
	fileMenu->insertItem( "Open", myView, SLOT(open()),    CTRL+Key_O );
	mainMenu->insertItem( "File", fileMenu );
    \endcode

    Not all insert functions take an object/slot parameter or an
    accelerator key. Use connectItem() and setAccel() on those items.

    If you need to translate accelerators, use tr() with the text and
    accelerator. (For translations use a string \link QKeySequence key
    sequence\endlink.):
    \code
	fileMenu->insertItem( tr("Open"), myView, SLOT(open()),
			      tr("Ctrl+O") );
    \endcode

    In the example above, pressing Ctrl+O or selecting "Open" from the
    menu activates the myView->open() function.

    Some insert functions take a QIconSet parameter to specify the
    little menu item icon. Note that you can always pass a QPixmap
    object instead.

    The \a id specifies the identification number associated with the
    menu item. Note that only positive values are valid, as a negative
    value will make Qt select a unique id for the item.

    The \a index specifies the position in the menu. The menu item is
    appended at the end of the list if \a index is negative.

    Note that keyboard accelerators in Qt are not application-global,
    instead they are bound to a certain top-level window. For example,
    accelerators in QPopupMenu items only work for menus that are
    associated with a certain window. This is true for popup menus
    that live in a menu bar since their accelerators will then be
    installed in the menu bar itself. This also applies to stand-alone
    popup menus that have a top-level widget in their parentWidget()
    chain. The menu will then install its accelerator object on that
    top-level widget. For all other cases use an independent QAccel
    object.

    \warning Be careful when passing a literal 0 to insertItem()
    because some C++ compilers choose the wrong overloaded function.
    Cast the 0 to what you mean, e.g. \c{(QObject*)0}.

    \warning On Mac OS X, items that connect to a slot that are inserted into a
    menubar will not function as we use the native menubar that knows nothing
    about signals or slots. Instead insert the items into a popup menu and
    insert the popup menu into the menubar. This may be fixed in a future Qt
    version.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
    qnamespace.h
*/

int QMenuData::insertItem( const QString &text,
			   const QObject *receiver, const char* member,
			   const QKeySequence& accel, int id, int index )
{
    int actualID = insertAny( &text, 0, 0, 0, id, index );
    connectItem( actualID, receiver, member );
#ifndef QT_NO_ACCEL
    if ( accel )
	setAccel( accel, actualID );
#endif
    return actualID;
}

/*!
    \overload

    Inserts a menu item with icon \a icon, text \a text, accelerator
    \a accel, optional id \a id, and optional \a index position. The
    menu item is connected it to the \a receiver's \a member slot. The
    icon will be displayed to the left of the text in the item.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
    qnamespace.h
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QString &text,
			   const QObject *receiver, const char* member,
			   const QKeySequence& accel, int id, int index )
{
    int actualID = insertAny( &text, 0, 0, &icon, id, index );
    connectItem( actualID, receiver, member );
#ifndef QT_NO_ACCEL
    if ( accel )
	setAccel( accel, actualID );
#endif
    return actualID;
}

/*!
    \overload

    Inserts a menu item with pixmap \a pixmap, accelerator \a accel,
    optional id \a id, and optional \a index position. The menu item
    is connected it to the \a receiver's \a member slot. The icon will
    be displayed to the left of the text in the item.

    To look best when being highlighted as a menu item, the pixmap
    should provide a mask (see QPixmap::mask()).

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QPixmap &pixmap,
			   const QObject *receiver, const char* member,
			   const QKeySequence& accel, int id, int index )
{
    int actualID = insertAny( 0, &pixmap, 0, 0, id, index );
    connectItem( actualID, receiver, member );
#ifndef QT_NO_ACCEL
    if ( accel )
	setAccel( accel, actualID );
#endif
    return actualID;
}


/*!
    \overload

    Inserts a menu item with icon \a icon, pixmap \a pixmap,
    accelerator \a accel, optional id \a id, and optional \a index
    position. The icon will be displayed to the left of the pixmap in
    the item. The item is connected to the \a member slot in the \a
    receiver object.

    To look best when being highlighted as a menu item, the pixmap
    should provide a mask (see QPixmap::mask()).

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem(), QAccel,
    qnamespace.h
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QPixmap &pixmap,
			   const QObject *receiver, const char* member,
			   const QKeySequence& accel, int id, int index )
{
    int actualID = insertAny( 0, &pixmap, 0, &icon, id, index );
    connectItem( actualID, receiver, member );
#ifndef QT_NO_ACCEL
    if ( accel )
	setAccel( accel, actualID );
#endif
    return actualID;
}



/*!
    \overload

    Inserts a menu item with text \a text, optional id \a id, and
    optional \a index position.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QString &text, int id, int index )
{
    return insertAny( &text, 0, 0, 0, id, index );
}

/*!
    \overload

    Inserts a menu item with icon \a icon, text \a text, optional id
    \a id, and optional \a index position. The icon will be displayed
    to the left of the text in the item.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QString &text, int id, int index )
{
    return insertAny( &text, 0, 0, &icon, id, index );
}

/*!
    \overload

    Inserts a menu item with text \a text, submenu \a popup, optional
    id \a id, and optional \a index position.

    The \a popup must be deleted by the programmer or by its parent
    widget. It is not deleted when this menu item is removed or when
    the menu is deleted.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QString &text, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( &text, 0, popup, 0, id, index );
}

/*!
    \overload

    Inserts a menu item with icon \a icon, text \a text, submenu \a
    popup, optional id \a id, and optional \a index position. The icon
    will be displayed to the left of the text in the item.

    The \a popup must be deleted by the programmer or by its parent
    widget. It is not deleted when this menu item is removed or when
    the menu is deleted.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QString &text, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( &text, 0, popup, &icon, id, index );
}

/*!
    \overload

    Inserts a menu item with pixmap \a pixmap, optional id \a id, and
    optional \a index position.

    To look best when being highlighted as a menu item, the pixmap
    should provide a mask (see QPixmap::mask()).

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QPixmap &pixmap, int id, int index )
{
    return insertAny( 0, &pixmap, 0, 0, id, index );
}

/*!
    \overload

    Inserts a menu item with icon \a icon, pixmap \a pixmap, optional
    id \a id, and optional \a index position. The icon will be
    displayed to the left of the pixmap in the item.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QPixmap &pixmap, int id, int index )
{
    return insertAny( 0, &pixmap, 0, &icon, id, index );
}


/*!
    \overload

    Inserts a menu item with pixmap \a pixmap, submenu \a popup,
    optional id \a id, and optional \a index position.

    The \a popup must be deleted by the programmer or by its parent
    widget. It is not deleted when this menu item is removed or when
    the menu is deleted.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QPixmap &pixmap, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( 0, &pixmap, popup, 0, id, index );
}


/*!
    \overload

    Inserts a menu item with icon \a icon, pixmap \a pixmap submenu \a
    popup, optional id \a id, and optional \a index position. The icon
    will be displayed to the left of the pixmap in the item.

    The \a popup must be deleted by the programmer or by its parent
    widget. It is not deleted when this menu item is removed or when
    the menu is deleted.

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem(), changeItem(), setAccel(), connectItem()
*/

int QMenuData::insertItem( const QIconSet& icon,
			   const QPixmap &pixmap, QPopupMenu *popup,
			   int id, int index )
{
    return insertAny( 0, &pixmap, popup, &icon, id, index );
}



/*!
    \overload

    Inserts a menu item that consists of the widget \a widget with
    optional id \a id, and optional \a index position.

    Ownership of \a widget is transferred to the popup menu or to the
    menu bar.

    Theoretically, any widget can be inserted into a popup menu. In
    practice, this only makes sense with certain widgets.

    If a widget is not focus-enabled (see
    \l{QWidget::isFocusEnabled()}), the menu treats it as a separator;
    this means that the item is not selectable and will never get
    focus. In this way you can, for example, simply insert a QLabel if
    you need a popup menu with a title.

    If the widget is focus-enabled it will get focus when the user
    traverses the popup menu with the arrow keys. If the widget does
    not accept \c ArrowUp and \c ArrowDown in its key event handler,
    the focus will move back to the menu when the respective arrow key
    is hit one more time. This works with a QLineEdit, for example. If
    the widget accepts the arrow key itself, it must also provide the
    possibility to put the focus back on the menu again by calling
    QWidget::focusNextPrevChild(). Futhermore, if the embedded widget
    closes the menu when the user made a selection, this can be done
    safely by calling:
    \code
	if ( isVisible() &&
	     parentWidget() &&
	     parentWidget()->inherits("QPopupMenu") )
	    parentWidget()->close();
    \endcode

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa removeItem()
*/
int QMenuData::insertItem( QWidget* widget, int id, int index )
{
    return insertAny( 0, 0, 0, 0, id, index, widget );
}


/*!
    \overload

    Inserts a custom menu item \a custom with optional id \a id, and
    optional \a index position.

    This only works with popup menus. It is not supported for menu
    bars. Ownership of \a custom is transferred to the popup menu.

    If you want to connect a custom item to a slot, use connectItem().

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa connectItem(), removeItem(), QCustomMenuItem
*/
int QMenuData::insertItem( QCustomMenuItem* custom, int id, int index )
{
    return insertAny( 0, 0, 0, 0, id, index, 0, custom );
}

/*!
    \overload

    Inserts a custom menu item \a custom with an \a icon and with
    optional id \a id, and optional \a index position.

    This only works with popup menus. It is not supported for menu
    bars. Ownership of \a custom is transferred to the popup menu.

    If you want to connect a custom item to a slot, use connectItem().

    Returns the allocated menu identifier number (\a id if \a id >= 0).

    \sa connectItem(), removeItem(), QCustomMenuItem
*/
int QMenuData::insertItem( const QIconSet& icon, QCustomMenuItem* custom, int id, int index )
{
    return insertAny( 0, 0, 0, &icon, id, index, 0, custom );
}


/*!
    Inserts a separator at position \a index, and returns the menu identifier
    number allocated to it. The separator becomes the last menu item if
    \a index is negative.

    In a popup menu a separator is rendered as a horizontal line. In a
    Motif menu bar a separator is spacing, so the rest of the items
    (normally just "Help") are drawn right-justified. In a Windows
    menu bar separators are ignored (to comply with the Windows style
    guidelines).
*/
int QMenuData::insertSeparator( int index )
{
    return insertAny( 0, 0, 0, 0, -1, index );
}

/*!
    \fn void QMenuData::removeItem( int id )

    Removes the menu item that has the identifier \a id.

    \sa removeItemAt(), clear()
*/

void QMenuData::removeItem( int id )
{
    QMenuData *parent;
    if ( findItem( id, &parent ) )
	parent->removeItemAt(parent->indexOf(id));
}

/*!
    Removes the menu item at position \a index.

    \sa removeItem(), clear()
*/

void QMenuData::removeItemAt( int index )
{
    if ( index < 0 || index >= (int)mitems->count() ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QMenuData::removeItem: Index %d out of range", index );
#endif
	return;
    }
    QMenuItem *mi = mitems->at( index );
    if ( mi->popup_menu )
	menuDelPopup( mi->popup_menu );
    mitems->remove();
    QPopupMenu* p = ::qt_cast<QPopupMenu*>(QMenuData::d->aWidget);
    if (p && p->isVisible() && p->mitems) {
	p->mitems->clear();
	for ( QMenuItemListIt it( *mitems ); it.current(); ++it ) {
	    if ( it.current()->id() != QMenuData::d->aInt && !it.current()->widget() )
		p->mitems->append( it.current() );
	}
    }
    if ( !QApplication::closingDown() )		// avoid trouble
	menuContentsChanged();
}


/*!
    Removes all menu items.

    \sa removeItem(), removeItemAt()
*/

void QMenuData::clear()
{
    register QMenuItem *mi = mitems->first();
    while ( mi ) {
	if ( mi->popup_menu )
	    menuDelPopup( mi->popup_menu );
	mitems->remove();
	mi = mitems->current();
    }
    QPopupMenu* p = ::qt_cast<QPopupMenu*>(QMenuData::d->aWidget);
    if (p && p->isVisible() && p->mitems) {
	p->mitems->clear();
    }
    if ( !QApplication::closingDown() )		// avoid trouble
	menuContentsChanged();
}

#ifndef QT_NO_ACCEL

/*!
    Returns the accelerator key that has been defined for the menu
    item \a id, or 0 if it has no accelerator key or if there is no
    such menu item.

    \sa setAccel(), QAccel, qnamespace.h
*/

QKeySequence QMenuData::accel( int id ) const
{
    QMenuItem *mi = findItem( id );
    if ( mi )
	return mi->key();
    return QKeySequence();
}

/*!
    Sets the accelerator key for the menu item \a id to \a key.

    An accelerator key consists of a key code and a combination of the
    modifiers \c SHIFT, \c CTRL, \c ALT or \c UNICODE_ACCEL (OR'ed or
    added). The header file \c qnamespace.h contains a list of key
    codes.

    Defining an accelerator key produces a text that is added to the
    menu item; for instance, \c CTRL + \c Key_O produces "Ctrl+O". The
    text is formatted differently for different platforms.

    Note that keyboard accelerators in Qt are not application-global,
    instead they are bound to a certain top-level window. For example,
    accelerators in QPopupMenu items only work for menus that are
    associated with a certain window. This is true for popup menus
    that live in a menu bar since their accelerators will then be
    installed in the menu bar itself. This also applies to stand-alone
    popup menus that have a top-level widget in their parentWidget()
    chain. The menu will then install its accelerator object on that
    top-level widget. For all other cases use an independent QAccel
    object.

    Example:
    \code
	QMenuBar *mainMenu = new QMenuBar;
	QPopupMenu *fileMenu = new QPopupMenu;       // file sub menu
	fileMenu->insertItem( "Open Document", 67 ); // add "Open" item
	fileMenu->setAccel( CTRL + Key_O, 67 );      // Ctrl+O to open
	fileMenu->insertItem( "Quit", 69 );          // add "Quit" item
	fileMenu->setAccel( CTRL + ALT + Key_Delete, 69 ); // add Alt+Del to quit
	mainMenu->insertItem( "File", fileMenu );    // add the file menu
    \endcode

    If you need to translate accelerators, use tr() with a string:
    \code
	fileMenu->setAccel( tr("Ctrl+O"), 67 );
    \endcode

    You can also specify the accelerator in the insertItem() function.
    You may prefer to use QAction to associate accelerators with menu
    items.

    \sa accel() insertItem() QAccel QAction
*/

void QMenuData::setAccel( const QKeySequence& key, int id )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {
	mi->accel_key = key;
	parent->menuContentsChanged();
    }
}

#endif // QT_NO_ACCEL

/*!
    Returns the icon set that has been set for menu item \a id, or 0
    if no icon set has been set.

    \sa changeItem(), text(), pixmap()
*/

QIconSet* QMenuData::iconSet( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->iconSet() : 0;
}

/*!
    Returns the text that has been set for menu item \a id, or
    QString::null if no text has been set.

    \sa changeItem(), pixmap(), iconSet()
*/

QString QMenuData::text( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->text() : QString::null;
}

/*!
    Returns the pixmap that has been set for menu item \a id, or 0 if
    no pixmap has been set.

    \sa changeItem(), text(), iconSet()
*/

QPixmap *QMenuData::pixmap( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->pixmap() : 0;
}

/*!
  \fn void QMenuData::changeItem( const QString &, int )
  \obsolete

  Changes the text of the menu item \a id. If the item has an icon,
  the icon remains unchanged.

  \sa text()
*/
/*!
  \fn void QMenuData::changeItem( const QPixmap &, int )
  \obsolete

  Changes the pixmap of the menu item \a id. If the item has an icon,
  the icon remains unchanged.

  \sa pixmap()
*/

/*!
  \fn void QMenuData::changeItem( const QIconSet &, const QString &, int )
  \obsolete

  Changes the icon and text of the menu item \a id.

  \sa pixmap()
*/

/*!
    Changes the text of the menu item \a id to \a text. If the item
    has an icon, the icon remains unchanged.

    \sa text()
*/

void QMenuData::changeItem( int id, const QString &text )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {					// item found
	if ( mi->text_data == text )		// same string
	    return;
	if ( mi->pixmap_data ) {		// delete pixmap
	    delete mi->pixmap_data;
	    mi->pixmap_data = 0;
	}
	mi->text_data = text;
#ifndef QT_NO_ACCEL
	if ( !mi->accel_key && text.find( '\t' ) != -1 )
	    mi->accel_key = Qt::Key_unknown;
#endif
	parent->menuContentsChanged();
    }
}

/*!
    \overload

    Changes the pixmap of the menu item \a id to the pixmap \a pixmap.
    If the item has an icon, the icon is unchanged.

    \sa pixmap()
*/

void QMenuData::changeItem( int id, const QPixmap &pixmap )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {					// item found
	register QPixmap *i = mi->pixmap_data;
	bool fast_refresh = i != 0 &&
	    i->width() == pixmap.width() &&
	    i->height() == pixmap.height() &&
	    !mi->text();
	if ( !mi->text_data.isNull() )		// delete text
	    mi->text_data = QString::null;
	if ( !pixmap.isNull() )
	    mi->pixmap_data = new QPixmap( pixmap );
	else
	    mi->pixmap_data = 0;
	delete i; // old mi->pixmap_data, could be &pixmap
	if ( fast_refresh )
	    parent->updateItem( id );
	else
	    parent->menuContentsChanged();
    }
}

/*!
    \overload

    Changes the iconset and text of the menu item \a id to the \a icon
    and \a text respectively.

    \sa pixmap()
*/

void QMenuData::changeItem( int id, const QIconSet &icon, const QString &text )
{
    changeItem( id, text );
    changeItemIconSet( id, icon );
}

/*!
    \overload

    Changes the iconset and pixmap of the menu item \a id to \a icon
    and \a pixmap respectively.

    \sa pixmap()
*/

void QMenuData::changeItem( int id, const QIconSet &icon, const QPixmap &pixmap )
{
    changeItem( id, pixmap );
    changeItemIconSet( id, icon );
}



/*!
    Changes the icon of the menu item \a id to \a icon.

    \sa pixmap()
*/

void QMenuData::changeItemIconSet( int id, const QIconSet &icon )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {					// item found
	register QIconSet *i = mi->iconset_data;
	bool fast_refresh = i != 0;
	if ( !icon.isNull() )
	    mi->iconset_data = new QIconSet( icon );
	else
	    mi->iconset_data = 0;
	delete i; // old mi->iconset_data, could be &icon
	if ( fast_refresh )
	    parent->updateItem( id );
	else
	    parent->menuContentsChanged();
    }
}


/*!
    Returns TRUE if the item with identifier \a id is enabled;
    otherwise returns FALSE

    \sa setItemEnabled(), isItemVisible()
*/

bool QMenuData::isItemEnabled( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->isEnabled() : FALSE;
}

/*!
    If \a enable is TRUE, enables the menu item with identifier \a id;
    otherwise disables the menu item with identifier \a id.

    \sa isItemEnabled()
*/

void QMenuData::setItemEnabled( int id, bool enable )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi && (bool)mi->is_enabled != enable ) {
	mi->is_enabled = enable;
#if !defined(QT_NO_ACCEL) && !defined(QT_NO_POPUPMENU)
	if ( mi->popup() )
	    mi->popup()->enableAccel( enable );
#endif
	parent->menuStateChanged();
    }
}


/*!
    Returns TRUE if the menu item with the id \a id is currently
    active; otherwise returns FALSE.
*/
bool QMenuData::isItemActive( int id ) const
{
    if ( actItem == -1 )
	return FALSE;
    return indexOf( id ) == actItem;
}

/*!
    Returns TRUE if the menu item with the id \a id has been checked;
    otherwise returns FALSE.

    \sa setItemChecked()
*/

bool QMenuData::isItemChecked( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->isChecked() : FALSE;
}

/*!
    If \a check is TRUE, checks the menu item with id \a id; otherwise
    unchecks the menu item with id \a id. Calls
    QPopupMenu::setCheckable( TRUE ) if necessary.

    \sa isItemChecked()
*/

void QMenuData::setItemChecked( int id, bool check )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi && (bool)mi->is_checked != check ) {
	mi->is_checked = check;
#ifndef QT_NO_POPUPMENU
	if ( parent->isPopupMenu && !((QPopupMenu *)parent)->isCheckable() )
	    ((QPopupMenu *)parent)->setCheckable( TRUE );
#endif
	parent->menuStateChanged();
    }
}

/*!
  Returns TRUE if the menu item with the id \a id is  visible;
  otherwise returns FALSE.

  \sa setItemVisible()
*/

bool QMenuData::isItemVisible( int id ) const
{
    QMenuItem *mi = findItem( id );
    return mi ? mi->isVisible() : FALSE;
}

/*!
  If \a visible is TRUE, shows the menu item with id \a id; otherwise
  hides the menu item with id \a id.

  \sa isItemVisible(), isItemEnabled()
*/

void QMenuData::setItemVisible( int id, bool visible )
{
    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi && (bool)mi->is_visible != visible ) {
	mi->is_visible = visible;
	parent->menuContentsChanged();
    }
}


/*!
    Returns the menu item with identifier \a id, or 0 if there is no
    item with this identifier.

    Note that QMenuItem is an internal class, and that you should not
    need to call this function. Use the higher level functions like
    text(), pixmap() and changeItem() to get and modify menu item
    attributes instead.

    \sa indexOf()
*/

QMenuItem *QMenuData::findItem( int id ) const
{
    return findItem( id, 0 );
}


/*!
    \overload

    Returns the menu item with identifier \a id, or 0 if there is no
    item with this identifier. Changes \a *parent to point to the
    parent of the return value.

    Note that QMenuItem is an internal class, and that you should not
    need to call this function. Use the higher level functions like
    text(), pixmap() and changeItem() to get and modify menu item
    attributes instead.

    \sa indexOf()
*/

QMenuItem * QMenuData::findItem( int id, QMenuData ** parent ) const
{
    if ( parent )
	*parent = (QMenuData *)this;		// ###

    if ( id == -1 )				// bad identifier
	return 0;
    QMenuItemListIt it( *mitems );
    QMenuItem *mi;
    while ( (mi=it.current()) ) {		// search this menu
	++it;
	if ( mi->ident == id )			// found item
	    return mi;
    }
    it.toFirst();
    while ( (mi=it.current()) ) {		// search submenus
	++it;
#ifndef QT_NO_POPUPMENU
	if ( mi->popup_menu ) {
	    QPopupMenu *p = mi->popup_menu;
	    if (!p->avoid_circularity) {
		p->avoid_circularity = 1;
		mi = mi->popup_menu->findItem( id, parent );
		p->avoid_circularity = 0;
		if ( mi )				// found item
		    return mi;
	    }
	}
#endif
    }
    return 0;					// not found
}

/*!
    Returns the index of the menu item with identifier \a id, or -1 if
    there is no item with this identifier.

    \sa idAt(), findItem()
*/

int QMenuData::indexOf( int id ) const
{
    if ( id == -1 )				// bad identifier
	return -1;
    QMenuItemListIt it( *mitems );
    QMenuItem *mi;
    int index = 0;
    while ( (mi=it.current()) ) {
	if ( mi->ident == id )			// this one?
	    return index;
	++index;
	++it;
    }
    return -1;					// not found
}

/*!
    Returns the identifier of the menu item at position \a index in
    the internal list, or -1 if \a index is out of range.

    \sa setId(), indexOf()
*/

int QMenuData::idAt( int index ) const
{
    return index < (int)mitems->count() && index >= 0 ?
	   mitems->at(index)->id() : -1;
}

/*!
    Sets the menu identifier of the item at \a index to \a id.

    If \a index is out of range, the operation is ignored.

    \sa idAt()
*/

void QMenuData::setId( int index, int id )
{
    if ( index < (int)mitems->count() )
	mitems->at(index)->ident = id;
}


/*!
    Sets the parameter of the activation signal of item \a id to \a
    param.

    If any receiver takes an integer parameter, this value is passed.

    \sa connectItem(), disconnectItem(), itemParameter()
*/
bool QMenuData::setItemParameter( int id, int param ) {
    QMenuItem *mi = findItem( id );
    if ( !mi )					// no such identifier
	return FALSE;
    if ( !mi->signal_data ) {			// create new signal
	mi->signal_data = new QSignal;
	Q_CHECK_PTR( mi->signal_data );
    }
    mi->signal_data->setValue( param );
    return TRUE;
}


/*!
    Returns the parameter of the activation signal of item \a id.

    If no parameter has been specified for this item with
    setItemParameter(), the value defaults to \a id.

    \sa connectItem(), disconnectItem(), setItemParameter()
*/
int QMenuData::itemParameter( int id ) const
{
    QMenuItem *mi = findItem( id );
    if ( !mi || !mi->signal_data )
	return id;
    return mi->signal_data->value().toInt();
}


/*!
    Connects the menu item with identifier \a id to \a{receiver}'s \a
    member slot or signal.

    The receiver's slot (or signal) is activated when the menu item is
    activated.

    \sa disconnectItem(), setItemParameter()
*/

bool QMenuData::connectItem( int id, const QObject *receiver,
			     const char* member )
{
    QMenuItem *mi = findItem( id );
    if ( !mi )					// no such identifier
	return FALSE;
    if ( !mi->signal_data ) {			// create new signal
	mi->signal_data = new QSignal;
	Q_CHECK_PTR( mi->signal_data );
	mi->signal_data->setValue( id );
    }
    return mi->signal_data->connect( receiver, member );
}


/*!
    Disconnects the \a{receiver}'s \a member from the menu item with
    identifier \a id.

    All connections are removed when the menu data object is
    destroyed.

    \sa connectItem(), setItemParameter()
*/

bool QMenuData::disconnectItem( int id, const QObject *receiver,
				const char* member )
{
    QMenuItem *mi = findItem( id );
    if ( !mi || !mi->signal_data )		// no identifier or no signal
	return FALSE;
    return mi->signal_data->disconnect( receiver, member );
}

/*!
    Sets \a text as What's This help for the menu item with identifier
    \a id.

    \sa whatsThis()
*/
void QMenuData::setWhatsThis( int id, const QString& text )
{

    QMenuData *parent;
    QMenuItem *mi = findItem( id, &parent );
    if ( mi ) {
	mi->setWhatsThis( text );
	parent->menuContentsChanged();
    }
}

/*!
    Returns the What's This help text for the item with identifier \a
    id or QString::null if no text has yet been defined.

    \sa setWhatsThis()
*/
QString QMenuData::whatsThis( int id ) const
{

    QMenuItem *mi = findItem( id );
    return mi? mi->whatsThis() : QString::null;
}



/*!
    \class QCustomMenuItem qmenudata.h
    \brief The QCustomMenuItem class is an abstract base class for custom menu items in popup menus.

    \ingroup misc

    A custom menu item is a menu item that is defined by two pure
    virtual functions, paint() and sizeHint(). The size hint tells the
    menu how much space it needs to reserve for this item, and paint
    is called whenever the item needs painting.

    This simple mechanism allows you to create all kinds of
    application specific menu items. Examples are items showing
    different fonts in a word processor or menus that allow the
    selection of drawing utilities in a vector drawing program.

    A custom item is inserted into a popup menu with
    QPopupMenu::insertItem().

    By default, a custom item can also have an icon and a keyboard
    accelerator. You can reimplement fullSpan() to return TRUE if you
    want the item to span the entire popup menu width. This is
    particularly useful for labels.

    If you want the custom item to be treated just as a separator,
    reimplement isSeparator() to return TRUE.

    Note that you can insert pixmaps or bitmaps as items into a popup
    menu without needing to create a QCustomMenuItem. However, custom
    menu items offer more flexibility, and -- especially important
    with Windows style -- provide the possibility of drawing the item
    with a different color when it is highlighted.

    \link menu-example.html menu/menu.cpp\endlink shows a simple
    example how custom menu items can be used.

    Note: the current implementation of QCustomMenuItem will not
    recognize shortcut keys that are from text with ampersands. Normal
    accelerators work though.

    <img src=qpopmenu-fancy.png>

    \sa QMenuData, QPopupMenu
*/



/*!
    Constructs a QCustomMenuItem
*/
QCustomMenuItem::QCustomMenuItem()
{
}

/*!
    Destroys a QCustomMenuItem
*/
QCustomMenuItem::~QCustomMenuItem()
{
}


/*!
    Sets the font of the custom menu item to \a font.

    This function is called whenever the font in the popup menu
    changes. For menu items that show their own individual font entry,
    you want to ignore this.
*/
void QCustomMenuItem::setFont( const QFont& /* font */ )
{
}



/*!
    Returns TRUE if this item wants to span the entire popup menu
    width; otherwise returns FALSE. The default is FALSE, meaning that
    the menu may show an icon and an accelerator key for this item as
    well.
*/
bool QCustomMenuItem::fullSpan() const
{
    return FALSE;
}

/*!
    Returns TRUE if this item is just a separator; otherwise returns
    FALSE.
*/
bool QCustomMenuItem::isSeparator() const
{
    return FALSE;
}


/*!
    \fn void QCustomMenuItem::paint( QPainter* p, const QColorGroup& cg, bool act,  bool enabled, int x, int y, int w, int h );

    Paints this item. When this function is invoked, the painter \a p
    is set to a font and foreground color suitable for a menu item
    text using color group \a cg. The item is active if \a act is TRUE
    and enabled if \a enabled is TRUE. The geometry values \a x, \a y,
    \a w and \a h specify where to draw the item.

    Do not draw any background, this has already been done by the
    popup menu according to the current GUI style.
*/


/*!
    \fn QSize QCustomMenuItem::sizeHint();

    Returns the item's size hint.
*/



/*!
    Activates the menu item at position \a index.

    If the index is invalid (for example, -1), the object itself is
    deactivated.
*/
void QMenuData::activateItemAt( int index )
{
#ifndef QT_NO_MENUBAR
    if ( isMenuBar )
	( (QMenuBar*)this )->activateItemAt( index );
    else
#endif
    {
#ifndef QT_NO_POPUPMENU
    if ( isPopupMenu )
	( (QPopupMenu*)this )->activateItemAt( index );
#endif
    }
}

#endif
