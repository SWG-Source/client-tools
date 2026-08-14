/****************************************************************************
** $Id: qtoolbar.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QToolBar class
**
** Created : 980315
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

#include "qtoolbar.h"
#ifndef QT_NO_TOOLBAR

#include "qmainwindow.h"
#include "qtooltip.h"
#include "qcursor.h"
#include "qlayout.h"
#include "qframe.h"
#include "qobjectlist.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qtoolbutton.h"
#include "qpopupmenu.h"
#include "qcombobox.h"
#include "qtimer.h"
#include "qwidgetlist.h"
#include "qstyle.h"

static const char * const arrow_v_xpm[] = {
    "7 9 3 1",
    "	    c None",
    ".	    c #000000",
    "+	    c none",
    ".+++++.",
    "..+++..",
    "+..+..+",
    "++...++",
    ".++.++.",
    "..+++..",
    "+..+..+",
    "++...++",
    "+++.+++"};

static const char * const arrow_h_xpm[] = {
    "9 7 3 1",
    "	    c None",
    ".	    c #000000",
    "+	    c none",
    "..++..+++",
    "+..++..++",
    "++..++..+",
    "+++..++..",
    "++..++..+",
    "+..++..++",
    "..++..+++"};

class QToolBarExtensionWidget;

class QToolBarPrivate
{
public:
    QToolBarPrivate() : moving( FALSE ) {
    }

    bool moving;
    QToolBarExtensionWidget *extension;
    QPopupMenu *extensionPopup;
};


class QToolBarSeparator : public QWidget
{
    Q_OBJECT
public:
    QToolBarSeparator( Orientation, QToolBar *parent, const char* name=0 );

    QSize sizeHint() const;
    Orientation orientation() const { return orient; }
public slots:
    void setOrientation( Orientation );
protected:
    void styleChange( QStyle& );
    void paintEvent( QPaintEvent * );

private:
    Orientation orient;
};

class QToolBarExtensionWidget : public QWidget
{
    Q_OBJECT

public:
    QToolBarExtensionWidget( QWidget *w );
    void setOrientation( Orientation o );
    QToolButton *button() const { return tb; }

protected:
    void resizeEvent( QResizeEvent *e ) {
	QWidget::resizeEvent( e );
	layOut();
    }

private:
    void layOut();
    QToolButton *tb;
    Orientation orient;

};

QToolBarExtensionWidget::QToolBarExtensionWidget( QWidget *w )
    : QWidget( w, "qt_dockwidget_internal" )
{
    tb = new QToolButton( this, "qt_toolbar_ext_button" );
    tb->setAutoRaise( TRUE );
    setOrientation( Horizontal );
}

void QToolBarExtensionWidget::setOrientation( Orientation o )
{
    orient = o;
    if ( orient == Horizontal )
	tb->setPixmap( QPixmap( (const char **)arrow_h_xpm ) );
    else
	tb->setPixmap( QPixmap( (const char **)arrow_v_xpm ) );
    layOut();
}

void QToolBarExtensionWidget::layOut()
{
    tb->setGeometry( 2, 2, width() - 4, height() - 4 );
}

QToolBarSeparator::QToolBarSeparator(Orientation o , QToolBar *parent,
				     const char* name )
    : QWidget( parent, name )
{
    connect( parent, SIGNAL(orientationChanged(Orientation)),
	     this, SLOT(setOrientation(Orientation)) );
    setOrientation( o );
    setBackgroundMode( parent->backgroundMode() );
    setBackgroundOrigin( ParentOrigin );
    setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
}



void QToolBarSeparator::setOrientation( Orientation o )
{
    orient = o;
}

void QToolBarSeparator::styleChange( QStyle& )
{
    setOrientation( orient );
}

QSize QToolBarSeparator::sizeHint() const
{
    int extent = style().pixelMetric( QStyle::PM_DockWindowSeparatorExtent,
				      this );
    if ( orient == Horizontal )
	return QSize( extent, 0 );
    else
	return QSize( 0, extent );
}

void QToolBarSeparator::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    QStyle::SFlags flags = QStyle::Style_Default;

    if ( orientation() == Horizontal )
	flags |= QStyle::Style_Horizontal;

    style().drawPrimitive( QStyle::PE_DockWindowSeparator, &p, rect(),
			   colorGroup(), flags );
}

#include "qtoolbar.moc"


/*!
    \class QToolBar qtoolbar.h
    \brief The QToolBar class provides a movable panel containing
    widgets such as tool buttons.

    \ingroup application
    \mainclass

    A toolbar is a panel that contains a set of controls, usually
    represented by small icons. It's purpose is to provide quick
    access to frequently used commands or options. Within a
    QMainWindow the user can drag toolbars within and between the
    \link QDockArea dock areas\endlink. Toolbars can also be dragged
    out of any dock area to float freely as top-level windows.

    QToolBar is a specialization of QDockWindow, and so provides all
    the functionality of a QDockWindow.

    To use QToolBar you simply create a QToolBar as a child of a
    QMainWindow, create a number of QToolButton widgets (or other
    widgets) in left to right (or top to bottom) order and call
    addSeparator() when you want a separator. When a toolbar is
    floated the caption used is the label given in the constructor
    call. This can be changed with setLabel().

    \quotefile action/application.cpp
    \skipto new QToolBar
    \printuntil fileSaveAction

    This extract from the \l application/application.cpp example shows
    the creation of a new toolbar as a child of a QMainWindow and
    adding two QActions.

    You may use most widgets within a toolbar, with QToolButton and
    QComboBox being the most common.

    If you create a new widget on an already visible QToolBar, this
    widget will automatically become visible without needing a show()
    call. (This differs from every other Qt widget container. We
    recommend calling show() anyway since we hope to fix this anomaly
    in a future release.)

    QToolBars, like QDockWindows, are located in \l{QDockArea}s or
    float as top-level windows. QMainWindow provides four QDockAreas
    (top, left, right and bottom). When you create a new toolbar (as
    in the example above) as a child of a QMainWindow the toolbar will
    be added to the top dock area. You can move it to another dock
    area (or float it) by calling QMainWindow::moveDockWindow(). QDock
    areas lay out their windows in \link qdockarea.html#lines
    Lines\endlink.

    If the main window is resized so that the area occupied by the
    toolbar is too small to show all its widgets a little arrow button
    (which looks like a right-pointing chevron, '&#187;') will appear
    at the right or bottom of the toolbar depending on its
    orientation. Clicking this button pops up a menu that shows the
    'overflowing' items. QToolButtons are represented in the menu using
    their textLabel property, other QButton subclasses are represented
    using their text property, and QComboBoxes are represented as submenus,
    with the caption text being used in the submenu item.

    Usually a toolbar will get precisely the space it needs. However,
    with setHorizontalStretchable(), setVerticalStretchable() or
    setStretchableWidget() you can tell the main window to expand the
    toolbar to fill all available space in the specified orientation.

    The toolbar arranges its buttons either horizontally or vertically
    (see orientation() for details). Generally, QDockArea will set the
    orientation correctly for you, but you can set it yourself with
    setOrientation() and track any changes by connecting to the
    orientationChanged() signal.

    You can use the clear() method to remove all items from a toolbar.

    \img qdockwindow.png Toolbar (dock window)
    \caption A floating QToolbar (dock window)

    \sa QToolButton QMainWindow \link http://www.iarchitect.com/visual.htm Parts of Isys on Visual Design\endlink \link guibooks.html#fowler GUI Design Handbook: Tool Bar\endlink.
*/

