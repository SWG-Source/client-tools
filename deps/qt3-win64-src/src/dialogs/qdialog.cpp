/****************************************************************************
** $Id: qdialog.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QDialog class
**
** Created : 950502
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
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

#include "qdialog.h"

#ifndef QT_NO_DIALOG

#include "qpushbutton.h"
#include "qfocusdata.h"
#include "qapplication.h"
#include "qobjectlist.h"
#include "qwidgetlist.h"
#include "qlayout.h"
#include "qsizegrip.h"
#include "qwhatsthis.h"
#include "qpopupmenu.h"
#include "qcursor.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif
#if defined( Q_OS_TEMP )
#include "qt_windows.h"
#endif

/*!
    \class QDialog
    \brief The QDialog class is the base class of dialog windows.

    \ingroup dialogs
    \ingroup abstractwidgets
    \mainclass

    A dialog window is a top-level window mostly used for short-term
    tasks and brief communications with the user. QDialogs may be
    modal or modeless. QDialogs support \link #extensibility
    extensibility\endlink and can provide a \link #return return
    value\endlink. They can have \link #default default
    buttons\endlink. QDialogs can also have a QSizeGrip in their
    lower-right corner, using setSizeGripEnabled().

    Note that QDialog uses the parent widget slightly differently from
    other classes in Qt. A dialog is always a top-level widget, but if
    it has a parent, its default location is centered on top of the
    parent's top-level widget (if it is not top-level itself). It will
    also share the parent's taskbar entry.

    \target modal
    \section1 Modal Dialogs

    A <b>modal</b> dialog is a dialog that blocks input to other
    visible windows in the same application. Users must finish
    interacting with the dialog and close it before they can access
    any other window in the application. Dialogs that are used to
    request a file name from the user or that are used to set
    application preferences are usually modal.

    The most common way to display a modal dialog is to call its
    exec() function. When the user closes the dialog, exec() will
    provide a useful \link #return return value\endlink. Typically we
    connect a default button, e.g. "OK", to the accept() slot and a
    "Cancel" button to the reject() slot, to get the dialog to close
    and return the appropriate value. Alternatively you can connect to
    the done() slot, passing it \c Accepted or \c Rejected.

    An alternative is to call setModal(TRUE), then show(). Unlike
    exec(), show() returns control to the caller immediately. Calling
    setModal(TRUE) is especially useful for progress dialogs, where
    the user must have the ability to interact with the dialog, e.g.
    to cancel a long running operation. If you use show() and
    setModal(TRUE) together you must call
    QApplication::processEvents() periodically during processing to
    enable the user to interact with the dialog. (See \l
    QProgressDialog.)

    \target modeless
    \section1 Modeless Dialogs

    A <b>modeless</b> dialog is a dialog that operates
    independently of other windows in the same application. Find and
    replace dialogs in word-processors are often modeless to allow the
    user to interact with both the application's main window and with
    the dialog.

    Modeless dialogs are displayed using show(), which returns control
    to the caller immediately.

    \target default
    \section1 Default button

    A dialog's \e default button is the button that's pressed when the
    user presses Enter (Return). This button is used to signify that
    the user accepts the dialog's settings and wants to close the
    dialog. Use QPushButton::setDefault(), QPushButton::isDefault()
    and QPushButton::autoDefault() to set and control the dialog's
    default button.

    \target escapekey
    \section1 Escape Key

    If the user presses the Esc key in a dialog, QDialog::reject()
    will be called. This will cause the window to close: the \link
    QCloseEvent closeEvent \endlink cannot be \link
    QCloseEvent::ignore() ignored \endlink.

    \target extensibility
    \section1 Extensibility

    Extensibility is the ability to show the dialog in two ways: a
    partial dialog that shows the most commonly used options, and a
    full dialog that shows all the options. Typically an extensible
    dialog will initially appear as a partial dialog, but with a
    "More" toggle button. If the user presses the "More" button down,
    the full dialog will appear. The extension widget will be resized
    to its sizeHint(). If orientation is \c Horizontal the extension
    widget's height() will be expanded to the height() of the dialog.
    If the orientation is \c Vertical the extension widget's width()
    will be expanded to the width() of the dialog. Extensibility is
    controlled with setExtension(), setOrientation() and
    showExtension().

    \target return
    \section1 Return value (modal dialogs)

    Modal dialogs are often used in situations where a return value is
    required, e.g. to indicate whether the user pressed "OK" or
    "Cancel". A dialog can be closed by calling the accept() or the
    reject() slots, and exec() will return \c Accepted or \c Rejected
    as appropriate. The exec() call returns the result of the dialog.
    The result is also available from result() if the dialog has not
    been destroyed. If the \c WDestructiveClose flag is set, the
    dialog is deleted after exec() returns.

    \target examples
    \section1 Examples

    A modal dialog.

    \quotefile network/networkprotocol/view.cpp
    \skipto QFileDialog *dlg
    \printuntil return

    A modeless dialog. After the show() call, control returns to the main
    event loop.
    \quotefile life/main.cpp
    \skipto argv
    \printuntil QApplication
    \skipto scale
    \printline
    \skipto LifeDialog
    \printuntil show
    \skipto exec
    \printuntil }

    \sa QTabDialog QWidget QProgressDialog
    \link guibooks.html#fowler GUI Design Handbook: Dialogs, Standard\endlink
*/

