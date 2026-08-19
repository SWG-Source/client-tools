/****************************************************************************
** $Id: qinputcontext_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QInputContext class
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#include "qapplication.h"
#include "qwidget.h"
#include "qinputcontext_p.h"

#include <stdlib.h>
#include <limits.h>


bool qt_compose_emptied = FALSE;

#if !defined(QT_NO_XIM)

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

// #define QT_XIM_DEBUG

// from qapplication_x11.cpp
extern XIM	qt_xim;
extern XIMStyle	qt_xim_style;

/* The cache here is needed, as X11 leaks a few kb for every
   XFreeFontSet call, so we avoid creating and deletion of fontsets as
   much as possible
*/
static XFontSet fontsetCache[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static int fontsetRefCount = 0;

static const char * const fontsetnames[] = {
    "-*-fixed-medium-r-*-*-16-*,-*-*-medium-r-*-*-16-*",
    "-*-fixed-medium-i-*-*-16-*,-*-*-medium-i-*-*-16-*",
    "-*-fixed-bold-r-*-*-16-*,-*-*-bold-r-*-*-16-*",
    "-*-fixed-bold-i-*-*-16-*,-*-*-bold-i-*-*-16-*",
    "-*-fixed-medium-r-*-*-24-*,-*-*-medium-r-*-*-24-*",
    "-*-fixed-medium-i-*-*-24-*,-*-*-medium-i-*-*-24-*",
    "-*-fixed-bold-r-*-*-24-*,-*-*-bold-r-*-*-24-*",
    "-*-fixed-bold-i-*-*-24-*,-*-*-bold-i-*-*-24-*"
};

static XFontSet getFontSet( const QFont &f )
{
    int i = 0;
    if (f.italic())
	i |= 1;
    if (f.bold())
	i |= 2;

    if ( f.pointSize() > 20 )
	i += 4;

    if ( !fontsetCache[i] ) {
	Display* dpy = QPaintDevice::x11AppDisplay();
	int missCount;
	char** missList;
	fontsetCache[i] = XCreateFontSet(dpy, fontsetnames[i], &missList, &missCount, 0);
	if(missCount > 0)
	    XFreeStringList(missList);
	if ( !fontsetCache[i] ) {
	    fontsetCache[i] = XCreateFontSet(dpy,  "-*-fixed-*-*-*-*-16-*", &missList, &missCount, 0);
	    if(missCount > 0)
		XFreeStringList(missList);
	    if ( !fontsetCache[i] )
		fontsetCache[i] = (XFontSet)-1;
	}
    }
    return (fontsetCache[i] == (XFontSet)-1) ? 0 : fontsetCache[i];
}


#ifdef Q_C_CALLBACKS
extern "C" {
#endif // Q_C_CALLBACKS

    static int xic_start_callback(XIC, XPointer client_data, XPointer) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic) {
#ifdef QT_XIM_DEBUG
	    qDebug("compose start: no qic");
#endif // QT_XIM_DEBUG

	    return 0;
	}

	qic->composing = TRUE;
	qic->text = QString::null;
	qic->focusWidget = 0;

	if ( qic->selectedChars.size() < 128 )
	    qic->selectedChars.resize( 128 );
	qic->selectedChars.fill( 0 );

#ifdef QT_XIM_DEBUG
	qDebug("compose start");
#endif // QT_XIM_DEBUG

	return 0;
    }

    static int xic_draw_callback(XIC, XPointer client_data, XPointer call_data) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic) {
#ifdef QT_XIM_DEBUG
	    qDebug("compose event: invalid compose event %p", qic);
#endif // QT_XIM_DEBUG

	    return 0;
	}

	bool send_imstart = FALSE;
	if (qApp->focusWidget() != qic->focusWidget && qic->text.isEmpty()) {
	    if (qic->focusWidget) {
#ifdef QT_XIM_DEBUG
		qDebug( "sending IMEnd (empty) to %p", qic->focusWidget );
#endif // QT_XIM_DEBUG

		QIMEvent endevent(QEvent::IMEnd, QString::null, -1);
		QApplication::sendEvent(qic->focusWidget, &endevent);
	    }

	    qic->text = QString::null;
	    qic->focusWidget = qApp->focusWidget();
	    qic->composing = FALSE;

	    if ( qic->selectedChars.size() < 128 )
		qic->selectedChars.resize( 128 );
	    qic->selectedChars.fill( 0 );

	    if (qic->focusWidget) {
		qic->composing = TRUE;
		send_imstart = TRUE;
	    }
	}

	if (! qic->composing || ! qic->focusWidget) {
#ifdef QT_XIM_DEBUG
	    qDebug("compose event: invalid compose event %d %p",
		   qic->composing, qic->focusWidget);
#endif // QT_XIM_DEBUG

	    return 0;
	}

	if ( send_imstart ) {
#ifdef QT_XIM_DEBUG
            qDebug( "sending IMStart to %p", qic->focusWidget );
#endif // QT_XIM_DEBUG

	    qt_compose_emptied = FALSE;
	    QIMEvent startevent(QEvent::IMStart, QString::null, -1);
	    QApplication::sendEvent(qic->focusWidget, &startevent);
	}

	XIMPreeditDrawCallbackStruct *drawstruct =
	    (XIMPreeditDrawCallbackStruct *) call_data;
	XIMText *text = (XIMText *) drawstruct->text;
	int cursor = drawstruct->caret, sellen = 0;

	if ( ! drawstruct->caret && ! drawstruct->chg_first &&
	     ! drawstruct->chg_length && ! text ) {
	    // nothing to do
	    return 0;
	}

	if (text) {
	    char *str = 0;
	    if (text->encoding_is_wchar) {
		int l = wcstombs(NULL, text->string.wide_char, text->length);
		if (l != -1) {
		    str = new char[l + 1];
		    wcstombs(str, text->string.wide_char, l);
		    str[l] = 0;
		}
	    } else
		str = text->string.multi_byte;

	    if (! str)
		return 0;

	    QString s = QString::fromLocal8Bit(str);

	    if (text->encoding_is_wchar)
		delete [] str;

	    if (drawstruct->chg_length < 0)
		qic->text.replace(drawstruct->chg_first, UINT_MAX, s);
	    else
		qic->text.replace(drawstruct->chg_first, drawstruct->chg_length, s);

	    if ( qic->selectedChars.size() < qic->text.length() ) {
		// expand the selectedChars array if the compose string is longer
		uint from = qic->selectedChars.size();
		qic->selectedChars.resize( qic->text.length() );
		for ( uint x = from; from < qic->selectedChars.size(); ++x )
		    qic->selectedChars[x] = 0;
	    }

	    uint x;
	    bool *p = qic->selectedChars.data() + drawstruct->chg_first;
	    // determine if the changed chars are selected based on text->feedback
	    for ( x = 0; x < s.length(); ++x )
		*p++ = ( text->feedback ? ( text->feedback[x] & XIMReverse ) : 0 );

	    // figure out where the selection starts, and how long it is
	    p = qic->selectedChars.data();
	    bool started = FALSE;
	    for ( x = 0; x < QMIN(qic->text.length(), qic->selectedChars.size()); ++x ) {
		if ( started ) {
		    if ( *p ) ++sellen;
		    else break;
		} else {
		    if ( *p ) {
			cursor = x;
			started = TRUE;
			sellen = 1;
		    }
		}
		++p;
	    }
	} else {
	    if (drawstruct->chg_length == 0)
		drawstruct->chg_length = -1;

	    qic->text.remove(drawstruct->chg_first, drawstruct->chg_length);
	    qt_compose_emptied = qic->text.isEmpty();
	    if ( qt_compose_emptied ) {
#ifdef QT_XIM_DEBUG
		qDebug( "compose emptied" );
#endif // QT_XIM_DEBUG

		// don't send an empty compose, since we will send an IMEnd with
		// either the correct compose text (or null text if the user has
		// cancelled the compose or deleted all chars).
		return 0;
	    }
	}

