/****************************************************************************
** $Id: qapplication_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of X11 startup routines and event handling
**
** Created : 931029
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

// ### 4.0: examine Q_EXPORT's below. The respective symbols had all
// been in use (e.g. in the KDE wm ) before the introduction of a version
// map. One might want to turn some of them into propert public API and
// provide a proper alternative for others. See also the exports in
// qapplication_win.cpp which suggest a unification.

// ### needed for solaris-g++ in beta5
#define QT_CLEAN_NAMESPACE

#include "qplatformdefs.h"

// POSIX Large File Support redefines open -> open64
#if defined(open)
# undef open
#endif

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED.
#if defined(connect)
# undef connect
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#include "qapplication.h"
#include "qapplication_p.h"
#include "qcolor_p.h"
#include "qcursor.h"
#include "qwidget.h"
#include "qwidget_p.h"
#include "qobjectlist.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qbitarray.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qtextcodec.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qsocketnotifier.h"
#include "qsessionmanager.h"
#include "qvaluelist.h"
#include "qdict.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qwhatsthis.h" // ######## dependency
#include "qsettings.h"
#include "qstylefactory.h"
#include "qfileinfo.h"

// Input method stuff - UNFINISHED
#include "qinputcontext_p.h"
#include "qinternal_p.h" // shared double buffer cleanup

#if defined(QT_THREAD_SUPPORT)
# include "qthread.h"
#endif

#if defined(QT_DEBUG) && defined(Q_OS_LINUX)
# include "qfile.h"
#endif

#include "qt_x11_p.h"

#if !defined(QT_NO_XFTFREETYPE)
// XFree86 4.0.3 implementation is missing XftInitFtLibrary forward
extern "C" Bool XftInitFtLibrary(void);
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

//#define X_NOT_BROKEN
#ifdef X_NOT_BROKEN
// Some X libraries are built with setlocale #defined to _Xsetlocale,
// even though library users are then built WITHOUT such a definition.
// This creates a problem - Qt might setlocale() one value, but then
// X looks and doesn't see the value Qt set. The solution here is to
// implement _Xsetlocale just in case X calls it - redirecting it to
// the real libC version.
//
# ifndef setlocale
extern "C" char *_Xsetlocale(int category, const char *locale);
char *_Xsetlocale(int category, const char *locale)
{
    //qDebug("_Xsetlocale(%d,%s),category,locale");
    return setlocale(category,locale);
}
# endif // setlocale
#endif // X_NOT_BROKEN


// resolve the conflict between X11's FocusIn and QEvent::FocusIn
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease


// Fix old X libraries
#ifndef XK_KP_Home
#define XK_KP_Home              0xFF95
#endif
#ifndef XK_KP_Left
#define XK_KP_Left              0xFF96
#endif
#ifndef XK_KP_Up
#define XK_KP_Up                0xFF97
#endif
#ifndef XK_KP_Right
#define XK_KP_Right             0xFF98
#endif
#ifndef XK_KP_Down
#define XK_KP_Down              0xFF99
#endif
#ifndef XK_KP_Prior
#define XK_KP_Prior             0xFF9A
#endif
#ifndef XK_KP_Next
#define XK_KP_Next              0xFF9B
#endif
#ifndef XK_KP_End
#define XK_KP_End               0xFF9C
#endif
#ifndef XK_KP_Insert
#define XK_KP_Insert            0xFF9E
#endif
#ifndef XK_KP_Delete
#define XK_KP_Delete            0xFF9F
#endif


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static const char *appName;			// application name
static const char *appClass;			// application class
static const char *appFont	= 0;		// application font
static const char *appBGCol	= 0;		// application bg color
static const char *appFGCol	= 0;		// application fg color
static const char *appBTNCol	= 0;		// application btn color
static const char *mwGeometry	= 0;		// main widget geometry
static const char *mwTitle	= 0;		// main widget title
//Ming-Che 10/10
static char    *ximServer	= 0;		// XIM Server will connect to
static bool	mwIconic	= FALSE;	// main widget iconified
//Ming-Che 10/10
static bool	noxim		= FALSE;	// connect to xim or not
static Display *appDpy		= 0;		// X11 application display
static char    *appDpyName	= 0;		// X11 display name
static bool	appForeignDpy	= FALSE;        // we didn't create display
static bool	appSync		= FALSE;	// X11 synchronization
#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// X11 grabbing enabled
static bool	appDoGrab	= FALSE;	// X11 grabbing override (gdb)
#endif
static int	appScreen;			// X11 screen number
static int	appScreenCount;			// X11 screen count
static bool	app_save_rootinfo = FALSE;	// save root info
static bool	app_do_modal	= FALSE;	// modal mode
static Window	curWin = 0;			// current window

static GC*	app_gc_ro	= 0;		// read-only GC
static GC*	app_gc_tmp	= 0;		// temporary GC
static GC*	app_gc_ro_m	= 0;		// read-only GC (monochrome)
static GC*	app_gc_tmp_m	= 0;		// temporary GC (monochrome)
// symbols needed by extern QXEmbed class
Q_EXPORT Atom	qt_wm_protocols		= 0;	// window manager protocols
Q_EXPORT Atom	qt_wm_delete_window	= 0;	// delete window protocol
Q_EXPORT Atom	qt_wm_take_focus	= 0;	// take focus window protocol

Atom		qt_qt_scrolldone	= 0;	// scroll synchronization
Atom		qt_net_wm_context_help	= 0;	// context help
Atom		qt_net_wm_ping		= 0;	// _NET_WM_PING protocol

static Atom	qt_xsetroot_id		= 0;
Atom            qt_xa_clipboard         = 0;
Atom		qt_selection_property	= 0;
Atom            qt_clipboard_sentinel   = 0;
Atom		qt_selection_sentinel	= 0;
Q_EXPORT Atom	qt_wm_state		= 0;
Atom		qt_wm_change_state	= 0;
static Atom     qt_settings_timestamp	= 0;    // Qt >=3 settings timestamp
static Atom	qt_input_encoding	= 0;	// Qt desktop properties
static Atom	qt_resource_manager	= 0;	// X11 Resource manager
Atom		qt_sizegrip		= 0;	// sizegrip
Atom		qt_wm_client_leader	= 0;
Q_EXPORT Atom	qt_window_role		= 0;
Q_EXPORT Atom	qt_sm_client_id		= 0;
Atom		qt_xa_motif_wm_hints	= 0;
Atom		qt_cde_running		= 0;
Atom		qt_kwin_running	= 0;
Atom		qt_kwm_running	= 0;
Atom		qt_gbackground_properties	= 0;
Atom		qt_x_incr		= 0;
Atom		qt_utf8_string = 0;

// detect broken window managers
Atom            qt_sgi_desks_manager    = 0;
bool		qt_broken_wm		= FALSE;
static void qt_detect_broken_window_manager();

// NET WM support
Atom		qt_net_supported	= 0;
Atom		qt_net_wm_name		= 0;
Atom		qt_net_wm_icon_name	= 0;
Atom		qt_net_virtual_roots	= 0;
Atom		qt_net_workarea		= 0;
Atom		qt_net_wm_state		= 0;
Atom		qt_net_wm_state_modal	= 0;
Atom		qt_net_wm_state_max_v	= 0;
Atom		qt_net_wm_state_max_h	= 0;
Atom		qt_net_wm_state_fullscreen = 0;
Atom		qt_net_wm_state_above	= 0;
Atom            qt_net_wm_window_type   = 0;
Atom            qt_net_wm_window_type_normal	= 0;
Atom            qt_net_wm_window_type_dialog	= 0;
Atom            qt_net_wm_window_type_toolbar	= 0;
Atom		qt_net_wm_window_type_menu	= 0;
Atom		qt_net_wm_window_type_utility	= 0;
Atom            qt_net_wm_window_type_splash    = 0;
Atom            qt_net_wm_window_type_override	= 0;	// KDE extension
Atom		qt_net_wm_frame_strut		= 0;	// KDE extension
Atom		qt_net_wm_state_stays_on_top	= 0;	// KDE extension
Atom		qt_net_wm_pid		= 0;
Atom		qt_net_wm_user_time	= 0;
// Enlightenment support
Atom		qt_enlightenment_desktop	= 0;

// window managers list of supported "stuff"
Atom		*qt_net_supported_list	= 0;
// list of virtual root windows
Window		*qt_net_virtual_root_list	= 0;



// client leader window
Window qt_x11_wm_client_leader = 0;

// function to update the workarea of the screen - in qdesktopwidget_x11.cpp
extern void qt_desktopwidget_update_workarea();

// current focus model
static const int FocusModel_Unknown = -1;
static const int FocusModel_Other = 0;
static const int FocusModel_PointerRoot = 1;
static int qt_focus_model = -1;

#ifndef QT_NO_XRANDR
// TRUE if Qt is compiled w/ XRandR support and XRandR exists on the connected
// Display
bool	qt_use_xrandr	= FALSE;
static int xrandr_eventbase;
#endif

// TRUE if Qt is compiled w/ XRender support and XRender exists on the connected
// Display
Q_EXPORT bool qt_use_xrender = FALSE;

// modifier masks for alt/meta - detected when the application starts
static long qt_alt_mask = 0;
static long qt_meta_mask = 0;
// modifier mask to remove mode switch from modifiers that have alt/meta set
// this problem manifests itself on HP/UX 10.20 at least, and without it
// modifiers do not work at all...
static long qt_mode_switch_remove_mask = 0;

// flags for extensions for special Languages, currently only for RTL languages
static bool 	qt_use_rtl_extensions = FALSE;
bool qt_hebrew_keyboard_hack = FALSE;

static Window	mouseActWindow	     = 0;	// window where mouse is
static int	mouseButtonPressed   = 0;	// last mouse button pressed
static int	mouseButtonState     = 0;	// mouse button state
static Time	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse pres position in act window
static short	mouseGlobalXPos, mouseGlobalYPos; // global mouse press position

extern QWidgetList *qt_modal_stack;		// stack of modal widgets
static bool	    ignoreNextMouseReleaseEvent = FALSE; // ignore the next mouse release
							 // event if return from a modal
							 // widget

static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;
static bool	    popupGrabOk;

static bool sm_blockUserInput = FALSE;		// session management

int qt_xfocusout_grab_counter = 0;

#if defined (QT_TABLET_SUPPORT)
// since XInput event classes aren't created until we actually open an XInput
// device, here is a static list that we will use later on...
const int INVALID_EVENT = -1;
const int TOTAL_XINPUT_EVENTS = 7;

XDevice *devStylus = NULL;
XDevice *devEraser = NULL;
XEventClass event_list_stylus[TOTAL_XINPUT_EVENTS];
XEventClass event_list_eraser[TOTAL_XINPUT_EVENTS];

int qt_curr_events_stylus = 0;
int qt_curr_events_eraser = 0;

// well, luckily we only need to do this once.
static int xinput_motion = INVALID_EVENT;
static int xinput_key_press = INVALID_EVENT;
static int xinput_key_release = INVALID_EVENT;
static int xinput_button_press = INVALID_EVENT;
static int xinput_button_release = INVALID_EVENT;

// making this assumption on XFree86, since we can only use 1 device,
// the pressure for the eraser and the stylus should be the same, if they aren't
// well, they certainly have a strange pen then...
static int max_pressure;
extern bool chokeMouse;
#endif

// last timestamp read from QSettings
static uint appliedstamp = 0;


typedef int (*QX11EventFilter) (XEvent*);
QX11EventFilter qt_set_x11_event_filter(QX11EventFilter filter);

static QX11EventFilter qt_x11_event_filter = 0;
Q_EXPORT QX11EventFilter qt_set_x11_event_filter(QX11EventFilter filter)
{
    QX11EventFilter old_filter = qt_x11_event_filter;
    qt_x11_event_filter = filter;
    return old_filter;
}
static bool qt_x11EventFilter( XEvent* ev )
{
    if ( qt_x11_event_filter  && qt_x11_event_filter( ev )  )
	return TRUE;
    return qApp->x11EventFilter( ev );
}





#if !defined(QT_NO_XIM)
XIM		qt_xim			= 0;
XIMStyle	qt_xim_style		= 0;
static XIMStyle xim_default_style	= XIMPreeditCallbacks | XIMStatusNothing;
static XIMStyle	xim_preferred_style	= 0;
#endif

static int composingKeycode=0;
static QTextCodec * input_mapper = 0;

Q_EXPORT Time	qt_x_time = CurrentTime;
Q_EXPORT Time	qt_x_user_time = CurrentTime;
extern bool     qt_check_clipboard_sentinel(); //def in qclipboard_x11.cpp
extern bool	qt_check_selection_sentinel(); //def in qclipboard_x11.cpp

static void	qt_save_rootinfo();
bool	qt_try_modal( QWidget *, XEvent * );

int		qt_ncols_option  = 216;		// used in qcolor_x11.cpp
int		qt_visual_option = -1;
bool		qt_cmap_option	 = FALSE;
QWidget	       *qt_button_down	 = 0;		// widget got last button-down

extern bool qt_tryAccelEvent( QWidget*, QKeyEvent* ); // def in qaccel.cpp

struct QScrollInProgress {
    static long serial;
    QScrollInProgress( QWidget* w, int x, int y ) :
    id( serial++ ), scrolled_widget( w ), dx( x ), dy( y ) {}
    long id;
    QWidget* scrolled_widget;
    int dx, dy;
};
long QScrollInProgress::serial=0;
static QPtrList<QScrollInProgress> *sip_list = 0;


// stuff in qt_xdnd.cpp
// setup
extern void qt_xdnd_setup();
// x event handling
extern void qt_handle_xdnd_enter( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_position( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_status( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_leave( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_drop( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_finished( QWidget *, const XEvent *, bool );
extern void qt_xdnd_handle_selection_request( const XSelectionRequestEvent * );
extern bool qt_xdnd_handle_badwindow();

extern void qt_motifdnd_handle_msg( QWidget *, const XEvent *, bool );
extern void qt_x11_motifdnd_init();

// client message atoms
extern Atom qt_xdnd_enter;
extern Atom qt_xdnd_position;
extern Atom qt_xdnd_status;
extern Atom qt_xdnd_leave;
extern Atom qt_xdnd_drop;
extern Atom qt_xdnd_finished;
// xdnd selection atom
extern Atom qt_xdnd_selection;
extern bool qt_xdnd_dragging;

// gui or non-gui from qapplication.cpp
extern bool qt_is_gui_used;
extern bool qt_app_has_font;

static bool qt_x11_cmdline_font = false;


extern bool qt_resolve_symlinks; // from qapplication.cpp

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();


// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();

void qt_x11_intern_atom( const char *, Atom * );

static QPtrList<QWidget>* deferred_map_list = 0;
static void qt_deferred_map_cleanup()
{
    delete deferred_map_list;
    deferred_map_list = 0;
}
void qt_deferred_map_add( QWidget* w)
{
    if ( !deferred_map_list ) {
	deferred_map_list = new QPtrList<QWidget>;
	qAddPostRoutine( qt_deferred_map_cleanup );
    }
    deferred_map_list->append( w );
}
void qt_deferred_map_take( QWidget* w )
{
    if (deferred_map_list ) {
	deferred_map_list->remove( w );
    }
}
bool qt_deferred_map_contains( QWidget* w )
{
    if (!deferred_map_list)
	return FALSE;
    else
	return deferred_map_list->contains( w );
}


class QETWidget : public QWidget		// event translator widget
{
public:
    void setWState( WFlags f )		{ QWidget::setWState(f); }
    void clearWState( WFlags f )	{ QWidget::clearWState(f); }
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
    bool translateMouseEvent( const XEvent * );
    bool translateKeyEventInternal( const XEvent *, int& count, QString& text, int& state, char& ascii, int &code,
				    QEvent::Type &type, bool willRepeat=FALSE );
    bool translateKeyEvent( const XEvent *, bool grab );
    bool translatePaintEvent( const XEvent * );
    bool translateConfigEvent( const XEvent * );
    bool translateCloseEvent( const XEvent * );
    bool translateScrollDoneEvent( const XEvent * );
    bool translateWheelEvent( int global_x, int global_y, int delta, int state, Orientation orient );
#if defined (QT_TABLET_SUPPORT)
    bool translateXinputEvent( const XEvent* );
#endif
    bool translatePropertyEvent(const XEvent *);
};




// ************************************************************************
// X Input Method support
// ************************************************************************

#if !defined(QT_NO_XIM)

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif // Q_C_CALLBACKS

#ifdef USE_X11R6_XIM
    static void xim_create_callback(XIM /*im*/,
				    XPointer /*client_data*/,
				    XPointer /*call_data*/)
    {
	// qDebug("xim_create_callback");
	QApplication::create_xim();
    }

    static void xim_destroy_callback(XIM /*im*/,
				     XPointer /*client_data*/,
				     XPointer /*call_data*/)
    {
	// qDebug("xim_destroy_callback");
	QApplication::close_xim();
	XRegisterIMInstantiateCallback(appDpy, 0, 0, 0,
				       (XIMProc) xim_create_callback, 0);
    }

#endif // USE_X11R6_XIM

#if defined(Q_C_CALLBACKS)
}
#endif // Q_C_CALLBACKS

#endif // QT_NO_XIM


/*! \internal
  Creates the application input method.
 */
void QApplication::create_xim()
{
#ifndef QT_NO_XIM
    qt_xim = XOpenIM( appDpy, 0, 0, 0 );
    if ( qt_xim ) {

#ifdef USE_X11R6_XIM
	XIMCallback destroy;
	destroy.callback = (XIMProc) xim_destroy_callback;
	destroy.client_data = 0;
	if ( XSetIMValues( qt_xim, XNDestroyCallback, &destroy, (char *) 0 ) != 0 )
	    qWarning( "Xlib dosn't support destroy callback");
#endif // USE_X11R6_XIM

	XIMStyles *styles = 0;
	XGetIMValues(qt_xim, XNQueryInputStyle, &styles, (char *) 0, (char *) 0);
	if ( styles ) {
	    int i;
	    for ( i = 0; !qt_xim_style && i < styles->count_styles; i++ ) {
		if ( styles->supported_styles[i] == xim_preferred_style ) {
		    qt_xim_style = xim_preferred_style;
		    break;
		}
	    }
	    // if the preferred input style couldn't be found, look for
	    // Nothing
	    for ( i = 0; !qt_xim_style && i < styles->count_styles; i++ ) {
		if ( styles->supported_styles[i] == (XIMPreeditNothing |
						     XIMStatusNothing) ) {
		    qt_xim_style = XIMPreeditNothing | XIMStatusNothing;
		    break;
		}
	    }
	    // ... and failing that, None.
	    for ( i = 0; !qt_xim_style && i < styles->count_styles; i++ ) {
		if ( styles->supported_styles[i] == (XIMPreeditNone |
						     XIMStatusNone) ) {
		    qt_xim_style = XIMPreeditNone | XIMStatusNone;
		    break;
		}
	    }

	    // qDebug("QApplication: using im style %lx", qt_xim_style);
	    XFree( (char *)styles );
	}

	if ( qt_xim_style ) {

#ifdef USE_X11R6_XIM
	    XUnregisterIMInstantiateCallback(appDpy, 0, 0, 0,
					     (XIMProc) xim_create_callback, 0);
#endif // USE_X11R6_XIM

	    QWidgetList *list= qApp->topLevelWidgets();
	    QWidgetListIt it(*list);
	    QWidget * w;
	    while( (w=it.current()) != 0 ) {
		++it;
		w->createTLSysExtra();
	    }
	    delete list;
	} else {
	    // Give up
	    qWarning( "No supported input style found."
		      "  See InputMethod documentation.");
	    close_xim();
	}
    }
#endif // QT_NO_XIM
}


/*! \internal
  Closes the application input method.
*/
void QApplication::close_xim()
{
#ifndef QT_NO_XIM
    // Calling XCloseIM gives a Purify FMR error
    // XCloseIM( qt_xim );
    // We prefer a less serious memory leak

    qt_xim = 0;
    QWidgetList *list = qApp->topLevelWidgets();
    QWidgetListIt it(*list);
    while(it.current()) {
	it.current()->destroyInputContext();
	++it;
    }
    delete list;
#endif // QT_NO_XIM
}


/*****************************************************************************
  Default X error handlers
 *****************************************************************************/

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static bool x11_ignore_badwindow;
static bool x11_badwindow;

    // starts to ignore bad window errors from X
void qt_ignore_badwindow()
{
    x11_ignore_badwindow = TRUE;
    x11_badwindow = FALSE;
}

    // ends ignoring bad window errors and returns whether an error
    // had happen.
bool qt_badwindow()
{
    x11_ignore_badwindow = FALSE;
    return x11_badwindow;
}

static int (*original_x_errhandler)( Display *dpy, XErrorEvent * );
static int (*original_xio_errhandler)( Display *dpy );

static int qt_x_errhandler( Display *dpy, XErrorEvent *err )
{
    if ( err->error_code == BadWindow ) {
	x11_badwindow = TRUE;
	if ( err->request_code == 25 /* X_SendEvent */ &&
	     qt_xdnd_handle_badwindow() )
	    return 0;
	if ( x11_ignore_badwindow )
	    return 0;
    } else if ( err->error_code == BadMatch &&
		err->request_code == 42 /* X_SetInputFocus */ ) {
	return 0;
    }

    char errstr[256];
    XGetErrorText( dpy, err->error_code, errstr, 256 );
    qWarning( "X Error: %s %d\n"
	      "  Major opcode:  %d\n"
	      "  Minor opcode:  %d\n"
	      "  Resource id:  0x%lx",
	      errstr, err->error_code,
	      err->request_code,
	      err->minor_code,
	      err->resourceid );

    // ### we really should distinguish between severe, non-severe and
    // ### application specific errors

    return 0;
}


static int qt_xio_errhandler( Display * )
{
    qWarning( "%s: Fatal IO error: client killed", appName );
    qApp = 0;
    exit( 1 );
    //### give the application a chance for a proper shutdown instead,
    //### exit(1) doesn't help.
    return 0;
}

#if defined(Q_C_CALLBACKS)
}
#endif


// Memory leak: if the app exits before qt_init_internal(), this dict
// isn't released correctly.
static QAsciiDict<Atom> *atoms_to_be_created = 0;
static bool create_atoms_now = 0;

/*****************************************************************************
  qt_x11_intern_atom() - efficiently interns an atom, now or later.

  If the application is being initialized, this function stores the
  adddress of the atom and qt_init_internal will do the actual work
  quickly. If the application is running, the atom is created here.

  Neither argument may point to temporary variables.
 *****************************************************************************/

void qt_x11_intern_atom( const char *name, Atom *result)
{
    if ( !name || !result || *result )
	return;

    if ( create_atoms_now ) {
	*result = XInternAtom( appDpy, name, False );
    } else {
	if ( !atoms_to_be_created ) {
	    atoms_to_be_created = new QAsciiDict<Atom>;
	    atoms_to_be_created->setAutoDelete( FALSE );
	}
	atoms_to_be_created->insert( name, result );
	*result = 0;
    }
}


static void qt_x11_process_intern_atoms()
{
    if ( atoms_to_be_created ) {
#if defined(XlibSpecificationRelease) && (XlibSpecificationRelease >= 6)
	int i = atoms_to_be_created->count();
	Atom * res = (Atom *)malloc( i * sizeof( Atom ) );
	Atom ** resp = (Atom **)malloc( i * sizeof( Atom* ) );
	char ** names = (char **)malloc( i * sizeof(const char*));

	i = 0;
	QAsciiDictIterator<Atom> it( *atoms_to_be_created );
	while( it.current() ) {
	    res[i] = 0;
	    resp[i] = it.current();
	    names[i] = qstrdup(it.currentKey());
	    i++;
	    ++it;
	}
	XInternAtoms( appDpy, names, i, False, res );
	while( i ) {
	    i--;
	    delete [] names[i];
	    if ( res[i] && resp[i] )
		*(resp[i]) = res[i];
	}
	free( res );
	free( resp );
	free( names );
#else
	QAsciiDictIterator<Atom> it( *atoms_to_be_created );
	Atom * result;
	const char * name;
	while( (result = it.current()) != 0 ) {
	    name = it.currentKey();
	    ++it;
	    *result = XInternAtom( appDpy, name, False );
	}
#endif
	delete atoms_to_be_created;
	atoms_to_be_created = 0;
	create_atoms_now = TRUE;
    }
}


