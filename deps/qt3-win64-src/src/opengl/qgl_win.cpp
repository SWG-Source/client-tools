/****************************************************************************
** $Id: qgl_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qgl.h"
#if defined(Q_OS_WIN32)

#include "qpixmap.h"
#include "qapplication.h"
#include "qpaintdevicemetrics.h"
#include "qintdict.h"
#include <windows.h>
#define INT8  dummy_INT8
#define INT32 dummy_INT32
#include <GL/gl.h>
#undef  INT8
#undef  INT32

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

/*
  The choose_cmap function is internal and used by QGLWidget::setContext()
  and GLX (not Windows).  If the application can't find any sharable
  colormaps, it must at least create as few colormaps as possible.  The
  dictionary solution below ensures only one colormap is created per visual.
  Colormaps are also deleted when the application terminates.
*/


/**
 * Getting lastError message from wgl-routines
 */
void qglError( const char* method, const char* func )
{
    char * pMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        0, GetLastError(),
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        ( char* ) & pMsgBuf, 0, 0 );
    qWarning( "%s : %s failed: %s", method, func, pMsgBuf );
    LocalFree( pMsgBuf );
}


/*****************************************************************************
  QGLFormat Windows/GLX-specific code
*****************************************************************************/

bool QGLFormat::hasOpenGL()
{
    //Win32 ALWAYS have OpenGL(but sometimes it is software-only but possible
    //MMX/SSE-accelerated MS OpenGL 1.1)
    return true;
}


bool QGLFormat::hasOpenGLOverlays()
{
    // Todo: Determine if OpenGL implementation support overlays, which it should.
    qDebug( "QGLFormat::hasOpenGLOverlays\n" );
    return false;
}


// Hack to make OpenGL be loaded
// Without it wglCreateContext will fail, even if correct pixel format and hdc is valid.
// This is only for Qt-API, I believe.
static bool force_opengl_loading = FALSE;
/*****************************************************************************
  QGLContext Windows-specific code
*****************************************************************************/ 
/* state:done?(simple version) */
bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    qDebug( "qgl_win.cpp:QGLContext::chooseContext()" );
    // Some kind of workaround, to force loading of opengl module....
    // Seems to be neccessary....why?
    if ( !force_opengl_loading ) {
        GLint params;
        glGetIntegerv( GL_DEPTH_BITS, &params );
        force_opengl_loading = TRUE;
    }
    /* simple implementation:
       assumes that:
       1.colordepth is SAME as desktop
       2.RGBA pixels
       3.no alpna
       4.no acc
       5.32-bit depth buffer
       6.no aux/stencil buffers
       7.
       More correct implementation would use glGetXXX to
       for example:glGetbooleanv for GL_DOUBLEBUFFER...*/ 
    //TODO:what if setContext is NOT yet called and so dc is invalid?
    HDC aDc;
    if ( deviceIsPixmap() ) {
        win = 0;
        aDc = d->paintDevice->handle();
    } else {
        win = ( ( QWidget* ) d->paintDevice ) ->winId();
        aDc = GetDC( win ); //wglGetCurrentDC();
    }
    if ( !aDc ) { // needs a device context
#if defined(QT_CHECK_NULL)
        qWarning( "qgl_win.cpp: QGLContext::chooseContext(): Paint device cannot be null" );
#endif

        if ( win ) {
            ReleaseDC( win, aDc );
        }
        return FALSE;
    }

    PIXELFORMATDESCRIPTOR pfd;
    pixelFormatId = choosePixelFormat( &pfd, aDc );
    if ( ( !SetPixelFormat( aDc, pixelFormatId, &pfd ) ) ) {
        qglError( "qgl_win.cpp: QGLContext::choosePixelFormat()", "SetPixelFormat" );
        if ( win ) {
            ReleaseDC( win, aDc );
            return FALSE;
        }
    }
    rc = wglCreateContext( aDc );
    if ( !rc ) {
        qglError( "qgl_win.cpp: QGLContext:chooseContext()", "wglCreateContext" );
        if ( win )
            ReleaseDC( win, aDc );
        return FALSE;
    }
    return TRUE;
}

void QGLContext::reset()
{

    if ( !d->valid )
        return ;
    doneCurrent();
    if ( rc )
        wglDeleteContext( rc );
    rc = 0;
    if ( win && dc )
        ReleaseDC( win, dc );
    win = 0;
    dc = 0;
    pixelFormatId = 0;
    d->crWin = FALSE;
    d->sharing = FALSE;
    d->valid = FALSE;
    d->transpColor = QColor();
    d->initDone = FALSE;
}


