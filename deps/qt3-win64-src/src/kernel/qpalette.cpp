/****************************************************************************
** $Id: qpalette.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QColorGroup and QPalette classes
**
** Created : 950323
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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

#include "qpalette.h"

#ifndef QT_NO_PALETTE
#include "qdatastream.h"
#include "qcleanuphandler.h"

/*****************************************************************************
  QColorGroup member functions
 *****************************************************************************/

/*!
    \class QColorGroup qpalette.h
    \brief The QColorGroup class contains a group of widget colors.

    \ingroup appearance
    \ingroup graphics
    \ingroup images

    A color group contains a group of colors used by widgets for
    drawing themselves. We recommend that widgets use color group
    roles such as "foreground" and "base" rather than literal colors
    like "red" or "turquoise". The color roles are enumerated and
    defined in the \l ColorRole documentation.

    The most common use of QColorGroup is like this:

    \code
	QPainter p;
	...
	p.setPen( colorGroup().foreground() );
	p.drawLine( ... )
    \endcode

    It is also possible to modify color groups or create new color
    groups from scratch.

    The color group class can be created using three different
    constructors or by modifying one supplied by Qt. The default
    constructor creates an all-black color group, which can then be
    modified using set functions; there's also a constructor for
    specifying all the color group colors. And there is also a copy
    constructor.

    We strongly recommend using a system-supplied color group and
    modifying that as necessary.

    You modify a color group by calling the access functions
    setColor() and setBrush(), depending on whether you want a pure
    color or a pixmap pattern.

    There are also corresponding color() and brush() getters, and a
    commonly used convenience function to get each ColorRole:
    background(), foreground(), base(), etc.

    \sa QColor QPalette QWidget::colorGroup()
*/


/*!
    \enum QColorGroup::ColorRole

    The ColorRole enum defines the different symbolic color roles used
    in current GUIs.

    The central roles are:

    \value Background  general background color.

    \value Foreground  general foreground color.

    \value Base  used as background color for text entry widgets, for example;
    usually white or another light color.

    \value Text  the foreground color used with \c Base. Usually this
    is the same as the \c Foreground, in which case it must provide good
    contrast with \c Background and \c Base.

    \value Button  general button background color in which buttons need a
    background different from \c Background, as in the Macintosh style.

    \value ButtonText  a foreground color used with the \c Button color.

    There are some color roles used mostly for 3D bevel and shadow
    effects:

    \value Light  lighter than \c Button color.

    \value Midlight  between \c Button and \c Light.

    \value Dark  darker than \c Button.

    \value Mid  between \c Button and \c Dark.

    \value Shadow  a very dark color.
    By default, the shadow color is \c Qt::black.

    All of these are normally derived from \c Background and used in
    ways that depend on that relationship. For example, buttons depend
    on it to make the bevels look attractive, and Motif scroll bars
    depend on \c Mid to be slightly different from \c Background.

    Selected (marked) items have two roles:

    \value Highlight   a color to indicate a selected item or the
    current item. By default, the highlight color is \c Qt::darkBlue.

    \value HighlightedText  a text color that contrasts with \c Highlight.
    By default, the highlighted text color is \c Qt::white.

    Finally, there is a special role for text that needs to be
    drawn where \c Text or \c Foreground would give poor contrast,
    such as on pressed push buttons:

    \value BrightText a text color that is very different from \c
    Foreground and contrasts well with e.g. \c Dark.

    \value Link a text color used for unvisited hyperlinks.
    By default, the link color is \c Qt::blue.

    \value LinkVisited a text color used for already visited hyperlinks.
    By default, the linkvisited color is \c Qt::magenta.

    \value NColorRoles Internal.

    Note that text colors can be used for things other than just
    words; text colors are \e usually used for text, but it's quite
    common to use the text color roles for lines, icons, etc.

    This image shows most of the color roles in use:
    \img palette.png Color Roles
*/


