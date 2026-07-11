#include <qapplication.h>
#include <qwidget.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qfile.h>

#include <stdlib.h>

class MoveMe : public QWidget
{
public:
    MoveMe( QWidget *parent=0, const char *name=0, WFlags f = 0)
	:QWidget(parent,name, f) {}

protected:
    void mousePressEvent( QMouseEvent *);
    void mouseMoveEvent( QMouseEvent *);
private:
    QPoint clickPos;
};

void MoveMe::mousePressEvent( QMouseEvent *e )
{
    clickPos = e->pos();
}

void MoveMe::mouseMoveEvent( QMouseEvent *e )
{
    move( e->globalPos() - clickPos );
}



int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QString fn="tux.png";

    if ( argc >= 2 )
	fn = argv[1];

    if ( ! QFile::exists( fn ) )
	exit( 1 );

    QImage img( fn );
    QPixmap p;
    p.convertFromImage( img );
    if ( !p.mask() )
	if ( img.hasAlphaBuffer() ) {
	    QBitmap bm;
	    bm = img.createAlphaMask();
	    p.setMask( bm );
	} else {
	    QBitmap bm;
	    bm = img.createHeuristicMask();
	    p.setMask( bm );
	}
    MoveMe w(0,0,Qt::WStyle_Customize|Qt::WStyle_NoBorder);
    w.setBackgroundPixmap( p );
    w.setFixedSize( p.size() );
    if ( p.mask() )
	w.setMask( *p.mask() );
    w.show();
    a.setMainWidget(&w);


    return a.exec();

}
