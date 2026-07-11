/**********************************************************************
** $Id: qlabel.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QLabel widget class
**
** Created : 941215
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qlabel.h"
#ifndef QT_NO_LABEL
#include "qpainter.h"
#include "qdrawutil.h"
#include "qaccel.h"
#include "qmovie.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qpicture.h"
#include "qapplication.h"
#include "qsimplerichtext.h"
#include "qstylesheet.h"
#include "qstyle.h"

class QLabelPrivate
{
public:
    QLabelPrivate()
	:img(0), pix(0), valid_hints( -1 )
    {}
    QImage* img; // for scaled contents
    QPixmap* pix; // for scaled contents
    QSize sh;
    QSize msh;
    int valid_hints; // stores the frameWidth() for the stored size hint, -1 otherwise
};


/*!
    \class QLabel qlabel.h
    \brief The QLabel widget provides a text or image display.

    \ingroup basic
    \ingroup text
    \mainclass

    QLabel is used for displaying text or an image. No user
    interaction functionality is provided. The visual appearance of
    the label can be configured in various ways, and it can be used
    for specifying a focus accelerator key for another widget.

    A QLabel can contain any of the following content types:
    \table
    \header \i Content \i Setting
    \row \i Plain text
	 \i Pass a QString to setText().
    \row \i Rich text
	 \i Pass a QString that contains rich text to setText().
    \row \i A pixmap
	 \i Pass a QPixmap to setPixmap().
    \row \i A movie
	 \i Pass a QMovie to setMovie().
    \row \i A number
	 \i Pass an \e int or a \e double to setNum(), which converts
	    the number to plain text.
    \row \i Nothing
	 \i The same as an empty plain text. This is the default. Set
	    by clear().
    \endtable

    When the content is changed using any of these functions, any
    previous content is cleared.

    The look of a QLabel can be tuned in several ways. All the
    settings of QFrame are available for specifying a widget frame.
    The positioning of the content within the QLabel widget area can
    be tuned with setAlignment() and setIndent(). For example, this
    code sets up a sunken panel with a two-line text in the bottom
    right corner (both lines being flush with the right side of the
    label):
    \code
    QLabel *label = new QLabel( this );
    label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    label->setText( "first line\nsecond line" );
    label->setAlignment( AlignBottom | AlignRight );
    \endcode

    A QLabel is often used as a label for an interactive widget. For
    this use QLabel provides a useful mechanism for adding an
    accelerator key (see QAccel) that will set the keyboard focus to
    the other widget (called the QLabel's "buddy"). For example:
    \code
    QLineEdit* phoneEdit = new QLineEdit( this, "phoneEdit" );
    QLabel* phoneLabel = new QLabel( phoneEdit, "&Phone:", this, "phoneLabel" );
    \endcode

    In this example, keyboard focus is transferred to the label's
    buddy (the QLineEdit) when the user presses Alt+P. You can
    also use the setBuddy() function to accomplish the same thing.

    <img src=qlabel-m.png> <img src=qlabel-w.png>

    \sa QLineEdit, QTextEdit, QPixmap, QMovie,
    \link guibooks.html#fowler GUI Design Handbook: Label\endlink
*/

/*!
    \fn QPicture * QLabel::picture() const

    Returns the label's picture or 0 if the label doesn't have a
    picture.
*/


/*!
    Constructs an empty label.

    The \a parent, \a name and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setAlignment(), setFrameStyle(), setIndent()
*/

QLabel::QLabel( QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f | WMouseNoMask  )
{
    init();
}


/*!
    Constructs a label that displays the text, \a text.

    The \a parent, \a name and widget flag \a f, arguments are passed
    to the QFrame constructor.

    \sa setText(), setAlignment(), setFrameStyle(), setIndent()
*/

QLabel::QLabel( const QString &text, QWidget *parent, const char *name,
		WFlags f )
	: QFrame( parent, name, f | WMouseNoMask  )
{
    init();
    setText( text );
}


