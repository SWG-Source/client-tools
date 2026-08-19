/****************************************************************************
** $Id: qgl.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#include "qgl.h"
#include "qpixmap.h"
#include "qpaintdevicemetrics.h"
#include "qimage.h"
#include "qcleanuphandler.h"
#include "qptrdict.h"

static QGLFormat* qgl_default_format = 0;
static QGLFormat* qgl_default_overlay_format = 0;

#if defined(Q_WS_X11)
#include "private/qt_x11_p.h"
#define INT32 dummy_INT32
#define INT8 dummy_INT8
#include <GL/glx.h>
#undef INT32
#undef INT8
#include "qgl_x11_p.h"
#endif

static QCleanupHandler<QGLFormat> qgl_cleanup_format;


/*!
    \class QGL qgl.h
    \brief The QGL class is a namespace for miscellaneous identifiers
    in the Qt OpenGL module.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \module OpenGL
    \ingroup graphics
    \ingroup images


    Normally you can ignore this class. QGLWidget and the other
    OpenGL<sup>*</sup> module classes inherit it, so when you make your
    own QGLWidget subclass you can use the identifiers in the QGL
    namespace without qualification.

    However, you may occasionally find yourself in situations where you
    need to refer to these identifiers from outside the QGL namespace's
    scope, e.g. in static functions. In such cases, simply write e.g. \c
    QGL::DoubleBuffer instead of just \c DoubleBuffer.

    <sup>*</sup> OpenGL is a trademark of Silicon Graphics, Inc. in the
    United States and other countries.

*/


/*****************************************************************************
  QGLFormat implementation
 *****************************************************************************/


/*!
    \class QGLFormat qgl.h
    \brief The QGLFormat class specifies the display format of an OpenGL
    rendering context.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup graphics
    \ingroup images
    \module OpenGL

    A display format has several characteristics:
    \list
    \i \link setDoubleBuffer() Double or single buffering.\endlink
    \i \link setDepth() Depth buffer.\endlink
    \i \link setRgba() RGBA or color index mode.\endlink
    \i \link setAlpha() Alpha channel.\endlink
    \i \link setAccum() Accumulation buffer.\endlink
    \i \link setStencil() Stencil buffer.\endlink
    \i \link setStereo() Stereo buffers.\endlink
    \i \link setDirectRendering() Direct rendering.\endlink
    \i \link setOverlay() Presence of an overlay.\endlink
    \i \link setPlane() The plane of an overlay format.\endlink
    \endlist

    You create and tell a QGLFormat object what rendering options you
    want from an OpenGL<sup>*</sup> rendering context.

    OpenGL drivers or accelerated hardware may or may not support
    advanced features such as alpha channel or stereographic viewing.
    If you request some features that the driver/hardware does not
    provide when you create a QGLWidget, you will get a rendering
    context with the nearest subset of features.

    There are different ways to define the display characteristics of
    a rendering context. One is to create a QGLFormat and make it the
    default for the entire application:
    \code
    QGLFormat f;
    f.setAlpha( TRUE );
    f.setStereo( TRUE );
    QGLFormat::setDefaultFormat( f );
    \endcode

    Or you can specify the desired format when creating an object of
    your QGLWidget subclass:
    \code
    QGLFormat f;
    f.setDoubleBuffer( FALSE );                 // single buffer
    f.setDirectRendering( FALSE );              // software rendering
    MyGLWidget* myWidget = new MyGLWidget( f, ... );
    \endcode

    After the widget has been created, you can find out which of the
    requested features the system was able to provide:
    \code
    QGLFormat f;
    f.setOverlay( TRUE );
    f.setStereo( TRUE );
    MyGLWidget* myWidget = new MyGLWidget( f, ... );
    if ( !w->format().stereo() ) {
	// ok, goggles off
	if ( !w->format().hasOverlay() ) {
	    qFatal( "Cool hardware required" );
	}
    }
    \endcode

    <sup>*</sup> OpenGL is a trademark of Silicon Graphics, Inc. in the
    United States and other countries.

    \sa QGLContext, QGLWidget
*/


/*!
    Constructs a QGLFormat object with the factory default settings:
    \list
    \i \link setDoubleBuffer() Double buffer:\endlink Enabled.
    \i \link setDepth() Depth buffer:\endlink Enabled.
    \i \link setRgba() RGBA:\endlink Enabled (i.e., color index disabled).
    \i \link setAlpha() Alpha channel:\endlink Disabled.
    \i \link setAccum() Accumulator buffer:\endlink Disabled.
    \i \link setStencil() Stencil buffer:\endlink Disabled.
    \i \link setStereo() Stereo:\endlink Disabled.
    \i \link setDirectRendering() Direct rendering:\endlink Enabled.
    \i \link setOverlay() Overlay:\endlink Disabled.
    \i \link setPlane() Plane:\endlink 0 (i.e., normal plane).
    \endlist
*/

QGLFormat::QGLFormat()
{
    opts = DoubleBuffer | DepthBuffer | Rgba | DirectRendering;
    pln = 0;
}


/*!
    Creates a QGLFormat object that is a copy of the current \link
    defaultFormat() application default format\endlink.

    If \a options is not 0, this copy is modified by these format
    options. The \a options parameter should be \c FormatOption values
    OR'ed together.

    This constructor makes it easy to specify a certain desired format
    in classes derived from QGLWidget, for example:
    \code
    // The rendering in MyGLWidget depends on using
    // stencil buffer and alpha channel
    MyGLWidget::MyGLWidget( QWidget* parent, const char* name )
	: QGLWidget( QGLFormat( StencilBuffer | AlphaChannel ), parent, name )
    {
	if ( !format().stencil() )
	    qWarning( "Could not get stencil buffer; results will be suboptimal" );
	if ( !format().alphaChannel() )
	    qWarning( "Could not get alpha channel; results will be suboptimal" );
	...
    }
    \endcode

    Note that there are \c FormatOption values to turn format settings
    both on and off, e.g. \c DepthBuffer and \c NoDepthBuffer,
    \c DirectRendering and \c IndirectRendering, etc.

    The \a plane parameter defaults to 0 and is the plane which this
    format should be associated with. Not all OpenGL implmentations
    supports overlay/underlay rendering planes.

    \sa defaultFormat(), setOption()
*/

QGLFormat::QGLFormat( int options, int plane )
{
    uint newOpts = options;
    opts = defaultFormat().opts;
    opts |= ( newOpts & 0xffff );
    opts &= ~( newOpts >> 16 );
    pln = plane;
}


/*!
    \fn bool QGLFormat::doubleBuffer() const

    Returns TRUE if double buffering is enabled; otherwise returns
    FALSE. Double buffering is enabled by default.

    \sa setDoubleBuffer()
*/

/*!
    If \a enable is TRUE sets double buffering; otherwise sets single
    buffering.

    Double buffering is enabled by default.

    Double buffering is a technique where graphics are rendered on an
    off-screen buffer and not directly to the screen. When the drawing
    has been completed, the program calls a swapBuffers() function to
    exchange the screen contents with the buffer. The result is
    flicker-free drawing and often better performance.

    \sa doubleBuffer(), QGLContext::swapBuffers(),
    QGLWidget::swapBuffers()
*/

void QGLFormat::setDoubleBuffer( bool enable )
{
    setOption( enable ? DoubleBuffer : SingleBuffer );
}


/*!
    \fn bool QGLFormat::depth() const

    Returns TRUE if the depth buffer is enabled; otherwise returns
    FALSE. The depth buffer is enabled by default.

    \sa setDepth()
*/

/*!
    If \a enable is TRUE enables the depth buffer; otherwise disables
    the depth buffer.

    The depth buffer is enabled by default.

    The purpose of a depth buffer (or Z-buffering) is to remove hidden
    surfaces. Pixels are assigned Z values based on the distance to
    the viewer. A pixel with a high Z value is closer to the viewer
    than a pixel with a low Z value. This information is used to
    decide whether to draw a pixel or not.

    \sa depth()
*/

void QGLFormat::setDepth( bool enable )
{
    setOption( enable ? DepthBuffer : NoDepthBuffer );
}


