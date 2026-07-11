/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qstylefactory.h>
#include "widgets.h"

class MyWidgetView : public WidgetView
{
    int s;
public:
    MyWidgetView( QWidget *parent=0, const char *name=0 )
	:WidgetView(parent, name), s(0)
    {
	setToolBarsMovable( TRUE );
    }

    void button1Clicked()
    {
	QStringList styles = QStyleFactory::keys();

	s = (++s)%styles.count();
	qApp->setStyle( styles[ s] );
	WidgetView::button1Clicked();
    }
};


//
// Create and display our WidgetView.
//

int main( int argc, char **argv )
{
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication a( argc, argv );

    MyWidgetView* w = new MyWidgetView;
    a.setMainWidget( w );

    w->show();
    int res = a.exec();
    delete w;
    return res;
}
