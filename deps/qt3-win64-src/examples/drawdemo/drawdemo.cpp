/****************************************************************************
** $Id: drawdemo.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qwidget.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qapplication.h>
#include <math.h>

//
// First we define the functionality our demo should present 
// to the user. You might add different demo-modes if you wish so.
//

//
// This function draws a color wheel.
// The coordinate system x=(0..500), y=(0..500) spans the paint device.
//

void drawColorWheel( QPainter *p )
{
    QFont f( "times", 18, QFont::Bold );
    p->setFont( f );
    p->setPen( Qt::black );
    p->setWindow( 0, 0, 500, 500 );		// defines coordinate system

    for ( int i=0; i<36; i++ ) {		// draws 36 rotated rectangles

        QWMatrix matrix;
        matrix.translate( 250.0F, 250.0F );	// move to center
        matrix.shear( 0.0F, 0.3F );		// twist it
        matrix.rotate( (float)i*10 );		// rotate 0,10,20,.. degrees
        p->setWorldMatrix( matrix );		// use this world matrix

        QColor c;
        c.setHsv( i*10, 255, 255 );		// rainbow effect
        p->setBrush( c );			// solid fill with color c
        p->drawRect( 70, -10, 80, 10 );		// draw the rectangle

        QString n;
        n.sprintf( "H=%d", i*10 );
        p->drawText( 80+70+5, 0, n );		// draw the hue number
    }
}


//
// This function draws a few lines of text using different fonts.
//

void drawFonts( QPainter *p )
{
    static const char *fonts[] = { "Helvetica", "Courier", "Times", 0 };
    static int	 sizes[] = { 10, 12, 18, 24, 36, 0 };
    int f = 0;
    int y = 0;
    while ( fonts[f] ) {
        int s = 0;
        while ( sizes[s] ) {
            QFont font( fonts[f], sizes[s] );
            p->setFont( font );
            QFontMetrics fm = p->fontMetrics();
            y += fm.ascent();
            p->drawText( 10, y, "Quartz Glyph Job Vex'd Cwm Finks" );
            y += fm.descent();
            s++;
        }
        f++;
    }
}


//
// This function draws some shapes
//

void drawShapes( QPainter *p )
{
    QBrush b1( Qt::blue );
    QBrush b2( Qt::green, Qt::Dense6Pattern );		// green 12% fill
    QBrush b3( Qt::NoBrush );				// void brush
    QBrush b4( Qt::CrossPattern );			// black cross pattern

    p->setPen( Qt::red );
    p->setBrush( b1 );
    p->drawRect( 10, 10, 200, 100 );
    p->setBrush( b2 );
    p->drawRoundRect( 10, 150, 200, 100, 20, 20 );
    p->setBrush( b3 );
    p->drawEllipse( 250, 10, 200, 100 );
    p->setBrush( b4 );
    p->drawPie( 250, 150, 200, 100, 45*16, 90*16 );
}


typedef void (*draw_func)(QPainter*);

struct DrawThing {
    draw_func	 f;
    const char	*name;
};

//
// All previously implemented functions are collected
// in the following "table". 
// If you implement different functionality, your new draw 
// function must be assigned here with a function pointer and 
// description. 
// Leave the zeros at the end, they will be used
// as markers referring to the end of the array. 
//

DrawThing ourDrawFunctions[] = {
// name of the function, title presented to the user 
    { drawColorWheel,	"Draw color wheel" },
    { drawFonts,	"Draw fonts" },
    { drawShapes,	"Draw shapes" },
    { 0,		0 } };



class DrawView : public QWidget
{
    Q_OBJECT
public:
    DrawView();
    ~DrawView();
public slots:
    void   updateIt( int );
    void   printIt();
protected:
    void   drawIt( QPainter * );
    void   paintEvent( QPaintEvent * );
    void   resizeEvent( QResizeEvent * );
private:
    QPrinter	 *printer;
    QButtonGroup *bgroup;
    QPushButton	 *print;
    int		  drawindex;
    int		  maxindex;
};


//
// Construct the DrawView with buttons.
//

DrawView::DrawView()
{
    setCaption( "Qt Draw Demo Application" );
    setBackgroundMode(PaletteBase);

    // Create a button group to contain all buttons
    bgroup = new QButtonGroup( this );
    bgroup->resize( 200, 200 );
    connect( bgroup, SIGNAL(clicked(int)), SLOT(updateIt(int)) );

    // Calculate the size for the radio buttons
    int maxwidth = 80;
    int maxheight = 10;
    int i;
    const char *n;
    QFontMetrics fm = bgroup->fontMetrics();
    
    // Find out the longest function description.
    // Here we make use of the last "0,0"-entry in the 
    // ourDrawFunctions-array.
    for ( i=0; (n=ourDrawFunctions[i].name) != 0; i++ ) {
        int w = fm.width( n );
        maxwidth = QMAX(w,maxwidth); // QMAX is a macro defined in qglobal.h
                                     // and returns the biggest of to values.
	// Due to its macro nature one should use it with care and with
	// constant parameters only. 
    }

    maxwidth = maxwidth + 30;			// allow 30 pixels for radiobuttons

    for ( i=0; (n=ourDrawFunctions[i].name) != 0; i++ ) {
        QRadioButton *rb = new QRadioButton( n, bgroup );
        rb->setGeometry( 10, i*30+10, maxwidth, 30 );

        maxheight += 30;

	if ( i == 0 )
            rb->setChecked( TRUE );
    }

    maxheight += 10;                            // maxheight is now 10 pixels upper margin
    						// plus number_of_drawfunctions * 30
    						// plus 10 pixels lower margin	

    drawindex = 0;				// draw first thing
    maxindex  = i;

    maxwidth += 20;				// add some margin, this results in the
   						// final width of bgroup 

    bgroup->resize( maxwidth, maxheight );      // resize bgroup to its final size
  						// when no printersupport is provided  


// If -- at compile time -- printer support will be disabled,
// we won't set up printing functionality.

#ifndef QT_NO_PRINTER
    
    printer = new QPrinter;

    // Create and setup the print button
    print = new QPushButton( "Print...", bgroup );
    print->resize( 80, 30 );
    print->move( maxwidth/2 - print->width()/2, maxindex*30+20 );
    connect( print, SIGNAL(clicked()), SLOT(printIt()) );

    // Resize bgroup to its final size when printersupport is given.
    bgroup->resize( maxwidth, print->y()+print->height()+10 );

#endif    

    resize( 640,300 );
}

//
// Clean up.
//
DrawView::~DrawView()
{
#ifndef QT_NO_PRINTER
    delete printer;
#endif
}

//
// Called when a radio button is clicked.
//

void DrawView::updateIt( int index )
{
    if ( index < maxindex ) {
        drawindex = index;
        update();
    }
}

//
// Calls the drawing function as specified by the radio buttons.
//

void DrawView::drawIt( QPainter *p )
{
    (*ourDrawFunctions[drawindex].f)(p);
}

//
// Called when the print button is clicked.
//

void DrawView::printIt()
{
    if ( printer->setup( this ) ) {
	QPainter paint;
	if( !paint.begin( printer ) )               
	    return;
        drawIt( &paint );
    }
}

//
// Called when the widget needs to be updated.
//

void DrawView::paintEvent( QPaintEvent * )
{
    QPainter paint( this );
    drawIt( &paint );
}

//
// Called when the widget has been resized.
// Moves the button group to the upper right corner
// of the widget.

void DrawView::resizeEvent( QResizeEvent * )
{
    bgroup->move( width()-bgroup->width(), 0 );
}


//
// Create and display our widget.
//

#include "drawdemo.moc"

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    DrawView   draw;
    app.setMainWidget( &draw );
    draw.setCaption("Qt Example - Drawdemo");
    draw.show();
    return app.exec();
}