/*! \internal
    apply the settings to the application
*/
bool QApplication::x11_apply_settings()
{
    if (! qt_std_pal)
	qt_create_std_palette();

    Atom type;
    int format;
    long offset = 0;
    unsigned long nitems, after = 1;
    unsigned char *data = 0;
    QDateTime timestamp, settingsstamp;
    bool update_timestamp = FALSE;

    if (XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow( 0 ),
			   qt_settings_timestamp, 0, 0,
			   False, AnyPropertyType, &type, &format, &nitems,
			   &after, &data) == Success && format == 8) {
	if (data)
	    XFree(data);

	QBuffer ts;
	ts.open(IO_WriteOnly);

	while (after > 0) {
	    XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow( 0 ),
			       qt_settings_timestamp,
			       offset, 1024, False, AnyPropertyType,
			       &type, &format, &nitems, &after, &data);
	    if (format == 8) {
		ts.writeBlock((const char *) data, nitems);
		offset += nitems / 4;
	    }

	    XFree(data);
	}

	QDataStream d(ts.buffer(), IO_ReadOnly);
	d >> timestamp;
    }

    QSettings settings;
    settingsstamp = settings.lastModificationTime( "/qt/font" );
    if (! settingsstamp.isValid())
	return FALSE;

    if ( appliedstamp && appliedstamp == settingsstamp.toTime_t() )
	return TRUE;
    appliedstamp = settingsstamp.toTime_t();

    if (! timestamp.isValid() || settingsstamp > timestamp)
	update_timestamp = TRUE;

    /*
      Qt settings. This is now they are written into the datastream.

      /qt/Palette/ *             - QPalette
      /qt/font                   - QFont
      /qt/libraryPath            - QStringList
      /qt/style                  - QString
      /qt/doubleClickInterval    - int
      /qt/cursorFlashTime        - int
      /qt/wheelScrollLines       - int
      /qt/colorSpec              - QString
      /qt/defaultCodec           - QString
      /qt/globalStrut            - QSize
      /qt/GUIEffects             - QStringList
      /qt/Font Substitutions/ *  - QStringList
      /qt/Font Substitutions/... - QStringList
    */

    QString str;
    QStringList strlist;
    int i, num;
    QPalette pal(QApplication::palette());
    strlist = settings.readListEntry("/qt/Palette/active");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Active, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }
    strlist = settings.readListEntry("/qt/Palette/inactive");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Inactive, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }
    strlist = settings.readListEntry("/qt/Palette/disabled");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Disabled, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }

    // workaround for KDE 3.0, which messes up the buttonText value of
    // the disabled palette in QSettings
    if ( pal.disabled().buttonText() == pal.active().buttonText() ) {
	pal.setColor( QPalette::Disabled, QColorGroup::ButtonText,
		      pal.disabled().foreground() );
    }

    if (pal != *qt_std_pal && pal != QApplication::palette()) {
	QApplication::setPalette(pal, TRUE);
	*qt_std_pal = pal;
    }

    QFont font(QApplication::font());
    if ( !qt_app_has_font && !qt_x11_cmdline_font ) {
        // read new font
        str = settings.readEntry("/qt/font");
        if (! str.isNull() && ! str.isEmpty()) {
            font.fromString(str);

            if (font != QApplication::font())
                QApplication::setFont(font, TRUE);
        }
    }

    // read library (ie. plugin) path list
    QString libpathkey =
	QString("/qt/%1.%2/libraryPath").arg( QT_VERSION >> 16 ).arg( (QT_VERSION & 0xff00 ) >> 8 );
    QStringList pathlist = settings.readListEntry(libpathkey, ':');
    if (! pathlist.isEmpty()) {
	QStringList::ConstIterator it = pathlist.begin();
	while (it != pathlist.end())
	    QApplication::addLibraryPath(*it++);
    }

    // read new QStyle
    extern bool qt_explicit_app_style; // defined in qapplication.cpp
    QString stylename = settings.readEntry( "/qt/style" );
    if ( !stylename.isEmpty() && !qt_explicit_app_style ) {
	QApplication::setStyle( stylename );
	// took the style from the user settings, so mark the explicit flag FALSE
	qt_explicit_app_style = FALSE;
    }

    num =
	settings.readNumEntry("/qt/doubleClickInterval",
			      QApplication::doubleClickInterval());
    QApplication::setDoubleClickInterval(num);

    num =
	settings.readNumEntry("/qt/cursorFlashTime",
			      QApplication::cursorFlashTime());
    QApplication::setCursorFlashTime(num);

    num =
	settings.readNumEntry("/qt/wheelScrollLines",
			      QApplication::wheelScrollLines());
    QApplication::setWheelScrollLines(num);

    QString colorspec = settings.readEntry("/qt/colorSpec", "default");
    if (colorspec == "normal")
	QApplication::setColorSpec(QApplication::NormalColor);
    else if (colorspec == "custom")
	QApplication::setColorSpec(QApplication::CustomColor);
    else if (colorspec == "many")
	QApplication::setColorSpec(QApplication::ManyColor);
    else if (colorspec != "default")
	colorspec = "default";

    QString defaultcodec = settings.readEntry("/qt/defaultCodec", "none");
    if (defaultcodec != "none") {
	QTextCodec *codec = QTextCodec::codecForName(defaultcodec);
	if (codec)
	    qApp->setDefaultCodec(codec);
    }

    QStringList strut = settings.readListEntry("/qt/globalStrut");
    if (! strut.isEmpty()) {
	if (strut.count() == 2) {
	    QSize sz(strut[0].toUInt(), strut[1].toUInt());

	    if (sz.isValid())
		QApplication::setGlobalStrut(sz);
	}
    }

    QStringList effects = settings.readListEntry("/qt/GUIEffects");

    QApplication::setEffectEnabled( Qt::UI_General, effects.contains("general") );
    QApplication::setEffectEnabled( Qt::UI_AnimateMenu, effects.contains("animatemenu") );
    QApplication::setEffectEnabled( Qt::UI_FadeMenu, effects.contains("fademenu") );
    QApplication::setEffectEnabled( Qt::UI_AnimateCombo, effects.contains("animatecombo") );
    QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, effects.contains("animatetooltip") );
    QApplication::setEffectEnabled( Qt::UI_FadeTooltip, effects.contains("fadetooltip") );
    QApplication::setEffectEnabled( Qt::UI_AnimateToolBox, effects.contains("animatetoolbox") );

    QStringList fontsubs =
	settings.entryList("/qt/Font Substitutions");
    if (!fontsubs.isEmpty()) {
	QStringList subs;
	QString fam, skey;
	QStringList::Iterator it = fontsubs.begin();
	while (it != fontsubs.end()) {
	    fam = (*it++);
	    skey = "/qt/Font Substitutions/" + fam;
	    subs = settings.readListEntry(skey);
	    QFont::insertSubstitutions(fam, subs);
	}
    }

    qt_broken_wm =
	settings.readBoolEntry("/qt/brokenWindowManager", qt_broken_wm);

    qt_resolve_symlinks =
	settings.readBoolEntry("/qt/resolveSymlinks", TRUE);

    qt_use_rtl_extensions =
    	settings.readBoolEntry("/qt/useRtlExtensions", FALSE);

#ifndef QT_NO_XIM
    if (xim_preferred_style == 0) {
        QString ximInputStyle =
            settings.readEntry( "/qt/XIMInputStyle",
                                QObject::trUtf8( "On The Spot" ) ).lower();
        if ( ximInputStyle == "on the spot" )
            xim_preferred_style = XIMPreeditCallbacks | XIMStatusNothing;
        else if ( ximInputStyle == "over the spot" )
            xim_preferred_style = XIMPreeditPosition | XIMStatusNothing;
        else if ( ximInputStyle == "off the spot" )
            xim_preferred_style = XIMPreeditArea | XIMStatusArea;
        else if ( ximInputStyle == "root" )
            xim_preferred_style = XIMPreeditNothing | XIMStatusNothing;
    }
#endif

    if (update_timestamp) {
	QBuffer stamp;
	QDataStream s(stamp.buffer(), IO_WriteOnly);
	s << settingsstamp;

	XChangeProperty(appDpy, QPaintDevice::x11AppRootWindow( 0 ),
			qt_settings_timestamp, qt_settings_timestamp, 8,
			PropModeReplace, (unsigned char *) stamp.buffer().data(),
			stamp.buffer().size());
    }

    return TRUE;
}


// read the _QT_INPUT_ENCODING property and apply the settings to
// the application
static void qt_set_input_encoding()
{
    Atom type;
    int format;
    ulong  nitems, after = 1;
    const char *data;

    int e = XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(),
				qt_input_encoding, 0, 1024,
				False, XA_STRING, &type, &format, &nitems,
				&after,  (unsigned char**)&data );
    if ( e != Success || !nitems || type == None ) {
	// Always use the locale codec, since we have no examples of non-local
	// XIMs, and since we cannot get a sensible answer about the encoding
	// from the XIM.
	input_mapper = QTextCodec::codecForLocale();

    } else {
	if ( !qstricmp( data, "locale" ) )
	    input_mapper = QTextCodec::codecForLocale();
	else
	    input_mapper = QTextCodec::codecForName( data );
	// make sure we have an input codec
	if( !input_mapper )
	    input_mapper = QTextCodec::codecForName( "ISO 8859-1" );
    }
    if ( input_mapper->mibEnum() == 11 ) // 8859-8
	input_mapper = QTextCodec::codecForName( "ISO 8859-8-I");
    if( data )
	XFree( (char *)data );
}

// set font, foreground and background from x11 resources. The
// arguments may override the resource settings.
static void qt_set_x11_resources( const char* font = 0, const char* fg = 0,
				  const char* bg = 0, const char* button = 0 )
{
    if ( !qt_std_pal )
	qt_create_std_palette();

    QCString resFont, resFG, resBG, resEF, sysFont;

    QApplication::setEffectEnabled( Qt::UI_General, FALSE);
    QApplication::setEffectEnabled( Qt::UI_AnimateMenu, FALSE);
    QApplication::setEffectEnabled( Qt::UI_FadeMenu, FALSE);
    QApplication::setEffectEnabled( Qt::UI_AnimateCombo, FALSE );
    QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, FALSE );
    QApplication::setEffectEnabled( Qt::UI_FadeTooltip, FALSE );
    QApplication::setEffectEnabled( Qt::UI_AnimateToolBox, FALSE );

    if ( QApplication::desktopSettingsAware() && !QApplication::x11_apply_settings()  ) {
	int format;
	ulong  nitems, after = 1;
	QCString res;
	long offset = 0;
	Atom type = None;

	while (after > 0) {
	    uchar *data;
	    XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow( 0 ),
				qt_resource_manager,
				offset, 8192, False, AnyPropertyType,
				&type, &format, &nitems, &after,
				&data );
	    res += (char*)data;
	    offset += 2048; // offset is in 32bit quantities... 8192/4 == 2048
	    if ( data )
		XFree( (char *)data );
	}

	QCString key, value;
	int l = 0, r;
	QCString apn = appName;
	QCString apc = appClass;
	int apnl = apn.length();
	int apcl = apc.length();
	int resl = res.length();

	while (l < resl) {
	    r = res.find( '\n', l );
	    if ( r < 0 )
		r = resl;
	    while ( isspace((uchar) res[l]) )
		l++;
	    bool mine = FALSE;
	    if ( res[l] == '*' &&
		 (res[l+1] == 'f' || res[l+1] == 'b' || res[l+1] == 'g' ||
		  res[l+1] == 'F' || res[l+1] == 'B' || res[l+1] == 'G' ||
		  res[l+1] == 's' || res[l+1] == 'S' ) ) {
		// OPTIMIZED, since we only want "*[fbgs].."

		QCString item = res.mid( l, r - l ).simplifyWhiteSpace();
		int i = item.find( ":" );
		key = item.left( i ).stripWhiteSpace().mid(1).lower();
		value = item.right( item.length() - i - 1 ).stripWhiteSpace();
		mine = TRUE;
	    } else if ( res[l] == appName[0] || (appClass && res[l] == appClass[0]) ) {
		if (res.mid(l,apnl) == apn && (res[l+apnl] == '.' || res[l+apnl] == '*')) {
		    QCString item = res.mid( l, r - l ).simplifyWhiteSpace();
		    int i = item.find( ":" );
		    key = item.left( i ).stripWhiteSpace().mid(apnl+1).lower();
		    value = item.right( item.length() - i - 1 ).stripWhiteSpace();
		    mine = TRUE;
		} else if (res.mid(l,apcl) == apc && (res[l+apcl] == '.' || res[l+apcl] == '*')) {
		    QCString item = res.mid( l, r - l ).simplifyWhiteSpace();
		    int i = item.find( ":" );
		    key = item.left( i ).stripWhiteSpace().mid(apcl+1).lower();
		    value = item.right( item.length() - i - 1 ).stripWhiteSpace();
		    mine = TRUE;
		}
	    }

	    if ( mine ) {
		if ( !font && key == "systemfont")
		    sysFont = value.left( value.findRev(':') ).copy();
		if ( !font && key == "font")
		    resFont = value.copy();
		else if  ( !fg &&  key == "foreground" )
		    resFG = value.copy();
		else if ( !bg && key == "background")
		    resBG = value.copy();
		else if ( key == "guieffects")
		    resEF = value.copy();
		// NOTE: if you add more, change the [fbg] stuff above
	    }

	    l = r + 1;
	}
    }
    if ( !sysFont.isEmpty() )
	resFont = sysFont;
    if ( resFont.isEmpty() )
	resFont = font;
    if ( resFG.isEmpty() )
	resFG = fg;
    if ( resBG.isEmpty() )
	resBG = bg;
    if ( (!qt_app_has_font || qt_x11_cmdline_font) && !resFont.isEmpty() ) { // set application font
        QFont fnt;
        fnt.setRawName( resFont );

        // the font we get may actually be an alias for another font,
        // so we reset the application font to the real font info.
        if ( ! fnt.exactMatch() ) {
            QFontInfo fontinfo( fnt );
            fnt.setFamily( fontinfo.family() );
            fnt.setRawMode( fontinfo.rawMode() );

            if ( ! fnt.rawMode() ) {
                fnt.setItalic( fontinfo.italic() );
                fnt.setWeight( fontinfo.weight() );
                fnt.setUnderline( fontinfo.underline() );
                fnt.setStrikeOut( fontinfo.strikeOut() );
                fnt.setStyleHint( fontinfo.styleHint() );

                if ( fnt.pointSize() <= 0 && fnt.pixelSize() <= 0 )
                    // size is all wrong... fix it
                    fnt.setPointSize( (int) ( ( fontinfo.pixelSize() * 72. /
                                                (float) QPaintDevice::x11AppDpiY() ) +
                                              0.5 ) );
            }
        }

        if ( fnt != QApplication::font() ) {
            QApplication::setFont( fnt, TRUE );
        }
    }

    if ( button || !resBG.isEmpty() || !resFG.isEmpty() ) {// set app colors
	QColor btn;
	QColor bg;
	QColor fg;
	if ( !resBG.isEmpty() )
	    bg = QColor(QString(resBG));
	else
	    bg = qt_std_pal->active().background();
	if ( !resFG.isEmpty() )
	    fg = QColor(QString(resFG));
	else
	    fg = qt_std_pal->active().foreground();
	if ( button )
	    btn = QColor( button );
	else if ( !resBG.isEmpty() )
	    btn = bg;
	else
	    btn = qt_std_pal->active().button();

	int h,s,v;
	fg.hsv(&h,&s,&v);
	QColor base = Qt::white;
	bool bright_mode = FALSE;
	if (v >= 255-50) {
	    base = btn.dark(150);
	    bright_mode = TRUE;
	}

	QColorGroup cg( fg, btn, btn.light(),
			btn.dark(), btn.dark(150), fg, Qt::white, base, bg );
	if (bright_mode) {
	    cg.setColor( QColorGroup::HighlightedText, base );
	    cg.setColor( QColorGroup::Highlight, Qt::white );
	} else {
	    cg.setColor( QColorGroup::HighlightedText, Qt::white );
	    cg.setColor( QColorGroup::Highlight, Qt::darkBlue );
	}
	QColor disabled( (fg.red()+btn.red())/2,
			 (fg.green()+btn.green())/2,
			 (fg.blue()+btn.blue())/2);
	QColorGroup dcg( disabled, btn, btn.light( 125 ), btn.dark(), btn.dark(150),
			 disabled, Qt::white, Qt::white, bg );
	if (bright_mode) {
	    dcg.setColor( QColorGroup::HighlightedText, base );
	    dcg.setColor( QColorGroup::Highlight, Qt::white );
	} else {
	    dcg.setColor( QColorGroup::HighlightedText, Qt::white );
	    dcg.setColor( QColorGroup::Highlight, Qt::darkBlue );
	}
	QPalette pal( cg, dcg, cg );
	if ( pal != *qt_std_pal && pal != QApplication::palette() )
	    QApplication::setPalette( pal, TRUE );
	*qt_std_pal = pal;
    }

    if ( !resEF.isEmpty() ) {
	QStringList effects = QStringList::split(" ",resEF);
	QApplication::setEffectEnabled( Qt::UI_General,  effects.contains("general") );
	QApplication::setEffectEnabled( Qt::UI_AnimateMenu, effects.contains("animatemenu") );
	QApplication::setEffectEnabled( Qt::UI_FadeMenu, effects.contains("fademenu") );
	QApplication::setEffectEnabled( Qt::UI_AnimateCombo, effects.contains("animatecombo") );
	QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, effects.contains("animatetooltip") );
	QApplication::setEffectEnabled( Qt::UI_FadeTooltip, effects.contains("fadetooltip") );
	QApplication::setEffectEnabled( Qt::UI_AnimateToolBox, effects.contains("animatetoolbox") );
    }
}


static void qt_detect_broken_window_manager()
{
    Atom type;
    int format;
    ulong nitems, after;
    uchar *data = 0;

    // look for SGI's 4Dwm
    int e = XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
                               qt_sgi_desks_manager, 0, 1, False, XA_WINDOW,
                               &type, &format, &nitems, &after, &data);
    if (data)
        XFree(data);

    if (e == Success && type == XA_WINDOW && format == 32 && nitems == 1 && after == 0) {
        // detected SGI 4Dwm
        qt_broken_wm = TRUE;
    }
}


// update the supported array
void qt_get_net_supported()
{
    Atom type;
    int format;
    long offset = 0;
    unsigned long nitems, after;
    unsigned char *data = 0;

    int e = XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_net_supported, 0, 0,
			       False, XA_ATOM, &type, &format, &nitems, &after, &data);
    if (data)
	XFree(data);

    if (qt_net_supported_list)
	delete [] qt_net_supported_list;
    qt_net_supported_list = 0;

    if (e == Success && type == XA_ATOM && format == 32) {
	QBuffer ts;
	ts.open(IO_WriteOnly);

	while (after > 0) {
	    XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_net_supported, offset, 1024,
			       False, XA_ATOM, &type, &format, &nitems, &after, &data);

	    if (type == XA_ATOM && format == 32) {
		ts.writeBlock((const char *) data, nitems * sizeof(long));
		offset += nitems;
	    } else
		after = 0;
	    if (data)
		XFree(data);
	}

	// compute nitems
	QByteArray buffer(ts.buffer());
	nitems = buffer.size() / sizeof(Atom);
	qt_net_supported_list = new Atom[nitems + 1];
	Atom *a = (Atom *) buffer.data();
	uint i;
	for (i = 0; i < nitems; i++)
	    qt_net_supported_list[i] = a[i];
	qt_net_supported_list[nitems] = 0;
    }
}


bool qt_net_supports(Atom atom)
{
    if (! qt_net_supported_list)
	return FALSE;

    bool supported = FALSE;
    int i = 0;
    while (qt_net_supported_list[i] != 0) {
	if (qt_net_supported_list[i++] == atom) {
	    supported = TRUE;
	    break;
	}
    }

    return supported;
}


// update the virtual roots array
void qt_get_net_virtual_roots()
{
    if (qt_net_virtual_root_list)
	delete [] qt_net_virtual_root_list;
    qt_net_virtual_root_list = 0;

    if (! qt_net_supports(qt_net_virtual_roots))
	return;

    Atom type;
    int format;
    long offset = 0;
    unsigned long nitems, after;
    unsigned char *data;

    int e = XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_net_virtual_roots, 0, 0,
			       False, XA_ATOM, &type, &format, &nitems, &after, &data);
    if (data)
	XFree(data);

    if (e == Success && type == XA_ATOM && format == 32) {
	QBuffer ts;
	ts.open(IO_WriteOnly);

	while (after > 0) {
	    XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_net_virtual_roots, offset, 1024,
			       False, XA_ATOM, &type, &format, &nitems, &after, &data);

	    if (type == XA_ATOM && format == 32) {
		ts.writeBlock((const char *) data, nitems * 4);
		offset += nitems;
	    } else
		after = 0;
	    if (data)
		XFree(data);
	}

	// compute nitems
	QByteArray buffer(ts.buffer());
	nitems = buffer.size() / sizeof(Window);
	qt_net_virtual_root_list = new Window[nitems + 1];
	Window *a = (Window *) buffer.data();
	uint i;
	for (i = 0; i < nitems; i++)
	    qt_net_virtual_root_list[i] = a[i];
	qt_net_virtual_root_list[nitems] = 0;
    }
}

void qt_x11_create_wm_client_leader()
{
    if ( qt_x11_wm_client_leader ) return;

    qt_x11_wm_client_leader =
	XCreateSimpleWindow( QPaintDevice::x11AppDisplay(),
			     QPaintDevice::x11AppRootWindow(),
			     0, 0, 1, 1, 0, 0, 0 );

    // set client leader property to itself
    XChangeProperty( QPaintDevice::x11AppDisplay(),
		     qt_x11_wm_client_leader, qt_wm_client_leader,
		     XA_WINDOW, 32, PropModeReplace,
		     (unsigned char *)&qt_x11_wm_client_leader, 1 );

    // If we are session managed, inform the window manager about it
    QCString session = qApp->sessionId().latin1();
    if ( !session.isEmpty() ) {
	XChangeProperty( QPaintDevice::x11AppDisplay(),
			 qt_x11_wm_client_leader, qt_sm_client_id,
			 XA_STRING, 8, PropModeReplace,
			 (unsigned char *)session.data(), session.length() );
    }
}

static void qt_net_update_user_time(QWidget *tlw)
{
    XChangeProperty(QPaintDevice::x11AppDisplay(), tlw->winId(), qt_net_wm_user_time, XA_CARDINAL,
		    32, PropModeReplace, (unsigned char *) &qt_x_user_time, 1);
}

static void qt_check_focus_model()
{
    Window fw = None;
    int unused;
    XGetInputFocus( appDpy, &fw, &unused );
    if ( fw == PointerRoot )
	qt_focus_model = FocusModel_PointerRoot;
    else
	qt_focus_model = FocusModel_Other;
}


/*
  Returns a truecolor visual (if there is one). 8-bit TrueColor visuals
  are ignored, unless the user has explicitly requested -visual TrueColor.
  The SGI X server usually has an 8 bit default visual, but the application
  can also ask for a truecolor visual. This is what we do if
  QApplication::colorSpec() is QApplication::ManyColor.
*/

static Visual *find_truecolor_visual( Display *dpy, int scr, int *depth, int *ncols )
{
    XVisualInfo *vi, rvi;
    int best=0, n, i;
    rvi.c_class = TrueColor;
    rvi.screen  = scr;
    vi = XGetVisualInfo( dpy, VisualClassMask | VisualScreenMask,
			 &rvi, &n );
    if ( vi ) {
	for ( i=0; i<n; i++ ) {
	    if ( vi[i].depth > vi[best].depth )
		best = i;
	}
    }
    Visual *v = DefaultVisual(dpy,scr);
    if ( !vi || (vi[best].visualid == XVisualIDFromVisual(v)) ||
	 (vi[best].depth <= 8 && qt_visual_option != TrueColor) )
	{
	*depth = DefaultDepth(dpy,scr);
	*ncols = DisplayCells(dpy,scr);
    } else {
	v = vi[best].visual;
	*depth = vi[best].depth;
	*ncols = vi[best].colormap_size;
    }
    if ( vi )
	XFree( (char *)vi );
    return v;
}


/*****************************************************************************
  qt_init() - initializes Qt for X11
 *****************************************************************************/

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

