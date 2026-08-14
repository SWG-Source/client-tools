#include "graph.h"
#include <qcanvas.h>
#include <stdlib.h>
#include <qdatetime.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlabel.h>
#include <qlayout.h>

const int bounce_rtti = 1234;

// We use a global variable to save memory - all the brushes and pens in
// the mesh are shared.
static QBrush *tb = 0;
static QPen *tp = 0;

class EdgeItem;
class NodeItem;
class FigureEditor;
typedef QValueList<NodeItem*> NodeItemList;
typedef QValueList<EdgeItem*> EdgeItemList;

#define SPEED2ADVANCE(x) (301-x)

class GraphWidgetPrivate
{
public:
    GraphWidgetPrivate() {
	moving = 0;
	speed = 275;
    }
    ~GraphWidgetPrivate() {
	delete canvas;
    }
    NodeItemList nodeItems;
    FigureEditor* editor;
    QCanvas* canvas;
    QCanvasItem* moving;
    int speed;
};

class EdgeItem: public QCanvasLine
{
public:
    EdgeItem( NodeItem*, NodeItem*, QCanvas* );
    void setFromPoint( int x, int y ) ;
    void setToPoint( int x, int y );
    void moveBy(double dx, double dy);

    NodeItem* from;
    NodeItem* to;
};



class NodeItem: public QCanvasEllipse
{
public:
    NodeItem( GraphWidgetPrivate* g );
    ~NodeItem() {}

    void addInEdge( EdgeItem *edge ) { inList.append( edge ); }
    void addOutEdge( EdgeItem *edge ) { outList.append( edge ); }

    void moveBy(double dx, double dy);

    void calcForce();
    void advance( int stage );

private:
    GraphWidgetPrivate* graph;
    EdgeItemList inList;
    EdgeItemList outList;
};



void EdgeItem::moveBy(double, double)
{
    //nothing
}

EdgeItem::EdgeItem( NodeItem *fromItem, NodeItem *toItem, QCanvas *canvas )
    : QCanvasLine( canvas )
{
    from = fromItem;
    to = toItem;
    setPen( *tp );
    setBrush( *tb );
    from->addOutEdge( this );
    to->addInEdge( this );
    setPoints( int(from->x()), int(from->y()), int(to->x()), int(to->y()) );
    setZ( 127 );
}

void EdgeItem::setFromPoint( int x, int y )
{
    setPoints( x,y, endPoint().x(), endPoint().y() );
}

void EdgeItem::setToPoint( int x, int y )
{
    setPoints( startPoint().x(), startPoint().y(), x, y );
}


void NodeItem::moveBy(double dx, double dy)
{
    double nx = x() + dx;
    double ny = y() + dy;
    if ( graph->moving != this ) {
	nx = QMAX( width()/2, nx );
	ny = QMAX( height()/2, ny );
	nx = QMIN( canvas()->width() - width()/2, nx );
	ny = QMIN( canvas()->height() - height()/2, ny );
    }
    QCanvasEllipse::moveBy( nx-x(), ny-y() );
    EdgeItemList::Iterator it;
    for (  it = inList.begin(); it != inList.end(); ++it )
	(*it)->setToPoint( int(x()), int(y()) );
    for (  it = outList.begin(); it != outList.end(); ++it )
	(*it)->setFromPoint( int(x()), int(y()) );
}

NodeItem::NodeItem( GraphWidgetPrivate* g )
    : QCanvasEllipse( 32, 32, g->canvas )
{
    graph = g;
    graph->nodeItems.append( this );
    setPen( *tp );
    setBrush( *tb );
    setZ( 128 );
}

void NodeItem::advance( int stage ) {
	switch ( stage ) {
	case 0:
	    calcForce();
	    break;
	case 1:
	    QCanvasItem::advance(stage);
	    break;
	}
}

