/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qfileinfo.h>
#include <qtoolbutton.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qstyle.h>
#include <qtimer.h>

#include "config.h"

static QString reduceLabelLength( const QString &s )
{
    uint maxLength = 16;
    QString str = s;
    if ( str.length() < maxLength )
	return str;
    str = str.left( maxLength - 3 );
    str += "...";
    return str;
}

void TabbedBrowser::forward()
{
    currentBrowser()->forward();
}

void TabbedBrowser::backward()
{
    currentBrowser()->backward();
}

void TabbedBrowser::setSource( const QString &ref )
{
    HelpWindow * win = currentBrowser();
    win->setSource(ref);
}

void TabbedBrowser::reload()
{
    currentBrowser()->reload();
}

void TabbedBrowser::home()
{
    currentBrowser()->home();
}

HelpWindow * TabbedBrowser::currentBrowser()
{
    return (HelpWindow*) tab->currentPage();
}

void TabbedBrowser::nextTab()
{
    if( tab->currentPageIndex()<=tab->count()-1 )
	tab->setCurrentPage( tab->currentPageIndex()+1 );
}

void TabbedBrowser::previousTab()
{
    int idx = tab->currentPageIndex()-1;
    if( idx>=0 )
	tab->setCurrentPage( idx );
}

HelpWindow *TabbedBrowser::createHelpWindow(const QString &title)
{
    MainWindow *mainWin = mainWindow();
    HelpWindow *win = new HelpWindow( mainWin, this, "qt_assistant_helpwin" );
    win->setFont( browserFont() );
    win->setPalette( palette() );
    win->setLinkUnderline( tabLinkUnderline );
    win->setStyleSheet( tabStyleSheet );
    win->setMimeSourceFactory( mimeSourceFactory );
    tab->addTab(win, reduceLabelLength(title));
    connect( win, SIGNAL( highlighted( const QString & ) ),
	     (const QObject*) (mainWin->statusBar()), SLOT( message( const QString & ) ) );
    connect( win, SIGNAL( chooseWebBrowser() ), mainWin, SLOT( showWebBrowserSettings() ) );
    connect( win, SIGNAL( backwardAvailable(bool) ),
	     mainWin, SLOT( backwardAvailable(bool) ) );
    connect( win, SIGNAL( forwardAvailable(bool) ),
	     mainWin, SLOT( forwardAvailable(bool) ) );
    connect( win, SIGNAL( sourceChanged(const QString &) ), this, SLOT( sourceChanged() ));

    tab->cornerWidget( Qt::TopRight )->setEnabled( tab->count() > 1 );
    return win;
}

HelpWindow *TabbedBrowser::newBackgroundTab( const QString &url )
{
    HelpWindow *win = createHelpWindow(url);
    return win;
}

void TabbedBrowser::newTab( const QString &lnk )
{
    QString link( lnk );
    if( link.isNull() ) {
	HelpWindow *w = currentBrowser();
	if( w )
	    link = w->source();
    }
    HelpWindow *win = createHelpWindow(link);
    tab->showPage( win );
    if( !link.isNull() ) {
 	win->setSource( link );
    }
}

void TabbedBrowser::zoomIn()
{
    currentBrowser()->zoomIn();
}

void TabbedBrowser::zoomOut()
{
    currentBrowser()->zoomOut();
}

void TabbedBrowser::init()
{
    tabLinkUnderline = FALSE;
    tabStyleSheet = new QStyleSheet( QStyleSheet::defaultSheet() );
    lastCurrentTab = 0;
    while( tab->count() )
	tab->removePage( tab->page(0) );

    mimeSourceFactory = new QMimeSourceFactory();
    mimeSourceFactory->setExtensionType("html","text/html;charset=UTF-8");
    mimeSourceFactory->setExtensionType("htm","text/html;charset=UTF-8");
    mimeSourceFactory->setExtensionType("png", "image/png" );
    mimeSourceFactory->setExtensionType("jpg", "image/jpeg" );
    mimeSourceFactory->setExtensionType("jpeg", "image/jpeg" );
    setMimePath( Config::configuration()->mimePaths() );

    connect( tab, SIGNAL( currentChanged( QWidget* ) ),
	     this, SLOT( transferFocus() ) );
    connect( tab, SIGNAL( currentChanged( QWidget* ) ),
        mainWindow(), SLOT( browserTabChanged() ) );

    QTabBar *tabBar = (QTabBar*)tab->child( 0, "QTabBar", FALSE );
    int m = ( tabBar ? style().pixelMetric( QStyle::PM_TabBarTabVSpace, (QWidget*)tabBar )
	      + style().pixelMetric( QStyle::PM_TabBarBaseHeight, (QWidget*)tabBar ) : 0 );
    int s = tab->height() - m;

    // workaround for sgi style
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Button, pal.active().background());
    pal.setColor(QPalette::Disabled, QColorGroup::Button, pal.disabled().background());
    pal.setColor(QPalette::Inactive, QColorGroup::Button, pal.inactive().background());

    QToolButton *newTabButton = new QToolButton( this );
    newTabButton->setPalette(pal);
    tab->setCornerWidget( newTabButton, Qt::TopLeft );
    newTabButton->setCursor( arrowCursor );
    newTabButton->setAutoRaise( TRUE );
    newTabButton->setPixmap( QPixmap::fromMimeSource( "addtab.png" ) );
    newTabButton->setFixedSize( s, s );
    QObject::connect( newTabButton, SIGNAL( clicked() ), this, SLOT( newTab() ) );
    QToolTip::add( newTabButton, tr( "Add page" ) );

    QToolButton *closeTabButton = new QToolButton( this );
    closeTabButton->setPalette(pal);
    tab->setCornerWidget( closeTabButton, Qt::TopRight );
    closeTabButton->setCursor( arrowCursor );
    closeTabButton->setAutoRaise( TRUE );
    QIconSet is( QPixmap::fromMimeSource( "closetab.png") );
    QPixmap disabledPix = QPixmap::fromMimeSource( "d_closetab.png" );
    is.setPixmap( disabledPix, QIconSet::Small, QIconSet::Disabled );
    closeTabButton->setIconSet( is );
    closeTabButton->setFixedSize( s, s );
    QObject::connect( closeTabButton, SIGNAL( clicked() ), this, SLOT( closeTab() ) );
    QToolTip::add( closeTabButton, tr( "Close page" ) );
    closeTabButton->setEnabled( FALSE );
}