class QColorGroupPrivate : public QShared
{
public:
    QBrush br[QColorGroup::NColorRoles];
    QColorGroupPrivate* detach() {
	if ( count > 1 ) {
	    deref();
	    QColorGroupPrivate* d = new QColorGroupPrivate;
	    for (int i=0; i<QColorGroup::NColorRoles; i++)
		d->br[i] = br[i];
	    return d;
	}
	return this;
    }
};

/*!
    Constructs a color group with all colors set to black.
*/

QColorGroup::QColorGroup()
{
    static QColorGroupPrivate* defColorGroupData = 0;
    if ( !defColorGroupData ) {
	static QSharedCleanupHandler<QColorGroupPrivate> defColorGroupCleanup;
	defColorGroupData = new QColorGroupPrivate;
	defColorGroupCleanup.set( &defColorGroupData );
    }
    d = defColorGroupData;
    br = d->br;
    d->ref();
}

/*!
    Constructs a color group that is an independent copy of \a other.
*/
QColorGroup::QColorGroup( const QColorGroup& other )
{
    d = other.d;
    d->ref();
    br = d->br;
}

/*!
    Copies the colors of \a other to this color group.
*/
QColorGroup& QColorGroup::operator =(const QColorGroup& other)
{
    if ( d != other.d ) {
	if ( d->deref() )
	    delete d;
	d = other.d;
	br = d->br;
	d->ref();
    }
    return *this;
}

static QColor qt_mix_colors( QColor a, QColor b)
{
    return QColor( (a.red() + b.red()) / 2, (a.green() + b.green()) / 2, (a.blue() + b.blue()) / 2 );
}


/*!
    Constructs a color group. You can pass either brushes, pixmaps or
    plain colors for \a foreground, \a button, \a light, \a dark, \a
    mid, \a text, \a bright_text, \a base and \a background.

    \sa QBrush
*/
 QColorGroup::QColorGroup( const QBrush &foreground, const QBrush &button,
			   const QBrush &light, const QBrush &dark,
			   const QBrush &mid, const QBrush &text,
			   const QBrush &bright_text, const QBrush &base,
			   const QBrush &background)
{
    d = new QColorGroupPrivate;
    br = d->br;
    br[Foreground]      = foreground;
    br[Button]		= button;
    br[Light]		= light;
    br[Dark]		= dark;
    br[Mid]		= mid;
    br[Text]		= text;
    br[BrightText]	= bright_text;
    br[ButtonText]	= text;
    br[Base]		= base;
    br[Background]	= background;
    br[Midlight]	= qt_mix_colors( br[Button].color(), br[Light].color() );
    br[Shadow]          = Qt::black;
    br[Highlight]       = Qt::darkBlue;
    br[HighlightedText] = Qt::white;
    br[Link]            = Qt::blue;
    br[LinkVisited]     = Qt::magenta;
}


/*!\obsolete

  Constructs a color group with the specified colors. The button
  color will be set to the background color.
*/

QColorGroup::QColorGroup( const QColor &foreground, const QColor &background,
			  const QColor &light, const QColor &dark,
			  const QColor &mid,
			  const QColor &text, const QColor &base )
{
    d = new QColorGroupPrivate;
    br = d->br;
    br[Foreground]      = QBrush(foreground);
    br[Button]          = QBrush(background);
    br[Light]           = QBrush(light);
    br[Dark]            = QBrush(dark);
    br[Mid]             = QBrush(mid);
    br[Text]            = QBrush(text);
    br[BrightText]      = br[Light];
    br[ButtonText]      = br[Text];
    br[Base]            = QBrush(base);
    br[Background]      = QBrush(background);
    br[Midlight]	= qt_mix_colors( br[Button].color(), br[Light].color() );
    br[Shadow]          = Qt::black;
    br[Highlight]       = Qt::darkBlue;
    br[HighlightedText] = Qt::white;
    br[Link]            = Qt::blue;
    br[LinkVisited]     = Qt::magenta;
}

/*!
    Destroys the color group.
*/

QColorGroup::~QColorGroup()
{
    if ( d->deref() )
	delete d;
}

/*!
    Returns the color that has been set for color role \a r.

    \sa brush() ColorRole
 */
const QColor &QColorGroup::color( ColorRole r ) const
{
    return br[r].color();
}

