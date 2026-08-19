/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Linguist.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*  TRANSLATOR TrWindow

  This is the application's main window.
*/

#include "trwindow.h"
#include "listviews.h"
#include "finddialog.h"
#include "msgedit.h"
#include "phrasebookbox.h"
#include "printout.h"
#include "about.h"
#include "phraselv.h"
#include "statistics.h"

#include <qaccel.h>
#include <qaction.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qdict.h>
#include <qdockarea.h>
#include <qdockwindow.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qprocess.h>
#include <qassistantclient.h>

#include <stdlib.h>

#define pagecurl_mask_width 53
#define pagecurl_mask_height 51
static const uchar pagecurl_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x0f, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xfe, 0xff,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0xc0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xfe, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0xfc,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x0f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 };

typedef QValueList<MetaTranslatorMessage> TML;

static const int ErrorMS = 600000; // for error messages
static const int MessageMS = 2500;

QPixmap * TrWindow::pxOn = 0;
QPixmap * TrWindow::pxOff = 0;
QPixmap * TrWindow::pxObsolete = 0;
QPixmap * TrWindow::pxDanger = 0;

enum Ending { End_None, End_FullStop, End_Interrobang, End_Colon,
	      End_Ellipsis };

static Ending ending( QString str )
{
    str = str.simplifyWhiteSpace();
    int ch = 0;
    if ( !str.isEmpty() )
	ch = str.right( 1 )[0].unicode();

    switch ( ch ) {
    case 0x002e: // full stop
	if ( str.endsWith(QString("...")) )
	    return End_Ellipsis;
	else
	    return End_FullStop;
    case 0x0589: // armenian full stop
    case 0x06d4: // arabic full stop
    case 0x3002: // ideographic full stop
	return End_FullStop;
    case 0x0021: // exclamation mark
    case 0x003f: // question mark
    case 0x00a1: // inverted exclamation mark
    case 0x00bf: // inverted question mark
    case 0x01c3: // latin letter retroflex click
    case 0x037e: // greek question mark
    case 0x061f: // arabic question mark
    case 0x203c: // double exclamation mark
    case 0x203d: // interrobang
    case 0x2048: // question exclamation mark
    case 0x2049: // exclamation question mark
    case 0x2762: // heavy exclamation mark ornament
	return End_Interrobang;
    case 0x003a: // colon
	return End_Colon;
    case 0x2026: // horizontal ellipsis
	return End_Ellipsis;
    default:
	return End_None;
    }
}

class Action : public QAction
{
public:
    Action( QPopupMenu *pop, const QString& menuText, QObject *receiver,
	    const char *member, const QString &imageName = QString::null,
	    int accel = 0, bool toggle = FALSE );
    Action( QPopupMenu *pop, const QString& menuText, int accel = 0,
	    bool toggle = FALSE );

    virtual void setWhatsThis( const QString& whatsThis );

    bool addToToolbar( QToolBar *tb, const QString& text );
};

Action::Action( QPopupMenu *pop, const QString& menuText, QObject *receiver,
		const char *member, const QString &imageName, int accel,
		bool toggle )
    : QAction( pop->parent(), (const char *) 0, toggle )
{
    setMenuText( menuText );
    setAccel( accel );

    if ( !imageName.isEmpty() ) {
	QPixmap enabledPix = QPixmap::fromMimeSource( imageName );
	QIconSet s( enabledPix );
	if ( imageName != "whatsthis.xpm" ) {
	    QPixmap disabledPix = QPixmap::fromMimeSource( "d_" + imageName );
	    s.setPixmap( disabledPix, QIconSet::Small, QIconSet::Disabled );
	}
	setIconSet( s );
    }
    QAction::addTo( pop );
    connect( this, SIGNAL(activated()), receiver, member );
}

Action::Action( QPopupMenu *pop, const QString& menuText, int accel,
		bool toggle )
    : QAction( pop->parent(), (const char *) 0, toggle )
{
    QAction::addTo( pop );
    setMenuText( menuText );
    setAccel( accel );
}

void Action::setWhatsThis( const QString& whatsThis )
{
    QAction::setWhatsThis( whatsThis );
    setStatusTip( whatsThis );
}

bool Action::addToToolbar( QToolBar *tb, const QString& text )
{
    setText( text );
    return QAction::addTo( tb );
}

const QPixmap TrWindow::pageCurl()
{
    QPixmap pixmap;
    pixmap = QPixmap::fromMimeSource( "pagecurl.png" );
    if ( !pixmap.isNull() ) {
	QBitmap pageCurlMask( pagecurl_mask_width, pagecurl_mask_height,
			pagecurl_mask_bits, TRUE );
	pixmap.setMask( pageCurlMask );
    }

    return pixmap;
}

TrWindow::TrWindow()
    : QMainWindow( 0, "translation window", WType_TopLevel | WDestructiveClose )
{

#ifndef Q_WS_MACX
    setIcon( QPixmap::fromMimeSource( "appicon.png" ) );
#endif

    // Create the application global listview symbols
    pxOn  = new QPixmap( QPixmap::fromMimeSource( "s_check_on.png" ) );
    pxOff = new QPixmap( QPixmap::fromMimeSource( "s_check_off.png" ) );
    pxObsolete = new QPixmap( QPixmap::fromMimeSource( "d_s_check_obs.png" ) );
    pxDanger = new QPixmap( QPixmap::fromMimeSource( "s_check_danger.png" ) );

    // Set up the Scope dock window
    QDockWindow * dwScope = new QDockWindow( QDockWindow::InDock, this,
					     "context");
    dwScope->setResizeEnabled( TRUE );
    dwScope->setCloseMode( QDockWindow::Always );
    addDockWindow( dwScope, tr("Context"), Qt::DockLeft );
    dwScope->setCaption( tr("Context") );
    dwScope->setFixedExtentWidth( 200 );
    lv = new QListView( dwScope, "context list view" );
    lv->setShowSortIndicator( TRUE );
    lv->setAllColumnsShowFocus( TRUE );
    lv->header()->setStretchEnabled( TRUE, 1 );
    QFontMetrics fm( font() );
    lv->addColumn( tr("Done"), fm.width( tr("Done") ) + 10 );
    lv->addColumn( tr("Context") );
    lv->addColumn( tr("Items"), 55 );
    lv->setColumnAlignment( 0, Qt::AlignCenter );
    lv->setColumnAlignment( 2, Qt::AlignRight );
    lv->setSorting( 0 );
    lv->setHScrollBarMode( QScrollView::AlwaysOff );
    dwScope->setWidget( lv );

    messageIsShown = FALSE;
    me = new MessageEditor( &tor, this, "message editor" );
    setCentralWidget( me );
    slv = me->sourceTextList();
    plv = me->phraseList();

    setupMenuBar();
    setupToolBars();

    progress = new QLabel( statusBar(), "progress" );
    statusBar()->addWidget( progress, 0, TRUE );
    modified = new QLabel( QString(" %1 ").arg(tr("MOD")), statusBar(),
			   "modified?" );
    statusBar()->addWidget( modified, 0, TRUE );

    dirtyItem = -1;
    numFinished = 0;
    numNonobsolete = 0;
    numMessages = 0;
    updateProgress();

    dirty = FALSE;
    updateCaption();

    phraseBooks.setAutoDelete( TRUE );

    f = new FindDialog( FALSE, this, "find", FALSE );
    f->setCaption( tr("Qt Linguist") );
    h = new FindDialog( TRUE, this, "replace", FALSE );
    h->setCaption( tr("Qt Linguist") );
    findMatchCase = FALSE;
    findWhere = 0;
    foundItem = 0;
    foundScope = 0;
    foundWhere = 0;
    foundOffset = 0;

    connect( lv, SIGNAL(selectionChanged(QListViewItem *)),
	     this, SLOT(showNewScope(QListViewItem *)) );

    connect( slv, SIGNAL(currentChanged(QListViewItem *)),
	     this, SLOT(showNewCurrent(QListViewItem *)) );

    connect( slv, SIGNAL(clicked(QListViewItem *, const QPoint&, int)),
	     this, SLOT(showNewCurrent(QListViewItem *)) );

    connect( slv, SIGNAL(clicked(QListViewItem *, const QPoint&, int)),
	     this, SLOT(toggleFinished(QListViewItem *, const QPoint&, int)) );

    connect( me, SIGNAL(translationChanged(const QString&)),
	     this, SLOT(updateTranslation(const QString&)) );
    connect( me, SIGNAL(finished(bool)), this, SLOT(updateFinished(bool)) );
    connect( me, SIGNAL(prevUnfinished()), this, SLOT(prevUnfinished()) );
    connect( me, SIGNAL(nextUnfinished()), this, SLOT(nextUnfinished()) );
    connect( me, SIGNAL(focusSourceList()), this, SLOT(focusSourceList()) );
    connect( me, SIGNAL(focusPhraseList()), this, SLOT(focusPhraseList()) );
    connect( f, SIGNAL(findNext(const QString&, int, bool)),
	     this, SLOT(findNext(const QString&, int, bool)) );

    QWhatsThis::add( lv, tr("This panel lists the source contexts.") );

    QWhatsThis::add( slv, tr("This panel lists the source texts. "
			    "Items that violate validation rules "
			    "are marked with a warning.") );
    showNewCurrent( 0 );

    QSize as( qApp->desktop()->size() );
    as -= QSize( 30, 30 );
    resize( QSize( 1000, 800 ).boundedTo( as ) );
    readConfig();
    stats = 0;
    srcWords = 0;
    srcChars = 0;
    srcCharsSpc = 0;
}