/*!
    Constructs a label that displays the text \a text. The label has a
    buddy widget, \a buddy.

    If the \a text contains an underlined letter (a letter preceded by
    an ampersand, \&), and the text is in plain text format, when the
    user presses Alt+ the underlined letter, focus is passed to the
    buddy widget.

    The \a parent, \a name and widget flag, \a f, arguments are passed
    to the QFrame constructor.

    \sa setText(), setBuddy(), setAlignment(), setFrameStyle(),
    setIndent()
*/
QLabel::QLabel( QWidget *buddy,  const QString &text,
		QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f | WMouseNoMask )
{
    init();
#ifndef QT_NO_ACCEL
    setBuddy( buddy );
#endif
    setText( text );
}

/*!
    Destroys the label.
*/

QLabel::~QLabel()
{
    clearContents();
    delete d;
}


void QLabel::init()
{
    lpixmap = 0;
#ifndef QT_NO_MOVIE
    lmovie = 0;
#endif
#ifndef QT_NO_ACCEL
    lbuddy = 0;
    accel = 0;
#endif
    lpixmap = 0;
#ifndef QT_NO_PICTURE
    lpicture = 0;
#endif
    align = AlignAuto | AlignVCenter | ExpandTabs;
    extraMargin = -1;
    autoresize = FALSE;
    scaledcontents = FALSE;
    textformat = Qt::AutoText;
#ifndef QT_NO_RICHTEXT
    doc = 0;
#endif

    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    d = new QLabelPrivate;
}


/*!
    \property QLabel::text
    \brief the label's text

    If no text has been set this will return an empty string. Setting
    the text clears any previous content, unless they are the same.

    The text will be interpreted either as a plain text or as a rich
    text, depending on the text format setting; see setTextFormat().
    The default setting is \c AutoText, i.e. QLabel will try to
    auto-detect the format of the text set.

    If the text is interpreted as a plain text and a buddy has been
    set, the buddy accelerator key is updated from the new text.

    The label resizes itself if auto-resizing is enabled.

    Note that Qlabel is well-suited to display small rich text
    documents, i.e. those small documents that get their document
    specific settings (font, text color, link color) from the label's
    palette and font properties. For large documents, use QTextEdit
    in read-only mode instead. QTextEdit will flicker less on resize
    and can also provide a scrollbar when necessary.

    \sa text, setTextFormat(), setBuddy(), alignment
*/

void QLabel::setText( const QString &text )
{
    if ( ltext == text )
	return;
    QSize osh = sizeHint();
#ifndef QT_NO_RICHTEXT
    bool hadRichtext = doc != 0;
#endif
    clearContents();
    ltext = text;
#ifndef QT_NO_RICHTEXT
    bool useRichText = (textformat == RichText ||
      ( ( textformat == AutoText ) && QStyleSheet::mightBeRichText(ltext) ) );
#else
    bool useRichText = TRUE;
#endif
#ifndef QT_NO_ACCEL
    // ### Setting accelerators for rich text labels will not work.
    // Eg. <b>&gt;Hello</b> will return ALT+G which is clearly
    // not intended.
    if ( !useRichText ) {
	int p = QAccel::shortcutKey( ltext );
	if ( p ) {
	    if ( !accel )
		accel = new QAccel( this, "accel label accel" );
	    accel->connectItem( accel->insertItem( p ),
				this, SLOT(acceleratorSlot()) );
	}
    }
#endif
#ifndef QT_NO_RICHTEXT
    if ( useRichText ) {
	if ( !hadRichtext )
	    align |= WordBreak;
	QString t = ltext;
	if ( align & AlignRight )
	    t.prepend( "<div align=\"right\">");
	else if ( align & AlignHCenter )
	    t.prepend( "<div align=\"center\">");
	if ( (align & WordBreak) == 0  )
	    t.prepend( "<nobr>" );
	doc = new QSimpleRichText( t, font() );
    }
#endif

    updateLabel( osh );
}


/*!
    Clears any label contents. Equivalent to setText( "" ).
*/

void QLabel::clear()
{
    setText( QString::fromLatin1("") );
}

