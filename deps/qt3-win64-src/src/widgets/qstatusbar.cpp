/****************************************************************************
** $Id: qstatusbar.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QStatusBar class
**
** Created : 980119
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

#include "qstatusbar.h"
#ifndef QT_NO_STATUSBAR

#include "qptrlist.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qdrawutil.h"
#include "qstyle.h"
#include "qsizegrip.h"

/*!
    \class QStatusBar qstatusbar.h
    \brief The QStatusBar class provides a horizontal bar suitable for
    presenting status information.

    \ingroup application
    \ingroup helpsystem
    \mainclass

    Each status indicator falls into one of three categories:

    \list
    \i \e Temporary - briefly occupies most of the status bar. Used
	to explain tool tip texts or menu entries, for example.
    \i \e Normal - occupies part of the status bar and may be hidden
	by temporary messages. Used to display the page and line
	number in a word processor, for example.
    \i \e Permanent - is never hidden. Used for important mode
	indications, for example, some applications put a Caps Lock
	indicator in the status bar.
    \endlist

    QStatusBar lets you display all three types of indicators.

    To display a \e temporary message, call message() (perhaps by
    connecting a suitable signal to it). To remove a temporary
    message, call clear(). There are two variants of message(): one
    that displays the message until the next clear() or message() and
    one that has a time limit:

    \code
	connect( loader, SIGNAL(progressMessage(const QString&)),
		 statusBar(), SLOT(message(const QString&)) );

	statusBar()->message("Loading...");  // Initial message
	loader.loadStuff();                  // Emits progress messages
	statusBar()->message("Done.", 2000); // Final message for 2 seconds
    \endcode

    \e Normal and \e Permanent messages are displayed by creating a
    small widget and then adding it to the status bar with
    addWidget(). Widgets like QLabel, QProgressBar or even QToolButton
    are useful for adding to status bars. removeWidget() is used to
    remove widgets.

    \code
	statusBar()->addWidget(new MyReadWriteIndication(statusBar()));
    \endcode

    By default QStatusBar provides a QSizeGrip in the lower-right
    corner. You can disable it with setSizeGripEnabled(FALSE);

    <img src=qstatusbar-m.png> <img src=qstatusbar-w.png>

    \sa QToolBar QMainWindow QLabel
    \link guibooks.html#fowler GUI Design Handbook: Status Bar.\endlink
*/


class QStatusBarPrivate
{
public:
    QStatusBarPrivate() {}

    struct SBItem {
	SBItem( QWidget* widget, int stretch, bool permanent )
	    : s( stretch ), w( widget ), p( permanent ) {}
	int s;
	QWidget * w;
	bool p;
    };

    QPtrList<SBItem> items;
    QString tempItem;

    QBoxLayout * box;
    QTimer * timer;

#ifndef QT_NO_SIZEGRIP
    QSizeGrip * resizer;
#endif

    int savedStrut;
};


/*!
    Constructs a status bar called \a name with parent \a parent and
    with a size grip.

    \sa setSizeGripEnabled()
*/
QStatusBar::QStatusBar( QWidget * parent, const char *name )
    : QWidget( parent, name )
{
    d = new QStatusBarPrivate;
    d->items.setAutoDelete( TRUE );
    d->box = 0;
    d->timer = 0;

#ifndef QT_NO_SIZEGRIP
    d->resizer = 0;
    setSizeGripEnabled(TRUE); // causes reformat()
#else
    reformat();
#endif
}


/*!
    Destroys the status bar and frees any allocated resources and
    child widgets.
*/
QStatusBar::~QStatusBar()
{
    delete d;
    d = 0;
}


/*!
    Adds \a widget to this status bar. \a widget is reparented if it
    isn't already a child of the QStatusBar.

    \a widget is permanently visible if \a permanent is TRUE and may
    be obscured by temporary messages if \a permanent is FALSE. The
    default is FALSE.

    If \a permanent is TRUE, \a widget is located at the far right of
    the status bar. If \a permanent is FALSE (the default), \a widget
    is located just to the left of the first permanent widget.

    \a stretch is used to compute a suitable size for \a widget as the
    status bar grows and shrinks. The default of 0 uses a minimum of
    space.

    This function may cause some flicker.

    \sa removeWidget()
*/

