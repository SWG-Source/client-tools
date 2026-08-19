/****************************************************************************
** $Id: qfiledialog.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QFileDialog class
**
** Created : 950429
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

#include "qplatformdefs.h"

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED.
#if defined(connect)
# undef connect
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#include "qfiledialog.h"

#ifndef QT_NO_FILEDIALOG

#include "private/qapplication_p.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qbuttongroup.h"
#include "qcheckbox.h"
#include "qcleanuphandler.h"
#include "qcombobox.h"
#include "qcstring.h"
#include "qcursor.h"
#include "qdragobject.h"
#include "qfile.h"
#include "qguardedptr.h"
#include "qhbox.h"
#include "qheader.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qlibrary.h"
#include "qlineedit.h"
#include "qlistbox.h"
#include "qlistview.h"
#include "qmap.h"
#include "qmessagebox.h"
#include "qmime.h"
#include "qnetworkprotocol.h"
#include "qobjectlist.h"
#include "qpainter.h"
#include "qpopupmenu.h"
#include "qprogressbar.h"
#include "qptrvector.h"
#include "qpushbutton.h"
#include "qregexp.h"
#include "qsemimodal.h"
#include "qsplitter.h"
#include "qstrlist.h"
#include "qstyle.h"
#include "qtimer.h"
#include "qtoolbutton.h"
#include "qtooltip.h"
#include "qvbox.h"
#include "qwidgetstack.h"

#ifdef Q_WS_WIN
#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT
#endif // Q_WS_WIN

#if !defined(Q_OS_TEMP)
#include <time.h>
#else
#include <shellapi.h>
#endif
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

#ifdef Q_WS_MAC
#include "qt_mac.h"
extern QString qt_mac_precomposeFileName(const QString &); // qglobal.cpp
#undef check
#endif

#if defined(Q_OS_CYGWIN) && defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
int getlogin_r (char* name, size_t namesize )
{
	char *s = getlogin();
	if (s) {
	 	strncpy (name, s, namesize);
		return 0;
	}
	else {
		return -1;
	}
}
#endif

/* XPM */
static const char * const start_xpm[]={
    "16 15 8 1",
    "a c #cec6bd",
    "# c #000000",
    "e c #ffff00",
    "b c #999999",
    "f c #cccccc",
    "d c #dcdcdc",
    "c c #ffffff",
    ". c None",
    ".....######aaaaa",
    "...bb#cccc##aaaa",
    "..bcc#cccc#d#aaa",
    ".bcef#cccc#dd#aa",
    ".bcfe#cccc#####a",
    ".bcef#ccccccccc#",
    "bbbbbbbbbbbbccc#",
    "bccccccccccbbcc#",
    "bcefefefefee#bc#",
    ".bcefefefefef#c#",
    ".bcfefefefefe#c#",
    "..bcfefefefeeb##",
    "..bbbbbbbbbbbbb#",
    "...#############",
    "................"};

/* XPM */
static const char * const end_xpm[]={
    "16 15 9 1",
    "d c #a0a0a0",
    "c c #c3c3c3",
    "# c #cec6bd",
    ". c #000000",
    "f c #ffff00",
    "e c #999999",
    "g c #cccccc",
    "b c #ffffff",
    "a c None",
    "......####aaaaaa",
    ".bbbb..###aaaaaa",
    ".bbbb.c.##aaaaaa",
    ".bbbb....ddeeeea",
    ".bbbbbbb.bbbbbe.",
    ".bbbbbbb.bcfgfe.",
    "eeeeeeeeeeeeefe.",
    "ebbbbbbbbbbeege.",
    "ebfgfgfgfgff.ee.",
    "aebfgfgfgfgfg.e.",
    "aebgfgfgfgfgf.e.",
    "aaebgfgfgfgffe..",
    "aaeeeeeeeeeeeee.",
    "aaa.............",
    "aaaaaaaaaaaaaaaa"};

/* XPM */
static const char* const open_xpm[]={
    "16 16 6 1",
    ". c None",
    "b c #ffff00",
    "d c #000000",
    "* c #999999",
    "c c #cccccc",
    "a c #ffffff",
    "................",
    "................",
    "...*****........",
    "..*aaaaa*.......",
    ".*abcbcba******.",
    ".*acbcbcaaaaaa*d",
    ".*abcbcbcbcbcb*d",
    "*************b*d",
    "*aaaaaaaaaa**c*d",
    "*abcbcbcbcbbd**d",
    ".*abcbcbcbcbcd*d",
    ".*acbcbcbcbcbd*d",
    "..*acbcbcbcbb*dd",
    "..*************d",
    "...ddddddddddddd",
    "................"};

/* XPM */
static const char * const link_dir_xpm[]={
    "16 16 10 1",
    "h c #808080",
    "g c #a0a0a0",
    "d c #000000",
    "b c #ffff00",
    "f c #303030",
    "# c #999999",
    "a c #cccccc",
    "e c #585858",
    "c c #ffffff",
    ". c None",
    "................",
    "................",
    "..#####.........",
    ".#ababa#........",
    "#abababa######..",
    "#cccccccccccc#d.",
    "#cbababababab#d.",
    "#cabababababa#d.",
    "#cbababdddddddd.",
    "#cababadccccccd.",
    "#cbababdcececcd.",
    "#cababadcefdfcd.",
    "#cbababdccgdhcd.",
    "#######dccchccd.",
    ".dddddddddddddd.",
    "................"};

/* XPM */
static const char * const link_file_xpm[]={
    "16 16 10 1",
    "h c #808080",
    "g c #a0a0a0",
    "d c #c3c3c3",
    ". c #7f7f7f",
    "c c #000000",
    "b c #bfbfbf",
    "f c #303030",
    "e c #585858",
    "a c #ffffff",
    "# c None",
    "################",
    "..........######",
    ".aaaaaaaab.#####",
    ".aaaaaaaaba.####",
    ".aaaaaaaacccc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaadc###",
    ".aaaaaaaaaadc###",
    ".aaaacccccccc###",
    ".aaaacaaaaaac###",
    ".aaaacaeaeaac###",
    ".aaaacaefcfac###",
    ".aaaacaagchac###",
    ".ddddcaaahaac###",
    "ccccccccccccc###"};

/* XPM */
static const char* const file_xpm[]={
    "16 16 5 1",
    ". c #7f7f7f",
    "# c None",
    "c c #000000",
    "b c #bfbfbf",
    "a c #ffffff",
    "################",
    "..........######",
    ".aaaaaaaab.#####",
    ".aaaaaaaaba.####",
    ".aaaaaaaacccc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".aaaaaaaaaabc###",
    ".bbbbbbbbbbbc###",
    "ccccccccccccc###"};

/* XPM */
static const char * const closed_xpm[]={
    "16 16 6 1",
    ". c None",
    "b c #ffff00",
    "d c #000000",
    "* c #999999",
    "a c #cccccc",
    "c c #ffffff",
    "................",
    "................",
    "..*****.........",
    ".*ababa*........",
    "*abababa******..",
    "*cccccccccccc*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "*cabababababa*d.",
    "*cbababababab*d.",
    "**************d.",
    ".dddddddddddddd.",
    "................"};


/* XPM */
static const char* const cdtoparent_xpm[]={
    "15 13 3 1",
    ". c None",
    "* c #000000",
    "a c #ffff99",
    "..*****........",
    ".*aaaaa*.......",
    "***************",
    "*aaaaaaaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaa***aaaaaaa*",
    "*aa*****aaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa*aaaaaaaa*",
    "*aaaa******aaa*",
    "*aaaaaaaaaaaaa*",
    "*aaaaaaaaaaaaa*",
    "***************"};


/* XPM */
static const char* const newfolder_xpm[] = {
    "15 14 4 1",
    "	c None",
    ".	c #000000",
    "+	c #FFFF00",
    "@	c #FFFFFF",
    "          .    ",
    "               ",
    "          .    ",
    "       .     . ",
    "  ....  . . .  ",
    " .+@+@.  . .   ",
    "..........  . .",
    ".@+@+@+@+@..   ",
    ".+@+@+@+@+. .  ",
    ".@+@+@+@+@.  . ",
    ".+@+@+@+@+.    ",
    ".@+@+@+@+@.    ",
    ".+@+@+@+@+.    ",
    "...........    "};

/* XPM */
static const char* const detailedview_xpm[]={
    "14 11 3 1",
    ". c None",
    "* c #000000",
    "a c #000099",
    ".****.***.***.",
    "..............",
    "aaaaaaaaaaaaaa",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***.",
    "..............",
    ".****.***.***."};

/* XPM */
static const char* const previewinfoview_xpm[]={
    "13 13 4 1",
    ". c #00007f",
    "a c black",
    "# c #cec6bd",
    "b c #000000",
    "..#####aaaaaa",
    ".#.#bb#a#####",
    "...####a#bbb#",
    "#######a#####",
    "#######a#bb##",
    "..#####a#####",
    ".#.#bb#a#bbb#",
    "...####a#####",
    "#######a#bb##",
    "#######a#####",
    "..#####a#bbb#",
    ".#.#bb#a#####",
    "...####aaaaaa"};

/* XPM */
static const char* const previewcontentsview_xpm[]={
    "14 13 5 1",
    ". c #00007f",
    "a c black",
    "c c #7f007f",
    "# c #cec6bd",
    "b c #000000",
    "..#####aaaaaaa",
    ".#.#bb#a#####a",
    "...####a#ccc#a",
    "#######a#ccc#a",
    "#######a#####a",
    "..#####a#bbb#a",
    ".#.#bb#a#####a",
    "...####a#bbb#a",
    "#######a#####a",
    "#######a#bbb#a",
    "..#####a#####a",
    ".#.#bb#a#####a",
    "...####aaaaaaa"};

/* XPM */
static const char* const mclistview_xpm[]={
    "15 11 4 1",
    "* c None",
    "b c #000000",
    ". c #000099",
    "a c #ffffff",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****",
    "***************",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****",
    "***************",
    "...*****...****",
    ".a.*bbb*.a.*bbb",
    "...*****...****"};

/* XPM */
static const char * const back_xpm [] = {
    "13 11 3 1",
    "a c #00ffff",
    "# c #000000",
    ". c None",
    ".....#.......",
    "....##.......",
    "...#a#.......",
    "..#aa########",
    ".#aaaaaaaaaa#",
    "#aaaaaaaaaaa#",
    ".#aaaaaaaaaa#",
    "..#aa########",
    "...#a#.......",
    "....##.......",
    ".....#......."};

static QPixmap * openFolderIcon = 0;
static QPixmap * closedFolderIcon = 0;
static QPixmap * detailViewIcon = 0;
static QPixmap * multiColumnListViewIcon = 0;
static QPixmap * cdToParentIcon = 0;
static QPixmap * newFolderIcon = 0;
static QPixmap * fifteenTransparentPixels = 0;
static QPixmap * symLinkDirIcon = 0;
static QPixmap * symLinkFileIcon = 0;
static QPixmap * fileIcon = 0;
static QPixmap * startCopyIcon = 0;
static QPixmap * endCopyIcon = 0;
static QPixmap * previewContentsViewIcon = 0;
static QPixmap * previewInfoViewIcon = 0;
static QPixmap *goBackIcon = 0;
static QFileIconProvider * fileIconProvider = 0;
static int lastWidth = 0;
static int lastHeight = 0;
static QString * workingDirectory = 0;

static bool bShowHiddenFiles = FALSE;
static int sortFilesBy = (int)QDir::Name;
static bool sortAscending = TRUE;
static bool detailViewMode = FALSE;

static QCleanupHandler<QPixmap> qfd_cleanup_pixmap;
static QCleanupHandler<QString> qfd_cleanup_string;

static QString toRootIfNotExists( const QString &path )
{
    if ( !path.isEmpty() )
        return path;

    const QFileInfoList *drives = QDir::drives();
    Q_ASSERT( drives && !drives->isEmpty() );
    return drives->getFirst()->filePath();
}

static bool isDirectoryMode( int m )
{
    return m == QFileDialog::Directory || m == QFileDialog::DirectoryOnly;
}

static void updateLastSize( QFileDialog *that )
{
    int extWidth = 0;
    int extHeight = 0;
    if ( that->extension() && that->extension()->isVisible() ) {
	if ( that->orientation() == Qt::Vertical )
	    extHeight = that->extension()->height();
	else
	    extWidth = that->extension()->width();
    }
    lastWidth = that->width() - extWidth;
    lastHeight = that->height() - extHeight;
}

// Don't remove the lines below!
//
// resolving the W methods manually is needed, because Windows 95 doesn't include
// these methods in Shell32.lib (not even stubs!), so you'd get an unresolved symbol
// when Qt calls getEsistingDirectory(), etc.
#if defined(Q_WS_WIN)

typedef UINT (WINAPI *PtrExtractIconEx)(LPCTSTR,int,HICON*,HICON*,UINT);
static PtrExtractIconEx ptrExtractIconEx = 0;

static void resolveLibs()
{
#ifndef Q_OS_TEMP
    static bool triedResolve = FALSE;

    if ( !triedResolve ) {
#ifdef QT_THREAD_SUPPORT
	// protect initialization
	QMutexLocker locker( qt_global_mutexpool ?
			     qt_global_mutexpool->get( &triedResolve ) : 0 );
	// check triedResolve again, since another thread may have already
	// done the initialization
	if ( triedResolve ) {
	    // another thread did initialize the security function pointers,
	    // so we shouldn't do it again.
	    return;
	}
#endif
	triedResolve = TRUE;
	if ( qt_winunicode ) {
	    QLibrary lib("shell32");
	    lib.setAutoUnload( FALSE );
	    ptrExtractIconEx = (PtrExtractIconEx) lib.resolve( "ExtractIconExW" );
	}
    }
#endif
}
#ifdef Q_OS_TEMP
#define PtrExtractIconEx ExtractIconEx
#endif

class QWindowsIconProvider : public QFileIconProvider
{
public:
    QWindowsIconProvider( QObject *parent=0, const char *name=0 );
    ~QWindowsIconProvider();

    const QPixmap * pixmap( const QFileInfo &fi );

private:
    QPixmap defaultFolder;
    QPixmap defaultFile;
    QPixmap defaultExe;
    QPixmap pix;
    int pixw, pixh;
    QMap< QString, QPixmap > cache;

};
#endif

static void makeVariables() {
    if ( !openFolderIcon ) {
	workingDirectory = new QString( ::toRootIfNotExists(QDir::currentDirPath()) );
        qfd_cleanup_string.add( &workingDirectory );

	openFolderIcon = new QPixmap( (const char **)open_xpm);
	qfd_cleanup_pixmap.add( &openFolderIcon );
	symLinkDirIcon = new QPixmap( (const char **)link_dir_xpm);
	qfd_cleanup_pixmap.add( &symLinkDirIcon );
	symLinkFileIcon = new QPixmap( (const char **)link_file_xpm);
	qfd_cleanup_pixmap.add( &symLinkFileIcon );
	fileIcon = new QPixmap( (const char **)file_xpm);
	qfd_cleanup_pixmap.add( &fileIcon );
	closedFolderIcon = new QPixmap( (const char **)closed_xpm);
	qfd_cleanup_pixmap.add( &closedFolderIcon );
	detailViewIcon = new QPixmap( (const char **)detailedview_xpm);
	qfd_cleanup_pixmap.add( &detailViewIcon );
	multiColumnListViewIcon = new QPixmap( (const char **)mclistview_xpm);
	qfd_cleanup_pixmap.add( &multiColumnListViewIcon );
	cdToParentIcon = new QPixmap( (const char **)cdtoparent_xpm);
	qfd_cleanup_pixmap.add( &cdToParentIcon );
	newFolderIcon = new QPixmap( (const char **)newfolder_xpm);
	qfd_cleanup_pixmap.add( &newFolderIcon );
	previewInfoViewIcon
	    = new QPixmap( (const char **)previewinfoview_xpm );
	qfd_cleanup_pixmap.add( &previewInfoViewIcon );
	previewContentsViewIcon
	    = new QPixmap( (const char **)previewcontentsview_xpm );
	qfd_cleanup_pixmap.add( &previewContentsViewIcon );
	startCopyIcon = new QPixmap( (const char **)start_xpm );
	qfd_cleanup_pixmap.add( &startCopyIcon );
	endCopyIcon = new QPixmap( (const char **)end_xpm );
	qfd_cleanup_pixmap.add( &endCopyIcon );
	goBackIcon = new QPixmap( (const char **)back_xpm );
	qfd_cleanup_pixmap.add( &goBackIcon );
	fifteenTransparentPixels = new QPixmap( closedFolderIcon->width(), 1 );
	qfd_cleanup_pixmap.add( &fifteenTransparentPixels );
	QBitmap m( fifteenTransparentPixels->width(), 1 );
	m.fill( Qt::color0 );
	fifteenTransparentPixels->setMask( m );
	bShowHiddenFiles = FALSE;
	sortFilesBy = (int)QDir::Name;
	detailViewMode = FALSE;
#if defined(Q_WS_WIN)
	if ( !fileIconProvider )
	    fileIconProvider = new QWindowsIconProvider( qApp );
#endif
    }
}

/******************************************************************
 *
 * Definitions of view classes
 *
 ******************************************************************/

class QRenameEdit : public QLineEdit
{
    Q_OBJECT

public:
    QRenameEdit( QWidget *parent )
	: QLineEdit( parent, "qt_rename_edit" ), doRenameAlreadyEmitted(FALSE)
    {
	connect( this, SIGNAL(returnPressed()), SLOT(slotReturnPressed()) );
    }

protected:
    void keyPressEvent( QKeyEvent *e );
    void focusOutEvent( QFocusEvent *e );

signals:
    void cancelRename();
    void doRename();

private slots:
    void slotReturnPressed();

private:
    bool doRenameAlreadyEmitted;
};

class QFileListBox : public QListBox
{
    friend class QFileDialog;

    Q_OBJECT

private:
    QFileListBox( QWidget *parent, QFileDialog *d );

    void clear();
    void show();
    void startRename( bool check = TRUE );
    void viewportMousePressEvent( QMouseEvent *e );
    void viewportMouseReleaseEvent( QMouseEvent *e );
    void viewportMouseDoubleClickEvent( QMouseEvent *e );
    void viewportMouseMoveEvent( QMouseEvent *e );
#ifndef QT_NO_DRAGANDDROP
    void viewportDragEnterEvent( QDragEnterEvent *e );
    void viewportDragMoveEvent( QDragMoveEvent *e );
    void viewportDragLeaveEvent( QDragLeaveEvent *e );
    void viewportDropEvent( QDropEvent *e );
    bool acceptDrop( const QPoint &pnt, QWidget *source );
    void setCurrentDropItem( const QPoint &pnt );
#endif
    void keyPressEvent( QKeyEvent *e );

private slots:
    void rename();
    void cancelRename();
    void doubleClickTimeout();
    void changeDirDuringDrag();
    void dragObjDestroyed();
    void contentsMoved( int, int );

private:
    QRenameEdit *lined;
    QFileDialog *filedialog;
    bool renaming;
    QTimer* renameTimer;
    QListBoxItem *renameItem, *dragItem;
    QPoint pressPos, oldDragPos;
    bool mousePressed;
    int urls;
    QString startDragDir;
    QListBoxItem *currDropItem;
    QTimer *changeDirTimer;
    bool firstMousePressEvent;
    QUrlOperator startDragUrl;

};


class QFileDialogQFileListView : public QListView
{
    Q_OBJECT

public:
    QFileDialogQFileListView( QWidget *parent, QFileDialog *d );

    void clear();
    void startRename( bool check = TRUE );
    void setSorting( int column, bool increasing = TRUE );

    QRenameEdit *lined;
    bool renaming;
    QListViewItem *renameItem;

private:
    void viewportMousePressEvent( QMouseEvent *e );
    void viewportMouseDoubleClickEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void viewportMouseReleaseEvent( QMouseEvent *e );
    void viewportMouseMoveEvent( QMouseEvent *e );
#ifndef QT_NO_DRAGANDDROP
    void viewportDragEnterEvent( QDragEnterEvent *e );
    void viewportDragMoveEvent( QDragMoveEvent *e );
    void viewportDragLeaveEvent( QDragLeaveEvent *e );
    void viewportDropEvent( QDropEvent *e );
    bool acceptDrop( const QPoint &pnt, QWidget *source );
    void setCurrentDropItem( const QPoint &pnt );
#endif

private slots:
    void rename();
    void cancelRename();
    void changeSortColumn2( int column );
    void doubleClickTimeout();
    void changeDirDuringDrag();
    void dragObjDestroyed();
    void contentsMoved( int, int );

private:
    QFileDialog *filedialog;
    QTimer* renameTimer;
    QPoint pressPos, oldDragPos;
    bool mousePressed;
    int urls;
    QString startDragDir;
    QListViewItem *currDropItem, *dragItem;
    QTimer *changeDirTimer;
    bool firstMousePressEvent;
    bool ascending;
    int sortcolumn;
    QUrlOperator startDragUrl;

};

/****************************************************************************
 *
 * Classes for copy progress dialog
 *
 ****************************************************************************/

class QFDProgressAnimation : public QWidget
{
    Q_OBJECT

public:
    QFDProgressAnimation( QWidget *parent );
    void start();

private slots:
    void next();

protected:
    void paintEvent( QPaintEvent *e );

private:
    int step;
    QTimer *timer;

};

QFDProgressAnimation::QFDProgressAnimation( QWidget *parent )
    : QWidget( parent, "qt_progressanimation" )
{
    setFixedSize( 300, 50 );
    step = -1;
    next();
    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ),
	     this, SLOT( next() ) );
}

void QFDProgressAnimation::start()
{
    timer->start( 150, FALSE );
}

void QFDProgressAnimation::next()
{
    ++step;
    if ( step > 10 )
	step = 0;
    repaint();
}

void QFDProgressAnimation::paintEvent( QPaintEvent * )
{
    erase();

    QPainter p;
    p.begin( this );
    if ( step == 0 ) {
	p.drawPixmap( 5, ( height() - startCopyIcon->height() ) / 2,
		      *startCopyIcon );
	p.drawPixmap( width() - 5 - openFolderIcon->width(),
		      ( height() - openFolderIcon->height() ) / 2 , *openFolderIcon );
    } else if ( step == 10 ) {
	p.drawPixmap( 5, ( height() - openFolderIcon->height() ) / 2,
		      *openFolderIcon );
	p.drawPixmap( width() - 5 - endCopyIcon->width(),
		      ( height() - endCopyIcon->height() ) / 2 , *endCopyIcon );
    } else {
	p.drawPixmap( 5, ( height() - openFolderIcon->height() ) / 2,
		      *openFolderIcon );
	p.drawPixmap( width() - 5 - openFolderIcon->width(),
		      ( height() - openFolderIcon->height() ) / 2 , *openFolderIcon );
	int x = 10 + openFolderIcon->width();
	int w = width() - 2 * x;
	int s = w / 9;
	p.drawPixmap( x + s * step, ( height() - fileIcon->height() ) / 2 - fileIcon->height(),
		      *fileIcon );
    }
}


class QFDProgressDialog : public QDialog
{
    Q_OBJECT

public:
    QFDProgressDialog( QWidget *parent, const QString &fn, int steps );

    void setReadProgress( int p );
    void setWriteProgress( int p );
    void setWriteLabel( const QString &s );

signals:
    void cancelled();

private:
    QProgressBar *readBar;
    QProgressBar *writeBar;
    QLabel *writeLabel;
    QFDProgressAnimation *animation;

};

QFDProgressDialog::QFDProgressDialog( QWidget *parent, const QString &fn, int steps )
    : QDialog( parent, "", TRUE )
{
#ifndef QT_NO_WIDGET_TOPEXTRA
    setCaption( QFileDialog::tr( "Copy or Move a File" ) );
#endif
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( 5 );
    layout->setMargin( 5 );

    animation = new QFDProgressAnimation( this );
    layout->addWidget( animation );

    layout->addWidget( new QLabel( QFileDialog::tr( "Read: %1" ).arg( fn ),
		       this, "qt_read_lbl" ) );
    readBar = new QProgressBar( steps, this, "qt_readbar" );
    readBar->reset();
    readBar->setProgress( 0 );
    layout->addWidget( readBar );
    writeLabel = new QLabel( QFileDialog::tr( "Write: %1" ).arg( QString::null ),
			     this, "qt_write_lbl" );
    layout->addWidget( writeLabel );
    writeBar = new QProgressBar( steps, this, "qt_writebar" );
    writeBar->reset();
    writeBar->setProgress( 0 );
    layout->addWidget( writeBar );

    QPushButton *b = new QPushButton( QFileDialog::tr( "Cancel" ), this,
				      "qt_cancel_btn" );
    b->setFixedSize( b->sizeHint() );
    layout->addWidget( b );
    connect( b, SIGNAL( clicked() ),
	     this, SIGNAL( cancelled() ) );

    animation->start();
}