TrWindow::~TrWindow()
{
    writeConfig();
    delete stats;
}

void TrWindow::openFile( const QString& name )
{
    if ( !name.isEmpty() ) {
	statusBar()->message( tr("Loading...") );
	qApp->processEvents();
	tor.clear();
	if ( tor.load(name) ) {
	    slv->clear();
	    slv->repaint();
	    slv->viewport()->repaint();
	    slv->setUpdatesEnabled( FALSE );
	    slv->viewport()->setUpdatesEnabled( FALSE );
	    lv->clear();
	    lv->repaint();
	    lv->viewport()->repaint();
	    lv->setUpdatesEnabled( FALSE );
	    lv->viewport()->setUpdatesEnabled( FALSE );
	    setEnabled( FALSE );
	    numFinished = 0;
	    numNonobsolete = 0;
	    numMessages = 0;
	    foundScope = 0;

	    TML all = tor.messages();
	    TML::Iterator it;
	    QDict<ContextLVI> contexts( 1009 );

	    srcWords = 0;
	    srcChars = 0;
	    srcCharsSpc = 0;
	    for ( it = all.begin(); it != all.end(); ++it ) {
		qApp->processEvents();
		ContextLVI *c = contexts.find( QString((*it).context()) );
		if ( c == 0 ) {
		    c = new ContextLVI( lv, tor.toUnicode((*it).context(),
							  (*it).utf8()) );
		    contexts.insert( QString((*it).context()), c );
		}
		if ( QCString((*it).sourceText()) == ContextComment ) {
		    c->appendToComment( tor.toUnicode((*it).comment(),
						      (*it).utf8()) );
		} else {
		    MessageLVI * tmp = new MessageLVI( slv, *it,
					   tor.toUnicode((*it).sourceText(),
							 (*it).utf8()),
					   tor.toUnicode((*it).comment(),
							 (*it).utf8()), c );
		    tmp->setDanger( danger(tmp->sourceText(),
					   tmp->translation()) &&
				    tmp->message().type() ==
				    MetaTranslatorMessage::Finished );
		    c->instantiateMessageItem( slv, tmp );
		    if ( (*it).type() != MetaTranslatorMessage::Obsolete ) {
			numNonobsolete++;
			if ( (*it).type() == MetaTranslatorMessage::Finished )
			    numFinished++;
			doCharCounting( tmp->sourceText(), srcWords, srcChars, srcCharsSpc );
		    } else {
			c->incrementObsoleteCount();
		    }
		    numMessages++;
		}
		c->updateStatus();
	    }
	    slv->viewport()->setUpdatesEnabled( TRUE );
	    slv->setUpdatesEnabled( TRUE );
	    lv->viewport()->setUpdatesEnabled( TRUE );
	    lv->setUpdatesEnabled( TRUE );
	    setEnabled( TRUE );
	    slv->repaint();
	    slv->viewport()->repaint();
	    lv->triggerUpdate();
	    updateProgress();
	    filename = name;
	    dirty = FALSE;
	    updateCaption();
	    me->showNothing();
	    doneAndNextAct->setEnabled( FALSE );
	    doneAndNextAlt->setEnabled( FALSE );
	    messageIsShown = FALSE;
	    statusBar()->message(
		    tr("%1 source phrase(s) loaded.").arg(numMessages),
		    MessageMS );

	    foundItem = 0;
	    foundWhere = 0;
	    foundOffset = 0;
	    if ( lv->childCount() > 0 ) {
		findAct->setEnabled( TRUE );
		findAgainAct->setEnabled( FALSE );
#ifdef notyet
		replaceAct->setEnabled( TRUE );
#endif
		lv->setCurrentItem( lv->firstChild() );
	    }
	    addRecentlyOpenedFile( name, recentFiles );
	    updateStatistics();
	} else {
	    statusBar()->clear();
	    QMessageBox::warning( this, tr("Qt Linguist"),
				  tr("Cannot open '%1'.").arg(name) );
	}
    }
}

void TrWindow::open()
{
    if ( maybeSave() ) {
	QString newFilename = QFileDialog::getOpenFileName( filename,
		tr("Qt translation source (*.ts)\n"
		   "All files (*)"),
		this, "open" );
	openFile( newFilename );
    }
}

void TrWindow::save()
{
    if ( filename.isEmpty() )
	return;

    if ( tor.save(filename) ) {
	dirty = FALSE;
	updateCaption();
	statusBar()->message( tr("File saved."), MessageMS );
    } else {
	QMessageBox::warning( this, tr("Qt Linguist"), tr("Cannot save '%1'.")
			      .arg(filename) );
    }
}

void TrWindow::saveAs()
{
    QString newFilename = QFileDialog::getSaveFileName( filename,
	    tr( "Qt translation source (*.ts)\n"
		"All files (*)"),
	    this, "save_as" );
    if ( !newFilename.isEmpty() ) {
	filename = newFilename;
	save();
	updateCaption();
    }
}

void TrWindow::release()
{
    QString newFilename = filename;
    newFilename.replace( QRegExp(".ts$"), "" );
    newFilename += QString( ".qm" );

    newFilename = QFileDialog::getSaveFileName( newFilename,
	    tr("Qt message files for released applications (*.qm)\n"
	       "All files (*)"),
	    this, "release",
	    tr("Release") );
    if ( !newFilename.isEmpty() ) {
	if ( tor.release(newFilename) )
	    statusBar()->message( tr("File created."), MessageMS );
	else
	    QMessageBox::warning( this, tr("Qt Linguist"),
				  tr("Cannot save '%1'.").arg(newFilename) );
    }
}

