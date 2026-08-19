/****************************************************************************
** $Id: qtextbrowser.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QTextBrowser class
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

#include "qtextbrowser.h"
#ifndef QT_NO_TEXTBROWSER
#include "../kernel/qrichtext_p.h"

#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"

#include "qvaluestack.h"
#include "stdio.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qlayout.h"
#include "qbitmap.h"
#include "qtimer.h"
#include "qimage.h"
#include "qsimplerichtext.h"
#include "qdragobject.h"
#include "qurl.h"
#include "qcursor.h"

/*!
    \class QTextBrowser qtextbrowser.h
    \brief The QTextBrowser class provides a rich text browser with hypertext navigation.

    \ingroup advanced
    \ingroup helpsystem
    \ingroup text
    \mainclass

    This class extends QTextEdit (in read-only mode), adding some
    navigation functionality so that users can follow links in
    hypertext documents. The contents of QTextEdit is set with
    setText(), but QTextBrowser has an additional function,
    setSource(), which makes it possible to set the text to a named
    document. The name is looked up in the text view's mime source
    factory. If a document name ends with an anchor (for example, "\c
    #anchor"), the text browser automatically scrolls to that position
    (using scrollToAnchor()). When the user clicks on a hyperlink, the
    browser will call setSource() itself, with the link's \c href
    value as argument. You can track the current source by connetion
    to the sourceChanged() signal.

    QTextBrowser provides backward() and forward() slots which you can
    use to implement Back and Forward buttons. The home() slot sets
    the text to the very first document displayed. The linkClicked()
    signal is emitted when the user clicks a link.

    By using QTextEdit::setMimeSourceFactory() you can provide your
    own subclass of QMimeSourceFactory. This makes it possible to
    access data from anywhere, for example from a network or from a
    database. See QMimeSourceFactory::data() for details.

    If you intend using the mime factory to read the data directly
    from the file system, you may have to specify the encoding for the
    file extension you are using. For example:
    \code
    mimeSourceFactory()->setExtensionType("qml", "text/utf8");
    \endcode
    This is to ensure that the factory is able to resolve the document
    names.

    QTextBrowser interprets the tags it processes in accordance with
    the default style sheet. Change the style sheet with
    \l{setStyleSheet()}; see QStyleSheet for details.

    If you want to provide your users with editable rich text use
    QTextEdit. If you want a text browser without hypertext navigation
    use QTextEdit, and use QTextEdit::setReadOnly() to disable
    editing. If you just need to display a small piece of rich text
    use QSimpleRichText or QLabel.

    <img src=qtextbrowser-m.png> <img src=qtextbrowser-w.png>
*/

class QTextBrowserData
{
public:
    QTextBrowserData():textOrSourceChanged(FALSE) {}

    QValueStack<QString> stack;
    QValueStack<QString> forwardStack;
    QString home;
    QString curmain;
    QString curmark;
    
    /*flag necessary to give the linkClicked() signal some meaningful
      semantics when somebody connected to it calls setText() or
      setSource() */
    bool textOrSourceChanged;
};


/*!
    Constructs an empty QTextBrowser called \a name, with parent \a
    parent.
*/
QTextBrowser::QTextBrowser(QWidget *parent, const char *name)
    : QTextEdit( parent, name )
{
    setReadOnly( TRUE );
    d = new QTextBrowserData;

    viewport()->setMouseTracking( TRUE );
}

/*!
    \reimp
*/
QTextBrowser::~QTextBrowser()
{
    delete d;
}


/*!
    \property QTextBrowser::source
    \brief the name of the displayed document.

    This is a QString::null if no document is displayed or if the
    source is unknown.

    Setting this property uses the mimeSourceFactory() to lookup the
    named document. It also checks for optional anchors and scrolls
    the document accordingly.

    If the first tag in the document is \c{<qt type=detail>}, the
    document is displayed as a popup rather than as new document in
    the browser window itself. Otherwise, the document is displayed
    normally in the text browser with the text set to the contents of
    the named document with setText().

    If you are using the filesystem access capabilities of the mime
    source factory, you must ensure that the factory knows about the
    encoding of specified files; otherwise no data will be available.
    The default factory handles a couple of common file extensions
    such as \c *.html and \c *.txt with reasonable defaults. See
    QMimeSourceFactory::data() for details.
*/

QString QTextBrowser::source() const
{
    if ( d->stack.isEmpty() )
	return QString::null;
    else
	return d->stack.top();
}

/*!
    \property QTextBrowser::undoDepth
    \brief This text browser's undo depth.
*/

/*!
    \property QTextBrowser::overwriteMode
    \brief This text browser's overwrite mode.
*/

/*!
    \property QTextBrowser::modified
    \brief Whether the contents have been modified.
*/