void NodeItem::calcForce() {
    if ( graph->moving == this ) {
	setVelocity( 0, 0 );
	return;
    }
    double xvel = 0;
    double yvel = 0;
    for ( NodeItemList::Iterator it = graph->nodeItems.begin(); it != graph->nodeItems.end(); ++it ) {
	NodeItem* n = (*it);
	if ( n == this )
	    continue;
	double dx  = x() - n->x();
	double dy  = y() - n->y();
	double l = 2 * ( dx * dx + dy * dy );
	if ( l > 0 ) {
	    xvel = xvel + dx*260 / l;
	    yvel = yvel + dy*260 / l;
	}
    }
    double w = 1 + outList.count() + inList.count();
    w *= 10;
    EdgeItemList::Iterator it2;
    EdgeItem * e;
    NodeItem* n;
    for ( it2 = outList.begin(); it2 != outList.end(); ++it2 ) {
	e = (*it2);
	n = e->to;
	xvel = xvel - ( x() - n->x() ) / w;
	yvel = yvel - ( y() - n->y() ) / w;
    }
    for ( it2 = inList.begin(); it2 != inList.end(); ++it2 ) {
	e = (*it2);
	n = e->from;
	xvel = xvel - ( x() - n->x() ) / w;
	yvel = yvel - ( y() - n->y() ) / w;
    }
    if ( QABS( xvel ) < .1 && QABS( yvel ) < .1 )
	xvel = yvel = 0;
   setVelocity( xvel, yvel );
}


class FigureEditor : public QCanvasView {
public:
    FigureEditor( GraphWidgetPrivate *g, QWidget* parent=0, const char* name=0, WFlags f=0);

    QSize sizeHint() const;


protected:
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);


    void resizeEvent( QResizeEvent* );
    void showEvent( QShowEvent* );
    void hideEvent( QHideEvent* e);

private:
    void initialize();
    QPoint moving_start;
    GraphWidgetPrivate* graph;
};


FigureEditor::FigureEditor(
	GraphWidgetPrivate* g, QWidget* parent,
	const char* name, WFlags f) :
    QCanvasView(g->canvas, parent,name,f)
{
    graph = g;
}

void FigureEditor::contentsMousePressEvent(QMouseEvent* e)
{
    QCanvasItemList l=canvas()->collisions(e->pos());
    for (QCanvasItemList::Iterator it=l.begin(); it!=l.end(); ++it) {
	if ((*it)->rtti()==bounce_rtti )
	    continue;
	graph->moving = *it;
	moving_start = e->pos();
	return;
    }
    graph->moving = 0;
}

void FigureEditor::contentsMouseReleaseEvent(QMouseEvent* )
{
    if ( graph->moving )
	graph->moving = 0;
}

void FigureEditor::contentsMouseMoveEvent(QMouseEvent* e)
{
    if ( graph->moving ) {
	graph->moving->moveBy(e->pos().x() - moving_start.x(),
		       e->pos().y() - moving_start.y());
	moving_start = e->pos();
	canvas()->update();
    }
}

class BouncyText : public QCanvasText {
    void initPos();
    void initSpeed();
public:
    int rtti() const;
    BouncyText(const QString&, QFont, QCanvas*);
    void advance(int);
};

BouncyText::BouncyText( const QString& text, QFont f, QCanvas* canvas) :
    QCanvasText(text, f, canvas)
{
    setAnimated(TRUE);
    initPos();
}


int BouncyText::rtti() const
{
    return bounce_rtti;
}

void BouncyText::initPos()
{
    initSpeed();
    int trial=1000;
    do {
	move(rand()%(canvas()->width()-boundingRect().width()),
	     rand()%(canvas()->height()-boundingRect().height()));
	advance(0);
    } while (trial-- && xVelocity()==0.0 && yVelocity()==0.0);
}

void BouncyText::initSpeed()
{
    const double speed = 2.0;
    double d = (double)(rand()%1024) / 1024.0;
    double e = (double)(rand()%1024) / 1024.0;

    if ( d < .5 )
	d = -1 - d;
    else
	d = d + 1;
    if ( e < .5 )
	e = -1 - e;
    else
	e = e + 1;

    setVelocity( d*speed, e * speed );
}