/*!
    \fn QToolBar::QToolBar( const QString &label,
	      QMainWindow *, ToolBarDock = Top,
	      bool newLine = FALSE, const char * name = 0 );
    \obsolete
*/

/*!
    Constructs an empty toolbar.

    The toolbar is called \a name and is a child of \a parent and is
    managed by \a parent. It is initially located in dock area \a dock
    and is labeled \a label. If \a newLine is TRUE the toolbar will be
    placed on a new line in the dock area.
*/

QToolBar::QToolBar( const QString &label,
		    QMainWindow * parent, QMainWindow::ToolBarDock dock,
		    bool newLine, const char * name )
    : QDockWindow( InDock, parent, name, 0, TRUE )
{
    mw = parent;
    init();

    if ( parent )
	parent->addToolBar( this, label, dock, newLine );
}


/*!
    Constructs an empty horizontal toolbar.

    The toolbar is called \a name and is a child of \a parent and is
    managed by \a mainWindow. The \a label and \a newLine parameters
    are passed straight to QMainWindow::addDockWindow(). \a name and
    the widget flags \a f are passed on to the QDockWindow constructor.

    Use this constructor if you want to create torn-off (undocked,
    floating) toolbars or toolbars in the \link QStatusBar status
    bar\endlink.
*/

QToolBar::QToolBar( const QString &label, QMainWindow * mainWindow,
		    QWidget * parent, bool newLine, const char * name,
		    WFlags f )
    : QDockWindow( InDock, parent, name, f, TRUE )
{
    mw = mainWindow;
    init();

    clearWFlags( WType_Dialog | WStyle_Customize | WStyle_NoBorder );
    reparent( parent, QPoint( 0, 0 ), FALSE );

    if ( mainWindow )
	mainWindow->addToolBar( this, label, QMainWindow::DockUnmanaged, newLine );
}


