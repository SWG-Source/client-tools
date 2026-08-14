/****************************************************************************
** $Id: main.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "main.h"

const char* red_icon[]={
"16 16 2 1",
"r c red",
". c None",
"................",
"................",
"..rrrrrrrrrrrr..",
"..rrrrrrrrrrrr..",
"..rrrrrrrrrrrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrr......rrr..",
"..rrrrrrrrrrrr..",
"..rrrrrrrrrrrr..",
"..rrrrrrrrrrrr..",
"................",
"................"};

const char* blue_icon[]={
"16 16 2 1",
"b c blue",
". c None",
"................",
"................",
"..bbbbbbbbbbbb..",
"..bbbbbbbbbbbb..",
"..bbbbbbbbbbbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbb......bbb..",
"..bbbbbbbbbbbb..",
"..bbbbbbbbbbbb..",
"..bbbbbbbbbbbb..",
"................",
"................"};

const char* green_icon[]={
"16 16 2 1",
"g c green",
". c None",
"................",
"................",
"..gggggggggggg..",
"..gggggggggggg..",
"..gggggggggggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..ggg......ggg..",
"..gggggggggggg..",
"..gggggggggggg..",
"..gggggggggggg..",
"................",
"................"};


// ListBox -- low level drag and drop

DDListBox::DDListBox( QWidget * parent, const char * name, WFlags f ) :
    QListBox( parent, name, f )
{
    setAcceptDrops( TRUE );
    dragging = FALSE;
}


void DDListBox::dragEnterEvent( QDragEnterEvent *evt )
{
    if ( QTextDrag::canDecode( evt ) ) 
	evt->accept();
}


void DDListBox::dropEvent( QDropEvent *evt )
{
    QString text;

    if ( QTextDrag::decode( evt, text ) ) 
	insertItem( text );
}


void DDListBox::mousePressEvent( QMouseEvent *evt )
{
    QListBox::mousePressEvent( evt );
    dragging = TRUE;
}


void DDListBox::mouseMoveEvent( QMouseEvent * )
{
    if ( dragging ) {
	QDragObject *d = new QTextDrag( currentText(), this );
	d->dragCopy(); // do NOT delete d.
	dragging = FALSE;
    }
}


// IconViewIcon -- high level drag and drop


bool DDIconViewItem::acceptDrop( const QMimeSource *mime ) const
{
    if ( mime->provides( "text/plain" ) )
	return TRUE;
    return FALSE;
}


void DDIconViewItem::dropped( QDropEvent *evt, const QValueList<QIconDragItem>& )
{
    QString label;

    if ( QTextDrag::decode( evt, label ) ) 
	setText( label );
}


// IconView -- high level drag and drop

QDragObject *DDIconView::dragObject()
{
  return new QTextDrag( currentItem()->text(), this );
}

void DDIconView::slotNewItem( QDropEvent *evt, const QValueList<QIconDragItem>& )
{
    QString label;

    if ( QTextDrag::decode( evt, label ) ) {
	DDIconViewItem *item = new DDIconViewItem( this, label );
	item->setRenameEnabled( TRUE );
    }
}



int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    // Create and show the widgets
    QSplitter *split = new QSplitter();
    DDIconView *iv   = new DDIconView( split );
    (void)	       new DDListBox( split );
    app.setMainWidget( split );
    split->resize( 600, 400 );
    split->show();

    // Set up the connection so that we can drop items into the icon view
    QObject::connect( 
	iv, SIGNAL(dropped(QDropEvent*, const QValueList<QIconDragItem>&)), 
	iv, SLOT(slotNewItem(QDropEvent*, const QValueList<QIconDragItem>&)));

    // Populate the QIconView with icons
    DDIconViewItem *item;
    item = new DDIconViewItem( iv, "Red",   QPixmap( red_icon ) );
    item->setRenameEnabled( TRUE );
    item = new DDIconViewItem( iv, "Green", QPixmap( green_icon ) );
    item->setRenameEnabled( TRUE );
    item = new DDIconViewItem( iv, "Blue",  QPixmap( blue_icon ) );
    item->setRenameEnabled( TRUE );

    return app.exec();
}


