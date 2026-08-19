#include <qimage.h>
#include "gllandscape.h"


#include <math.h>

#include "fbm.h"

#ifndef PI
#define PI 3.14159
#endif

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#pragma warning(disable:4244) // init: truncation from const double to float
#endif

GLLandscape::GLLandscape( QWidget * parent, const char * name )
    : QGLWidget( parent, name )
{
    mouseButtonDown  = FALSE;
    animationRunning = FALSE;
    oldX = oldY = oldZ = 0.0;
    landscape     = 0;
    vertexNormals = 0;
    normals       = 0;
    wave          = 0;
    wt            = 0;
    cubeRot 	  = 0;
    createGrid( 50 );
    setWireframe( 0 );
}

GLLandscape::~GLLandscape()
{
    destroyGrid();
}

void GLLandscape::initializeGL()
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glGetFloatv( GL_MODELVIEW_MATRIX,(GLfloat *) views[AxisView].model );
    
    glTranslatef( 0.0, 0.0, -50.0 );
    glRotatef( -45, 1, 0, 0 );
    glRotatef( -45, 0, 0, 1 );
    glGetFloatv( GL_MODELVIEW_MATRIX,(GLfloat *) views[CurrentView].model );
    glGetFloatv( GL_MODELVIEW_MATRIX,(GLfloat *) views[DefaultView].model );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    /* Use GL utility library function to obtain desired view */
    gluPerspective( 60, 1, 1, 250 );
    glGetFloatv( GL_PROJECTION_MATRIX, (GLfloat *)views[CurrentView].projection );
    glGetFloatv( GL_PROJECTION_MATRIX, (GLfloat *)views[DefaultView].projection );

    qglClearColor( black );
    glDepthFunc( GL_LESS );
    calculateVertexNormals();

    QImage tex;
    tex.load("opengl/qtlogo.png");
    tex = QGLWidget::convertToGLFormat(tex);  // flipped 32bit RGBA
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, tex.width(), tex.height(), 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
    initDisplayLists();
}

void GLLandscape::resizeGL( int width, int height )
{
    glViewport( 0, 0, width, height );
}

void GLLandscape::paintGL()
{
    QString str;
    GLboolean lighting;
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    switch ( mode ) {
	case Wireframe:
	    drawWireframe();
	    str = tr( "Wireframe" );
	    break;
	case Filled:
	    drawFilled();
	    str = tr( "Flat shaded" );
	    break;
	case SmoothShaded:
	    str = tr( "Smooth shaded" );
	    drawSmoothShaded();
	    break;
	case Landscape:
	    drawSmoothShaded();
	    str = tr( "Landscape" );
	    break;
    }
    glGetBooleanv( GL_LIGHTING, &lighting );
    if ( lighting )
	glDisable( GL_LIGHTING );
    qglColor( white );
    renderText(15, height() - 15, str);
    drawAxis();
    drawCube();
    if ( lighting )
	glEnable( GL_LIGHTING );
}

void GLLandscape::drawAxis()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glViewport(15, 20, 50, 50);	 
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.1, 1.1, -1.1, 1.1, 0.1, 10);
    glTranslatef(0, 0, -1.2);
    glRotatef(-45, 1, 0, 0);
    glRotatef(-45, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf((GLfloat *) views[AxisView].model);
    
    glCallList(axisList);
    qglColor(white);
    renderText(1.1f, 0, 0, "x");
    renderText(0, 1.1f, 0, "y");
    renderText(0, 0, 1.1f, "z");
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void GLLandscape::drawCube()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glViewport(width()-75, 0, 75, 75);	 
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.1, 1.1, -1.1, 1.1, 0.1, 10);
    glTranslatef(0, 0, -1.2);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(cubeRot, 1, 0, 0);
    glRotatef(cubeRot, 0, 1, 0);
    glRotatef(cubeRot, 0, 0, 1);
    glTranslatef(-0.5, -0.5, -0.5);
    
    glCallList(cubeList);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void GLLandscape::drawWireframe()
{
    qglColor( white );
    glBegin( GL_LINES );
    {
	for ( int y = 0; y < (gridSize-1); y++ )
	    for ( int x = 0; x < (gridSize-1); x++) {
		glVertex3f( x-gridHalf, y-gridHalf, landscape[x][y] );
		glVertex3f( x+1-gridHalf, y-gridHalf, landscape[x+1][y] );
		glVertex3f( x-gridHalf, y-gridHalf, landscape[x][y] );
		glVertex3f( x+1-gridHalf, y+1-gridHalf, landscape[x+1][y+1] );

		glVertex3f( x-gridHalf, y-gridHalf, landscape[x][y] );
		glVertex3f( x-gridHalf, y+1-gridHalf, landscape[x][y+1] );
	    }
    }
    glEnd();
    glBegin( GL_LINE_STRIP );
    {
	for ( int x = 0; x < gridSize; x++ ) {
	    glVertex3f( x-gridHalf, gridHalf-1, landscape[x][gridSize-1] );
	}
    }
    glEnd();
    glBegin( GL_LINE_STRIP );
    {
	for ( int y = 0; y < gridSize; y++ ) {
	    glVertex3f( gridHalf-1, y-gridHalf, landscape[gridSize-1][y] );
	}
    }
    glEnd();
}

