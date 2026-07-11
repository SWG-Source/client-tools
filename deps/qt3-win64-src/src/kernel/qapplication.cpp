/****************************************************************************
** $Id: qapplication.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QApplication class
**
** Created : 931107
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

#include "qobjectlist.h"
#ifdef Q_OS_CYGWIN
#define QT_NO_INLINE_GLOBALSTRUT
#endif 
#include "qapplication.h"
#include "qeventloop.h"
#include "qeventloop_p.h"
#include "qwidget.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qptrdict.h"
#include "qcleanuphandler.h"

#include "qtranslator.h"
#include "qtextcodec.h"
#include "qsessionmanager.h"
#include "qdragobject.h"
#include "qclipboard.h"
#include "qcursor.h"
#include "qstyle.h"
#include "qstylefactory.h"
#include "qfile.h"
#include "qmessagebox.h"
#include "qdir.h"
#include "qfileinfo.h"
#ifdef Q_WS_WIN
#include "qinputcontext_p.h"
#endif
#include "qfontdata_p.h"

#if defined(QT_THREAD_SUPPORT)
#  include "qmutex.h"
#  include "qthread.h"
#endif // QT_THREAD_SUPPORT

#include <stdlib.h>

#ifdef truncate
# undef truncate
#endif

/*!
  \class QApplication qapplication.h
  \brief The QApplication class manages the GUI application's control
  flow and main settings.

  \ingroup application
  \mainclass

  It contains the main event loop, where all events from the window
  system and other sources are processed and dispatched. It also
  handles the application's initialization and finalization, and
  provides session management. It also handles most system-wide and
  application-wide settings.

  For any GUI application that uses Qt, there is precisely one
  QApplication object, no matter whether the application has 0, 1, 2
  or more windows at any time.

  The QApplication object is accessible through the global pointer \c
  qApp. Its main areas of responsibility are:
  \list

  \i It initializes the application with the user's desktop settings
  such as palette(), font() and doubleClickInterval(). It keeps track
  of these properties in case the user changes the desktop globally, for
  example through some kind of control panel.

  \i It performs event handling, meaning that it receives events
  from the underlying window system and dispatches them to the relevant
  widgets. By using sendEvent() and postEvent() you can send your own
  events to widgets.

  \i It parses common command line arguments and sets its internal
  state accordingly. See the \link QApplication::QApplication()
  constructor documentation\endlink below for more details about this.

  \i It defines the application's look and feel, which is
  encapsulated in a QStyle object. This can be changed at runtime
  with setStyle().

  \i It specifies how the application is to allocate colors.
  See setColorSpec() for details.

  \i It provides localization of strings that are visible to the user
  via translate().

  \i It provides some magical objects like the desktop() and the
  clipboard().

  \i It knows about the application's windows. You can ask which
  widget is at a certain position using widgetAt(), get a list of
  topLevelWidgets() and closeAllWindows(), etc.

  \i It manages the application's mouse cursor handling,
  see setOverrideCursor() and setGlobalMouseTracking().

  \i On the X window system, it provides functions to flush and sync
  the communication stream, see flushX() and syncX().

  \i It provides support for sophisticated \link
  session.html session management \endlink. This makes it possible
  for applications to terminate gracefully when the user logs out, to
  cancel a shutdown process if termination isn't possible and even to
  preserve the entire application's state for a future session. See
  isSessionRestored(), sessionId() and commitData() and saveState()
  for details.

  \endlist

  The <a href="simple-application.html">Application walk-through
  example</a> contains a typical complete main() that does the usual
  things with QApplication.

  Since the QApplication object does so much initialization, it
  <b>must</b> be created before any other objects related to the user
  interface are created.

  Since it also deals with common command line arguments, it is
  usually a good idea to create it \e before any interpretation or
  modification of \c argv is done in the application itself. (Note
  also that for X11, setMainWidget() may change the main widget
  according to the \c -geometry option. To preserve this
  functionality, you must set your defaults before setMainWidget() and
  any overrides after.)

  \table
    \header \i21 Groups of functions
    \row
     \i System settings
     \i
	desktopSettingsAware(),
	setDesktopSettingsAware(),
	cursorFlashTime(),
	setCursorFlashTime(),
	doubleClickInterval(),
	setDoubleClickInterval(),
	wheelScrollLines(),
	setWheelScrollLines(),
	palette(),
	setPalette(),
	font(),
	setFont(),
	fontMetrics().

    \row
     \i Event handling
     \i
	exec(),
	processEvents(),
	enter_loop(),
	exit_loop(),
	exit(),
	quit().
	sendEvent(),
	postEvent(),
	sendPostedEvents(),
	removePostedEvents(),
	hasPendingEvents(),
	notify(),
	macEventFilter(),
	qwsEventFilter(),
	x11EventFilter(),
	x11ProcessEvent(),
	winEventFilter().

    \row
     \i GUI Styles
     \i
	style(),
	setStyle(),
	polish().

    \row
     \i Color usage
     \i
	colorSpec(),
	setColorSpec(),
	qwsSetCustomColors().

    \row
     \i Text handling
     \i
	installTranslator(),
	removeTranslator()
	translate().

    \row
     \i Widgets
     \i
	mainWidget(),
	setMainWidget(),
	allWidgets(),
	topLevelWidgets(),
	desktop(),
	activePopupWidget(),
	activeModalWidget(),
	clipboard(),
	focusWidget(),
	winFocus(),
	activeWindow(),
	widgetAt().

    \row
     \i Advanced cursor handling
     \i
	hasGlobalMouseTracking(),
	setGlobalMouseTracking(),
	overrideCursor(),
	setOverrideCursor(),
	restoreOverrideCursor().

    \row
     \i X Window System synchronization
     \i
	flushX(),
	syncX().

    \row
     \i Session management
     \i
	isSessionRestored(),
	sessionId(),
	commitData(),
	saveState().

    \row
    \i Threading
    \i
	lock(), unlock(), locked(), tryLock(),
	wakeUpGuiThread()

    \row
     \i Miscellaneous
     \i
	closeAllWindows(),
	startingUp(),
	closingDown(),
	type().
  \endtable

  \e {Non-GUI programs:} While Qt is not optimized or
  designed for writing non-GUI programs, it's possible to use
  \link tools.html some of its classes \endlink without creating a
  QApplication. This can be useful if you wish to share code between
  a non-GUI server and a GUI client.

  \headerfile qnamespace.h
  \headerfile qwindowdefs.h
  \headerfile qglobal.h
*/

/*! \enum Qt::HANDLE
    \internal
*/

/*!
    \enum QApplication::Type

    \value Tty a console application
    \value GuiClient a GUI client application
    \value GuiServer a GUI server application
*/

/*!
    \enum QApplication::ColorSpec

    \value NormalColor the default color allocation policy
    \value CustomColor the same as NormalColor for X11; allocates colors
    to a palette on demand under Windows
    \value ManyColor the right choice for applications that use thousands of
    colors

    See setColorSpec() for full details.
*/

/*
  The qt_init() and qt_cleanup() functions are implemented in the
  qapplication_xyz.cpp file.
*/

void qt_init( int *, char **, QApplication::Type );
void qt_cleanup();
#if defined(Q_WS_X11)
void qt_init( Display* dpy, Qt::HANDLE, Qt::HANDLE );
#endif
Q_EXPORT bool qt_tryModalHelper( QWidget *widget, QWidget **rettop );

QApplication *qApp = 0;			// global application object

QStyle   *QApplication::app_style      = 0;	// default application style
bool      qt_explicit_app_style	       = FALSE; // style explicitly set by programmer

int	  QApplication::app_cspec      = QApplication::NormalColor;
#ifndef QT_NO_PALETTE
QPalette *QApplication::app_pal	       = 0;	// default application palette
#endif
QFont	 *QApplication::app_font       = 0;	// default application font
bool	  qt_app_has_font	       = FALSE;
#ifndef QT_NO_CURSOR
QCursor	 *QApplication::app_cursor     = 0;	// default application cursor
#endif
int	  QApplication::app_tracking   = 0;	// global mouse tracking
bool	  QApplication::is_app_running = FALSE;	// app starting up if FALSE
bool	  QApplication::is_app_closing = FALSE;	// app closing down if TRUE
int	  QApplication::loop_level     = 0;	// event loop level
QWidget	 *QApplication::main_widget    = 0;	// main application widget
QWidget	 *QApplication::focus_widget   = 0;	// has keyboard input focus
QWidget	 *QApplication::active_window  = 0;	// toplevel with keyboard focus
bool	  QApplication::obey_desktop_settings = TRUE;	// use winsys resources
int	  QApplication::cursor_flash_time = 1000;	// text caret flash time
int	  QApplication::mouse_double_click_time = 400;	// mouse dbl click limit
#ifndef QT_NO_WHEELEVENT
int	  QApplication::wheel_scroll_lines = 3;		// number of lines to scroll
#endif
bool	  qt_is_gui_used;
bool      Q_EXPORT qt_resolve_symlinks = TRUE;
bool      Q_EXPORT qt_tab_all_widgets  = TRUE;
QRect qt_maxWindowRect;
static int drag_time = 500;
static int drag_distance = 4;
static bool reverse_layout = FALSE;
QSize     QApplication::app_strut	= QSize( 0,0 ); // no default application strut
bool	  QApplication::animate_ui	= TRUE;
bool	  QApplication::animate_menu	= FALSE;
bool	  QApplication::fade_menu	= FALSE;
bool	  QApplication::animate_combo	= FALSE;
bool	  QApplication::animate_tooltip	= FALSE;
bool	  QApplication::fade_tooltip	= FALSE;
bool	  QApplication::animate_toolbox	= FALSE;
bool	  QApplication::widgetCount	= FALSE;
QApplication::Type qt_appType=QApplication::Tty;
#ifndef QT_NO_COMPONENT
QStringList *QApplication::app_libpaths = 0;
#endif
bool	  QApplication::metaComposeUnicode = FALSE;
int	  QApplication::composedUnicode   = 0;

#ifdef QT_THREAD_SUPPORT
QMutex *QApplication::qt_mutex		= 0;
static QMutex *postevent_mutex		= 0;
static Qt::HANDLE qt_application_thread_id = 0;
Q_EXPORT Qt::HANDLE qt_get_application_thread_id()
{
    return qt_application_thread_id;
}
#endif // QT_THREAD_SUPPORT

QEventLoop *QApplication::eventloop = 0;	// application event loop

#ifndef QT_NO_ACCEL
extern bool qt_dispatchAccelEvent( QWidget*, QKeyEvent* ); // def in qaccel.cpp
extern bool qt_tryComposeUnicode( QWidget*, QKeyEvent* ); // def in qaccel.cpp
#endif

#if defined(QT_TABLET_SUPPORT)
bool chokeMouse = FALSE;
#endif

void qt_setMaxWindowRect(const QRect& r)
{
    qt_maxWindowRect = r;
    // Re-resize any maximized windows
    QWidgetList* l = QApplication::topLevelWidgets();
    if ( l ) {
	QWidget *w = l->first();
	while ( w ) {
	    if ( w->isVisible() && w->isMaximized() )
	    {
		w->showNormal(); //#### flicker
		w->showMaximized();
	    }
	    w = l->next();
	}
	delete l;
    }
}

typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

/*!
  \relates QApplication

  Adds a global routine that will be called from the QApplication
  destructor. This function is normally used to add cleanup routines
  for program-wide functionality.

  The function given by \a p should take no arguments and return
  nothing, like this:
  \code
    static int *global_ptr = 0;

    static void cleanup_ptr()
    {
	delete [] global_ptr;
	global_ptr = 0;
    }

    void init_ptr()
    {
	global_ptr = new int[100];	// allocate data
	qAddPostRoutine( cleanup_ptr );	// delete later
    }
  \endcode

  Note that for an application- or module-wide cleanup,
  qAddPostRoutine() is often not suitable. People have a tendency to
  make such modules dynamically loaded, and then unload those modules
  long before the QApplication destructor is called, for example.

  For modules and libraries, using a reference-counted initialization
  manager or Qt' parent-child delete mechanism may be better. Here is
  an example of a private class which uses the parent-child mechanism
  to call a cleanup function at the right time:

  \code
    class MyPrivateInitStuff: public QObject {
    private:
	MyPrivateInitStuff( QObject * parent ): QObject( parent) {
	    // initialization goes here
	}
	MyPrivateInitStuff * p;

    public:
	static MyPrivateInitStuff * initStuff( QObject * parent ) {
	    if ( !p )
		p = new MyPrivateInitStuff( parent );
	    return p;
	}

	~MyPrivateInitStuff() {
	    // cleanup (the "post routine") goes here
	}
    }
  \endcode

  By selecting the right parent widget/object, this can often be made
  to clean up the module's data at the exact right moment.
*/

Q_EXPORT void qAddPostRoutine( QtCleanUpFunction p)
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	Q_CHECK_PTR( postRList );
    }
    postRList->prepend( p );
}


Q_EXPORT void qRemovePostRoutine( QtCleanUpFunction p )
{
    if ( !postRList ) return;
    QVFuncList::Iterator it = postRList->begin();
    while ( it != postRList->end() ) {
	if ( *it == p ) {
	    postRList->remove( it );
	    it = postRList->begin();
	} else {
	    ++it;
	}
    }
}

// Default application palettes and fonts (per widget type)
QAsciiDict<QPalette> *QApplication::app_palettes = 0;
QAsciiDict<QFont>    *QApplication::app_fonts = 0;

#ifndef QT_NO_SESSIONMANAGER
QString *QApplication::session_key = 0;		// ## session key. Should be a member in 4.0
#endif
QWidgetList *QApplication::popupWidgets = 0;	// has keyboard input focus

QDesktopWidget *qt_desktopWidget = 0;		// root window widgets
#ifndef QT_NO_CLIPBOARD
QClipboard	      *qt_clipboard = 0;	// global clipboard object
#endif
QWidgetList * qt_modal_stack=0;		// stack of modal widgets

// Definitions for posted events
struct QPostEvent {
    QPostEvent( QObject *r, QEvent *e ): receiver( r ), event( e ) {}
   ~QPostEvent()			{ delete event; }
    QObject  *receiver;
    QEvent   *event;
};

class Q_EXPORT QPostEventList : public QPtrList<QPostEvent>
{
public:
    QPostEventList() : QPtrList<QPostEvent>() {}
    QPostEventList( const QPostEventList &list ) : QPtrList<QPostEvent>(list) {}
   ~QPostEventList() { clear(); }
    QPostEventList &operator=(const QPostEventList &list)
	{ return (QPostEventList&)QPtrList<QPostEvent>::operator=(list); }
};
class Q_EXPORT QPostEventListIt : public QPtrListIterator<QPostEvent>
{
public:
    QPostEventListIt( const QPostEventList &l ) : QPtrListIterator<QPostEvent>(l) {}
    QPostEventListIt &operator=(const QPostEventListIt &i)
{ return (QPostEventListIt&)QPtrListIterator<QPostEvent>::operator=(i); }
};

static QPostEventList *globalPostedEvents = 0;	// list of posted events

uint qGlobalPostedEventsCount()
{
    if (!globalPostedEvents)
	return 0;
    return globalPostedEvents->count();
}

static QSingleCleanupHandler<QPostEventList> qapp_cleanup_events;

#ifndef QT_NO_PALETTE
QPalette *qt_std_pal = 0;