/*!
    Returns the brush that has been set for color role \a r.

    \sa color() setBrush() ColorRole
*/
const QBrush &QColorGroup::brush( ColorRole r ) const
{
    return br[r];
}

/*!
    Sets the brush used for color role \a r to a solid color \a c.

    \sa brush() setColor() ColorRole
*/
void QColorGroup::setColor( ColorRole r, const QColor &c )
{
    setBrush( r, QBrush(c) );
}

/*!
    Sets the brush used for color role \a r to \a b.

    \sa brush() setColor() ColorRole
*/
void QColorGroup::setBrush( ColorRole r, const QBrush &b )
{
    d = d->detach();
    br = d->br;
    br[r] = b;
}


/*!
    \fn const QColor & QColorGroup::foreground() const

    Returns the foreground color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::button() const

    Returns the button color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::light() const

    Returns the light color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor& QColorGroup::midlight() const

    Returns the midlight color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::dark() const

    Returns the dark color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::mid() const

    Returns the mid color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::text() const

    Returns the text foreground color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::brightText() const

    Returns the bright text foreground color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::buttonText() const

    Returns the button text foreground color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::base() const

    Returns the base color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::background() const

    Returns the background color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::shadow() const

    Returns the shadow color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::highlight() const

    Returns the highlight color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::highlightedText() const

    Returns the highlighted text color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::link() const

    Returns the unvisited link text color of the color group.

    \sa ColorRole
*/

/*!
    \fn const QColor & QColorGroup::linkVisited() const

    Returns the visited link text color of the color group.

    \sa ColorRole
*/

/*!
    \fn bool QColorGroup::operator!=( const QColorGroup &g ) const

    Returns TRUE if this color group is different from \a g; otherwise
    returns  FALSE.

    \sa operator!=()
*/

/*!
    Returns TRUE if this color group is equal to \a g; otherwise
    returns FALSE.

    \sa operator==()
*/

bool QColorGroup::operator==( const QColorGroup &g ) const
{
    if ( d == g.d )
	return TRUE;
    for( int r = 0 ; r < NColorRoles ; r++ )
	if ( br[r] != g.br[r] )
	    return FALSE;
    return TRUE;
}


/*****************************************************************************
  QPalette member functions
 *****************************************************************************/

/*!
    \class QPalette qpalette.h

    \brief The QPalette class contains color groups for each widget state.

    \ingroup appearance
    \ingroup shared
    \ingroup graphics
    \ingroup images
    \mainclass

    A palette consists of three color groups: \e active, \e disabled,
    and \e inactive. All widgets contain a palette, and all widgets in
    Qt use their palette to draw themselves. This makes the user
    interface easily configurable and easier to keep consistent.

    If you create a new widget we strongly recommend that you use the
    colors in the palette rather than hard-coding specific colors.

    The color groups:
    \list
    \i The active() group is used for the window that has keyboard focus.
    \i The inactive() group is used for other windows.
    \i The disabled() group is used for widgets (not windows) that are
    disabled for some reason.
    \endlist

    Both active and inactive windows can contain disabled widgets.
    (Disabled widgets are often called \e inaccessible or \e{grayed
    out}.)

    In Motif style, active() and inactive() look the same. In Windows
    2000 style and Macintosh Platinum style, the two styles look
    slightly different.

    There are setActive(), setInactive(), and setDisabled() functions
    to modify the palette. (Qt also supports a normal() group; this is
    an obsolete alias for active(), supported for backwards
    compatibility.)

    Colors and brushes can be set for particular roles in any of a
    palette's color groups with setColor() and setBrush().

    You can copy a palette using the copy constructor and test to see
    if two palettes are \e identical using isCopyOf().

    \sa QApplication::setPalette(), QWidget::setPalette(), QColorGroup, QColor
*/

/*!
    \enum QPalette::ColorGroup

    \value Disabled
    \value Active
    \value Inactive
    \value NColorGroups
    \value Normal synonym for Active
*/

/*!
    \obsolete

    \fn const QColorGroup &QPalette::normal() const

    Returns the active color group. Use active() instead.

    \sa setActive() active()
*/

