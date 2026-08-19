/****************************************************************************
** $Id: gltexobj.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
**
** This is a simple QGLWidget demonstrating the use of QImages for textures.
**
** Much of the GL code is inspired by the 'spectex' and 'texcyl'
** public domain demo programs by Brian Paul.
**
****************************************************************************/

#include "gltexobj.h"
#include <qimage.h>

/*!
  Create a GLTexobj widget
*/

GLTexobj::GLTexobj( QWidget* parent, const char* name, WFlags f )
    : GLControlWidget( parent, name, 0, f ), impX( -2 ), impY( 0.5 ), impZ( 1 )
{
    object = 0;
}

/*!
  Release allocated resources
*/

GLTexobj::~GLTexobj()
{
    makeCurrent();
    glDeleteLists( object, 1 );
}


/*!
  Paint the texobj. The actual openGL commands for drawing the texobj are
  performed here.
*/

void GLTexobj::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glPushMatrix();
    transform();
    glCallList( object );
    drawText();
    glPushAttrib( GL_LIGHTING_BIT | GL_TEXTURE_BIT );
    glDisable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );
    qglColor( green );
    glLineWidth( 1.0 );
    glBegin( GL_LINES );
    {
	glVertex3f( 0.0f, 0.0f, 1.0f );
	glVertex3f( 0.98f, 0.98f, 0.98f );
    }
    glEnd();
    renderText( 1.0, 1.0, 1.0, "Can", QFont( "helvetica", 12, QFont::Bold, TRUE ) );    
    glPopAttrib();
    glPopMatrix();
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLTexobj::initializeGL()
{
    // Set up the lights

    GLfloat whiteDir[4] = {2.0, 2.0, 2.0, 1.0};
    GLfloat whiteAmb[4] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lightPos[4] = {30.0, 30.0, 30.0, 1.0};

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, whiteAmb);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, whiteDir);
    glMaterialfv(GL_FRONT, GL_SPECULAR, whiteDir);
    glMaterialf(GL_FRONT, GL_SHININESS, 20.0);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDir);		// enable diffuse
    glLightfv(GL_LIGHT0, GL_SPECULAR, whiteDir);	// enable specular
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // Set up the textures

    QImage tex1, tex2, buf;

    if ( !buf.load( "opengl/gllogo.bmp" ) ) {	// Load first image from file
	qWarning( "Could not read image file, using single-color instead." );
	QImage dummy( 128, 128, 32 );
	dummy.fill( Qt::green.rgb() );
	buf = dummy;
    }
    tex1 = QGLWidget::convertToGLFormat( buf );  // flipped 32bit RGBA

    if ( !buf.load( "opengl/qtlogo.bmp" ) ) {	// Load first image from file
	qWarning( "Could not read image file, using single-color instead." );
	QImage dummy( 128, 128, 32 );
	dummy.fill( Qt::red.rgb() );
	buf = dummy;
    }
    tex2 = QGLWidget::convertToGLFormat( buf );  // flipped 32bit RGBA

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glEnable( GL_TEXTURE_2D );

    // Set up various other stuff

    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // Let OpenGL clear to black
    glEnable( GL_CULL_FACE );  	// don't need Z testing for convex objects
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    // Make the object display list

    object = makeObject( tex1, tex2 );	// Generate an OpenGL display list
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLTexobj::resizeGL( int w, int h )
{
    glViewport( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 10.0, 100.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -70.0 );
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the texobj
*/

GLuint GLTexobj::makeObject( const QImage& tex1, const QImage& tex2 )
{
    GLUquadricObj* q = gluNewQuadric();
    GLuint cylinderObj = glGenLists(1);
    glNewList( cylinderObj, GL_COMPILE );

    glTranslatef( 0.0, 0.0, -1.0 );

    // cylinder
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1.width(), tex1.height(), 0,
		  GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits() );
    gluQuadricTexture( q, GL_TRUE );
    gluCylinder(q, 0.6, 0.6, 2.0, 24, 1);

    // end cap
    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex2.width(), tex2.height(), 0,
		  GL_RGBA, GL_UNSIGNED_BYTE, tex2.bits() );
    glTranslatef( 0.0, 0.0, 2.0 );
    gluDisk( q, 0.0, 0.6, 24, 1 );

    // other end cap
    glTranslatef( 0.0, 0.0, -2.0 );
    gluQuadricOrientation( q, (GLenum)GLU_INSIDE );
    gluDisk( q, 0.0, 0.6, 24, 1 );

    glEndList();
    gluDeleteQuadric( q );

    return cylinderObj;
}

void GLTexobj::animate()
{
    xRot += impX;
    yRot += impY;
    zRot -= impZ;

    impX -= impX * 0.05;
    impY -= impY * 0.05;
    impZ -= impZ * 0.05;

    if ( impX > 0.1 || impY > 0.1 || impZ > 0.1 || 
	 impX < -0.1 || impY < -0.1 || impZ < -0.1 )
	updateGL();
}

void GLTexobj::setRotationImpulse( double x, double y, double z )
{
    if ( animation ) {
	impX += 180*x;
	impY += 180*y;
	impZ += 180*z;
    } else {
	GLControlWidget::setRotationImpulse( x, y, z );
    }
}