// ### This should be static but it isn't because of the friend declaration
// ### in qpaintdevice.h which then should have a static too but can't have
// ### it because "storage class specifiers invalid in friend function
// ### declarations" :-) Ideas anyone?
void qt_init_internal( int *argcptr, char **argv,
		       Display *display, Qt::HANDLE visual, Qt::HANDLE colormap )
{
    setlocale( LC_ALL, "" );		// use correct char set mapping
    setlocale( LC_NUMERIC, "C" );	// make sprintf()/scanf() work

    if ( display ) {
	// Qt part of other application

	appForeignDpy = TRUE;
	appDpy  = display;

	// Set application name and class
	appName = qstrdup( "Qt-subapplication" );
	char *app_class = 0;
	if (argv) {
	    const char* p = strrchr( argv[0], '/' );
	    app_class = qstrdup(p ? p + 1 : argv[0]);
	    if (app_class[0])
		app_class[0] = toupper(app_class[0]);
	}
	appClass = app_class;

	// Install default error handlers
	original_x_errhandler = XSetErrorHandler( qt_x_errhandler );
	original_xio_errhandler = XSetIOErrorHandler( qt_xio_errhandler );
    } else {
	// Qt controls everything (default)

	int argc = *argcptr;
	int j;

	// Install default error handlers
	original_x_errhandler = XSetErrorHandler( qt_x_errhandler );
	original_xio_errhandler = XSetIOErrorHandler( qt_xio_errhandler );

	// Set application name and class
	char *app_class = 0;
	if (argv) {
	    const char *p = strrchr( argv[0], '/' );
	    appName = p ? p + 1 : argv[0];
	    app_class = qstrdup(appName);
	    if (app_class[0])
		app_class[0] = toupper(app_class[0]);
	}
	appClass = app_class;

	// Get command line params
	j = argc ? 1 : 0;
	for ( int i=1; i<argc; i++ ) {
	    if ( argv[i] && *argv[i] != '-' ) {
		argv[j++] = argv[i];
		continue;
	    }
	    QCString arg = argv[i];
	    if ( arg == "-display" ) {
		if ( ++i < argc )
		    appDpyName = argv[i];
	    } else if ( arg == "-fn" || arg == "-font" ) {
		if ( ++i < argc ) {
		    appFont = argv[i];
		    qt_x11_cmdline_font = true;
		}
	    } else if ( arg == "-bg" || arg == "-background" ) {
		if ( ++i < argc )
		    appBGCol = argv[i];
	    } else if ( arg == "-btn" || arg == "-button" ) {
		if ( ++i < argc )
		    appBTNCol = argv[i];
	    } else if ( arg == "-fg" || arg == "-foreground" ) {
		if ( ++i < argc )
		    appFGCol = argv[i];
	    } else if ( arg == "-name" ) {
		if ( ++i < argc )
		    appName = argv[i];
	    } else if ( arg == "-title" ) {
		if ( ++i < argc )
		    mwTitle = argv[i];
	    } else if ( arg == "-geometry" ) {
		if ( ++i < argc )
		    mwGeometry = argv[i];
		//Ming-Che 10/10
	    } else if ( arg == "-im" ) {
		if ( ++i < argc )
		    ximServer = argv[i];
	    } else if ( arg == "-noxim" ) {
		noxim=TRUE;
		//
	    } else if ( arg == "-iconic" ) {
		mwIconic = !mwIconic;
	    } else if ( arg == "-ncols" ) {   // xv and netscape use this name
		if ( ++i < argc )
		    qt_ncols_option = QMAX(0,atoi(argv[i]));
	    } else if ( arg == "-visual" ) {  // xv and netscape use this name
		if ( ++i < argc ) {
		    QCString s = QCString(argv[i]).lower();
		    if ( s == "truecolor" ) {
			qt_visual_option = TrueColor;
		    } else {
			// ### Should we honor any others?
		    }
		}
#ifndef QT_NO_XIM
	    } else if ( arg == "-inputstyle" ) {
		if ( ++i < argc ) {
		    QCString s = QCString(argv[i]).lower();
		    if ( s == "onthespot" )
			xim_preferred_style = XIMPreeditCallbacks |
					      XIMStatusNothing;
		    else if ( s == "overthespot" )
			xim_preferred_style = XIMPreeditPosition |
					      XIMStatusNothing;
		    else if ( s == "offthespot" )
			xim_preferred_style = XIMPreeditArea |
					      XIMStatusArea;
		    else if ( s == "root" )
			xim_preferred_style = XIMPreeditNothing |
					      XIMStatusNothing;
		}
#endif
	    } else if ( arg == "-cmap" ) {    // xv uses this name
		qt_cmap_option = TRUE;
	    }
#if defined(QT_DEBUG)
	    else if ( arg == "-sync" )
		appSync = !appSync;
	    else if ( arg == "-nograb" )
		appNoGrab = !appNoGrab;
	    else if ( arg == "-dograb" )
		appDoGrab = !appDoGrab;
#endif
	    else
		argv[j++] = argv[i];
	}

	*argcptr = j;

#if defined(QT_DEBUG) && defined(Q_OS_LINUX)
	if ( !appNoGrab && !appDoGrab ) {
	    QCString s;
	    s.sprintf( "/proc/%d/cmdline", getppid() );
	    QFile f( s );
	    if ( f.open( IO_ReadOnly ) ) {
		s.truncate( 0 );
		int c;
		while ( (c = f.getch()) > 0 ) {
		    if ( c == '/' )
			s.truncate( 0 );
		    else
			s += (char)c;
		}
		if ( s == "gdb" ) {
		    appNoGrab = TRUE;
		    qDebug( "Qt: gdb: -nograb added to command-line options.\n"
			    "\t Use the -dograb option to enforce grabbing." );
		}
		f.close();
	    }
	}
#endif
	// Connect to X server

	if( qt_is_gui_used ) {
	    if ( ( appDpy = XOpenDisplay(appDpyName) ) == 0 ) {
		qWarning( "%s: cannot connect to X server %s", appName,
			  XDisplayName(appDpyName) );
		qApp = 0;
		exit( 1 );
	    }

	    if ( appSync )				// if "-sync" argument
		XSynchronize( appDpy, TRUE );
	}
    }
    // Common code, regardless of whether display is foreign.

    // Get X parameters

    if( qt_is_gui_used ) {
	appScreen = DefaultScreen(appDpy);
	appScreenCount = ScreenCount(appDpy);

	QPaintDevice::x_appdisplay = appDpy;
	QPaintDevice::x_appscreen = appScreen;

	// allocate the arrays for the QPaintDevice data
	QPaintDevice::x_appdepth_arr = new int[ appScreenCount ];
	QPaintDevice::x_appcells_arr = new int[ appScreenCount ];
	QPaintDevice::x_approotwindow_arr = new Qt::HANDLE[ appScreenCount ];
	QPaintDevice::x_appcolormap_arr = new Qt::HANDLE[ appScreenCount ];
	QPaintDevice::x_appdefcolormap_arr = new bool[ appScreenCount ];
	QPaintDevice::x_appvisual_arr = new void*[ appScreenCount ];
	QPaintDevice::x_appdefvisual_arr = new bool[ appScreenCount ];
	Q_CHECK_PTR( QPaintDevice::x_appdepth_arr );
	Q_CHECK_PTR( QPaintDevice::x_appcells_arr );
	Q_CHECK_PTR( QPaintDevice::x_approotwindow_arr );
	Q_CHECK_PTR( QPaintDevice::x_appcolormap_arr );
	Q_CHECK_PTR( QPaintDevice::x_appdefcolormap_arr );
	Q_CHECK_PTR( QPaintDevice::x_appvisual_arr );
	Q_CHECK_PTR( QPaintDevice::x_appdefvisual_arr );

	int screen;
	QString serverVendor( ServerVendor( appDpy) );
	if (serverVendor.contains("XFree86") && VendorRelease(appDpy) < 40300000)
	    qt_hebrew_keyboard_hack = TRUE;

	for ( screen = 0; screen < appScreenCount; ++screen ) {
	    QPaintDevice::x_appdepth_arr[ screen ] = DefaultDepth(appDpy, screen);
	    QPaintDevice::x_appcells_arr[ screen ] = DisplayCells(appDpy, screen);
	    QPaintDevice::x_approotwindow_arr[ screen ] = RootWindow(appDpy, screen);

	    // setup the visual and colormap for each screen
	    Visual *vis = 0;
	    if ( visual && screen == appScreen ) {
		// use the provided visual on the default screen only
		vis = (Visual *) visual;

		// figure out the depth of the visual we are using
		XVisualInfo *vi, rvi;
		int n;
		rvi.visualid = XVisualIDFromVisual(vis);
		rvi.screen  = screen;
		vi = XGetVisualInfo( appDpy, VisualIDMask | VisualScreenMask, &rvi, &n );
		if (vi) {
		    QPaintDevice::x_appdepth_arr[ screen ] = vi->depth;
		    QPaintDevice::x_appcells_arr[ screen ] = vi->visual->map_entries;
		    QPaintDevice::x_appvisual_arr[ screen ] = vi->visual;
		    QPaintDevice::x_appdefvisual_arr[ screen ] = FALSE;
		    XFree(vi);
		} else {
		    // couldn't get info about the visual, use the default instead
		    vis = 0;
		}
	    }

	    if (!vis) {
		// use the default visual
		vis = DefaultVisual(appDpy, screen);
		QPaintDevice::x_appdefvisual_arr[ screen ] = TRUE;

		if ( qt_visual_option == TrueColor ||
		     QApplication::colorSpec() == QApplication::ManyColor ) {
		    // find custom visual

		    int d, c;
		    vis = find_truecolor_visual( appDpy, screen, &d, &c );
		    QPaintDevice::x_appdepth_arr[ screen ] = d;
		    QPaintDevice::x_appcells_arr[ screen ] = c;

		    QPaintDevice::x_appvisual_arr[ screen ] = vis;
		    QPaintDevice::x_appdefvisual_arr[ screen ] =
			(XVisualIDFromVisual(vis) ==
			 XVisualIDFromVisual(DefaultVisual(appDpy, screen)));
		}

		QPaintDevice::x_appvisual_arr[ screen ] = vis;
	    }

	    // we assume that 8bpp == pseudocolor, but this is not
	    // always the case (according to the X server), so we need
	    // to make sure that our internal data is setup in a way
	    // that is compatible with our assumptions
	    if ( vis->c_class == TrueColor &&
		 QPaintDevice::x_appdepth_arr[ screen ] == 8 &&
		 QPaintDevice::x_appcells_arr[ screen ] == 8 )
		QPaintDevice::x_appcells_arr[ screen ] = 256;

	    if ( colormap && screen == appScreen ) {
		// use the provided colormap for the default screen only
		QPaintDevice::x_appcolormap_arr[ screen ] = colormap;
		QPaintDevice::x_appdefcolormap_arr[ screen ] = FALSE;
	    } else {
		if ( vis->c_class == TrueColor ) {
		    QPaintDevice::x_appdefcolormap_arr[ screen ] =
			QPaintDevice::x_appdefvisual_arr[ screen ];
		} else {
		    QPaintDevice::x_appdefcolormap_arr[ screen ] =
			!qt_cmap_option && QPaintDevice::x_appdefvisual_arr[ screen ];
		}

		if ( QPaintDevice::x_appdefcolormap_arr[ screen ] ) {
		    // use default colormap
		    XStandardColormap *stdcmap;
		    VisualID vid =
			XVisualIDFromVisual((Visual *)
					    QPaintDevice::x_appvisual_arr[ screen ]);
		    int i, count;

		    QPaintDevice::x_appcolormap_arr[ screen ] = 0;

		    if ( ! serverVendor.contains( "Hewlett-Packard" ) ) {
			// on HPUX 10.20 local displays, the RGB_DEFAULT_MAP colormap
			// doesn't give us correct colors. Why this happens, I have
			// no clue, so we disable this for HPUX
			if (XGetRGBColormaps(appDpy,
					     QPaintDevice::x11AppRootWindow( screen ),
					     &stdcmap, &count, XA_RGB_DEFAULT_MAP)) {
			    i = 0;
			    while (i < count &&
				   QPaintDevice::x_appcolormap_arr[ screen ] == 0) {
				if (stdcmap[i].visualid == vid) {
				    QPaintDevice::x_appcolormap_arr[ screen ] =
					stdcmap[i].colormap;
				}
				i++;
			    }

			    XFree( (char *)stdcmap );
			}
		    }

		    if (QPaintDevice::x_appcolormap_arr[ screen ] == 0) {
			QPaintDevice::x_appcolormap_arr[ screen ] =
			    DefaultColormap(appDpy, screen);
		    }
		} else {
		    // create a custom colormap
		    QPaintDevice::x_appcolormap_arr[ screen ] =
			XCreateColormap(appDpy, QPaintDevice::x11AppRootWindow( screen ),
					vis, AllocNone);
		}
	    }
	}

	// Set X paintdevice parameters for the default screen
	QPaintDevice::x_appdepth = QPaintDevice::x_appdepth_arr[ appScreen ];
	QPaintDevice::x_appcells = QPaintDevice::x_appcells_arr[ appScreen ];
	QPaintDevice::x_approotwindow = QPaintDevice::x_approotwindow_arr[ appScreen ];
	QPaintDevice::x_appcolormap = QPaintDevice::x_appcolormap_arr[ appScreen ];
	QPaintDevice::x_appdefcolormap = QPaintDevice::x_appdefcolormap_arr[ appScreen ];
	QPaintDevice::x_appvisual = QPaintDevice::x_appvisual_arr[ appScreen ];
	QPaintDevice::x_appdefvisual = QPaintDevice::x_appdefvisual_arr[ appScreen ];

	// Support protocols

	qt_x11_intern_atom( "WM_PROTOCOLS", &qt_wm_protocols );
	qt_x11_intern_atom( "WM_DELETE_WINDOW", &qt_wm_delete_window );
	qt_x11_intern_atom( "WM_STATE", &qt_wm_state );
	qt_x11_intern_atom( "WM_CHANGE_STATE", &qt_wm_change_state );
	qt_x11_intern_atom( "WM_TAKE_FOCUS", &qt_wm_take_focus );
	qt_x11_intern_atom( "WM_CLIENT_LEADER", &qt_wm_client_leader);
	qt_x11_intern_atom( "WM_WINDOW_ROLE", &qt_window_role);
	qt_x11_intern_atom( "SM_CLIENT_ID", &qt_sm_client_id);
	qt_x11_intern_atom( "CLIPBOARD", &qt_xa_clipboard );
	qt_x11_intern_atom( "RESOURCE_MANAGER", &qt_resource_manager );
	qt_x11_intern_atom( "INCR", &qt_x_incr );
	qt_x11_intern_atom( "_XSETROOT_ID", &qt_xsetroot_id );
	qt_x11_intern_atom( "_QT_SELECTION", &qt_selection_property );
	qt_x11_intern_atom( "_QT_CLIPBOARD_SENTINEL", &qt_clipboard_sentinel );
	qt_x11_intern_atom( "_QT_SELECTION_SENTINEL", &qt_selection_sentinel );
	qt_x11_intern_atom( "_QT_SCROLL_DONE", &qt_qt_scrolldone );
	qt_x11_intern_atom( "_QT_INPUT_ENCODING", &qt_input_encoding );
	qt_x11_intern_atom( "_QT_SIZEGRIP", &qt_sizegrip );
	qt_x11_intern_atom( "_NET_WM_CONTEXT_HELP", &qt_net_wm_context_help );
	qt_x11_intern_atom( "_NET_WM_PING", &qt_net_wm_ping );
	qt_x11_intern_atom( "_MOTIF_WM_HINTS", &qt_xa_motif_wm_hints );
	qt_x11_intern_atom( "DTWM_IS_RUNNING", &qt_cde_running );
	qt_x11_intern_atom( "KWIN_RUNNING", &qt_kwin_running );
	qt_x11_intern_atom( "KWM_RUNNING", &qt_kwm_running );
	qt_x11_intern_atom( "GNOME_BACKGROUND_PROPERTIES", &qt_gbackground_properties );

	QString atomname("_QT_SETTINGS_TIMESTAMP_");
	atomname += XDisplayName(appDpyName);
	qt_x11_intern_atom( atomname.latin1(), &qt_settings_timestamp );

	qt_x11_intern_atom( "_NET_SUPPORTED", &qt_net_supported );
	qt_x11_intern_atom( "_NET_VIRTUAL_ROOTS", &qt_net_virtual_roots );
	qt_x11_intern_atom( "_NET_WORKAREA", &qt_net_workarea );
	qt_x11_intern_atom( "_NET_WM_STATE", &qt_net_wm_state );
	qt_x11_intern_atom( "_NET_WM_STATE_MODAL", &qt_net_wm_state_modal );
	qt_x11_intern_atom( "_NET_WM_STATE_MAXIMIZED_VERT", &qt_net_wm_state_max_v );
	qt_x11_intern_atom( "_NET_WM_STATE_MAXIMIZED_HORZ", &qt_net_wm_state_max_h );
	qt_x11_intern_atom( "_NET_WM_STATE_FULLSCREEN", &qt_net_wm_state_fullscreen );
	qt_x11_intern_atom( "_NET_WM_STATE_ABOVE", &qt_net_wm_state_above );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE", &qt_net_wm_window_type );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_NORMAL", &qt_net_wm_window_type_normal );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_DIALOG", &qt_net_wm_window_type_dialog );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_TOOLBAR", &qt_net_wm_window_type_toolbar );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_MENU", &qt_net_wm_window_type_menu );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_UTILITY", &qt_net_wm_window_type_utility );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_SPLASH", &qt_net_wm_window_type_splash );
	qt_x11_intern_atom( "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE", &qt_net_wm_window_type_override );
	qt_x11_intern_atom( "_KDE_NET_WM_FRAME_STRUT", &qt_net_wm_frame_strut );
	qt_x11_intern_atom( "_NET_WM_STATE_STAYS_ON_TOP",
			    &qt_net_wm_state_stays_on_top );
	qt_x11_intern_atom( "_NET_WM_PID", &qt_net_wm_pid );
	qt_x11_intern_atom( "_NET_WM_USER_TIME", &qt_net_wm_user_time );
	qt_x11_intern_atom( "ENLIGHTENMENT_DESKTOP", &qt_enlightenment_desktop );
	qt_x11_intern_atom( "_NET_WM_NAME", &qt_net_wm_name );
	qt_x11_intern_atom( "_NET_WM_ICON_NAME", &qt_net_wm_icon_name );
	qt_x11_intern_atom( "UTF8_STRING", &qt_utf8_string );
        qt_x11_intern_atom( "_SGI_DESKS_MANAGER", &qt_sgi_desks_manager );

	qt_xdnd_setup();
	qt_x11_motifdnd_init();

	// Finally create all atoms
	qt_x11_process_intern_atoms();

        // look for broken window managers
        qt_detect_broken_window_manager();

	// initialize NET lists
	qt_get_net_supported();
	qt_get_net_virtual_roots();

#ifndef QT_NO_XRANDR
	// See if XRandR is supported on the connected display
	int xrandr_errorbase;
	Q_UNUSED( xrandr_eventbase );
	if ( XRRQueryExtension( appDpy, &xrandr_eventbase, &xrandr_errorbase ) ) {
	    // XRandR is supported
	    qt_use_xrandr = TRUE;
	}
#endif // QT_NO_XRANDR

#ifndef QT_NO_XRENDER
	// See if XRender is supported on the connected display
	int xrender_eventbase, xrender_errorbase;
	if (XRenderQueryExtension(appDpy, &xrender_eventbase, &xrender_errorbase)) {
	    // XRender is supported, let's see if we have a PictFormat for the
	    // default visual
	    XRenderPictFormat *format =
		XRenderFindVisualFormat(appDpy,
					(Visual *) QPaintDevice::x_appvisual);
	    qt_use_xrender = (format != 0) && (QPaintDevice::x_appdepth != 8);
	}
#endif // QT_NO_XRENDER

#ifndef QT_NO_XKB
	// If XKB is detected, set the GrabsUseXKBState option so input method
	// compositions continue to work (ie. deadkeys)
	unsigned int state = XkbPCF_GrabsUseXKBStateMask;
	(void) XkbSetPerClientControls(appDpy, state, &state);
#endif

#if !defined(QT_NO_XFTFREETYPE)
	// defined in qfont_x11.cpp
	extern bool qt_has_xft;
#ifndef QT_XFT2
        if (!qt_use_xrender)
            qt_has_xft = FALSE;
        else
#endif
            qt_has_xft = XftInit(0) && XftInitFtLibrary();

        if (qt_has_xft) {
            char *dpi_str = XGetDefault(appDpy, "Xft", "dpi");
            if (dpi_str) {
                // use a custom DPI
                char *end = 0;
                int dpi = strtol(dpi_str, &end, 0);
                if (dpi_str != end) {
                    for (int s = 0; s < ScreenCount(appDpy); ++s) {
                        QPaintDevice::x11SetAppDpiX(dpi, s);
                        QPaintDevice::x11SetAppDpiY(dpi, s);
                    }
                }
            }
        }
#endif // QT_NO_XFTFREETYPE

	// look at the modifier mapping, and get the correct masks for alt/meta
	// find the alt/meta masks
	XModifierKeymap *map = XGetModifierMapping(appDpy);
	if (map) {
	    int i, maskIndex = 0, mapIndex = 0;
	    for (maskIndex = 0; maskIndex < 8; maskIndex++) {
		for (i = 0; i < map->max_keypermod; i++) {
		    if (map->modifiermap[mapIndex]) {
			KeySym sym =
			    XKeycodeToKeysym(appDpy, map->modifiermap[mapIndex], 0);
			if ( qt_alt_mask == 0 &&
			     ( sym == XK_Alt_L || sym == XK_Alt_R ) ) {
			    qt_alt_mask = 1 << maskIndex;
			}
			if ( qt_meta_mask == 0 &&
			     (sym == XK_Meta_L || sym == XK_Meta_R ) ) {
			    qt_meta_mask = 1 << maskIndex;
			}
		    }
		    mapIndex++;
		}
	    }

	    // not look for mode_switch in qt_alt_mask and qt_meta_mask - if it is
	    // present in one or both, then we set qt_mode_switch_remove_mask.
	    // see QETWidget::translateKeyEventInternal for an explanation
	    // of why this is needed
	    mapIndex = 0;
	    for ( maskIndex = 0; maskIndex < 8; maskIndex++ ) {
		if ( qt_alt_mask  != ( 1 << maskIndex ) &&
		     qt_meta_mask != ( 1 << maskIndex ) ) {
		    for ( i = 0; i < map->max_keypermod; i++ )
			mapIndex++;
		    continue;
		}

		for ( i = 0; i < map->max_keypermod; i++ ) {
		    if ( map->modifiermap[ mapIndex ] ) {
			KeySym sym =
			    XKeycodeToKeysym( appDpy, map->modifiermap[ mapIndex ], 0 );
			if ( sym == XK_Mode_switch ) {
			    qt_mode_switch_remove_mask |= 1 << maskIndex;
			}
		    }
		    mapIndex++;
		}
	    }

	    XFreeModifiermap(map);
	} else {
	    // assume defaults
	    qt_alt_mask = Mod1Mask;
	    qt_meta_mask = Mod4Mask;
	    qt_mode_switch_remove_mask = 0;
	}

	// Misc. initialization

	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();
    }

#if defined(QT_THREAD_SUPPORT)
    QThread::initialize();
#endif

    if( qt_is_gui_used ) {
	qApp->setName( appName );

	int screen;
	for ( screen = 0; screen < appScreenCount; ++screen ) {
	    XSelectInput( appDpy, QPaintDevice::x11AppRootWindow( screen ),
			  KeymapStateMask | EnterWindowMask | LeaveWindowMask |
			  PropertyChangeMask );

#ifndef QT_NO_XRANDR
	    if (qt_use_xrandr)
		XRRSelectInput( appDpy, QPaintDevice::x11AppRootWindow( screen ), True );
#endif // QT_NO_XRANDR
	}
    }

    if ( qt_is_gui_used ) {
	qt_set_input_encoding();

	qt_set_x11_resources( appFont, appFGCol, appBGCol, appBTNCol);

	// be smart about the size of the default font. most X servers have helvetica
	// 12 point available at 2 resolutions:
	//     75dpi (12 pixels) and 100dpi (17 pixels).
	// At 95 DPI, a 12 point font should be 16 pixels tall - in which case a 17
	// pixel font is a closer match than a 12 pixel font
	int ptsz =
	    (int) ( ( ( QPaintDevice::x11AppDpiY() >= 95 ? 17. : 12. ) *
		      72. / (float) QPaintDevice::x11AppDpiY() ) + 0.5 );

	if ( !qt_app_has_font && !qt_x11_cmdline_font ) {
	    QFont f( "Helvetica", ptsz );
	    QApplication::setFont( f );
	}

#ifndef QT_NO_XIM
	if ( ! xim_preferred_style ) // no configured input style, use the default
	    xim_preferred_style = xim_default_style;

	qt_xim = 0;
	QString ximServerName(ximServer);
	if (ximServer)
	    ximServerName.prepend("@im=");
	else
	    ximServerName = "";

	if ( !XSupportsLocale() )
	    qWarning("Qt: Locales not supported on X server");

#ifdef USE_X11R6_XIM
	else if ( XSetLocaleModifiers (ximServerName.ascii()) == 0 )
	    qWarning( "Qt: Cannot set locale modifiers: %s",
		      ximServerName.ascii());
	else if (! noxim)
	    XRegisterIMInstantiateCallback(appDpy, 0, 0, 0,
					   (XIMProc) xim_create_callback, 0);
#else // !USE_X11R6_XIM
	else if ( XSetLocaleModifiers ("") == 0 )
	    qWarning("Qt: Cannot set locale modifiers");
	else if (! noxim)
	    QApplication::create_xim();
#endif // USE_X11R6_XIM
#endif // QT_NO_XIM

#if defined (QT_TABLET_SUPPORT)
	int ndev,
	    i,
	    j;
	bool gotStylus,
	    gotEraser;
	XDeviceInfo *devices, *devs;
	XInputClassInfo *ip;
	XAnyClassPtr any;
	XValuatorInfoPtr v;
	XAxisInfoPtr a;
	XDevice *dev;
	XEventClass *ev_class;
	int curr_event_count;

#if !defined(Q_OS_IRIX)
	// XFree86 divides a stylus and eraser into 2 devices, so we must do for both...
	const QString XFREENAMESTYLUS = "stylus";
	const QString XFREENAMEPEN = "pen";
	const QString XFREENAMEERASER = "eraser";
#endif

	devices = XListInputDevices( appDpy, &ndev);
	if ( devices == NULL ) {
	    qWarning( "Failed to get list of devices" );
	    ndev = -1;
	}
	dev = NULL;
	for ( devs = devices, i = 0; i < ndev; i++, devs++ ) {
	    gotEraser = FALSE;
#if defined(Q_OS_IRIX)

	    gotStylus = ( !strncmp(devs->name,
				   WACOM_NAME, sizeof(WACOM_NAME) - 1) );
#else
	    QString devName = devs->name;
	    devName = devName.lower();
	    gotStylus = ( devName.startsWith(XFREENAMEPEN)
			  || devName.startsWith(XFREENAMESTYLUS) );
	    if ( !gotStylus )
		gotEraser = devName.startsWith( XFREENAMEERASER );

#endif
	    if ( gotStylus || gotEraser ) {
		// I only wanted to do this once, so wrap pointers around these
		curr_event_count = 0;

		if ( gotStylus ) {
		    devStylus = XOpenDevice( appDpy, devs->id );
		    dev = devStylus;
		    ev_class = event_list_stylus;
		} else if ( gotEraser ) {
		    devEraser = XOpenDevice( appDpy, devs->id );
		    dev = devEraser;
		    ev_class = event_list_eraser;
		}
		if ( dev == NULL ) {
		    qWarning( "Failed to open device" );
		} else {
		    if ( dev->num_classes > 0 ) {
			for ( ip = dev->classes, j = 0; j < devs->num_classes;
			      ip++, j++ ) {
			    switch ( ip->input_class ) {
			    case KeyClass:
				DeviceKeyPress( dev, xinput_key_press,
						ev_class[curr_event_count] );
				curr_event_count++;
				DeviceKeyRelease( dev, xinput_key_release,
						  ev_class[curr_event_count] );
				curr_event_count++;
				break;
			    case ButtonClass:
				DeviceButtonPress( dev, xinput_button_press,
						   ev_class[curr_event_count] );
				curr_event_count++;
				DeviceButtonRelease( dev, xinput_button_release,
						     ev_class[curr_event_count] );
				curr_event_count++;
				break;
			    case ValuatorClass:
				// I'm only going to be interested in motion when the
				// stylus is already down anyway!
				DeviceMotionNotify( dev, xinput_motion,
						    ev_class[curr_event_count] );
				curr_event_count++;
				break;
			    default:
				break;
			    }
			}
		    }
		}
		// get the min/max value for pressure!
		any = (XAnyClassPtr) ( devs->inputclassinfo );
		if ( dev == devStylus ) {
		    qt_curr_events_stylus = curr_event_count;
		    for (j = 0; j < devs->num_classes; j++) {
			if ( any->c_class == ValuatorClass ) {
			    v = (XValuatorInfoPtr) any;
			    a = (XAxisInfoPtr) ((char *) v +
						sizeof (XValuatorInfo));
#if defined (Q_OS_IRIX)
			    max_pressure = a[WAC_PRESSURE_I].max_value;
#else
			    max_pressure = a[2].max_value;
#endif
			    // got the max pressure no need to go further...
			    break;
			}
			any = (XAnyClassPtr) ((char *) any + any->length);
		    }
		} else {
		    qt_curr_events_eraser = curr_event_count;
		}
		// at this point we are assuming there is only one
		// wacom device...
#if defined (Q_OS_IRIX)
		if ( devStylus != NULL ) {
#else
		    if ( devStylus != NULL && devEraser != NULL ) {
#endif
			break;
		    }
		}
	    } // end for loop
	    XFreeDeviceList( devices );
#endif // QT_TABLET_SUPPORT

	} else {
	    // read some non-GUI settings when not using the X server...

	    if ( QApplication::desktopSettingsAware() ) {
		QSettings settings;

		// read library (ie. plugin) path list
		QString libpathkey = QString("/qt/%1.%2/libraryPath")
				     .arg( QT_VERSION >> 16 )
				     .arg( (QT_VERSION & 0xff00 ) >> 8 );
		QStringList pathlist =
		    settings.readListEntry(libpathkey, ':');
		if (! pathlist.isEmpty()) {
		    QStringList::ConstIterator it = pathlist.begin();
		    while (it != pathlist.end())
			QApplication::addLibraryPath(*it++);
		}

		QString defaultcodec = settings.readEntry("/qt/defaultCodec", "none");
		if (defaultcodec != "none") {
		    QTextCodec *codec = QTextCodec::codecForName(defaultcodec);
		    if (codec)
			qApp->setDefaultCodec(codec);
		}

		qt_resolve_symlinks =
		    settings.readBoolEntry("/qt/resolveSymlinks", TRUE);
	    }
	}
    }


#ifndef QT_NO_STYLE
    // run-time search for default style
void QApplication::x11_initialize_style()
{
    Atom type;
    int format;
    unsigned long length, after;
    uchar *data;
    if ( !app_style &&
	 XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(), qt_kwin_running,
			     0, 1, False, AnyPropertyType, &type, &format, &length,
			     &after, &data ) == Success && length ) {
	if ( data ) XFree( (char *)data );
	// kwin is there. check if KDE's styles are available,
	// otherwise use windows style
	if ( (app_style = QStyleFactory::create("highcolor") ) == 0 )
	    app_style = QStyleFactory::create("windows");
    }
    if ( !app_style &&
	 XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(), qt_kwm_running,
			     0, 1, False, AnyPropertyType, &type, &format, &length,
			     &after, &data ) == Success && length ) {
	if ( data ) XFree( (char *)data );
	app_style = QStyleFactory::create("windows");
    }
    if ( !app_style &&
	 XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(), qt_cde_running,
			     0, 1, False, AnyPropertyType, &type, &format, &length,
			     &after, &data ) == Success && length ) {
	// DTWM is running, meaning most likely CDE is running...
	if ( data ) XFree( (char *) data );
	app_style = QStyleFactory::create( "cde" );
    }
    // maybe another desktop?
    if ( !app_style &&
	 XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(),
			     qt_gbackground_properties, 0, 1, False, AnyPropertyType,
			     &type, &format, &length, &after, &data ) == Success &&
	 length ) {
	if ( data ) XFree( (char *)data );
	// default to MotifPlus with hovering
	app_style = QStyleFactory::create("motifplus" );
    }
}
#endif

void qt_init( int *argcptr, char **argv, QApplication::Type )
{
    qt_init_internal( argcptr, argv, 0, 0, 0 );
}