/*!
    \property QLabel::pixmap
    \brief the label's pixmap

    If no pixmap has been set this will return an invalid pixmap.

    Setting the pixmap clears any previous content, and resizes the
    label if \l QLabel::autoResize() is TRUE. The buddy accelerator,
    if any, is disabled.
*/
void QLabel::setPixmap( const QPixmap &pixmap )
{
    QSize osh = sizeHint();

    if ( !lpixmap || lpixmap->serialNumber() != pixmap.serialNumber() ) {
	clearContents();
	lpixmap = new QPixmap( pixmap );
    }

    if ( lpixmap->depth() == 1 && !lpixmap->mask() )
	lpixmap->setMask( *((QBitmap *)lpixmap) );

    updateLabel( osh );
}

#ifndef QT_NO_PICTURE
/*!
    Sets the label contents to \a picture. Any previous content is
    cleared.

    The buddy accelerator, if any, is disabled.

    \sa picture(), setBuddy()
*/

void QLabel::setPicture( const QPicture &picture )
{
    QSize osh = sizeHint();
    clearContents();
    lpicture = new QPicture( picture );

    updateLabel( osh );
}
#endif // QT_NO_PICTURE

/*!
    Sets the label contents to plain text containing the textual
    representation of integer \a num. Any previous content is cleared.
    Does nothing if the integer's string representation is the same as
    the current contents of the label.

    The buddy accelerator, if any, is disabled.

    The label resizes itself if auto-resizing is enabled.

    \sa setText(), QString::setNum(), setBuddy()
*/

void QLabel::setNum( int num )
{
    QString str;
    str.setNum( num );
	setText( str );
}

/*!
    \overload

    Sets the label contents to plain text containing the textual
    representation of double \a num. Any previous content is cleared.
    Does nothing if the double's string representation is the same as
    the current contents of the label.

    The buddy accelerator, if any, is disabled.

    The label resizes itself if auto-resizing is enabled.

    \sa setText(), QString::setNum(), setBuddy()
*/

void QLabel::setNum( double num )
{
    QString str;
    str.setNum( num );
	setText( str );
}

/*!
    \property QLabel::alignment
    \brief the alignment of the label's contents

    The alignment is a bitwise OR of \c Qt::AlignmentFlags and \c
    Qt::TextFlags values. The \c ExpandTabs, \c SingleLine and \c
    ShowPrefix flags apply only if the label contains plain text;
    otherwise they are ignored. The \c DontClip flag is always
    ignored. \c WordBreak applies to both rich text and plain text
    labels. The \c BreakAnywhere flag is not supported in QLabel.

    If the label has a buddy, the \c ShowPrefix flag is forced to
    TRUE.

    The default alignment is \c{AlignAuto | AlignVCenter | ExpandTabs}
    if the label doesn't have a buddy and \c{AlignAuto | AlignVCenter
    | ExpandTabs | ShowPrefix} if the label has a buddy. If the label
    contains rich text, additionally \c WordBreak is turned on.

    \sa Qt::AlignmentFlags, alignment, setBuddy(), text
*/

void QLabel::setAlignment( int alignment )
{
    if ( alignment == align )
	return;
    QSize osh = sizeHint();
#ifndef QT_NO_ACCEL
    if ( lbuddy )
	align = alignment | ShowPrefix;
    else
#endif
	align = alignment;

#ifndef QT_NO_RICHTEXT
    QString t = ltext;
    if ( !t.isNull() ) {
	ltext = QString::null;
	setText( t );
    }
#endif

    updateLabel( osh );
}


/*!
    \property QLabel::indent
    \brief the label's text indent in pixels

    If a label displays text, the indent applies to the left edge if
    alignment() is \c AlignLeft, to the right edge if alignment() is
    \c AlignRight, to the top edge if alignment() is \c AlignTop, and
    to to the bottom edge if alignment() is \c AlignBottom.

    If indent is negative, or if no indent has been set, the label
    computes the effective indent as follows: If frameWidth() is 0,
    the effective indent becomes 0. If frameWidth() is greater than 0,
    the effective indent becomes half the width of the "x" character
    of the widget's current font().

    \sa alignment, frameWidth(), font()
*/

void QLabel::setIndent( int indent )
{
    extraMargin = indent;
    updateLabel( QSize( -1, -1 ) );
}