/*! \enum QDialog::DialogCode

    The value returned by a modal dialog.

    \value Accepted
    \value Rejected

*/

/*!
  \property QDialog::sizeGripEnabled
  \brief whether the size grip is enabled

  A QSizeGrip is placed in the bottom right corner of the dialog when this
  property is enabled. By default, the size grip is disabled.
*/

class QDialogPrivate : public Qt
{
public:

    QDialogPrivate()
	: mainDef(0), orientation(Horizontal),extension(0), doShowExtension(FALSE)
#ifndef QT_NO_SIZEGRIP
	,resizer(0)
#endif
	{
    }

    QPushButton* mainDef;
    Orientation orientation;
    QWidget* extension;
    bool doShowExtension;
    QSize size, min, max;
#ifndef QT_NO_SIZEGRIP
    QSizeGrip* resizer;
#endif
    QPoint lastRMBPress;
    QPoint relPos; // relative position to the main window
};

/*!
  Constructs a dialog called \a name, with parent \a parent.

  A dialog is always a top-level widget, but if it has a parent, its
  default location is centered on top of the parent. It will also
  share the parent's taskbar entry.

  The widget flags \a f are passed on to the QWidget constructor.
  If, for example, you don't want a What's This button in the titlebar
  of the dialog, pass WStyle_Customize | WStyle_NormalBorder |
  WStyle_Title | WStyle_SysMenu in \a f.

  \warning In Qt 3.2, the \a modal flag is obsolete. There is now a
  setModal() function that can be used for obtaining a modal behavior
  when calling show(). This is rarely needed, because modal dialogs
  are usually invoked using exec(), which ignores the \a modal flag.

  \sa QWidget::setWFlags() Qt::WidgetFlags
*/

QDialog::QDialog( QWidget *parent, const char *name, bool modal, WFlags f )
    : QWidget( parent, name,
	       (modal ? (f|WShowModal) : f) | WType_Dialog ),
      rescode(0), did_move(0), has_relpos(0), did_resize(0), in_loop(0)
{
    d = new QDialogPrivate;
}

/*!
  Destroys the QDialog, deleting all its children.
*/

QDialog::~QDialog()
{
    // Need to hide() here, as our (to-be) overridden hide()
    // will not be called in ~QWidget.
    hide();
    delete d;
}

/*!
  \internal
  This function is called by the push button \a pushButton when it
  becomes the default button. If \a pushButton is 0, the dialogs
  default default button becomes the default button. This is what a
  push button calls when it loses focus.
*/