void qt_create_std_palette()
{
    if ( qt_std_pal )
	delete qt_std_pal;

    QColor standardLightGray( 192, 192, 192 );
    QColor light( 255, 255, 255 );
    QColor dark( standardLightGray.dark( 150 ) );
    QColorGroup std_act( Qt::black, standardLightGray,
			 light, dark, Qt::gray,
			 Qt::black, Qt::white );
    QColorGroup std_dis( Qt::darkGray, standardLightGray,
			 light, dark, Qt::gray,
			 Qt::darkGray, std_act.background() );
    QColorGroup std_inact( Qt::black, standardLightGray,
			   light, dark, Qt::gray,
			   Qt::black, Qt::white );
    qt_std_pal = new QPalette( std_act, std_dis, std_inact );
}

static void qt_fix_tooltips()
{
    // No resources for this yet (unlike on Windows).
    QColorGroup cg( Qt::black, QColor(255,255,220),
		    QColor(96,96,96), Qt::black, Qt::black,
		    Qt::black, QColor(255,255,220) );
    QPalette pal( cg, cg, cg );
    QApplication::setPalette( pal, TRUE, "QTipLabel");
}
#endif

void QApplication::process_cmdline( int* argcptr, char ** argv )
{
    // process platform-indep command line
    if ( !qt_is_gui_used || !*argcptr)
	return;

    int argc = *argcptr;
    int i, j;

    j = 1;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	QCString s;
	if ( arg == "-qdevel" || arg == "-qdebug") {
	    // obsolete argument
	} else if ( arg.find( "-style=", 0, FALSE ) != -1 ) {
	    s = arg.right( arg.length() - 7 );
	} else if ( qstrcmp(arg,"-style") == 0 && i < argc-1 ) {
	    s = argv[++i];
	    s = s.lower();
#ifndef QT_NO_SESSIONMANAGER
	} else if ( qstrcmp(arg,"-session") == 0 && i < argc-1 ) {
	    QCString s = argv[++i];
	    if ( !s.isEmpty() ) {
		session_id = QString::fromLatin1( s );
		int p = session_id.find( '_' );
		if ( p >= 0 ) {
		    if ( !session_key )
			session_key = new QString;
		    *session_key = session_id.mid( p +1 );
		    session_id = session_id.left( p );
		}
		is_session_restored = TRUE;
	    }
#endif
	} else if ( qstrcmp(arg, "-reverse") == 0 ) {
	    setReverseLayout( TRUE );
	} else if ( qstrcmp(arg, "-widgetcount") == 0 ) {
	    widgetCount = TRUE;;
	} else {
	    argv[j++] = argv[i];
	}
#ifndef QT_NO_STYLE
	if ( !s.isEmpty() ) {
	    setStyle( s );
	}
#endif
    }

    if(j < argc) {
#ifdef Q_WS_MACX
	static char* empty = "\0";
	argv[j] = empty;
#else
	argv[j] = 0;
#endif
	*argcptr = j;
    }
}

/*!
  Initializes the window system and constructs an application object
  with \a argc command line arguments in \a argv.

  The global \c qApp pointer refers to this application object. Only
  one application object should be created.

  This application object must be constructed before any \link
  QPaintDevice paint devices\endlink (including widgets, pixmaps, bitmaps
  etc.).

  Note that \a argc and \a argv might be changed. Qt removes command
  line arguments that it recognizes. The modified \a argc and \a argv
  can also be accessed later with \c qApp->argc() and \c qApp->argv().
  The documentation for argv() contains a detailed description of how
  to process command line arguments.

  Qt debugging options (not available if Qt was compiled with the
  QT_NO_DEBUG flag defined):
  \list
  \i -nograb, tells Qt that it must never grab the mouse or the keyboard.
  \i -dograb (only under X11), running under a debugger can cause
  an implicit -nograb, use -dograb to override.
  \i -sync (only under X11), switches to synchronous mode for
	debugging.
  \endlist

  See \link debug.html Debugging Techniques \endlink for a more
  detailed explanation.

  All Qt programs automatically support the following command line options:
  \list
  \i -reverse causes text to be formatted for right-to-left languages
       rather than in the usual left-to-right direction.
  \i -style= \e style, sets the application GUI style. Possible values
       are \c motif, \c windows, and \c platinum. If you compiled Qt
       with additional styles or have additional styles as plugins these
       will be available to the \c -style command line option.
  \i -style \e style, is the same as listed above.
  \i -session= \e session, restores the application from an earlier
       \link session.html session \endlink.
  \i -session \e session, is the same as listed above.
  \i -widgetcount, prints debug message at the end about number of widgets left
       undestroyed and maximum number of widgets existed at the same time
  \endlist

  The X11 version of Qt also supports some traditional X11
  command line options:
  \list
  \i -display \e display, sets the X display (default is $DISPLAY).
  \i -geometry \e geometry, sets the client geometry of the
	\link setMainWidget() main widget\endlink.
  \i -fn or \c -font \e font, defines the application font. The
  font should be specified using an X logical font description.
  \i -bg or \c -background \e color, sets the default background color
	and an application palette (light and dark shades are calculated).
  \i -fg or \c -foreground \e color, sets the default foreground color.
  \i -btn or \c -button \e color, sets the default button color.
  \i -name \e name, sets the application name.
  \i -title \e title, sets the application title (caption).
  \i -visual \c TrueColor, forces the application to use a TrueColor visual
       on an 8-bit display.
  \i -ncols \e count, limits the number of colors allocated in the
       color cube on an 8-bit display, if the application is using the
       \c QApplication::ManyColor color specification. If \e count is
       216 then a 6x6x6 color cube is used (i.e. 6 levels of red, 6 of green,
       and 6 of blue); for other values, a cube
       approximately proportional to a 2x3x1 cube is used.
  \i -cmap, causes the application to install a private color map
       on an 8-bit display.
  \endlist

  \sa argc(), argv()
*/

//######### BINARY COMPATIBILITY constructor
QApplication::QApplication( int &argc, char **argv )
{
    construct( argc, argv, GuiClient );
}


/*!
  Constructs an application object with \a argc command line arguments
  in \a argv. If \a GUIenabled is TRUE, a GUI application is
  constructed, otherwise a non-GUI (console) application is created.

  Set \a GUIenabled to FALSE for programs without a graphical user
  interface that should be able to run without a window system.

  On X11, the window system is initialized if \a GUIenabled is TRUE.
  If \a GUIenabled is FALSE, the application does not connect to the
  X-server.
  On Windows and Macintosh, currently the window system is always
  initialized, regardless of the value of GUIenabled. This may change in
  future versions of Qt.

  The following example shows how to create an application that
  uses a graphical interface when available.
  \code
  int main( int argc, char **argv )
  {
#ifdef Q_WS_X11
    bool useGUI = getenv( "DISPLAY" ) != 0;
#else
    bool useGUI = TRUE;
#endif
    QApplication app(argc, argv, useGUI);

    if ( useGUI ) {
       //start GUI version
       ...
    } else {
       //start non-GUI version
       ...
    }
    return app.exec();
  }
\endcode
*/

QApplication::QApplication( int &argc, char **argv, bool GUIenabled  )
{
    construct( argc, argv, GUIenabled ? GuiClient : Tty );
}

/*!
  Constructs an application object with \a argc command line arguments
  in \a argv.

  For Qt/Embedded, passing \c QApplication::GuiServer for \a type
  makes this application the server (equivalent to running with the
  -qws option).
*/
QApplication::QApplication( int &argc, char **argv, Type type )
{
    construct( argc, argv, type );
}

Q_EXPORT void qt_ucm_initialize( QApplication *theApp )
{
    if ( qApp )
	return;
    int argc = theApp->argc();
    char **argv = theApp->argv();
    theApp->construct( argc, argv, qApp->type() );

    Q_ASSERT( qApp == theApp );
}

void QApplication::construct( int &argc, char **argv, Type type )
{
    qt_appType = type;
    qt_is_gui_used = (type != Tty);
    init_precmdline();
    static const char *empty = "";
    if ( argc == 0 || argv == 0 ) {
	argc = 0;
	argv = (char **)&empty; // ouch! careful with QApplication::argv()!
    }
    app_argc = argc;
    app_argv = argv;

    qt_init( &argc, argv, type );   // Must be called before initialize()
    process_cmdline( &argc, argv );
    initialize( argc, argv );
    if ( qt_is_gui_used )
	qt_maxWindowRect = desktop()->rect();
    if ( eventloop )
	eventloop->appStartingUp();
}

/*!
    Returns the type of application, Tty, GuiClient or GuiServer.
*/

QApplication::Type QApplication::type() const
{
    return qt_appType;
}

#if defined(Q_WS_X11)
/*!
  Create an application, given an already open display \a dpy. If \a
  visual and \a colormap are non-zero, the application will use those as
  the default Visual and Colormap contexts.

  \warning Qt only supports TrueColor visuals at depths higher than 8
  bits-per-pixel.

  This is available only on X11.
*/

QApplication::QApplication( Display* dpy, HANDLE visual, HANDLE colormap )
{
    static int aargc = 1;
    // ### a string literal is a cont char*
    // ### using it as a char* is wrong and could lead to segfaults
    // ### if aargv is modified someday
    static char *aargv[] = { (char*)"unknown", 0 };

    app_argc = aargc;
    app_argv = aargv;

    qt_appType = GuiClient;
    qt_is_gui_used = TRUE;
    qt_appType = GuiClient;
    init_precmdline();
    // ... no command line.

    if ( ! dpy ) {
#ifdef QT_CHECK_STATE
	qWarning( "QApplication: invalid Display* argument." );
#endif // QT_CHECK_STATE

	qt_init( &aargc, aargv, GuiClient );
    } else {
	qt_init( dpy, visual, colormap );
    }

    initialize( aargc, aargv );

    if ( qt_is_gui_used )
	qt_maxWindowRect = desktop()->rect();
    if ( eventloop )
	eventloop->appStartingUp();
}

/*!
  Create an application, given an already open display \a dpy and using
  \a argc command line arguments in \a argv. If \a
  visual and \a colormap are non-zero, the application will use those as
  the default Visual and Colormap contexts.

  \warning Qt only supports TrueColor visuals at depths higher than 8
  bits-per-pixel.

  This is available only on X11.

*/
QApplication::QApplication(Display *dpy, int argc, char **argv,
			   HANDLE visual, HANDLE colormap)
{
    qt_appType = GuiClient;
    qt_is_gui_used = TRUE;
    qt_appType = GuiClient;
    init_precmdline();

    app_argc = argc;
    app_argv = argv;

    if ( ! dpy ) {
#ifdef QT_CHECK_STATE
	qWarning( "QApplication: invalid Display* argument." );
#endif // QT_CHECK_STATE

	qt_init( &argc, argv, GuiClient );
    } else {
	qt_init(dpy, visual, colormap);
    }

    process_cmdline( &argc, argv );
    initialize(argc, argv);

    if ( qt_is_gui_used )
	qt_maxWindowRect = desktop()->rect();
    if ( eventloop )
	eventloop->appStartingUp();
}


#endif // Q_WS_X11


void QApplication::init_precmdline()
{
    translators = 0;
    is_app_closing = FALSE;
#ifndef QT_NO_SESSIONMANAGER
    is_session_restored = FALSE;
#endif
#if defined(QT_CHECK_STATE)
    if ( qApp )
	qWarning( "QApplication: There should be max one application object" );
#endif
    qApp = (QApplication*)this;
}

/*!
  Initializes the QApplication object, called from the constructors.
*/

void QApplication::initialize( int argc, char **argv )
{
#ifdef QT_THREAD_SUPPORT
    qt_mutex = new QMutex( TRUE );
    postevent_mutex = new QMutex( TRUE );
    qt_application_thread_id = QThread::currentThread();
#endif // QT_THREAD_SUPPORT

    app_argc = argc;
    app_argv = argv;
    quit_now = FALSE;
    quit_code = 0;
    QWidget::createMapper(); // create widget mapper
#ifndef QT_NO_PALETTE
    (void) palette();  // trigger creation of application palette
#endif
    is_app_running = TRUE; // no longer starting up

#ifndef QT_NO_SESSIONMANAGER
    // connect to the session manager
    if ( !session_key )
	session_key = new QString;
    session_manager = new QSessionManager( qApp, session_id, *session_key );
#endif

}


/*****************************************************************************
  Functions returning the active popup and modal widgets.
 *****************************************************************************/

/*!
  Returns the active popup widget.

  A popup widget is a special top level widget that sets the \c
  WType_Popup widget flag, e.g. the QPopupMenu widget. When the
  application opens a popup widget, all events are sent to the popup.
  Normal widgets and modal widgets cannot be accessed before the popup
  widget is closed.

  Only other popup widgets may be opened when a popup widget is shown.
  The popup widgets are organized in a stack. This function returns
  the active popup widget at the top of the stack.

  \sa activeModalWidget(), topLevelWidgets()
*/

QWidget *QApplication::activePopupWidget()
{
    return popupWidgets ? popupWidgets->getLast() : 0;
}


/*!
  Returns the active modal widget.

  A modal widget is a special top level widget which is a subclass of
  QDialog that specifies the modal parameter of the constructor as
  TRUE. A modal widget must be closed before the user can continue
  with other parts of the program.

  Modal widgets are organized in a stack. This function returns
  the active modal widget at the top of the stack.

  \sa activePopupWidget(), topLevelWidgets()
*/

QWidget *QApplication::activeModalWidget()
{
    return qt_modal_stack ? qt_modal_stack->getFirst() : 0;
}

/*!
  Cleans up any window system resources that were allocated by this
  application. Sets the global variable \c qApp to 0.
*/

QApplication::~QApplication()
{
#ifndef QT_NO_CLIPBOARD
    // flush clipboard contents
    if ( qt_clipboard ) {
	QCustomEvent event( QEvent::Clipboard );
	QApplication::sendEvent( qt_clipboard, &event );
    }
#endif

    if ( eventloop )
	eventloop->appClosingDown();
    if ( postRList ) {
	QVFuncList::Iterator it = postRList->begin();
	while ( it != postRList->end() ) {	// call post routines
	    (**it)();
	    postRList->remove( it );
	    it = postRList->begin();
	}
	delete postRList;
	postRList = 0;
    }

    QObject *tipmanager = child( "toolTipManager", "QTipManager", FALSE );
    delete tipmanager;

    delete qt_desktopWidget;
    qt_desktopWidget = 0;
    is_app_closing = TRUE;

#ifndef QT_NO_CLIPBOARD
    delete qt_clipboard;
    qt_clipboard = 0;
#endif
    QWidget::destroyMapper();
#ifndef QT_NO_PALETTE
    delete qt_std_pal;
    qt_std_pal = 0;
    delete app_pal;
    app_pal = 0;
    delete app_palettes;
    app_palettes = 0;
#endif
    delete app_font;
    app_font = 0;
    delete app_fonts;
    app_fonts = 0;
#ifndef QT_NO_STYLE
    delete app_style;
    app_style = 0;
#endif
#ifndef QT_NO_CURSOR
    delete app_cursor;
    app_cursor = 0;
#endif
#ifndef QT_NO_TRANSLATION
    delete translators;
#endif

#ifndef QT_NO_DRAGANDDROP
    extern QDragManager *qt_dnd_manager;
    delete qt_dnd_manager;
#endif

    qt_cleanup();

#ifndef QT_NO_COMPONENT
    delete app_libpaths;
    app_libpaths = 0;
#endif

#ifdef QT_THREAD_SUPPORT
    delete qt_mutex;
    qt_mutex = 0;
    delete postevent_mutex;
    postevent_mutex = 0;
#endif // QT_THREAD_SUPPORT

    if( qApp == this ) {
	if ( postedEvents )
	    removePostedEvents( this );
	qApp = 0;
    }
    is_app_running = FALSE;

    if ( widgetCount ) {
	qDebug( "Widgets left: %i    Max widgets: %i \n", QWidget::instanceCounter, QWidget::maxInstances );
    }
#ifndef QT_NO_SESSIONMANAGER
    delete session_manager;
    session_manager = 0;
    delete session_key;
    session_key = 0;
#endif //QT_NO_SESSIONMANAGER

    qt_explicit_app_style = FALSE;
    qt_app_has_font = FALSE;
    app_tracking = 0;
    obey_desktop_settings = TRUE;
    cursor_flash_time = 1000;
    mouse_double_click_time = 400;
#ifndef QT_NO_WHEELEVENT
    wheel_scroll_lines = 3;
#endif
    drag_time = 500;
    drag_distance = 4;
    reverse_layout = FALSE;
    app_strut = QSize( 0, 0 );
    animate_ui = TRUE;
    animate_menu = FALSE;
    fade_menu = FALSE;
    animate_combo = FALSE;
    animate_tooltip = FALSE;
    fade_tooltip = FALSE;
    widgetCount = FALSE;
}