/*!
  \fn bool QLabel::autoResize() const

  \obsolete

  Returns TRUE if auto-resizing is enabled, or FALSE if auto-resizing
  is disabled.

  Auto-resizing is disabled by default.

  \sa setAutoResize()
*/

/*! \obsolete
  Enables auto-resizing if \a enable is TRUE, or disables it if \a
  enable is FALSE.

  When auto-resizing is enabled the label will resize itself to fit
  the contents whenever the contents change. The top-left corner is
  not moved. This is useful for QLabel widgets that are not managed by
  a QLayout (e.g., top-level widgets).

  Auto-resizing is disabled by default.

  \sa autoResize(), adjustSize(), sizeHint()
*/

void QLabel::setAutoResize( bool enable )
{
    if ( (bool)autoresize != enable ) {
	autoresize = enable;
	if ( autoresize )
	    adjustSize();			// calls resize which repaints
    }
}



/*!
    Returns the size that will be used if the width of the label is \a
    w. If \a w is -1, the sizeHint() is returned.
*/

QSize QLabel::sizeForWidth( int w ) const
{
    QRect br;
    QPixmap *pix = pixmap();
#ifndef QT_NO_PICTURE
    QPicture *pic = picture();
#else
    const int pic = 0;
#endif
#ifndef QT_NO_MOVIE
    QMovie *mov = movie();
#else
    const int mov = 0;
#endif
    int hextra = 2 * frameWidth();
    int vextra = hextra;
    QFontMetrics fm( fontMetrics() );
    int xw = fm.width( 'x' );
    if ( !mov && !pix && !pic ) {
	int m = indent();
	if ( m < 0 && hextra ) // no indent, but we do have a frame
	    m = xw / 2 - margin();
	if ( m >= 0 ) {
	    int horizAlign = QApplication::horizontalAlignment( align );
	    if ( (horizAlign & AlignLeft) || (horizAlign & AlignRight ) )
		hextra += m;
	    if ( (align & AlignTop) || (align & AlignBottom ) )
		vextra += m;
	}
    }

    if ( pix )
	br = pix->rect();
#ifndef QT_NO_PICTURE
    else if ( pic )
	br = pic->boundingRect();
#endif
#ifndef QT_NO_MOVIE
    else if ( mov )
	br = mov->framePixmap().rect();
#endif
#ifndef QT_NO_RICHTEXT
    else if ( doc ) {
	int oldW = doc->width();
	if ( align & WordBreak ) {
	    if ( w < 0 )
		doc->adjustSize();
	    else
		doc->setWidth( w-hextra );
	}
	br = QRect( 0, 0, doc->widthUsed(), doc->height() );
	doc->setWidth( oldW );
    }
#endif
    else {
	bool tryWidth = (w < 0) && (align & WordBreak);
	if ( tryWidth )
	    w = xw * 80;
	else if ( w < 0 )
	    w = 2000;
	w -= hextra;
	br = fm.boundingRect( 0, 0, w ,2000, alignment(), text() );
	if ( tryWidth && br.height() < 4*fm.lineSpacing() && br.width() > w/2 )
		br = fm.boundingRect( 0, 0, w/2, 2000, alignment(), text() );
	if ( tryWidth && br.height() < 2*fm.lineSpacing() && br.width() > w/4 )
	    br = fm.boundingRect( 0, 0, w/4, 2000, alignment(), text() );
    }
    int wid = br.width() + hextra;
    int hei = br.height() + vextra;

    return QSize( wid, hei );
}


/*!
  \reimp
*/

int QLabel::heightForWidth( int w ) const
{
    if (
#ifndef QT_NO_RICHTEXT
	doc ||
#endif
	(align & WordBreak) )
	return sizeForWidth( w ).height();
    return QWidget::heightForWidth( w );
}



/*!\reimp
*/
QSize QLabel::sizeHint() const
{
    if ( d->valid_hints != frameWidth() )
	(void) QLabel::minimumSizeHint();
    return d->sh;
}

/*!
  \reimp
*/