void QDialog::setDefault( QPushButton *pushButton )
{
#ifndef QT_NO_PUSHBUTTON
    QObjectList *list = queryList( "QPushButton" );
    Q_ASSERT(list);
    QObjectListIt it( *list );
    QPushButton *pb;
    bool hasMain = FALSE;
    while ( (pb = (QPushButton*)it.current()) ) {
	++it;
	if ( pb->topLevelWidget() != this )
	    continue;
	if ( pb == d->mainDef )
	    hasMain = TRUE;
	if ( pb != pushButton )
	    pb->setDefault( FALSE );
    }
    if (!pushButton && hasMain)
	d->mainDef->setDefault( TRUE );
    if (!hasMain)
	d->mainDef = pushButton;
    delete list;
#endif
}

/*!
  \internal
  This function sets the default default pushbutton to \a pushButton.
  This function is called by QPushButton::setDefault().
*/
void QDialog::setMainDefault( QPushButton *pushButton )
{
#ifndef QT_NO_PUSHBUTTON
    d->mainDef = 0;
    setDefault(pushButton);
#endif
}

/*!
  \internal
  Hides the default button indicator. Called when non auto-default
  push button get focus.
 */
void QDialog::hideDefault()
{
#ifndef QT_NO_PUSHBUTTON
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    while ( (pb = (QPushButton*)it.current()) ) {
	++it;
	pb->setDefault( FALSE );
    }
    delete list;
#endif
}

#ifdef Q_OS_TEMP
/*!
  \internal
  Hides special buttons which are rather shown in the titlebar
  on WinCE, to conserve screen space.
*/
# include "qmessagebox.h"
extern const char * mb_texts[]; // Defined in qmessagebox.cpp
void QDialog::hideSpecial()
{
    // "OK"     buttons are hidden, and (Ok) shown on titlebar
    // "Cancel" buttons are hidden, and (X)  shown on titlebar
    // "Help"   buttons are hidden, and (?)  shown on titlebar
    bool showOK = FALSE,
	 showX  = FALSE,
	 showQ  = FALSE;
    QObjectList *list = queryList( "QPushButton" );
    QObjectListIt it( *list );
    QPushButton *pb;
    while ( (pb = (QPushButton*)it.current()) ) {
	if ( !showOK &&
	     pb->text() == qApp->translate( "QMessageBox", mb_texts[QMessageBox::Ok] ) ) {
	    pb->hide();
	    showOK = TRUE;
	} else if ( !showX &&
		    pb->text() == qApp->translate( "QMessageBox", mb_texts[QMessageBox::Cancel] ) ) {
	    pb->hide();
	    showX = TRUE;
	} else if ( !showQ &&
		    pb->text() == qApp->tr("Help") ) {
	    pb->hide();
	    showQ = TRUE;
	}
        ++it;
    }
    delete list;
    if ( showOK || showQ ) {
	DWORD ext = GetWindowLong( winId(), GWL_EXSTYLE );
	ext |= showOK ? WS_EX_CAPTIONOKBTN : 0;
	ext |= showQ  ? WS_EX_CONTEXTHELP: 0;
	SetWindowLong( winId(), GWL_EXSTYLE, ext );
    }
    if ( !showX ) {
	DWORD ext = GetWindowLong( winId(), GWL_STYLE );
	ext &= ~WS_SYSMENU;
	SetWindowLong( winId(), GWL_STYLE, ext );
    }
}
#endif

/*!
  \fn int QDialog::result() const

  Returns the modal dialog's result code, \c Accepted or \c Rejected.

  Do not call this function if the dialog was constructed with the \c
  WDestructiveClose flag.
*/

/*!
  \fn void QDialog::setResult( int i )

  Sets the modal dialog's result code to \a i.
*/


/*!
    Shows the dialog as a \link #modal modal \endlink dialog,
    blocking until the user closes it. The function returns a \l
    DialogCode result.

    Users cannot interact with any other window in the same
    application until they close the dialog.

  \sa show(), result()
*/

int QDialog::exec()
{
    if ( in_loop ) {
	qWarning( "QDialog::exec: Recursive call detected" );
	return -1;
    }

    bool destructiveClose = testWFlags( WDestructiveClose );
    clearWFlags( WDestructiveClose );

    bool wasShowModal = testWFlags( WShowModal );
    setWFlags( WShowModal );
    setResult( 0 );

    show();

    in_loop = TRUE;
    qApp->enter_loop();

    if ( !wasShowModal )
	clearWFlags( WShowModal );

    int res = result();

    if ( destructiveClose )
	delete this;

    return res;
}