#ifdef QT_XIM_DEBUG
	qDebug( "sending IMCompose to %p with %d chars",
                qic->focusWidget, qic->text.length() );
#endif // QT_XIM_DEBUG

	QIMComposeEvent event( QEvent::IMCompose, qic->text, cursor, sellen );
	QApplication::sendEvent(qic->focusWidget, &event);
	return 0;
    }

    static int xic_done_callback(XIC, XPointer client_data, XPointer) {
	QInputContext *qic = (QInputContext *) client_data;
	if (! qic)
	    return 0;

	if (qic->composing && qic->focusWidget) {
#ifdef QT_XIM_DEBUG
	    qDebug( "sending IMEnd (empty) to %p", qic->focusWidget );
#endif // QT_XIM_DEBUG

       	    QIMEvent event(QEvent::IMEnd, QString::null, -1);
	    QApplication::sendEvent(qic->focusWidget, &event);
	}

	qic->composing = FALSE;
	qic->focusWidget = 0;

	if ( qic->selectedChars.size() < 128 )
	    qic->selectedChars.resize( 128 );
	qic->selectedChars.fill( 0 );

	return 0;
    }

#ifdef Q_C_CALLBACKS
}
#endif // Q_C_CALLBACKS

#endif // !QT_NO_XIM



QInputContext::QInputContext(QWidget *widget)
    : ic(0), focusWidget(0), composing(FALSE), fontset(0)
{
#if !defined(QT_NO_XIM)
    fontsetRefCount++;
    if (! qt_xim) {
	qWarning("QInputContext: no input method context available");
	return;
    }

    if (! widget->isTopLevel()) {
	qWarning("QInputContext: cannot create input context for non-toplevel widgets");
	return;
    }

    XPoint spot;
    XRectangle rect;
    XVaNestedList preedit_attr = 0;
    XIMCallback startcallback, drawcallback, donecallback;

    font = widget->font();
    fontset = getFontSet( font );

    if (qt_xim_style & XIMPreeditArea) {
	rect.x = 0;
	rect.y = 0;
	rect.width = widget->width();
	rect.height = widget->height();

	preedit_attr = XVaCreateNestedList(0,
					   XNArea, &rect,
					   XNFontSet, fontset,
					   (char *) 0);
    } else if (qt_xim_style & XIMPreeditPosition) {
	spot.x = 1;
	spot.y = 1;

	preedit_attr = XVaCreateNestedList(0,
					   XNSpotLocation, &spot,
					   XNFontSet, fontset,
					   (char *) 0);
    } else if (qt_xim_style & XIMPreeditCallbacks) {
	startcallback.client_data = (XPointer) this;
	startcallback.callback = (XIMProc) xic_start_callback;
	drawcallback.client_data = (XPointer) this;
	drawcallback.callback = (XIMProc)xic_draw_callback;
	donecallback.client_data = (XPointer) this;
	donecallback.callback = (XIMProc) xic_done_callback;

	preedit_attr = XVaCreateNestedList(0,
					   XNPreeditStartCallback, &startcallback,
					   XNPreeditDrawCallback, &drawcallback,
					   XNPreeditDoneCallback, &donecallback,
					   (char *) 0);
    }

    if (preedit_attr) {
	ic = XCreateIC(qt_xim,
		       XNInputStyle, qt_xim_style,
		       XNClientWindow, widget->winId(),
		       XNPreeditAttributes, preedit_attr,
		       (char *) 0);
	XFree(preedit_attr);
    } else
	ic = XCreateIC(qt_xim,
		       XNInputStyle, qt_xim_style,
		       XNClientWindow, widget->winId(),
		       (char *) 0);

    if (! ic)
	qFatal("Failed to create XIM input context!");

    // when resetting the input context, preserve the input state
    (void) XSetICValues((XIC) ic, XNResetState, XIMPreserveState, (char *) 0);
#endif // !QT_NO_XIM
}