void QFDProgressDialog::setReadProgress( int p )
{
    readBar->setProgress( p );
}

void QFDProgressDialog::setWriteProgress( int p )
{
    writeBar->setProgress( p );
}

void QFDProgressDialog::setWriteLabel( const QString &s )
{
    writeLabel->setText( QFileDialog::tr( "Write: %1" ).arg( s ) );
}

/************************************************************************
 *
 * Private QFileDialog members
 *
 ************************************************************************/

class QFileDialogPrivate {
public:
    ~QFileDialogPrivate();

    QStringList history;

    bool geometryDirty;
    QComboBox * paths;
    QComboBox * types;
    QLabel * pathL;
    QLabel * fileL;
    QLabel * typeL;

    QVBoxLayout * topLevelLayout;
    QHBoxLayout *buttonLayout, *leftLayout, *rightLayout;
    QPtrList<QHBoxLayout> extraWidgetsLayouts;
    QPtrList<QLabel> extraLabels;
    QPtrList<QWidget> extraWidgets;
    QPtrList<QWidget> extraButtons;
    QPtrList<QButton> toolButtons;

    QWidgetStack * stack;

    QToolButton * cdToParent, *newFolder, * detailView, * mcView,
	*previewInfo, *previewContents, *goBack;
    QButtonGroup * modeButtons;

    QString currentFileName;
    QListViewItem *last;

    QListBoxItem *lastEFSelected;

    struct File: public QListViewItem {
	File( QFileDialogPrivate * dlgp,
	      const QUrlInfo * fi, QListViewItem * parent )
	    : QListViewItem( parent, dlgp->last ), info( *fi ), d(dlgp), i( 0 ), hasMimePixmap( FALSE )
	{ setup(); dlgp->last = this; }
	File( QFileDialogPrivate * dlgp,
	      const QUrlInfo * fi, QListView * parent )
	    : QListViewItem( parent, dlgp->last ), info( *fi ), d(dlgp), i( 0 ), hasMimePixmap( FALSE )
	{ setup(); dlgp->last = this; }
	File( QFileDialogPrivate * dlgp,
	      const QUrlInfo * fi, QListView * parent, QListViewItem * after )
	    : QListViewItem( parent, after ), info( *fi ), d(dlgp), i( 0 ), hasMimePixmap( FALSE )
	{ setup(); if ( !nextSibling() ) dlgp->last = this; }
	~File();

	QString text( int column ) const;
	const QPixmap * pixmap( int ) const;

	QUrlInfo info;
	QFileDialogPrivate * d;
	QListBoxItem *i;
	bool hasMimePixmap;
    };

    class MCItem: public QListBoxItem {
    public:
	MCItem( QListBox *, QListViewItem * item );
	MCItem( QListBox *, QListViewItem * item, QListBoxItem *after );
	QString text() const;
	const QPixmap *pixmap() const;
	int height( const QListBox * ) const;
	int width( const QListBox * ) const;
	void paint( QPainter * );
	QListViewItem * i;
    };

    class UrlInfoList : public QPtrList<QUrlInfo> {
    public:
	UrlInfoList() { setAutoDelete( TRUE ); }
	int compareItems( QPtrCollection::Item n1, QPtrCollection::Item n2 ) {
	    if ( !n1 || !n2 )
		return 0;

	    QUrlInfo *i1 = ( QUrlInfo *)n1;
	    QUrlInfo *i2 = ( QUrlInfo *)n2;

	    if ( i1->isDir() && !i2->isDir() )
		return -1;
	    if ( !i1->isDir() && i2->isDir() )
		return 1;

	    if ( i1->name() == ".." )
		return -1;
	    if ( i2->name() == ".." )
		return 1;

	    if ( sortFilesBy == QDir::Name ) {
#if defined(Q_OS_WIN32)
		QString name1 = i1->name().lower();
		QString name2 = i2->name().lower();
		return name1.localeAwareCompare( name2 );
#else
		QString name1 = i1->name();
		QString name2 = i2->name();
		return name1.localeAwareCompare( name2 );
#endif
	    }
	    if ( QUrlInfo::equal( *i1, *i2, sortFilesBy ) )
		return 0;
	    else if ( QUrlInfo::greaterThan( *i1, *i2, sortFilesBy ) )
		return 1;
	    else if ( QUrlInfo::lessThan( *i1, *i2, sortFilesBy ) )
		return -1;
	    // can't happen...
	    return 0;
	}
	QUrlInfo *operator[]( int i ) {
	    return at( i );
	}
    };

    UrlInfoList sortedList;
    QPtrList<File> pendingItems;

    QFileListBox * moreFiles;

    QFileDialog::Mode mode;

    QString rw;
    QString ro;
    QString wo;
    QString inaccessible;

    QString symLinkToFile;
    QString file;
    QString symLinkToDir;
    QString dir;
    QString symLinkToSpecial;
    QString special;
    QWidgetStack *preview;
    bool infoPreview, contentsPreview;
    QSplitter *splitter;
    QUrlOperator url, oldUrl;
    QWidget *infoPreviewWidget, *contentsPreviewWidget;
    QFilePreview *infoPreviewer, *contentsPreviewer;
    bool hadDotDot;

    bool ignoreNextKeyPress;
    // ignores the next refresh operation in case the user forced a selection
    bool ignoreNextRefresh;
    QFDProgressDialog *progressDia;
    bool checkForFilter;
    bool ignoreStop;

    QTimer *mimeTypeTimer;
    const QNetworkOperation *currListChildren;

    // this is similar to QUrl::encode but does encode "*" and
    // doesn't encode whitespaces
    static QString encodeFileName( const QString& fName ) {

	QString newStr;
	QCString cName = fName.utf8();
	const QCString sChars(
#ifdef Q_WS_WIN
	    "#%"
#else
	    "<>#@\"&%$:,;?={}|^~[]\'`\\*"
#endif
	    );

	int len = cName.length();
	if ( !len )
	    return QString::null;
	for ( int i = 0; i < len ;++i ) {
	    uchar inCh = (uchar)cName[ i ];
	    if ( inCh >= 128 || sChars.contains(inCh) )
	    {
		newStr += QChar( '%' );
		ushort c = inCh / 16;
		c += c > 9 ? 'A' - 10 : '0';
		newStr += (char)c;
		c = inCh % 16;
		c += c > 9 ? 'A' - 10 : '0';
		newStr += (char)c;
	    } else {
		newStr += (char)inCh;
	    }
	}
	return newStr;
    }

    static bool fileExists( const QUrlOperator &url, const QString& name )
    {
	QUrl u( url, QFileDialogPrivate::encodeFileName(name) );
	if ( u.isLocalFile() ) {
	    QFileInfo f( u.path() );
	    return f.exists();
	} else {
	    QNetworkProtocol *p = QNetworkProtocol::getNetworkProtocol( url.protocol() );
	    if ( p && (p->supportedOperations()&QNetworkProtocol::OpListChildren) ) {
		QUrlInfo ui( url, name );
		return ui.isValid();
	    }
	}
	return TRUE;
    }

#ifndef Q_NO_CURSOR
    bool cursorOverride; // Remember if the cursor was overridden or not.
#endif
#ifdef Q_WS_WIN
    int oldPermissionLookup;
#endif
};

QFileDialogPrivate::~QFileDialogPrivate()
{
    delete modeButtons;
}



/************************************************************************
 *
 * Internal class QRenameEdit
 *
 ************************************************************************/

void QRenameEdit::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Escape )
	emit cancelRename();
    else
	QLineEdit::keyPressEvent( e );
    e->accept();
}

void QRenameEdit::focusOutEvent( QFocusEvent * )
{
    if ( !doRenameAlreadyEmitted )
	emit doRename();
    else
	doRenameAlreadyEmitted = FALSE;
}

void QRenameEdit::slotReturnPressed()
{
    doRenameAlreadyEmitted = TRUE;
    emit doRename();
}

/************************************************************************
 *
 * Internal class QFileListBox
 *
 ************************************************************************/

QFileListBox::QFileListBox( QWidget *parent, QFileDialog *dlg )
    : QListBox( parent, "filelistbox" ), filedialog( dlg ),
      renaming( FALSE ), renameItem( 0 ), mousePressed( FALSE ),
      firstMousePressEvent( TRUE )
{
    changeDirTimer = new QTimer( this );
    QVBox *box = new QVBox( viewport(), "qt_vbox" );
    box->setFrameStyle( QFrame::Box | QFrame::Plain );
    lined = new QRenameEdit( box );
    lined->setFixedHeight( lined->sizeHint().height() );
    box->hide();
    box->setBackgroundMode( PaletteBase );
    renameTimer = new QTimer( this );
    connect( lined, SIGNAL( doRename() ),
	     this, SLOT (rename() ) );
    connect( lined, SIGNAL( cancelRename() ),
	     this, SLOT( cancelRename() ) );
    connect( renameTimer, SIGNAL( timeout() ),
	     this, SLOT( doubleClickTimeout() ) );
    connect( changeDirTimer, SIGNAL( timeout() ),
	     this, SLOT( changeDirDuringDrag() ) );
    connect( this, SIGNAL( contentsMoving(int,int) ),
	     this, SLOT( contentsMoved(int,int) ) );
    viewport()->setAcceptDrops( TRUE );
    dragItem = 0;
}

void QFileListBox::show()
{
    setBackgroundMode( PaletteBase );
    viewport()->setBackgroundMode( PaletteBase );
    QListBox::show();
}

void QFileListBox::keyPressEvent( QKeyEvent *e )
{
    if ( ( e->key() == Key_Enter ||
	   e->key() == Key_Return ) &&
	 renaming )
	return;

    QString keyPressed = ((QKeyEvent *)e)->text().lower();
    QChar keyChar = keyPressed[0];
    bool handled = false;
    if ( keyChar.isLetterOrNumber() ) {
	QListBoxItem * i = 0;
	if ( currentItem() )
	    i = item( currentItem() );
	else
	    i = firstItem();
	if ( i->next() )
	    i = i->next();
	else
	    i = firstItem();
	while ( i != item( currentItem() ) ) {
	    QString it = text( index( i ) );
	    if ( it[0].lower() == keyChar ) {
	        clearSelection();
	        setCurrentItem( i );
	        handled = true;
	        e->accept();
            } else {
	        if ( i->next() )
	            i = i->next();
	        else
	            i = firstItem();
	    }
	}
    }
    cancelRename();
    if (!handled){
        QListBox::keyPressEvent( e );
    }
}

void QFileListBox::viewportMousePressEvent( QMouseEvent *e )
{
    pressPos = e->pos();
    mousePressed = FALSE;

    bool didRename = renaming;

    cancelRename();
    if ( !hasFocus() && !viewport()->hasFocus() )
	setFocus();

    if ( e->button() != LeftButton ) {
	QListBox::viewportMousePressEvent( e );
	firstMousePressEvent = FALSE;
	return;
    }

    int i = currentItem();
    bool wasSelected = FALSE;
    if ( i != -1 )
	wasSelected = item( i )->isSelected();
    QListBox::mousePressEvent( e );

    QFileDialogPrivate::MCItem *i1 = (QFileDialogPrivate::MCItem*)item( currentItem() );
    if ( i1 )
	mousePressed =  ( !( (QFileDialogPrivate::File*)i1->i )->info.isDir() )
			|| ( filedialog->mode() == QFileDialog::Directory ) || ( filedialog->mode() == QFileDialog::DirectoryOnly );

    if ( itemAt( e->pos() ) != item( i ) ) {
	firstMousePressEvent = FALSE;
	return;
    }

     if ( !firstMousePressEvent && !didRename && i == currentItem() && currentItem() != -1 &&
	  wasSelected && QUrlInfo( filedialog->d->url, "." ).isWritable() && item( currentItem() )->text() != ".." ) {
	renameTimer->start( QApplication::doubleClickInterval(), TRUE );
	renameItem = item( i );
    }

    firstMousePressEvent = FALSE;
}

void QFileListBox::viewportMouseReleaseEvent( QMouseEvent *e )
{
    dragItem = 0;
    QListBox::viewportMouseReleaseEvent( e );
    mousePressed = FALSE;
}

void QFileListBox::viewportMouseDoubleClickEvent( QMouseEvent *e )
{
    renameTimer->stop();
    QListBox::viewportMouseDoubleClickEvent( e );
}

