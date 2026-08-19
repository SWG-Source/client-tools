/****************************************************************************
** $Id: qclipboard_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** Implementation of QClipboard class for Windows
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2004 Ralf Habacker
** Copyright (C) 2004 Tom and Timi Cecka
** Copyright (C) 2004 Christian Ehrlicher
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact kde-cygwin@kde.org if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qclipboard.h"

#ifndef QT_NO_CLIPBOARD

//#define QT_CLIPBOARD_DEBUG
#include "qapplication.h"
#include "qbitmap.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbuffer.h"
#include "qt_windows.h"

static QWidget * owner = 0;
HWND hwndNextViewer = 0;
bool clipboardHasChanged = false;

static void cleanup()
{
    ChangeClipboardChain( owner->winId(), hwndNextViewer );
    delete owner;
    owner = 0;
}

static void setupOwner()
{
    if ( owner )
        return ;
    owner = new QWidget( 0, "internal clipboard owner" );
    /* We want to receive WM_DRAWCLIPBOARD - messages to know when
       Clipboard data has changed.
       This happens in qapplication_win.cpp */
    hwndNextViewer = SetClipboardViewer( owner->winId() );
    qAddPostRoutine( cleanup );
}

class QClipboardWatcher : public QMimeSource
{
public:
    QClipboardWatcher( );
    const char* format( int n ) const;
    QByteArray encodedData( const char* fmt ) const;
};

QClipboard::~QClipboard()
{}

class QClipboardData
{
public:
    QClipboardData();
    ~QClipboardData();

    void setSource( QMimeSource* s )
    {
        clear( TRUE );
        src = s;
    }

    QMimeSource *source() const
    {
        return src;
    }
    void clear( bool destruct = TRUE );

private:
    QMimeSource *src;
};

QClipboardData::QClipboardData()
{
    src = 0;
}

QClipboardData::~QClipboardData()
{
    clear();
}

void QClipboardData::clear( bool destruct )
{
    QMimeSource * s2 = src;
    src = NULL;
    if ( destruct )
        delete s2;
}

static QClipboardData *internalCbData = 0;
static void cleanupClipboardData()
{
    delete internalCbData;
    internalCbData = 0;
}

static QClipboardData *clipboardData()
{
    if ( internalCbData == 0 ) {
        internalCbData = new QClipboardData;
        CHECK_PTR( internalCbData );
        qAddPostRoutine( cleanupClipboardData );
    }
    return internalCbData;
}


void qt_clipboard_cleanup_mime_source( QMimeSource *src )
{
    if ( internalCbData && internalCbData->source() == src )
        internalCbData->clear( FALSE );
}

QClipboardWatcher::QClipboardWatcher( )
{
    setupOwner();
}

const char* QClipboardWatcher::format( int n ) const
{
#ifdef QT_CLIPBOARD_DEBUG
    qDebug( "qclipboard_win.cpp: qclipboardwatcher::format ( %d )", n );
#endif

    if ( n >= 0 ) {
        QPtrList<QWindowsMime> ptrlst = QWindowsMime::all();
        if ( ( unsigned int ) n < ptrlst.count() ) {
            return ptrlst.at( n ) ->mimeFor( ptrlst.at( n ) ->cf( 0 ) );
        }
    }
    return 0;
}

QByteArray QClipboardWatcher::encodedData( const char* mime ) const
{
#ifdef QT_CLIPBOARD_DEBUG
    qDebug( "qclipboard_win.cpp: qclipboardwatcher::encodedData ( %s )", mime );
#endif

    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for ( QWindowsMime * c = all.first(); c; c = all.next() ) {
        int cf = c->cfFor( mime );
        if ( cf ) {
            OpenClipboard( owner->winId() );
            HANDLE hclp = GetClipboardData( cf );
            if ( !hclp ) {
                int lasterr = GetLastError();
                /* lasterr == 0 -> no data */
                if ( lasterr )
                    qWarning( "Error GetClipboardData(%d) Error: %d, Mime: %s", cf, lasterr, mime );
                CloseClipboard();
                break;
            }
            LPVOID ptr = GlobalLock( hclp );
            int size = GlobalSize( hclp );
            QByteArray ba, tr;
            if ( ptr && size ) {
                QBuffer buf ( ba );
                buf.open ( IO_WriteOnly );
                buf.writeBlock ( ( char * ) ptr, size );
                buf.close();
                tr = c->convertToMime( ba, mime, cf );
                GlobalUnlock( hclp );
            }
            CloseClipboard();
            return tr;
        }
    }
    return QByteArray();
}