/*!
    \fn bool QGLFormat::rgba() const

    Returns TRUE if RGBA color mode is set. Returns FALSE if color
    index mode is set. The default color mode is RGBA.

    \sa setRgba()
*/

/*!
    If \a enable is TRUE sets RGBA mode. If \a enable is FALSE sets
    color index mode.

    The default color mode is RGBA.

    RGBA is the preferred mode for most OpenGL applications. In RGBA
    color mode you specify colors as red + green + blue + alpha
    quadruplets.

    In color index mode you specify an index into a color lookup
    table.

    \sa rgba()
*/

void QGLFormat::setRgba( bool enable )
{
    setOption( enable ? Rgba : ColorIndex );
}


/*!
    \fn bool QGLFormat::alpha() const

    Returns TRUE if the alpha channel of the framebuffer is enabled;
    otherwise returns FALSE. The alpha channel is disabled by default.

    \sa setAlpha()
*/

/*!
    If \a enable is TRUE enables the alpha channel; otherwise disables
    the alpha channel.

    The alpha buffer is disabled by default.

    The alpha channel is typically used for implementing transparency
    or translucency. The A in RGBA specifies the transparency of a
    pixel.

    \sa alpha()
*/

void QGLFormat::setAlpha( bool enable )
{
    setOption( enable ? AlphaChannel : NoAlphaChannel );
}


/*!
    \fn bool QGLFormat::accum() const

    Returns TRUE if the accumulation buffer is enabled; otherwise
    returns FALSE. The accumulation buffer is disabled by default.

    \sa setAccum()
*/

/*!
    If \a enable is TRUE enables the accumulation buffer; otherwise
    disables the accumulation buffer.

    The accumulation buffer is disabled by default.

    The accumulation buffer is used to create blur effects and
    multiple exposures.

    \sa accum()
*/

void QGLFormat::setAccum( bool enable )
{
    setOption( enable ? AccumBuffer : NoAccumBuffer );
}


/*!
    \fn bool QGLFormat::stencil() const

    Returns TRUE if the stencil buffer is enabled; otherwise returns
    FALSE. The stencil buffer is disabled by default.

    \sa setStencil()
*/

/*!
    If \a enable is TRUE enables the stencil buffer; otherwise
    disables the stencil buffer.

    The stencil buffer is disabled by default.

    The stencil buffer masks certain parts of the drawing area so that
    masked parts are not drawn on.

    \sa stencil()
*/

void QGLFormat::setStencil( bool enable )
{
    setOption( enable ? StencilBuffer: NoStencilBuffer );
}


/*!
    \fn bool QGLFormat::stereo() const

    Returns TRUE if stereo buffering is enabled; otherwise returns
    FALSE. Stereo buffering is disabled by default.

    \sa setStereo()
*/

/*!
    If \a enable is TRUE enables stereo buffering; otherwise disables
    stereo buffering.

    Stereo buffering is disabled by default.

    Stereo buffering provides extra color buffers to generate left-eye
    and right-eye images.

    \sa stereo()
*/

void QGLFormat::setStereo( bool enable )
{
    setOption( enable ? StereoBuffers : NoStereoBuffers );
}


/*!
    \fn bool QGLFormat::directRendering() const

    Returns TRUE if direct rendering is enabled; otherwise returns
    FALSE.

    Direct rendering is enabled by default.

    \sa setDirectRendering()
*/

/*!
    If \a enable is TRUE enables direct rendering; otherwise disables
    direct rendering.

    Direct rendering is enabled by default.

    Enabling this option will make OpenGL bypass the underlying window
    system and render directly from hardware to the screen, if this is
    supported by the system.

    \sa directRendering()
*/

void QGLFormat::setDirectRendering( bool enable )
{
    setOption( enable ? DirectRendering : IndirectRendering );
}


/*!
    \fn bool QGLFormat::hasOverlay() const

    Returns TRUE if overlay plane is enabled; otherwise returns FALSE.

    Overlay is disabled by default.

    \sa setOverlay()
*/

/*!
    If \a enable is TRUE enables an overlay plane; otherwise disables
    the overlay plane.

    Enabling the overlay plane will cause QGLWidget to create an
    additional context in an overlay plane. See the QGLWidget
    documentation for further information.

    \sa hasOverlay()
*/

void QGLFormat::setOverlay( bool enable )
{
    setOption( enable ? HasOverlay : NoOverlay );
}

/*!
    Returns the plane of this format. The default for normal formats
    is 0, which means the normal plane. The default for overlay
    formats is 1, which is the first overlay plane.

    \sa setPlane()
*/
int QGLFormat::plane() const
{
    return pln;
}

/*!
    Sets the requested plane to \a plane. 0 is the normal plane, 1 is
    the first overlay plane, 2 is the second overlay plane, etc.; -1,
    -2, etc. are underlay planes.

    Note that in contrast to other format specifications, the plane
    specifications will be matched exactly. This means that if you
    specify a plane that the underlying OpenGL system cannot provide,
    an \link QGLWidget::isValid() invalid\endlink QGLWidget will be
    created.

    \sa plane()
*/
void QGLFormat::setPlane( int plane )
{
    pln = plane;
}

/*!
    Sets the format option to \a opt.

    \sa testOption()
*/

void QGLFormat::setOption( FormatOption opt )
{
    if ( opt & 0xffff )
	opts |= opt;
    else
       opts &= ~( opt >> 16 );
}



/*!
    Returns TRUE if format option \a opt is set; otherwise returns FALSE.

    \sa setOption()
*/

bool QGLFormat::testOption( FormatOption opt ) const
{
    if ( opt & 0xffff )
       return ( opts & opt ) != 0;
    else
       return ( opts & ( opt >> 16 ) ) == 0;
}



/*!
    \fn bool QGLFormat::hasOpenGL()

    Returns TRUE if the window system has any OpenGL support;
    otherwise returns FALSE.

    \warning This function must not be called until the QApplication
    object has been created.
*/



/*!
    \fn bool QGLFormat::hasOpenGLOverlays()

    Returns TRUE if the window system supports OpenGL overlays;
    otherwise returns FALSE.

    \warning This function must not be called until the QApplication
    object has been created.
*/

/*!
    Returns the default QGLFormat for the application. All QGLWidgets
    that are created use this format unless another format is
    specified, e.g. when they are constructed.

    If no special default format has been set using
    setDefaultFormat(), the default format is the same as that created
    with QGLFormat().

    \sa setDefaultFormat()
*/

QGLFormat QGLFormat::defaultFormat()
{
    if ( !qgl_default_format ) {
	qgl_default_format = new QGLFormat;
	qgl_cleanup_format.add( &qgl_default_format );
    }
    return *qgl_default_format;
}

/*!
    Sets a new default QGLFormat for the application to \a f. For
    example, to set single buffering as the default instead of double
    buffering, your main() might contain code like this:
    \code
    QApplication a(argc, argv);
    QGLFormat f;
    f.setDoubleBuffer( FALSE );
    QGLFormat::setDefaultFormat( f );
    \endcode

    \sa defaultFormat()
*/

void QGLFormat::setDefaultFormat( const QGLFormat &f )
{
    if ( !qgl_default_format ) {
	qgl_default_format = new QGLFormat;
	qgl_cleanup_format.add( &qgl_default_format );
    }
    *qgl_default_format = f;
}


/*!
    Returns the default QGLFormat for overlay contexts.

    The factory default overlay format is:
    \list
    \i \link setDoubleBuffer() Double buffer:\endlink Disabled.
    \i \link setDepth() Depth buffer:\endlink Disabled.
    \i \link setRgba() RGBA:\endlink Disabled (i.e., color index enabled).
    \i \link setAlpha() Alpha channel:\endlink Disabled.
    \i \link setAccum() Accumulator buffer:\endlink Disabled.
    \i \link setStencil() Stencil buffer:\endlink Disabled.
    \i \link setStereo() Stereo:\endlink Disabled.
    \i \link setDirectRendering() Direct rendering:\endlink Enabled.
    \i \link setOverlay() Overlay:\endlink Disabled.
    \i \link setPlane() Plane:\endlink 1 (i.e., first overlay plane).
    \endlist

    \sa setDefaultFormat()
*/