void QFileListBox::viewportMouseMoveEvent( QMouseEvent *e )
{
    if ( !dragItem )
	dragItem = itemAt( e->pos() );
    renameTimer->stop();
#ifndef QT_NO_DRAGANDDROP
    if (  ( pressPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() && mousePressed ) {
	QListBoxItem *item = dragItem;
	dragItem = 0;
	if ( item ) {
	    if ( !itemRect( item ).contains( e->pos() ) )
		return;
	    QUriDrag* drag = new QUriDrag( viewport() );
	    QStringList files;
	    if ( filedialog->mode() == QFileDialog::ExistingFiles )
		files = filedialog->selectedFiles();
	    else
		files = filedialog->selectedFile();
	    drag->setFileNames( files );

	    if ( lined->parentWidget()->isVisible() )
		cancelRename();

	    connect( drag, SIGNAL( destroyed() ),
		     this, SLOT( dragObjDestroyed() ) );
	    drag->drag();

	    mousePressed = FALSE;
	}
    } else
#endif
    {
	QListBox::viewportMouseMoveEvent( e );
    }

}

void QFileListBox::dragObjDestroyed()
{
#ifndef QT_NO_DRAGANDDROP
    //#######
    //filedialog->rereadDir();
#endif
}

#ifndef QT_NO_DRAGANDDROP
void QFileListBox::viewportDragEnterEvent( QDragEnterEvent *e )
{
    startDragUrl = filedialog->d->url;
    startDragDir = filedialog->dirPath();
    currDropItem = 0;

    if ( !QUriDrag::canDecode( e ) ) {
	e->ignore();
	return;
    }

    QStringList l;
    QUriDrag::decodeLocalFiles( e, l );
    urls = (int)l.count();

    if ( acceptDrop( e->pos(), e->source() ) ) {
	e->accept();
	setCurrentDropItem( e->pos() );
    } else {
	e->ignore();
	setCurrentDropItem( QPoint( -1, -1 ) );
    }

    oldDragPos = e->pos();
}

void QFileListBox::viewportDragMoveEvent( QDragMoveEvent *e )
{
    if ( acceptDrop( e->pos(), e->source() ) ) {
	switch ( e->action() ) {
	case QDropEvent::Copy:
	    e->acceptAction();
	    break;
	case QDropEvent::Move:
	    e->acceptAction();
	    break;
	case QDropEvent::Link:
	    break;
	default:
	    break;
	}
	if ( oldDragPos != e->pos() )
	    setCurrentDropItem( e->pos() );
    } else {
	changeDirTimer->stop();
	e->ignore();
	setCurrentDropItem( QPoint( -1, -1 ) );
    }

    oldDragPos = e->pos();
}

void QFileListBox::viewportDragLeaveEvent( QDragLeaveEvent * )
{
    changeDirTimer->stop();
    setCurrentDropItem( QPoint( -1, -1 ) );
//########
//     if ( startDragDir != filedialog->d->url )
//	filedialog->setUrl( startDragUrl );
}

void QFileListBox::viewportDropEvent( QDropEvent *e )
{
    changeDirTimer->stop();

    if ( !QUriDrag::canDecode( e ) ) {
	e->ignore();
	return;
    }

    uint i;
    QStrList l;
    QUriDrag::decode( e, l );

    bool move = e->action() == QDropEvent::Move;
//     bool supportAction = move || e->action() == QDropEvent::Copy;

    QUrlOperator dest;
    if ( currDropItem )
	dest = QUrlOperator( filedialog->d->url, QFileDialogPrivate::encodeFileName( currDropItem->text() ) );
    else
	dest = filedialog->d->url;
    QStringList lst;
    for ( i = 0; i < l.count(); ++i ) {
	lst << l.at( i );
    }

    // make sure that we can write to the destination before performing the action
    bool exists = false;
    QString name = (currDropItem ? QFileDialogPrivate::encodeFileName(currDropItem->text()) : QString("."));
    QUrlInfo info(filedialog->d->url, name);
    for ( i = 0; i < lst.count(); ++i ) {
        int slash = lst[i].findRev('/');
        QString filename = lst[i].right(lst[i].length() - slash - 1);
        exists = exists || QFileDialogPrivate::fileExists( dest, filename);
    }
    if (info.isWritable() && !exists)
        filedialog->d->url.copy( lst, dest, move );

    // ##### what is supportAction for?
    e->acceptAction();
    currDropItem = 0;
}

bool QFileListBox::acceptDrop( const QPoint &pnt, QWidget *source )
{
    QListBoxItem *item = itemAt( pnt );
    if ( !item || item && !itemRect( item ).contains( pnt ) ) {
	if ( source == viewport() && startDragDir == filedialog->dirPath() )
	    return FALSE;
	return TRUE;
    }

    QUrlInfo fi( filedialog->d->url, item->text() );

    if ( fi.isDir() && itemRect( item ).contains( pnt ) )
	return TRUE;
    return FALSE;
}

void QFileListBox::setCurrentDropItem( const QPoint &pnt )
{
    changeDirTimer->stop();

    QListBoxItem *item = 0;
    if ( pnt != QPoint( -1, -1 ) )
	item = itemAt( pnt );
    if ( item && !QUrlInfo( filedialog->d->url, item->text() ).isDir() )
	item = 0;
    if ( item && !itemRect( item ).contains( pnt ) )
	item = 0;

    currDropItem = item;
    if ( currDropItem )
	setCurrentItem( currDropItem );
    changeDirTimer->start( 750 );
}
#endif // QT_NO_DRAGANDDROP

void QFileListBox::changeDirDuringDrag()
{
#ifndef QT_NO_DRAGANDDROP
    if ( !currDropItem )
	return;
    changeDirTimer->stop();
    QUrl u( filedialog->d->url, QFileDialogPrivate::encodeFileName(currDropItem->text()) );
    filedialog->setDir( u );
    currDropItem = 0;
#endif
}

void QFileListBox::doubleClickTimeout()
{
    startRename();
    renameTimer->stop();
}

void QFileListBox::startRename( bool check )
{
    if ( check && ( !renameItem || renameItem != item( currentItem() ) ) )
	return;

    int i = currentItem();
    setSelected( i, TRUE );
    QRect r = itemRect( item( i ) );
    int bdr = item( i )->pixmap() ?
	      item( i )->pixmap()->width() : 16;
    int x = r.x() + bdr;
    int y = r.y();
    int w = item( i )->width( this ) - bdr;
    int h = QMAX( lined->height() + 2, r.height() );
    y = y + r.height() / 2 - h / 2;

    lined->parentWidget()->setGeometry( x, y, w + 6, h );
    lined->setFocus();
    lined->setText( item( i )->text() );
    lined->selectAll();
    lined->setFrame( FALSE );
    lined->parentWidget()->show();
    viewport()->setFocusProxy( lined );
    renaming = TRUE;
}

void QFileListBox::clear()
{
    cancelRename();
    QListBox::clear();
}

void QFileListBox::rename()
{
    if ( !lined->text().isEmpty() ) {
	QString file = currentText();

	if ( lined->text() != file )
	    filedialog->d->url.rename( file, lined->text() );
    }
    cancelRename();
}

void QFileListBox::cancelRename()
{
    renameItem = 0;
    lined->parentWidget()->hide();
    viewport()->setFocusProxy( this );
    renaming = FALSE;
    updateItem( currentItem() );
    if ( lined->hasFocus() )
	viewport()->setFocus();
}

void QFileListBox::contentsMoved( int, int )
{
    changeDirTimer->stop();
#ifndef QT_NO_DRAGANDDROP
    setCurrentDropItem( QPoint( -1, -1 ) );
#endif
}

/************************************************************************
 *
 * Internal class QFileListView
 *
 ************************************************************************/

QFileDialogQFileListView::QFileDialogQFileListView( QWidget *parent, QFileDialog *dlg )
    : QListView( parent, "qt_filedlg_listview" ), renaming( FALSE ), renameItem( 0 ),
    filedialog( dlg ), mousePressed( FALSE ),
    firstMousePressEvent( TRUE )
{
    changeDirTimer = new QTimer( this );
    QVBox *box = new QVBox( viewport(), "qt_vbox" );
    box->setFrameStyle( QFrame::Box | QFrame::Plain );
    lined = new QRenameEdit( box );
    lined->setFixedHeight( lined->sizeHint().height() );
    box->hide();
    box->setBackgroundMode( PaletteBase );
    renameTimer = new QTimer( this );
    connect( lined, SIGNAL( doRename() ),
	     this, SLOT (rename() ) );
    connect( lined, SIGNAL( cancelRename() ),
	     this, SLOT( cancelRename() ) );
    header()->setMovingEnabled( FALSE );
    connect( renameTimer, SIGNAL( timeout() ),
	     this, SLOT( doubleClickTimeout() ) );
    connect( changeDirTimer, SIGNAL( timeout() ),
	     this, SLOT( changeDirDuringDrag() ) );
    disconnect( header(), SIGNAL( sectionClicked(int) ),
		this, SLOT( changeSortColumn(int) ) );
    connect( header(), SIGNAL( sectionClicked(int) ),
	     this, SLOT( changeSortColumn2(int) ) );
    connect( this, SIGNAL( contentsMoving(int,int) ),
	     this, SLOT( contentsMoved(int,int) ) );

    viewport()->setAcceptDrops( TRUE );
    sortcolumn = 0;
    ascending = TRUE;
    dragItem = 0;
}

void QFileDialogQFileListView::setSorting( int column, bool increasing )
{
    if ( column == -1 ) {
	QListView::setSorting( column, increasing );
	return;
    }

    sortAscending = ascending = increasing;
    sortcolumn = column;
    switch ( column ) {
    case 0:
	sortFilesBy = QDir::Name;
	break;
    case 1:
	sortFilesBy = QDir::Size;
	break;
    case 3:
	sortFilesBy = QDir::Time;
	break;
    default:
	sortFilesBy = QDir::Name; // #### ???
	break;
    }

    filedialog->resortDir();
}

void QFileDialogQFileListView::changeSortColumn2( int column )
{
    int lcol = header()->mapToLogical( column );
    setSorting( lcol, sortcolumn == lcol ? !ascending : TRUE );
}

void QFileDialogQFileListView::keyPressEvent( QKeyEvent *e )
{
    if ( ( e->key() == Key_Enter ||
	   e->key() == Key_Return ) &&
	 renaming )
	return;

    QString keyPressed = e->text().lower();
    QChar keyChar = keyPressed[0];
    if ( keyChar.isLetterOrNumber() ) {
	QListViewItem * i = 0;
	if ( currentItem() )
	i = currentItem();
	else
	i = firstChild();
	if ( i->nextSibling() )
	i = i->nextSibling();
	else
	i = firstChild();
	while ( i != currentItem() ) {
	    QString it = i->text(0);
	    if ( it[0].lower() == keyChar ) {
	    clearSelection();
	    ensureItemVisible( i );
	    setCurrentItem( i );
	    } else {
	    if ( i->nextSibling() )
	    i = i->nextSibling();
	    else
	    i = firstChild();
	    }
	}
	return;
    }

    cancelRename();
    QListView::keyPressEvent( e );
}

void QFileDialogQFileListView::viewportMousePressEvent( QMouseEvent *e )
{
    pressPos = e->pos();
    mousePressed = FALSE;

    bool didRename = renaming;
    cancelRename();
    if ( !hasFocus() && !viewport()->hasFocus() )
	setFocus();

    if ( e->button() != LeftButton ) {
	QListView::viewportMousePressEvent( e );
	firstMousePressEvent = FALSE;
	return;
    }

    QListViewItem *i = currentItem();
    QListView::viewportMousePressEvent( e );

    QFileDialogPrivate::File *i1 = (QFileDialogPrivate::File*)currentItem();
    if ( i1 )
	mousePressed = !i1->info.isDir() || ( filedialog->mode() == QFileDialog::Directory ) || ( filedialog->mode() == QFileDialog::DirectoryOnly );


    if ( itemAt( e->pos() ) != i ||
	 e->x() + contentsX() > columnWidth( 0 ) ) {
	firstMousePressEvent = FALSE;
	return;
    }

    if ( !firstMousePressEvent && !didRename && i == currentItem() && currentItem() &&
	 QUrlInfo( filedialog->d->url, "." ).isWritable() && currentItem()->text( 0 ) != ".." ) {
	renameTimer->start( QApplication::doubleClickInterval(), TRUE );
	renameItem = currentItem();
    }

    firstMousePressEvent = FALSE;
}

void QFileDialogQFileListView::viewportMouseDoubleClickEvent( QMouseEvent *e )
{
    renameTimer->stop();
    QListView::viewportMouseDoubleClickEvent( e );
}

void QFileDialogQFileListView::viewportMouseReleaseEvent( QMouseEvent *e )
{
    QListView::viewportMouseReleaseEvent( e );
    mousePressed = FALSE;
    dragItem = 0;
}

void QFileDialogQFileListView::viewportMouseMoveEvent( QMouseEvent *e )
{
    renameTimer->stop();
    if ( !dragItem )
	dragItem = itemAt( e->pos() );
#ifndef QT_NO_DRAGANDDROP
    if (  ( pressPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() && mousePressed ) {
	QListViewItem *item = dragItem;
	dragItem = 0;
	if ( item ) {
	    QUriDrag* drag = new QUriDrag( viewport() );
	    QStringList files;
	    if ( filedialog->mode() == QFileDialog::ExistingFiles )
		files = filedialog->selectedFiles();
	    else
		files = filedialog->selectedFile();
	    drag->setFileNames( files );

	    if ( lined->isVisible() )
		cancelRename();

	    connect( drag, SIGNAL( destroyed() ),
		     this, SLOT( dragObjDestroyed() ) );
	    drag->drag();

	    mousePressed = FALSE;
	}
    }
#endif
}

void QFileDialogQFileListView::dragObjDestroyed()
{
#ifndef QT_NO_DRAGANDDROP
    //######
    //filedialog->rereadDir();
#endif
}

#ifndef QT_NO_DRAGANDDROP
void QFileDialogQFileListView::viewportDragEnterEvent( QDragEnterEvent *e )
{
    startDragUrl = filedialog->d->url;
    startDragDir = filedialog->dirPath();
    currDropItem = 0;

    if ( !QUriDrag::canDecode( e ) ) {
	e->ignore();
	return;
    }

    QStringList l;
    QUriDrag::decodeLocalFiles( e, l );
    urls = (int)l.count();

    if ( acceptDrop( e->pos(), e->source() ) ) {
	e->accept();
	setCurrentDropItem( e->pos() );
    } else {
	e->ignore();
	setCurrentDropItem( QPoint( -1, -1 ) );
    }

    oldDragPos = e->pos();
}

void QFileDialogQFileListView::viewportDragMoveEvent( QDragMoveEvent *e )
{
    if ( acceptDrop( e->pos(), e->source() ) ) {
	if ( oldDragPos != e->pos() )
	    setCurrentDropItem( e->pos() );
	switch ( e->action() ) {
	case QDropEvent::Copy:
	    e->acceptAction();
	    break;
	case QDropEvent::Move:
	    e->acceptAction();
	    break;
	case QDropEvent::Link:
	    break;
	default:
	    break;
	}
    } else {
	changeDirTimer->stop();
	e->ignore();
	setCurrentDropItem( QPoint( -1, -1 ) );
    }

    oldDragPos = e->pos();
}

void QFileDialogQFileListView::viewportDragLeaveEvent( QDragLeaveEvent * )
{
    changeDirTimer->stop();
    setCurrentDropItem( QPoint( -1, -1 ) );
//########
//     if ( startDragDir != filedialog->d->url )
//	filedialog->setUrl( startDragUrl );
}

void QFileDialogQFileListView::viewportDropEvent( QDropEvent *e )
{
    changeDirTimer->stop();

    if ( !QUriDrag::canDecode( e ) ) {
	e->ignore();
	return;
    }

    QStringList l;
    QUriDrag::decodeToUnicodeUris( e, l );

    bool move = e->action() == QDropEvent::Move;
//     bool supportAction = move || e->action() == QDropEvent::Copy;

    QUrlOperator dest;
    if ( currDropItem )
	dest = QUrlOperator( filedialog->d->url, QFileDialogPrivate::encodeFileName( currDropItem->text( 0 ) ) );
    else
	dest = filedialog->d->url;
    
    // make sure that we can write to the destination before performing the action
    bool exists = false;
    QString name = (currDropItem ? QFileDialogPrivate::encodeFileName(currDropItem->text(0)) : QString("."));
    QUrlInfo info(filedialog->d->url, name);
    for (uint i = 0; i < l.count(); ++i) {
        int slash = l[i].findRev('/');
        QString filename = l[i].right(l[i].length() - slash - 1);
        exists = exists || QFileDialogPrivate::fileExists(dest, filename);
    }
    if (info.isWritable() && !exists)
        filedialog->d->url.copy( l, dest, move );

    // ##### what is supportAction for?
    e->acceptAction();
    currDropItem = 0;
}

bool QFileDialogQFileListView::acceptDrop( const QPoint &pnt, QWidget *source )
{
    QListViewItem *item = itemAt( pnt );
    if ( !item || item && !itemRect( item ).contains( pnt ) ) {
	if ( source == viewport() && startDragDir == filedialog->dirPath() )
	    return FALSE;
	return TRUE;
    }

    QUrlInfo fi( filedialog->d->url, item->text( 0 ) );

    if ( fi.isDir() && itemRect( item ).contains( pnt ) )
	return TRUE;
    return FALSE;
}

void QFileDialogQFileListView::setCurrentDropItem( const QPoint &pnt )
{
    changeDirTimer->stop();

    QListViewItem *item = itemAt( pnt );
    if ( pnt == QPoint( -1, -1 ) )
	item = 0;
    if ( item && !QUrlInfo( filedialog->d->url, item->text( 0 ) ).isDir() )
	item = 0;

    if ( item && !itemRect( item ).contains( pnt ) )
	item = 0;

    currDropItem = item;

    if ( currDropItem )
	setCurrentItem( currDropItem );

    changeDirTimer->start( 750 );
}
#endif // QT_NO_DRAGANDDROP

void QFileDialogQFileListView::changeDirDuringDrag()
{
#ifndef QT_NO_DRAGANDDROP
    if ( !currDropItem )
	return;
    changeDirTimer->stop();
    QUrl u( filedialog->d->url, QFileDialogPrivate::encodeFileName(currDropItem->text( 0 ) ) );
    filedialog->setDir( u );
    currDropItem = 0;
#endif // QT_NO_DRAGANDDROP
}


void QFileDialogQFileListView::doubleClickTimeout()
{
    startRename();
    renameTimer->stop();
}

void QFileDialogQFileListView::startRename( bool check )
{
    if ( check && ( !renameItem || renameItem != currentItem() ) )
	return;

    QListViewItem *i = currentItem();
    setSelected( i, TRUE );

    QRect r = itemRect( i );
    int bdr = i->pixmap( 0 ) ?
	      i->pixmap( 0 )->width() : 16;
    int x = r.x() + bdr;
    int y = r.y();
    int w = columnWidth( 0 ) - bdr;
    int h = QMAX( lined->height() + 2, r.height() );
    y = y + r.height() / 2 - h / 2;

    lined->parentWidget()->setGeometry( x, y, w + 6, h );
    lined->setFocus();
    lined->setText( i->text( 0 ) );
    lined->selectAll();
    lined->setFrame( FALSE );
    lined->parentWidget()->show();
    viewport()->setFocusProxy( lined );
    renaming = TRUE;
}

void QFileDialogQFileListView::clear()
{
    cancelRename();
    QListView::clear();
}

void QFileDialogQFileListView::rename()
{
    if ( !lined->text().isEmpty() ) {
	QString file = currentItem()->text( 0 );

	if ( lined->text() != file )
	    filedialog->d->url.rename( file, lined->text() );
    }
    cancelRename();
}

void QFileDialogQFileListView::cancelRename()
{
    renameItem = 0;
    lined->parentWidget()->hide();
    viewport()->setFocusProxy( this );
    renaming = FALSE;
    if ( currentItem() )
	currentItem()->repaint();
    if ( lined->hasFocus() )
	viewport()->setFocus();
}

void QFileDialogQFileListView::contentsMoved( int, int )
{
    changeDirTimer->stop();
#ifndef QT_NO_DRAGANDDROP
    setCurrentDropItem( QPoint( -1, -1 ) );
#endif
}


QFileDialogPrivate::File::~File()
{
    if ( d->pendingItems.findRef( this ) )
	d->pendingItems.removeRef( this );
}

QString QFileDialogPrivate::File::text( int column ) const
{
    makeVariables();

    switch( column ) {
    case 0:
        return info.name();
    case 1:
	if ( info.isFile() ) {
#if (QT_VERSION-0 >= 0x040000)
#error "clean up Large File Support"
#elif defined(QT_ABI_QT4)
	    QIODevice::Offset size = info.size();
#else
	    uint size = info.size();
#endif
#if defined(QT_LARGEFILE_SUPPORT) && defined(Q_OS_UNIX)
	    // ### the following code should not be needed as soon
	    // ### as QUrlInfo::size() can return 64-bit
	    if ( size > INT_MAX ) {
		struct stat buffer;
		if ( ::stat( QFile::encodeName(info.name()), &buffer ) == 0 ) {
		    Q_ULLONG size64 = (Q_ULLONG)buffer.st_size;
		    return QString::number(size64);
		}
	    }
#endif
	    return QString::number(size);
	} else {
	    return QString::fromLatin1("");
	}
    case 2:
	if ( info.isFile() && info.isSymLink() ) {
	    return d->symLinkToFile;
	} else if ( info.isFile() ) {
	    return d->file;
	} else if ( info.isDir() && info.isSymLink() ) {
	    return d->symLinkToDir;
	} else if ( info.isDir() ) {
	    return d->dir;
	} else if ( info.isSymLink() ) {
	    return d->symLinkToSpecial;
	} else {
	    return d->special;
	}
    case 3: {
	return info.lastModified().toString( Qt::LocalDate );
    }
    case 4:
	if ( info.isReadable() )
	    return info.isWritable() ? d->rw : d->ro;
	else
	    return info.isWritable() ? d->wo : d->inaccessible;
    }

    return QString::fromLatin1("<--->");
}

const QPixmap * QFileDialogPrivate::File::pixmap( int column ) const
{
    if ( column ) {
	return 0;
    } else if ( QListViewItem::pixmap( column ) ) {
	return QListViewItem::pixmap( column );
    } else if ( info.isSymLink() ) {
	if ( info.isFile() )
	    return symLinkFileIcon;
	else
	    return symLinkDirIcon;
    } else if ( info.isDir() ) {
	return closedFolderIcon;
    } else if ( info.isFile() ) {
	return fileIcon;
    } else {
	return fifteenTransparentPixels;
    }
}

QFileDialogPrivate::MCItem::MCItem( QListBox * lb, QListViewItem * item )
    : QListBoxItem()
{
    i = item;
    if ( lb )
	lb->insertItem( this );
}

QFileDialogPrivate::MCItem::MCItem( QListBox * lb, QListViewItem * item, QListBoxItem *after )
    : QListBoxItem()
{
    i = item;
    if ( lb )
	lb->insertItem( this, after );
}

QString QFileDialogPrivate::MCItem::text() const
{
    return i->text( 0 );
}


const QPixmap *QFileDialogPrivate::MCItem::pixmap() const
{
    return i->pixmap( 0 );
}


int QFileDialogPrivate::MCItem::height( const QListBox * lb ) const
{
    int hf = lb->fontMetrics().height();
    int hp = pixmap() ? pixmap()->height() : 0;
    return QMAX(hf, hp) + 2;
}


int QFileDialogPrivate::MCItem::width( const QListBox * lb ) const
{
    QFontMetrics fm = lb->fontMetrics();
    int w = 2;
    if ( pixmap() )
	w += pixmap()->width() + 4;
    else
	w += 18;
    w += fm.width( text() );
    w += -fm.minLeftBearing();
    w += -fm.minRightBearing();
    w += 6;
    return w;
}


void QFileDialogPrivate::MCItem::paint( QPainter * ptr )
{
    QFontMetrics fm = ptr->fontMetrics();

    int h;

    if ( pixmap() )
	h = QMAX( fm.height(), pixmap()->height()) + 2;
    else
	h = fm.height() + 2;

    const QPixmap * pm = pixmap();
    if ( pm )
	ptr->drawPixmap( 2, 1, *pm );

    ptr->drawText( pm ? pm->width() + 4 : 22, h - fm.descent() - 2,
		   text() );
}

static QStringList makeFiltersList( const QString &filter )
{
    if ( filter.isEmpty() )
	return QStringList();

    int i = filter.find( ";;", 0 );
    QString sep( ";;" );
    if ( i == -1 ) {
	if ( filter.find( "\n", 0 ) != -1 ) {
	    sep = "\n";
	    i = filter.find( sep, 0 );
	}
    }

    return QStringList::split( sep, filter );
}

/*!
  \class QFileDialog qfiledialog.h
  \brief The QFileDialog class provides dialogs that allow users to select files or directories.
  \ingroup dialogs
  \mainclass

  The QFileDialog class enables a user to traverse their file system in
  order to select one or many files or a directory.

  The easiest way to create a QFileDialog is to use the static
  functions. On Windows, these static functions will call the native
  Windows file dialog and on Mac OS X, these static function will call
  the native Mac OS X file dialog.

  \code
    QString s = QFileDialog::getOpenFileName(
		    "/home",
		    "Images (*.png *.xpm *.jpg)",
		    this,
		    "open file dialog",
		    "Choose a file" );
  \endcode

  In the above example, a modal QFileDialog is created using a static
  function. The startup directory is set to "/home". The file filter
  is set to "Images (*.png *.xpm *.jpg)". The parent of the file dialog
  is set to \e this and it is given the identification name - "open file
  dialog". The caption at the top of file dialog is set to "Choose a
  file". If you want to use multiple filters, separate each one with
  \e two semi-colons, e.g.
  \code
  "Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
  \endcode

  You can create your own QFileDialog without using the static
  functions. By calling setMode(), you can set what can be returned by
  the QFileDialog.

  \code
    QFileDialog* fd = new QFileDialog( this, "file dialog", TRUE );
    fd->setMode( QFileDialog::AnyFile );
  \endcode

  In the above example, the mode of the file dialog is set to \c
  AnyFile, meaning that the user can select any file, or even specify a
  file that doesn't exist. This mode is useful for creating a "File Save
  As" file dialog. Use \c ExistingFile if the user must select an
  existing file or \c Directory if only a directory may be selected.
  (See the \l QFileDialog::Mode enum for the complete list of modes.)

  You can retrieve the dialog's mode with mode(). Use setFilter() to set
  the dialog's file filter, e.g.

  \code
    fd->setFilter( "Images (*.png *.xpm *.jpg)" );
  \endcode

  In the above example, the filter is set to "Images (*.png *.xpm
  *.jpg)", this means that only files with the extension \c png, \c xpm
  or \c jpg will be shown in the QFileDialog. You can apply
  several filters by using setFilters() and add additional filters with
  addFilter(). Use setSelectedFilter() to select one of the filters
  you've given as the file dialog's default filter. Whenever the user
  changes the filter the filterSelected() signal is emitted.

  The file dialog has two view modes, QFileDialog::List which simply
  lists file and directory names and QFileDialog::Detail which
  displays additional information alongside each name, e.g. file size,
  modification date, etc. Set the mode with setViewMode().

  \code
    fd->setViewMode( QFileDialog::Detail );
  \endcode

  The last important function you will need to use when creating your
  own file dialog is selectedFile().

  \code
    QString fileName;
    if ( fd->exec() == QDialog::Accepted )
	fileName = fd->selectedFile();
  \endcode

  In the above example, a modal file dialog is created and shown. If
  the user clicked OK, then the file they selected is put in \c
  fileName.

  If you are using the \c ExistingFiles mode then you will need to use
  selectedFiles() which will return the selected files in a QStringList.

  The dialog's working directory can be set with setDir(). The display
  of hidden files is controlled with setShowHiddenFiles(). The dialog
  can be forced to re-read the directory with rereadDir() and re-sort
  the directory with resortDir(). All the files in the current directory
  can be selected with selectAll().

  \section1 Creating and using preview widgets

  There are two kinds of preview widgets that can be used with
  QFileDialogs: \e content preview widgets and \e information preview
  widgets. They are created and used in the same way except that the
  function names differ, e.g. setContentsPreview() and setInfoPreview().

  A preview widget is a widget that is placed inside a QFileDialog so
  that the user can see either the contents of the file, or information
  about the file.

  \code
    class Preview : public QLabel, public QFilePreview
    {
    public:
	Preview( QWidget *parent=0 ) : QLabel( parent ) {}

	void previewUrl( const QUrl &u )
	{
	    QString path = u.path();
	    QPixmap pix( path );
	    if ( pix.isNull() )
		setText( "This is not a pixmap" );
	    else
		setPixmap( pix );
	}
    };
  \endcode

  In the above snippet, we create a preview widget which inherits from
  QLabel and QFilePreview. File preview widgets \e must inherit from
  QFilePreview.

  Inside the class we reimplement QFilePreview::previewUrl(), this is
  where we determine what happens when a file is selected. In the
  above example we only show a preview of the file if it is a valid
  pixmap. Here's how to make a file dialog use a preview widget:

  \code
    Preview* p = new Preview;

    QFileDialog* fd = new QFileDialog( this );
    fd->setContentsPreviewEnabled( TRUE );
    fd->setContentsPreview( p, p );
    fd->setPreviewMode( QFileDialog::Contents );
    fd->show();
  \endcode

  The first line creates an instance of our preview widget. We then
  create our file dialog and call setContentsPreviewEnabled( TRUE ),
  this tell the file dialog to preview the contents of the currently
  selected file. We then call setContentsPreview() -- note that we pass
  the same preview widget twice. Finally, before showing the file
  dialog, we call setPreviewMode() setting the mode to \e Contents which
  will show the contents preview of the file that the user has selected.

  If you create another preview widget that is used for displaying
  information about a file, create it in the same way as the contents
  preview widget and call setInfoPreviewEnabled(), and
  setInfoPreview(). Then the user will be able to switch between the
  two preview modes.

  For more information about creating a QFilePreview widget see
  \l{QFilePreview}.

  <img src=qfiledlg-m.png> <img src=qfiledlg-w.png>

*/


/*! \enum QFileDialog::Mode

  This enum is used to indicate what the user may select in the file
  dialog, i.e. what the dialog will return if the user clicks OK.

  \value AnyFile  The name of a file, whether it exists or not.
  \value ExistingFile  The name of a single existing file.
  \value Directory  The name of a directory. Both files and directories
  are displayed.
  \value DirectoryOnly  The name of a directory. The file dialog will only display directories.
  \value ExistingFiles  The names of zero or more existing files.

  See setMode().
*/

/*!
  \enum QFileDialog::ViewMode

  This enum describes the view mode of the file dialog, i.e. what
  information about each file will be displayed.

  \value List  Display file and directory names with icons.
  \value Detail  Display file and directory names with icons plus
  additional information, such as file size and modification date.

  See setViewMode().
*/

/*!
  \enum QFileDialog::PreviewMode

  This enum describes the preview mode of the file dialog.

  \value NoPreview  No preview is shown at all.
  \value Contents  Show a preview of the contents of the current file
  using the contents preview widget.
  \value Info  Show information about the current file using the
  info preview widget.

  See setPreviewMode(), setContentsPreview() and setInfoPreview().
*/

/*!
  \fn void QFileDialog::detailViewSelectionChanged()
  \internal
*/

/*!
  \fn void QFileDialog::listBoxSelectionChanged()
  \internal
*/

extern const char qt_file_dialog_filter_reg_exp[] =
	"([a-zA-Z0-9 ]*)\\(([a-zA-Z0-9_.*? +;#\\[\\]]*)\\)$";

/*!
  Constructs a file dialog called \a name, with the parent, \a parent.
  If \a modal is TRUE then the file dialog is modal; otherwise it is
  modeless.
*/

QFileDialog::QFileDialog( QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal,
	       (modal ?
		(WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu) : 0) )
{
    init();
    d->mode = ExistingFile;
    d->types->insertItem( tr( "All Files (*)" ) );
    d->cursorOverride = FALSE;
    emit dirEntered( d->url.dirPath() );
    rereadDir();
}


/*!
  Constructs a file dialog called \a name with the parent, \a parent.
  If \a modal is TRUE then the file dialog is modal; otherwise it is
  modeless.

  If \a dirName is specified then it will be used as the dialog's
  working directory, i.e. it will be the directory that is shown when
  the dialog appears. If \a filter is specified it will be used as the
  dialog's file filter.

*/

QFileDialog::QFileDialog( const QString& dirName, const QString & filter,
			  QWidget *parent, const char *name, bool modal )
    : QDialog( parent, name, modal,
	       (modal ?
		(WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu) : 0) )
{
    init();
    d->mode = ExistingFile;
    rereadDir();
    QUrlOperator u( dirName );
    if ( !dirName.isEmpty() && ( !u.isLocalFile() || QDir( dirName ).exists() ) )
	setSelection( dirName );
    else if ( workingDirectory && !workingDirectory->isEmpty() )
	setDir( *workingDirectory );

    if ( !filter.isEmpty() ) {
	setFilters( filter );
	if ( !dirName.isEmpty() ) {
	    int dotpos = dirName.find( QChar('.'), 0, FALSE );
	    if ( dotpos != -1 ) {
		for ( int b=0 ; b<d->types->count() ; b++ ) {
		    if ( d->types->text(b).contains( dirName.right( dirName.length() - dotpos ) ) ) {
			d->types->setCurrentItem( b );
			setFilter( d->types->text( b ) );
			return;
		    }
		}
	    }
	}
    } else {
	d->types->insertItem( tr( "All Files (*)" ) );
    }
}


#if defined(Q_WS_WIN)
extern int qt_ntfs_permission_lookup;
#endif

/*!
  \internal
  Initializes the file dialog.
*/

void QFileDialog::init()
{
    setSizeGripEnabled( TRUE );
    d = new QFileDialogPrivate();
    d->mode = AnyFile;
    d->last = 0;
    d->lastEFSelected = 0;
    d->moreFiles = 0;
    d->infoPreview = FALSE;
    d->contentsPreview = FALSE;
    d->hadDotDot = FALSE;
    d->ignoreNextKeyPress = FALSE;
    d->progressDia = 0;
    d->checkForFilter = FALSE;
    d->ignoreNextRefresh = FALSE;
    d->ignoreStop = FALSE;
    d->pendingItems.setAutoDelete( FALSE );
    d->mimeTypeTimer = new QTimer( this );
    d->cursorOverride = FALSE;
#if defined(Q_WS_WIN)
    d->oldPermissionLookup = qt_ntfs_permission_lookup;
#endif
    connect( d->mimeTypeTimer, SIGNAL( timeout() ),
	     this, SLOT( doMimeTypeLookup() ) );

    d->url = QUrlOperator( ::toRootIfNotExists( QDir::currentDirPath() ) );
    d->oldUrl = d->url;
    d->currListChildren = 0;

    connect( &d->url, SIGNAL( start(QNetworkOperation*) ),
	     this, SLOT( urlStart(QNetworkOperation*) ) );
    connect( &d->url, SIGNAL( finished(QNetworkOperation*) ),
	     this, SLOT( urlFinished(QNetworkOperation*) ) );
    connect( &d->url, SIGNAL( newChildren(const QValueList<QUrlInfo>&,QNetworkOperation*) ),
	     this, SLOT( insertEntry(const QValueList<QUrlInfo>&,QNetworkOperation*) ) );
    connect( &d->url, SIGNAL( removed(QNetworkOperation*) ),
	     this, SLOT( removeEntry(QNetworkOperation*) ) );
    connect( &d->url, SIGNAL( createdDirectory(const QUrlInfo&,QNetworkOperation*) ),
	     this, SLOT( createdDirectory(const QUrlInfo&,QNetworkOperation*) ) );
    connect( &d->url, SIGNAL( itemChanged(QNetworkOperation*) ),
	     this, SLOT( itemChanged(QNetworkOperation*) ) );
    connect( &d->url, SIGNAL( dataTransferProgress(int,int,QNetworkOperation*) ),
	     this, SLOT( dataTransferProgress(int,int,QNetworkOperation*) ) );

    nameEdit = new QLineEdit( this, "name/filter editor" );
    nameEdit->setMaxLength( 255 ); //_POSIX_MAX_PATH
    connect( nameEdit, SIGNAL(textChanged(const QString&)),
	     this,  SLOT(fileNameEditDone()) );
    nameEdit->installEventFilter( this );

    d->splitter = new QSplitter( this, "qt_splitter" );

    d->stack = new QWidgetStack( d->splitter, "files and more files" );

    d->splitter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    files = new QFileDialogQFileListView( d->stack, this );
    QFontMetrics fm = fontMetrics();
    files->addColumn( tr("Name") );
    files->addColumn( tr("Size") );
    files->setColumnAlignment( 1, AlignRight );
    files->addColumn( tr("Type") );
    files->addColumn( tr("Date") );
    files->addColumn( tr("Attributes") );
    files->header()->setStretchEnabled( TRUE, 0 );

    files->setMinimumSize( 50, 25 + 2*fm.lineSpacing() );

    connect( files, SIGNAL( selectionChanged() ),
	     this, SLOT( detailViewSelectionChanged() ) );
    connect( files, SIGNAL(currentChanged(QListViewItem*)),
	     this, SLOT(updateFileNameEdit(QListViewItem*)) );
    connect( files, SIGNAL(doubleClicked(QListViewItem*)),
	     this, SLOT(selectDirectoryOrFile(QListViewItem*)) );
    connect( files, SIGNAL(returnPressed(QListViewItem*)),
	     this, SLOT(selectDirectoryOrFile(QListViewItem*)) );
    connect( files, SIGNAL(rightButtonPressed(QListViewItem*,const QPoint&,int)),
	     this, SLOT(popupContextMenu(QListViewItem*,const QPoint&,int)) );

    files->installEventFilter( this );
    files->viewport()->installEventFilter( this );

    d->moreFiles = new QFileListBox( d->stack, this );
    d->moreFiles->setRowMode( QListBox::FitToHeight );
    d->moreFiles->setVariableWidth( TRUE );

    connect( d->moreFiles, SIGNAL(selected(QListBoxItem*)),
	     this, SLOT(selectDirectoryOrFile(QListBoxItem*)) );
    connect( d->moreFiles, SIGNAL( selectionChanged() ),
	     this, SLOT( listBoxSelectionChanged() ) );
    connect( d->moreFiles, SIGNAL(highlighted(QListBoxItem*)),
      this, SLOT(updateFileNameEdit(QListBoxItem*)) );
    connect( d->moreFiles, SIGNAL( rightButtonPressed(QListBoxItem*,const QPoint&) ),
	     this, SLOT( popupContextMenu(QListBoxItem*,const QPoint&) ) );

    d->moreFiles->installEventFilter( this );
    d->moreFiles->viewport()->installEventFilter( this );

    okB = new QPushButton( tr("&OK"), this, "OK" ); //### Or "Save (see other "OK")
    okB->setDefault( TRUE );
    okB->setEnabled( FALSE );
    connect( okB, SIGNAL(clicked()), this, SLOT(okClicked()) );
    cancelB = new QPushButton( tr("Cancel") , this, "Cancel" );
    connect( cancelB, SIGNAL(clicked()), this, SLOT(cancelClicked()) );

    d->paths = new QComboBox( TRUE, this, "directory history/editor" );
    d->paths->setDuplicatesEnabled( FALSE );
    d->paths->setInsertionPolicy( QComboBox::NoInsertion );
    const QFileInfoList * rootDrives = QDir::drives();
    QFileInfoListIterator it( *rootDrives );
    QFileInfo *fi;
    makeVariables();

    while ( (fi = it.current()) != 0 ) {
	++it;
	d->paths->insertItem( *openFolderIcon, fi->absFilePath() );
    }

    if ( !!QDir::homeDirPath() ) {
	if ( !d->paths->listBox()->findItem( QDir::homeDirPath() ) )
	    d->paths->insertItem( *openFolderIcon, QDir::homeDirPath() );
    }

    connect( d->paths, SIGNAL(activated(const QString&)),
	     this, SLOT(setDir(const QString&)) );

    d->paths->installEventFilter( this );
    QObjectList *ol = d->paths->queryList( "QLineEdit" );
    if ( ol && ol->first() )
	( (QLineEdit*)ol->first() )->installEventFilter( this );
    delete ol;

    d->geometryDirty = TRUE;
    d->types = new QComboBox( TRUE, this, "file types" );
    d->types->setDuplicatesEnabled( FALSE );
    d->types->setEditable( FALSE );
    connect( d->types, SIGNAL(activated(const QString&)),
	     this, SLOT(setFilter(const QString&)) );
    connect( d->types, SIGNAL(activated(const QString&)),
	     this, SIGNAL(filterSelected(const QString&)) );

    d->pathL = new QLabel( d->paths, tr("Look &in:"), this, "qt_looin_lbl" );
    d->fileL = new QLabel( nameEdit, tr("File &name:"), this, "qt_filename_lbl" );
    d->typeL = new QLabel( d->types, tr("File &type:"), this, "qt_filetype_lbl" );

    d->goBack = new QToolButton( this, "go back" );
    d->goBack->setEnabled( FALSE );
    d->goBack->setFocusPolicy( TabFocus );
    connect( d->goBack, SIGNAL( clicked() ), this, SLOT( goBack() ) );
#ifndef QT_NO_TOOLTIP
    QToolTip::add( d->goBack, tr( "Back" ) );
#endif
    d->goBack->setIconSet( *goBackIcon );

    d->cdToParent = new QToolButton( this, "cd to parent" );
    d->cdToParent->setFocusPolicy( TabFocus );
#ifndef QT_NO_TOOLTIP
    QToolTip::add( d->cdToParent, tr( "One directory up" ) );
#endif
    d->cdToParent->setIconSet( *cdToParentIcon );
    connect( d->cdToParent, SIGNAL(clicked()),
	     this, SLOT(cdUpClicked()) );

    d->newFolder = new QToolButton( this, "new folder" );
    d->newFolder->setFocusPolicy( TabFocus );
#ifndef QT_NO_TOOLTIP
    QToolTip::add( d->newFolder, tr( "Create New Folder" ) );
#endif
    d->newFolder->setIconSet( *newFolderIcon );
    connect( d->newFolder, SIGNAL(clicked()),
	     this, SLOT(newFolderClicked()) );

    d->modeButtons = new QButtonGroup( 0, "invisible group" );
    connect( d->modeButtons, SIGNAL(destroyed()),
	     this, SLOT(modeButtonsDestroyed()) );
    d->modeButtons->setExclusive( TRUE );
    connect( d->modeButtons, SIGNAL(clicked(int)),
	     d->stack, SLOT(raiseWidget(int)) );
    connect( d->modeButtons, SIGNAL(clicked(int)),
	     this, SLOT(changeMode(int)) );

    d->mcView = new QToolButton( this, "mclistbox view" );
    d->mcView->setFocusPolicy( TabFocus );
#ifndef QT_NO_TOOLTIP
    QToolTip::add( d->mcView, tr( "List View" ) );
#endif
    d->mcView->setIconSet( *multiColumnListViewIcon );
    d->mcView->setToggleButton( TRUE );
    d->stack->addWidget( d->moreFiles, d->modeButtons->insert( d->mcView ) );
    d->detailView = new QToolButton( this, "list view" );
    d->detailView->setFocusPolicy( TabFocus );
#ifndef QT_NO_TOOLTIP
    QToolTip::add( d->detailView, tr( "Detail View" ) );
#endif
    d->detailView->setIconSet( *detailViewIcon );
    d->detailView->setToggleButton( TRUE );
    d->stack->addWidget( files, d->modeButtons->insert( d->detailView ) );

    d->previewInfo = new QToolButton( this, "preview info view" );
    d->previewInfo->setFocusPolicy( TabFocus );
#ifndef QT_NO_TOOLTIP
    QToolTip::add( d->previewInfo, tr( "Preview File Info" ) );
#endif
    d->previewInfo->setIconSet( *previewInfoViewIcon );
    d->previewInfo->setToggleButton( TRUE );
    d->modeButtons->insert( d->previewInfo );

    d->previewContents = new QToolButton( this, "preview info view" );
#if defined(Q_WS_WIN) && !defined(Q_OS_TEMP)
    if ( (qt_winver & WV_NT_based) > Qt::WV_NT )
#else
    if ( !qstrcmp(style().className(), "QWindowsStyle") )
#endif
    {
	d->goBack->setAutoRaise( TRUE );
	d->cdToParent->setAutoRaise( TRUE );
	d->newFolder->setAutoRaise( TRUE );
	d->mcView->setAutoRaise( TRUE );
	d->detailView->setAutoRaise( TRUE );
	d->previewInfo->setAutoRaise( TRUE );
	d->previewContents->setAutoRaise( TRUE );
    }
    d->previewContents->setFocusPolicy( TabFocus );
#ifndef QT_NO_TOOLTIP
    QToolTip::add( d->previewContents, tr( "Preview File Contents" ) );
#endif
    d->previewContents->setIconSet( *previewContentsViewIcon );
    d->previewContents->setToggleButton( TRUE );
    d->modeButtons->insert( d->previewContents );

    connect( d->detailView, SIGNAL( clicked() ),
	     d->moreFiles, SLOT( cancelRename() ) );
    connect( d->detailView, SIGNAL( clicked() ),
	     files, SLOT( cancelRename() ) );
    connect( d->mcView, SIGNAL( clicked() ),
	     d->moreFiles, SLOT( cancelRename() ) );
    connect( d->mcView, SIGNAL( clicked() ),
	     files, SLOT( cancelRename() ) );

    d->stack->raiseWidget( d->moreFiles );
    d->mcView->setOn( TRUE );

    QHBoxLayout *lay = new QHBoxLayout( this );
    lay->setMargin( 6 );
    d->leftLayout = new QHBoxLayout( lay, 5 );
    d->topLevelLayout = new QVBoxLayout( (QWidget*)0, 5 );
    lay->addLayout( d->topLevelLayout, 1 );
    d->extraWidgetsLayouts.setAutoDelete( FALSE );
    d->extraLabels.setAutoDelete( FALSE );
    d->extraWidgets.setAutoDelete( FALSE );
    d->extraButtons.setAutoDelete( FALSE );
    d->toolButtons.setAutoDelete( FALSE );

    QHBoxLayout * h;

    d->preview = new QWidgetStack( d->splitter, "qt_preview" );

    d->infoPreviewWidget = new QWidget( d->preview, "qt_preview_info" );
    d->contentsPreviewWidget = new QWidget( d->preview, "qt_preview_contents" );
    d->infoPreviewer = d->contentsPreviewer = 0;

    h = new QHBoxLayout( 0 );
    d->buttonLayout = h;
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->pathL );
    h->addSpacing( 8 );
    h->addWidget( d->paths );
    h->addSpacing( 8 );
    if ( d->goBack )
	h->addWidget( d->goBack );
    h->addWidget( d->cdToParent );
    h->addSpacing( 2 );
    h->addWidget( d->newFolder );
    h->addSpacing( 4 );
    h->addWidget( d->mcView );
    h->addWidget( d->detailView );
    h->addWidget( d->previewInfo );
    h->addWidget( d->previewContents );

    d->topLevelLayout->addWidget( d->splitter );

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->fileL );
    h->addWidget( nameEdit );
    h->addSpacing( 15 );
    h->addWidget( okB );

    h = new QHBoxLayout();
    d->topLevelLayout->addLayout( h );
    h->addWidget( d->typeL );
    h->addWidget( d->types );
    h->addSpacing( 15 );
    h->addWidget( cancelB );

    d->rightLayout = new QHBoxLayout( lay, 5 );
    d->topLevelLayout->setStretchFactor( d->mcView, 1 );
    d->topLevelLayout->setStretchFactor( files, 1 );

    updateGeometries();

    if ( d->goBack ) {
	setTabOrder( d->paths, d->goBack );
	setTabOrder( d->goBack, d->cdToParent );
    } else {
	setTabOrder( d->paths, d->cdToParent );
    }
    setTabOrder( d->cdToParent, d->newFolder );
    setTabOrder( d->newFolder, d->mcView );
    setTabOrder( d->mcView, d->detailView );
    setTabOrder( d->detailView, d->moreFiles );
    setTabOrder( d->moreFiles, files );
    setTabOrder( files, nameEdit );
    setTabOrder( nameEdit, d->types );
    setTabOrder( d->types, okB );
    setTabOrder( okB, cancelB );

    d->rw = tr( "Read-write" );
    d->ro = tr( "Read-only" );
    d->wo = tr( "Write-only" );
    d->inaccessible = tr( "Inaccessible" );

    d->symLinkToFile = tr( "Symlink to File" );
    d->symLinkToDir = tr( "Symlink to Directory" );
    d->symLinkToSpecial = tr( "Symlink to Special" );
    d->file = tr( "File" );
    d->dir = tr( "Dir" );
    d->special = tr( "Special" );

    if ( lastWidth == 0 ) {
	QRect screen = QApplication::desktop()->screenGeometry( pos() );
	if ( screen.width() < 1024 || screen.height() < 768 ) {
	    resize( QMIN(screen.width(), 420), QMIN(screen.height(), 236) );
	} else {
	    QSize s = files->sizeHint();
	    s = QSize( s.width() + 300, s.height() + 82 );

	    if ( s.width() * 3 > screen.width() * 2 )
		s.setWidth( screen.width() * 2 / 3 );

	    if ( s.height() * 3 > screen.height() * 2 )
		s.setHeight( screen.height() * 2 / 3 );
	    else if ( s.height() * 3 < screen.height() )
		s.setHeight( screen.height() / 3 );

	    resize( s );
	}
	updateLastSize(this);
    } else {
	resize( lastWidth, lastHeight );
    }

    if ( detailViewMode ) {
	d->stack->raiseWidget( files );
	d->mcView->setOn( FALSE );
	d->detailView->setOn( TRUE );
    }

    d->preview->hide();
    nameEdit->setFocus();

    connect( nameEdit, SIGNAL( returnPressed() ),
	     this, SLOT( fileNameEditReturnPressed() ) );
}