void qt_init( Display *display, Qt::HANDLE visual, Qt::HANDLE colormap )
{
    qt_init_internal( 0, 0, display, visual, colormap );
}


/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    appliedstamp = 0;

    if ( app_save_rootinfo )			// root window must keep state
	qt_save_rootinfo();

    if ( qt_is_gui_used ) {
	QPixmapCache::clear();
	QPainter::cleanup();
	QCursor::cleanup();
	QFont::cleanup();
	QColor::cleanup();
	QSharedDoubleBuffer::cleanup();
    }
#if defined(QT_THREAD_SUPPORT)
    QThread::cleanup();
#endif

#if defined (QT_TABLET_SUPPORT)
    if ( devStylus != NULL )
	XCloseDevice( appDpy, devStylus );
    if ( devEraser != NULL )
	XCloseDevice( appDpy, devEraser );
#endif

#if !defined(QT_NO_XIM)
    if ( qt_xim )
	QApplication::close_xim();
#endif

    if ( qt_is_gui_used ) {
	int screen;
	for ( screen = 0; screen < appScreenCount; screen++ ) {
	    if ( ! QPaintDevice::x11AppDefaultColormap( screen ) )
		XFreeColormap( QPaintDevice::x11AppDisplay(),
			       QPaintDevice::x11AppColormap( screen ) );
	}
    }

#define QT_CLEANUP_GC(g) if (g) { for (int i=0;i<appScreenCount;i++){if(g[i])XFreeGC(appDpy,g[i]);} delete [] g; g = 0; }
    QT_CLEANUP_GC(app_gc_ro);
    QT_CLEANUP_GC(app_gc_ro_m);
    QT_CLEANUP_GC(app_gc_tmp);
    QT_CLEANUP_GC(app_gc_tmp_m);
#undef QT_CLEANUP_GC

    delete sip_list;
    sip_list = 0;

    // Reset the error handlers
    XSetErrorHandler( original_x_errhandler );
    XSetIOErrorHandler( original_xio_errhandler );

    if ( qt_is_gui_used && !appForeignDpy )
	XCloseDisplay( appDpy );		// close X display
    appDpy = 0;

    qt_x11_wm_client_leader = 0;

    if ( QPaintDevice::x_appdepth_arr )
	delete [] QPaintDevice::x_appdepth_arr;
    if ( QPaintDevice::x_appcells_arr )
	delete [] QPaintDevice::x_appcells_arr;
    if ( QPaintDevice::x_appcolormap_arr )
	delete []QPaintDevice::x_appcolormap_arr;
    if ( QPaintDevice::x_appdefcolormap_arr )
	delete [] QPaintDevice::x_appdefcolormap_arr;
    if ( QPaintDevice::x_appvisual_arr )
	delete [] QPaintDevice::x_appvisual_arr;
    if ( QPaintDevice::x_appdefvisual_arr )
	delete [] QPaintDevice::x_appdefvisual_arr;

    if ( appForeignDpy ) {
	delete [] (char *)appName;
	appName = 0;
	delete [] (char *)appClass;
	appClass = 0;
    }

    if (qt_net_supported_list)
	delete [] qt_net_supported_list;
    qt_net_supported_list = 0;

    if (qt_net_virtual_root_list)
	delete [] qt_net_virtual_root_list;
    qt_net_virtual_root_list = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qt_save_rootinfo()				// save new root info
{
    Atom type;
    int format;
    unsigned long length, after;
    uchar *data;

    if ( qt_xsetroot_id ) {			// kill old pixmap
	if ( XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(),
				 qt_xsetroot_id, 0, 1,
				 True, AnyPropertyType, &type, &format,
				 &length, &after, &data ) == Success ) {
	    if ( type == XA_PIXMAP && format == 32 && length == 1 &&
		 after == 0 && data ) {
		XKillClient( appDpy, *((Pixmap*)data) );
	    }
	    Pixmap dummy = XCreatePixmap( appDpy, QPaintDevice::x11AppRootWindow(),
					  1, 1, 1 );
	    XChangeProperty( appDpy, QPaintDevice::x11AppRootWindow(),
			     qt_xsetroot_id, XA_PIXMAP, 32,
			     PropModeReplace, (uchar *)&dummy, 1 );
	    XSetCloseDownMode( appDpy, RetainPermanent );
	}
    }
    if ( data )
	XFree( (char *)data );
}

void qt_updated_rootinfo()
{
    app_save_rootinfo = TRUE;
}

bool qt_wstate_iconified( WId winid )
{
    Atom type;
    int format;
    unsigned long length, after;
    uchar *data;
    int r = XGetWindowProperty( appDpy, winid, qt_wm_state, 0, 2,
				 False, AnyPropertyType, &type, &format,
				 &length, &after, &data );
    bool iconic = FALSE;
    if ( r == Success && data && format == 32 ) {
	// Q_UINT32 *wstate = (Q_UINT32*)data;
	unsigned long *wstate = (unsigned long *) data;
	iconic = (*wstate == IconicState );
	XFree( (char *)data );
    }
    return iconic;
}

const char *qAppName()				// get application name
{
    return appName;
}

const char *qAppClass()				// get application class
{
    return appClass;
}

Display *qt_xdisplay()				// get current X display
{
    return appDpy;
}

int qt_xscreen()				// get current X screen
{
    return appScreen;
}

// ### REMOVE 4.0
WId qt_xrootwin()				// get X root window
{
    return QPaintDevice::x11AppRootWindow();
}

WId qt_xrootwin( int scrn )			// get X root window for screen
{
    return QPaintDevice::x11AppRootWindow( scrn );
}

bool qt_nograb()				// application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}

static GC create_gc( int scrn, bool monochrome )
{
    GC gc;
    if ( monochrome ) {
	Pixmap pm = XCreatePixmap( appDpy, RootWindow( appDpy, scrn ), 8, 8, 1 );
	gc = XCreateGC( appDpy, pm, 0, 0 );
	XFreePixmap( appDpy, pm );
    } else {
	if ( QPaintDevice::x11AppDefaultVisual( scrn ) ) {
	    gc = XCreateGC( appDpy, RootWindow( appDpy, scrn ), 0, 0 );
	} else {
	    Window w;
	    XSetWindowAttributes a;
	    a.background_pixel = Qt::black.pixel( scrn );
	    a.border_pixel = Qt::black.pixel( scrn );
	    a.colormap = QPaintDevice::x11AppColormap( scrn );
	    w = XCreateWindow( appDpy, RootWindow( appDpy, scrn ), 0, 0, 100, 100,
			       0, QPaintDevice::x11AppDepth( scrn ), InputOutput,
			       (Visual*)QPaintDevice::x11AppVisual( scrn ),
			       CWBackPixel|CWBorderPixel|CWColormap, &a );
	    gc = XCreateGC( appDpy, w, 0, 0 );
	    XDestroyWindow( appDpy, w );
	}
    }
    XSetGraphicsExposures( appDpy, gc, False );
    return gc;
}

GC qt_xget_readonly_gc( int scrn, bool monochrome )	// get read-only GC
{
    if ( scrn < 0 || scrn >= appScreenCount ) {
	qDebug("invalid screen %d %d", scrn, appScreenCount );
	QWidget* bla = 0;
	bla->setName("hello");
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_ro_m )			// create GC for bitmap
	    memset( (app_gc_ro_m = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_ro_m[scrn] )
	    app_gc_ro_m[scrn] = create_gc( scrn, TRUE );
	gc = app_gc_ro_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_ro )
	    memset( (app_gc_ro = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_ro[scrn] )
	    app_gc_ro[scrn] = create_gc( scrn, FALSE );
	gc = app_gc_ro[scrn];
    }
    return gc;
}

GC qt_xget_temp_gc( int scrn, bool monochrome )		// get temporary GC
{
    if ( scrn < 0 || scrn >= appScreenCount ) {
	qDebug("invalid screen (tmp) %d %d", scrn, appScreenCount );
	QWidget* bla = 0;
	bla->setName("hello");
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_tmp_m )			// create GC for bitmap
	    memset( (app_gc_tmp_m = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_tmp_m[scrn] )
	    app_gc_tmp_m[scrn] = create_gc( scrn, TRUE );
	gc = app_gc_tmp_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_tmp )
	    memset( (app_gc_tmp = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_tmp[scrn] )
	    app_gc_tmp[scrn] = create_gc( scrn, FALSE );
	gc = app_gc_tmp[scrn];
    }
    return gc;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

/*!
    \fn QWidget *QApplication::mainWidget() const

    Returns the main application widget, or 0 if there is no main
    widget.

    \sa setMainWidget()
*/

/*!
    Sets the application's main widget to \a mainWidget.

    In most respects the main widget is like any other widget, except
    that if it is closed, the application exits. Note that
    QApplication does \e not take ownership of the \a mainWidget, so
    if you create your main widget on the heap you must delete it
    yourself.

    You need not have a main widget; connecting lastWindowClosed() to
    quit() is an alternative.

    For X11, this function also resizes and moves the main widget
    according to the \e -geometry command-line option, so you should
    set the default geometry (using \l QWidget::setGeometry()) before
    calling setMainWidget().

    \sa mainWidget(), exec(), quit()
*/

void QApplication::setMainWidget( QWidget *mainWidget )
{
#if defined(QT_CHECK_STATE)
    if ( mainWidget && mainWidget->parentWidget() &&
	 ! mainWidget->parentWidget()->isDesktop() )
	qWarning( "QApplication::setMainWidget(): New main widget (%s/%s) "
		  "has a parent!",
		  mainWidget->className(), mainWidget->name() );
#endif
    main_widget = mainWidget;
    if ( main_widget ) {			// give WM command line
	XSetWMProperties( main_widget->x11Display(), main_widget->winId(),
			  0, 0, app_argv, app_argc, 0, 0, 0 );
	if ( mwTitle )
	    XStoreName( main_widget->x11Display(), main_widget->winId(), (char*)mwTitle );
	if ( mwGeometry ) {			// parse geometry
	    int x, y;
	    int w, h;
	    int m = XParseGeometry( (char*)mwGeometry, &x, &y, (uint*)&w, (uint*)&h );
	    QSize minSize = main_widget->minimumSize();
	    QSize maxSize = main_widget->maximumSize();
	    if ( (m & XValue) == 0 )
		x = main_widget->geometry().x();
	    if ( (m & YValue) == 0 )
		y = main_widget->geometry().y();
	    if ( (m & WidthValue) == 0 )
		w = main_widget->width();
	    if ( (m & HeightValue) == 0 )
		h = main_widget->height();
	    w = QMIN(w,maxSize.width());
	    h = QMIN(h,maxSize.height());
	    w = QMAX(w,minSize.width());
	    h = QMAX(h,minSize.height());
	    if ( (m & XNegative) ) {
		x = desktop()->width()  + x - w;
		qt_widget_tlw_gravity = NorthEastGravity;
	    }
	    if ( (m & YNegative) ) {
		y = desktop()->height() + y - h;
		qt_widget_tlw_gravity = (m & XNegative) ? SouthEastGravity : SouthWestGravity;
	    }
	    main_widget->setGeometry( x, y, w, h );
	}
    }
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

extern void qt_x11_enforce_cursor( QWidget * w );

typedef QPtrList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;

/*!
    \fn QCursor *QApplication::overrideCursor()

    Returns the active application override cursor.

    This function returns 0 if no application cursor has been defined
    (i.e. the internal cursor stack is empty).

    \sa setOverrideCursor(), restoreOverrideCursor()
*/

/*!
    Sets the application override cursor to \a cursor.

    Application override cursors are intended for showing the user
    that the application is in a special state, for example during an
    operation that might take some time.

    This cursor will be displayed in all the application's widgets
    until restoreOverrideCursor() or another setOverrideCursor() is
    called.

    Application cursors are stored on an internal stack.
    setOverrideCursor() pushes the cursor onto the stack, and
    restoreOverrideCursor() pops the active cursor off the stack.
    Every setOverrideCursor() must eventually be followed by a
    corresponding restoreOverrideCursor(), otherwise the stack will
    never be emptied.

    If \a replace is TRUE, the new cursor will replace the last
    override cursor (the stack keeps its depth). If \a replace is
    FALSE, the new stack is pushed onto the top of the stack.

    Example:
    \code
	QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
	calculateHugeMandelbrot();              // lunch time...
	QApplication::restoreOverrideCursor();
    \endcode

    \sa overrideCursor(), restoreOverrideCursor(), QWidget::setCursor()
*/

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	Q_CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    Q_CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );

    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that have
	if ( w->testWState( WState_OwnCursor ) )
	    qt_x11_enforce_cursor( w );
	++it;
    }
    XFlush( appDpy );				// make X execute it NOW
}

/*!
    Undoes the last setOverrideCursor().

    If setOverrideCursor() has been called twice, calling
    restoreOverrideCursor() will activate the first cursor set.
    Calling this function a second time restores the original widgets'
    cursors.

    \sa setOverrideCursor(), overrideCursor().
*/

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    if ( QWidget::mapper != 0 && !closingDown() ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// set back to original cursors
	    if ( w->testWState( WState_OwnCursor ) )
		qt_x11_enforce_cursor( w );
	    ++it;
	}
	XFlush( appDpy );
    }
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
    }
}

#endif

/*!
    \fn bool QApplication::hasGlobalMouseTracking()

    Returns TRUE if global mouse tracking is enabled; otherwise
    returns FALSE.

    \sa setGlobalMouseTracking()
*/

/*!
    Enables global mouse tracking if \a enable is TRUE, or disables it
    if \a enable is FALSE.

    Enabling global mouse tracking makes it possible for widget event
    filters or application event filters to get all mouse move events,
    even when no button is depressed. This is useful for special GUI
    elements, e.g. tooltips.

    Global mouse tracking does not affect widgets and their
    mouseMoveEvent(). For a widget to get mouse move events when no
    button is depressed, it must do QWidget::setMouseTracking(TRUE).

    This function uses an internal counter. Each
    setGlobalMouseTracking(TRUE) must have a corresponding
    setGlobalMouseTracking(FALSE):
    \code
	// at this point global mouse tracking is off
	QApplication::setGlobalMouseTracking( TRUE );
	QApplication::setGlobalMouseTracking( TRUE );
	QApplication::setGlobalMouseTracking( FALSE );
	// at this point it's still on
	QApplication::setGlobalMouseTracking( FALSE );
	// but now it's off
    \endcode

    \sa hasGlobalMouseTracking(), QWidget::hasMouseTracking()
*/

void QApplication::setGlobalMouseTracking( bool enable )
{
    bool tellAllWidgets;
    if ( enable ) {
	tellAllWidgets = (++app_tracking == 1);
    } else {
	tellAllWidgets = (--app_tracking == 0);
    }
    if ( tellAllWidgets ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {
	    if ( app_tracking > 0 ) {		// switch on
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setMouseTracking( TRUE );
		    w->clearWState( WState_MouseTracking );
		}
	    } else {				// switch off
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setWState( WState_MouseTracking );
		    w->setMouseTracking( FALSE );
		}
	    }
	    ++it;
	}
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

Window qt_x11_findClientWindow( Window win, Atom property, bool leaf )
{
    Atom   type = None;
    int	   format, i;
    ulong  nitems, after;
    uchar *data;
    Window root, parent, target=0, *children=0;
    uint   nchildren;
    if ( XGetWindowProperty( appDpy, win, property, 0, 0, FALSE, AnyPropertyType,
			     &type, &format, &nitems, &after, &data ) == Success ) {
	if ( data )
	    XFree( (char *)data );
	if ( type )
	    return win;
    }
    if ( !XQueryTree(appDpy,win,&root,&parent,&children,&nchildren) ) {
	if ( children )
	    XFree( (char *)children );
	return 0;
    }
    for ( i=nchildren-1; !target && i >= 0; i-- )
	target = qt_x11_findClientWindow( children[i], property, leaf );
    if ( children )
	XFree( (char *)children );
    return target;
}


/*!
    Returns a pointer to the widget at global screen position \a
    (x, y), or 0 if there is no Qt widget there.

    If \a child is FALSE and there is a child widget at position \a
    (x, y), the top-level widget containing it is returned. If \a child
    is TRUE the child widget at position \a (x, y) is returned.

    This function is normally rather slow.

    \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    int screen = QCursor::x11Screen();
    int lx, ly;

    Window target;
    if ( !XTranslateCoordinates(appDpy,
				QPaintDevice::x11AppRootWindow(screen),
				QPaintDevice::x11AppRootWindow(screen),
				x, y, &lx, &ly, &target) ) {
	return 0;
    }
    if ( !target || target == QPaintDevice::x11AppRootWindow(screen) )
	return 0;
    QWidget *w, *c;
    w = QWidget::find( (WId)target );

    if ( !w ) {
	qt_ignore_badwindow();
	target = qt_x11_findClientWindow( target, qt_wm_state, TRUE );
	if (qt_badwindow() )
	    return 0;
	w = QWidget::find( (WId)target );
#if 0
	if ( !w ) {
	    // Perhaps the widgets at (x,y) is inside a foreign application?
	    // Search all toplevel widgets to see if one is within target
	    QWidgetList *list   = topLevelWidgets();
	    QWidget     *widget = list->first();
	    while ( widget && !w ) {
		Window	ctarget = target;
		if ( widget->isVisible() && !widget->isDesktop() ) {
		    Window wid = widget->winId();
		    while ( ctarget && !w ) {
			XTranslateCoordinates(appDpy,
					      QPaintDevice::x11AppRootWindow(screen),
					      ctarget, x, y, &lx, &ly, &ctarget);
			if ( ctarget == wid ) {
			    // Found
			    w = widget;
			    XTranslateCoordinates(appDpy,
						  QPaintDevice::x11AppRootWindow(screen),
						  ctarget, x, y, &lx, &ly, &ctarget);
			}
		    }
		}
		widget = list->next();
	    }
	    delete list;
	}
#endif
    }
    if ( child && w ) {
	if ( (c = w->childAt( w->mapFromGlobal(QPoint(x, y ) ) ) ) )
	    return c;
    }
    return w;
}

/*!
    \overload QWidget *QApplication::widgetAt( const QPoint &pos, bool child )

    Returns a pointer to the widget at global screen position \a pos,
    or 0 if there is no Qt widget there.

    If \a child is FALSE and there is a child widget at position \a
    pos, the top-level widget containing it is returned. If \a child
    is TRUE the child widget at position \a pos is returned.
*/


/*!
    Flushes the X event queue in the X11 implementation. This normally
    returns almost immediately. Does nothing on other platforms.

    \sa syncX()
*/

void QApplication::flushX()
{
    if ( appDpy )
	XFlush( appDpy );
}

/*!
    Flushes the window system specific event queues.

    If you are doing graphical changes inside a loop that does not
    return to the event loop on asynchronous window systems like X11
    or double buffered window systems like MacOS X, and you want to
    visualize these changes immediately (e.g. Splash Screens), call
    this function.

    \sa flushX() sendPostedEvents() QPainter::flush()
*/

void QApplication::flush()
{
    flushX();
}

/*!
    Synchronizes with the X server in the X11 implementation. This
    normally takes some time. Does nothing on other platforms.

    \sa flushX()
*/

void QApplication::syncX()
{
    if ( appDpy )
	XSync( appDpy, False );			// don't discard events
}


/*!
    Sounds the bell, using the default volume and sound.
*/

void QApplication::beep()
{
    if ( appDpy )
	XBell( appDpy, 0 );
}



/*****************************************************************************
  Special lookup functions for windows that have been reparented recently
 *****************************************************************************/

static QWidgetIntDict *wPRmapper = 0;		// alternative widget mapper

void qPRCreate( const QWidget *widget, Window oldwin )
{						// QWidget::reparent mechanism
    if ( !wPRmapper ) {
	wPRmapper = new QWidgetIntDict;
	Q_CHECK_PTR( wPRmapper );
    }
    wPRmapper->insert( (long)oldwin, widget );	// add old window to mapper
    QETWidget *w = (QETWidget *)widget;
    w->setWState( Qt::WState_Reparented );	// set reparented flag
}

void qPRCleanup( QWidget *widget )
{
    QETWidget *etw = (QETWidget *)widget;
    if ( !(wPRmapper && etw->testWState(Qt::WState_Reparented)) )
	return;					// not a reparented widget
    QWidgetIntDictIt it(*wPRmapper);
    QWidget *w;
    while ( (w=it.current()) ) {
	int key = it.currentKey();
	++it;
	if ( w == etw ) {                       // found widget
	    etw->clearWState( Qt::WState_Reparented ); // clear flag
	    wPRmapper->remove( key );// old window no longer needed
	    if ( wPRmapper->count() == 0 ) {	// became empty
		delete wPRmapper;		// then reset alt mapper
		wPRmapper = 0;
		return;
	    }
	}
    }
}

static QETWidget *qPRFindWidget( Window oldwin )
{
    return wPRmapper ? (QETWidget*)wPRmapper->find((long)oldwin) : 0;
}

