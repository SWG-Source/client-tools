/****************************************************************************
** $Id: glinfo_x11.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#define QT_CLEAN_NAMESPACE // avoid clashes with Xmd.h

#include <qstringlist.h>
#include <qgl.h>
#include "glinfo.h"

#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>

#ifndef GLX_NONE_EXT
#define GLX_NONE_EXT  0x8000
#endif

typedef struct visualAttribs {
    /* X visual attribs */
    int id;
    int c_class;
    int depth;
    int redMask, greenMask, blueMask;
    int colormapSize;
    int bitsPerRGB;
    /* GL visual attribs */
    int supportsGL;
    int transparent;
    int bufferSize;
    int level;
    int rgba;
    int doubleBuffer;
    int stereo;
    int auxBuffers;
    int redSize, greenSize, blueSize, alphaSize;
    int depthSize;
    int stencilSize;
    int accumRedSize, accumGreenSize, accumBlueSize, accumAlphaSize;
    int numSamples, numMultisample;
    int visualCaveat;
} VisualAttribs;

static void screenInfo( Display *dpy, int scrnum, QString &infotext )
{
    const char *serverVendor = glXQueryServerString( dpy, scrnum, GLX_VENDOR );
    const char *serverVersion = glXQueryServerString( dpy, scrnum, GLX_VERSION );
    const char *serverExtensions = glXQueryServerString( dpy, scrnum, GLX_EXTENSIONS );
    const char *clientVendor = glXGetClientString( dpy, GLX_VENDOR );
    const char *clientVersion = glXGetClientString( dpy, GLX_VERSION );
    const char *clientExtensions = glXGetClientString( dpy, GLX_EXTENSIONS );
    const char *glxExtensions = glXQueryExtensionsString( dpy, scrnum );
    const char *glVendor = (const char *) glGetString( GL_VENDOR );
    const char *glRenderer = (const char *) glGetString( GL_RENDERER );
    const char *glVersion = (const char *) glGetString( GL_VERSION );
    const char *glExtensions = (const char *) glGetString( GL_EXTENSIONS );
    const char *gluVersion = (const char *) gluGetString( (GLenum) GLU_VERSION );
    const char *gluExtensions = (const char *) gluGetString( (GLenum) GLU_EXTENSIONS );

    infotext.sprintf( "%sServer GLX vendor string: %s\n", infotext.ascii(), serverVendor );
    infotext.sprintf( "%sServer GLX version string: %s\n", infotext.ascii(), serverVersion );
    infotext.sprintf( "%sServer GLX extensions:\n", infotext.ascii() );

    infotext += QString( serverExtensions ).replace( ' ', '\n' ) + "\n\n";

    infotext.sprintf( "%sClient GLX vendor string: %s\n", infotext.ascii(), clientVendor );
    infotext.sprintf( "%sClient GLX version string: %s\n", infotext.ascii(), clientVersion );
    infotext.sprintf( "%sClient GLX extensions:\n", infotext.ascii() );

    infotext += QString( clientExtensions ).replace( ' ', '\n' ) + "\n\n";

    infotext.sprintf( "%sGLX extensions:\n", infotext.ascii() );

    infotext += QString( glxExtensions ).replace( ' ', '\n' ) + "\n\n";

    infotext.sprintf( "%sOpenGL vendor string: %s\n", infotext.ascii(), glVendor );
    infotext.sprintf( "%sOpenGL renderer string: %s\n", infotext.ascii(), glRenderer );
    infotext.sprintf( "%sOpenGL version string: %s\n", infotext.ascii(), glVersion );
    infotext.sprintf( "%sOpenGL extensions:\n", infotext.ascii() );

    infotext += QString( glExtensions ).replace( ' ', '\n' ) + "\n\n";

    infotext.sprintf( "%sGLU version: %s\n", infotext.ascii(), gluVersion );
    infotext.sprintf( "%sGLU extensions:\n", infotext.ascii() );

    infotext += QString( gluExtensions ).replace( ' ', '\n' ) + "\n\n";
}

static const char * visualClassName( int cls )
{
    switch ( cls ) {
	case StaticColor:
	    return "StaticColor";
	case PseudoColor:
	    return "PseudoColor";
	case StaticGray:
	    return "StaticGray";
	case GrayScale:
	    return "GrayScale";
	case TrueColor:
	    return "TrueColor";
	case DirectColor:
	    return "DirectColor";
	default:
	    return "";
   }
}

static const char * caveatString( int caveat )
{
    switch ( caveat ) {
#if defined(GLX_EXT_visual_rating)
	case GLX_NONE_EXT:
	    return "None";
	case GLX_SLOW_VISUAL_EXT:
	    return "Slow";
	case GLX_NON_CONFORMANT_VISUAL_EXT:
	    return "Nonconformant";
#endif
	default:
	    return "<none>";
   }
}