/*!
  \internal
*/

void QFileDialog::fileNameEditReturnPressed()
{
    d->oldUrl = d->url;
    if ( !isDirectoryMode( d->mode ) ) {
	okClicked();
    } else {
	d->currentFileName = QString::null;
	if ( nameEdit->text().isEmpty() ) {
	    emit fileSelected( selectedFile() );
	    accept();
	} else {
	    QUrlInfo f;
	    QFileDialogPrivate::File * c
		= (QFileDialogPrivate::File *)files->currentItem();
	    if ( c && files->isSelected(c) )
		f = c->info;
	    else
		f = QUrlInfo( d->url, nameEdit->text() );
	    if ( f.isDir() ) {
		setUrl( QUrlOperator( d->url,
				      QFileDialogPrivate::encodeFileName(nameEdit->text() + "/" ) ) );
		d->checkForFilter = TRUE;
		trySetSelection( TRUE, d->url, TRUE );
		d->checkForFilter = FALSE;
	    }
	}
	nameEdit->setText( QString::null );
    }
}

/*!
  \internal
  Update the info and content preview widgets to display \a u.
*/

void QFileDialog::updatePreviews( const QUrl &u )
{
    if ( d->infoPreviewer )
	d->infoPreviewer->previewUrl( u );
    if ( d->contentsPreviewer )
	d->contentsPreviewer->previewUrl( u );
}

/*!
  \internal
  Changes the preview mode to the mode specified at \a id.
*/

void QFileDialog::changeMode( int id )
{
    if ( !d->infoPreview && !d->contentsPreview )
	return;

    QButton *btn = (QButton*)d->modeButtons->find( id );
    if ( !btn )
	return;

    if ( btn == d->previewContents && !d->contentsPreview )
	return;
    if ( btn == d->previewInfo && !d->infoPreview )
	return;

    if ( btn != d->previewContents && btn != d->previewInfo ) {
	d->preview->hide();
    } else {
	if ( files->currentItem() )
	    updatePreviews( QUrl( d->url, files->currentItem()->text( 0 ) ) );
	if ( btn == d->previewInfo )
	    d->preview->raiseWidget( d->infoPreviewWidget );
	else
	    d->preview->raiseWidget( d->contentsPreviewWidget );
	d->preview->show();
    }
}

/*!
  Destroys the file dialog.
*/

QFileDialog::~QFileDialog()
{
    // since clear might call setContentsPos which would emit
    // a signal and thus cause a recompute of sizes...
    files->blockSignals( TRUE );
    d->moreFiles->blockSignals( TRUE );
    files->clear();
    d->moreFiles->clear();
    d->moreFiles->blockSignals( FALSE );
    files->blockSignals( FALSE );

#ifndef QT_NO_CURSOR
    if ( d->cursorOverride )
	QApplication::restoreOverrideCursor();
#endif

    delete d;
    d = 0;
}


/*!
  \property QFileDialog::selectedFile

  \brief the name of the selected file

  If a file was selected selectedFile contains the file's name including
  its absolute path; otherwise selectedFile is empty.

  \sa QString::isEmpty(), selectedFiles, selectedFilter
*/

QString QFileDialog::selectedFile() const
{
    QString s = d->currentFileName;
    // remove the protocol because we do not want to encode it...
    QString prot = QUrl( s ).protocol();
    if ( !prot.isEmpty() ) {
        prot += ":";
	s.remove( 0, prot.length() );
    }
    QUrl u( prot + QFileDialogPrivate::encodeFileName( s ) );
    if ( u.isLocalFile() ) {
	QString s = u.toString();
	if ( s.left( 5 ) == "file:" )
	    s.remove( (uint)0, 5 );
	return s;
    }
    return d->currentFileName;
}

/*!
  \property QFileDialog::selectedFilter

  \brief the filter which the user has selected in the file dialog

  \sa filterSelected(), selectedFiles, selectedFile
*/

QString QFileDialog::selectedFilter() const
{
    return d->types->currentText();
}

/*! \overload

  Sets the current filter selected in the file dialog to the
  \a{n}-th filter in the filter list.

  \sa filterSelected(), selectedFilter(), selectedFiles(), selectedFile()
*/

void QFileDialog::setSelectedFilter( int n )
{
    d->types->setCurrentItem( n );
    QString f = d->types->currentText();
    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
    int index = r.search( f );
    if ( index >= 0 )
	f = r.cap( 2 );
    d->url.setNameFilter( f );
    rereadDir();
}

/*!
  Sets the current filter selected in the file dialog to the first
  one that contains the text \a mask.
*/

void QFileDialog::setSelectedFilter( const QString& mask )
{
    int n;

    for ( n = 0; n < d->types->count(); n++ ) {
	if ( d->types->text( n ).contains( mask, FALSE ) ) {
	    d->types->setCurrentItem( n );
	    QString f = mask;
	    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
	    int index = r.search( f );
	    if ( index >= 0 )
		f = r.cap( 2 );
	    d->url.setNameFilter( f );
	    rereadDir();
	    return;
	}
    }
}

/*!
  \property QFileDialog::selectedFiles

  \brief the list of selected files

  If one or more files are selected, selectedFiles contains their
  names including their absolute paths. If no files are selected or
  the mode isn't ExistingFiles selectedFiles is an empty list.

  It is more convenient to use selectedFile() if the mode is
  \c ExistingFile, \c Directory or \c DirectoryOnly.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = myFileDialog.selectedFiles();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa selectedFile, selectedFilter, QValueList::empty()
*/

QStringList QFileDialog::selectedFiles() const
{
    QStringList lst;

    if ( mode() == ExistingFiles ) {
	QStringList selectedLst;
	QString selectedFiles = nameEdit->text();
        if (selectedFiles.findRev('\"') == -1) {
            //probably because Enter was pressed on the nameEdit, so we have one file
            //not in "" but raw
            selectedLst.append(selectedFiles);
        } else {
            selectedFiles.truncate( selectedFiles.findRev( '\"' ) );
            selectedLst = selectedLst.split( QString("\" "), selectedFiles );
        }
	for ( QStringList::Iterator it = selectedLst.begin(); it != selectedLst.end(); ++it ) {
	    QUrl u;
	    if ( (*it)[0] == '\"' ) {
		u = QUrl( d->url, QFileDialogPrivate::encodeFileName( (*it).mid(1) ) );
	    } else {
		u = QUrl( d->url, QFileDialogPrivate::encodeFileName( (*it) ) );
	    }
	    if ( u.isLocalFile() ) {
		QString s = u.toString();
		if ( s.left( 5 ) == "file:" )
		    s.remove( (uint)0, 5 );
		lst << s;
	    } else {
		lst << u.toString();
	    }
	}
    }

    return lst;
}

/*!
  Sets the default selection to \a filename. If \a filename is
  absolute, setDir() is also called to set the file dialog's working
  directory to the filename's directory.

  \omit
  Only for external use. Not useful inside QFileDialog.
  \endomit
*/

void QFileDialog::setSelection( const QString & filename )
{
    d->oldUrl = d->url;
    QString nf = d->url.nameFilter();
    if ( QUrl::isRelativeUrl( filename ) )
	d->url = QUrlOperator( d->url, QFileDialogPrivate::encodeFileName( filename ) );
    else
	d->url = QUrlOperator( filename );
    d->url.setNameFilter( nf );
    d->checkForFilter = TRUE;
    bool isDirOk;
    bool isDir = d->url.isDir( &isDirOk );
    if ( !isDirOk )
	isDir = d->url.path().right( 1 ) == "/";
    if ( !isDir ) {
	QUrlOperator u( d->url );
	d->url.setPath( d->url.dirPath() );
	trySetSelection( FALSE, u, TRUE );
	d->ignoreNextRefresh = TRUE;
	nameEdit->selectAll();
	rereadDir();
	emit dirEntered( d->url.dirPath() );
    } else {
	if ( !d->url.path().isEmpty() &&
	     d->url.path().right( 1 ) != "/" ) {
	    QString p = d->url.path();
	    p += "/";
	    d->url.setPath( p );
	}
	trySetSelection( TRUE, d->url, FALSE );
	rereadDir();
	emit dirEntered( d->url.dirPath() );
	nameEdit->setText( QString::fromLatin1("") );
    }
    d->checkForFilter = FALSE;
}

/*!
  \property QFileDialog::dirPath

  \brief the file dialog's working directory

  \sa dir(), setDir()
*/

QString QFileDialog::dirPath() const
{
    return d->url.dirPath();
}


/*!

  Sets the filter used in the file dialog to \a newFilter.

  If \a newFilter contains a pair of parentheses containing one or more
  of <em><b>anything*something</b></em> separated by spaces or by
  semi-colons then only the text contained in the parentheses is used as
  the filter. This means that these calls are all equivalent:

  \code
     fd->setFilter( "All C++ files (*.cpp *.cc *.C *.cxx *.c++)" );
     fd->setFilter( "*.cpp *.cc *.C *.cxx *.c++" );
     fd->setFilter( "All C++ files (*.cpp;*.cc;*.C;*.cxx;*.c++)" );
     fd->setFilter( "*.cpp;*.cc;*.C;*.cxx;*.c++" );
  \endcode

  \sa setFilters()
*/

void QFileDialog::setFilter( const QString & newFilter )
{
    if ( newFilter.isEmpty() )
	return;
    QString f = newFilter;
    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
    int index = r.search( f );
    if ( index >= 0 )
	f = r.cap( 2 );
    d->url.setNameFilter( f );
    if ( d->types->count() == 1 )  {
	d->types->clear();
	d->types->insertItem( newFilter );
    } else {
	for ( int i = 0; i < d->types->count(); ++i ) {
	    if ( d->types->text( i ).left( newFilter.length() ) == newFilter ||
		 d->types->text( i ).left( f.length() ) == f ) {
		d->types->setCurrentItem( i );
		break;
	    }
	}
    }
    rereadDir();
}


/*! \overload
  Sets the file dialog's working directory to \a pathstr.

  \sa dir()
*/

void QFileDialog::setDir( const QString & pathstr )
{
    QString dr = pathstr;
    if ( dr.isEmpty() )
	return;

#if defined(Q_OS_UNIX)
    if ( dr.length() && dr[0] == '~' ) {
	int i = 0;
	while( i < (int)dr.length() && dr[i] != '/' )
	    i++;
	QCString user;
	if ( i == 1 ) {
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)

#  ifndef _POSIX_LOGIN_NAME_MAX
#    define _POSIX_LOGIN_NAME_MAX 9
#  endif

	    char name[_POSIX_LOGIN_NAME_MAX];
	    if ( ::getlogin_r( name, _POSIX_LOGIN_NAME_MAX ) == 0 )
		user = name;
	    else
#else
	    user = ::getlogin();
	    if ( !user )
#endif
		user = getenv( "LOGNAME" );
	} else
	    user = dr.mid( 1, i-1 ).local8Bit();
	dr = dr.mid( i, dr.length() );
	struct passwd *pw;
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_FREEBSD) && !defined(Q_OS_OPENBSD)
	struct passwd mt_pw;
	char buffer[2048];
	if ( ::getpwnam_r( user, &mt_pw, buffer, 2048, &pw ) == 0 && pw == &mt_pw )
#else
	pw = ::getpwnam( user );
	if ( pw )
#endif
	    dr.prepend( QString::fromLocal8Bit(pw->pw_dir) );
    }
#endif

    setUrl( dr );
}

/*!
  Returns the current directory shown in the file dialog.

  The ownership of the QDir pointer is transferred to the caller, so
  it must be deleted by the caller when no longer required.

  \sa setDir()
*/

const QDir *QFileDialog::dir() const
{
    if ( d->url.isLocalFile() )
	return  new QDir( d->url.path() );
    else
	return 0;
}

/*!
  Sets the file dialog's working directory to \a dir.
  \sa dir()
*/