/*!
    \internal
*/
int QApplication::x11ClientMessage(QWidget* w, XEvent* event, bool passive_only)
{
    QETWidget *widget = (QETWidget*)w;
    if ( event->xclient.format == 32 && event->xclient.message_type ) {
	if ( event->xclient.message_type == qt_wm_protocols ) {
	    Atom a = event->xclient.data.l[0];
	    if ( a == qt_wm_delete_window ) {
		if ( passive_only ) return 0;
		widget->translateCloseEvent(event);
	    }
	    else if ( a == qt_wm_take_focus ) {
		QWidget * amw = activeModalWidget();
		if ( (ulong) event->xclient.data.l[1] > qt_x_time )
		    qt_x_time = event->xclient.data.l[1];
		if ( amw && amw != widget ) {
		    QWidget* groupLeader = widget;
		    while ( groupLeader && !groupLeader->testWFlags( Qt::WGroupLeader )
                            && groupLeader != amw )
			groupLeader = groupLeader->parentWidget();
		    if ( !groupLeader ) {
                        QWidget *p = amw->parentWidget();
                        while (p && p != widget)
                            p = p->parentWidget();
                        if (!p || !qt_net_supported_list)
			    amw->raise(); // help broken window managers
			amw->setActiveWindow();
		    }
		}
#ifndef QT_NO_WHATSTHIS
	    } else if ( a == qt_net_wm_context_help ) {
		QWhatsThis::enterWhatsThisMode();
#endif // QT_NO_WHATSTHIS
	    } else if ( a == qt_net_wm_ping ) {
		// avoid send/reply loops
		Window root = QPaintDevice::x11AppRootWindow( w->x11Screen() );
		if (event->xclient.window != root) {
		    event->xclient.window = root;
		    XSendEvent( event->xclient.display, event->xclient.window,
				False, SubstructureNotifyMask|SubstructureRedirectMask, event );
		}
	    }
	} else if ( event->xclient.message_type == qt_qt_scrolldone ) {
	    widget->translateScrollDoneEvent(event);
	} else if ( event->xclient.message_type == qt_xdnd_position ) {
	    qt_handle_xdnd_position( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_enter ) {
	    qt_handle_xdnd_enter( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_status ) {
	    qt_handle_xdnd_status( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_leave ) {
	    qt_handle_xdnd_leave( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_drop ) {
	    qt_handle_xdnd_drop( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_finished ) {
	    qt_handle_xdnd_finished( widget, event, passive_only );
	} else {
	    if ( passive_only ) return 0;
	    // All other are interactions
	}
    } else {
	qt_motifdnd_handle_msg( widget, event, passive_only );
    }

    return 0;
}

/*!
    This function does the core processing of individual X
    \a{event}s, normally by dispatching Qt events to the right
    destination.

    It returns 1 if the event was consumed by special handling, 0 if
    the \a event was consumed by normal handling, and -1 if the \a
    event was for an unrecognized widget.

    \sa x11EventFilter()
*/
int QApplication::x11ProcessEvent( XEvent* event )
{
    switch ( event->type ) {
    case ButtonPress:
	ignoreNextMouseReleaseEvent = FALSE;
	qt_x_user_time = event->xbutton.time;
	// fallthrough intended
    case ButtonRelease:
	qt_x_time = event->xbutton.time;
	break;
    case MotionNotify:
	qt_x_time = event->xmotion.time;
	break;
    case XKeyPress:
	qt_x_user_time = event->xkey.time;
	// fallthrough intended
    case XKeyRelease:
	qt_x_time = event->xkey.time;
	break;
    case PropertyNotify:
	qt_x_time = event->xproperty.time;
	break;
    case EnterNotify:
    case LeaveNotify:
	qt_x_time = event->xcrossing.time;
	break;
    case SelectionClear:
	qt_x_time = event->xselectionclear.time;
	break;
    default:
	break;
    }

    QETWidget *widget = (QETWidget*)QWidget::find( (WId)event->xany.window );

    if ( wPRmapper ) {				// just did a widget reparent?
	if ( widget == 0 ) {			// not in std widget mapper
	    switch ( event->type ) {		// only for mouse/key events
	    case ButtonPress:
	    case ButtonRelease:
	    case MotionNotify:
	    case XKeyPress:
	    case XKeyRelease:
		widget = qPRFindWidget( event->xany.window );
		break;
	    }
	}
	else if ( widget->testWState(WState_Reparented) )
	    qPRCleanup( widget );		// remove from alt mapper
    }

    QETWidget *keywidget=0;
    bool grabbed=FALSE;
    if ( event->type==XKeyPress || event->type==XKeyRelease ) {
	keywidget = (QETWidget*)QWidget::keyboardGrabber();
	if ( keywidget ) {
	    grabbed = TRUE;
	} else {
	    if ( focus_widget )
		keywidget = (QETWidget*)focus_widget;
	    if ( !keywidget ) {
		if ( inPopupMode() ) // no focus widget, see if we have a popup
		    keywidget = (QETWidget*) activePopupWidget();
		else if ( widget )
		    keywidget = (QETWidget*)widget->topLevelWidget();
	    }
	}
    }

    int xkey_keycode = event->xkey.keycode;
    if ( XFilterEvent( event,
		       keywidget ? keywidget->topLevelWidget()->winId() : None ) ) {
	if ( keywidget )
	    composingKeycode = xkey_keycode; // ### not documented in xlib

#ifndef QT_NO_XIM
 	if ( event->type != XKeyPress || ! (qt_xim_style & XIMPreeditCallbacks) )
	    return 1;

	/*
	 * The Solaris htt input method will transform a ClientMessage
	 * event into a filtered KeyPress event, in which case our
	 * keywidget is still zero.
	 */
        if ( ! keywidget ) {
 	    keywidget = (QETWidget*)QWidget::keyboardGrabber();
	    if ( keywidget ) {
	        grabbed = TRUE;
	    } else {
	        if ( focus_widget )
		    keywidget = (QETWidget*)focus_widget;
	        if ( !keywidget ) {
		    if ( inPopupMode() ) // no focus widget, see if we have a popup
		        keywidget = (QETWidget*) activePopupWidget();
		    else if ( widget )
		        keywidget = (QETWidget*)widget->topLevelWidget();
	        }
	    }
        }

	/*
	  if the composition string has been emptied, we need to send
	  an IMEnd event.  however, we have no way to tell if the user
	  has cancelled input, or if the user has accepted the
	  composition.

	  so, we have to look for the next keypress and see if it is
	  the 'commit' key press (keycode == 0).  if it is, we deliver
	  an IMEnd event with the final text, otherwise we deliver an
	  IMEnd with empty text (meaning the user has cancelled the
	  input).
	*/
	QInputContext *qic =
	    (QInputContext *) keywidget->topLevelWidget()->topData()->xic;
	extern bool qt_compose_emptied; // qinputcontext_x11.cpp
	if ( qic && qic->composing && qic->focusWidget && qt_compose_emptied ) {
	    XEvent event2;
	    bool found = FALSE;
	    if ( XCheckTypedEvent( QPaintDevice::x11AppDisplay(),
				   XKeyPress, &event2 ) ) {
		if ( event2.xkey.keycode == 0 ) {
		    // found a key event with the 'commit' string
		    found = TRUE;
		    XPutBackEvent( QPaintDevice::x11AppDisplay(), &event2 );
		}
	    }

	    if ( !found ) {
		// no key event, so the user must have cancelled the composition
		QIMEvent endevent( QEvent::IMEnd, QString::null, -1 );
		QApplication::sendEvent( qic->focusWidget, &endevent );

		qic->focusWidget = 0;
	    }

	    qt_compose_emptied = FALSE;
	}
#endif // QT_NO_XIM

	return 1;
    }

    if ( qt_x11EventFilter(event) )		// send through app filter
	return 1;

    if ( event->type == MappingNotify ) {	// keyboard mapping changed
	XRefreshKeyboardMapping( &event->xmapping );
	return 0;
    }

    if ( event->type == PropertyNotify ) {	// some properties changed
	if ( event->xproperty.window == QPaintDevice::x11AppRootWindow( 0 ) ) {
	    // root properties for the first screen
	    if ( event->xproperty.atom == qt_clipboard_sentinel ) {
		if (qt_check_clipboard_sentinel() )
		    emit clipboard()->dataChanged();
	    } else if ( event->xproperty.atom == qt_selection_sentinel ) {
		if (qt_check_selection_sentinel() )
		    emit clipboard()->selectionChanged();
	    } else if ( obey_desktop_settings ) {
		if ( event->xproperty.atom == qt_resource_manager )
		    qt_set_x11_resources();
		else if ( event->xproperty.atom == qt_settings_timestamp )
		    QApplication::x11_apply_settings();
	    }
	}
	if ( event->xproperty.window == QPaintDevice::x11AppRootWindow() ) {
	    // root properties for the default screen
	    if ( event->xproperty.atom == qt_input_encoding ) {
		qt_set_input_encoding();
	    } else if ( event->xproperty.atom == qt_net_supported ) {
		qt_get_net_supported();
	    } else if ( event->xproperty.atom == qt_net_virtual_roots ) {
		qt_get_net_virtual_roots();
	    } else if ( event->xproperty.atom == qt_net_workarea ) {
		qt_desktopwidget_update_workarea();
	    }
	} else if ( widget ) {
	    widget->translatePropertyEvent(event);
	}  else {
	    return -1; // don't know this window
	}
	return 0;
    }

    if ( !widget ) {				// don't know this windows
	QWidget* popup = QApplication::activePopupWidget();
	if ( popup ) {

	    /*
	      That is more than suboptimal. The real solution should
	      do some keyevent and buttonevent translation, so that
	      the popup still continues to work as the user expects.
	      Unfortunately this translation is currently only
	      possible with a known widget. I'll change that soon
	      (Matthias).
	    */

	    // Danger - make sure we don't lock the server
	    switch ( event->type ) {
	    case ButtonPress:
	    case ButtonRelease:
	    case XKeyPress:
	    case XKeyRelease:
		do {
		    popup->close();
		} while ( (popup = qApp->activePopupWidget()) );
		return 1;
	    }
	}
	return -1;
    }

    if ( event->type == XKeyPress || event->type == XKeyRelease )
	widget = keywidget; // send XKeyEvents through keywidget->x11Event()

    if ( app_do_modal )				// modal event handling
	if ( !qt_try_modal(widget, event) ) {
	    if ( event->type == ClientMessage )
		x11ClientMessage( widget, event, TRUE );
	    return 1;
	}


    if ( widget->x11Event(event) )		// send through widget filter
	return 1;
#if defined (QT_TABLET_SUPPORT)
    if ( event->type == xinput_motion ||
	 event->type == xinput_button_release ||
	 event->type == xinput_button_press ) {
	widget->translateXinputEvent( event );
	return 0;
    }
#endif

#ifndef QT_NO_XRANDR
    if (event->type == xrandr_eventbase + RRScreenChangeNotify) {
	// update Xlib internals with the latest screen configuration
	XRRUpdateConfiguration(event);

	// update the size for desktop widget
	int scr = XRRRootToScreen( appDpy, event->xany.window );
	QWidget *w = desktop()->screen( scr );
	QSize oldSize( w->size() );
	w->crect.setWidth( DisplayWidth( appDpy, scr ) );
        w->crect.setHeight( DisplayHeight( appDpy, scr ) );
	if ( w->size() != oldSize ) {
	    QResizeEvent e( w->size(), oldSize );
	    QApplication::sendEvent( w, &e );
	    emit desktop()->resized( scr );
	}
    }
#endif // QT_NO_XRANDR

    switch ( event->type ) {

    case ButtonRelease:			// mouse event
	if ( ignoreNextMouseReleaseEvent ) {
	    ignoreNextMouseReleaseEvent = FALSE;
	    break;
	}
	// fall through intended
    case ButtonPress:
	if (event->xbutton.root != RootWindow(widget->x11Display(), widget->x11Screen())
	    && ! qt_xdnd_dragging) {
	    while ( activePopupWidget() )
		activePopupWidget()->close();
	    return 1;
	}
	if (event->type == ButtonPress)
	    qt_net_update_user_time(widget->topLevelWidget());
	// fall through intended
    case MotionNotify:
#if defined(QT_TABLET_SUPPORT)
	if ( !chokeMouse ) {
#endif
	    widget->translateMouseEvent( event );
#if defined(QT_TABLET_SUPPORT)
	} else {
	    chokeMouse = FALSE;
	}
#endif
	break;

    case XKeyPress:				// keyboard event
	qt_net_update_user_time(widget->topLevelWidget());
	// fallthrough intended
    case XKeyRelease:
	{
	    if ( keywidget && keywidget->isEnabled() ) { // should always exist
#ifndef QT_NO_XIM
		QInputContext *qic =
		    (QInputContext *) keywidget->topLevelWidget()->topData()->xic;

		if ((qt_xim_style & XIMPreeditCallbacks) && event->xkey.keycode == 0 &&
		    qic && qic->composing && qic->focusWidget) {
		    // input method has sent us a commit string
		    QCString data(513);
		    KeySym sym;    // unused
		    Status status; // unused
		    QString text;
		    int count = qic->lookupString( &(event->xkey), data,
						   &sym, &status );
		    if ( count > 0 )
			text = input_mapper->toUnicode( data, count );

		    // qDebug( "sending IMEnd with %d chars", text.length() );
		    QIMEvent endevent( QEvent::IMEnd, text, -1 );
		    QApplication::sendEvent( qic->focusWidget, &endevent );

		    qic->focusWidget = 0;
		    qic->text = QString::null;
		} else
#endif // !QT_NO_XIM
		    {
			// qDebug( "sending key event" );
			keywidget->translateKeyEvent( event, grabbed );
		    }
	    }
	    break;
	}

    case GraphicsExpose:
    case Expose:				// paint event
	widget->translatePaintEvent( event );
	break;

    case ConfigureNotify:			// window move/resize event
	if ( event->xconfigure.event == event->xconfigure.window )
	    widget->translateConfigEvent( event );
	break;

    case XFocusIn: {				// got focus
	if ( widget->isDesktop() )
	    break;
	if ( inPopupMode() ) // some delayed focus event to ignore
	    break;
	if ( !widget->isTopLevel() )
	    break;
	if ( event->xfocus.detail != NotifyAncestor &&
	     event->xfocus.detail != NotifyInferior &&
	     event->xfocus.detail != NotifyNonlinear )
	    break;
	widget->createInputContext();
	setActiveWindow( widget );
	if ( qt_focus_model == FocusModel_PointerRoot ) {
	    // We got real input focus from somewhere, but we were in PointerRoot
	    // mode, so we don't trust this event.  Check the focus model to make
	    // sure we know what focus mode we are using...
	    qt_check_focus_model();
	}
    }
	break;

    case XFocusOut:				// lost focus
	if ( widget->isDesktop() )
	    break;
	if ( !widget->isTopLevel() )
	    break;
	if ( event->xfocus.mode == NotifyGrab )
	    qt_xfocusout_grab_counter++;
	if ( event->xfocus.mode != NotifyNormal )
	    break;
	if ( event->xfocus.detail != NotifyAncestor &&
	     event->xfocus.detail != NotifyNonlinearVirtual &&
	     event->xfocus.detail != NotifyNonlinear )
	    break;
	if ( !inPopupMode() && widget == active_window )
	    setActiveWindow( 0 );
	break;

    case EnterNotify: {			// enter window
	if ( QWidget::mouseGrabber()  && widget != QWidget::mouseGrabber() )
	    break;
	if ( inPopupMode() && widget->topLevelWidget() != activePopupWidget() )
	    break;
	if ( event->xcrossing.mode != NotifyNormal ||
	     event->xcrossing.detail == NotifyVirtual  ||
	     event->xcrossing.detail == NotifyNonlinearVirtual )
	    break;
	if ( event->xcrossing.focus &&
	     !widget->isDesktop() && !widget->isActiveWindow() ) {
	    if ( qt_focus_model == FocusModel_Unknown ) // check focus model
		qt_check_focus_model();
	    if ( qt_focus_model == FocusModel_PointerRoot ) // PointerRoot mode
		setActiveWindow( widget );
	}
	qt_dispatchEnterLeave( widget, QWidget::find( curWin ) );
	curWin = widget->winId();
	widget->translateMouseEvent( event ); //we don't get MotionNotify, emulate it
    }
	break;

    case LeaveNotify: {			// leave window
	if ( QWidget::mouseGrabber()  && widget != QWidget::mouseGrabber() )
	    break;
	if ( curWin && widget->winId() != curWin )
	    break;
	if ( event->xcrossing.mode != NotifyNormal )
	    break;
	if ( !widget->isDesktop() )
	    widget->translateMouseEvent( event ); //we don't get MotionNotify, emulate it

	QWidget* enter = 0;
	XEvent ev;
	while ( XCheckMaskEvent( widget->x11Display(), EnterWindowMask | LeaveWindowMask , &ev )
		&& !qt_x11EventFilter( &ev )) {
            QWidget* event_widget = QWidget::find( ev.xcrossing.window );
            if( event_widget && event_widget->x11Event( &ev ) )
                break;
	    if ( ev.type == LeaveNotify && ev.xcrossing.mode == NotifyNormal ){
		enter = event_widget;
		XPutBackEvent( widget->x11Display(), &ev );
		break;
	    }
	    if (  ev.xcrossing.mode != NotifyNormal ||
		  ev.xcrossing.detail == NotifyVirtual  ||
		  ev.xcrossing.detail == NotifyNonlinearVirtual )
		continue;
	    enter = event_widget;
	    if ( ev.xcrossing.focus &&
		 enter && !enter->isDesktop() && !enter->isActiveWindow() ) {
		if ( qt_focus_model == FocusModel_Unknown ) // check focus model
		    qt_check_focus_model();
		if ( qt_focus_model == FocusModel_PointerRoot ) // PointerRoot mode
		    setActiveWindow( enter );
	    }
	    break;
	}

	if ( ( ! enter || enter->isDesktop() ) &&
	     event->xcrossing.focus && widget == active_window &&
	     qt_focus_model == FocusModel_PointerRoot // PointerRoot mode
	     ) {
	    setActiveWindow( 0 );
	}

	if ( !curWin )
	    qt_dispatchEnterLeave( widget, 0 );

	qt_dispatchEnterLeave( enter, widget );
	curWin = enter ? enter->winId() : 0;
    }
	break;

    case UnmapNotify:				// window hidden
	if ( widget->isTopLevel() && widget->isShown() ) {
	    widget->topData()->spont_unmapped = 1;
	    QHideEvent e;
	    QApplication::sendSpontaneousEvent( widget, &e );
	    widget->hideChildren( TRUE );
	}
	break;

    case MapNotify:				// window shown
	if ( widget->isTopLevel() &&
	     widget->topData()->spont_unmapped ) {
	    widget->topData()->spont_unmapped = 0;
	    widget->showChildren( TRUE );
	    QShowEvent e;
	    QApplication::sendSpontaneousEvent( widget, &e );
	}
	break;

    case ClientMessage:			// client message
	return x11ClientMessage(widget,event,False);

    case ReparentNotify:			// window manager reparents
	while ( XCheckTypedWindowEvent( widget->x11Display(),
					widget->winId(),
					ReparentNotify,
					event ) )
	    ;	// skip old reparent events
	if ( event->xreparent.parent == QPaintDevice::x11AppRootWindow() ) {
	    if ( widget->isTopLevel() ) {
		widget->topData()->parentWinId = event->xreparent.parent;
		if ( qt_deferred_map_contains( widget ) ) {
		    qt_deferred_map_take( widget );
		    XMapWindow( appDpy, widget->winId() );
		}
	    }
	} else
	    // store the parent. Useful for many things, embedding for instance.
	    widget->topData()->parentWinId = event->xreparent.parent;
	if ( widget->isTopLevel() ) {
	    // the widget frame strut should also be invalidated
	    widget->topData()->fleft = widget->topData()->fright =
	     widget->topData()->ftop = widget->topData()->fbottom = 0;

	    if ( qt_focus_model != FocusModel_Unknown ) {
		// toplevel reparented...
		QWidget *newparent = QWidget::find( event->xreparent.parent );
		if ( ! newparent || newparent->isDesktop() ) {
		    // we dont' know about the new parent (or we've been
		    // reparented to root), perhaps a window manager
		    // has been (re)started?  reset the focus model to unknown
		    qt_focus_model = FocusModel_Unknown;
		}
	    }
	}
	break;

    case SelectionRequest: {
	XSelectionRequestEvent *req = &event->xselectionrequest;
	if (! req)
	    break;

	if ( qt_xdnd_selection && req->selection == qt_xdnd_selection ) {
	    qt_xdnd_handle_selection_request( req );

	} else if (qt_clipboard) {
	    QCustomEvent e( QEvent::Clipboard, event );
	    QApplication::sendSpontaneousEvent( qt_clipboard, &e );
	}
	break;
    }
    case SelectionClear: {
	XSelectionClearEvent *req = &event->xselectionclear;
	// don't deliver dnd events to the clipboard, it gets confused
	if (! req || qt_xdnd_selection && req->selection == qt_xdnd_selection)
	    break;

	if (qt_clipboard) {
	    QCustomEvent e( QEvent::Clipboard, event );
	    QApplication::sendSpontaneousEvent( qt_clipboard, &e );
	}
	break;
    }

    case SelectionNotify: {
	XSelectionEvent *req = &event->xselection;
	// don't deliver dnd events to the clipboard, it gets confused
	if (! req || qt_xdnd_selection && req->selection == qt_xdnd_selection)
	    break;

	if (qt_clipboard) {
	    QCustomEvent e( QEvent::Clipboard, event );
	    QApplication::sendSpontaneousEvent( qt_clipboard, &e );
	}
	break;
    }

    default:
	break;
    }

    return 0;
}

/*!
    This virtual function is only implemented under X11.

    If you create an application that inherits QApplication and
    reimplement this function, you get direct access to all X events
    that the are received from the X server.

    Return TRUE if you want to stop the event from being processed.
    Return FALSE for normal event dispatching.

    \sa x11ProcessEvent()
*/

bool QApplication::x11EventFilter( XEvent * )
{
    return FALSE;
}



/*****************************************************************************
  Modal widgets; Since Xlib has little support for this we roll our own
  modal widget mechanism.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  qt_enter_modal()
	Enters modal state
	Arguments:
	    QWidget *widget	A modal widget

  qt_leave_modal()
	Leaves modal state for a widget
	Arguments:
	    QWidget *widget	A modal widget
 *****************************************************************************/

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal( QWidget *widget )
{
    if ( !qt_modal_stack ) {			// create modal stack
	qt_modal_stack = new QWidgetList;
	Q_CHECK_PTR( qt_modal_stack );
    }
    if (widget->parentWidget()) {
	QEvent e(QEvent::WindowBlocked);
	QApplication::sendEvent(widget->parentWidget(), &e);
    }

    qt_dispatchEnterLeave( 0, QWidget::find((WId)curWin) );
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
    curWin = 0;
    ignoreNextMouseReleaseEvent = FALSE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->removeRef(widget) ) {
	if ( qt_modal_stack->isEmpty() ) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	    QPoint p( QCursor::pos() );
	    QWidget* w = QApplication::widgetAt( p.x(), p.y(), TRUE );
	    qt_dispatchEnterLeave( w, QWidget::find( curWin ) ); // send synthetic enter event
	    curWin = w? w->winId() : 0;
	}
    }
    app_do_modal = qt_modal_stack != 0;
    ignoreNextMouseReleaseEvent = TRUE;

    if (widget->parentWidget()) {
	QEvent e(QEvent::WindowUnblocked);
	QApplication::sendEvent(widget->parentWidget(), &e);
    }
}


bool qt_try_modal( QWidget *widget, XEvent *event )
{
    if (qt_xdnd_dragging) {
	// allow mouse events while DnD is active
	switch (event->type) {
	case ButtonPress:
	case ButtonRelease:
	case MotionNotify:
	    return TRUE;
	default:
	    break;
	}
    }

    if ( qt_tryModalHelper( widget ) )
	return TRUE;

    bool block_event  = FALSE;
    switch ( event->type ) {
	case ButtonPress:			// disallow mouse/key events
	case ButtonRelease:
	case MotionNotify:
	case XKeyPress:
	case XKeyRelease:
	case EnterNotify:
	case LeaveNotify:
	case ClientMessage:
	    block_event	 = TRUE;
	    break;
    	default:
            break;
    }

    return !block_event;
}


/*****************************************************************************
  Popup widget mechanism

  openPopup()
	Adds a widget to the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be added

  closePopup()
	Removes a widget from the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be removed
 *****************************************************************************/


static int openPopupCount = 0;
void QApplication::openPopup( QWidget *popup )
{
    openPopupCount++;
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	Q_CHECK_PTR( popupWidgets );
    }
    popupWidgets->append( popup );		// add to end of list

    if ( popupWidgets->count() == 1 && !qt_nograb() ){ // grab mouse/keyboard
	int r = XGrabKeyboard( popup->x11Display(), popup->winId(), FALSE,
			       GrabModeSync, GrabModeAsync, CurrentTime );
	if ( (popupGrabOk = (r == GrabSuccess)) ) {
	    r = XGrabPointer( popup->x11Display(), popup->winId(), TRUE,
			      (uint)(ButtonPressMask | ButtonReleaseMask |
				     ButtonMotionMask | EnterWindowMask |
				     LeaveWindowMask | PointerMotionMask),
			      GrabModeSync, GrabModeAsync,
			      None, None, CurrentTime );

	    if ( (popupGrabOk = (r == GrabSuccess)) )
		XAllowEvents( popup->x11Display(), SyncPointer, CurrentTime );
	    else
		XUngrabKeyboard( popup->x11Display(), CurrentTime );
	}
    } else if ( popupGrabOk ) {
	XAllowEvents(  popup->x11Display(), SyncPointer, CurrentTime );
    }

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason( QFocusEvent::Popup );
    if ( popup->focusWidget())
	popup->focusWidget()->setFocus();
    else
	popup->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup( QWidget *popup )
{
    if ( !popupWidgets )
	return;
    popupWidgets->removeRef( popup );
    if (popup == popupOfPopupButtonFocus) {
	popupButtonFocus = 0;
	popupOfPopupButtonFocus = 0;
    }
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !qt_nograb() && popupGrabOk ) {	// grabbing not disabled
	    if ( mouseButtonState != 0
		 || popup->geometry(). contains(QPoint(mouseGlobalXPos, mouseGlobalYPos) ) )
		{	// mouse release event or inside
		    XAllowEvents( popup->x11Display(), AsyncPointer,
				  CurrentTime );
	    } else {				// mouse press event
		mouseButtonPressTime -= 10000;	// avoid double click
		XAllowEvents( popup->x11Display(), ReplayPointer,CurrentTime );
	    }
	    XUngrabPointer( popup->x11Display(), CurrentTime );
	    XFlush( popup->x11Display() );
	}
	if ( active_window ) {
	    QFocusEvent::setReason( QFocusEvent::Popup );
	    if ( active_window->focusWidget() )
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
	    QFocusEvent::resetReason();
	}
    } else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	 QFocusEvent::setReason( QFocusEvent::Popup );
	 QWidget* aw = popupWidgets->getLast();
	 if (aw->focusWidget())
	     aw->focusWidget()->setFocus();
	 else
	     aw->setFocus();
	 QFocusEvent::resetReason();
	 if ( popupWidgets->count() == 1 && !qt_nograb() ){ // grab mouse/keyboard
	     int r = XGrabKeyboard( aw->x11Display(), aw->winId(), FALSE,
				    GrabModeSync, GrabModeAsync, CurrentTime );
	     if ( (popupGrabOk = (r == GrabSuccess)) ) {
		 r = XGrabPointer( aw->x11Display(), aw->winId(), TRUE,
				   (uint)(ButtonPressMask | ButtonReleaseMask |
					  ButtonMotionMask | EnterWindowMask |
					  LeaveWindowMask | PointerMotionMask),
				   GrabModeSync, GrabModeAsync,
				   None, None, CurrentTime );

		 if ( (popupGrabOk = (r == GrabSuccess)) )
		     XAllowEvents( aw->x11Display(), SyncPointer, CurrentTime );
	     }
	 }
     }
}

/*****************************************************************************
  Event translation; translates X11 events to Qt events
 *****************************************************************************/

//
// Mouse event translation
//
// Xlib doesn't give mouse double click events, so we generate them by
// comparing window, time and position between two mouse press events.
//

//
// Keyboard event translation
//

static int translateButtonState( int s )
{
    int bst = 0;
    if ( s & Button1Mask )
	bst |= Qt::LeftButton;
    if ( s & Button2Mask )
	bst |= Qt::MidButton;
    if ( s & Button3Mask )
	bst |= Qt::RightButton;
    if ( s & ShiftMask )
	bst |= Qt::ShiftButton;
    if ( s & ControlMask )
	bst |= Qt::ControlButton;
    if ( s & qt_alt_mask )
	bst |= Qt::AltButton;
    if ( s & qt_meta_mask )
	bst |= Qt::MetaButton;
    return bst;
}

