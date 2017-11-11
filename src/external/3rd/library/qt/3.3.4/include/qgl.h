/****************************************************************************
** $Id: qt/qgl.h   3.3.4   edited Jul 9 2003 $
**
** Definition of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
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

#ifndef QGL_H
#define QGL_H

#ifndef QT_H
#include "qwidget.h"
#include "qglcolormap.h"
#endif // QT_H

#if !defined( QT_MODULE_OPENGL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_OPENGL
#else
#define QM_EXPORT_OPENGL Q_EXPORT
#endif

#ifndef QT_NO_COMPAT
#define QGL_VERSION	450
#define QGL_VERSION_STR	"4.5"
QM_EXPORT_OPENGL inline const char *qGLVersion() {
    qObsolete( 0, "qGLVersion", "qVersion" );
    return QGL_VERSION_STR;
}
#endif

#if defined(Q_WS_WIN)
# include "qt_windows.h"
#endif

#if defined(Q_WS_MAC)
#if !defined( QMAC_OPENGL_DOUBLEBUFFER )
/* This macro is different now. If the macro is not defined QGLWidget will
 * try to determine when you need double buffering.  If set to 0 it will
 * never double buffer and *can* be acclerated. If set to 1 (the default)
 * it will always double buffer. Unlike before the value of this macro does
 * not upset binary compatability either. */
#if QT_MACOSX_VERSION >= 0x1020
# define QMAC_OPENGL_DOUBLEBUFFER 0
#endif
#endif
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
#endif

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
class QGLCmap;
#endif

class QPixmap;
#if defined(Q_WS_X11)
class QGLOverlayWidget;
#endif

// Namespace class:
class QM_EXPORT_OPENGL QGL
{
public:
    enum FormatOption {
	DoubleBuffer		= 0x0001,
	DepthBuffer		= 0x0002,
	Rgba			= 0x0004,
	AlphaChannel		= 0x0008,
	AccumBuffer		= 0x0010,
	StencilBuffer		= 0x0020,
	StereoBuffers		= 0x0040,
	DirectRendering		= 0x0080,
	HasOverlay		= 0x0100,
	SingleBuffer            = DoubleBuffer  << 16,
	NoDepthBuffer           = DepthBuffer   << 16,
	ColorIndex              = Rgba          << 16,
	NoAlphaChannel          = AlphaChannel  << 16,
	NoAccumBuffer           = AccumBuffer   << 16,
	NoStencilBuffer         = StencilBuffer << 16,
	NoStereoBuffers         = StereoBuffers << 16,
	IndirectRendering       = DirectRendering << 16,
	NoOverlay		= HasOverlay << 16
    };
};



class QM_EXPORT_OPENGL QGLFormat : public QGL
{
public:
    QGLFormat();
    QGLFormat( int options, int plane = 0 );

    bool doubleBuffer() const;
    void setDoubleBuffer( bool enable );
    bool depth() const;
    void setDepth( bool enable );
    bool rgba() const;
    void setRgba( bool enable );
    bool alpha() const;
    void setAlpha( bool enable );
    bool accum() const;
    void setAccum( bool enable );
    bool stencil() const;
    void setStencil( bool enable );
    bool stereo() const;
    void setStereo( bool enable );
    bool directRendering() const;
    void setDirectRendering( bool enable );
    bool hasOverlay() const;
    void setOverlay( bool enable );

    int plane() const;
    void setPlane( int plane );

    void setOption( FormatOption opt );
    bool testOption( FormatOption opt ) const;

    static QGLFormat defaultFormat();
    static void setDefaultFormat( const QGLFormat& f );

    static QGLFormat defaultOverlayFormat();
    static void setDefaultOverlayFormat( const QGLFormat& f );

    static bool hasOpenGL();
    static bool hasOpenGLOverlays();

    friend QM_EXPORT_OPENGL bool operator==( const QGLFormat&,
					     const QGLFormat& );
    friend QM_EXPORT_OPENGL bool operator!=( const QGLFormat&,
					     const QGLFormat& );
private:
    uint opts;
    int pln;
};


QM_EXPORT_OPENGL bool operator==( const QGLFormat&, const QGLFormat& );
QM_EXPORT_OPENGL bool operator!=( const QGLFormat&, const QGLFormat& );