/*!
    \obsolete

    \fn void QPalette::setNormal( const QColorGroup & cg )

    Sets the active color group to \a cg. Use setActive() instead.

    \sa setActive() active()
*/


static int palette_count = 1;

/*!
    Constructs a palette that consists of color groups with only black
    colors.
*/

QPalette::QPalette()
{
    static QPalData *defPalData = 0;
    if ( !defPalData ) {                // create common palette data
	defPalData = new QPalData;      //   for the default palette
	static QSharedCleanupHandler<QPalData> defPalCleanup;
	defPalCleanup.set( &defPalData );
	defPalData->ser_no = palette_count++;
    }
    data = defPalData;
    data->ref();
}

/*!\obsolete
  Constructs a palette from the \a button color. The other colors are
  automatically calculated, based on this color. Background will be
  the button color as well.
*/

QPalette::QPalette( const QColor &button )
{
    data = new QPalData;
    Q_CHECK_PTR( data );
    data->ser_no = palette_count++;
    QColor bg = button, btn = button, fg, base, disfg;
    int h, s, v;
    bg.hsv( &h, &s, &v );
    if ( v > 128 ) {				// light background
	fg   = Qt::black;
	base = Qt::white;
	disfg = Qt::darkGray;
    } else {					// dark background
	fg   = Qt::white;
	base = Qt::black;
	disfg = Qt::darkGray;
    }
    data->active   = QColorGroup( fg, btn, btn.light(150), btn.dark(),
				  btn.dark(150), fg, Qt::white, base, bg );
    data->disabled = QColorGroup( disfg, btn, btn.light(150), btn.dark(),
				  btn.dark(150), disfg, Qt::white, base, bg );
    data->inactive = data->active;
}

/*!
    Constructs a palette from a \a button color and a \a background.
    The other colors are automatically calculated, based on these
    colors.
*/

QPalette::QPalette( const QColor &button, const QColor &background )
{
    data = new QPalData;
    Q_CHECK_PTR( data );
    data->ser_no = palette_count++;
    QColor bg = background, btn = button, fg, base, disfg;
    int h, s, v;
    bg.hsv( &h, &s, &v );
    if ( v > 128 ) {				// light background
	fg   = Qt::black;
	base = Qt::white;
	disfg = Qt::darkGray;
    } else {					// dark background
	fg   = Qt::white;
	base = Qt::black;
	disfg = Qt::darkGray;
    }
    data->active   = QColorGroup( fg, btn, btn.light(150), btn.dark(),
				  btn.dark(150), fg, Qt::white, base, bg );
    data->disabled = QColorGroup( disfg, btn, btn.light(150), btn.dark(),
				  btn.dark(150), disfg, Qt::white, base, bg );
    data->inactive = data->active;
}

/*!
    Constructs a palette that consists of the three color groups \a
    active, \a disabled and \a inactive. See the \link #details
    Detailed Description\endlink for definitions of the color groups
    and \l QColorGroup::ColorRole for definitions of each color role
    in the three groups.

    \sa QColorGroup QColorGroup::ColorRole QPalette
*/

QPalette::QPalette( const QColorGroup &active, const QColorGroup &disabled,
		    const QColorGroup &inactive )
{
    data = new QPalData;
    Q_CHECK_PTR( data );
    data->ser_no = palette_count++;
    data->active = active;
    data->disabled = disabled;
    data->inactive = inactive;
}

/*!
    Constructs a copy of \a p.

    This constructor is fast (it uses copy-on-write).
*/

QPalette::QPalette( const QPalette &p )
{
    data = p.data;
    data->ref();
}

/*!
    Destroys the palette.
*/

QPalette::~QPalette()
{
    if ( data->deref() )
	delete data;
}

/*!
    Assigns \a p to this palette and returns a reference to this
    palette.

    This is fast (it uses copy-on-write).

    \sa copy()
*/

QPalette &QPalette::operator=( const QPalette &p )
{
    p.data->ref();
    if ( data->deref() )
	delete data;
    data = p.data;
    return *this;
}