void TrWindow::print()
{
    int pageNum = 0;

    if ( printer.setup(this) ) {
	QApplication::setOverrideCursor( WaitCursor );
	printer.setDocName( filename );
	statusBar()->message( tr("Printing...") );
	PrintOut pout( &printer );
	ContextLVI *c = (ContextLVI *) lv->firstChild();
	while ( c != 0 ) {
	    setCurrentContextItem( c );
	    pout.vskip();
	    pout.setRule( PrintOut::ThickRule );
	    pout.setGuide( c->context() );
	    pout.addBox( 100, tr("Context: %1").arg(c->context()),
			 PrintOut::Strong );
	    pout.flushLine();
	    pout.addBox( 4 );
	    pout.addBox( 92, c->comment(), PrintOut::Emphasis );
	    pout.flushLine();
	    pout.setRule( PrintOut::ThickRule );

	    MessageLVI *m = (MessageLVI *) slv->firstChild();
	    while ( m != 0 ) {
		pout.setRule( PrintOut::ThinRule );

		QString type;
		switch ( m->message().type() ) {
		case MetaTranslatorMessage::Finished:
		    type = tr( "finished" );
		    break;
		case MetaTranslatorMessage::Unfinished:
		    type = m->danger() ? tr( "unresolved" ) : QString( "unfinished" );
		    break;
		case MetaTranslatorMessage::Obsolete:
		    type = tr( "obsolete" );
		    break;
		default:
		    type = QString( "" );
		}
		pout.addBox( 40, m->sourceText() );
		pout.addBox( 4 );
		pout.addBox( 40, m->translation() );
		pout.addBox( 4 );
		pout.addBox( 12, type, PrintOut::Normal, Qt::AlignRight );
		if ( !m->comment().isEmpty() ) {
		    pout.flushLine();
		    pout.addBox( 4 );
		    pout.addBox( 92, m->comment(), PrintOut::Emphasis );
		}
		pout.flushLine( TRUE );

		if ( pout.pageNum() != pageNum ) {
		    pageNum = pout.pageNum();
		    statusBar()->message( tr("Printing... (page %1)")
					  .arg(pageNum) );
		}
		m = (MessageLVI *) m->nextSibling();
	    }
	    c = (ContextLVI *) c->nextSibling();
	}
	pout.flushLine( TRUE );
	QApplication::restoreOverrideCursor();
	statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
	statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::find()
{
    h->hide();
    f->show();
    f->setActiveWindow();
    f->raise();
}

void TrWindow::findAgain()
{
    int pass = 0;
    int oldItemNo = itemToIndex( slv, slv->currentItem() );
    QString delayedMsg;
    QListViewItem * j = foundScope;
    QListViewItem * k = indexToItem( slv, foundItem );
    QListViewItem * oldScope = lv->currentItem();

    if ( lv->childCount() == 0 )
	return;
#if 1
    /*
      As long as we don't implement highlighting of the text in the QTextView,
      we may have only one match per message.
    */
    foundOffset = (int) 0x7fffffff;
#else
    foundOffset++;
#endif
    slv->setUpdatesEnabled( FALSE );
    do {
	// Iterate through every item in all contexts
	if ( j == 0 ) {
	    j = lv->firstChild();
	    setCurrentContextItem( j );
	    if ( foundScope != 0 )
		delayedMsg = tr("Search wrapped.");
	}
	if ( k == 0 )
	    k = slv->firstChild();

	while ( k ) {
	    MessageLVI * m = (MessageLVI *) k;
	    switch ( foundWhere ) {
		case 0:
		    foundWhere  = FindDialog::SourceText;
		    foundOffset = 0;
		    // fall-through
		case FindDialog::SourceText:
		    if ( searchItem( m->sourceText(), j, k ) ) {
			f->hide();
			if ( !delayedMsg.isEmpty() )
			    statusBar()->message( delayedMsg, MessageMS );
 			return;
		    }
		    foundWhere  = FindDialog::Translations;
		    foundOffset = 0;
		    // fall-through
		case FindDialog::Translations:
		    if ( searchItem( m->translation(), j, k ) ) {
			f->hide();
			if ( !delayedMsg.isEmpty() )
			    statusBar()->message( delayedMsg, MessageMS );
			return;
		    }
		    foundWhere  = FindDialog::Comments;
		    foundOffset = 0;
		    // fall-through
		case FindDialog::Comments:
		    if ( searchItem( ((ContextLVI *) j)->fullContext(), j, k) ) {
			f->hide();
			if ( !delayedMsg.isEmpty() )
			    statusBar()->message( delayedMsg, MessageMS );
			return;
		    }
		    foundWhere  = 0;
		    foundOffset = 0;
	    }
	    k = k->nextSibling();
	}

	j = j->nextSibling();
	if ( j ) {
	    setCurrentContextItem( j );
	    k = slv->firstChild();
	}
    } while ( pass++ != lv->childCount() );

    // This is just to keep the current scope and source text item
    // selected after a search failed.
    if ( oldScope ) {
	setCurrentContextItem( oldScope );
	QListViewItem * tmp = indexToItem( slv, oldItemNo );
	if( tmp )
	    setCurrentMessageItem( tmp );
    } else {
	if( lv->firstChild() )
	    setCurrentContextItem( lv->firstChild() );
	if( slv->firstChild() )
	    setCurrentMessageItem( slv->firstChild() );
    }

    slv->setUpdatesEnabled( TRUE );
    slv->triggerUpdate();
    qApp->beep();
    QMessageBox::warning( this, tr("Qt Linguist"),
			  QString( tr("Cannot find the string '%1'.") ).arg(findText) );
    foundItem   = 0;
    foundWhere  = 0;
    foundOffset = 0;
}

void TrWindow::replace()
{
    f->hide();
    h->show();
    h->setActiveWindow();
    h->raise();
}

int TrWindow::itemToIndex( QListView * view, QListViewItem * item )
{
    int no = 0;
    QListViewItem * tmp;

    if( view && item ){
	if( (tmp = view->firstChild()) != 0 )
	    do {
		no++;
		tmp = tmp->nextSibling();
	    } while( tmp && (tmp != item) );
    }
    return no;
}

QListViewItem * TrWindow::indexToItem( QListView * view, int index )
{
    QListViewItem * item = 0;

    if ( view && index > 0 ) {
	item = view->firstChild();
	while( item && index-- > 0 )
	    item = item->nextSibling();
    }
    return item;
}

bool TrWindow::searchItem( const QString & searchWhat, QListViewItem * j,
			   QListViewItem * k )
{
    if ( (findWhere & foundWhere) != 0 ) {
	foundOffset = searchWhat.find( findText, foundOffset, findMatchCase );
	if ( foundOffset >= 0 ) {
	    foundItem = itemToIndex( slv, k );
	    foundScope = j;
	    setCurrentMessageItem( k );
	    slv->setUpdatesEnabled( TRUE );
	    slv->triggerUpdate();
	    return TRUE;
	}
    }
    foundOffset = 0;
    return FALSE;
}

void TrWindow::newPhraseBook()
{
    QString name;
    for (;;) {
	name = QFileDialog::getSaveFileName( QString::null,
	    tr("Qt phrase books (*.qph)\n"
	       "All files (*)"),
	    this, "new_phrasebook",
	    tr("Create New Phrase Book") );
	if ( !QFile::exists(name) )
	    break;
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("A file called '%1' already exists."
				 "  Please choose another name.").arg(name) );
    }
    if ( !name.isEmpty() ) {
	PhraseBook pb;
	if ( savePhraseBook(name, pb) ) {
	    if ( openPhraseBook(name) )
		statusBar()->message( tr("Phrase book created."), MessageMS );
	}
    }
}

void TrWindow::openPhraseBook()
{
    QString phrasebooks( qInstallPathData() );
    QString name = QFileDialog::getOpenFileName( phrasebooks + "/phrasebooks",
	tr("Qt phrase books (*.qph)\n"
	   "All files (*)"),
	this, "open_phrasebook",
	tr("Open Phrase Book") );
    if ( !name.isEmpty() && !phraseBookNames.contains(name) ) {
	if ( openPhraseBook(name) ) {
	    int n = (int)(phraseBooks.at( phraseBooks.count() - 1 )->count());
	    statusBar()->message( tr("%1 phrase(s) loaded.").arg(n),
				  MessageMS );
	}
    }
}

void TrWindow::closePhraseBook( int id )
{
    int index = closePhraseBookp->indexOf( id );
    phraseBooks.remove( index );
    phraseBookNames.remove( phraseBookNames.at(index) );
    updatePhraseDict();

    dirtyItem = index; // remove the item next time the menu is opened
    editPhraseBookp->removeItem( editPhraseBookp->idAt(index) );
    printPhraseBookp->removeItem( printPhraseBookp->idAt(index) );
}

void TrWindow::editPhraseBook( int id )
{
    int index = editPhraseBookp->indexOf( id );
    PhraseBookBox box( phraseBookNames[index], *phraseBooks.at(index), this,
		       "phrase book box", TRUE );
    box.setCaption( tr("%1 - %2").arg(tr("Qt Linguist"))
				 .arg(friendlyPhraseBookName(index)) );
    box.resize( 500, 300 );
    box.exec();
    *phraseBooks.at( index ) = box.phraseBook();
    updatePhraseDict();
}

void TrWindow::printPhraseBook( int id )
{
    int index = printPhraseBookp->indexOf( id );
    int pageNum = 0;

    if ( printer.setup(this) ) {
	printer.setDocName( phraseBookNames[index] );
	statusBar()->message( tr("Printing...") );
	PrintOut pout( &printer );
	PhraseBook *phraseBook = phraseBooks.at( index );
	PhraseBook::Iterator p;
	pout.setRule( PrintOut::ThinRule );
	for ( p = phraseBook->begin(); p != phraseBook->end(); ++p ) {
	    pout.setGuide( (*p).source() );
	    pout.addBox( 29, (*p).source() );
	    pout.addBox( 4 );
	    pout.addBox( 29, (*p).target() );
	    pout.addBox( 4 );
	    pout.addBox( 34, (*p).definition(), PrintOut::Emphasis );

	    if ( pout.pageNum() != pageNum ) {
		pageNum = pout.pageNum();
		statusBar()->message( tr("Printing... (page %1)")
				      .arg(pageNum) );
	    }
	    pout.setRule( PrintOut::NoRule );
	    pout.flushLine( TRUE );
	}
	pout.flushLine( TRUE );
	statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
	statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::revertSorting()
{
    lv->setSorting( 0 );
    slv->setSorting( 0 );
}

void TrWindow::manual()
{
    QAssistantClient *ac = new QAssistantClient( qInstallPathBins(), this );
    ac->showPage( QString( qInstallPathDocs() ) + "/html/linguist-manual.html" );
}

void TrWindow::about()
{
    AboutDialog about( this, 0, TRUE );
    about.versionLabel->setText( tr("Version %1").arg(QT_VERSION_STR) );
    about.exec();
}

void TrWindow::aboutQt()
{
    QMessageBox::aboutQt( this, tr("Qt Linguist") );
}

void TrWindow::setupPhrase()
{
    bool enabled = !phraseBooks.isEmpty();
    phrasep->setItemEnabled( closePhraseBookId, enabled );
    phrasep->setItemEnabled( editPhraseBookId, enabled );
    phrasep->setItemEnabled( printPhraseBookId, enabled );
}

void TrWindow::closeEvent( QCloseEvent *e )
{
    if ( maybeSave() )
	e->accept();
    else
	e->ignore();
}

bool TrWindow::maybeSave()
{
    if ( dirty ) {
	switch ( QMessageBox::information(this, tr("Qt Linguist"),
				  tr("Do you want to save '%1'?")
				  .arg(filename),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No,
				  QMessageBox::Cancel | QMessageBox::Escape ) )
	{
	    case QMessageBox::Cancel:
		return FALSE;
	    case QMessageBox::Yes:
		save();
                return !dirty;
	    case QMessageBox::No:
		break;
	}
    }
    return TRUE;
}

void TrWindow::updateCaption()
{
    QString cap;
    bool enable = !filename.isEmpty();
    saveAct->setEnabled( enable );
    saveAsAct->setEnabled( enable );
    releaseAct->setEnabled( enable );
    printAct->setEnabled( enable );
    acceleratorsAct->setEnabled( enable );
    endingPunctuationAct->setEnabled( enable );
    phraseMatchesAct->setEnabled( enable );
    revertSortingAct->setEnabled( enable );

    if ( filename.isEmpty() )
	cap = tr( "Qt Linguist by Trolltech" );
    else
	cap = tr( "%1 - %2" ).arg( tr("Qt Linguist by Trolltech") )
			     .arg( filename );
    setCaption( cap );
    modified->setEnabled( dirty );
}

//
// New scope selected - build a new list of source text items
// for that scope.
//
void TrWindow::showNewScope( QListViewItem *item )
{
    static ContextLVI * oldContext = 0;

    if( item != 0 ) {
	ContextLVI * c = (ContextLVI *) item;
	bool upe = slv->isUpdatesEnabled();
	slv->setUpdatesEnabled( FALSE );
	slv->viewport()->setUpdatesEnabled( FALSE );
	if ( oldContext != 0 ) {
	    MessageLVI * tmp;
	    slv->blockSignals( TRUE );
	    while ( (tmp = (MessageLVI *) slv->firstChild()) != 0 )
		oldContext->appendMessageItem( slv, tmp );
	    slv->blockSignals( FALSE );
	}
	MessageLVI * tmp;
	while ( c->messageItemsInList() ) {
	    tmp = c->takeMessageItem( c->messageItemsInList() - 1);
	    slv->insertItem( tmp );
	    tmp->updateTranslationText();
	}

	// Make sure that we update the source text and translation text. Use the first message in the new scope.
	setCurrentMessageItem(slv->firstChild());
	// This must be explicitly called since the signal is not emitted if the context only contains one item
	showNewCurrent(slv->firstChild());
	slv->viewport()->setUpdatesEnabled( upe );
	slv->setUpdatesEnabled( upe );
	if( upe )
	    slv->triggerUpdate();
	oldContext = (ContextLVI *) item;
	statusBar()->clear();
    }
}

void TrWindow::showNewCurrent( QListViewItem *item )
{
    messageIsShown = (item != 0);
    MessageLVI *m = (MessageLVI *) item;
    ContextLVI *c = (ContextLVI *) m ? m->contextLVI() : 0;

    if ( messageIsShown ) {
	me->showMessage( m->sourceText(), m->comment(), c->fullContext(),
			 m->translation(), m->message().type(),
			 getPhrases(m->sourceText()) );
	if ( (m->message().type() != MetaTranslatorMessage::Finished) &&
	     m->danger() )
	    danger( m->sourceText(), m->translation(), TRUE );
	else
	    statusBar()->clear();

	doneAndNextAct->setEnabled( m->message().type() !=
				    MetaTranslatorMessage::Obsolete );
    } else {
	if ( item == 0 )
	    me->showNothing();
	else
	    me->showContext( c->fullContext(), c->finished() );
	doneAndNextAct->setEnabled( FALSE );
    }
    doneAndNextAlt->setEnabled( doneAndNextAct->isEnabled() );

    selectAllAct->setEnabled( messageIsShown );
}

void TrWindow::updateTranslation( const QString& translation )
{
    QListViewItem *item = slv->currentItem();
    if ( item != 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	if ( translation != m->translation() ) {
	    bool dngr;
	    m->setTranslation( translation );
	    if ( m->finished() &&
		 (dngr = danger( m->sourceText(), m->translation(), TRUE )) ) {
		numFinished -= 1;
		m->setDanger( dngr );
		m->setFinished( FALSE );
		m->contextLVI()->updateStatus();
		updateProgress();
	    }
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	    m->updateTranslationText();
	}
    }
}

void TrWindow::updateFinished( bool finished )
{
    QListViewItem *item = slv->currentItem();
    if ( item != 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	if ( finished != m->finished() ) {
	    numFinished += finished ? +1 : -1;
	    updateProgress();
	    m->setFinished( finished );
	    bool oldDanger = m->danger();
	    m->setDanger( /*m->finished() &&*/
			  danger(m->sourceText(), m->translation(),
			  !oldDanger) );
	    if ( !oldDanger && m->danger() )
		qApp->beep();
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
    }
}

void TrWindow::doneAndNext()
{
    MessageLVI * m = (MessageLVI *) slv->currentItem();
    bool dngr = FALSE;

    if ( !m ) return;
    dngr = danger( m->sourceText(), m->translation(), TRUE );
    if ( !dngr ) {
	me->finishAndNext();
	m->contextLVI()->updateStatus();
    } else {
	if ( m->danger() != dngr )
	    m->setDanger( dngr );
	tor.insert( m->message() );
	if ( !dirty ) {
	    dirty = TRUE;
	    updateCaption();
	}
	qApp->beep();
    }
    updateStatistics();
}

void TrWindow::toggleFinished( QListViewItem *item, const QPoint& /* p */,
			       int column )
{
    if ( item != 0 && column == 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	bool dngr = FALSE;

	if ( m->message().type() == MetaTranslatorMessage::Unfinished ) {
	    dngr = danger( m->sourceText(), m->translation(), TRUE );
	}
	if ( !dngr && m->message().type() != MetaTranslatorMessage::Obsolete) {
	    setCurrentMessageItem( m );
	    me->setFinished( !m->finished() );
	    m->contextLVI()->updateStatus();
	} else {
	    bool oldDanger = m->danger();
	    m->setDanger( danger(m->sourceText(), m->translation(),
				 !oldDanger) );
	    if ( !oldDanger && m->danger() )
		qApp->beep();
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
	updateStatistics();
    }
}

void TrWindow::nextUnfinished()
{
    if ( nextUnfinishedAct->isEnabled() ) {
	// Select a message to translate, grab the first available if
	// there are no current selection.
	QListViewItem * cItem = lv->currentItem(); // context item
	QListViewItem * mItem = slv->currentItem(); // message item

	// Make sure an item is selected from both the context and the
	// message list.
	if( (mItem == 0) && !(mItem = slv->firstChild()) ) {
	    if( (cItem == 0) && !(cItem = lv->firstChild()) ) {
		statusBar()->message( tr("No phrase to translate."),
				      MessageMS );
		qApp->beep();
		return;
	    } else {
		showNewScope( cItem );
		while( cItem && !(mItem = slv->firstChild()) ) {
		    // no children in this node - try next one
		    cItem = cItem->nextSibling();
		    showNewScope( cItem );
		}
		setCurrentContextItem( cItem );
		if( mItem ) {
		    setCurrentMessageItem( mItem );
		} else {
		    statusBar()->message( tr("No phrase to translate."),
					  MessageMS );
		    qApp->beep();
		    return;
		}
	    }
	} else {
	    setCurrentMessageItem( mItem );
	}

	MessageLVI * m = (MessageLVI *) mItem;
	MessageLVI * n;
	ContextLVI * p = (ContextLVI *) cItem;
	ContextLVI * q;

	// Find the next Unfinished sibling within the same context.
	m = (MessageLVI *) mItem->nextSibling();
	n = m;
	do {
	    if ( n == 0 )
		break;
	    if ( n && !n->finished() && n != mItem ) {
		setCurrentMessageItem( n );
		return;
	    }
	    n = (MessageLVI *) n->nextSibling();
	} while ( n != m );

	// If all siblings are Finished or Obsolete, look in the first
	// Unfinished context.
	p = (ContextLVI *) p->nextSibling();
	q = p;
	do {
	    if ( q == 0 )
		q = (ContextLVI *) lv->firstChild();
	    if ( q && !q->finished() ) {
		showNewScope( q );
		setCurrentContextItem( q );
		n = (MessageLVI *) slv->firstChild();
		while ( n && n->finished() )
		    n = (MessageLVI *) n->nextSibling();
		if ( n && q ) {
		    setCurrentMessageItem( n );
		    showNewCurrent( n );
		    return;
		}
	    }
	    q = (ContextLVI *) q->nextSibling();
	} while ( q != p );
    }

    // If no Unfinished message is left, the user has finished the job.  We
    // congratulate on a job well done with this ringing bell.
    statusBar()->message( tr("No untranslated phrases left."), MessageMS );
    qApp->beep();
}

static QListViewItem * lastChild( QListView * view )
{
    if ( view ) {
	QListViewItem * ret, * tmp;
	ret = view->firstChild();
	while ( ret ) {
	    tmp = ret->nextSibling();
	    if ( tmp == 0 )
		return ret;
	    ret = tmp;
	}
    }
    return 0;
}

void TrWindow::prevUnfinished()
{
    if ( prevUnfinishedAct->isEnabled() ) {
	// Select a message to translate, grab the first available if
	// there are no current selection.
	QListViewItem * cItem = lv->currentItem();  // context item
	QListViewItem * mItem = slv->currentItem(); // message item

	// Make sure an item is selected from both the context and the
	// message list.
	if( (mItem == 0) && !(mItem = slv->firstChild()) ) {
	    if( (cItem == 0) && !(cItem = lv->firstChild()) ) {
		statusBar()->message( tr("No phrase to translate."),
				      MessageMS );
		qApp->beep();
		return;
	    } else {
		showNewScope( cItem );
		while( cItem && !(mItem = slv->firstChild()) ) {
		    // no children in this node - try next one
		    cItem = cItem->nextSibling();
		    showNewScope( cItem );
		}
		setCurrentContextItem( cItem );
		if( mItem ) {
		    setCurrentMessageItem( cItem );
		} else {
		    statusBar()->message( tr("No phrase to translate."),
					  MessageMS );
		    qApp->beep();
		    return;
		}
	    }
	} else {
	    setCurrentMessageItem( mItem );
	}

	MessageLVI * m = (MessageLVI *) mItem;
	MessageLVI * n;
	ContextLVI * p = (ContextLVI *) cItem;
	ContextLVI * q;

	// Find the next Unfinished sibling within the same context.
	n = m;
	do {
	    n = (MessageLVI * ) n->itemAbove();
	    if ( n == 0 )
		break;
	    if ( n && !n->finished() ) {
		setCurrentMessageItem( n );
		return;
	    }
	} while ( !((ContextLVI *) cItem)->finished() && n != 0 );

	// If all siblings are Finished or Obsolete, look in the prev
	// Unfinished context.
	q = p;
	do {
	    q = (ContextLVI *) q->itemAbove();
	    if ( q == 0 )
		q = (ContextLVI *) lastChild( lv );
	    if ( q && !q->finished() ) {
		showNewScope( q );
		setCurrentContextItem( q );
		n = (MessageLVI *) lastChild( slv );
		while ( n && n->finished() )
		    n = (MessageLVI *) n->itemAbove();
		if ( n && q ) {
		    setCurrentMessageItem( n );
		    return;
		}
	    }
	} while ( q != 0 );
    }
    statusBar()->message( tr("No untranslated phrases left."), MessageMS );
    qApp->beep();
}

void TrWindow::prev()
{
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item
    QListViewItem * tmp;

    if ( !cItem ) {
	cItem = lv->firstChild();
	if ( !cItem ) return;
	setCurrentContextItem( cItem );
    }

    if ( !mItem ) {
	mItem = lastChild( slv );
	if ( !mItem ) return;
	setCurrentMessageItem( mItem );
    } else {
	if ( (tmp = mItem->itemAbove()) != 0 ) {
	    setCurrentMessageItem( tmp );
	    return;
	} else {
	    if ( (tmp = cItem->itemAbove()) == 0 ) {
		tmp = lastChild( lv );
	    }
	    if ( !tmp ) return;
	    setCurrentContextItem( tmp );
	    setCurrentMessageItem( lastChild( slv ) );
	}
    }
}

void TrWindow::next()
{
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item
    QListViewItem * tmp;

    if ( !cItem ) {
	cItem = lv->firstChild();
	if ( !cItem ) return;
	setCurrentContextItem( cItem );
    }

    if ( !mItem ) {
	mItem = slv->firstChild();
	if ( !mItem ) return;
	setCurrentMessageItem( mItem );
    } else {
	if ( (tmp = mItem->nextSibling()) != 0 ) {
	    setCurrentMessageItem( tmp );
	    return;
	} else {
	    if ( (tmp = cItem->nextSibling()) == 0 ) {
		tmp = lv->firstChild();
	    }
	    if ( !tmp ) return;
	    setCurrentContextItem( tmp );
	    setCurrentMessageItem( slv->firstChild() );
	}
    }
}


void TrWindow::findNext( const QString& text, int where, bool matchCase )
{
    findText = text;
    if ( findText.isEmpty() )
	findText = QString( "magicwordthatyoushouldavoid" );
    findWhere = where;
    findMatchCase = matchCase;
    findAgainAct->setEnabled( TRUE );
    findAgain();
}

void TrWindow::revalidate()
{
    ContextLVI *c = (ContextLVI *) lv->firstChild();
    QListViewItem * oldScope = lv->currentItem();
    int oldItemNo = itemToIndex( slv, slv->currentItem() );
    slv->setUpdatesEnabled( FALSE );

    while ( c != 0 ) {
	showNewScope( c );
	MessageLVI *m = (MessageLVI *) slv->firstChild();
	while ( m != 0 ) {
	    m->setDanger( danger(m->sourceText(), m->translation()) &&
		    m->message().type() == MetaTranslatorMessage::Finished );
	    m = (MessageLVI *) m->nextSibling();
	}
	c = (ContextLVI *) c->nextSibling();
    }

    if ( oldScope ){
	showNewScope( oldScope );
	QListViewItem * tmp = indexToItem( slv, oldItemNo );
	if( tmp )
	    setCurrentMessageItem( tmp );
    }
    slv->setUpdatesEnabled( TRUE );
    slv->triggerUpdate();
}

QString TrWindow::friendlyString( const QString& str )
{
    QString f = str.lower();
    f.replace( QRegExp(QString("[.,:;!?()-]")), QString(" ") );
    f.replace( "&", QString("") );
    f = f.simplifyWhiteSpace();
    f = f.lower();
    return f;
}

void TrWindow::setupMenuBar()
{
    QMenuBar * m = menuBar();
    QPopupMenu * filep = new QPopupMenu( this );
    QPopupMenu * editp  = new QPopupMenu( this );
    QPopupMenu * translationp = new QPopupMenu( this );
    QPopupMenu * validationp = new QPopupMenu( this );
    validationp->setCheckable( TRUE );
    phrasep = new QPopupMenu( this );
    closePhraseBookp = new QPopupMenu( this );
    editPhraseBookp = new QPopupMenu( this );
    printPhraseBookp = new QPopupMenu( this );
    QPopupMenu * viewp = new QPopupMenu( this );
    viewp->setCheckable( TRUE );
    QPopupMenu * helpp = new QPopupMenu( this );

    m->insertItem( tr("&File"), filep );
    m->insertItem( tr("&Edit"), editp );
    m->insertItem( tr("&Translation"), translationp );
    m->insertItem( tr("V&alidation"), validationp );
    m->insertItem( tr("&Phrases"), phrasep );
    m->insertItem( tr("&View"), viewp );
    m->insertSeparator();
    m->insertItem( tr("&Help"), helpp );

    connect( closePhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(closePhraseBook(int)) );
    connect( closePhraseBookp, SIGNAL(aboutToShow()),
	     this, SLOT(updateClosePhraseBook()) );
    connect( editPhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(editPhraseBook(int)) );
    connect( printPhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(printPhraseBook(int)) );
    // File menu
    openAct = new Action( filep, tr("&Open..."), this, SLOT(open()),
			  "fileopen.png", QAccel::stringToKey(tr("Ctrl+O")) );

    filep->insertSeparator();

    saveAct = new Action( filep, tr("&Save"), this, SLOT(save()),
			  "filesave.png", QAccel::stringToKey(tr("Ctrl+S")) );
    saveAsAct = new Action( filep, tr("Save &As..."), this, SLOT(saveAs()) );
    releaseAct = new Action( filep, tr("&Release..."), this, SLOT(release()) );
    filep->insertSeparator();
    printAct = new Action( filep, tr("&Print..."), this, SLOT(print()),
			   "print.png", QAccel::stringToKey(tr("Ctrl+P")) );

    filep->insertSeparator();

    recentFilesMenu = new QPopupMenu( this );
    filep->insertItem( tr("Re&cently opened files"), recentFilesMenu );
    connect( recentFilesMenu, SIGNAL(aboutToShow()), this,
	     SLOT(setupRecentFilesMenu()) );
    connect( recentFilesMenu, SIGNAL(activated( int )), this,
	     SLOT(recentFileActivated( int )) );

    filep->insertSeparator();

    exitAct = new Action( filep, tr("E&xit"), this, SLOT(close()),
			  QString::null, QAccel::stringToKey(tr("Ctrl+Q")) );
    // Edit menu
    undoAct = new Action( editp, tr("&Undo"), me, SLOT(undo()),
			  "undo.png", QAccel::stringToKey(tr("Ctrl+Z")) );
    undoAct->setEnabled( FALSE );
    connect( me, SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)) );
    redoAct = new Action( editp, tr("&Redo"), me, SLOT(redo()),
			  "redo.png", QAccel::stringToKey(tr("Ctrl+Y")) );
    redoAct->setEnabled( FALSE );
    connect( me, SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)) );
    editp->insertSeparator();
    cutAct = new Action( editp, tr("Cu&t"), me, SLOT(cut()),
			 "editcut.png", QAccel::stringToKey(tr("Ctrl+X")) );
    cutAct->setEnabled( FALSE );
    connect( me, SIGNAL(cutAvailable(bool)), cutAct, SLOT(setEnabled(bool)) );
    copyAct = new Action( editp, tr("&Copy"), me, SLOT(copy()),
			  "editcopy.png", QAccel::stringToKey(tr("Ctrl+C")) );
    copyAct->setEnabled( FALSE );
    connect( me, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)) );
    pasteAct = new Action( editp, tr("&Paste"), me, SLOT(paste()),
			   "editpaste.png", QAccel::stringToKey(tr("Ctrl+V")) );
    pasteAct->setEnabled( FALSE );
    connect( me, SIGNAL(pasteAvailable(bool)),
	     pasteAct, SLOT(setEnabled(bool)) );
    selectAllAct = new Action( editp, tr("Select &All"), me, SLOT(selectAll()),
			       QString::null, QAccel::stringToKey(tr("Ctrl+A")) );
    selectAllAct->setEnabled( FALSE );
    editp->insertSeparator();
    findAct = new Action( editp, tr("&Find..."), this, SLOT(find()),
			  "searchfind.png", QAccel::stringToKey(tr("Ctrl+F")) );
    findAct->setEnabled( FALSE );
    findAgainAct = new Action( editp, tr("Find &Next"),
			       this, SLOT(findAgain()), QString::null, Key_F3 );
    findAgainAct->setEnabled( FALSE );
