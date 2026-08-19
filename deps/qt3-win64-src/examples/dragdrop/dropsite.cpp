/****************************************************************************
** $Id: dropsite.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Drop site example implementation
**
** Created : 979899
**
** Copyright (C) 1997-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "dropsite.h"
#include "secret.h"
#include <qevent.h>
#include <qpixmap.h>
#include <qdragobject.h>
#include <qimage.h>
#include <qdir.h>


DropSite::DropSite( QWidget * parent, const char * name )
    : QLabel( parent, name )
{
    setAcceptDrops(TRUE);
}


DropSite::~DropSite()
{
    // nothing necessary
}


void DropSite::dragMoveEvent( QDragMoveEvent *e )
{
    // Check if you want the drag at e->pos()...
    // Give the user some feedback - only copy is possible
    e->acceptAction( e->action() == QDropEvent::Copy );
}


void DropSite::dragEnterEvent( QDragEnterEvent *e )
{
    // Check if you want the drag...
    if ( SecretDrag::canDecode( e )
      || QTextDrag::canDecode( e )
      || QImageDrag::canDecode( e )
      || QUriDrag::canDecode( e ) )
    {
	e->accept();
    }


    // Give the user some feedback...
    QString t;
    const char *f;
    for( int i=0; (f=e->format( i )); i++ ) {
	if ( *(f) ) {
	    if ( !t.isEmpty() )
		t += "\n";
	    t += f;
	}
    }
    emit message( t );
    setBackgroundColor(white);
}

void DropSite::dragLeaveEvent( QDragLeaveEvent * )
{
    // Give the user some feedback...
    emit message("");
    setBackgroundColor(lightGray);
}


void DropSite::dropEvent( QDropEvent * e )
{
    setBackgroundColor(lightGray);

    // Try to decode to the data you understand...
    QStrList strings;
    if ( QUriDrag::decode( e, strings ) ) {
	QString m("Full URLs:\n");
	for (const char* u=strings.first(); u; u=strings.next())
	    m = m + "   " + u + '\n';
	QStringList files;
	if ( QUriDrag::decodeLocalFiles( e, files ) ) {
	    m += "Files:\n";
	    for (QStringList::Iterator i=files.begin(); i!=files.end(); ++i)
		m = m + "   " + QDir::convertSeparators(*i) + '\n';
	}
	setText( m );
	setMinimumSize( minimumSize().expandedTo( sizeHint() ) );
	return;
    }

    QString str;
    if ( QTextDrag::decode( e, str ) ) {
	setText( str );
	setMinimumSize( minimumSize().expandedTo( sizeHint() ) );
	return;
    }

    QPixmap pm;
    if ( QImageDrag::decode( e, pm ) ) {
	setPixmap( pm );
	setMinimumSize( minimumSize().expandedTo( sizeHint() ) );
	return;
    }

    if ( SecretDrag::decode( e, str ) ) {
	setText( str );
	setMinimumSize( minimumSize().expandedTo( sizeHint() ) );
	return;
    }
}

DragMoviePlayer::DragMoviePlayer( QDragObject* p ) :
    QObject(p),
    dobj(p),
    movie("trolltech.gif" )
{
    movie.connectUpdate(this,SLOT(updatePixmap(const QRect&)));
}

void DragMoviePlayer::updatePixmap( const QRect& )
{
    dobj->setPixmap(movie.framePixmap());
}

void DropSite::mousePressEvent( QMouseEvent * /*e*/ )
{
    QDragObject *drobj;
    if ( pixmap() ) {
	drobj = new QImageDrag( pixmap()->convertToImage(), this );
#if 1
	QPixmap pm;
	pm.convertFromImage(pixmap()->convertToImage().smoothScale(
	    pixmap()->width()/3,pixmap()->height()/3));
	drobj->setPixmap(pm,QPoint(-5,-7));
#else
	// Try it.
	(void)new DragMoviePlayer(drobj);
#endif
    } else {
	drobj = new QTextDrag( text(), this );
    }
    drobj->dragCopy();
}


void DropSite::backgroundColorChange( const QColor & )
{
    // Reduce flicker by using repaint() rather than update()
    repaint();
}