/*! Closes the dialog and sets its result code to \a r. If this dialog
  is shown with exec(), done() causes the local event loop to finish,
  and exec() to return \a r.

  As with QWidget::close(), done() deletes the dialog if the \c
  WDestructiveClose flag is set. If the dialog is the application's
  main widget, the application terminates. If the dialog is the
  last window closed, the QApplication::lastWindowClosed() signal is
  emitted.

  \sa accept(), reject(), QApplication::mainWidget(), QApplication::quit()
*/

void QDialog::done( int r )
{
    hide();
    setResult( r );

    // emulate QWidget::close()
    bool isMain = qApp->mainWidget() == this;
    bool checkLastWindowClosed = isTopLevel() && !isPopup();
    if ( checkLastWindowClosed
	 && qApp->receivers(SIGNAL(lastWindowClosed())) ) {
	/* if there is no non-withdrawn top level window left (except
	   the desktop, popups, or dialogs with parents), we emit the
	   lastWindowClosed signal */
	QWidgetList *list   = qApp->topLevelWidgets();
	QWidget     *widget = list->first();
	while ( widget ) {
	    if ( !widget->isHidden()
		 && !widget->isDesktop()
		 && !widget->isPopup()
		 && (!widget->isDialog() || !widget->parentWidget()))
		break;
	    widget = list->next();
	}
	delete list;
	if ( widget == 0 )
	    emit qApp->lastWindowClosed();
    }
    if ( isMain )
	qApp->quit();
    else if ( testWFlags(WDestructiveClose) ) {
	clearWFlags(WDestructiveClose);
	deleteLater();
    }
}

/*!
  Hides the modal dialog and sets the result code to \c Accepted.

  \sa reject() done()
*/

void QDialog::accept()
{
    done( Accepted );
}

/*!
  Hides the modal dialog and sets the result code to \c Rejected.

  \sa accept() done()
*/

void QDialog::reject()
{
    done( Rejected );
}

/*! \reimp */
bool QDialog::eventFilter( QObject *o, QEvent *e )
{
    return QWidget::eventFilter( o, e );
}

/*****************************************************************************
  Event handlers
 *****************************************************************************/

/*! \reimp */
void QDialog::contextMenuEvent( QContextMenuEvent *e )
{
#if !defined(QT_NO_WHATSTHIS) && !defined(QT_NO_POPUPMENU)
    QWidget* w = childAt( e->pos(), TRUE );
    if ( !w )
	return;
    QString s;
    while ( s.isEmpty() && w ) {
	s = QWhatsThis::textFor( w, e->pos(), FALSE );
	if ( s.isEmpty() )
	    w = w->parentWidget(TRUE);
    }
    if ( !s.isEmpty() ) {
	QPopupMenu p(0,"qt_whats_this_menu");
	p.insertItem( tr("What's This?"), 42 );
	if ( p.exec( e->globalPos() ) >= 42 )
	    QWhatsThis::display( s, w->mapToGlobal( w->rect().center() ), w );
    }
#endif
}