/*!
    \fn int QApplication::argc() const

    Returns the number of command line arguments.

    The documentation for argv() describes how to process command line
    arguments.

    \sa argv(), QApplication::QApplication()
*/

/*!
    \fn char **QApplication::argv() const

    Returns the command line argument vector.

    \c argv()[0] is the program name, \c argv()[1] is the first
    argument and \c argv()[argc()-1] is the last argument.

    A QApplication object is constructed by passing \e argc and \e
    argv from the \c main() function. Some of the arguments may be
    recognized as Qt options and removed from the argument vector. For
    example, the X11 version of Qt knows about \c -display, \c -font
    and a few more options.

    Example:
    \code
	// showargs.cpp - displays program arguments in a list box

	#include <qapplication.h>
	#include <qlistbox.h>

	int main( int argc, char **argv )
	{
	    QApplication a( argc, argv );
	    QListBox b;
	    a.setMainWidget( &b );
	    for ( int i = 0; i < a.argc(); i++ )  // a.argc() == argc
		b.insertItem( a.argv()[i] );      // a.argv()[i] == argv[i]
	    b.show();
	    return a.exec();
	}
    \endcode

    If you run \c{showargs -display unix:0 -font 9x15bold hello world}
    under X11, the list box contains the three strings "showargs",
    "hello" and "world".

    Qt provides a global pointer, \c qApp, that points to the
    QApplication object, and through which you can access argc() and
    argv() in functions other than main().

    \sa argc(), QApplication::QApplication()
*/

/*!
    \fn void QApplication::setArgs( int argc, char **argv )
    \internal
*/


#ifndef QT_NO_STYLE

static QString *qt_style_override = 0;

/*!
  Returns the application's style object.

  \sa setStyle(), QStyle
*/
QStyle& QApplication::style()
{
#ifndef QT_NO_STYLE
    if ( app_style )
	return *app_style;
    if ( !qt_is_gui_used )
	qFatal( "No style available in non-gui applications!" );

#if defined(Q_WS_X11)
    if(!qt_style_override)
	x11_initialize_style(); // run-time search for default style
#endif
    if ( !app_style ) {
	// Compile-time search for default style
	//
	QString style;
	if ( qt_style_override ) {
	    style = *qt_style_override;
	    delete qt_style_override;
	    qt_style_override = 0;
	} else {
#  if defined(Q_WS_WIN) && defined(Q_OS_TEMP)
	    style = "PocketPC";
#elif defined(Q_WS_WIN)
	    if ( qWinVersion() >= Qt::WV_XP && qWinVersion() < Qt::WV_NT_based )
		style = "WindowsXP";
	    else
		style = "Windows";		// default styles for Windows
#elif defined(Q_WS_X11) && defined(Q_OS_SOLARIS)
	    style = "CDE";			// default style for X11 on Solaris
#elif defined(Q_WS_X11) && defined(Q_OS_IRIX)
	    style = "SGI";			// default style for X11 on IRIX
#elif defined(Q_WS_X11)
		style = "Motif";		// default style for X11
#elif defined(Q_WS_MAC)
		style = "Macintosh";		// default style for all Mac's
#elif defined(Q_WS_QWS)
	    style = "Compact";		// default style for small devices
#endif
	}
	app_style = QStyleFactory::create( style );
	if ( !app_style &&		// platform default style not available, try alternatives
	     !(app_style = QStyleFactory::create( "Windows" ) ) &&
	     !(app_style = QStyleFactory::create( "Platinum" ) ) &&
	     !(app_style = QStyleFactory::create( "MotifPlus" ) ) &&
	     !(app_style = QStyleFactory::create( "Motif" ) ) &&
	     !(app_style = QStyleFactory::create( "CDE" ) ) &&
	     !(app_style = QStyleFactory::create( "Aqua" ) ) &&
	     !(app_style = QStyleFactory::create( "SGI" ) ) &&
	     !(app_style = QStyleFactory::create( "Compact" ) )
#ifndef QT_NO_STRINGLIST
	    && !(app_style = QStyleFactory::create( QStyleFactory::keys()[0]  ) )
#endif
	)
	    qFatal( "No %s style available!", style.latin1() );
    }

    QPalette app_pal_copy ( *app_pal );
    app_style->polish( *app_pal );

    if ( is_app_running && !is_app_closing && (*app_pal != app_pal_copy) ) {
	QEvent e( QEvent::ApplicationPaletteChange );
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    sendEvent( w, &e );
	}
    }

    app_style->polish( qApp );
#endif
    return *app_style;
}

/*!
  Sets the application's GUI style to \a style. Ownership of the style
  object is transferred to QApplication, so QApplication will delete
  the style object on application exit or when a new style is set.

  Example usage:
  \code
    QApplication::setStyle( new QWindowsStyle );
  \endcode

  When switching application styles, the color palette is set back to
  the initial colors or the system defaults. This is necessary since
  certain styles have to adapt the color palette to be fully
  style-guide compliant.

  \sa style(), QStyle, setPalette(), desktopSettingsAware()
*/
void QApplication::setStyle( QStyle *style )
{
    QStyle* old = app_style;
    app_style = style;
#ifdef Q_WS_X11
    qt_explicit_app_style = TRUE;
#endif // Q_WS_X11

    if ( startingUp() ) {
	delete old;
	return;
    }

    // clean up the old style
    if (old) {
	if ( is_app_running && !is_app_closing ) {
	    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	    register QWidget *w;
	    while ( (w=it.current()) ) {		// for all widgets...
		++it;
		if ( !w->testWFlags(WType_Desktop) &&	// except desktop
		     w->testWState(WState_Polished) ) { // has been polished
		    old->unPolish(w);
		}
	    }
	}
	old->unPolish( qApp );
    }

    // take care of possible palette requirements of certain gui
    // styles. Do it before polishing the application since the style
    // might call QApplication::setStyle() itself
    if ( !qt_std_pal )
	qt_create_std_palette();
    QPalette tmpPal = *qt_std_pal;
    setPalette( tmpPal, TRUE );

    // initialize the application with the new style
    app_style->polish( qApp );

    // re-polish existing widgets if necessary
    if (old) {
	if ( is_app_running && !is_app_closing ) {
	    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	    register QWidget *w;
	    while ( (w=it.current()) ) {		// for all widgets...
		++it;
		if ( !w->testWFlags(WType_Desktop) ) {	// except desktop
		    if ( w->testWState(WState_Polished) )
			app_style->polish(w);		// repolish
		    w->styleChange( *old );
		    if ( w->isVisible() ){
			w->update();
		    }
		}
	    }
	}
	delete old;
    }
}

/*!
  \overload

  Requests a QStyle object for \a style from the QStyleFactory.

  The string must be one of the QStyleFactory::keys(), typically one
  of "windows", "motif", "cde", "motifplus", "platinum", "sgi" and
  "compact". Depending on the platform, "windowsxp", "aqua" or
  "macintosh" may be available.

  A later call to the QApplication constructor will override the
  requested style when a "-style" option is passed in as a commandline
  parameter.

  Returns 0 if an unknown \a style is passed, otherwise the QStyle object
  returned is set as the application's GUI style.
*/
QStyle* QApplication::setStyle( const QString& style )
{
#ifdef Q_WS_X11
    qt_explicit_app_style = TRUE;
#endif // Q_WS_X11

    if ( startingUp() ) {
	if(qt_style_override)
	    *qt_style_override = style;
	else
	    qt_style_override = new QString(style);
	return 0;
    }
    QStyle *s = QStyleFactory::create( style );
    if ( !s )
	return 0;

    setStyle( s );
    return s;
}

#endif


#if 1  /* OBSOLETE */

QApplication::ColorMode QApplication::colorMode()
{
    return (QApplication::ColorMode)app_cspec;
}

void QApplication::setColorMode( QApplication::ColorMode mode )
{
    app_cspec = mode;
}
#endif


/*!
  Returns the color specification.
  \sa QApplication::setColorSpec()
 */

int QApplication::colorSpec()
{
    return app_cspec;
}

/*!
  Sets the color specification for the application to \a spec.

  The color specification controls how the application allocates colors
  when run on a display with a limited amount of colors, e.g. 8 bit / 256
  color displays.

  The color specification must be set before you create the QApplication
  object.

  The options are:
  \list
  \i QApplication::NormalColor.
    This is the default color allocation strategy. Use this option if
    your application uses buttons, menus, texts and pixmaps with few
    colors. With this option, the application uses system global
    colors. This works fine for most applications under X11, but on
    Windows machines it may cause dithering of non-standard colors.
  \i QApplication::CustomColor.
    Use this option if your application needs a small number of custom
    colors. On X11, this option is the same as NormalColor. On Windows, Qt
    creates a Windows palette, and allocates colors to it on demand.
  \i QApplication::ManyColor.
    Use this option if your application is very color hungry
    (e.g. it requires thousands of colors).
    Under X11 the effect is:
    \list
    \i For 256-color displays which have at best a 256 color true color
       visual, the default visual is used, and colors are allocated
       from a color cube. The color cube is the 6x6x6 (216 color) "Web
       palette"<sup>*</sup>, but the number of colors can be changed
       by the \e -ncols option. The user can force the application to
       use the true color visual with the \link
       QApplication::QApplication() -visual \endlink option.
    \i For 256-color displays which have a true color visual with more
       than 256 colors, use that visual. Silicon Graphics X servers
       have this feature, for example. They provide an 8 bit visual
       by default but can deliver true color when asked.
    \endlist
    On Windows, Qt creates a Windows palette, and fills it with a color cube.
  \endlist

  Be aware that the CustomColor and ManyColor choices may lead to colormap
  flashing: The foreground application gets (most) of the available
  colors, while the background windows will look less attractive.

  Example:
  \code
  int main( int argc, char **argv )
  {
      QApplication::setColorSpec( QApplication::ManyColor );
      QApplication a( argc, argv );
      ...
  }
  \endcode

  QColor provides more functionality for controlling color allocation and
  freeing up certain colors. See QColor::enterAllocContext() for more
  information.

  To check what mode you end up with, call QColor::numBitPlanes() once
  the QApplication object exists. A value greater than 8 (typically
  16, 24 or 32) means true color.

  <sup>*</sup> The color cube used by Qt has 216 colors whose red,
  green, and blue components always have one of the following values:
  0x00, 0x33, 0x66, 0x99, 0xCC, or 0xFF.

  \sa colorSpec(), QColor::numBitPlanes(), QColor::enterAllocContext() */

void QApplication::setColorSpec( int spec )
{
#if defined(QT_CHECK_STATE)
    if ( qApp ) {
	qWarning( "QApplication::setColorSpec: This function must be "
		 "called before the QApplication object is created" );
    }
#endif
    app_cspec = spec;
}

/*!
  \fn QSize QApplication::globalStrut()

  Returns the application's global strut.

  The strut is a size object whose dimensions are the minimum that any
  GUI element that the user can interact with should have. For example
  no button should be resized to be smaller than the global strut size.

  \sa setGlobalStrut()
*/

/*!
  Sets the application's global strut to \a strut.

  The strut is a size object whose dimensions are the minimum that any
  GUI element that the user can interact with should have. For example
  no button should be resized to be smaller than the global strut size.

  The strut size should be considered when reimplementing GUI controls
  that may be used on touch-screens or similar IO-devices.

  Example:
  \code
  QSize& WidgetClass::sizeHint() const
  {
      return QSize( 80, 25 ).expandedTo( QApplication::globalStrut() );
  }
  \endcode

  \sa globalStrut()
*/

void QApplication::setGlobalStrut( const QSize& strut )
{
    app_strut = strut;
}

// before cygwin qt release 3.3 release, the runtime pseudo relocation support (using vars from a dll with offset) 
// wasn't present and inlining globalsStrut wasn't possible. Applications build with older qt releases expects this 
// symbol, so we define it for compatibility. Please note, thas for compiling qt applications with this release 
// globalStrut is inlined.  
#ifdef QT_NO_INLINE_GLOBALSTRUT
QSize &QApplication::globalStrut()
{
    return app_strut;
}
#endif 

#if defined( Q_WS_WIN ) || defined( Q_WS_MAC )
extern const char *qAppFileName();
#endif

#ifndef QT_NO_DIR
#ifndef Q_WS_WIN
static QString resolveSymlinks( const QString& path, int depth = 0 )
{
    bool foundLink = FALSE;
    QString linkTarget;
    QString part = path;
    int slashPos = path.length();

    // too deep; we give up
    if ( depth == 128 )
	return QString::null;

    do {
	part = part.left( slashPos );
	QFileInfo fileInfo( part );
	if ( fileInfo.isSymLink() ) {
	    foundLink = TRUE;
	    linkTarget = fileInfo.readLink();
	    break;
	}
    } while ( (slashPos = part.findRev('/')) != -1 );

    if ( foundLink ) {
	QString path2;
	if ( linkTarget[0] == '/' ) {
	    path2 = linkTarget;
	    if ( slashPos < (int) path.length() )
		path2 += "/" + path.right( path.length() - slashPos - 1 );
	} else {
	    QString relPath;
	    relPath = part.left( part.findRev('/') + 1 ) + linkTarget;
	    if ( slashPos < (int) path.length() ) {
		if ( !linkTarget.endsWith( "/" ) )
		    relPath += "/";
		relPath += path.right( path.length() - slashPos - 1 );
	    }
	    path2 = QDir::current().absFilePath( relPath );
	}
	path2 = QDir::cleanDirPath( path2 );
	return resolveSymlinks( path2, depth + 1 );
    } else {
	return path;
    }
}
#endif // Q_WS_WIN

/*!
    Returns the directory that contains the application executable.

    For example, if you have installed Qt in the \c{C:\Trolltech\Qt}
    directory, and you run the \c{demo} example, this function will
    return "C:/Trolltech/Qt/examples/demo".

    On Mac OS X this will point to the directory actually containing the
    executable, which may be inside of an application bundle (if the
    application is bundled).

    \warning On Unix, this function assumes that argv[0] contains the file
    name of the executable (which it normally does). It also assumes that
    the current directory hasn't been changed by the application.

    \sa applicationFilePath()
*/
QString QApplication::applicationDirPath()
{
    return QFileInfo( applicationFilePath() ).dirPath();
}