/*!
    Returns the color in color group \a gr, used for color role \a r.

    \sa brush() setColor() QColorGroup::ColorRole
*/
const QColor &QPalette::color( ColorGroup gr, QColorGroup::ColorRole r ) const
{
    return directBrush( gr, r ).color();
}

/*!
    Returns the brush in color group \a gr, used for color role \a r.

    \sa color() setBrush() QColorGroup::ColorRole
*/
const QBrush &QPalette::brush( ColorGroup gr, QColorGroup::ColorRole r ) const
{
    return directBrush( gr, r );
}

/*!
    Sets the brush in color group \a gr, used for color role \a r, to
    the solid color \a c.

    \sa setBrush() color() QColorGroup::ColorRole
*/
void QPalette::setColor( ColorGroup gr, QColorGroup::ColorRole r,
			 const QColor &c)
{
    setBrush( gr, r, QBrush(c) );
}

/*!
    Sets the brush in color group \a gr, used for color role \a r, to
    \a b.

    \sa brush() setColor() QColorGroup::ColorRole
*/
void QPalette::setBrush( ColorGroup gr, QColorGroup::ColorRole r,
			 const QBrush &b)
{
    detach();
    data->ser_no = palette_count++;
    directSetBrush( gr, r, b);
}

/*!
    \overload

    Sets the brush color used for color role \a r to color \a c in all
    three color groups.

    \sa color() setBrush() QColorGroup::ColorRole
*/
void QPalette::setColor( QColorGroup::ColorRole r, const QColor &c )
{
    setBrush( r, QBrush(c) );
}

/*!
    \overload

    Sets the brush in for color role \a r in all three color groups to
    \a b.

    \sa brush() setColor() QColorGroup::ColorRole active() inactive() disabled()
*/
void QPalette::setBrush( QColorGroup::ColorRole r, const QBrush &b )
{
    detach();
    data->ser_no = palette_count++;
    directSetBrush( Active, r, b );
    directSetBrush( Disabled, r, b );
    directSetBrush( Inactive, r, b );
}


/*!
    Returns a deep copy of this palette.

    \warning This is slower than the copy constructor and assignment
    operator and offers no benefits.
*/

QPalette QPalette::copy() const
{
    QPalette p( data->active, data->disabled, data->inactive );
    return p;
}


/*!
    Detaches this palette from any other QPalette objects with which
    it might implicitly share QColorGroup objects. In essence, does
    the copying part of copy-on-write.

    Calling this should generally not be necessary; QPalette calls it
    itself when necessary.
*/

void QPalette::detach()
{
    if ( data->count != 1 )
	*this = copy();
}

/*!
    \fn const QColorGroup & QPalette::disabled() const

    Returns the disabled color group of this palette.

    \sa QColorGroup, setDisabled(), active(), inactive()
*/

/*!
    Sets the \c Disabled color group to \a g.

    \sa disabled() setActive() setInactive()
*/

void QPalette::setDisabled( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->disabled = g;
}

/*!
    \fn const QColorGroup & QPalette::active() const

    Returns the active color group of this palette.

    \sa QColorGroup, setActive(), inactive(), disabled()
*/

/*!
    Sets the \c Active color group to \a g.

    \sa active() setDisabled() setInactive() QColorGroup
*/

void QPalette::setActive( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->active = g;
}

/*!
    \fn const QColorGroup & QPalette::inactive() const

    Returns the inactive color group of this palette.

    \sa QColorGroup,  setInactive(), active(), disabled()
*/

/*!
    Sets the \c Inactive color group to \a g.

    \sa active() setDisabled() setActive() QColorGroup
*/

void QPalette::setInactive( const QColorGroup &g )
{
    detach();
    data->ser_no = palette_count++;
    data->inactive = g;
}


/*!
    \fn bool QPalette::operator!=( const QPalette &p ) const

    Returns TRUE (slowly) if this palette is different from \a p;
    otherwise returns FALSE (usually quickly).
*/

/*!
    Returns TRUE (usually quickly) if this palette is equal to \a p;
    otherwise returns FALSE (slowly).
*/