#ifdef notyet
    replaceAct = new Action( editp, tr("&Replace..."), this, SLOT(replace()),
			     QAccel::stringToKey(tr("Ctrl+H")) );
    replaceAct->setEnabled( FALSE );
#endif

    // Translation menu
    // when updating the accelerators, remember the status bar
    prevUnfinishedAct = new Action( translationp, tr("&Prev Unfinished"),
				    this, SLOT(prevUnfinished()),
				    "prevunfinished.png", QAccel::stringToKey(tr("Ctrl+K")) );
    nextUnfinishedAct = new Action( translationp, tr("&Next Unfinished"),
				    this, SLOT(nextUnfinished()),
				    "nextunfinished.png", QAccel::stringToKey(tr("Ctrl+L")) );

    prevAct = new Action( translationp, tr("P&rev"),
			  this, SLOT(prev()), "prev.png",
			  QAccel::stringToKey(tr("Ctrl+Shift+K")) );
    nextAct = new Action( translationp, tr("Ne&xt"),
			  this, SLOT(next()), "next.png",
			  QAccel::stringToKey(tr("Ctrl+Shift+L")) );
    doneAndNextAct = new Action( translationp, tr("Done and &Next"),
				 this, SLOT(doneAndNext()), "doneandnext.png",
				 QAccel::stringToKey(tr("Ctrl+Enter")) );
    doneAndNextAlt = new QAction( this );
    doneAndNextAlt->setAccel( QAccel::stringToKey(tr("Ctrl+Return")) );
    connect( doneAndNextAlt, SIGNAL(activated()), this, SLOT(doneAndNext()) );
    beginFromSourceAct = new Action( translationp, tr("&Begin from Source"),
				     me, SLOT(beginFromSource()),
				     QString::null, QAccel::stringToKey(tr("Ctrl+B")) );
    connect( me, SIGNAL(updateActions(bool)), beginFromSourceAct,
	     SLOT(setEnabled(bool)) );

    // Phrasebook menu
    newPhraseBookAct = new Action( phrasep, tr("&New Phrase Book..."),
				   this, SLOT(newPhraseBook()),
				   QString::null, QAccel::stringToKey(tr("Ctrl+N")) );
    openPhraseBookAct = new Action( phrasep, tr("&Open Phrase Book..."),
				    this, SLOT(openPhraseBook()),
				    "book.png", QAccel::stringToKey(tr("Ctrl+H")) );
    closePhraseBookId = phrasep->insertItem( tr("&Close Phrase Book"),
					     closePhraseBookp );
    phrasep->insertSeparator();
    editPhraseBookId = phrasep->insertItem( tr("&Edit Phrase Book..."),
					    editPhraseBookp );
    printPhraseBookId = phrasep->insertItem( tr("&Print Phrase Book..."),
					     printPhraseBookp );
    connect( phrasep, SIGNAL(aboutToShow()), this, SLOT(setupPhrase()) );

    // Validation menu
    acceleratorsAct = new Action( validationp, tr("&Accelerators"),
				  this, SLOT(revalidate()), "accelerator.png", 0, TRUE );
    acceleratorsAct->setOn( TRUE );
    endingPunctuationAct = new Action( validationp, tr("&Ending Punctuation"),
				       this, SLOT(revalidate()), "punctuation.png", 0, TRUE );
    endingPunctuationAct->setOn( TRUE );
    phraseMatchesAct = new Action( validationp, tr("&Phrase Matches"),
				   this, SLOT(revalidate()), "phrase.png", 0, TRUE );
    phraseMatchesAct->setOn( TRUE );

    // View menu
    revertSortingAct = new Action( viewp, tr("&Revert Sorting"),
				   this, SLOT(revertSorting()) );
    doGuessesAct = new Action( viewp, tr("&Display guesses"),
			       this, SLOT(toggleGuessing()) );
    doGuessesAct->setToggleAction( TRUE );
    doGuessesAct->setOn( TRUE );
    toggleStats = new Action( viewp, tr("&Statistics"), this, SLOT(toggleStatistics()) );
    toggleStats->setToggleAction( TRUE );
    viewp->insertSeparator();
    viewp->insertItem( tr("Vie&ws"), createDockWindowMenu( NoToolBars ) );
    viewp->insertItem( tr("&Toolbars"), createDockWindowMenu( OnlyToolBars ) );

    // Help
    manualAct = new Action( helpp, tr("&Manual"), this, SLOT(manual()), 0,
			    Key_F1 );
    helpp->insertSeparator();
    aboutAct = new Action( helpp, tr("&About"), this, SLOT(about()) );
    aboutQtAct = new Action( helpp, tr("About &Qt"), this, SLOT(aboutQt()) );
    helpp->insertSeparator();
    whatsThisAct = new Action( helpp, tr("&What's This?"),
			       this, SLOT(whatsThis()), "whatsthis.xpm", SHIFT + Key_F1 );

    openAct->setWhatsThis( tr("Open a Qt translation source file (TS file) for"
			      " editing.") );
    saveAct->setWhatsThis( tr("Save changes made to this Qt translation "
				"source file.") );
    saveAsAct->setWhatsThis( tr("Save changes made to this Qt translation"
				"source file into a new file.") );
    releaseAct->setWhatsThis( tr("Create a Qt message file suitable for"
				 " released applications"
				 " from the current message file.") );
    printAct->setWhatsThis( tr("Print a list of all the phrases in the current"
			       " Qt translation source file.") );
    exitAct->setWhatsThis( tr("Close this window and exit.") );

    undoAct->setWhatsThis( tr("Undo the last editing operation performed on the"
			      " translation.") );
    redoAct->setWhatsThis( tr("Redo an undone editing operation performed on"
			      " the translation.") );
    cutAct->setWhatsThis( tr("Copy the selected translation text to the"
			     " clipboard and deletes it.") );
    copyAct->setWhatsThis( tr("Copy the selected translation text to the"
			      " clipboard.") );
    pasteAct->setWhatsThis( tr("Paste the clipboard text into the"
			       " translation.") );
    selectAllAct->setWhatsThis( tr("Select the whole translation text.") );
    findAct->setWhatsThis( tr("Search for some text in the translation "
				"source file.") );
    findAgainAct->setWhatsThis( tr("Continue the search where it was left.") );