/*!
    Returns the file path of the application executable.

    For example, if you have installed Qt in the \c{C:\Trolltech\Qt}
    directory, and you run the \c{demo} example, this function will
    return "C:/Trolltech/Qt/examples/demo/demo.exe".

    \warning On Unix, this function assumes that argv[0] contains the file
    name of the executable (which it normally does). It also assumes that
    the current directory hasn't been changed by the application.

    \sa applicationDirPath()
*/
QString QApplication::applicationFilePath()
{
#if defined( Q_WS_WIN )
    QFileInfo filePath;
    QT_WA({
        WCHAR module_name[256];
        GetModuleFileNameW(0, module_name, sizeof(module_name)/sizeof(WCHAR));
        filePath = QString::fromUcs2((const unsigned short *)module_name);
    }, {
        char module_name[256];
        GetModuleFileNameA(0, module_name, sizeof(module_name));
        filePath = QString::fromLocal8Bit(module_name);
    });

    return filePath.filePath();
#elif defined( Q_WS_MAC )
    return QDir::cleanDirPath( QFile::decodeName( qAppFileName() ) );
#else
    QString argv0 = QFile::decodeName( argv()[0] );
    QString absPath;

    if ( argv0[0] == '/' ) {
	/*
	  If argv0 starts with a slash, it is already an absolute
	  file path.
	*/
	absPath = argv0;
    } else if ( argv0.find('/') != -1 ) {
	/*
	  If argv0 contains one or more slashes, it is a file path
	  relative to the current directory.
	*/
	absPath = QDir::current().absFilePath( argv0 );
    } else {
	/*
	  Otherwise, the file path has to be determined using the
	  PATH environment variable.
	*/
	char *pEnv = getenv( "PATH" );
	QStringList paths( QStringList::split(QChar(':'), pEnv) );
	QStringList::const_iterator p = paths.begin();
	while ( p != paths.end() ) {
	    QString candidate = QDir::current().absFilePath( *p + "/" + argv0 );
	    if ( QFile::exists(candidate) ) {
		absPath = candidate;
		break;
	    }
	    ++p;
	}
    }

    absPath = QDir::cleanDirPath( absPath );
    if ( QFile::exists(absPath) ) {
	return resolveSymlinks( absPath );
    } else {
	return QString::null;
    }
#endif
}
#endif // QT_NO_DIR

#ifndef QT_NO_COMPONENT

/*!
  Returns a list of paths that the application will search when
  dynamically loading libraries.
  The installation directory for plugins is the only entry if no
  paths have been set.  The default installation directory for plugins
  is \c INSTALL/plugins, where \c INSTALL is the directory where Qt was
  installed. The directory of the application executable (NOT the
  working directory) is also added to the plugin paths.

  If you want to iterate over the list, you should iterate over a
  copy, e.g.
    \code
    QStringList list = app.libraryPaths();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  See the \link plugins-howto.html plugins documentation\endlink for a
  description of how the library paths are used.

  \sa setLibraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
*/
QStringList QApplication::libraryPaths()
{
    if ( !app_libpaths ) {
	app_libpaths = new QStringList;
	QString installPathPlugins = QString::fromLocal8Bit(qInstallPathPlugins());
	if ( QFile::exists(installPathPlugins) ) {
#ifdef Q_WS_WIN
	    installPathPlugins.replace('\\', '/');
#endif
	    app_libpaths->append(installPathPlugins);
	}

	QString app_location;
	if (qApp)
	    app_location = qApp->applicationFilePath();
#ifdef Q_WS_WIN
	else {
	    app_location = QString(qAppFileName());
	    app_location.replace('\\', '/');
	}
#endif
	if (!app_location.isEmpty()) {
	    app_location.truncate( app_location.findRev( '/' ) );
	    if ( app_location != qInstallPathPlugins() && QFile::exists( app_location ) )
		app_libpaths->append( app_location );
	}
    }
    return *app_libpaths;
}


/*!
  Sets the list of directories to search when loading libraries to \a paths.
  All existing paths will be deleted and the path list will consist of the
  paths given in \a paths.

  \sa libraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
 */
void QApplication::setLibraryPaths( const QStringList &paths )
{
    delete app_libpaths;
    app_libpaths = new QStringList( paths );
}

/*!
  Append \a path to the end of the library path list. If \a path is
  empty or already in the path list, the path list is not changed.

  The default path list consists of a single entry, the installation
  directory for plugins.  The default installation directory for plugins
  is \c INSTALL/plugins, where \c INSTALL is the directory where Qt was
  installed.

  \sa removeLibraryPath(), libraryPaths(), setLibraryPaths()
 */
void QApplication::addLibraryPath( const QString &path )
{
    if ( path.isEmpty() )
	return;

    // make sure that library paths is initialized
    libraryPaths();

    if ( !app_libpaths->contains( path ) )
	app_libpaths->prepend( path );
}

/*!
  Removes \a path from the library path list. If \a path is empty or not
  in the path list, the list is not changed.

  \sa addLibraryPath(), libraryPaths(), setLibraryPaths()
*/
void QApplication::removeLibraryPath( const QString &path )
{
    if ( path.isEmpty() )
	return;

    // make sure that library paths is initialized
    libraryPaths();

    if ( app_libpaths->contains( path ) )
	app_libpaths->remove( path );
}
#endif //QT_NO_COMPONENT

/*!
  Returns the application palette.

  If a widget is passed in \a w, the default palette for the
  widget's class is returned. This may or may not be the application
  palette. In most cases there isn't a special palette for certain
  types of widgets, but one notable exception is the popup menu under
  Windows, if the user has defined a special background color for
  menus in the display settings.

  \sa setPalette(), QWidget::palette()
*/
#ifndef QT_NO_PALETTE
QPalette QApplication::palette(const QWidget* w)
{
#if defined(QT_CHECK_STATE)
    if ( !qApp )
	qWarning( "QApplication::palette: This function can only be "
		  "called after the QApplication object has been created" );
#endif
    if ( !app_pal ) {
	if ( !qt_std_pal )
	    qt_create_std_palette();
	app_pal = new QPalette( *qt_std_pal );
	qt_fix_tooltips();
    }

    if ( w && app_palettes ) {
	QPalette* wp = app_palettes->find( w->className() );
	if ( wp )
	    return *wp;
	QAsciiDictIterator<QPalette> it( *app_palettes );
	const char* name;
	while ( (name=it.currentKey()) != 0 ) {
	    if ( w->inherits( name ) )
		return *it.current();
	    ++it;
	}
    }
    return *app_pal;
}

/*!
  Changes the default application palette to \a palette. If \a
  informWidgets is TRUE, then existing widgets are informed about the
  change and may adjust themselves to the new application
  setting. If \a informWidgets is FALSE, the change only affects newly
  created widgets.

  If \a className is passed, the change applies only to widgets that
  inherit \a className (as reported by QObject::inherits()). If
  \a className is left 0, the change affects all widgets, thus overriding
  any previously set class specific palettes.

  The palette may be changed according to the current GUI style in
  QStyle::polish().

  \sa QWidget::setPalette(), palette(), QStyle::polish()
*/

void QApplication::setPalette( const QPalette &palette, bool informWidgets,
			       const char* className )
{
    QPalette pal = palette;
    QPalette *oldpal = 0;
#ifndef QT_NO_STYLE
    if ( !startingUp() ) // on startup this has been done already
	qApp->style().polish( pal );	// NB: non-const reference
#endif
    bool all = FALSE;
    if ( !className ) {
	if ( !app_pal ) {
	    app_pal = new QPalette( pal );
	    Q_CHECK_PTR( app_pal );
	} else {
	    *app_pal = pal;
	}
	all = app_palettes != 0;
	delete app_palettes;
	app_palettes = 0;
	qt_fix_tooltips();
    } else {
	if ( !app_palettes ) {
	    app_palettes = new QAsciiDict<QPalette>;
	    Q_CHECK_PTR( app_palettes );
	    app_palettes->setAutoDelete( TRUE );
	}
	oldpal = app_palettes->find( className );
	app_palettes->insert( className, new QPalette( pal ) );
    }
    if ( informWidgets && is_app_running && !is_app_closing ) {
	if ( !oldpal || ( *oldpal != pal ) ) {
	    QEvent e( QEvent::ApplicationPaletteChange );
	    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	    register QWidget *w;
	    while ( (w=it.current()) ) {		// for all widgets...
		++it;
		if ( all || (!className && w->isTopLevel() ) || w->inherits(className) ) // matching class
		    sendEvent( w, &e );
	    }
	}
    }
}

#endif // QT_NO_PALETTE

/*!
  Returns the default font for the widget \a w, or the default
  application font if \a w is 0.

  \sa setFont(), fontMetrics(), QWidget::font()
*/

QFont QApplication::font( const QWidget *w )
{
    if ( w && app_fonts ) {
	QFont* wf = app_fonts->find( w->className() );
	if ( wf )
	    return *wf;
	QAsciiDictIterator<QFont> it( *app_fonts );
	const char* name;
	while ( (name=it.currentKey()) != 0 ) {
	    if ( w->inherits( name ) )
		return *it.current();
	    ++it;
	}
    }
    if ( !app_font ) {
	app_font = new QFont( "Helvetica" );
	Q_CHECK_PTR( app_font );
    }
    return *app_font;
}

/*! Changes the default application font to \a font. If \a
  informWidgets is TRUE, then existing widgets are informed about the
  change and may adjust themselves to the new application
  setting. If \a informWidgets is FALSE, the change only affects newly
  created widgets. If \a className is passed, the change applies only
  to classes that inherit \a className (as reported by
  QObject::inherits()).

  On application start-up, the default font depends on the window
  system. It can vary depending on both the window system version and
  the locale. This function lets you override the default font; but
  overriding may be a bad idea because, for example, some locales need
  extra-large fonts to support their special characters.

  \sa font(), fontMetrics(), QWidget::setFont()
*/

void QApplication::setFont( const QFont &font, bool informWidgets,
			    const char* className )
{
    bool all = FALSE;
    if ( !className ) {
	qt_app_has_font = TRUE;
	if ( !app_font ) {
	    app_font = new QFont( font );
	    Q_CHECK_PTR( app_font );
	} else {
	    *app_font = font;
	}

	// make sure the application font is complete
	app_font->detach();
	app_font->d->mask = QFontPrivate::Complete;

	all = app_fonts != 0;
	delete app_fonts;
	app_fonts = 0;
    } else {
	if (!app_fonts){
	    app_fonts = new QAsciiDict<QFont>;
	    Q_CHECK_PTR( app_fonts );
	    app_fonts->setAutoDelete( TRUE );
	}
	QFont* fnt = new QFont(font);
	Q_CHECK_PTR( fnt );
	app_fonts->insert(className, fnt);
    }
    if ( informWidgets && is_app_running && !is_app_closing ) {
	QEvent e( QEvent::ApplicationFontChange );
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( all || (!className && w->isTopLevel() ) || w->inherits(className) ) // matching class
		sendEvent( w, &e );
	}
    }
}


/*!
  Initialization of the appearance of the widget \a w \e before it is first
  shown.

  Usually widgets call this automatically when they are polished. It
  may be used to do some style-based central customization of widgets.

  Note that you are not limited to the public functions of QWidget.
  Instead, based on meta information like QObject::className() you are
  able to customize any kind of widget.

  \sa QStyle::polish(), QWidget::polish(), setPalette(), setFont()
*/

void QApplication::polish( QWidget *w )
{
#ifndef QT_NO_STYLE
    w->style().polish( w );
#endif
}


/*!
  Returns a list of the top level widgets in the application.

  The list is created using \c new and must be deleted by the caller.

  The list is empty (QPtrList::isEmpty()) if there are no top level
  widgets.

  Note that some of the top level widgets may be hidden, for example
  the tooltip if no tooltip is currently shown.

  Example:
  \code
    // Show all hidden top level widgets.
    QWidgetList	 *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );	// iterate over the widgets
    QWidget * w;
    while ( (w=it.current()) != 0 ) {	// for each top level widget...
	++it;
	if ( !w->isVisible() )
	    w->show();
    }
    delete list;		// delete the list, not the widgets
  \endcode

  \warning Delete the list as soon you have finished using it.
  The widgets in the list may be deleted by someone else at any time.

  \sa allWidgets(), QWidget::isTopLevel(), QWidget::isVisible(),
      QPtrList::isEmpty()
*/

QWidgetList *QApplication::topLevelWidgets()
{
    return QWidget::tlwList();
}

/*!
  Returns a list of all the widgets in the application.

  The list is created using \c new and must be deleted by the caller.

  The list is empty (QPtrList::isEmpty()) if there are no widgets.

  Note that some of the widgets may be hidden.

  Example that updates all widgets:
  \code
    QWidgetList	 *list = QApplication::allWidgets();
    QWidgetListIt it( *list );         // iterate over the widgets
    QWidget * w;
    while ( (w=it.current()) != 0 ) {  // for each widget...
	++it;
	w->update();
    }
    delete list;                      // delete the list, not the widgets
  \endcode

  The QWidgetList class is defined in the \c qwidgetlist.h header
  file.

  \warning Delete the list as soon as you have finished using it.
  The widgets in the list may be deleted by someone else at any time.

  \sa topLevelWidgets(), QWidget::isVisible(), QPtrList::isEmpty(),
*/

QWidgetList *QApplication::allWidgets()
{
    return QWidget::wList();
}

/*!
  \fn QWidget *QApplication::focusWidget() const

  Returns the application widget that has the keyboard input focus, or
  0 if no widget in this application has the focus.

  \sa QWidget::setFocus(), QWidget::hasFocus(), activeWindow()
*/

/*!
  \fn QWidget *QApplication::activeWindow() const

  Returns the application top-level window that has the keyboard input
  focus, or 0 if no application window has the focus. Note that
  there might be an activeWindow() even if there is no focusWidget(),
  for example if no widget in that window accepts key events.

  \sa QWidget::setFocus(), QWidget::hasFocus(), focusWidget()
*/

/*!
  Returns display (screen) font metrics for the application font.

  \sa font(), setFont(), QWidget::fontMetrics(), QPainter::fontMetrics()
*/

QFontMetrics QApplication::fontMetrics()
{
    return desktop()->fontMetrics();
}



/*!
  Tells the application to exit with return code 0 (success).
  Equivalent to calling QApplication::exit( 0 ).

  It's common to connect the lastWindowClosed() signal to quit(), and
  you also often connect e.g. QButton::clicked() or signals in
  QAction, QPopupMenu or QMenuBar to it.

  Example:
  \code
    QPushButton *quitButton = new QPushButton( "Quit" );
    connect( quitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );
  \endcode

  \sa exit() aboutToQuit() lastWindowClosed() QAction
*/

void QApplication::quit()
{
    QApplication::exit( 0 );
}


/*!
  Closes all top-level windows.

  This function is particularly useful for applications with many
  top-level windows. It could, for example, be connected to a "Quit"
  entry in the file menu as shown in the following code example:

  \code
    // the "Quit" menu entry should try to close all windows
    QPopupMenu* file = new QPopupMenu( this );
    file->insertItem( "&Quit", qApp, SLOT(closeAllWindows()), CTRL+Key_Q );

    // when the last window is closed, the application should quit
    connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
  \endcode

  The windows are closed in random order, until one window does not
  accept the close event.

  \sa QWidget::close(), QWidget::closeEvent(), lastWindowClosed(),
  quit(), topLevelWidgets(), QWidget::isTopLevel()

 */
void QApplication::closeAllWindows()
{
    bool did_close = TRUE;
    QWidget *w;
    while((w = activeModalWidget()) && did_close) {
	if(w->isHidden())
	    break;
	did_close = w->close();
    }
    QWidgetList *list = QApplication::topLevelWidgets();
    for ( w = list->first(); did_close && w; ) {
	if ( !w->isHidden() ) {
	    did_close = w->close();
	    delete list;
	    list = QApplication::topLevelWidgets();
	    w = list->first();
	} else {
	    w = list->next();
	}
    }
    delete list;
}

/*!
    Displays a simple message box about Qt. The message includes the
    version number of Qt being used by the application.

    This is useful for inclusion in the Help menu of an application.
    See the examples/menu/menu.cpp example.

    This function is a convenience slot for QMessageBox::aboutQt().
*/
void QApplication::aboutQt()
{
#ifndef QT_NO_MESSAGEBOX
    QMessageBox::aboutQt( mainWidget() );
#endif // QT_NO_MESSAGEBOX
}


