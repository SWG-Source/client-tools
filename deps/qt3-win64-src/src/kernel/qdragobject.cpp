/****************************************************************************
** $Id: qdragobject.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qplatformdefs.h"

// POSIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

#ifndef QT_NO_MIME

#include "qdragobject.h"
#include "qtextcodec.h"
#include "qapplication.h"
#include "qpoint.h"
#include "qwidget.h"
#include "qbuffer.h"
#include "qgif.h"
#include "qregexp.h"
#include "qdir.h"
#include <ctype.h>

// both a struct for storing stuff in and a wrapper to avoid polluting
// the name space

class QDragObjectData
{
public:
    QDragObjectData(): hot(0,0) {}
    QPixmap pixmap;
    QPoint hot;
    // store default cursors
    QPixmap *pm_cursor;
};

static QWidget* last_target;

/*!
    After the drag completes, this function will return the QWidget
    which received the drop, or 0 if the data was dropped on another
    application.

    This can be useful for detecting the case where drag and drop is
    to and from the same widget.
*/
QWidget * QDragObject::target()
{
    return last_target;
}

/*!
    \internal
    Sets the target.
*/
void QDragObject::setTarget(QWidget* t)
{
    last_target = t;
}

class QStoredDragData
{
public:
    QStoredDragData() {}
    const char* fmt;
    QByteArray enc;
};


// These pixmaps approximate the images in the Windows User Interface Guidelines.

// XPM

static const char * const move_xpm[] = {
"11 20 3 1",
".	c None",
#if defined(Q_WS_WIN)
"a	c #000000",
"X	c #FFFFFF", // Windows cursor is traditionally white
#else
"a	c #FFFFFF",
"X	c #000000", // X11 cursor is traditionally black
#endif
"aa.........",
"aXa........",
"aXXa.......",
"aXXXa......",
"aXXXXa.....",
"aXXXXXa....",
"aXXXXXXa...",
"aXXXXXXXa..",
"aXXXXXXXXa.",
"aXXXXXXXXXa",
"aXXXXXXaaaa",
"aXXXaXXa...",
"aXXaaXXa...",
"aXa..aXXa..",
"aa...aXXa..",
"a.....aXXa.",
"......aXXa.",
".......aXXa",
".......aXXa",
"........aa."};

/* XPM */
static const char * const copy_xpm[] = {
"24 30 3 1",
".	c None",
"a	c #000000",
"X	c #FFFFFF",
#if defined(Q_WS_WIN) // Windows cursor is traditionally white
"aa......................",
"aXa.....................",
"aXXa....................",
"aXXXa...................",
"aXXXXa..................",
"aXXXXXa.................",
"aXXXXXXa................",
"aXXXXXXXa...............",
"aXXXXXXXXa..............",
"aXXXXXXXXXa.............",
"aXXXXXXaaaa.............",
"aXXXaXXa................",
"aXXaaXXa................",
"aXa..aXXa...............",
"aa...aXXa...............",
"a.....aXXa..............",
"......aXXa..............",
".......aXXa.............",
".......aXXa.............",
"........aa...aaaaaaaaaaa",
#else
"XX......................",
"XaX.....................",
"XaaX....................",
"XaaaX...................",
"XaaaaX..................",
"XaaaaaX.................",
"XaaaaaaX................",
"XaaaaaaaX...............",
"XaaaaaaaaX..............",
"XaaaaaaaaaX.............",
"XaaaaaaXXXX.............",
"XaaaXaaX................",
"XaaXXaaX................",
"XaX..XaaX...............",
"XX...XaaX...............",
"X.....XaaX..............",
"......XaaX..............",
".......XaaX.............",
".......XaaX.............",
"........XX...aaaaaaaaaaa",
#endif
".............aXXXXXXXXXa",
".............aXXXXXXXXXa",
".............aXXXXaXXXXa",
".............aXXXXaXXXXa",
".............aXXaaaaaXXa",
".............aXXXXaXXXXa",
".............aXXXXaXXXXa",
".............aXXXXXXXXXa",
".............aXXXXXXXXXa",
".............aaaaaaaaaaa"};

/* XPM */
static const char * const link_xpm[] = {
"24 30 3 1",
".	c None",
"a	c #000000",
"X	c #FFFFFF",
#if defined(Q_WS_WIN) // Windows cursor is traditionally white
"aa......................",
"aXa.....................",
"aXXa....................",
"aXXXa...................",
"aXXXXa..................",
"aXXXXXa.................",
"aXXXXXXa................",
"aXXXXXXXa...............",
"aXXXXXXXXa..............",
"aXXXXXXXXXa.............",
"aXXXXXXaaaa.............",
"aXXXaXXa................",
"aXXaaXXa................",
"aXa..aXXa...............",
"aa...aXXa...............",
"a.....aXXa..............",
"......aXXa..............",
".......aXXa.............",
".......aXXa.............",
"........aa...aaaaaaaaaaa",
#else
"XX......................",
"XaX.....................",
"XaaX....................",
"XaaaX...................",
"XaaaaX..................",
"XaaaaaX.................",
"XaaaaaaX................",
"XaaaaaaaX...............",
"XaaaaaaaaX..............",
"XaaaaaaaaaX.............",
"XaaaaaaXXXX.............",
"XaaaXaaX................",
"XaaXXaaX................",
"XaX..XaaX...............",
"XX...XaaX...............",
"X.....XaaX..............",
"......XaaX..............",
".......XaaX.............",
".......XaaX.............",
"........XX...aaaaaaaaaaa",
#endif
".............aXXXXXXXXXa",
".............aXXXaaaaXXa",
".............aXXXXaaaXXa",
".............aXXXaaaaXXa",
".............aXXaaaXaXXa",
".............aXXaaXXXXXa",
".............aXXaXXXXXXa",
".............aXXXaXXXXXa",
".............aXXXXXXXXXa",
".............aaaaaaaaaaa"};