#ifdef notyet
    replaceAct->setWhatsThis( tr("Search for some text in the translation"
				 " source file and replace it by another"
				 " text.") );
#endif

    newPhraseBookAct->setWhatsThis( tr("Create a new phrase book.") );
    openPhraseBookAct->setWhatsThis( tr("Open a phrase book to assist"
					" translation.") );
    acceleratorsAct->setWhatsThis( tr("Toggle validity checks of"
				      " accelerators.") );
    endingPunctuationAct->setWhatsThis( tr("Toggle validity checks"
					   " of ending punctuation.") );
    phraseMatchesAct->setWhatsThis( tr("Toggle checking that phrase"
				       " suggestions are used.") );

    revertSortingAct->setWhatsThis( tr("Sort the items back in the same order"
				       " as in the message file.") );

    doGuessesAct->setWhatsThis( tr("Set whether or not to display translation guesses.") );
    manualAct->setWhatsThis( tr("Display the manual for %1.")
			       .arg(tr("Qt Linguist")) );
    aboutAct->setWhatsThis( tr("Display information about %1.")
			    .arg(tr("Qt Linguist")) );
    aboutQtAct->setWhatsThis( tr("Display information about the Qt toolkit by"
				 " Trolltech.") );
    whatsThisAct->setWhatsThis( tr("Enter What's This? mode.") );

    beginFromSourceAct->setWhatsThis( tr("Copies the source text into"
					 " the translation field.") );
    nextAct->setWhatsThis( tr("Moves to the next item.") );
    prevAct->setWhatsThis( tr("Moves to the previous item.") );
    nextUnfinishedAct->setWhatsThis( tr("Moves to the next unfinished item.") );
    prevUnfinishedAct->setWhatsThis( tr("Moves to the previous unfinished item.") );
    doneAndNextAct->setWhatsThis( tr("Marks this item as done and moves to the"
				     " next unfinished item.") );
    doneAndNextAlt->setWhatsThis( doneAndNextAct->whatsThis() );
}