/*!
  \fn void QApplication::lastWindowClosed()

  This signal is emitted when the user has closed the last
  top level window.

  The signal is very useful when your application has many top level
  widgets but no main widget. You can then connect it to the quit()
  slot.

  For convenience, this signal is \e not emitted for transient top level
  widgets such as popup menus and dialogs.

  \sa mainWidget(), topLevelWidgets(), QWidget::isTopLevel(), QWidget::close()
*/

/*!
  \fn void QApplication::aboutToQuit()

  This signal is emitted when the application is about to quit the
  main event loop, e.g. when the event loop level drops to zero.
  This may happen either after a call to quit() from inside the
  application or when the users shuts down the entire desktop session.

  The signal is particularly useful if your application has to do some
  last-second cleanup. Note that no user interaction is possible in
  this state.

  \sa quit()
*/


/*!
  \fn void QApplication::guiThreadAwake()

  This signal is emitted after the event loop returns from a function
  that could block.

  \sa wakeUpGuiThread()
*/


/*!
  \fn bool QApplication::sendEvent( QObject *receiver, QEvent *event )

  Sends event \a event directly to receiver \a receiver, using the
  notify() function. Returns the value that was returned from the event
  handler.

    The event is \e not deleted when the event has been sent. The normal
    approach is to create the event on the stack, e.g.
    \code
    QMouseEvent me( QEvent::MouseButtonPress, pos, 0, 0 );
    QApplication::sendEvent( mainWindow, &me );
    \endcode
    If you create the event on the heap you must delete it.

  \sa postEvent(), notify()
*/

/*!
  Sends event \a e to \a receiver: \a {receiver}->event(\a e).
  Returns the value that is returned from the receiver's event handler.

  For certain types of events (e.g. mouse and key events),
  the event will be propagated to the receiver's parent and so on up to
  the top-level object if the receiver is not interested in the event
  (i.e., it returns FALSE).

  There are five different ways that events can be processed;
  reimplementing this virtual function is just one of them. All five
  approaches are listed below:
  \list 1
  \i Reimplementing this function. This is very powerful, providing
  complete control; but only one subclass can be qApp.

  \i Installing an event filter on qApp. Such an event filter is able
  to process all events for all widgets, so it's just as powerful as
  reimplementing notify(); furthermore, it's possible to have more
  than one application-global event filter. Global event filters even
  see mouse events for \link QWidget::isEnabled() disabled
  widgets, \endlink and if \link setGlobalMouseTracking() global mouse
  tracking \endlink is enabled, as well as mouse move events for all
  widgets.

  \i Reimplementing QObject::event() (as QWidget does). If you do
  this you get Tab key presses, and you get to see the events before
  any widget-specific event filters.

  \i Installing an event filter on the object. Such an event filter
  gets all the events except Tab and Shift-Tab key presses.

  \i Reimplementing paintEvent(), mousePressEvent() and so
  on. This is the commonest, easiest and least powerful way.
  \endlist

  \sa QObject::event(), installEventFilter()
*/

bool QApplication::notify( QObject *receiver, QEvent *e )
{
    // no events are delivered after ~QApplication() has started
    if ( is_app_closing )
	return FALSE;

    if ( receiver == 0 ) {			// serious error
#if defined(QT_CHECK_NULL)
	qWarning( "QApplication::notify: Unexpected null receiver" );
#endif
	return FALSE;
    }

    if ( e->type() == QEvent::ChildRemoved && receiver->postedEvents ) {

#ifdef QT_THREAD_SUPPORT
	QMutexLocker locker( postevent_mutex );
#endif // QT_THREAD_SUPPORT

	// the QObject destructor calls QObject::removeChild, which calls
	// QApplication::sendEvent() directly.  this can happen while the event
	// loop is in the middle of posting events, and when we get here, we may
	// not have any more posted events for this object.
	if ( receiver->postedEvents ) {
	    // if this is a child remove event and the child insert
	    // hasn't been dispatched yet, kill that insert
	    QPostEventList * l = receiver->postedEvents;
	    QObject * c = ((QChildEvent*)e)->child();
	    QPostEvent * pe;
	    l->first();
	    while( ( pe = l->current()) != 0 ) {
		if ( pe->event && pe->receiver == receiver &&
		     pe->event->type() == QEvent::ChildInserted &&
		     ((QChildEvent*)pe->event)->child() == c ) {
		    pe->event->posted = FALSE;
		    delete pe->event;
		    pe->event = 0;
		    l->remove();
		    continue;
		}
		l->next();
	    }
	}
    }

    bool res = FALSE;
    if ( !receiver->isWidgetType() )
	res = internalNotify( receiver, e );
    else switch ( e->type() ) {
#ifndef QT_NO_ACCEL
    case QEvent::Accel:
	{
	    QKeyEvent* key = (QKeyEvent*) e;
	    res = internalNotify( receiver, e );

	    if ( !res && !key->isAccepted() )
		res = qt_dispatchAccelEvent( (QWidget*)receiver, key );

	    // next lines are for compatibility with Qt <= 3.0.x: old
	    // QAccel was listening on toplevel widgets
	    if ( !res && !key->isAccepted() && !((QWidget*)receiver)->isTopLevel() )
		res = internalNotify( ((QWidget*)receiver)->topLevelWidget(), e );
	}
    break;
#endif //QT_NO_ACCEL
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::AccelOverride:
	{
	    QWidget* w = (QWidget*)receiver;
	    QKeyEvent* key = (QKeyEvent*) e;
#ifndef QT_NO_ACCEL
	    if ( qt_tryComposeUnicode( w, key ) )
		break;
#endif
	    bool def = key->isAccepted();
	    while ( w ) {
		if ( def )
		    key->accept();
		else
		    key->ignore();
		res = internalNotify( w, e );
		if ( res || key->isAccepted() )
		    break;
		w = w->parentWidget( TRUE );
	    }
	}
    break;
    case QEvent::MouseButtonPress:
	    if ( e->spontaneous() ) {
		QWidget* fw = (QWidget*)receiver;
		while ( fw->focusProxy() )
		    fw = fw->focusProxy();
		if ( fw->isEnabled() && fw->focusPolicy() & QWidget::ClickFocus ) {
		    QFocusEvent::setReason( QFocusEvent::Mouse);
		    fw->setFocus();
		    QFocusEvent::resetReason();
		}
	    }
	    // fall through intended
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
	{
	    QWidget* w = (QWidget*)receiver;
	    QMouseEvent* mouse = (QMouseEvent*) e;
	    QPoint relpos = mouse->pos();
	    while ( w ) {
		QMouseEvent me(mouse->type(), relpos, mouse->globalPos(), mouse->button(), mouse->state());
		me.spont = mouse->spontaneous();
		res = internalNotify( w, w == receiver ? mouse : &me );
		e->spont = FALSE;
		if (res || w->isTopLevel() || w->testWFlags(WNoMousePropagation))
		    break;

		relpos += w->pos();
		w = w->parentWidget();
	    }
	    if ( res )
		mouse->accept();
	    else
		mouse->ignore();
	}
    break;
#ifndef QT_NO_WHEELEVENT
    case QEvent::Wheel:
	{
	    if ( e->spontaneous() ) {
		QWidget* fw = (QWidget*)receiver;
		while ( fw->focusProxy() )
		    fw = fw->focusProxy();
		if ( fw->isEnabled() && (fw->focusPolicy() & QWidget::WheelFocus) == QWidget::WheelFocus ) {
		    QFocusEvent::setReason( QFocusEvent::Mouse);
		    fw->setFocus();
		    QFocusEvent::resetReason();
		}
	    }

	    QWidget* w = (QWidget*)receiver;
	    QWheelEvent* wheel = (QWheelEvent*) e;
	    QPoint relpos = wheel->pos();
	    while ( w ) {
		QWheelEvent we(relpos, wheel->globalPos(), wheel->delta(), wheel->state(), wheel->orientation());
		we.spont = wheel->spontaneous();
		res = internalNotify( w,  w == receiver ? wheel : &we );
		e->spont = FALSE;
		if (res || w->isTopLevel() || w->testWFlags(WNoMousePropagation))
		    break;

		relpos += w->pos();
		w = w->parentWidget();
	    }
	    if ( res )
		wheel->accept();
	    else
		wheel->ignore();
	}
    break;
#endif
    case QEvent::ContextMenu:
	{
	    QWidget* w = (QWidget*)receiver;
	    QContextMenuEvent *context = (QContextMenuEvent*) e;
	    QPoint relpos = context->pos();
	    while ( w ) {
		QContextMenuEvent ce(context->reason(), relpos, context->globalPos(), context->state());
		ce.spont = e->spontaneous();
		res = internalNotify( w,  w == receiver ? context : &ce );
		e->spont = FALSE;

		if (res || w->isTopLevel() || w->testWFlags(WNoMousePropagation))
		    break;

		relpos += w->pos();
		w = w->parentWidget();
	    }
	    if ( res )
		context->accept();
	    else
		context->ignore();
	}
    break;
#if defined (QT_TABLET_SUPPORT)
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
	{
	    QWidget *w = (QWidget*)receiver;
	    QTabletEvent *tablet = (QTabletEvent*)e;
	    QPoint relpos = tablet->pos();
	    while ( w ) {
		QTabletEvent te(tablet->pos(), tablet->globalPos(), tablet->device(),
				tablet->pressure(), tablet->xTilt(), tablet->yTilt(),
				tablet->uniqueId());
		te.spont = e->spontaneous();
		res = internalNotify( w, w == receiver ? tablet : &te );
		e->spont = FALSE;
		if (res || w->isTopLevel() || w->testWFlags(WNoMousePropagation))
		    break;

		relpos += w->pos();
		w = w->parentWidget();
	    }
	    if ( res )
		tablet->accept();
	    else
		tablet->ignore();
	    chokeMouse = tablet->isAccepted();
	}
    break;
#endif
    default:
	res = internalNotify( receiver, e );
	break;
    }

    return res;
}

/*!\reimp

*/
bool QApplication::event( QEvent *e )
{
    if(e->type() == QEvent::Close) {
	QCloseEvent *ce = (QCloseEvent*)e;
	ce->accept();
	closeAllWindows();

	QWidgetList *list = topLevelWidgets();
	for(QWidget *w = list->first(); w; w = list->next()) {
	    if ( !w->isHidden() && !w->isDesktop() && !w->isPopup() &&
		 (!w->isDialog() || !w->parentWidget())) {
		ce->ignore();
		break;
	    }
	}
	if(ce->isAccepted())
	    return TRUE;
    } else if (e->type() == QEvent::Quit) {
	quit();
	return TRUE;
    }
    return QObject::event(e);
}

/*!\internal

  Helper function called by notify()
 */
bool QApplication::internalNotify( QObject *receiver, QEvent * e)
{
    if ( eventFilters ) {
	QObjectListIt it( *eventFilters );
	register QObject *obj;
	while ( (obj=it.current()) != 0 ) {	// send to all filters
	    ++it;				//   until one returns TRUE
	    if ( obj->eventFilter(receiver,e) )
		return TRUE;
	}
    }

    bool consumed = FALSE;
    bool handled = FALSE;
    if ( receiver->isWidgetType() ) {
	QWidget *widget = (QWidget*)receiver;

	// toggle HasMouse widget state on enter and leave
	if ( e->type() == QEvent::Enter || e->type() == QEvent::DragEnter )
	    widget->setWState( WState_HasMouse );
	else if ( e->type() == QEvent::Leave || e->type() == QEvent::DragLeave )
	    widget->clearWState( WState_HasMouse );

	// throw away any mouse-tracking-only mouse events
	if ( e->type() == QEvent::MouseMove &&
	     (((QMouseEvent*)e)->state()&QMouseEvent::MouseButtonMask) == 0 &&
	     !widget->hasMouseTracking() ) {
	    handled = TRUE;
	    consumed = TRUE;
	} else if ( !widget->isEnabled() ) { // throw away mouse events to disabled widgets
	    switch(e->type()) {
	    case QEvent::MouseButtonPress:
	    case QEvent::MouseButtonRelease:
	    case QEvent::MouseButtonDblClick:
	    case QEvent::MouseMove:
		( (QMouseEvent*) e)->ignore();
		handled = TRUE;
		consumed = TRUE;
		break;
#ifndef QT_NO_DRAGANDDROP
	    case QEvent::DragEnter:
	    case QEvent::DragMove:
		( (QDragMoveEvent*) e)->ignore();
		handled = TRUE;
		break;

	    case QEvent::DragLeave:
	    case QEvent::DragResponse:
		handled = TRUE;
		break;

	    case QEvent::Drop:
		( (QDropEvent*) e)->ignore();
		handled = TRUE;
		break;
#endif
#ifndef QT_NO_WHEELEVENT
	    case QEvent::Wheel:
		( (QWheelEvent*) e)->ignore();
		handled = TRUE;
		break;
#endif
	    case QEvent::ContextMenu:
		( (QContextMenuEvent*) e)->ignore();
		handled = TRUE;
		break;
	    default:
		break;
	    }
	}

    }

    if (!handled)
	consumed = receiver->event( e );
    e->spont = FALSE;
    return consumed;
}

/*!
  Returns TRUE if an application object has not been created yet;
  otherwise returns FALSE.

  \sa closingDown()
*/

bool QApplication::startingUp()
{
    return !is_app_running;
}

/*!
  Returns TRUE if the application objects are being destroyed;
  otherwise returns FALSE.

  \sa startingUp()
*/

bool QApplication::closingDown()
{
    return is_app_closing;
}


/*!
    Processes pending events, for 3 seconds or until there are no more
    events to process, whichever is shorter.

    You can call this function occasionally when your program is busy
    performing a long operation (e.g. copying a file).

    \sa exec(), QTimer, QEventLoop::processEvents()
*/

void QApplication::processEvents()
{
    processEvents( 3000 );
}

/*!
    \overload

    Processes pending events for \a maxtime milliseconds or until
    there are no more events to process, whichever is shorter.

    You can call this function occasionally when you program is busy
    doing a long operation (e.g. copying a file).

    \sa exec(), QTimer, QEventLoop::processEvents()
*/
void QApplication::processEvents( int maxtime )
{
    eventLoop()->processEvents( QEventLoop::AllEvents, maxtime );
}

/*! \obsolete
  Waits for an event to occur, processes it, then returns.

  This function is useful for adapting Qt to situations where the
  event processing must be grafted onto existing program loops.

  Using this function in new applications may be an indication of design
  problems.

  \sa processEvents(), exec(), QTimer
*/

void QApplication::processOneEvent()
{
    eventLoop()->processEvents( QEventLoop::AllEvents |
				QEventLoop::WaitForMore );
}

/*****************************************************************************
  Main event loop wrappers
 *****************************************************************************/

/*!
    Returns the application event loop. This function will return
    zero if called during and after destroying QApplication.

    To create your own instance of QEventLoop or QEventLoop subclass create
    it before you create the QApplication object.

    \sa QEventLoop
*/
QEventLoop *QApplication::eventLoop()
{
    if ( !eventloop && !is_app_closing )
	(void) new QEventLoop( qApp, "default event loop" );
    return eventloop;
}


