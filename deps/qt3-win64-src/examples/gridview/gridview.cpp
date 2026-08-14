/****************************************************************************
** $Id: gridview.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qgridview.h>
#include <qpainter.h>

// Grid size
const int numRows = 100;
const int numCols = 100;

class MyGridView : public QGridView
{
public:
    MyGridView() {
	setNumRows( ::numRows );
	setNumCols( ::numCols );
	setCellWidth( fontMetrics().width( QString("%1 / %2").arg(numRows()).arg(numCols()))); 
	setCellHeight( 2*fontMetrics().lineSpacing() );
	setCaption( tr( "Qt Example - This is a grid with 100 x 100 cells" ) );
    }

protected:
    void paintCell( QPainter *p, int row, int col ) {
	p->drawLine( cellWidth()-1, 0, cellWidth()-1, cellHeight()-1 );
	p->drawLine( 0, cellHeight()-1, cellWidth()-1, cellHeight()-1 );
	p->drawText( cellRect(), AlignCenter, QString("%1 / %1").arg(row).arg(col) );
    }
};

// The program starts here.
int main( int argc, char **argv )
{
    QApplication app( argc, argv );			

    MyGridView gridview;
    app.setMainWidget( &gridview );
    gridview.show();
    return app.exec();
}