class QM_EXPORT_OPENGL QGLContext : public QGL
{
public:
    QGLContext( const QGLFormat& format, QPaintDevice* device );
    QGLContext( const QGLFormat& format );
    virtual ~QGLContext();

    virtual bool create( const QGLContext* shareContext = 0 );
    bool isValid() const;
    bool isSharing() const;
    virtual void reset();

    QGLFormat format() const;
    QGLFormat requestedFormat() const;
    virtual void setFormat( const QGLFormat& format );

    virtual void makeCurrent();
    virtual void swapBuffers() const;

    QPaintDevice* device() const;

    QColor overlayTransparentColor() const;

    static const QGLContext* currentContext();

protected:
    virtual bool chooseContext( const QGLContext* shareContext = 0 );
    virtual void doneCurrent(); // ### 4.0: make this public - needed for multithreading stuff

#if defined(Q_WS_WIN)
    virtual int choosePixelFormat( void* pfd, HDC pdc );
#endif
#if defined(Q_WS_X11)
    virtual void* tryVisual( const QGLFormat& f, int bufDepth = 1 );
    virtual void* chooseVisual();
#endif
#if defined(Q_WS_MAC)
    virtual void* chooseMacVisual(GDHandle);
#endif

    bool deviceIsPixmap() const;
    bool windowCreated() const;
    void setWindowCreated( bool on );
    bool initialized() const;
    void setInitialized( bool on );
    void generateFontDisplayLists( const QFont & fnt, int listBase );

    uint colorIndex( const QColor& c ) const;
    void setValid( bool valid );
    void setDevice( QPaintDevice *pDev );

protected:
#if  defined(Q_WS_WIN)
    HGLRC rc;
    HDC dc;
    WId	win;
    int pixelFormatId;
    QGLCmap* cmap;
#elif defined(Q_WS_X11) || defined(Q_WS_MAC)
    void* vi;
    void* cx;
#if defined(Q_WS_X11)
    Q_UINT32 gpm;
#endif
#endif
    QGLFormat glFormat;
    QGLFormat reqFormat;
    static QGLContext*	currentCtx;

private:
    void init( QPaintDevice *dev = 0 );
    class Private {
    public:
	bool valid;
	bool sharing;
	bool initDone;
	bool crWin;
	QPaintDevice* paintDevice;
	QColor transpColor;
#ifdef Q_WS_MAC
	QRect oldR;
#endif
    };
    Private* d;

    friend class QGLWidget;
#ifdef Q_WS_MAC
    void fixBufferRect();
#endif

private:	// Disabled copy constructor and operator=
    QGLContext() {}
    QGLContext( const QGLContext& ) {}
    QGLContext& operator=( const QGLContext& ) { return *this; }
};




class QM_EXPORT_OPENGL QGLWidget : public QWidget, public QGL
{
    Q_OBJECT
public:
    QGLWidget( QWidget* parent=0, const char* name=0,
	       const QGLWidget* shareWidget = 0, WFlags f=0 );
    QGLWidget( QGLContext *context, QWidget* parent, const char* name=0,
	       const QGLWidget* shareWidget = 0, WFlags f=0 );
    QGLWidget( const QGLFormat& format, QWidget* parent=0, const char* name=0,
	       const QGLWidget* shareWidget = 0, WFlags f=0 );
    ~QGLWidget();

    void qglColor( const QColor& c ) const;
    void qglClearColor( const QColor& c ) const;

    bool isValid() const;
    bool isSharing() const;
    virtual void makeCurrent();
    void doneCurrent();
    
    bool doubleBuffer() const;
    virtual void swapBuffers();

    QGLFormat format() const;
#ifndef Q_QDOC
    virtual void setFormat( const QGLFormat& format );
#endif

    const QGLContext* context() const;
#ifndef Q_QDOC
    virtual void setContext( QGLContext* context,
			     const QGLContext* shareContext = 0,
			     bool deleteOldContext = TRUE );
#endif

    virtual QPixmap renderPixmap( int w = 0, int h = 0,
				  bool useContext = FALSE );
    virtual QImage grabFrameBuffer( bool withAlpha = FALSE );

    virtual void makeOverlayCurrent();
    const QGLContext* overlayContext() const;

    static QImage convertToGLFormat( const QImage& img );

    void setMouseTracking( bool enable );
    virtual void  reparent( QWidget* parent, WFlags f, const QPoint& p,
			    bool showIt = FALSE );

    const QGLColormap & colormap() const;
    void  setColormap( const QGLColormap & map );