/*!
    Enters the main event loop and waits until exit() is called or the
    main widget is destroyed, and returns the value that was set to
    exit() (which is 0 if exit() is called via quit()).

    It is necessary to call this function to start event handling. The
    main event loop receives events from the window system and
    dispatches these to the application widgets.

    Generally speaking, no user interaction can take place before
    calling exec(). As a special case, modal widgets like QMessageBox
    can be used before calling exec(), because modal widgets call
    exec() to start a local event loop.

    To make your application perform idle processing, i.e. executing a
    special function whenever there are no pending events, use a
    QTimer with 0 timeout. More advanced idle processing schemes can
    be achieved using processEvents().

    \sa quit(), exit(), processEvents(), setMainWidget()
*/
int QApplication::exec()
{
    return eventLoop()->exec();
}

/*!
  Tells the application to exit with a return code.

  After this function has been called, the application leaves the main
  event loop and returns from the call to exec(). The exec() function
  returns \a retcode.

  By convention, a \a retcode of 0 means success, and any non-zero
  value indicates an error.

  Note that unlike the C library function of the same name, this
  function \e does return to the caller -- it is event processing that
  stops.

  \sa quit(), exec()
*/
void QApplication::exit( int retcode )
{
    qApp->eventLoop()->exit( retcode );
}

/*!
    \obsolete

    This function enters the main event loop (recursively). Do not call
    it unless you really know what you are doing.

    Use QApplication::eventLoop()->enterLoop() instead.

*/
int QApplication::enter_loop()
{
    return eventLoop()->enterLoop();
}

/*!
    \obsolete

    This function exits from a recursive call to the main event loop.
    Do not call it unless you are an expert.

    Use QApplication::eventLoop()->exitLoop() instead.

*/
void QApplication::exit_loop()
{
    eventLoop()->exitLoop();
}

/*!
    \obsolete

    Returns the current loop level.

    Use QApplication::eventLoop()->loopLevel() instead.

*/
int QApplication::loopLevel() const
{
    return eventLoop()->loopLevel();
}

/*!

  Wakes up the GUI thread.

  \sa guiThreadAwake() \link threads.html Thread Support in Qt\endlink
*/
void QApplication::wakeUpGuiThread()
{
    eventLoop()->wakeUp();
}

/*!
    This function returns TRUE if there are pending events; otherwise
    returns FALSE. Pending events can be either from the window system
    or posted events using QApplication::postEvent().
*/
bool QApplication::hasPendingEvents()
{
    return eventLoop()->hasPendingEvents();
}

#if !defined(Q_WS_X11)

// The doc and X implementation of these functions is in qapplication_x11.cpp

void QApplication::flushX()	{}		// do nothing

void QApplication::syncX()	{}		// do nothing

#endif

/*!
  \fn void QApplication::setWinStyleHighlightColor( const QColor & )
  \obsolete

  Sets the color used to mark selections in windows style for all widgets
  in the application. Will repaint all widgets if the color is changed.

  The default color is \c darkBlue.
  \sa winStyleHighlightColor()
*/

/*!
  \fn const QColor& QApplication::winStyleHighlightColor()
  \obsolete

  Returns the color used to mark selections in windows style.

  \sa setWinStyleHighlightColor()
*/

/*!
  Returns the version of the Windows operating system that is running:

  \list
  \i Qt::WV_95 - Windows 95
  \i Qt::WV_98 - Windows 98
  \i Qt::WV_Me - Windows Me
  \i Qt::WV_NT - Windows NT 4.x
  \i Qt::WV_2000 - Windows 2000 (NT5)
  \i Qt::WV_XP - Windows XP
  \i Qt::WV_2003 - Windows Server 2003 family
  \i Qt::WV_CE - Windows CE
  \i Qt::WV_CENET - Windows CE.NET
  \endlist

  Note that this function is implemented for the Windows version
  of Qt only.
*/

#if defined(Q_OS_CYGWIN) && !defined(Q_CYGWIN_WIN)
#include "qapplication_p.h"

Qt::WindowsVersion QApplication::winVersion()
{
    return qt_winver;
}
#endif

#ifndef QT_NO_TRANSLATION

bool qt_detectRTLLanguage()
{
    return QApplication::tr( "QT_LAYOUT_DIRECTION",
	    "Translate this string to the string 'LTR' in left-to-right"
	    " languages or to 'RTL' in right-to-left languages (such as Hebrew"
	    " and Arabic) to get proper widget layout." ) == "RTL";
}

/*!
  Adds the message file \a mf to the list of message files to be used
  for translations.

  Multiple message files can be installed. Translations are searched
  for in the last installed message file, then the one from last, and
  so on, back to the first installed message file. The search stops as
  soon as a matching translation is found.

  \sa removeTranslator() translate() QTranslator::load()
*/

void QApplication::installTranslator( QTranslator * mf )
{
    if ( !mf )
	return;
    if ( !translators )
	translators = new QValueList<QTranslator*>;

    translators->prepend( mf );

#ifndef QT_NO_TRANSLATION_BUILDER
    if ( mf->isEmpty() )
	return;
#endif

    // hook to set the layout direction of dialogs
    setReverseLayout( qt_detectRTLLanguage() );

    QWidgetList *list = topLevelWidgets();
    QWidgetListIt it( *list );
    QWidget *w;
    while ( ( w=it.current() ) != 0 ) {
	++it;
	if (!w->isDesktop())
	    postEvent( w, new QEvent( QEvent::LanguageChange ) );
    }
    delete list;
}

/*!
  Removes the message file \a mf from the list of message files used by
  this application. (It does not delete the message file from the file
  system.)

  \sa installTranslator() translate(), QObject::tr()
*/

void QApplication::removeTranslator( QTranslator * mf )
{
    if ( !translators || !mf )
	return;

    if ( translators->remove( mf ) && ! qApp->closingDown() ) {
	setReverseLayout( qt_detectRTLLanguage() );

	QWidgetList *list = topLevelWidgets();
	QWidgetListIt it( *list );
	QWidget *w;
	while ( ( w=it.current() ) != 0 ) {
	    ++it;
	    postEvent( w, new QEvent( QEvent::LanguageChange ) );
	}
	delete list;
    }
}

#ifndef QT_NO_TEXTCODEC
/*! \obsolete
  This is the same as QTextCodec::setCodecForTr().
*/
void QApplication::setDefaultCodec( QTextCodec* codec )
{
    QTextCodec::setCodecForTr( codec );
}

/*! \obsolete
  Returns QTextCodec::codecForTr().
*/
QTextCodec* QApplication::defaultCodec() const
{
    return QTextCodec::codecForTr();
}
#endif //QT_NO_TEXTCODEC

/*! \enum QApplication::Encoding

  This enum type defines the 8-bit encoding of character string
  arguments to translate():

  \value DefaultCodec - the encoding specified by
  QTextCodec::codecForTr() (Latin-1 if none has been set)
  \value UnicodeUTF8 - UTF-8

  \sa QObject::tr(), QObject::trUtf8(), QString::fromUtf8()
*/

/*! \reentrant
  Returns the translation text for \a sourceText, by querying the
  installed messages files. The message files are searched from the most
  recently installed message file back to the first installed message
  file.

  QObject::tr() and QObject::trUtf8() provide this functionality more
  conveniently.

  \a context is typically a class name (e.g., "MyDialog") and
  \a sourceText is either English text or a short identifying text, if
  the output text will be very long (as for help texts).

  \a comment is a disambiguating comment, for when the same \a
  sourceText is used in different roles within the same context. By
  default, it is null. \a encoding indicates the 8-bit encoding of
  character stings

  See the \l QTranslator documentation for more information about
  contexts and comments.

  If none of the message files contain a translation for \a
  sourceText in \a context, this function returns a QString
  equivalent of \a sourceText. The encoding of \a sourceText is
  specified by \e encoding; it defaults to \c DefaultCodec.

  This function is not virtual. You can use alternative translation
  techniques by subclassing \l QTranslator.

  \warning This method is reentrant only if all translators are
  installed \e before calling this method.  Installing or removing
  translators while performing translations is not supported.  Doing
  so will most likely result in crashes or other undesirable behavior.

  \sa QObject::tr() installTranslator() defaultCodec()
*/

QString QApplication::translate( const char * context, const char * sourceText,
				 const char * comment, Encoding encoding ) const
{
    if ( !sourceText )
	return QString::null;

    if ( translators ) {
	QValueList<QTranslator*>::iterator it;
	QTranslator * mf;
	QString result;
	for ( it = translators->begin(); it != translators->end(); ++it ) {
	    mf = *it;
	    result = mf->findMessage( context, sourceText, comment ).translation();
	    if ( !result.isNull() )
		return result;
	}
    }
#ifndef QT_NO_TEXTCODEC
    if ( encoding == UnicodeUTF8 )
	return QString::fromUtf8( sourceText );
    else if ( QTextCodec::codecForTr() != 0 )
	return QTextCodec::codecForTr()->toUnicode( sourceText );
    else
#endif
	return QString::fromLatin1( sourceText );
}

#endif

/*****************************************************************************
  QApplication management of posted events
 *****************************************************************************/

//see also notify(), which does the removal of ChildInserted when ChildRemoved.

/*!
  Adds the event \a event with the object \a receiver as the receiver of the
  event, to an event queue and returns immediately.

  The event must be allocated on the heap since the post event queue
  will take ownership of the event and delete it once it has been posted.

  When control returns to the main event loop, all events that are
  stored in the queue will be sent using the notify() function.

  \threadsafe

  \sa sendEvent(), notify()
*/

void QApplication::postEvent( QObject *receiver, QEvent *event )
{
    if ( receiver == 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QApplication::postEvent: Unexpected null receiver" );
#endif
	delete event;
	return;
    }

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( postevent_mutex );
#endif // QT_THREAD_SUPPORT

    if ( !globalPostedEvents ) {			// create list
	globalPostedEvents = new QPostEventList;
	Q_CHECK_PTR( globalPostedEvents );
	globalPostedEvents->setAutoDelete( TRUE );
	qapp_cleanup_events.set( &globalPostedEvents );
    }

    if ( !receiver->postedEvents )
	receiver->postedEvents = new QPostEventList;
    QPostEventList * l = receiver->postedEvents;

    // if this is one of the compressible events, do compression
    if ( event->type() == QEvent::Paint ||
	 event->type() == QEvent::LayoutHint ||
	 event->type() == QEvent::Resize ||
	 event->type() == QEvent::Move ||
	 event->type() == QEvent::LanguageChange ) {
	l->first();
	QPostEvent * cur = 0;
	for ( ;; ) {
	    while ( (cur=l->current()) != 0 &&
		    ( cur->receiver != receiver ||
		      cur->event == 0 ||
		      cur->event->type() != event->type() ) )
		l->next();
	    if ( l->current() != 0 ) {
		if ( cur->event->type() == QEvent::Paint ) {
		    QPaintEvent * p = (QPaintEvent*)(cur->event);
		    if ( p->erase != ((QPaintEvent*)event)->erase ) {
			l->next();
			continue;
		    }
		    p->reg = p->reg.unite( ((QPaintEvent *)event)->reg );
		    p->rec = p->rec.unite( ((QPaintEvent *)event)->rec );
		    delete event;
		    return;
		} else if ( cur->event->type() == QEvent::LayoutHint ) {
		    delete event;
		    return;
		} else if ( cur->event->type() == QEvent::Resize ) {
		    ((QResizeEvent *)(cur->event))->s = ((QResizeEvent *)event)->s;
		    delete event;
		    return;
		} else if ( cur->event->type() == QEvent::Move ) {
		    ((QMoveEvent *)(cur->event))->p = ((QMoveEvent *)event)->p;
		    delete event;
		    return;
		} else if ( cur->event->type() == QEvent::LanguageChange ) {
		    delete event;
		    return;
		}
	    }
	    break;
	};
    }

    // if no compression could be done, just append something
    event->posted = TRUE;
    QPostEvent * pe = new QPostEvent( receiver, event );
    l->append( pe );
    globalPostedEvents->append( pe );

    if (eventloop)
	eventloop->wakeUp();
}


/*! \overload

    Dispatches all posted events, i.e. empties the event queue.
*/
void QApplication::sendPostedEvents()
{
    sendPostedEvents( 0, 0 );
}



/*!
  Immediately dispatches all events which have been previously queued
  with QApplication::postEvent() and which are for the object \a receiver
  and have the event type \a event_type.

  Note that events from the window system are \e not dispatched by this
  function, but by processEvents().

  If \a receiver is null, the events of \a event_type are sent for all
  objects. If \a event_type is 0, all the events are sent for \a receiver.
*/

void QApplication::sendPostedEvents( QObject *receiver, int event_type )
{
    // Make sure the object hierarchy is stable before processing events
    // to avoid endless loops
    if ( receiver == 0 && event_type == 0 )
	sendPostedEvents( 0, QEvent::ChildInserted );

    if ( !globalPostedEvents || ( receiver && !receiver->postedEvents ) )
	return;

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( postevent_mutex );
#endif

    bool sent = TRUE;
    while ( sent ) {
	sent = FALSE;

	if ( !globalPostedEvents || ( receiver && !receiver->postedEvents ) )
	    return;

	// if we have a receiver, use the local list. Otherwise, use the
	// global list
	QPostEventList * l = receiver ? receiver->postedEvents : globalPostedEvents;

	// okay. here is the tricky loop. be careful about optimizing
	// this, it looks the way it does for good reasons.
	QPostEventListIt it( *l );
	QPostEvent *pe;
	while ( (pe=it.current()) != 0 ) {
	    ++it;
	    if ( pe->event // hasn't been sent yet
		 && ( receiver == 0 // we send to all receivers
		      || receiver == pe->receiver ) // we send to THAT receiver
		 && ( event_type == 0 // we send all types
		      || event_type == pe->event->type() ) ) { // we send THAT type
		// first, we diddle the event so that we can deliver
		// it, and that noone will try to touch it later.
		pe->event->posted = FALSE;
		QEvent * e = pe->event;
		QObject * r = pe->receiver;
		pe->event = 0;

		// next, update the data structure so that we're ready
		// for the next event.

		// look for the local list, and take whatever we're
		// delivering out of it. r->postedEvents maybe *l
		if ( r->postedEvents ) {
		    r->postedEvents->removeRef( pe );
		    // if possible, get rid of that list. this is not
		    // ideal - we will create and delete a list for
		    // each update() call. it would be better if we'd
		    // leave the list empty here, and delete it
		    // somewhere else if it isn't being used.
		    if ( r->postedEvents->isEmpty() ) {
			delete r->postedEvents;
			r->postedEvents = 0;
		    }
		}

#ifdef QT_THREAD_SUPPORT
		if ( locker.mutex() ) locker.mutex()->unlock();
#endif // QT_THREAD_SUPPORT
		// after all that work, it's time to deliver the event.
		if ( e->type() == QEvent::Paint && r->isWidgetType() ) {
		    QWidget * w = (QWidget*)r;
		    QPaintEvent * p = (QPaintEvent*)e;
		    if ( w->isVisible() )
			w->repaint( p->reg, p->erase );
		} else {
		    sent = TRUE;
		    QApplication::sendEvent( r, e );
		}
#ifdef QT_THREAD_SUPPORT
		if ( locker.mutex() ) locker.mutex()->lock();
#endif // QT_THREAD_SUPPORT

		delete e;
		// careful when adding anything below this point - the
		// sendEvent() call might invalidate any invariants this
		// function depends on.
	    }
	}

	// clear the global list, i.e. remove everything that was
	// delivered.
	if ( l == globalPostedEvents ) {
	    globalPostedEvents->first();
	    while( (pe=globalPostedEvents->current()) != 0 ) {
		if ( pe->event )
		    globalPostedEvents->next();
		else
		    globalPostedEvents->remove();
	    }
	}
    }
}

/*!
  Removes all events posted using postEvent() for \a receiver.

  The events are \e not dispatched, instead they are removed from the
  queue. You should never need to call this function. If you do call it,
  be aware that killing events may cause \a receiver to break one or
  more invariants.

  \threadsafe
*/

