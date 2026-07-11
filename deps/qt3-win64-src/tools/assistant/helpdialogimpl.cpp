/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Assistant.
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

#include "helpdialogimpl.h"
#include "helpwindow.h"
#include "topicchooserimpl.h"
#include "docuparser.h"
#include "mainwindow.h"
#include "config.h"
#include "tabbedbrowser.h"

#include <qaccel.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qdir.h>
#include <qeventloop.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qprogressbar.h>
#include <qptrlist.h>
#include <qptrstack.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qurl.h>
#include <qvalidator.h>

#include <stdlib.h>
#include <limits.h>

static QString stripAmpersand( const QString &str )
{
    QString s( str );
    s = s.replace( '&', "" );
    return s;
}

static bool verifyDirectory(const QString &str)
{
    QFileInfo dirInfo(str);
    if (!dirInfo.exists())
	return QDir().mkdir(str);
    if (!dirInfo.isDir()) {
	qWarning("'%s' exists but is not a directory", str.latin1());
	return FALSE;
    }
    return TRUE;
}

struct IndexKeyword {
    IndexKeyword( const QString &kw, const QString &l )
	: keyword( kw ), link( l ) {}
    IndexKeyword() : keyword( QString::null ), link( QString::null ) {}
    bool operator<( const IndexKeyword &ik ) const {
	return keyword.lower() < ik.keyword.lower();
    }
    bool operator<=( const IndexKeyword &ik ) const {
	return keyword.lower() <= ik.keyword.lower();
    }
    bool operator>( const IndexKeyword &ik ) const {
	return keyword.lower() > ik.keyword.lower();
    }
    Q_DUMMY_COMPARISON_OPERATOR( IndexKeyword )
    QString keyword;
    QString link;
};

QDataStream &operator>>( QDataStream &s, IndexKeyword &ik )
{
    s >> ik.keyword;
    s >> ik.link;
    return s;
}

QDataStream &operator<<( QDataStream &s, const IndexKeyword &ik )
{
    s << ik.keyword;
    s << ik.link;
    return s;
}