void QStatusBar::addWidget( QWidget * widget, int stretch, bool permanent )
{
    if ( !widget ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QStatusBar::addWidget(): Cannot add null widget" );
#endif
	return;
    }

    if ( widget->parentWidget() != this )
	widget->reparent( this, QPoint(0, 0), TRUE );

    QStatusBarPrivate::SBItem* item
	= new QStatusBarPrivate::SBItem( widget, stretch, permanent );

    d->items.last();
    while( !permanent && d->items.current() && d->items.current()->p )
	d->items.prev();

    d->items.insert( d->items.at() >= 0 ? d->items.at()+1 : 0, item );

    if ( !d->tempItem.isEmpty() && !permanent )
	widget->hide();

    reformat();
}


/*!
    Removes \a widget from the status bar.

    This function may cause some flicker.

    Note that \a widget is not deleted.

    \sa addWidget()
*/

void QStatusBar::removeWidget( QWidget* widget )
{
    if ( !widget )
	return;
    bool found = FALSE;
    QStatusBarPrivate::SBItem* item = d->items.first();
    while ( item && !found ) {
	if ( item->w == widget ) {
	    d->items.remove();
	    found = TRUE;
	}
	item = d->items.next();
    }

    if ( found )
	reformat();
#if defined(QT_DEBUG)
    else
	qDebug( "QStatusBar::removeWidget(): Widget not found." );
#endif
}

/*!
    \property QStatusBar::sizeGripEnabled
    \brief whether the QSizeGrip in the bottom right of the status bar is enabled

    Enables or disables the QSizeGrip in the bottom right of the
    status bar. By default, the size grip is enabled.
*/

bool QStatusBar::isSizeGripEnabled() const
{
#ifdef QT_NO_SIZEGRIP
    return FALSE;
#else
    return !!d->resizer;
#endif
}

void QStatusBar::setSizeGripEnabled(bool enabled)
{
#ifndef QT_NO_SIZEGRIP
    if ( !enabled != !d->resizer ) {
	if ( enabled ) {
	    d->resizer = new QSizeGrip( this, "QStatusBar::resizer" );
	} else {
	    delete d->resizer;
	    d->resizer = 0;
	}
	reformat();
	if ( d->resizer && isVisible() )
	    d->resizer->show();
    }
#endif
}


/*!
    Changes the status bar's appearance to account for item changes.
    Special subclasses may need this, but geometry management will
    usually take care of any necessary rearrangements.
*/
void QStatusBar::reformat()
{
    if ( d->box )
	delete d->box;

    QBoxLayout *vbox;
    if ( isSizeGripEnabled() ) {
	d->box = new QHBoxLayout( this );
	vbox = new QVBoxLayout( d->box );
    } else {
	vbox = d->box = new QVBoxLayout( this );
    }
    vbox->addSpacing( 3 );
    QBoxLayout* l = new QHBoxLayout( vbox );
    l->addSpacing( 3 );
    l->setSpacing( 4 );

    int maxH = fontMetrics().height();

    QStatusBarPrivate::SBItem* item = d->items.first();
    while ( item && !item->p ) {
	l->addWidget( item->w, item->s );
	int itemH = QMIN(item->w->sizeHint().height(),
			 item->w->maximumHeight());
	maxH = QMAX( maxH, itemH );
	item = d->items.next();
    }

    l->addStretch( 0 );

    while ( item ) {
	l->addWidget( item->w, item->s );
	int itemH = QMIN(item->w->sizeHint().height(),
			 item->w->maximumHeight());
	maxH = QMAX( maxH, itemH );
	item = d->items.next();
    }
    l->addSpacing( 4 );
#ifndef QT_NO_SIZEGRIP
    if ( d->resizer ) {
	maxH = QMAX( maxH, d->resizer->sizeHint().height() );
	d->box->addSpacing( 1 );
	d->box->addWidget( d->resizer, 0, AlignBottom );
    }
#endif
    l->addStrut( maxH );
    d->savedStrut = maxH;
    vbox->addSpacing( 2 );
    d->box->activate();
    repaint();
}




/*!
    Hides the normal status indicators and displays \a message until
    clear() or another message() is called.

    \sa clear()
*/
void QStatusBar::message( const QString &message )
{
    if ( d->tempItem == message )
	return;
    d->tempItem = message;
    if ( d->timer ) {
	delete d->timer;
	d->timer = 0;
    }
    hideOrShow();
}


