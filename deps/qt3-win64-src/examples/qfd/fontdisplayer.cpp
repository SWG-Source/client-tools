/****************************************************************************
** $Id: fontdisplayer.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "fontdisplayer.h"
#include <qapplication.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qpainter.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qfontdialog.h>
#include <stdlib.h>


FontRowTable::FontRowTable( QWidget* parent, const char* name ) :
    QFrame(parent,name)
{
    setBackgroundMode(PaletteBase);
    setFrameStyle(Panel|Sunken);
    setMargin(8);
    setRow(0);
    tablefont = QApplication::font();
}

QSize FontRowTable::sizeHint() const
{
    return 24*cellSize()+QSize(2,2)*(margin()+frameWidth());
}

QSize FontRowTable::cellSize() const
{
    QFontMetrics fm = fontMetrics();
    return QSize( fm.maxWidth(), fm.lineSpacing()+1 );
}

void FontRowTable::paintEvent( QPaintEvent* e )
{
    QFrame::paintEvent(e);
    QPainter p(this);
    p.setClipRegion(e->region());
    QRect r = e->rect();
    QFontMetrics fm = fontMetrics();
    int ml = frameWidth()+margin() + 1 + QMAX(0,-fm.minLeftBearing());
    int mt = frameWidth()+margin();
    QSize cell((width()-15-ml)/16,(height()-15-mt)/16);

    if ( !cell.width() || !cell.height() )
	return;

    int mini = r.left() / cell.width();
    int maxi = (r.right()+cell.width()-1) / cell.width();
    int minj = r.top() / cell.height();
    int maxj = (r.bottom()+cell.height()-1) / cell.height();

    int h = fm.height();

    QColor body(255,255,192);
    QColor negative(255,192,192);
    QColor positive(192,192,255);
    QColor rnegative(255,128,128);
    QColor rpositive(128,128,255);

    for (int j = minj; j<=maxj; j++) {
	for (int i = mini; i<=maxi; i++) {
	    if ( i < 16 && j < 16 ) {
		int x = i*cell.width();
		int y = j*cell.height();

		QChar ch = QChar(j*16+i,row);

		if ( fm.inFont(ch) ) {
		    int w = fm.width(ch);
		    int l = fm.leftBearing(ch);
		    int r = fm.rightBearing(ch);

		    x += ml;
		    y += mt+h;

		    p.fillRect(x,y,w,-h,body);
		    if ( w ) {
			if ( l ) {
			    p.fillRect(x+(l>0?0:l), y-h/2, abs(l),-h/2,
				       l < 0 ? negative : positive);
			}
			if ( r ) {
			    p.fillRect(x+w-(r>0?r:0),y+2, abs(r),-h/2,
				       r < 0 ? rnegative : rpositive);
			}
		    }
		    QString s;
		    s += ch;
		    p.setPen(QPen(Qt::black));
		    p.drawText(x,y,s);
		}
	    }
	}
    }
}

void FontRowTable::setRow(int r)
{
    row = r;

    QFontMetrics fm = fontMetrics();
    QFontInfo fi = fontInfo();
    QString str = QString("%1 %2pt%3%4 mLB=%5 mRB=%6 mW=%7")
		    .arg(fi.family())
		    .arg(fi.pointSize())
		    .arg(fi.bold() ? " bold" : "")
		    .arg(fi.italic() ? " italic" : "")
		    .arg(fm.minLeftBearing())
		    .arg(fm.minRightBearing())
		    .arg(fm.maxWidth());

    emit fontInformation(str);
    update();
}

void FontRowTable::chooseFont()
{
    bool ok;
    QFont oldfont = tablefont;
    tablefont = QFontDialog::getFont(&ok, oldfont, this);

    if (ok)
	setFont(tablefont);
    else
	tablefont = oldfont;
}

FontDisplayer::FontDisplayer( QWidget* parent, const char* name ) :
    QMainWindow(parent,name)
{
    FontRowTable* table = new FontRowTable(this);
    QToolBar* controls = new QToolBar(this);
    (void) new QLabel(tr("Row:"), controls);
    QSpinBox *row = new QSpinBox(0,255,1,controls);
    controls->addSeparator();
    QPushButton *fontbutton = new QPushButton(tr("Font..."), controls);

    connect(row,SIGNAL(valueChanged(int)),table,SLOT(setRow(int)));
    connect(fontbutton, SIGNAL(clicked()), table, SLOT(chooseFont()));
    connect(table,SIGNAL(fontInformation(const QString&)),
	    statusBar(),SLOT(message(const QString&)));
    table->setRow(0);
    setCentralWidget(table);
}