QValidator::State SearchValidator::validate( QString &str, int & ) const
{
    for ( int i = 0; i < (int) str.length(); ++i ) {
	QChar c = str[i];
	if ( !c.isLetterOrNumber() && c != '\'' && c != '`'
	    && c != '\"' && c != ' ' && c != '-' && c != '_'
	    && c!= '*' )
	    return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}

HelpNavigationListItem::HelpNavigationListItem( QListBox *ls, const QString &txt )
    : QListBoxText( ls, txt )
{
}

void HelpNavigationListItem::addLink( const QString &link )
{
    int hash = link.find( '#' );
    if ( hash == -1 ) {
	linkList << link;
	return;
    }

    QString preHash = link.left( hash );
    if ( linkList.grep( preHash, FALSE ).count() > 0 )
	return;
    linkList << link;
}

HelpNavigationContentsItem::HelpNavigationContentsItem( QListView *v, QListViewItem *after )
    : QListViewItem( v, after )
{
}

HelpNavigationContentsItem::HelpNavigationContentsItem( QListViewItem *v, QListViewItem *after )
    : QListViewItem( v, after )
{
}

void HelpNavigationContentsItem::setLink( const QString &lnk )
{
    theLink = lnk;
}

QString HelpNavigationContentsItem::link() const
{
    return theLink;
}



HelpDialog::HelpDialog( QWidget *parent, MainWindow *h )
    : HelpDialogBase( parent, 0, FALSE ),  lwClosed( FALSE ), help( h )
{
}

void HelpDialog::initialize()
{
    connect( tabWidget, SIGNAL( selected(const QString&) ),
	     this, SLOT( currentTabChanged(const QString&) ) );
    connect( listContents, SIGNAL( mouseButtonClicked(int, QListViewItem*, const QPoint &, int ) ),
	     this, SLOT( showTopic(int,QListViewItem*, const QPoint &) ) );
    connect( listContents, SIGNAL( currentChanged(QListViewItem*) ),
	     this, SLOT( currentContentsChanged(QListViewItem*) ) );
    connect( listContents, SIGNAL( selectionChanged(QListViewItem*) ),
	     this, SLOT( currentContentsChanged(QListViewItem*) ) );
    connect( listContents, SIGNAL( doubleClicked(QListViewItem*) ),
	     this, SLOT( showTopic(QListViewItem*) ) );
    connect( listContents, SIGNAL( returnPressed(QListViewItem*) ),
	     this, SLOT( showTopic(QListViewItem*) ) );
    connect( listContents, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
	     this, SLOT( showItemMenu( QListViewItem*, const QPoint& ) ) );
    connect( editIndex, SIGNAL( returnPressed() ),
	     this, SLOT( showTopic() ) );
    connect( editIndex, SIGNAL( textChanged(const QString&) ),
	     this, SLOT( searchInIndex(const QString&) ) );

    connect( listIndex, SIGNAL( selectionChanged(QListBoxItem*) ),
	     this, SLOT( currentIndexChanged(QListBoxItem*) ) );
    connect( listIndex, SIGNAL( returnPressed(QListBoxItem*) ),
	     this, SLOT( showTopic() ) );
    connect( listIndex, SIGNAL( mouseButtonClicked(int, QListBoxItem*, const QPoint &) ),
	     this, SLOT( showTopic( int, QListBoxItem *, const QPoint & ) ) );
    connect( listIndex, SIGNAL( currentChanged(QListBoxItem*) ),
	     this, SLOT( currentIndexChanged(QListBoxItem*) ) );
    connect( listIndex, SIGNAL( contextMenuRequested( QListBoxItem*, const QPoint& ) ),
	     this, SLOT( showItemMenu( QListBoxItem*, const QPoint& ) ) );

    connect( listBookmarks, SIGNAL( mouseButtonClicked(int, QListViewItem*, const QPoint&, int ) ),
	     this, SLOT( showTopic(int, QListViewItem*, const QPoint &) ) );
    connect( listBookmarks, SIGNAL( returnPressed(QListViewItem*) ),
	     this, SLOT( showTopic(QListViewItem*) ) );
    connect( listBookmarks, SIGNAL( selectionChanged(QListViewItem*) ),
	     this, SLOT( currentBookmarkChanged(QListViewItem*) ) );
    connect( listBookmarks, SIGNAL( currentChanged(QListViewItem*) ),
	     this, SLOT( currentBookmarkChanged(QListViewItem*) ) );
    connect( listBookmarks, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
	     this, SLOT( showItemMenu( QListViewItem*, const QPoint& ) ) );
    connect( resultBox, SIGNAL( contextMenuRequested( QListBoxItem*, const QPoint& ) ),
	     this, SLOT( showItemMenu( QListBoxItem*, const QPoint& ) ) );

    cacheFilesPath = QDir::homeDirPath() + "/.assistant/"; //### Find a better location for the dbs

    editIndex->installEventFilter( this );
    listBookmarks->header()->hide();
    listBookmarks->header()->setStretchEnabled( TRUE );
    listContents->header()->hide();
    listContents->header()->setStretchEnabled( TRUE );
    framePrepare->hide();
    connect( qApp, SIGNAL(lastWindowClosed()), SLOT(lastWinClosed()) );

    termsEdit->setValidator( new SearchValidator( termsEdit ) );

    itemPopup = new QPopupMenu( this );
    itemPopup->insertItem( tr( "Open Link in Current Tab" ), 0 );
    itemPopup->insertItem( tr( "Open Link in New Window" ), 1 );
    itemPopup->insertItem( tr( "Open Link in New Tab" ), 2 );

    contentList.setAutoDelete( TRUE );
    contentList.clear();

    initDoneMsgShown = FALSE;
    fullTextIndex = 0;
    indexDone = FALSE;
    titleMapDone = FALSE;
    contentsInserted = FALSE;
    bookmarksInserted = FALSE;
    setupTitleMap();
}


void HelpDialog::processEvents()
{
    qApp->eventLoop()->processEvents( QEventLoop::ExcludeUserInput );
}


void HelpDialog::lastWinClosed()
{
    lwClosed = TRUE;
}


void HelpDialog::removeOldCacheFiles()
{
    QString dir = cacheFilesPath; // ### remove the last '/' ?
    if (!verifyDirectory(cacheFilesPath)) {
	qWarning( "Failed to created assistant directory" );
	return;
    }
    QString pname = "." + Config::configuration()->profileName();

    QStringList fileList;
    fileList <<  "indexdb" << "indexdb.dict" << "indexdb.doc" << "contentdb";
    QStringList::iterator it = fileList.begin();
    for ( ; it != fileList.end(); ++it ) {
	if ( QFile::exists( cacheFilesPath + *it + pname ) ) {
	    QFile f( cacheFilesPath + *it + pname );
	    f.remove();
	}
    }
}

void HelpDialog::timerEvent(QTimerEvent *e)
{
    static int opacity = 255;
    // To allow patching of 3.2.3 assistant for qsa.
#if QT_VERSION >= 0x030300
    help->setWindowOpacity((opacity-=4)/255.0);
    if (opacity<=0)
#endif
	qApp->quit();
}


void HelpDialog::loadIndexFile()
{
    if ( indexDone )
	return;

    setCursor( waitCursor );
    indexDone = TRUE;
    labelPrepare->setText( tr( "Prepare..." ) );
    framePrepare->show();
    processEvents();

    QProgressBar *bar = progressPrepare;
    bar->setTotalSteps( 100 );
    bar->setProgress( 0 );

    keywordDocuments.clear();
    QValueList<IndexKeyword> lst;
    QFile indexFile( cacheFilesPath + "indexdb." +
		     Config::configuration()->profileName() );
    if ( !indexFile.open( IO_ReadOnly ) ) {
	buildKeywordDB();
	processEvents();
	if( lwClosed )
	    return;
	if (!indexFile.open(IO_ReadOnly)) {
	    QMessageBox::warning(help, tr("Qt Assistant"), tr("Failed to load keyword index file\n"
							      "Assistant will not work!"));
#if defined Q_WS_WIN || defined Q_WS_MACX
	    startTimer(50);
#endif
	    return;
	}
    }

    editIndex->setEnabled(FALSE);

    QDataStream ds( &indexFile );
    Q_UINT32 fileAges;
    ds >> fileAges;
    if ( fileAges != getFileAges() ) {
	indexFile.close();
	buildKeywordDB();
	if ( !indexFile.open( IO_ReadOnly ) ) {
	    QMessageBox::warning( help, tr( "Qt Assistant" ),
		tr( "Cannot open the index file %1" ).arg( QFileInfo( indexFile ).absFilePath() ) );
	    editIndex->setEnabled(TRUE);
	    return;
	}
	ds.setDevice( &indexFile );
	ds >> fileAges;
    }
    ds >> lst;
    indexFile.close();

    bar->setProgress( bar->totalSteps() );
    processEvents();

    listIndex->clear();
    HelpNavigationListItem *lastItem = 0;
    QString lastKeyword = QString::null;
    QValueList<IndexKeyword>::ConstIterator it = lst.begin();
    for ( ; it != lst.end(); ++it ) {
        if ( lastKeyword.lower() != (*it).keyword.lower() )
            lastItem = new HelpNavigationListItem( listIndex, (*it).keyword );
        lastItem->addLink( (*it).link );
        lastKeyword = (*it).keyword;

        QString lnk = (*it).link;
        int i = lnk.findRev('#');
        if ( i > -1 )
            lnk = lnk.left( i );
        if (!keywordDocuments.contains(lnk))
            keywordDocuments.append(lnk);
    }
    framePrepare->hide();
    showInitDoneMessage();
    setCursor( arrowCursor );
    editIndex->setEnabled(TRUE);
}

Q_UINT32 HelpDialog::getFileAges()
{
    QStringList addDocuFiles = Config::configuration()->docFiles();
    QStringList::const_iterator i = addDocuFiles.begin();

    Q_UINT32 fileAges = 0;
    for( ; i != addDocuFiles.end(); ++i ) {
	QFileInfo fi( *i );
	if ( fi.exists() )
	    fileAges += fi.lastModified().toTime_t();
    }

    return fileAges;
}

void HelpDialog::buildKeywordDB()
{
    QStringList addDocuFiles = Config::configuration()->docFiles();
    QStringList::iterator i = addDocuFiles.begin();

    int steps = 0;
    for( ; i != addDocuFiles.end(); i++ )
	steps += QFileInfo( *i ).size();

    labelPrepare->setText( tr( "Prepare..." ) );
    progressPrepare->setTotalSteps( steps );
    progressPrepare->setProgress( 0 );
    processEvents();

    QValueList<IndexKeyword> lst;
    Q_UINT32 fileAges = 0;
    for( i = addDocuFiles.begin(); i != addDocuFiles.end(); i++ ){
	QFile file( *i );
	if ( !file.exists() ) {
	    QMessageBox::warning( this, tr( "Warning" ),
		tr( "Documentation file %1 does not exist!\n"
		    "Skipping file." ).arg( QFileInfo( file ).absFilePath() ) );
	    continue;
        }
	fileAges += QFileInfo( file ).lastModified().toTime_t();
	DocuParser *handler = DocuParser::createParser( *i );
	bool ok = handler->parse( &file );
	file.close();
	if( !ok ){
	    QString msg = QString( "In file %1:\n%2" )
			  .arg( QFileInfo( file ).absFilePath() )
			  .arg( handler->errorProtocol() );
	    QMessageBox::critical( this, tr( "Parse Error" ), tr( msg ) );
	    delete handler;
	    continue;
	}

	QPtrList<IndexItem> indLst = handler->getIndexItems();
	QPtrListIterator<IndexItem> it( indLst );
	IndexItem *indItem;
	int counter = 0;
	while ( ( indItem = it.current() ) != 0 ) {
	    QFileInfo fi( indItem->reference );
	    lst.append( IndexKeyword( indItem->keyword, fi.absFilePath() ) );
	    if ( progressPrepare )
		progressPrepare->setProgress( progressPrepare->progress() +
					      int(fi.absFilePath().length() * 1.6) );

	    if( ++counter%100 == 0 ) {
		processEvents();
		if( lwClosed ) {
		    return;
		}
	    }
	    ++it;
	}
	delete handler;
    }
    if ( !lst.isEmpty() )
	qHeapSort( lst );

    QFile indexout( cacheFilesPath + "indexdb." + Config::configuration()->profileName() );
    if ( verifyDirectory(cacheFilesPath) && indexout.open( IO_WriteOnly ) ) {
	QDataStream s( &indexout );
	s << fileAges;
	s << lst;
	indexout.close();
    }
}

void HelpDialog::setupTitleMap()
{
    if ( titleMapDone )
	return;
    if ( Config::configuration()->docRebuild() ) {
	removeOldCacheFiles();
	Config::configuration()->setDocRebuild( FALSE );
	Config::configuration()->saveProfile( Config::configuration()->profile() );
    }
    if ( contentList.isEmpty() )
	getAllContents();

    titleMapDone = TRUE;
    titleMap.clear();
    QDictIterator<ContentList> lstIt( contentList );
    for ( ; lstIt.current(); ++lstIt ) {
	QValueList<ContentItem> &lst = *(lstIt.current());
	QValueListConstIterator<ContentItem> it;
	for ( it = lst.begin(); it != lst.end(); ++it ) {
	    QFileInfo link( (*it).reference.simplifyWhiteSpace() );
	    titleMap[ link.absFilePath() ] = (*it).title.stripWhiteSpace();
	}
    }
    processEvents();
}

void HelpDialog::getAllContents()
{
    QFile contentFile( cacheFilesPath + "contentdb." + Config::configuration()->profileName() );
    contentList.clear();
    if ( !contentFile.open( IO_ReadOnly ) ) {
	buildContentDict();
	return;
    }

    QDataStream ds( &contentFile );
    Q_UINT32 fileAges;
    ds >> fileAges;
    if ( fileAges != getFileAges() ) {
	contentFile.close();
	buildContentDict();
	return;
    }
    QString key;
    QValueList<ContentItem> lst;
    while ( !ds.atEnd() ) {
	ds >> key;
	ds >> lst;
	contentList.insert( key, new QValueList<ContentItem>( lst ) );
    }
    contentFile.close();
    processEvents();

}

void HelpDialog::buildContentDict()
{
    QStringList docuFiles = Config::configuration()->docFiles();

    Q_UINT32 fileAges = 0;
    for( QStringList::iterator it = docuFiles.begin(); it != docuFiles.end(); it++ ) {
	QFile file( *it );
	if ( !file.exists() ) {
	    QMessageBox::warning( this, tr( "Warning" ),
	    tr( "Documentation file %1 does not exist!\n"
	        "Skipping file." ).arg( QFileInfo( file ).absFilePath() ) );
	    continue;
        }
	fileAges += QFileInfo( file ).lastModified().toTime_t();
	DocuParser *handler = DocuParser::createParser( *it );
	if( !handler ) {
	    QMessageBox::warning( this, tr( "Warning" ),
	    tr( "Documentation file %1 is not compatible!\n"
	        "Skipping file." ).arg( QFileInfo( file ).absFilePath() ) );
	    continue;
	}
	bool ok = handler->parse( &file );
	file.close();
	if( ok ) {
	    contentList.insert( *it, new QValueList<ContentItem>( handler->getContentItems() ) );
	    delete handler;
	} else {
	    QString msg = QString( "In file %1:\n%2" )
			  .arg( QFileInfo( file ).absFilePath() )
			  .arg( handler->errorProtocol() );
	    QMessageBox::critical( this, tr( "Parse Error" ), tr( msg ) );
	    continue;
	}
    }

    QString pname = Config::configuration()->profileName();
    QFile contentOut( cacheFilesPath + "contentdb." + pname );
    QFile::remove( cacheFilesPath + "indexdb.dict." + pname );
    QFile::remove( cacheFilesPath + "indexdb.doc." + pname );
    if ( contentOut.open( IO_WriteOnly ) ) {
	QDataStream s( &contentOut );
	s << fileAges;
	QDictIterator<ContentList> it( contentList );
	for ( ; it.current(); ++it ) {
	    s << it.currentKey();
	    s << *(it.current());
	}
	contentOut.close();
    }
}

void HelpDialog::currentTabChanged( const QString &s )
{
    if ( stripAmpersand( s ).contains( tr( "Index" ) ) )
	QTimer::singleShot( 0, this, SLOT( loadIndexFile() ) );
    else if ( stripAmpersand( s ).contains( tr( "Bookmarks" ) ) )
	insertBookmarks();
    else if ( stripAmpersand( s ).contains( tr( "Contents" ) ) )
	QTimer::singleShot( 0, this, SLOT( insertContents() ) );
    else if ( stripAmpersand( s ).contains( tr( "Search" ) ) )
	QTimer::singleShot( 0, this, SLOT( setupFullTextIndex() ) );
}

void HelpDialog::showInitDoneMessage()
{
    if ( initDoneMsgShown )
	return;
    initDoneMsgShown = TRUE;
    help->statusBar()->message( tr( "Done" ), 3000 );
}

void HelpDialog::currentIndexChanged( QListBoxItem * )
{
}


void HelpDialog::showTopic( int button, QListBoxItem *item,
			    const QPoint & )
{
    if( button == LeftButton && item )
	showTopic();
}

void HelpDialog::showTopic( int button, QListViewItem *item,
			    const QPoint & )
{
    if( button == LeftButton && item )
	showTopic();
}

void HelpDialog::showTopic( QListViewItem *item )
{
    if( item )
	showTopic();
}

void HelpDialog::showTopic()
{
    if ( stripAmpersand( tabWidget->tabLabel( tabWidget->currentPage() ) ).contains( tr( "Index" ) ) )
	showIndexTopic();
    else if ( stripAmpersand( tabWidget->tabLabel( tabWidget->currentPage() ) ).contains( tr( "Bookmarks" ) ) )
	showBookmarkTopic();
    else if ( stripAmpersand( tabWidget->tabLabel( tabWidget->currentPage() ) ).contains( tr( "Contents" ) ) )
	showContentsTopic();
}

void HelpDialog::showIndexTopic()
{
    QListBoxItem *i = listIndex->item( listIndex->currentItem() );
    if ( !i )
	return;

    editIndex->blockSignals( TRUE );
    editIndex->setText( i->text() );
    editIndex->blockSignals( FALSE );

    HelpNavigationListItem *item = (HelpNavigationListItem*)i;

    QStringList links = item->links();
    if ( links.count() == 1 ) {
	emit showLink( links.first() );
    } else {
	qHeapSort( links );
	QStringList::Iterator it = links.begin();
	QStringList linkList;
	QStringList linkNames;
	for ( ; it != links.end(); ++it ) {
	    linkList << *it;
	    linkNames << titleOfLink( *it );
	}
	QString link = TopicChooser::getLink( this, linkNames, linkList, i->text() );
	if ( !link.isEmpty() )
	    emit showLink( link );
    }
}

void HelpDialog::searchInIndex( const QString &s )
{
    QListBoxItem *i = listIndex->firstItem();
    QString sl = s.lower();
    while ( i ) {
	QString t = i->text();
	if ( t.length() >= sl.length() &&
	     i->text().left(s.length()).lower() == sl ) {
	    listIndex->setCurrentItem( i );
	    listIndex->setTopItem(listIndex->index(i));
	    break;
	}
	i = i->next();
    }
}

QString HelpDialog::titleOfLink( const QString &link )
{
    QString s( link );
    s.remove( s.find( '#' ), s.length() );
    s = titleMap[ s ];
    if ( s.isEmpty() )
	return link;
    return s;
}

bool HelpDialog::eventFilter( QObject * o, QEvent * e )
{
    if ( !o || !e )
	return TRUE;

    if ( o == editIndex && e->type() == QEvent::KeyPress ) {
	QKeyEvent *ke = (QKeyEvent*)e;
	if ( ke->key() == Key_Up ) {
	    int i = listIndex->currentItem();
	    if ( --i >= 0 ) {
		listIndex->setCurrentItem( i );
		editIndex->blockSignals( TRUE );
		editIndex->setText( listIndex->currentText() );
		editIndex->blockSignals( FALSE );
	    }
	    return TRUE;
	} else if ( ke->key() == Key_Down ) {
	    int i = listIndex->currentItem();
	    if ( ++i < int(listIndex->count()) ) {
		listIndex->setCurrentItem( i );
		editIndex->blockSignals( TRUE );
		editIndex->setText( listIndex->currentText() );
		editIndex->blockSignals( FALSE );
	    }
	    return TRUE;
	} else if ( ke->key() == Key_Next || ke->key() == Key_Prior ) {
	    QApplication::sendEvent( listIndex, e);
	    editIndex->blockSignals( TRUE );
	    editIndex->setText( listIndex->currentText() );
	    editIndex->blockSignals( FALSE );
	}
    }

    return QWidget::eventFilter( o, e );
}

void HelpDialog::addBookmark()
{
    if ( !bookmarksInserted )
	insertBookmarks();
    QString link = QUrl(  help->browsers()->currentBrowser()->context(),
			  help->browsers()->currentBrowser()->source() ).path();
    QString title = help->browsers()->currentBrowser()->documentTitle();
    if ( title.isEmpty() )
	title = titleOfLink( link );
    HelpNavigationContentsItem *i = new HelpNavigationContentsItem( listBookmarks, 0 );
    i->setText( 0, title );
    i->setLink( link );
    saveBookmarks();
    help->updateBookmarkMenu();
}

void HelpDialog::removeBookmark()
{
    if ( !listBookmarks->currentItem() )
	return;

    delete listBookmarks->currentItem();
    saveBookmarks();
    if ( listBookmarks->firstChild() ) {
	listBookmarks->setSelected( listBookmarks->firstChild(), TRUE );
    }
    help->updateBookmarkMenu();
}

void HelpDialog::insertBookmarks()
{
    if ( bookmarksInserted )
	return;
    bookmarksInserted = TRUE;
    listBookmarks->clear();
    QFile f( cacheFilesPath + "bookmarks." + Config::configuration()->profileName() );
    if ( !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    while ( !ts.atEnd() ) {
	HelpNavigationContentsItem *i = new HelpNavigationContentsItem( listBookmarks, 0 );
	i->setText( 0, ts.readLine() );
	i->setLink( ts.readLine() );
    }
    help->updateBookmarkMenu();
    showInitDoneMessage();
}

void HelpDialog::currentBookmarkChanged( QListViewItem * )
{
}

void HelpDialog::showBookmarkTopic()
{
    if ( !listBookmarks->currentItem() )
	return;

    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)listBookmarks->currentItem();
    QString absPath = "";
    if ( QFileInfo( i->link() ).isRelative() )
	absPath = documentationPath + "/";
    emit showLink( absPath + i->link() );
}

void HelpDialog::saveBookmarks()
{
    QFile f( cacheFilesPath + "bookmarks." + Config::configuration()->profileName() );
    if ( !f.open( IO_WriteOnly ) )
	return;
    QTextStream ts( &f );
    QListViewItemIterator it( listBookmarks );
    for ( ; it.current(); ++it ) {
	HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)it.current();
	ts << i->text( 0 ) << endl;
	ts << i->link() << endl;
    }
    f.close();
}

