/****************************************************************************
** $Id: qfontdialog.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QFontDialog
**
** Created : 970605
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
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

#include "qwindowdefs.h"

#ifndef QT_NO_FONTDIALOG

#include "qfontdialog.h"

#include "qlineedit.h"
#include "qlistbox.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qcombobox.h"
#include "qlayout.h"
#include "qvgroupbox.h"
#include "qhgroupbox.h"
#include "qlabel.h"
#include "qapplication.h"
#include "qfontdatabase.h"
#include "qstyle.h"
#include <private/qfontdata_p.h>
#include <qvalidator.h>

/*!
  \class QFontDialog qfontdialog.h
  \ingroup dialogs
  \mainclass
  \brief The QFontDialog class provides a dialog widget for selecting a font.

  The usual way to use this class is to call one of the static convenience
  functions, e.g. getFont().

  Examples:

  \code
    bool ok;
    QFont font = QFontDialog::getFont(
		    &ok, QFont( "Helvetica [Cronyx]", 10 ), this );
    if ( ok ) {
	// font is set to the font the user selected
    } else {
	// the user canceled the dialog; font is set to the initial
	// value, in this case Helvetica [Cronyx], 10
    }
  \endcode

    The dialog can also be used to set a widget's font directly:
  \code
    myWidget.setFont( QFontDialog::getFont( 0, myWidget.font() ) );
  \endcode
  If the user clicks OK the font they chose will be used for myWidget,
  and if they click Cancel the original font is used.

  \sa QFont QFontInfo QFontMetrics

  <img src=qfontdlg-w.png>
*/

class QFontDialogPrivate
{
public:
    QFontDialogPrivate() : script( QFontPrivate::defaultScript ) {};
    QLabel * familyAccel;
    QLineEdit * familyEdit;
    QListBox * familyList;

    QLabel * styleAccel;
    QLineEdit * styleEdit;
    QListBox * styleList;

    QLabel * sizeAccel;
    QLineEdit * sizeEdit;
    QListBox * sizeList;

    QVGroupBox * effects;
    QCheckBox * strikeout;
    QCheckBox * underline;
    QComboBox * color;

    QHGroupBox * sample;
    QLineEdit * sampleEdit;

    QLabel * scriptAccel;
    QComboBox * scriptCombo;

    QPushButton * ok;
    QPushButton * cancel;

    QBoxLayout * buttonLayout;
    QBoxLayout * effectsLayout;
    QBoxLayout * sampleLayout;
    QBoxLayout * sampleEditLayout;

    QFontDatabase fdb;

    QString       family;
    QFont::Script script;
    QString       style;
    int           size;

    bool smoothScalable;
};


/*!
  \internal
  Constructs a standard font dialog.

  Use setFont() to set the initial font attributes.

  The \a parent, \a name, \a modal and \a f parameters are passed to
  the QDialog constructor.

  \sa getFont()
*/