    void renderText( int x, int y, const QString & str,
		     const QFont & fnt = QFont(), int listBase = 2000 );
    void renderText( double x, double y, double z, const QString & str,
		     const QFont & fnt = QFont(), int listBase = 2000 );
public slots:
    virtual void updateGL();
    virtual void updateOverlayGL();

protected:
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();

    virtual void initializeOverlayGL();
    virtual void resizeOverlayGL( int w, int h );
    virtual void paintOverlayGL();

    void setAutoBufferSwap( bool on );
    bool autoBufferSwap() const;

    void paintEvent( QPaintEvent* );
    void resizeEvent( QResizeEvent* );

    virtual void glInit();
    virtual void glDraw();

private:
    int displayListBase( const QFont & fnt, int listBase );
    void cleanupColormaps();
    void init( QGLContext *context, const QGLWidget* shareWidget );
    bool renderCxPm( QPixmap* pm );
    QGLContext* glcx;
    bool autoSwap;

    QGLColormap cmap;

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    QGLContext* olcx;
#elif defined(Q_WS_X11)
    QGLOverlayWidget*	olw;
    friend class QGLOverlayWidget;
#endif

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGLWidget( const QGLWidget& );
    QGLWidget& operator=( const QGLWidget& );
#endif

#if defined(Q_WS_MAC)
private:
    const QGLContext *slcx;
    uint pending_fix : 1,
	 glcx_dblbuf : 2,
	 dblbuf : 1,
	 clp_serial : 15;
    QPixmap *gl_pix;
    QGLFormat req_format;

    void macInternalRecreateContext( QGLContext *ctx,
				     const QGLContext* = NULL,
				     bool update = TRUE );
    bool macInternalDoubleBuffer( bool fix = TRUE );
    virtual void setRegionDirty( bool );
    virtual void macWidgetChangedWindow();
#endif
private slots:
    void macInternalFixBufferRect();
};


//
// QGLFormat inline functions
//

inline bool QGLFormat::doubleBuffer() const
{
    return testOption( DoubleBuffer );
}

inline bool QGLFormat::depth() const
{
    return testOption( DepthBuffer );
}

inline bool QGLFormat::rgba() const
{
    return testOption( Rgba );
}

inline bool QGLFormat::alpha() const
{
    return testOption( AlphaChannel );
}

inline bool QGLFormat::accum() const
{
    return testOption( AccumBuffer );
}

inline bool QGLFormat::stencil() const
{
    return testOption( StencilBuffer );
}

inline bool QGLFormat::stereo() const
{
    return testOption( StereoBuffers );
}

inline bool QGLFormat::directRendering() const
{
    return testOption( DirectRendering );
}

inline bool QGLFormat::hasOverlay() const
{
    return testOption( HasOverlay );
}

//
// QGLContext inline functions
//

inline bool QGLContext::isValid() const
{
    return d->valid;
}

inline void QGLContext::setValid( bool valid )
{
    d->valid = valid;
}

inline bool QGLContext::isSharing() const
{
    return d->sharing;
}

inline QGLFormat QGLContext::format() const
{
    return glFormat;
}

inline QGLFormat QGLContext::requestedFormat() const
{
    return reqFormat;
}

inline QPaintDevice* QGLContext::device() const
{
    return d->paintDevice;
}

inline bool QGLContext::deviceIsPixmap() const
{
    return d->paintDevice->devType() == QInternal::Pixmap;
}


inline bool QGLContext::windowCreated() const
{
    return d->crWin;
}


inline void QGLContext::setWindowCreated( bool on )
{
    d->crWin = on;
}

inline bool QGLContext::initialized() const
{
    return d->initDone;
}

inline void QGLContext::setInitialized( bool on )
{
    d->initDone = on;
}

inline const QGLContext* QGLContext::currentContext()
{
    return currentCtx;
}

//
// QGLWidget inline functions
//

inline QGLFormat QGLWidget::format() const
{
    return glcx->format();
}

inline const QGLContext *QGLWidget::context() const
{
    return glcx;
}

inline bool QGLWidget::doubleBuffer() const
{
    return glcx->format().doubleBuffer();
}

inline void QGLWidget::setAutoBufferSwap( bool on )
{
    autoSwap = on;
}

inline bool QGLWidget::autoBufferSwap() const
{
    return autoSwap;
}

#endif
