/****************************************************************************
** $Id: qsvgdevice.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of the QSvgDevice class
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the xml module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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
*****************************************************************************/

#include <private/qsvgdevice_p.h>

#ifndef QT_NO_SVG

#include "qpainter.h"
#include "qpaintdevicemetrics.h"
#include "qfile.h"
#include "qmap.h"
#include "qregexp.h"
#include "qvaluelist.h"
#include "qtextstream.h"
#include "qimage.h"
#include "qpixmap.h"

#include <math.h>

const double deg2rad = 0.017453292519943295769;	// pi/180
const char piData[] = "version=\"1.0\" standalone=\"no\"";
const char publicId[] = "-//W3C//DTD SVG 20001102//EN";
const char systemId[] = "http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd";

struct QM_EXPORT_SVG ImgElement {
    QDomElement element;
    QImage image;
    Q_DUMMY_COMPARISON_OPERATOR( ImgElement )
};

struct QM_EXPORT_SVG PixElement {
    QDomElement element;
    QPixmap pixmap;
    Q_DUMMY_COMPARISON_OPERATOR( PixElement )
};

struct QSvgDeviceState {
    int textx, texty;			// current text position
    int textalign;			// text alignment
    Q_DUMMY_COMPARISON_OPERATOR( QSvgDeviceState )
};

typedef QValueList<ImgElement> ImageList;
typedef QValueList<PixElement> PixmapList;
typedef QValueList<QSvgDeviceState> StateList;

class QSvgDevicePrivate {
public:
    ImageList images;
    PixmapList pixmaps;
    StateList stack;
    int currentClip;

    uint justRestored : 1;

    QMap<QString, QRegion> clipPathTable;
};

enum ElementType {
    InvalidElement = 0,
    AnchorElement,
    CircleElement,
    ClipElement,
    CommentElement,
    DescElement,
    EllipseElement,
    GroupElement,
    ImageElement,
    LineElement,
    PolylineElement,
    PolygonElement,
    PathElement,
    RectElement,
    SvgElement,
    TextElement,
    TitleElement,
    TSpanElement
};

typedef QMap<QString,ElementType> QSvgTypeMap;
static QSvgTypeMap *qSvgTypeMap=0; // element types
static QMap<QString,QString> *qSvgColMap=0; // recognized color keyword names

/*!
    \class QSvgDevice qsvgdevice.h
    \brief The QSvgDevice class provides a paint device for SVG vector graphics.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup xml-tools
    \module XML
    \internal

    SVG is an XML vector graphics format. This class supports the
    loading and saving of SVG files with load() and save(), and the
    rendering of an SVG onto a QPainter using play(). Use toString()
    to put the SVG into a string.

    \sa QPaintDevice QPainter
*/

/*!
    Creates a QSvgDevice object.
*/

QSvgDevice::QSvgDevice()
    : QPaintDevice( QInternal::ExternalDevice ),
      pt( 0 )
{
    d = new QSvgDevicePrivate;
    d->currentClip = 0;
    d->justRestored = FALSE;
}

/*!
    Destroys the QSvgDevice object and frees the resources it used.
*/

QSvgDevice::~QSvgDevice()
{
    delete qSvgTypeMap; qSvgTypeMap = 0;	// static
    delete qSvgColMap; qSvgColMap = 0;
    delete d;
}

/*!
    Loads and parses a SVG from \a dev into the device. Returns TRUE
    on success (i.e. loaded and parsed without error); otherwise
    returns FALSE.
*/

bool QSvgDevice::load( QIODevice *dev )
{
    return doc.setContent( dev );
}

/*!
    Renders (replays) the SVG on the \a painter and returns TRUE if
    successful (i.e. it is a valid SVG); otherwise returns FALSE.
*/

bool QSvgDevice::play( QPainter *painter )
{
    if ( !painter ) {
#if defined(QT_CHECK_RANGE)
	Q_ASSERT( painter );
#endif
	return FALSE;
    }
    pt = painter;
    pt->setPen( Qt::NoPen ); // SVG default pen and brush
    pt->setBrush( Qt::black );
    if ( doc.isNull() ) {
	qWarning( "QSvgDevice::play: No SVG data set." );
	return FALSE;
    }

    QDomNode svg = doc.namedItem( "svg" );
    if ( svg.isNull() || !svg.isElement() ) {
	qWarning( "QSvgDevice::play: Couldn't find any svg element." );
	return FALSE;
    }

    // force transform to be activated in case our sequences
    // are replayed later with a transformed painter
    painter->setWorldXForm( TRUE );

    QDomNamedNodeMap attr = svg.attributes();
    int x = lenToInt( attr, "x" );
    int y = lenToInt( attr, "y" );
    brect.setX( x );
    brect.setY( y );
    QString wstr = attr.contains( "width" )
		   ? attr.namedItem( "width" ).nodeValue() : QString( "100%" );
    QString hstr = attr.contains( "height" )
		   ? attr.namedItem( "height" ).nodeValue() : QString( "100%" );
    double width = parseLen( wstr, 0, TRUE );
    double height = parseLen( hstr, 0, FALSE );
    // SVG doesn't respect x and y. But we want a proper bounding rect.
    brect.setWidth( int(width) - x );
    brect.setHeight( int(height) - y );
    painter->setClipRect( brect, QPainter::CoordPainter );

    if ( attr.contains( "viewBox" ) ) {
	QRegExp re( QString::fromLatin1("\\s*(\\S+)\\s*,?\\s*(\\S+)\\s*,?"
					"\\s*(\\S+)\\s*,?\\s*(\\S+)\\s*") );
	if ( re.search( attr.namedItem( "viewBox" ).nodeValue() ) < 0 ) {
	    qWarning( "QSvgDevice::play: Invalid viewBox attribute.");
	    return FALSE;
	} else {
	    double x = re.cap( 1 ).toDouble();
	    double y = re.cap( 2 ).toDouble();
	    double w = re.cap( 3 ).toDouble();
	    double h = re.cap( 4 ).toDouble();
	    if ( w < 0 || h < 0 ) {
		qWarning( "QSvgDevice::play: Invalid viewBox dimension.");
		return FALSE;
	    } else if ( w == 0 || h == 0 ) {
		return TRUE;
	    }
	    painter->scale( width/w, height/h );
	    painter->translate( -x, -y );
	}
    }

    const struct ElementTable {
	const char *name;
	ElementType type;
    } etab[] = {
	{ "a",        AnchorElement   },
	{ "#comment", CommentElement  },
        { "circle",   CircleElement   },
	{ "clipPath", ClipElement     },
	{ "desc",     DescElement     },
        { "ellipse",  EllipseElement  },
	{ "g",        GroupElement    },
        { "image",    ImageElement    },
        { "line",     LineElement     },
        { "polyline", PolylineElement },
        { "polygon",  PolygonElement  },
        { "path",     PathElement     },
        { "rect",     RectElement     },
        { "svg",      SvgElement      },
        { "text",     TextElement     },
        { "tspan",    TSpanElement    },
	{ "title",    TitleElement    },
	{ 0,          InvalidElement  }
    };
    // initialize only once
    if ( !qSvgTypeMap ) {
	qSvgTypeMap = new QSvgTypeMap;
	const ElementTable *t = etab;
	while ( t->name ) {
	    qSvgTypeMap->insert( t->name, t->type );
	    t++;
	}
    }

    // initial state
    QSvgDeviceState st;
    st.textx = st.texty = 0;
    st.textalign = Qt::AlignLeft;
    d->stack.append(st);
    curr = &d->stack.last();
    // 'play' all elements recursively starting with 'svg' as root
    bool b = play( svg );
    d->stack.remove( d->stack.begin() );
    return b;
}