/*!
    \overload

    Constructs an empty toolbar called \a name, with parent \a parent,
    in its \a parent's top dock area, without any label and without
    requiring a newline.
*/

QToolBar::QToolBar( QMainWindow * parent, const char * name )
    : QDockWindow( InDock, parent, name, 0, TRUE )
{
    mw = parent;
    init();

    if ( parent )
	parent->addToolBar( this, QString::null, QMainWindow::DockTop );
}

/*!
    \internal

  Common initialization code. Requires that \c mw and \c o are set.
  Does not call QMainWindow::addDockWindow().
*/
void QToolBar::init()
{
    d = new QToolBarPrivate;
    d->extension = 0;
    d->extensionPopup = 0;
    sw = 0;

    setBackgroundMode( PaletteButton);
    setFocusPolicy( NoFocus );
    setFrameStyle( QFrame::ToolBarPanel | QFrame::Raised);
    boxLayout()->setSpacing(style().pixelMetric(QStyle::PM_ToolBarItemSpacing));
}

/*!
    \reimp
*/

QToolBar::~QToolBar()
{
    delete d;
    d = 0;
}

/*!
    \reimp
*/

void QToolBar::setOrientation( Orientation o )
{
    QDockWindow::setOrientation( o );
    if (d->extension)
	d->extension->setOrientation( o );
    QObjectList *childs = queryList( "QToolBarSeparator" );
    if ( childs ) {
        QObject *ob = 0;
	for ( ob = childs->first(); ob; ob = childs->next() ) {
	    QToolBarSeparator* w = (QToolBarSeparator*)ob;
	    w->setOrientation( o );
        }
    }
    delete childs;
}

/*!
    Adds a separator to the right/bottom of the toolbar.
*/

void QToolBar::addSeparator()
{
    (void) new QToolBarSeparator( orientation(), this, "toolbar separator" );
}

/*!
    \reimp
*/

void QToolBar::styleChange( QStyle& )
{
    QObjectList *childs = queryList( "QWidget" );
    if ( childs ) {
        QObject *ob = 0;
	for ( ob = childs->first(); ob; ob = childs->next() ) {
	    QWidget *w = (QWidget*)ob;
            if ( ::qt_cast<QToolButton*>(w) || ::qt_cast<QToolBarSeparator*>(w) )
                w->setStyle( &style() );
        }
    }
    delete childs;
    boxLayout()->setSpacing(style().pixelMetric(QStyle::PM_ToolBarItemSpacing));
}