void QApplication::removePostedEvents( QObject *receiver )
{
    if ( !receiver )
	return;

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( postevent_mutex );
#endif // QT_THREAD_SUPPORT

    // the QObject destructor calls this function directly.  this can
    // happen while the event loop is in the middle of posting events,
    // and when we get here, we may not have any more posted events
    // for this object.
    if ( !receiver->postedEvents )
	return;

    // iterate over the object-specifc list and delete the events.
    // leave the QPostEvent objects; they'll be deleted by
    // sendPostedEvents().
    QPostEventList * l = receiver->postedEvents;
    receiver->postedEvents = 0;
    l->first();
    QPostEvent * pe;
    while( (pe=l->current()) != 0 ) {
	if ( pe->event ) {
	    pe->event->posted = FALSE;
	    delete pe->event;
	    pe->event = 0;
	}
	l->remove();
    }
    delete l;
}


/*!
  Removes \a event from the queue of posted events, and emits a
  warning message if appropriate.

  \warning This function can be \e really slow. Avoid using it, if
  possible.

  \threadsafe
*/

void QApplication::removePostedEvent( QEvent *  event )
{
    if ( !event || !event->posted )
	return;

    if ( !globalPostedEvents ) {
#if defined(QT_DEBUG)
	qDebug( "QApplication::removePostedEvent: %p %d is posted: impossible",
		(void*)event, event->type() );
	return;
#endif
    }

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( postevent_mutex );
#endif // QT_THREAD_SUPPORT

    QPostEventListIt it( *globalPostedEvents );
    QPostEvent * pe;
    while( (pe = it.current()) != 0 ) {
	++it;
	if ( pe->event == event ) {
#if defined(QT_DEBUG)
	    const char *n;
	    switch ( event->type() ) {
	    case QEvent::Timer:
		n = "Timer";
		break;
	    case QEvent::MouseButtonPress:
		n = "MouseButtonPress";
		break;
	    case QEvent::MouseButtonRelease:
		n = "MouseButtonRelease";
		break;
	    case QEvent::MouseButtonDblClick:
		n = "MouseButtonDblClick";
		break;
	    case QEvent::MouseMove:
		n = "MouseMove";
		break;
#ifndef QT_NO_WHEELEVENT
	    case QEvent::Wheel:
		n = "Wheel";
		break;
#endif
	    case QEvent::KeyPress:
		n = "KeyPress";
		break;
	    case QEvent::KeyRelease:
		n = "KeyRelease";
		break;
	    case QEvent::FocusIn:
		n = "FocusIn";
		break;
	    case QEvent::FocusOut:
		n = "FocusOut";
		break;
	    case QEvent::Enter:
		n = "Enter";
		break;
	    case QEvent::Leave:
		n = "Leave";
		break;
	    case QEvent::Paint:
		n = "Paint";
		break;
	    case QEvent::Move:
		n = "Move";
		break;
	    case QEvent::Resize:
		n = "Resize";
		break;
	    case QEvent::Create:
		n = "Create";
		break;
	    case QEvent::Destroy:
		n = "Destroy";
		break;
	    case QEvent::Close:
		n = "Close";
		break;
	    case QEvent::Quit:
		n = "Quit";
		break;
	    default:
		n = "<other>";
		break;
	    }
	    qWarning("QEvent: Warning: %s event deleted while posted to %s %s",
		     n,
		     pe->receiver ? pe->receiver->className() : "null",
		     pe->receiver ? pe->receiver->name() : "object" );
	    // note the beautiful uglehack if !pe->receiver :)
#endif
	    event->posted = FALSE;
	    delete pe->event;
	    pe->event = 0;
	    return;
	}
    }
}

/*!\internal

  Sets the active window in reaction to a system event. Call this
  from the platform specific event handlers.

  It sets the activeWindow() and focusWidget() attributes and sends
  proper WindowActivate/WindowDeactivate and FocusIn/FocusOut events
  to all appropriate widgets.

  \sa activeWindow()
 */
void QApplication::setActiveWindow( QWidget* act )
{
    QWidget* window = act?act->topLevelWidget():0;

    if ( active_window == window )
	return;

    // first the activation/deactivation events
    if ( active_window ) {
	QWidgetList deacts;
#ifndef QT_NO_STYLE
	if ( style().styleHint(QStyle::SH_Widget_ShareActivation, active_window ) ) {
	    QWidgetList *list = topLevelWidgets();
	    if ( list ) {
		for ( QWidget *w = list->first(); w; w = list->next() ) {
		    if ( w->isVisible() && w->isActiveWindow() )
			deacts.append(w);
		}
		delete list;
	    }
	} else
#endif
	    deacts.append(active_window);
	active_window = 0;
	QEvent e( QEvent::WindowDeactivate );
	for(QWidget *w = deacts.first(); w; w = deacts.next())
	    QApplication::sendSpontaneousEvent( w, &e );
    }

    active_window = window;
    if ( active_window ) {
	QEvent e( QEvent::WindowActivate );
	QWidgetList acts;
#ifndef QT_NO_STYLE
	if ( style().styleHint(QStyle::SH_Widget_ShareActivation, active_window ) ) {
	    QWidgetList *list = topLevelWidgets();
	    if ( list ) {
		for ( QWidget *w = list->first(); w; w = list->next() ) {
		    if ( w->isVisible() && w->isActiveWindow() )
			acts.append(w);
		}
		delete list;
	    }
	} else
#endif
	    acts.append(active_window);
	for(QWidget *w = acts.first(); w; w = acts.next())
	    QApplication::sendSpontaneousEvent( w, &e );
    }

    // then focus events
    QFocusEvent::setReason( QFocusEvent::ActiveWindow );
    if ( !active_window && focus_widget ) {
	QFocusEvent out( QEvent::FocusOut );
	QWidget *tmp = focus_widget;
	focus_widget = 0;
#ifdef Q_WS_WIN
	QInputContext::accept( tmp );
#endif
	QApplication::sendSpontaneousEvent( tmp, &out );
    } else if ( active_window ) {
	QWidget *w = active_window->focusWidget();
	if ( w && w->focusPolicy() != QWidget::NoFocus )
	    w->setFocus();
	else
	    active_window->focusNextPrevChild( TRUE );
    }
    QFocusEvent::resetReason();
}


/*!\internal

  Creates the proper Enter/Leave event when widget \a enter is entered
  and widget \a leave is left.
 */
Q_EXPORT void qt_dispatchEnterLeave( QWidget* enter, QWidget* leave ) {
#if 0
    if ( leave ) {
	QEvent e( QEvent::Leave );
	QApplication::sendEvent( leave, & e );
    }
    if ( enter ) {
	QEvent e( QEvent::Enter );
	QApplication::sendEvent( enter, & e );
    }
    return;
#endif

    QWidget* w ;
    if ( !enter && !leave )
	return;
    QWidgetList leaveList;
    QWidgetList enterList;

    bool sameWindow = leave && enter && leave->topLevelWidget() == enter->topLevelWidget();
    if ( leave && !sameWindow ) {
	w = leave;
	do {
	    leaveList.append( w );
	} while ( (w = w->parentWidget( TRUE ) ) );
    }
    if ( enter && !sameWindow ) {
	w = enter;
	do {
	    enterList.prepend( w );
	} while ( (w = w->parentWidget(TRUE) ) );
    }
    if ( sameWindow ) {
	int enterDepth = 0;
	int leaveDepth = 0;
	w = enter;
	while ( ( w = w->parentWidget( TRUE ) ) )
	    enterDepth++;
	w = leave;
	while ( ( w = w->parentWidget( TRUE ) ) )
	    leaveDepth++;
	QWidget* wenter = enter;
	QWidget* wleave = leave;
	while ( enterDepth > leaveDepth ) {
	    wenter = wenter->parentWidget();
	    enterDepth--;
	}
	while ( leaveDepth > enterDepth ) {
	    wleave = wleave->parentWidget();
	    leaveDepth--;
	}
	while ( !wenter->isTopLevel() && wenter != wleave ) {
	    wenter = wenter->parentWidget();
	    wleave = wleave->parentWidget();
	}

	w = leave;
	while ( w != wleave ) {
	    leaveList.append( w );
	    w = w->parentWidget();
	}
	w = enter;
	while ( w != wenter ) {
	    enterList.prepend( w );
	    w = w->parentWidget();
	}
    }

    QEvent leaveEvent( QEvent::Leave );
    for ( w = leaveList.first(); w; w = leaveList.next() ) {
	if ( !qApp->activeModalWidget() || qt_tryModalHelper( w, 0 ))
	    QApplication::sendEvent( w, &leaveEvent );
    }
    QEvent enterEvent( QEvent::Enter );
    for ( w = enterList.first(); w; w = enterList.next() ) {
	if ( !qApp->activeModalWidget() || qt_tryModalHelper( w, 0 ))
	    QApplication::sendEvent( w, &enterEvent );
    }
}


#ifdef Q_WS_MACX
extern QWidget *qt_tryModalHelperMac( QWidget * top ); //qapplication_mac.cpp
#endif


/*!\internal

  Called from qapplication_<platform>.cpp, returns TRUE
  if the widget should accept the event.
 */
Q_EXPORT bool qt_tryModalHelper( QWidget *widget, QWidget **rettop ) {
    QWidget *modal=0, *top=QApplication::activeModalWidget();
    if ( rettop ) *rettop = top;

    if ( qApp->activePopupWidget() )
	return TRUE;

#ifdef Q_WS_MACX
    top = qt_tryModalHelperMac( top );
    if ( rettop ) *rettop = top;
#endif

    QWidget* groupLeader = widget;
    widget = widget->topLevelWidget();

    if ( widget->testWFlags(Qt::WShowModal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )			// don't block event
	return TRUE;

    QWidget * p = widget->parentWidget(); // Check if the active modal widget is a parent of our widget
    while ( p ) {
	if ( p == top )
	    return TRUE;
	p = p->parentWidget();
    }

    while ( groupLeader && !groupLeader->testWFlags( Qt::WGroupLeader ) )
	groupLeader = groupLeader->parentWidget();

    if ( groupLeader ) {
	// Does groupLeader have a child in qt_modal_stack?
	bool unrelated = TRUE;
	modal = qt_modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != groupLeader && !p->testWFlags( Qt::WGroupLeader) ) {
		p = p->parentWidget();
	    }
	    modal = qt_modal_stack->next();
	    if ( p == groupLeader ) unrelated = FALSE;
	}

	if ( unrelated )
	    return TRUE;		// don't block event
    }
    return FALSE;
}


/*!
  Returns the desktop widget (also called the root window).

  The desktop widget is useful for obtaining the size of the screen.
  It may also be possible to draw on the desktop. We recommend against
  assuming that it's possible to draw on the desktop, since this does
  not work on all operating systems.

  \code
    QDesktopWidget *d = QApplication::desktop();
    int w = d->width();	    // returns desktop width
    int h = d->height();    // returns desktop height
  \endcode
*/

QDesktopWidget *QApplication::desktop()
{
    if ( !qt_desktopWidget || // not created yet
	 !qt_desktopWidget->isDesktop() ) { // reparented away
	qt_desktopWidget = new QDesktopWidget();
	Q_CHECK_PTR( qt_desktopWidget );
    }
    return qt_desktopWidget;
}

#ifndef QT_NO_CLIPBOARD
/*!
  Returns a pointer to the application global clipboard.
*/
QClipboard *QApplication::clipboard()
{
    if ( qt_clipboard == 0 ) {
	qt_clipboard = new QClipboard;
	Q_CHECK_PTR( qt_clipboard );
    }
    return qt_clipboard;
}
#endif // QT_NO_CLIPBOARD

/*!
  By default, Qt will try to use the current standard colors, fonts
  etc., from the underlying window system's desktop settings,
  and use them for all relevant widgets. This behavior can be switched off
  by calling this function with \a on set to FALSE.

  This static function must be called before creating the QApplication
  object, like this:

  \code
  int main( int argc, char** argv ) {
    QApplication::setDesktopSettingsAware( FALSE ); // I know better than the user
    QApplication myApp( argc, argv ); // Use default fonts & colors
    ...
  }
  \endcode

  \sa desktopSettingsAware()
*/

void QApplication::setDesktopSettingsAware( bool on )
{
    obey_desktop_settings = on;
}

/*!
  Returns the value set by setDesktopSettingsAware(); by default TRUE.

  \sa setDesktopSettingsAware()
*/

bool QApplication::desktopSettingsAware()
{
    return obey_desktop_settings;
}

/*! \fn void QApplication::lock()

  Lock the Qt Library Mutex. If another thread has already locked the
  mutex, the calling thread will block until the other thread has
  unlocked the mutex.

  \sa unlock() locked() \link threads.html Thread Support in Qt\endlink
*/


/*! \fn void QApplication::unlock(bool wakeUpGui)

  Unlock the Qt Library Mutex. If \a wakeUpGui is TRUE (the default),
  then the GUI thread will be woken with QApplication::wakeUpGuiThread().

  \sa lock(), locked() \link threads.html Thread Support in Qt\endlink
*/


/*! \fn bool QApplication::locked()

  Returns TRUE if the Qt Library Mutex is locked by a different thread;
  otherwise returns FALSE.

  \warning Due to different implementations of recursive mutexes on
  the supported platforms, calling this function from the same thread
  that previously locked the mutex will give undefined results.

  \sa lock() unlock() \link threads.html Thread Support in Qt\endlink
*/

/*! \fn bool QApplication::tryLock()

  Attempts to lock the Qt Library Mutex, and returns immediately. If
  the lock was obtained, this function returns TRUE. If another thread
  has locked the mutex, this function returns FALSE, instead of
  waiting for the lock to become available.

  The mutex must be unlocked with unlock() before another thread can
  successfully lock it.

  \sa lock(), unlock() \link threads.html Thread Support in Qt\endlink
*/

#if defined(QT_THREAD_SUPPORT)
void QApplication::lock()
{
    qt_mutex->lock();
}

void QApplication::unlock(bool wakeUpGui)
{
    qt_mutex->unlock();

    if (wakeUpGui)
	wakeUpGuiThread();
}

bool QApplication::locked()
{
    return qt_mutex->locked();
}

bool QApplication::tryLock()
{
    return qt_mutex->tryLock();
}
#endif


/*!
  \fn bool QApplication::isSessionRestored() const

  Returns TRUE if the application has been restored from an earlier
  \link session.html session\endlink; otherwise returns FALSE.

  \sa sessionId(), commitData(), saveState()
*/


/*!
  \fn QString QApplication::sessionId() const

  Returns the current \link session.html session's\endlink identifier.

  If the application has been restored from an earlier session, this
  identifier is the same as it was in that previous session.

  The session identifier is guaranteed to be unique both for different
  applications and for different instances of the same application.

  \sa isSessionRestored(), sessionKey(), commitData(), saveState()
 */

/*!
  \fn QString QApplication::sessionKey() const

  Returns the session key in the current \link session.html
  session\endlink.

  If the application has been restored from an earlier session, this
  key is the same as it was when the previous session ended.

  The session key changes with every call of commitData() or
  saveState().

  \sa isSessionRestored(), sessionId(), commitData(), saveState()
 */