void QGLContext::makeCurrent()
{
    if ( currentCtx ) { // check my current context
        if ( currentCtx == this )
            return ;
        currentCtx->doneCurrent();
    }
    if ( !d->valid ) {
#if defined(QT_CHECK_STATE)
        qWarning( "qgl_win.cpp: QGLContext::makeCurrent(): Cannot make invalid context current." );
#endif

        return ;
    }
    if ( win )
        dc = GetDC( win );
    else
        dc = d->paintDevice->handle();
    //    bool ok = TRUE;
    // Should I care if deviceIsPixmap or not?
    //     if ( deviceIsPixmap() ) {
    //         qWarning( "qgl_win.cpp::OGLContext::makeCurrent() - current device is pixmap" );
    //         //DKZM:pixmaps->recheck later
    //         ok = FALSE;
    //     } else {
    if ( !wglMakeCurrent( dc, rc ) )
        qglError( "qgl_win.cpp::OGLContext::makeCurrent()", "wglMakeCurrent" );
    currentCtx = this;
}

void QGLContext::doneCurrent()
{
    qDebug( "qgl_win.cpp:QGLContext::doneCurrent()" );
    if ( currentCtx != this )  // check my current context
        return ;
    currentCtx = 0;
    wglMakeCurrent( 0, 0 );
    if ( win && dc ) {
        ReleaseDC( win, dc );
        dc = 0;
    }
}


void QGLContext::swapBuffers() const
{
    if ( !d->valid )
        return ;
    if ( !deviceIsPixmap() )
        //swap buffers
        SwapBuffers( dc );

    /* glXSwapBuffers( d->paintDevice->x11Display(),
       ((QWidget *)d->paintDevice)->winId() );
    */
}

QColor QGLContext::overlayTransparentColor() const
{
    return QColor();  // Invalid color
}


uint QGLContext::colorIndex( const QColor& c ) const
{
    return 0;
}

void QGLContext::generateFontDisplayLists( QFont const &, int )
{
    // Generates a set of 256 display lists for the 256 first characters in the font font.
    // The first list will start at index listBase.
    //for renderText
    //X11's version simple call glXUseXFont( (Font) fnt.handle(), 0, 256, listBase );

}

/*****************************************************************************
  QGLWidget Windows-specific code
*****************************************************************************/ 
/* call seq:init,setContext,create(start),reset,
   chooseContext,glcx->makeCurrent(); */
void QGLWidget::init( QGLContext *context, const QGLWidget* shareWidget )
{
    qDebug( "qgl_win.cpp; QGLWidget::init( QGLContext *context, const QGLWidget* shareWidget )" );
    glcx = 0;
    autoSwap = TRUE;
    if ( shareWidget )
        setContext( context, shareWidget->context() );
    else
        setContext( context );
    setBackgroundMode( NoBackground ); //the widget is not cleared before paintEvent().

    /*
      if (isValid() && context->format().hasOverlay() ) {
      QCString olcxName (name() );
      olcxName+="-OGL_internal_overlay_widget";
      olcx = new QGLWidget( QGLFormat::defaultOverlayFormat(),
      this, olwName, shareWidget );
      if ( olcx->isValid() ) {
      olcx->setAutoBufferSwap( FALSE );
      }
      else {
      delete olcx;
      olcx = 0;
      glcx->glFormat.setOverlay( FALSE );
      }
      }*/


}

void QGLWidget::reparent( QWidget * parent, unsigned int f, QPoint const & p, bool showIt )
{
    qDebug( "qgl_win.cpp:QGLWidget::reparent()" );
    /*
      QWidget::reparent( parent, f, p, FALSE );
      if ( showIt )
      show();
    */
}

void QGLWidget::setMouseTracking( bool enable )
{
//    if ( olcx )
//        olcx->setMouseTracking( enable );
    QWidget::setMouseTracking( enable );
}