/*!
    Returns the SVG as a single string of XML.
*/
QString QSvgDevice::toString() const
{
    if ( doc.isNull() )
	return QString();

    return doc.toString();
}

/*!
    Saves the SVG to \a fileName.
*/

bool QSvgDevice::save( const QString &fileName )
{
    // guess svg id from fileName
    QString svgName = fileName.endsWith( ".svg" ) ?
		      fileName.left( fileName.length()-4 ) : fileName;

    // now we have the info about name and dimensions available
    QDomElement root = doc.documentElement();
    root.setAttribute( "id", svgName );
    // the standard doesn't take respect x and y. But we want a
    // proper bounding rect. We make width and height bigger when
    // writing out and subtract x and y when reading in.
    root.setAttribute( "x", brect.x() );
    root.setAttribute( "y", brect.y() );
    root.setAttribute( "width", brect.width() + brect.x() );
    root.setAttribute( "height", brect.height() + brect.y() );

    // ... and know how to name any image files to be written out
    int icount = 0;
    ImageList::Iterator iit = d->images.begin();
    for ( ; iit != d->images.end(); ++iit ) {
	QString href = QString( "%1_%2.png" ).arg( svgName ).arg( icount );
	(*iit).image.save( href, "PNG" );
	(*iit).element.setAttribute( "xlink:href", href );
	icount++;
    }
    PixmapList::Iterator pit = d->pixmaps.begin();
    for ( ; pit != d->pixmaps.end(); ++pit ) {
	QString href = QString( "%1_%2.png" ).arg( svgName ).arg( icount );
	(*pit).pixmap.save( href, "PNG" );
	(*pit).element.setAttribute( "xlink:href", href );
	icount++;
    }

    QFile f( fileName );
    if ( !f.open ( IO_WriteOnly ) )
	return FALSE;
    QTextStream s( &f );
    s.setEncoding( QTextStream::UnicodeUTF8 );
    s << doc;

    return TRUE;
}

/*!
    \overload

    \a dev is the device to use for saving.
*/

bool QSvgDevice::save( QIODevice *dev )
{
#if defined(CHECK_RANGE)
    if ( !d->images.isEmpty() || !d->pixmaps.isEmpty() )
	qWarning( "QSvgDevice::save: skipping external images" );
#endif

    QTextStream s( dev );
    s.setEncoding( QTextStream::UnicodeUTF8 );
    s << doc;

    return TRUE;
}

/*!
    \fn QRect QSvgDevice::boundingRect() const

    Returns the bounding rectangle of the SVG.
*/

/*!
    Sets the bounding rectangle of the SVG to rectangle \a r.
*/

void QSvgDevice::setBoundingRect( const QRect &r )
{
    brect = r;
}

/*!
    Internal implementation of the virtual QPaintDevice::metric()
    function.

    \warning Use the QPaintDeviceMetrics class instead.

    A QSvgDevice has the following hard coded values: dpi=72,
    numcolors=16777216 and depth=24. \a m is the metric to get.
*/

int QSvgDevice::metric( int m ) const
{
    int val;
    switch ( m ) {
    case QPaintDeviceMetrics::PdmWidth:
	val = brect.width();
	break;
    case QPaintDeviceMetrics::PdmHeight:
	val = brect.height();
	break;
    case QPaintDeviceMetrics::PdmWidthMM:
	val = int(25.4/72.0*brect.width());
	break;
    case QPaintDeviceMetrics::PdmHeightMM:
	val = int(25.4/72.0*brect.height());
	break;
    case QPaintDeviceMetrics::PdmDpiX:
	val = 72;
	break;
    case QPaintDeviceMetrics::PdmDpiY:
	val = 72;
	break;
    case QPaintDeviceMetrics::PdmNumColors:
	val = 16777216;
	break;
    case QPaintDeviceMetrics::PdmDepth:
	val = 24;
	break;
    default:
	val = 0;
#if defined(QT_CHECK_RANGE)
	qWarning( "QSvgDevice::metric: Invalid metric command" );
#endif
    }
    return val;
}

/*!
    \internal

    Records painter commands and stores them in the QDomDocument doc.
*/