void HelpDialog::insertContents()
{
    if ( contentsInserted )
	return;

    if ( contentList.isEmpty() )
	getAllContents();

    contentsInserted = TRUE;
    listContents->clear();
    setCursor( waitCursor );
    if ( !titleMapDone )
	setupTitleMap();

    listContents->setSorting( -1 );

    QDictIterator<ContentList> lstIt( contentList );
    for ( ; lstIt.current(); ++lstIt ) {
	HelpNavigationContentsItem *newEntry;

	HelpNavigationContentsItem *contentEntry = 0;
	QPtrStack<HelpNavigationContentsItem> stack;
	stack.clear();
	int depth = 0;
	bool root = FALSE;

	HelpNavigationContentsItem *lastItem[64];
	for( int j = 0; j < 64; ++j )
	    lastItem[j] = 0;


	QValueList<ContentItem> &lst = *(lstIt.current());
	QValueListConstIterator<ContentItem> it;
	for( it = lst.begin(); it != lst.end(); ++it ){
	    ContentItem item = *it;
	    if( item.depth == 0 ){
		newEntry = new HelpNavigationContentsItem( listContents, 0 );
		newEntry->setPixmap( 0, QPixmap::fromMimeSource( "book.png" ) );
		newEntry->setText( 0, item.title );
		newEntry->setLink( item.reference );
		stack.push( newEntry );
		depth = 1;
		root = TRUE;
	    }
	    else{
		if( (item.depth > depth) && root ) {
		    depth = item.depth;
		    stack.push( contentEntry );
		}
		if( item.depth == depth ) {
		    contentEntry = new HelpNavigationContentsItem( stack.top(), lastItem[ depth ] );
		    lastItem[ depth ] = contentEntry;
		    contentEntry->setText( 0, item.title );
		    contentEntry->setLink( item.reference );
		}
		else if( item.depth < depth ) {
		    stack.pop();
		    depth--;
		    item = *(--it);
		}
	    }
	}
	processEvents();
    }
    setCursor( arrowCursor );
    showInitDoneMessage();
}

