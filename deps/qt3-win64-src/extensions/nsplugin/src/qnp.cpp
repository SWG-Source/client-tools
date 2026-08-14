/****************************************************************************
** $Id: qnp.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of Qt extension classes for Netscape Plugin support.
**
** Created : 970601
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
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


// Remaining Q_WS_X11 considerations:
//   - What if !piApp upon NPP_NewStream?  Are we safe?
//      - Yes, but users need to know of this:  that no GUI can be
//         done until after setWindow is called.
//   - Use NPN_GetValue in Communicator4.0 to get the display earlier!
//   - For ClientMessage events, trap them, and if they are not for us,
//	untrap them and retransmit them and set a timer to retrap them
//	after N seconds.

// Remaining Q_WS_WIN considerations:
//   - we need to activateZeroTimers() at some time.
//   - we need to call winEventFilter on events
//   - timers:
//    if ( msg.message == WM_TIMER ) {            // timer message received
//        activateTimer( msg.wParam );
//        return TRUE;
//    }
//    if ( msg.message == WM_KEYDOWN || msg.message == WM_KEYUP ) {
//        if ( translateKeyCode(msg.wParam) == 0 ) {
//            TranslateMessage( &msg );           // translate to WM_CHAR
//            return TRUE;
//        }
//    }
//   - qWinProcessConfigRequests?

// Remaining general stuff:
//   - Provide the "reason" parameter to streamDestroyed

// Qt stuff
#include <qapplication.h>
#include <qeventloop.h>
#include <qwidget.h>
#include <qobjectlist.h>
#include <qcursor.h>
#include <qprinter.h>
#include <qfile.h>
#include <qpainter.h>

#include "qnp.h"

#include <stdlib.h>		// Must be here for Borland C++
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#ifdef Q_WS_X11
#include <X11/Intrinsic.h>

class QNPXtPrivate;

class QNPXt : public QEventLoop
{
public:
    QNPXt( const char *applicationClass, XtAppContext context = NULL, XrmOptionDescRec *options = 0, int numOptions = 0);
    ~QNPXt();

    XtAppContext applicationContext() const;

    void registerSocketNotifier( QSocketNotifier * );
    void unregisterSocketNotifier( QSocketNotifier * );

    static void registerWidget( QWidget* );
    static void unregisterWidget( QWidget* );
    static bool redeliverEvent( XEvent *event );
    static XEvent* lastEvent();

protected:
    bool processEvents( ProcessEventsFlags flags );

private:
    void appStartingUp();
    void appClosingDown();
    QNPXtPrivate *d;

};

#define	 GC GC_QQQ
#endif

extern "C" {
//
// Netscape plugin API
//
#ifdef Q_WS_WIN
#ifndef _WINDOWS
#define _WINDOWS
#endif
#endif
#ifdef Q_WS_X11
#define XP_UNIX
#endif


// This is to allow mingw support on windows without altering the sun header files
#if defined(Q_CC_GNU) && defined(Q_WS_WIN) && !defined(_MSC_VER)
#define _MSC_VER 1
#include "npapi.h"
#undef _MSC_VER
#else
#include "npapi.h"
#endif

#ifdef Q_WS_X11
#undef XP_UNIX
#include "npunix.c"
#endif

//
// Stuff for the NPP_SetWindow function:
//
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h> // for XtCreateWindow
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
//#include <dlfcn.h>
#endif
#ifdef Q_WS_WIN
#include <windows.h>
#endif
}

#ifdef Q_WS_WIN
#include "npwin.cpp"
#endif

static QEventLoop* event_loop = 0;
static QApplication* application = 0;

struct _NPInstance
{
    uint16            fMode;

#ifdef Q_WS_WIN
    HWND            window;
#endif

    NPP npp;

#ifdef Q_WS_X11
    Window window;
    Display *display;
#endif

    uint32 x, y;
    uint32 width, height;

    QNPWidget* widget;
    QNPInstance* instance;

    int16 argc;
    QString *argn;
    QString *argv;
};



// The single global plugin
static QNPlugin *qNP=0;
static int instance_count=0;

// Temporary parameter passed `around the side' of calls to user functions
static _NPInstance* next_pi=0;

// To avoid looping when browser OR plugin can delete streams
static int qnps_no_call_back = 0;

#ifdef Q_WS_WIN
// defined in qapplication_win.cpp
Q_EXPORT extern bool qt_win_use_simple_timers;
Q_EXPORT void qWinProcessConfigRequests();
static HHOOK hhook = 0;

LRESULT CALLBACK FilterProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if ( qApp ) {
	qApp->sendPostedEvents();
	qApp->eventLoop()->activateSocketNotifiers();
	qWinProcessConfigRequests();
    }

    return CallNextHookEx( hhook, nCode, wParam, lParam );
}

#endif

#ifdef Q_WS_X11
static int (*original_x_errhandler)( Display *dpy, XErrorEvent * ) = 0;
static int dummy_x_errhandler( Display *, XErrorEvent * )
{
    return 0;
}
#endif

/******************************************************************************
 * Plug-in Calls - these are called by Netscape
 *****************************************************************************/


// Instance state information about the plugin.

#ifdef Q_WS_X11

extern "C" char*
NPP_GetMIMEDescription(void)
{
    if (!qNP) qNP = QNPlugin::create();
    return (char*)qNP->getMIMEDescription();
}



extern "C" NPError
NPP_GetValue(void * /*future*/, NPPVariable variable, void *value)
{
    if (!qNP) qNP = QNPlugin::create();
    NPError err = NPERR_NO_ERROR;
    if (variable == NPPVpluginNameString)
	*((const char **)value) = qNP->getPluginNameString();
    else if (variable == NPPVpluginDescriptionString)
	*((const char **)value) = qNP->getPluginDescriptionString();
    else
	err = NPERR_GENERIC_ERROR;

    return err;
}

#endif

/*
** NPP_Initialize is called when your DLL is being loaded to do any
** DLL-specific initialization.
*/
extern "C" NPError
NPP_Initialize(void)
{
#ifdef Q_WS_WIN
    qt_win_use_simple_timers = TRUE;
    // Nothing more - we do it in DLLMain
#endif

    if (!qNP) qNP = QNPlugin::create();
    return NPERR_NO_ERROR;
}

static jref plugin_java_class = 0;

/*
** NPP_GetJavaClass is called during initialization to ask your plugin
** what its associated Java class is. If you don't have one, just return
** NULL. Otherwise, use the javah-generated "use_" function to both
** initialize your class and return it. If you can't find your class, an
** error will be signalled by "use_" and will cause the Navigator to
** complain to the user.
*/
extern "C" jref
NPP_GetJavaClass(void)
{
    if (!qNP) qNP = QNPlugin::create();
    plugin_java_class = (jref)qNP->getJavaClass();
    return plugin_java_class;
}