bool QPalette::operator==( const QPalette &p ) const
{
    return data->active == p.data->active &&
	   data->disabled == p.data->disabled &&
	   data->inactive == p.data->inactive;
}


/*!
    \fn int QPalette::serialNumber() const

    Returns a number that uniquely identifies this QPalette object.
    The serial number is intended for caching. Its value may not be
    used for anything other than equality testing.

    Note that QPalette uses copy-on-write, and the serial number
    changes during the lazy copy operation (detach()), not during a
    shallow copy (copy constructor or assignment).

    \sa QPixmap QPixmapCache QCache
*/


/*****************************************************************************
  QColorGroup/QPalette stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
    \relates QColorGroup

    Writes color group, \a g to the stream \a s.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QColorGroup &g )
{
    if ( s.version() == 1 ) {
	// Qt 1.x
	s << g.foreground()
	  << g.background()
	  << g.light()
	  << g.dark()
	  << g.mid()
	  << g.text()
	  << g.base();
    } else {
	int max = QColorGroup::NColorRoles;
	if ( s.version() <= 3) // Qt 2.x
	    max = 14;

	for( int r = 0 ; r < max ; r++ )
	    s << g.brush( (QColorGroup::ColorRole)r);
    }
    return s;
}

/*!
    \related QColorGroup

    Reads a color group from the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QColorGroup &g )
{
    if ( s.version() == 1 ) {
	// Qt 1.x
	QColor fg, bg, light, dark, mid, text, base;
	s >> fg >> bg >> light >> dark >> mid >> text >> base;
	QPalette p( bg );
	QColorGroup n( p.active() );
	n.setColor( QColorGroup::Foreground, fg );
	n.setColor( QColorGroup::Light, light );
	n.setColor( QColorGroup::Dark, dark );
	n.setColor( QColorGroup::Mid, mid );
	n.setColor( QColorGroup::Text, text );
	n.setColor( QColorGroup::Base, base );
	g = n;
    } else {
	int max = QColorGroup::NColorRoles;
	if (s.version() <= 3) // Qt 2.x
	    max = 14;

	QBrush tmp;
	for( int r = 0 ; r < max; r++ ) {
	    s >> tmp;
	    g.setBrush( (QColorGroup::ColorRole)r, tmp);
	}
    }
    return s;
}


/*!
    \relates QPalette

    Writes the palette, \a p to the stream \a s and returns a
    reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QPalette &p )
{
    return s << p.active()
	     << p.disabled()
	     << p.inactive();
}


static void readV1ColorGroup( QDataStream &s, QColorGroup &g,
		       QPalette::ColorGroup r )
{
    QColor fg, bg, light, dark, mid, text, base;
    s >> fg >> bg >> light >> dark >> mid >> text >> base;
    QPalette p( bg );
    QColorGroup n;
    switch ( r ) {
	case QPalette::Disabled:
	    n = p.disabled();
	    break;
	case QPalette::Inactive:
	    n = p.inactive();
	    break;
	default:
	    n = p.active();
	    break;
    }
    n.setColor( QColorGroup::Foreground, fg );
    n.setColor( QColorGroup::Light, light );
    n.setColor( QColorGroup::Dark, dark );
    n.setColor( QColorGroup::Mid, mid );
    n.setColor( QColorGroup::Text, text );
    n.setColor( QColorGroup::Base, base );
    g = n;
}


/*!
    \relates QPalette

    Reads a palette from the stream, \a s into the palette \a p, and
    returns a reference to the stream.

    \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QPalette &p )
{
    QColorGroup active, disabled, inactive;
    if ( s.version() == 1 ) {
	readV1ColorGroup( s, active, QPalette::Active );
	readV1ColorGroup( s, disabled, QPalette::Disabled );
	readV1ColorGroup( s, inactive, QPalette::Inactive );
    } else {
	s >> active >> disabled >> inactive;
    }
    QPalette newpal( active, disabled, inactive );
    p = newpal;
    return s;
}
#endif //QT_NO_DATASTREAM

/*!
    Returns TRUE if this palette and \a p are copies of each other,
    i.e. one of them was created as a copy of the other and neither
    was subsequently modified; otherwise returns FALSE. This is much
    stricter than equality.

    \sa operator=() operator==()
*/