bool QSvgDevice::cmd ( int c, QPainter *painter, QPDevCmdParam *p )
{
    pt = painter;

    if ( c == PdcBegin ) {
	QDomImplementation domImpl;
	QDomDocumentType docType = domImpl.createDocumentType( "svg",
							       publicId,
							       systemId );
	doc = domImpl.createDocument( "http://www.w3.org/2000/svg",
				      "svg", docType );
	doc.insertBefore( doc.createProcessingInstruction( "xml", piData ),
			  doc.firstChild() );
	current = doc.documentElement();
	d->images.clear();
	d->pixmaps.clear();
	dirtyTransform = dirtyStyle = FALSE; // ###
	return TRUE;
    } else if ( c == PdcEnd ) {
	return TRUE;
    }

    QDomElement e;
    QString str;
    QRect rect;
    QPointArray a;
    int i, width, height, x, y;
    switch ( c ) {
    case PdcNOP:
	break;
    case PdcMoveTo:
	curPt = *p[0].point;
	break;
    case PdcLineTo:
	e = doc.createElement( "line" );
	e.setAttribute( "x1", curPt.x() );
	e.setAttribute( "y1", curPt.y() );
	e.setAttribute( "x2", p[0].point->x() );
	e.setAttribute( "y2", p[0].point->y() );
	break;
    case PdcDrawPoint:
    case PdcDrawLine:
	e = doc.createElement( "line" );
	e.setAttribute( "x1", p[0].point->x() );
	e.setAttribute( "y1", p[0].point->y() );
	i = ( c == PdcDrawLine ) ? 1 : 0;
	e.setAttribute( "x2", p[i].point->x() );
	e.setAttribute( "y2", p[i].point->y() );
	break;
    case PdcDrawRect:
    case PdcDrawRoundRect:
	e = doc.createElement( "rect" );
	x = p[0].rect->x();
	y = p[0].rect->y();
	width = p[0].rect->width();
	height = p[0].rect->height();
	if ( width < 0 ) {
	    width = -width;
	    x -= width - 1;
	}
	if ( height < 0 ) {
	    height = -height;
	    y -= height - 1;
	}
	e.setAttribute( "x", x );
	e.setAttribute( "y", y );
	e.setAttribute( "width", width );
	e.setAttribute( "height", height );
	if ( c == PdcDrawRoundRect ) {
	    e.setAttribute( "rx", (p[1].ival*p[0].rect->width())/200 );
	    e.setAttribute( "ry", (p[2].ival*p[0].rect->height())/200 );
	}
	break;
    case PdcDrawEllipse:
	rect = *p[0].rect;
	if ( rect.width() == rect.height() ) {
	    e = doc.createElement( "circle" );
	    double cx = rect.x() + (rect.width() / 2.0);
	    double cy = rect.y() + (rect.height() / 2.0);
	    e.setAttribute( "cx", cx );
	    e.setAttribute( "cy", cy );
	    e.setAttribute( "r", cx - rect.x() );
	} else {
	    e = doc.createElement( "ellipse" );
    	    double cx = rect.x() + (rect.width() / 2.0);
	    double cy = rect.y() + (rect.height() / 2.0);
	    e.setAttribute( "cx", cx );
	    e.setAttribute( "cy", cy );
	    e.setAttribute( "rx", cx - rect.x() );
	    e.setAttribute( "ry", cy - rect.y() );
	}
	break;
    case PdcDrawArc:
    case PdcDrawPie:
    case PdcDrawChord: {
	rect = *p[0].rect;
	double a = (double)p[1].ival / 16.0 * deg2rad;
	double al = (double)p[2].ival / 16.0 * deg2rad;
	double rx = rect.width() / 2.0;
	double ry = rect.height() / 2.0;
	double x0 = (double)rect.x() + rx;
	double y0 = (double)rect.y() + ry;
	double x1 = x0 + rx*cos(a);
	double y1 = y0 - ry*sin(a);
	double x2 = x0 + rx*cos(a+al);
	double y2 = y0 - ry*sin(a+al);
	int large = QABS( al ) > ( 180.0 * deg2rad ) ? 1 : 0;
	int sweep = al < 0.0 ? 1 : 0;
	if ( c == PdcDrawPie )
	    str = QString( "M %1 %2 L %3 %4 " ).arg( x0 ).arg( y0 )
		  .arg( x1 ).arg( y1 );
	else
	    str = QString( "M %1 %2 " ).arg( x1 ).arg( y1 );
	str += QString( "A %1 %2 %3 %4 %5 %6 %7" )
	       .arg( rx ).arg( ry ).arg( a/deg2rad ). arg( large ).arg( sweep )
	       .arg( x2 ).arg( y2 );
	if ( c != PdcDrawArc )
	    str += "z";
	e = doc.createElement( "path" );
	e.setAttribute( "d", str );
    }
	break;
    case PdcDrawLineSegments:
	{
	    a = *p[0].ptarr;
	    uint end = a.size() / 2;
	    for (uint i = 0; i < end; i++) {
		e = doc.createElement( "line" );
		e.setAttribute( "x1", a[int(2*i)].x() );
		e.setAttribute( "y1", a[int(2*i)].y() );
		e.setAttribute( "x2", a[int(2*i+1)].x() );
		e.setAttribute( "y2", a[int(2*i+1)].y() );
		if ( i < end - 1 ) // The last one will be done at the end
		    appendChild( e, c );
	    }
	}
	break;
    case PdcDrawPolyline:
    case PdcDrawPolygon:
	{
	    a = *p[0].ptarr;
	    e = doc.createElement( ( c == PdcDrawPolyline ) ?
				   "polyline" : "polygon" );
	    for (uint i = 0; i < a.size(); i++) {
		QString tmp;
		tmp.sprintf( "%d %d ", a[ (int)i ].x(), a[ (int)i ].y() );
		str += tmp;
	    }
	    e.setAttribute( "points", str.stripWhiteSpace() );
	}
	break;
#ifndef QT_NO_BEZIER
    case PdcDrawCubicBezier:
	a = *p[0].ptarr;
	e = doc.createElement( "path" );
	str.sprintf( "M %d %d C %d %d %d %d %d %d", a[0].x(), a[0].y(),
		     a[1].x(), a[1].y(), a[2].x(), a[2].y(),
		     a[3].x(), a[3].y() );
	e.setAttribute( "d", str );
	break;
#endif
    case PdcDrawText2:
	e = doc.createElement( "text" );
	if ( p[0].point->x() )
	    e.setAttribute( "x", p[0].point->x() );
	if ( p[0].point->y() )
	    e.setAttribute( "y", p[0].point->y() );
	e.appendChild( doc.createTextNode( *p[1].str ) );
	break;
    case PdcDrawText2Formatted: {
	e = doc.createElement( "text" );
	const QRect *r = p[0].rect;
	int tf = p[1].ival;
	int x, y;
	// horizontal text alignment
	if ( ( tf & Qt::AlignHCenter ) != 0 ) {
	    x = r->x() + r->width() / 2;
	    e.setAttribute( "text-anchor", "middle" );
	} else if ( ( tf & Qt::AlignRight ) != 0 ) {
	    x = r->right();
	    e.setAttribute( "text-anchor", "end" );
	} else {
	    x = r->x();
	}
	// vertical text alignment
	if ( ( tf & Qt::AlignVCenter ) != 0 )
	    y = r->y() + ( r->height() + painter->fontMetrics().ascent() ) / 2;
	else if ( ( tf & Qt::AlignBottom ) != 0 )
	    y = r->bottom();
	else
	    y = r->y() + painter->fontMetrics().ascent();
	if ( x )
	    e.setAttribute( "x", x );
	if ( y )
	    e.setAttribute( "y", y );
	e.appendChild( doc.createTextNode( *p[2].str ) );
    }
	break;
    case PdcDrawPixmap:
    case PdcDrawImage:
	e = doc.createElement( "image" );
	e.setAttribute( "x", p[0].rect->x() );
	e.setAttribute( "y", p[0].rect->y() );
	e.setAttribute( "width", p[0].rect->width() );
	e.setAttribute( "height", p[0].rect->height() );
	if ( c == PdcDrawImage ) {
	    ImgElement ie;
	    ie.element = e;
	    ie.image = *p[1].image;
	    d->images.append( ie );
	} else {
	    PixElement pe;
	    pe.element = e;
	    pe.pixmap = *p[1].pixmap;
	    d->pixmaps.append( pe );
	}
	// saving to disk and setting the xlink:href attribute will be
	// done later in save() once we now the svg document name.
	break;
    case PdcSave:
	e = doc.createElement( "g" );
	break;
    case PdcRestore:
	current = current.parentNode();
	dirtyTransform = !pt->worldMatrix().isIdentity();
        d->justRestored = TRUE;
	// ### reset dirty flags
	break;
    case PdcSetBkColor:
    case PdcSetBkMode:
    case PdcSetROP:
    case PdcSetBrushOrigin:
    case PdcSetFont:
    case PdcSetPen:
    case PdcSetBrush:
	dirtyStyle = TRUE;
	break;
    case PdcSetTabStops:
	// ###
	break;
    case PdcSetTabArray:
	// ###
	break;
    case PdcSetVXform:
    case PdcSetWindow:
    case PdcSetViewport:
    case PdcSetWXform:
    case PdcSetWMatrix:
    case PdcSaveWMatrix:
    case PdcRestoreWMatrix:
	dirtyTransform = TRUE;
	break;
    case PdcSetClip:
	// ###
	break;
    case PdcSetClipRegion:
	{
            // We skip the clip after restore, since restoring the clip is done automatically by
            // the viewer as part of the tree structure. It doesn't hurt to write the region
            // out, but it doubles the number of clipregions defined in the final svg.
            if (d->justRestored) {
                d->justRestored = FALSE;
                return TRUE;
            }

            QMemArray<QRect> rects = p[0].rgn->rects();
            if (rects.count() == 0)
                return TRUE;
	    d->currentClip++;
	    e = doc.createElement( "clipPath" );
	    e.setAttribute( "id", QString("clip%1").arg(d->currentClip) );
            for (int i=0; i<(int)rects.count(); ++i) {
                QDomElement ce = doc.createElement("rect");
                ce.setAttribute( "x", rects.at(i).x() );
                ce.setAttribute( "y", rects.at(i).y() );
                ce.setAttribute( "width", rects.at(i).width() );
                ce.setAttribute( "height", rects.at(i).height() );
                e.appendChild(ce);
	    }
	    break;
	}
    default:
#if defined(CHECK_RANGE)
	qWarning( "QSVGDevice::cmd: Invalid command %d", c );
#endif
	break;
    }

    appendChild( e, c );

    return TRUE;
}