void QFileDialog::setDir( const QDir &dir )
{
    d->oldUrl = d->url;
    QString nf( d->url.nameFilter() );
    d->url = dir.canonicalPath();
    d->url.setNameFilter( nf );
    QUrlInfo i( d->url, nameEdit->text() );
    d->checkForFilter = TRUE;
    trySetSelection( i.isDir(), QUrlOperator( d->url, QFileDialogPrivate::encodeFileName(nameEdit->text() ) ), FALSE );
    d->checkForFilter = FALSE;
    rereadDir();
    emit dirEntered( d->url.path() );
}

/*!
  Sets the file dialog's working directory to the directory specified at \a url.

  \sa url()
*/

void QFileDialog::setUrl( const QUrlOperator &url )
{
    d->oldUrl = d->url;
    QString nf = d->url.nameFilter();

    QString operatorPath = url.toString( FALSE, FALSE );
    if ( QUrl::isRelativeUrl( operatorPath ) ) {
	d->url = QUrl( d->url, operatorPath );
    } else {
	d->url = url;
    }
    d->url.setNameFilter( nf );

    d->checkForFilter = TRUE;
    if ( !d->url.isDir() ) {
	QUrlOperator u = d->url;
	d->url.setPath( d->url.dirPath() );
	trySetSelection( FALSE, u, FALSE );
	rereadDir();
	emit dirEntered( d->url.dirPath() );
	QString fn = u.fileName();
	nameEdit->setText( fn );
    } else {
	trySetSelection( TRUE, d->url, FALSE );
	rereadDir();
	emit dirEntered( d->url.dirPath() );
    }
    d->checkForFilter = FALSE;
}

/*!
  \property QFileDialog::showHiddenFiles

  \brief whether hidden files are shown in the file dialog

  The default is FALSE, i.e. don't show hidden files.
*/

void QFileDialog::setShowHiddenFiles( bool s )
{
    if ( s == bShowHiddenFiles )
	return;

    bShowHiddenFiles = s;
    rereadDir();
}

bool QFileDialog::showHiddenFiles() const
{
    return bShowHiddenFiles;
}

/*!
  Rereads the current directory shown in the file dialog.

  The only time you will need to call this function is if the contents of
  the directory change and you wish to refresh the file dialog to reflect
  the change.

  \sa resortDir()
*/

void QFileDialog::rereadDir()
{
#ifndef QT_NO_CURSOR
    if ( !d->cursorOverride ) {
	QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
	d->cursorOverride = TRUE;
    }
#endif
    d->pendingItems.clear();
    if ( d->mimeTypeTimer->isActive() )
	d->mimeTypeTimer->stop();
    d->currListChildren = d->url.listChildren();
#ifndef QT_NO_CURSOR
    if ( d->cursorOverride ) {
	QApplication::restoreOverrideCursor();
	d->cursorOverride = FALSE;
    }
#endif
}


/*!
  \fn void QFileDialog::fileHighlighted( const QString& )

  This signal is emitted when the user highlights a file, i.e. makes
  it the current file.

  \sa fileSelected(), filesSelected()
*/

/*!
  \fn void QFileDialog::fileSelected( const QString& )

  This signal is emitted when the user selects a file.

  \sa filesSelected(), fileHighlighted(), selectedFile()
*/

/*!
  \fn void QFileDialog::filesSelected( const QStringList& )

  This signal is emitted when the user selects one or more files in \e
  ExistingFiles mode.

  \sa fileSelected(), fileHighlighted(), selectedFiles()
*/

/*!
  \fn void QFileDialog::dirEntered( const QString& )

  This signal is emitted when the user enters a directory.

  \sa dir()
*/

/*!
  \fn void QFileDialog::filterSelected( const QString& )

  This signal is emitted when the user selects a filter.

  \sa selectedFilter()
*/

extern bool qt_resolve_symlinks; // defined in qapplication.cpp
bool Q_EXPORT qt_use_native_dialogs = TRUE;

/*!
  This is a convenience static function that returns an existing file
  selected by the user. If the user pressed Cancel, it returns a null
  string.

  \code
    QString s = QFileDialog::getOpenFileName(
                    "/home",
		    "Images (*.png *.xpm *.jpg)",
		    this,
		    "open file dialog",
		    "Choose a file to open" );
  \endcode

  The function creates a modal file dialog called \a name, with
  parent, \a parent. If a parent is not 0, the dialog will be shown
  centered over the parent.

  The file dialog's working directory will be set to \a startWith. If \a
  startWith includes a file name, the file will be selected. The filter
  is set to \a filter so that only those files which match the filter
  are shown. The filter selected is set to \a selectedFilter. The parameters
  \a startWith, \a selectedFilter and \a filter may be QString::null.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style (Note that on Windows the
  dialog will spin a blocking modal event loop that will not dispatch any
  QTimers and if parent is not 0 then it will position the dialog just under
  the parent's titlebar).

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is FALSE, the file dialog will treat
  symlinks as regular directories.

  \sa getOpenFileNames(), getSaveFileName(), getExistingDirectory()
*/

QString QFileDialog::getOpenFileName( const QString & startWith,
				      const QString& filter,
				      QWidget *parent, const char* name,
				      const QString& caption,
				      QString *selectedFilter,
				      bool resolveSymlinks )
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = resolveSymlinks;

    QStringList filters;
    if ( !filter.isEmpty() )
	filters = makeFiltersList( filter );

    makeVariables();
    QString initialSelection;
    //### Problem with the logic here: If a startWith is given and a file
    // with that name exists in D->URL, the box will be opened at D->URL instead of
    // the last directory used ('workingDirectory').
    //
    // hm... isn't that problem exactly the documented behaviour? the
    // documented behaviour sounds meaningful.
    if ( !startWith.isEmpty() ) {
	QUrlOperator u( QFileDialogPrivate::encodeFileName( startWith ) );
	if ( u.isLocalFile() && QFileInfo( u.path() ).isDir() ) {
	    *workingDirectory = startWith;
	} else {
	    if ( u.isLocalFile() ) {
		QFileInfo fi( u.dirPath() );
		if ( fi.exists() ) {
		    *workingDirectory = u.dirPath();
		    initialSelection = u.fileName();
		}
	    } else {
		*workingDirectory = u.toString();
		initialSelection = QString::null;//u.fileName();
	    }
	}
    }

    if ( workingDirectory->isNull() )
	*workingDirectory = ::toRootIfNotExists( QDir::currentDirPath() );

#if defined(Q_WS_WIN)
    if ( qt_use_native_dialogs && qApp->style().styleHint( QStyle::SH_GUIStyle ) == WindowsStyle )
	return winGetOpenFileName( initialSelection, filter, workingDirectory,
				   parent, name, caption, selectedFilter );
#elif defined(Q_WS_MAC)
    if (qt_use_native_dialogs && (qApp->style().inherits(QMAC_DEFAULT_STYLE)
                                  || qApp->style().inherits("QMacStyle")))
        return qt_mac_precomposeFileName(macGetOpenFileNames(filter,
                                         startWith.isEmpty() ? 0 : workingDirectory,
                                         parent, name, caption, selectedFilter, FALSE).first());
#endif

    QFileDialog *dlg = new QFileDialog( *workingDirectory, QString::null, parent, name ? name : "qt_filedlg_gofn", TRUE );

    Q_CHECK_PTR( dlg );
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( !caption.isNull() )
	dlg->setCaption( caption );
    else
	dlg->setCaption( QFileDialog::tr( "Open" ) );
#endif

    dlg->setFilters( filters );
    if ( selectedFilter )
	dlg->setFilter( *selectedFilter );
    dlg->setMode( QFileDialog::ExistingFile );
    QString result;
    if ( !initialSelection.isEmpty() )
	dlg->setSelection( initialSelection );
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	*workingDirectory = dlg->d->url;
	if ( selectedFilter )
	    *selectedFilter = dlg->selectedFilter();
    }
    delete dlg;

    qt_resolve_symlinks = save_qt_resolve_symlinks;

    return result;
}

/*!
  This is a convenience static function that will return a file name
  selected by the user. The file does not have to exist.

  It creates a modal file dialog called \a name, with parent, \a parent.
  If a parent is not 0, the dialog will be shown centered over the
  parent.

  \code
    QString s = QFileDialog::getSaveFileName(
                    "/home",
		    "Images (*.png *.xpm *.jpg)",
		    this,
		    "save file dialog",
		    "Choose a filename to save under" );
  \endcode

  The file dialog's working directory will be set to \a startWith. If \a
  startWith includes a file name, the file will be selected. The filter
  is set to \a filter so that only those files which match the filter
  are shown. The filter selected is set to \a selectedFilter. The parameters
  \a startWith, \a selectedFilter and \a filter may be QString::null.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style. (Note that on Windows the
  dialog will spin a blocking modal event loop that will not dispatch any
  QTimers and if parent is not 0 then it will position the dialog just under
  the parent's titlebar.

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is FALSE, the file dialog will treat
  symlinks as regular directories.

  \sa getOpenFileName(), getOpenFileNames(), getExistingDirectory()
*/

QString QFileDialog::getSaveFileName( const QString & startWith,
				      const QString& filter,
				      QWidget *parent, const char* name,
				      const QString& caption,
				      QString *selectedFilter,
				      bool resolveSymlinks)
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = resolveSymlinks;

    QStringList filters;
    if ( !filter.isEmpty() )
	filters = makeFiltersList( filter );

    makeVariables();
    QString initialSelection;
    if ( !startWith.isEmpty() ) {
	QUrlOperator u( QFileDialogPrivate::encodeFileName( startWith ) );
	if ( u.isLocalFile() && QFileInfo( u.path() ).isDir() ) {
	    *workingDirectory = startWith;
	} else {
	    if ( u.isLocalFile() ) {
		QFileInfo fi( u.dirPath() );
		if ( fi.exists() ) {
		    *workingDirectory = u.dirPath();
		    initialSelection = u.fileName();
		}
	    } else {
		*workingDirectory = u.toString();
		initialSelection = QString::null;//u.fileName();
	    }
	}
    }

    if ( workingDirectory->isNull() )
	*workingDirectory = ::toRootIfNotExists( QDir::currentDirPath() );

#if defined(Q_WS_WIN)
    if ( qt_use_native_dialogs && qApp->style().styleHint( QStyle::SH_GUIStyle ) == WindowsStyle )
	return winGetSaveFileName( initialSelection, filter, workingDirectory,
				   parent, name, caption, selectedFilter );
#elif defined(Q_WS_MAC)
    if (qt_use_native_dialogs && (qApp->style().inherits(QMAC_DEFAULT_STYLE)
                                  || qApp->style().inherits("QMacStyle")))
        return qt_mac_precomposeFileName(macGetSaveFileName(initialSelection, filter,
                                         startWith.isEmpty() ? 0 : workingDirectory, parent, name,
                                         caption, selectedFilter));
#endif

    QFileDialog *dlg = new QFileDialog( *workingDirectory, QString::null, parent, name ? name : "qt_filedlg_gsfn", TRUE );

    Q_CHECK_PTR( dlg );
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( !caption.isNull() )
	dlg->setCaption( caption );
    else
	dlg->setCaption( QFileDialog::tr( "Save As" ) );
#endif

    QString result;
    dlg->setFilters( filters );
    if ( selectedFilter )
	dlg->setFilter( *selectedFilter );
    dlg->setMode( QFileDialog::AnyFile );
    if ( !initialSelection.isEmpty() )
	dlg->setSelection( initialSelection );
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	*workingDirectory = dlg->d->url;
	if ( selectedFilter )
	    *selectedFilter = dlg->selectedFilter();
    }
    delete dlg;

    qt_resolve_symlinks = save_qt_resolve_symlinks;

    return result;
}

/*!
  \internal
  Activated when the "OK" button is clicked.
*/

void QFileDialog::okClicked()
{
    QString fn( nameEdit->text() );

#if defined(Q_WS_WIN)
    QFileInfo fi( d->url.path() + fn );
    if ( fi.isSymLink() ) {
	nameEdit->setText( fi.readLink() );
    }
#endif

    if ( fn.contains("*") ) {
	addFilter( fn );
	nameEdit->blockSignals( TRUE );
	nameEdit->setText( QString::fromLatin1("") );
	nameEdit->blockSignals( FALSE );
	return;
    }

    *workingDirectory = d->url;
    detailViewMode = files->isVisible();
    updateLastSize(this);

    if ( isDirectoryMode( d->mode ) ) {
	QUrlInfo f( d->url, nameEdit->text() );
	if ( f.isDir() ) {
	    d->currentFileName = d->url;
	    if ( d->currentFileName.right(1) != "/" )
		d->currentFileName += '/';
	    if ( f.name() != "." )
		d->currentFileName += f.name();
	    accept();
	    return;
	}
	// Since it's not a directory and we clicked ok, we
	// don't really want to do anything else
	return;
    }

    // if we're in multi-selection mode and something is selected,
    // accept it and be done.
    if ( mode() == ExistingFiles ) {
	if ( ! nameEdit->text().isEmpty() ) {
	    QStringList sf = selectedFiles();
	    bool isdir = FALSE;
	    if ( sf.count() == 1 ) {
		QUrlOperator u( d->url, sf[0] );
		bool ok;
		isdir = u.isDir(&ok) && ok;
	    }
	    if ( !isdir ) {
		emit filesSelected( sf );
		accept();
		return;
	    }
	}
    }

    if ( mode() == AnyFile ) {
	QUrlOperator u( d->url, QFileDialogPrivate::encodeFileName(nameEdit->text()) );
	if ( !u.isDir() ) {
	    d->currentFileName = u;
	    emit fileSelected( selectedFile() );
	    accept();
	    return;
	}
    }

    if ( mode() == ExistingFile ) {
	if ( !QFileDialogPrivate::fileExists( d->url, nameEdit->text() ) )
	    return;
    }

    // If selection is valid, return it, else try
    // using selection as a directory to change to.
    if ( !d->currentFileName.isNull() && !d->currentFileName.contains( "*" ) ) {
	emit fileSelected( selectedFile() );
	accept();
    } else {
	QUrlInfo f;
	QFileDialogPrivate::File * c
	    = (QFileDialogPrivate::File *)files->currentItem();
	QFileDialogPrivate::MCItem * m
	    = (QFileDialogPrivate::MCItem *)d->moreFiles->item( d->moreFiles->currentItem() );
	if ( c && files->isVisible() && files->hasFocus() ||
	     m && d->moreFiles->isVisible() && d->moreFiles->hasFocus() ) {
	    if ( c && files->isVisible() )
		f = c->info;
	    else
		f = ( (QFileDialogPrivate::File*)m->i )->info;
	} else {
	    f = QUrlInfo( d->url, nameEdit->text() );
	}
	if ( f.isDir() ) {
	    setUrl( QUrlOperator( d->url, QFileDialogPrivate::encodeFileName(f.name() + "/" ) ) );
	    d->checkForFilter = TRUE;
	    trySetSelection( TRUE, d->url, TRUE );
	    d->checkForFilter = FALSE;
	} else {
	    if ( !nameEdit->text().contains( "/" ) &&
		 !nameEdit->text().contains( "\\" )
#if defined(Q_OS_WIN32)
		 && nameEdit->text()[ 1 ] != ':'
#endif
		 )
		addFilter( nameEdit->text() );
	    else if ( nameEdit->text()[ 0 ] == '/' ||
		      nameEdit->text()[ 0 ] == '\\'
#if defined(Q_OS_WIN32)
		      || nameEdit->text()[ 1 ] == ':'
#endif
		      )
		setDir( nameEdit->text() );
	    else if ( nameEdit->text().left( 3 ) == "../" || nameEdit->text().left( 3 ) == "..\\" )
		setDir( QUrl( d->url.toString(), QFileDialogPrivate::encodeFileName(nameEdit->text() ) ).toString() );
	}
	nameEdit->setText( "" );
    }
}

/*!
  \internal
  Activated when the "Filter" button is clicked.
*/

void QFileDialog::filterClicked()
{
    // unused
}

/*!
  \internal
  Activated when the "Cancel" button is clicked.
*/

void QFileDialog::cancelClicked()
{
    *workingDirectory = d->url;
    detailViewMode = files->isVisible();
    updateLastSize(this);
    reject();
}


/*!\reimp
*/

void QFileDialog::resizeEvent( QResizeEvent * e )
{
    QDialog::resizeEvent( e );
    updateGeometries();
}

/*
  \internal
  The only correct way to try to set currentFileName
*/
bool QFileDialog::trySetSelection( bool isDir, const QUrlOperator &u, bool updatelined )
{
    if ( !isDir && !u.path().isEmpty() && u.path().right( 1 ) == "/" )
	isDir = TRUE;
    if ( u.fileName().contains( "*") && d->checkForFilter ) {
	QString fn( u.fileName() );
	if ( fn.contains( "*" ) ) {
	    addFilter( fn );
	    d->currentFileName = QString::null;
	    d->url.setFileName( QString::null );
	    nameEdit->setText( QString::fromLatin1("") );
	    return FALSE;
	}
    }

    if ( d->preview && d->preview->isVisible() )
	updatePreviews( u );

    QString old = d->currentFileName;

    if ( isDirectoryMode( mode() ) ) {
	if ( isDir )
	    d->currentFileName = u;
	else
	    d->currentFileName = QString::null;
    } else if ( !isDir && mode() == ExistingFiles ) {
	d->currentFileName = u;
    } else if ( !isDir || ( mode() == AnyFile && !isDir ) ) {
	d->currentFileName = u;
    } else {
	d->currentFileName = QString::null;
    }
    if ( updatelined && !d->currentFileName.isEmpty() ) {
	// If the selection is valid, or if its a directory, allow OK.
	if ( !d->currentFileName.isNull() || isDir ) {
	    if ( u.fileName() != ".." ) {
		QString fn = u.fileName();
		nameEdit->setText( fn );
	    } else {
		nameEdit->setText("");
	    }
	} else
	    nameEdit->setText( QString::fromLatin1("") );
    }

    if ( !d->currentFileName.isNull() || isDir ) {
	okB->setEnabled( TRUE );
    } else if ( !isDirectoryMode( d->mode ) ) {
	okB->setEnabled( FALSE );
    }

    if ( d->currentFileName.length() && old != d->currentFileName )
	emit fileHighlighted( selectedFile() );

    return !d->currentFileName.isNull();
}


/*!  Make sure the minimum and maximum sizes of everything are sane.
*/

void QFileDialog::updateGeometries()
{
    if ( !d || !d->geometryDirty )
	return;

    d->geometryDirty = FALSE;

    QSize r, t;

    // we really should have a QSize::unite()
#define RM r.setWidth( QMAX(r.width(),t.width()) ); \
r.setHeight( QMAX(r.height(),t.height()) )

    // labels first
    r = d->pathL->sizeHint();
    t = d->fileL->sizeHint();
    RM;
    t = d->typeL->sizeHint();
    RM;
    d->pathL->setFixedSize( d->pathL->sizeHint() );
    d->fileL->setFixedSize( r );
    d->typeL->setFixedSize( r );

    // single-line input areas
    r = d->paths->sizeHint();
    t = nameEdit->sizeHint();
    RM;
    t = d->types->sizeHint();
    RM;
    r.setWidth( t.width() * 2 / 3 );
    t.setWidth( QWIDGETSIZE_MAX );
    t.setHeight( r.height() );
    d->paths->setMinimumSize( r );
    d->paths->setMaximumSize( t );
    nameEdit->setMinimumSize( r );
    nameEdit->setMaximumSize( t );
    d->types->setMinimumSize( r );
    d->types->setMaximumSize( t );

    // buttons on top row
    r = QSize( 0, d->paths->minimumSize().height() );
    t = QSize( 21, 20 );
    RM;
    if ( r.height()+1 > r.width() )
	r.setWidth( r.height()+1 );
    if ( d->goBack )
	d->goBack->setFixedSize( r );
    d->cdToParent->setFixedSize( r );
    d->newFolder->setFixedSize( r );
    d->mcView->setFixedSize( r );
    d->detailView->setFixedSize( r );

    QButton *b = 0;
    if ( !d->toolButtons.isEmpty() ) {
	for ( b = d->toolButtons.first(); b; b = d->toolButtons.next() )
	    b->setFixedSize( b->sizeHint().width(), r.height() );
    }

    if ( d->infoPreview ) {
	d->previewInfo->show();
	d->previewInfo->setFixedSize( r );
    } else {
	d->previewInfo->hide();
	d->previewInfo->setFixedSize( QSize( 0, 0 ) );
    }

    if ( d->contentsPreview ) {
	d->previewContents->show();
	d->previewContents->setFixedSize( r );
    } else {
	d->previewContents->hide();
	d->previewContents->setFixedSize( QSize( 0, 0 ) );
    }

    // open/save, cancel
    r = QSize( 75, 20 );
    t = okB->sizeHint();
    RM;
    t = cancelB->sizeHint();
    RM;

    okB->setFixedSize( r );
    cancelB->setFixedSize( r );

    d->topLevelLayout->activate();

#undef RM
}


/*! Updates the file name edit box to \a newItem in the file dialog
 when the cursor moves in the listview.
*/

void QFileDialog::updateFileNameEdit( QListViewItem * newItem )
{
    if ( !newItem )
	return;

    if ( mode() == ExistingFiles ) {
	detailViewSelectionChanged();
        QUrl u( d->url, QFileDialogPrivate::encodeFileName( ((QFileDialogPrivate::File*)files->currentItem())->info.name() ) );
	QFileInfo fi( u.toString( FALSE, FALSE ) );
	if ( !fi.isDir() )
	    emit fileHighlighted( u.toString( FALSE, FALSE ) );
    } else if ( files->isSelected( newItem ) ) {
	QFileDialogPrivate::File * i = (QFileDialogPrivate::File *)newItem;
	if ( i && i->i && !i->i->isSelected() ) {
	    d->moreFiles->blockSignals( TRUE );
	    d->moreFiles->setSelected( i->i, TRUE );
	    d->moreFiles->blockSignals( FALSE );
	}
	// Encode the filename in case it had any special characters in it
	QString encFile = QFileDialogPrivate::encodeFileName( newItem->text( 0 ) );
	trySetSelection( i->info.isDir(), QUrlOperator( d->url, encFile ), TRUE );
    }
}

void QFileDialog::detailViewSelectionChanged()
{
    if ( d->mode != ExistingFiles )
	return;

    nameEdit->clear();
    QString str;
    QListViewItem * i = files->firstChild();
    d->moreFiles->blockSignals( TRUE );
    while( i ) {
	if ( d->moreFiles && isVisible() ) {
	    QFileDialogPrivate::File *f = (QFileDialogPrivate::File *)i;
	    if ( f->i && f->i->isSelected() != i->isSelected() )
		d->moreFiles->setSelected( f->i, i->isSelected() );
	}
	if ( i->isSelected() && !( (QFileDialogPrivate::File *)i )->info.isDir() )
	    str += QString( "\"%1\" " ).arg( i->text( 0 ) );
	i = i->nextSibling();
    }
    d->moreFiles->blockSignals( FALSE );
    nameEdit->setText( str );
    nameEdit->setCursorPosition( str.length() );
    okB->setEnabled( TRUE );
    if ( d->preview && d->preview->isVisible() && files->currentItem() ) {
	QUrl u = QUrl( d->url, QFileDialogPrivate::encodeFileName( ((QFileDialogPrivate::File*)files->currentItem())->info.name() ) );
	updatePreviews( u );
    }
}

void QFileDialog::listBoxSelectionChanged()
{
    if ( d->mode != ExistingFiles )
	return;

    if ( d->ignoreNextRefresh ) {
	d->ignoreNextRefresh = FALSE;
	return;
    }

    nameEdit->clear();
    QString str;
    QListBoxItem * i = d->moreFiles->item( 0 );
    QListBoxItem * j = 0;
    int index = 0;
    files->blockSignals( TRUE );
    while( i ) {
	QFileDialogPrivate::MCItem *mcitem = (QFileDialogPrivate::MCItem *)i;
	if ( files && isVisible() ) {
	    if ( mcitem->i->isSelected() != mcitem->isSelected() ) {
		files->setSelected( mcitem->i, mcitem->isSelected() );

		// What happens here is that we want to emit signal highlighted for
		// newly added items.  But QListBox apparently emits selectionChanged even
		// when a user clicks on the same item twice.  So, basically emulate the behaivor
		// we have in the "Details" view which only emits highlighted the first time we
		// click on the item.  Perhaps at some point we should have a call to
		// updateFileNameEdit(QListViewItem) which also emits fileHighlighted() for
		// ExistingFiles.  For better or for worse, this clones the behaivor of the
		// "Details" view quite well.
		if ( mcitem->isSelected() && i != d->lastEFSelected ) {
		    QUrl u( d->url, QFileDialogPrivate::encodeFileName( ((QFileDialogPrivate::File*)(mcitem)->i)->info.name()) );
		    d->lastEFSelected = i;
		    emit fileHighlighted( u.toString(FALSE, FALSE) );
		}
	    }
	}
	if ( d->moreFiles->isSelected( i )
	     && !( (QFileDialogPrivate::File*)(mcitem)->i )->info.isDir() ) {
	    str += QString( "\"%1\" " ).arg( i->text() );
	    if ( j == 0 )
		j = i;
	}
	i = d->moreFiles->item( ++index );
    }

    files->blockSignals( FALSE );
    nameEdit->setText( str );
    nameEdit->setCursorPosition( str.length() );
    okB->setEnabled( TRUE );
    if ( d->preview && d->preview->isVisible() && j ) {
	QUrl u = QUrl( d->url,
		       QFileDialogPrivate::encodeFileName( ( (QFileDialogPrivate::File*)( (QFileDialogPrivate::MCItem*)j )->i )->info.name() ) );
	updatePreviews( u );
    }
}