//state:done?
//must be kept in sync with chooseContext
int QGLContext::choosePixelFormat( void * pfd_void, HDC phdc )
{
    PIXELFORMATDESCRIPTOR * ppfd = ( PIXELFORMATDESCRIPTOR * ) pfd_void;
    memset( ppfd, 0, sizeof( PIXELFORMATDESCRIPTOR ) );
    ppfd->nSize = sizeof( PIXELFORMATDESCRIPTOR );
    ppfd->nVersion = 1;
    ppfd->dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    ppfd->iPixelType = PFD_TYPE_RGBA;          // RGBA type

    if ( deviceIsPixmap() )
        ppfd->dwFlags |= PFD_DRAW_TO_BITMAP;
    else
        ppfd->dwFlags |= PFD_DRAW_TO_WINDOW;
    if ( glFormat.doubleBuffer() && !deviceIsPixmap() )
        ppfd->dwFlags |= PFD_DOUBLEBUFFER;
    if ( glFormat.stereo() )
        ppfd->dwFlags |= PFD_STEREO;
    if ( glFormat.depth() )
        ppfd->cDepthBits = 16;
    else
        ppfd->dwFlags |= PFD_DEPTH_DONTCARE;
    if ( glFormat.rgba() ) {
        ppfd->iPixelType = PFD_TYPE_RGBA;
        if ( deviceIsPixmap() )
            ppfd->cColorBits = 16;
        else
            ppfd->cColorBits = 16;
    } else {
        ppfd->iPixelType = PFD_TYPE_COLORINDEX;
        ppfd->cColorBits = 0;
    }
    ppfd->cAlphaBits = 0;
    ppfd->cAccumBits = 0;
    ppfd->cStencilBits = 0;
    ppfd->iLayerType = PFD_MAIN_PLANE;
    int pixelFormat;
    if ( ( pixelFormat = ChoosePixelFormat( phdc, ppfd ) ) == 0 )
        qglError( "qgl_win.cpp:QGLContext::choosePixelFormat()", "ChoosePixelFormat()" );
    return pixelFormat;
}

void QGLWidget::setContext( QGLContext *context,
                            const QGLContext* shareContext,
                            bool deleteOldContext )
{
    qDebug( "qgl_win.cpp:QGLWidget::setContext(QGLContext *context,const QGLContext* shareContext,bool deleteOldContext)" );
    if ( context == 0 ) {
#if defined(QT_CHECK_NULL)
        qWarning( "qgl_win.cpp:QGLWidget::setContext: Cannot set null context" );
#endif

        return ;
    }
    if ( !context->deviceIsPixmap() && context->device() != this ) {
#if defined(QT_CHECK_STATE)
        qWarning( "qgl_win.cpp:QGLWidget::setContext: Context must refer to this widget" );
#endif

        return ;
    }

    if ( glcx )
        glcx->doneCurrent();

    QGLContext* oldcx = glcx;
    glcx = context;

    bool createFailed = FALSE;
    if ( !glcx->isValid() ) {
        if ( !glcx->create( shareContext ? shareContext : oldcx ) )
            createFailed = TRUE;
    }
    if ( createFailed ) {
        if ( deleteOldContext )
            delete oldcx;
        return ;
    }

    if ( glcx->windowCreated() || glcx->deviceIsPixmap() ) {
        if ( deleteOldContext )
            delete oldcx;
        return ;
    }

    bool visible = isVisible();
    if ( visible )
        hide();
    if ( deleteOldContext )
        delete oldcx;

    oldcx = 0;

    //TODO:also show/hide it if visible gets changed
    /* if (visible)
       ShowWindow(hWnd,SW_SHOW);
       else
       ShowWindow(hWnd,SW_HIDE);

       UpdateWindow(hWnd);
    */
    if ( visible )
        show();
    /* XFlush( x11Display() ); */
    //GdiFlush();
    glcx->setWindowCreated( TRUE );


}

void QGLWidget::makeOverlayCurrent( void )
{
    qDebug( "qgl_win.cpp:QGLWidget::makeOverlayCurrent" );
    // if (olcx)
    //  olcx->makeCurrent();
}

void QGLWidget::updateOverlayGL( void )
{
    qDebug( "QGLWidget::updateOverlayGL" );
    // if (olcx)
    //  olcx->updateGL();
}

void QGLWidget::resizeEvent( QResizeEvent * )
{
    if ( !isValid() )
        return ;
    makeCurrent();
    if ( !glcx->initialized() )
        glInit();
    resizeGL( width(), height() );
if ( olcx ) {} //TODO:reimplement!!!
}

bool QGLWidget::renderCxPm( QPixmap* pm )
{
    return FALSE;
}

/*\internal
  Check whether the given visual supports dynamic colormaps or not.
*/
static bool qCanAllocColors( QWidget * w )
{}

void QGLWidget::setColormap( const QGLColormap & c )
{}

/*! \internal
  Free up any allocated colormaps. This fn is only called for
  top-level widgets.
*/
void QGLWidget::cleanupColormaps()
{}

void QGLWidget::macInternalFixBufferRect()
{}

const QGLContext* QGLWidget::overlayContext() const
{
    //  if ( olcx )
    //  return olcx->context();
    //else
    return 0;
}

const QGLColormap & QGLWidget::colormap() const
{
    return cmap;
}

#endif