/*!
    \internal

    Appends the child and applys any style and transformation.

*/

void QSvgDevice::appendChild( QDomElement &e, int c )
{
    if ( !e.isNull() ) {
	current.appendChild( e );
	if ( c == PdcSave )
	    current = e;
	// ### optimize application of attributes utilizing <g>
	if ( c == PdcSetClipRegion ) {
	    QDomElement ne;
	    ne = doc.createElement( "g" );
	    ne.setAttribute( "style", QString("clip-path:url(#clip%1)").arg(d->currentClip) );
	    current.appendChild( ne );
	    current = ne;
	} else {
	    if ( dirtyStyle )		// only reset when entering
		applyStyle( &e, c );	// or leaving a <g> tag
	    if ( dirtyTransform && e.tagName() != "g" ) {
		// same as above but not for <g> tags
		applyTransform( &e );
		if ( c == PdcSave )
		    dirtyTransform = FALSE;
	    }
	}
    }
}


/*!
    \internal

    Push the current drawing attributes on a stack.

    \sa restoreAttributes()
*/

void QSvgDevice::saveAttributes()
{
    pt->save();
    // copy old state
    QSvgDeviceState st( *curr );
    d->stack.append( st );
    curr = &d->stack.last();
}

/*!
    \internal

    Pop the current drawing attributes off the stack.

    \sa saveAttributes()
*/

void QSvgDevice::restoreAttributes()
{
    pt->restore();
    Q_ASSERT( d->stack.count() > 1 );
    d->stack.remove( d->stack.fromLast() );
    curr = &d->stack.last();
}

/*!
    \internal

    Evaluate \a node, drawing on \a p. Allows recursive calls.
*/

