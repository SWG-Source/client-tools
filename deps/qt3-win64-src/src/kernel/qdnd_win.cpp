/****************************************************************************
** $Id: qdnd_win.cpp 1964 2006-12-15 12:14:12Z chehrlic $
**
** DND implementation for Qt.
**
** Created : 20030119
**
** Copyright (C) 2003 Holger Schroeder
** Copyright (C) 2003 Richard Lärkäng
** Copyright (C) 2004 Tom and Timi Cecka
** Copyright (C) 2004,2005 Christian Ehrlicher
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
****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP

//#define DEBUG_QDND_WIN
//#define DEBUG_QDND_SRC
//#define DEBUG_QDND_TGT

#include <shlobj.h>
#include "qapplication_p.h"
#include "qbitmap.h"
#include "qbuffer.h"
#include "qcursor.h"
#include "qdragobject.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qt_windows.h"
#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

// qcursor_win.cpp
HCURSOR qt_createPixmapCursor( HDC hdc, const QPixmap &pm, const QBitmap &bm, const QPoint &hs, const QSize &size );
// qdragobject.cpp
extern QDragManager *qt_dnd_manager;

static IDataObject *pIDataObject = NULL;

static QDragObject *global_src = 0;
static QDragObject::DragMode drag_mode;

// cursor handling
// current cursors
static QCursor *noDropCursor = 0;
static QCursor *moveCursor = 0;
static QCursor *copyCursor = 0;
static QCursor *linkCursor = 0;
// standard cursors
static QCursor *stdNoDropCursor = 0;
static QCursor *stdMoveCursor = 0;
static QCursor *stdCopyCursor = 0;
static QCursor *stdLinkCursor = 0;
// true, if pixmap cursors used
static bool usePixmapCursors = false;

static QWidget *src = NULL;

QDropEvent::Action translateKeyStateToQDropAction ( DWORD grfKeyState, DWORD dwAllowedEffects )
{
    grfKeyState &= ( ~( MK_LBUTTON | MK_RBUTTON ) );
    if ( ( ( grfKeyState == 0 ) || grfKeyState == MK_CONTROL ) && ( dwAllowedEffects & DROPEFFECT_COPY ) )
        return QDropEvent::Copy;
    if ( ( grfKeyState == MK_SHIFT ) && ( dwAllowedEffects & DROPEFFECT_MOVE ) )
        return QDropEvent::Move;
    if ( grfKeyState == ( MK_CONTROL | MK_SHIFT ) && ( dwAllowedEffects & DROPEFFECT_LINK ) )
        return QDropEvent::Link;
    return QDropEvent::Private;
}

QDropEvent::Action translateToQDropAction( DWORD pdwEffect )
{
    if ( pdwEffect & DROPEFFECT_LINK )
        return QDropEvent::Link;
    if ( pdwEffect & DROPEFFECT_COPY )
        return QDropEvent::Copy;
    if ( pdwEffect & DROPEFFECT_MOVE )
        return QDropEvent::Move;
    return QDropEvent::Private;
}

DWORD translateToWinDragEffects( QDropEvent::Action action )
{
    if ( action == QDropEvent::Link )
        return DROPEFFECT_LINK;
    if ( action == QDropEvent::Copy )
        return DROPEFFECT_COPY;
    if ( action == QDropEvent::Move )
        return DROPEFFECT_MOVE;
    return DROPEFFECT_NONE;
}

DWORD translateToWinDragEffects( QDragObject::DragMode mode )
{
    if ( mode == QDragObject::DragDefault )
        return DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;
    if ( mode == QDragObject::DragCopy )
        return DROPEFFECT_COPY;
    if ( mode == QDragObject::DragMove )
        return DROPEFFECT_MOVE;
    if ( mode == QDragObject::DragLink )
        return DROPEFFECT_LINK;
    if ( mode == QDragObject::DragCopyOrMove )
        return DROPEFFECT_COPY | DROPEFFECT_MOVE;
    return DROPEFFECT_NONE;
}

/***********************************************
        QOleDropSource
************************************************/
class QOleDropSource : public IDropSource
{
public:
    QOleDropSource();
    virtual ~QOleDropSource();

    /* IDropSource methods */
    STDMETHOD( QueryContinueDrag ) ( BOOL fEscapePressed, DWORD grfKeyState );
    STDMETHOD( GiveFeedback ) ( DWORD dwEffect );

    /* IDropUnknown methods */
    STDMETHOD( QueryInterface ) ( REFIID iid, void** ppv );
    STDMETHOD_( ULONG, AddRef ) ();
    STDMETHOD_( ULONG, Release ) ();

protected:
    ULONG m_refs;
    QDropEvent::Action currentAction;

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=

    QOleDropSource( const QOleDropSource& );
    QOleDropSource& operator=( const QOleDropSource& );
#endif
};

QOleDropSource::QOleDropSource()
{
    m_refs = 1;
    currentAction = QDropEvent::Private;
}

QOleDropSource::~QOleDropSource()
{}