bool QETWidget::translateMouseEvent( const XEvent *event )
{
    static bool manualGrab = FALSE;
    QEvent::Type type;				// event parameters
    QPoint pos;
    QPoint globalPos;
    int button = 0;
    int state;
    XEvent nextEvent;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    static int x_root_save = -1, y_root_save = -1;

    if ( event->type == MotionNotify ) { // mouse move
	if (event->xmotion.root != RootWindow(appDpy, x11Screen()) &&
	    ! qt_xdnd_dragging )
	    return FALSE;

	XMotionEvent lastMotion = event->xmotion;
	while( XPending( appDpy ) )  { // compres mouse moves
	    XNextEvent( appDpy, &nextEvent );
	    if ( nextEvent.type == ConfigureNotify
		 || nextEvent.type == PropertyNotify
		 || nextEvent.type == Expose
		 || nextEvent.type == NoExpose ) {
		qApp->x11ProcessEvent( &nextEvent );
		continue;
	    } else if ( nextEvent.type != MotionNotify ||
			nextEvent.xmotion.window != event->xmotion.window ||
			nextEvent.xmotion.state != event->xmotion.state ) {
		XPutBackEvent( appDpy, &nextEvent );
		break;
	    }
	    if ( !qt_x11EventFilter(&nextEvent)
		 && !x11Event( &nextEvent ) ) // send event through filter
		lastMotion = nextEvent.xmotion;
	    else
		break;
	}
	type = QEvent::MouseMove;
	pos.rx() = lastMotion.x;
	pos.ry() = lastMotion.y;
	globalPos.rx() = lastMotion.x_root;
	globalPos.ry() = lastMotion.y_root;
	state = translateButtonState( lastMotion.state );
	if ( qt_button_down && (state & (LeftButton |
					 MidButton |
					 RightButton ) ) == 0 )
	    qt_button_down = 0;

	// throw away mouse move events that are sent multiple times to the same
	// position
	bool throw_away = FALSE;
	if ( x_root_save == globalPos.x() &&
	     y_root_save == globalPos.y() )
	    throw_away = TRUE;
	x_root_save = globalPos.x();
	y_root_save = globalPos.y();
	if ( throw_away )
	    return TRUE;
    } else if ( event->type == EnterNotify || event->type == LeaveNotify) {
	XEvent *xevent = (XEvent *)event;
	//unsigned int xstate = event->xcrossing.state;
	type = QEvent::MouseMove;
	pos.rx() = xevent->xcrossing.x;
	pos.ry() = xevent->xcrossing.y;
	globalPos.rx() = xevent->xcrossing.x_root;
	globalPos.ry() = xevent->xcrossing.y_root;
	state = translateButtonState( xevent->xcrossing.state );
	if ( qt_button_down && (state & (LeftButton |
					 MidButton |
					 RightButton ) ) == 0 )
	    qt_button_down = 0;
	if ( !qt_button_down )
	    state = state & ~(LeftButton | MidButton | RightButton );
    } else {					// button press or release
	pos.rx() = event->xbutton.x;
	pos.ry() = event->xbutton.y;
	globalPos.rx() = event->xbutton.x_root;
	globalPos.ry() = event->xbutton.y_root;
	state = translateButtonState( event->xbutton.state );
	switch ( event->xbutton.button ) {
	case Button1: button = LeftButton; break;
	case Button2: button = MidButton; break;
	case Button3: button = RightButton; break;
	case Button4:
	case Button5:
	case 6:
	case 7:
	    // the fancy mouse wheel.

	    // take care about grabbing. We do this here since it
	    // is clear that we return anyway
	    if ( qApp->inPopupMode() && popupGrabOk )
		XAllowEvents( x11Display(), SyncPointer, CurrentTime );

	    // We are only interested in ButtonPress.
	    if (event->type == ButtonPress ){

		// compress wheel events (the X Server will simply
		// send a button press for each single notch,
		// regardless whether the application can catch up
		// or not)
		int delta = 1;
		XEvent xevent;
		while ( XCheckTypedWindowEvent(x11Display(),winId(),
					       ButtonPress,&xevent) ){
		    if (xevent.xbutton.button != event->xbutton.button){
			XPutBackEvent(x11Display(), &xevent);
			break;
		    }
		    delta++;
		}

		// the delta is defined as multiples of
		// WHEEL_DELTA, which is set to 120. Future wheels
		// may offer a finer-resolution. A positive delta
		// indicates forward rotation, a negative one
		// backward rotation respectively.
		int btn = event->xbutton.button;
		delta *= 120 * ( (btn == Button4 || btn == 6) ? 1 : -1 );
		bool hor = ( (btn == Button4 || btn == Button5) && (state&AltButton) ||
			     (btn == 6 || btn == 7) );
		translateWheelEvent( globalPos.x(), globalPos.y(), delta, state, (hor)?Horizontal:Vertical );
	    }
	    return TRUE;
	}
	if ( event->type == ButtonPress ) {	// mouse button pressed
#if defined(Q_OS_IRIX) && defined(QT_TABLET_SUPPORT)
	    XEvent myEv;
	    if ( XCheckTypedEvent( appDpy, xinput_button_press, &myEv ) ) {
		if ( translateXinputEvent( &myEv ) ) {
		    //Spontaneous event sent.  Check if we need to continue.
		    if ( chokeMouse ) {
			chokeMouse = FALSE;
			return FALSE;
		    }
		}
	    }
#endif
	    qt_button_down = childAt( pos );	//magic for masked widgets
	    if ( !qt_button_down || !qt_button_down->testWFlags(WMouseNoMask) )
		qt_button_down = this;
	    if ( mouseActWindow == event->xbutton.window &&
		 mouseButtonPressed == button &&
		 (long)event->xbutton.time -(long)mouseButtonPressTime
		 < QApplication::doubleClickInterval() &&
		 QABS(event->xbutton.x - mouseXPos) < 5 &&
		 QABS(event->xbutton.y - mouseYPos) < 5 ) {
		type = QEvent::MouseButtonDblClick;
		mouseButtonPressTime -= 2000;	// no double-click next time
	    } else {
		type = QEvent::MouseButtonPress;
		mouseButtonPressTime = event->xbutton.time;
	    }
	    mouseButtonPressed = button;	// save event params for
	    mouseXPos = pos.x();		// future double click tests
	    mouseYPos = pos.y();
	    mouseGlobalXPos = globalPos.x();
	    mouseGlobalYPos = globalPos.y();
	} else {				// mouse button released
#if defined(Q_OS_IRIX) && defined(QT_TABLET_SUPPORT)
	    XEvent myEv;
	    if ( XCheckTypedEvent( appDpy, xinput_button_release, &myEv ) ) {
		if ( translateXinputEvent( &myEv ) ) {
		    //Spontaneous event sent.  Check if we need to continue.
		    if ( chokeMouse ) {
			chokeMouse = FALSE;
			return FALSE;
		    }
		}
	    }
#endif
	    if ( manualGrab ) {			// release manual grab
		manualGrab = FALSE;
		XUngrabPointer( x11Display(), CurrentTime );
		XFlush( x11Display() );
	    }

	    type = QEvent::MouseButtonRelease;
	}
    }
    mouseActWindow = winId();			// save some event params
    mouseButtonState = state;
    if ( type == 0 )				// don't send event
	return FALSE;

    if ( qApp->inPopupMode() ) {			// in popup mode
	QWidget *popup = qApp->activePopupWidget();
	if ( popup != this ) {
	    if ( testWFlags(WType_Popup) && rect().contains(pos) )
		popup = this;
	    else				// send to last popup
		pos = popup->mapFromGlobal( globalPos );
	}
	bool releaseAfter = FALSE;
	QWidget *popupChild  = popup->childAt( pos );
	QWidget *popupTarget = popupChild ? popupChild : popup;

	if (popup != popupOfPopupButtonFocus){
	    popupButtonFocus = 0;
	    popupOfPopupButtonFocus = 0;
	}

	if ( !popupTarget->isEnabled() ) {
	    if ( popupGrabOk )
		XAllowEvents( x11Display(), SyncPointer, CurrentTime );
	}

	switch ( type ) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonDblClick:
	    popupButtonFocus = popupChild;
	    popupOfPopupButtonFocus = popup;
	    break;
	case QEvent::MouseButtonRelease:
	    releaseAfter = TRUE;
	    break;
	default:
	    break;				// nothing for mouse move
	}

	Display* dpy = x11Display(); // store display, send() may destroy us


	int oldOpenPopupCount = openPopupCount;

	if ( popupButtonFocus ) {
	    QMouseEvent e( type, popupButtonFocus->mapFromGlobal(globalPos),
			   globalPos, button, state );
	    QApplication::sendSpontaneousEvent( popupButtonFocus, &e );
	    if ( releaseAfter ) {
		popupButtonFocus = 0;
		popupOfPopupButtonFocus = 0;
	    }
	} else if ( popupChild ) {
	    QMouseEvent e( type, popupChild->mapFromGlobal(globalPos),
			   globalPos, button, state );
	    QApplication::sendSpontaneousEvent( popupChild, &e );
	} else {
	    QMouseEvent e( type, pos, globalPos, button, state );
	    QApplication::sendSpontaneousEvent( popup, &e );
	}

	if ( type == QEvent::MouseButtonPress && button == RightButton && ( openPopupCount == oldOpenPopupCount ) ) {
	    QWidget *popupEvent = popup;
	    if(popupButtonFocus)
		popupEvent = popupButtonFocus;
	    else if(popupChild)
		popupEvent = popupChild;
	    QContextMenuEvent e( QContextMenuEvent::Mouse, pos, globalPos, state );
	    QApplication::sendSpontaneousEvent( popupEvent, &e );
	}

	if ( releaseAfter )
	    qt_button_down = 0;

	if ( qApp->inPopupMode() ) { // still in popup mode
	    if ( popupGrabOk )
		XAllowEvents( dpy, SyncPointer, CurrentTime );
	} else {
	    if ( type != QEvent::MouseButtonRelease && state != 0 &&
		 QWidget::find((WId)mouseActWindow) ) {
		manualGrab = TRUE;		// need to manually grab
		XGrabPointer( dpy, mouseActWindow, False,
			      (uint)(ButtonPressMask | ButtonReleaseMask |
				     ButtonMotionMask |
				     EnterWindowMask | LeaveWindowMask),
			      GrabModeAsync, GrabModeAsync,
			      None, None, CurrentTime );
	    }
	}

    } else {
	QWidget *widget = this;
	QWidget *w = QWidget::mouseGrabber();
	if ( !w )
	    w = qt_button_down;
	if ( w && w != this ) {
	    widget = w;
	    pos = w->mapFromGlobal( globalPos );
	}

	if ( popupCloseDownMode ) {
	    popupCloseDownMode = FALSE;
	    if ( testWFlags(WType_Popup) )	// ignore replayed event
		return TRUE;
	}

	if ( type == QEvent::MouseButtonRelease &&
	     (state & (~button) & ( LeftButton |
				    MidButton |
				    RightButton)) == 0 ) {
	    qt_button_down = 0;
	}

	int oldOpenPopupCount = openPopupCount;

	QMouseEvent e( type, pos, globalPos, button, state );
	QApplication::sendSpontaneousEvent( widget, &e );

	if ( type == QEvent::MouseButtonPress && button == RightButton && ( openPopupCount == oldOpenPopupCount ) ) {
	    QContextMenuEvent e( QContextMenuEvent::Mouse, pos, globalPos, state );
	    QApplication::sendSpontaneousEvent( widget, &e );
	}
    }
    return TRUE;
}


//
// Wheel event translation
//
bool QETWidget::translateWheelEvent( int global_x, int global_y, int delta, int state, Orientation orient )
{
    // send the event to the widget or its ancestors
    {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && topLevelWidget() != popup )
	    popup->close();
	QWheelEvent e( mapFromGlobal(QPoint( global_x, global_y)),
		       QPoint(global_x, global_y), delta, state, orient );
	if ( QApplication::sendSpontaneousEvent( this, &e ) )
	    return TRUE;
    }

    // send the event to the widget that has the focus or its ancestors, if different
    QWidget *w = this;
    if ( w != qApp->focusWidget() && ( w = qApp->focusWidget() ) ) {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w != popup )
	    popup->hide();
	QWheelEvent e( mapFromGlobal(QPoint( global_x, global_y)),
		       QPoint(global_x, global_y), delta, state, orient );
	if ( QApplication::sendSpontaneousEvent( w, &e ) )
	    return TRUE;
    }
    return FALSE;
}


//
// XInput Translation Event
//
#if defined (QT_TABLET_SUPPORT)
bool QETWidget::translateXinputEvent( const XEvent *ev )
{
#if defined (Q_OS_IRIX)
    // Wacom has put defines in their wacom.h file so it would be quite wise
    // to use them, need to think of a decent way of not using
    // it when it doesn't exist...
    XDeviceState *s;
    XInputClass *iClass;
    XValuatorState *vs;
    int j;
#endif
    QWidget *w = this;
    QPoint global,
	curr;
    static int pressure = 0;
    static int xTilt = 0,
	       yTilt = 0;
    int deviceType = QTabletEvent::NoDevice;
    QPair<int, int> tId;
    XEvent xinputMotionEvent;
    XEvent mouseMotionEvent;
    XDevice *dev;
    const XDeviceMotionEvent *motion = 0;
    XDeviceButtonEvent *button = 0;
    QEvent::Type t;

    if ( ev->type == xinput_motion ) {
	motion = (const XDeviceMotionEvent*)ev;
	for (;;) {
	    if (!XCheckTypedWindowEvent(x11Display(), winId(), MotionNotify, &mouseMotionEvent))
		break;
	    if (!XCheckTypedWindowEvent(x11Display(), winId(), xinput_motion, &xinputMotionEvent)) {
		XPutBackEvent(x11Display(), &mouseMotionEvent);
		break;
	    }
	    if (mouseMotionEvent.xmotion.time != motion->time) {
		XPutBackEvent(x11Display(), &mouseMotionEvent);
		XPutBackEvent(x11Display(), &xinputMotionEvent);
		break;
	    }
	    motion = ((const XDeviceMotionEvent*)&xinputMotionEvent);
	}
	t = QEvent::TabletMove;
	curr = QPoint( motion->x, motion->y );
    } else {
	if ( ev->type == xinput_button_press ) {
	    t = QEvent::TabletPress;
        } else {
	    t = QEvent::TabletRelease;
	}
	button = (XDeviceButtonEvent*)ev;
/*
	qDebug( "\n\nXInput Button Event" );
	qDebug( "serial:\t%d", button->serial );
	qDebug( "send_event:\t%d", button->send_event );
	qDebug( "display:\t%p", button->display );
	qDebug( "window:\t%d", button->window );
	qDebug( "deviceID:\t%d", button->deviceid );
	qDebug( "root:\t%d", button->root );
	qDebug( "subwindot:\t%d", button->subwindow );
	qDebug( "x:\t%d", button->x );
	qDebug( "y:\t%d", button->y );
	qDebug( "x_root:\t%d", button->x_root );
	qDebug( "y_root:\t%d", button->y_root );
	qDebug( "state:\t%d", button->state );
	qDebug( "button:\t%d", button->button );
	qDebug( "same_screen:\t%d", button->same_screen );
	qDebug( "time:\t%d", button->time );
*/
	curr = QPoint( button->x, button->y );
    }
#if defined(Q_OS_IRIX)
    // default...
    dev = devStylus;
#else
    if ( ev->type == xinput_motion ) {
	if ( motion->deviceid == devStylus->device_id ) {
	    dev = devStylus;
	    deviceType = QTabletEvent::Stylus;
	} else if ( motion->deviceid == devEraser->device_id ) {
	    dev = devEraser;
	    deviceType = QTabletEvent::Eraser;
	}
    } else {
	if ( button->deviceid == devStylus->device_id ) {
	    dev = devStylus;
	    deviceType = QTabletEvent::Stylus;
	} else if ( button->deviceid == devEraser->device_id ) {
	    dev = devEraser;
	    deviceType = QTabletEvent::Eraser;
	}
    }
#endif

    const int PRESSURE_LEVELS = 255;
    // we got the maximum pressure at start time, since various tablets have
    // varying levels of distinguishing pressure changes, let's standardize and
    // scale everything to 256 different levels...
    static int scaleFactor = -1;
    if ( scaleFactor == -1 ) {
	if ( max_pressure > PRESSURE_LEVELS )
	    scaleFactor = max_pressure / PRESSURE_LEVELS;
	else
	    scaleFactor = PRESSURE_LEVELS / max_pressure;
    }
#if defined (Q_OS_IRIX)
    s = XQueryDeviceState( appDpy, dev );
    if ( s == NULL )
        return FALSE;
    iClass = s->data;
    for ( j = 0; j < s->num_classes; j++ ) {
        if ( iClass->c_class == ValuatorClass ) {
            vs = (XValuatorState *)iClass;
            // figure out what device we have, based on bitmasking...
            if ( vs->valuators[WAC_TRANSDUCER_I]
                 & WAC_TRANSDUCER_PROX_MSK ) {
                switch ( vs->valuators[WAC_TRANSDUCER_I]
                         & WAC_TRANSDUCER_MSK ) {
                case WAC_PUCK_ID:
                    deviceType = QTabletEvent::Puck;
                    break;
                case WAC_STYLUS_ID:
                    deviceType = QTabletEvent::Stylus;
                    break;
                case WAC_ERASER_ID:
                    deviceType = QTabletEvent::Eraser;
                    break;
                }
                // Get a Unique Id for the device, Wacom gives us this ability
                tId.first = vs->valuators[WAC_TRANSDUCER_I] & WAC_TRANSDUCER_ID_MSK;
                tId.second = vs->valuators[WAC_SERIAL_NUM_I];
            } else
                deviceType = QTabletEvent::NoDevice;
            // apparently Wacom needs a cast for the +/- values to make sense
            xTilt = short(vs->valuators[WAC_XTILT_I]);
            yTilt = short(vs->valuators[WAC_YTILT_I]);
            if ( max_pressure > PRESSURE_LEVELS )
                pressure = vs->valuators[WAC_PRESSURE_I] / scaleFactor;
            else
                pressure = vs->valuators[WAC_PRESSURE_I] * scaleFactor;
	    global = QPoint( vs->valuators[WAC_XCOORD_I],
                             vs->valuators[WAC_YCOORD_I] );
	    break;
	}
	iClass = (XInputClass*)((char*)iClass + iClass->length);
    }
    XFreeDeviceState( s );
#else
    if ( motion ) {
	xTilt = short(motion->axis_data[3]);
	yTilt = short(motion->axis_data[4]);
	if ( max_pressure > PRESSURE_LEVELS )
	    pressure = motion->axis_data[2] / scaleFactor;
	else
	    pressure = motion->axis_data[2] * scaleFactor;
	global = QPoint( motion->axis_data[0], motion->axis_data[1] );
    } else {
	xTilt = short(button->axis_data[3]);
	yTilt = short(button->axis_data[4]);
	if ( max_pressure > PRESSURE_LEVELS )
	    pressure = button->axis_data[2]  / scaleFactor;
	else
	    pressure = button->axis_data[2] * scaleFactor;
	global = QPoint( button->axis_data[0], button->axis_data[1] );
    }
    // The only way to get these Ids is to scan the XFree86 log, which I'm not going to do.
    tId.first = tId.second = -1;
#endif

    QTabletEvent e( t, curr, global, deviceType, pressure, xTilt, yTilt, tId );
    QApplication::sendSpontaneousEvent( w, &e );
    return TRUE;
}
#endif

bool QETWidget::translatePropertyEvent(const XEvent *event)
{
    if (!isTopLevel()) return TRUE;

    Atom ret;
    int format, e;
    unsigned char *data = 0;
    unsigned long nitems, after;

    if (event->xproperty.atom == qt_net_wm_frame_strut) {
	topData()->fleft = topData()->fright = topData()->ftop = topData()->fbottom = 0;
	fstrut_dirty = 1;

	if (event->xproperty.state == PropertyNewValue) {
	    e = XGetWindowProperty(appDpy, event->xproperty.window, qt_net_wm_frame_strut,
				   0, 4, // struts are 4 longs
				   False, XA_CARDINAL, &ret, &format, &nitems, &after, &data);

	    if (e == Success && ret == XA_CARDINAL &&
		format == 32 && nitems == 4) {
		long *strut = (long *) data;
		topData()->fleft   = strut[0];
		topData()->fright  = strut[1];
		topData()->ftop    = strut[2];
		topData()->fbottom = strut[3];
		fstrut_dirty = 0;
	    }
	}
    } else if (event->xproperty.atom == qt_net_wm_state) {
	bool max = FALSE;
	bool full = FALSE;

	if (event->xproperty.state == PropertyNewValue) {
	    // using length of 1024 should be safe for all current and
	    // possible NET states...
	    e = XGetWindowProperty(appDpy, event->xproperty.window, qt_net_wm_state, 0, 1024,
				   False, XA_ATOM, &ret, &format, &nitems, &after, &data);

	    if (e == Success && ret == XA_ATOM && format == 32 && nitems > 0) {
		Atom *states = (Atom *) data;

		unsigned long i;
		for (i = 0; i < nitems; i++) {
		    if (states[i] == qt_net_wm_state_max_v || states[i] == qt_net_wm_state_max_h)
			max = TRUE;
		    else if (states[i] == qt_net_wm_state_fullscreen)
			full = TRUE;
		}
	    }
	}

	bool send_event = FALSE;

        if (qt_net_supports(qt_net_wm_state_max_v)
            && qt_net_supports(qt_net_wm_state_max_h)) {
            if (max && !isMaximized()) {
                setWState(WState_Maximized);
                send_event = TRUE;
            } else if (!max && isMaximized()) {
                clearWState(WState_Maximized);
                send_event = TRUE;
            }
        }

        if (qt_net_supports(qt_net_wm_state_fullscreen)) {
            if (full && !isFullScreen()) {
                setWState(WState_FullScreen);
                send_event = TRUE;
            } else if (!full && isFullScreen()) {
                clearWState(WState_FullScreen);
                send_event = TRUE;
            }
        }

	if (send_event) {
	    QEvent e(QEvent::WindowStateChange);
	    QApplication::sendSpontaneousEvent(this, &e);
	}
    } else if (event->xproperty.atom == qt_wm_state) {
	// the widget frame strut should also be invalidated
	topData()->fleft = topData()->fright = topData()->ftop = topData()->fbottom = 0;
	fstrut_dirty = 1;

	if (event->xproperty.state == PropertyDelete) {
	    // the window manager has removed the WM State property,
	    // so it is now in the withdrawn state (ICCCM 4.1.3.1) and
	    // we are free to reuse this window
	    topData()->parentWinId = 0;
	    // map the window if we were waiting for a transition to
	    // withdrawn
	    if ( qt_deferred_map_contains( this ) ) {
		qt_deferred_map_take( this );
		XMapWindow( appDpy, winId() );
	    }
	} else if (topData()->parentWinId != QPaintDevice::x11AppRootWindow(x11Screen())) {
	    // the window manager has changed the WM State property...
	    // we are wanting to see if we are withdrawn so that we
	    // can reuse this window... we only do this check *IF* we
	    // haven't been reparented to root - (the parentWinId !=
	    // QPaintDevice::x11AppRootWindow(x11Screen())) check
	    // above

	    e = XGetWindowProperty(appDpy, winId(), qt_wm_state, 0, 2, False, qt_wm_state,
				   &ret, &format, &nitems, &after, &data );

	    if (e == Success && ret == qt_wm_state && format == 32 && nitems > 0) {
		long *state = (long *) data;
		switch (state[0]) {
		case WithdrawnState:
		    // if we are in the withdrawn state, we are free
		    // to reuse this window provided we remove the
		    // WM_STATE property (ICCCM 4.1.3.1)
		    XDeleteProperty(appDpy, winId(), qt_wm_state);

		    // set the parent id to zero, so that show() will
		    // work again
		    topData()->parentWinId = 0;
		    // map the window if we were waiting for a
		    // transition to withdrawn
		    if ( qt_deferred_map_contains( this ) ) {
			qt_deferred_map_take( this );
			XMapWindow( appDpy, winId() );
		    }
		    break;

		case IconicState:
		    if (!isMinimized()) {
			// window was minimized
			setWState(WState_Minimized);
			QEvent e(QEvent::WindowStateChange);
			QApplication::sendSpontaneousEvent(this, &e);
		    }
		    break;

		default:
		    if (isMinimized()) {
			// window was un-minimized
			clearWState(WState_Minimized);
			QEvent e(QEvent::WindowStateChange);
			QApplication::sendSpontaneousEvent(this, &e);
		    }
		    break;
		}
	    }
	}
    }

    if (data)
	XFree(data);

    return TRUE;
}

#ifndef XK_ISO_Left_Tab
#define	XK_ISO_Left_Tab					0xFE20
#endif

// the next lines are taken from XFree > 4.0 (X11/XF86keysyms.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_Standby		0x1008FF10
#define XF86XK_AudioLowerVolume	0x1008FF11
#define XF86XK_AudioMute	0x1008FF12
#define XF86XK_AudioRaiseVolume	0x1008FF13
#define XF86XK_AudioPlay	0x1008FF14
#define XF86XK_AudioStop	0x1008FF15
#define XF86XK_AudioPrev	0x1008FF16
#define XF86XK_AudioNext	0x1008FF17
#define XF86XK_HomePage		0x1008FF18
#define XF86XK_Calculator	0x1008FF1D
#define XF86XK_Mail		0x1008FF19
#define XF86XK_Start		0x1008FF1A
#define XF86XK_Search		0x1008FF1B
#define XF86XK_AudioRecord	0x1008FF1C
#define XF86XK_Back		0x1008FF26
#define XF86XK_Forward		0x1008FF27
#define XF86XK_Stop		0x1008FF28
#define XF86XK_Refresh		0x1008FF29
#define XF86XK_Favorites	0x1008FF30
#define XF86XK_AudioPause	0x1008FF31
#define XF86XK_AudioMedia	0x1008FF32
#define XF86XK_MyComputer	0x1008FF33
#define XF86XK_OpenURL		0x1008FF38
#define XF86XK_Launch0		0x1008FF40
#define XF86XK_Launch1		0x1008FF41
#define XF86XK_Launch2		0x1008FF42
#define XF86XK_Launch3		0x1008FF43
#define XF86XK_Launch4		0x1008FF44
#define XF86XK_Launch5		0x1008FF45
#define XF86XK_Launch6		0x1008FF46
#define XF86XK_Launch7		0x1008FF47
#define XF86XK_Launch8		0x1008FF48
#define XF86XK_Launch9		0x1008FF49
#define XF86XK_LaunchA		0x1008FF4A
#define XF86XK_LaunchB		0x1008FF4B
#define XF86XK_LaunchC		0x1008FF4C
#define XF86XK_LaunchD		0x1008FF4D
#define XF86XK_LaunchE		0x1008FF4E
#define XF86XK_LaunchF		0x1008FF4F
// end of XF86keysyms.h



static const KeySym KeyTbl[] = {		// keyboard mapping table
    XK_Escape,		Qt::Key_Escape,		// misc keys
    XK_Tab,		Qt::Key_Tab,
    XK_ISO_Left_Tab,    Qt::Key_Backtab,
    XK_BackSpace,	Qt::Key_Backspace,
    XK_Return,		Qt::Key_Return,
    XK_Insert,		Qt::Key_Insert,
    XK_KP_Insert,	Qt::Key_Insert,
    XK_Delete,		Qt::Key_Delete,
    XK_KP_Delete,	Qt::Key_Delete,
    XK_Clear,		Qt::Key_Delete,
    XK_Pause,		Qt::Key_Pause,
    XK_Print,		Qt::Key_Print,
    XK_KP_Begin,	Qt::Key_Clear,
    0x1005FF60,		Qt::Key_SysReq,		// hardcoded Sun SysReq
    0x1007ff00,		Qt::Key_SysReq,		// hardcoded X386 SysReq
    XK_Home,		Qt::Key_Home,		// cursor movement
    XK_End,		Qt::Key_End,
    XK_Left,		Qt::Key_Left,
    XK_Up,		Qt::Key_Up,
    XK_Right,		Qt::Key_Right,
    XK_Down,		Qt::Key_Down,
    XK_Prior,		Qt::Key_Prior,
    XK_Next,		Qt::Key_Next,
    XK_KP_Home,		Qt::Key_Home,
    XK_KP_End,		Qt::Key_End,
    XK_KP_Left,		Qt::Key_Left,
    XK_KP_Up,		Qt::Key_Up,
    XK_KP_Right,	Qt::Key_Right,
    XK_KP_Down,		Qt::Key_Down,
    XK_KP_Prior,	Qt::Key_Prior,
    XK_KP_Next,		Qt::Key_Next,
    XK_Shift_L,		Qt::Key_Shift,		// modifiers
    XK_Shift_R,		Qt::Key_Shift,
    XK_Shift_Lock,	Qt::Key_Shift,
    XK_Control_L,	Qt::Key_Control,
    XK_Control_R,	Qt::Key_Control,
    XK_Meta_L,		Qt::Key_Meta,
    XK_Meta_R,		Qt::Key_Meta,
    XK_Alt_L,		Qt::Key_Alt,
    XK_Alt_R,		Qt::Key_Alt,
    XK_Caps_Lock,	Qt::Key_CapsLock,
    XK_Num_Lock,	Qt::Key_NumLock,
    XK_Scroll_Lock,	Qt::Key_ScrollLock,
    XK_KP_Space,	Qt::Key_Space,		// numeric keypad
    XK_KP_Tab,		Qt::Key_Tab,
    XK_KP_Enter,	Qt::Key_Enter,
    XK_KP_Equal,	Qt::Key_Equal,
    XK_KP_Multiply,	Qt::Key_Asterisk,
    XK_KP_Add,		Qt::Key_Plus,
    XK_KP_Separator,	Qt::Key_Comma,
    XK_KP_Subtract,	Qt::Key_Minus,
    XK_KP_Decimal,	Qt::Key_Period,
    XK_KP_Divide,	Qt::Key_Slash,
    XK_Super_L,		Qt::Key_Super_L,
    XK_Super_R,		Qt::Key_Super_R,
    XK_Menu,		Qt::Key_Menu,
    XK_Hyper_L,		Qt::Key_Hyper_L,
    XK_Hyper_R,		Qt::Key_Hyper_R,
    XK_Help,		Qt::Key_Help,
    0x1000FF74,         Qt::Key_BackTab,     // hardcoded HP backtab
    0x1005FF10,         Qt::Key_F11,         // hardcoded Sun F36 (labeled F11)
    0x1005FF11,         Qt::Key_F12,         // hardcoded Sun F37 (labeled F12)

    // Special multimedia keys
    // currently only tested with MS internet keyboard

    // browsing keys
    XF86XK_Back,	Qt::Key_Back,
    XF86XK_Forward,	Qt::Key_Forward,
    XF86XK_Stop,	Qt::Key_Stop,
    XF86XK_Refresh,	Qt::Key_Refresh,
    XF86XK_Favorites,	Qt::Key_Favorites,
    XF86XK_AudioMedia,	Qt::Key_LaunchMedia,
    XF86XK_OpenURL,	Qt::Key_OpenUrl,
    XF86XK_HomePage,	Qt::Key_HomePage,
    XF86XK_Search,	Qt::Key_Search,

    // media keys
    XF86XK_AudioLowerVolume, Qt::Key_VolumeDown,
    XF86XK_AudioMute,	Qt::Key_VolumeMute,
    XF86XK_AudioRaiseVolume, Qt::Key_VolumeUp,
    XF86XK_AudioPlay,	Qt::Key_MediaPlay,
    XF86XK_AudioStop,	Qt::Key_MediaStop,
    XF86XK_AudioPrev,	Qt::Key_MediaPrev,
    XF86XK_AudioNext,	Qt::Key_MediaNext,
    XF86XK_AudioRecord,	Qt::Key_MediaRecord,

    // launch keys
    XF86XK_Mail,	Qt::Key_LaunchMail,
    XF86XK_MyComputer,	Qt::Key_Launch0,
    XF86XK_Calculator,	Qt::Key_Launch1,
    XF86XK_Standby, 	Qt::Key_Standby,

    XF86XK_Launch0,	Qt::Key_Launch2,
    XF86XK_Launch1,	Qt::Key_Launch3,
    XF86XK_Launch2,	Qt::Key_Launch4,
    XF86XK_Launch3,	Qt::Key_Launch5,
    XF86XK_Launch4,	Qt::Key_Launch6,
    XF86XK_Launch5,	Qt::Key_Launch7,
    XF86XK_Launch6,	Qt::Key_Launch8,
    XF86XK_Launch7,	Qt::Key_Launch9,
    XF86XK_Launch8,	Qt::Key_LaunchA,
    XF86XK_Launch9,	Qt::Key_LaunchB,
    XF86XK_LaunchA,	Qt::Key_LaunchC,
    XF86XK_LaunchB,	Qt::Key_LaunchD,
    XF86XK_LaunchC,	Qt::Key_LaunchE,
    XF86XK_LaunchD,	Qt::Key_LaunchF,

    0,			0
};