void GLLandscape::drawFilled()
{
    for ( int y = 0; y < gridSize-1; y++ )
	for ( int x = 0; x < gridSize-1; x++ ) {
	    qglColor( red );
	    glBegin( GL_TRIANGLE_STRIP );
	    {
		glVertex3f(x-gridHalf,y-gridHalf, landscape[x][y]);
		glVertex3f(x+1-gridHalf,y-gridHalf, landscape[x+1][y]);
		glVertex3f(x-gridHalf,y+1-gridHalf, landscape[x][y+1]);
	    }
	    glEnd();
	    qglColor( white );
	    glBegin( GL_TRIANGLE_STRIP );
	    {
		glVertex3f(x+1-gridHalf,y-gridHalf, landscape[x+1][y]);
		glVertex3f(x+1-gridHalf,y+1-gridHalf, landscape[x+1][y+1]);
		glVertex3f(x-gridHalf,y+1-gridHalf, landscape[x][y+1]);
	    }
	    glEnd();
	}
}

void GLLandscape::drawSmoothShaded()
{
    if ( mode == SmoothShaded ) {
	GLfloat materialAmbient[]   = { 0.00, 0.00, 1.0, 0.0 };
	GLfloat materialShininess[] = { 128.0 };
	GLfloat materialSpecular[]  = { 1.0, 1.0, 1.0, 0.0 };

	glMaterialfv( GL_FRONT, GL_SPECULAR, materialSpecular );
	glMaterialfv( GL_FRONT, GL_AMBIENT, materialAmbient );
	glMaterialfv( GL_FRONT, GL_SHININESS, materialShininess );
    } else {
	GLfloat materialAmbient[]   = { 0.20, 0.33, 0.20, 0.0 };
	GLfloat materialShininess[] = { 1.0 };
	GLfloat materialSpecular[]  = { 0.1, 0.1, 0.1, 0.1 };

	glMaterialfv( GL_FRONT, GL_SPECULAR, materialSpecular );
	glMaterialfv( GL_FRONT, GL_AMBIENT, materialAmbient );
	glMaterialfv( GL_FRONT, GL_SHININESS, materialShininess );
    }

    for ( int y = 0; y < gridSize-1; y++ )
	for ( int x = 0; x < gridSize-1; x++ ) {
	    glBegin( GL_POLYGON );
	    {
		glNormal3dv(vertexNormals[x][y].n);
		glVertex3f(x-gridHalf,y-gridHalf,landscape[x][y]);

		glNormal3dv(vertexNormals[x+1][y].n);
		glVertex3f(x+1-gridHalf, y-gridHalf, landscape[x+1][y]);

		glNormal3dv(vertexNormals[x+1][y+1].n);
		glVertex3f(x+1-gridHalf, y+1-gridHalf, landscape[x+1][y+1]);
	    }
	    glEnd();

	    glBegin( GL_POLYGON );
	    {
		glNormal3dv(vertexNormals[x][y].n);
		glVertex3f(x-gridHalf,y-gridHalf, landscape[x][y]);

		glNormal3dv(vertexNormals[x+1][y+1].n);
		glVertex3f(x+1-gridHalf,y+1-gridHalf, landscape[x+1][y+1]);

		glNormal3dv(vertexNormals[x][y+1].n);
		glVertex3f(x-gridHalf,y+1-gridHalf, landscape[x][y+1]);
	    }
	    glEnd();
	}

    // Draw water
    if ( mode == Landscape ) {
	GLfloat materialAmbient[]   = { 0.00, 0.00, 1.0, 0.0 };
	GLfloat materialShininess[] = { 128.0 };
	GLfloat materialSpecular[]  = { 1.0, 1.0, 1.0, 0.0 };

	glMaterialfv( GL_FRONT, GL_SPECULAR, materialSpecular );
	glMaterialfv( GL_FRONT, GL_AMBIENT, materialAmbient );
	glMaterialfv( GL_FRONT, GL_SHININESS, materialShininess );
	
	glEnable( GL_BLEND );
	glBegin( GL_POLYGON );
	{
	    glNormal3f( 0, 0, 1 );
	    glVertex3f( -gridHalf, -gridHalf, .2 );
	    glNormal3f( 0, 0, 1 );
	    glVertex3f( -gridHalf, gridHalf, .2 );
	    glNormal3f( 0, 0, 1 );
	    glVertex3f( gridHalf, gridHalf, .2 );
	    glNormal3f( 0, 0, 1 );
	    glVertex3f( gridHalf, -gridHalf, .2 );
	
	}
	glEnd();
	glDisable( GL_BLEND );
    }
}