/*! \reimp */
void QDialog::keyPressEvent( QKeyEvent *e )
{
    //   Calls reject() if Escape is pressed. Simulates a button
    //   click for the default button if Enter is pressed. Move focus
    //   for the arrow keys. Ignore the rest.
#ifdef Q_OS_MAC
    if(e->state() == ControlButton && e->key() == Key_Period) {
	reject();
    } else
#endif
    if ( e->state() == 0 || ( e->state() & Keypad && e->key() == Key_Enter ) ) {
	switch ( e->key() ) {
	case Key_Enter:
	case Key_Return: {
#ifndef QT_NO_PUSHBUTTON
	    QObjectList *list = queryList( "QPushButton" );
	    QObjectListIt it( *list );
	    QPushButton *pb;
	    while ( (pb = (QPushButton*)it.current()) ) {
		if ( pb->isDefault() && pb->isVisible() ) {
		    delete list;
		    if ( pb->isEnabled() ) {
			emit pb->clicked();
		    }
		    return;
		}
		++it;
	    }
	    delete list;
#endif
	}
	break;
	case Key_Escape:
	    reject();
	    break;
	case Key_Up:
	case Key_Left:
	    if ( focusWidget() &&
		 ( focusWidget()->focusPolicy() == QWidget::StrongFocus ||
		   focusWidget()->focusPolicy() == QWidget::WheelFocus ) ) {
		e->ignore();
		break;
	    }
	    // call ours, since c++ blocks us from calling the one
	    // belonging to focusWidget().
	    QFocusEvent::setReason(QFocusEvent::Backtab);
	    focusNextPrevChild( FALSE );
	    QFocusEvent::resetReason();
	    break;
	case Key_Down:
	case Key_Right:
	    if ( focusWidget() &&
		 ( focusWidget()->focusPolicy() == QWidget::StrongFocus ||
		   focusWidget()->focusPolicy() == QWidget::WheelFocus ) ) {
		e->ignore();
		break;
	    }
	    QFocusEvent::setReason(QFocusEvent::Tab);
	    focusNextPrevChild( TRUE );
	    QFocusEvent::resetReason();
	    break;
	default:
	    e->ignore();
	    return;
	}
    } else {
	e->ignore();
    }
}

/*! \reimp */
void QDialog::closeEvent( QCloseEvent *e )
{
#ifndef QT_NO_WHATSTHIS
    if ( isModal() && QWhatsThis::inWhatsThisMode() )
	QWhatsThis::leaveWhatsThisMode();
#endif
    if ( isShown() )
	reject();
    if ( isHidden() )
	e->accept();
}

#ifdef Q_OS_TEMP
/*! \internal
    \reimp
*/
bool QDialog::event( QEvent *e )
{
    switch ( e->type() ) {
    case QEvent::OkRequest:
    case QEvent::HelpRequest:
	{
	    QString bName =
		(e->type() == QEvent::OkRequest)
		? qApp->translate( "QMessageBox", mb_texts[QMessageBox::Ok] )
		: qApp->tr( "Help" );

	    QObjectList *list = queryList( "QPushButton" );
	    QObjectListIt it( *list );
	    QPushButton *pb;
	    while ( (pb = (QPushButton*)it.current()) ) {
		if ( pb->text() == bName ) {
		    delete list;
		    if ( pb->isEnabled() )
			emit pb->clicked();
		    return pb->isEnabled();
		}
		++it;
	    }
	    delete list;
	}
    }
    return QWidget::event( e );
}
#endif


/*****************************************************************************
  Geometry management.
 *****************************************************************************/

#if defined(Q_WS_X11)
extern "C" { int XSetTransientForHint( Display *, unsigned long, unsigned long ); }
#endif // Q_WS_X11

/*!
    Shows the dialog as a \link #modeless modeless \endlink dialog.
    Control returns immediately to the calling code.

    The dialog will be modal or modeless according to the value
    of the \l modal property.

    \sa exec(), modal
*/