void TabbedBrowser::setMimePath( QStringList lst )
{
    mimeSourceFactory->setFilePath( lst );
}

void TabbedBrowser::setMimeExtension( const QString &ext )
{
    mimeSourceFactory->setExtensionType( "html", ext );
    mimeSourceFactory->setExtensionType( "htm", ext );
}

void TabbedBrowser::updateTitle( const QString &title )
{
    tab->changeTab( currentBrowser(), title );
}

MainWindow* TabbedBrowser::mainWindow()
{
    return (MainWindow*) parent();
}

void TabbedBrowser::newTab()
{
    newTab( QString::null );
}

void TabbedBrowser::transferFocus()
{
    if( currentBrowser() ) {
	currentBrowser()->setFocus();
    }
    mainWindow()->setCaption(Config::configuration()->title()
			     + " - " + currentBrowser()->documentTitle());
}

void TabbedBrowser::initHelpWindow( HelpWindow * /*win*/ )
{
}

void TabbedBrowser::setup()
{
    Config *config = Config::configuration();

    QFont fnt( font() );
    QFontInfo fntInfo( fnt );
    fnt.setFamily( config->fontFamily() );
    fnt.setPointSize( config->fontSize() );
    setBrowserFont( fnt );

    QPalette pal = palette();
    QColor lc( config->linkColor() );
    pal.setColor( QPalette::Active, QColorGroup::Link, lc );
    pal.setColor( QPalette::Inactive, QColorGroup::Link, lc );
    pal.setColor( QPalette::Disabled, QColorGroup::Link, lc );
    setPalette( pal );

    tabLinkUnderline = config->isLinkUnderline();

    QString family = config->fontFixedFamily();
    tabStyleSheet->item( "pre" )->setFontFamily( family );
    tabStyleSheet->item( "code" )->setFontFamily( family );
    tabStyleSheet->item( "tt" )->setFontFamily( family );

    newTab( QString::null );
}

void TabbedBrowser::setLinkUnderline( bool uline )
{
    if( uline==tabLinkUnderline )
	return;
    tabLinkUnderline = uline;
    int cnt = tab->count();
    for( int i=0; i<cnt; i++ )
	( (QTextBrowser*) tab->page( i ) )->setLinkUnderline( tabLinkUnderline );
}

QFont TabbedBrowser::browserFont()
{
    return tabFont;
}

void TabbedBrowser::setBrowserFont( const QFont &fnt )
{
    if( tabFont == fnt )
	return;
    tabFont = fnt;
    int cnt = tab->count();
    for( int i=0; i<cnt; i++ )
	( (QTextBrowser*) tab->page( i ) )->setFont( fnt );
}

void TabbedBrowser::setPalette( const QPalette &pal )
{
    if( palette()==pal )
	return;
    QWidget::setPalette( pal );
    int cnt = tab->count();
    for( int i=0; i<cnt; i++ )
	( (QTextBrowser*) tab->page( i ) )->setPalette( pal );
}

QStyleSheet* TabbedBrowser::styleSheet()
{
    return tabStyleSheet;
}

bool TabbedBrowser::linkUnderline()
{
    return tabLinkUnderline;
}

void TabbedBrowser::copy()
{
    currentBrowser()->copy();
}

void TabbedBrowser::closeTab()
{
    if( tab->count()==1 )
	return;
    HelpWindow *win = currentBrowser();
    tab->removePage( win );
    QTimer::singleShot(0, win, SLOT(deleteLater()));
    tab->cornerWidget( Qt::TopRight )->setEnabled( tab->count() > 1 );
}

QStringList TabbedBrowser::sources()
{
    QStringList lst;
    int cnt = tab->count();
    for( int i=0; i<cnt; i++ ) {
	lst.append( ( (QTextBrowser*) tab->page(i) )->source() );
    }
    return lst;
}

QPtrList<HelpWindow> TabbedBrowser::browsers() const
{
    QPtrList<HelpWindow> list;
    for (int i=0; i<tab->count(); ++i) {
	Q_ASSERT(::qt_cast<HelpWindow*>(tab->page(i)));
	list.append(::qt_cast<HelpWindow*>(tab->page(i)));
    }
    return list;
}

void TabbedBrowser::sourceChanged()
{
    HelpWindow *win = ::qt_cast<HelpWindow *>( QObject::sender() );
    Q_ASSERT( win );
    QString docTitle(win->documentTitle());
    if (docTitle.isEmpty())
	docTitle = "...";
    setTitle(win, docTitle);
}

void TabbedBrowser::setTitle(HelpWindow *win, const QString &title)
{
    tab->setTabLabel(win, reduceLabelLength(title));
    if (win == currentBrowser())
	mainWindow()->setCaption(Config::configuration()->title() + " - " + title);
}