#ifndef QT_NO_DRAGANDDROP

// the universe's only drag manager
QDragManager * qt_dnd_manager = 0;


QDragManager::QDragManager()
    : QObject( qApp, "global drag manager" )
{
    n_cursor = 3;
    pm_cursor = new QPixmap[n_cursor];
    pm_cursor[0] = QPixmap((const char **)move_xpm);
    pm_cursor[1] = QPixmap((const char **)copy_xpm);
    pm_cursor[2] = QPixmap((const char **)link_xpm);
#if defined(Q_WS_X11)
    createCursors(); // Xcursors cache can hold only 8 bitmaps (4 cursors)
#endif
    object = 0;
    dragSource = 0;
    dropWidget = 0;
    if ( !qt_dnd_manager )
	qt_dnd_manager = this;
    beingCancelled = FALSE;
    restoreCursor = FALSE;
    willDrop = FALSE;
}


QDragManager::~QDragManager()
{
#ifndef QT_NO_CURSOR
    if ( restoreCursor )
	QApplication::restoreOverrideCursor();
#endif
    qt_dnd_manager = 0;
    delete [] pm_cursor;
}

#endif


/*!
    Constructs a drag object called \a name, which is a child of \a
    dragSource.

    Note that the drag object will be deleted when \a dragSource is
    deleted.
*/

QDragObject::QDragObject( QWidget * dragSource, const char * name )
    : QObject( dragSource, name )
{
    d = new QDragObjectData();
    d->pm_cursor = 0;
#ifndef QT_NO_DRAGANDDROP
    if ( !qt_dnd_manager && qApp )
	(void)new QDragManager();
#endif
}


/*!
    Destroys the drag object, canceling any drag and drop operation in
    which it is involved, and frees up the storage used.
*/

QDragObject::~QDragObject()
{
#ifndef QT_NO_DRAGANDDROP
    if ( qt_dnd_manager && qt_dnd_manager->object == this )
	qt_dnd_manager->cancel( FALSE );
    if ( d->pm_cursor ) {
	for ( int i = 0; i < qt_dnd_manager->n_cursor; i++ )
	    qt_dnd_manager->pm_cursor[i] = d->pm_cursor[i];
	delete [] d->pm_cursor;
    }
#endif
    delete d;
}

#ifndef QT_NO_DRAGANDDROP
/*!
    Set the pixmap \a pm to display while dragging the object. The
    platform-specific implementation will use this where it can - so
    provide a small masked pixmap, and do not assume that the user
    will actually see it. For example, cursors on Windows 95 are of
    limited size.

    The \a hotspot is the point on (or off) the pixmap that should be
    under the cursor as it is dragged. It is relative to the top-left
    pixel of the pixmap.

    \warning We have seen problems with drag cursors on different
    graphics hardware and driver software on Windows. Setting the
    graphics acceleration in the display settings down one tick solved
    the problems in all cases.
*/
void QDragObject::setPixmap(QPixmap pm, const QPoint& hotspot)
{
    d->pixmap = pm;
    d->hot = hotspot;
    if ( qt_dnd_manager && qt_dnd_manager->object == this )
	qt_dnd_manager->updatePixmap();
}

/*!
    \overload
    Uses a hotspot that positions the pixmap below and to the right of
    the mouse pointer. This allows the user to clearly see the point
    on the window which they are dragging the data onto.
*/
void QDragObject::setPixmap(QPixmap pm)
{
    setPixmap(pm,QPoint(-10, -10));
}

/*!
    Returns the currently set pixmap (which \link QPixmap::isNull()
    isNull()\endlink if none is set).
*/
QPixmap QDragObject::pixmap() const
{
    return d->pixmap;
}

/*!
    Returns the currently set pixmap hotspot.
*/
QPoint QDragObject::pixmapHotSpot() const
{
    return d->hot;
}

#if 0

// ## reevaluate for Qt 4
/*!
    Set the \a cursor used when dragging in mode \a m.
    Note: X11 only allow bitmaps for cursors.
*/
void QDragObject::setCursor( DragMode m, const QPixmap &cursor )
{
    if ( d->pm_cursor == 0 ) {
	// safe default cursors
	d->pm_cursor = new QPixmap[qt_dnd_manager->n_cursor];
	for ( int i = 0; i < qt_dnd_manager->n_cursor; i++ )
	    d->pm_cursor[i] = qt_dnd_manager->pm_cursor[i];
    }

    int index;
    switch ( m ) {
    case DragCopy:
	index = 1;
	break;
    case DragLink:
	index = 2;
	break;
    default:
	index = 0;
	break;
    }

    // override default cursor
    for ( index = 0; index < qt_dnd_manager->n_cursor; index++ )
	qt_dnd_manager->pm_cursor[index] = cursor;
}

/*!
    Returns the cursor used when dragging in mode \a m, or null if no cursor
    has been set for that mode.
*/
QPixmap *QDragObject::cursor( DragMode m ) const
{
    if ( !d->pm_cursor )
	return 0;

    int index;
    switch ( m ) {
    case DragCopy:
	index = 1;
	break;
    case DragLink:
	index = 2;
	break;
    default:
	index = 0;
	break;
    }

    return qt_dnd_manager->pm_cursor+index;
}

#endif // 0

/*!
    Starts a drag operation using the contents of this object, using
    DragDefault mode.

    The function returns TRUE if the caller should delete the original
    copy of the dragged data (but see target()); otherwise returns
    FALSE.

    If the drag contains \e references to information (e.g. file names
    in a QUriDrag are references) then the return value should always
    be ignored, as the target is expected to manipulate the
    referred-to content directly. On X11 the return value should
    always be correct anyway, but on Windows this is not necessarily
    the case (e.g. the file manager starts a background process to
    move files, so the source \e{must not} delete the files!)

    Note that on Windows the drag operation will spin a blocking modal
    event loop that will not dispatch any QTimers.
*/
bool QDragObject::drag()
{
    return drag( DragDefault );
}


