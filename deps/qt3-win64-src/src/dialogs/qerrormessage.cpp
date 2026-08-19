/****************************************************************************
** $Id: qerrormessage.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of a nice qInstallMsgHandler() handler
**
** Created : 000527, after Kalle Dalheimer's birthday
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

#include "qerrormessage.h"

#ifndef QT_NO_ERRORMESSAGE

#include "qapplication.h"
#include "qcheckbox.h"
#include "qdict.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qmessagebox.h"
#include "qpushbutton.h"
#include "qstringlist.h"
#include "qstylesheet.h"
#include "qtextview.h"

#include <stdio.h>
#include <stdlib.h>

class QErrorMessageTextView : public QTextView
{
public:
    QErrorMessageTextView( QWidget *parent, const char *name )
	: QTextView( parent, name ) { }

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;
};

QSize QErrorMessageTextView::minimumSizeHint() const
{
    return QSize( 50, 50 );
}

QSize QErrorMessageTextView::sizeHint() const
{
    return QSize( 250, 75 );
}

/*! \class QErrorMessage

  \brief The QErrorMessage class provides an error message display dialog.

  \ingroup dialogs
  \ingroup misc

This is basically a QLabel and a "show this message again" checkbox which
remembers what not to show.

There are two ways to use this class:
\list 1
\i For production applications. In this context the class can be used to
display messages which you don't need the user to see more than once. To use
QErrorMessage like this, you create the dialog in the usual way and call the
message() slot, or connect signals to it.

\i For developers. In this context the static qtHandler() installs
a message handler using qInstallMsgHandler() and creates a QErrorMessage
that displays qDebug(), qWarning() and qFatal() messages.
\endlist

In both cases QErrorMessage will queue pending messages, and display
them (or not) in order, as soon as the user presses Enter or clicks OK
after seeing each message.

\img qerrormessage.png

\sa QMessageBox QStatusBar::message()
*/

static QErrorMessage * qtMessageHandler = 0;

static void deleteStaticcQErrorMessage() // post-routine
{
    if ( qtMessageHandler ) {
	delete qtMessageHandler;
	qtMessageHandler = 0;
    }
}

static bool metFatal = FALSE;

void jump( QtMsgType t, const char * m )
{
    if ( !qtMessageHandler )
	return;

    QString rich;

    switch ( t ) {
    case QtDebugMsg:
    default:
	rich = QErrorMessage::tr( "Debug Message:" );
	break;
    case QtWarningMsg:
	rich = QErrorMessage::tr( "Warning:" );
	break;
    case QtFatalMsg:
	rich = QErrorMessage::tr( "Fatal Error:" );
    }
    rich = QString( "<p><b>%1</b></p>" ).arg( rich );
    rich += QStyleSheet::convertFromPlainText( m,
		QStyleSheetItem::WhiteSpaceNormal );

    // ### work around text engine quirk
    if ( rich.endsWith("</p>") )
	rich.truncate( rich.length() - 4 );

    if ( !metFatal ) {
	qtMessageHandler->message( rich );
	metFatal = ( t == QtFatalMsg );
    }
}


/*!  Constructs and installs an error handler window.
    The parent \a parent and name \a name are passed on to the QDialog
    constructor.
*/

QErrorMessage::QErrorMessage( QWidget * parent, const char * name )
    : QDialog( parent, name )
{
    QGridLayout * grid = new QGridLayout( this, 3, 2, 11, 6 );
    icon = new QLabel( this, "qt_icon_lbl" );
#ifndef QT_NO_MESSAGEBOX
    icon->setPixmap( QMessageBox::standardIcon(QMessageBox::Information) );
#endif
    grid->addWidget( icon, 0, 0, AlignTop );
    errors = new QErrorMessageTextView( this, "errors" );
    grid->addWidget( errors, 0, 1 );
    again = new QCheckBox( tr( "&Show this message again" ), this, "again" );
    again->setChecked( TRUE );
    grid->addWidget( again, 1, 1, AlignTop + AlignAuto );
    ok = new QPushButton( tr( "&OK" ), this, "ok" );
    connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
    ok->setFocus();
    grid->addMultiCellWidget( ok, 2, 2, 0, 1, AlignCenter );
    grid->setColStretch( 1, 42 );
    grid->setRowStretch( 0, 42 );
    pending = new QStringList;
    doNotShow = new QDict<int>;
}


/*! Destroys the object and frees any allocated resources.  Notably,
the list of "do not show again" messages is deleted. */

QErrorMessage::~QErrorMessage()
{
    if ( this == qtMessageHandler ) {
	qtMessageHandler = 0;
	QtMsgHandler tmp = qInstallMsgHandler( 0 );
	// in case someone else has later stuck in another...
	if ( tmp != jump )
	    qInstallMsgHandler( tmp );
    }

    delete pending;
    delete doNotShow;
}


/*! \reimp */

void QErrorMessage::done( int a )
{
    int dummy = 0;
    if ( !again->isChecked() )
	doNotShow->insert( errors->text(), &dummy );
    if ( !nextPending() ) {
	QDialog::done( a );
	if ( this == qtMessageHandler && metFatal )
	    exit( 1 );
    }
}


/*!  Returns a pointer to a QErrorMessage object that outputs the
default Qt messages.  This function creates such an object, if there
isn't one already.
*/

QErrorMessage * QErrorMessage::qtHandler()
{
    if ( !qtMessageHandler ) {
	qtMessageHandler = new QErrorMessage( 0, "automatic message handler" );
	qAddPostRoutine( deleteStaticcQErrorMessage ); // clean up
#ifndef QT_NO_WIDGET_TOPEXTRA
	if ( qApp->mainWidget() )
	    qtMessageHandler->setCaption( qApp->mainWidget()->caption() );
#endif
	qInstallMsgHandler( jump );
    }
    return qtMessageHandler;
}


/*! \internal */

bool QErrorMessage::nextPending()
{
    while ( !pending->isEmpty() ) {
	QString p = *pending->begin();
	pending->remove( pending->begin() );
	if ( !p.isEmpty() && !doNotShow->find( p ) ) {
	    errors->setText( p );
	    return TRUE;
	}
    }
    return FALSE;
}


/*! Shows message \a m and returns immediately.  If the user has requested
  that \a m not be shown, this function does nothing.

  Normally, \a m is shown at once, but if there are pending messages,
  \a m is queued for later display.
*/

void QErrorMessage::message( const QString & m )
{
    if ( doNotShow->find( m ) )
	return;
    pending->append( m );
    if ( !isVisible() && nextPending() )
	show();
}

#endif // QT_NO_ERRORMESSAGE
