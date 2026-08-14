/**********************************************************************
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "glwidget.h"

/*!
  Create a GLWidget widget
*/

GLWidget::GLWidget( QWidget* parent, const char* name )
    : QGLWidget( parent, name )
{
    xrot = yrot = zrot = 25;		// default object rotation
    scale_ = 1.25;			// default object scale
    object = 0;
}

/*!
  Release allocated resources
*/

GLWidget::~GLWidget()
{
    glDeleteLists( object, 1 );
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLWidget::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT );

    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -10.0 );
    glScalef( scale_, scale_, scale_ );

    glRotatef( xrot, 1.0, 0.0, 0.0 ); 
    glRotatef( yrot, 0.0, 1.0, 0.0 ); 
    glRotatef( zrot, 0.0, 0.0, 1.0 );

    glCallList( object );
}

/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLWidget::initializeGL()
{
    qglClearColor( black ); 		// Let OpenGL clear to black
    object = makeObject();		// Generate an OpenGL display list
    glShadeModel( GL_FLAT );
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLWidget::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 15.0 );
    glMatrixMode( GL_MODELVIEW );
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the box
*/

GLuint GLWidget::makeObject()
{	
    GLuint list;

    list = glGenLists( 1 );

    glNewList( list, GL_COMPILE );

    qglColor( white );		      // Shorthand for glColor3f or glIndex

    glLineWidth( 2.0 );

    glBegin( GL_LINE_LOOP );
    glVertex3f(  1.0f,  0.5f, -0.4f );
    glVertex3f(  1.0f, -0.5f, -0.4f );
    glVertex3f( -1.0f, -0.5f, -0.4f );
    glVertex3f( -1.0f,  0.5f, -0.4f );
    glEnd();

    glBegin( GL_LINE_LOOP );
    glVertex3f(  1.0f,  0.5f, 0.4f );
    glVertex3f(  1.0f, -0.5f, 0.4f );
    glVertex3f( -1.0f, -0.5f, 0.4f );
    glVertex3f( -1.0f,  0.5f, 0.4f );
    glEnd();

    glBegin( GL_LINES );
    glVertex3f(  1.0f,  0.5f, -0.4f );   glVertex3f(  1.0f,  0.5f, 0.4f );
    glVertex3f(  1.0f, -0.5f, -0.4f );   glVertex3f(  1.0f, -0.5f, 0.4f );
    glVertex3f( -1.0f, -0.5f, -0.4f );   glVertex3f( -1.0f, -0.5f, 0.4f );
    glVertex3f( -1.0f,  0.5f, -0.4f );   glVertex3f( -1.0f,  0.5f, 0.4f );
    glEnd();

    glEndList();

    return list;
}

void GLWidget::setXRot( double x )
{
    xrot = x;
    update();
}

void GLWidget::setYRot( double y )
{
    yrot = y;
    update();
}

void GLWidget::setZRot( double z )
{
    zrot = z;
    update();
}

void GLWidget::setScale( double s )
{
    scale_ = s;
    update();
}
