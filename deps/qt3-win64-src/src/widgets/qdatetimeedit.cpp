/****************************************************************************
** $Id: qdatetimeedit.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of date and time edit classes
**
** Created : 001103
**
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
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
**********************************************************************/

#include "qdatetimeedit.h"

#ifndef QT_NO_DATETIMEEDIT

#include "../kernel/qinternal_p.h"
#include "../kernel/qrichtext_p.h"
#include "qrangecontrol.h"
#include "qapplication.h"
#include "qpixmap.h"
#include "qapplication.h"
#include "qvaluelist.h"
#include "qstring.h"
#include "qstyle.h"

#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif

#define QDATETIMEEDIT_HIDDEN_CHAR '0'

class Q_EXPORT QNumberSection
{
public:
    QNumberSection( int selStart = 0, int selEnd = 0, bool separat = TRUE, int actual = -1 )
	: selstart( selStart ), selend( selEnd ), act( actual ), sep( separat )
    {}
    int selectionStart() const { return selstart; }
    void setSelectionStart( int s ) { selstart = s; }
    int selectionEnd() const { return selend; }
    void setSelectionEnd( int s ) { selend = s; }
    int width() const { return selend - selstart; }
    int index() const { return act; }
    bool separator() const { return sep; }
    Q_DUMMY_COMPARISON_OPERATOR( QNumberSection )
private:
    int selstart :12;
    int selend	 :12;
    int act	 :7;
    bool sep	 :1;
};

static QString	*lDateSep = 0;
static QString	*lTimeSep = 0;
static bool	lAMPM	  = FALSE;
static QString	*lAM	  = 0;
static QString	*lPM	  = 0;
static QDateEdit::Order	lOrder = QDateEdit::YMD;
static int refcount = 0;

static void cleanup()
{
    delete lDateSep;
    lDateSep = 0;
    delete lTimeSep;
    lTimeSep = 0;
    delete lAM;
    lAM = 0;
    delete lPM;
    lPM = 0;
}

/*!
\internal
try to get the order of DMY and the date/time separator from the locale settings
*/
static void readLocaleSettings()
{
    int dpos, mpos, ypos;
    cleanup();

    lDateSep = new QString();
    lTimeSep = new QString();

#if defined(Q_WS_WIN)
    QT_WA( {
	TCHAR data[10];
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SDATE, data, 10 );
	*lDateSep = QString::fromUcs2( (ushort*)data );
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STIME, data, 10 );
	*lTimeSep = QString::fromUcs2( (ushort*)data );
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_ITIME, data, 10 );
	lAMPM = QString::fromUcs2( (ushort*)data ).toInt()==0;
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_S1159, data, 10 );
	QString am = QString::fromUcs2( (ushort*)data );
	if ( !am.isEmpty() )
	    lAM = new QString( am );
	GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_S2359, data, 10 );
	QString pm = QString::fromUcs2( (ushort*)data );
	if ( !pm.isEmpty()  )
	    lPM = new QString( pm );
    } , {
	char data[10];
	GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_SDATE, (char*)&data, 10 );
	*lDateSep = QString::fromLocal8Bit( data );
	GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_STIME, (char*)&data, 10 );
	*lTimeSep = QString::fromLocal8Bit( data );
	GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_ITIME, (char*)&data, 10 );
	lAMPM = QString::fromLocal8Bit( data ).toInt()==0;
	GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_S1159, (char*)&data, 10 );
	QString am = QString::fromLocal8Bit( data );
	if ( !am.isEmpty() )
	    lAM = new QString( am );
	GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_S2359, (char*)&data, 10 );
	QString pm = QString::fromLocal8Bit( data );
	if ( !pm.isEmpty() )
	    lPM = new QString( pm );
    } );
#else
    *lDateSep = "-";
    *lTimeSep = ":";
#endif
    QString d = QDate( 1999, 11, 22 ).toString( Qt::LocalDate );
    dpos = d.find( "22" );
    mpos = d.find( "11" );
    ypos = d.find( "99" );
    if ( dpos > -1 && mpos > -1 && ypos > -1 ) {
	// test for DMY, MDY, YMD, YDM
	if ( dpos < mpos && mpos < ypos ) {
	    lOrder = QDateEdit::DMY;
	} else if ( mpos < dpos && dpos < ypos ) {
	    lOrder = QDateEdit::MDY;
	} else if ( ypos < mpos && mpos < dpos ) {
	    lOrder = QDateEdit::YMD;
	} else if ( ypos < dpos && dpos < mpos ) {
	    lOrder = QDateEdit::YDM;
	} else {
	    // cannot determine the dateformat - use the default
	    return;
	}

	// this code needs to change if new formats are added

#ifndef Q_WS_WIN
	QString sep = d.mid( QMIN( dpos, mpos ) + 2, QABS( dpos - mpos ) - 2 );
	if ( d.contains( sep ) == 2 ) {
	    *lDateSep = sep;
	}
#endif
    }

#ifndef Q_WS_WIN
    QString t = QTime( 11, 22, 33 ).toString( Qt::LocalDate );
    dpos = t.find( "11" );
    mpos = t.find( "22" );
    ypos = t.find( "33" );
    // We only allow hhmmss
    if ( dpos > -1 && dpos < mpos && mpos < ypos ) {
	QString sep = t.mid( dpos + 2, mpos - dpos - 2 );
	if ( sep == t.mid( mpos + 2, ypos - mpos - 2 ) ) {
	    *lTimeSep = sep;
	}
    }
#endif
}

static QDateEdit::Order localOrder() {
    if ( !lDateSep ) {
	readLocaleSettings();
    }
    return lOrder;
}

static QString localDateSep() {
    if ( !lDateSep ) {
	readLocaleSettings();
    }
    return *lDateSep;
}

static QString localTimeSep() {
    if ( !lTimeSep ) {
	readLocaleSettings();
    }
    return *lTimeSep;
}

class QDateTimeEditorPrivate
{
public:
    QDateTimeEditorPrivate()
	: frm( TRUE ),
	  parag( new QTextParagraph( 0, 0, 0, FALSE ) ),
	  focusSec(0)
    {
	parag->formatter()->setWrapEnabled( FALSE );
	cursor = new QTextCursor( 0 );
	cursor->setParagraph( parag );
	offset = 0;
	sep = localDateSep();
	refcount++;
    }
    ~QDateTimeEditorPrivate()
    {
	delete parag;
	delete cursor;
	if ( !--refcount )
	    cleanup();
    }

    void appendSection( const QNumberSection& sec )
    {
	sections.append( sec );

    }
    void clearSections()
    {
	sections.clear();
    }
    void setSectionSelection( int sec, int selstart, int selend )
    {
	if ( sec < 0 || sec > (int)sections.count() )
	    return;
	sections[sec].setSelectionStart( selstart );
	sections[sec].setSelectionEnd( selend );
    }
    uint sectionCount() const { return (uint)sections.count(); }
    void setSeparator( const QString& s ) { sep = s; }
    QString separator() const { return sep; }

    void setFrame( bool f ) { frm = f; }
    bool frame() const { return frm; }

    int focusSection() const { return focusSec; }
    int section( const QPoint& p )
    {
	cursor->place( p + QPoint( offset, 0 ), parag );
	int idx = cursor->index();
	for ( uint i = 0; i < sections.count(); ++i ) {
	    if ( idx >= sections[i].selectionStart() &&
		 idx <= sections[i].selectionEnd() )
		return i;
	}
	return -1;
    }
    QNumberSection section( int idx ) const
    {
	return sections[idx];
    }
    bool setFocusSection( int idx )
    {
	if ( idx > (int)sections.count()-1 || idx < 0 )
	    return FALSE;
	if ( idx != focusSec ) {
	    focusSec = idx;
	    applyFocusSelection();
	    return TRUE;
	}
	return FALSE;
    }

    bool inSectionSelection( int idx )
    {
	for ( uint i = 0; i < sections.count(); ++i ) {
	    if ( idx >= sections[i].selectionStart() &&
		 idx <= sections[i].selectionEnd() )
		return TRUE;
	}
	return FALSE;
    }

    void paint( const QString& txt, bool focus, QPainter& p,
		const QColorGroup& cg, const QRect& rect, QStyle& style )
    {
	int fw = 0;
	if ( frm )
	    fw = style.pixelMetric(QStyle::PM_DefaultFrameWidth);

	parag->truncate( 0 );
	parag->append( txt );
	if ( !focus )
	    parag->removeSelection( QTextDocument::Standard );
	else {
	    applyFocusSelection();
	}

	/* color all QDATETIMEEDIT_HIDDEN_CHAR chars to background color */
	QTextFormat *fb = parag->formatCollection()->format( p.font(),
							     cg.base() );
	QTextFormat *nf = parag->formatCollection()->format( p.font(),
							     cg.text() );
	for ( uint i = 0; i < txt.length(); ++i ) {
	    parag->setFormat( i, 1, nf );
	    if ( inSectionSelection( i ) )
		continue;
	    if ( txt.at(i) == QDATETIMEEDIT_HIDDEN_CHAR )
		parag->setFormat( i, 1, fb );
	    else
		parag->setFormat( i, 1, nf );
	}
	fb->removeRef();
	nf->removeRef();

	QRect r( rect.x(), rect.y(), rect.width() - 2 * ( 2 + fw ), rect.height() );
	parag->pseudoDocument()->docRect = r;
	parag->invalidate(0);
	parag->format();

	int xoff = 2 + fw - offset;
	int yoff = ( rect.height() - parag->rect().height() + 1 ) / 2;
	if ( yoff < 0 )
	    yoff = 0;

	p.translate( xoff, yoff );
	parag->paint( p, cg, 0, TRUE );
	if ( frm )
	    p.translate( -xoff, -yoff );
    }