void TrWindow::setupToolBars()
{
    QToolBar *filet = new QToolBar( tr("File"), this );
    QToolBar *editt = new QToolBar( tr("Edit"), this );
    QToolBar *translationst = new QToolBar( tr("Translation"), this );
    QToolBar *validationt   = new QToolBar( tr("Validation"), this );
    QToolBar *helpt = new QToolBar( tr("Help"), this );

    openAct->addToToolbar( filet, tr("Open") );
    saveAct->addToToolbar( filet, tr("Save") );
    printAct->addToToolbar( filet, tr("Print") );
    filet->addSeparator();
    openPhraseBookAct->addToToolbar( filet, tr("Open Phrase Book") );

    undoAct->addToToolbar( editt, tr("Undo") );
    redoAct->addToToolbar( editt, tr("Redo") );
    editt->addSeparator();
    cutAct->addToToolbar( editt, tr("Cut") );
    copyAct->addToToolbar( editt, tr("Copy") );
    pasteAct->addToToolbar( editt, tr("Paste") );
    editt->addSeparator();
    findAct->addToToolbar( editt, tr("Find") );
#ifdef notyet
    replaceAct->addToToolbar( editt, tr("Replace") );
#endif

    // beginFromSourceAct->addToToolbar( translationst,
    //                                tr("Begin from Source"), "searchfind" );
    prevAct->addToToolbar( translationst, tr("Prev") );
    nextAct->addToToolbar( translationst, tr("Next") );
    prevUnfinishedAct->addToToolbar( translationst, tr("Prev Unfinished") );
    nextUnfinishedAct->addToToolbar( translationst, tr("Next Unfinished") );
    doneAndNextAct->addToToolbar( translationst, tr("Done and Next") );

    acceleratorsAct->addToToolbar( validationt, tr("Accelerators") );
    endingPunctuationAct->addToToolbar( validationt, tr("Punctuation") );
    phraseMatchesAct->addToToolbar( validationt, tr("Phrases") );

    whatsThisAct->addToToolbar( helpt, tr("What's This?") );
}