/*!
    Starts a drag operation using the contents of this object, using
    \c DragMove mode. Be sure to read the constraints described in
    drag().

    \sa drag() dragCopy() dragLink()
*/
bool QDragObject::dragMove()
{
    return drag( DragMove );
}


/*!
    Starts a drag operation using the contents of this object, using
    \c DragCopy mode. Be sure to read the constraints described in
    drag().

    \sa drag() dragMove() dragLink()
*/
void QDragObject::dragCopy()
{
    (void)drag( DragCopy );
}

/*!
    Starts a drag operation using the contents of this object, using
    \c DragLink mode. Be sure to read the constraints described in
    drag().

    \sa drag() dragCopy() dragMove()
*/
void QDragObject::dragLink()
{
    (void)drag( DragLink );
}


/*!
    \enum QDragObject::DragMode

    This enum describes the possible drag modes.

    \value DragDefault  The mode is determined heuristically.
    \value DragCopy  The data is copied, never moved.
    \value DragMove  The data is moved, if dragged at all.
    \value DragLink  The data is linked, if dragged at all.
    \value DragCopyOrMove  The user chooses the mode by using a
			   control key to switch from the default.
*/


/*!
    \overload
    Starts a drag operation using the contents of this object.

    At this point, the object becomes owned by Qt, not the
    application. You should not delete the drag object or anything it
    references. The actual transfer of data to the target application
    will be done during future event processing - after that time the
    drag object will be deleted.

    Returns TRUE if the dragged data was dragged as a \e move,
    indicating that the caller should remove the original source of
    the data (the drag object must continue to have a copy); otherwise
    returns FALSE.

    The \a mode specifies the drag mode (see
    \l{QDragObject::DragMode}.) Normally one of the simpler drag(),
    dragMove(), or dragCopy() functions would be used instead.
*/
bool QDragObject::drag( DragMode mode )
{
    if ( qt_dnd_manager )
	return qt_dnd_manager->drag( this, mode );
    else
	return FALSE;
}

#endif


/*!
    Returns a pointer to the drag source where this object originated.
*/

QWidget * QDragObject::source()
{
    if ( parent() && parent()->isWidgetType() )
	return (QWidget *)parent();
    else
	return 0;
}


/*!
    \class QDragObject qdragobject.h

    \brief The QDragObject class encapsulates MIME-based data
    transfer.

    \ingroup draganddrop

    QDragObject is the base class for all data that needs to be
    transferred between and within applications, both for drag and
    drop and for the \link qclipboard.html clipboard\endlink.

    See the \link dnd.html Drag-and-drop documentation\endlink for an
    overview of how to provide drag and drop in your application.

    See the QClipboard documentation for an overview of how to provide
    cut-and-paste in your application.

    The drag() function is used to start a drag operation. You can
    specify the \l DragMode in the call or use one of the convenience
    functions dragCopy(), dragMove() or dragLink(). The drag source
    where the data originated is retrieved with source(). If the data
    was dropped on a widget within the application, target() will
    return a pointer to that widget. Specify the pixmap to display
    during the drag with setPixmap().
*/

static
void stripws(QCString& s)
{
    int f;
    while ( (f=s.find(' ')) >= 0 )
	s.remove(f,1);
}

static
const char * staticCharset(int i)
{
    static QCString localcharset;

    switch ( i ) {
      case 0:
	return "UTF-8";
      case 1:
	return "ISO-10646-UCS-2";
      case 2:
	return ""; // in the 3rd place - some Xdnd targets might only look at 3
      case 3:
	if ( localcharset.isNull() ) {
	    QTextCodec *localCodec = QTextCodec::codecForLocale();
	    if ( localCodec ) {
		localcharset = localCodec->name();
		localcharset = localcharset.lower();
		stripws(localcharset);
	    } else {
		localcharset = "";
	    }
	}
	return localcharset;
    }
    return 0;
}


class QTextDragPrivate {
public:
    QTextDragPrivate();

    enum { nfmt=4 };

    QString txt;
    QCString fmt[nfmt];
    QCString subtype;

    void setSubType(const QCString & st)
    {
	subtype = st.lower();
	for ( int i=0; i<nfmt; i++ ) {
	    fmt[i] = "text/";
	    fmt[i].append(subtype);
	    QCString cs = staticCharset(i);
	    if ( !cs.isEmpty() ) {
		fmt[i].append(";charset=");
		fmt[i].append(cs);
	    }
	}
    }
};

inline QTextDragPrivate::QTextDragPrivate()
{
    setSubType("plain");
}

/*!
    Sets the MIME subtype of the text being dragged to \a st. The
    default subtype is "plain", so the default MIME type of the text
    is "text/plain". You might use this to declare that the text is
    "text/html" by calling setSubtype("html").
*/
void QTextDrag::setSubtype( const QCString & st)
{
    d->setSubType(st);
}

/*!
    \class QTextDrag qdragobject.h

    \brief The QTextDrag class is a drag and drop object for
    transferring plain and Unicode text.

    \ingroup draganddrop

    Plain text is passed in a QString which may contain multiple lines
    (i.e. may contain newline characters). The drag target will receive
    the newlines according to the runtime environment, e.g. LF on Unix,
    and CRLF on Windows.

    Qt provides no built-in mechanism for delivering only a single-line.

    For more information about drag and drop, see the QDragObject class
    and the \link dnd.html drag and drop documentation\endlink.
*/


/*!
    Constructs a text drag object and sets its data to \a text. \a
    dragSource must be the drag source; \a name is the object name.
*/

QTextDrag::QTextDrag( const QString &text,
		      QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
    d = new QTextDragPrivate;
    setText( text );
}


/*!
    Constructs a default text drag object. \a dragSource must be the
    drag source; \a name is the object name.
*/