/*! \overload */

void QFileDialog::updateFileNameEdit( QListBoxItem * newItem )
{
    if ( !newItem )
	return;
    QFileDialogPrivate::MCItem * i = (QFileDialogPrivate::MCItem *)newItem;
    if ( i->i ) {
	i->i->listView()->setSelected( i->i, i->isSelected() );
	updateFileNameEdit( i->i );
    }
}


/*!  Updates the dialog when the file name edit changes. */

void QFileDialog::fileNameEditDone()
{
    QUrlInfo f( d->url, nameEdit->text() );
    if ( mode() != QFileDialog::ExistingFiles ) {
	QUrlOperator u( d->url, QFileDialogPrivate::encodeFileName( nameEdit->text() ) );
	trySetSelection( f.isDir(), u, FALSE );
	if ( d->preview && d->preview->isVisible() )
	    updatePreviews( u );
    }
}



/*! This private slot reacts to double-clicks in the list view. The item that
was double-clicked is specified in \a newItem */

void QFileDialog::selectDirectoryOrFile( QListViewItem * newItem )
{

    *workingDirectory = d->url;
    detailViewMode = files->isVisible();
    updateLastSize(this);

    if ( !newItem )
	return;

    if ( d->url.isLocalFile() ) {
	QFileInfo fi( d->url.path() + newItem->text(0) );
#if defined(Q_WS_WIN)
	if ( fi.isSymLink() ) {
	    nameEdit->setText( fi.readLink() );
	    okClicked();
	    return;
	}
#endif
    }

    QFileDialogPrivate::File * i = (QFileDialogPrivate::File *)newItem;

    QString oldName = nameEdit->text();
    if ( i->info.isDir() ) {
	setUrl( QUrlOperator( d->url, QFileDialogPrivate::encodeFileName( i->info.name() ) + "/" ) );
	if ( isDirectoryMode( mode() ) ) {
	    QUrlInfo f ( d->url, QString::fromLatin1( "." ) );
	    trySetSelection( f.isDir(), d->url, TRUE );
	}
    } else if ( newItem->isSelectable() &&
		trySetSelection( i->info.isDir(), QUrlOperator( d->url, QFileDialogPrivate::encodeFileName( i->info.name() ) ), TRUE ) ) {
        if ( !isDirectoryMode( mode() ) ) {
	    if ( mode() == ExistingFile ) {
		if ( QFileDialogPrivate::fileExists( d->url, nameEdit->text() ) ) {
		    emit fileSelected( selectedFile() );
		    accept();
		}
	    } else {
		emit fileSelected( selectedFile() );
		accept();
	    }
	}
    } else if ( isDirectoryMode( d->mode ) ) {
	d->currentFileName = d->url;
	accept();
    }
    if ( !oldName.isEmpty() && !isDirectoryMode( mode() ) )
	nameEdit->setText( oldName );
}


void QFileDialog::selectDirectoryOrFile( QListBoxItem * newItem )
{
    if ( !newItem )
	return;

    QFileDialogPrivate::MCItem * i = (QFileDialogPrivate::MCItem *)newItem;
    if ( i->i ) {
	i->i->listView()->setSelected( i->i, i->isSelected() );
	selectDirectoryOrFile( i->i );
    }
}


void QFileDialog::popupContextMenu( QListViewItem *item, const QPoint &p,
				    int )
{
    if ( item ) {
	files->setCurrentItem( item );
	files->setSelected( item, TRUE );
    }

    PopupAction action;
    popupContextMenu( item ? item->text( 0 ) : QString::null, TRUE, action, p );

    if ( action == PA_Open )
	selectDirectoryOrFile( item );
    else if ( action == PA_Rename )
	files->startRename( FALSE );
    else if ( action == PA_Delete )
	deleteFile( item ? item->text( 0 ) : QString::null );
    else if ( action == PA_Reload )
	rereadDir();
    else if ( action == PA_Hidden ) {
	bShowHiddenFiles = !bShowHiddenFiles;
	rereadDir();
    } else if ( action == PA_SortName ) {
	sortFilesBy = (int)QDir::Name;
	sortAscending = TRUE;
	resortDir();
    } else if ( action == PA_SortSize ) {
	sortFilesBy = (int)QDir::Size;
	sortAscending = TRUE;
	resortDir();
    } else if ( action == PA_SortDate ) {
	sortFilesBy = (int)QDir::Time;
	sortAscending = TRUE;
	resortDir();
    } else if ( action == PA_SortUnsorted ) {
	sortFilesBy = (int)QDir::Unsorted;
	sortAscending = TRUE;
	resortDir();
    }

}

void QFileDialog::popupContextMenu( QListBoxItem *item, const QPoint & p )
{
    PopupAction action;
    popupContextMenu( item ? item->text() : QString::null, FALSE, action, p );

    if ( action == PA_Open )
	selectDirectoryOrFile( item );
    else if ( action == PA_Rename )
	d->moreFiles->startRename( FALSE );
    else if ( action == PA_Delete )
	deleteFile( item->text() );
    else if ( action == PA_Reload )
	rereadDir();
    else if ( action == PA_Hidden ) {
	bShowHiddenFiles = !bShowHiddenFiles;
	rereadDir();
    } else if ( action == PA_SortName ) {
	sortFilesBy = (int)QDir::Name;
	sortAscending = TRUE;
	resortDir();
    } else if ( action == PA_SortSize ) {
	sortFilesBy = (int)QDir::Size;
	sortAscending = TRUE;
	resortDir();
    } else if ( action == PA_SortDate ) {
	sortFilesBy = (int)QDir::Time;
	sortAscending = TRUE;
	resortDir();
    } else if ( action == PA_SortUnsorted ) {
	sortFilesBy = (int)QDir::Unsorted;
	sortAscending = TRUE;
	resortDir();
    }
}

void QFileDialog::popupContextMenu( const QString &filename, bool,
				    PopupAction &action, const QPoint &p )
{
    action = PA_Cancel;

    bool glob = filename.isEmpty();

    QPopupMenu m( 0, "file dialog context menu" );
    m.setCheckable( TRUE );

    if ( !glob ) {
	QString okt;
	if ( QUrlInfo( d->url, filename ).isDir() ) {
	    okt = tr( "&Open" );
	} else {
	    if ( mode() == AnyFile )
		okt = tr( "&Save" );
	    else
		okt = tr( "&Open" );
	}
	int ok = m.insertItem( okt );

	m.insertSeparator();
	int rename = m.insertItem( tr( "&Rename" ) );
	int del = m.insertItem( tr( "&Delete" ) );

	if ( filename.isEmpty() || !QUrlInfo( d->url, filename ).isWritable() ||
	     filename == ".." ) {
	    if ( filename.isEmpty() || !QUrlInfo( d->url, filename ).isReadable() )
		m.setItemEnabled( ok, FALSE );
	    m.setItemEnabled( rename, FALSE );
	    m.setItemEnabled( del, FALSE );
	}

	m.move( p );
	int res = m.exec();

	if ( res == ok )
	    action = PA_Open;
	else if ( res == rename )
	    action = PA_Rename;
	else if ( res == del )
	    action = PA_Delete;
    } else {
	int reload = m.insertItem( tr( "R&eload" ) );

	QPopupMenu m2( 0, "sort menu" );

	int sname = m2.insertItem( tr( "Sort by &Name" ) );
	//int stype = m2.insertItem( tr( "Sort by &Type" ) );
	int ssize = m2.insertItem( tr( "Sort by &Size" ) );
	int sdate = m2.insertItem( tr( "Sort by &Date" ) );
	m2.insertSeparator();
	int sunsorted = m2.insertItem( tr( "&Unsorted" ) );

	//m2.setItemEnabled( stype, FALSE );

	if ( sortFilesBy == (int)QDir::Name )
	    m2.setItemChecked( sname, TRUE );
	else if ( sortFilesBy == (int)QDir::Size )
	    m2.setItemChecked( ssize, TRUE );
//	else if ( sortFilesBy == 0x16 )
//	    m2.setItemChecked( stype, TRUE );
	else if ( sortFilesBy == (int)QDir::Time )
	    m2.setItemChecked( sdate, TRUE );
	else if ( sortFilesBy == (int)QDir::Unsorted )
	    m2.setItemChecked( sunsorted, TRUE );

	m.insertItem( tr( "Sort" ), &m2 );

	m.insertSeparator();

	int hidden = m.insertItem( tr( "Show &hidden files" ) );
	m.setItemChecked( hidden, bShowHiddenFiles );

	m.move( p );
	int res = m.exec();

	if ( res == reload )
	    action = PA_Reload;
	else if ( res == hidden )
	    action = PA_Hidden;
	else if ( res == sname )
	    action = PA_SortName;
//	else if ( res == stype )
//	    action = PA_SortType;
	else if ( res == sdate )
	    action = PA_SortDate;
	else if ( res == ssize )
	    action = PA_SortSize;
	else if ( res == sunsorted )
	    action = PA_SortUnsorted;
    }

}

void QFileDialog::deleteFile( const QString &filename )
{
    if ( filename.isEmpty() )
	return;

    QUrlInfo fi( d->url, QFileDialogPrivate::encodeFileName( filename ) );
    QString t = tr( "the file" );
    if ( fi.isDir() )
	t = tr( "the directory" );
    if ( fi.isSymLink() )
	t = tr( "the symlink" );

    if ( QMessageBox::warning( this,
			       tr( "Delete %1" ).arg( t ),
			       tr( "<qt>Are you sure you wish to delete %1 \"%2\"?</qt>" )
			       .arg( t ).arg(filename),
			       tr( "&Yes" ), tr( "&No" ), QString::null, 1 ) == 0 )
	d->url.remove( QFileDialogPrivate::encodeFileName( filename ) );

}

void QFileDialog::fileSelected( int  )
{
    // unused
}

void QFileDialog::fileHighlighted( int )
{
    // unused
}

void QFileDialog::dirSelected( int )
{
    // unused
}

void QFileDialog::pathSelected( int )
{
    // unused
}


void QFileDialog::cdUpClicked()
{
    QString oldName = nameEdit->text();
    setUrl( QUrlOperator( d->url, ".." ) );
    if ( !oldName.isEmpty() )
	nameEdit->setText( oldName );
}

void QFileDialog::newFolderClicked()
{
    QString foldername( tr( "New Folder 1" ) );
    int i = 0;
    QStringList lst;
    QListViewItemIterator it( files );
    for ( ; it.current(); ++it )
	if ( it.current()->text( 0 ).contains( tr( "New Folder" ) ) )
	    lst.append( it.current()->text( 0 ) );

    if ( !lst.count() == 0 )
	while ( lst.contains( foldername ) )
	    foldername = tr( "New Folder %1" ).arg( ++i );

    d->url.mkdir( foldername );
}

void QFileDialog::createdDirectory( const QUrlInfo &info, QNetworkOperation * )
{
    resortDir();
    if ( d->moreFiles->isVisible() ) {
	for ( uint i = 0; i < d->moreFiles->count(); ++i ) {
	    if ( d->moreFiles->text( i ) == info.name() ) {
		d->moreFiles->setCurrentItem( i );
		d->moreFiles->startRename( FALSE );
		break;
	    }
	}
    } else {
	QListViewItem *item = files->firstChild();
	while ( item ) {
	    if ( item->text( 0 ) == info.name() ) {
		files->setSelected( item, TRUE );
		files->setCurrentItem( item );
		files->startRename( FALSE );
		break;
	    }
	    item = item->nextSibling();
	}
    }
}


/*!
  This is a convenience static function that will return an existing directory
  selected by the user.

  \code
    QString s = QFileDialog::getExistingDirectory(
                    "/home",
		    this,
		    "get existing directory",
		    "Choose a directory",
		    TRUE );
  \endcode

  This function creates a modal file dialog called \a name, with
  parent, \a parent. If parent is not 0, the dialog will be shown
  centered over the parent.

  The dialog's working directory is set to \a dir, and the caption is
  set to \a caption. Either of these may be QString::null in which case
  the current directory and a default caption will be used respectively.

  Note on Windows that if \a dir is QString::null then the dialog's working
  directory will be set to the user's My Documents directory.

  If \a dirOnly is TRUE, then only directories will be shown in
  the file dialog; otherwise both directories and files will be shown.

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is FALSE, the file dialog will treat
  symlinks as regular directories.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style. (Note that on Windows the
  dialog will spin a blocking modal event loop that will not dispatch any
  QTimers and if parent is not 0 then it will position the dialog just under
  the parent's titlebar).

  \sa getOpenFileName(), getOpenFileNames(), getSaveFileName()
*/

QString QFileDialog::getExistingDirectory( const QString & dir,
					   QWidget *parent,
					   const char* name,
					   const QString& caption,
					   bool dirOnly,
					   bool resolveSymlinks)
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = resolveSymlinks;

    makeVariables();
    QString wd;
    if ( workingDirectory )
	wd = *workingDirectory;

#if defined(Q_WS_WIN)
    QString initialDir;
    if ( !dir.isEmpty() ) {
	QUrlOperator u( dir );
	if ( QFileInfo( u.path() ).isDir() )
	    initialDir = dir;
    } else
	initialDir = QString::null;
    if ( qt_use_native_dialogs && qApp->style().styleHint( QStyle::SH_GUIStyle ) == WindowsStyle && dirOnly )
        return winGetExistingDirectory( initialDir, parent, name, caption );
#endif
#if defined(Q_WS_MAC)
    QString *initialDir = 0;
    if (!dir.isEmpty()) {
        QUrlOperator u(dir);
        if (QFileInfo(u.path()).isDir())
            initialDir = (QString *)&dir;
    }
    if( qt_use_native_dialogs && (qApp->style().inherits(QMAC_DEFAULT_STYLE)
                                  || qApp->style().inherits("QMacStyle")))
	return qt_mac_precomposeFileName(macGetOpenFileNames("", initialDir, parent, name,
                                                      caption, 0, FALSE, TRUE).first());
#endif

    QFileDialog *dlg = new QFileDialog( parent, name ? name : "qt_filedlg_ged", TRUE );

    Q_CHECK_PTR( dlg );
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( !caption.isNull() )
	dlg->setCaption( caption );
    else
	dlg->setCaption( QFileDialog::tr("Find Directory") );
#endif

    dlg->setMode( dirOnly ? DirectoryOnly : Directory );

    dlg->d->types->clear();
    dlg->d->types->insertItem( QFileDialog::tr("Directories") );
    dlg->d->types->setEnabled( FALSE );

    QString dir_( dir );
    dir_ = dir_.simplifyWhiteSpace();
    if ( dir_.isEmpty() && !wd.isEmpty() )
	dir_ = wd;
    QUrlOperator u( dir_ );
    if ( u.isLocalFile() ) {
	if ( !dir_.isEmpty() ) {
	    QFileInfo f( u.path() );
	if ( f.exists() )
	if ( f.isDir() ) {
		dlg->setDir( dir_ );
		wd = dir_;
	    }
	} else if ( !wd.isEmpty() ) {
	    QUrl tempUrl( wd );
	    QFileInfo f( tempUrl.path() );
	    if ( f.isDir() ) {
		dlg->setDir( wd );
	    }
	} else {
	    QString theDir = dir_;
	    if ( theDir.isEmpty() ) {
		theDir = ::toRootIfNotExists( QDir::currentDirPath() );
	    } if ( !theDir.isEmpty() ) {
		QUrl tempUrl( theDir );
		QFileInfo f( tempUrl.path() );
		if ( f.isDir() ) {
		    wd = theDir;
		    dlg->setDir( theDir );
		}
	    }
	}
    } else {
	dlg->setUrl( dir_ );
    }

    QString result;
    dlg->setSelection( dlg->d->url.toString() );

    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->selectedFile();
	wd = result;
    }
    delete dlg;

    if ( !result.isEmpty() && result.right( 1 ) != "/" )
	result += "/";

    qt_resolve_symlinks = save_qt_resolve_symlinks;

    return result;
}


/*!
  \property QFileDialog::mode
  \brief the file dialog's mode

  The default mode is \c ExistingFile.
*/

void QFileDialog::setMode( Mode newMode )
{
    if ( d->mode != newMode ) {
	d->mode = newMode;
	QString sel = d->currentFileName;
	int maxnamelen = 255; // _POSIX_MAX_PATH
	if ( isDirectoryMode( newMode ) ) {
	    files->setSelectionMode( QListView::Single );
	    d->moreFiles->setSelectionMode( QListBox::Single );
	    if ( sel.isNull() )
		sel = QString::fromLatin1(".");
	    d->types->setEnabled( FALSE );
	} else if ( newMode == ExistingFiles ) {
	    maxnamelen = INT_MAX;
	    files->setSelectionMode( QListView::Extended );
	    d->moreFiles->setSelectionMode( QListBox::Extended );
	    d->types->setEnabled( TRUE );
	} else {
	    files->setSelectionMode( QListView::Single );
	    d->moreFiles->setSelectionMode( QListBox::Single );
	    d->types->setEnabled( TRUE );
	}
	nameEdit->setMaxLength(maxnamelen);
	rereadDir();
	QUrlInfo f( d->url, "." );
	trySetSelection( f.isDir(), d->url, FALSE );
    }

    QString okt;
    bool changeFilters = FALSE;
    if ( mode() == AnyFile ) {
	okt = tr("&Save");
	d->fileL->setText( tr("File &name:") );
	if ( d->types->count() == 1 ) {
	    d->types->setCurrentItem( 0 );
	    if ( d->types->currentText() == "Directories" ) {
		changeFilters = TRUE;
	    }
	}
    }
    else if ( mode() == Directory || mode() == DirectoryOnly ) {
	okt = tr("&OK");
	d->fileL->setText( tr("Directory:") );
	d->types->clear();
	d->types->insertItem( tr("Directories") );
    }
    else {
	okt = tr("&Open");
	d->fileL->setText( tr("File &name:") );
	if ( d->types->count() == 1 ) {
	    d->types->setCurrentItem( 0 );
	    if ( d->types->currentText() == "Directories" ) {
		changeFilters = TRUE;
	    }
	}
    }

    if ( changeFilters ) {
	d->types->clear();
	d->types->insertItem( tr("All Files (*)") );
    }

    okB->setText( okt );
}

QFileDialog::Mode QFileDialog::mode() const
{
    return d->mode;
}

/*! \reimp
*/

void QFileDialog::done( int i )
{
    if ( i == QDialog::Accepted && (d->mode == ExistingFile || d->mode == ExistingFiles) ) {
	QStringList selection = selectedFiles();
	for ( uint f = 0; f < selection.count(); f++ ) {
	    QString file = selection[f];
	    if ( file.isNull() )
		continue;
	    if ( d->url.isLocalFile() && !QFile::exists( file ) ) {
		QMessageBox::information( this, tr("Error"),
					  tr("%1\nFile not found.\nCheck path and filename.").arg( file ) );
		return;
	    }
	}
    }
    QDialog::done( i );
}

/*!
  \property QFileDialog::viewMode

  \brief the file dialog's view mode

  If you set the view mode to be \e Detail (the default), then you
  will see the file's details, such as the size of the file and the
  date the file was last modified in addition to the file's name.

  If you set the view mode to be \e List, then you will just
  see a list of the files and folders.

  See \l QFileDialog::ViewMode
*/


QFileDialog::ViewMode QFileDialog::viewMode() const
{
    if ( detailViewMode )
	return Detail;
    else
	return List;
}

void QFileDialog::setViewMode( ViewMode m )
{
    if ( m == Detail ) {
	detailViewMode = TRUE;
	d->stack->raiseWidget( files );
	d->detailView->setOn( TRUE );
	d->mcView->setOn( FALSE );
    } else if ( m == List ) {
	detailViewMode = FALSE;
	d->stack->raiseWidget( d->moreFiles );
	d->detailView->setOn( FALSE );
	d->mcView->setOn( TRUE );
    }
}


/*!
  \property QFileDialog::previewMode

  \brief the preview mode for the file dialog

  If you set the mode to be a mode other than \e NoPreview, you must
  use setInfoPreview() or setContentsPreview() to set the dialog's
  preview widget to your preview widget and enable the preview
  widget(s) with setInfoPreviewEnabled() or
  setContentsPreviewEnabled().

  \sa infoPreview, contentsPreview, viewMode
*/

void QFileDialog::setPreviewMode( PreviewMode m )
{
    if ( m == NoPreview ) {
	d->previewInfo->setOn( FALSE );
	d->previewContents->setOn( FALSE );
    } else if ( m == Info && d->infoPreview ) {
	d->previewInfo->setOn( TRUE );
	d->previewContents->setOn( FALSE );
	changeMode( d->modeButtons->id( d->previewInfo ) );
    } else if ( m == Contents && d->contentsPreview ) {
	d->previewInfo->setOn( FALSE );
	d->previewContents->setOn( TRUE );
	changeMode( d->modeButtons->id( d->previewContents ) );
    }
}
QFileDialog::PreviewMode QFileDialog::previewMode() const
{
    if ( d->infoPreview && d->infoPreviewWidget->isVisible() )
	return Info;
    else if ( d->contentsPreview && d->contentsPreviewWidget->isVisible() )
	return Contents;

    return NoPreview;
}


/*!
  Adds the specified widgets to the bottom of the file dialog. The
  label \a l is placed underneath the "file name" and the "file types"
  labels. The widget \a w is placed underneath the file types combobox.
  The button \a b is placed underneath the Cancel pushbutton.

  \code
    MyFileDialog::MyFileDialog( QWidget* parent, const char* name ) :
	QFileDialog( parent, name )
    {
	QLabel* label = new QLabel( "Added widgets", this );
	QLineEdit* lineedit = new QLineEdit( this );
	QPushButton* pushbutton = new QPushButton( this );

	addWidgets( label, lineedit, pushbutton );
    }
  \endcode

  If you don't want to have one of the widgets added, pass 0 in that
  widget's position.

  Every time you call this function, a new row of widgets will be added
  to the bottom of the file dialog.

  \sa addToolButton(), addLeftWidget(), addRightWidget()
*/

void QFileDialog::addWidgets( QLabel * l, QWidget * w, QPushButton * b )
{
    if ( !l && !w && !b )
	return;

    d->geometryDirty = TRUE;

    QHBoxLayout *lay = new QHBoxLayout();
    d->extraWidgetsLayouts.append( lay );
    d->topLevelLayout->addLayout( lay );

    if ( !l )
	l = new QLabel( this, "qt_intern_lbl" );
    d->extraLabels.append( l );
    lay->addWidget( l );

    if ( !w )
	w = new QWidget( this, "qt_intern_widget" );
    d->extraWidgets.append( w );
    lay->addWidget( w );
    lay->addSpacing( 15 );

    if ( b ) {
	d->extraButtons.append( b );
	lay->addWidget( b );
    } else {
	QWidget *wid = new QWidget( this, "qt_extrabuttons_widget" );
	d->extraButtons.append( wid );
	lay->addWidget( wid );
    }

    updateGeometries();
}

/*!
  Adds the tool button \a b to the row of tool buttons at the top of the
  file dialog. The button is appended to the right of
  this row. If \a separator is TRUE, a small space is inserted between the
  last button of the row and the new button \a b.

  \sa addWidgets(), addLeftWidget(), addRightWidget()
*/

void QFileDialog::addToolButton( QButton *b, bool separator )
{
    if ( !b || !d->buttonLayout )
	return;

    d->geometryDirty = TRUE;

    d->toolButtons.append( b );
    if ( separator )
	d->buttonLayout->addSpacing( 8 );
    d->buttonLayout->addWidget( b );

    updateGeometries();
}

/*!
  Adds the widget \a w to the left-hand side of the file dialog.

  \sa addRightWidget(), addWidgets(), addToolButton()
*/

void QFileDialog::addLeftWidget( QWidget *w )
{
    if ( !w )
	return;
    d->geometryDirty = TRUE;

    d->leftLayout->addWidget( w );
    d->leftLayout->addSpacing( 5 );

    updateGeometries();
}

/*!
  Adds the widget \a w to the right-hand side of the file dialog.

  \sa addLeftWidget(), addWidgets(), addToolButton()
*/