QGLFormat QGLFormat::defaultOverlayFormat()
{
    if ( !qgl_default_overlay_format ) {
	qgl_default_overlay_format = new QGLFormat;
	qgl_default_overlay_format->opts = DirectRendering;
	qgl_default_overlay_format->pln = 1;
	qgl_cleanup_format.add( &qgl_default_overlay_format );
    }
    return *qgl_default_overlay_format;
}

/*!
    Sets a new default QGLFormat for overlay contexts to \a f. This
    format is used whenever a QGLWidget is created with a format that
    hasOverlay() enabled.

    For example, to get a double buffered overlay context (if
    available), use code like this:

    \code
    QGLFormat f = QGLFormat::defaultOverlayFormat();
    f.setDoubleBuffer( TRUE );
    QGLFormat::setDefaultOverlayFormat( f );
    \endcode

    As usual, you can find out after widget creation whether the
    underlying OpenGL system was able to provide the requested
    specification:

    \code
    // ...continued from above
    MyGLWidget* myWidget = new MyGLWidget( QGLFormat( QGL::HasOverlay ), ... );
    if ( myWidget->format().hasOverlay() ) {
	// Yes, we got an overlay, let's check _its_ format:
	QGLContext* olContext = myWidget->overlayContext();
	if ( olContext->format().doubleBuffer() )
	    ; // yes, we got a double buffered overlay
	else
	    ; // no, only single buffered overlays are available
    }
    \endcode

    \sa defaultOverlayFormat()
*/

void QGLFormat::setDefaultOverlayFormat( const QGLFormat &f )
{
    if ( !qgl_default_overlay_format ) {
	qgl_default_overlay_format = new QGLFormat;
	qgl_cleanup_format.add( &qgl_default_overlay_format );
    }
    *qgl_default_overlay_format = f;
    // Make sure the user doesn't request that the overlays themselves
    // have overlays, since it is unlikely that the system supports
    // infinitely many planes...
    qgl_default_overlay_format->setOverlay( FALSE );
}


/*!
    Returns TRUE if all the options of the two QGLFormats are equal;
    otherwise returns FALSE.
*/

bool operator==( const QGLFormat& a, const QGLFormat& b )
{
    return (a.opts == b.opts) && (a.pln == b.pln);
}


/*!
    Returns FALSE if all the options of the two QGLFormats are equal;
    otherwise returns TRUE.
*/

bool operator!=( const QGLFormat& a, const QGLFormat& b )
{
    return !( a == b );
}



/*****************************************************************************
  QGLContext implementation
 *****************************************************************************/

QGLContext* QGLContext::currentCtx = 0;

/*!
    \class QGLContext qgl.h
    \brief The QGLContext class encapsulates an OpenGL rendering context.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup graphics
    \ingroup images
    \module OpenGL

    An OpenGL<sup>*</sup> rendering context is a complete set of
    OpenGL state variables.

    The context's \link QGL::FormatOption format\endlink is set in the
    constructor or later with setFormat(). The format options that are
    actually set are returned by format(); the options you asked for
    are returned by requestedFormat(). Note that after a QGLContext
    object has been constructed, the actual OpenGL context must be
    created by explicitly calling the \link create() create()\endlink
    function. The makeCurrent() function makes this context the
    current rendering context. You can make \e no context current
    using doneCurrent(). The reset() function will reset the context
    and make it invalid.

    You can examine properties of the context with, e.g. isValid(),
    isSharing(), initialized(), windowCreated() and
    overlayTransparentColor().

    If you're using double buffering you can swap the screen contents
    with the off-screen buffer using swapBuffers().

    Please note that QGLContext is not thread safe.

    <sup>*</sup> OpenGL is a trademark of Silicon Graphics, Inc. in the
    United States and other countries.

*/


/*!
    Constructs an OpenGL context for the paint device \a device, which
    can be a widget or a pixmap. The \a format specifies several
    display options for the context.

    If the underlying OpenGL/Window system cannot satisfy all the
    features requested in \a format, the nearest subset of features
    will be used. After creation, the format() method will return the
    actual format obtained.

    Note that after a QGLContext object has been constructed, \link
    create() create()\endlink must be called explicitly to create
    the actual OpenGL context. The context will be \link isValid()
    invalid\endlink if it was not possible to obtain a GL context at
    all.

    \sa format(), isValid()
*/

QGLContext::QGLContext( const QGLFormat &format, QPaintDevice *device )
    : glFormat(format), reqFormat(format)
{
    init( device );
}

/*!
    \overload
    \internal
*/
QGLContext::QGLContext( const QGLFormat &format )
    : glFormat( format ), reqFormat(format)
{
    init();
}

/*!
    Destroys the OpenGL context and frees its resources.
*/

QGLContext::~QGLContext()
{
    reset();
    if ( d )
	delete d;
}


/*!
    \fn QGLFormat QGLContext::format() const

    Returns the frame buffer format that was obtained (this may be a
    subset of what was requested).

    \sa requestedFormat()
*/

/*!
    \fn QGLFormat QGLContext::requestedFormat() const

    Returns the frame buffer format that was originally requested in
    the constructor or setFormat().

    \sa format()
*/

/*!
    Sets a \a format for this context. The context is \link reset()
    reset\endlink.

    Call create() to create a new GL context that tries to match the
    new format.

    \code
    QGLContext *cx;
    //  ...
    QGLFormat f;
    f.setStereo( TRUE );
    cx->setFormat( f );
    if ( !cx->create() )
	exit(); // no OpenGL support, or cannot render on the specified paintdevice
    if ( !cx->format().stereo() )
	exit(); // could not create stereo context
    \endcode

    \sa format(), reset(), create()
*/

void QGLContext::setFormat( const QGLFormat &format )
{
    reset();
    glFormat = reqFormat = format;
}

/*!
    \internal
*/
void QGLContext::setDevice( QPaintDevice *pDev )
{
    if ( isValid() )
	reset();
    d->paintDevice = pDev;
    if ( d->paintDevice && (d->paintDevice->devType() != QInternal::Widget
			&& d->paintDevice->devType() != QInternal::Pixmap) ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QGLContext: Unsupported paint device type" );
#endif
    }
}

void QGLContext::init( QPaintDevice *dev )
{
    d = new Private;
    d->valid = FALSE;
#if defined(Q_WS_X11)
    qt_resolve_gl_symbols();
    gpm = 0;
#endif
    setDevice( dev );
#if defined(Q_WS_WIN)
    dc = 0;
    win = 0;
    pixelFormatId = 0;
    cmap = 0;
#endif
#if defined(Q_WS_MAC)
    d->oldR = QRect(1, 1, 1, 1);
#endif
    d->crWin = FALSE;
    d->initDone = FALSE;
    d->sharing = FALSE;
}

/*!
    \fn bool QGLContext::isValid() const

    Returns TRUE if a GL rendering context has been successfully
    created; otherwise returns FALSE.
*/

/*!
    \fn void QGLContext::setValid( bool valid )
    \internal

    Forces the GL rendering context to be valid.
*/

/*!
    \fn bool QGLContext::isSharing() const

    Returns TRUE if display list sharing with another context was
    requested in the create() call and the GL system was able to
    fulfill this request; otherwise returns FALSE. Note that display
    list sharing might not be supported between contexts with
    different formats.
*/

/*!
    \fn bool QGLContext::deviceIsPixmap() const

    Returns TRUE if the paint device of this context is a pixmap;
    otherwise returns FALSE.
*/

/*!
    \fn bool QGLContext::windowCreated() const

    Returns TRUE if a window has been created for this context;
    otherwise returns FALSE.

    \sa setWindowCreated()
*/

/*!
    \fn void QGLContext::setWindowCreated( bool on )

    If \a on is TRUE the context has had a window created for it. If
    \a on is FALSE no window has been created for the context.

    \sa windowCreated()
*/