void TrWindow::setCurrentContextItem( QListViewItem *item )
{
    lv->ensureItemVisible( item );
    lv->setSelected( item, TRUE );
}

void TrWindow::setCurrentMessageItem( QListViewItem *item )
{
    slv->ensureItemVisible( item );
    slv->setSelected( item, TRUE );
}

QString TrWindow::friendlyPhraseBookName( int k )
{
    return QFileInfo( phraseBookNames[k] ).fileName();
}

bool TrWindow::openPhraseBook( const QString& name )
{
    PhraseBook *pb = new PhraseBook;
    if ( !pb->load(name) ) {
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("Cannot read from phrase book '%1'.")
			      .arg(name) );
	return FALSE;
    }

    int index = (int) phraseBooks.count();
    phraseBooks.append( pb );
    phraseBookNames.append( name );
    int id = closePhraseBookp->insertItem( friendlyPhraseBookName(index) );
    closePhraseBookp->setWhatsThis( id, tr("Close this phrase book.") );
    id = editPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    editPhraseBookp->setWhatsThis( id, tr("Allow you to add, modify, or delete"
					  " phrases of this phrase book.") );
    id = printPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    printPhraseBookp->setWhatsThis( id, tr("Print the entries of the phrase"
					   " book.") );
    updatePhraseDict();
    return TRUE;
}

bool TrWindow::savePhraseBook( QString& name, const PhraseBook& pb )
{
    if ( !name.contains( ".qph" ) && !name.contains(".") )
	name += ".qph";

    if ( !pb.save(name) ) {
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("Cannot create phrase book '%1'.")
			      .arg(name) );
	return FALSE;
    }
    return TRUE;
}

void TrWindow::updateProgress()
{
    if ( numNonobsolete == 0 )
	progress->setText( QString("    " "    ") );
    else
	progress->setText( QString(" %1/%2 ").arg(numFinished)
			   .arg(numNonobsolete) );
    prevUnfinishedAct->setEnabled( numFinished != numNonobsolete );
    nextUnfinishedAct->setEnabled( numFinished != numNonobsolete );
    prevAct->setEnabled( lv->firstChild() != 0 );
    nextAct->setEnabled( lv->firstChild() != 0 );
}

void TrWindow::updatePhraseDict()
{
    QPtrListIterator<PhraseBook> pb = phraseBooks;
    PhraseBook::Iterator p;
    PhraseBook *ent;
    phraseDict.clear();
    while ( pb.current() != 0 ) {
	for ( p = (*pb)->begin(); p != (*pb)->end(); ++p ) {
	    QString f = friendlyString( (*p).source() );
	    if ( f.length() > 0 ) {
		f = QStringList::split( QChar(' '), f ).first();
		ent = phraseDict.find( f );
		if ( ent == 0 ) {
		    ent = new PhraseBook;
		    phraseDict.insert( f, ent );
		}
		ent->append( *p );
	    }
	}
	++pb;
    }
    revalidate();
}

PhraseBook TrWindow::getPhrases( const QString& source )
{
    PhraseBook phrases;
    QString f = friendlyString( source );
    QStringList lookupWords = QStringList::split( QChar(' '), f );
    QStringList::Iterator w;
    PhraseBook::Iterator p;

    for ( w = lookupWords.begin(); w != lookupWords.end(); ++w ) {
	PhraseBook *ent = phraseDict.find( *w );
	if ( ent != 0 ) {
	    for ( p = ent->begin(); p != ent->end(); ++p ) {
		if ( f.find(friendlyString((*p).source())) >= 0 )
		    phrases.append( *p );
	    }
	}
    }
    return phrases;
}