bool QSvgDevice::play( const QDomNode &node )
{
    saveAttributes();

	ElementType t = (*qSvgTypeMap)[ node.nodeName() ];

	if ( t == LineElement && pt->pen().style() == Qt::NoPen ) {
	    QPen p = pt->pen();
	    p.setStyle( Qt::SolidLine );
	    pt->setPen( p );
	}
	QDomNamedNodeMap attr = node.attributes();
	if ( attr.contains( "style" ) )
	    setStyle( attr.namedItem( "style" ).nodeValue() );
	// ### might have to exclude more elements from transform
	if ( t != SvgElement && attr.contains( "transform" ) )
	    setTransform( attr.namedItem( "transform" ).nodeValue() );
	uint i = attr.length();
	if ( i > 0 ) {
	    QPen pen = pt->pen();
	    QFont font = pt->font();
	    while ( i-- ) {
		QDomNode n = attr.item( i );
		QString a = n.nodeName();
		QString val = n.nodeValue().lower().stripWhiteSpace();
		setStyleProperty( a, val, &pen, &font, &curr->textalign );
	    }
	    pt->setPen( pen );
	    pt->setFont( font );
	}

	int x1, y1, x2, y2, rx, ry, w, h;
	double cx1, cy1, crx, cry;
	switch ( t ) {
	case CommentElement:
	    // ignore
	    break;
	case RectElement:
	    rx = ry = 0;
	    x1 = lenToInt( attr, "x" );
	    y1 = lenToInt( attr, "y" );
	    w = lenToInt( attr, "width" );
	    h = lenToInt( attr, "height" );
	    if ( w == 0 || h == 0 )	// prevent div by zero below
		break;
	    x2 = (int)attr.contains( "rx" ); // tiny abuse of x2 and y2
	    y2 = (int)attr.contains( "ry" );
	    if ( x2 )
		rx = lenToInt( attr, "rx" );
	    if ( y2 )
		ry = lenToInt( attr, "ry" );
	    if ( x2 && !y2 )
		ry = rx;
	    else if ( !x2 && y2 )
		rx = ry;
	    rx = int(200.0*double(rx)/double(w));
	    ry = int(200.0*double(ry)/double(h));
	    pt->drawRoundRect( x1, y1, w, h, rx, ry );
	    break;
	case CircleElement:
	    cx1 = lenToDouble( attr, "cx" ) + 0.5;
	    cy1 = lenToDouble( attr, "cy" ) + 0.5;
	    crx = lenToDouble( attr, "r" );
	    pt->drawEllipse( (int)(cx1-crx), (int)(cy1-crx), (int)(2*crx), (int)(2*crx) );
	    break;
	case EllipseElement:
	    cx1 = lenToDouble( attr, "cx" ) + 0.5;
	    cy1 = lenToDouble( attr, "cy" ) + 0.5;
	    crx = lenToDouble( attr, "rx" );
	    cry = lenToDouble( attr, "ry" );
	    pt->drawEllipse( (int)(cx1-crx), (int)(cy1-cry), (int)(2*crx), (int)(2*cry) );
	    break;
	case LineElement:
	    {
		x1 = lenToInt( attr, "x1" );
		x2 = lenToInt( attr, "x2" );
		y1 = lenToInt( attr, "y1" );
		y2 = lenToInt( attr, "y2" );
		QPen p = pt->pen();
		w = p.width();
		p.setWidth( (unsigned int)(w * (QABS(pt->worldMatrix().m11()) + QABS(pt->worldMatrix().m22())) / 2) );
		pt->setPen( p );
		pt->drawLine( x1, y1, x2, y2 );
		p.setWidth( w );
		pt->setPen( p );
	    }
	    break;
	case PolylineElement:
	case PolygonElement:
	    {
		QString pts = attr.namedItem( "points" ).nodeValue();
		pts = pts.simplifyWhiteSpace();
		QStringList sl = QStringList::split( QRegExp( QString::fromLatin1("[ ,]") ), pts );
		QPointArray ptarr( (uint)sl.count() / 2);
		for ( int i = 0; i < (int)sl.count() / 2; i++ ) {
		    double dx = sl[2*i].toDouble();
		    double dy = sl[2*i+1].toDouble();
		    ptarr.setPoint( i, int(dx), int(dy) );
		}
  		if ( t == PolylineElement ) {
		    if ( pt->brush().style() != Qt::NoBrush ) {
			QPen pn = pt->pen();
			pt->setPen( Qt::NoPen );
			pt->drawPolygon( ptarr );
			pt->setPen( pn );
		    }
		    pt->drawPolyline( ptarr ); // ### closes when filled. bug ?
  		} else {
		    pt->drawPolygon( ptarr );
		}
	    }
	    break;
	case SvgElement:
	case GroupElement:
	case AnchorElement:
	    {
		QDomNode child = node.firstChild();
		while ( !child.isNull() ) {
		    play( child );
		    child = child.nextSibling();
		}
	    }
	    break;
	case PathElement:
	    drawPath( attr.namedItem( "d" ).nodeValue() );
	    break;
	case TSpanElement:
	case TextElement:
	    {
		if ( attr.contains( "x" ) )
		     curr->textx = lenToInt( attr, "x" );
		if ( attr.contains( "y" ) )
		     curr->texty = lenToInt( attr, "y" );
		if ( t == TSpanElement ) {
		    curr->textx += lenToInt( attr, "dx" );
		    curr->texty += lenToInt( attr, "dy" );
		}
		// backup old colors
		QPen pn = pt->pen();
		QColor pcolor = pn.color();
		QColor bcolor = pt->brush().color();
		QDomNode c = node.firstChild();
		while ( !c.isNull() ) {
		    if ( c.isText() ) {
			// we have pen and brush reversed for text drawing
			pn.setColor( bcolor );
			pt->setPen( pn );
			QString text = c.toText().nodeValue();
			text = text.simplifyWhiteSpace(); // ### 'preserve'
			w = pt->fontMetrics().width( text );
			if ( curr->textalign == Qt::AlignHCenter )
			    curr->textx -= w / 2;
			else if ( curr->textalign == Qt::AlignRight )
			    curr->textx -= w;
			pt->drawText( curr->textx, curr->texty, text );
			// restore pen
			pn.setColor( pcolor );
			pt->setPen( pn );
			curr->textx += w;
		    } else if ( c.isElement() &&
				c.toElement().tagName() == "tspan" ) {
			play( c );

		    }
		    c = c.nextSibling();
		}
		if ( t == TSpanElement ) {
		    // move current text position in parent text element
		    StateList::Iterator it = --d->stack.fromLast();
		    (*it).textx = curr->textx;
		    (*it).texty = curr->texty;
		}
	    }
	    break;
	case ImageElement:
	    {
		x1 = lenToInt( attr, "x" );
		y1 = lenToInt( attr, "y" );
		w = lenToInt( attr, "width" );
		h = lenToInt( attr, "height" );
		QString href = attr.namedItem( "xlink:href" ).nodeValue();
		// ### catch references to embedded .svg files
		QPixmap pix;
		if ( !pix.load( href ) ) {
		    qWarning( "QSvgDevice::play: Couldn't load image "+href );
		    break;
		}
		pt->drawPixmap( QRect( x1, y1, w, h ), pix );
	    }
	    break;
	case DescElement:
	case TitleElement:
	    // ignored for now
	    break;
	case ClipElement:
	    {
		QDomNode child = node.firstChild();
                QRegion region;
                while (!child.isNull()) {
                    QDomNamedNodeMap childAttr = child.attributes();
                    if ( child.nodeName() == "rect" ) {
                        QRect r;
                        r.setX(lenToInt( childAttr, "x" ));
                        r.setY(lenToInt( childAttr, "y" ));
                        r.setWidth(lenToInt( childAttr, "width" ));
                        r.setHeight(lenToInt( childAttr, "height" ));
                        region |= r;
                    } else if ( child.nodeName() == "ellipse" ) {
                        QRect r;
                        int x = lenToInt( childAttr, "cx" );
                        int y = lenToInt( childAttr, "cy" );
                        int width = lenToInt( childAttr, "rx" );
                        int height = lenToInt( childAttr, "ry" );
                        r.setX( x - width );
                        r.setY( y - height );
                        r.setWidth( width * 2 );
                        r.setHeight( height * 2 );
                        QRegion rgn( r, QRegion::Ellipse );
                        region |= rgn;
                    }
                    child = child.nextSibling();
                }
                // Store the region in a named map so that it can be used when the
                // group node is entered.
                QString idString = attr.namedItem("id").nodeValue();
                if (!idString.isEmpty())
                    d->clipPathTable[idString] = region;
                break;
	    }
	case InvalidElement:
	    qWarning( "QSvgDevice::play: unknown element type " +
		      node.nodeName() );
	    break;
	};

	restoreAttributes();

    return TRUE;
}

/*!
    \internal

    Parses a CSS2-compatible color specification. Either a keyword or
    a numerical RGB specification like #ff00ff or rgb(255,0,50%).
*/

QColor QSvgDevice::parseColor( const QString &col )
{
    static const struct ColorTable {
	const char *name;
	const char *rgb;
    } coltab[] = {
	{ "black",   "#000000" },
	{ "silver",  "#c0c0c0" },
	{ "gray",    "#808080" },
	{ "white",   "#ffffff" },
	{ "maroon",  "#800000" },
	{ "red",     "#ff0000" },
	{ "purple",  "#800080" },
	{ "fuchsia", "#ff00ff" },
	{ "green",   "#008000" },
	{ "lime",    "#00ff00" },
	{ "olive",   "#808000" },
	{ "yellow",  "#ffff00" },
	{ "navy",    "#000080" },
	{ "blue",    "#0000ff" },
	{ "teal",    "#008080" },
	{ "aqua",    "#00ffff" },
	// ### the latest spec has more
	{ 0,         0         }
    };

    // initialize color map on first use
    if ( !qSvgColMap ) {
	qSvgColMap = new QMap<QString, QString>;
	const struct ColorTable *t = coltab;
	while ( t->name ) {
	    qSvgColMap->insert( t->name, t->rgb );
	    t++;
	}
    }

    // a keyword ?
    if ( qSvgColMap->contains ( col ) )
	return QColor( (*qSvgColMap)[ col ] );
    // in rgb(r,g,b) form ?
    QString c = col;
    c.replace( QRegExp( QString::fromLatin1("\\s*") ), "" );
    QRegExp reg( QString::fromLatin1("^rgb\\((\\d+)(%?),(\\d+)(%?),(\\d+)(%?)\\)$") );
    if ( reg.search( c ) >= 0 ) {
	int comp[3];
	for ( int i = 0; i < 3; i++ ) {
	    comp[ i ] = reg.cap( 2*i+1 ).toInt();
	    if ( !reg.cap( 2*i+2 ).isEmpty() )		// percentage ?
		comp[ i ] = int((double(255*comp[ i ])/100.0));
	}
	return QColor( comp[ 0 ], comp[ 1 ], comp[ 2 ] );
    }

    // check for predefined Qt color objects, #RRGGBB and #RGB
    return QColor( col );
}