QInputContext::~QInputContext()
{

#if !defined(QT_NO_XIM)
    if (ic)
	XDestroyIC((XIC) ic);

    if ( --fontsetRefCount == 0 ) {
	Display *dpy = QPaintDevice::x11AppDisplay();
	for ( int i = 0; i < 8; i++ ) {
	    if ( fontsetCache[i] && fontsetCache[i] != (XFontSet)-1 ) {
		XFreeFontSet(dpy, fontsetCache[i]);
		fontsetCache[i] = 0;
	    }
	}
    }

#endif // !QT_NO_XIM

    ic = 0;
    focusWidget = 0;
    composing = FALSE;
}


void QInputContext::reset()
{
#if !defined(QT_NO_XIM)
    if (focusWidget && composing && ! text.isNull()) {
#ifdef QT_XIM_DEBUG
	qDebug("QInputContext::reset: composing - sending IMEnd (empty) to %p",
	       focusWidget);
#endif // QT_XIM_DEBUG

	QIMEvent endevent(QEvent::IMEnd, QString::null, -1);
	QApplication::sendEvent(focusWidget, &endevent);
	focusWidget = 0;
	text = QString::null;
	if ( selectedChars.size() < 128 )
	    selectedChars.resize( 128 );
	selectedChars.fill( 0 );

	char *mb = XmbResetIC((XIC) ic);
	if (mb)
	    XFree(mb);
    }
#endif // !QT_NO_XIM
}