/*!
    \reimp.
*/

void QToolBar::show()
{
    QDockWindow::show();
    if ( mw )
	mw->triggerLayout( FALSE );
    checkForExtension( size() );
}


/*!
    \reimp
*/

void QToolBar::hide()
{
    QDockWindow::hide();
    if ( mw )
	mw->triggerLayout( FALSE );
}

/*!
    Returns a pointer to the QMainWindow which manages this toolbar.
*/

QMainWindow * QToolBar::mainWindow() const
{
    return mw;
}


/*!
    Sets the widget \a w to be expanded if this toolbar is requested
    to stretch.

    The request to stretch might occur because QMainWindow
    right-justifies the dock area the toolbar is in, or because this
    toolbar's isVerticalStretchable() or isHorizontalStretchable() is
    set to TRUE.

    If you call this function and the toolbar is not yet stretchable,
    setStretchable() is called.

    \sa QMainWindow::setRightJustification(), setVerticalStretchable(),
    setHorizontalStretchable()
*/

void QToolBar::setStretchableWidget( QWidget * w )
{
    sw = w;
    boxLayout()->setStretchFactor( w, 1 );

    if ( !isHorizontalStretchable() && !isVerticalStretchable() ) {
	if ( orientation() == Horizontal )
	    setHorizontalStretchable( TRUE );
	else
	    setVerticalStretchable( TRUE );
    }
}


/*!
    \reimp
*/

bool QToolBar::event( QEvent * e )
{
    bool r =  QDockWindow::event( e );
    // After the event filters have dealt with it, do our stuff.
    if ( e->type() == QEvent::ChildInserted ) {
	QObject * child = ((QChildEvent*)e)->child();
	if ( child && child->isWidgetType() && !((QWidget*)child)->isTopLevel()
	     && child->parent() == this
	     && qstrcmp("qt_dockwidget_internal", child->name()) != 0 ) {
	    boxLayout()->addWidget( (QWidget*)child );
	    if ( isVisible() ) {
		if ( ((QWidget*)child)->testWState( WState_CreatedHidden ) )
		    ((QWidget*)child)->show();
		checkForExtension( size() );
	    }
	}
	if ( child && child->isWidgetType() && ((QWidget*)child) == sw )
	    boxLayout()->setStretchFactor( (QWidget*)child, 1 );
    } else if ( e->type() == QEvent::Show ) {
	layout()->activate();
    } else if ( e->type() == QEvent::LayoutHint && place() == OutsideDock ) {
	adjustSize();
    }
    return r;
}


/*!
    \property QToolBar::label
    \brief the toolbar's label.

    If the toolbar is floated the label becomes the toolbar window's
    caption. There is no default label text.
*/

void QToolBar::setLabel( const QString & label )
{
    l = label;
    setCaption( l );
}

QString QToolBar::label() const
{
    return l;
}


/*!
    Deletes all the toolbar's child widgets.
*/

void QToolBar::clear()
{
    if ( !children() )
	return;
    QObjectListIt it( *children() );
    QObject * obj;
    while( (obj=it.current()) != 0 ) {
	++it;
	if ( obj->isWidgetType() &&
	     qstrcmp( "qt_dockwidget_internal", obj->name() ) != 0 )
	    delete obj;
    }
}

/*!
    \reimp
*/

QSize QToolBar::minimumSize() const
{
    if ( orientation() == Horizontal )
	return QSize( 0, QDockWindow::minimumSize().height() );
    return QSize( QDockWindow::minimumSize().width(), 0 );
}

/*!
    \reimp
*/

QSize QToolBar::minimumSizeHint() const
{
    if ( orientation() == Horizontal )
	return QSize( 0, QDockWindow::minimumSizeHint().height() );
    return QSize( QDockWindow::minimumSizeHint().width(), 0 );
}

