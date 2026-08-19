/****************************************************************************
** $Id: gltexobj.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
****************************************************************************/

#ifndef GLTEXOBJ_H
#define GLTEXOBJ_H

#include "glcontrolwidget.h"

class GLTexobj : public GLControlWidget
{
    Q_OBJECT

public:
    GLTexobj( QWidget* parent, const char* name = 0, WFlags f = 0 );
    ~GLTexobj();

protected:
    void		animate();
    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject( const QImage& tex1, const QImage& tex2 );

    void		setRotationImpulse( double x, double y, double z );

private:
    GLuint object;
    double impX, impY, impZ;
};

#endif // GLTEXOBJ_H