QTextDrag::QTextDrag( QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name )
{
    d = new QTextDragPrivate;
}


/*!
    Destroys the text drag object and frees up all allocated
    resources.
*/
QTextDrag::~QTextDrag()
{
    delete d;
}


/*!
    Sets the text to be dragged to \a text. You will need to call this
    if you did not pass the text during construction.
*/
void QTextDrag::setText( const QString &text )
{
    d->txt = text;
}


/*!
    \reimp
*/
const char * QTextDrag::format(int i) const
{
    if ( i >= d->nfmt )
	return 0;
    return d->fmt[i];
}

QTextCodec* qt_findcharset(const QCString& mimetype)
{
    int i=mimetype.find("charset=");
    if ( i >= 0 ) {
	QCString cs = mimetype.mid(i+8);
	stripws(cs);
	i = cs.find(';');
	if ( i >= 0 )
	    cs = cs.left(i);
	// win98 often has charset=utf16, and we need to get the correct codec for
	// it to be able to get Unicode text drops.
	if ( cs == "utf16" )
	    cs = "ISO-10646-UCS-2";
	// May return 0 if unknown charset
	return QTextCodec::codecForName(cs);
    }
    // no charset=, use locale
    return QTextCodec::codecForLocale();
}

static QTextCodec *codecForHTML(const QCString &ba)
{
    // determine charset
    int mib = 0;
    int pos;
    QTextCodec *c = 0;

    if (ba.size() > 1 && (((uchar)ba[0] == 0xfe && (uchar)ba[1] == 0xff)
			  || ((uchar)ba[0] == 0xff && (uchar)ba[1] == 0xfe))) {
	mib = 1000; // utf16
    } else if (ba.size() > 2
	       && (uchar)ba[0] == 0xef
	       && (uchar)ba[1] == 0xbb
	       && (uchar)ba[2] == 0xbf) {
	mib = 106; // utf-8
    } else {
	pos = 0;
	while ((pos = ba.find("<meta http-equiv=", pos, FALSE)) != -1) {
	    int end = ba.find('>', pos+1);
	    if (end == -1)
		break;
	    pos = ba.find("charset=", pos, FALSE) + (int)strlen("charset=");
	    if (pos != -1 && pos < end) {
		int pos2 = ba.find('\"', pos+1);
		QCString cs = ba.mid(pos, pos2-pos);
		c = QTextCodec::codecForName(cs);
		if (c)
		    return c;
	    }
	    pos = end;
	}
    }
    if (mib)
	c = QTextCodec::codecForMib(mib);

    return c;
}

static
QTextCodec* findcodec(const QMimeSource* e)
{
    QTextCodec* r = 0;
    const char* f;
    int i;
    for ( i=0; (f=e->format(i)); i++ ) {
	bool html = !qstrnicmp(f, "text/html", 9);
	if (html)
	    r = codecForHTML(QCString(e->encodedData(f)));
	if (!r)
	    r = qt_findcharset(QCString(f).lower());
	if (r)
	    return r;
    }
    return 0;
}



/*!
    \reimp
*/
QByteArray QTextDrag::encodedData(const char* mime) const
{
    QCString r;
    if ( 0==qstrnicmp(mime,"text/",5) ) {
	QCString m(mime);
	m = m.lower();
	QTextCodec *codec = qt_findcharset(m);
	if ( !codec )
	    return r;
	QString text( d->txt );
#if defined(Q_WS_WIN)
	int index = text.find( QString::fromLatin1("\r\n"), 0 );
	while ( index != -1 ) {
	    text.replace( index, 2, QChar('\n') );
	    index = text.find( "\r\n", index );
	}
#endif
	r = codec->fromUnicode(text);
	if (!codec || codec->mibEnum() != 1000) {
	    // Don't include NUL in size (QCString::resize() adds NUL)
#if defined(Q_WS_WIN)
	    // This is needed to ensure the \0 isn't lost on Windows 95
	    if ( qWinVersion() & Qt::WV_DOS_based )
		((QByteArray&)r).resize(r.length()+1);
	    else
#endif
		((QByteArray&)r).resize(r.length());
	}
    }
    return r;
}

/*!
    Returns TRUE if the information in \a e can be decoded into a
    QString; otherwise returns FALSE.

    \sa decode()
*/
bool QTextDrag::canDecode( const QMimeSource* e )
{
    const char* f;
    for (int i=0; (f=e->format(i)); i++) {
	if ( 0==qstrnicmp(f,"text/",5) ) {
	    return findcodec(e) != 0;
	}
    }
    return 0;
}

/*!
    \overload

    Attempts to decode the dropped information in \a e into \a str.
    Returns TRUE if successful; otherwise returns FALSE. If \a subtype
    is null, any text subtype is accepted; otherwise only the
    specified \a subtype is accepted.

    \sa canDecode()
*/
bool QTextDrag::decode( const QMimeSource* e, QString& str, QCString& subtype )
{
    if(!e)
	return FALSE;

    if ( e->cacheType == QMimeSource::Text ) {
	str = *e->cache.txt.str;
	subtype = *e->cache.txt.subtype;
	return TRUE;
    }

    const char* mime;
    for (int i=0; (mime = e->format(i)); i++) {
	if ( 0==qstrnicmp(mime,"text/",5) ) {
	    QCString m(mime);
	    m = m.lower();
	    int semi = m.find(';');
	    if ( semi < 0 )
		semi = m.length();
	    QCString foundst = m.mid(5,semi-5);
	    if ( subtype.isNull() || foundst == subtype ) {
		bool html = !qstrnicmp(mime, "text/html", 9);
		QTextCodec* codec = 0;
                if (html) {
                    QByteArray bytes = e->encodedData(mime);
		    // search for the charset tag in the HTML
		    codec = codecForHTML(QCString(bytes.data(), bytes.size()));
                }
		if (!codec)
		    codec = qt_findcharset(m);
		if ( codec ) {
		    QByteArray payload;

		    payload = e->encodedData(mime);
		    if ( payload.size() ) {
			int l;
			if ( codec->mibEnum() != 1000) {
			    // length is at NUL or payload.size()
			    l = 0;
			    while ( l < (int)payload.size() && payload[l] )
				l++;
			} else {
			    l = payload.size();
			}

			str = codec->toUnicode(payload,l);

			if ( subtype.isNull() )
			    subtype = foundst;

			QMimeSource *m = (QMimeSource*)e;
			m->clearCache();
			m->cacheType = QMimeSource::Text;
			m->cache.txt.str = new QString( str );
			m->cache.txt.subtype = new QCString( subtype );

			return TRUE;
		    }
		}
	    }
	}
    }
    return FALSE;
}