/*!
    \internal

    Parse a <length> datatype consisting of a number followed by an
    optional unit specifier. Can be used for type <coordinate> as
    well. For relative units the value of \a horiz will determine
    whether the horizontal or vertical dimension will be used.
*/

double QSvgDevice::parseLen( const QString &str, bool *ok, bool horiz ) const
{
    QRegExp reg( QString::fromLatin1("([+-]?\\d*\\.*\\d*[Ee]?[+-]?\\d*)(em|ex|px|%|pt|pc|cm|mm|in|)$") );
    if ( reg.search( str ) == -1 ) {
	qWarning( "QSvgDevice::parseLen: couldn't parse " + str );
	if ( ok )
	    *ok = FALSE;
	return 0.0;
    }

    double dbl = reg.cap( 1 ).toDouble();
    QString u = reg.cap( 2 );
    if ( !u.isEmpty() && u != "px" ) {
	QPaintDeviceMetrics m( pt->device() );
	if ( u == "em" ) {
	    QFontInfo fi( pt->font() );
	    dbl *= fi.pixelSize();
	} else if ( u == "ex" ) {
	    QFontInfo fi( pt->font() );
	    dbl *= 0.5 * fi.pixelSize();
	} else if ( u == "%" )
	    dbl *= (horiz ? pt->window().width() : pt->window().height())/100.0;
	else if ( u == "cm" )
	    dbl *= m.logicalDpiX() / 2.54;
	else if ( u == "mm" )
	    dbl *= m.logicalDpiX() / 25.4;
	else if ( u == "in" )
	    dbl *= m.logicalDpiX();
	else if ( u == "pt" )
	    dbl *= m.logicalDpiX() / 72.0;
	else if ( u == "pc" )
	    dbl *= m.logicalDpiX() / 6.0;
	else
	    qWarning( "QSvgDevice::parseLen: Unknown unit " + u );
    }
    if ( ok )
	*ok = TRUE;
    return dbl;
}

/*!
    \internal

    Returns the length specified in attribute \a attr in \a map. If
    the specified attribute doesn't exist or can't be parsed \a def is
    returned.
*/

int QSvgDevice::lenToInt( const QDomNamedNodeMap &map, const QString &attr,
			  int def ) const
{
    if ( map.contains( attr ) ) {
	bool ok;
	double dbl = parseLen( map.namedItem( attr ).nodeValue(), &ok );
	if ( ok )
	    return qRound( dbl );
    }
    return def;
}

double QSvgDevice::lenToDouble( const QDomNamedNodeMap &map, const QString &attr,
				int def ) const
{
    if ( map.contains( attr ) ) {
	bool ok;
	double d = parseLen( map.namedItem( attr ).nodeValue(), &ok );
	if ( ok )
	    return d;
    }
    return def;
}

void QSvgDevice::setStyleProperty( const QString &prop, const QString &val,
				   QPen *pen, QFont *font, int *talign )
{
    if ( prop == "stroke" ) {
	if ( val == "none" ) {
	    pen->setStyle( Qt::NoPen );
	} else {
	    pen->setColor( parseColor( val ));
	    if ( pen->style() == Qt::NoPen )
		pen->setStyle( Qt::SolidLine );
	    if ( pen->width() == 0 )
		pen->setWidth( 1 );
	}
    } else if ( prop == "stroke-width" ) {
	double w = parseLen( val );
	if ( w > 0.0001 )
	    pen->setWidth( int(w) );
	else
	    pen->setStyle( Qt::NoPen );
    } else if ( prop == "stroke-linecap" ) {
	if ( val == "butt" )
	    pen->setCapStyle( Qt::FlatCap );
	else if ( val == "round" )
	    pen->setCapStyle( Qt::RoundCap );
	else if ( val == "square" )
	    pen->setCapStyle( Qt::SquareCap );
    } else if ( prop == "stroke-linejoin" ) {
	if ( val == "miter" )
	    pen->setJoinStyle( Qt::MiterJoin );
	else if ( val == "round" )
	    pen->setJoinStyle( Qt::RoundJoin );
	else if ( val == "bevel" )
	    pen->setJoinStyle( Qt::BevelJoin );
    } else if ( prop == "stroke-dasharray" ) {
        if ( val == "18,6" )
	    pen->setStyle( Qt::DashLine );
	else if ( val == "3" )
	    pen->setStyle( Qt::DotLine );
	else if ( val == "9,6,3,6" )
	    pen->setStyle( Qt::DashDotLine );
	else if ( val == "9,3,3" )
	    pen->setStyle( Qt::DashDotDotLine );
        else
            pen->setStyle( Qt::DotLine );
    } else if ( prop == "fill" ) {
	if ( val == "none" )
	    pt->setBrush( Qt::NoBrush );
	else
	    pt->setBrush( parseColor( val ) );
    } else if ( prop == "font-size" ) {
	font->setPointSizeFloat( float(parseLen( val )) );
    } else if ( prop == "font-family" ) {
	font->setFamily( val );
    } else if ( prop == "font-style" ) {
	if ( val == "normal" )
	    font->setItalic( FALSE );
	else if ( val == "italic" )
	    font->setItalic( TRUE );
	else
	    qWarning( "QSvgDevice::setStyleProperty: unhandled "
		      "font-style: %s", val.latin1() );
    } else if ( prop == "font-weight" ) {
	int w = font->weight();
	// no exact equivalents so we have to "round" a little bit
	if ( val == "100" || val == "200" )
	    w = QFont::Light;
	if ( val == "300" || val == "400" || val == "normal" )
	    w = QFont::Normal;
	else if ( val == "500" || val == "600" )
	    w = QFont::DemiBold;
	else if ( val == "700" || val == "bold" || val == "800" )
	    w = QFont::Bold;
	else if ( val == "900" )
	    w = QFont::Black;
	font->setWeight( w );
    } else if ( prop == "text-anchor" ) {
	if ( val == "middle" )
	    *talign = Qt::AlignHCenter;
	else if ( val == "end" )
	    *talign = Qt::AlignRight;
	else
	    *talign = Qt::AlignLeft;
    } else if ( prop == "clip-path" ) {
        if (val.startsWith("url(#")) {
            QString clipName = val.mid(5, val.length() - 6);
            if (!clipName.isEmpty()) {
                QRegion clipRegion = d->clipPathTable[clipName];
                if (!clipRegion.isEmpty())
                    pt->setClipRegion(pt->clipRegion() & clipRegion, QPainter::CoordPainter);
            }
        }
    }
}