/*!
    \fn uint QGLContext::colorIndex( const QColor& c ) const

    \internal

    Returns a colormap index for the color c, in ColorIndex mode. Used
    by qglColor() and qglClearColor().
*/


/*!
    \fn bool QGLContext::initialized() const

    Returns TRUE if this context has been initialized, i.e. if
    QGLWidget::initializeGL() has been performed on it; otherwise
    returns FALSE.

    \sa setInitialized()
*/

/*!
    \fn void QGLContext::setInitialized( bool on )

    If \a on is TRUE the context has been initialized, i.e.
    QGLContext::setInitialized() has been called on it. If \a on is
    FALSE the context has not been initialized.

    \sa initialized()
*/

/*!
    \fn const QGLContext* QGLContext::currentContext()

    Returns the current context, i.e. the context to which any OpenGL
    commands will currently be directed. Returns 0 if no context is
    current.

    \sa makeCurrent()
*/

/*!
    \fn QColor QGLContext::overlayTransparentColor() const

    If this context is a valid context in an overlay plane, returns
    the plane's transparent color. Otherwise returns an \link
    QColor::isValid() invalid \endlink color.

    The returned color's \link QColor::pixel() pixel \endlink value is
    the index of the transparent color in the colormap of the overlay
    plane. (Naturally, the color's RGB values are meaningless.)

    The returned QColor object will generally work as expected only
    when passed as the argument to QGLWidget::qglColor() or
    QGLWidget::qglClearColor(). Under certain circumstances it can
    also be used to draw transparent graphics with a QPainter. See the
    examples/opengl/overlay_x11 example for details.
*/


/*!
    Creates the GL context. Returns TRUE if it was successful in
    creating a valid GL rendering context on the paint device
    specified in the constructor; otherwise returns FALSE (i.e. the
    context is invalid).

    After successful creation, format() returns the set of features of
    the created GL rendering context.

    If \a shareContext points to a valid QGLContext, this method will
    try to establish OpenGL display list sharing between this context
    and the \a shareContext. Note that this may fail if the two
    contexts have different formats. Use isSharing() to see if sharing
    succeeded.

    \warning Implementation note: initialization of C++ class
    members usually takes place in the class constructor. QGLContext
    is an exception because it must be simple to customize. The
    virtual functions chooseContext() (and chooseVisual() for X11) can
    be reimplemented in a subclass to select a particular context. The
    problem is that virtual functions are not properly called during
    construction (even though this is correct C++) because C++
    constructs class hierarchies from the bottom up. For this reason
    we need a create() function.

    \sa chooseContext(), format(), isValid()
*/

bool QGLContext::create( const QGLContext* shareContext )
{
    reset();
    d->valid = chooseContext( shareContext );
    return d->valid;
}



/*!
    \fn bool QGLContext::chooseContext( const QGLContext* shareContext = 0 )

    This semi-internal function is called by create(). It creates a
    system-dependent OpenGL handle that matches the format() of \a
    shareContext as closely as possible.

    On Windows, it calls the virtual function choosePixelFormat(),
    which finds a matching pixel format identifier. On X11, it calls
    the virtual function chooseVisual() which finds an appropriate X
    visual. On other platforms it may work differently.
*/


/*!
    \fn void QGLContext::reset()

    Resets the context and makes it invalid.

    \sa create(), isValid()
*/


/*!
    \fn void QGLContext::makeCurrent()

    Makes this context the current OpenGL rendering context. All GL
    functions you call operate on this context until another context
    is made current.

    In some very rare cases the underlying call may fail. If this
    occurs an error message is output to stderr.
*/


/*!
    \fn void QGLContext::swapBuffers() const

    Swaps the screen contents with an off-screen buffer. Only works if
    the context is in double buffer mode.

    \sa QGLFormat::setDoubleBuffer()
*/


/*!
    \fn void QGLContext::doneCurrent()

    Makes no GL context the current context. Normally, you do not need
    to call this function; QGLContext calls it as necessary.
*/


/*!
    \fn QPaintDevice* QGLContext::device() const

    Returns the paint device set for this context.

    \sa QGLContext::QGLContext()
*/

/*!
    \fn void QGLContext::generateFontDisplayLists( const QFont& font, int listBase )

    Generates a set of 256 display lists for the 256 first characters
    in the font \a font. The first list will start at index \a listBase.

    \sa QGLWidget::renderText()
*/


/*****************************************************************************
  QGLWidget implementation
 *****************************************************************************/


/*!
    \class QGLWidget qgl.h
    \brief The QGLWidget class is a widget for rendering OpenGL graphics.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup graphics
    \ingroup images
    \mainclass
    \module OpenGL

    QGLWidget provides functionality for displaying OpenGL<sup>*</sup>
    graphics integrated into a Qt application. It is very simple to
    use. You inherit from it and use the subclass like any other
    QWidget, except that instead of drawing the widget's contents
    using QPainter etc. you use the standard OpenGL rendering
    commands.

    QGLWidget provides three convenient virtual functions that you can
    reimplement in your subclass to perform the typical OpenGL tasks:

    \list
    \i paintGL() - Renders the OpenGL scene. Gets called whenever the widget
    needs to be updated.
    \i resizeGL() - Sets up the OpenGL viewport, projection, etc. Gets
    called whenever the the widget has been resized (and also when it
    is shown for the first time because all newly created widgets get a
    resize event automatically).
    \i initializeGL() - Sets up the OpenGL rendering context, defines display
    lists, etc. Gets called once before the first time resizeGL() or
    paintGL() is called.
    \endlist

    Here is a rough outline of how a QGLWidget subclass might look:

    \code
    class MyGLDrawer : public QGLWidget
    {
	Q_OBJECT	// must include this if you use Qt signals/slots

    public:
	MyGLDrawer( QWidget *parent, const char *name )
	    : QGLWidget(parent, name) {}

    protected:

	void initializeGL()
	{
	    // Set up the rendering context, define display lists etc.:
	    ...
	    glClearColor( 0.0, 0.0, 0.0, 0.0 );
	    glEnable(GL_DEPTH_TEST);
	    ...
	}

	void resizeGL( int w, int h )
	{
	    // setup viewport, projection etc.:
	    glViewport( 0, 0, (GLint)w, (GLint)h );
	    ...
	    glFrustum( ... );
	    ...
	}

	void paintGL()
	{
	    // draw the scene:
	    ...
	    glRotatef( ... );
	    glMaterialfv( ... );
	    glBegin( GL_QUADS );
	    glVertex3f( ... );
	    glVertex3f( ... );
	    ...
	    glEnd();
	    ...
	}

    };
    \endcode

    If you need to trigger a repaint from places other than paintGL()
    (a typical example is when using \link QTimer timers\endlink to
    animate scenes), you should call the widget's updateGL() function.

    Your widget's OpenGL rendering context is made current when
    paintGL(), resizeGL(), or initializeGL() is called. If you need to
    call the standard OpenGL API functions from other places (e.g. in
    your widget's constructor or in your own paint functions), you
    must call makeCurrent() first.

    QGLWidget provides functions for requesting a new display \link
    QGLFormat format\endlink and you can also create widgets with
    customized rendering \link QGLContext contexts\endlink.

    You can also share OpenGL display lists between QGLWidgets (see
    the documentation of the QGLWidget constructors for details).

    \section1 Overlays

    The QGLWidget creates a GL overlay context in addition to the
    normal context if overlays are supported by the underlying system.

    If you want to use overlays, you specify it in the \link QGLFormat
    format\endlink. (Note: Overlay must be requested in the format
    passed to the QGLWidget constructor.) Your GL widget should also
    implement some or all of these virtual methods:

    \list
    \i paintOverlayGL()
    \i resizeOverlayGL()
    \i initializeOverlayGL()
    \endlist

    These methods work in the same way as the normal paintGL() etc.
    functions, except that they will be called when the overlay
    context is made current. You can explicitly make the overlay
    context current by using makeOverlayCurrent(), and you can access
    the overlay context directly (e.g. to ask for its transparent
    color) by calling overlayContext().

    On X servers in which the default visual is in an overlay plane,
    non-GL Qt windows can also be used for overlays. See the
    examples/opengl/overlay_x11 example program for details.

    <sup>*</sup> OpenGL is a trademark of Silicon Graphics, Inc. in the
    United States and other countries.
*/

