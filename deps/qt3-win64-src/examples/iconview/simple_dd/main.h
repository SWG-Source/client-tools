/****************************************************************************
** $Id: main.h 2051 2007-02-21 10:04:20Z chehrlic $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include <qapplication.h>
#include <qcursor.h>
#include <qsplitter.h>
#include <qlistbox.h>
#include <qiconview.h>
#include <qpixmap.h>

class QDragEnterEvent;
class QDragDropEvent;


class DDListBox : public QListBox
{
    Q_OBJECT
public:
    DDListBox( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
    // Low-level drag and drop
    void dragEnterEvent( QDragEnterEvent *evt );
    void dropEvent( QDropEvent *evt );
    void mousePressEvent( QMouseEvent *evt );
    void mouseMoveEvent( QMouseEvent * );
private:
    int dragging;
};


class DDIconViewItem : public QIconViewItem
{
public:
    DDIconViewItem( QIconView *parent, const QString& text, const QPixmap& icon ) :
	QIconViewItem( parent, text, icon ) {}
    DDIconViewItem( QIconView *parent, const QString &text ) :
	QIconViewItem( parent, text ) {}
    // High-level drag and drop
    bool acceptDrop( const QMimeSource *mime ) const;
    void dropped( QDropEvent *evt, const QValueList<QIconDragItem>& );
};


class DDIconView : public QIconView
{
    Q_OBJECT
public:
    DDIconView( QWidget * parent = 0, const char * name = 0, WFlags f = 0 ) :
	QIconView( parent, name, f ) {}
    // High-level drag and drop
    QDragObject *dragObject();
public slots:
    void slotNewItem( QDropEvent *evt, const QValueList<QIconDragItem>& list );
};