/*
** NPP_Shutdown is called when your DLL is being unloaded to do any
** DLL-specific shut-down. You should be a good citizen and declare that
** you're not using your java class any more. This allows java to unload
** it, freeing up memory.
*/
extern "C" void
NPP_Shutdown(void)
{
    if (qNP) {
	if (plugin_java_class)
	    qNP->unuseJavaClass();
	delete qNP;
	qNP = 0;
    }

#ifdef Q_WS_X11
    if ( original_x_errhandler )
    	XSetErrorHandler( original_x_errhandler );
#endif
    if ( qApp) {
#ifdef Q_WS_WIN32
	if ( hhook )
	    UnhookWindowsHookEx( hhook );
	hhook = 0;
#endif
	delete application;
	delete event_loop;
    }

}


struct NS_Private {
    uchar* a;
    uchar* b;
};

/*
** NPP_New is called when your plugin is instantiated (i.e. when an EMBED
** tag appears on a page).
*/
extern "C" NPError
NPP_New(NPMIMEType /*pluginType*/,
    NPP instance,
    uint16 mode,
    int16 argc,
    char* argn[],
    char* argv[],
    NPSavedData* /*saved*/)
{
    NPError result = NPERR_NO_ERROR;
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    instance->pdata = new _NPInstance;

    This = (_NPInstance*) instance->pdata;

    if (This == NULL)
	return NPERR_OUT_OF_MEMORY_ERROR;

    This->npp = instance;

    /* mode is NP_EMBED, NP_FULL, or NP_BACKGROUND (see npapi.h) */
    This->fMode = mode;

    This->window = 0;
    This->widget = 0;

    This->argc = argc;
    This->argn = new QString[argc+1];
    This->argv = new QString[argc+1];
    for (int i=0; i<This->argc; i++) {
	This->argn[i] = argn[i];
	This->argv[i] = argv[i];
    }

    // Everything is set up - we can let QNPInstance be created now.
    next_pi = This;
    qNP->newInstance();
    instance_count++;

    return result;
}

extern "C" NPError
NPP_Destroy(NPP instance, NPSavedData** /*save*/)
{
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    This = (_NPInstance*) instance->pdata;

    if (This != NULL) {
	delete This->widget;
	delete This->instance;
	delete [] This->argn;
	delete [] This->argv;

	delete This;
	instance->pdata = NULL;

	instance_count--;
    }

    return NPERR_NO_ERROR;
}

extern "C" NPError
NPP_SetWindow(NPP instance, NPWindow* window)
{
    if (!qNP) qNP = QNPlugin::create();
    NPError result = NPERR_NO_ERROR;
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    This = (_NPInstance*) instance->pdata;


    // take a shortcut if all that was changed is the geometry
    if ( This->widget && window
#ifdef Q_WS_X11
	 && This->window == (Window) window->window
#endif
#ifdef Q_WS_WIN
	 && This->window == (HWND) window->window
#endif
	) {
	This->x = window->x;
	This->y = window->y;
	This->width = window->width;
	This->height = window->height;
	This->widget->resize( This->width, This->height );
	return result;
    }

    delete This->widget;

    if ( !window )
	return result;

#ifdef Q_WS_X11
    This->window = (Window) window->window;
    This->display =
	((NPSetWindowCallbackStruct *)window->ws_info)->display;
#endif
#ifdef Q_WS_WIN
    This->window = (HWND) window->window;
#endif

    This->x = window->x;
    This->y = window->y;
    This->width = window->width;
    This->height = window->height;


    if (!qApp) {
#ifdef Q_WS_X11
	// We are the first Qt-based plugin to arrive
	event_loop = new QNPXt( "qnp", XtDisplayToApplicationContext(This->display) );
	application = new QApplication(This->display);
#endif
#ifdef Q_WS_WIN
	static int argc=0;
	static char **argv={ 0 };
	application = new QApplication( argc, argv );
#ifdef UNICODE
	if ( qWinVersion() & Qt::WV_NT_based )
	    hhook = SetWindowsHookExW( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
	else
#endif
	    hhook = SetWindowsHookExA( WH_GETMESSAGE, FilterProc, 0, GetCurrentThreadId() );
#endif
    }

#ifdef Q_WS_X11
    if ( !original_x_errhandler )
    	original_x_errhandler = XSetErrorHandler( dummy_x_errhandler );
#endif

    // New widget on this new window.
    next_pi = This;
    /* This->widget = */ // (happens sooner - in QNPWidget constructor)
    This->instance->newWindow();

    if ( !This->widget )
	return result;

#ifdef Q_WS_X11
    This->widget->resize( This->width, This->height );
    XReparentWindow( This->widget->x11Display(), This->widget->winId(), This->window, 0, 0 );
    XSync( This->widget->x11Display(), False );
#endif
#ifdef Q_WS_WIN
    LONG oldLong = GetWindowLong(This->window, GWL_STYLE);
    ::SetWindowLong(This->window, GWL_STYLE, oldLong | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    ::SetWindowLong( This->widget->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS );
    ::SetParent( This->widget->winId(), This->window );
    This->widget->raise();
    This->widget->setGeometry( 0, 0, This->width, This->height );
#endif
    This->widget->show();
    return result;
}


extern "C" NPError
NPP_NewStream(NPP instance,
	  NPMIMEType type,
	  NPStream *stream,
	  NPBool seekable,
	  uint16 *stype)
{
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    This = (_NPInstance*) instance->pdata;

    if ( This ) {
	QNPStream* qnps = new QNPStream(This->instance,type,stream,seekable);
	stream->pdata = qnps;
	QNPInstance::StreamMode sm = (QNPInstance::StreamMode)*stype;
	if (!This->instance->newStreamCreated(qnps, sm)) {
	    return NPERR_GENERIC_ERROR;
	}
	*stype = sm;
    }

    return NPERR_NO_ERROR;
}


int32 STREAMBUFSIZE = 0X0FFFFFFF; /* If we are reading from a file in NPAsFile
				   * mode so we can take any size stream in our
				   * write call (since we ignore it) */

extern "C" int32
NPP_WriteReady(NPP instance, NPStream *stream)
{
    _NPInstance* This;
    if (instance != NULL) {
	This = (_NPInstance*) instance->pdata;
    } else {
	// Yikes, that's unusual!
	return 0;
    }

    if (This) {
	return This->instance->writeReady((QNPStream*)stream->pdata);
    }

    /* Number of bytes ready to accept in NPP_Write() */
    return STREAMBUFSIZE;
}


extern "C" int32
NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
    if (instance != NULL)
    {
	_NPInstance* This = (_NPInstance*) instance->pdata;

	if (This) {
	    return This->instance->write((QNPStream*)stream->pdata,
		offset, len, buffer);
	}
    }

    return len;        /* The number of bytes accepted */
}


extern "C" NPError
NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
    _NPInstance* This;

    if (instance == NULL)
	return NPERR_INVALID_INSTANCE_ERROR;

    if (!qnps_no_call_back) {
	This = (_NPInstance*) instance->pdata;

	QNPStream* qnps = (QNPStream*)stream->pdata;
	if ( qnps )
	    switch (reason) {
		case NPRES_DONE:
		    qnps->setComplete(TRUE);
		    break;
		case NPRES_USER_BREAK:
		    break;
		case NPRES_NETWORK_ERR:
		    qnps->setOkay(FALSE);
		    break;
	    }

	if (This) {
	    // Give the instance a chance to do something
	    This->instance->streamDestroyed(qnps);
	}

	qnps_no_call_back++;
	delete qnps;
	qnps_no_call_back--;
    }

    return NPERR_NO_ERROR;
}