QFontDialog::QFontDialog( QWidget *parent, const char *name,
			  bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
    setSizeGripEnabled( TRUE );
    d = new QFontDialogPrivate;
    // grid
    d->familyEdit = new QLineEdit( this, "font family I" );
    d->familyEdit->setReadOnly( TRUE );
    d->familyList = new QListBox( this, "font family II" );
    d->familyEdit->setFocusProxy( d->familyList );

    d->familyAccel
	= new QLabel( d->familyList, tr("&Font"), this, "family accelerator" );
    d->familyAccel->setIndent( 2 );

    d->styleEdit = new QLineEdit( this, "font style I" );
    d->styleEdit->setReadOnly( TRUE );
    d->styleList = new QListBox( this, "font style II" );
    d->styleEdit->setFocusProxy( d->styleList );

    d->styleAccel
	= new QLabel( d->styleList, tr("Font st&yle"), this, "style accelerator" );
    d->styleAccel->setIndent( 2 );

    d->sizeEdit = new QLineEdit( this, "font size I" );
    d->sizeEdit->setFocusPolicy( ClickFocus );
    QIntValidator *validator = new QIntValidator( 1, 512, this );
    d->sizeEdit->setValidator( validator );
    d->sizeList = new QListBox( this, "font size II" );

    d->sizeAccel
	= new QLabel ( d->sizeEdit, tr("&Size"), this, "size accelerator" );
    d->sizeAccel->setIndent( 2 );

    // effects box
    d->effects = new QVGroupBox( tr("Effects"), this, "font effects" );
    d->strikeout = new QCheckBox( d->effects, "strikeout on/off" );
    d->strikeout->setText( tr("Stri&keout") );
    d->underline = new QCheckBox( d->effects, "underline on/off" );
    d->underline->setText( tr("&Underline") );

    d->sample = new QHGroupBox( tr("Sample"), this, "sample text" );
    d->sampleEdit = new QLineEdit( d->sample, "r/w sample text" );
    d->sampleEdit->setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored) );
    d->sampleEdit->setAlignment( AlignCenter );
    // Note that the sample text is *not* translated with tr(), as the
    // characters used depend on the charset encoding.
    d->sampleEdit->setText( "AaBbYyZz" );

    d->scriptCombo = new QComboBox( FALSE, this, "font encoding" );

    d->scriptAccel
	= new QLabel( d->scriptCombo, tr("Scr&ipt"), this,"encoding label");
    d->scriptAccel->setIndent( 2 );

    d->size = 0;
    d->smoothScalable = FALSE;

    connect( d->scriptCombo, SIGNAL(activated(int)),
	     SLOT(scriptHighlighted(int)) );
    connect( d->familyList, SIGNAL(highlighted(int)),
	     SLOT(familyHighlighted(int)) );
    connect( d->styleList, SIGNAL(highlighted(int)),
	     SLOT(styleHighlighted(int)) );
    connect( d->sizeList, SIGNAL(highlighted(const QString&)),
	     SLOT(sizeHighlighted(const QString&)) );
    connect( d->sizeEdit, SIGNAL(textChanged(const QString&)),
	     SLOT(sizeChanged(const QString&)) );

    connect( d->strikeout, SIGNAL(clicked()),
	     SLOT(updateSample()) );
    connect( d->underline, SIGNAL(clicked()),
	     SLOT(updateSample()) );

    (void)d->familyList->sizeHint();
    (void)d->styleList->sizeHint();
    (void)d->sizeList->sizeHint();

    for (int i = 0; i < QFont::NScripts; i++) {
	QString scriptname = QFontDatabase::scriptName((QFont::Script) i);
	if ( !scriptname.isEmpty() )
	    d->scriptCombo->insertItem( scriptname );
    }

    updateFamilies();
    if ( d->familyList->count() != 0 )
	d->familyList->setCurrentItem( 0 );

    // grid layout
    QGridLayout * mainGrid = new QGridLayout( this, 9, 6, 12, 0 );

    mainGrid->addWidget( d->familyAccel, 0, 0 );
    mainGrid->addWidget( d->familyEdit, 1, 0 );
    mainGrid->addWidget( d->familyList, 2, 0 );

    mainGrid->addWidget( d->styleAccel, 0, 2 );
    mainGrid->addWidget( d->styleEdit, 1, 2 );
    mainGrid->addWidget( d->styleList, 2, 2 );

    mainGrid->addWidget( d->sizeAccel, 0, 4 );
    mainGrid->addWidget( d->sizeEdit, 1, 4 );
    mainGrid->addWidget( d->sizeList, 2, 4 );

    mainGrid->setColStretch( 0, 38 );
    mainGrid->setColStretch( 2, 24 );
    mainGrid->setColStretch( 4, 10 );

    mainGrid->addColSpacing( 1, 6 );
    mainGrid->addColSpacing( 3, 6 );
    mainGrid->addColSpacing( 5, 6 );

    mainGrid->addRowSpacing( 3, 12 );

    mainGrid->addWidget( d->effects, 4, 0 );

    mainGrid->addMultiCellWidget( d->sample, 4, 7, 2, 4 );

    mainGrid->addWidget( d->scriptAccel, 5, 0 );
    mainGrid->addRowSpacing( 6, 2 );
    mainGrid->addWidget( d->scriptCombo, 7, 0 );

    mainGrid->addRowSpacing( 8, 12 );

    QHBoxLayout *buttonBox = new QHBoxLayout;
    mainGrid->addMultiCell( buttonBox, 9, 9, 0, 4 );

    buttonBox->addStretch( 1 );
    QString okt = modal ? tr("OK") : tr("Apply");
    d->ok = new QPushButton( okt, this, "accept font selection" );
    buttonBox->addWidget( d->ok );
    if ( modal )
	connect( d->ok, SIGNAL(clicked()), SLOT(accept()) );
    d->ok->setDefault( TRUE );

    buttonBox->addSpacing( 12 );

    QString cancelt = modal ? tr("Cancel") : tr("Close");
    d->cancel = new QPushButton( cancelt, this, "cancel/close" );
    buttonBox->addWidget( d->cancel );
    connect( d->cancel, SIGNAL(clicked()), SLOT(reject()) );

    resize( 500, 360 );

    d->sizeEdit->installEventFilter( this );
    d->familyList->installEventFilter( this );
    d->styleList->installEventFilter( this );
    d->sizeList->installEventFilter( this );

    d->familyList->setFocus();
}

