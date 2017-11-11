/****************************************************************************
** $Id: qt/qpainter.h   3.3.4   edited Jun 1 2004 $
**
** Definition of QPainter class
**
** Created : 940112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QPAINTER_H
#define QPAINTER_H


#ifndef QT_H
#include "qcolor.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"
#include "qregion.h"
#include "qpen.h"
#include "qbrush.h"
#include "qpointarray.h"
#include "qwmatrix.h"
#endif // QT_H

class QGfx;
class QTextCodec;
class QTextParag;
class QPaintDevice;
class QTextItem;
#if defined( Q_WS_MAC )
class QMacSavedPortInfo;
#endif
class QPainterPrivate;

#if defined(Q_WS_QWS)
class QScreen;
#endif

class Q_EXPORT QPainter : public Qt
{
public:
    enum CoordinateMode { CoordDevice, CoordPainter };

    QPainter();
    QPainter( const QPaintDevice *, bool unclipped = FALSE );
    QPainter( const QPaintDevice *, const QWidget *, bool unclipped = FALSE );
   ~QPainter();

    bool	begin( const QPaintDevice *, bool unclipped = FALSE );
    bool	begin( const QPaintDevice *, const QWidget *, bool unclipped = FALSE );
    bool	end();
    QPaintDevice *device() const;

#ifdef Q_WS_QWS
    QGfx * internalGfx();
#ifdef QT_QWS_EXPERIMENTAL_SCREENPAINTER
    bool begin(QScreen *screen);
#endif
#endif

    static void redirect( QPaintDevice *pdev, QPaintDevice *replacement );
    static QPaintDevice *redirect( QPaintDevice *pdev );

    bool	isActive() const;

    void	flush( const QRegion &region, CoordinateMode cm = CoordDevice );
    void	flush();
    void	save();
    void	restore();

  // Drawing tools

    QFontMetrics fontMetrics()	const;
    QFontInfo	 fontInfo()	const;

    const QFont &font()		const;
    void	setFont( const QFont & );
    const QPen &pen()		const;
    void	setPen( const QPen & );
    void	setPen( PenStyle );
    void	setPen( const QColor & );
    const QBrush &brush()	const;
    void	setBrush( const QBrush & );
    void	setBrush( BrushStyle );
    void	setBrush( const QColor & );
    QPoint	pos() const;

  // Drawing attributes/modes

    const QColor &backgroundColor() const;
    void	setBackgroundColor( const QColor & );
    BGMode	backgroundMode() const;
    void	setBackgroundMode( BGMode );
    RasterOp	rasterOp()	const;
    void	setRasterOp( RasterOp );
    const QPoint &brushOrigin() const;
    void	setBrushOrigin( int x, int y );
    void	setBrushOrigin( const QPoint & );

  // Scaling and transformations

//    PaintUnit unit()	       const;		// get set painter unit
//    void	setUnit( PaintUnit );		// NOT IMPLEMENTED!!!

    bool	hasViewXForm() const;
    bool	hasWorldXForm() const;

#ifndef QT_NO_TRANSFORMATIONS
    void	setViewXForm( bool );		// set xform on/off
    QRect	window()       const;		// get window
    void	setWindow( const QRect & );	// set window
    void	setWindow( int x, int y, int w, int h );
    QRect	viewport()   const;		// get viewport
    void	setViewport( const QRect & );	// set viewport
    void	setViewport( int x, int y, int w, int h );

    void	setWorldXForm( bool );		// set world xform on/off
    const QWMatrix &worldMatrix() const;	// get/set world xform matrix
    void	setWorldMatrix( const QWMatrix &, bool combine=FALSE );

    void	saveWorldMatrix();
    void	restoreWorldMatrix();

    void	scale( double sx, double sy );
    void	shear( double sh, double sv );
    void	rotate( double a );
#endif
    void	translate( double dx, double dy );
    void	resetXForm();
    double	translationX() const;
    double	translationY() const;

    QPoint	xForm( const QPoint & ) const;	// map virtual -> device
    QRect	xForm( const QRect & )	const;
    QPointArray xForm( const QPointArray & ) const;
    QPointArray xForm( const QPointArray &, int index, int npoints ) const;
    QPoint	xFormDev( const QPoint & ) const; // map device -> virtual
    QRect	xFormDev( const QRect & )  const;
    QPointArray xFormDev( const QPointArray & ) const;
    QPointArray xFormDev( const QPointArray &, int index, int npoints ) const;

  // Clipping

    void	setClipping( bool );		// set clipping on/off
    bool	hasClipping() const;
    QRegion clipRegion( CoordinateMode = CoordDevice ) const;
    void	setClipRect( const QRect &, CoordinateMode = CoordDevice );	// set clip rectangle
    void	setClipRect( int x, int y, int w, int h, CoordinateMode = CoordDevice );
    void	setClipRegion( const QRegion &, CoordinateMode = CoordDevice );// set clip region

  // Graphics drawing functions

    void	drawPoint( int x, int y );
    void	drawPoint( const QPoint & );
    void	drawPoints( const QPointArray& a,
			    int index=0, int npoints=-1 );
    void	moveTo( int x, int y );
    void	moveTo( const QPoint & );
    void	lineTo( int x, int y );
    void	lineTo( const QPoint & );
    void	drawLine( int x1, int y1, int x2, int y2 );
    void	drawLine( const QPoint &, const QPoint & );
    void	drawRect( int x, int y, int w, int h );
    void	drawRect( const QRect & );
    void	drawWinFocusRect( int x, int y, int w, int h );
    void	drawWinFocusRect( int x, int y, int w, int h,
				  const QColor &bgColor );
    void	drawWinFocusRect( const QRect & );
    void	drawWinFocusRect( const QRect &,
				  const QColor &bgColor );
    void	drawRoundRect( int x, int y, int w, int h, int = 25, int = 25 );
    void	drawRoundRect( const QRect &, int = 25, int = 25 );
    void	drawEllipse( int x, int y, int w, int h );
    void	drawEllipse( const QRect & );
    void	drawArc( int x, int y, int w, int h, int a, int alen );
    void	drawArc( const QRect &, int a, int alen );
    void	drawPie( int x, int y, int w, int h, int a, int alen );
    void	drawPie( const QRect &, int a, int alen );
    void	drawChord( int x, int y, int w, int h, int a, int alen );
    void	drawChord( const QRect &, int a, int alen );
    void	drawLineSegments( const QPointArray &,
				  int index=0, int nlines=-1 );
    void	drawPolyline( const QPointArray &,
			      int index=0, int npoints=-1 );
    void	drawPolygon( const QPointArray &, bool winding=FALSE,
			     int index=0, int npoints=-1 );
    void	drawConvexPolygon( const QPointArray &,
			     int index=0, int npoints=-1 );
#ifndef QT_NO_BEZIER
    void	drawCubicBezier( const QPointArray &, int index=0 );
#endif
    void	drawPixmap( int x, int y, const QPixmap &,
			    int sx=0, int sy=0, int sw=-1, int sh=-1 );
    void	drawPixmap( const QPoint &, const QPixmap &,
			    const QRect &sr );
    void	drawPixmap( const QPoint &, const QPixmap & );
    void	drawPixmap( const QRect &, const QPixmap & );
    void	drawImage( int x, int y, const QImage &,
			   int sx = 0, int sy = 0, int sw = -1, int sh = -1,
			   int conversionFlags = 0 );
    void	drawImage( const QPoint &, const QImage &,
			   const QRect &sr, int conversionFlags = 0 );
    void	drawImage( const QPoint &, const QImage &,
			   int conversion_flags = 0 );
    void	drawImage( const QRect &, const QImage & );
    void	drawTiledPixmap( int x, int y, int w, int h, const QPixmap &,
				 int sx=0, int sy=0 );
    void	drawTiledPixmap( const QRect &, const QPixmap &,
				 const QPoint & );
    void	drawTiledPixmap( const QRect &, const QPixmap & );
#ifndef QT_NO_PICTURE
    void	drawPicture( const QPicture & );
    void	drawPicture( int x, int y, const QPicture & );
    void	drawPicture( const QPoint &, const QPicture & );
#endif

    void	fillRect( int x, int y, int w, int h, const QBrush & );
    void	fillRect( const QRect &, const QBrush & );
    void	eraseRect( int x, int y, int w, int h );
    void	eraseRect( const QRect & );

  // Text drawing functions

    enum TextDirection {
	Auto,
	RTL,
	LTR
    };

    void	drawText( int x, int y, const QString &, int len = -1, TextDirection dir = Auto );
    void	drawText( const QPoint &, const QString &, int len = -1, TextDirection dir = Auto );

    void     drawText( int x, int y, const QString &, int pos, int len, TextDirection dir = Auto );
    void     drawText( const QPoint &p, const QString &, int pos, int len, TextDirection dir = Auto );

    void	drawText( int x, int y, int w, int h, int flags,
			  const QString&, int len = -1, QRect *br=0,
			  QTextParag **intern=0 );
    void	drawText( const QRect &, int flags,
			  const QString&, int len = -1, QRect *br=0,
			  QTextParag **intern=0 );

    void drawTextItem( int x, int y, const QTextItem &ti, int textflags = 0 );
    void drawTextItem( const QPoint& p, const QTextItem &ti, int textflags = 0 );

    QRect	boundingRect( int x, int y, int w, int h, int flags,
			      const QString&, int len = -1, QTextParag **intern=0 );
    QRect	boundingRect( const QRect &, int flags,
			      const QString&, int len = -1, QTextParag **intern=0 );

    int		tabStops() const;
    void	setTabStops( int );
    int	       *tabArray() const;
    void	setTabArray( int * );

    // Other functions

#if defined(Q_WS_WIN)
    HDC		handle() const;
#elif defined(Q_WS_X11) || defined(Q_WS_MAC)
    HANDLE	handle() const;
#endif


    static void initialize();
    static void cleanup();

private:
    void	init();
    void        destroy();
    void	updateFont();
    void	updatePen();
    void	updateBrush();
#ifndef QT_NO_TRANSFORMATIONS
    void	updateXForm();
    void	updateInvXForm();
#endif
    void	map( int, int, int *rx, int *ry ) const;
    void	map( int, int, int, int, int *, int *, int *, int * ) const;
    void	mapInv( int, int, int *, int * ) const;
    void	mapInv( int, int, int, int, int *, int *, int *, int * ) const;
    void	drawPolyInternal( const QPointArray &, bool close=TRUE );
    void	drawWinFocusRect( int x, int y, int w, int h, bool xorPaint,
				  const QColor &penColor );

    enum { IsActive=0x01, ExtDev=0x02, IsStartingUp=0x04, NoCache=0x08,
	   VxF=0x10, WxF=0x20, ClipOn=0x40, SafePolygon=0x80, MonoDev=0x100,
	   DirtyFont=0x200, DirtyPen=0x400, DirtyBrush=0x800,
	   RGBColor=0x1000, FontMet=0x2000, FontInf=0x4000, CtorBegin=0x8000,
           UsePrivateCx = 0x10000, VolatileDC = 0x20000, Qt2Compat = 0x40000 };
    uint	flags;
    bool	testf( uint b ) const { return (flags&b)!=0; }
    void	setf( uint b )	{ flags |= b; }
    void	setf( uint b, bool v );
    void	clearf( uint b )	{ flags &= (uint)(~b); }
    void	fix_neg_rect( int *x, int *y, int *w, int *h );

    QPainterPrivate *d;
    QPaintDevice *pdev;
    QColor	bg_col;
    uchar	bg_mode;
    uchar	rop;
    uchar	pu;
    QPoint	bro;
    QFont	cfont;
    QFont *pfont; 	// font used for metrics (might be different for printers)
    QPen	cpen;
    QBrush	cbrush;
    QRegion	crgn;
    int		tabstops;
    int	       *tabarray;
    int		tabarraylen;
    bool	block_ext;	// for temporary blocking of external devices

    // Transformations
#ifndef QT_NO_TRANSFORMATIONS
    QCOORD	wx, wy, ww, wh;
    QCOORD	vx, vy, vw, vh;
    QWMatrix	wxmat;

    // Cached composition (and inverse) of transformations
    QWMatrix	xmat;
    QWMatrix	ixmat;



    double	m11() const { return xmat.m11(); }
    double      m12() const { return xmat.m12(); }
    double      m21() const { return xmat.m21(); }
    double      m22() const { return xmat.m22(); }
    double      dx() const { return xmat.dx(); }
    double      dy() const { return xmat.dy(); }
    double	im11() const { return ixmat.m11(); }
    double      im12() const { return ixmat.m12(); }
    double      im21() const { return ixmat.m21(); }
    double      im22() const { return ixmat.m22(); }
    double      idx() const { return ixmat.dx(); }
    double      idy() const { return ixmat.dy(); }

    int		txop;
    bool	txinv;

#else
    // even without transformations we still have translations
    int		xlatex;
    int		xlatey;
#endif

    void       *penRef;				// pen cache ref
    void       *brushRef;			// brush cache ref
    void       *ps_stack;
    void       *wm_stack;
    void	killPStack();

protected:
#ifdef Q_OS_TEMP
    QPoint	internalCurrentPos;
    uint	old_pix;			// ### All win platforms in 4.0
#endif
#if defined(Q_WS_WIN)
    friend class QFontEngineWin;
    friend class QFontEngineBox;
    QT_WIN_PAINTER_MEMBERS
#elif defined(Q_WS_X11)
    friend class QFontEngineXLFD;
    friend class QFontEngineXft;
    friend class QFontEngineBox;
    Display    *dpy;				// current display
    int		scrn;				// current screen
    Qt::HANDLE	hd;				// handle to drawable
    Qt::HANDLE  rendhd;				// handle to Xft draw
    GC		gc;				// graphics context (standard)
    GC		gc_brush;			// graphics contect for brush
    QPoint	curPt;				// current point
    uint	clip_serial;			// clipping serial number
#elif defined(Q_WS_MAC)
    Qt::HANDLE	hd;				// handle to drawable
    void initPaintDevice(bool force=FALSE, QPoint *off=NULL, QRegion *rgn=NULL);
    friend const QRegion &qt_mac_update_painter(QPainter *, bool);
    friend class QFontEngineMac;
    friend class QMacPainter;
#elif defined(Q_WS_QWS)
    friend class QFontEngine;
    QGfx * gfx;
    friend void qwsUpdateActivePainters();
#endif
    friend class QFontMetrics;
    friend class QFontInfo;
    friend class QTextLayout;
    friend void qt_format_text( const QFont &, const QRect &r,
		     int tf, const QString& str, int len, QRect *brect,
		     int tabstops, int* tabarray, int tabarraylen,
		     QTextParag **internal, QPainter* painter );
    friend void qt_draw_background( QPainter *p, int x, int y, int w,  int h );
    friend void qt_draw_transformed_rect( QPainter *p,  int x, int y, int w,  int h, bool fill );
    friend class QPrinter;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPainter( const QPainter & );
    QPainter &operator=( const QPainter & );
#endif

    enum TransformationCodes {
	TxNone      = 0,		// transformation codes
	TxTranslate = 1,		// copy in qpainter_*.cpp
	TxScale     = 2,
	TxRotShear  = 3
    };
};


/*****************************************************************************
  QPainter member functions
 *****************************************************************************/