QSize QLabel::minimumSizeHint() const
{
    if ( d->valid_hints == frameWidth() )
	return d->msh;

    constPolish();
    d->valid_hints = frameWidth();
    d->sh = sizeForWidth( -1 );
    QSize sz( -1, -1 );

    if (
#ifndef QT_NO_RICHTEXT
	 !doc &&
#endif
	 (align & WordBreak) == 0 ) {
	sz = d->sh;
    } else {
	// think about caching these for performance
	sz.rwidth() = sizeForWidth( 0 ).width();
	sz.rheight() = sizeForWidth(QWIDGETSIZE_MAX).height();
	if ( d->sh.height() < sz.height() )
	    sz.rheight() = d->sh.height();
    }
    if ( sizePolicy().horData() == QSizePolicy::Ignored )
	sz.rwidth() = -1;
    if ( sizePolicy().verData() == QSizePolicy::Ignored )
	sz.rheight() = -1;
    d->msh = sz;
    return sz;
}

/*!
  \reimp
*/
void QLabel::resizeEvent( QResizeEvent* e )
{
    QFrame::resizeEvent( e );

#ifdef QT_NO_RICHTEXT
    static const bool doc = FALSE;
#endif

    // optimize for standard labels
    if ( frameShape() == NoFrame && (align & WordBreak) == 0 && !doc &&
	 ( e->oldSize().width() >= e->size().width() && (align & AlignLeft ) == AlignLeft )
	 && ( e->oldSize().height() >= e->size().height() && (align & AlignTop ) == AlignTop ) ) {
	setWFlags( WResizeNoErase );
	return;
    }

    clearWFlags( WResizeNoErase );
    QRect cr = contentsRect();
    if ( !lpixmap ||  !cr.isValid() ||
	 // masked pixmaps can only reduce flicker when being top/left
	 // aligned and when we do not perform scaled contents
	 ( lpixmap->hasAlpha() && ( scaledcontents || ( ( align & (AlignLeft|AlignTop) ) != (AlignLeft|AlignTop) ) ) ) )
	return;

    setWFlags( WResizeNoErase );

    if ( !scaledcontents ) {
	// don't we all love QFrame? Reduce pixmap flicker
	QRegion reg = QRect( QPoint(0, 0), e->size() );
	reg = reg.subtract( cr );
	int x = cr.x();
	int y = cr.y();
	int w = lpixmap->width();
	int h = lpixmap->height();
	if ( (align & Qt::AlignVCenter) == Qt::AlignVCenter )
	    y += cr.height()/2 - h/2;
	else if ( (align & Qt::AlignBottom) == Qt::AlignBottom)
	    y += cr.height() - h;
	if ( (align & Qt::AlignRight) == Qt::AlignRight )
	    x += cr.width() - w;
	else if ( (align & Qt::AlignHCenter) == Qt::AlignHCenter )
	    x += cr.width()/2 - w/2;
	if ( x > cr.x() )
	    reg = reg.unite( QRect( cr.x(), cr.y(), x - cr.x(), cr.height() ) );
	if ( y > cr.y() )
	    reg = reg.unite( QRect( cr.x(), cr.y(), cr.width(), y - cr.y() ) );

	if ( x + w < cr.right() )
	    reg = reg.unite( QRect( x + w, cr.y(),  cr.right() - x - w, cr.height() ) );
	if ( y + h < cr.bottom() )
	    reg = reg.unite( QRect( cr.x(), y +  h, cr.width(), cr.bottom() - y - h ) );

	erase( reg );
    }
}


/*!
    Draws the label contents using the painter \a p.
*/