/*!
  \fn void QApplication::commitData( QSessionManager& sm )

  This function deals with \link session.html session
  management\endlink. It is invoked when the QSessionManager wants the
  application to commit all its data.

  Usually this means saving all open files, after getting
  permission from the user. Furthermore you may want to provide a means
  by which the user can cancel the shutdown.

  Note that you should not exit the application within this function.
  Instead, the session manager may or may not do this afterwards,
  depending on the context.

  \warning Within this function, no user interaction is possible, \e
  unless you ask the session manager \a sm for explicit permission.
  See QSessionManager::allowsInteraction() and
  QSessionManager::allowsErrorInteraction() for details and example
  usage.

  The default implementation requests interaction and sends a close
  event to all visible top level widgets. If any event was
  rejected, the shutdown is canceled.

  \sa isSessionRestored(), sessionId(), saveState(), \link session.html the Session Management overview\endlink
*/
#ifndef QT_NO_SESSIONMANAGER
void QApplication::commitData( QSessionManager& sm  )
{

    if ( sm.allowsInteraction() ) {
	QWidgetList done;
	QWidgetList *list = QApplication::topLevelWidgets();
	bool cancelled = FALSE;
	QWidget* w = list->first();
	while ( !cancelled && w ) {
	    if ( !w->isHidden() ) {
		QCloseEvent e;
		sendEvent( w, &e );
		cancelled = !e.isAccepted();
		if ( !cancelled )
		    done.append( w );
		delete list; // one never knows...
		list = QApplication::topLevelWidgets();
		w = list->first();
	    } else {
		w = list->next();
	    }
	    while ( w && done.containsRef( w ) )
		w = list->next();
	}
	delete list;
	if ( cancelled )
	    sm.cancel();
    }
}


/*!
  \fn void QApplication::saveState( QSessionManager& sm )

  This function deals with \link session.html session
  management\endlink. It is invoked when the
  \link QSessionManager session manager \endlink wants the application
  to preserve its state for a future session.

  For example, a text editor would create a temporary file that
  includes the current contents of its edit buffers, the location of
  the cursor and other aspects of the current editing session.

  Note that you should never exit the application within this
  function. Instead, the session manager may or may not do this
  afterwards, depending on the context. Futhermore, most session
  managers will very likely request a saved state immediately after
  the application has been started. This permits the session manager
  to learn about the application's restart policy.

  \warning Within this function, no user interaction is possible, \e
  unless you ask the session manager \a sm for explicit permission.
  See QSessionManager::allowsInteraction() and
  QSessionManager::allowsErrorInteraction() for details.

  \sa isSessionRestored(), sessionId(), commitData(), \link session.html the Session Management overview\endlink
*/

void QApplication::saveState( QSessionManager& /* sm */ )
{
}
#endif //QT_NO_SESSIONMANAGER
/*!
  Sets the time after which a drag should start to \a ms ms.

  \sa startDragTime()
*/

void QApplication::setStartDragTime( int ms )
{
    drag_time = ms;
}

/*!
  If you support drag and drop in you application and a drag should
  start after a mouse click and after a certain time elapsed, you
  should use the value which this method returns as the delay (in ms).

  Qt also uses this delay internally, e.g. in QTextEdit and QLineEdit,
  for starting a drag.

  The default value is 500 ms.

  \sa setStartDragTime(), startDragDistance()
*/

int QApplication::startDragTime()
{
    return drag_time;
}

/*!
  Sets the distance after which a drag should start to \a l pixels.

  \sa startDragDistance()
*/

void QApplication::setStartDragDistance( int l )
{
    drag_distance = l;
}

/*!
  If you support drag and drop in you application and a drag should
  start after a mouse click and after moving the mouse a certain
  distance, you should use the value which this method returns as the
  distance.

  For example, if the mouse position of the click is stored in \c
  startPos and the current position (e.g. in the mouse move event) is
  \c currPos, you can find out if a drag should be started with code
  like this:
  \code
  if ( ( startPos - currPos ).manhattanLength() >
       QApplication::startDragDistance() )
    startTheDrag();
  \endcode

  Qt uses this value internally, e.g. in QFileDialog.

  The default value is 4 pixels.

  \sa setStartDragDistance(), startDragTime(), QPoint::manhattanLength()
*/

int QApplication::startDragDistance()
{
    return drag_distance;
}

/*!
  If \a b is TRUE, all dialogs and widgets will be laid out in a
  mirrored fashion, as required by right to left languages such as
  Arabic and Hebrew. If \a b is FALSE, dialogs and widgets are laid
  out left to right.

  Changing this flag in runtime does not cause a relayout of already
  instantiated widgets.

  \sa reverseLayout()
*/
void QApplication::setReverseLayout( bool b )
{
    if ( reverse_layout == b )
	return;

    reverse_layout = b;

    QWidgetList *list = topLevelWidgets();
    QWidgetListIt it( *list );
    QWidget *w;
    while ( ( w=it.current() ) != 0 ) {
	++it;
	postEvent( w, new QEvent( QEvent::LayoutDirectionChange ) );
    }
    delete list;
}

/*!
    Returns TRUE if all dialogs and widgets will be laid out in a
    mirrored (right to left) fashion. Returns FALSE if dialogs and
    widgets will be laid out left to right.

  \sa setReverseLayout()
*/
bool QApplication::reverseLayout()
{
    return reverse_layout;
}


/*!
  \class QSessionManager qsessionmanager.h
  \brief The QSessionManager class provides access to the session manager.

  \ingroup application
  \ingroup environment

  The session manager is responsible for session management, most
  importantly for interruption and resumption. A "session" is a kind
  of record of the state of the system, e.g. which applications were
  run at start up and which applications are currently running. The
  session manager is used to save the session, e.g. when the machine
  is shut down; and to restore a session, e.g. when the machine is
  started up. Use QSettings to save and restore an individual
  application's settings, e.g. window positions, recently used files,
  etc.

  QSessionManager provides an interface between the application and
  the session manager so that the program can work well with the
  session manager. In Qt, session management requests for action
  are handled by the two virtual functions QApplication::commitData()
  and QApplication::saveState(). Both provide a reference to
  a session manager object as argument, to allow the application
  to communicate with the session manager.

  During a session management action (i.e. within commitData() and
  saveState()), no user interaction is possible \e unless the
  application got explicit permission from the session manager. You
  ask for permission by calling allowsInteraction() or, if it's really
  urgent, allowsErrorInteraction(). Qt does not enforce this, but the
  session manager may.

  You can try to abort the shutdown process by calling cancel(). The
  default commitData() function does this if some top-level window
  rejected its closeEvent().

  For sophisticated session managers provided on Unix/X11, QSessionManager
  offers further possibilites to fine-tune an application's session
  management behavior: setRestartCommand(), setDiscardCommand(),
  setRestartHint(), setProperty(), requestPhase2(). See the respective
  function descriptions for further details.
*/

/*! \enum QSessionManager::RestartHint

  This enum type defines the circumstances under which this
  application wants to be restarted by the session manager. The
  current values are

  \value RestartIfRunning  if the application is still running when
  the session is shut down, it wants to be restarted at the start of
  the next session.

  \value RestartAnyway  the application wants to be started at the
  start of the next session, no matter what. (This is useful for
  utilities that run just after startup and then quit.)

  \value RestartImmediately  the application wants to be started
  immediately whenever it is not running.

  \value RestartNever  the application does not want to be restarted
  automatically.

  The default hint is \c RestartIfRunning.
*/


/*!
  \fn QString QSessionManager::sessionId() const

  Returns the identifier of the current session.

  If the application has been restored from an earlier session, this
  identifier is the same as it was in that earlier session.

  \sa sessionKey(), QApplication::sessionId()
 */

/*!
  \fn QString QSessionManager::sessionKey() const

  Returns the session key in the current session.

  If the application has been restored from an earlier session, this
  key is the same as it was when the previous session ended.

  The session key changes with every call of commitData() or
  saveState().

  \sa sessionId(), QApplication::sessionKey()
 */

// ### Note: This function is undocumented, since it is #ifdef'd.

/*!
  \fn void* QSessionManager::handle() const

  X11 only: returns a handle to the current \c SmcConnection.
*/


/*!
  \fn bool QSessionManager::allowsInteraction()

  Asks the session manager for permission to interact with the
  user. Returns TRUE if interaction is permitted; otherwise
  returns FALSE.

  The rationale behind this mechanism is to make it possible to
  synchronize user interaction during a shutdown. Advanced session
  managers may ask all applications simultaneously to commit their
  data, resulting in a much faster shutdown.

  When the interaction is completed we strongly recommend releasing the
  user interaction semaphore with a call to release(). This way, other
  applications may get the chance to interact with the user while your
  application is still busy saving data. (The semaphore is implicitly
  released when the application exits.)

  If the user decides to cancel the shutdown process during the
  interaction phase, you must tell the session manager that this has
  happened by calling cancel().

  Here's an example of how an application's QApplication::commitData()
  might be implemented:

\code
void MyApplication::commitData( QSessionManager& sm ) {
    if ( sm.allowsInteraction() ) {
	switch ( QMessageBox::warning(
		    yourMainWindow,
		    tr("Application Name"),
		    tr("Save changes to document Foo?"),
		    tr("&Yes"),
		    tr("&No"),
		    tr("Cancel"),
		    0, 2) ) {
	case 0: // yes
	    sm.release();
	    // save document here; if saving fails, call sm.cancel()
	    break;
	case 1: // continue without saving
	    break;
	default: // cancel
	    sm.cancel();
	    break;
	}
    } else {
	// we did not get permission to interact, then
	// do something reasonable instead.
    }
}
\endcode

  If an error occurred within the application while saving its data,
  you may want to try allowsErrorInteraction() instead.

  \sa QApplication::commitData(), release(), cancel()
*/


/*!
  \fn bool QSessionManager::allowsErrorInteraction()

  This is similar to allowsInteraction(), but also tells the session
  manager that an error occurred. Session managers may give error
  interaction request higher priority, which means that it is more likely
  that an error interaction is permitted. However, you are still not
  guaranteed that the session manager will allow interaction.

  \sa allowsInteraction(), release(), cancel()
*/

/*!
  \fn void QSessionManager::release()

  Releases the session manager's interaction semaphore after an
  interaction phase.

  \sa allowsInteraction(), allowsErrorInteraction()
*/

/*!
  \fn void QSessionManager::cancel()

  Tells the session manager to cancel the shutdown process.  Applications
  should not call this function without first asking the user.

  \sa allowsInteraction(), allowsErrorInteraction()

*/

/*!
  \fn void QSessionManager::setRestartHint( RestartHint hint )

  Sets the application's restart hint to \a hint. On application
  startup the hint is set to \c RestartIfRunning.

  Note that these flags are only hints, a session manager may or may
  not respect them.

  We recommend setting the restart hint in QApplication::saveState()
  because most session managers perform a checkpoint shortly after an
  application's startup.

  \sa restartHint()
*/

/*!
  \fn QSessionManager::RestartHint QSessionManager::restartHint() const

  Returns the application's current restart hint. The default is
  \c RestartIfRunning.

  \sa setRestartHint()
*/

/*!
  \fn void QSessionManager::setRestartCommand( const QStringList& command )

  If the session manager is capable of restoring sessions it will
  execute \a command in order to restore the application. The command
  defaults to

  \code
	appname -session id
  \endcode

  The \c -session option is mandatory; otherwise QApplication cannot
  tell whether it has been restored or what the current session
  identifier is. See QApplication::isSessionRestored() and
  QApplication::sessionId() for details.

  If your application is very simple, it may be possible to store the
  entire application state in additional command line options. This
  is usually a very bad idea because command lines are often limited
  to a few hundred bytes. Instead, use QSettings, or temporary files
  or a database for this purpose. By marking the data with the unique
  sessionId(), you will be able to restore the application in a future
  session.

  \sa restartCommand(), setDiscardCommand(), setRestartHint()
*/

/*!
  \fn QStringList QSessionManager::restartCommand() const

  Returns the currently set restart command.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = mySession.restartCommand();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa setRestartCommand(), restartHint()
*/

/*!
  \fn void QSessionManager::setDiscardCommand( const QStringList& )

  \sa discardCommand(), setRestartCommand()
*/


/*!
  \fn QStringList QSessionManager::discardCommand() const

  Returns the currently set discard command.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = mySession.discardCommand();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa setDiscardCommand(), restartCommand(), setRestartCommand()
*/

/*!
  \overload void QSessionManager::setManagerProperty( const QString& name,
						      const QString& value )

  Low-level write access to the application's identification and state
  records are kept in the session manager.

    The property called \a name has its value set to the string \a value.
*/

/*!
  \fn void QSessionManager::setManagerProperty( const QString& name,
						const QStringList& value )

  Low-level write access to the application's identification and state
  record are kept in the session manager.

    The property called \a name has its value set to the string list \a value.
*/

/*!
  \fn bool QSessionManager::isPhase2() const

  Returns TRUE if the session manager is currently performing a second
  session management phase; otherwise returns FALSE.

  \sa requestPhase2()
*/

/*!
  \fn void QSessionManager::requestPhase2()

  Requests a second session management phase for the application. The
  application may then return immediately from the
  QApplication::commitData() or QApplication::saveState() function,
  and they will be called again once most or all other applications have
  finished their session management.

  The two phases are useful for applications such as the X11 window manager
  that need to store information about another application's windows
  and therefore have to wait until these applications have completed their
  respective session management tasks.

  Note that if another application has requested a second phase it
  may get called before, simultaneously with, or after your
  application's second phase.

  \sa isPhase2()
*/

/*!
  \fn int QApplication::horizontalAlignment( int align )

  Strips out vertical alignment flags and transforms an
  alignment \a align of AlignAuto into AlignLeft or
  AlignRight according to the language used. The other horizontal
  alignment flags are left untouched.
*/


/*****************************************************************************
  Stubbed session management support
 *****************************************************************************/
#ifndef QT_NO_SESSIONMANAGER
#if defined( QT_NO_SM_SUPPORT ) || defined( Q_WS_WIN ) || defined( Q_WS_MAC ) || defined( Q_WS_QWS )

class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QString sessionKey;
    QSessionManager::RestartHint restartHint;
};

QSessionManager* qt_session_manager_self = 0;
QSessionManager::QSessionManager( QApplication * app, QString &id, QString &key )
    : QObject( app, "qt_sessionmanager" )
{
    qt_session_manager_self = this;
    d = new QSessionManagerData;
#if defined(Q_WS_WIN) && !defined(Q_OS_TEMP)
    wchar_t guidstr[40];
    GUID guid;
    CoCreateGuid( &guid );
    StringFromGUID2(guid, guidstr, 40);
    id = QString::fromUcs2((ushort*)guidstr);
    CoCreateGuid( &guid );
    StringFromGUID2(guid, guidstr, 40);
    key = QString::fromUcs2((ushort*)guidstr);
#endif
    d->sessionId = id;
    d->sessionKey = key;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    delete d;
    qt_session_manager_self = 0;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

QString QSessionManager::sessionKey() const
{
    return d->sessionKey;
}


#if defined(Q_WS_X11) || defined(Q_WS_MAC)
void* QSessionManager::handle() const
{
    return 0;
}
#endif

#if !defined(Q_WS_WIN)
bool QSessionManager::allowsInteraction()
{
    return TRUE;
}

bool QSessionManager::allowsErrorInteraction()
{
    return TRUE;
}
void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}
#endif


void QSessionManager::setRestartHint( QSessionManager::RestartHint hint)
{
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return d->restartHint;
}

void QSessionManager::setRestartCommand( const QStringList& command)
{
    d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
    return d->restartCommand;
}

void QSessionManager::setDiscardCommand( const QStringList& command)
{
    d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
    return d->discardCommand;
}

void QSessionManager::setManagerProperty( const QString&, const QString&)
{
}

void QSessionManager::setManagerProperty( const QString&, const QStringList& )
{
}

bool QSessionManager::isPhase2() const
{
    return FALSE;
}

void QSessionManager::requestPhase2()
{
}

#endif // QT_NO_SM_SUPPORT
#endif //QT_NO_SESSIONMANAGER