inline QPaintDevice *QPainter::device() const
{
    return pdev;
}

inline bool QPainter::isActive() const
{
    return testf(IsActive);
}

inline const QFont &QPainter::font() const
{
    return cfont;
}

inline const QPen &QPainter::pen() const
{
    return cpen;
}

inline const QBrush &QPainter::brush() const
{
    return cbrush;
}

/*
inline PaintUnit QPainter::unit() const
{
    return (PaintUnit)pu;
}
*/

inline const QColor &QPainter::backgroundColor() const
{
    return bg_col;
}

inline Qt::BGMode QPainter::backgroundMode() const
{
    return (BGMode)bg_mode;
}

inline Qt::RasterOp QPainter::rasterOp() const
{
    return (RasterOp)rop;
}

inline const QPoint &QPainter::brushOrigin() const
{
    return bro;
}

inline bool QPainter::hasViewXForm() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return testf(VxF);
#else
    return xlatex || xlatey;
#endif
}

inline bool QPainter::hasWorldXForm() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return testf(WxF);
#else
    return xlatex || xlatey;
#endif
}

inline double QPainter::translationX() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return worldMatrix().dx();
#else
    return xlatex;
#endif
}

inline double QPainter::translationY() const
{
#ifndef QT_NO_TRANSFORMATIONS
    return worldMatrix().dy();
#else
    return xlatey;
#endif
}