// ### BCI - fix in 4.0

// the display list cache can't be global because display lists are
// tied to the GL contexts for each individual widget

class QGLWidgetPrivate
{
public:
    QMap<QString, int> displayListCache;
};

static QPtrDict<QGLWidgetPrivate> * qgl_d_ptr = 0;
static QSingleCleanupHandler< QPtrDict<QGLWidgetPrivate> > qgl_cleanup_d_ptr;

static QGLWidgetPrivate * qgl_d( const QGLWidget * w )
{
    if ( !qgl_d_ptr ) {
	qgl_d_ptr = new QPtrDict<QGLWidgetPrivate>;
	qgl_cleanup_d_ptr.set( &qgl_d_ptr );
	qgl_d_ptr->setAutoDelete( TRUE );
    }
    QGLWidgetPrivate * ret = qgl_d_ptr->find( (void *) w );
    if ( !ret ) {
	ret = new QGLWidgetPrivate;
	qgl_d_ptr->replace( (void *) w, ret );
    }
    return ret;
}

void qgl_delete_d( const QGLWidget * w )
{
    if ( qgl_d_ptr )
	qgl_d_ptr->remove( (void *) w );
}

/*!
    Constructs an OpenGL widget with a \a parent widget and a \a name.

    The \link QGLFormat::defaultFormat() default format\endlink is
    used. The widget will be \link isValid() invalid\endlink if the
    system has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.

    The \a parent, \a name and widget flag, \a f, arguments are passed
    to the QWidget constructor.

    If the \a shareWidget parameter points to a valid QGLWidget, this
    widget will share OpenGL display lists with \a shareWidget. If
    this widget and \a shareWidget have different \link format()
    formats\endlink, display list sharing may fail. You can check
    whether display list sharing succeeded by calling isSharing().

    The initialization of OpenGL rendering state, etc. should be done
    by overriding the initializeGL() function, rather than in the
    constructor of your QGLWidget subclass.

    \sa QGLFormat::defaultFormat()
*/

QGLWidget::QGLWidget( QWidget *parent, const char *name,
		      const QGLWidget* shareWidget, WFlags f )
    : QWidget( parent, name, f | Qt::WWinOwnDC | Qt::WNoAutoErase )
{
    init( new QGLContext(QGLFormat::defaultFormat(), this), shareWidget );
}


/*!
    Constructs an OpenGL widget with parent \a parent, called \a name.

    The \a format argument specifies the desired \link QGLFormat
    rendering options \endlink. If the underlying OpenGL/Window system
    cannot satisfy all the features requested in \a format, the
    nearest subset of features will be used. After creation, the
    format() method will return the actual format obtained.

    The widget will be \link isValid() invalid\endlink if the system
    has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.

    The \a parent, \a name and widget flag, \a f, arguments are passed
    to the QWidget constructor.

    If the \a shareWidget parameter points to a valid QGLWidget, this
    widget will share OpenGL display lists with \a shareWidget. If
    this widget and \a shareWidget have different \link format()
    formats\endlink, display list sharing may fail. You can check
    whether display list sharing succeeded by calling isSharing().

    The initialization of OpenGL rendering state, etc. should be done
    by overriding the initializeGL() function, rather than in the
    constructor of your QGLWidget subclass.

    \sa QGLFormat::defaultFormat(), isValid()
*/

QGLWidget::QGLWidget( const QGLFormat &format, QWidget *parent,
		      const char *name, const QGLWidget* shareWidget,
		      WFlags f )
    : QWidget( parent, name, f | Qt::WWinOwnDC | Qt::WNoAutoErase )
{
    init( new QGLContext(format, this), shareWidget );
}

/*!
    Constructs an OpenGL widget with parent \a parent, called \a name.

    The \a context argument is a pointer to the QGLContext that
    you wish to be bound to this widget. This allows you to pass in
    your own QGLContext sub-classes.

    The widget will be \link isValid() invalid\endlink if the system
    has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.

    The \a parent, \a name and widget flag, \a f, arguments are passed
    to the QWidget constructor.

    If the \a shareWidget parameter points to a valid QGLWidget, this
    widget will share OpenGL display lists with \a shareWidget. If
    this widget and \a shareWidget have different \link format()
    formats\endlink, display list sharing may fail. You can check
    whether display list sharing succeeded by calling isSharing().

    The initialization of OpenGL rendering state, etc. should be done
    by overriding the initializeGL() function, rather than in the
    constructor of your QGLWidget subclass.

    \sa QGLFormat::defaultFormat(), isValid()
*/
QGLWidget::QGLWidget( QGLContext *context, QWidget *parent,
		      const char *name, const QGLWidget *shareWidget, WFlags f )
    : QWidget( parent, name, f | Qt::WWinOwnDC | Qt::WNoAutoErase )
{
    init( context, shareWidget );
}

/*!
    Destroys the widget.
*/

QGLWidget::~QGLWidget()
{
#if defined(GLX_MESA_release_buffers) && defined(QGL_USE_MESA_EXT)
    bool doRelease = ( glcx && glcx->windowCreated() );
#endif
    qgl_delete_d( this );
    delete glcx;
#if defined(Q_WGL)
    delete olcx;
#endif
#if defined(GLX_MESA_release_buffers) && defined(QGL_USE_MESA_EXT)
    if ( doRelease )
	glXReleaseBuffersMESA( x11Display(), winId() );
#endif
#if defined(Q_WS_MAC)
    if(gl_pix) {
	delete gl_pix;
	gl_pix = NULL;
    }
#endif
    cleanupColormaps();
}





/*!
    \fn QGLFormat QGLWidget::format() const

    Returns the format of the contained GL rendering context.
*/

/*!
    \fn bool QGLWidget::doubleBuffer() const

    Returns TRUE if the contained GL rendering context has double
    buffering; otherwise returns FALSE.

    \sa QGLFormat::doubleBuffer()
*/

/*!
    \fn void QGLWidget::setAutoBufferSwap( bool on )

    If \a on is TRUE automatic GL buffer swapping is switched on;
    otherwise it is switched off.

    If \a on is TRUE and the widget is using a double-buffered format,
    the background and foreground GL buffers will automatically be
    swapped after each paintGL() call.

    The buffer auto-swapping is on by default.

    \sa autoBufferSwap(), doubleBuffer(), swapBuffers()
*/

/*!
    \fn bool QGLWidget::autoBufferSwap() const

    Returns TRUE if the widget is doing automatic GL buffer swapping;
    otherwise returns FALSE.

    \sa setAutoBufferSwap()
*/

/*!
    \fn bool QGLWidget::isValid() const

    Returns TRUE if the widget has a valid GL rendering context;
    otherwise returns FALSE. A widget will be invalid if the system
    has no \link QGLFormat::hasOpenGL() OpenGL support\endlink.
*/

bool QGLWidget::isValid() const
{
    return glcx->isValid();
}

/*!
    \fn bool QGLWidget::isSharing() const

    Returns TRUE if display list sharing with another QGLWidget was
    requested in the constructor, and the GL system was able to
    provide it; otherwise returns FALSE. The GL system may fail to
    provide display list sharing if the two QGLWidgets use different
    formats.

    \sa format()
*/

bool QGLWidget::isSharing() const
{
    return glcx->isSharing();
}

/*!
    \fn void QGLWidget::makeCurrent()

    Makes this widget the current widget for OpenGL operations, i.e.
    makes the widget's rendering context the current OpenGL rendering
    context.
*/

void QGLWidget::makeCurrent()
{
#if defined( Q_WS_MAC )
    macInternalDoubleBuffer(); //make sure the correct context is used
#endif
    glcx->makeCurrent();
}

/*!
    \fn void QGLWidget::doneCurrent()

    Makes no GL context the current context. Normally, you do not need
    to call this function; QGLContext calls it as necessary. However,
    it may be useful in multithreaded environments.
*/

