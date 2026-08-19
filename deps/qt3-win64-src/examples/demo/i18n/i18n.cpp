#include "i18n.h"
#include "wrapper.h"
#include "../textdrawing/textedit.h"

#include <qaction.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qworkspace.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qlabel.h>
#include <qtextedit.h>


static int windowIdNumber = 5000;
static bool firstShow = TRUE;


I18nDemo::I18nDemo(QWidget *parent, const char *name)
    : QMainWindow(parent, name, 0), lastwrapper(0)
{
    initActions();
    initMenuBar();

    QVBox *box = new QVBox(this);
    box->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    box->setMargin( 1 );
    box->setLineWidth( 1 );

    workspace = new QWorkspace(box);
    connect(workspace, SIGNAL(windowActivated(QWidget *)),
	    SLOT(windowActivated(QWidget *)));
    workspace->setBackgroundMode(PaletteMid);

    setCentralWidget(box);
}


I18nDemo::~I18nDemo()
{
}


void I18nDemo::initActions()
{
    actionClose = new QAction(tr("Close the current window."),
			      tr("Close"),
			      CTRL + Key_F4,
			      this);
    connect(actionClose, SIGNAL(activated()), SLOT(closeSlot()));

    actionCloseAll = new QAction(tr("Close all opened windows."),
				 tr("Close All"),
				 0,
				 this);
    connect(actionCloseAll, SIGNAL(activated()), SLOT(closeAllSlot()));

    actionTile = new QAction(tr("Tile opened windows."),
			     tr("Tile"),
			     0,
			     this);
    connect(actionTile, SIGNAL(activated()), SLOT(tileSlot()));

    actionCascade = new QAction(tr("Cascade opened windows."),
				tr("Cascade"),
				0,
				this);
    connect(actionCascade, SIGNAL(activated()), SLOT(cascadeSlot()));
}


void I18nDemo::initMenuBar()
{
    newMenu = new QPopupMenu(this);
    connect(newMenu, SIGNAL(activated(int)), SLOT(newSlot(int)));

    newMenu->insertItem("&English", 0);
    newMenu->insertItem("&Japanese", 1);
    newMenu->insertItem("&Korean", 2);
    newMenu->insertItem("&Norwegian", 3);

    windowMenu = new QPopupMenu(this);
    connect(windowMenu, SIGNAL(activated(int)), SLOT(windowSlot(int)));

    windowMenu->setCheckable(TRUE);

    actionClose->addTo(windowMenu);
    actionCloseAll->addTo(windowMenu);
    windowMenu->insertSeparator();
    actionTile->addTo(windowMenu);
    actionCascade->addTo(windowMenu);
    windowMenu->insertSeparator();

    menuBar()->insertItem(tr("&New"), newMenu);
    menuBar()->insertItem(tr("&Window"), windowMenu);
}


void I18nDemo::newSlot(int id)
{
    QString qmfile;
    switch (id) {
    default:
    case 0: qmfile = "i18n/en.qm"; break;
    case 1: qmfile = "i18n/ja.qm"; break;
    case 2: qmfile = "i18n/ko.qm"; break;
    case 3: qmfile = "i18n/no.qm"; break;
    }

    if (lastwrapper) {
	qApp->removeTranslator(&lastwrapper->translator);
	lastwrapper = 0;
    }

    Wrapper *wrapper = new Wrapper(workspace, windowIdNumber);
    wrapper->translator.load(qmfile, ".");

    qApp->installTranslator(&wrapper->translator);

    connect(wrapper, SIGNAL(destroyed()), SLOT(wrapperDead()));
    wrapper->setCaption(tr("--language--"));

    TextEdit *te = new TextEdit(wrapper);
    te->layout()->setResizeMode( QLayout::FreeResize );
    te->setMinimumSize(500, 400);
    te->fileNew();
    te->currentEditor()->
	setText(tr("<h3>About Qt</h3>"
		   "<p>This program uses Qt version %1, a multiplatform C++ "
		   "GUI toolkit from Trolltech. Qt provides single-source "
		   "portability across Windows 95/98/NT/2000, Mac OS X, Linux, Solaris, "
		   "HP-UX and many other versions of Unix with X11.</p>"
		   "<p>See <tt>http://www.trolltech.com/qt/</tt> for more "
		   "information.</p>").arg(QT_VERSION_STR));

    qApp->removeTranslator(&wrapper->translator);

    te->show();
    wrapper->show();

    windowMenu->insertItem(wrapper->caption(), wrapper->id);
    windowMenu->setItemChecked(wrapper->id, TRUE);
    lastwrapper = wrapper;

    windowIdNumber++;
}


void I18nDemo::windowSlot(int id)
{
    if (id < 5000)
	return;

    QWidgetList list = workspace->windowList();
    Wrapper *wrapper = (Wrapper *) list.first();
    while (wrapper) {
	if (wrapper->id == id) {
	    wrapper->setFocus();
	    break;
	}

	wrapper = (Wrapper *) list.next();
    }
}


void I18nDemo::windowActivated(QWidget *w)
{
    if (lastwrapper) {
	qApp->removeTranslator(&lastwrapper->translator);
	windowMenu->setItemChecked(lastwrapper->id, FALSE);
    }

    if (! w) {
	lastwrapper = 0;
	return;
    }

    Wrapper *wrapper = (Wrapper *) w;

    windowMenu->setItemChecked(wrapper->id, TRUE);
    lastwrapper = wrapper;
}


void I18nDemo::closeSlot()
{
    QWidget *w = workspace->activeWindow();
    delete w;
}


void I18nDemo::closeAllSlot()
{
    QWidget *w;
    while ((w = workspace->activeWindow()))
	w->close(TRUE);
}


void I18nDemo::tileSlot()
{
    workspace->tile();
}


void I18nDemo::cascadeSlot()
{
    workspace->cascade();
}


void I18nDemo::wrapperDead()
{
    Wrapper *w = (Wrapper *) sender();

    if (w == lastwrapper) {
	qApp->removeTranslator(&w->translator);
	lastwrapper = 0;
    }

    windowMenu->removeItem(w->id);
}


void I18nDemo::showEvent(QShowEvent *)
{
    if (firstShow) {
	newSlot(1);
	firstShow = FALSE;
	return;
    }

    if (! lastwrapper)
	return;

    qApp->installTranslator(&lastwrapper->translator);
}


void I18nDemo::hideEvent(QHideEvent *)
{
    if (! lastwrapper)
	return;

    qApp->removeTranslator(&lastwrapper->translator);
}