static QIntDict<void>    *keyDict  = 0;
static QIntDict<void>    *textDict = 0;

static void deleteKeyDicts()
{
    if ( keyDict )
	delete keyDict;
    keyDict = 0;
    if ( textDict )
	delete textDict;
    textDict = 0;
}

#if !defined(QT_NO_XIM)
static const unsigned short katakanaKeysymsToUnicode[] = {
    0x0000, 0x3002, 0x300C, 0x300D, 0x3001, 0x30FB, 0x30F2, 0x30A1,
    0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30E3, 0x30E5, 0x30E7, 0x30C3,
    0x30FC, 0x30A2, 0x30A4, 0x30A6, 0x30A8, 0x30AA, 0x30AB, 0x30AD,
    0x30AF, 0x30B1, 0x30B3, 0x30B5, 0x30B7, 0x30B9, 0x30BB, 0x30BD,
    0x30BF, 0x30C1, 0x30C4, 0x30C6, 0x30C8, 0x30CA, 0x30CB, 0x30CC,
    0x30CD, 0x30CE, 0x30CF, 0x30D2, 0x30D5, 0x30D8, 0x30DB, 0x30DE,
    0x30DF, 0x30E0, 0x30E1, 0x30E2, 0x30E4, 0x30E6, 0x30E8, 0x30E9,
    0x30EA, 0x30EB, 0x30EC, 0x30ED, 0x30EF, 0x30F3, 0x309B, 0x309C
};

static const unsigned short cyrillicKeysymsToUnicode[] = {
    0x0000, 0x0452, 0x0453, 0x0451, 0x0454, 0x0455, 0x0456, 0x0457,
    0x0458, 0x0459, 0x045a, 0x045b, 0x045c, 0x0000, 0x045e, 0x045f,
    0x2116, 0x0402, 0x0403, 0x0401, 0x0404, 0x0405, 0x0406, 0x0407,
    0x0408, 0x0409, 0x040a, 0x040b, 0x040c, 0x0000, 0x040e, 0x040f,
    0x044e, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
    0x0445, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e,
    0x043f, 0x044f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
    0x044c, 0x044b, 0x0437, 0x0448, 0x044d, 0x0449, 0x0447, 0x044a,
    0x042e, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
    0x0425, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e,
    0x041f, 0x042f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
    0x042c, 0x042b, 0x0417, 0x0428, 0x042d, 0x0429, 0x0427, 0x042a
};

static const unsigned short greekKeysymsToUnicode[] = {
    0x0000, 0x0386, 0x0388, 0x0389, 0x038a, 0x03aa, 0x0000, 0x038c,
    0x038e, 0x03ab, 0x0000, 0x038f, 0x0000, 0x0000, 0x0385, 0x2015,
    0x0000, 0x03ac, 0x03ad, 0x03ae, 0x03af, 0x03ca, 0x0390, 0x03cc,
    0x03cd, 0x03cb, 0x03b0, 0x03ce, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
    0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
    0x03a0, 0x03a1, 0x03a3, 0x0000, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
    0x03a8, 0x03a9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7,
    0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
    0x03c0, 0x03c1, 0x03c3, 0x03c2, 0x03c4, 0x03c5, 0x03c6, 0x03c7,
    0x03c8, 0x03c9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static const unsigned short technicalKeysymsToUnicode[] = {
    0x0000, 0x23B7, 0x250C, 0x2500, 0x2320, 0x2321, 0x2502, 0x23A1,
    0x23A3, 0x23A4, 0x23A6, 0x239B, 0x239D, 0x239E, 0x23A0, 0x23A8,
    0x23AC, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x2264, 0x2260, 0x2265, 0x222B,
    0x2234, 0x221D, 0x221E, 0x0000, 0x0000, 0x2207, 0x0000, 0x0000,
    0x223C, 0x2243, 0x0000, 0x0000, 0x0000, 0x21D4, 0x21D2, 0x2261,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x221A, 0x0000,
    0x0000, 0x0000, 0x2282, 0x2283, 0x2229, 0x222A, 0x2227, 0x2228,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2202,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0192, 0x0000,
    0x0000, 0x0000, 0x0000, 0x2190, 0x2191, 0x2192, 0x2193, 0x0000
};

static const unsigned short specialKeysymsToUnicode[] = {
    0x25C6, 0x2592, 0x2409, 0x240C, 0x240D, 0x240A, 0x0000, 0x0000,
    0x2424, 0x240B, 0x2518, 0x2510, 0x250C, 0x2514, 0x253C, 0x23BA,
    0x23BB, 0x2500, 0x23BC, 0x23BD, 0x251C, 0x2524, 0x2534, 0x252C,
    0x2502, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

static const unsigned short publishingKeysymsToUnicode[] = {
    0x0000, 0x2003, 0x2002, 0x2004, 0x2005, 0x2007, 0x2008, 0x2009,
    0x200a, 0x2014, 0x2013, 0x0000, 0x0000, 0x0000, 0x2026, 0x2025,
    0x2153, 0x2154, 0x2155, 0x2156, 0x2157, 0x2158, 0x2159, 0x215a,
    0x2105, 0x0000, 0x0000, 0x2012, 0x2329, 0x0000, 0x232a, 0x0000,
    0x0000, 0x0000, 0x0000, 0x215b, 0x215c, 0x215d, 0x215e, 0x0000,
    0x0000, 0x2122, 0x2613, 0x0000, 0x25c1, 0x25b7, 0x25cb, 0x25af,
    0x2018, 0x2019, 0x201c, 0x201d, 0x211e, 0x0000, 0x2032, 0x2033,
    0x0000, 0x271d, 0x0000, 0x25ac, 0x25c0, 0x25b6, 0x25cf, 0x25ae,
    0x25e6, 0x25ab, 0x25ad, 0x25b3, 0x25bd, 0x2606, 0x2022, 0x25aa,
    0x25b2, 0x25bc, 0x261c, 0x261e, 0x2663, 0x2666, 0x2665, 0x0000,
    0x2720, 0x2020, 0x2021, 0x2713, 0x2717, 0x266f, 0x266d, 0x2642,
    0x2640, 0x260e, 0x2315, 0x2117, 0x2038, 0x201a, 0x201e, 0x0000
};

static const unsigned short aplKeysymsToUnicode[] = {
    0x0000, 0x0000, 0x0000, 0x003c, 0x0000, 0x0000, 0x003e, 0x0000,
    0x2228, 0x2227, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x00af, 0x0000, 0x22a5, 0x2229, 0x230a, 0x0000, 0x005f, 0x0000,
    0x0000, 0x0000, 0x2218, 0x0000, 0x2395, 0x0000, 0x22a4, 0x25cb,
    0x0000, 0x0000, 0x0000, 0x2308, 0x0000, 0x0000, 0x222a, 0x0000,
    0x2283, 0x0000, 0x2282, 0x0000, 0x22a2, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x22a3, 0x0000, 0x0000, 0x0000
};

static const unsigned short koreanKeysymsToUnicode[] = {
    0x0000, 0x3131, 0x3132, 0x3133, 0x3134, 0x3135, 0x3136, 0x3137,
    0x3138, 0x3139, 0x313a, 0x313b, 0x313c, 0x313d, 0x313e, 0x313f,
    0x3140, 0x3141, 0x3142, 0x3143, 0x3144, 0x3145, 0x3146, 0x3147,
    0x3148, 0x3149, 0x314a, 0x314b, 0x314c, 0x314d, 0x314e, 0x314f,
    0x3150, 0x3151, 0x3152, 0x3153, 0x3154, 0x3155, 0x3156, 0x3157,
    0x3158, 0x3159, 0x315a, 0x315b, 0x315c, 0x315d, 0x315e, 0x315f,
    0x3160, 0x3161, 0x3162, 0x3163, 0x11a8, 0x11a9, 0x11aa, 0x11ab,
    0x11ac, 0x11ad, 0x11ae, 0x11af, 0x11b0, 0x11b1, 0x11b2, 0x11b3,
    0x11b4, 0x11b5, 0x11b6, 0x11b7, 0x11b8, 0x11b9, 0x11ba, 0x11bb,
    0x11bc, 0x11bd, 0x11be, 0x11bf, 0x11c0, 0x11c1, 0x11c2, 0x316d,
    0x3171, 0x3178, 0x317f, 0x3181, 0x3184, 0x3186, 0x318d, 0x318e,
    0x11eb, 0x11f0, 0x11f9, 0x0000, 0x0000, 0x0000, 0x0000, 0x20a9
};


static QChar keysymToUnicode(unsigned char byte3, unsigned char byte4)
{
    if ( byte3 == 0x04 ) {
        // katakana
        if ( byte4 > 0xa0 && byte4 < 0xe0 )
           return QChar( katakanaKeysymsToUnicode[byte4 - 0xa0] );
        else if ( byte4 == 0x7e )
            return QChar( 0x203e ); // Overline
    } else if ( byte3 == 0x06 ) {
	// russian, use lookup table
	if ( byte4 > 0xa0 )
	    return QChar( cyrillicKeysymsToUnicode[byte4 - 0xa0] );
    } else if ( byte3 == 0x07 ) {
	// greek
	if ( byte4 > 0xa0 )
	    return QChar( greekKeysymsToUnicode[byte4 - 0xa0] );
    } else if ( byte3 == 0x08 ) {
       // technical
       if ( byte4 > 0xa0 )
           return QChar( technicalKeysymsToUnicode[byte4 - 0xa0] );
    } else if ( byte3 == 0x09 ) {
       // special
       if ( byte4 >= 0xe0 )
           return QChar( specialKeysymsToUnicode[byte4 - 0xe0] );
    } else if ( byte3 == 0x0a ) {
       // publishing
       if ( byte4 > 0xa0 )
           return QChar( publishingKeysymsToUnicode[byte4 - 0xa0] );
    } else if ( byte3 == 0x0b ) {
       // APL
       if ( byte4 > 0xa0 )
           return QChar( aplKeysymsToUnicode[byte4 - 0xa0] );
    } else if ( byte3 == 0x0e ) {
       // Korean
       if ( byte4 > 0xa0 )
           return QChar( koreanKeysymsToUnicode[byte4 - 0xa0] );
    }
    return QChar(0x0);
}
#endif


bool QETWidget::translateKeyEventInternal( const XEvent *event, int& count,
					   QString& text,
					   int& state,
					   char& ascii, int& code, QEvent::Type &type, bool willRepeat )
{
    QTextCodec *mapper = input_mapper;
    // some XmbLookupString implementations don't return buffer overflow correctly,
    // so we increase the input buffer to allow for long strings...
    // 256 chars * 2 bytes + 1 null-term == 513 bytes
    QCString chars(513);
    QChar converted;
    KeySym key = 0;

    if ( !keyDict ) {
	keyDict = new QIntDict<void>( 13 );
	keyDict->setAutoDelete( FALSE );
	textDict = new QIntDict<void>( 13 );
	textDict->setAutoDelete( FALSE );
	qAddPostRoutine( deleteKeyDicts );
    }

    QWidget* tlw = topLevelWidget();

    XKeyEvent xkeyevent = event->xkey;

    // save the modifier state, we will use the keystate uint later by passing
    // it to translateButtonState
    uint keystate = event->xkey.state;
    // remove the modifiers where mode_switch exists... HPUX machines seem
    // to have alt *AND* mode_switch both in Mod1Mask, which causes
    // XLookupString to return things like 'å' (aring) for ALT-A.  This
    // completely breaks modifiers.  If we remove the modifier for Mode_switch,
    // then things work correctly...
    xkeyevent.state &= ~qt_mode_switch_remove_mask;

    type = (event->type == XKeyPress)
           ? QEvent::KeyPress : QEvent::KeyRelease;
#if defined(QT_NO_XIM)

    count = XLookupString( &xkeyevent, chars.data(), chars.size(), &key, 0 );

    if ( count == 1 )
	ascii = chars[0];

#else
    // Implementation for X11R5 and newer, using XIM

    int	       keycode = event->xkey.keycode;
    Status     status;

    if ( type == QEvent::KeyPress ) {
	bool mb=FALSE;
	if ( qt_xim ) {
	    QTLWExtra*  xd = tlw->topData();
	    QInputContext *qic = (QInputContext *) xd->xic;
	    if ( qic ) {
		mb=TRUE;
		count = qic->lookupString(&xkeyevent, chars, &key, &status);
	    }
	}
	if ( !mb ) {
	    count = XLookupString( &xkeyevent,
				   chars.data(), chars.size(), &key, 0 );
	}
	if ( count && !keycode ) {
	    keycode = composingKeycode;
	    composingKeycode = 0;
	}
	if ( key )
	    keyDict->replace( keycode, (void*)key );
	// all keysyms smaller than that are actally keys that can be mapped
	// to unicode chars
	if ( count == 0 && key < 0xff00 ) {
	    unsigned char byte3 = (unsigned char )(key >> 8);
	    int mib = -1;
	    switch( byte3 ) {
	    case 0: // Latin 1
	    case 1: // Latin 2
	    case 2: //latin 3
	    case 3: // latin4
		mib = byte3 + 4; break;
	    case 5: // arabic
		mib = 82; break;
	    case 12: // Hebrew
		mib = 85; break;
	    case 13: // Thai
		mib = 2259; break;
	    case 4: // kana
	    case 6: // cyrillic
	    case 7: // greek
	    case 8: // technical, no mapping here at the moment
	    case 9: // Special
	    case 10: // Publishing
	    case 11: // APL
	    case 14: // Korean, no mapping
		mib = -1; // manual conversion
		mapper = 0;
		converted = keysymToUnicode( byte3, key & 0xff );
	    case 0x20:
		// currency symbols
		if ( key >= 0x20a0 && key <= 0x20ac ) {
		    mib = -1; // manual conversion
		    mapper = 0;
		    converted = (uint)key;
		}
		break;
	    default:
		break;
	    }
	    if ( mib != -1 ) {
		mapper = QTextCodec::codecForMib( mib );
		chars[0] = (unsigned char) (key & 0xff); // get only the fourth bit for conversion later
		count++;
	    }
	} else if ( key >= 0x1000000 && key <= 0x100ffff ) {
	    converted = (ushort) (key - 0x1000000);
	    mapper = 0;
	}
	if ( count < (int)chars.size()-1 )
	    chars[count] = '\0';
	if ( count == 1 ) {
	    ascii = chars[0];
	    // +256 so we can store all eight-bit codes, including ascii 0,
	    // and independent of whether char is signed or not.
	    textDict->replace( keycode, (void*)(long)(256+ascii) );
	}
	tlw = 0;
    } else {
	key = (int)(long)keyDict->find( keycode );
	if ( key )
	    if( !willRepeat ) // Take out key of dictionary only if this call.
		keyDict->take( keycode );
	long s = (long)textDict->find( keycode );
	if ( s ) {
	    textDict->take( keycode );
	    ascii = (char)(s-256);
	}
    }
#endif // !QT_NO_XIM

    state = translateButtonState( keystate );

    static int directionKeyEvent = 0;
    if ( qt_use_rtl_extensions && type == QEvent::KeyRelease ) {
	if (directionKeyEvent == Key_Direction_R || directionKeyEvent == Key_Direction_L ) {
	    type = QEvent::KeyPress;
	    code = directionKeyEvent;
	    chars[0] = 0;
	    directionKeyEvent = 0;
	    return TRUE;
	} else {
	    directionKeyEvent = 0;
	}
    }

    // Watch for keypresses and if its a key belonging to the Ctrl-Shift
    // direction-changing accel, remember it.
    // We keep track of those keys instead of using the event's state
    // (to figure out whether the Ctrl modifier is held while Shift is pressed,
    // or Shift is held while Ctrl is pressed) since the 'state' doesn't tell
    // us whether the modifier held is Left or Right.
    if (qt_use_rtl_extensions && type  == QEvent::KeyPress)
        if (key == XK_Control_L || key == XK_Control_R || key == XK_Shift_L || key == XK_Shift_R) {
           if (!directionKeyEvent)
	      directionKeyEvent = key;
        } else {
           // this can no longer be a direction-changing accel.
	   // if any other key was pressed.
           directionKeyEvent = Key_Space;
        }

    // Commentary in X11/keysymdef says that X codes match ASCII, so it
    // is safe to use the locale functions to process X codes in ISO8859-1.
    //
    // This is mainly for compatibility - applications should not use the
    // Qt keycodes between 128 and 255, but should rather use the
    // QKeyEvent::text().
    //
    if ( key < 128 || (key < 256 && (!input_mapper || input_mapper->mibEnum()==4)) ) {
	code = isprint((int)key) ? toupper((int)key) : 0; // upper-case key, if known
    } else if ( key >= XK_F1 && key <= XK_F35 ) {
	code = Key_F1 + ((int)key - XK_F1);	// function keys
    } else if ( key >= XK_KP_0 && key <= XK_KP_9) {
	code = Key_0 + ((int)key - XK_KP_0);	// numeric keypad keys
	state |= Keypad;
    } else {
	int i = 0;				// any other keys
	while ( KeyTbl[i] ) {
	    if ( key == KeyTbl[i] ) {
		code = (int)KeyTbl[i+1];
		break;
	    }
	    i += 2;
	}
	switch ( key ) {
	case XK_KP_Insert:
	case XK_KP_Delete:
	case XK_KP_Home:
	case XK_KP_End:
	case XK_KP_Left:
	case XK_KP_Up:
	case XK_KP_Right:
	case XK_KP_Down:
	case XK_KP_Prior:
	case XK_KP_Next:
	case XK_KP_Space:
	case XK_KP_Tab:
	case XK_KP_Enter:
	case XK_KP_Equal:
	case XK_KP_Multiply:
	case XK_KP_Add:
	case XK_KP_Separator:
	case XK_KP_Subtract:
	case XK_KP_Decimal:
	case XK_KP_Divide:
	    state |= Keypad;
	    break;
	default:
	    break;
	}

	if ( code == Key_Tab &&
	     (state & ShiftButton) == ShiftButton ) {
            // map shift+tab to shift+backtab, QAccel knows about it
            // and will handle it.
	    code = Key_Backtab;
	    chars[0] = 0;
	}

	if ( qt_use_rtl_extensions && type  == QEvent::KeyPress ) {
	    if ( directionKeyEvent ) {
		if ( key == XK_Shift_L && directionKeyEvent == XK_Control_L ||
		     key == XK_Control_L && directionKeyEvent == XK_Shift_L ) {
		    directionKeyEvent = Key_Direction_L;
		} else if ( key == XK_Shift_R && directionKeyEvent == XK_Control_R ||
			    key == XK_Control_R && directionKeyEvent == XK_Shift_R ) {
		    directionKeyEvent = Key_Direction_R;
		}
	    }
	    else if ( directionKeyEvent == Key_Direction_L || directionKeyEvent == Key_Direction_R ) {
		directionKeyEvent = Key_Space; // invalid
	    }
	}
    }

#if 0
#ifndef Q_EE
    static int c  = 0;
    extern void qt_dialog_default_key();
#define Q_EE(x) c = (c == x || (!c && x == 0x1000) )? x+1 : 0
    if ( tlw && state == '0' ) {
	switch ( code ) {
	case 0x4f: Q_EE(Key_Backtab); break;
	case 0x52: Q_EE(Key_Tab); break;
	case 0x54: Q_EE(Key_Escape); break;
	case 0x4c:
	    if (c == Key_Return )
		qt_dialog_default_key();
	    else
		Q_EE(Key_Backspace);
	    break;
	}
    }
#undef Q_EE
#endif
#endif

    // convert chars (8bit) to text (unicode).
    if ( mapper )
	text = mapper->toUnicode(chars,count);
    else if ( !mapper && converted.unicode() != 0x0 )
	text = converted;
    else
	text = chars;
    return TRUE;
}


struct qt_auto_repeat_data
{
    // match the window and keycode with timestamp delta of 10ms
    Window window;
    KeyCode keycode;
    Time timestamp;

    // queue scanner state
    bool release;
    bool error;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static Bool qt_keypress_scanner(Display *, XEvent *event, XPointer arg)
{
    if (event->type != XKeyPress && event->type != XKeyRelease)
        return FALSE;

    qt_auto_repeat_data *d = (qt_auto_repeat_data *) arg;
    if (d->error ||
        event->xkey.window  != d->window ||
        event->xkey.keycode != d->keycode)
        return FALSE;

    if (event->type == XKeyPress) {
        d->error = (! d->release || event->xkey.time - d->timestamp > 10);
        return (! d->error);
    }

    // must be XKeyRelease event
    if (d->release) {
        // found a second release
        d->error = TRUE;
        return FALSE;
    }

    // found a single release
    d->release = TRUE;
    d->timestamp = event->xkey.time;

    return FALSE;
}

static Bool qt_keyrelease_scanner(Display *, XEvent *event, XPointer arg)
{
    const qt_auto_repeat_data *d = (const qt_auto_repeat_data *) arg;
    return (event->type == XKeyRelease &&
            event->xkey.window  == d->window &&
            event->xkey.keycode == d->keycode);
}

#if defined(Q_C_CALLBACKS)
}
#endif

bool QETWidget::translateKeyEvent( const XEvent *event, bool grab )
{
    int	   code = -1;
    int	   count = 0;
    int	   state;
    char   ascii = 0;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    Display *dpy = x11Display();

    if ( !isEnabled() )
	return TRUE;

    QEvent::Type type;
    bool    autor = FALSE;
    QString text;

    translateKeyEventInternal( event, count, text, state, ascii, code, type,
			       qt_mode_switch_remove_mask != 0 );

    static uint curr_autorep = 0;
    // was this the last auto-repeater?
    qt_auto_repeat_data auto_repeat_data;
    auto_repeat_data.window = event->xkey.window;
    auto_repeat_data.keycode = event->xkey.keycode;
    auto_repeat_data.timestamp = event->xkey.time;

    if ( event->type == XKeyPress ) {
        if ( curr_autorep == event->xkey.keycode ) {
            autor = TRUE;
            curr_autorep = 0;
        }
    } else {
	// look ahead for auto-repeat
        XEvent nextpress;

        auto_repeat_data.release = TRUE;
        auto_repeat_data.error = FALSE;
        if (XCheckIfEvent(dpy, &nextpress, &qt_keypress_scanner,
                          (XPointer) &auto_repeat_data)) {
            autor = TRUE;

	    // Put it back... we COULD send the event now and not need
	    // the static curr_autorep variable.
	    XPutBackEvent(dpy,&nextpress);
	}
	curr_autorep = autor ? event->xkey.keycode : 0;
    }

    // process accelerators before doing key compression
    if ( type == QEvent::KeyPress && !grab ) {
	// send accel events if the keyboard is not grabbed
	QKeyEvent a( type, code, ascii, state, text, autor,
		     QMAX( QMAX(count,1), int(text.length())) );
	if ( qt_tryAccelEvent( this, &a ) )
	    return TRUE;
    }

    long save = 0;
    if ( qt_mode_switch_remove_mask != 0 ) {
	save = qt_mode_switch_remove_mask;
	qt_mode_switch_remove_mask = 0;

	// translate the key event again, but this time apply any Mode_switch
	// modifiers
	translateKeyEventInternal( event, count, text, state, ascii, code, type );
    }

    // compress keys
    if ( !text.isEmpty() && testWState(WState_CompressKeys) &&
	 // do not compress keys if the key event we just got above matches
	 // one of the key ranges used to compute stopCompression
	 ! ( ( code >= Key_Escape && code <= Key_SysReq ) ||
	     ( code >= Key_Home && code <= Key_Next ) ||
	     ( code >= Key_Super_L && code <= Key_Direction_R ) ||
	     ( ( code == 0 ) && ( ascii == '\n' ) ) ) ) {
	// the widget wants key compression so it gets it
	int	codeIntern = -1;
	int	countIntern = 0;
	int	stateIntern;
	char	asciiIntern = 0;
	XEvent	evRelease;
	XEvent	evPress;

	// sync the event queue, this makes key compress work better
	XSync( dpy, FALSE );

	for (;;) {
	    QString textIntern;
	    if ( !XCheckTypedWindowEvent(dpy,event->xkey.window,
					 XKeyRelease,&evRelease) )
		break;
	    if ( !XCheckTypedWindowEvent(dpy,event->xkey.window,
					 XKeyPress,&evPress) ) {
		XPutBackEvent(dpy, &evRelease);
		break;
	    }
	    QEvent::Type t;
	    translateKeyEventInternal( &evPress, countIntern, textIntern,
				       stateIntern, asciiIntern, codeIntern, t );
	    // use stopCompression to stop key compression for the following
	    // key event ranges:
	    bool stopCompression =
		// 1) misc keys
		( codeIntern >= Key_Escape && codeIntern <= Key_SysReq ) ||
		// 2) cursor movement
		( codeIntern >= Key_Home && codeIntern <= Key_Next ) ||
		// 3) extra keys
		( codeIntern >= Key_Super_L && codeIntern <= Key_Direction_R ) ||
		// 4) something that a) doesn't translate to text or b) translates
		//    to newline text
		((codeIntern == 0) && (asciiIntern == '\n'));
	    if (stateIntern == state && !textIntern.isEmpty() && !stopCompression) {
		text += textIntern;
		count += countIntern;
	    } else {
		XPutBackEvent(dpy, &evPress);
		XPutBackEvent(dpy, &evRelease);
		break;
	    }
	}
    }

    if ( save != 0 )
	qt_mode_switch_remove_mask = save;

    // autorepeat compression makes sense for all widgets (Windows
    // does it automatically .... )
    if ( event->type == XKeyPress && text.length() <= 1 ) {
	XEvent dummy;

        for (;;) {
            auto_repeat_data.release = FALSE;
            auto_repeat_data.error = FALSE;
            if (! XCheckIfEvent(dpy, &dummy, &qt_keypress_scanner,
                                (XPointer) &auto_repeat_data))
                break;
            if (! XCheckIfEvent(dpy, &dummy, &qt_keyrelease_scanner,
                                (XPointer) &auto_repeat_data))
                break;

	    count++;
	    if (!text.isEmpty())
		text += text[0];
	}
    }

    if (code == 0 && ascii == '\n') {
	code = Key_Return;
	ascii = '\r';
	text = "\r";
    }

    // try the menukey first
    if ( type == QEvent::KeyPress && code == Qt::Key_Menu ) {
	QContextMenuEvent e( QContextMenuEvent::Keyboard, QPoint( 5, 5 ), mapToGlobal( QPoint( 5, 5 ) ), 0 );
	QApplication::sendSpontaneousEvent( this, &e );
	if( e.isAccepted() )
	    return TRUE;
    }

    QKeyEvent e( type, code, ascii, state, text, autor,
		 QMAX(QMAX(count,1), int(text.length())) );
    return QApplication::sendSpontaneousEvent( this, &e );
}


//
// Paint event translation
//
// When receiving many expose events, we compress them (union of all expose
// rectangles) into one event which is sent to the widget.

struct PaintEventInfo {
    Window window;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static Bool isPaintOrScrollDoneEvent( Display *, XEvent *ev, XPointer a )
{
    PaintEventInfo *info = (PaintEventInfo *)a;
    if ( ev->type == Expose || ev->type == GraphicsExpose
      ||    ev->type == ClientMessage
	 && ev->xclient.message_type == qt_qt_scrolldone )
    {
	if ( ev->xexpose.window == info->window )
	    return True;
    }
    return False;
}

#if defined(Q_C_CALLBACKS)
}
#endif


// declared above: static QPtrList<QScrollInProgress> *sip_list = 0;

void qt_insert_sip( QWidget* scrolled_widget, int dx, int dy )
{
    if ( !sip_list ) {
	sip_list = new QPtrList<QScrollInProgress>;
	sip_list->setAutoDelete( TRUE );
    }

    QScrollInProgress* sip = new QScrollInProgress( scrolled_widget, dx, dy );
    sip_list->append( sip );

    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = scrolled_widget->winId();
    client_message.format = 32;
    client_message.message_type = qt_qt_scrolldone;
    client_message.data.l[0] = sip->id;

    XSendEvent( appDpy, scrolled_widget->winId(), False, NoEventMask,
	(XEvent*)&client_message );
}

int qt_sip_count( QWidget* scrolled_widget )
{
    if ( !sip_list )
	return 0;

    int sips=0;

    for (QScrollInProgress* sip = sip_list->first();
	sip; sip=sip_list->next())
    {
	if ( sip->scrolled_widget == scrolled_widget )
	    sips++;
    }

    return sips;
}

static
bool translateBySips( QWidget* that, QRect& paintRect )
{
    if ( sip_list ) {
	int dx=0, dy=0;
	int sips=0;
	for (QScrollInProgress* sip = sip_list->first();
	    sip; sip=sip_list->next())
	{
	    if ( sip->scrolled_widget == that ) {
		if ( sips ) {
		    dx += sip->dx;
		    dy += sip->dy;
		}
		sips++;
	    }
	}
	if ( sips > 1 ) {
	    paintRect.moveBy( dx, dy );
	    return TRUE;
	}
    }
    return FALSE;
}

bool QETWidget::translatePaintEvent( const XEvent *event )
{
    setWState( WState_Exposed );
    QRect  paintRect( event->xexpose.x,	   event->xexpose.y,
		      event->xexpose.width, event->xexpose.height );
    bool   merging_okay = !testWFlags(WPaintClever);
    XEvent xevent;
    PaintEventInfo info;
    info.window = winId();
    bool should_clip = translateBySips( this, paintRect );

    QRegion paintRegion( paintRect );

    if ( merging_okay ) {
	// WARNING: this is O(number_of_events * number_of_matching_events)
	while ( XCheckIfEvent(x11Display(),&xevent,isPaintOrScrollDoneEvent,
			      (XPointer)&info) &&
		!qt_x11EventFilter(&xevent)  &&
		!x11Event( &xevent ) ) // send event through filter
	{
	    if ( xevent.type == Expose || xevent.type == GraphicsExpose ) {
		QRect exposure(xevent.xexpose.x,
			       xevent.xexpose.y,
			       xevent.xexpose.width,
			       xevent.xexpose.height);
		if ( translateBySips( this, exposure ) )
		    should_clip = TRUE;
		paintRegion = paintRegion.unite( exposure );
	    } else {
		translateScrollDoneEvent( &xevent );
	    }
	}
    }

    if ( should_clip ) {
	paintRegion = paintRegion.intersect( rect() );
	if ( paintRegion.isEmpty() )
	    return TRUE;
    }

    QPaintEvent e( paintRegion );
    setWState( WState_InPaintEvent );
    if ( !isTopLevel() && backgroundOrigin() != WidgetOrigin )
	erase( paintRegion );
    qt_set_paintevent_clipping( this, paintRegion );
    QApplication::sendSpontaneousEvent( this, &e );
    qt_clear_paintevent_clipping();
    clearWState( WState_InPaintEvent );
    return TRUE;
}

//
// Scroll-done event translation.
//

bool QETWidget::translateScrollDoneEvent( const XEvent *event )
{
    if ( !sip_list ) return FALSE;

    long id = event->xclient.data.l[0];

    // Remove any scroll-in-progress record for the given id.
    for (QScrollInProgress* sip = sip_list->first(); sip; sip=sip_list->next()) {
	if ( sip->id == id ) {
	    sip_list->remove( sip_list->current() );
	    return TRUE;
	}
    }

    return FALSE;
}


//
// ConfigureNotify (window move and resize) event translation

bool QETWidget::translateConfigEvent( const XEvent *event )
{
    // config pending is only set on resize, see qwidget_x11.cpp, internalSetGeometry()
    bool was_resize = testWState( WState_ConfigPending );

    clearWState(WState_ConfigPending);

    if ( isTopLevel() ) {
	QPoint newCPos( geometry().topLeft() );
	QSize  newSize( event->xconfigure.width, event->xconfigure.height );

	bool trust = (topData()->parentWinId == None ||
		      topData()->parentWinId == QPaintDevice::x11AppRootWindow());

	if (event->xconfigure.send_event || trust ) {
	    // if a ConfigureNotify comes from a real sendevent request, we can
	    // trust its values.
	    newCPos.rx() = event->xconfigure.x + event->xconfigure.border_width;
	    newCPos.ry() = event->xconfigure.y + event->xconfigure.border_width;
	}

	if ( isVisible() )
	    QApplication::syncX();

        if (! extra || extra->compress_events) {
            // ConfigureNotify compression for faster opaque resizing
            XEvent otherEvent;
            while ( XCheckTypedWindowEvent( x11Display(), winId(), ConfigureNotify,
                                            &otherEvent ) ) {
                if ( qt_x11EventFilter( &otherEvent ) )
                    continue;

                if (x11Event( &otherEvent ) )
                    continue;

                if ( otherEvent.xconfigure.event != otherEvent.xconfigure.window )
                    continue;

                newSize.setWidth( otherEvent.xconfigure.width );
                newSize.setHeight( otherEvent.xconfigure.height );

                if ( otherEvent.xconfigure.send_event || trust ) {
                    newCPos.rx() = otherEvent.xconfigure.x +
                                   otherEvent.xconfigure.border_width;
                    newCPos.ry() = otherEvent.xconfigure.y +
                                   otherEvent.xconfigure.border_width;
                }
            }
        }

	QRect cr ( geometry() );
	if ( newSize != cr.size() ) { // size changed
	    was_resize = TRUE;
	    QSize oldSize = size();
	    cr.setSize( newSize );
	    crect = cr;

	    if ( isVisible() ) {
		QResizeEvent e( newSize, oldSize );
		QApplication::sendSpontaneousEvent( this, &e );
	    } else {
		QResizeEvent * e = new QResizeEvent( newSize, oldSize );
		QApplication::postEvent( this, e );
	    }
	}

	if ( newCPos != cr.topLeft() ) { // compare with cpos (exluding frame)
	    QPoint oldPos = geometry().topLeft();
	    cr.moveTopLeft( newCPos );
	    crect = cr;
	    if ( isVisible() ) {
		QMoveEvent e( newCPos, oldPos ); // pos (including frame), not cpos
		QApplication::sendSpontaneousEvent( this, &e );
	    } else {
		QMoveEvent * e = new QMoveEvent( newCPos, oldPos );
		QApplication::postEvent( this, e );
	    }
	}
    } else {
	XEvent xevent;
	while ( XCheckTypedWindowEvent(x11Display(),winId(), ConfigureNotify,&xevent) &&
		!qt_x11EventFilter(&xevent)  &&
		!x11Event( &xevent ) ) // send event through filter
	    ;
    }

    bool transbg = backgroundOrigin() != WidgetOrigin;
    // we ignore NorthWestGravity at the moment for reversed layout
    if ( transbg ||
	 (!testWFlags( WStaticContents ) &&
	  testWState( WState_Exposed ) && was_resize ) ||
	 QApplication::reverseLayout() ) {
	// remove unnecessary paint events from the queue
	XEvent xevent;
	while ( XCheckTypedWindowEvent( x11Display(), winId(), Expose, &xevent ) &&
		! qt_x11EventFilter( &xevent )  &&
		! x11Event( &xevent ) ) // send event through filter
	    ;
	repaint( !testWFlags(WResizeNoErase) || transbg );
    }

    return TRUE;
}


//
// Close window event translation.
//
bool QETWidget::translateCloseEvent( const XEvent * )
{
    return close(FALSE);
}


/*!
    Sets the text cursor's flash (blink) time to \a msecs
    milliseconds. The flash time is the time required to display,
    invert and restore the caret display. Usually the text cursor is
    displayed for \a msecs/2 milliseconds, then hidden for \a msecs/2
    milliseconds, but this may vary.

    Note that on Microsoft Windows, calling this function sets the
    cursor flash time for all windows.

    \sa cursorFlashTime()
*/
void  QApplication::setCursorFlashTime( int msecs )
{
    cursor_flash_time = msecs;
}


/*!
    Returns the text cursor's flash (blink) time in milliseconds. The
    flash time is the time required to display, invert and restore the
    caret display.

    The default value on X11 is 1000 milliseconds. On Windows, the
    control panel value is used.

    Widgets should not cache this value since it may be changed at any
    time by the user changing the global desktop settings.

    \sa setCursorFlashTime()
*/
int QApplication::cursorFlashTime()
{
    return cursor_flash_time;
}

/*!
    Sets the time limit that distinguishes a double click from two
    consecutive mouse clicks to \a ms milliseconds.

    Note that on Microsoft Windows, calling this function sets the
    double click interval for all windows.

    \sa doubleClickInterval()
*/

void QApplication::setDoubleClickInterval( int ms )
{
    mouse_double_click_time = ms;
}


/*!
    Returns the maximum duration for a double click.

    The default value on X11 is 400 milliseconds. On Windows, the
    control panel value is used.

    \sa setDoubleClickInterval()
*/

int QApplication::doubleClickInterval()
{
    return mouse_double_click_time;
}


/*!
    Sets the number of lines to scroll when the mouse wheel is rotated
    to \a n.

    If this number exceeds the number of visible lines in a certain
    widget, the widget should interpret the scroll operation as a
    single page up / page down operation instead.

    \sa wheelScrollLines()
*/
void QApplication::setWheelScrollLines( int n )
{
    wheel_scroll_lines = n;
}

/*!
    Returns the number of lines to scroll when the mouse wheel is
    rotated.

    \sa setWheelScrollLines()
*/
int QApplication::wheelScrollLines()
{
    return wheel_scroll_lines;
}

/*!
    Enables the UI effect \a effect if \a enable is TRUE, otherwise
    the effect will not be used.

    Note: All effects are disabled on screens running at less than
    16-bit color depth.

    \sa isEffectEnabled(), Qt::UIEffect, setDesktopSettingsAware()
*/
void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    switch (effect) {
    case UI_AnimateMenu:
	if ( enable ) fade_menu = FALSE;
	animate_menu = enable;
	break;
    case UI_FadeMenu:
	if ( enable )
	    animate_menu = TRUE;
	fade_menu = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
	break;
    case UI_AnimateTooltip:
	if ( enable ) fade_tooltip = FALSE;
	animate_tooltip = enable;
	break;
    case UI_FadeTooltip:
	if ( enable )
	    animate_tooltip = TRUE;
	fade_tooltip = enable;
	break;
    case UI_AnimateToolBox:
	animate_toolbox = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }
}

/*!
    Returns TRUE if \a effect is enabled; otherwise returns FALSE.

    By default, Qt will try to use the desktop settings. Call
    setDesktopSettingsAware(FALSE) to prevent this.

    Note: All effects are disabled on screens running at less than
    16-bit color depth.

    \sa setEffectEnabled(), Qt::UIEffect
*/
bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    if ( QColor::numBitPlanes() < 16 || !animate_ui )
	return FALSE;

    switch( effect ) {
    case UI_AnimateMenu:
	return animate_menu;
    case UI_FadeMenu:
	return fade_menu;
    case UI_AnimateCombo:
	return animate_combo;
    case UI_AnimateTooltip:
	return animate_tooltip;
    case UI_FadeTooltip:
	return fade_tooltip;
    case UI_AnimateToolBox:
	return animate_toolbox;
    default:
	return animate_ui;
    }
}