/*!
    Attempts to decode the dropped information in \a e into \a str.
    Returns TRUE if successful; otherwise returns FALSE.

    \sa canDecode()
*/
bool QTextDrag::decode( const QMimeSource* e, QString& str )
{
    QCString st;
    return decode(e,str,st);
}


/*
  QImageDrag could use an internal MIME type for communicating QPixmaps
  and QImages rather than always converting to raw data. This is available
  for that purpose and others. It is not currently used.
*/
class QImageDragData
{
public:
};


/*!
    \class QImageDrag qdragobject.h

    \brief The QImageDrag class provides a drag and drop object for
    transferring images.

    \ingroup draganddrop

    Images are offered to the receiving application in multiple
    formats, determined by Qt's \link QImage::outputFormats() output
    formats\endlink.

    For more information about drag and drop, see the QDragObject
    class and the \link dnd.html drag and drop documentation\endlink.
*/

/*!
    Constructs an image drag object and sets its data to \a image. \a
    dragSource must be the drag source; \a name is the object name.
*/

QImageDrag::QImageDrag( QImage image,
			QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name ),
	d(0)
{
    setImage( image );
}

/*!
    Constructs a default image drag object. \a dragSource must be the
    drag source; \a name is the object name.
*/

QImageDrag::QImageDrag( QWidget * dragSource, const char * name )
    : QDragObject( dragSource, name ),
	d(0)
{
}


/*!
    Destroys the image drag object and frees up all allocated
    resources.
*/

QImageDrag::~QImageDrag()
{
    // nothing
}


/*!
    Sets the image to be dragged to \a image. You will need to call
    this if you did not pass the image during construction.
*/
void QImageDrag::setImage( QImage image )
{
    img = image; // ### detach?
    ofmts = QImage::outputFormats();
    ofmts.remove("PBM"); // remove non-raw PPM
    if ( image.depth()!=32 ) {
	// BMP better than PPM for paletted images
	if ( ofmts.remove("BMP") ) // move to front
	    ofmts.insert(0,"BMP");
    }
    // PNG is best of all
    if ( ofmts.remove("PNG") ) // move to front
	ofmts.insert(0,"PNG");

    if(cacheType == QMimeSource::NoCache) { //cache it
	cacheType = QMimeSource::Graphics;
	cache.gfx.img = new QImage( img );
	cache.gfx.pix = 0;
    }
}

/*!
    \reimp
*/
const char * QImageDrag::format(int i) const
{
    if ( i < (int)ofmts.count() ) {
	static QCString str;
	str.sprintf("image/%s",(((QImageDrag*)this)->ofmts).at(i));
	str = str.lower();
	if ( str == "image/pbmraw" )
	    str = "image/ppm";
	return str;
    } else {
	return 0;
    }
}

/*!
    \reimp
*/
QByteArray QImageDrag::encodedData(const char* fmt) const
{
    if ( qstrnicmp( fmt, "image/", 6 )==0 ) {
	QCString f = fmt+6;
	QByteArray data;
	QBuffer w( data );
	w.open( IO_WriteOnly );
	QImageIO io( &w, f.upper() );
	io.setImage( img );
	if  ( !io.write() )
	    return QByteArray();
	w.close();
	return data;
    } else {
	return QByteArray();
    }
}

/*!
    Returns TRUE if the information in mime source \a e can be decoded
    into an image; otherwise returns FALSE.

    \sa decode()
*/
bool QImageDrag::canDecode( const QMimeSource* e ) {
    QStrList fileFormats = QImageIO::inputFormats();

    fileFormats.first();
    while ( fileFormats.current()) {
	QCString format = fileFormats.current();
	QCString type = "image/" + format.lower();
	if ( e->provides(type.data()))
	    return TRUE;
	fileFormats.next();
    }

    return FALSE;
}

/*!
    Attempts to decode the dropped information in mime source \a e
    into \a img. Returns TRUE if successful; otherwise returns FALSE.

    \sa canDecode()
*/
bool QImageDrag::decode( const QMimeSource* e, QImage& img )
{
    if ( !e )
	return FALSE;
    if ( e->cacheType == QMimeSource::Graphics ) {
	img = *e->cache.gfx.img;
	return TRUE;
    }

    QByteArray payload;
    QStrList fileFormats = QImageIO::inputFormats();
    // PNG is best of all
    if ( fileFormats.remove("PNG") ) // move to front
	fileFormats.insert(0,"PNG");
    fileFormats.first();
    while ( fileFormats.current() ) {
	QCString format = fileFormats.current();
	fileFormats.next();

       	QCString type = "image/" + format.lower();
	if ( ! e->provides( type.data() ) ) continue;
	payload = e->encodedData( type.data() );
	if ( !payload.isEmpty() )
	    break;
    }

    if ( payload.isEmpty() )
	return FALSE;

    img.loadFromData(payload);
    if ( img.isNull() )
	return FALSE;
    QMimeSource *m = (QMimeSource*)e;
    m->clearCache();
    m->cacheType = QMimeSource::Graphics;
    m->cache.gfx.img = new QImage( img );
    m->cache.gfx.pix = 0;
    return TRUE;
}