void GLLandscape::setGridSize( int size )
{
    destroyGrid();      // destroy old grid
    createGrid( size ); // create new grid
    initializeGL();
    updateGL();
}

void GLLandscape::createGrid( int size )
{
    if ( (size % 2) != 0 )
	size++;
    gridSize = size;
    gridHalf = gridSize / 2;
    initFractals  = TRUE;
    landscape     = new double*[gridSize];
    normals       = new gridNormals*[gridSize];
    vertexNormals = new avgNormals*[gridSize];
    wt            = new double*[gridSize];
    wave          = new double*[gridSize];
    for ( int i = 0; i < gridSize; i++ ) {
	landscape[i]     = new double[gridSize];
	normals[i]       = new gridNormals[gridSize];
	vertexNormals[i] = new avgNormals[gridSize];
	wt[i]   = new double[gridSize];
	wave[i] = new double[gridSize];

	memset( landscape[i], 0, gridSize*sizeof(double) );
	memset( normals[i], 0, gridSize*sizeof(gridNormals) );
	memset( vertexNormals[i], 0, gridSize*sizeof(avgNormals) );
	memset( wt[i], 0, gridSize*sizeof(double) );
	memset( wave[i], 0, gridSize*sizeof(double) );
    }
}

void GLLandscape::destroyGrid()
{
    if ( landscape != NULL ) {
	for( int i = 0; i < gridSize; i++ ) {
	    delete[] landscape[i];
	    delete[] normals[i];
	    delete[] vertexNormals[i];
	    delete[] wt[i];
	    delete[] wave[i];
	}
	delete[] landscape;
	delete[] normals;
	delete[] vertexNormals;
	delete[] wt;
	delete[] wave;
    }
    landscape = 0;
}

void GLLandscape::rotate( GLfloat deg, Axis axis )
{
    makeCurrent();
    glMatrixMode( GL_MODELVIEW );
    for ( int i = DefaultView; i <= AxisView; i++ ) {
	glLoadMatrixf((GLfloat *) views[i].model);
	if ( axis == XAxis )
	    glRotatef( deg, 1, 0, 0 );
	else if ( axis == YAxis )
	    glRotatef( deg, 0, 1, 0 );
	else
	    glRotatef( deg, 0, 0, 1 );
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) views[i].model);
    }
    glLoadMatrixf((GLfloat *) views[CurrentView].model);
}

void GLLandscape::rotateX( int deg )
{
    static int oldDeg = 0;

    rotate( deg-oldDeg, XAxis );
    oldDeg = deg;
    updateGL();
}

void GLLandscape::rotateY( int deg )
{
    static int oldDeg = 0;

    rotate( deg-oldDeg, YAxis );
    oldDeg = deg;
    updateGL();
}

void GLLandscape::rotateZ( int deg )
{
    static int oldDeg = 0;

    rotate( deg-oldDeg, ZAxis );
    oldDeg = deg;
    updateGL();
}

void GLLandscape::zoom( int z )
{
    float zoom;
    if ( z < 100 ) {
	zoom = 1 + 4.99 - (z*5.0 / 100.0);
    } else {
	z = 200 - z;
	zoom = z / 100.0;
    }
    makeCurrent();
    glMatrixMode( GL_MODELVIEW );
    // Always scale the original model matrix
    glLoadMatrixf((GLfloat *) views[DefaultView].model);
    glScalef( zoom, zoom, zoom );
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) views[CurrentView].model);
    updateGL();
}