/*!
  \internal
 Destroys the font dialog and frees up its storage.
*/

QFontDialog::~QFontDialog()
{
    delete d;
    d = 0;
}

/*!
  Executes a modal font dialog and returns a font.

  If the user clicks OK, the selected font is returned. If the user
  clicks Cancel, the \a initial font is returned.

  The dialog is called \a name, with the parent \a parent.
  \a initial is the initially selected font.
  If the \a ok parameter is not-null, \e *\a ok is set to TRUE if the
  user clicked OK, and set to FALSE if the user clicked Cancel.

  This static function is less flexible than the full QFontDialog
  object, but is convenient and easy to use.

  Examples:
  \code
    bool ok;
    QFont font = QFontDialog::getFont( &ok, QFont( "Times", 12 ), this );
    if ( ok ) {
	// font is set to the font the user selected
    } else {
	// the user canceled the dialog; font is set to the initial
	// value, in this case Times, 12.
    }
  \endcode

    The dialog can also be used to set a widget's font directly:
  \code
    myWidget.setFont( QFontDialog::getFont( 0, myWidget.font() ) );
  \endcode
  In this example, if the user clicks OK the font they chose will be
  used, and if they click Cancel the original font is used.
*/
QFont QFontDialog::getFont( bool *ok, const QFont &initial,
			    QWidget *parent, const char* name)
{
    return getFont( ok, &initial, parent, name );
}

/*!
    \overload

  Executes a modal font dialog and returns a font.

  If the user clicks OK, the selected font is returned. If the user
  clicks Cancel, the Qt default font is returned.

  The dialog is called \a name, with parent \a parent.
  If the \a ok parameter is not-null, \e *\a ok is set to TRUE if the
  user clicked OK, and FALSE if the user clicked Cancel.

  This static function is less functional than the full QFontDialog
  object, but is convenient and easy to use.

  Example:
  \code
    bool ok;
    QFont font = QFontDialog::getFont( &ok, this );
    if ( ok ) {
	// font is set to the font the user selected
    } else {
	// the user canceled the dialog; font is set to the default
	// application font, QApplication::font()
    }
  \endcode

*/
QFont QFontDialog::getFont( bool *ok, QWidget *parent,const char* name)
{
    return getFont( ok, 0, parent, name );
}