/*!
    \overload

    Attempts to decode the dropped information in mime source \a e
    into pixmap \a pm. Returns TRUE if successful; otherwise returns
    FALSE.

    This is a convenience function that converts to a QPixmap via a
    QImage.

    \sa canDecode()
*/
bool QImageDrag::decode( const QMimeSource* e, QPixmap& pm )
{
    if ( !e )
	return FALSE;

    if ( e->cacheType == QMimeSource::Graphics && e->cache.gfx.pix) {
	pm = *e->cache.gfx.pix;
	return TRUE;
    }

    QImage img;
    // We avoid dither, since the image probably came from this display
    if ( decode( e, img ) ) {
	if ( !pm.convertFromImage( img, AvoidDither ) )
	    return FALSE;
	// decode initialized the cache for us

	QMimeSource *m = (QMimeSource*)e;
	m->cache.gfx.pix = new QPixmap( pm );
	return TRUE;
    }
    return FALSE;
}




/*!
    \class QStoredDrag qdragobject.h
    \brief The QStoredDrag class provides a simple stored-value drag object for arbitrary MIME data.

    \ingroup draganddrop

    When a block of data has only one representation, you can use a
    QStoredDrag to hold it.

    For more information about drag and drop, see the QDragObject
    class and the \link dnd.html drag and drop documentation\endlink.
*/

/*!
    Constructs a QStoredDrag. The \a dragSource and \a name are passed
    to the QDragObject constructor, and the format is set to \a
    mimeType.

    The data will be unset. Use setEncodedData() to set it.
*/
QStoredDrag::QStoredDrag( const char* mimeType, QWidget * dragSource, const char * name ) :
    QDragObject(dragSource,name)
{
    d = new QStoredDragData();
    d->fmt = qstrdup(mimeType);
}

/*!
    Destroys the drag object and frees up all allocated resources.
*/
QStoredDrag::~QStoredDrag()
{
    delete [] (char*)d->fmt;
    delete d;
}

/*!
    \reimp
*/
const char * QStoredDrag::format(int i) const
{
    if ( i==0 )
	return d->fmt;
    else
	return 0;
}


/*!
    Sets the encoded data of this drag object to \a encodedData. The
    encoded data is what's delivered to the drop sites. It must be in
    a strictly defined and portable format.

    The drag object can't be dropped (by the user) until this function
    has been called.
*/

void QStoredDrag::setEncodedData( const QByteArray & encodedData )
{
    d->enc = encodedData.copy();
}

/*!
    Returns the stored data. \a m contains the data's format.

    \sa setEncodedData()
*/
QByteArray QStoredDrag::encodedData(const char* m) const
{
    if ( !qstricmp(m,d->fmt) )
	return d->enc;
    else
	return QByteArray();
}


/*!
    \class QUriDrag qdragobject.h
    \brief The QUriDrag class provides a drag object for a list of URI references.

    \ingroup draganddrop

    URIs are a useful way to refer to files that may be distributed
    across multiple machines. A URI will often refer to a file on a
    machine local to both the drag source and the drop target, so the
    URI can be equivalent to passing a file name but is more
    extensible.

    Use URIs in Unicode form so that the user can comfortably edit and
    view them. For use in HTTP or other protocols, use the correctly
    escaped ASCII form.

    You can convert a list of file names to file URIs using
    setFileNames(), or into human-readble form with setUnicodeUris().

    Static functions are provided to convert between filenames and
    URIs, e.g. uriToLocalFile() and localFileToUri(), and to and from
    human-readable form, e.g. uriToUnicodeUri(), unicodeUriToUri().
    You can also decode URIs from a mimesource into a list with
    decodeLocalFiles() and decodeToUnicodeUris().
*/

/*!
    Constructs an object to drag the list of URIs in \a uris. The \a
    dragSource and \a name arguments are passed on to QStoredDrag.
    Note that URIs are always in escaped UTF8 encoding.
*/
QUriDrag::QUriDrag( QStrList uris,
	    QWidget * dragSource, const char * name ) :
    QStoredDrag( "text/uri-list", dragSource, name )
{
    setUris(uris);
}

/*!
    Constructs an object to drag. You must call setUris() before you
    start the drag(). Passes \a dragSource and \a name to the
    QStoredDrag constructor.
*/
QUriDrag::QUriDrag( QWidget * dragSource, const char * name ) :
    QStoredDrag( "text/uri-list", dragSource, name )
{
}

/*!
    Destroys the object.
*/
QUriDrag::~QUriDrag()
{
}

/*!
    Changes the list of \a uris to be dragged.

    Note that URIs are always in escaped UTF8 encoding.
*/
void QUriDrag::setUris( QStrList uris )
{
    QByteArray a;
    int c=0;
    for ( const char* s = uris.first(); s; s = uris.next() ) {
	int l = qstrlen(s);
	a.resize(c+l+2);
	memcpy(a.data()+c,s,l);
	memcpy(a.data()+c+l,"\r\n",2);
	c+=l+2;
    }
    a.resize(c+1);
    a[c] = 0;
    setEncodedData(a);
}


/*!
    Returns TRUE if decode() would be able to decode \a e; otherwise
    returns FALSE.
*/
bool QUriDrag::canDecode( const QMimeSource* e )
{
    return e->provides( "text/uri-list" );
}

/*!
    Decodes URIs from \a e, placing the result in \a l (which is first
    cleared).

    Returns TRUE if \a e contained a valid list of URIs; otherwise
    returns FALSE.
*/
bool QUriDrag::decode( const QMimeSource* e, QStrList& l )
{
    QByteArray payload = e->encodedData( "text/uri-list" );
    if ( payload.size() ) {
	l.clear();
	l.setAutoDelete(TRUE);
	uint c=0;
	const char* d = payload.data();
	while (c < payload.size() && d[c]) {
	    uint f = c;
	    // Find line end
	    while (c < payload.size() && d[c] && d[c]!='\r'
		    && d[c] != '\n')
		c++;
	    QCString s(d+f,c-f+1);
	    if ( s[0] != '#' ) // non-comment?
		l.append( s );
	    // Skip junk
	    while (c < payload.size() && d[c] &&
		    (d[c]=='\n' || d[c]=='\r'))
		c++;
	}
	return TRUE;
    }
    return FALSE;
}