void QLabel::drawContents( QPainter *p )
{
    QRect cr = contentsRect();

    QPixmap *pix = pixmap();
#ifndef QT_NO_PICTURE
    QPicture *pic = picture();
#else
    const int pic = 0;
#endif
#ifndef QT_NO_MOVIE
    QMovie *mov = movie();
#else
    const int mov = 0;
#endif

    if ( !mov && !pix && !pic ) {
	int m = indent();
	if ( m < 0 && frameWidth() ) // no indent, but we do have a frame
	    m = fontMetrics().width('x') / 2 - margin();
	if ( m > 0 ) {
	    int hAlign = QApplication::horizontalAlignment( align );
	    if ( hAlign & AlignLeft )
		cr.setLeft( cr.left() + m );
	    if ( hAlign & AlignRight )
		cr.setRight( cr.right() - m );
	    if ( align & AlignTop )
		cr.setTop( cr.top() + m );
	    if ( align & AlignBottom )
		cr.setBottom( cr.bottom() - m );
	}
    }

#ifndef QT_NO_MOVIE
    if ( mov ) {
	// ### should add movie to qDrawItem
	QRect r = style().itemRect( p, cr, align, isEnabled(), &(mov->framePixmap()),
				    QString::null );
	// ### could resize movie frame at this point
	p->drawPixmap(r.x(), r.y(), mov->framePixmap() );
    }
    else
#endif
#ifndef QT_NO_RICHTEXT
    if ( doc ) {
	doc->setWidth(p, cr.width() );
	int rh = doc->height();
	int yo = 0;
	if ( align & AlignVCenter )
	    yo = (cr.height()-rh)/2;
	else if ( align & AlignBottom )
	    yo = cr.height()-rh;
	if (! isEnabled() &&
	    style().styleHint(QStyle::SH_EtchDisabledText, this)) {
	    QColorGroup cg = colorGroup();
	    cg.setColor( QColorGroup::Text, cg.light() );
	    doc->draw(p, cr.x()+1, cr.y()+yo+1, cr, cg, 0);
	}

	// QSimpleRichText always draws with QColorGroup::Text as with
	// background mode PaletteBase. QLabel typically has
	// background mode PaletteBackground, so we create a temporary
	// color group with the text color adjusted.
	QColorGroup cg = colorGroup();
	if ( backgroundMode() != PaletteBase && isEnabled() )
	    cg.setColor( QColorGroup::Text, paletteForegroundColor() );

	doc->draw(p, cr.x(), cr.y()+yo, cr, cg, 0);
    } else
#endif
#ifndef QT_NO_PICTURE
    if ( pic ) {
	QRect br = pic->boundingRect();
	int rw = br.width();
	int rh = br.height();
	if ( scaledcontents ) {
	    p->save();
	    p->translate( cr.x(), cr.y() );
#ifndef QT_NO_TRANSFORMATIONS
	    p->scale( (double)cr.width()/rw, (double)cr.height()/rh );
#endif
	    p->drawPicture( -br.x(), -br.y(), *pic );
	    p->restore();
	} else {
	    int xo = 0;
	    int yo = 0;
	    if ( align & AlignVCenter )
		yo = (cr.height()-rh)/2;
	    else if ( align & AlignBottom )
		yo = cr.height()-rh;
	    if ( align & AlignRight )
		xo = cr.width()-rw;
	    else if ( align & AlignHCenter )
		xo = (cr.width()-rw)/2;
	    p->drawPicture( cr.x()+xo-br.x(), cr.y()+yo-br.y(), *pic );
	}
    } else
#endif
    {
#ifndef QT_NO_IMAGE_SMOOTHSCALE
	if ( scaledcontents && pix ) {
	    if ( !d->img )
		d->img = new QImage( lpixmap->convertToImage() );

	    if ( !d->pix )
		d->pix = new QPixmap;
	    if ( d->pix->size() != cr.size() )
		d->pix->convertFromImage( d->img->smoothScale( cr.width(), cr.height() ) );
	    pix = d->pix;
	}
#endif
	int alignment = align;
	if ((align & ShowPrefix) && !style().styleHint(QStyle::SH_UnderlineAccelerator, this))
	    alignment |= NoAccel;
	// ordinary text or pixmap label
	style().drawItem( p, cr, alignment, colorGroup(), isEnabled(),
			  pix, ltext );
    }
}


/*!
    Updates the label, but not the frame.
*/

void QLabel::updateLabel( QSize oldSizeHint )
{
    d->valid_hints = -1;
    QSizePolicy policy = sizePolicy();
    bool wordBreak = align & WordBreak;
    policy.setHeightForWidth( wordBreak );
    if ( policy != sizePolicy() )
	setSizePolicy( policy );
    if ( sizeHint() != oldSizeHint )
	updateGeometry();
    if ( autoresize ) {
	adjustSize();
	update( contentsRect() );
    } else {
	update( contentsRect() );
    }
}