    void resize( const QSize& size ) { sz = size; }

    int mapSection( int sec )
    {
	return sections[sec].index();
    }

protected:
    void applyFocusSelection()
    {
	if ( focusSec > -1 ) {
	    int selstart = sections[ focusSec ].selectionStart();
	    int selend = sections[ focusSec ].selectionEnd();
	    parag->setSelection( QTextDocument::Standard, selstart, selend );
	    parag->format();
	    if ( parag->at( selstart )->x < offset ||
		 parag->at( selend )->x + parag->string()->width( selend ) > offset + sz.width() ) {
		offset = parag->at( selstart )->x;
	    }
	}
    }
private:
    bool frm;
    QTextParagraph *parag;
    QTextCursor *cursor;
    QSize sz;
    int focusSec;
    QValueList< QNumberSection > sections;
    QString sep;
    int offset;
};

class QDateTimeEditor : public QWidget
{
    Q_OBJECT
public:
    QDateTimeEditor( QDateTimeEditBase * widget, QWidget *parent,
		       const char * name=0 );
    ~QDateTimeEditor();

    void setControlWidget( QDateTimeEditBase * widget );
    QDateTimeEditBase * controlWidget() const;

    void setSeparator( const QString& s );
    QString separator() const;

    int  focusSection() const;
    bool setFocusSection( int s );
    void appendSection( const QNumberSection& sec );
    void clearSections();
    void setSectionSelection( int sec, int selstart, int selend );
    bool eventFilter( QObject *o, QEvent *e );
    int  sectionAt( const QPoint &p );
    int mapSection( int sec );

protected:
    void init();
    bool event( QEvent *e );
    void resizeEvent( QResizeEvent * );
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent *e );

private:
    QDateTimeEditBase* cw;
    QDateTimeEditorPrivate* d;
};

class QDateTimeSpinWidget : public QSpinWidget
{
public:
    QDateTimeSpinWidget( QWidget *parent, const char *name )
	: QSpinWidget( parent, name )
    {
    }

    void enabledChange(bool notenabled)
    {
        QDateEdit *de = ::qt_cast<QDateEdit*>(parentWidget());
        if (de && !notenabled) {
	    setUpEnabled(de->date() < de->maxValue());
	    setDownEnabled(de->date() > de->minValue());
        } else {
            setUpEnabled(!notenabled);
            setDownEnabled(!notenabled);
        }
    }

protected:
#ifndef QT_NO_WHEELEVENT
    void wheelEvent( QWheelEvent *e )
    {
	QDateTimeEditor *editor = (QDateTimeEditor*)editWidget()->qt_cast( "QDateTimeEditor" );
	Q_ASSERT( editor );
	if ( !editor )
	    return;

	int section = editor->sectionAt( e->pos() );
	editor->setFocusSection( section );

	if ( section == -1 )
	    return;
	QSpinWidget::wheelEvent( e );
    }
#endif
};

/*!
    Constructs an empty datetime editor with parent \a parent and
    called \a name.
*/
QDateTimeEditor::QDateTimeEditor( QDateTimeEditBase * widget, QWidget *parent,
				  const char * name )
    : QWidget( parent, name, WNoAutoErase )
{
    d = new QDateTimeEditorPrivate();
    cw = widget;
    init();
}

/*!
    Destroys the object and frees any allocated resources.
*/

QDateTimeEditor::~QDateTimeEditor()
{
    delete d;
}

/*! \internal

*/

void QDateTimeEditor::init()
{
    setBackgroundMode( PaletteBase );
    setFocusSection( -1 );
    installEventFilter( this );
    setFocusPolicy( WheelFocus );
}


/*! \reimp

*/

bool QDateTimeEditor::event( QEvent *e )
{
    if ( e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut ) {
 	if ( e->type() == QEvent::FocusOut )
  	    qApp->sendEvent( cw, e );
	update( rect() );
    } else if ( e->type() == QEvent::AccelOverride ) {
	QKeyEvent* ke = (QKeyEvent*) e;
	switch ( ke->key() ) {
	case Key_Delete:
	case Key_Backspace:
	case Key_Up:
	case Key_Down:
	case Key_Left:
	case Key_Right:
	    ke->accept();
	default:
	    break;
	}
    }
    return QWidget::event( e );
}

/*! \reimp

*/

void QDateTimeEditor::resizeEvent( QResizeEvent *e )
{
    d->resize( e->size() );
    QWidget::resizeEvent( e );
}


/*! \reimp

*/

void QDateTimeEditor::paintEvent( QPaintEvent * )
{
    QString txt;
    for ( uint i = 0; i < d->sectionCount(); ++i ) {
	txt += cw->sectionFormattedText( i );
	if ( i < d->sectionCount()-1 ) {
	    if ( d->section( i+1 ).separator() )
		txt += d->separator();
	    else
		txt += " ";
	}
    }

    QSharedDoubleBuffer buffer( this );
    const QBrush &bg =
	colorGroup().brush( isEnabled() ? QColorGroup::Base : QColorGroup::Background );
    buffer.painter()->fillRect( 0, 0, width(), height(), bg );
    d->paint( txt, hasFocus(), *buffer.painter(), colorGroup(), rect(),
	      style() );
    buffer.end();
}


/*!
    Returns the section index at point \a p.
*/
int QDateTimeEditor::sectionAt( const QPoint &p )
{
    return d->section( p );
}

int QDateTimeEditor::mapSection( int sec )
{
    return d->mapSection( sec );
}


/*! \reimp

*/

void QDateTimeEditor::mousePressEvent( QMouseEvent *e )
{
    QPoint p( e->pos().x(), 0 );
    int sec = sectionAt( p );
    if ( sec != -1 ) {
	cw->setFocusSection( sec );
	repaint( rect(), FALSE );
    }
}