extern "C" void
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
    _NPInstance* This;

    if (instance == NULL) return;

    This = (_NPInstance*) instance->pdata;

    if ( This ) {
	QNPStream* qnps = (QNPStream*)stream->pdata;
	This->instance->streamAsFile(qnps, fname);
    }
}

typedef struct
{
    int32    type;
    FILE*    fp;
} NPPrintCallbackStruct;

#ifdef Q_WS_X11

class QNPPrinter : public QPrinter {
    QFile file;
public:
    QNPPrinter(FILE* fp)
    {
	file.open(IO_WriteOnly, fp);
	QPDevCmdParam param;
	param.device = &file;
	cmd(PdcSetdev, 0, &param);
    }
    void end()
    {
	QPDevCmdParam param;
	param.device = 0;
	cmd(PdcSetdev, 0, &param);
    }
};
#endif

extern "C" void
NPP_Print(NPP instance, NPPrint* printInfo)
{
    if(printInfo == NULL)
	return;

    if (instance != NULL) {
	_NPInstance* This = (_NPInstance*) instance->pdata;

	if (printInfo->mode == NP_FULL) {
	    printInfo->print.fullPrint.pluginPrinted =
		This->instance->printFullPage();
	} else if (printInfo->mode == NP_EMBED) {
#ifdef Q_WS_X11
	    void* platformPrint =
		printInfo->print.embedPrint.platformPrint;
	    FILE* outfile = ((NPPrintCallbackStruct*)platformPrint)->fp;
	    if (ftell(outfile)) {
// 		NPWindow* w =
// 		    &(printInfo->print.embedPrint.window);
		QNPPrinter prn(outfile);
		QPainter painter(&prn);
		// #### config viewport with w->{x,y,width,height}
		This->instance->print(&painter);
		prn.end();
	    } else {
		// Why does the browser make spurious NPP_Print calls?
	    }
#endif
#ifdef Q_WS_WIN
	    NPWindow* printWindow =
		&(printInfo->print.embedPrint.window);
	    void* platformPrint =
		printInfo->print.embedPrint.platformPrint;
	    // #### Nothing yet.
#endif
	}
    }
}

extern "C" void
NPP_URLNotify(NPP instance,
	      const char* url,
	      NPReason reason,
	      void* notifyData)
{
    if (instance != NULL) {
	QNPInstance::Reason r;
	switch (reason) {
	case NPRES_DONE:
	    r = QNPInstance::ReasonDone;
	    break;
	case NPRES_USER_BREAK:
	    r = QNPInstance::ReasonBreak;
	    break;
	case NPRES_NETWORK_ERR:
	    r = QNPInstance::ReasonError;
	    break;
	default:
	    r = QNPInstance::ReasonUnknown;
	    break;
	}
	_NPInstance* This = (_NPInstance*) instance->pdata;
	This->instance->notifyURL(url, r, notifyData);
    }
}



#ifdef Q_WS_WIN

BOOL   WINAPI   DllMain (HANDLE hInst,
			ULONG ul_reason_for_call,
			LPVOID lpReserved)
{
    return TRUE;
}

#endif



/*!
    \class QNPWidget qnp.h
    \brief The QNPWidget class provides a QWidget that is a web browser plugin window.

    \extension Netscape Plugin

    Derive from QNPWidget to create a widget that can be used as a
    web browser plugin window, or create one and add child widgets.
    Instances of QNPWidget may only be created when
    QNPInstance::newWindow() is called by the browser.

    A common way to develop a plugin widget is to develop it as a
    stand-alone application window, then make it a \e child of a
    plugin widget to use it as a browser plugin. The technique is:

\code
class MyPluginWindow : public QNPWidget
{
    QWidget* child;
public:
    MyPluginWindow()
    {
	// Some widget that is normally used as a top-level widget
	child = new MyIndependentlyDevelopedWidget();

	// Use the background color of the web page
	child->setBackgroundColor( backgroundColor() );

	// Fill the plugin widget
	child->setGeometry( 0, 0, width(), height() );
    }

    void resizeEvent(QResizeEvent*)
    {
	// Fill the plugin widget
	child->resize(size());
    }
};
\endcode

    The default implementation is an empty window.
*/

/*!
    Creates a QNPWidget.
*/
QNPWidget::QNPWidget() :
     pi(next_pi)
{
    if (!next_pi) {
	qFatal("QNPWidget must only be created within call to newWindow");
    }
    next_pi->widget = this;
    next_pi = 0;

#ifdef Q_WS_WIN
    clearWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu );
    topData()->ftop = 0;
    topData()->fright = 0;
    topData()->fleft = 0;
    topData()->fbottom = 0;
#endif
}

/*!
    Destroys the window. This will be called by the plugin binding
    code when the window is no longer required. The web browser will
    delete windows when they leave the page. The bindings will change
    the QWidget::winId() of the window when the window is resized, but
    this should not affect normal widget behavior.
*/
QNPWidget::~QNPWidget()
{
#ifdef Q_WS_X11
    destroy( FALSE, FALSE ); // X has destroyed all windows
#endif
}


/*!\internal */
void QNPWidget::enterEvent(QEvent*)
{
    enterInstance();
}

/*!\internal */
void QNPWidget:: leaveEvent(QEvent*)
{
    if ( !QApplication::activePopupWidget() )
	leaveInstance();
}