/*!
  \internal

  Internal slot, used to set focus for accelerator labels.
*/
#ifndef QT_NO_ACCEL
void QLabel::acceleratorSlot()
{
    if ( !lbuddy )
	return;
    QWidget * w = lbuddy;
    while ( w->focusProxy() )
	w = w->focusProxy();
    if ( !w->hasFocus() &&
	 w->isEnabled() &&
	 w->isVisible() &&
	 w->focusPolicy() != NoFocus ) {
	QFocusEvent::setReason( QFocusEvent::Shortcut );
	w->setFocus();
	QFocusEvent::resetReason();
    }
}
#endif

/*!
  \internal

  Internal slot, used to clean up if the buddy widget dies.
*/
#ifndef QT_NO_ACCEL
void QLabel::buddyDied() // I can't remember if I cried.
{
    lbuddy = 0;
}

/*!
    Sets this label's buddy to \a buddy.

    When the user presses the accelerator key indicated by this label,
    the keyboard focus is transferred to the label's buddy widget.

    The buddy mechanism is only available for QLabels that contain
    plain text in which one letter is prefixed with an ampersand, \&.
    This letter is set as the accelerator key. The letter is displayed
    underlined, and the '\&' is not displayed (i.e. the \c ShowPrefix
    alignment flag is turned on; see setAlignment()).

    In a dialog, you might create two data entry widgets and a label
    for each, and set up the geometry layout so each label is just to
    the left of its data entry widget (its "buddy"), for example:
    \code
    QLineEdit *nameEd  = new QLineEdit( this );
    QLabel    *nameLb  = new QLabel( "&Name:", this );
    nameLb->setBuddy( nameEd );
    QLineEdit *phoneEd = new QLineEdit( this );
    QLabel    *phoneLb = new QLabel( "&Phone:", this );
    phoneLb->setBuddy( phoneEd );
    // ( layout setup not shown )
    \endcode

    With the code above, the focus jumps to the Name field when the
    user presses Alt+N, and to the Phone field when the user presses
    Alt+P.

    To unset a previously set buddy, call this function with \a buddy
    set to 0.

    \sa buddy(), setText(), QAccel, setAlignment()
*/

void QLabel::setBuddy( QWidget *buddy )
{
    if ( buddy )
	setAlignment( alignment() | ShowPrefix );
    else
	setAlignment( alignment() & ~ShowPrefix );

    if ( lbuddy )
	disconnect( lbuddy, SIGNAL(destroyed()), this, SLOT(buddyDied()) );

    lbuddy = buddy;

    if ( !lbuddy )
	return;
#ifndef QT_NO_RICHTEXT
    if ( !( textformat == RichText || (textformat == AutoText &&
				       QStyleSheet::mightBeRichText(ltext) ) ) )
#endif
    {
	int p = QAccel::shortcutKey( ltext );
	if ( p ) {
	    if ( !accel )
		accel = new QAccel( this, "accel label accel" );
	    accel->connectItem( accel->insertItem( p ),
				this, SLOT(acceleratorSlot()) );
	}
    }

    connect( lbuddy, SIGNAL(destroyed()), this, SLOT(buddyDied()) );
}


/*!
    Returns this label's buddy, or 0 if no buddy is currently set.

    \sa setBuddy()
*/

QWidget * QLabel::buddy() const
{
    return lbuddy;
}
#endif //QT_NO_ACCEL


#ifndef QT_NO_MOVIE
void QLabel::movieUpdated(const QRect& rect)
{
    QMovie *mov = movie();
    if ( mov && !mov->isNull() ) {
	QRect r = contentsRect();
	r = style().itemRect( 0, r, align, isEnabled(), &(mov->framePixmap()),
			      QString::null );
	r.moveBy(rect.x(), rect.y());
	r.setWidth(QMIN(r.width(), rect.width()));
	r.setHeight(QMIN(r.height(), rect.height()));
	repaint( r, mov->framePixmap().mask() != 0 );
    }
}