void QDialog::show()
{
    if ( testWState(WState_Visible) )
	return;

    uint state = windowState();

    if ( !did_resize )
	adjustSize();
    if ( has_relpos && !did_move ) {
	adjustPositionInternal( parentWidget(), TRUE );
    } else if ( !did_move ) {
	adjustPositionInternal( parentWidget() );
    }

    if (windowState() != state)
	setWindowState(state);

#if defined(Q_WS_X11)
    if (!parentWidget() && testWFlags(WShowModal)
	&& qApp->mainWidget() && qApp->mainWidget()->isVisible()
	&& !qApp->mainWidget()->isMinimized()) {
	// make sure the transient for hint is set properly for modal dialogs
        XSetTransientForHint( x11Display(), winId(), qApp->mainWidget()->winId() );
    }
#endif // Q_WS_X11

#ifdef Q_OS_TEMP
    hideSpecial();
#endif

    QWidget::show();
    showExtension( d->doShowExtension );
#ifndef QT_NO_PUSHBUTTON
    QWidget *fw = focusWidget();
    QFocusData *fd = focusData();

    /*
      The following block is to handle a special case, and does not
      really follow propper logic in concern of autoDefault and TAB
      order. However, it's here to ease usage for the users. If a
      dialog has a default QPushButton, and first widget in the TAB
      order also is a QPushButton, then we give focus to the main
      default QPushButton. This simplifies code for the developers,
      and actually catches most cases... If not, then they simply
      have to use [widget*]->setFocus() themselves...
    */
    if ( !fw || fw->focusPolicy() == NoFocus ) {
	fd->home(); // Skip main form
	QWidget *first = fd->next(); // Get first main widget
	if ( d->mainDef &&
	     first != d->mainDef &&
	     ::qt_cast<QPushButton*>(first) )
	    d->mainDef->setFocus();
    }

    if ( !d->mainDef && isTopLevel() ) {
	if ( !fw || fw->focusPolicy() == NoFocus ) {
	    focusNextPrevChild( TRUE );
	    fw = focusWidget();
	}
	if ( fw ) {
	    fd = focusData();
	    QWidget *home = fd->home();
	    QWidget *candidate = home;
	    Q_ASSERT( candidate == fw );
	    do {
		QPushButton *pb = ::qt_cast<QPushButton*>(candidate);
		if ( pb && pb->autoDefault() ) {
		    pb->setDefault( TRUE );
		    break;
		}
		candidate = fd->next();
	    } while ( candidate != home );
	}
    }
    if ( fw ) {
	QFocusEvent e( QEvent::FocusIn );
	QFocusEvent::setReason( QFocusEvent::Tab );
	QApplication::sendEvent( fw, &e );
	QFocusEvent::resetReason();
    }

#endif
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility( this, 0, QAccessible::DialogStart );
#endif
}

/*! \internal */
void QDialog::adjustPosition( QWidget* w)
{
    adjustPositionInternal( w );
}


void QDialog::adjustPositionInternal( QWidget*w, bool useRelPos)
{
    /* need to make sure these events are already sent to be sure
       our information below is correct --sam */
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    QApplication::sendPostedEvents( this, QEvent::Resize );

    // processing the events might call polish(), which is a nice place
    // to restore geometries, so return if the dialog has been positioned
    if ( did_move || has_relpos )
	return;

    QPoint p( 0, 0 );
    int extraw = 0, extrah = 0, scrn = 0;
    if ( w )
	w = w->topLevelWidget();
    QRect desk;
    if ( w ) {
	scrn = QApplication::desktop()->screenNumber( w );
    } else if ( QApplication::desktop()->isVirtualDesktop() ) {
	scrn = QApplication::desktop()->screenNumber( QCursor::pos() );
    } else {
	scrn = QApplication::desktop()->screenNumber( this );
    }
    desk = QApplication::desktop()->availableGeometry( scrn );

    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );
    while ( (extraw == 0 || extrah == 0) &&
	    it.current() != 0 ) {
	int framew, frameh;
	QWidget * current = it.current();
	++it;
	if ( ! current->isVisible() )
	    continue;

	framew = current->geometry().x() - current->x();
	frameh = current->geometry().y() - current->y();

	extraw = QMAX( extraw, framew );
	extrah = QMAX( extrah, frameh );
    }
    delete list;

    // sanity check for decoration frames. With embedding, we
    // might get extraordinary values
    if ( extraw == 0 || extrah == 0 || extraw >= 10 || extrah >= 40 ) {
	extrah = 40;
	extraw = 10;
    }

    if ( useRelPos && w ) {
	p = w->pos() + d->relPos;
    } else {
#ifndef Q_OS_TEMP
	if ( w ) {
	    // Use mapToGlobal rather than geometry() in case w might
	    // be embedded in another application
	    QPoint pp = w->mapToGlobal( QPoint(0,0) );
	    p = QPoint( pp.x() + w->width()/2,
			pp.y() + w->height()/ 2 );
	} else {
	    // p = middle of the desktop
	    p = QPoint( desk.x() + desk.width()/2, desk.y() + desk.height()/2 );
	}
#else
	p = QPoint( desk.x() + desk.width()/2, desk.y() + desk.height()/2 );
#endif

	// p = origin of this
	p = QPoint( p.x()-width()/2 - extraw,
		    p.y()-height()/2 - extrah );
    }


    if ( p.x() + extraw + width() > desk.x() + desk.width() )
	p.setX( desk.x() + desk.width() - width() - extraw );
    if ( p.x() < desk.x() )
	p.setX( desk.x() );

    if ( p.y() + extrah + height() > desk.y() + desk.height() )
	p.setY( desk.y() + desk.height() - height() - extrah );
    if ( p.y() < desk.y() )
	p.setY( desk.y() );

    move( p );
    did_move = !useRelPos;
}