QFont QFontDialog::getFont( bool *ok, const QFont *def,
			    QWidget *parent, const char* name)
{
    QFont result;
    if ( def )
	result = *def;

    QFontDialog *dlg = new QFontDialog( parent, name, TRUE );

    dlg->setFont( ( def ? *def : QFont() ) );
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg->setCaption( tr("Select Font") );
#endif

    bool res = (dlg->exec() == QDialog::Accepted);
    if ( res )
	result = dlg->font();
    if ( ok )
	*ok = res;
    delete dlg;
    return result;
}


/*!
    \internal
    An event filter to make the Up, Down, PageUp and PageDown keys work
    correctly in the line edits. The source of the event is the object
    \a o and the event is \a e.
*/

bool QFontDialog::eventFilter( QObject * o , QEvent * e )
{
    if ( e->type() == QEvent::KeyPress) {
	QKeyEvent * k = (QKeyEvent *)e;
	if ( o == d->sizeEdit &&
        (k->key() == Key_Up ||
	     k->key() == Key_Down ||
         k->key() == Key_Prior ||
         k->key() == Key_Next) ) {

	    int ci = d->sizeList->currentItem();
	    (void)QApplication::sendEvent( d->sizeList, k );

	    if ( ci != d->sizeList->currentItem() &&
		style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this))
		d->sizeEdit->selectAll();
	    return TRUE;
	} else if ( ( o == d->familyList || o == d->styleList ) &&
	            ( k->key() == Key_Return || k->key() == Key_Enter) ) {
	    k->accept();
        accept();
	    return TRUE;
	}
    } else if ( e->type() == QEvent::FocusIn &&
		style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) ) {
	if ( o == d->familyList )
	    d->familyEdit->selectAll();
	else if ( o == d->styleList )
	    d->styleEdit->selectAll();
	else if ( o == d->sizeList )
	    d->sizeEdit->selectAll();
    } else if ( e->type() == QEvent::MouseButtonPress && o == d->sizeList ) {
	    d->sizeEdit->setFocus();
    }
    return QDialog::eventFilter( o, e );
}

#ifdef Q_WS_MAC
// #define SHOW_FONTS_IN_FAMILIES
#endif

#ifdef SHOW_FONTS_IN_FAMILIES
#include "qpainter.h"
#include <sizeedit.h>

class QListBoxFontText : public QListBoxText
{
    QFont cfont;
public:
    QListBoxFontText( const QString & text );
    ~QListBoxFontText() { }

    int	 height( const QListBox * ) const;
    int	 width( const QListBox * )  const;

protected:
    void  paint( QPainter * );
};

QListBoxFontText::QListBoxFontText( const QString & text )
    : QListBoxText(text), cfont(text)
{
}

int QListBoxFontText::height( const QListBox * ) const
{
    QFontMetrics fm(cfont);
    return QMAX( fm.lineSpacing() + 2, QApplication::globalStrut().height() );
}

int QListBoxFontText::width( const QListBox * )  const
{
    QFontMetrics fm(cfont);
    return QMAX( fm.width( text() ) + 6, QApplication::globalStrut().width() );
}

void QListBoxFontText::paint( QPainter *painter )
{
    painter->save();
    painter->setFont(cfont);
    QListBoxText::paint(painter);
    painter->restore();
}

#endif