/*!
    Called when the mouse enters the plugin window. Does nothing by
    default.
*/
void QNPWidget::enterInstance()
{
}

/*!
    Called when the mouse leaves the plugin window. Does nothing by
    default.
*/
void QNPWidget::leaveInstance()
{
}

/*!
    Returns the instance for which this widget is the plugin window.
*/
QNPInstance* QNPWidget::instance()
{
    return pi->instance;
}





/*!
    \class QNPInstance qnp.h
    \brief The QNPInstance class provides a QObject that is a web browser plugin.

    \extension Netscape Plugin

    Deriving from QNPInstance creates an object that represents a
    single \c{<EMBED>} tag in an HTML document.

    The QNPInstance is responsible for creating an appropriate
    QNPWidget window if required (not all plugins have windows), and
    for interacting with the input/output facilities intrinsic to
    plugins.

    Note that there is \e{absolutely no guarantee} regarding the order
    in which functions are called. Sometimes the browser will call
    newWindow() first, at other times, newStreamCreated() will be
    called first (assuming the \c{<EMBED>} tag has a SRC parameter).

    \e{None of Qt's GUI functionality} may be used until after the
    first call to newWindow(). This includes any use of QPaintDevice
    (i.e. QPixmap, QWidget, and all subclasses), QApplication, anything
    related to QPainter (QBrush, etc.), fonts, QMovie, QToolTip, etc.
    Useful classes which specifically \e can be used are QImage,
    QFile, and QBuffer.

    This restriction can easily be accommodated by structuring your
    plugin so that the task of the QNPInstance is to gather data,
    while the task of the QNPWidget is to provide a graphical
    interface to that data.
*/

/*!
    \enum QNPInstance::InstanceMode

    This enum type provides Qt-style names for three #defines in
    \c npapi.h:

    \value Embed - corresponds to NP_EMBED
    \value Full - corresponds to NP_FULL
    \value Background - corresponds to NP_BACKGROUND

*/

/*!
    \enum QNPInstance::Reason

    \value ReasonDone
    \value ReasonBreak
    \value ReasonError
    \value ReasonUnknown
*/

/*!
    \enum QNPInstance::StreamMode

    \value Normal
    \value Seek
    \value AsFile
    \value AsFileOnly
*/

/*!
    Creates a QNPInstance.

    Can only be called from within a derived class created within
    QNPlugin::newInstance().
*/
QNPInstance::QNPInstance() :
    pi(next_pi)
{
    if (!next_pi) {
	qFatal("QNPInstance must only be created within call to newInstance");
    }
    next_pi->instance = this;
    next_pi = 0;
}

/*!
    Called when the plugin instance is about to be deleted.
*/
QNPInstance::~QNPInstance()
{
}

/*!
    Called at most once, at some time after the QNPInstance is
    created. If the plugin requires a window, this function should
    return a derived class of QNPWidget that provides the required
    interface.
*/
QNPWidget* QNPInstance::newWindow()
{
    // No window by default
    next_pi = 0;
    return 0;
}

/*!
    Returns the plugin window created by newWindow(), if any.
*/
QNPWidget* QNPInstance::widget()
{
    return pi->widget;
}

/*!
    \fn bool QNPInstance::newStreamCreated(QNPStream*, StreamMode& smode)

    This function is called when a new stream has been created. The
    instance should return TRUE if it accepts the processing of the
    stream. If the instance requires the stream as a file, it should
    set \a smode to \c AsFileOnly, in which case the data will be
    delivered some time later to the streamAsFile() function.
    Otherwise, the data will be delivered in chunks to the write()
    function, which must consume at least as much data as returned
    by the most recent call to writeReady().

    Note that the \c AsFileOnly method is not supported by Netscape
    2.0 and MSIE 3.0.

    The default implementation accepts any stream.
*/
bool QNPInstance::newStreamCreated(QNPStream*, StreamMode&)
{
    return TRUE;
}

/*!
    Called when a stream is delivered as a single file called \a fname
    rather than as chunks. This may be simpler for a plugin to deal
    with, but precludes any incremental behavior.

    Note that the \c AsFileOnly method is not supported by Netscape
    2.0 and MSIE 3.0.

    \sa newStreamCreated(), newStream()
*/
void QNPInstance::streamAsFile(QNPStream*, const char* fname)
{
}

/*!
    Called when a stream is destroyed. At this point, the stream may
    be complete() and okay(). If it is not okay(), then an error has
    occurred. If it is okay(), but not complete(), then the user has
    cancelled the transmission; do not give an error message in this
    case.
*/
void QNPInstance::streamDestroyed(QNPStream*)
{
}

/*!
    Returns the minimum amount of data the instance is willing to
    receive from the given stream.

    The default returns a very large value.
*/
int QNPInstance::writeReady(QNPStream*)
{
    // Yes, we can handle any amount of data at once.
    return 0X0FFFFFFF;
}

/*!
    \fn int QNPInstance::write(QNPStream*, int offset, int len, void* buffer)

    Called when incoming data is available for processing by the
    instance. The instance \e must consume at least the amount that it
    returned in the most recent call to writeReady(), but it may
    consume up to the amount given by \a len. \a buffer is the data
    available for consumption. The \a offset argument is merely an
    informational value indicating the total amount of data that has
    been consumed in prior calls.

    This function should return the amount of data actually consumed.
*/
int QNPInstance::write(QNPStream*, int, int len, void*)
{
    // Yes, we processed it all... into the bit bucket.
    return len;
}

/*!
    Requests that the \a url be retrieved and sent to the named \a
    window. See Netscape's JavaScript documentation for an explanation
    of window names.
*/
void QNPInstance::getURL(const char* url, const char* window)
{
    NPN_GetURL( pi->npp, url, window );
}

/*!
    \preliminary

    This function is \e{not tested}.

    It is an interface to the NPN_PostURL function of the Netscape
    Plugin API.

    Passes \a url, \a window, \a buf, \a len, and \a file to
    NPN_PostURL.
*/
void QNPInstance::postURL(const char* url, const char* window,
	     uint len, const char* buf, bool file)
{
    NPN_PostURL( pi->npp, url, window, len, buf, file );
}

/*!
    Requests that the given \a url be retrieved and sent to
    the named \a window. See Netscape's JavaScript documentation for
    an explanation of window names. Passes the arguments including \a
    data to NPN_GetURLNotify.

    \sa
    \link http://developer.netscape.com/docs/manuals/communicator/plugin/refpgur.htm#npngeturlnotify
    Netscape: NPN_GetURLNotify method\endlink
*/
void QNPInstance::getURLNotify(const char* url, const char* window, void*data)
{
#ifdef Q_WS_WIN // Only on Windows?
    NPN_GetURLNotify( pi->npp, url, window, data );
#else
    Q_UNUSED( url );
    Q_UNUSED( window );
    Q_UNUSED( data );
#endif
}

