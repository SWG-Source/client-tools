/****************************************************************************
** $Id: qsplashscreen.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Definition of QSplashScreen class
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
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

#include "qsplashscreen.h"

#ifndef QT_NO_SPLASHSCREEN

#include "qapplication.h"
#include "qpainter.h"
#include "qpixmap.h"

class QSplashScreenPrivate
{
public:
    QPixmap pixmap;
    QString currStatus;
    QColor currColor;
    int currAlign;
};

/*!
   \class QSplashScreen qsplashscreen.h
   \brief The QSplashScreen widget provides a splash screen that can
   be shown during application startup.

    \ingroup misc
    \mainclass

   A splash screen is a widget that is usually displayed when an
   application is being started. Splash screens are often used for
   applications that have long start up times (e.g. database or
   networking applications that take time to establish connections) to
   provide the user with feedback that the application is loading.

   The splash screen appears centered on the screen. It may be useful to add
   the \c WStyle_StaysOnTop if you desire to keep above all the windows in the
   GUI.

   Some X11 window managers do not support the "stays on top" flag. A
   solution is to set up a timer that periodically calls raise() on
   the splash screen to simulate the "stays on top" effect.

   The most common usage is to show a splash screen before the main
   widget is displayed on the screen. This is illustrated in the
   following code snippet.

   \code
   int main( int argc, char **argv )
   {
       QApplication app( argc, argv );
       QPixmap pixmap( "splash.png" );
       QSplashScreen *splash = new QSplashScreen( pixmap );
       splash->show();
       QMainWindow *mainWin = new QMainWindow;
       ...
       app.setMainWidget( mainWin );
       mainWin->show();
       splash->finish( mainWin );
       delete splash;
       return app.exec();
   }
   \endcode

   It is sometimes useful to update the splash screen with messages,
   for example, announcing connections established or modules loaded
   as the application starts up. QSplashScreen supports this with the
   message() function. If you wish to do your own drawing you can
   get a pointer to the pixmap used in the splash screen with pixmap().
   Alternatively, you can subclass QSplashScreen and reimplement
   drawContents().

   The user can hide the splash screen by clicking on it with the
   mouse. Since the splash screen is typically displayed before the
   event loop has started running, it is necessary to periodically
   call QApplication::processEvents() to receive the mouse clicks.

   \code
   QPixmap pixmap( "splash.png" );
   QSplashScreen *splash = new QSplashScreen( pixmap );
   splash->show();
   ... // Loading some items
   splash->message( "Loaded modules" );
   qApp->processEvents();
   ... // Establishing connections
   splash->message( "Established connections" );
   qApp->processEvents();
   \endcode

*/

/*!
    Construct a splash screen that will display the \a pixmap.

    There should be no need to set the widget flags, \a f, except
    perhaps \c WDestructiveClose or \c WStyle_StaysOnTop.
*/
QSplashScreen::QSplashScreen( const QPixmap &pixmap, WFlags f )
    : QWidget( 0, 0, WStyle_Customize | WStyle_Splash | f )
{
    d = new QSplashScreenPrivate();
    d->pixmap = pixmap;
    setPixmap( d->pixmap );  // Does an implicit repaint
}

/*!
  Destructor.
*/
QSplashScreen::~QSplashScreen()
{
    delete d;
}

/*!
    \reimp
*/
void QSplashScreen::mousePressEvent( QMouseEvent * )
{
    hide();
}

/*!
    This overrides QWidget::repaint(). It differs from the standard
    repaint function in that it also calls QApplication::flush() to
    ensure the updates are displayed, even when there is no event loop
    present.
*/
void QSplashScreen::repaint()
{
    drawContents();
    QWidget::repaint();
    QApplication::flush();
}

/*!
    \fn QSplashScreen::messageChanged( const QString &message )

    This signal is emitted when the message on the splash screen
    changes. \a message is the new message and is a null-string
    when the message has been removed.

    \sa message(), clear()
*/



/*!
    Draws the \a message text onto the splash screen with color \a
    color and aligns the text according to the flags in \a alignment.

    \sa Qt::AlignmentFlags clear()
*/
void QSplashScreen::message( const QString &message, int alignment,
			     const QColor &color )
{
    d->currStatus = message;
    d->currAlign = alignment;
    d->currColor = color;
    emit messageChanged( d->currStatus );
    repaint();
}

/*!
    Removes the message being displayed on the splash screen

    \sa message()
 */
void QSplashScreen::clear()
{
    d->currStatus = QString::null;
    emit messageChanged( d->currStatus );
    repaint();
}

/*!
    Makes the splash screen wait until the widget \a mainWin is displayed
    before calling close() on itself.
*/
void QSplashScreen::finish( QWidget *mainWin )
{
    if ( mainWin ) {
#if defined(Q_WS_X11)
	extern void qt_wait_for_window_manager( QWidget *mainWin );
	qt_wait_for_window_manager( mainWin );
#endif
    }
    close();
}

/*!
    Sets the pixmap that will be used as the splash screen's image to
    \a pixmap.
*/
void QSplashScreen::setPixmap( const QPixmap &pixmap )
{
    d->pixmap = pixmap;
    QRect r(0, 0, d->pixmap.size().width(), d->pixmap.size().height());
    resize( d->pixmap.size() );
    move( QApplication::desktop()->screenGeometry().center() - r.center() );
    repaint();
}

/*!
    Returns the pixmap that is used in the splash screen. The image
    does not have any of the text drawn by message() calls.
*/
QPixmap* QSplashScreen::pixmap() const
{
    return &( d->pixmap );
}

/*!
  \internal
*/
void QSplashScreen::drawContents()
{
    QPixmap textPix = d->pixmap;
    QPainter painter( &textPix, this );
    drawContents( &painter );
    setErasePixmap( textPix );
}

/*!
    Draw the contents of the splash screen using painter \a painter.
    The default implementation draws the message passed by message().
    Reimplement this function if you want to do your own drawing on
    the splash screen.
*/
void QSplashScreen::drawContents( QPainter *painter )
{
    painter->setPen( d->currColor );
    QRect r = rect();
    r.setRect( r.x() + 5, r.y() + 5, r.width() - 10, r.height() - 10 );
    painter->drawText( r, d->currAlign, d->currStatus );
}

#endif //QT_NO_SPLASHSCREEN