/*!
  \internal
    Updates the contents of the "font family" list box. This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateFamilies()
{
    d->familyList->blockSignals( TRUE );

    enum match_t { MATCH_NONE=0, MATCH_LAST_RESORT=1, MATCH_APP=2, MATCH_FALLBACK, MATCH_FAMILY=3 };

    QStringList familyNames = d->fdb.families(d->script);
    {
	// merge the unicode/unknown family list with the above list.
	QStringList l = d->fdb.families(QFont::Unicode) +
			d->fdb.families(QFont::UnknownScript);
	QStringList::ConstIterator it = l.begin(), end = l.end();
	for (; it != end; ++it) {
	    if (! familyNames.contains(*it))
		familyNames << *it;
	}
    }

    familyNames.sort();

    d->familyList->clear();
#ifdef SHOW_FONTS_IN_FAMILIES
    QStringList::Iterator it = familyNames.begin();
    int idx = 0;
    for( ; it != familyNames.end() ; ++it )
	d->familyList->insertItem(new QListBoxFontText(*it), idx++);
#else
    d->familyList->insertStringList( familyNames );
#endif

    QString foundryName1, familyName1, foundryName2, familyName2;
    int bestFamilyMatch = -1;
    match_t bestFamilyType = MATCH_NONE;

    QFont f;

    // ##### do the right thing for a list of family names in the font.
    QFontDatabase::parseFontName(d->family, foundryName1, familyName1);

    QStringList::Iterator it = familyNames.begin();
    int i = 0;
    for( ; it != familyNames.end(); ++it, ++i ) {

	QFontDatabase::parseFontName(*it, foundryName2, familyName2);

	//try to match..
	if ( familyName1 == familyName2 ) {
	    bestFamilyType = MATCH_FAMILY;
	    if ( foundryName1 == foundryName2 ) {
		bestFamilyMatch = i;
		break;
	    }
	    if ( bestFamilyMatch < MATCH_FAMILY )
		bestFamilyMatch = i;
	}

	//and try some fall backs
	match_t type = MATCH_NONE;
	if ( bestFamilyType <= MATCH_NONE && familyName2 == f.lastResortFamily() )
		type = MATCH_LAST_RESORT;
	if ( bestFamilyType <= MATCH_LAST_RESORT && familyName2 == f.family() )
		type = MATCH_APP;
	// ### add fallback for script
	if ( type != MATCH_NONE ) {
	    bestFamilyType = type;
	    bestFamilyMatch = i;
	}
    }

    if (i != -1 && bestFamilyType != MATCH_NONE)
	d->familyList->setCurrentItem(bestFamilyMatch);
    else
	d->familyList->setCurrentItem( 0 );
    d->familyEdit->setText( d->familyList->currentText() );
    if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	 d->familyList->hasFocus() )
	d->familyEdit->selectAll();

    d->familyList->blockSignals( FALSE );
    updateStyles();
}

/*!
    \internal
    Updates the contents of the "font style" list box. This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateStyles()
{
    d->styleList->blockSignals( TRUE );

    d->styleList->clear();

    QStringList styles = d->fdb.styles( d->familyList->currentText() );

    if ( styles.isEmpty() ) {
	d->styleEdit->clear();
	d->smoothScalable = FALSE;
    } else {
	d->styleList->insertStringList( styles );

	if ( !d->style.isEmpty() ) {
	    bool found = FALSE;
	    bool first = TRUE;
	    QString cstyle = d->style;
	redo:
	    for ( int i = 0 ; i < (int)d->styleList->count() ; i++ ) {
		if ( cstyle == d->styleList->text(i) ) {
		    d->styleList->setCurrentItem( i );
		    found = TRUE;
		    break;
		}
	    }
	    if (!found && first) {
		if (cstyle.contains("Italic")) {
		    cstyle.replace("Italic", "Oblique");
		    first = FALSE;
		    goto redo;
		} else if (cstyle.contains("Oblique")) {
		    cstyle.replace("Oblique", "Italic");
		    first = FALSE;
		    goto redo;
		}
	    }
	    if ( !found )
		d->styleList->setCurrentItem( 0 );
	}

	d->styleEdit->setText( d->styleList->currentText() );
	if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	     d->styleList->hasFocus() )
	    d->styleEdit->selectAll();

	d->smoothScalable = d->fdb.isSmoothlyScalable( d->familyList->currentText(), d->styleList->currentText() );
    }

    d->styleList->blockSignals( FALSE );

    updateSizes();
}

/*!
    \internal
    Updates the contents of the "font size" list box. This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateSizes()
{
    d->sizeList->blockSignals( TRUE );

    d->sizeList->clear();

    if ( !d->familyList->currentText().isEmpty() ) {
	QValueList<int> sizes = d->fdb.pointSizes( d->familyList->currentText(), d->styleList->currentText() );

	int i = 0;
	bool found = FALSE;
	for( QValueList<int>::iterator it = sizes.begin() ; it != sizes.end(); ++it ) {
	    d->sizeList->insertItem( QString::number( *it ) );
	    if ( !found && *it >= d->size ) {
		d->sizeList->setCurrentItem( i );
		found = TRUE;
	    }
	    ++i;
	}
	if ( !found ) {
	    // we request a size bigger than the ones in the list, select the biggest one
	    d->sizeList->setCurrentItem( d->sizeList->count() - 1 );
	}

	d->sizeEdit->blockSignals( TRUE );
	d->sizeEdit->setText( ( d->smoothScalable ? QString::number( d->size ) : d->sizeList->currentText() ) );
	if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	     d->sizeList->hasFocus() )
	    d->sizeEdit->selectAll();
	d->sizeEdit->blockSignals( FALSE );
    } else {
	d->sizeEdit->clear();
    }

    d->sizeList->blockSignals( FALSE );
    updateSample();
}

void QFontDialog::updateSample()
{
    if ( d->familyList->currentText().isEmpty() )
	d->sampleEdit->clear();
    else
	d->sampleEdit->setFont( font() );
}

/*!
    \internal
*/
void QFontDialog::scriptHighlighted( int index )
{
    d->script = (QFont::Script)index;
    d->sampleEdit->setText( d->fdb.scriptSample( d->script ) );
    updateFamilies();
}