static uint htod( int h )
{
    if ( isdigit(h) )
	return h - '0';
    return tolower( h ) - 'a' + 10;
}

/*!
  \fn QUriDrag::setFilenames( const QStringList & )
  \obsolete

  Use setFileNames() instead (notice the N).
*/

/*!
    Sets the URIs to be the local-file URIs equivalent to \a fnames.

    \sa localFileToUri(), setUris()
*/
void QUriDrag::setFileNames( const QStringList & fnames )
{
    QStrList uris;
    for ( QStringList::ConstIterator i = fnames.begin();
    i != fnames.end(); ++i ) {
	QCString fileUri = localFileToUri(*i);
	if (!fileUri.isEmpty())
	    uris.append(fileUri);
    }
    setUris( uris );
}

/*!
    Sets the URIs in \a uuris to be the Unicode URIs (only useful for
    displaying to humans).

    \sa localFileToUri(), setUris()
*/
void QUriDrag::setUnicodeUris( const QStringList & uuris )
{
    QStrList uris;
    for ( QStringList::ConstIterator i = uuris.begin();
	    i != uuris.end(); ++i )
	uris.append( unicodeUriToUri(*i) );
    setUris( uris );
}

/*!
    Returns the URI equivalent of the Unicode URI given in \a uuri
    (only useful for displaying to humans).

    \sa uriToLocalFile()
*/
QCString QUriDrag::unicodeUriToUri(const QString& uuri)
{
    QCString utf8 = uuri.utf8();
    QCString escutf8;
    int n = utf8.length();
    bool isFile = uuri.startsWith("file://");
    for (int i=0; i<n; i++) {
	if ( utf8[i] >= 'a' && utf8[i] <= 'z'
	  || utf8[i] == '/'
	  || utf8[i] >= '0' && utf8[i] <= '9'
	  || utf8[i] >= 'A' && utf8[i] <= 'Z'

	  || utf8[i] == '-' || utf8[i] == '_'
	  || utf8[i] == '.' || utf8[i] == '!'
	  || utf8[i] == '~' || utf8[i] == '*'
	  || utf8[i] == '(' || utf8[i] == ')'
	  || utf8[i] == '\''

	  // Allow this through, so that all URI-references work.
          || (!isFile && utf8[i] == '#')

	  || utf8[i] == ';'
	  || utf8[i] == '?' || utf8[i] == ':'
	  || utf8[i] == '@' || utf8[i] == '&'
	  || utf8[i] == '=' || utf8[i] == '+'
	  || utf8[i] == '$' || utf8[i] == ',' )
	{
	    escutf8 += utf8[i];
	} else {
	    // Everything else is escaped as %HH
	    QCString s(4);
	    s.sprintf("%%%02x",(uchar)utf8[i]);
	    escutf8 += s;
	}
    }
    return escutf8;
}

/*!
    Returns the URI equivalent to the absolute local file \a filename.

    \sa uriToLocalFile()
*/
QCString QUriDrag::localFileToUri(const QString& filename)
{
    QString r = filename;

    //check that it is an absolute file
    if (QDir::isRelativePath(r))
	return QCString();

#ifdef Q_WS_WIN


    bool hasHost = FALSE;
    // convert form network path
    if (r.left(2) == "\\\\" || r.left(2) == "//") {
	r.remove(0, 2);
	hasHost = TRUE;
    }

    // Slosh -> Slash
    int slosh;
    while ( (slosh=r.find('\\')) >= 0 ) {
	r[slosh] = '/';
    }

    // Drive
    if ( r[0] != '/' && !hasHost)
	r.insert(0,'/');

#endif
#if defined ( Q_WS_X11 ) && 0
    // URL without the hostname is considered to be errorneous by XDnD.
    // See: http://www.newplanetsoftware.com/xdnd/dragging_files.html
    // This feature is not active because this would break dnd between old and new qt apps.
    char hostname[257];
    if ( gethostname( hostname, 255 ) == 0 ) {
	hostname[256] = '\0';
	r.prepend( QString::fromLatin1( hostname ) );
    }
#endif
    return unicodeUriToUri(QString("file://" + r));
}

/*!
    Returns the Unicode URI (only useful for displaying to humans)
    equivalent of \a uri.

    Note that URIs are always in escaped UTF8 encoding.

    \sa localFileToUri()
*/
QString QUriDrag::uriToUnicodeUri(const char* uri)
{
    QCString utf8;

    while (*uri) {
	switch (*uri) {
	  case '%': {
		uint ch = (uchar) uri[1];
		if ( ch && uri[2] ) {
		    ch = htod( ch ) * 16 + htod( (uchar) uri[2] );
		    utf8 += (char) ch;
		    uri += 2;
		}
	    }
	    break;
	  default:
	    utf8 += *uri;
	}
	++uri;
    }

    return QString::fromUtf8(utf8);
}