/*!
    \property QTextBrowser::readOnly
    \brief Whether the contents are read only.
*/

/*!
    \property QTextBrowser::undoRedoEnabled
    \brief Whether undo and redo are enabled.
*/



/*!
    Reloads the current set source.
*/

void QTextBrowser::reload()
{
    QString s = d->curmain;
    d->curmain = "";
    setSource( s );
}


void QTextBrowser::setSource(const QString& name)
{
#ifndef QT_NO_CURSOR
    if ( isVisible() )
	qApp->setOverrideCursor( waitCursor );
#endif
    d->textOrSourceChanged = TRUE;
    QString source = name;
    QString mark;
    int hash = name.find('#');
    if ( hash != -1) {
	source = name.left( hash );
	mark = name.mid( hash+1 );
    }

    if ( source.left(5) == "file:" )
	source = source.mid(6);

    QString url = mimeSourceFactory()->makeAbsolute( source, context() );
    QString txt;
    bool dosettext = FALSE;

    if ( !source.isEmpty() && url != d->curmain ) {
	const QMimeSource* m =
		    mimeSourceFactory()->data( source, context() );
	if ( !m ){
	    qWarning("QTextBrowser: no mimesource for %s", source.latin1() );
	}
	else {
	    if ( !QTextDrag::decode( m, txt ) ) {
		qWarning("QTextBrowser: cannot decode %s", source.latin1() );
	    }
	}
	if ( isVisible() ) {
	    QString firstTag = txt.left( txt.find( '>' ) + 1 );
	    if ( firstTag.left( 3 ) == "<qt" && firstTag.contains( "type" ) && firstTag.contains( "detail" ) ) {
		popupDetail( txt, QCursor::pos() );
#ifndef QT_NO_CURSOR
		qApp->restoreOverrideCursor();
#endif
		return;
	    }
	}

	d->curmain = url;
	dosettext = TRUE;
    }

    d->curmark = mark;

    if ( !mark.isEmpty() ) {
	url += "#";
	url += mark;
    }
    if ( !d->home )
	d->home = url;

    if ( d->stack.isEmpty() || d->stack.top() != url)
	d->stack.push( url );

    int stackCount = (int)d->stack.count();
    if ( d->stack.top() == url )
	stackCount--;
    emit backwardAvailable( stackCount > 0 );
    stackCount = (int)d->forwardStack.count();
    if ( d->forwardStack.isEmpty() || d->forwardStack.top() == url )
	stackCount--;
    emit forwardAvailable( stackCount > 0 );

    if ( dosettext )
	QTextEdit::setText( txt, url );

    if ( !mark.isEmpty() )
	scrollToAnchor( mark );
    else
	setContentsPos( 0, 0 );

#ifndef QT_NO_CURSOR
    if ( isVisible() )
	qApp->restoreOverrideCursor();
#endif
    
    emit sourceChanged( url );
}

/*!
    \fn void QTextBrowser::backwardAvailable(bool available)

    This signal is emitted when the availability of backward()
    changes. \a available is FALSE when the user is at home();
    otherwise it is TRUE.
*/

/*!
    \fn void QTextBrowser::forwardAvailable(bool available)

    This signal is emitted when the availability of forward() changes.
    \a available is TRUE after the user navigates backward() and FALSE
    when the user navigates or goes forward().
*/

/*!
    \fn void QTextBrowser::sourceChanged( const QString& src)

    This signal is emitted when the mime source has changed, \a src
    being the new source.
    
    Source changes happen both programmatically when calling
    setSource(), forward(), backword() or home() or when the user
    clicks on links or presses the equivalent key sequences.
*/

/*!  \fn void QTextBrowser::highlighted (const QString &link)

    This signal is emitted when the user has selected but not
    activated a link in the document. \a link is the value of the \c
    href i.e. the name of the target document.
*/

/*!
    \fn void QTextBrowser::linkClicked( const QString& link)

    This signal is emitted when the user clicks a link. The \a link is
    the value of the \c href i.e. the name of the target document.

    The \a link will be the absolute location of the document, based
    on the value of the anchor's href tag and the current context of
    the document.

    \sa anchorClicked(), context()
*/

/*!
    \fn void QTextBrowser::anchorClicked( const QString& name, const QString &link)

    This signal is emitted when the user clicks an anchor. The \a link is
    the value of the \c href i.e. the name of the target document.  The \a name
    is the name of the anchor.

    \sa linkClicked()
*/

/*!
    Changes the document displayed to the previous document in the
    list of documents built by navigating links. Does nothing if there
    is no previous document.

    \sa forward(), backwardAvailable()
*/
void QTextBrowser::backward()
{
    if ( d->stack.count() <= 1)
	return;
    d->forwardStack.push( d->stack.pop() );
    setSource( d->stack.pop() );
    emit forwardAvailable( TRUE );
}