void QToolBar::createPopup()
{
    if (!d->extensionPopup) {
	d->extensionPopup = new QPopupMenu( this, "qt_dockwidget_internal" );
	connect( d->extensionPopup, SIGNAL( aboutToShow() ), this, SLOT( createPopup() ) );
    }

    if (!d->extension) {
	d->extension = new QToolBarExtensionWidget( this );
	d->extension->setOrientation(orientation());
	d->extension->button()->setPopup( d->extensionPopup );
	d->extension->button()->setPopupDelay( -1 );
    }

    d->extensionPopup->clear();
    // clear doesn't delete submenus, so do this explicitly
    QObjectList *childlist = d->extensionPopup->queryList( "QPopupMenu", 0, FALSE, TRUE );
    childlist->setAutoDelete(TRUE);
    delete childlist;

    childlist = queryList( "QWidget", 0, FALSE, TRUE );
    QObjectListIt it( *childlist );
    bool hide = FALSE;
    bool doHide = FALSE;
    int id;
    while ( it.current() ) {
        int j = 2;
        if ( !it.current()->isWidgetType() || it.current() == d->extension->button() ||
	    qstrcmp( "qt_dockwidget_internal", it.current()->name() ) == 0 ) {
	    ++it;
	    continue;
	}
	QWidget *w = (QWidget*)it.current();
#ifndef QT_NO_COMBOBOX
	if ( ::qt_cast<QComboBox*>(w) )
	    j = 1;
#endif
	hide = FALSE;
	QPoint p = w->parentWidget()->mapTo( this, w->geometry().bottomRight() );
	if ( orientation() == Horizontal ) {
	    if ( p.x() > ( doHide ? width() - d->extension->width() / j : width() ) )
	        hide = TRUE;
	} else {
	    if ( p.y() > ( doHide ? height()- d->extension->height() / j : height() ) )
	        hide = TRUE;
	}
	if ( hide && w->isVisible() ) {
	    doHide = TRUE;
	    if ( ::qt_cast<QToolButton*>(w) ) {
		QToolButton *b = (QToolButton*)w;
	        QString s = b->textLabel();
	        if ( s.isEmpty() )
		    s = b->text();
	        if ( b->popup() && b->popupDelay() <= 0 )
		    id = d->extensionPopup->insertItem( b->iconSet(), s, b->popup() );
		else
		    id = d->extensionPopup->insertItem( b->iconSet(), s, b, SLOT( emulateClick() ) ) ;
	        if ( b->isToggleButton() )
		    d->extensionPopup->setItemChecked( id, b->isOn() );
		if ( !b->isEnabled() )
		    d->extensionPopup->setItemEnabled( id, FALSE );
	    } else if ( ::qt_cast<QButton*>(w) ) {
		QButton *b = (QButton*)w;
		QString s = b->text();
		if ( s.isEmpty() )
		    s = "";
		if ( b->pixmap() )
		    id = d->extensionPopup->insertItem( *b->pixmap(), s, b, SLOT( emulateClick() ) );
		else
		    id = d->extensionPopup->insertItem( s, b, SLOT( emulateClick() ) );
		if ( b->isToggleButton() )
		    d->extensionPopup->setItemChecked( id, b->isOn() );
		if ( !b->isEnabled() )
		    d->extensionPopup->setItemEnabled( id, FALSE );
#ifndef QT_NO_COMBOBOX
	    } else if ( ::qt_cast<QComboBox*>(w) ) {
		QComboBox *c = (QComboBox*)w;
		if ( c->count() != 0 ) {
#ifndef QT_NO_WIDGET_TOPEXTRA
		    QString s = c->caption();
#else
		    QString s;
#endif
		    if ( s.isEmpty() )
		        s = c->currentText();
		    uint maxItems = 0;
		    QPopupMenu *cp = new QPopupMenu(d->extensionPopup);
                    cp->setEnabled(c->isEnabled());
		    d->extensionPopup->insertItem( s, cp );
		    connect( cp, SIGNAL( activated(int) ), c, SLOT( internalActivate(int) ) );
		    for ( int i = 0; i < c->count(); ++i ) {
		        QString tmp = c->text( i );
			cp->insertItem( tmp, i );
		        if ( c->currentText() == tmp )
			    cp->setItemChecked( i, TRUE );
			if ( !maxItems ) {
			    if ( cp->count() == 10 ) {
			        int h = cp->sizeHint().height();
			        maxItems = QApplication::desktop()->height() * 10 / h;
			    }
			} else if ( cp->count() >= maxItems - 1 ) {
			    QPopupMenu* sp = new QPopupMenu(d->extensionPopup);
			    cp->insertItem( tr( "More..." ), sp );
			    cp = sp;
			    connect( cp, SIGNAL( activated(int) ), c, SLOT( internalActivate(int) ) );
			}
		    }
		}
#endif //QT_NO_COMBOBOX
	    }
	}
        ++it;
    }
    delete childlist;
}