void QGLWidget::doneCurrent()
{
    glcx->doneCurrent();
}

/*!
    \fn void QGLWidget::swapBuffers()

    Swaps the screen contents with an off-screen buffer. This only
    works if the widget's format specifies double buffer mode.

    Normally, there is no need to explicitly call this function
    because it is done automatically after each widget repaint, i.e.
    each time after paintGL() has been executed.

    \sa doubleBuffer(), setAutoBufferSwap(), QGLFormat::setDoubleBuffer()
*/

void QGLWidget::swapBuffers()
{
    glcx->swapBuffers();
#if defined(Q_WS_MAC)
    if(macInternalDoubleBuffer() && gl_pix)
	bitBlt(this, 0, 0, gl_pix);
#endif
}


/*!
    \fn const QGLContext* QGLWidget::overlayContext() const

    Returns the overlay context of this widget, or 0 if this widget
    has no overlay.

    \sa context()
*/



/*!
    \fn void QGLWidget::makeOverlayCurrent()

    Makes the overlay context of this widget current. Use this if you
    need to issue OpenGL commands to the overlay context outside of
    initializeOverlayGL(), resizeOverlayGL(), and paintOverlayGL().

    Does nothing if this widget has no overlay.

    \sa makeCurrent()
*/


/*
  \obsolete

  Sets a new format for this widget.

  If the underlying OpenGL/Window system cannot satisfy all the
  features requested in \a format, the nearest subset of features will
  be used. After creation, the format() method will return the actual
  rendering context format obtained.

  The widget will be assigned a new QGLContext, and the initializeGL()
  function will be executed for this new context before the first
  resizeGL() or paintGL().

  This method will try to keep any existing display list sharing with
  other QGLWidgets, but it may fail. Use isSharing() to test.

  \sa format(), isSharing(), isValid()
*/

void QGLWidget::setFormat( const QGLFormat &format )
{
    setContext( new QGLContext(format,this) );
}




/*!
    \fn const QGLContext *QGLWidget::context() const

    Returns the context of this widget.

    It is possible that the context is not valid (see isValid()), for
    example, if the underlying hardware does not support the format
    attributes that were requested.
*/

/*
  \obsolete

  \fn void QGLWidget::setContext( QGLContext *context,
				  const QGLContext* shareContext,
				  bool deleteOldContext )

  Sets a new context for this widget. The QGLContext \a context must
  be created using \e new. QGLWidget will delete \a context when
  another context is set or when the widget is destroyed.

  If \a context is invalid, QGLContext::create() is performed on
  it. The initializeGL() function will then be executed for the new
  context before the first resizeGL() or paintGL().

  If \a context is invalid, this method will try to keep any existing
  display list sharing with other QGLWidgets this widget currently
  has, or (if \a shareContext points to a valid context) start display
  list sharing with that context, but it may fail. Use isSharing() to
  test.

  If \a deleteOldContext is TRUE (the default), the existing context
  will be deleted. You may use FALSE here if you have kept a pointer
  to the old context (as returned by context()), and want to restore
  that context later.

  \sa context(), isSharing()
*/



/*!
    \fn void QGLWidget::updateGL()

    Updates the widget by calling glDraw().
*/

void QGLWidget::updateGL()
{
    glDraw();
}


/*!
    \fn void QGLWidget::updateOverlayGL()

    Updates the widget's overlay (if any). Will cause the virtual
    function paintOverlayGL() to be executed.

    The widget's rendering context will become the current context and
    initializeGL() will be called if it hasn't already been called.
*/


/*!
    This virtual function is called once before the first call to
    paintGL() or resizeGL(), and then once whenever the widget has
    been assigned a new QGLContext. Reimplement it in a subclass.

    This function should set up any required OpenGL context rendering
    flags, defining display lists, etc.

    There is no need to call makeCurrent() because this has already
    been done when this function is called.
*/

void QGLWidget::initializeGL()
{
}


/*!
    This virtual function is called whenever the widget needs to be
    painted. Reimplement it in a subclass.

    There is no need to call makeCurrent() because this has already
    been done when this function is called.
*/

void QGLWidget::paintGL()
{
}


/*!
    \fn void QGLWidget::resizeGL( int width , int height )

    This virtual function is called whenever the widget has been
    resized. The new size is passed in \a width and \a height.
    Reimplement it in a subclass.

    There is no need to call makeCurrent() because this has already
    been done when this function is called.
*/

void QGLWidget::resizeGL( int, int )
{
}



/*!
    This virtual function is used in the same manner as initializeGL()
    except that it operates on the widget's overlay context instead of
    the widget's main context. This means that initializeOverlayGL()
    is called once before the first call to paintOverlayGL() or
    resizeOverlayGL(). Reimplement it in a subclass.

    This function should set up any required OpenGL context rendering
    flags, defining display lists, etc. for the overlay context.

    There is no need to call makeOverlayCurrent() because this has
    already been done when this function is called.
*/

void QGLWidget::initializeOverlayGL()
{
}


/*!
    This virtual function is used in the same manner as paintGL()
    except that it operates on the widget's overlay context instead of
    the widget's main context. This means that paintOverlayGL() is
    called whenever the widget's overlay needs to be painted.
    Reimplement it in a subclass.

    There is no need to call makeOverlayCurrent() because this has
    already been done when this function is called.
*/

void QGLWidget::paintOverlayGL()
{
}


/*!
    \fn void QGLWidget::resizeOverlayGL( int width , int height )

    This virtual function is used in the same manner as paintGL()
    except that it operates on the widget's overlay context instead of
    the widget's main context. This means that resizeOverlayGL() is
    called whenever the widget has been resized. The new size is
    passed in \a width and \a height. Reimplement it in a subclass.

    There is no need to call makeOverlayCurrent() because this has
    already been done when this function is called.
*/

void QGLWidget::resizeOverlayGL( int, int )
{
}




/*!
    Handles paint events. Will cause the virtual paintGL() function to
    be called.

    The widget's rendering context will become the current context and
    initializeGL() will be called if it hasn't already been called.
*/

void QGLWidget::paintEvent( QPaintEvent * )
{
    glDraw();
    updateOverlayGL();
}


/*!
    \fn void QGLWidget::resizeEvent( QResizeEvent * )

    Handles resize events. Calls the virtual function resizeGL().
*/


/*!
  \fn void QGLWidget::setMouseTracking( bool enable )

  \reimp
*/


/*!
    Renders the current scene on a pixmap and returns the pixmap.

    You can use this method on both visible and invisible QGLWidgets.

    This method will create a pixmap and a temporary QGLContext to
    render on the pixmap. It will then call initializeGL(),
    resizeGL(), and paintGL() on this context. Finally, the widget's
    original GL context is restored.

    The size of the pixmap will be \a w pixels wide and \a h pixels
    high unless one of these parameters is 0 (the default), in which
    case the pixmap will have the same size as the widget.

    If \a useContext is TRUE, this method will try to be more
    efficient by using the existing GL context to render the pixmap.
    The default is FALSE. Only use TRUE if you understand the risks.

    Overlays are not rendered onto the pixmap.

    If the GL rendering context and the desktop have different bit
    depths, the result will most likely look surprising.

    Note that the creation of display lists, modifications of the view
    frustum etc. should be done from within initializeGL(). If this is
    not done, the temporary QGLContext will not be initialized
    properly, and the rendered pixmap may be incomplete/corrupted.
*/