void GLLandscape::resetGrid()
{
    setGridSize( gridSize );
}

void GLLandscape::fractalize()
{
    Vector p;
    double value;
    double roughness = 0.5;
    int frequency    = 50;

    p.x = p.y = p.z = 0;
    // Initialise fbm routine
    if ( initFractals ) {
	initFractals = FALSE;
	value = fBm( p, roughness, 2.0, 8.0, 1 );
    }

    // Fractalize grid
    for ( int x = 0; x < gridSize; x++ ) {
	for ( int y = 0; y < gridSize; y++ ) {
	    p.x = (double) x / (101 - frequency);
	    p.y = (double) y / (101 - frequency);
	    p.z = (double) landscape[x][y] / (101 - frequency);
	    value = fBm(p, roughness, 2.0, 8.0, 0);
	    landscape[x][y] += value;
	}
    }
    calculateVertexNormals();
    updateGL();
}


//
// Calculate the vector cross product of v and w, store result in n.
//
static void crossProduct( double v[3], double w[3], double n[3] )
{
    n[0] = v[1]*w[2]-w[1]*v[2];
    n[1] = w[0]*v[2]-v[0]*w[2];
    n[2] = v[0]*w[1]-w[0]*v[1];
}

void GLLandscape::calculateVertexNormals()
{
    double len, v[3], v2[3], w[3], w2[3], n[3], n2[3];

    // Calculate the surface normals for all polygons in the
    // height field
    for ( int i = 0; i < (gridSize-1); i++ )
	for ( int k = 0; k < (gridSize-1); k++ ) {
	    /* Lower poly normal */
	    v[0] = 1; // (i+1)-i
	    v[1] = 0; // k-k
	    v[2] = landscape[i+1][k]-landscape[i][k];
	    w[0] = 1; // (i+1)-i
	    w[1] = 1; // (k+1)-k
	    w[2] = landscape[i+1][k+1]-landscape[i][k];
	    crossProduct( v, w, n );
	    len = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
	    normals[i][k].l[0] = n[0]/len;
	    normals[i][k].l[1] = n[1]/len;
	    normals[i][k].l[2] = n[2]/len;

	    /* Upper poly normal */
	    v2[0] = -1.0; // i-(i+1);
	    v2[1] = 0.0;  // (k+1)-(k+1);
	    v2[2] = landscape[i][k+1]-landscape[i+1][k+1];
	    w2[0] = -1.0; // i-(i+1);
	    w2[1] = -1.0; // k-(k+1);
	    w2[2] = landscape[i][k]-landscape[i+1][k+1];
	    crossProduct( v2, w2, n2 );
	    len = sqrt(n2[0]*n2[0]+n2[1]*n2[1]+n2[2]*n2[2]);
	    normals[i][k].u[0] = n2[0]/len;
	    normals[i][k].u[1] = n2[1]/len;
	    normals[i][k].u[2] = n2[2]/len;
	}

    // Calculate proper vertex normals
    averageNormals();
}

void GLLandscape::averageNormals()
{
    // Calculate the average surface normal for a vertex based on
    // the normals of the surrounding polygons
    for ( int i = 0; i < gridSize; i++ )
	for ( int k = 0; k < gridSize; k++ ) {
	    if ( i > 0 && k > 0 && i < (gridSize-1) && k < (gridSize-1) ) {
		// For vertices that are *not* on the edge of the height field
		for ( int t = 0; t < 3; t++ ) // X, Y and Z components
		    vertexNormals[i][k].n[t] = ( normals[i][k].u[t] +
						 normals[i][k].l[t] +
						 normals[i][k-1].u[t] +
						 normals[i-1][k-1].u[t] +
						 normals[i-1][k-1].l[t] +
						 normals[i-1][k].l[t] )/6.0;
	    } else {
		// Vertices that are on the edge of the height field require
		// special attention..
		if ( i == 0 && k == 0 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = ( normals[i][k].u[t] +
						     normals[i][k].l[t] )/2.0;
		} else if ( i == gridSize-1 && k == gridSize-1 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = ( normals[i][k].u[t] +
						     normals[i][k].l[t] )/2.0;
		} else if ( i == gridSize-1) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = vertexNormals[i-1][k].n[t];
		} else if ( k == gridSize-1 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = vertexNormals[i][k-1].n[t];
		} else if ( k > 0 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = (normals[i][k].u[t] +
						    normals[i][k].l[t] +
			                            normals[i][k-1].u[t])/3.0;
		} else if ( i > 0 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = (normals[i][k].u[t] +
						    normals[i][k].l[t] +
			                            normals[i-1][k].l[t])/3.0;
		}
	    }
	}
}