/*!
    \reimp
*/

void QToolBar::resizeEvent( QResizeEvent *e )
{
    checkForExtension( e->size() );
}

void QToolBar::checkForExtension( const QSize &sz )
{
    if (!isVisible())
	return;

    bool tooSmall;
    if ( orientation() == Horizontal )
	tooSmall = sz.width() < sizeHint().width();
    else
	tooSmall = sz.height() < sizeHint().height();

    if ( tooSmall ) {
	createPopup();
	if ( d->extensionPopup->count() ) {
	    if ( orientation() == Horizontal )
	        d->extension->setGeometry( width() - 20, 1, 20, height() - 2 );
	    else
	        d->extension->setGeometry( 1, height() - 20, width() - 2, 20 );
	    d->extension->show();
	    d->extension->raise();
	} else {
	    delete d->extension;
	    d->extension = 0;
	    delete d->extensionPopup;
	    d->extensionPopup = 0;
	}
    } else {
	delete d->extension;
	d->extension = 0;
	delete d->extensionPopup;
	d->extensionPopup = 0;
    }
}


/*!
    \reimp
*/

void QToolBar::setMinimumSize( int, int )
{
}

/* from chaunsee:

1.  Tool Bars should contain only high-frequency functions.  Avoid putting
things like About and Exit on a tool bar unless they are frequent functions.

2.  All tool bar buttons must have some keyboard access method (it can be a
menu or shortcut key or a function in a dialog box that can be accessed
through the keyboard).

3.  Make tool bar functions as efficient as possible (the common example is to
Print in Microsoft applications, it doesn't bring up the Print dialog box, it
prints immediately to the default printer).

4.  Avoid turning tool bars into graphical menu bars.  To me, a tool bar should
be efficient. Once you make almost all the items in a tool bar into graphical
pull-down menus, you start to lose efficiency.

5.  Make sure that adjacent icons are distinctive. There are some tool bars
where you see a group of 4-5 icons that represent related functions, but they
are so similar that you can't differentiate among them.	 These tool bars are
often a poor attempt at a "common visual language".

6.  Use any de facto standard icons of your platform (for windows use the
cut, copy and paste icons provided in dev kits rather than designing your
own).

7.  Avoid putting a highly destructive tool bar button (delete database) by a
safe, high-frequency button (Find) -- this will yield 1-0ff errors).

8.  Tooltips in many Microsoft products simply reiterate the menu text even
when that is not explanatory.  Consider making your tooltips slightly more
verbose and explanatory than the corresponding menu item.

9.  Keep the tool bar as stable as possible when you click on different
objects. Consider disabling tool bar buttons if they are used in most, but not
all contexts.

10.  If you have multiple tool bars (like the Microsoft MMC snap-ins have),
put the most stable tool bar to at the left with less stable ones to the
right. This arrangement (stable to less stable) makes the tool bar somewhat
more predictable.

11.  Keep a single tool bar to fewer than 20 items divided into 4-7 groups of
items.
*/
#endif