void QSvgDevice::setStyle( const QString &s )
{
    QStringList rules = QStringList::split( QChar(';'), s );

    QPen pen = pt->pen();
    QFont font = pt->font();

    QStringList::ConstIterator it = rules.begin();
    for ( ; it != rules.end(); it++ ) {
	int col = (*it).find( ':' );
	if ( col > 0 ) {
	    QString prop = (*it).left( col ).simplifyWhiteSpace();
	    QString val = (*it).right( (*it).length() - col - 1 );
	    val = val.lower().stripWhiteSpace();
	    setStyleProperty( prop, val, &pen, &font, &curr->textalign );
	}
    }

    pt->setPen( pen );
    pt->setFont( font );
}

void QSvgDevice::setTransform( const QString &tr )
{
    QString t = tr.simplifyWhiteSpace();

    QRegExp reg( QString::fromLatin1("\\s*([\\w]+)\\s*\\(([^\\(]*)\\)") );
    int index = 0;
    while ( (index = reg.search(t, index)) >= 0 ) {
	QString command = reg.cap( 1 );
	QString params = reg.cap( 2 );
	QStringList plist = QStringList::split( QRegExp(QString::fromLatin1("[,\\s]")), params );
	if ( command == "translate" ) {
	    double tx = 0, ty = 0;
	    tx = plist[0].toDouble();
	    if ( plist.count() >= 2 )
		ty = plist[1].toDouble();
	    pt->translate( tx, ty );
	} else if ( command == "rotate" ) {
	    pt->rotate( plist[0].toDouble() );
	} else if ( command == "scale" ) {
	    double sx, sy;
	    sx = sy = plist[0].toDouble();
	    if ( plist.count() >= 2 )
		sy = plist[1].toDouble();
	    pt->scale( sx, sy );
	} else if ( command == "matrix" && plist.count() >= 6 ) {
	    double m[ 6 ];
	    for (int i = 0; i < 6; i++)
		m[ i ] = plist[ i ].toDouble();
	    QWMatrix wm( m[ 0 ], m[ 1 ], m[ 2 ],
			 m[ 3 ], m[ 4 ], m[ 5 ] );
	    pt->setWorldMatrix( wm, TRUE );
	} else if ( command == "skewX" ) {
	    pt->shear( 0.0, tan( plist[0].toDouble() * deg2rad ) );
	} else if ( command == "skewY" ) {
	    pt->shear( tan( plist[0].toDouble() * deg2rad ), 0.0 );
	}

	// move on to next command
	index += reg.matchedLength();
    }
}

void QSvgDevice::drawPath( const QString &data )
{
    double x0 = 0, y0 = 0;		// starting point
    double x = 0, y = 0;		// current point
    double controlX = 0, controlY = 0;	// last control point for curves
    QPointArray path( 500 );		// resulting path
    QValueList<int> subIndex;		// start indices for subpaths
    QPointArray quad( 4 ), bezier;	// for curve calculations
    int pcount = 0;			// current point array index
    uint idx = 0;			// current data position
    int mode = 0, lastMode = 0;		// parser state
    bool relative = FALSE;		// e.g. 'h' vs. 'H'
    QString commands( "MZLHVCSQTA" );	// recognized commands
    int cmdArgs[] = { 2, 0, 2, 1, 1, 6, 4, 4, 2, 7 };	// no of arguments
    QRegExp reg( QString::fromLatin1("\\s*,?\\s*([+-]?\\d*\\.?\\d*)") );	// floating point

    subIndex.append( 0 );
    // detect next command
    while ( idx < data.length() ) {
	QChar ch = data[ (int)idx++ ];
	if ( ch.isSpace() )
	    continue;
	QChar chUp = ch.upper();
	int cmd = commands.find( chUp );
	if ( cmd >= 0 ) {
	    // switch to new command mode
	    mode = cmd;
	    relative = ( ch != chUp );		// e.g. 'm' instead of 'M'
	} else {
	    if ( mode && !ch.isLetter() ) {
		cmd = mode;			// continue in previous mode
		idx--;
	    } else {
		qWarning( "QSvgDevice::drawPath: Unknown command" );
		return;
	    }
	}

	// read in the required number of arguments
	const int maxArgs = 7;
	double arg[ maxArgs ];
	int numArgs = cmdArgs[ cmd ];
	for ( int i = 0; i < numArgs; i++ ) {
	    int pos = reg.search( data, idx );
	    if ( pos == -1 ) {
		qWarning( "QSvgDevice::drawPath: Error parsing arguments" );
		return;
	    }
	    arg[ i ] = reg.cap( 1 ).toDouble();
	    idx = pos + reg.matchedLength();
	};

	// process command
	double offsetX = relative ? x : 0;	// correction offsets
	double offsetY = relative ? y : 0;	// for relative commands
	switch ( mode ) {
	case 0:					// 'M' move to
	    if ( x != x0 || y != y0 )
		path.setPoint( pcount++, int(x0), int(y0) );
	    x = x0 = arg[ 0 ] + offsetX;
	    y = y0 = arg[ 1 ] + offsetY;
	    subIndex.append( pcount );
	    path.setPoint( pcount++, int(x0), int(y0) );
	    mode = 2;				// -> 'L'
	    break;
	case 1:					// 'Z' close path
	    path.setPoint( pcount++, int(x0), int(y0) );
	    x = x0;
	    y = y0;
	    mode = 0;
	    break;
	case 2:					// 'L' line to
	    x = arg[ 0 ] + offsetX;
	    y = arg[ 1 ] + offsetY;
	    path.setPoint( pcount++, int(x), int(y) );
	    break;
	case 3:					// 'H' horizontal line
	    x = arg[ 0 ] + offsetX;
	    path.setPoint( pcount++, int(x), int(y) );
	    break;
	case 4:					// 'V' vertical line
	    y = arg[ 0 ] + offsetY;
	    path.setPoint( pcount++, int(x), int(y) );
	    break;
#ifndef QT_NO_BEZIER
	case 5:					// 'C' cubic bezier curveto
	case 6:					// 'S' smooth shorthand
	case 7:					// 'Q' quadratic bezier curves
	case 8: {				// 'T' smooth shorthand
	    quad.setPoint( 0, int(x), int(y) );
	    // if possible, reflect last control point if smooth shorthand
	    if ( mode == 6 || mode == 8 ) { 	// smooth 'S' and 'T'
		bool cont = mode == lastMode ||
		     mode == 6 && lastMode == 5 || 	// 'S' and 'C'
		     mode == 8 && lastMode == 7;	// 'T' and 'Q'
		x = cont ? 2*x-controlX : x;
		y = cont ? 2*y-controlY : y;
		quad.setPoint( 1, int(x), int(y) );
		quad.setPoint( 2, int(x), int(y) );
	    }
	    for ( int j = 0; j < numArgs/2; j++ ) {
		x = arg[ 2*j   ] + offsetX;
		y = arg[ 2*j+1 ] + offsetY;
		quad.setPoint( j+4-numArgs/2, int(x), int(y) );
	    }
	    // remember last control point for next shorthand
	    controlX = quad[ 2 ].x();
	    controlY = quad[ 2 ].y();
	    // transform quadratic into cubic Bezier
	    if ( mode == 7 || mode == 8 ) {	// cubic 'Q' and 'T'
		int x31 = quad[0].x()+int(2.0*(quad[2].x()-quad[0].x())/3.0);
		int y31 = quad[0].y()+int(2.0*(quad[2].y()-quad[0].y())/3.0);
		int x32 = quad[2].x()+int(2.0*(quad[3].x()-quad[2].x())/3.0);
		int y32 = quad[2].y()+int(2.0*(quad[3].y()-quad[2].y())/3.0);
		quad.setPoint( 1, x31, y31 );
		quad.setPoint( 2, x32, y32 );
	    }
	    // calculate points on curve
	    bezier = quad.cubicBezier();
	    // reserve more space if needed
	    if ( bezier.size() > path.size() - pcount )
		path.resize( path.size() - pcount + bezier.size() );
	    // copy
	    for ( int k = 0; k < (int)bezier.size(); k ++ )
		path.setPoint( pcount++, bezier[ k ] );
	    break;
	}
#endif // QT_NO_BEZIER
	case 9:					// 'A' elliptical arc curve
	    // ### just a straight line
	    x = arg[ 5 ] + offsetX;
	    y = arg[ 6 ] + offsetY;
	    path.setPoint( pcount++, int(x), int(y) );
	    break;
	};
	lastMode = mode;
	// array almost full ? expand for next loop
	if ( pcount >= (int)path.size() - 4 )
	    path.resize( 2 * path.size() );
    }

    subIndex.append( pcount );			// dummy marking the end
    if ( pt->brush().style() != Qt::NoBrush ) {
	// fill the area without stroke first
	if ( x != x0 || y != y0 )
	    path.setPoint( pcount++, int(x0), int(y0) );
	QPen pen = pt->pen();
	pt->setPen( Qt::NoPen );
	pt->drawPolygon( path, FALSE, 0, pcount );
	pt->setPen( pen );
    }
    // draw each subpath stroke seperately
    QValueListConstIterator<int> it = subIndex.begin();
    int start = 0;
    while ( it != subIndex.fromLast() ) {
	int next = *++it;
	// ### always joins ends if first and last point coincide.
	// ### 'Z' can't have the desired effect
	pt->drawPolyline( path, start, next-start );
	start = next;
    }
}