/*! \reimp

*/
bool QDateTimeEditor::eventFilter( QObject *o, QEvent *e )
{
    if ( o == this ) {
	if ( e->type() == QEvent::KeyPress ) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    switch ( ke->key() ) {
	    case Key_Right:
		if ( d->focusSection() < (int)d->sectionCount()-1 ) {
		    if ( cw->setFocusSection( focusSection()+1 ) )
			repaint( rect(), FALSE );
		}
		return TRUE;
	    case Key_Left:
		if ( d->focusSection() > 0 ) {
		    if ( cw->setFocusSection( focusSection()-1 ) )
			repaint( rect(), FALSE );
		}
		return TRUE;
	    case Key_Up:
		cw->stepUp();
		return TRUE;
	    case Key_Down:
		cw->stepDown();
		return TRUE;
	    case Key_Backspace:
		if ( ::qt_cast<QDateEdit*>(cw) )
		    ((QDateEdit*)cw)->removeFirstNumber( d->focusSection() );
		else if ( ::qt_cast<QTimeEdit*>(cw) )
		    ((QTimeEdit*)cw)->removeFirstNumber( d->focusSection() );
		return TRUE;
	    case Key_Delete:
		cw->removeLastNumber( d->focusSection() );
		return TRUE;
	    case Key_Tab:
	    case Key_BackTab: {
		if ( ke->state() == Qt::ControlButton )
		    return FALSE;

		QWidget *w = this;
		bool hadDateEdit = FALSE;
		while ( w ) {
		    if ( ::qt_cast<QDateTimeSpinWidget*>(w) && qstrcmp( w->name(), "qt_spin_widget" ) != 0 ||
			 ::qt_cast<QDateTimeEdit*>(w) )
			break;
		    hadDateEdit = hadDateEdit || ::qt_cast<QDateEdit*>(w);
		    w = w->parentWidget();
		}

		if ( w ) {
		    if ( !::qt_cast<QDateTimeEdit*>(w) ) {
			w = w->parentWidget();
		    } else {
			QDateTimeEdit *ed = (QDateTimeEdit*)w;
			if ( hadDateEdit && ke->key() == Key_Tab ) {
			    ed->timeEdit()->setFocus();
			    return TRUE;
			} else if ( !hadDateEdit && ke->key() == Key_BackTab ) {
			    ed->dateEdit()->setFocus();
			    return TRUE;
			} else {
			    while ( w && !::qt_cast<QDateTimeEdit*>(w) )
				w = w->parentWidget();
			}
		    }

		    qApp->sendEvent( w, e );
		    return TRUE;
		}
	    } break;
	    default:
		QString txt = ke->text().lower();
		if ( !txt.isEmpty() && !separator().isEmpty() && txt[0] == separator()[0] ) {
		    // do the same thing as KEY_RIGHT when the user presses the separator key
		    if ( d->focusSection() < 2 ) {
			if ( cw->setFocusSection( focusSection()+1 ) )
			    repaint( rect(), FALSE );
		    }
		    return TRUE;
		} else if ( !txt.isEmpty() && ::qt_cast<QTimeEdit*>(cw) && focusSection() == (int) d->sectionCount()-1 ) {
		    // the first character of the AM/PM indicator toggles if the section has focus
		    QTimeEdit *te = (QTimeEdit*)cw;
		    QTime time = te->time();
		    if ( lAMPM && lAM && lPM && (te->display()&QTimeEdit::AMPM) ) {
			if ( txt[0] == (*lAM).lower()[0] && time.hour() >= 12 ) {
			    time.setHMS( time.hour()-12, time.minute(), time.second(), time.msec() );
			    te->setTime( time );
			} else if ( txt[0] == (*lPM).lower()[0] && time.hour() < 12 ) {
			    time.setHMS( time.hour()+12, time.minute(), time.second(), time.msec() );
			    te->setTime( time );
			}
		    }
		}

		int num = txt[0].digitValue();
		if ( num != -1 ) {
		    cw->addNumber( d->focusSection(), num );
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}


/*!
    Appends the number section \a sec to the editor.
*/

void QDateTimeEditor::appendSection( const QNumberSection& sec )
{
    d->appendSection( sec );
}

/*!
    Removes all sections from the editor.
*/

void QDateTimeEditor::clearSections()
{
    d->clearSections();
}

/*!
    Sets the selection of \a sec to start at \a selstart and end at \a
    selend.
*/

void QDateTimeEditor::setSectionSelection( int sec, int selstart, int selend )
{
    d->setSectionSelection( sec, selstart, selend );
}

/*!
    Sets the separator for all numbered sections to \a s. Note that
    currently, only the first character of \a s is used.
*/

void QDateTimeEditor::setSeparator( const QString& s )
{
    d->setSeparator( s );
    update();
}


/*!
    Returns the editor's separator.
*/

QString QDateTimeEditor::separator() const
{
    return d->separator();
}

/*!
    Returns the number of the section that has focus.
*/

int QDateTimeEditor::focusSection() const
{
    return d->focusSection();
}


/*!
    Sets the focus to section \a sec. If \a sec does not exist,
    nothing happens.
*/

bool QDateTimeEditor::setFocusSection( int sec )
{
    return d->setFocusSection( sec );
}

/*! \class QDateTimeEditBase
    \brief The QDateTimeEditBase class provides an abstraction for date and edit editors.

    Small abstract class that provides some functions that are common
    for both QDateEdit and QTimeEdit. It is used internally by
    QDateTimeEditor.
*/

/*!
    \fn QDateTimeEditBase::QDateTimeEditBase(QWidget *, const char*)
    \internal
*/

/*!
    \fn QDateTimeEditBase::setFocusSection(int)
    \internal
*/

/*! \fn QString QDateTimeEditBase::sectionFormattedText( int sec )
    \internal

  Pure virtual function which returns the formatted text of section \a
  sec.

*/

/*! \fn void QDateTimeEditBase::stepUp()
    \internal

  Pure virtual slot which is called whenever the user increases the
  number in a section by pressing the widget's arrow buttons or the
  keyboard's arrow keys.
*/

/*! \fn void QDateTimeEditBase::stepDown()
    \internal

  Pure virtual slot which is called whenever the user decreases the
  number in a section by pressing the widget's arrow buttons or the
  keyboard's arrow keys.

*/

/*! \fn void QDateTimeEditBase::addNumber( int sec, int num )
    \internal

  Pure virtual function which is called whenever the user types a number.
  \a sec indicates the section where the number should be added. \a
  num is the number that was pressed.
*/

/*! \fn void QDateTimeEditBase::removeLastNumber( int sec )
    \internal

  Pure virtual function which is called whenever the user tries to
  remove the last number from \a sec by pressing the delete key.
*/

////////////////

class QDateEditPrivate
{
public:
    int y;
    int m;
    int d;
    // remebers the last entry for the day.
    // if the day is 31 and you cycle through the months,
    // the day will be 31 again if you reach a month with 31 days
    // otherwise it will be the highest day in the month
    int dayCache;
    int yearSection;
    int monthSection;
    int daySection;
    QDateEdit::Order ord;
    bool overwrite;
    bool adv;
    int timerId;
    bool typing;
    QDate min;
    QDate max;
    bool changed;
    QDateTimeEditor *ed;
    QSpinWidget *controls;
};


/*!
    \class QDateEdit qdatetimeedit.h
    \brief The QDateEdit class provides a date editor.

    \ingroup advanced
    \ingroup time
    \mainclass

    QDateEdit allows the user to edit dates by using the keyboard or
    the arrow keys to increase/decrease date values. The arrow keys
    can be used to move from section to section within the QDateEdit
    box. Dates appear in accordance with the local date/time settings
    or in year, month, day order if the system doesn't provide this
    information. It is recommended that the QDateEdit be initialised
    with a date, e.g.

    \code
    QDateEdit *dateEdit = new QDateEdit( QDate::currentDate(), this );
    dateEdit->setRange( QDate::currentDate().addDays( -365 ),
			QDate::currentDate().addDays(  365 ) );
    dateEdit->setOrder( QDateEdit::MDY );
    dateEdit->setAutoAdvance( TRUE );
    \endcode

    Here we've created a new QDateEdit object initialised with today's
    date and restricted the valid date range to today plus or minus
    365 days. We've set the order to month, day, year. If the auto
    advance property is TRUE (as we've set it here) when the user
    completes a section of the date, e.g. enters two digits for the
    month, they are automatically taken to the next section.

    The maximum and minimum values for a date value in the date editor
    default to the maximum and minimum values for a QDate. You can
    change this by calling setMinValue(), setMaxValue() or setRange().

    Terminology: A QDateEdit widget comprises three 'sections', one
    each for the year, month and day. You can change the separator
    character using QDateTimeEditor::setSeparator(), by default the
    separator will be taken from the systems settings. If that is
    not possible, it defaults to "-".

    \img datetimewidgets.png Date Time Widgets

    \sa QDate QTimeEdit QDateTimeEdit
*/

/*!
    \enum QDateEdit::Order

    This enum defines the order in which the sections that comprise a
    date appear.
    \value MDY month-day-year
    \value DMY day-month-year
    \value YMD year-month-day (the default)
    \value YDM year-day-month (included for completeness; but should
    not be used)
*/

/*!
    \enum QTimeEdit::Display

    This enum defines the sections that comprise a time

    \value Hours The hours section
    \value Minutes The minutes section
    \value Seconds The seconds section
    \value AMPM The AM/PM section

    The values can be or'ed together to show any combination.
*/

/*!
    Constructs an empty date editor which is a child of \a parent and
    called name \a name.
*/

QDateEdit::QDateEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
    updateButtons();
}

/*!
    \overload

    Constructs a date editor with the initial value \a date, parent \a
    parent and called \a name.

    The date editor is initialized with \a date.
*/

QDateEdit::QDateEdit( const QDate& date, QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
    setDate( date );
}

/*! \internal
*/
void QDateEdit::init()
{
    d = new QDateEditPrivate();
    d->controls = new QDateTimeSpinWidget( this, qstrcmp( name(), "qt_datetime_dateedit" ) == 0 ? "qt_spin_widget" : "date edit controls" );
    d->ed = new QDateTimeEditor( this, d->controls, "date editor" );
    d->controls->setEditWidget( d->ed );
    setFocusProxy( d->ed );
    connect( d->controls, SIGNAL( stepUpPressed() ), SLOT( stepUp() ) );
    connect( d->controls, SIGNAL( stepDownPressed() ), SLOT( stepDown() ) );
    connect( this, SIGNAL( valueChanged(const QDate&) ),
	     SLOT( updateButtons() ) );
    d->ed->appendSection( QNumberSection( 0,4 ) );
    d->ed->appendSection( QNumberSection( 5,7 ) );
    d->ed->appendSection( QNumberSection( 8,10 ) );

    d->yearSection = -1;
    d->monthSection = -1;
    d->daySection = -1;

    d->y = 0;
    d->m = 0;
    d->d = 0;
    d->dayCache = 0;
    setOrder( localOrder() );
    setFocusSection( 0 );
    d->overwrite = TRUE;
    d->adv = FALSE;
    d->timerId = 0;
    d->typing = FALSE;
    d->min = QDate( 1752, 9, 14 );
    d->max = QDate( 8000, 12, 31 );
    d->changed = FALSE;

    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );

    refcount++;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QDateEdit::~QDateEdit()
{
    delete d;
    if ( !--refcount )
	cleanup();
}

/*!
    \property QDateEdit::minValue

    \brief the editor's minimum value

    Setting the minimum date value is equivalent to calling
    QDateEdit::setRange( \e d, maxValue() ), where \e d is the minimum
    date. The default minimum date is 1752-09-14.

    \sa maxValue setRange()
*/

QDate QDateEdit::minValue() const
{
    return d->min;
}

/*!
    \property QDateEdit::maxValue

    \brief the editor's maximum value

    Setting the maximum date value for the editor is equivalent to
    calling QDateEdit::setRange( minValue(), \e d ), where \e d is the
    maximum date. The default maximum date is 8000-12-31.

    \sa minValue setRange()
*/

QDate QDateEdit::maxValue() const
{
    return d->max;
}


/*!
    Sets the valid input range for the editor to be from \a min to \a
    max inclusive. If \a min is invalid no minimum date will be set.
    Similarly, if \a max is invalid no maximum date will be set.
*/

void QDateEdit::setRange( const QDate& min, const QDate& max )
{
    if ( min.isValid() )
	d->min = min;
    if ( max.isValid() )
	d->max = max;
}

/*!
    Sets the separator to \a s. Note that currently only the first
    character of \a s is used.
*/

void QDateEdit::setSeparator( const QString& s )
{
    d->ed->setSeparator( s );
}

/*!
    Returns the editor's separator.
*/

QString QDateEdit::separator() const
{
    return d->ed->separator();
}


/*!
    Enables/disables the push buttons according to the min/max date
    for this widget.
*/

void QDateEdit::updateButtons()
{
    if ( !isEnabled() )
	return;

    bool upEnabled = date() < maxValue();
    bool downEnabled = date() > minValue();

    d->controls->setUpEnabled( upEnabled );
    d->controls->setDownEnabled( downEnabled );
}

/*! \reimp
 */
void QDateEdit::resizeEvent( QResizeEvent * )
{
    d->controls->resize( width(), height() );
}

/*! \reimp

*/
QSize QDateEdit::sizeHint() const
{
    constPolish();
    QFontMetrics fm( font() );
    int fw = style().pixelMetric( QStyle::PM_DefaultFrameWidth, this );
    int h = QMAX( fm.lineSpacing(), 14 ) + 2;
    int w = 2 + fm.width( '9' ) * 8 + fm.width( d->ed->separator() ) * 2 + d->controls->upRect().width() + fw * 4;

    return QSize( w, QMAX(h + fw * 2,20) ).expandedTo( QApplication::globalStrut() );
}

/*! \reimp

*/
QSize QDateEdit::minimumSizeHint() const
{
    return sizeHint();
}


/*!
    Returns the formatted number for section \a sec. This will
    correspond to either the year, month or day section, depending on
    the current display order.

    \sa setOrder()
*/

QString QDateEdit::sectionFormattedText( int sec )
{
    QString txt;
    txt = sectionText( sec );
    if ( d->typing && sec == d->ed->focusSection() )
	d->ed->setSectionSelection( sec, sectionOffsetEnd( sec ) - txt.length(),
			     sectionOffsetEnd( sec ) );
    else
	d->ed->setSectionSelection( sec, sectionOffsetEnd( sec ) - sectionLength( sec ),
			     sectionOffsetEnd( sec ) );
    txt = txt.rightJustify( sectionLength( sec ), QDATETIMEEDIT_HIDDEN_CHAR );
    return txt;
}


/*!
    Returns the desired length (number of digits) of section \a sec.
    This will correspond to either the year, month or day section,
    depending on the current display order.

    \sa setOrder()
*/

int QDateEdit::sectionLength( int sec ) const
{
    int val = 0;
    if ( sec == d->yearSection ) {
	val = 4;
    } else if ( sec == d->monthSection ) {
	val = 2;
    } else if ( sec == d->daySection ) {
	val = 2;
    }
    return val;
}

/*!
    Returns the text of section \a sec. This will correspond to either
    the year, month or day section, depending on the current display
    order.

    \sa setOrder()
*/

QString QDateEdit::sectionText( int sec ) const
{
    int val = 0;
    if ( sec == d->yearSection ) {
	val = d->y;
    } else if ( sec == d->monthSection ) {
	val = d->m;
    } else if ( sec == d->daySection ) {
	val = d->d;
    }
    return QString::number( val );
}

/*! \internal

  Returns the end of the section offset \a sec.

*/

int QDateEdit::sectionOffsetEnd( int sec ) const
{
    if ( sec == d->yearSection ) {
	switch( d->ord ) {
	case DMY:
	case MDY:
	    return sectionOffsetEnd( sec-1) + separator().length() + sectionLength( sec );
	case YMD:
	case YDM:
	    return sectionLength( sec );
	}
    } else if ( sec == d->monthSection ) {
	switch( d->ord ) {
	case DMY:
	case YDM:
	case YMD:
	    return sectionOffsetEnd( sec-1) + separator().length() + sectionLength( sec );
	case MDY:
	    return sectionLength( sec );
	}
    } else if ( sec == d->daySection ) {
	switch( d->ord ) {
	case DMY:
	    return sectionLength( sec );
	case YMD:
	case MDY:
	case YDM:
	    return sectionOffsetEnd( sec-1 ) + separator().length() + sectionLength( sec );
	}
    }
    return 0;
}


/*!
    \property QDateEdit::order
    \brief the order in which the year, month and day appear

    The default order is locale dependent.

    \sa Order
*/

void QDateEdit::setOrder( QDateEdit::Order order )
{
    d->ord = order;
    switch( d->ord ) {
    case DMY:
	d->yearSection = 2;
	d->monthSection = 1;
	d->daySection = 0;
	break;
    case MDY:
	d->yearSection = 2;
	d->monthSection = 0;
	d->daySection = 1;
	break;
    case YMD:
	d->yearSection = 0;
	d->monthSection = 1;
	d->daySection = 2;
	break;
    case YDM:
	d->yearSection = 0;
	d->monthSection = 2;
	d->daySection = 1;
	break;
    }
    if ( isVisible() )
	d->ed->repaint( d->ed->rect(), FALSE );
}


QDateEdit::Order QDateEdit::order() const
{
    return d->ord;
}


/*! \reimp

*/
void QDateEdit::stepUp()
{
    int sec = d->ed->focusSection();
    bool accepted = FALSE;
    if ( sec == d->yearSection ) {
	if ( !outOfRange( d->y+1, d->m, d->d ) ) {
	    accepted = TRUE;
	    setYear( d->y+1 );
	}
    } else if ( sec == d->monthSection ) {
	if ( !outOfRange( d->y, d->m+1, d->d ) ) {
	    accepted = TRUE;
	    setMonth( d->m+1 );
	}
    } else if ( sec == d->daySection ) {
	if ( !outOfRange( d->y, d->m, d->d+1 ) ) {
	    accepted = TRUE;
	    setDay( d->d+1 );
	}
    }
    if ( accepted ) {
	d->changed = FALSE;
	emit valueChanged( date() );
    }
    d->ed->repaint( d->ed->rect(), FALSE );
}



/*! \reimp

*/

void QDateEdit::stepDown()
{
    int sec = d->ed->focusSection();
    bool accepted = FALSE;
    if ( sec == d->yearSection ) {
	if ( !outOfRange( d->y-1, d->m, d->d ) ) {
	    accepted = TRUE;
	    setYear( d->y-1 );
	}
    } else if ( sec == d->monthSection ) {
	if ( !outOfRange( d->y, d->m-1, d->d ) ) {
	    accepted = TRUE;
	    setMonth( d->m-1 );
	}
    } else if ( sec == d->daySection ) {
	if ( !outOfRange( d->y, d->m, d->d-1 ) ) {
	    accepted = TRUE;
	    setDay( d->d-1 );
	}
    }
    if ( accepted ) {
	d->changed = FALSE;
	emit valueChanged( date() );
    }
    d->ed->repaint( d->ed->rect(), FALSE );
}

/*!
    Sets the year to \a year, which must be a valid year. The range
    currently supported is from 1752 to 8000.

    \sa QDate
*/

void QDateEdit::setYear( int year )
{
    if ( year < 1752 )
	year = 1752;
    if ( year > 8000 )
	year = 8000;
    if ( !outOfRange( year, d->m, d->d ) ) {
	d->y = year;
	setMonth( d->m );
	int tmp = d->dayCache;
	setDay( d->dayCache );
	d->dayCache = tmp;
    }
}


/*!
    Sets the month to \a month, which must be a valid month, i.e.
    between 1 and 12.
*/

void QDateEdit::setMonth( int month )
{
    if ( month < 1 )
	month = 1;
    if ( month > 12 )
	month = 12;
    if ( !outOfRange( d->y, month, d->d ) ) {
	d->m = month;
	int tmp = d->dayCache;
	setDay( d->dayCache );
	d->dayCache = tmp;
    }
}


/*!
    Sets the day to \a day, which must be a valid day. The function
    will ensure that the \a day set is valid for the month and year.
*/

void QDateEdit::setDay( int day )
{
    if ( day < 1 )
	day = 1;
    if ( day > 31 )
	day = 31;
    if ( d->m > 0 && d->y > 1752 ) {
	while ( !QDate::isValid( d->y, d->m, day ) )
	    --day;
	if ( !outOfRange( d->y, d->m, day ) )
	    d->d = day;
    } else if ( d->m > 0 ) {
	if ( day > 0 && day < 32 ) {
	    if ( !outOfRange( d->y, d->m, day ) )
		d->d = day;
	}
    }
    d->dayCache = d->d;
}


/*!
    \property QDateEdit::date
    \brief the editor's date value.

    If the date property is not valid, the editor displays all zeroes
    and QDateEdit::date() will return an invalid date. It is strongly
    recommended that the editor is given a default date value (e.g.
    currentDate()). That way, attempts to set the date property to an
    invalid date will fail.

    When changing the date property, if the date is less than
    minValue(), or is greater than maxValue(), nothing happens.
*/

void QDateEdit::setDate( const QDate& date )
{
    if ( !date.isValid() ) {
	d->y = 0;
	d->m = 0;
	d->d = 0;
	d->dayCache = 0;
    } else {
	if ( date > maxValue() || date < minValue() )
	    return;
	d->y = date.year();
	d->m = date.month();
	d->d = date.day();
	d->dayCache = d->d;
	emit valueChanged( date );
    }
    d->changed = FALSE;
    d->ed->repaint( d->ed->rect(), FALSE );
}

QDate QDateEdit::date() const
{
    if ( QDate::isValid( d->y, d->m, d->d ) )
	return QDate( d->y, d->m, d->d );
    return QDate();
}

/*!  \internal

  Returns TRUE if \a y, \a m, \a d is out of range, otherwise returns
  FALSE.

  \sa setRange()

*/

bool QDateEdit::outOfRange( int y, int m, int d ) const
{
    if ( QDate::isValid( y, m, d ) ) {
	QDate currentDate( y, m, d );
	if ( currentDate > maxValue() ||
	     currentDate < minValue() ) {
	    //## outOfRange should set overwrite?
	    return TRUE;
	}
	return FALSE;
    }
    return FALSE; /* assume ok */
}

/*!  \reimp

*/

void QDateEdit::addNumber( int sec, int num )
{
    if ( sec == -1 )
	return;
    killTimer( d->timerId );
    bool overwrite = FALSE;
    bool accepted = FALSE;
    d->typing = TRUE;
    QString txt;
    if ( sec == d->yearSection ) {
	txt = QString::number( d->y );
	if ( d->overwrite || txt.length() == 4 ) {
	    accepted = TRUE;
	    d->y = num;
	} else {
	    txt += QString::number( num );
	    if ( txt.length() == 4  ) {
		int val = txt.toInt();
		if ( val < 1792 )
		    d->y = 1792;
		else if ( val > 8000 )
		    d->y = 8000;
		else if ( outOfRange( val, d->m, d->d ) )
		    txt = QString::number( d->y );
		else {
		    accepted = TRUE;
		    d->y = val;
		}
	    } else {
		accepted = TRUE;
		d->y = txt.toInt();
	    }
	    if ( d->adv && txt.length() == 4 ) {
		d->ed->setFocusSection( d->ed->focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    } else if ( sec == d->monthSection ) {
	txt = QString::number( d->m );
	if ( d->overwrite || txt.length() == 2 ) {
	    accepted = TRUE;
	    d->m = num;
	} else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 12 )
		temp = num;
	    if ( outOfRange( d->y, temp, d->d ) )
		txt = QString::number( d->m );
	    else {
		accepted = TRUE;
		d->m = temp;
	    }
	    if ( d->adv && txt.length() == 2 ) {
		d->ed->setFocusSection( d->ed->focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    } else if ( sec == d->daySection ) {
	txt = QString::number( d->d );
	if ( d->overwrite || txt.length() == 2 ) {
	    accepted = TRUE;
	    d->d = num;
	    d->dayCache = d->d;
	} else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 31 )
		temp = num;
	    if ( outOfRange( d->y, d->m, temp ) )
		txt = QString::number( d->d );
	    else {
		accepted = TRUE;
		d->d = temp;
		d->dayCache = d->d;
	    }
	    if ( d->adv && txt.length() == 2 ) {
		d->ed->setFocusSection( d->ed->focusSection()+1 );
		overwrite = TRUE;
	    }
	}
    }
    if ( accepted ) {
	d->changed = FALSE;
	emit valueChanged( date() );
    }
    d->overwrite = overwrite;
    d->timerId = startTimer( qApp->doubleClickInterval()*4 );
    d->ed->repaint( d->ed->rect(), FALSE );
}


/*! \reimp

*/

bool QDateEdit::setFocusSection( int s )
{
    if ( s != d->ed->focusSection() ) {
	killTimer( d->timerId );
	d->overwrite = TRUE;
	d->typing = FALSE;
	fix(); // will emit valueChanged if necessary
    }
    return d->ed->setFocusSection( s );
}


/*!
    Attempts to fix any invalid date entries.

    The rules applied are as follows:

    \list
    \i If the year has four digits it is left unchanged.
    \i If the year has two digits, the year will be changed to four
    digits in the range current year - 70 to current year + 29.
    \i If the year has three digits in the range 100..999, the
    current millennium, i.e. 2000, will be added giving a year
    in the range 2100..2999.
    \i If the day or month is 0 then it will be set to 1 or the
    minimum valid day\month in the range.
    \endlist

*/

void QDateEdit::fix()
{
    bool changed = FALSE;
    int currentYear = QDate::currentDate().year();
    int year = d->y;
    if ( year < 100 ) {
	int currentCentury = currentYear / 100;
	year += currentCentury * 100;
	if ( currentYear > year ) {
	    if ( currentYear > year + 70 )
		year += 100;
	} else {
	    if ( year >= currentYear + 30 )
		year -= 100;
	}
	changed = TRUE;
    } else if ( year < 1000 ) {
	int currentMillennium = currentYear / 10;
	year += currentMillennium * 10;
	changed = TRUE;
    } else if (d->d == 0) {
	d->d = 1;
	changed = TRUE;
    } else if (d->m == 0) {
	d->m = 1;
	changed = TRUE;
    }
    if ( outOfRange( year, d->m, d->d ) ) {
	if ( minValue().isValid() && date() < minValue() ) {
	    d->d =  minValue().day();
	    d->dayCache = d->d;
	    d->m = minValue().month();
	    d->y = minValue().year();
	}
	if ( date() > maxValue() ) {
	    d->d =  maxValue().day();
	    d->dayCache = d->d;
	    d->m = maxValue().month();
	    d->y = maxValue().year();
	}
	changed = TRUE;
    } else if ( changed )
	setYear( year );
    if ( changed ) {
	emit valueChanged( date() );
	d->changed = FALSE;
    }
}


/*! \reimp

*/

bool QDateEdit::event( QEvent *e )
{
    if( e->type() == QEvent::FocusOut ) {
	d->typing = FALSE;
	fix();
	// the following can't be done in fix() because fix() called
	// from all over the place and it will break the old behaviour
	if ( !QDate::isValid( d->y, d->m, d->d ) ) {
	    d->dayCache = d->d;
	    int i = d->d;
	    for ( ; i > 0; i-- ) {
		d->d = i;
		if ( QDate::isValid( d->y, d->m, d->d ) )
		    break;
	    }
	    d->changed = TRUE;
	}
	if ( d->changed ) {
	    emit valueChanged( date() );
	    d->changed = FALSE;
	}
    } else if ( e->type() == QEvent::LocaleChange ) {
	readLocaleSettings();
	d->ed->setSeparator( localDateSep() );
	setOrder( localOrder() );
    }
    return QDateTimeEditBase::event( e );
}

/*!
  \internal

  Function which is called whenever the user tries to
  remove the first number from \a sec by pressing the backspace key.
*/

void QDateEdit::removeFirstNumber( int sec )
{
    if ( sec == -1 )
	return;
    QString txt;
    if ( sec == d->yearSection ) {
	txt = QString::number( d->y );
	txt = txt.mid( 1, txt.length() ) + "0";
	d->y = txt.toInt();
    } else if ( sec == d->monthSection ) {
	txt = QString::number( d->m );
	txt = txt.mid( 1, txt.length() ) + "0";
	d->m = txt.toInt();
    } else if ( sec == d->daySection ) {
	txt = QString::number( d->d );
	txt = txt.mid( 1, txt.length() ) + "0";
	d->d = txt.toInt();
	d->dayCache = d->d;
    }
    d->ed->repaint( d->ed->rect(), FALSE );
}

/*! \reimp

*/

void QDateEdit::removeLastNumber( int sec )
{
    if ( sec == -1 )
	return;
    QString txt;
    if ( sec == d->yearSection ) {
	txt = QString::number( d->y );
	txt = txt.mid( 0, txt.length()-1 );
	d->y = txt.toInt();
    } else if ( sec == d->monthSection ) {
	txt = QString::number( d->m );
	txt = txt.mid( 0, txt.length()-1 );
	d->m = txt.toInt();
    } else if ( sec == d->daySection ) {
	txt = QString::number( d->d );
	txt = txt.mid( 0, txt.length()-1 );
	d->d = txt.toInt();
	d->dayCache = d->d;
    }
    d->ed->repaint( d->ed->rect(), FALSE );
}

/*!
    \property QDateEdit::autoAdvance
    \brief whether the editor automatically advances to the next
    section

    If autoAdvance is TRUE, the editor will automatically advance
    focus to the next date section if a user has completed a section.
    The default is FALSE.
*/

void QDateEdit::setAutoAdvance( bool advance )
{
    d->adv = advance;
}


bool QDateEdit::autoAdvance() const
{
    return d->adv;
}

/*! \reimp
*/

void QDateEdit::timerEvent( QTimerEvent * )
{
    d->overwrite = TRUE;
}

/*!
    \fn void QDateEdit::valueChanged( const QDate& date )

    This signal is emitted whenever the editor's value changes. The \a
    date parameter is the new value.
*/

///////////

class QTimeEditPrivate
{
public:
    int h;
    int m;
    int s;
    uint display;
    bool adv;
    bool overwrite;
    int timerId;
    bool typing;
    QTime min;
    QTime max;
    bool changed;
    QDateTimeEditor *ed;
    QSpinWidget *controls;
};

/*!
    \class QTimeEdit qdatetimeedit.h
    \brief The QTimeEdit class provides a time editor.

    \ingroup advanced
    \ingroup time
    \mainclass

    QTimeEdit allows the user to edit times by using the keyboard or
    the arrow keys to increase/decrease time values. The arrow keys
    can be used to move from section to section within the QTimeEdit
    box. The user can automatically be moved to the next section once
    they complete a section using setAutoAdvance(). Times appear in
    hour, minute, second order. It is recommended that the QTimeEdit
    is initialised with a time, e.g.
    \code
    QTime timeNow = QTime::currentTime();
    QTimeEdit *timeEdit = new QTimeEdit( timeNow, this );
    timeEdit->setRange( timeNow, timeNow.addSecs( 60 * 60 ) );
    \endcode
    Here we've created a QTimeEdit widget set to the current time.
    We've also set the minimum value to the current time and the
    maximum time to one hour from now.

    The maximum and minimum values for a time value in the time editor
    default to the maximum and minimum values for a QTime. You can
    change this by calling setMinValue(), setMaxValue() or setRange().

    Terminology: A QTimeWidget consists of three sections, one each
    for the hour, minute and second. You can change the separator
    character using setSeparator(), by default the separator is read
    from the system's settings.

    \img datetimewidgets.png Date Time Widgets

    \sa QTime QDateEdit QDateTimeEdit
*/


/*!
    Constructs an empty time edit with parent \a parent and called \a
    name.
*/

QTimeEdit::QTimeEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
}

/*!
    \overload

    Constructs a time edit with the initial time value, \a time,
    parent \a parent and called \a name.
*/

QTimeEdit::QTimeEdit( const QTime& time, QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    init();
    setTime( time );
}

/*! \internal
 */

void QTimeEdit::init()
{
    d = new QTimeEditPrivate();
    d->controls = new QDateTimeSpinWidget( this, qstrcmp( name(), "qt_datetime_timeedit" ) == 0 ? "qt_spin_widget" : "time edit controls" );
    d->ed = new QDateTimeEditor( this, d->controls, "time edit base" );
    d->controls->setEditWidget( d->ed );
    setFocusProxy( d->ed );
    connect( d->controls, SIGNAL( stepUpPressed() ), SLOT( stepUp() ) );
    connect( d->controls, SIGNAL( stepDownPressed() ), SLOT( stepDown() ) );

    d->ed->appendSection( QNumberSection( 0,0, TRUE, 0 ) );
    d->ed->appendSection( QNumberSection( 0,0, TRUE, 1 ) );
    d->ed->appendSection( QNumberSection( 0,0, TRUE, 2 ) );
    d->ed->setSeparator( localTimeSep() );

    d->h = 0;
    d->m = 0;
    d->s = 0;
    d->display = Hours | Minutes | Seconds;
    if ( lAMPM ) {
	d->display |= AMPM;
	d->ed->appendSection( QNumberSection( 0,0, FALSE, 3 ) );
    }
    d->adv = FALSE;
    d->overwrite = TRUE;
    d->timerId = 0;
    d->typing = FALSE;
    d->min = QTime( 0, 0, 0 );
    d->max = QTime( 23, 59, 59 );
    d->changed = FALSE;

    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );

    refcount++;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QTimeEdit::~QTimeEdit()
{
    delete d;
    if ( !--refcount )
	cleanup();
}

/*!
    \property QTimeEdit::minValue
    \brief the minimum time value

    Setting the minimum time value is equivalent to calling
    QTimeEdit::setRange( \e t, maxValue() ), where \e t is the minimum
    time. The default minimum time is 00:00:00.

    \sa maxValue setRange()
*/

QTime QTimeEdit::minValue() const
{
    return d->min;
}

/*!
    \property QTimeEdit::maxValue
    \brief the maximum time value

    Setting the maximum time value is equivalent to calling
    QTimeEdit::setRange( minValue(), \e t ), where \e t is the maximum
    time. The default maximum time is 23:59:59.

    \sa minValue setRange()
*/

QTime QTimeEdit::maxValue() const
{
    return d->max;
}


/*!
    Sets the valid input range for the editor to be from \a min to \a
    max inclusive. If \a min is invalid no minimum time is set.
    Similarly, if \a max is invalid no maximum time is set.
*/

void QTimeEdit::setRange( const QTime& min, const QTime& max )
{
    if ( min.isValid() )
	d->min = min;
    if ( max.isValid() )
	d->max = max;
}

/*!
  \property QTimeEdit::display
  \brief the sections that are displayed in the time edit

  The value can be any combination of the values in the Display enum.
  By default, the widget displays hours, minutes and seconds.
*/
void QTimeEdit::setDisplay( uint display )
{
    if ( d->display == display )
	return;

    d->ed->clearSections();
    d->display = display;
    if ( d->display & Hours )
	d->ed->appendSection( QNumberSection( 0,0, TRUE, 0 ) );
    if ( d->display & Minutes )
	d->ed->appendSection( QNumberSection( 0,0, TRUE, 1 ) );
    if ( d->display & Seconds )
	d->ed->appendSection( QNumberSection( 0,0, TRUE, 2 ) );
    if ( d->display & AMPM )
	d->ed->appendSection( QNumberSection( 0,0, FALSE, 3 ) );

    d->ed->setFocusSection( 0 );
    d->ed->update();
}

uint QTimeEdit::display() const
{
    return d->display;
}

/*!
    \property QTimeEdit::time
    \brief the editor's time value.

    When changing the time property, if the time is less than
    minValue(), or is greater than maxValue(), nothing happens.
*/

void QTimeEdit::setTime( const QTime& time )
{
    if ( !time.isValid() ) {
	d->h = 0;
	d->m = 0;
	d->s = 0;
    } else {
	if ( time > maxValue() || time < minValue() )
	    return;
	d->h = time.hour();
	d->m = time.minute();
	d->s = time.second();
	emit valueChanged( time );
    }
    d->changed = FALSE;
    d->ed->repaint( d->ed->rect(), FALSE );
}

QTime QTimeEdit::time() const
{
    if ( QTime::isValid( d->h, d->m, d->s ) )
	return QTime( d->h, d->m, d->s );
    return QTime();
}

/*!
    \property QTimeEdit::autoAdvance
    \brief whether the editor automatically advances to the next
    section

    If autoAdvance is TRUE, the editor will automatically advance
    focus to the next time section if a user has completed a section.
    The default is FALSE.
*/

void QTimeEdit::setAutoAdvance( bool advance )
{
    d->adv = advance;
}

bool QTimeEdit::autoAdvance() const
{
    return d->adv;
}

/*!
    Sets the separator to \a s. Note that currently only the first
    character of \a s is used.
*/

void QTimeEdit::setSeparator( const QString& s )
{
    d->ed->setSeparator( s );
}

/*!
    Returns the editor's separator.
*/

QString QTimeEdit::separator() const
{
    return d->ed->separator();
}


/*!
    \fn void QTimeEdit::valueChanged( const QTime& time )

    This signal is emitted whenever the editor's value changes. The \a
    time parameter is the new value.
*/

/*! \reimp

*/

bool QTimeEdit::event( QEvent *e )
{
    if ( e->type() == QEvent::FocusOut ) {
	d->typing = FALSE;
	if ( d->changed ) {
	    emit valueChanged( time() );
	    d->changed = FALSE;
	}
    } else if ( e->type() == QEvent::LocaleChange ) {
	readLocaleSettings();
	d->ed->setSeparator( localTimeSep() );
    }
    return QDateTimeEditBase::event( e );
}

/*! \reimp

*/

void QTimeEdit::timerEvent( QTimerEvent * )
{
    d->overwrite = TRUE;
}


/*! \reimp

*/

void QTimeEdit::stepUp()
{
    if (minValue() > maxValue()) {
        return;
    }
    int sec = d->ed->mapSection( d->ed->focusSection() );
    bool accepted = TRUE;
    switch( sec ) {
    case 0:
        do {
            d->h = (d->h + 1) % 24;
        } while (outOfRange(d->h, d->m, d->s));
	break;
    case 1:
        do {
            d->m = (d->m + 1) % 60;
        } while (outOfRange(d->h, d->m, d->s));
	break;
    case 2:
        do {
            d->s = (d->s + 1) % 60;
        } while (outOfRange(d->h, d->m, d->s));
	break;
    case 3:
	if ( d->h < 12 )
	    setHour( d->h+12 );
	else
	    setHour( d->h-12 );
	break;
    default:
	accepted = FALSE;
#ifdef QT_CHECK_RANGE
	qWarning( "QTimeEdit::stepUp: Focus section out of range!" );
#endif
	break;
    }
    if ( accepted ) {
	d->changed = FALSE;
	emit valueChanged( time() );
    }
    d->ed->repaint( d->ed->rect(), FALSE );
}


/*! \reimp

*/

void QTimeEdit::stepDown()
{
    if (minValue() > maxValue()) {
        return;
    }

    int sec = d->ed->mapSection( d->ed->focusSection() );
    bool accepted = TRUE;
    switch( sec ) {
    case 0:
        do {
            if (--d->h < 0)
                d->h = 23;
        } while (outOfRange(d->h, d->m, d->s));
	break;
    case 1:
        do {
            if (--d->m < 0)
                d->m = 59;
        } while (outOfRange(d->h, d->m, d->s));
	break;
    case 2:
        do {
            if (--d->s < 0)
                d->s = 59;
        } while (outOfRange(d->h, d->m, d->s));
	break;
    case 3:
	if ( d->h > 11 )
	    setHour( d->h-12 );
	else
	    setHour( d->h+12 );
	break;
    default:
	accepted = FALSE;
#ifdef QT_CHECK_RANGE
	qWarning( "QTimeEdit::stepDown: Focus section out of range!" );
#endif
	break;
    }
    if ( accepted ) {
	d->changed = FALSE;
	emit valueChanged( time() );
    }
    d->ed->repaint( d->ed->rect(), FALSE );
}


/*!
    Returns the formatted number for section \a sec. This will
    correspond to either the hour, minute or second section, depending
    on \a sec.
*/

QString QTimeEdit::sectionFormattedText( int sec )
{
    QString txt;
    txt = sectionText( sec );
    txt = txt.rightJustify( 2, QDATETIMEEDIT_HIDDEN_CHAR );
    int offset = sec*2+sec*separator().length() + txt.length();
    if ( d->typing && sec == d->ed->focusSection() )
	d->ed->setSectionSelection( sec, offset - txt.length(), offset );
    else
	d->ed->setSectionSelection( sec, offset - txt.length(), offset );

    return txt;
}


/*! \reimp

*/

bool QTimeEdit::setFocusSection( int sec )
{
    if ( sec != d->ed->focusSection() ) {
	killTimer( d->timerId );
	d->overwrite = TRUE;
	d->typing = FALSE;
	QString txt = sectionText( sec );
	txt = txt.rightJustify( 2, QDATETIMEEDIT_HIDDEN_CHAR );
	int offset = sec*2+sec*separator().length() + txt.length();
	d->ed->setSectionSelection( sec, offset - txt.length(), offset );
	if ( d->changed ) {
	    emit valueChanged( time() );
	    d->changed = FALSE;
	}
    }
    return d->ed->setFocusSection( sec );
}


/*!
    Sets the hour to \a h, which must be a valid hour, i.e. in the
    range 0..24.
*/

void QTimeEdit::setHour( int h )
{
    if ( h < 0 )
	h = 0;
    if ( h > 23 )
	h = 23;
    d->h = h;
}


/*!
    Sets the minute to \a m, which must be a valid minute, i.e. in the
    range 0..59.
*/

void QTimeEdit::setMinute( int m )
{
    if ( m < 0 )
	m = 0;
    if ( m > 59 )
	m = 59;
    d->m = m;
}


/*!
    Sets the second to \a s, which must be a valid second, i.e. in the
    range 0..59.
*/

void QTimeEdit::setSecond( int s )
{
    if ( s < 0 )
	s = 0;
    if ( s > 59 )
	s = 59;
    d->s = s;
}


/*! \internal

  Returns the text of section \a sec.

*/

QString QTimeEdit::sectionText( int sec )
{
    sec = d->ed->mapSection( sec );

    QString txt;
    switch( sec ) {
    case 0:
	if ( !(d->display & AMPM) || ( d->h < 13 && d->h ) ) {    // I wished the day stared at 0:00 for everybody
	    txt = QString::number( d->h );
	} else {
	    if ( d->h )
		txt = QString::number( d->h - 12 );
	    else
		txt = "12";
	}
	break;
    case 1:
	txt = QString::number( d->m );
	break;
    case 2:
	txt = QString::number( d->s );
	break;
    case 3:
	if ( d->h < 12 ) {
	    if ( lAM )
		txt = *lAM;
	    else
		txt = QString::fromLatin1( "AM" );
	} else {
	    if ( lPM )
		txt = *lPM;
	    else
		txt = QString::fromLatin1( "PM" );
	}
	break;
    default:
	break;
    }
    return txt;
}


/*! \internal
 Returns TRUE if \a h, \a m, and \a s are out of range.
 */

bool QTimeEdit::outOfRange( int h, int m, int s ) const
{
    if ( QTime::isValid( h, m, s ) ) {
	QTime currentTime( h, m, s );
	if ( currentTime > maxValue() ||
	     currentTime < minValue() )
	    return TRUE;
	else
	    return FALSE;
    }
    return TRUE;
}

/*! \reimp

*/

void QTimeEdit::addNumber( int sec, int num )
{
    if ( sec == -1 )
	return;
    sec = d->ed->mapSection( sec );
    killTimer( d->timerId );
    bool overwrite = FALSE;
    bool accepted = FALSE;
    d->typing = TRUE;
    QString txt;

    switch( sec ) {
    case 0:
	txt = ( d->display & AMPM && d->h > 12 ) ?
	    QString::number( d->h - 12 ) : QString::number( d->h );

	if ( d->overwrite || txt.length() == 2 ) {
	    if ( d->display & AMPM && num == 0 )
		break; // Don't process 0 in 12 hour clock mode
	    if ( d->display & AMPM && d->h > 11 )
		num += 12;
	    if ( !outOfRange( num, d->m, d->s ) ) {
		accepted = TRUE;
		d->h = num;
	    }
	} else {
	    txt += QString::number( num );
	    int temp = txt.toInt();

	    if ( d->display & AMPM ) {
		if ( temp == 12 ) {
		    if ( d->h < 12 ) {
			temp = 0;
		    }
		    accepted = TRUE;
		} else if ( outOfRange( temp + 12, d->m, d->s ) ) {
		    txt = QString::number( d->h );
		} else {
		    if ( d->h > 11 ) {
			temp += 12;
		    }
		    accepted = TRUE;
		}
	    } else if ( !(d->display & AMPM) && outOfRange( temp, d->m, d->s ) ) {
		txt = QString::number( d->h );
	    } else {
		accepted = TRUE;
	    }

	    if ( accepted )
		d->h = temp;

	    if ( d->adv && txt.length() == 2 ) {
		setFocusSection( d->ed->focusSection()+1 );
		overwrite = TRUE;
	    }
	}
	break;

    case 1:
	txt = QString::number( d->m );
	if ( d->overwrite || txt.length() == 2 ) {
	    if ( !outOfRange( d->h, num, d->s ) ) {
		accepted = TRUE;
		d->m = num;
	    }
	} else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 59 )
		temp = num;
	    if ( outOfRange( d->h, temp, d->s ) )
		txt = QString::number( d->m );
	    else {
		accepted = TRUE;
		d->m = temp;
	    }
	    if ( d->adv && txt.length() == 2 ) {
		setFocusSection( d->ed->focusSection()+1 );
		overwrite = TRUE;
	    }
	}
	break;

    case 2:
	txt = QString::number( d->s );
	if ( d->overwrite || txt.length() == 2 ) {
	    if ( !outOfRange( d->h, d->m, num ) ) {
		accepted = TRUE;
		d->s = num;
	    }
	} else {
	    txt += QString::number( num );
	    int temp = txt.toInt();
	    if ( temp > 59 )
		temp = num;
	    if ( outOfRange( d->h, d->m, temp ) )
		txt = QString::number( d->s );
	    else {
		accepted = TRUE;
		d->s = temp;
	    }
	    if ( d->adv && txt.length() == 2 ) {
		setFocusSection( d->ed->focusSection()+1 );
		overwrite = TRUE;
	    }
	}
	break;

    case 3:
	break;

    default:
	break;
    }
    d->changed = !accepted;
    if ( accepted )
	emit valueChanged( time() );
    d->overwrite = overwrite;
    d->timerId = startTimer( qApp->doubleClickInterval()*4 );
    d->ed->repaint( d->ed->rect(), FALSE );
}


/*!
  \internal

  Function which is called whenever the user tries to
  remove the first number from \a sec by pressing the backspace key.
*/

void QTimeEdit::removeFirstNumber( int sec )
{
    if ( sec == -1 )
	return;
    sec = d->ed->mapSection( sec );
    QString txt;
    switch( sec ) {
    case 0:
	txt = QString::number( d->h );
	break;
    case 1:
	txt = QString::number( d->m );
	break;
    case 2:
	txt = QString::number( d->s );
	break;
    }
    txt = txt.mid( 1, txt.length() ) + "0";
    switch( sec ) {
    case 0:
	d->h = txt.toInt();
	break;
    case 1:
	d->m = txt.toInt();
	break;
    case 2:
	d->s = txt.toInt();
	break;
    }
    d->ed->repaint( d->ed->rect(), FALSE );
}

/*! \reimp

*/
void QTimeEdit::removeLastNumber( int sec )
{
    if ( sec == -1 )
	return;
    sec = d->ed->mapSection( sec );
    QString txt;
    switch( sec ) {
    case 0:
	txt = QString::number( d->h );
	break;
    case 1:
	txt = QString::number( d->m );
	break;
    case 2:
	txt = QString::number( d->s );
	break;
    }
    txt = txt.mid( 0, txt.length()-1 );
    switch( sec ) {
    case 0:
	d->h = txt.toInt();
	break;
    case 1:
	d->m = txt.toInt();
	break;
    case 2:
	d->s = txt.toInt();
	break;
    }
    d->ed->repaint( d->ed->rect(), FALSE );
}

/*! \reimp
 */
void QTimeEdit::resizeEvent( QResizeEvent * )
{
    d->controls->resize( width(), height() );
}

/*! \reimp
*/
QSize QTimeEdit::sizeHint() const
{
    constPolish();
    QFontMetrics fm( font() );
    int fw = style().pixelMetric( QStyle::PM_DefaultFrameWidth, this );
    int h = fm.lineSpacing() + 2;
    int w = 2 + fm.width( '9' ) * 6 + fm.width( d->ed->separator() ) * 2 +
	d->controls->upRect().width() + fw * 4;
    if ( d->display & AMPM ) {
	if ( lAM )
	    w += fm.width( *lAM ) + 4;
	else
	    w += fm.width( QString::fromLatin1( "AM" ) ) + 4;
    }

    return QSize( w, QMAX(h + fw * 2,20) ).expandedTo( QApplication::globalStrut() );
}

/*! \reimp
*/
QSize QTimeEdit::minimumSizeHint() const
{
    return sizeHint();
}

/*!
    \internal
    Enables/disables the push buttons according to the min/max time
    for this widget.
*/

// ### Remove in 4.0?

void QTimeEdit::updateButtons()
{
    if ( !isEnabled() )
	return;

    bool upEnabled = time() < maxValue();
    bool downEnabled = time() > minValue();

    d->controls->setUpEnabled( upEnabled );
    d->controls->setDownEnabled( downEnabled );
}


class QDateTimeEditPrivate
{
public:
    bool adv;
};

/*!
    \class QDateTimeEdit qdatetimeedit.h
    \brief The QDateTimeEdit class combines a QDateEdit and QTimeEdit
    widget into a single widget for editing datetimes.

    \ingroup advanced
    \ingroup time
    \mainclass

    QDateTimeEdit consists of a QDateEdit and QTimeEdit widget placed
    side by side and offers the functionality of both. The user can
    edit the date and time by using the keyboard or the arrow keys to
    increase/decrease date or time values. The Tab key can be used to
    move from section to section within the QDateTimeEdit widget, and
    the user can be moved automatically when they complete a section
    using setAutoAdvance(). The datetime can be set with
    setDateTime().

    The date format is read from the system's locale settings. It is
    set to year, month, day order if that is not possible. See
    QDateEdit::setOrder() to change this. Times appear in the order
    hours, minutes, seconds using the 24 hour clock.

    It is recommended that the QDateTimeEdit is initialised with a
    datetime, e.g.
    \code
    QDateTimeEdit *dateTimeEdit = new QDateTimeEdit( QDateTime::currentDateTime(), this );
    dateTimeEdit->dateEdit()->setRange( QDateTime::currentDate(),
					QDateTime::currentDate().addDays( 7 ) );
    \endcode
    Here we've created a new QDateTimeEdit set to the current date and
    time, and set the date to have a minimum date of now and a maximum
    date of a week from now.

    Terminology: A QDateEdit widget consists of three 'sections', one
    each for the year, month and day. Similarly a QTimeEdit consists
    of three sections, one each for the hour, minute and second. The
    character that separates each date section is specified with
    setDateSeparator(); similarly setTimeSeparator() is used for the
    time sections.

    \img datetimewidgets.png Date Time Widgets

    \sa QDateEdit QTimeEdit
*/

/*!
    Constructs an empty datetime edit with parent \a parent and called
    \a name.
*/
QDateTimeEdit::QDateTimeEdit( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    init();
}


/*!
    \overload

    Constructs a datetime edit with the initial value \a datetime,
    parent \a parent and called \a name.
*/
QDateTimeEdit::QDateTimeEdit( const QDateTime& datetime,
			      QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    init();
    setDateTime( datetime );
}



/*!
    Destroys the object and frees any allocated resources.
*/

QDateTimeEdit::~QDateTimeEdit()
{
    delete d;
}


/*!
    \reimp

    Intercepts and handles resize events which have special meaning
    for the QDateTimeEdit.
*/

void QDateTimeEdit::resizeEvent( QResizeEvent * )
{
    int dw = de->sizeHint().width();
    int tw = te->sizeHint().width();
    int w = width();
    int h = height();
    int extra = w - ( dw + tw );

    if ( tw + extra < 0 ) {
	dw = w;
    } else {
	dw += 9 * extra / 16;
    }
    tw = w - dw;

    de->setGeometry( 0, 0, dw, h );
    te->setGeometry( dw, 0, tw, h );
}

/*! \reimp
*/

QSize QDateTimeEdit::minimumSizeHint() const
{
    QSize dsh = de->minimumSizeHint();
    QSize tsh = te->minimumSizeHint();
    return QSize( dsh.width() + tsh.width(),
		  QMAX( dsh.height(), tsh.height() ) );
}

/*!  \internal
 */

void QDateTimeEdit::init()
{
    d = new QDateTimeEditPrivate();
    de = new QDateEdit( this, "qt_datetime_dateedit" );
    te = new QTimeEdit( this, "qt_datetime_timeedit" );
    d->adv = FALSE;
    connect( de, SIGNAL( valueChanged(const QDate&) ),
	     this, SLOT( newValue(const QDate&) ) );
    connect( te, SIGNAL( valueChanged(const QTime&) ),
	     this, SLOT( newValue(const QTime&) ) );
    setFocusProxy( de );
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
}

/*! \reimp
 */

QSize QDateTimeEdit::sizeHint() const
{
    constPolish();
    QSize dsh = de->sizeHint();
    QSize tsh = te->sizeHint();
    return QSize( dsh.width() + tsh.width(),
		  QMAX( dsh.height(), tsh.height() ) );
}

/*!
    \property QDateTimeEdit::dateTime
    \brief the editor's datetime value

    The datetime edit's datetime which may be an invalid datetime.
*/

void QDateTimeEdit::setDateTime( const QDateTime & dt )
{
    if ( dt.isValid() ) {
	de->setDate( dt.date() );
	te->setTime( dt.time() );
	emit valueChanged( dt );
    }
}

QDateTime QDateTimeEdit::dateTime() const
{
    return QDateTime( de->date(), te->time() );
}

/*!
    \fn void QDateTimeEdit::valueChanged( const QDateTime& datetime )

    This signal is emitted every time the date or time changes. The \a
    datetime argument is the new datetime.
*/


/*! \internal

  Re-emits the value \a d.
 */

void QDateTimeEdit::newValue( const QDate& )
{
    QDateTime dt = dateTime();
    emit valueChanged( dt );
}

/*! \internal
  \overload
  Re-emits the value \a t.
 */

void QDateTimeEdit::newValue( const QTime& )
{
    QDateTime dt = dateTime();
    emit valueChanged( dt );
}


/*!
    Sets the auto advance property of the editor to \a advance. If set
    to TRUE, the editor will automatically advance focus to the next
    date or time section if the user has completed a section.
*/

void QDateTimeEdit::setAutoAdvance( bool advance )
{
    de->setAutoAdvance( advance );
    te->setAutoAdvance( advance );
}

/*!
    Returns TRUE if auto-advance is enabled, otherwise returns FALSE.

    \sa setAutoAdvance()
*/

bool QDateTimeEdit::autoAdvance() const
{
    return de->autoAdvance();
}

/*!
    \fn QDateEdit* QDateTimeEdit::dateEdit()

    Returns the internal widget used for editing the date part of the
    datetime.
*/

/*!
    \fn QTimeEdit* QDateTimeEdit::timeEdit()

    Returns the internal widget used for editing the time part of the
    datetime.
*/

#include "qdatetimeedit.moc"

#endif