/*!
    \preliminary

    This function is \e{not tested}.

    It is an encapsulation of the NPP_Print function of the Netscape
    Plugin API.
*/
bool QNPInstance::printFullPage()
{
    return FALSE;
}

/*!
    \preliminary

    This function is \e{not tested}.

    Print the instance embedded in a page.

    It is an encapsulation of the NPP_Print function of the Netscape
    Plugin API.
*/
void QNPInstance::print(QPainter*)
{
    // ### default could redirected-print the window.
}

/*!
    Returns the number of arguments to the instance. Note that you
    should not normally rely on the ordering of arguments, and
    note that the SGML specification does not permit multiple
    arguments with the same name.

    \sa arg(), argn()
*/
int QNPInstance::argc() const
{
    return pi->argc;
}

/*!
    Returns the name of the \a{i}-th argument.

    \sa argc(), argv()
*/
const char* QNPInstance::argn(int i) const
{
    return pi->argn[i];
}

/*!
    \preliminary

    This function is \e{not tested}.

    Called whenever a \a url is notified after a call to
    NPN_GetURLNotify with \a notifyData. The reason is given in \a r.

    It is an encapsulation of the NPP_URLNotify function of the
    Netscape Plugin API.

    See also:
    \link http://developer.netscape.com/docs/manuals/communicator/plugin/refpgur.htm#nppurlnotify
    Netscape: NPP_URLNotify method\endlink
*/
void QNPInstance::notifyURL(const char*, Reason, void*)
{
}

/*!
    Returns the value of the \a{i}-th argument.

    \as argc(), arg()
*/
const char* QNPInstance::argv(int i) const
{
    return pi->argv[i];
}

/*!
    Returns the mode of the plugin.
*/
QNPInstance::InstanceMode QNPInstance::mode() const
{
    return (QNPInstance::InstanceMode)pi->fMode;
}

/*!
    Returns the value of the named arguments, or 0 if no argument
    called \a name appears in the \c{<EMBED>} tag of this instance.
    If the argument appears, but has no value assigned, the empty
    string is returned. In summary:

    \table
    \header \i Tag \i Result
    \row \i \c{<EMBED ...>} \i arg("FOO") == 0
    \row \i \c{<EMBED FOO ...>} \i arg("FOO") == ""
    \row \i \c{<EMBED FOO=BAR ...>} \i arg("FOO") == "BAR"
    \endtable
*/
const char* QNPInstance::arg(const char* name) const
{
    for (int i=0; i<pi->argc; i++) {
	// SGML: names are case insensitive
	if ( qstricmp( name, pi->argn[i] ) == 0 ) {
	    if (pi->argv[i].isEmpty())
		return "";
	    else
		return pi->argv[i];
	}
    }
    return 0;
}

/*!
    Returns the user agent (browser name) containing this instance.
*/
const char* QNPInstance::userAgent() const
{
    return NPN_UserAgent(pi->npp);
}

/*!
    \preliminary

    This function is \e{not tested}.

    Requests the creation of a new data stream \e from the plugin.
    The MIME type and window are passed in \a mimetype and \a window.
    \a as_file holds the \c AsFileOnly flag. It is an interface to the
    NPN_NewStream function of the Netscape Plugin API.
*/
QNPStream* QNPInstance::newStream(const char* mimetype, const char* window,
    bool as_file)
{
    NPStream* s=0;
    NPError err = NPN_NewStream(pi->npp, (char*)mimetype, window, &s);
    if (err != NPERR_NO_ERROR) return 0;
    return s ? new QNPStream(this, mimetype, s, as_file) : 0;
}

/*!
    Sets the status message in the browser containing this instance to
    \a msg.
*/
void QNPInstance::status(const char* msg)
{
    NPN_Status(pi->npp, msg);
}


/*!
    Returns the Java object associated with the plugin instance, an
    object of the \link QNPlugin::getJavaClass() plugin's Java
    class\endlink, or 0 if the plug-in does not have a Java class,
    Java is disabled, or an error occurred.

    The return value is actually a \c{jref} we use \c{void*} so as to
    avoid burdening plugins which do not require Java.

    \sa QNPlugin::getJavaClass(), QNPlugin::getJavaEnv(), getJavaPeer()
*/
void* QNPInstance::getJavaPeer() const
{
    return NPN_GetJavaPeer(pi->npp);
}


/*!
    \class QNPStream qnp.h
    \brief The QNPStream class provides a stream of data provided to a QNPInstance by the browser.

    \extension Netscape Plugin

    Note that this is neither a QTextStream nor a QDataStream.

    \sa QNPInstance::write(), QNPInstance::newStreamCreated()
*/

/*!
    Creates a stream. Plugins should not call this; they should call
    QNPInstance::newStream() if they need a stream.

    Takes a QNPInstance \a in, MIME type \a mt, a pointer to an
    _NPStream \a st and a seekable flag \a se.
*/
QNPStream::QNPStream(QNPInstance* in,const char* mt, _NPStream* st, bool se) :
    inst(in),
    stream(st),
    mtype(mt),
    seek(se)
{
    isokay = TRUE;
    iscomplete = FALSE;
}

/*!
    Destroys the stream.
*/
QNPStream::~QNPStream()
{
    if (!qnps_no_call_back) {
	qnps_no_call_back++;
	NPN_DestroyStream(inst->pi->npp, stream, NPRES_USER_BREAK);
	qnps_no_call_back--;
    }
}

/*!
    \fn QNPInstance* QNPStream::instance()

    Returns the QNPInstance for which this stream was created.
*/

/*!
    Returns the URL from which the stream was created.
*/
const char* QNPStream::url() const
{
    return stream->url;
}

/*!
    Returns the length of the stream in bytes. The function might
    return 0 for streams of unknown length.
*/
uint QNPStream::end() const
{
    return stream->end;
}

/*!
    Returns the time when the source of the stream was last modified.
*/
uint QNPStream::lastModified() const
{
    return stream->lastmodified;
}

/*!
    Returns the MIME type of the stream.
*/
const char* QNPStream::type() const
{
    return mtype;
}

/*!
    Returns TRUE if the stream is seekable; otherwise returns FALSE.
*/
bool QNPStream::seekable() const
{
    return seek;
}