/*!
    Changes the document displayed to the next document in the list of
    documents built by navigating links. Does nothing if there is no
    next document.

    \sa backward(), forwardAvailable()
*/
void QTextBrowser::forward()
{
    if ( d->forwardStack.isEmpty() )
	return;
    setSource( d->forwardStack.pop() );
    emit forwardAvailable( !d->forwardStack.isEmpty() );
}

/*!
    Changes the document displayed to be the first document the
    browser displayed.
*/
void QTextBrowser::home()
{
    if (!d->home.isNull() )
	setSource( d->home );
}

/*!
    The event \a e is used to provide the following keyboard shortcuts:
    \table
    \header \i Keypress	    \i Action
    \row \i Alt+Left Arrow  \i \l backward()
    \row \i Alt+Right Arrow \i \l forward()
    \row \i Alt+Up Arrow    \i \l home()
    \endtable
*/
void QTextBrowser::keyPressEvent( QKeyEvent * e )
{
    if ( e->state() & AltButton ) {
	switch (e->key()) {
	case Key_Right:
	    forward();
	    return;
	case Key_Left:
	    backward();
	    return;
	case Key_Up:
	    home();
	    return;
	}
    }
    QTextEdit::keyPressEvent(e);
}

class QTextDetailPopup : public QWidget
{
public:
    QTextDetailPopup()
	: QWidget ( 0, "automatic QText detail widget", WType_Popup | WDestructiveClose )
	{
	}

protected:

    void mousePressEvent( QMouseEvent*)
	{
	close();
	}
};


void QTextBrowser::popupDetail( const QString& contents, const QPoint& pos )
{

    const int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
    const int vMargin = 8;
    const int hMargin = 12;

    QWidget* popup = new QTextDetailPopup;
    popup->setBackgroundMode( QWidget::NoBackground );

    QSimpleRichText* doc = new QSimpleRichText( contents, popup->font() );
    doc->adjustSize();
    QRect r( 0, 0, doc->width(), doc->height() );

    int w = r.width() + 2*hMargin;
    int h = r.height() + 2*vMargin;

    popup->resize( w + shadowWidth, h + shadowWidth );

    // okay, now to find a suitable location
    //###### we need a global fancy popup positioning somewhere
    popup->move(pos - popup->rect().center());
    if (popup->geometry().right() > QApplication::desktop()->width())
	popup->move( QApplication::desktop()->width() - popup->width(),
		     popup->y() );
    if (popup->geometry().bottom() > QApplication::desktop()->height())
	popup->move( popup->x(),
		     QApplication::desktop()->height() - popup->height() );
    if ( popup->x() < 0 )
	popup->move( 0, popup->y() );
    if ( popup->y() < 0 )
	popup->move( popup->x(), 0 );


    popup->show();

    // now for super-clever shadow stuff.  super-clever mostly in
    // how many window system problems it skirts around.

    QPainter p( popup );
    p.setPen( QApplication::palette().active().foreground() );
    p.drawRect( 0, 0, w, h );
    p.setPen( QApplication::palette().active().mid() );
    p.setBrush( QColor( 255, 255, 240 ) );
    p.drawRect( 1, 1, w-2, h-2 );
    p.setPen( black );

    doc->draw( &p, hMargin, vMargin, r, popup->colorGroup(), 0 );
    delete doc;

    p.drawPoint( w + 5, 6 );
    p.drawLine( w + 3, 6,
		w + 5, 8 );
    p.drawLine( w + 1, 6,
		w + 5, 10 );
    int i;
    for( i=7; i < h; i += 2 )
	p.drawLine( w, i,
		    w + 5, i + 5 );
    for( i = w - i + h; i > 6; i -= 2 )
	p.drawLine( i, h,
		    i + 5, h + 5 );
    for( ; i > 0 ; i -= 2 )
	p.drawLine( 6, h + 6 - i,
		    i + 5, h + 5 );
}

/*!
    \fn void QTextBrowser::setText( const QString &txt )

    \overload

    Sets the text to \a txt.
*/

/*!
    \reimp
*/

void QTextBrowser::setText( const QString &txt, const QString &context )
{
    d->textOrSourceChanged = TRUE;
    d->curmark = "";
    d->curmain = "";
    QTextEdit::setText( txt, context );
}

void QTextBrowser::emitHighlighted( const QString &s ) 
{ 
    emit highlighted( s ); 
}

void QTextBrowser::emitLinkClicked( const QString &s ) 
{ 
    d->textOrSourceChanged = FALSE;
    emit linkClicked( s );
    if ( !d->textOrSourceChanged )
	setSource( s );
}

#endif  // QT_NO_TEXTBROWSER