void QFileDialog::addRightWidget( QWidget *w )
{
    if ( !w )
	return;
    d->geometryDirty = TRUE;

    d->rightLayout->addSpacing( 5 );
    d->rightLayout->addWidget( w );

    updateGeometries();
}

/*! \reimp */

void QFileDialog::keyPressEvent( QKeyEvent * ke )
{
    if ( !d->ignoreNextKeyPress &&
	 ke && ( ke->key() == Key_Enter ||
		 ke->key() == Key_Return ) ) {
	ke->ignore();
	if ( d->paths->hasFocus() ) {
	    ke->accept();
	    if ( d->url == QUrl(d->paths->currentText()) )
		nameEdit->setFocus();
	} else if ( d->types->hasFocus() ) {
	    ke->accept();
	    // ### is there a suitable condition for this?  only valid
	    // wildcards?
	    nameEdit->setFocus();
	} else if ( nameEdit->hasFocus() ) {
	    if ( d->currentFileName.isNull() ) {
		// maybe change directory
		QUrlInfo i( d->url, nameEdit->text() );
		if ( i.isDir() ) {
		    nameEdit->setText( QString::fromLatin1("") );
		    setDir( QUrlOperator( d->url, QFileDialogPrivate::encodeFileName(i.name()) ) );
		}
		ke->accept();
	    } else if ( mode() == ExistingFiles ) {
		QUrlInfo i( d->url, nameEdit->text() );
		if ( i.isFile() ) {
		    QListViewItem * i = files->firstChild();
		    while ( i && nameEdit->text() != i->text( 0 ) )
			i = i->nextSibling();
		    if ( i )
			files->setSelected( i, TRUE );
		    else
			ke->accept(); // strangely, means to ignore that event
		}
	    }
	} else if ( files->hasFocus() || d->moreFiles->hasFocus() ) {
	    ke->accept();
	}
    } else if ( ke->key() == Key_Escape ) {
	ke->ignore();
    }

    d->ignoreNextKeyPress = FALSE;

    if ( !ke->isAccepted() ) {
	QDialog::keyPressEvent( ke );
    }
}


/*! \class QFileIconProvider qfiledialog.h

  \brief The QFileIconProvider class provides icons for QFileDialog to
  use.

  \ingroup misc

  By default QFileIconProvider is not used, but any application or
  library can subclass it, reimplement pixmap() to return a suitable
  icon, and make all QFileDialog objects use it by calling the static
  function QFileDialog::setIconProvider().

  It is advisable to make all the icons that QFileIconProvider returns be
  the same size or at least the same width. This makes the list view
  look much better.

  \sa QFileDialog
*/


/*! Constructs an empty file icon provider called \a name, with the
  parent \a parent.
*/

QFileIconProvider::QFileIconProvider( QObject * parent, const char* name )
    : QObject( parent, name )
{
    // nothing necessary
}


/*!
  Returns a pointer to a pixmap that should be used to
  signify the file with the information \a info.

  If pixmap() returns 0, QFileDialog draws the default pixmap.

  The default implementation returns particular icons for files, directories,
  link-files and link-directories. It returns a blank "icon" for other types.

  If you return a pixmap here, it should measure 16x16 pixels.
*/

const QPixmap * QFileIconProvider::pixmap( const QFileInfo & info )
{
    if ( info.isSymLink() ) {
	if ( info.isFile() )
	    return symLinkFileIcon;
	else
	    return symLinkDirIcon;
    } else if ( info.isDir() ) {
	return closedFolderIcon;
    } else if ( info.isFile() ) {
	return fileIcon;
    } else {
	return fifteenTransparentPixels;
    }
}

/*!
  Sets the QFileIconProvider used by the file dialog to \a provider.

  The default is that there is no QFileIconProvider and QFileDialog
  just draws a folder icon next to each directory and nothing next
  to files.

  \sa QFileIconProvider, iconProvider()
*/

void QFileDialog::setIconProvider( QFileIconProvider * provider )
{
    fileIconProvider = provider;
}


/*!
  Returns a pointer to the icon provider currently set on the file dialog.
  By default there is no icon provider, and this function returns 0.

  \sa setIconProvider(), QFileIconProvider
*/

QFileIconProvider * QFileDialog::iconProvider()
{
    return fileIconProvider;
}


#if defined(Q_WS_WIN)

// ### FIXME: this code is duplicated in qdns.cpp
static QString getWindowsRegString( HKEY key, const QString &subKey )
{
    QString s;
    QT_WA( {
	char buf[1024];
	DWORD bsz = sizeof(buf);
	int r = RegQueryValueEx( key, (TCHAR*)subKey.ucs2(), 0, 0, (LPBYTE)buf, &bsz );
	if ( r == ERROR_SUCCESS ) {
	    s = QString::fromUcs2( (unsigned short *)buf );
	} else if ( r == ERROR_MORE_DATA ) {
	    char *ptr = new char[bsz+1];
	    r = RegQueryValueEx( key, (TCHAR*)subKey.ucs2(), 0, 0, (LPBYTE)ptr, &bsz );
	    if ( r == ERROR_SUCCESS )
		s = ptr;
	    delete [] ptr;
	}
    } , {
	char buf[512];
	DWORD bsz = sizeof(buf);
	int r = RegQueryValueExA( key, subKey.local8Bit(), 0, 0, (LPBYTE)buf, &bsz );
	if ( r == ERROR_SUCCESS ) {
	    s = buf;
	} else if ( r == ERROR_MORE_DATA ) {
	    char *ptr = new char[bsz+1];
	    r = RegQueryValueExA( key, subKey.local8Bit(), 0, 0, (LPBYTE)ptr, &bsz );
	    if ( r == ERROR_SUCCESS )
		s = ptr;
	    delete [] ptr;
	}
    } );
    return s;
}

static void initPixmap( QPixmap &pm )
{
    pm.fill( Qt::white );
}


QWindowsIconProvider::QWindowsIconProvider( QObject *parent, const char *name )
    : QFileIconProvider( parent, name )
{
    pixw = GetSystemMetrics( SM_CXSMICON );
    pixh = GetSystemMetrics( SM_CYSMICON );

    HKEY k;
    HICON si;
    int r;
    QString s;
    UINT res = 0;

    // ---------- get default folder pixmap
    const wchar_t iconFolder[] = L"folder\\DefaultIcon"; // workaround for Borland
    QT_WA( {
	r = RegOpenKeyEx( HKEY_CLASSES_ROOT,
			   iconFolder,
			   0, KEY_READ, &k );
    } , {
	r = RegOpenKeyExA( HKEY_CLASSES_ROOT,
			   "folder\\DefaultIcon",
			   0, KEY_READ, &k );
    } );
    resolveLibs();
    if ( r == ERROR_SUCCESS ) {
	s = getWindowsRegString( k, QString::null );
	RegCloseKey( k );

	QStringList lst = QStringList::split( ",", s );
        
    if (lst.count() >= 2) { // don't just assume that lst has two entries
#ifndef Q_OS_TEMP
            QT_WA( {
                    res = ptrExtractIconEx( (TCHAR*)lst[ 0 ].simplifyWhiteSpace().ucs2(),
                        lst[ 1 ].simplifyWhiteSpace().toInt(),
                        0, &si, 1 );
                    } , {
                    res = ExtractIconExA( lst[ 0 ].simplifyWhiteSpace().local8Bit(),
                        lst[ 1 ].simplifyWhiteSpace().toInt(),
                        0, &si, 1 );
                    } );
#else
            res = (UINT)ExtractIconEx( (TCHAR*)lst[ 0 ].simplifyWhiteSpace().ucs2(),
                    lst[ 1 ].simplifyWhiteSpace().toInt(),
                    0, &si, 1 );
#endif
    }
	if ( res ) {
	    defaultFolder.resize( pixw, pixh );
	    initPixmap( defaultFolder );
	    QPainter p( &defaultFolder );
	    DrawIconEx( p.handle(), 0, 0, si, pixw, pixh, 0, 0,  DI_NORMAL );
	    p.end();
	    defaultFolder.setMask( defaultFolder.createHeuristicMask() );
	    *closedFolderIcon = defaultFolder;
	    DestroyIcon( si );
	} else {
	    defaultFolder = *closedFolderIcon;
	}
    } else {
	RegCloseKey( k );
    }

    //------------------------------- get default file pixmap
#ifndef Q_OS_TEMP
    QT_WA( {
	res = ptrExtractIconEx( L"shell32.dll",
				 0, 0, &si, 1 );
    } , {
	res = ExtractIconExA( "shell32.dll",
				 0, 0, &si, 1 );
    } );
#else
	res = (UINT)ExtractIconEx( L"shell32.dll",
				    0, 0, &si, 1 );
#endif

    if ( res ) {
	defaultFile.resize( pixw, pixh );
	initPixmap( defaultFile );
	QPainter p( &defaultFile );
	DrawIconEx( p.handle(), 0, 0, si, pixw, pixh, 0, 0,  DI_NORMAL );
	p.end();
	defaultFile.setMask( defaultFile.createHeuristicMask() );
	*fileIcon = defaultFile;
	DestroyIcon( si );
    } else {
	defaultFile = *fileIcon;
    }

    //------------------------------- get default exe pixmap
#ifndef Q_OS_TEMP
    QT_WA( {
	res = ptrExtractIconEx( L"shell32.dll",
			      2, 0, &si, 1 );
    } , {
	res = ExtractIconExA( "shell32.dll",
			  2, 0, &si, 1 );
    } );
#else
	res = (UINT)ExtractIconEx( L"ceshell.dll",
				    10, 0, &si, 1 );
#endif

    if ( res ) {
	defaultExe.resize( pixw, pixh );
	initPixmap( defaultExe );
	QPainter p( &defaultExe );
	DrawIconEx( p.handle(), 0, 0, si, pixw, pixh, 0, 0,  DI_NORMAL );
	p.end();
	defaultExe.setMask( defaultExe.createHeuristicMask() );
	DestroyIcon( si );
    } else {
	defaultExe = *fileIcon;
    }
}

QWindowsIconProvider::~QWindowsIconProvider()
{
    if ( this == fileIconProvider )
	fileIconProvider = 0;
}

const QPixmap * QWindowsIconProvider::pixmap( const QFileInfo &fi )
{
    if (fi.isSymLink()) {
        QString real = fi.readLink();
        if (!real.isEmpty())
            return pixmap(QFileInfo(real));
    }

    QString ext = fi.extension( FALSE ).upper();
    QString key = ext;
    ext.prepend( "." );
    QMap< QString, QPixmap >::Iterator it;

    if ( fi.isDir() ) {
	return &defaultFolder;
    } else if ( ext != ".EXE" ) {
	it = cache.find( key );
	if ( it != cache.end() )
	    return &( *it );

	HKEY k, k2;
	int r;
	QT_WA( {
	    r = RegOpenKeyEx( HKEY_CLASSES_ROOT, (TCHAR*)ext.ucs2(),
			      0, KEY_READ, &k );
	} , {
	    r = RegOpenKeyExA( HKEY_CLASSES_ROOT, ext.local8Bit(),
			       0, KEY_READ, &k );
	} );
	QString s;
	if ( r == ERROR_SUCCESS ) {
	    s = getWindowsRegString( k, QString::null );
	} else {
	    cache[ key ] = defaultFile;
	    RegCloseKey( k );
	    return &defaultFile;
	}
	RegCloseKey( k );

	QT_WA( {
	    r = RegOpenKeyEx( HKEY_CLASSES_ROOT, (TCHAR*)QString( s + "\\DefaultIcon" ).ucs2(),
			       0, KEY_READ, &k2 );
	} , {
	    r = RegOpenKeyExA( HKEY_CLASSES_ROOT, QString( s + "\\DefaultIcon" ).local8Bit() ,
	    		       0, KEY_READ, &k2 );
	} );
	if ( r == ERROR_SUCCESS ) {
	    s = getWindowsRegString( k2, QString::null );
	} else {
	    cache[ key ] = defaultFile;
	    RegCloseKey( k2 );
	    return &defaultFile;
	}
	RegCloseKey( k2 );

	QStringList lst = QStringList::split( ",", s );

        HICON si;
        UINT res = 0;
        if (lst.count() >= 2) { // don't just assume that lst has two entries
            QString filepath = lst[ 0 ].stripWhiteSpace();
            if ( !filepath.isEmpty() ) {
                if ( filepath.find("%1") != -1 ) {
                    filepath = filepath.arg( fi.filePath() );
                    if ( ext == ".DLL" ) {
                        pix = defaultFile;
                        return &pix;
                    }
                }
                if ( filepath[0] == '"' && filepath[(int)filepath.length()-1] == '"' )
                    filepath = filepath.mid( 1, filepath.length()-2 );

                resolveLibs();
#ifndef Q_OS_TEMP
                QT_WA( {
                        res = ptrExtractIconEx( (TCHAR*)filepath.ucs2(), lst[ 1 ].stripWhiteSpace().toInt(),
                            0, &si, 1 );
                        } , {
                        res = ExtractIconExA( filepath.local8Bit(), lst[ 1 ].stripWhiteSpace().toInt(),
                            0, &si, 1 );
                        } );
#else
                res = (UINT)ExtractIconEx( (TCHAR*)filepath.ucs2(), lst[ 1 ].stripWhiteSpace().toInt(),
                        0, &si, 1 );
#endif
            }
        }

	if ( res ) {
	    pix.resize( pixw, pixh );
	    initPixmap( pix );
	    QPainter p( &pix );
	    DrawIconEx( p.handle(), 0, 0, si, pixw, pixh, 0, 0,  DI_NORMAL );
	    p.end();
	    pix.setMask( pix.createHeuristicMask() );
	    DestroyIcon( si );
	} else {
	    pix = defaultFile;
	}

	cache[ key ] = pix;
	return &pix;
    } else {
	HICON si;
	UINT res = 0;
	if ( !fi.absFilePath().isEmpty() ) {
#ifndef Q_OS_TEMP
	    QT_WA( {
		res = ptrExtractIconEx( (TCHAR*)fi.absFilePath().ucs2(), -1,
				      0, 0, 1 );
	    } , {
		res = ExtractIconExA( fi.absFilePath().local8Bit(), -1,
				      0, 0, 1 );
	    } );

	    if ( res ) {
		QT_WA( {
		    res = ptrExtractIconEx( (TCHAR*)fi.absFilePath().ucs2(), res - 1,
					  0, &si, 1 );
		} , {
		    res = ExtractIconExA( fi.absFilePath().local8Bit(), res - 1,
					  0, &si, 1 );
		} );
	    }
#else
		res = (UINT)ExtractIconEx( (TCHAR*)fi.absFilePath().ucs2(), -1,
					    0, 0, 1 );
		if ( res )
		    res = (UINT)ExtractIconEx( (TCHAR*)fi.absFilePath().ucs2(), res - 1,
						0, &si, 1 );
#endif

	}

	if ( res ) {
	    pix.resize( pixw, pixh );
	    initPixmap( pix );
	    QPainter p( &pix );
	    DrawIconEx( p.handle(), 0, 0, si, pixw, pixh, 0, 0,  DI_NORMAL );
	    p.end();
	    pix.setMask( pix.createHeuristicMask() );
	    DestroyIcon( si );
	} else {
	    pix = defaultExe;
	}

	return &pix;
    }

    // can't happen!
    return 0;
}
#endif