/*!
  \internal
*/
void QNPStream::setOkay(bool y)
{
    isokay = y;
}

/*!
  \internal
*/
void QNPStream::setComplete(bool y)
{
    iscomplete = y;
}

/*!
    Returns TRUE if no errors have occurred on the stream; otherwise
    returns FALSE.
*/
bool QNPStream::okay() const
{
    return isokay;
}

/*!
    Returns TRUE if the stream has received all the data from the
    source; otherwise returns FALSE.
*/
bool QNPStream::complete() const
{
    return iscomplete;
}

/*!
    Requests the section of the stream, of \a length bytes from \a
    offset, be sent to the QNPInstance::write() function of the
    instance() of this stream.
*/
void QNPStream::requestRead(int offset, uint length)
{
    NPByteRange range;
    range.offset = offset;
    range.length = length;
    range.next = 0; // ### Only one supported at this time
    NPN_RequestRead(stream, &range);
}

/*!
    Writes \a len bytes from \a buffer \e to the stream.
*/
int QNPStream::write( int len, void* buffer )
{
    return NPN_Write(inst->pi->npp, stream, len, buffer);
}



/******************************************************************************
 * The plugin itself - only one ever exists, created by QNPlugin::create()
 *****************************************************************************/


/*!
    \class QNPlugin qnp.h
    \brief The QNPlugin class provides the main factory for plugin objects.

    \extension Netscape Plugin

    This class is the heart of the plugin. One instance of this object
    is created when the plugin is \e first needed, by calling
    QNPlugin::create(), which must be implemented in your plugin code
    to return some derived class of QNPlugin. The one QNPlugin object
    creates all QNPInstance instances for a web browser running in a
    single process.

    Additionally, if Qt is linked to the plugin as a dynamic library,
    only one instance of QApplication will exist \e{across all plugins
    that have been made with Qt}. So, your plugin should tread lightly
    on global settings. Do not, for example, use
    QApplication::setFont() - that will change the font in every
    widget of every Qt-based plugin currently loaded!
*/

/*!
    \fn QNPlugin* QNPlugin::create()

    This function must be implemented by your plugin code. It should return a
    derived class of QNPlugin.
*/

/*!
    Returns the plugin most recently returned by QNPlugin::create().
*/
QNPlugin* QNPlugin::actual()
{
    return qNP;
}

/*!
    Creates a QNPlugin. This may only be used by the constructor
    of the class, derived from QNPlugin, that is returned by your
    plugin's implementation of the QNPlugin::create() function.
*/
QNPlugin::QNPlugin()
{
    // Encourage linker to include stuff.
    static void* a;
    a = (void*)NP_Initialize;
    a = (void*)NP_Shutdown;
}

/*!
    Destroys the QNPlugin. This is called by the plugin binding code
    just before the plugin is about to be unloaded from memory. If
    newWindow() has been called, a QApplication will still exist at
    this time, but will be deleted shortly after, just before the plugin
    is deleted.
*/
QNPlugin::~QNPlugin()
{
}

/*!
    Populates \e *\a plugin_major and \e *\a plugin_minor with the
    version of the plugin API and populates \e *\a browser_major and
    \e *\a browser_minor with the version of the web browser.
*/
void QNPlugin::getVersionInfo(int& plugin_major, int& plugin_minor,
	     int& browser_major, int& browser_minor)
{
    NPN_Version(&plugin_major, &plugin_minor, &browser_major, &browser_minor);
}

/*!
    \fn QNPInstance* QNPlugin::newInstance()

    Override this function to return an appropriate derived class of
    QNPInstance.
*/

/*!
    \fn const char* QNPlugin::getMIMEDescription() const

    Override this function to return the MIME description of the data formats
    supported by your plugin. The format of this string is shown by
    the following example:

\code
    const char* getMIMEDescription() const
    {
	return "image/x-png:png:PNG Image;"
	       "image/png:png:PNG Image;"
	       "image/x-portable-bitmap:pbm:PBM Image;"
	       "image/x-portable-graymap:pgm:PGM Image;"
	       "image/x-portable-pixmap:ppm:PPM Image;"
	       "image/bmp:bmp:BMP Image;"
	       "image/x-ms-bmp:bmp:BMP Image;"
	       "image/x-xpixmap:xpm:XPM Image;"
	       "image/xpm:xpm:XPM Image";
    }
\endcode
*/

/*!
    \fn const char* QNPlugin::getPluginNameString() const

    Returns a pointer to the plain-text name of the plugin.
*/

/*!
    \fn const char* QNPlugin::getPluginDescriptionString() const

    Returns a pointer to the plain-text description of the plugin.
*/

/*!
    Override this function to return a reference to the Java class that represents
    the plugin. The default returns 0, indicating no class.

    If you override this class, you must also override
    QNPlugin::unuseJavaClass().

    The return value is actually a \c{jref}; we use \c{void*} so as to
    avoid burdening plugins which do not require Java.

    \sa getJavaEnv(), QNPInstance::getJavaPeer()
*/
void* QNPlugin::getJavaClass()
{
    return NULL;
}

/*!
    This function is called when the plugin is shutting down. The
    function should \e unuse the Java class returned earlier by
    getJavaClass().
*/
void QNPlugin::unuseJavaClass()
{
    qFatal("QNPlugin::unuseJavaClass() must be overridden along with getJavaClass()");
}

/*!
    Returns a pointer to the Java execution environment, or 0 if
    either Java is disabled or an error occurred.

    The return value is actually a \c{JRIEnv*}; we use \c{void*} so as
    to avoid burdening plugins which do not require Java.

    \sa getJavaClass(), QNPInstance::getJavaPeer()
*/
void* QNPlugin::getJavaEnv() const
{
    return NPN_GetJavaEnv();
}

#ifdef Q_WS_X11

#include <qapplication.h>
#include <qwidgetintdict.h>

// resolve the conflict between X11's FocusIn and QEvent::FocusIn
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease

Boolean qnpxt_event_dispatcher( XEvent *event );
static void qnpxt_keep_alive();
void qnpxt_timeout_handler( XtPointer, XtIntervalId * );

class QNPXtPrivate
{
public:
    QNPXtPrivate();

    void hookMeUp();
    void unhook();

    XtAppContext appContext, ownContext;
    QMemArray<XtEventDispatchProc> dispatchers;
    QWidgetIntDict mapper;

    QIntDict<QSocketNotifier> socknotDict;
    uint pending_socknots;
    bool activate_timers;
    int timerid;

    // arguments for Xt display initialization
    const char* applicationClass;
    XrmOptionDescRec* options;
    int numOptions;
};
static QNPXtPrivate *static_d = 0;
static XEvent* last_xevent = 0;