bool QPalette::isCopyOf( const QPalette & p )
{
    return data && data == p.data;
}

const QBrush &QPalette::directBrush( ColorGroup gr, QColorGroup::ColorRole r ) const
{
    if ( (uint)gr > (uint)QPalette::NColorGroups ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPalette::directBrush: colorGroup(%i) out of range", gr );
#endif
	return data->active.br[QColorGroup::Foreground];
    }
    if ( (uint)r >= (uint)QColorGroup::NColorRoles ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPalette::directBrush: colorRole(%i) out of range", r );
#endif
	return data->active.br[QColorGroup::Foreground];
    }
    switch( gr ) {
    case Active:
	return data->active.br[r];
	//break;
    case Disabled:
	return data->disabled.br[r];
	//break;
    case Inactive:
	return data->inactive.br[r];
	//break;
    default:
	break;
    }
#if defined(QT_CHECK_RANGE)
    qWarning( "QPalette::directBrush: colorGroup(%i) internal error", gr );
#endif
    return data->active.br[QColorGroup::Foreground]; // Satisfy compiler
}

void QPalette::directSetBrush( ColorGroup gr, QColorGroup::ColorRole r, const QBrush& b)
{
    if ( (uint)gr > (uint)QPalette::NColorGroups ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPalette::directBrush: colorGroup(%i) out of range", gr );
#endif
	return;
    }
    if ( (uint)r >= (uint)QColorGroup::NColorRoles ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QPalette::directBrush: colorRole(%i) out of range", r );
#endif
	return;
    }
    switch( gr ) {
    case Active:
	data->active.setBrush(r,b);
	break;
    case Disabled:
	data->disabled.setBrush(r,b);
	break;
    case Inactive:
	data->inactive.setBrush(r,b);
	break;
    default:
#if defined(QT_CHECK_RANGE)
	qWarning( "QPalette::directBrush: colorGroup(%i) internal error", gr );
#endif
	break;
    }
}


/*!\internal*/
QColorGroup::ColorRole QPalette::foregroundRoleFromMode( Qt::BackgroundMode mode )
{
    switch (mode) {
    case Qt::PaletteButton:
	return QColorGroup::ButtonText;
    case Qt::PaletteBase:
	return QColorGroup::Text;
    case Qt::PaletteDark:
    case Qt::PaletteShadow:
	return QColorGroup::Light;
    case Qt::PaletteHighlight:
	return QColorGroup::HighlightedText;
    case Qt::PaletteBackground:
    default:
	return QColorGroup::Foreground;
    }
}

/*!\internal*/
QColorGroup::ColorRole QPalette::backgroundRoleFromMode( Qt::BackgroundMode mode)
{
    switch (mode) {
    case Qt::PaletteForeground:
	return QColorGroup::Foreground;
    case Qt::PaletteButton:
	return QColorGroup::Button;
    case Qt::PaletteLight:
	return QColorGroup::Light;
    case Qt::PaletteMidlight:
	return QColorGroup::Midlight;
    case Qt::PaletteDark:
	return QColorGroup::Dark;
    case Qt::PaletteMid:
	return QColorGroup::Mid;
    case Qt::PaletteText:
	return QColorGroup::Text;
    case Qt::PaletteBrightText:
	return QColorGroup::BrightText;
    case Qt::PaletteButtonText:
	return QColorGroup::ButtonText;
    case Qt::PaletteBase:
	return QColorGroup::Base;
    case Qt::PaletteShadow:
	return QColorGroup::Shadow;
    case Qt::PaletteHighlight:
	return QColorGroup::Highlight;
    case Qt::PaletteHighlightedText:
	return QColorGroup::HighlightedText;
    case Qt::PaletteLink:
	return QColorGroup::Link;
    case Qt::PaletteLinkVisited:
	return QColorGroup::LinkVisited;
    case Qt::PaletteBackground:
    default:
	return QColorGroup::Background;
    }
}

#endif // QT_NO_PALETTE