inline bool QPainter::hasClipping() const
{
    return testf(ClipOn);
}

inline int QPainter::tabStops() const
{
    return tabstops;
}

inline int *QPainter::tabArray() const
{
    return tabarray;
}

#if defined(Q_WS_WIN)
inline HDC QPainter::handle() const
{
    return hdc;
}
#elif defined(Q_WS_X11) || defined(Q_WS_MAC)
inline Qt::HANDLE QPainter::handle() const
{
    return hd;
}
#endif

inline void QPainter::setBrushOrigin( const QPoint &p )
{
    setBrushOrigin( p.x(), p.y() );
}

#ifndef QT_NO_TRANSFORMATIONS
inline void QPainter::setWindow( const QRect &r )
{
    setWindow( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::setViewport( const QRect &r )
{
    setViewport( r.x(), r.y(), r.width(), r.height() );
}
#endif

inline void QPainter::setClipRect( int x, int y, int w, int h, CoordinateMode m )
{
    setClipRect( QRect(x,y,w,h), m );
}

inline void QPainter::drawPoint( const QPoint &p )
{
    drawPoint( p.x(), p.y() );
}

inline void QPainter::moveTo( const QPoint &p )
{
    moveTo( p.x(), p.y() );
}

inline void QPainter::lineTo( const QPoint &p )
{
    lineTo( p.x(), p.y() );
}

inline void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
{
    drawLine( p1.x(), p1.y(), p2.x(), p2.y() );
}

inline void QPainter::drawRect( const QRect &r )
{
    drawRect( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::drawWinFocusRect( const QRect &r )
{
    drawWinFocusRect( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::drawWinFocusRect( const QRect &r,const QColor &penColor )
{
    drawWinFocusRect( r.x(), r.y(), r.width(), r.height(), penColor );
}

inline void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
{
    drawRoundRect( r.x(), r.y(), r.width(), r.height(), xRnd, yRnd );
}

inline void QPainter::drawEllipse( const QRect &r )
{
    drawEllipse( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::drawArc( const QRect &r, int a, int alen )
{
    drawArc( r.x(), r.y(), r.width(), r.height(), a, alen );
}

inline void QPainter::drawPie( const QRect &r, int a, int alen )
{
    drawPie( r.x(), r.y(), r.width(), r.height(), a, alen );
}

inline void QPainter::drawChord( const QRect &r, int a, int alen )
{
    drawChord( r.x(), r.y(), r.width(), r.height(), a, alen );
}

inline void QPainter::drawPixmap( const QPoint &p, const QPixmap &pm,
				  const QRect &sr )
{
    drawPixmap( p.x(), p.y(), pm, sr.x(), sr.y(), sr.width(), sr.height() );
}

inline void QPainter::drawImage( const QPoint &p, const QImage &pm,
                                 const QRect &sr, int conversionFlags )
{
    drawImage( p.x(), p.y(), pm,
	       sr.x(), sr.y(), sr.width(), sr.height(), conversionFlags );
}

inline void QPainter::drawTiledPixmap( const QRect &r, const QPixmap &pm,
				       const QPoint &sp )
{
    drawTiledPixmap( r.x(), r.y(), r.width(), r.height(), pm, sp.x(), sp.y() );
}

inline void QPainter::drawTiledPixmap( const QRect &r, const QPixmap &pm )
{
    drawTiledPixmap( r.x(), r.y(), r.width(), r.height(), pm, 0, 0 );
}

inline void QPainter::fillRect( const QRect &r, const QBrush &brush )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), brush );
}

inline void QPainter::eraseRect( int x, int y, int w, int h )
{
    fillRect( x, y, w, h, backgroundColor() );
}

inline void QPainter::eraseRect( const QRect &r )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), backgroundColor() );
}

inline void QPainter::drawText( const QPoint &p, const QString &s, int len, TextDirection dir )
{
    drawText( p.x(), p.y(), s, 0, len, dir );
}

inline void QPainter::drawText( const QPoint &p, const QString &s, int pos, int len, TextDirection dir )
{
    drawText( p.x(), p.y(), s, pos, len, dir );
}

inline void QPainter::drawText( int x, int y, int w, int h, int tf,
				const QString& str, int len, QRect *br, QTextParag **i )
{
    QRect r(x, y, w, h);
    drawText( r, tf, str, len, br, i );
}

inline void QPainter::drawTextItem( const QPoint& p, const QTextItem &ti, int textflags )
{
    drawTextItem( p.x(), p.y(), ti, textflags );
}

inline QRect QPainter::boundingRect( int x, int y, int w, int h, int tf,
				     const QString& str, int len, QTextParag **i )
{
    QRect r(x, y, w, h);
    return boundingRect( r, tf, str, len, i );
}

#if defined(Q_WS_QWS)
inline QGfx * QPainter::internalGfx()
{
    return gfx;
}
#endif

#endif // QPAINTER_H