void GLLandscape::setWireframe( int state )
{
    if ( state != 1 ) {
	// Enable line antialiasing
	makeCurrent();
	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glHint( GL_LINE_SMOOTH_HINT, GL_DONT_CARE );

	mode = Wireframe;
	updateGL();
    }
}

void GLLandscape::setFilled( int state )
{
    if ( state != 1 ) {
	makeCurrent();
	glEnable( GL_DEPTH_TEST );
	glDisable( GL_LINE_SMOOTH );
	glDisable( GL_BLEND );
	glDisable( GL_LIGHTING );
	glDisable( GL_NORMALIZE );

	mode = Filled;
	updateGL();
    }
}

void GLLandscape::setSmoothShaded( int state )
{
    if ( state != 1 ) {
	makeCurrent();
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glEnable( GL_NORMALIZE );
	glDisable( GL_LINE_SMOOTH );
	glDisable( GL_BLEND );

	glShadeModel( GL_SMOOTH );
	glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );

	// Setup lighting and material properties
	GLfloat position[] = { 15.0, -15.0, 15.0, 0.0 };
	GLfloat ambient[]  = { 0.50, 0.50, 0.50, 0.0 };
	GLfloat diffuse[]  = { 1.00, 1.00, 1.00, 0.0 };
	GLfloat specular[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat materialAmbient[]   = { 0.00, 0.00, 1.0, 0.0 };
        // GLfloat materialDiffuse[]   = { 1.00, 1.00, 1.0, 0.0 };
	GLfloat materialShininess[] = { 128.0 };
	GLfloat materialSpecular[]  = { 1.0, 1.0, 1.0, 0.0 };

	glMaterialfv( GL_FRONT, GL_SPECULAR, materialSpecular );
        // glMaterialfv( GL_FRONT, GL_DIFFUSE, materialDiffuse );
	glMaterialfv( GL_FRONT, GL_AMBIENT, materialAmbient );
	glMaterialfv( GL_FRONT, GL_SHININESS, materialShininess );

	glLightfv( GL_LIGHT0, GL_POSITION, position );
	glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
	glLightfv( GL_LIGHT0, GL_SPECULAR, specular );

	mode = SmoothShaded;
	calculateVertexNormals();
	updateGL();
    }
}

void GLLandscape::setLandscape( int state )
{
    if ( state != 1 ) {
	makeCurrent();
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glEnable( GL_NORMALIZE );
	glDisable( GL_LINE_SMOOTH );
	glDisable( GL_BLEND );

	glShadeModel( GL_SMOOTH );
	glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );

	// Setup lighting and material properties
	GLfloat position[] = { 15.0, -15.0, 15.0, 0.0 };
	GLfloat ambient[]  = { 0.50, 0.50, 0.50, 0.0 };
	GLfloat diffuse[]  = { 1.00, 1.00, 1.00, 0.0 };
	GLfloat specular[] = { 1.0, 1.0, 1.0, 0.0 };
	
	glLightfv( GL_LIGHT0, GL_POSITION, position );
	glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
	glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
	glLightfv( GL_LIGHT0, GL_SPECULAR, specular );

	glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR );

	mode = Landscape;
	calculateVertexNormals();
	updateGL();
    }
}

void GLLandscape::mousePressEvent( QMouseEvent *e )
{
    oldPos = e->pos();
    mouseButtonDown = TRUE;
}

void GLLandscape::mouseReleaseEvent( QMouseEvent *e )
{
    oldPos = e->pos();
    mouseButtonDown = FALSE;
}

void GLLandscape::mouseMoveEvent( QMouseEvent *e )
{
    GLfloat rx = (GLfloat) (e->x() - oldPos.x()) / width();
    GLfloat ry = (GLfloat) (e->y() - oldPos.y()) / height();

    if ( e->state() == LeftButton ) {
	// Left button down - rotate around X and Y axes
	oldX = 180*ry;
	oldY = 180*rx;
	rotate( oldX, XAxis );
	rotate( oldY, YAxis );
	updateGL();
    } else if ( e->state() == RightButton ) {
	// Right button down - rotate around X and Z axes
	oldX = 180*ry;
	oldZ = 180*rx;
	rotate( oldX, XAxis );
	rotate( oldZ, ZAxis );
	updateGL();
    }
    oldPos = e->pos();
}