void QSvgDevice::applyStyle( QDomElement *e, int c ) const
{
    // ### do not write every attribute each time
    QColor pcol = pt->pen().color();
    QColor bcol = pt->brush().color();
    QString s;
    if ( c == PdcDrawText2 || c == PdcDrawText2Formatted ) {
	// QPainter has a reversed understanding of pen/stroke vs.
	// brush/fill for text
	s += QString( "fill:rgb(%1,%2,%3);" )
	     .arg( pcol.red() ).arg( pcol.green() ).arg( pcol.blue() );
	s += QString( "stroke-width:0;" );
	QFont f = pt->font();
	QFontInfo fi( f );
	s += QString( "font-size:%1;" ).arg( fi.pointSize() );
	s += QString( "font-style:%1;" )
	     .arg( f.italic() ? "italic" : "normal" );
	// not a very scientific distribution
	QString fw;
	if ( f.weight() <= QFont::Light )
	    fw = "100";
	else if ( f.weight() <= QFont::Normal )
	    fw = "400";
	else if ( f.weight() <= QFont::DemiBold )
	    fw = "600";
	else if ( f.weight() <= QFont::Bold )
	    fw = "700";
	else if ( f.weight() <= QFont::Black )
	    fw = "800";
	else
	    fw = "900";
	s += QString( "font-weight:%1;" ).arg( fw );
	s += QString( "font-family:%1;" ).arg( f.family() );
    } else {
	s += QString( "stroke:rgb(%1,%2,%3);" )
	     .arg( pcol.red() ).arg( pcol.green() ).arg( pcol.blue() );
	double pw = pt->pen().width();
	if ( pw == 0 && pt->pen().style() != Qt::NoPen )
	    pw = 0.9;
	if ( c == PdcDrawLine )
	    pw /= (QABS(pt->worldMatrix().m11()) + QABS(pt->worldMatrix().m22())) / 2.0;
	s += QString( "stroke-width:%1;" ).arg( pw );
	if ( pt->pen().style() == Qt::DashLine )
	    s+= QString( "stroke-dasharray:18,6;" );
	else if ( pt->pen().style() == Qt::DotLine )
	    s+= QString( "stroke-dasharray:3;" );
	else if ( pt->pen().style() == Qt::DashDotLine )
	    s+= QString( "stroke-dasharray:9,6,3,6;" );
	else if ( pt->pen().style() == Qt::DashDotDotLine )
	    s+= QString( "stroke-dasharray:9,3,3;" );
	if ( pt->brush().style() == Qt::NoBrush || c == PdcDrawPolyline ||
	     c == PdcDrawCubicBezier )
	    s += "fill:none;"; // Qt polylines use no brush, neither do Beziers
	else
	    s += QString( "fill:rgb(%1,%2,%3);" )
		 .arg( bcol.red() ).arg( bcol.green() ).arg( bcol.blue() );
    }
    e->setAttribute( "style", s );
}

void QSvgDevice::applyTransform( QDomElement *e ) const
{
    QWMatrix m = pt->worldMatrix();

    QString s;
    bool rot = ( m.m11() != 1.0 || m.m12() != 0.0 ||
		 m.m21() != 0.0 || m.m22() != 1.0 );
    if ( !rot && ( m.dx() != 0.0 || m.dy() != 0.0 ) )
	s = QString( "translate(%1,%2)" ).arg( m.dx() ).arg( m.dy() );
    else if ( rot ) {
	if ( m.m12() == 0.0 && m.m21() == 0.0 &&
	     m.dx() == 0.0 && m.dy() == 0.0 )
	    s = QString( "scale(%1,%2)" ).arg( m.m11() ).arg( m.m22() );
	else
	    s = QString( "matrix(%1,%2,%3,%4,%5,%6)" )
		.arg( m.m11() ).arg( m.m12() )
		.arg( m.m21() ).arg( m.m22() )
		.arg( m.dx() ).arg( m.dy() );
    }
    else
	return;

    e->setAttribute( "transform", s );
}

#endif // QT_NO_SVG