void HelpDialog::currentContentsChanged( QListViewItem * )
{
}

void HelpDialog::showContentsTopic()
{
    HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)listContents->currentItem();
    if ( !i )
	return;
    emit showLink( i->link() );
}

void HelpDialog::toggleContents()
{
    if ( !isVisible() || tabWidget->currentPageIndex() != 0 ) {
	tabWidget->setCurrentPage( 0 );
	parentWidget()->show();
    }
    else
	parentWidget()->hide();
}

void HelpDialog::toggleIndex()
{
    if ( !isVisible() || tabWidget->currentPageIndex() != 1 || !editIndex->hasFocus() ) {
	tabWidget->setCurrentPage( 1 );
	parentWidget()->show();
	editIndex->setFocus();
    }
    else
	parentWidget()->hide();
}

void HelpDialog::toggleBookmarks()
{
    if ( !isVisible() || tabWidget->currentPageIndex() != 2 ) {
	tabWidget->setCurrentPage( 2 );
	parentWidget()->show();
    }
    else
	parentWidget()->hide();
}

void HelpDialog::toggleSearch()
{
    if ( !isVisible() || tabWidget->currentPageIndex() != 3 ) {
	tabWidget->setCurrentPage( 3 );
	parentWidget()->show();
    }
    else
	parentWidget()->hide();
}