/*!
    Returns the name of a local file equivalent to \a uri or a null
    string if \a uri is not a local file.

    Note that URIs are always in escaped UTF8 encoding.

    \sa localFileToUri()
*/
QString QUriDrag::uriToLocalFile(const char* uri)
{
    QString file;

    if (!uri)
	return file;
    if (0==qstrnicmp(uri,"file:/",6)) // It is a local file uri
	uri += 6;
    else if (QString(uri).find(":/") != -1) // It is a different scheme uri
	return file;

    bool local = uri[0] != '/' || ( uri[0] != '\0' && uri[1] == '/' );
#ifdef Q_WS_X11
    // do we have a hostname?
    if ( !local && uri[0] == '/' && uri[2] != '/' ) {
	// then move the pointer to after the 'hostname/' part of the uri
	const char* hostname_end = strchr( uri+1, '/' );
	if ( hostname_end != NULL ) {
	    char hostname[ 257 ];
	    if ( gethostname( hostname, 255 ) == 0 ) {
		hostname[ 256 ] = '\0';
		if ( qstrncmp( uri+1, hostname, hostname_end - ( uri+1 )) == 0 ) {
		    uri = hostname_end + 1; // point after the slash
		    local = TRUE;
		}
	    }
	}
    }
#endif
    if ( local ) {
	file = uriToUnicodeUri(uri);
	if ( uri[1] == '/' ) {
	    file.remove((uint)0,1);
	} else {
		file.insert(0,'/');
	}
#ifdef Q_WS_WIN
	if ( file.length() > 2 && file[0] == '/' && file[2] == '|' ) {
	    file[2] = ':';
	    file.remove(0,1);
	} else if (file.length() > 2 && file[0] == '/' && file[1].isLetter() && file[2] == ':') {
	    file.remove(0, 1);
	}
	// Leave slash as slashes.
#endif
    }
#ifdef Q_WS_WIN
    else {
	file = uriToUnicodeUri(uri);
	// convert to network path
	file.insert(1, '/'); // leave as forward slashes
    }
#endif

    return file;
}

/*!
    Decodes URIs from the mime source event \a e, converts them to
    local files if they refer to local files, and places them in \a l
    (which is first cleared).

    Returns TRUE if \e contained a valid list of URIs; otherwise
    returns FALSE. The list will be empty if no URIs were local files.
*/
bool QUriDrag::decodeLocalFiles( const QMimeSource* e, QStringList& l )
{
    QStrList u;
    if ( !decode( e, u ) )
	return FALSE;

    l.clear();
    for (const char* s=u.first(); s; s=u.next()) {
	QString lf = uriToLocalFile(s);
	if ( !lf.isNull() )
	    l.append( lf );
    }
    return TRUE;
}

/*!
    Decodes URIs from the mime source event \a e, converts them to
    Unicode URIs (only useful for displaying to humans), placing them
    in \a l (which is first cleared).

    Returns TRUE if \e contained a valid list of URIs; otherwise
    returns FALSE.
*/
bool QUriDrag::decodeToUnicodeUris( const QMimeSource* e, QStringList& l )
{
    QStrList u;
    if ( !decode( e, u ) )
	return FALSE;

    l.clear();
    for (const char* s=u.first(); s; s=u.next())
	l.append( uriToUnicodeUri(s) );

    return TRUE;
}


#ifndef QT_NO_DRAGANDDROP
/*!
    If the source of the drag operation is a widget in this
    application, this function returns that source, otherwise it
    returns 0. The source of the operation is the first parameter to
    drag object subclasses.

    This is useful if your widget needs special behavior when dragging
    to itself, etc.

    See QDragObject::QDragObject() and subclasses.
*/
QWidget* QDropEvent::source() const
{
    return qt_dnd_manager ? qt_dnd_manager->dragSource : 0;
}
#endif

/*!
    \class QColorDrag qdragobject.h

    \brief The QColorDrag class provides a drag and drop object for
    transferring colors.

    \ingroup draganddrop

    This class provides a drag object which can be used to transfer data
    about colors for drag and drop and in the clipboard. For example, it
    is used in QColorDialog.

    The color is set in the constructor but can be changed with
    setColor().

    For more information about drag and drop, see the QDragObject class
    and the \link dnd.html drag and drop documentation\endlink.
*/

/*!
    Constructs a color drag object with the color \a col. Passes \a
    dragsource and \a name to the QStoredDrag constructor.
*/

QColorDrag::QColorDrag( const QColor &col, QWidget *dragsource, const char *name )
    : QStoredDrag( "application/x-color", dragsource, name )
{
    setColor( col );
}

/*!
    Constructs a color drag object with a white color. Passes \a
    dragsource and \a name to the QStoredDrag constructor.
*/

QColorDrag::QColorDrag( QWidget *dragsource, const char *name )
    : QStoredDrag( "application/x-color", dragsource, name )
{
    setColor( Qt::white );
}

/*!
    Sets the color of the color drag to \a col.
*/

void QColorDrag::setColor( const QColor &col )
{
    short r = (col.red()   << 8) | col.red();
    short g = (col.green() << 8) | col.green();
    short b = (col.blue()  << 8) | col.blue();

    // make sure we transmit data in network order
    r = htons(r);
    g = htons(g);
    b = htons(b);

    ushort rgba[4] = {
	r, g, b,
	0xffff // Alpha not supported yet.
    };
    QByteArray data(sizeof(rgba));
    memcpy(data.data(), rgba, sizeof(rgba));
    setEncodedData(data);
}

/*!
    Returns TRUE if the color drag object can decode the mime source
    \a e; otherwise returns FALSE.
*/

bool QColorDrag::canDecode( QMimeSource *e )
{
    return e->provides( "application/x-color" );
}

/*!
    Decodes the mime source \a e and sets the decoded values to \a
    col.
*/

bool QColorDrag::decode( QMimeSource *e, QColor &col )
{
    QByteArray data = e->encodedData("application/x-color");
    ushort rgba[4];
    if (data.size() != sizeof(rgba))
	return FALSE;

    memcpy(rgba, data.data(), sizeof(rgba));

    short r = rgba[0];
    short g = rgba[1];
    short b = rgba[2];

    // data is in network order
    r = ntohs(r);
    g = ntohs(g);
    b = ntohs(b);

    r = (r >> 8) & 0xff;
    g = (g >> 8) & 0xff;
    b = (b >> 8) & 0xff;

    col.setRgb(r, g, b);
    return TRUE;
}

#endif // QT_NO_MIME