bool TrWindow::danger( const QString& source, const QString& translation,
		       bool verbose )
{
    if ( acceleratorsAct->isOn() ) {
	int sk = QAccel::shortcutKey( source );
	int tk = QAccel::shortcutKey( translation );
	if ( sk == 0 && tk != 0 ) {
	    if ( verbose )
		statusBar()->message( tr("Accelerator possibly superfluous in"
					 " translation."), ErrorMS );
	    return TRUE;
	} else if ( sk != 0 && tk == 0 ) {
	    if ( verbose )
		statusBar()->message( tr("Accelerator possibly missing in"
					 " translation."), ErrorMS );
	    return TRUE;
	}
    }
    if ( endingPunctuationAct->isOn() ) {
	if ( ending(source) != ending(translation) ) {
	    if ( verbose )
		statusBar()->message( tr("Translation does not end with the"
					 " same punctuation as the source"
					 " text."), ErrorMS );
	    return TRUE;
	}
    }
    if ( phraseMatchesAct->isOn() ) {
	QString fsource = friendlyString( source );
	QString ftranslation = friendlyString( translation );
	QStringList lookupWords = QStringList::split( QChar(' '), fsource );
	QStringList::Iterator w;
	PhraseBook::Iterator p;

	for ( w = lookupWords.begin(); w != lookupWords.end(); ++w ) {
	    PhraseBook *ent = phraseDict.find( *w );
	    if ( ent != 0 ) {
		for ( p = ent->begin(); p != ent->end(); ++p ) {
		    if ( fsource.find(friendlyString((*p).source())) < 0 ||
			 ftranslation.find(friendlyString((*p).target())) >= 0 )
			break;
		}
		if ( p == ent->end() ) {
		    if ( verbose )
			statusBar()->message( tr("A phrase book suggestion for"
						 " '%1' was ignored.")
						 .arg(*w), ErrorMS );
		    return TRUE;
		}
	    }
	}
    }
    if ( verbose )
	statusBar()->clear();

    return FALSE;
}

void TrWindow::readConfig()
{
    QString keybase( "/Qt Linguist/" +
		     QString::number( (QT_VERSION >> 16) & 0xff ) +
		     "." + QString::number( (QT_VERSION >> 8) & 0xff ) + "/" );
    QSettings config;

    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QRect r( pos(), size() );
    recentFiles = config.readListEntry( keybase + "RecentlyOpenedFiles" );
    if ( !config.readBoolEntry( keybase + "Geometry/MainwindowMaximized", FALSE ) ) {
	r.setX( config.readNumEntry( keybase + "Geometry/MainwindowX", r.x() ) );
	r.setY( config.readNumEntry( keybase + "Geometry/MainwindowY", r.y() ) );
	r.setWidth( config.readNumEntry( keybase + "Geometry/MainwindowWidth", r.width() ) );
	r.setHeight( config.readNumEntry( keybase + "Geometry/MainwindowHeight", r.height() ) );

	QRect desk = QApplication::desktop()->geometry();
	QRect inter = desk.intersect( r );
	resize( r.size() );
	if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
	    move( r.topLeft() );
	}
    }

    QDockWindow * dw;
    dw = (QDockWindow *) lv->parent();
    int place;
    place = config.readNumEntry( keybase + "Geometry/ContextwindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/ContextwindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/ContextwindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/ContextwindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/ContextwindowHeight" ) );
    if ( place == QDockWindow::OutsideDock ) {
	dw->undock();
	dw->show();
    }
    dw->setGeometry( r );

    dw = (QDockWindow *) slv->parent();
    place = config.readNumEntry( keybase + "Geometry/SourcewindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/SourcewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/SourcewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/SourcewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/SourcewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock ) {
	dw->undock();
	dw->show();
    }
    dw->setGeometry( r );

    dw = (QDockWindow *) plv->parent()->parent();
    place = config.readNumEntry( keybase + "Geometry/PhrasewindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/PhrasewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/PhrasewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/PhrasewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/PhrasewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock ) {
	dw->undock();
	dw->show();
    }
    dw->setGeometry( r );
    QApplication::sendPostedEvents();
}

void TrWindow::writeConfig()
{
    QString keybase( "/Qt Linguist/" +
		     QString::number( (QT_VERSION >> 16) & 0xff ) +
		     "." + QString::number( (QT_VERSION >> 8) & 0xff ) + "/" );
    QSettings config;

    config.insertSearchPath( QSettings::Windows, "/Trolltech" );
    config.writeEntry( keybase + "RecentlyOpenedFiles", recentFiles );
    config.writeEntry( keybase + "Geometry/MainwindowMaximized", isMaximized() );
    config.writeEntry( keybase + "Geometry/MainwindowX", x() );
    config.writeEntry( keybase + "Geometry/MainwindowY", y() );
    config.writeEntry( keybase + "Geometry/MainwindowWidth", width() );
    config.writeEntry( keybase + "Geometry/MainwindowHeight", height() );

    QDockWindow * dw =(QDockWindow *) lv->parent();
    config.writeEntry( keybase + "Geometry/ContextwindowInDock", dw->place() );
    config.writeEntry( keybase + "Geometry/ContextwindowX", dw->x() );
    config.writeEntry( keybase + "Geometry/ContextwindowY", dw->y() );
    config.writeEntry( keybase + "Geometry/ContextwindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/ContextwindowHeight", dw->height() );

    dw =(QDockWindow *) slv->parent();
    config.writeEntry( keybase + "Geometry/SourcewindowInDock",
		       dw->place() );
    config.writeEntry( keybase + "Geometry/SourcewindowX", dw->geometry().x() );
    config.writeEntry( keybase + "Geometry/SourcewindowY", dw->geometry().y() );
    config.writeEntry( keybase + "Geometry/SourcewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/SourcewindowHeight", dw->height() );

    dw =(QDockWindow *) plv->parent()->parent();
    config.writeEntry( keybase + "Geometry/PhrasewindowInDock",
		       dw->place() );
    config.writeEntry( keybase + "Geometry/PhrasewindowX", dw->geometry().x() );
    config.writeEntry( keybase + "Geometry/PhrasewindowY", dw->geometry().y() );
    config.writeEntry( keybase + "Geometry/PhrasewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/PhrasewindowHeight", dw->height() );
}

void TrWindow::setupRecentFilesMenu()
{
    recentFilesMenu->clear();
    int id = 0;
    QStringList::Iterator it = recentFiles.begin();
    for ( ; it != recentFiles.end(); ++it )
    {
	recentFilesMenu->insertItem( *it, id );
	id++;
    }
}

void TrWindow::recentFileActivated( int id )
{
    if ( id != -1 ) {
	if ( maybeSave() )
	    openFile( *recentFiles.at( id ) );
    }
}

void TrWindow::addRecentlyOpenedFile( const QString &fn, QStringList &lst )
{
    if ( lst.find( fn ) != lst.end() )
	return;
    if ( lst.count() >= 10 )
	lst.remove( lst.begin() );
    lst << fn;
}

void TrWindow::toggleGuessing()
{
    me->toggleGuessing();
}

void TrWindow::focusSourceList()
{
    slv->setFocus();
}

void TrWindow::focusPhraseList()
{
    plv->setFocus();
}

void TrWindow::updateClosePhraseBook()
{
    if ( dirtyItem != -1 ) {
	closePhraseBookp->removeItem( closePhraseBookp->idAt(dirtyItem) );
	dirtyItem = -1;
    }
}

void TrWindow::toggleStatistics()
{
    if ( toggleStats->isOn() ) {
	if ( !stats ) {
	    stats = new Statistics( this, "linguist_stats" );
	    connect( this, SIGNAL(statsChanged(int,int,int,int,int,int)), stats,
		     SLOT(updateStats(int,int,int,int,int,int)) );
	    connect( stats, SIGNAL(closed()), toggleStats, SLOT(toggle()) );
	}
	updateStatistics();
	stats->show();
    } else if ( stats ) {
	stats->close();
    }
}

void TrWindow::updateStatistics()
{
    QListViewItem * ci = lv->firstChild();
    int trW = 0;
    int trC = 0;
    int trCS = 0;
    while ( ci ) {
 	countStats( ci, ((ContextLVI *)ci)->firstMessageItem(), trW, trC, trCS );
	ci = ci->nextSibling();
    }
    // ..and the items in the source list
    countStats( 0, slv->firstChild(), trW, trC, trCS );
    emit statsChanged( srcWords, srcChars, srcCharsSpc, trW, trC, trCS );
}


void TrWindow::countStats( QListViewItem* ci, QListViewItem* mi, int& trW, int& trC, int& trCS )
{
    MessageLVI * m;
    while ( mi ) {
	m = (MessageLVI *) mi;
	if ( m->finished() && !(m->message().type() == MetaTranslatorMessage::Obsolete) )
	    doCharCounting( m->translation(), trW, trC, trCS );
	if ( ci )
	    mi = ((ContextLVI *)ci)->nextMessageItem();
	else
	    mi = mi->nextSibling();
    }
}

void TrWindow::doCharCounting( const QString& text, int& trW, int& trC, int& trCS )
{
    trCS += text.length();
    bool inWord = FALSE;
    for ( int i = 0; i < (int) text.length(); i++ ) {
	if ( text[i].isLetterOrNumber() || text[i] == QChar('_') ) {
	    if ( !inWord ) {
		trW++;
		inWord = TRUE;
	    }
	} else {
	    inWord = FALSE;
	}
	if ( !text[i].isSpace() )
	    trC++;
    }
}