void HelpDialog::setupFullTextIndex()
{
    if ( fullTextIndex )
	return;

    QStringList documentList;    
    QString pname = Config::configuration()->profileName();
    fullTextIndex = new Index( documentList, QDir::homeDirPath() ); // ### Is this correct ?
    if (!verifyDirectory(cacheFilesPath)) {
        QMessageBox::warning(help, tr("Qt Assistant"),
            tr("Failed to save fulltext search index\n"
            "Assistant will not work!"));
        return;
    }
    searchButton->setEnabled(FALSE);
    helpButton->setEnabled(FALSE);
    termsEdit->setEnabled(FALSE);

    fullTextIndex->setDictionaryFile( cacheFilesPath + "indexdb.dict." + pname );
    fullTextIndex->setDocListFile( cacheFilesPath + "indexdb.doc." + pname );
    processEvents();

    connect( fullTextIndex, SIGNAL( indexingProgress( int ) ),
        this, SLOT( setIndexingProgress( int ) ) );
    QFile f( cacheFilesPath + "indexdb.dict." + pname );
    if ( !f.exists() ) {
        QMap<QString, QString>::ConstIterator it = titleMap.begin();
        QString documentName;
        for ( ; it != titleMap.end(); ++it ) {
            documentName = it.key();
            int i = documentName.findRev('#');
            if ( i > -1 )
                documentName = documentName.left( i );

            if (!documentList.contains(documentName))
                documentList << documentName;
        }
        loadIndexFile();
        for (QStringList::Iterator it2 = keywordDocuments.begin(); it2 != keywordDocuments.end(); ++it2) {
            if (!documentList.contains(*it2))
                documentList << *it2;
        }
        fullTextIndex->setDocList( documentList );

        help->statusBar()->clear();
        setCursor( waitCursor );
        labelPrepare->setText( tr( "Indexing files..." ) );
        progressPrepare->setTotalSteps( 100 );
        progressPrepare->reset();
        progressPrepare->show();
        framePrepare->show();
        processEvents();
        if ( fullTextIndex->makeIndex() != -1 ) {
            fullTextIndex->writeDict();
            progressPrepare->setProgress( 100 );
            framePrepare->hide();
            setCursor( arrowCursor );
            showInitDoneMessage();
            keywordDocuments.clear();
        }
    } else {
        setCursor( waitCursor );
        help->statusBar()->message( tr( "Reading dictionary..." ) );
        processEvents();
        fullTextIndex->readDict();
        help->statusBar()->message( tr( "Done" ), 3000 );
        setCursor( arrowCursor );
    }
    searchButton->setEnabled(TRUE);
    termsEdit->setEnabled(TRUE);
    helpButton->setEnabled(TRUE);
}