bool QNPXt::redeliverEvent( XEvent *event )
{
    // redeliver the event to Xt, NOT through Qt
    if ( static_d->dispatchers[ event->type ]( event ) ) {
	// qDebug( "Xt: redelivered event" );
	return TRUE;
    }
    return FALSE;
};


XEvent* QNPXt::lastEvent()
{
    return last_xevent;
}


QNPXtPrivate::QNPXtPrivate()
    : appContext(NULL), ownContext(NULL),
      pending_socknots(0), activate_timers(FALSE), timerid(-1)
{
}

void QNPXtPrivate::hookMeUp()
{
    // worker to plug Qt into Xt (event dispatchers)
    // and Xt into Qt (QNPXtEventLoop)

    // ### TODO extensions?
    dispatchers.resize( LASTEvent );
    dispatchers.fill( 0 );
    int et;
    for ( et = 2; et < LASTEvent; et++ )
	dispatchers[ et ] =
	    XtSetEventDispatcher( QPaintDevice::x11AppDisplay(),
				  et, ::qnpxt_event_dispatcher );
}

void QNPXtPrivate::unhook()
{
    // unhook Qt from Xt (event dispatchers)
    // unhook Xt from Qt? (QNPXtEventLoop)

    // ### TODO extensions?
    int et;
    for ( et = 2; et < LASTEvent; et++ )
	(void) XtSetEventDispatcher( QPaintDevice::x11AppDisplay(),
				     et, dispatchers[ et ] );
    dispatchers.resize( 0 );

    /*
      We cannot destroy the app context here because it closes the X
      display, something QApplication does as well a bit later.
      if ( ownContext )
          XtDestroyApplicationContext( ownContext );
     */
    appContext = ownContext = 0;
}

extern bool qt_try_modal( QWidget *, XEvent * ); // defined in qapplication_x11.cpp
Boolean qnpxt_event_dispatcher( XEvent *event )
{
    static bool grabbed = FALSE;

    QApplication::sendPostedEvents();

    QWidgetIntDict *mapper = &static_d->mapper;
    QWidget* qnpxt = mapper->find( event->xany.window );
    if ( !qnpxt && QWidget::find( event->xany.window) == 0 ) {
	// event is not for Qt, try Xt
	Widget w = XtWindowToWidget( QPaintDevice::x11AppDisplay(),
				     event->xany.window );
	while ( w && ! ( qnpxt = mapper->find( XtWindow( w ) ) ) ) {
	    if ( XtIsShell( w ) ) {
		break;
	    }
	    w = XtParent( w );
	}

 	if ( qnpxt && ( event->type == XKeyPress ||
			 event->type == XKeyRelease ) )  {
	    // remap key events to keep accelerators working
 	    event->xany.window = qnpxt->winId();
 	}

	if ( w ) {
	    if ( !grabbed && ( event->type        == XFocusIn &&
			       event->xfocus.mode == NotifyGrab ) ) {
		// qDebug( "Xt: grab started" );
		grabbed = TRUE;
	    } else if ( grabbed && ( event->type        == XFocusOut &&
				     event->xfocus.mode == NotifyUngrab ) ) {
		// qDebug( "Xt: grab ended" );
		grabbed = FALSE;
	    }
	}
    }

    /*
      If the mouse has been grabbed for a window that we don't know
      about, we shouldn't deliver any pointer events, since this will
      intercept the event that ends the mouse grab that Xt/Motif
      started.
    */
    bool do_deliver = TRUE;
    if ( grabbed && ( event->type == ButtonPress   ||
		      event->type == ButtonRelease ||
		      event->type == MotionNotify  ||
		      event->type == EnterNotify   ||
		      event->type == LeaveNotify ) )
	do_deliver = FALSE;

    last_xevent = event;
    bool delivered = do_deliver && ( qApp->x11ProcessEvent( event ) != -1 );
    last_xevent = 0;
    if ( qnpxt ) {
	switch ( event->type ) {
	case EnterNotify:
	case LeaveNotify:
	    event->xcrossing.focus = False;
	    delivered = FALSE;
	    break;
	case XKeyPress:
	case XKeyRelease:
	    delivered = TRUE;
	    break;
	case XFocusIn:
	case XFocusOut:
	    delivered = FALSE;
	    break;
	default:
	    delivered = FALSE;
	    break;
	}
    }

    qnpxt_keep_alive();

    if ( delivered ) {
	// qDebug( "Qt: delivered event" );
	return True;
    }

    // discard user input events when we have an active popup widget
    if ( QApplication::activePopupWidget() ) {
	switch ( event->type ) {
	case ButtonPress:			// disallow mouse/key events
	case ButtonRelease:
	case MotionNotify:
	case XKeyPress:
	case XKeyRelease:
	case EnterNotify:
	case LeaveNotify:
	case ClientMessage:
	    // qDebug( "Qt: active popup - discarding event" );
	    return True;

	default:
	    break;
	}
    }

    if ( QApplication::activeModalWidget() ) {
	if ( qnpxt ) {
	    // send event through Qt modality handling...
	    if ( !qt_try_modal( qnpxt, event ) ) {
		// qDebug( "Qt: active modal widget discarded event" );
		return True;
	    }
	} else if ( !grabbed ) {
	    // we could have a pure Xt shell as a child of the active
	    // modal widget
	    QWidget *qw = 0;
	    Widget xw = XtWindowToWidget( QPaintDevice::x11AppDisplay(),
					  event->xany.window );
	    while ( xw && !( qw = mapper->find( XtWindow( xw ) ) ) )
		xw = XtParent( xw );

	    while ( qw && qw != QApplication::activeModalWidget() )
		qw = qw->parentWidget();

	    if ( !qw ) {
		// event is destined for an Xt widget, but since Qt has an
		// active modal widget, we stop here...
		switch ( event->type ) {
		case ButtonPress:			// disallow mouse/key events
		case ButtonRelease:
		case MotionNotify:
		case XKeyPress:
		case XKeyRelease:
		case EnterNotify:
		case LeaveNotify:
		case ClientMessage:
		    // qDebug( "Qt: active modal widget discarded unknown event" );
		    return True;
		default:
		    break;
		}
	    }
	}
    }

    if ( static_d->dispatchers[ event->type ]( event ) ) {
	// qDebug( "Xt: delivered event" );
	// Xt handled the event.
	return True;
    }

    return False;
}