/*!
    \overload

    Hides the normal status indications and displays \a message for \a
    ms milli-seconds or until clear() or another message() is called,
    whichever occurs first.
*/
void QStatusBar::message( const QString &message, int ms )
{
    d->tempItem = message;

    if ( !d->timer ) {
	d->timer = new QTimer( this );
	connect( d->timer, SIGNAL(timeout()), this, SLOT(clear()) );
    }
    if ( ms > 0 ) {
	d->timer->start( ms );
    } else if ( d->timer ) {
	delete d->timer;
	d->timer = 0;
    }

    hideOrShow();
}


/*!
    Removes any temporary message being shown.

    \sa message()
*/

void QStatusBar::clear()
{
    if ( d->tempItem.isEmpty() )
	return;
    if ( d->timer ) {
	delete d->timer;
	d->timer = 0;
    }
    d->tempItem = QString::null;
    hideOrShow();
}

/*!
    \fn QStatusBar::messageChanged( const QString &message )

    This signal is emitted when the temporary status messages
    changes. \a message is the new temporary message, and is a
    null-string when the message has been removed.

    \sa message(), clear()
*/

/*!
    Ensures that the right widgets are visible. Used by message() and
    clear().
*/
void QStatusBar::hideOrShow()
{
    bool haveMessage = !d->tempItem.isEmpty();

    QStatusBarPrivate::SBItem* item = d->items.first();

    while( item && !item->p ) {
	if ( haveMessage )
	    item->w->hide();
	else
	    item->w->show();
	item = d->items.next();
    }

    emit messageChanged( d->tempItem );
    repaint();
}


/*!
    Shows the temporary message, if appropriate.
*/
void QStatusBar::paintEvent( QPaintEvent * )
{
    bool haveMessage = !d->tempItem.isEmpty();

    QPainter p( this );
    QStatusBarPrivate::SBItem* item = d->items.first();

#ifndef QT_NO_SIZEGRIP
    int psx = ( d->resizer && d->resizer->isVisible() ) ? d->resizer->x() : width()-12;
#else
    int psx = width() - 12;
#endif

    while ( item ) {
	if ( !haveMessage || item->p )
	    if ( item->w->isVisible() ) {
		if ( item->p && item->w->x()-1 < psx )
		    psx = item->w->x()-1;
		style().drawPrimitive( QStyle::PE_StatusBarSection, &p,
				       QRect(item->w->x() - 1, item->w->y() - 1,
					     item->w->width()+2, item->w->height()+2),
				       colorGroup(), QStyle::Style_Default,
				       QStyleOption(item->w) );
	    }
	item = d->items.next();
    }
    if ( haveMessage ) {
	p.setPen( colorGroup().foreground() );
	p.drawText( 6, 0, psx, height(), AlignVCenter | SingleLine, d->tempItem );
    }
}

/*!
    \reimp
*/
void QStatusBar::resizeEvent( QResizeEvent * e )
{
    QWidget::resizeEvent( e );
}

/*!
    \reimp
*/

bool QStatusBar::event( QEvent *e )
{
    if ( e->type() == QEvent::LayoutHint ) {
	// Calculate new strut height and call reformat() if it has changed
	int maxH = fontMetrics().height();

	QStatusBarPrivate::SBItem* item = d->items.first();
	while ( item ) {
	    int itemH = QMIN(item->w->sizeHint().height(),
			    item->w->maximumHeight());
	    maxH = QMAX( maxH, itemH );
	    item = d->items.next();
	}

#ifndef QT_NO_SIZEGRIP
	if ( d->resizer )
	    maxH = QMAX( maxH, d->resizer->sizeHint().height() );
#endif

	if ( maxH != d->savedStrut )
	    reformat();
	else
	    update();
    }
    if ( e->type() == QEvent::ChildRemoved ) {
	QStatusBarPrivate::SBItem* item = d->items.first();
	while ( item ) {
	    if ( item->w == ( (QChildEvent*)e )->child() )
		d->items.removeRef( item );
	    item = d->items.next();
	}
    }
    return QWidget::event( e );
}

#endif