void HelpDialog::setIndexingProgress( int prog )
{
    progressPrepare->setProgress( prog );
    processEvents();
}

void HelpDialog::startSearch()
{
    QString str = termsEdit->text();
    str = str.replace( "\'", "\"" );
    str = str.replace( "`", "\"" );
    QString buf = str;
    str = str.replace( "-", " " );
    str = str.replace( QRegExp( "\\s[\\S]?\\s" ), " " );
    terms = QStringList::split( " ", str );
    QStringList termSeq;
    QStringList seqWords;
    QStringList::iterator it = terms.begin();
    for ( ; it != terms.end(); ++it ) {
	(*it) = (*it).simplifyWhiteSpace();
	(*it) = (*it).lower();
	(*it) = (*it).replace( "\"", "" );
    }
    if ( str.contains( '\"' ) ) {
	if ( (str.contains( '\"' ))%2 == 0 ) {
	    int beg = 0;
	    int end = 0;
	    QString s;
	    beg = str.find( '\"', beg );
	    while ( beg != -1 ) {
		beg++;
		end = str.find( '\"', beg );
		s = str.mid( beg, end - beg );
		s = s.lower();
		s = s.simplifyWhiteSpace();
		if ( s.contains( '*' ) ) {
		    QMessageBox::warning( this, tr( "Full Text Search" ),
			tr( "Using a wildcard within phrases is not allowed." ) );
		    return;
		}
		seqWords += QStringList::split( ' ', s );
		termSeq << s;
		beg = str.find( '\"', end + 1);
	    }
	} else {
	    QMessageBox::warning( this, tr( "Full Text Search" ),
		tr( "The closing quotation mark is missing." ) );
	    return;
	}
    }
    setCursor( waitCursor );
    foundDocs.clear();
    foundDocs = fullTextIndex->query( terms, termSeq, seqWords );
    QString msg( QString( "%1 documents found." ).arg( foundDocs.count() ) );
    help->statusBar()->message( tr( msg ), 3000 );
    resultBox->clear();
    for ( it = foundDocs.begin(); it != foundDocs.end(); ++it )
	resultBox->insertItem( fullTextIndex->getDocumentTitle( *it ) );

    terms.clear();
    bool isPhrase = FALSE;
    QString s = "";
    for ( int i = 0; i < (int)buf.length(); ++i ) {
	if ( buf[i] == '\"' ) {
	    isPhrase = !isPhrase;
	    s = s.simplifyWhiteSpace();
	    if ( !s.isEmpty() )
		terms << s;
	    s = "";
	} else if ( buf[i] == ' ' && !isPhrase ) {
	    s = s.simplifyWhiteSpace();
	    if ( !s.isEmpty() )
		terms << s;
	    s = "";
	} else
	    s += buf[i];
    }
    if ( !s.isEmpty() )
	terms << s;

    setCursor( arrowCursor );
}