QNPXt::QNPXt( const char *applicationClass, XtAppContext context,
		XrmOptionDescRec *options , int numOptions)
{
#if defined(QT_CHECK_STATE)
    if ( static_d )
	qWarning( "QNPXt: should only have one QNPXt instance!" );
#endif

    d = static_d = new QNPXtPrivate;
    XtToolkitInitialize();
    if ( context )
	d->appContext = context;
    else
	d->ownContext = d->appContext = XtCreateApplicationContext();

    d->applicationClass = applicationClass;
    d->options = options;
    d->numOptions = numOptions;
}


QNPXt::~QNPXt()
{
    delete d;
    static_d = 0;
}

XtAppContext QNPXt::applicationContext() const
{
    return d->appContext;
}


void QNPXt::appStartingUp()
{
    /*
      QApplication could be using a Display from an outside source, so
      we should only initialize the display if the current application
      context does not contain the QApplication display
    */

    bool display_found = FALSE;
    Display **displays;
    Cardinal x, count;
    XtGetDisplays( d->appContext, &displays, &count );
    for ( x = 0; x < count && ! display_found; ++x ) {
	if ( displays[x] == QPaintDevice::x11AppDisplay() )
	    display_found = TRUE;
    }
    if ( displays )
	XtFree( (char *) displays );

    if ( ! display_found ) {
	int argc = qApp->argc();
	XtDisplayInitialize( d->appContext,
			     QPaintDevice::x11AppDisplay(),
			     qApp->name(),
			     d->applicationClass,
			     d->options,
			     d->numOptions,
			     &argc,
			     qApp->argv() );
    }

    d->hookMeUp();

    // start a zero-timer to get the timer keep-alive working
    d->timerid = XtAppAddTimeOut( d->appContext, 0, qnpxt_timeout_handler, 0 );
}

void QNPXt::appClosingDown()
{
    if ( d->timerid != -1 )
	XtRemoveTimeOut( d->timerid );
    d->timerid = -1;

    d->unhook();
}


void QNPXt::registerWidget( QWidget* w )
{
    if ( !static_d )
	return;
    static_d->mapper.insert( w->winId(), w );
}


void QNPXt::unregisterWidget( QWidget* w )
{
    if ( !static_d )
	return;
    static_d->mapper.remove( w->winId() );
}


void qnpxt_socknot_handler( XtPointer pointer, int *, XtInputId *id )
{
    QNPXt *eventloop = (QNPXt *) pointer;
    QSocketNotifier *socknot = static_d->socknotDict.find( *id );
    if ( ! socknot ) // this shouldn't happen
	return;
    eventloop->setSocketNotifierPending( socknot );
    if ( ++static_d->pending_socknots > static_d->socknotDict.count() ) {
	/*
	  We have too many pending socket notifiers.  Since Xt prefers
	  socket notifiers over X events, we should go ahead and
	  activate all our pending socket notifiers so that the event
	  loop doesn't freeze up because of this.
	*/
	eventloop->activateSocketNotifiers();
	static_d->pending_socknots = 0;
    }
}

void QNPXt::registerSocketNotifier( QSocketNotifier *notifier )
{
    XtInputMask mask;
    switch ( notifier->type() ) {
    case QSocketNotifier::Read:
	mask = XtInputReadMask;
	break;

    case QSocketNotifier::Write:
	mask = XtInputWriteMask;
	break;

    case QSocketNotifier::Exception:
	mask = XtInputExceptMask;
	break;

    default:
	qWarning( "QNPXtEventLoop: socket notifier has invalid type" );
	return;
    }

    XtInputId id = XtAppAddInput( d->appContext,
				  notifier->socket(), (XtPointer) mask,
				  qnpxt_socknot_handler, this );
    d->socknotDict.insert( id, notifier );

    QEventLoop::registerSocketNotifier( notifier );
}

void QNPXt::unregisterSocketNotifier( QSocketNotifier *notifier )
{
    QIntDictIterator<QSocketNotifier> it( d->socknotDict );
    while ( it.current() && notifier != it.current() )
	++it;
    if ( ! it.current() ) {
	// this shouldn't happen
	qWarning( "QNPXtEventLoop: failed to unregister socket notifier" );
	return;
    }

    XtRemoveInput( it.currentKey() );
    d->socknotDict.remove( it.currentKey() );

    QEventLoop::unregisterSocketNotifier( notifier );
}

static void qnpxt_keep_alive() {
    // make sure we fire off Qt's timers
    int ttw = QApplication::eventLoop()->timeToWait();
    if ( static_d->timerid != -1 )
	XtRemoveTimeOut( static_d->timerid );
    static_d->timerid = -1;
    if ( ttw != -1 ) {
	static_d->timerid =
	    XtAppAddTimeOut( static_d->appContext, ttw, qnpxt_timeout_handler, 0 );
    }
}

void qnpxt_timeout_handler( XtPointer, XtIntervalId * )
{
    static_d->timerid = -1;

    if ( ! QApplication::eventLoop()->loopLevel() ) {
	/*
	  when the Qt eventloop is not running, make sure that Qt
	  timers still work with an Xt keep-alive timer
	*/
	QApplication::eventLoop()->activateTimers();
	static_d->activate_timers = FALSE;

	qnpxt_keep_alive();
    } else {
	static_d->activate_timers = TRUE;
    }
}

bool QNPXt::processEvents( ProcessEventsFlags flags )
{
    // Qt uses posted events to do lots of delayed operations, like
    // repaints... these need to be delivered before we go to sleep
    QApplication::sendPostedEvents();

    bool canWait = ( flags & WaitForMore );

    qnpxt_keep_alive();

    // get the pending event mask from Xt and process the next event
    XtInputMask pendingmask = XtAppPending( d->appContext );
    XtInputMask mask = pendingmask;
    if ( pendingmask & XtIMTimer ) {
	mask &= ~XtIMTimer;
	// zero timers will starve the Xt X event dispatcher... so
	// process something *instead* of a timer first...
	if ( mask != 0 )
	    XtAppProcessEvent( d->appContext, mask );
	// and process a timer afterwards
	mask = pendingmask & XtIMTimer;
    }

    if ( canWait )
	XtAppProcessEvent( d->appContext, XtIMAll );
    else
	XtAppProcessEvent( d->appContext, mask );

    int nevents = 0;
    if ( ! ( flags & ExcludeSocketNotifiers ) ) {
	nevents += activateSocketNotifiers();
	d->pending_socknots = 0;
    }

    if ( d->activate_timers ) {
	nevents += activateTimers();
    }
    d->activate_timers = FALSE;

    return ( canWait || ( pendingmask != 0 ) || nevents > 0 );
}


#endif // Q_WS_X11