//---------------------------------------------------------------------
//                    IDropSource Methods
//---------------------------------------------------------------------
STDMETHODIMP QOleDropSource::QueryContinueDrag( BOOL fEscapePressed, DWORD grfKeyState )
{
#ifdef DEBUG_QDND_SRC
    qDebug( "QOleDropSource::QueryContinueDrag( fEscapePressed=%d, grfKeyState=%d )", fEscapePressed, grfKeyState );
#endif

    if ( fEscapePressed ) {
        return ResultFromScode( DRAGDROP_S_CANCEL );
    } else if ( !( grfKeyState & ( MK_LBUTTON | MK_MBUTTON | MK_RBUTTON ) ) ) {
        return ResultFromScode( DRAGDROP_S_DROP );
    } else {
        qApp->processEvents();
        return NOERROR;
    }
}

STDMETHODIMP QOleDropSource::GiveFeedback( DWORD dwEffect )
{
    QCursor * cur;
    QDropEvent::Action action = translateToQDropAction( dwEffect );

#ifdef DEBUG_QDND_SRC

    qDebug( "QOleDropSource::GiveFeedback(dwEffect = %d)", action );
#endif

    if ( currentAction != action ) {
        currentAction = action;
    }

    if ( ( ( dwEffect & DROPEFFECT_COPY ) == DROPEFFECT_COPY ) &&
            ( ( drag_mode == QDragObject::DragCopy ) ||
              ( drag_mode == QDragObject::DragCopyOrMove ) ||
              ( drag_mode == QDragObject::DragDefault ) ) ) {
        cur = copyCursor;
    } else if ( ( ( dwEffect & DROPEFFECT_MOVE ) == DROPEFFECT_MOVE ) &&
                ( ( drag_mode == QDragObject::DragMove ) ||
                  ( drag_mode == QDragObject::DragCopy ) ||      // windows-bug?
                  ( drag_mode == QDragObject::DragCopyOrMove ) ||
                  ( drag_mode == QDragObject::DragDefault ) ) ) {
        cur = moveCursor;
    } else if ( ( ( dwEffect & DROPEFFECT_LINK ) == DROPEFFECT_LINK ) &&
                ( ( drag_mode == QDragObject::DragLink ) ||
                  ( drag_mode == QDragObject::DragDefault ) ) ) {
        cur = linkCursor;
    } else {
        return ResultFromScode( DRAGDROP_S_USEDEFAULTCURSORS );
    }
    SetCursor( cur->handle() );
#ifndef QT_NO_CURSOR

    qApp->setOverrideCursor( *cur, TRUE );
#endif

    return ResultFromScode( S_OK );
}

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------
STDMETHODIMP QOleDropSource::QueryInterface( REFIID iid, void** ppv )
{
    if ( iid == IID_IUnknown || iid == IID_IDropSource ) {
        *ppv = this;
        ++m_refs;
        return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode( E_NOINTERFACE );
}

STDMETHODIMP_( ULONG ) QOleDropSource::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_( ULONG )
QOleDropSource::Release()
{
    if ( --m_refs == 0 ) {
        delete this;
        return 0;
    }
    return m_refs;
}

/***********************************************
        QOleDropTarget
************************************************/
class QOleDropTarget : public IDropTarget
{
public:
    QOleDropTarget( QWidget *widget );

    /* own methods */
    QWidget *getWidget()
    {
        return widget;
    }
    void releaseQt()
    {
        widget = 0;
    }

    /* IDropTarget methods */
    STDMETHOD( DragEnter ) ( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
    STDMETHOD( DragOver ) ( DWORD, POINTL, LPDWORD );
    STDMETHOD( DragLeave() );
    STDMETHOD( Drop ) ( LPDATAOBJECT, DWORD, POINTL, LPDWORD );

    /* IDropUnknown methods */
    STDMETHOD( QueryInterface ) ( REFIID iid, void** ppv );
    STDMETHOD_( ULONG, AddRef ) ();
    STDMETHOD_( ULONG, Release ) ();

protected:
    ULONG m_refs;
    QWidget* widget;
    QRect answerRect;
    QPoint lastPoint;
    DWORD choosenEffect;
    DWORD lastKeyState;
private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=

    QOleDropTarget( const QOleDropTarget& );
    QOleDropTarget& operator=( const QOleDropTarget& );
#endif
};

//---------------------------------------------------------------------
//                    QOleDropTarget
//---------------------------------------------------------------------
QOleDropTarget::QOleDropTarget( QWidget *w )
        : widget( w )
{
    m_refs = 1;
}

//---------------------------------------------------------------------
//                    IDropTarget Methods
//---------------------------------------------------------------------

STDMETHODIMP QOleDropTarget::DragEnter( LPDATAOBJECT pIDataSource,
                                        DWORD grfKeyState,
                                        POINTL pt,
                                        LPDWORD pdwEffect )
{
#ifdef DEBUG_QDND_TGT
    qDebug( "QOleDropTarget::DragEnter( %p, %d, %d/%d, %d )",
            pIDataSource, grfKeyState, pt.x, pt.y, *pdwEffect );
    qDebug( "widget: %p, winID: %08x", widget, widget ? widget->winId() : 0 );
#endif

    if ( !qt_tryModalHelper( widget ) ) {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    pIDataObject = pIDataSource;
    pIDataObject->AddRef();

    lastPoint = widget->mapFromGlobal( QPoint( pt.x, pt.y ) );
    lastKeyState = grfKeyState;

    QDragEnterEvent e( lastPoint );
    e.setAction( translateKeyStateToQDropAction( grfKeyState, *pdwEffect ) );
    e.accept();
    QApplication::sendEvent( widget, &e );

    QDragResponseEvent re ( e.isAccepted() );
    QApplication::sendEvent( widget, &re );

    answerRect = e.answerRect();
    if ( e.isAccepted() )
        choosenEffect = translateToWinDragEffects( e.action() );
    else
        choosenEffect = DROPEFFECT_NONE;
    *pdwEffect = choosenEffect;

    return NOERROR;
}

STDMETHODIMP QOleDropTarget::DragOver( DWORD grfKeyState,
                                       POINTL pt,
                                       LPDWORD pdwEffect )
{
#ifdef DEBUG_QDND_TGT
    qDebug( "QOleDropTarget::DragOver( %d, %d/%d, %d )",
            grfKeyState, pt.x, pt.y, *pdwEffect );
    qDebug( "widget: %p, winID: %08x", widget, widget ? widget->winId() : 0 );
#endif

    if ( !qt_tryModalHelper( widget ) ) {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    QPoint tmpPoint = widget->mapFromGlobal( QPoint( pt.x, pt.y ) );
    // see if we should compress this event
    if ( ( tmpPoint == lastPoint || answerRect.contains( tmpPoint ) ) && lastKeyState == grfKeyState ) {
        *pdwEffect = choosenEffect;
        return NOERROR;
    }

    lastPoint = tmpPoint;
    lastKeyState = grfKeyState;

    QDragMoveEvent e( lastPoint );
    e.setAction( translateKeyStateToQDropAction( grfKeyState, *pdwEffect ) );
    e.accept();
    QApplication::sendEvent( widget, &e );

    answerRect = e.answerRect();
    if ( e.isAccepted() )
        choosenEffect = translateToWinDragEffects( e.action() );
    else
        choosenEffect = DROPEFFECT_NONE;
    *pdwEffect = choosenEffect;

    return NOERROR;
}

STDMETHODIMP QOleDropTarget::DragLeave()
{
#ifdef DEBUG_QDND_TGT
    qDebug( "QOleDropTarget::DragLeave()" );
    qDebug( "widget: %p, winID: %08x", widget, widget ? widget->winId() : 0 );
#endif

    if ( !qt_tryModalHelper( widget ) ) {
        return NOERROR;
    }

    QDragLeaveEvent e;
    QApplication::sendEvent( widget, &e );

    pIDataObject->Release();
    pIDataObject = NULL;

    return NOERROR;
}

#define KEY_STATE_BUTTON_MASK ( MK_LBUTTON | MK_MBUTTON | MK_RBUTTON )

STDMETHODIMP QOleDropTarget::Drop( LPDATAOBJECT pIDataSource,
                                   DWORD grfKeyState,
                                   POINTL pt,
                                   LPDWORD pdwEffect )
{
#ifdef DEBUG_QDND_TGT
    qDebug( "QOleDropTarget::Drop( %p, %d, %d/%d, %d )",
            pIDataSource, grfKeyState, pt.x, pt.y, *pdwEffect );
    qDebug( "widget: %p, winID: %08x", widget, widget ? widget->winId() : 0 );
#endif

    if ( !qt_tryModalHelper( widget ) ) {
        *pdwEffect = DROPEFFECT_NONE;
        return NOERROR;
    }

    lastPoint = widget->mapFromGlobal( QPoint( pt.x, pt.y ) );
    // grfKeyState does not all ways contain button state in the drop so if
    // it doesn't then use the last known button state;
    if ( ( grfKeyState & KEY_STATE_BUTTON_MASK ) == 0 )
        grfKeyState |= lastKeyState & KEY_STATE_BUTTON_MASK;
    lastKeyState = grfKeyState;

    FORMATETC s_fmtMemory = { 0,
                              NULL,
                              DVASPECT_CONTENT,
                              -1,
                              TYMED_HGLOBAL };

    /* Can we convert the data? */
    bool found = false;
    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for ( QWindowsMime * c = all.first(); c && !found; c = all.next() ) {
        for ( int i = 0; i < c->countCf(); i++ ) {
            int cf = c->cf( i );
            s_fmtMemory.cfFormat = cf;
            if ( pIDataSource->QueryGetData( &s_fmtMemory ) == S_OK ) {
                found = true;
                break;
            }

        }
    }
    if ( !found ) {
        return S_OK;
    }
    /* Just to be sure */
    pIDataObject = pIDataSource;

    pIDataObject->AddRef();
    QDropEvent de( lastPoint );
    de.setAction( translateKeyStateToQDropAction( grfKeyState, *pdwEffect ) );
    if ( global_src )
        global_src->setTarget( widget );
    QApplication::sendEvent( widget, &de );

    pIDataObject->Release();
    pIDataObject = NULL;

    return S_OK;
}

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------
STDMETHODIMP QOleDropTarget::QueryInterface( REFIID iid, void** ppv )
{
    if ( iid == IID_IUnknown || iid == IID_IDropTarget ) {
        *ppv = this;
        AddRef();
        return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode( E_NOINTERFACE );
}

STDMETHODIMP_( ULONG )
QOleDropTarget::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_( ULONG )
QOleDropTarget::Release()
{
    if ( --m_refs == 0 ) {
        delete this;
        return 0;
    }
    return m_refs;
}

/***********************************************
        qIEnumFORMATETC
************************************************/
class qIEnumFORMATETC : public IEnumFORMATETC
{
public:
    qIEnumFORMATETC( int* formats, ULONG nCount );
    virtual ~qIEnumFORMATETC()
    {
        delete [] m_formats;
    }

    /* IEnumFORMATETC */
    STDMETHODIMP Next( ULONG celt, FORMATETC *rgelt, ULONG *pceltFetched );
    STDMETHODIMP Skip( ULONG celt );
    STDMETHODIMP Reset();
    STDMETHODIMP Clone( IEnumFORMATETC **ppenum );

    /* IUnknown methods */
    STDMETHODIMP QueryInterface( REFIID iid, void** ppv );
    STDMETHOD_( ULONG, AddRef ) ();
    STDMETHOD_( ULONG, Release ) ();

private:
    int *m_formats;  // formats we can provide data in
    ULONG m_nCount,    // number of formats we support
    m_nCurrent;      // current enum position
    /* IUnknown vars */
    ulong m_refs;

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=

    qIEnumFORMATETC( const qIEnumFORMATETC& );
    qIEnumFORMATETC& operator=( const qIEnumFORMATETC& );
#endif
};

qIEnumFORMATETC::qIEnumFORMATETC( int* formats, ULONG nCount )
{
    m_refs = 1;
    m_nCurrent = 0;
    m_nCount = nCount;
    m_formats = new int[ nCount ];
    for ( ULONG n = 0; n < nCount; n++ ) {
        m_formats[ n ] = formats[ n ];
    }
}
STDMETHODIMP qIEnumFORMATETC::Next( ULONG celt,
                                    FORMATETC *rgelt,
                                    ULONG *pceltFetched )
{
    ULONG numFetched = 0;
    while ( m_nCurrent < m_nCount && numFetched < celt ) {
        FORMATETC format;
        format.cfFormat = m_formats[ m_nCurrent++ ];
        format.ptd = NULL;
        format.dwAspect = DVASPECT_CONTENT;
        format.lindex = -1;
        format.tymed = TYMED_HGLOBAL;

        *rgelt++ = format;
        numFetched++;
    }

    if ( pceltFetched )
        * pceltFetched = numFetched;

    return numFetched == celt ? S_OK : S_FALSE;
}

STDMETHODIMP qIEnumFORMATETC::Skip( ULONG celt )
{
    m_nCurrent += celt;
    if ( m_nCurrent < m_nCount )
        return S_OK;

    // no, can't skip this many elements
    m_nCurrent -= celt;

    return S_FALSE;
}

STDMETHODIMP qIEnumFORMATETC::Reset()
{
    m_nCurrent = 0;
    return S_OK;
}

STDMETHODIMP qIEnumFORMATETC::Clone( IEnumFORMATETC **ppenum )
{
    qIEnumFORMATETC * pNew = new qIEnumFORMATETC( NULL, 0 );
    pNew->m_nCount = m_nCount;
    pNew->m_formats = new int[ m_nCount ];
    for ( ULONG n = 0; n < m_nCount; n++ ) {
        pNew->m_formats[ n ] = m_formats[ n ];
    }
    pNew->AddRef();
    *ppenum = pNew;

    return S_OK;
}

STDMETHODIMP qIEnumFORMATETC::QueryInterface( REFIID iid, void** ppv )
{
    if ( iid == IID_IUnknown || iid == IID_IDropTarget ) {
        *ppv = this;
        AddRef();
        return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode( E_NOINTERFACE );
}

STDMETHODIMP_( ULONG )
qIEnumFORMATETC::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_( ULONG )
qIEnumFORMATETC::Release()
{
    if ( --m_refs == 0 ) {
        delete this;
        return 0;
    }
    return m_refs;
}

/***********************************************
        QOleDataObject
************************************************/
class QOleDataObject : public IDataObject
{
public:
    QOleDataObject( QDragObject * o );
    virtual ~QOleDataObject();

    /* own methods */
    QWidget *getWidget();
    void releaseQt()
    {
        m_dragObj = 0;
    }

    /* IDataObject methods */
    STDMETHODIMP GetData( FORMATETC *pformatetcIn, STGMEDIUM *pmedium );
    STDMETHODIMP GetDataHere( FORMATETC *pformatetc, STGMEDIUM *pmedium );
    STDMETHODIMP QueryGetData( FORMATETC *pformatetc );
    STDMETHODIMP GetCanonicalFormatEtc( FORMATETC *, FORMATETC *pOut );
    STDMETHODIMP SetData( FORMATETC *pfetc, STGMEDIUM *pmedium, BOOL fRelease );
    STDMETHODIMP EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC **ppenumFEtc );
    STDMETHODIMP DAdvise( FORMATETC *, DWORD , IAdviseSink *, DWORD * );
    STDMETHODIMP DUnadvise( DWORD );
    STDMETHODIMP EnumDAdvise( IEnumSTATDATA ** );
    /* IDropUnknown methods */
    STDMETHODIMP QueryInterface( REFIID iid, void** ppv );
    STDMETHOD_( ULONG, AddRef ) ();
    STDMETHOD_( ULONG, Release ) ();

protected:
    ulong m_refs;
    QDragObject * m_dragObj;
private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=

    QOleDataObject ( const QOleDataObject & );
    QOleDataObject & operator=( const QOleDataObject & );
#endif
};

QOleDataObject::QOleDataObject( QDragObject * o )
{
    m_refs = 1;
    m_dragObj = o;
}

QOleDataObject::~QOleDataObject()
{}

STDMETHODIMP QOleDataObject::GetData( FORMATETC *pformatetcIn, STGMEDIUM *pmedium )
{
    // is data is in our format?
    HRESULT hr = QueryGetData( pformatetcIn );
    if ( hr != S_OK )
        return hr;

    int cf = pformatetcIn->cfFormat;
    pmedium->tymed = TYMED_HGLOBAL;

    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for ( QWindowsMime * c = all.first(); c ; c = all.next() ) {
        const char * mime = c->mimeFor( cf );
        if ( mime && m_dragObj->provides( mime ) ) {
            QByteArray ba = m_dragObj->encodedData( mime );

            ba = c->convertFromMime( ba, mime, cf );
            HGLOBAL hGlobal = GlobalAlloc ( GMEM_MOVEABLE | GMEM_SHARE, ba.size() );
            if ( !hGlobal )
                return E_OUTOFMEMORY;

            memcpy ( GlobalLock ( hGlobal ), ba.data(), ba.size() );
            GlobalUnlock ( hGlobal );
            pmedium->hGlobal = hGlobal;
            pmedium->pUnkForRelease = NULL;
            return S_OK;
        }
    }
    return E_UNEXPECTED;
}

STDMETHODIMP QOleDataObject::GetDataHere( FORMATETC *pformatetc, STGMEDIUM *pmedium )
{
    // is data is in our format?
    HRESULT hr = QueryGetData( pformatetc );
    if ( hr != S_OK )
        return hr;

    if ( pmedium->tymed != TYMED_HGLOBAL )
        return DV_E_TYMED;

    if ( !pmedium->hGlobal )
        return STG_E_MEDIUMFULL;

    HGLOBAL hGlobal = pmedium->hGlobal;
    uint size = GlobalSize( hGlobal );

    int cf = pformatetc->cfFormat;
    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for ( QWindowsMime * c = all.first(); c ; c = all.next() ) {
        const char * mime = c->mimeFor( cf );
        if ( mime && m_dragObj->provides( mime ) ) {
            QByteArray ba = m_dragObj->encodedData( mime );
            if ( ba.size() > size )
                return STG_E_MEDIUMFULL;
            memcpy ( GlobalLock ( hGlobal ), ba.data(), ba.size() );
            GlobalUnlock ( hGlobal );
            return S_OK;
        }
    }
    return E_UNEXPECTED;
}

STDMETHODIMP QOleDataObject::QueryGetData( FORMATETC *pformatetc )
{
#ifdef DEBUG_QDND_SRC
    qDebug( "QOleDataObject::QueryGetData( %p )", pformatetc );
#endif

    if ( !pformatetc ) {
        return E_INVALIDARG;
    }
    if ( pformatetc->lindex != -1 ) {
        return DV_E_LINDEX;
    }
    if ( pformatetc->dwAspect != DVASPECT_CONTENT ) {
        return DV_E_DVASPECT;
    }
    int tymed = pformatetc->tymed;
    /* Currently we only support HGLOBAL */
    if ( !( tymed & TYMED_HGLOBAL ) ) {
        return DV_E_TYMED;
    }

    int cf = pformatetc->cfFormat;
    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for ( QWindowsMime * c = all.first(); c ; c = all.next() ) {
        const char * mime = c->mimeFor( cf );
        if ( mime && m_dragObj->provides( mime ) ) {
            return S_OK;
        }
    }
    return DV_E_FORMATETC;
}

STDMETHODIMP QOleDataObject::GetCanonicalFormatEtc( FORMATETC *, FORMATETC *pformatetcOut )
{
    pformatetcOut->ptd = NULL;
    return ResultFromScode( E_NOTIMPL );
}

STDMETHODIMP QOleDataObject::SetData( FORMATETC *pfetc, STGMEDIUM *pmedium, BOOL fRelease )
{
    return ResultFromScode( E_NOTIMPL );
}

STDMETHODIMP QOleDataObject::EnumFormatEtc( DWORD dwDir, IEnumFORMATETC **ppenumFormatEtc )
{

    if ( dwDir == DATADIR_SET )
        return E_NOTIMPL;

    int count = 0;

    while ( m_dragObj->format( count ) )
        count++;

    int *formats = new int[ count ];
    for ( int i = 0; i < count; i++ ) {
        const char *mime = m_dragObj->format( i );
        QPtrList<QWindowsMime> all = QWindowsMime::all();
        for ( QWindowsMime * c = all.first(); c ; c = all.next() ) {
            int cf = c->cfFor( mime );
            if ( cf ) {
                formats[ i ] = cf;
                break;
            }
        }
    }

    qIEnumFORMATETC *pEnum = new qIEnumFORMATETC( formats, count );
    pEnum->AddRef();
    *ppenumFormatEtc = pEnum;

    delete[] formats;

    return ResultFromScode( S_OK );
}

STDMETHODIMP QOleDataObject::DAdvise( FORMATETC*, DWORD, IAdviseSink *, DWORD* )
{
    return ResultFromScode( OLE_E_ADVISENOTSUPPORTED );
}

STDMETHODIMP QOleDataObject::DUnadvise( DWORD )
{
    return ResultFromScode( OLE_E_ADVISENOTSUPPORTED );
}

STDMETHODIMP QOleDataObject::EnumDAdvise( IEnumSTATDATA ** )
{
    return ResultFromScode( OLE_E_ADVISENOTSUPPORTED );
}

STDMETHODIMP QOleDataObject::QueryInterface( REFIID iid, void** ppv )
{
    if ( iid == IID_IUnknown || iid == IID_IDataObject ) {
        *ppv = this;
        AddRef();
        return NOERROR;
    }
    *ppv = NULL;
    return ResultFromScode( E_NOINTERFACE );
}

STDMETHODIMP_( ULONG )
QOleDataObject::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_( ULONG )
QOleDataObject::Release()
{
    if ( --m_refs == 0 ) {
        releaseQt();
        delete this;
        return 0;
    }
    return m_refs;
}

/***********************************************
        QDragManager
************************************************/
void QDragManager::timerEvent( QTimerEvent* e )
{
    // not used in windows implementation
}

bool QDragManager::eventFilter( QObject * o, QEvent * e )
{
    // not used in windows implementation
    return false;
}

void QDragManager::updateMode( ButtonState newstate )
{
#ifdef DEBUG_QDND_WIN
    qDebug( "TODO QDragManager::updateMode( %d )", newstate );
#endif

}


void QDragManager::updateCursor()
{
#ifdef DEBUG_QDND_WIN
    qDebug( "QDragManager::updateCursor()" );
#endif

    if ( !noDropCursor ) {
        stdNoDropCursor = new QCursor( ForbiddenCursor );
        if ( !pm_cursor[ 0 ].isNull() )
            stdMoveCursor = new QCursor( pm_cursor[ 0 ], 0, 0 );
        if ( !pm_cursor[ 1 ].isNull() )
            stdCopyCursor = new QCursor( pm_cursor[ 1 ], 0, 0 );
        if ( !pm_cursor[ 2 ].isNull() )
            stdLinkCursor = new QCursor( pm_cursor[ 2 ], 0, 0 );
    }

    if ( !object->pixmap().isNull() ) {
        updatePixmap();
    } else {
        delete noDropCursor;
        delete moveCursor;
        delete copyCursor;
        delete linkCursor;
        if ( usePixmapCursors ) {
            usePixmapCursors = false;
        }
        noDropCursor = new QCursor ( *stdNoDropCursor );
        moveCursor = new QCursor ( *stdMoveCursor );
        copyCursor = new QCursor ( *stdCopyCursor );
        linkCursor = new QCursor ( *stdLinkCursor );
    }
}


void QDragManager::cancel( bool /* deleteSource */ )
{
    if ( object ) {
        beingCancelled = true;
        object = 0;
    }

#ifndef QT_NO_CURSOR
    // insert cancel code here ######## todo

    if ( restoreCursor ) {
        QApplication::restoreOverrideCursor();
        restoreCursor = false;
    }
#endif
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility( this, 0, QAccessible::DragDropEnd );
#endif
}

void QDragManager::move( const QPoint & globalPos )
{
    // not used in windows implementation
}

void QDragManager::drop()
{
    // not used in windows implementation
}

bool QDragManager::drag( QDragObject * o, QDragObject::DragMode mode )
{
#ifdef DEBUG_QDND_SRC
    qDebug( "QDragManager::drag ( %p, %d )", o, mode );
#endif

    if ( object == o || !o || !o->parent() )
        return false;

    if ( object ) {
        cancel();
        qApp->removeEventFilter( this );
        beingCancelled = false;
    }

    object = o;
    drag_mode = mode;
    updatePixmap();
    willDrop = FALSE;

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility( this, 0, QAccessible::DragDropStart );
#endif

    DWORD dwEffect, dwOKEffect;

    QOleDataObject *obj = new QOleDataObject( o );
    QOleDropSource *qsrc = new QOleDropSource();
    src = o->target();
    dwOKEffect = translateToWinDragEffects ( mode );

    updateCursor();
#ifdef Q_OS_TEMP
    HRESULT r = 0;
    resultEffect = 0;
#else
    HRESULT r = DoDragDrop( obj, qsrc, dwOKEffect, &dwEffect );
#endif

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
    restoreCursor = false;
#endif

    // clean up
    obj->releaseQt();
    obj->Release();        // Will delete obj if refcount becomes 0
    qsrc->Release();       // Will delete src if refcount becomes 0
    object = 0;

#ifndef QT_NO_ACCESSIBILITY

    QAccessible::updateAccessibility( this, 0, QAccessible::DragDropEnd );
#endif

    return true;
}

static HCURSOR qt_createPixmapCursor( const QCursor &cursor, const QPixmap &pixmap, const QPoint &hotspot )
{
    QSize size;

    /* calculate size of cursor */
    QPoint pt = hotspot - cursor.hotSpot();
    QPoint pixmap_point( 0, 0 );
    QPoint cursor_point( 0, 0 );

    if ( qWinVersion() <= Qt::WV_95 ) {
        /* Win95 can only fixed size */
        size.setWidth( GetSystemMetrics ( SM_CXCURSOR ) );
        size.setHeight ( GetSystemMetrics ( SM_CYCURSOR ) );
    } else {
        /* calculate size (enlarge if needed) */
        const QBitmap *tmp = cursor.bitmap();

        /* Only bitmap cursors allowed... */
        if ( tmp ) {
            QPoint point;

            // curent size
            size = pixmap.size();

            // calc position of lower right cursor pos
            point.setX( pt.x() + tmp->size().width() );
            point.setY( pt.y() + tmp->size().height() );
            // resize when cursor to large
            if ( point.x() > pixmap.width() )
                size.setWidth( point.x() );
            if ( point.y() > pixmap.height() )
                size.setHeight( point.y() );

            // calc upper left corner where both pixmaps have to be drawn
            if ( pt.x() >= 0 ) {
                cursor_point.setX( pt.x() );
            } else {
                pixmap_point.setX( -pt.x() );
                // negative position -> resize
                size.setWidth( size.width() - pt.x() );
            }
            if ( pt.y() >= 0 ) {
                cursor_point.setX( pt.x() );
            } else {
                pixmap_point.setY( -pt.y() );
                // negative position -> resize
                size.setHeight( size.height() - pt.y() );
            }

        }
    }

    /* Mask */
    QBitmap andMask( size );
    BitBlt( andMask.handle(), 0, 0, size.width(), size.height(), NULL, 0, 0, WHITENESS );
    if ( pixmap.mask() )
        BitBlt( andMask.handle(), pixmap_point.x(), pixmap_point.y(), pixmap.width(), pixmap.height(), pixmap.mask() ->handle(), 0, 0, SRCAND );
    else
        BitBlt( andMask.handle(), pixmap_point.x(), pixmap_point.y(), pixmap.width(), pixmap.height(), NULL, 0, 0, BLACKNESS );
    const QBitmap *curMsk = cursor.mask();
    if ( curMsk )
        BitBlt( andMask.handle(), cursor_point.x(), cursor_point.y(), curMsk->width(), curMsk->height(), curMsk->handle(), 0, 0, SRCAND );

    /* create Pixmap */
    QPixmap xorMask( size );
    xorMask.fill ( Qt::color1 );
    QPainter paint2( &xorMask );
    paint2.drawPixmap( pixmap_point, pixmap );

    QPixmap pm;
    pm.convertFromImage( cursor.bitmap() ->convertToImage().convertDepth( 8 ) );
    pm.setMask( *cursor.mask() );
    paint2.drawPixmap ( cursor_point, pm );
    paint2.end();

#ifdef DEBUG_QDND_SRC

    andMask.save ( "pand.png", "PNG" );
    xorMask.save ( "pxor.png", "PNG" );
#endif

    HCURSOR cur = qt_createPixmapCursor ( qt_display_dc(), xorMask, andMask, hotspot, size );

    if ( !cur ) {
        qWarning( "Error creating cursor: %d", GetLastError() );
    }
    return cur;
}

void QDragManager::updatePixmap()
{
#ifdef DEBUG_QDND_SRC
    qDebug( "QDragManager::updatePixmap()" );
#endif

    if ( !stdNoDropCursor || object->pixmap().isNull() )
        return ;

    HCURSOR curMove, curLink, curCopy;
    //    curNoDrop = qt_createPixmapCursor ( *stdNoDropCursor, object->pixmap(), object->pixmapHotSpot() );
    curMove = qt_createPixmapCursor ( *stdMoveCursor, object->pixmap(), object->pixmapHotSpot() );
    curLink = qt_createPixmapCursor ( *stdLinkCursor, object->pixmap(), object->pixmapHotSpot() );
    curCopy = qt_createPixmapCursor ( *stdCopyCursor, object->pixmap(), object->pixmapHotSpot() );

    /* delete old pixmap cursors */
    //        delete noDropCursor;
    delete moveCursor;
    delete linkCursor;
    delete copyCursor;
    //    noDropCursor = new QCursor ( curNoDrop );
    moveCursor = new QCursor ( curMove );
    linkCursor = new QCursor ( curLink );
    copyCursor = new QCursor ( curCopy );
#ifdef DEBUG_QDND_SRC

    qDebug( "move: %p, link: %p, copy: %p", moveCursor, linkCursor, copyCursor );
#endif

    usePixmapCursors = true;
}

/***********************************************
        QDropEvent
************************************************/
QByteArray QDropEvent::encodedData( const char * format ) const
{
#ifdef DEBUG_QDND_WIN
    qDebug( "QDropEvent::encodedData ( %s )", format );
#endif

    QByteArray ba;
    /* Currently we only support TYMED_HGLOBAL ... */
    FORMATETC fmtMemory = { 0,
                            NULL,
                            DVASPECT_CONTENT,
                            -1,
                            TYMED_HGLOBAL };
    STGMEDIUM stm;

    if ( !pIDataObject ) {
        return ba;
    }

    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for ( QWindowsMime * c = all.first(); c ; c = all.next() ) {
        int cf = c->cfFor( format );
        if ( c->canConvert( format, cf ) ) {
            fmtMemory.cfFormat = cf;
            if ( pIDataObject->QueryGetData( &fmtMemory ) == S_OK ) {
                HRESULT hr = pIDataObject->GetData( &fmtMemory, &stm );
                if ( ( hr != S_OK ) || ( !stm.hGlobal ) )
                    continue;
                QByteArray data;
                QBuffer buf ( data );
                buf.open( IO_WriteOnly );
                buf.writeBlock ( ( char* ) GlobalLock( stm.hGlobal ), GlobalSize ( stm.hGlobal ) );
                buf.close();

                GlobalUnlock ( stm.hGlobal );
                ReleaseStgMedium( &stm );

                ba = c->convertToMime( data, format, cf );
                ReleaseStgMedium( &stm );
                return ba;
            }
        }
    }
    return ba;
}

const char* QDropEvent::format( int n ) const
{
#ifdef DEBUG_QDND_WIN
    qDebug( "QDropEvent::format ( %d )", n );
#endif

    if ( !pIDataObject )
        return NULL;

    FORMATETC fmtMemory = { 0,
                            NULL,
                            DVASPECT_CONTENT,
                            -1,
                            TYMED_HGLOBAL };
    if ( n >= 0 ) {
        QPtrList<QWindowsMime> all = QWindowsMime::all();
        for ( QWindowsMime * c = all.first(); c ; c = all.next() ) {
            for ( int i = 0; i < c->countCf(); i++ ) {
                int cf = c->cf( i );
                fmtMemory.cfFormat = cf;
                if ( pIDataObject->QueryGetData( &fmtMemory ) == S_OK ) {
                    if ( n == 0 ) {
#ifdef DEBUG_QDND_WIN
                        qDebug( "format: %s", c->mimeFor( cf ) );
#endif

                        return c->mimeFor( cf );
                    }
                    n--;
                }

            }
        }
    }
    return NULL;
}

bool QDropEvent::provides( const char * mimeType ) const
{
#ifdef DEBUG_QDND_WIN
    qDebug( "QDropEvent::provides ( %s )", mimeType );
#endif

    if ( !pIDataObject )
        return false;

    FORMATETC fmtMemory = { 0,
                            NULL,
                            DVASPECT_CONTENT,
                            -1,
                            TYMED_HGLOBAL };

    /* Search all available mimes for mimeType and look if pIDataObject
       can give us data in this clipboard format */
    QPtrList<QWindowsMime> all = QWindowsMime::all();
    for ( QWindowsMime * c = all.first(); c ; c = all.next() ) {
        int cf = c->cfFor( mimeType );
        if ( c->canConvert( mimeType, cf ) ) {
            fmtMemory.cfFormat = cf;
            if ( pIDataObject->QueryGetData( &fmtMemory ) == S_OK ) {
                return true;
            }

        }
    }
    return false;
}

/***********************************************
  Enables / disables drag and drop for widget w
************************************************/
void qt_olednd_unregister( QWidget* widget, QOleDropTarget *dst )
{
#ifdef DEBUG_QDND_WIN
    qDebug("qt_olednd_unregister( %p ) winID: %08x", widget, widget ? widget->winId() : 0 );
#endif
    dst->releaseQt();
    dst->Release();
#ifndef Q_OS_TEMP

    CoLockObjectExternal( dst, false, true );
    RevokeDragDrop( widget->winId() );
#endif
}

QOleDropTarget* qt_olednd_register( QWidget* widget )
{
#ifdef DEBUG_QDND_WIN
    qDebug("qt_olednd_register( %p ) winID: %08x", widget, widget ? widget->winId() : 0 );
#endif
    QOleDropTarget * dst = new QOleDropTarget( widget );
#ifndef Q_OS_TEMP

    HRESULT ret = RegisterDragDrop( widget->winId(), dst );
#ifdef DEBUG_QDND_WIN
    qDebug("ret RegisterDragDrop = %x", ret );
#endif
    CoLockObjectExternal( dst, true, true );
#endif

    return dst;
}

#endif  // QT_NO_DRAGANDDROP