void BouncyText::advance( int stage )
{
    switch ( stage ) {
      case 0: {
	double vx = xVelocity();
	double vy = yVelocity();

	if ( vx == 0.0 && vy == 0.0 ) {
	    // stopped last turn
	    initSpeed();
	    vx = xVelocity();
	    vy = yVelocity();
	}

	QRect r = boundingRect();
	r.moveBy( int(vx), int(vy) );

	if ( r.left() < 0 || r.right() > canvas()->width() )
	    vx = -vx;
	if ( r.top() < 0 || r.bottom() > canvas()->height() )
	    vy = -vy;

	r = boundingRect();
	r.moveBy( int(vx), int(vy) );
	if ( r.left() < 0 || r.right() > canvas()->width() )
	    vx = 0;
	if ( r.top() < 0 || r.bottom() > canvas()->height() )
	    vy = 0;

	setVelocity( vx, vy );
      } break;
      case 1:
	QCanvasItem::advance( stage );
	break;
    }
}

GraphWidget::GraphWidget( QWidget *parent, const char *name)
    : QWidget( parent, name )
{
    d = new GraphWidgetPrivate;
    d->canvas = 0;
    QVBoxLayout* vb = new QVBoxLayout(  this, 11, 6 );
    d->editor = new FigureEditor( d, this  );
    vb->addWidget( d->editor );
    QHBoxLayout* hb = new QHBoxLayout(  vb );
    hb->addWidget( new QLabel("Slow", this ) );
    QSlider* slider = new QSlider( 0, 300, 25, d->speed, Horizontal, this );
    connect( slider, SIGNAL( valueChanged(int) ), this, SLOT( setSpeed(int) ) );
    hb->addWidget( slider );
    hb->addWidget( new QLabel("Fast", this ) );
    hb->addSpacing( 10 );
    QPushButton* btn = new QPushButton( "Shuffle Nodes", this );
    connect( btn, SIGNAL( clicked() ), this, SLOT( shuffle() ) );
    hb->addWidget( btn );
}


GraphWidget::~GraphWidget()
{
    delete d;
}

void GraphWidget::setSpeed(int s)
{
    d->speed = s;
    if ( isVisible() && d->canvas )
	d->canvas->setAdvancePeriod( SPEED2ADVANCE( s ) );
}

void GraphWidget::shuffle()
{

    for ( NodeItemList::Iterator it = d->nodeItems.begin(); it != d->nodeItems.end(); ++it ) {
	NodeItem* ni = (*it);
	ni->move(rand()%(d->canvas->width()-ni->width()),rand()%(d->canvas->height()-ni->height()));
    }
}


QSize FigureEditor::sizeHint() const
{
    return QSize( 600, 400 );
}

void FigureEditor::resizeEvent( QResizeEvent* e )
{
    if ( canvas() )
	canvas()->resize( contentsRect().width(), contentsRect().height() );
    QCanvasView::resizeEvent( e );
}

void FigureEditor::showEvent( QShowEvent* )
{
    initialize();
    canvas()->setAdvancePeriod( SPEED2ADVANCE(graph->speed) );
}

void FigureEditor::hideEvent( QHideEvent* )
{
    initialize();
    canvas()->setAdvancePeriod( -10 );
}

void FigureEditor::initialize()
{
    if ( canvas() )
	return;
    resize( sizeHint() );
    graph->canvas = new QCanvas( contentsRect().width(), contentsRect().height() );
    if ( !tb ) tb = new QBrush( Qt::red );
    if ( !tp ) tp = new QPen( Qt::black );
    srand( QTime::currentTime().msec() );
    int nodecount = 0;

    int rows = 3;
    int cols = 3;

    QMemArray<NodeItem*> lastRow(cols);
    for ( int r = 0; r < rows; r++ ) {
	NodeItem *prev = 0;
	for ( int c = 0; c < cols; c++ ) {
	    NodeItem *ni = new NodeItem( graph );
	    ni->setAnimated( TRUE );
	    nodecount++;
	    ni->move(rand()%(graph->canvas->width()-ni->width()),rand()%(graph->canvas->height()-ni->height()));

	    if ( r > 0 )
		(new EdgeItem( lastRow[c], ni, graph->canvas ))->show();
	    if ( prev )
		(new EdgeItem( prev, ni, graph->canvas ))->show();
	    prev = ni;
	    lastRow[c] = ni;
	    ni->show();
	}
    }

    graph->canvas->advance();

    QCanvasItem* i = new BouncyText( tr( "Drag the nodes around!" ), QFont("helvetica", 24), graph->canvas);
    i->show();
    setCanvas( graph->canvas );
    setMinimumSize( 600, 400 );
    setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
}