static void visualAttribs( Display * dpy, XVisualInfo * vi, VisualAttribs * attribs )
{
    memset( attribs, 0, sizeof( VisualAttribs ) );

    attribs->id = vi->visualid;
    attribs->c_class = vi->c_class;
    attribs->depth = vi->depth;
    attribs->redMask = vi->red_mask;
    attribs->greenMask = vi->green_mask;
    attribs->blueMask = vi->blue_mask;
    attribs->colormapSize = vi->colormap_size;
    attribs->bitsPerRGB = vi->bits_per_rgb;

    if ( glXGetConfig( dpy, vi, GLX_USE_GL, &attribs->supportsGL ) != 0 )
        return;
    attribs->accumAlphaSize = 0;
    glXGetConfig( dpy, vi, GLX_BUFFER_SIZE, &attribs->bufferSize );
    glXGetConfig( dpy, vi, GLX_LEVEL, &attribs->level );
    glXGetConfig( dpy, vi, GLX_RGBA, &attribs->rgba );
    glXGetConfig( dpy, vi, GLX_DOUBLEBUFFER, &attribs->doubleBuffer );
    glXGetConfig( dpy, vi, GLX_STEREO, &attribs->stereo );
    glXGetConfig( dpy, vi, GLX_AUX_BUFFERS, &attribs->auxBuffers );
    glXGetConfig( dpy, vi, GLX_RED_SIZE, &attribs->redSize );
    glXGetConfig( dpy, vi, GLX_GREEN_SIZE, &attribs->greenSize );
    glXGetConfig( dpy, vi, GLX_BLUE_SIZE, &attribs->blueSize );
    glXGetConfig( dpy, vi, GLX_ALPHA_SIZE, &attribs->alphaSize );
    glXGetConfig( dpy, vi, GLX_DEPTH_SIZE, &attribs->depthSize );
    glXGetConfig( dpy, vi, GLX_STENCIL_SIZE, &attribs->stencilSize );
    glXGetConfig( dpy, vi, GLX_ACCUM_RED_SIZE, &attribs->accumRedSize );
    glXGetConfig( dpy, vi, GLX_ACCUM_GREEN_SIZE, &attribs->accumGreenSize );
    glXGetConfig( dpy, vi, GLX_ACCUM_BLUE_SIZE, &attribs->accumBlueSize );
    glXGetConfig( dpy, vi, GLX_ACCUM_ALPHA_SIZE, &attribs->accumAlphaSize );

    attribs->transparent = 0; // transparent pixel missing
    attribs->numSamples = 0; // multisample tests missing
    attribs->numMultisample = 0;

#if defined(GLX_EXT_visual_rating)
    const char *ext = glXQueryExtensionsString( dpy, vi->screen );
    if ( ext && strstr( ext, "GLX_EXT_visual_rating" ) )
        glXGetConfig(dpy, vi, GLX_VISUAL_CAVEAT_EXT, &attribs->visualCaveat);
    else
        attribs->visualCaveat = GLX_NONE_EXT;
#else
    attribs->visualCaveat = 0;
#endif
}

static void visualInfo( Display *dpy, int scrnum, QString &configs )
{
    QString str;
    XVisualInfo *visuals, temp;
    int numVisuals;
    long mask;
    int i;

    /* get list of all visuals on this screen */
    temp.screen = scrnum;
    mask = VisualScreenMask;
    visuals = XGetVisualInfo( dpy, mask, &temp, &numVisuals );

    VisualAttribs attribs;
    configs.append( "Vis  Vis   Visual Trans  buff lev render DB ste  r   g   b   a  aux dep ste  accum buffers  MS   MS\n"
		    " ID Depth   Type  parent size el   type     reo sz  sz  sz  sz  buf th  ncl  r   g   b   a  num bufs Caveat\n"
		    "-----------------------------------------------------------------------------------------------------------\n" );
    for ( i = 0; i < numVisuals; i++ ) {
	visualAttribs( dpy, &visuals[i], &attribs );
	str.sprintf( "0x%2x %2d %-12s %d     %2d  %d  %-4s   %d   %d   %d   %d   %d   %d"
		     "   %d   %2d  %d  %2d  %2d  %2d  %2d   %d   %d   %s\n",
		     attribs.id,
		     attribs.depth,
		     visualClassName( attribs.c_class ),
		     attribs.transparent,
		     attribs.bufferSize,
		     attribs.level,
		     attribs.rgba ? "rgba" : "ci",
		     attribs.doubleBuffer,
		     attribs.stereo,
		     attribs.redSize,
		     attribs.greenSize,
		     attribs.blueSize,
		     attribs.alphaSize,
		     attribs.auxBuffers,
		     attribs.depthSize,
		     attribs.stencilSize,
		     attribs.accumRedSize,
		     attribs.accumGreenSize,
		     attribs.accumBlueSize,
		     attribs.accumAlphaSize,
		     attribs.numSamples,
		     attribs.numMultisample,
		     caveatString( attribs.visualCaveat ) );
	configs.append( str );
    }


    XFree( visuals );
}

GLInfo::GLInfo()
{
    QGLWidget gl((QWidget *) 0);
    gl.makeCurrent();
    Display *dpy;
    char *displayName = 0;
    int numScreens, scrNum;

    dpy = gl.x11Display();
    if ( !dpy ) {
        qWarning( "Error: unable to open display %s\n", displayName );
    }

    numScreens = ScreenCount( dpy );
    infotext.sprintf( "Display name: %s\nDirect rendering: %s\n", DisplayString( dpy ),
 		       gl.format().directRendering() ? "Yes" : "No" );

    for ( scrNum = 0; scrNum < numScreens; scrNum++ ) {
	screenInfo( dpy, scrNum, infotext );
	visualInfo( dpy, scrNum, infotext );
	if ( scrNum + 1 < numScreens )
	    infotext.append( "\n\n" );
    }
}

QString GLInfo::info()
{
    return infotext;
}