/*!
    \internal
*/
void QFontDialog::familyHighlighted( int i )
{
    d->family = d->familyList->text( i );
    d->familyEdit->setText( d->family );
    if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	 d->familyList->hasFocus() )
	d->familyEdit->selectAll();

    updateStyles();
}


/*!
    \internal
*/

void QFontDialog::styleHighlighted( int index )
{
    QString s = d->styleList->text( index );
    d->styleEdit->setText( s );
    if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	 d->styleList->hasFocus() )
	d->styleEdit->selectAll();

    d->style = s;

    updateSizes();
}


/*!
    \internal
*/

void QFontDialog::sizeHighlighted( const QString &s )
{
    d->sizeEdit->setText( s );
    if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	 d->sizeEdit->hasFocus() )
	d->sizeEdit->selectAll();

    d->size = s.toInt();
    updateSample();
}

/*!
    \internal
    This slot is called if the user changes the font size.
    The size is passed in the \a s argument as a \e string.
*/

void QFontDialog::sizeChanged( const QString &s )
{
    // no need to check if the conversion is valid, since we have an QIntValidator in the size edit
    int size = s.toInt();
    if ( d->size == size )
	return;

    d->size = size;
    if ( d->sizeList->count() != 0 ) {
	int i;
	for ( i = 0 ; i < (int)d->sizeList->count() - 1 ; i++ ) {
	    if ( d->sizeList->text(i).toInt() >= d->size )
		break;
	}
	d->sizeList->blockSignals( TRUE );
	d->sizeList->setCurrentItem( i );
	d->sizeList->blockSignals( FALSE );
    }
    updateSample();
}

/*!
  \internal
  Sets the font highlighted in the QFontDialog to font \a f.

  \sa font()
*/

void QFontDialog::setFont( const QFont &f )
{
    d->family = f.family();
    d->style = d->fdb.styleString( f );
    d->size = f.pointSize();
    if ( d->size == -1 ) {
	    QFontInfo fi( f );
	    d->size = fi.pointSize();
    }
    d->strikeout->setChecked( f.strikeOut() );
    d->underline->setChecked( f.underline() );

    updateFamilies();
}

/*!
  \internal
  Returns the font which the user has chosen.

  \sa setFont()
*/

QFont QFontDialog::font() const
{
    int pSize = d->sizeEdit->text().toInt();

    QFont f = d->fdb.font( d->familyList->currentText(), d->style, pSize );
    f.setStrikeOut( d->strikeout->isChecked() );
    f.setUnderline( d->underline->isChecked() );
    return f;
}

#endif
