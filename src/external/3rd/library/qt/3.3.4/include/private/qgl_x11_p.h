/****************************************************************************
** $Id: qt/qgl_x11_p.h   3.3.4   edited Feb 24 2004 $
**
** Definitions needed for resolving GL/GLX symbols using dlopen()
** under X11.
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the OpenGL module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QGL_P_H
#define QGL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifdef QT_DLOPEN_OPENGL
// resolve the GL symbols we use ourselves
bool qt_resolve_gl_symbols(bool = TRUE);
extern "C" {
// GL symbols
typedef void (*_glCallLists)( GLsizei n, GLenum type, const GLvoid *lists );
typedef void (*_glClearColor)( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
typedef void (*_glClearIndex)( GLfloat c );
typedef void (*_glColor3ub)( GLubyte red, GLubyte green, GLubyte blue );
typedef void (*_glDeleteLists)( GLuint list, GLsizei range );
typedef void (*_glDrawBuffer)( GLenum mode );
typedef void (*_glFlush)( void );
typedef void (*_glIndexi)( GLint c );
typedef void (*_glListBase)( GLuint base );
typedef void (*_glLoadIdentity)( void );
typedef void (*_glMatrixMode)( GLenum mode );
typedef void (*_glOrtho)( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val );
typedef void (*_glPopAttrib)( void );
typedef void (*_glPopMatrix)( void );
typedef void (*_glPushAttrib)( GLbitfield mask );
typedef void (*_glPushMatrix)( void );
typedef void (*_glRasterPos2i)( GLint x, GLint y );
typedef void (*_glRasterPos3d)( GLdouble x, GLdouble y, GLdouble z );
typedef void (*_glReadPixels)( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels );
typedef void (*_glViewport)( GLint x, GLint y, GLsizei width, GLsizei height );
typedef void (*_glPixelStorei)( GLenum pname, GLint param );
typedef void (*_glBitmap)( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove,
			   const GLubyte *bitmap );
typedef void (*_glDrawPixels)( GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels );
typedef void (*_glNewList)( GLuint list, GLenum mode );
typedef void (*_glGetFloatv)( GLenum pname, GLfloat *params );
typedef void (*_glGetIntegerv)( GLenum pname, GLint *params );
typedef void (*_glEndList)( void );


// GLX symbols - should be in the GL lib as well
typedef XVisualInfo* (*_glXChooseVisual)(Display *dpy, int screen, int *attribList);
typedef GLXContext (*_glXCreateContext)(Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct);
typedef GLXPixmap (*_glXCreateGLXPixmap)(Display *dpy, XVisualInfo *vis, Pixmap pixmap);
typedef void (*_glXDestroyContext)(Display *dpy, GLXContext ctx);
typedef void (*_glXDestroyGLXPixmap)(Display *dpy, GLXPixmap pix);
typedef const char* (*_glXGetClientString)(Display *dpy, int name );
typedef int (*_glXGetConfig)(Display *dpy, XVisualInfo *vis, int attrib, int *value);
typedef Bool (*_glXIsDirect)(Display *dpy, GLXContext ctx);
typedef Bool (*_glXMakeCurrent)(Display *dpy, GLXDrawable drawable, GLXContext ctx);
typedef Bool (*_glXQueryExtension)(Display *dpy, int *errorBase, int *eventBase);
typedef const char* (*_glXQueryExtensionsString)(Display *dpy, int screen);
typedef const char* (*_glXQueryServerString)(Display *dpy, int screen, int name);
typedef void (*_glXSwapBuffers)(Display *dpy, GLXDrawable drawable);
typedef void (*_glXUseXFont)(Font font, int first, int count, int listBase);
typedef void (*_glXWaitX)(void);

extern _glCallLists qt_glCallLists;
extern _glClearColor qt_glClearColor;
extern _glClearIndex qt_glClearIndex;
extern _glColor3ub qt_glColor3ub;
extern _glDeleteLists qt_glDeleteLists;
extern _glDrawBuffer qt_glDrawBuffer;
extern _glFlush qt_glFlush;
extern _glIndexi qt_glIndexi;
extern _glListBase qt_glListBase;
extern _glLoadIdentity qt_glLoadIdentity;
extern _glMatrixMode qt_glMatrixMode;
extern _glOrtho qt_glOrtho;
extern _glPopAttrib qt_glPopAttrib;
extern _glPopMatrix qt_glPopMatrix;
extern _glPushAttrib qt_glPushAttrib;
extern _glPushMatrix qt_glPushMatrix;
extern _glRasterPos2i qt_glRasterPos2i;
extern _glRasterPos3d qt_glRasterPos3d;
extern _glReadPixels qt_glReadPixels;
extern _glViewport qt_glViewport;
extern _glPixelStorei qt_glPixelStorei;
extern _glBitmap qt_glBitmap;
extern _glDrawPixels qt_glDrawPixels;
extern _glNewList qt_glNewList;
extern _glGetFloatv qt_glGetFloatv;
extern _glGetIntegerv qt_glGetIntegerv;
extern _glEndList qt_glEndList;

extern _glXChooseVisual qt_glXChooseVisual;
extern _glXCreateContext qt_glXCreateContext;
extern _glXCreateGLXPixmap qt_glXCreateGLXPixmap;
extern _glXDestroyContext qt_glXDestroyContext;
extern _glXDestroyGLXPixmap qt_glXDestroyGLXPixmap;
extern _glXGetClientString qt_glXGetClientString;
extern _glXGetConfig qt_glXGetConfig;
extern _glXIsDirect qt_glXIsDirect;
extern _glXMakeCurrent qt_glXMakeCurrent;
extern _glXQueryExtension qt_glXQueryExtension;
extern _glXQueryExtensionsString qt_glXQueryExtensionsString;
extern _glXQueryServerString qt_glXQueryServerString;
extern _glXSwapBuffers qt_glXSwapBuffers;
extern _glXUseXFont qt_glXUseXFont;
extern _glXWaitX qt_glXWaitX;
}; // extern "C"

#define glCallLists qt_glCallLists
#define glClearColor qt_glClearColor
#define glClearIndex qt_glClearIndex
#define glColor3ub qt_glColor3ub
#define glDeleteLists qt_glDeleteLists
#define glDrawBuffer qt_glDrawBuffer
#define glFlush qt_glFlush
#define glIndexi qt_glIndexi
#define glListBase qt_glListBase
#define glLoadIdentity qt_glLoadIdentity
#define glMatrixMode qt_glMatrixMode
#define glOrtho qt_glOrtho
#define glPopAttrib qt_glPopAttrib
#define glPopMatrix qt_glPopMatrix
#define glPushAttrib qt_glPushAttrib
#define glPushMatrix qt_glPushMatrix
#define glRasterPos2i qt_glRasterPos2i
#define glRasterPos3d qt_glRasterPos3d
#define glReadPixels qt_glReadPixels
#define glViewport qt_glViewport
#define glPixelStorei qt_glPixelStorei
#define glBitmap qt_glBitmap
#define glDrawPixels qt_glDrawPixels
#define glNewList qt_glNewList
#define glGetFloatv qt_glGetFloatv
#define glGetIntegerv qt_glGetIntegerv
#define glEndList qt_glEndList

#define glXChooseVisual qt_glXChooseVisual
#define glXCreateContext qt_glXCreateContext
#define glXCreateGLXPixmap qt_glXCreateGLXPixmap
#define glXDestroyContext qt_glXDestroyContext
#define glXDestroyGLXPixmap qt_glXDestroyGLXPixmap
#define glXGetClientString qt_glXGetClientString
#define glXGetConfig qt_glXGetConfig
#define glXIsDirect qt_glXIsDirect
#define glXMakeCurrent qt_glXMakeCurrent
#define glXQueryExtension qt_glXQueryExtension
#define glXQueryExtensionsString qt_glXQueryExtensionsString
#define glXQueryServerString qt_glXQueryServerString
#define glXSwapBuffers qt_glXSwapBuffers
#define glXUseXFont qt_glXUseXFont
#define glXWaitX qt_glXWaitX

#else
inline bool qt_resolve_gl_symbols(bool = TRUE) { return TRUE; }
#endif // QT_DLOPEN_OPENGL
#endif // QGL_P_H