void HelpDialog::showSearchHelp()
{
    emit showLink( Config::configuration()->assistantDocPath() + "/assistant-5.html" );
}

void HelpDialog::showResultPage( int button, QListBoxItem *i, const QPoint & )
{
    if( button == LeftButton ) {
	showResultPage( i );
    }
}

void HelpDialog::showResultPage( QListBoxItem *i )
{
    if( !i )
	return;
    emit showSearchLink( foundDocs[resultBox->index( i )], terms );
}

void HelpDialog::showItemMenu( QListBoxItem *item, const QPoint &pos )
{
    if ( !item )
	return;
    int id = itemPopup->exec( pos );
    if ( id == 0 ) {
	if ( stripAmpersand( tabWidget->tabLabel( tabWidget->currentPage() ) ).contains( tr( "Index" ) ) )
	    showTopic();
	else {
	    showResultPage( item );
	}
    } else if ( id > 0 ) {
	HelpWindow *hw = help->browsers()->currentBrowser();
	if ( stripAmpersand( tabWidget->tabLabel( tabWidget->currentPage() ) ).contains( tr( "Index" ) ) ) {
	    editIndex->blockSignals( TRUE );
	    editIndex->setText( item->text() );
	    editIndex->blockSignals( FALSE );

	    HelpNavigationListItem *hi = (HelpNavigationListItem*)item;

	    QStringList links = hi->links();
	    if ( links.count() == 1 ) {
		if ( id == 1 )
		    hw->openLinkInNewWindow( links.first() );
		else
		    hw->openLinkInNewPage( links.first() );
	    } else {
		QStringList::Iterator it = links.begin();
		QStringList linkList;
		QStringList linkNames;
		for ( ; it != links.end(); ++it ) {
		    linkList << *it;
		    linkNames << titleOfLink( *it );
		}
		QString link = TopicChooser::getLink( this, linkNames, linkList, item->text() );
		if ( !link.isEmpty() ) {
		    if ( id == 1 )
			hw->openLinkInNewWindow( link );
		    else
			hw->openLinkInNewPage( link );
		}
	    }
	} else {
	    QString link = foundDocs[ resultBox->index( item ) ];
	    if ( id == 1 )
		hw->openLinkInNewWindow( link );
	    else
		hw->openLinkInNewPage( link );
	}
    }
}

void HelpDialog::showItemMenu( QListViewItem *item, const QPoint &pos )
{
    if ( !item )
	return;
    int id = itemPopup->exec( pos );
    if ( id == 0 ) {
	if ( stripAmpersand( tabWidget->tabLabel( tabWidget->currentPage() ) ).contains( tr( "Contents" ) ) )
	    showContentsTopic();
	else
	    showBookmarkTopic();
    } else if ( id > 0 ) {
	HelpNavigationContentsItem *i = (HelpNavigationContentsItem*)item;
	if ( id == 1 )
	    help->browsers()->currentBrowser()->openLinkInNewWindow( i->link() );
	else
	    help->browsers()->currentBrowser()->openLinkInNewPage( i->link() );
    }
}