void QInputContext::setComposePosition(int x, int y)
{
#if !defined(QT_NO_XIM)
    if (qt_xim && ic) {
	XPoint point;
	point.x = x;
	point.y = y;

	XVaNestedList preedit_attr =
	    XVaCreateNestedList(0,
				XNSpotLocation, &point,

				(char *) 0);
	XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, (char *) 0);
	XFree(preedit_attr);
    }
#endif // !QT_NO_XIM
}


void QInputContext::setComposeArea(int x, int y, int w, int h)
{
#if !defined(QT_NO_XIM)
    if (qt_xim && ic) {
	XRectangle rect;
	rect.x = x;
	rect.y = y;
	rect.width = w;
	rect.height = h;

	XVaNestedList preedit_attr = XVaCreateNestedList(0,
							 XNArea, &rect,

							 (char *) 0);
	XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, (char *) 0);
	XFree(preedit_attr);
    }
#endif
}


int QInputContext::lookupString(XKeyEvent *event, QCString &chars,
				KeySym *key, Status *status) const
{
    int count = 0;

#if !defined(QT_NO_XIM)
    if (qt_xim && ic) {
	count = XmbLookupString((XIC) ic, event, chars.data(),
				chars.size(), key, status);

	if ((*status) == XBufferOverflow ) {
	    chars.resize(count + 1);
	    count = XmbLookupString((XIC) ic, event, chars.data(),
				    chars.size(), key, status);
	}
    }

#endif // QT_NO_XIM

    return count;
}

void QInputContext::setFocus()
{
#if !defined(QT_NO_XIM)
    if (qt_xim && ic)
	XSetICFocus((XIC) ic);
#endif // !QT_NO_XIM
}

void QInputContext::setXFontSet(const QFont &f)
{
#if !defined(QT_NO_XIM)
    if (font == f) return; // nothing to do
    font = f;

    XFontSet fs = getFontSet(font);
    if (fontset == fs) return; // nothing to do
    fontset = fs;

    XVaNestedList preedit_attr = XVaCreateNestedList(0, XNFontSet, fontset, (char *) 0);
    XSetICValues((XIC) ic, XNPreeditAttributes, preedit_attr, (char *) 0);
    XFree(preedit_attr);
#else
    Q_UNUSED( f );
#endif
}