QPixmap QGLWidget::renderPixmap( int w, int h, bool useContext )
{
    QSize sz = size();
    if ( (w > 0) && (h > 0) )
	sz = QSize( w, h );

#if defined(Q_WS_X11)
    QPixmap pm( sz.width(), sz.height(), x11Depth() );
    bool needConversion = x11Visual() != QPaintDevice::x11AppVisual();

    // make sure the pixmap uses the same visual as the widget itself
    if ( needConversion ) {
        QPaintDeviceX11Data* xd = pm.getX11Data( TRUE );
        xd->x_depth = x11Depth();
        xd->x_visual = (Visual *) x11Visual();
        pm.setX11Data( xd );
    }
#else
    QPixmap pm;
    pm.resize( sz );
#endif

    glcx->doneCurrent();

    bool success = TRUE;

    if ( useContext && isValid() && renderCxPm( &pm ) )
	return pm;

    QGLFormat fmt = glcx->requestedFormat();
    fmt.setDirectRendering( FALSE );		// Direct is unlikely to work
    fmt.setDoubleBuffer( FALSE );		// We don't need dbl buf

    QGLContext* ocx = glcx;
    bool wasCurrent = (QGLContext::currentContext() == ocx );
    ocx->doneCurrent();
    glcx = new QGLContext( fmt, &pm );
    glcx->create();

    if ( glcx->isValid() )
	updateGL();
    else
	success = FALSE;

    delete glcx;
    glcx = ocx;

    if ( wasCurrent )
	ocx->makeCurrent();

    if ( success ) {
#if defined(Q_WS_X11)
	if ( needConversion ) {
	    QImage image = pm.convertToImage();
	    QPixmap p;
	    p = image;
	    return p;
	}
#endif
	return pm;
    } else
	return QPixmap();
}



/*!
    Returns an image of the frame buffer. If \a withAlpha is TRUE the
    alpha channel is included.

    Depending on your hardware, you can explicitly select which color
    buffer to grab with a glReadBuffer() call before calling this
    function.
*/
QImage QGLWidget::grabFrameBuffer( bool withAlpha )
{
#if defined( Q_WS_MAC )
    if(dblbuf == macInternalDoubleBuffer(FALSE) && gl_pix) //why not optimize?
	return ((QPixmap*)gl_pix)->convertToImage();
#endif
    makeCurrent();
    QImage res;
    int w = width();
    int h = height();
    if ( format().rgba() ) {
	res = QImage( w, h, 32 );
	glReadPixels( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, res.bits() );
	if ( QImage::systemByteOrder() == QImage::BigEndian ) {
	    // OpenGL gives RGBA; Qt wants ARGB
	    uint *p = (uint*)res.bits();
	    uint *end = p + w*h;
	    if ( withAlpha && format().alpha() ) {
		while ( p < end ) {
		    uint a = *p << 24;
		    *p = (*p >> 8) | a;
		    p++;
		}
	    }
	    else {
		while ( p < end )
		    *p++ >>= 8;
	    }
	}
	else {
	    // OpenGL gives ABGR (i.e. RGBA backwards); Qt wants ARGB
	    res = res.swapRGB();
	}
	res.setAlphaBuffer( withAlpha && format().alpha() );
    }
    else {
#if defined (Q_WS_WIN)
	res = QImage( w, h, 8 );
	glReadPixels( 0, 0, w, h, GL_COLOR_INDEX, GL_UNSIGNED_BYTE,
		      res.bits() );
	int palSize = 0;
	const QRgb* pal = QColor::palette( &palSize );
	if ( pal && palSize ) {
	    res.setNumColors( palSize );
	    for ( int i = 0; i < palSize; i++ )
		res.setColor( i, pal[i] );
	}
#endif
    }

    return res.mirror();
}



/*!
    Initializes OpenGL for this widget's context. Calls the virtual
    function initializeGL().
*/

void QGLWidget::glInit()
{
    if ( !isValid() )
	return;
    makeCurrent();
    initializeGL();
    glcx->setInitialized( TRUE );
}


/*!
    Executes the virtual function paintGL().

    The widget's rendering context will become the current context and
    initializeGL() will be called if it hasn't already been called.
*/

void QGLWidget::glDraw()
{
    if ( !isValid() )
	return;
    makeCurrent();
    if ( glcx->deviceIsPixmap() )
	glDrawBuffer( GL_FRONT );
    if ( !glcx->initialized() ) {
	glInit();
	QPaintDeviceMetrics dm( glcx->device() );
	resizeGL( dm.width(), dm.height() ); // New context needs this "resize"
    }
    paintGL();
    if ( doubleBuffer() ) {
	if ( autoSwap )
	    swapBuffers();
    } else {
	glFlush();
#if defined( Q_WS_MAC )
	if(dblbuf && gl_pix)
	    bitBlt(this, 0, 0, gl_pix);
#endif
    }
}


/*!
    Convenience function for specifying a drawing color to OpenGL.
    Calls glColor3 (in RGBA mode) or glIndex (in color-index mode)
    with the color \a c. Applies to the current GL context.

    \sa qglClearColor(), QGLContext::currentContext(), QColor
*/

void QGLWidget::qglColor( const QColor& c ) const
{
    const QGLContext* ctx = QGLContext::currentContext();
    if ( ctx ) {
	if ( ctx->format().rgba() )
	    glColor3ub( c.red(), c.green(), c.blue() );

	else if ( ctx->device() == context()->device()
                  && !cmap.isEmpty() ) { // QGLColormap in use?
	    int i = cmap.find( c.rgb() );
	    if ( i < 0 )
		i = cmap.findNearest( c.rgb() );
	    glIndexi( i );
	} else
	    glIndexi( ctx->colorIndex( c ) );
    }
}

/*!
    Convenience function for specifying the clearing color to OpenGL.
    Calls glClearColor (in RGBA mode) or glClearIndex (in color-index
    mode) with the color \a c. Applies to the current GL context.

    \sa qglColor(), QGLContext::currentContext(), QColor
*/

void QGLWidget::qglClearColor( const QColor& c ) const
{
    const QGLContext* ctx = QGLContext::currentContext();
    if ( ctx ) {
	if ( ctx->format().rgba() )
	    glClearColor( (GLfloat)c.red() / 255.0, (GLfloat)c.green() / 255.0,
			  (GLfloat)c.blue() / 255.0, (GLfloat) 0.0 );
	else if ( ctx->device() == context()->device()
                  && !cmap.isEmpty() ) { // QGLColormap in use?
	    int i = cmap.find( c.rgb() );
	    if ( i < 0 )
		i = cmap.findNearest( c.rgb() );
	    glClearIndex( i );
	} else
	    glClearIndex( ctx->colorIndex( c ) );
    }
}


/*!
    Converts the image \a img into the unnamed format expected by
    OpenGL functions such as glTexImage2D(). The returned image is not
    usable as a QImage, but QImage::width(), QImage::height() and
    QImage::bits() may be used with OpenGL. The following few lines
    are from the texture example. Most of the code is irrelevant, so
    we just quote the relevant bits:

    \quotefile opengl/texture/gltexobj.cpp
    \skipto tex1
    \printline tex1
    \printline gllogo.bmp

    We create \e tex1 (and another variable) for OpenGL, and load a real
    image into \e buf.

    \skipto convertToGLFormat
    \printline convertToGLFormat

    A few lines later, we convert \e buf into OpenGL format and store it
    in \e tex1.

    \skipto glTexImage2D
    \printline glTexImage2D
    \printline tex1.bits

    Note the dimension restrictions for texture images as described in
    the glTexImage2D() documentation. The width must be 2^m + 2*border
    and the height 2^n + 2*border where m and n are integers and
    border is either 0 or 1.

    Another function in the same example uses \e tex1 with OpenGL.
*/


QImage QGLWidget::convertToGLFormat( const QImage& img )
{
    QImage res = img.convertDepth( 32 );
    res = res.mirror();

    if ( QImage::systemByteOrder() == QImage::BigEndian ) {
	// Qt has ARGB; OpenGL wants RGBA
	for ( int i=0; i < res.height(); i++ ) {
	    uint *p = (uint*)res.scanLine( i );
	    uint *end = p + res.width();
	    while ( p < end ) {
		*p = (*p << 8) | ((*p >> 24) & 0xFF);
		p++;
	    }
	}
    }
    else {
	// Qt has ARGB; OpenGL wants ABGR (i.e. RGBA backwards)
	res = res.swapRGB();
    }
    return res;
}


/*!
    \fn QGLColormap & QGLWidget::colormap() const

    Returns the colormap for this widget.

    Usually it is only top-level widgets that can have different
    colormaps installed. Asking for the colormap of a child widget
    will return the colormap for the child's top-level widget.

    If no colormap has been set for this widget, the QColormap
    returned will be empty.

    \sa setColormap()
*/