/*****************************************************************************
  Session management support
 *****************************************************************************/

#ifndef QT_NO_SM_SUPPORT

#include <X11/SM/SMlib.h>

class QSessionManagerData
{
public:
    QSessionManagerData( QSessionManager* mgr, QString& id, QString& key )
	: sm( mgr ), sessionId( id ), sessionKey( key ) {}
    QSessionManager* sm;
    QStringList restartCommand;
    QStringList discardCommand;
    QString& sessionId;
    QString& sessionKey;
    QSessionManager::RestartHint restartHint;
};

class QSmSocketReceiver : public QObject
{
    Q_OBJECT
public:
    QSmSocketReceiver( int socket )
	: QObject(0,0)
	{
	    QSocketNotifier* sn = new QSocketNotifier( socket, QSocketNotifier::Read, this );
	    connect( sn, SIGNAL( activated(int) ), this, SLOT( socketActivated(int) ) );
	}

public slots:
     void socketActivated(int);
};


static SmcConn smcConnection = 0;
static bool sm_interactionActive;
static bool sm_smActive;
static int sm_interactStyle;
static int sm_saveType;
static bool sm_cancel;
// static bool sm_waitingForPhase2;  ### never used?!?
static bool sm_waitingForInteraction;
static bool sm_isshutdown;
// static bool sm_shouldbefast;  ### never used?!?
static bool sm_phase2;
static bool sm_in_phase2;

static QSmSocketReceiver* sm_receiver = 0;

static void resetSmState();
static void sm_setProperty( const char* name, const char* type,
			    int num_vals, SmPropValue* vals);
static void sm_saveYourselfCallback( SmcConn smcConn, SmPointer clientData,
				  int saveType, Bool shutdown , int interactStyle, Bool fast);
static void sm_saveYourselfPhase2Callback( SmcConn smcConn, SmPointer clientData ) ;
static void sm_dieCallback( SmcConn smcConn, SmPointer clientData ) ;
static void sm_shutdownCancelledCallback( SmcConn smcConn, SmPointer clientData );
static void sm_saveCompleteCallback( SmcConn smcConn, SmPointer clientData );
static void sm_interactCallback( SmcConn smcConn, SmPointer clientData );
static void sm_performSaveYourself( QSessionManagerData* );

static void resetSmState()
{
//    sm_waitingForPhase2 = FALSE; ### never used?!?
    sm_waitingForInteraction = FALSE;
    sm_interactionActive = FALSE;
    sm_interactStyle = SmInteractStyleNone;
    sm_smActive = FALSE;
    sm_blockUserInput = FALSE;
    sm_isshutdown = FALSE;
//    sm_shouldbefast = FALSE; ### never used?!?
    sm_phase2 = FALSE;
    sm_in_phase2 = FALSE;
}


// theoretically it's possible to set several properties at once. For
// simplicity, however, we do just one property at a time
static void sm_setProperty( const char* name, const char* type,
			    int num_vals, SmPropValue* vals)
{
    if (num_vals ) {
      SmProp prop;
      prop.name = (char*)name;
      prop.type = (char*)type;
      prop.num_vals = num_vals;
      prop.vals = vals;

      SmProp* props[1];
      props[0] = &prop;
      SmcSetProperties( smcConnection, 1, props );
    }
    else {
      char* names[1];
      names[0] = (char*) name;
      SmcDeleteProperties( smcConnection, 1, names );
    }
}

static void sm_setProperty( const QString& name, const QString& value)
{
    SmPropValue prop;
    prop.length = value.length();
    prop.value = (SmPointer) value.latin1();
    sm_setProperty( name.latin1(), SmARRAY8, 1, &prop );
}

static void sm_setProperty( const QString& name, const QStringList& value)
{
    SmPropValue *prop = new SmPropValue[ value.count() ];
    int count = 0;
    for ( QStringList::ConstIterator it = value.begin(); it != value.end(); ++it ) {
      prop[ count ].length = (*it).length();
      prop[ count ].value = (char*)(*it).latin1();
      ++count;
    }
    sm_setProperty( name.latin1(), SmLISTofARRAY8, count, prop );
    delete [] prop;
}


// workaround for broken libsm, see below
struct QT_smcConn {
    unsigned int save_yourself_in_progress : 1;
    unsigned int shutdown_in_progress : 1;
};

static void sm_saveYourselfCallback( SmcConn smcConn, SmPointer clientData,
				  int saveType, Bool shutdown , int interactStyle, Bool /*fast*/)
{
    if (smcConn != smcConnection )
	return;
    sm_cancel = FALSE;
    sm_smActive = TRUE;
    sm_isshutdown = shutdown;
    sm_saveType = saveType;
    sm_interactStyle = interactStyle;
//    sm_shouldbefast = fast; ### never used?!?

    // ugly workaround for broken libSM. libSM should do that _before_
    // actually invoking the callback in sm_process.c
    ( (QT_smcConn*)smcConn )->save_yourself_in_progress = TRUE;
    if ( sm_isshutdown )
	( (QT_smcConn*)smcConn )->shutdown_in_progress = TRUE;

    sm_performSaveYourself( (QSessionManagerData*) clientData );
    if ( !sm_isshutdown ) // we cannot expect a confirmation message in that case
	resetSmState();
}

static void sm_performSaveYourself( QSessionManagerData* smd )
{
    if ( sm_isshutdown )
	sm_blockUserInput = TRUE;

    QSessionManager* sm = smd->sm;

    // generate a new session key
    timeval tv;
    gettimeofday( &tv, 0 );
    smd->sessionKey  = QString::number( tv.tv_sec ) + "_" + QString::number(tv.tv_usec);

    // tell the session manager about our program in best POSIX style
    sm_setProperty( SmProgram, QString( qApp->argv()[0] ) );
    // tell the session manager about our user as well.
    struct passwd* entry = getpwuid( geteuid() );
    if ( entry )
	sm_setProperty( SmUserID, QString::fromLatin1( entry->pw_name ) );

    // generate a restart and discard command that makes sense
    QStringList restart;
    restart  << qApp->argv()[0] << "-session" << smd->sessionId + "_" + smd->sessionKey;
    if (qstricmp(qAppName(), qAppClass()) != 0)
	restart << "-name" << qAppName();
    sm->setRestartCommand( restart );
    QStringList discard;
    sm->setDiscardCommand( discard );

    switch ( sm_saveType ) {
    case SmSaveBoth:
	qApp->commitData( *sm );
	if ( sm_isshutdown && sm_cancel)
	    break; // we cancelled the shutdown, no need to save state
    // fall through
    case SmSaveLocal:
	qApp->saveState( *sm );
	break;
    case SmSaveGlobal:
	qApp->commitData( *sm );
	break;
    default:
	break;
    }

    if ( sm_phase2 && !sm_in_phase2 ) {
	SmcRequestSaveYourselfPhase2( smcConnection, sm_saveYourselfPhase2Callback, (SmPointer*) smd );
	sm_blockUserInput = FALSE;
    }
    else {
	// close eventual interaction monitors and cancel the
	// shutdown, if required. Note that we can only cancel when
	// performing a shutdown, it does not work for checkpoints
	if ( sm_interactionActive ) {
	    SmcInteractDone( smcConnection, sm_isshutdown && sm_cancel);
	    sm_interactionActive = FALSE;
	}
	else if ( sm_cancel && sm_isshutdown ) {
	    if ( sm->allowsErrorInteraction() ) {
		SmcInteractDone( smcConnection, True );
		sm_interactionActive = FALSE;
	    }
	}

	// set restart and discard command in session manager
	sm_setProperty( SmRestartCommand, sm->restartCommand() );
	sm_setProperty( SmDiscardCommand, sm->discardCommand() );

	// set the restart hint
	SmPropValue prop;
	prop.length = sizeof( int );
	int value = sm->restartHint();
	prop.value = (SmPointer) &value;
	sm_setProperty( SmRestartStyleHint, SmCARD8, 1, &prop );

	// we are done
	SmcSaveYourselfDone( smcConnection, !sm_cancel );
    }
}

static void sm_dieCallback( SmcConn smcConn, SmPointer /* clientData  */)
{
    if (smcConn != smcConnection )
	return;
    resetSmState();
    QEvent quitEvent(QEvent::Quit);
    QApplication::sendEvent(qApp, &quitEvent);
}

static void sm_shutdownCancelledCallback( SmcConn smcConn, SmPointer /* clientData */)
{
    if (smcConn != smcConnection )
	return;
    if ( sm_waitingForInteraction )
	qApp->exit_loop();
    resetSmState();
}

static void sm_saveCompleteCallback( SmcConn smcConn, SmPointer /*clientData */)
{
    if (smcConn != smcConnection )
	return;
    resetSmState();
}

static void sm_interactCallback( SmcConn smcConn, SmPointer /* clientData */ )
{
    if (smcConn != smcConnection )
	return;
    if ( sm_waitingForInteraction )
	qApp->exit_loop();
}

static void sm_saveYourselfPhase2Callback( SmcConn smcConn, SmPointer clientData )
{
    if (smcConn != smcConnection )
	return;
    sm_in_phase2 = TRUE;
    sm_performSaveYourself( (QSessionManagerData*) clientData );
}


void QSmSocketReceiver::socketActivated(int)
{
    IceProcessMessages( SmcGetIceConnection( smcConnection ), 0, 0);
}


#undef Bool
#include "qapplication_x11.moc"

QSessionManager::QSessionManager( QApplication * app, QString &id, QString& key )
    : QObject( app, "session manager" )
{
    d = new QSessionManagerData( this, id, key );
    d->restartHint = RestartIfRunning;

    resetSmState();
    char cerror[256];
    char* myId = 0;
    char* prevId = (char*)id.latin1(); // we know what we are doing

    SmcCallbacks cb;
    cb.save_yourself.callback = sm_saveYourselfCallback;
    cb.save_yourself.client_data = (SmPointer) d;
    cb.die.callback = sm_dieCallback;
    cb.die.client_data = (SmPointer) d;
    cb.save_complete.callback = sm_saveCompleteCallback;
    cb.save_complete.client_data = (SmPointer) d;
    cb.shutdown_cancelled.callback = sm_shutdownCancelledCallback;
    cb.shutdown_cancelled.client_data = (SmPointer) d;

    // avoid showing a warning message below
    const char* session_manager = getenv("SESSION_MANAGER");
    if ( !session_manager || !session_manager[0] )
	return;

    smcConnection = SmcOpenConnection( 0, 0, 1, 0,
				       SmcSaveYourselfProcMask |
				       SmcDieProcMask |
				       SmcSaveCompleteProcMask |
				       SmcShutdownCancelledProcMask,
				       &cb,
				       prevId,
				       &myId,
				       256, cerror );

    id = QString::fromLatin1( myId );
    ::free( myId ); // it was allocated by C

    QString error = cerror;
    if (!smcConnection ) {
	qWarning("Session management error: %s", error.latin1() );
    }
    else {
	sm_receiver = new QSmSocketReceiver(  IceConnectionNumber( SmcGetIceConnection( smcConnection ) ) );
    }
}

QSessionManager::~QSessionManager()
{
    if ( smcConnection )
      SmcCloseConnection( smcConnection, 0, 0 );
    smcConnection = 0;
    delete sm_receiver;
    delete d;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

QString QSessionManager::sessionKey() const
{
    return d->sessionKey;
}


void* QSessionManager::handle() const
{
    return (void*) smcConnection;
}


bool QSessionManager::allowsInteraction()
{
    if ( sm_interactionActive )
	return TRUE;

    if ( sm_waitingForInteraction )
	return FALSE;

    if ( sm_interactStyle == SmInteractStyleAny ) {
	sm_waitingForInteraction =  SmcInteractRequest( smcConnection, SmDialogNormal,
							sm_interactCallback, (SmPointer*) this );
    }
    if ( sm_waitingForInteraction ) {
	qApp->enter_loop();
	sm_waitingForInteraction = FALSE;
	if ( sm_smActive ) { // not cancelled
	    sm_interactionActive = TRUE;
	    sm_blockUserInput = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

bool QSessionManager::allowsErrorInteraction()
{
    if ( sm_interactionActive )
	return TRUE;

    if ( sm_waitingForInteraction )
	return FALSE;

    if ( sm_interactStyle == SmInteractStyleAny || sm_interactStyle == SmInteractStyleErrors ) {
	sm_waitingForInteraction =  SmcInteractRequest( smcConnection, SmDialogError,
							sm_interactCallback, (SmPointer*) this );
    }
    if ( sm_waitingForInteraction ) {
	qApp->enter_loop();
	sm_waitingForInteraction = FALSE;
	if ( sm_smActive ) { // not cancelled
	    sm_interactionActive = TRUE;
	    sm_blockUserInput = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

void QSessionManager::release()
{
    if ( sm_interactionActive ) {
	SmcInteractDone( smcConnection, False );
	sm_interactionActive = FALSE;
	if ( sm_smActive && sm_isshutdown )
	    sm_blockUserInput = TRUE;
    }
}

void QSessionManager::cancel()
{
    sm_cancel = TRUE;
}

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

void QSessionManager::setManagerProperty( const QString& name, const QString& value)
{
    SmPropValue prop;
    prop.length = value.length();
    prop.value = (SmPointer) value.utf8().data();
    sm_setProperty( name.latin1(), SmARRAY8, 1, &prop );
}

void QSessionManager::setManagerProperty( const QString& name, const QStringList& value)
{
    SmPropValue *prop = new SmPropValue[ value.count() ];
    int count = 0;
    for ( QStringList::ConstIterator it = value.begin(); it != value.end(); ++it ) {
      prop[ count ].length = (*it).length();
      prop[ count ].value = (char*)(*it).utf8().data();
      ++count;
    }
    sm_setProperty( name.latin1(), SmLISTofARRAY8, count, prop );
    delete [] prop;
}

bool QSessionManager::isPhase2() const
{
    return sm_in_phase2;
}

void QSessionManager::requestPhase2()
{
    sm_phase2 = TRUE;
}


#endif // QT_NO_SM_SUPPORT
