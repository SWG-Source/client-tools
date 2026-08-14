/****************************************************************************
** $Id: glinfo_win.cpp 750 2004-12-10 16:27:09Z syntheticpp $
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

#include "glinfo.h"
#include <qgl.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);

GLInfo::GLInfo()
{
    QGLWidget gl((QGLWidget *) 0);
    gl.makeCurrent();

    // get hold of WGL extensions
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = 
	(PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress( "wglGetExtensionsStringARB" );
    QString wglExts;
    if ( wglGetExtensionsStringARB ) {
	wglExts = (char *) wglGetExtensionsStringARB( wglGetCurrentDC() );
	wglExts.replace( ' ', '\n' );
    }
    infotext.sprintf( "OpenGL vendor string: %s\n", (const char *) glGetString( GL_VENDOR ) );
    infotext.sprintf( "%sOpenGL renderer string: %s\n", infotext.ascii(), glGetString( GL_RENDERER ) );
    infotext.sprintf( "%sOpenGL version string: %s\n", infotext.ascii(), glGetString( GL_VERSION ) );
    infotext.sprintf( "%s\nWGL extensions:\n%s\n", infotext.ascii(),
		       !wglExts.isEmpty() ? wglExts.ascii() : "None\n" );
    infotext.sprintf( "%sOpenGL extensions:\n", infotext.ascii() );
    infotext += QString( (char *) glGetString( GL_EXTENSIONS ) ).replace( ' ', '\n' );
    
    infotext += "\n PF  Color  Draw  Trans  buff lev render DB ste  r   g   b   a  aux dep ste  accum buffers  MS   MS\n"
	     " id  depth   to   parent size el   type     reo sz  sz  sz  sz  buf th  ncl  r   g   b   a  num bufs\n"
	     "----------------------------------------------------------------------------------------------------\n";
    HDC dc;
    dc = GetDC( gl.winId() );
    
    QString str;
    int i, maxpf;
    PIXELFORMATDESCRIPTOR pfd;
	
    /* calling DescribePixelFormat() with NULL args return maximum
       number of pixel formats */
    maxpf = DescribePixelFormat( dc, 0, 0, NULL );
	
    for(i = 1; i <= maxpf; i++) { 
        str = "";
	DescribePixelFormat( dc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd );
	if ( !(pfd.dwFlags & PFD_SUPPORT_OPENGL) )
	    continue;
	str.sprintf( "0x%02x   %2d  ", i, pfd.cColorBits );
	if( pfd.dwFlags & PFD_DRAW_TO_WINDOW && pfd.dwFlags & PFD_DRAW_TO_BITMAP )
	    str.sprintf( "%swin/bmp  ", str.ascii() );
	else if( pfd.dwFlags & PFD_DRAW_TO_WINDOW )
	    str.sprintf( "%swindow   ", str.ascii() );
	else if( pfd.dwFlags & PFD_DRAW_TO_BITMAP ) 
	    str.sprintf( "%sbitmap   ", str.ascii() );
	else 
	    str.sprintf( "%s.        ", str.ascii() );
		
	LAYERPLANEDESCRIPTOR lpfd;
	if ( pfd.bReserved ) {
	    wglDescribeLayerPlane( dc, i, pfd.bReserved,
				   sizeof( LAYERPLANEDESCRIPTOR ), &lpfd );
	    if ( lpfd.dwFlags & LPD_TRANSPARENT && lpfd.iPixelType == PFD_TYPE_RGBA )
		str.sprintf( "%s%06x %2d   ", str.ascii(), lpfd.crTransparent );
	    else
		str.sprintf( "%s%d     %2d   ", str.ascii(), lpfd.crTransparent, pfd.cColorBits );
	} else {
	    str.sprintf( "%s0     %2d   ", str.ascii(), pfd.cColorBits );
	}
	str.sprintf( "%s%d   ", str.ascii(), pfd.bReserved );
	if ( pfd.bReserved ) {
	    bool mpTypeRgba = pfd.iPixelType == PFD_TYPE_RGBA;
	    bool lpTypeRgba = lpfd.iPixelType == PFD_TYPE_RGBA;
	    QString type;
	    if ( mpTypeRgba && lpTypeRgba )
		type = "rg/rg ";
	    else if ( mpTypeRgba && !lpTypeRgba )
		type = "rg/ci ";
	    else if ( !mpTypeRgba && lpTypeRgba )
		type = "ci/rg ";
	    else
		type = "ci/ci ";
	    str.sprintf("%s%s ", str.ascii(), type.ascii() );	    
	} else {
	    str.sprintf("%s%s", str.ascii(), pfd.iPixelType == PFD_TYPE_RGBA ? "rgba   " : "ci     " );
	}
	str.sprintf( "%s%c   %c   ", str.ascii(),
		     pfd.dwFlags & PFD_DOUBLEBUFFER ? 'y' : 'n',
		     pfd.dwFlags & PFD_STEREO ? 'y' : 'n' );
	str.sprintf( "%s%d   ", str.ascii(), pfd.cRedBits );
	str.sprintf( "%s%d   ", str.ascii(), pfd.cGreenBits );
	str.sprintf( "%s%d   ", str.ascii(), pfd.cBlueBits );
	str.sprintf( "%s%d   ", str.ascii(), pfd.cAlphaBits );
	str.sprintf( "%s%d  ", str.ascii(), pfd.cAuxBuffers );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cDepthBits );
	str.sprintf( "%s%2d   ", str.ascii(), pfd.cStencilBits );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cAccumRedBits );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cAccumGreenBits );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cAccumBlueBits );
	str.sprintf( "%s%2d  ", str.ascii(), pfd.cAccumAlphaBits );

	/* no multisample in Win32 */
	str.append( "0   0\n" );
	infotext.append( str );
    }
    ReleaseDC( gl.winId(), dc );
}

QString GLInfo::info()
{
    return infotext;
}