/*! \reimp */
void QDialog::hide()
{
    if ( isHidden() )
	return;

#if defined(QT_ACCESSIBILITY_SUPPORT)
    if ( isVisible() )
	QAccessible::updateAccessibility( this, 0, QAccessible::DialogEnd );
#endif

    if ( parentWidget() && !did_move ) {
	d->relPos = pos() - parentWidget()->topLevelWidget()->pos();
	has_relpos = 1;
    }

    // Reimplemented to exit a modal when the dialog is hidden.
    QWidget::hide();
    if ( in_loop ) {
	in_loop = FALSE;
	qApp->exit_loop();
    }
}


/*****************************************************************************
  Detects any widget geometry changes done by the user.
 *****************************************************************************/

/*! \reimp */

void QDialog::move( int x, int y )
{
    did_move = TRUE;
    QWidget::move( x, y );
}

/*! \reimp */

void QDialog::move( const QPoint &p )
{
    did_move = TRUE;
    QWidget::move( p );
}

/*! \reimp */

void QDialog::resize( int w, int h )
{
    did_resize = TRUE;
    QWidget::resize( w, h );
}

/*! \reimp */

void QDialog::resize( const QSize &s )
{
    did_resize = TRUE;
    QWidget::resize( s );
}

/*! \reimp */

void QDialog::setGeometry( int x, int y, int w, int h )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( x, y, w, h );
}

/*! \reimp */

void QDialog::setGeometry( const QRect &r )
{
    did_move   = TRUE;
    did_resize = TRUE;
    QWidget::setGeometry( r );
}


/*!
    If \a orientation is \c Horizontal, the extension will be displayed
    to the right of the dialog's main area. If \a orientation is \c
    Vertical, the extension will be displayed below the dialog's main
    area.

  \sa orientation(), setExtension()
*/
void QDialog::setOrientation( Orientation orientation )
{
    d->orientation = orientation;
}

/*!
  Returns the dialog's extension orientation.

  \sa setOrientation()
*/
Qt::Orientation QDialog::orientation() const
{
    return d->orientation;
}

/*!
    Sets the widget, \a extension, to be the dialog's extension,
    deleting any previous extension. The dialog takes ownership of the
    extension. Note that if 0 is passed any existing extension will be
    deleted.

  This function must only be called while the dialog is hidden.

  \sa showExtension(), setOrientation(), extension()
 */
void QDialog::setExtension( QWidget* extension )
{
    delete d->extension;
    d->extension = extension;

    if ( !extension )
	return;

    if ( extension->parentWidget() != this )
	extension->reparent( this, QPoint(0,0) );
    extension->hide();
}

/*!
  Returns the dialog's extension or 0 if no extension has been
  defined.

  \sa setExtension()
 */
QWidget* QDialog::extension() const
{
    return d->extension;
}


/*!
  If \a showIt is TRUE, the dialog's extension is shown; otherwise the
  extension is hidden.

  This slot is usually connected to the \l QButton::toggled() signal
  of a QPushButton.

  A dialog with a visible extension is not resizeable.

  \sa show(), setExtension(), setOrientation()
 */
