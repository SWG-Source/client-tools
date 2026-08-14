#ifndef GLCONTROLWIDGET_H
#define GLCONTROLWIDGET_H

#include <qgl.h>

class GLControlWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLControlWidget( QWidget *parent, const char *name = 0, QGLWidget *share = 0, WFlags f = 0 );

    virtual void	transform();

public slots:
    void		setXRotation( double degrees );
    void		setYRotation( double degrees );
    void		setZRotation( double degrees );

    void		setScale( double s );

    void		setXTrans( double x );
    void		setYTrans( double y );
    void		setZTrans( double z );

    virtual void	setRotationImpulse( double x, double y, double z );
    virtual void	setTranslationImpulse( double x, double y, double z );
    void		drawText();

protected:
    void		setAnimationDelay( int ms );
    void		mousePressEvent( QMouseEvent *e );
    void		mouseReleaseEvent( QMouseEvent *e );
    void		mouseMoveEvent( QMouseEvent * );
    void		mouseDoubleClickEvent( QMouseEvent * );
    void		wheelEvent( QWheelEvent * );

    void		showEvent( QShowEvent * );
    void		hideEvent( QHideEvent * );

    GLfloat xRot, yRot, zRot;
    GLfloat xTrans, yTrans, zTrans;
    GLfloat scale;
    bool animation;

protected slots:
    virtual void	animate();

private:
    bool wasAnimated;
    QPoint oldPos;
    QTimer* timer;
    int delay;
};

#endif