void GLLandscape::timerEvent( QTimerEvent *e )
{
    if (e->timerId() == cubeTimer) {
  	cubeRot += 1;
	if (!animationRunning)
	    updateGL();
	return;
    } 
    
    int dx, dy; // disturbance point
    float s, v, W, t;
    int i, j;

    if ( mode == Landscape ) {
	dx = dy = 0;
    } else {
	dx = dy = gridSize >> 1;
    }
    W = 0.3;
    v = -4; // wave speed

    for ( i = 0; i < gridSize; i++ )
	for ( j = 0; j < gridSize; j++ )
	{
	    s = sqrt( (double) ( (j - dx) * (j - dx) + (i - dy) * (i - dy) ) );
	    wt[i][j] += 0.1;
	    t = s / v;

	    if ( mode == Landscape ) {
		if ( (landscape[i][j] + wave[i][j]) < 0 )
		    landscape[i][j] -= wave[i][j];
		if ( (dy - j != 0) || (dx - i != 0) )
		    wave[i][j] = (3 * sin( 2 * PI * W * (wt[i][j] + t ))) / 
				 (0.2*(sqrt( pow((double)(dx-i), 2) + pow((double)(dy-j), 2))+2));
		else
		    wave[i][j] = ( 3 * sin( 2 * PI * W * ( wt[i][j] + t ) ) );
		if ( landscape[i][j] + wave[i][j] < 0 )
		    landscape[i][j] += wave[i][j];

	    } else {
		landscape[i][j] -= wave[i][j];

		if ( s != 0 )
		    wave[i][j] = 2 * sin(2 * PI * W * ( wt[i][j] + t )) / 
				 (0.2*(s + 2));
		else
		    wave[i][j] = 2 * sin( 2 * PI * W * ( wt[i][j] + t ) );
		landscape[i][j] += wave[i][j];
	    }
	    
	}
    if ( mode == SmoothShaded || mode == Landscape )
	calculateVertexNormals();
    updateGL();
}

void GLLandscape::toggleWaveAnimation( bool state )
{
    if (state) {
 	animTimer = startTimer(20);
	animationRunning = TRUE;
    } else {
	killTimer(animTimer);
	animationRunning = FALSE;
    }
}

void GLLandscape::showEvent( QShowEvent * )
{
    if (animationRunning)
 	animTimer = startTimer(20);	
    cubeTimer = startTimer(50);
}

void GLLandscape::hideEvent( QHideEvent * )
{
    if (animationRunning)
	killTimer(animTimer);
    killTimer(cubeTimer);
}

void GLLandscape::initDisplayLists()
{
    // axisList
    axisList = glGenLists(1);
    glNewList(axisList, GL_COMPILE);
    
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    {
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(-1.0f, 0, 0); // x axis
	glVertex3f(1.0f, 0, 0);

	glVertex3f(1.0f, 0, 0);
	glVertex3f(0.8f, 0.2f, 0);
	glVertex3f(1.0f, 0, 0);
	glVertex3f(0.8f, -.2f, 0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, -1.0f, 0); // y axis
	glVertex3f(0, 1.0f, 0);
	
	glVertex3f(0, 1.0f, 0);
	glVertex3f(0.2f, 0.8f, 0);
	glVertex3f(0, 1.0f, 0);
	glVertex3f(-0.2f, 0.8f, 0);
	
	glColor3f(0.5, 0.5, 1.0);
	glVertex3f(0, 0, -1.0f); // z axis
	glVertex3f(0, 0, 1.0f);

	glVertex3f(0, 0, 1.0f);
	glVertex3f(0, 0.2f, 0.8f);
	glVertex3f(0, 0, 1.0f);
	glVertex3f(0,-0.2f, 0.8f);
    }
    glEnd();
    
    glEndList();
    
    // cubeList
    cubeList = glGenLists(1);
    glNewList(cubeList, GL_COMPILE);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
    
    glBegin( GL_QUADS );
    {
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );

	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );

	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );

	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );

	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 1.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 1.0, 1.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 );

	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 1.0, 0.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 1.0 ); glVertex3f( 0.0, 0.0, 1.0 );
	glTexCoord2f( 1.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 );
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEndList();
}