/*****************************************************************************
  QClipboard member functions for Windows.
 *****************************************************************************/

void QClipboard::clear( Mode mode )
{
    if ( mode != Clipboard )
        return ;
    EmptyClipboard();
}

void QClipboard::ownerDestroyed()
{
#ifdef QT_CLIPBOARD_DEBUG
    qDebug( "qclipboard_win.cpp: ownerDestroyed ( )" );
#endif

    owner = NULL;
    clipboardData() ->clear();
    clipboardHasChanged = true;
    emit dataChanged();
}

static int clipWatcherId = -1;

void QClipboard::connectNotify( const char *signal )
{
#ifdef QT_CLIPBOARD_DEBUG
    qDebug( "qclipboard_win.cpp: connectNotify ( %s )", signal );
#endif

    if ( qstrcmp( signal, SIGNAL( dataChanged() ) ) == 0 && clipWatcherId == -1 )
        clipWatcherId = startTimer( 100 );
}

bool QClipboard::event( QEvent *e )
{
#ifdef QT_CLIPBOARD_DEBUG
    qDebug( "qclipboard_win.cpp: event ( %p )", e );
#endif

    bool check_clip = FALSE;
    if ( e->type() == QEvent::Clipboard ) {
        check_clip = TRUE;
    } else if ( clipWatcherId != -1 && e->type() == QEvent::Timer ) {
        QTimerEvent * te = ( QTimerEvent * ) e;
        if ( te->timerId() == clipWatcherId ) {
            if ( !receivers( SIGNAL( dataChanged() ) ) ) {
                killTimer( clipWatcherId );
                clipWatcherId = -1;
            } else {
                check_clip = TRUE;
            }
        }
    }
    if ( check_clip && clipboardHasChanged ) {
        clipboardData()->clear();
        emit dataChanged();
        clipboardHasChanged = false;
    }
    if( !ownsClipboard() ) {
        clipboardData()->clear();
    }
    return QObject::event( e );
}


QMimeSource* QClipboard::data( Mode mode ) const
{
#ifdef QT_CLIPBOARD_DEBUG
    qDebug( "qclipboard_win.cpp: data ( %d )", mode );
#endif

    if ( mode != Clipboard )
        return 0;

    QClipboardData *d = clipboardData();
    if ( !d->source() )
        d->setSource( new QClipboardWatcher() );
    return d->source();
}

void QClipboard::setData( QMimeSource* src, Mode mode )
{
#ifdef QT_CLIPBOARD_DEBUG
    qDebug( "qclipboard_win.cpp: setData ( %p, %d )", src, mode );
#endif

    if ( mode != Clipboard )
        return ;

    QClipboardData *d = clipboardData();
    d->setSource( src );

    QPtrList<QWindowsMime> all = QWindowsMime::all();
    const char* mime;
    for ( QWindowsMime * c = all.first(); c; c = all.next() ) {
        for ( int i = 0; ( mime = src->format( i ) ); i++ ) {
            if ( c->cfFor( mime ) ) {
                for ( int j = 0; j < c->countCf(); j++ ) {
                    uint cf = c->cf( j );
                    if ( c->canConvert( mime, cf ) ) {
                        if ( !OpenClipboard( 0L ) )
                            return ;
                        EmptyClipboard();
                        QByteArray ba = c->convertFromMime( src->encodedData( mime ), mime, cf );
                        HANDLE hGlobal = GlobalAlloc( GMEM_MOVEABLE, ba.size() );
                        if ( !hGlobal ) {
                            CloseClipboard();
                            return ;
                        }
                        LPVOID ptr = GlobalLock( hGlobal );
                        memcpy( ptr, ba.data(), ba.size() );
                        GlobalUnlock( hGlobal );
                        SetClipboardData( cf, hGlobal );
                        emit dataChanged();
                        setupOwner();  /* need it here, but it's ugly! */
                        CloseClipboard();
                        return ;
                    }
                }
            }
        }
    }
}

void QClipboard::setSelectionMode( bool )
{}

bool QClipboard::selectionModeEnabled() const
{
    return FALSE;
}

bool QClipboard::supportsSelection() const
{
    return false;
}

bool QClipboard::ownsSelection() const
{
    return false;
}

bool QClipboard::ownsClipboard() const
{
    if ( ( owner ) && ( GetClipboardOwner() == owner->winId() ) )
        return true;
    return false;
}

#endif // QT_NO_CLIPBOARD