/*!
    \fn void QGLWidget::setColormap( const QGLColormap & cmap )

    Set the colormap for this widget to \a cmap. Usually it is only
    top-level widgets that can have colormaps installed.

    \sa colormap()
*/

int QGLWidget::displayListBase( const QFont & fnt, int listBase )
{
    int base;

    QGLWidgetPrivate * d = qgl_d( this );
    if ( !d || !glcx ) { // this can't happen unless we run out of mem
	return 0;
    }

    // always regenerate font disp. lists for pixmaps - hw accelerated
    // contexts can't handle this otherwise
    bool regenerate = glcx->deviceIsPixmap();

#if 0 // QT_NO_XFTFREETYPE
    // font color needs to be part of the font cache key when using
    // antialiased fonts since one set of glyphs needs to be generated
    // for each font color
    QString color_key;
    if (fnt.styleStrategy() != QFont::NoAntialias) {
	GLfloat color[4];
	glGetFloatv(GL_CURRENT_COLOR, color);
	color_key.sprintf("%f_%f_%f",color[0], color[1], color[2]);
    }
    QString key = fnt.key() + color_key + QString::number((int) regenerate);
#else
    QString key = fnt.key() + QString::number((int) regenerate);
#endif

    if ( !regenerate && (d->displayListCache.find( key ) != d->displayListCache.end()) ) {
	base = d->displayListCache[ key ];
    } else {
	int maxBase = listBase - 256;
	QMapConstIterator<QString,int> it;
	for ( it = d->displayListCache.begin(); it != d->displayListCache.end(); ++it ) {
	    if ( maxBase < it.data() ) {
		maxBase = it.data();
	    }
	}
	maxBase += 256;
	glcx->generateFontDisplayLists( fnt, maxBase );
	d->displayListCache[ key ] = maxBase;
	base = maxBase;
    }
    return base;
}

/*!
   Renders the string \a str into the GL context of this widget.

   \a x and \a y are specified in window coordinates, with the origin
   in the upper left-hand corner of the window. If \a fnt is not
   specified, the currently set application font will be used to
   render the string. To change the color of the rendered text you can
   use the glColor() call (or the qglColor() convenience function),
   just before the renderText() call. Note that if you have
   GL_LIGHTING enabled, the string will not appear in the color you
   want. You should therefore switch lighting off before using
   renderText().

   \a listBase specifies the index of the first display list that is
   generated by this function. The default value is 2000. 256 display
   lists will be generated, one for each of the first 256 characters
   in the font that is used to render the string. If several fonts are
   used in the same widget, the display lists for these fonts will
   follow the last generated list. You would normally not have to
   change this value unless you are using lists in the same range. The
   lists are deleted when the widget is destroyed.

   Note: This function only works reliably with ASCII strings.
*/

void QGLWidget::renderText( int x, int y, const QString & str, const QFont & fnt, int listBase )
{
    makeCurrent();
    glPushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_LIST_BIT | GL_CURRENT_BIT );
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0, width(), height(), 0, -1, 1 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2i( 0, 0 );
    glBitmap(0, 0, 0, 0, x, -y, NULL);
    glListBase( displayListBase( fnt, listBase ) );
    const char *cstr = str.latin1();
    glCallLists( strlen(cstr), GL_UNSIGNED_BYTE, cstr );

    // restore the matrix stacks and GL state
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glPopAttrib();
}

/*! \overload

    \a x, \a y and \a z are specified in scene or object coordinates
    relative to the currently set projection and model matrices. This
    can be useful if you want to annotate models with text labels and
    have the labels move with the model as it is rotated etc.
*/
void QGLWidget::renderText( double x, double y, double z, const QString & str, const QFont & fnt,
			    int listBase )
{
    makeCurrent();
    glRasterPos3d( x, y, z );
    glPushAttrib( GL_LIST_BIT );
    glListBase( displayListBase( fnt, listBase ) );
    const char *cstr = str.latin1();
    glCallLists( strlen(cstr), GL_UNSIGNED_BYTE, cstr );
    glPopAttrib();
}

/*****************************************************************************
  QGL classes overview documentation.
 *****************************************************************************/

/*!

\page opengl.html

\title Qt OpenGL 3D Graphics

\if defined(commercial)
This module is part of the \link commercialeditions.html Qt Enterprise
Edition\endlink.
\endif

\section1 Introduction

OpenGL is a standard API for rendering 3D graphics.

OpenGL only deals with 3D rendering and provides little or no support
for GUI programming issues. The user interface for an
OpenGL<sup>*</sup> application must be created with another toolkit,
such as Motif on the X platform, Microsoft Foundation Classes (MFC)
under Windows, or Qt on \e both platforms.

The Qt OpenGL module makes it easy to use OpenGL in Qt applications.
It provides an OpenGL widget class that can be used just like any
other Qt widget, except that it opens an OpenGL display buffer where
you can use the OpenGL API to render the contents.

The Qt OpenGL module is implemented as a platform-independent Qt/C++
wrapper around the platform-dependent GLX, WGL, or AGL C APIs. The
functionality provided is very similar to Mark Kilgard's GLUT library,
but with much more non-OpenGL-specific GUI functionality, i.e. the
whole Qt API.

\section1 Installation

When you install Qt for X11, the configure script will autodetect if
OpenGL headers and libraries are installed on your system, and if so,
it will include the Qt OpenGL module in the Qt library. (If your
OpenGL headers or libraries are placed in a non-standard directory,
you may need to change the QMAKE_INCDIR_OPENGL and/or
QMAKE_LIBDIR_OPENGL in the config file for your system). Some
configurations require threading to be enabled for OpenGL, so if
OpenGL is not detected, try \c{configure -thread}.

When you install Qt for Windows, the Qt OpenGL module is always
included.

The Qt OpenGL module is not licensed for use with the Qt Professional
Edition. Consider upgrading to the Qt Enterprise Edition if you
require OpenGL support.

Note about using Mesa on X11: Mesa versions earlier than 3.1 would use
the name "MesaGL" and "MesaGLU" for the libraries, instead of "GL" and
"GLU". If you want to use a pre-3.1 version of Mesa, you must change
the Makefiles to use these library names instead. The easiest way to
do this is to edit the QMAKE_LIBS_OPENGL line in the config file you
are using, changing "-lGL -lGLU" to "-lMesaGL -lMesaGLU"; then run
"configure" again.

\section1 The QGL Classes

The OpenGL support classes in Qt are:
\list
\i \link QGLWidget QGLWidget\endlink: An easy-to-use Qt
  widget for rendering OpenGL scenes.
\i \link QGLContext QGLContext\endlink: Encapsulates an OpenGL rendering context.
\i \link QGLFormat QGLFormat\endlink: Specifies the
display format of a rendering context.
\i \link QGLColormap QGLColormap\endlink: Handles indexed
colormaps in GL-index mode.
\endlist

Many applications only need the high-level QGLWidget class. The other
QGL classes provide advanced features. X11 users might like to read
the notes on \link opengl-x11-overlays.html overlays\endlink.

See also the \link opengl-examples.html OpenGL examples\endlink.

The QGL documentation assumes that you are familiar with OpenGL
programming. If you're new to the subject a good starting point is
\l{http://www.opengl.org/}.


<sup>*</sup> OpenGL is a trademark of Silicon Graphics, Inc. in the
United States and other countries.

*/

/*!
    \enum QGL::FormatOption

    This enum specifies the format options.

    \value DoubleBuffer
    \value DepthBuffer
    \value Rgba
    \value AlphaChannel
    \value AccumBuffer
    \value StencilBuffer
    \value StereoBuffers
    \value DirectRendering
    \value HasOverlay
    \value SingleBuffer
    \value NoDepthBuffer
    \value ColorIndex
    \value NoAlphaChannel
    \value NoAccumBuffer
    \value NoStencilBuffer
    \value NoStereoBuffers
    \value IndirectRendering
    \value NoOverlay
*/