/*!
  \reimp
*/
bool QFileDialog::eventFilter( QObject * o, QEvent * e )
{
    if ( e->type() == QEvent::KeyPress && ( (QKeyEvent*)e )->key() == Key_F5 ) {
	rereadDir();
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( e->type() == QEvent::KeyPress && ( (QKeyEvent*)e )->key() == Key_F2 &&
		( o == files || o == files->viewport() ) ) {
	if ( files->isVisible() && files->currentItem() ) {
	    if ( QUrlInfo( d->url, "." ).isWritable() && files->currentItem()->text( 0 ) != ".." ) {
		files->renameItem = files->currentItem();
		files->startRename( TRUE );
	    }
	}
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( e->type() == QEvent::KeyPress && ( (QKeyEvent*)e )->key() == Key_F2 &&
		( o == d->moreFiles || o == d->moreFiles->viewport() ) ) {
	if ( d->moreFiles->isVisible() && d->moreFiles->currentItem() != -1 ) {
	    if ( QUrlInfo( d->url, "." ).isWritable() &&
		 d->moreFiles->item( d->moreFiles->currentItem() )->text() != ".." ) {
		d->moreFiles->renameItem = d->moreFiles->item( d->moreFiles->currentItem() );
		d->moreFiles->startRename( TRUE );
	    }
	}
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( e->type() == QEvent::KeyPress && d->moreFiles->renaming ) {
	d->moreFiles->lined->setFocus();
	QApplication::sendEvent( d->moreFiles->lined, e );
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( e->type() == QEvent::KeyPress && files->renaming ) {
	files->lined->setFocus();
	QApplication::sendEvent( files->lined, e );
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( e->type() == QEvent::KeyPress &&
		((QKeyEvent *)e)->key() == Key_Backspace &&
		( o == files ||
		  o == d->moreFiles ||
		  o == files->viewport() ||
		  o == d->moreFiles->viewport() ) ) {
	cdUpClicked();
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( e->type() == QEvent::KeyPress &&
		((QKeyEvent *)e)->key() == Key_Delete &&
		( o == files ||
		  o == files->viewport() ) ) {
	if ( files->currentItem() )
	    deleteFile( files->currentItem()->text( 0 ) );
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( e->type() == QEvent::KeyPress &&
		((QKeyEvent *)e)->key() == Key_Delete &&
		( o == d->moreFiles ||
		  o == d->moreFiles->viewport() ) ) {
	int c = d->moreFiles->currentItem();
	if ( c >= 0 )
	    deleteFile( d->moreFiles->item( c )->text() );
	((QKeyEvent *)e)->accept();
	return TRUE;
    } else if ( o == files && e->type() == QEvent::FocusOut && files->currentItem() ) {
    } else if ( o == files && e->type() == QEvent::KeyPress ) {
	QTimer::singleShot( 0, this, SLOT(fixupNameEdit()) );
    } else if ( o == nameEdit && e->type() == QEvent::KeyPress && d->mode != AnyFile ) {
	if ( ( nameEdit->cursorPosition() == (int)nameEdit->text().length() || nameEdit->hasSelectedText() ) &&
	     isprint(((QKeyEvent *)e)->ascii()) ) {
#if defined(Q_WS_WIN)
	    QString nt( nameEdit->text().lower() );
#else
	    QString nt( nameEdit->text() );
#endif
	    nt.truncate( nameEdit->cursorPosition() );
	    nt += (char)(((QKeyEvent *)e)->ascii());
	    QListViewItem * i = files->firstChild();
#if defined(Q_WS_WIN)
	    while( i && i->text( 0 ).left(nt.length()).lower() != nt )
#else
	    while( i && i->text( 0 ).left(nt.length()) != nt )
#endif
		i = i->nextSibling();
	    if ( i ) {
		nt = i->text( 0 );
		int cp = nameEdit->cursorPosition()+1;
		nameEdit->validateAndSet( nt, cp, cp, nt.length() );
		return TRUE;
	    }
	}
    } else if ( o == nameEdit && e->type() == QEvent::FocusIn ) {
	fileNameEditDone();
    } else if ( d->moreFiles->renaming && o != d->moreFiles->lined && e->type() == QEvent::FocusIn ) {
	d->moreFiles->lined->setFocus();
	return TRUE;
    } else if ( files->renaming && o != files->lined && e->type() == QEvent::FocusIn ) {
	files->lined->setFocus();
	return TRUE;
    } else if ( ( o == d->moreFiles || o == d->moreFiles->viewport() ) &&
		e->type() == QEvent::FocusIn ) {
	if ( o == d->moreFiles->viewport() && !d->moreFiles->viewport()->hasFocus() ||
	     o == d->moreFiles && !d->moreFiles->hasFocus() )
	    ((QWidget*)o)->setFocus();
	return FALSE;
    }

    return QDialog::eventFilter( o, e );
}

/*!
  Sets the filters used in the file dialog to \a filters. Each group
  of filters must be separated by \c{;;} (\e two semi-colons).

  \code
    QString types("Image files (*.png *.xpm *.jpg);;"
                  "Text files (*.txt);;"
		  "Any files (*)");
    QFileDialog fd = new QFileDialog( this );
    fd->setFilters( types );
    fd->show();
  \endcode

*/

void QFileDialog::setFilters( const QString &filters )
{
    QStringList lst = makeFiltersList( filters );
    setFilters( lst );
}

/*!
  \overload

  \a types must be a null-terminated list of strings.

*/

void QFileDialog::setFilters( const char ** types )
{
    if ( !types || !*types )
	return;

    d->types->clear();
    while( types && *types ) {
	d->types->insertItem( QString::fromLatin1(*types) );
	types++;
    }
    d->types->setCurrentItem( 0 );
    setFilter( d->types->text( 0 ) );
}


/*! \overload void QFileDialog::setFilters( const QStringList & )
*/

void QFileDialog::setFilters( const QStringList & types )
{
    if ( types.count() < 1 )
	return;

    d->types->clear();
    for ( QStringList::ConstIterator it = types.begin(); it != types.end(); ++it )
	d->types->insertItem( *it );
    d->types->setCurrentItem( 0 );
    setFilter( d->types->text( 0 ) );
}

/*!
  Adds the filter \a filter to the list of filters and makes it the
  current filter.

  \code
    QFileDialog* fd = new QFileDialog( this );
    fd->addFilter( "Images (*.png *.jpg *.xpm)" );
    fd->show();
  \endcode

  In the above example, a file dialog is created, and the file filter "Images
  (*.png *.jpg *.xpm)" is added and is set as the current filter. The original
  filter, "All Files (*)", is still available.

  \sa setFilter(), setFilters()
*/

void QFileDialog::addFilter( const QString &filter )
{
    if ( filter.isEmpty() )
	return;
    QString f = filter;
    QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
    int index = r.search( f );
    if ( index >= 0 )
	f = r.cap( 2 );
    for ( int i = 0; i < d->types->count(); ++i ) {
	QString f2( d->types->text( i ) );
	int index = r.search( f2 );
	if ( index >= 0 )
	    f2 = r.cap( 1 );
	if ( f2 == f ) {
	    d->types->setCurrentItem( i );
	    setFilter( f2 );
	    return;
	}
    }

    d->types->insertItem( filter );
    d->types->setCurrentItem( d->types->count() - 1 );
    setFilter( d->types->text( d->types->count() - 1 ) );
}

/*!
  Since modeButtons is a top-level widget, it may be destroyed by the
  kernel at application exit. Notice if this happens to
  avoid double deletion.
*/

void QFileDialog::modeButtonsDestroyed()
{
    if ( d )
	d->modeButtons = 0;
}


/*!
  This is a convenience static function that will return one or more
  existing files selected by the user.

  \code
    QStringList files = QFileDialog::getOpenFileNames(
			    "Images (*.png *.xpm *.jpg)",
			    "/home",
			    this,
			    "open files dialog",
			    "Select one or more files to open" );
  \endcode

  This function creates a modal file dialog called \a name, with
  parent \a parent. If \a parent is not 0, the dialog will be shown
  centered over the parent.

  The file dialog's working directory will be set to \a dir. If \a
  dir includes a file name, the file will be selected. The filter
  is set to \a filter so that only those files which match the filter
  are shown. The filter selected is set to \a selectedFilter. The parameters
  \a dir, \a selectedFilter and \a filter may be QString::null.

  The dialog's caption is set to \a caption. If \a caption is not
  specified then a default caption will be used.

  Under Windows and Mac OS X, this static function will use the native
  file dialog and not a QFileDialog, unless the style of the application
  is set to something other than the native style. (Note that on Windows the
  dialog will spin a blocking modal event loop that will not dispatch any
  QTimers and if parent is not 0 then it will position the dialog just under
  the parent's titlebar).

  Under Unix/X11, the normal behavior of the file dialog is to resolve
  and follow symlinks. For example, if /usr/tmp is a symlink to /var/tmp,
  the file dialog will change to /var/tmp after entering /usr/tmp.
  If \a resolveSymlinks is FALSE, the file dialog will treat
  symlinks as regular directories.

  Note that if you want to iterate over the list of files, you should
  iterate over a copy, e.g.
    \code
    QStringList list = files;
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa getOpenFileName(), getSaveFileName(), getExistingDirectory()
*/

QStringList QFileDialog::getOpenFileNames( const QString & filter,
					   const QString& dir,
					   QWidget *parent,
					   const char* name,
					   const QString& caption,
					   QString *selectedFilter,
					   bool resolveSymlinks )
{
    bool save_qt_resolve_symlinks = qt_resolve_symlinks;
    qt_resolve_symlinks = resolveSymlinks;

    QStringList filters;
    if ( !filter.isEmpty() )
	filters = makeFiltersList( filter );

    makeVariables();

    if ( workingDirectory->isNull() )
	*workingDirectory = ::toRootIfNotExists( QDir::currentDirPath() );

    if ( !dir.isEmpty() ) {
	// #### works only correct for local files
	QUrlOperator u( QFileDialogPrivate::encodeFileName(dir) );
	if ( u.isLocalFile() && QFileInfo( u.path() ).isDir() ) {
	    *workingDirectory = dir;
	} else {
	    *workingDirectory = u.toString();
	}
    }

#if defined(Q_WS_WIN)
    if ( qt_use_native_dialogs && qApp->style().styleHint( QStyle::SH_GUIStyle ) == WindowsStyle )
	return winGetOpenFileNames( filter, workingDirectory, parent, name, caption, selectedFilter );
#elif defined(Q_WS_MAC)
    if (qt_use_native_dialogs && (qApp->style().inherits(QMAC_DEFAULT_STYLE)
                                  || qApp->style().inherits("QMacStyle"))) {
        QStringList sl = macGetOpenFileNames(filter, dir.isEmpty() ? 0 : workingDirectory, parent,
                                             name, caption, selectedFilter);
        QStringList::iterator it = sl.begin();
        while (it != sl.end()) {
            *it = qt_mac_precomposeFileName(*it);
            ++it;
        }
	return sl;
    }
#endif

    QFileDialog *dlg = new QFileDialog( *workingDirectory, QString::null, parent, name ? name : "qt_filedlg_gofns", TRUE );

    Q_CHECK_PTR( dlg );
#ifndef QT_NO_WIDGET_TOPEXTRA
    if ( !caption.isNull() )
	dlg->setCaption( caption );
    else
	dlg->setCaption( QFileDialog::tr("Open") );
#endif

    dlg->setFilters( filters );
    if ( selectedFilter )
	dlg->setFilter( *selectedFilter );
    dlg->setMode( QFileDialog::ExistingFiles );
    QString result;
    QStringList lst;
    if ( dlg->exec() == QDialog::Accepted ) {
	lst = dlg->selectedFiles();
	*workingDirectory = dlg->d->url;
	if ( selectedFilter )
	    *selectedFilter = dlg->selectedFilter();
    }
    delete dlg;

    qt_resolve_symlinks = save_qt_resolve_symlinks;
    return lst;
}

/*!  Updates the line edit to match the speed-key usage in QListView. */

void QFileDialog::fixupNameEdit()
{
    if ( files->currentItem() ) {
	if ( ( (QFileDialogPrivate::File*)files->currentItem() )->info.isFile() )
	    nameEdit->setText( files->currentItem()->text( 0 ) );
    }
}

/*!
  Returns the URL of the current working directory in the file dialog.

  \sa setUrl()
*/

QUrl QFileDialog::url() const
{
    return d->url;
}

#if defined(Q_CYGWIN_WIN)
Q_EXPORT int qt_ntfs_permission_lookup = 0;
#endif

static bool isRoot( const QUrl &u )
{
#if defined(Q_OS_MAC9)
     QString p = QDir::convertSeparators(u.path());
     if(p.contains(':') == 1)
	return TRUE;
#elif defined(Q_OS_UNIX)
    if ( u.path() == "/" )
	return TRUE;
#elif defined(Q_OS_WIN32)
    QString p = u.path();
    if ( p.length() == 3 &&
	 p.right( 2 ) == ":/" )
	return TRUE;
    if ( p[ 0 ] == '/' && p[ 1 ] == '/' ) {
	int slashes = p.contains( '/' );
	if ( slashes <= 3 )
	    return TRUE;
	if ( slashes == 4 && p[ (int)p.length() - 1 ] == '/' )
	    return TRUE;
    }
#else
#if defined(Q_CC_GNU)
#warning "case not covered.."
#endif
#endif

    if ( !u.isLocalFile() && u.path() == "/" )
	return TRUE;

    return FALSE;
}

void QFileDialog::urlStart( QNetworkOperation *op )
{
    if ( !op )
	return;

#if defined(Q_WS_WIN)
    qt_ntfs_permission_lookup--;
#endif
    if ( op->operation() == QNetworkProtocol::OpListChildren ) {
#ifndef QT_NO_CURSOR
	if ( !d->cursorOverride ) {
	    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
	    d->cursorOverride = TRUE;
	}
#endif
	if ( isRoot( d->url ) )
	    d->cdToParent->setEnabled( FALSE );
	else
	    d->cdToParent->setEnabled( TRUE );
	d->mimeTypeTimer->stop();
	d->sortedList.clear();
	d->pendingItems.clear();
	d->moreFiles->clearSelection();
	files->clearSelection();
	d->moreFiles->clear();
	files->clear();
	files->setSorting( -1 );

	QString s = d->url.toString( FALSE, FALSE );
	bool found = FALSE;
	for ( int i = 0; i < d->paths->count(); ++i ) {
#if defined(Q_WS_WIN)
	    if ( d->paths->text( i ).lower() == s.lower() ) {
#else
	    if ( d->paths->text( i ) == s ) {
#endif
		found = TRUE;
		d->paths->setCurrentItem( i );
		break;
	    }
	}
	if ( !found ) {
	    d->paths->insertItem( *openFolderIcon, s, -1 );
	    d->paths->setCurrentItem( d->paths->count() - 1 );
	}
	d->last = 0;
	d->hadDotDot = FALSE;

	if ( d->goBack && d->history.last() != d->url.toString() ) {
	    d->history.append( d->url.toString() );
	    if ( d->history.count() > 1 )
		d->goBack->setEnabled( TRUE );
	}
    }
}

void QFileDialog::urlFinished( QNetworkOperation *op )
{
    if ( !op )
	return;

#ifndef QT_NO_CURSOR
    if ( op->operation() == QNetworkProtocol::OpListChildren &&
	 d->cursorOverride ) {
	QApplication::restoreOverrideCursor();
	d->cursorOverride = FALSE;
    }
#endif

    if ( op->state() == QNetworkProtocol::StFailed ) {
	if ( d->paths->hasFocus() )
	    d->ignoreNextKeyPress = TRUE;

	if ( d->progressDia ) {
	    d->ignoreStop = TRUE;
	    d->progressDia->close();
	    delete d->progressDia;
	    d->progressDia = 0;
	}

	int ecode = op->errorCode();
	QMessageBox::critical( this, tr( "Error" ), op->protocolDetail() );

	if ( ecode == QNetworkProtocol::ErrListChildren || ecode == QNetworkProtocol::ErrParse ||
	     ecode == QNetworkProtocol::ErrUnknownProtocol || ecode == QNetworkProtocol::ErrLoginIncorrect ||
	     ecode == QNetworkProtocol::ErrValid || ecode == QNetworkProtocol::ErrHostNotFound ||
	     ecode == QNetworkProtocol::ErrFileNotExisting ) {
	    if (d->url != d->oldUrl) {
		d->url = d->oldUrl;
		rereadDir();
	    }
	} else {
	    // another error happened, no need to go back to last dir
	}
    } else if ( op->operation() == QNetworkProtocol::OpListChildren &&
		op == d->currListChildren ) {
	if ( !d->hadDotDot && !isRoot( d->url ) ) {
	    bool ok = TRUE;
#if defined(Q_WS_WIN)
	    if ( d->url.path().left( 2 ) == "//" )
		ok = FALSE;
#endif
	    if ( ok ) {
		QUrlInfo ui( d->url, ".." );
		ui.setName( ".." );
		ui.setDir( TRUE );
		ui.setFile( FALSE );
		ui.setSymLink( FALSE );
		ui.setSize( 0 );
		QValueList<QUrlInfo> lst;
		lst << ui;
		insertEntry( lst, 0 );
	    }
	}
	resortDir();
    } else if ( op->operation() == QNetworkProtocol::OpGet ) {
    } else if ( op->operation() == QNetworkProtocol::OpPut ) {
	rereadDir();
	if ( d->progressDia ) {
	    d->ignoreStop = TRUE;
	    d->progressDia->close();
	}
	delete d->progressDia;
	d->progressDia = 0;
    }

#if defined(Q_WS_WIN)
    if (d->oldPermissionLookup != qt_ntfs_permission_lookup)
        qt_ntfs_permission_lookup++;
#endif
}

void QFileDialog::dataTransferProgress( int bytesDone, int bytesTotal, QNetworkOperation *op )
{
    if ( !op )
	return;

    QString label;
    QUrl u( op->arg( 0 ) );
    if ( u.isLocalFile() ) {
	label = u.path();
    } else {
	label = QString( "%1 (on %2)" );
	label = label.arg( u.path() ).arg( u.host() );
    }

    if ( !d->progressDia ) {
	if ( bytesDone < bytesTotal) {
	    d->ignoreStop = FALSE;
	    d->progressDia = new QFDProgressDialog( this, label, bytesTotal );
	    connect( d->progressDia, SIGNAL( cancelled() ),
		     this, SLOT( stopCopy() ) );
	    d->progressDia->show();
	} else
	    return;
    }

    if ( d->progressDia ) {
	if ( op->operation() == QNetworkProtocol::OpGet ) {
	    if ( d->progressDia ) {
		d->progressDia->setReadProgress( bytesDone );
	    }
	} else if ( op->operation() == QNetworkProtocol::OpPut ) {
	    if ( d->progressDia ) {
		d->progressDia->setWriteLabel( label );
		d->progressDia->setWriteProgress( bytesDone );
	    }
	} else {
	    return;
	}
    }
}

void QFileDialog::insertEntry( const QValueList<QUrlInfo> &lst, QNetworkOperation *op )
{
    if ( op && op->operation() == QNetworkProtocol::OpListChildren &&
	 op != d->currListChildren )
	return;
    QValueList<QUrlInfo>::ConstIterator it = lst.begin();
    for ( ; it != lst.end(); ++it ) {
	const QUrlInfo &inf = *it;
	if ( d->mode == DirectoryOnly && !inf.isDir() )
	    continue;
	if ( inf.name() == ".." ) {
	    d->hadDotDot = TRUE;
	    if ( isRoot( d->url ) )
		continue;
#if defined(Q_WS_WIN)
	    if ( d->url.path().left( 2 ) == "//" )
		continue;
#endif
	} else if ( inf.name() == "." )
	    continue;

#if defined(Q_WS_WIN)
	// Workaround a Windows bug, '..' is apparantly hidden in directories
	// that are one level away from root
	if ( !bShowHiddenFiles && inf.name() != ".." ) {
	    if ( d->url.isLocalFile() ) {
		QString file = d->url.path();
		if ( !file.endsWith( "/" ) )
		    file.append( "/" );
	        file += inf.name();
		QT_WA( {
		    if ( GetFileAttributesW( (TCHAR*)file.ucs2() ) & FILE_ATTRIBUTE_HIDDEN )
			continue;
		} , {
		    if ( GetFileAttributesA( file.local8Bit() ) & FILE_ATTRIBUTE_HIDDEN )
			continue;
		} );
	    } else {
		if ( inf.name() != ".." && inf.name()[0] == QChar('.') )
		    continue;
	    }
	}
#else
 	if ( !bShowHiddenFiles && inf.name() != ".." ) {
 	    if ( inf.name()[ 0 ] == QChar( '.' ) )
 		continue;
 	}
#endif
	if ( !d->url.isLocalFile() ) {
	    QFileDialogPrivate::File * i = 0;
	    QFileDialogPrivate::MCItem *i2 = 0;
	    i = new QFileDialogPrivate::File( d, &inf, files );
	    i2 = new QFileDialogPrivate::MCItem( d->moreFiles, i );

	    if ( d->mode == ExistingFiles && inf.isDir() ||
		( isDirectoryMode( d->mode ) && inf.isFile() ) ) {
		i->setSelectable( FALSE );
		i2->setSelectable( FALSE );
	    }

	    i->i = i2;
	}

	d->sortedList.append( new QUrlInfo( inf ) );
    }
}

void QFileDialog::removeEntry( QNetworkOperation *op )
{
    if ( !op )
	return;

    QUrlInfo *i = 0;
    QListViewItemIterator it( files );
    bool ok1 = FALSE, ok2 = FALSE;
    for ( i = d->sortedList.first(); it.current(); ++it, i = d->sortedList.next() ) {
        QString encName = QFileDialogPrivate::encodeFileName(
            ( (QFileDialogPrivate::File*)it.current() )->info.name() );
	if ( encName == op->arg( 0 ) ) {
	    d->pendingItems.removeRef( (QFileDialogPrivate::File*)it.current() );
	    delete ( (QFileDialogPrivate::File*)it.current() )->i;
	    delete it.current();
	    ok1 = TRUE;
	}
	if ( i && i->name() == op->arg( 0 ) ) {
	    d->sortedList.removeRef( i );
	    i = d->sortedList.prev();
	    ok2 = TRUE;
	}
	if ( ok1 && ok2 )
	    break;
    }
}

void QFileDialog::itemChanged( QNetworkOperation *op )
{
    if ( !op )
	return;

    QUrlInfo *i = 0;
    QListViewItemIterator it1( files );
    bool ok1 = FALSE, ok2 = FALSE;
    // first check whether the new file replaces an existing file.
    for ( i = d->sortedList.first(); it1.current(); ++it1, i = d->sortedList.next() ) {
	if ( ( (QFileDialogPrivate::File*)it1.current() )->info.name() == op->arg( 1 ) ) {
	    delete ( (QFileDialogPrivate::File*)it1.current() )->i;
	    delete it1.current();
	    ok1 = TRUE;
	}
	if ( i && i->name() == op->arg( 1 ) ) {
	    d->sortedList.removeRef( i );
	    i = d->sortedList.prev();
	    ok2 = TRUE;
	}
	if ( ok1 && ok2 )
	    break;
    }

    i = 0;
    QListViewItemIterator it( files );
    ok1 = FALSE;
    ok2 = FALSE;
    for ( i = d->sortedList.first(); it.current(); ++it, i = d->sortedList.next() ) {
	if ( ( (QFileDialogPrivate::File*)it.current() )->info.name() == op->arg( 0 ) ) {
	    ( (QFileDialogPrivate::File*)it.current() )->info.setName( op->arg( 1 ) );
	    ok1 = TRUE;
	}
	if ( i && i->name() == op->arg( 0 ) ) {
	    i->setName( op->arg( 1 ) );
	    ok2 = TRUE;
	}
	if ( ok1 && ok2 )
	    break;
    }

    resortDir();
}

/*!
  \property QFileDialog::infoPreview

  \brief whether the file dialog can provide preview information about
  the currently selected file

  The default is FALSE.
*/
bool QFileDialog::isInfoPreviewEnabled() const
{
    return d->infoPreview;
}

void QFileDialog::setInfoPreviewEnabled( bool info )
{
    if ( info == d->infoPreview )
	return;
    d->geometryDirty = TRUE;
    d->infoPreview = info;
    updateGeometries();
}


/*!
  \property QFileDialog::contentsPreview

  \brief whether the file dialog can provide a contents preview of the
  currently selected file

  The default is FALSE.

  \sa setContentsPreview() setInfoPreviewEnabled()
*/
// ### improve the above documentation: how is the preview done, how can I add
// support for customized preview, etc.

bool QFileDialog::isContentsPreviewEnabled() const
{
    return d->contentsPreview;
}

void QFileDialog::setContentsPreviewEnabled( bool contents )
{
    if ( contents == d->contentsPreview )
	return;
    d->geometryDirty = TRUE;
    d->contentsPreview = contents;
    updateGeometries();
}


/*!
  Sets the widget to be used for displaying information about the file
  to the widget \a w and a preview of that information to the
  QFilePreview \a preview.

  Normally you would create a preview widget that derives from both QWidget and
  QFilePreview, so you should pass the same widget twice. If you
  don't, you must remember to delete the preview object in order to
  avoid memory leaks.

  \code
    class Preview : public QLabel, public QFilePreview
    {
    public:
	Preview( QWidget *parent=0 ) : QLabel( parent ) {}

	void previewUrl( const QUrl &u )
	{
	    QString path = u.path();
	    QPixmap pix( path );
	    if ( pix.isNull() )
		setText( "This is not a pixmap" );
	    else
		setText( "This is a pixmap" );
	}
    };

  //...

  int main( int argc, char** argv )
  {
    Preview* p = new Preview;

    QFileDialog* fd = new QFileDialog( this );
    fd->setInfoPreviewEnabled( TRUE );
    fd->setInfoPreview( p, p );
    fd->setPreviewMode( QFileDialog::Info );
    fd->show();
  }

  \endcode

  \sa setContentsPreview(), setInfoPreviewEnabled(), setPreviewMode()

*/

void QFileDialog::setInfoPreview( QWidget *w, QFilePreview *preview )
{
    if ( !w || !preview )
	return;

    if ( d->infoPreviewWidget ) {
	d->preview->removeWidget( d->infoPreviewWidget );
	delete d->infoPreviewWidget;
    }
    d->infoPreviewWidget = w;
    d->infoPreviewer = preview;
    w->reparent( d->preview, 0, QPoint( 0, 0 ) );
}

/*!
  Sets the widget to be used for displaying the contents of the file
  to the widget \a w and a preview of those contents to the
  QFilePreview \a preview.

  Normally you would create a preview widget that derives from both QWidget and
  QFilePreview, so you should pass the same widget twice. If you
  don't, you must remember to delete the preview object in order to
  avoid memory leaks.

  \code
    class Preview : public QLabel, public QFilePreview
    {
    public:
	Preview( QWidget *parent=0 ) : QLabel( parent ) {}

	void previewUrl( const QUrl &u )
	{
	    QString path = u.path();
	    QPixmap pix( path );
	    if ( pix.isNull() )
		setText( "This is not a pixmap" );
	    else
		setPixmap( pix );
	}
    };

  //...

  int main( int argc, char** argv )
  {
    Preview* p = new Preview;

    QFileDialog* fd = new QFileDialog( this );
    fd->setContentsPreviewEnabled( TRUE );
    fd->setContentsPreview( p, p );
    fd->setPreviewMode( QFileDialog::Contents );
    fd->show();
  }
  \endcode

  \sa setContentsPreviewEnabled(), setInfoPreview(), setPreviewMode()
*/

void QFileDialog::setContentsPreview( QWidget *w, QFilePreview *preview )
{
    if ( !w || !preview )
	return;

    if ( d->contentsPreviewWidget ) {
	d->preview->removeWidget( d->contentsPreviewWidget );
	delete d->contentsPreviewWidget;
    }
    d->contentsPreviewWidget = w;
    d->contentsPreviewer = preview;
    w->reparent( d->preview, 0, QPoint( 0, 0 ) );
}

/*!
  Re-sorts the displayed directory.

  \sa rereadDir()
*/

void QFileDialog::resortDir()
{
    d->mimeTypeTimer->stop();
    d->pendingItems.clear();

    QFileDialogPrivate::File *item = 0;
    QFileDialogPrivate::MCItem *item2 = 0;

    d->sortedList.sort();

    if ( files->childCount() > 0 || d->moreFiles->count() > 0 ) {
	d->moreFiles->clear();
	files->clear();
	d->last = 0;
	files->setSorting( -1 );
    }

    QUrlInfo *i = sortAscending ? d->sortedList.first() : d->sortedList.last();
    for ( ; i; i = sortAscending ? d->sortedList.next() : d->sortedList.prev() ) {
	item = new QFileDialogPrivate::File( d, i, files );
	item2 = new QFileDialogPrivate::MCItem( d->moreFiles, item, item2 );
	item->i = item2;
	d->pendingItems.append( item );
	if ( d->mode == ExistingFiles && item->info.isDir() ||
	    ( isDirectoryMode( d->mode ) && item->info.isFile() ) ) {
	    item->setSelectable( FALSE );
	    item2->setSelectable( FALSE );
	}
    }

    // ##### As the QFileIconProvider only support QFileInfo and no
    // QUrlInfo it can be only used for local files at the moment. In
    // 3.0 we have to change the API of QFileIconProvider to work on
    // QUrlInfo so that also remote filesystems can be show mime-type
    // specific icons.
    if ( d->url.isLocalFile() )
	d->mimeTypeTimer->start( 0 );
}

/*!
  Stops the current copy operation.
*/

void QFileDialog::stopCopy()
{
    if ( d->ignoreStop )
	return;

    d->url.blockSignals( TRUE );
    d->url.stop();
    if ( d->progressDia ) {
	d->ignoreStop = TRUE;
	QTimer::singleShot( 100, this, SLOT( removeProgressDia() ) );
    }
    d->url.blockSignals( FALSE );
}

/*!
  \internal
*/

void QFileDialog::removeProgressDia()
{
    if ( d->progressDia )
	delete d->progressDia;
    d->progressDia = 0;
}

/*!
  \internal
*/

void QFileDialog::doMimeTypeLookup()
{
    if ( !iconProvider() ) {
	d->pendingItems.clear();
	d->mimeTypeTimer->stop();
	return;
    }

    d->mimeTypeTimer->stop();
    if ( d->pendingItems.count() == 0 ) {
	return;
    }

    QRect r;
    QFileDialogPrivate::File *item = d->pendingItems.first();
    if ( item ) {
	QFileInfo fi;
	if ( d->url.isLocalFile() ) {
	    fi.setFile( QUrl( d->url.path(), QFileDialogPrivate::encodeFileName( item->info.name() ) ).path( FALSE ) );
	} else
	    fi.setFile( item->info.name() ); // #####
	const QPixmap *p = iconProvider()->pixmap( fi );
	if ( p && p != item->pixmap( 0 ) &&
	     ( !item->pixmap( 0 ) || p->serialNumber() != item->pixmap( 0 )->serialNumber() ) &&
	     p != fifteenTransparentPixels ) {
	    item->hasMimePixmap = TRUE;

	    // evil hack to avoid much too much repaints!
	    QGuardedPtr<QFileDialog> that( this ); // this may be deleted by an event handler
	    qApp->processEvents();
	    if ( that.isNull() )
		return;
	    files->setUpdatesEnabled( FALSE );
	    files->viewport()->setUpdatesEnabled( FALSE );
	    if ( item != d->pendingItems.first() )
		return;
	    item->setPixmap( 0, *p );
	    qApp->processEvents();
	    if ( that.isNull() )
		return;
	    files->setUpdatesEnabled( TRUE );
	    files->viewport()->setUpdatesEnabled( TRUE );

	    if ( files->isVisible() ) {
		QRect ir( files->itemRect( item ) );
		if ( ir != QRect( 0, 0, -1, -1 ) ) {
		    r = r.unite( ir );
		}
	    } else {
		QRect ir( d->moreFiles->itemRect( item->i ) );
		if ( ir != QRect( 0, 0, -1, -1 ) ) {
		    r = r.unite( ir );
		}
	    }
	}
	if ( d->pendingItems.count() )
	    d->pendingItems.removeFirst();
    }

    if ( d->moreFiles->isVisible() ) {
	d->moreFiles->viewport()->repaint( r, FALSE );
    } else {
	files->viewport()->repaint( r, FALSE );
    }

    if ( d->pendingItems.count() )
	d->mimeTypeTimer->start( 0 );
    else if ( d->moreFiles->isVisible() )
	d->moreFiles->triggerUpdate( TRUE );
}

/*!
  If \a b is TRUE then all the files in the current directory are selected;
  otherwise, they are deselected.
*/

void QFileDialog::selectAll( bool b )
{
    if ( d->mode != ExistingFiles )
	return;
    d->moreFiles->selectAll( b );
    files->selectAll( b );
}

void QFileDialog::goBack()
{
    if ( !d->goBack || !d->goBack->isEnabled() )
	return;
    d->history.remove( d->history.last() );
    if ( d->history.count() < 2 )
	d->goBack->setEnabled( FALSE );
    setUrl( d->history.last() );
}

// a class with wonderfully inflexible flexibility. why doesn't it
// just subclass QWidget in the first place? 'you have to derive your
// preview widget from QWidget and from this class' indeed.

/*!
  \class QFilePreview qfiledialog.h
  \ingroup misc
  \brief The QFilePreview class provides file previewing in QFileDialog.

  This class is an abstract base class which is used to implement
  widgets that can display a preview of a file in a QFileDialog.

  You must derive the preview widget from both QWidget and from this
  class. Then you must reimplement this class's previewUrl() function,
  which is called by the file dialog if the preview of a file
  (specified as a URL) should be shown.

  See also QFileDialog::setPreviewMode(), QFileDialog::setContentsPreview(),
  QFileDialog::setInfoPreview(), QFileDialog::setInfoPreviewEnabled(),
  QFileDialog::setContentsPreviewEnabled().

  For an example of a preview widget see qt/examples/qdir/qdir.cpp.
*/

/*!
  Constructs the QFilePreview.
*/

QFilePreview::QFilePreview()
{
}

/*!
  \fn void QFilePreview::previewUrl( const QUrl &url )

  This function is called by QFileDialog if a preview
  for the \a url should be shown. Reimplement this
  function to provide file previewing.
*/


#include "qfiledialog.moc"

#endif