void QLabel::movieResized( const QSize& size )
{
    d->valid_hints = -1;
    if ( autoresize )
	adjustSize();
    movieUpdated( QRect( QPoint(0,0), size ) );
    updateGeometry();
}

/*!
    Sets the label contents to \a movie. Any previous content is
    cleared.

    The buddy accelerator, if any, is disabled.

    The label resizes itself if auto-resizing is enabled.

    \sa movie(), setBuddy()
*/

void QLabel::setMovie( const QMovie& movie )
{
    QSize osh = sizeHint();
    clearContents();

    lmovie = new QMovie( movie );
	lmovie->connectResize(this, SLOT(movieResized(const QSize&)));
	lmovie->connectUpdate(this, SLOT(movieUpdated(const QRect&)));

    if ( !lmovie->running() )	// Assume that if the movie is running,
	updateLabel( osh );	// resize/update signals will come soon enough
}

#endif // QT_NO_MOVIE

/*!
  \internal

  Clears any contents, without updating/repainting the label.
*/

void QLabel::clearContents()
{
#ifndef QT_NO_RICHTEXT
    delete doc;
    doc = 0;
#endif

    delete lpixmap;
    lpixmap = 0;
#ifndef QT_NO_PICTURE
    delete lpicture;
    lpicture = 0;
#endif
    delete d->img;
    d->img = 0;
    delete d->pix;
    d->pix = 0;

    ltext = QString::null;
#ifndef QT_NO_ACCEL
    if ( accel )
	accel->clear();
#endif
#ifndef QT_NO_MOVIE
    if ( lmovie ) {
	lmovie->disconnectResize(this, SLOT(movieResized(const QSize&)));
	lmovie->disconnectUpdate(this, SLOT(movieUpdated(const QRect&)));
	delete lmovie;
	lmovie = 0;
    }
#endif
}


#ifndef QT_NO_MOVIE

/*!
    Returns a pointer to the label's movie, or 0 if no movie has been
    set.

    \sa setMovie()
*/

QMovie* QLabel::movie() const
{
    return lmovie;
}

#endif  // QT_NO_MOVIE

/*!
    \property QLabel::backgroundMode
    \brief the label's background mode

    Get this property with backgroundMode().

    \sa QWidget::setBackgroundMode()
*/

/*!
    \property QLabel::textFormat
    \brief the label's text format

    See the \c Qt::TextFormat enum for an explanation of the possible
    options.

    The default format is \c AutoText.

    \sa text
*/

Qt::TextFormat QLabel::textFormat() const
{
    return textformat;
}

void QLabel::setTextFormat( Qt::TextFormat format )
{
    if ( format != textformat ) {
	textformat = format;
	QString t = ltext;
	if ( !t.isNull() ) {
	    ltext = QString::null;
	    setText( t );
	}
    }
}

/*!
  \reimp
*/

void QLabel::fontChange( const QFont & )
{
    if ( !ltext.isEmpty() ) {
#ifndef QT_NO_RICHTEXT
	if ( doc )
	    doc->setDefaultFont( font() );
#endif
	updateLabel( QSize( -1, -1 ) );
    }
}

#ifndef QT_NO_IMAGE_SMOOTHSCALE
/*!
    \property QLabel::scaledContents
    \brief whether the label will scale its contents to fill all
    available space.

    When enabled and the label shows a pixmap, it will scale the
    pixmap to fill the available space.

    This property's default is FALSE.

    \sa setScaledContents()
*/
bool QLabel::hasScaledContents() const
{
    return scaledcontents;
}

void QLabel::setScaledContents( bool enable )
{
    if ( (bool)scaledcontents == enable )
	return;
    scaledcontents = enable;
    if ( !enable ) {
	delete d->img;
	d->img = 0;
	delete d->pix;
	d->pix = 0;
    }
    update( contentsRect() );
}

#endif // QT_NO_IMAGE_SMOOTHSCALE

/*!
    Sets the font used on the QLabel to font \a f.
*/

void QLabel::setFont( const QFont &f )
{
    QFrame::setFont( f );
}

#endif // QT_NO_LABEL