void QDialog::showExtension( bool showIt )
{
    d->doShowExtension = showIt;
    if ( !d->extension )
	return;
    if ( !testWState(WState_Visible) )
	return;
    if ( d->extension->isVisible() == showIt )
	return;

    if ( showIt ) {
	d->size = size();
	d->min = minimumSize();
	d->max = maximumSize();
#ifndef QT_NO_LAYOUT
	if ( layout() )
	    layout()->setEnabled( FALSE );
#endif
	QSize s( d->extension->sizeHint()
		 .expandedTo( d->extension->minimumSize() )
		 .boundedTo( d->extension->maximumSize() ) );
	if ( d->orientation == Horizontal ) {
	    int h = QMAX( height(), s.height() );
	    d->extension->setGeometry( width(), 0, s.width(), h );
	    setFixedSize( width() + s.width(), h );
	} else {
	    int w = QMAX( width(), s.width() );
	    d->extension->setGeometry( 0, height(), w, s.height() );
	    setFixedSize( w, height() + s.height() );
	}
	d->extension->show();
    } else {
	d->extension->hide();
	// workaround for CDE window manager that won't shrink with (-1,-1)
	setMinimumSize( d->min.expandedTo( QSize( 1, 1 ) ) );
	setMaximumSize( d->max );
	resize( d->size );
#ifndef QT_NO_LAYOUT
	if ( layout() )
	    layout()->setEnabled( TRUE );
#endif
    }
}


/*! \reimp */
QSize QDialog::sizeHint() const
{
    if ( d->extension )
	if ( d->orientation == Horizontal )
	    return QSize( QWidget::sizeHint().width(),
			QMAX( QWidget::sizeHint().height(),d->extension->sizeHint().height() ) );
	else
	    return QSize( QMAX( QWidget::sizeHint().width(), d->extension->sizeHint().width() ),
			QWidget::sizeHint().height() );

    return QWidget::sizeHint();
}


/*! \reimp */
QSize QDialog::minimumSizeHint() const
{
    if ( d->extension )
	if (d->orientation == Horizontal )
	    return QSize( QWidget::minimumSizeHint().width(),
			QMAX( QWidget::minimumSizeHint().height(), d->extension->minimumSizeHint().height() ) );
	else
	    return QSize( QMAX( QWidget::minimumSizeHint().width(), d->extension->minimumSizeHint().width() ),
			QWidget::minimumSizeHint().height() );

    return QWidget::minimumSizeHint();
}

/*! \property QDialog::modal
    \brief whether show() should pop up the dialog as modal or modeless

    By default, this property is false and show() pops up the dialog as
    modeless.

    exec() ignores the value of this property and always pops up the
    dialog as modal.

    \sa show(), exec()
*/

void QDialog::setModal( bool modal )
{
    if ( modal )
	setWFlags( WShowModal );
    else
	clearWFlags( WShowModal );
}

bool QDialog::isModal() const
{
    return testWFlags( WShowModal ) != 0;
}

bool QDialog::isSizeGripEnabled() const
{
#ifndef QT_NO_SIZEGRIP
    return !!d->resizer;
#else
    return FALSE;
#endif
}


void QDialog::setSizeGripEnabled(bool enabled)
{
#ifndef QT_NO_SIZEGRIP
    if ( !enabled != !d->resizer ) {
	if ( enabled ) {
	    d->resizer = new QSizeGrip( this, "QDialog::resizer" );
	    // adjustSize() processes all events, which is suboptimal
	    d->resizer->resize( d->resizer->sizeHint() );
	    if ( QApplication::reverseLayout() )
		d->resizer->move( rect().bottomLeft() -d->resizer->rect().bottomLeft() );
	    else
		d->resizer->move( rect().bottomRight() -d->resizer->rect().bottomRight() );
	    d->resizer->raise();
	    d->resizer->show();
	} else {
	    delete d->resizer;
	    d->resizer = 0;
	}
    }
#endif //QT_NO_SIZEGRIP
}



/*! \reimp */
void QDialog::resizeEvent( QResizeEvent * )
{
#ifndef QT_NO_SIZEGRIP
    if ( d->resizer ) {
	if ( QApplication::reverseLayout() )
	    d->resizer->move( rect().bottomLeft() -d->resizer->rect().bottomLeft() );
	else
	    d->resizer->move( rect().bottomRight() -d->resizer->rect().bottomRight() );
    }
#endif
}

#endif // QT_NO_DIALOG
